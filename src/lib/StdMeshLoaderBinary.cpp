/*
 * OpenClonk, http://www.openclonk.org
 *
 * Copyright (c) 2010-2013, The OpenClonk Team and contributors
 *
 * Distributed under the terms of the ISC license; see accompanying file
 * "COPYING" for details.
 *
 * "Clonk" is a registered trademark of Matthes Bender, used with permission.
 * See accompanying file "TRADEMARK" for details.
 *
 * To redistribute this file separately, substitute the full license texts
 * for the above references.
 */

// A loader for the OGRE .mesh binary file format

#include "C4Include.h"
#include "StdMesh.h"
#include "StdMeshLoader.h"
#include "StdMeshLoaderBinaryChunks.h"
#include "StdMeshLoaderDataStream.h"
#include "StdMeshMaterial.h"
#include <cassert>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <C4DefList.h>
#include <C4Def.h>

namespace
{
	bool VertexDeclarationIsSane(const boost::ptr_vector<Ogre::Mesh::ChunkGeometryVertexDeclElement> &decl, const char *filename)
	{
		bool semanticSeen[Ogre::Mesh::ChunkGeometryVertexDeclElement::VDES_MAX + 1] = { false };
		BOOST_FOREACH(Ogre::Mesh::ChunkGeometryVertexDeclElement element, decl)
		{
			switch (element.semantic)
			{
			case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDES_Texcoords:
				// FIXME: The Ogre format supports denoting multiple texture coordinates, but the rendering code only supports one
				// currently only the first set is read, any additional ones are ignored
				break;
			case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDES_Position:
			case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDES_Normals:
			case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDES_Diffuse:
			case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDES_Specular:
				// Only one set of each of these elements allowed
				if (semanticSeen[element.semantic])
					return false;
				break;
			default:
				// We ignore unhandled element semantics.
				break;
			}
			semanticSeen[element.semantic] = true;
		}
		return true;
	}

	template<size_t N>
	void ReadNormalizedVertexData(float (&dest)[N], const char *source, Ogre::Mesh::ChunkGeometryVertexDeclElement::Type vdet)
	{
		BOOST_STATIC_ASSERT(N >= 4);
		dest[0] = dest[1] = dest[2] = 0; dest[3] = 1;
		switch (vdet)
		{
			// All VDET_Float* fall through.
		case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDET_Float4:
			dest[3] = *reinterpret_cast<const float*>(source + sizeof(float) * 3);
		case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDET_Float3:
			dest[2] = *reinterpret_cast<const float*>(source + sizeof(float) * 2);
		case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDET_Float2:
			dest[1] = *reinterpret_cast<const float*>(source + sizeof(float) * 1);
		case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDET_Float1:
			dest[0] = *reinterpret_cast<const float*>(source + sizeof(float) * 0);
			break;
		case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDET_Color_ABGR:
			dest[3] = source[0] / 255.0f;
			for (int i = 0; i < 3; ++i)
				dest[i] = source[3 - i] / 255.0f;
			break;
		case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDET_Color_ARGB:
			dest[3] = source[0] / 255.0f;
			for (int i = 0; i < 3; ++i)
				dest[i] = source[i + 1] / 255.0f;
			break;
		default:
			assert(!"Unexpected enum value");
			break;
		}
	}

	std::vector<StdSubMesh::Vertex> ReadSubmeshGeometry(const Ogre::Mesh::ChunkGeometry &geo, const char *filename)
	{
		if (!VertexDeclarationIsSane(geo.vertexDeclaration, filename))
			throw Ogre::Mesh::InvalidVertexDeclaration();

		// Get maximum size of a vertex according to the declaration
		std::map<int, size_t> max_offset;
		BOOST_FOREACH(const Ogre::Mesh::ChunkGeometryVertexDeclElement &el, geo.vertexDeclaration)
		{
			size_t elsize = 0;
			switch (el.type)
			{
			case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDET_Float1: elsize = sizeof(float) * 1; break;
			case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDET_Float2: elsize = sizeof(float) * 2; break;
			case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDET_Float3: elsize = sizeof(float) * 3; break;
			case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDET_Float4: elsize = sizeof(float) * 4; break;
			case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDET_Color_ABGR:
			case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDET_Color_ARGB: elsize = sizeof(uint8_t) * 4; break;
			default: assert(!"Unexpected enum value"); break;
			}
			max_offset[el.source] = std::max<size_t>(max_offset[el.source], el.offset + elsize);
		}

		// Generate array of vertex buffer cursors
		std::map<int, const char *> cursors;
		BOOST_FOREACH(const Ogre::Mesh::ChunkGeometryVertexBuffer &buf, geo.vertexBuffers)
		{
			if (cursors.find(buf.index) != cursors.end())
				throw Ogre::MultipleSingletonChunks("Multiple vertex buffers were bound to the same stream");
			cursors[buf.index] = static_cast<const char *>(buf.data->data);
			// Check that the vertices don't overlap
			if (buf.vertexSize < max_offset[buf.index])
				throw Ogre::InsufficientData("Vertices overlapping");
			// Check that the vertex buffer has enough room for all vertices
			if (buf.GetSize() < (geo.vertexCount - 1) * buf.vertexSize + max_offset[buf.index])
				throw Ogre::InsufficientData("Vertex buffer too small");
			max_offset.erase(buf.index);
		}

		if (!max_offset.empty())
			throw Ogre::InsufficientData("A vertex element references an unbound stream");

		// Generate vertices
		std::vector<StdSubMesh::Vertex> vertices;
		vertices.reserve(geo.vertexCount);
		for (size_t i = 0; i < geo.vertexCount; ++i)
		{
			StdSubMesh::Vertex vertex;
			vertex.nx = vertex.ny = vertex.nz = 0;
			vertex.x = vertex.y = vertex.z = 0;
			vertex.u = vertex.v = 0;
			bool read_tex = false;
			// Read vertex declaration
			BOOST_FOREACH(Ogre::Mesh::ChunkGeometryVertexDeclElement element, geo.vertexDeclaration)
			{
				float values[4];
				ReadNormalizedVertexData(values, cursors[element.source] + element.offset, element.type);
				switch (element.semantic)
				{
				case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDES_Position:
					vertex.x = values[0];
					vertex.y = values[1];
					vertex.z = values[2];
					break;
				case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDES_Normals:
					vertex.nx = values[0];
					vertex.ny = values[1];
					vertex.nz = values[2];
					break;
				case Ogre::Mesh::ChunkGeometryVertexDeclElement::VDES_Texcoords:
					if (!read_tex) {
						vertex.u = values[0];
						vertex.v = values[1];
						read_tex = true;
					}
					break;
				default:
					// We ignore unhandled element semantics.
					break;
				}
			}
			vertices.push_back(vertex);
			// Advance vertex buffer cursors
			BOOST_FOREACH(const Ogre::Mesh::ChunkGeometryVertexBuffer &buf, geo.vertexBuffers)
			cursors[buf.index] += buf.vertexSize;
		}

		return vertices;
	}
}

void StdMeshSkeletonLoader::StoreSkeleton(const char* groupname, const char* filename, std::shared_ptr<StdMeshSkeleton> skeleton)
{
	assert(groupname != NULL);
	assert(filename != NULL);
	assert(skeleton != NULL);

	// Create mirrored animations (#401)
	// this is still going to be somewhere else, but for now it will keep moving around
	skeleton->PostInit();

	
	// save in map
	StdCopyStrBuf filepath;
	MakeFullSkeletonPath(filepath, groupname, filename);
	Skeletons.insert(std::make_pair(filepath, skeleton));

	// memorize which skeletons can be appended
	// skins get broken down to their original definition, which is a little messy at the moment.
	StdCopyStrBuf buf_filename(GetFilenameOnly(filename));
	StdCopyStrBuf command_with_definition(GetFilenameOnly(buf_filename.getData())); // include.Clonk becomes include, include.Clonk.Farmer becomes include.Clonk
	StdCopyStrBuf command(GetFilenameOnly(command_with_definition.getData()));      // include stays include, include.Clonk becomes include
	StdCopyStrBuf definition(GetExtension(buf_filename.getData())); // include.Clonk becomes Clonk, include.Clonk.Farmer becomes Farmer

	if (!(command_with_definition == command)) // include.Clonk != include?
	{
		definition = StdCopyStrBuf(GetExtension(command_with_definition.getData())); // change definition to the part behind the .: Clonk
	}

	const char* appendto = "appendto"; // has to be a constant
	const char* include = "include";   // dito

	// check where to store
	if (command == appendto)
	{
		AppendtoSkeletons.insert(std::make_pair(skeleton, definition));
	}
	else if (command == include)
	{
		IncludeSkeletons.insert(std::make_pair(skeleton, definition));
	}
}

std::shared_ptr<StdMeshSkeleton> StdMeshSkeletonLoader::GetSkeletonByName(const StdStrBuf& name) const
{
	StdCopyStrBuf filename(name);

	//DebugLogF("Loading skeleton %s\n", filename.getData());

	std::map<StdCopyStrBuf, std::shared_ptr<StdMeshSkeleton>>::const_iterator iter = Skeletons.find(filename);
	if (iter == Skeletons.end()) return NULL;
	return iter->second;
}

void StdMeshSkeletonLoader::LoadSkeletonBinary(const char* groupname, const char* filename, const char *sourcefile, size_t size)
{
	boost::scoped_ptr<Ogre::Skeleton::Chunk> chunk;
	Ogre::DataStream stream(sourcefile, size);

	std::shared_ptr<StdMeshSkeleton> Skeleton(new StdMeshSkeleton);

	// First chunk must be the header
	chunk.reset(Ogre::Skeleton::Chunk::Read(&stream));
	if (chunk->GetType() != Ogre::Skeleton::CID_Header)
		throw Ogre::Skeleton::InvalidVersion();

	boost::ptr_map<uint16_t, StdMeshBone> bones;
	boost::ptr_vector<Ogre::Skeleton::ChunkAnimation> animations;
	for (Ogre::Skeleton::ChunkID id = Ogre::Skeleton::Chunk::Peek(&stream);
		id == Ogre::Skeleton::CID_BlendMode || id == Ogre::Skeleton::CID_Bone || id == Ogre::Skeleton::CID_Bone_Parent || id == Ogre::Skeleton::CID_Animation;
		id = Ogre::Skeleton::Chunk::Peek(&stream)
		)
	{
		std::unique_ptr<Ogre::Skeleton::Chunk> chunk(Ogre::Skeleton::Chunk::Read(&stream));
		switch (chunk->GetType())
		{
		case Ogre::Skeleton::CID_BlendMode:
		{
			Ogre::Skeleton::ChunkBlendMode& cblend = *static_cast<Ogre::Skeleton::ChunkBlendMode*>(chunk.get());
			// TODO: Handle it
			if (cblend.blend_mode != 0) // 0 is average, 1 is cumulative. I'm actually not sure what the difference really is... anyway we implement only one method yet. I think it's average, but not 100% sure.
				LogF("StdMeshLoader: CID_BlendMode not implemented.");
		}
		break;
		case Ogre::Skeleton::CID_Bone:
		{
			Ogre::Skeleton::ChunkBone &cbone = *static_cast<Ogre::Skeleton::ChunkBone*>(chunk.get());
			// Check that the bone ID is unique
			if (bones.find(cbone.handle) != bones.end())
				throw Ogre::Skeleton::IdNotUnique();
			StdMeshBone *bone = new StdMeshBone;
			bone->Parent = NULL;
			bone->ID = cbone.handle;
			bone->Name = cbone.name.c_str();
			bone->Transformation.translate = cbone.position;
			bone->Transformation.rotate = cbone.orientation;
			bone->Transformation.scale = cbone.scale;
			bone->InverseTransformation = StdMeshTransformation::Inverse(bone->Transformation);
			bones.insert(cbone.handle, bone);
		}
		break;
		case Ogre::Skeleton::CID_Bone_Parent:
		{
			Ogre::Skeleton::ChunkBoneParent &cbparent = *static_cast<Ogre::Skeleton::ChunkBoneParent*>(chunk.get());
			if (bones.find(cbparent.parentHandle) == bones.end() || bones.find(cbparent.childHandle) == bones.end())
				throw Ogre::Skeleton::BoneNotFound();
			bones[cbparent.parentHandle].Children.push_back(&bones[cbparent.childHandle]);
			bones[cbparent.childHandle].Parent = &bones[cbparent.parentHandle];
		}
		break;
		case Ogre::Skeleton::CID_Animation:
			// Collect animations for later (need bone table index, which we don't know yet)
			animations.push_back(static_cast<Ogre::Skeleton::ChunkAnimation*>(chunk.release()));
			break;
		default:
			assert(!"Unexpected enum value");
			break;
		}
		if (stream.AtEof()) break;
	}

	// Find master bone (i.e., the one without a parent)
	StdMeshBone *master = NULL;
	for (boost::ptr_map<uint16_t, StdMeshBone>::iterator it = bones.begin(); it != bones.end(); ++it)
	{
		if (!it->second->Parent)
		{
			master = it->second;
			Skeleton->AddMasterBone(master);
		}
	}
	if (!master)
		throw Ogre::Skeleton::MissingMasterBone();

	// Transfer bone ownership to mesh (double .release() is correct)
	while (!bones.empty()) bones.release(bones.begin()).release();

	// Build handle->index quick access table
	std::map<uint16_t, size_t> handle_lookup;
	for (size_t i = 0; i < Skeleton->GetNumBones(); ++i)
	{
		handle_lookup[Skeleton->GetBone(i).ID] = i;
	}

	// Fixup animations
	BOOST_FOREACH(Ogre::Skeleton::ChunkAnimation &canim, animations)
	{
		StdMeshAnimation &anim = Skeleton->Animations[StdCopyStrBuf(canim.name.c_str())];
		anim.Name = canim.name.c_str();
		anim.Length = canim.duration;
		anim.Tracks.resize(Skeleton->GetNumBones());
		anim.OriginSkeleton = &(*Skeleton);

		BOOST_FOREACH(Ogre::Skeleton::ChunkAnimationTrack &catrack, canim.tracks)
		{
			const StdMeshBone &bone = Skeleton->GetBone(handle_lookup[catrack.bone]);
			StdMeshTrack *&track = anim.Tracks[bone.Index];
			if (track != NULL)
				throw Ogre::Skeleton::MultipleBoneTracks();
			track = new StdMeshTrack;
			BOOST_FOREACH(Ogre::Skeleton::ChunkAnimationTrackKF &catkf, catrack.keyframes)
			{
				StdMeshKeyFrame &kf = track->Frames[catkf.time];
				kf.Transformation.rotate = catkf.rotation;
				kf.Transformation.scale = catkf.scale;
				kf.Transformation.translate = bone.InverseTransformation.rotate * (bone.InverseTransformation.scale * catkf.translation);
			}
		}
	}

	// Fixup bone transforms
	BOOST_FOREACH(StdMeshBone *bone, Skeleton->Bones)
	{
		if (bone->Parent)
		{
			bone->Transformation = bone->Parent->Transformation * bone->Transformation;
			bone->InverseTransformation = StdMeshTransformation::Inverse(bone->Transformation);
		}
	}

	StoreSkeleton(groupname, filename, Skeleton);
}

StdMesh *StdMeshLoader::LoadMeshBinary(const char *sourcefile, size_t length, const StdMeshMatManager &mat_mgr, StdMeshSkeletonLoader &loader, const char *filename)
{
	boost::scoped_ptr<Ogre::Mesh::Chunk> root;
	Ogre::DataStream stream(sourcefile, length);

	// First chunk must be the header
	root.reset(Ogre::Mesh::Chunk::Read(&stream));
	if (root->GetType() != Ogre::Mesh::CID_Header)
		throw Ogre::Mesh::InvalidVersion();

	// Second chunk is the mesh itself
	root.reset(Ogre::Mesh::Chunk::Read(&stream));
	if (root->GetType() != Ogre::Mesh::CID_Mesh)
		throw Ogre::Mesh::InvalidVersion();

	// Generate mesh from data
	Ogre::Mesh::ChunkMesh &cmesh = *static_cast<Ogre::Mesh::ChunkMesh*>(root.get());
	std::unique_ptr<StdMesh> mesh(new StdMesh);
	mesh->BoundingBox = cmesh.bounds;
	mesh->BoundingRadius = cmesh.radius;

	// We allow bounding box to be empty if it's only due to X direction since
	// this is what goes inside the screen in Clonk.
	if(mesh->BoundingBox.y1 == mesh->BoundingBox.y2 || mesh->BoundingBox.z1 == mesh->BoundingBox.z2)
		throw Ogre::Mesh::EmptyBoundingBox();

	// if the mesh has a skeleton, then try loading
	// it from the loader by the definition name
	if (!cmesh.skeletonFile.empty())
	{
		StdCopyStrBuf skeleton_filename = StdCopyStrBuf();
		StdMeshSkeletonLoader::MakeFullSkeletonPath(skeleton_filename, filename, cmesh.skeletonFile.c_str());

		mesh->Skeleton = loader.GetSkeletonByName(skeleton_filename);

		// with this exception the assert below is useless
		// also, I think the bone_lookup should only be used if there is a skeleton anyway
		// so there could be meshes without bones even?
		if (mesh->Skeleton == NULL)
		{
			StdCopyStrBuf exception("The specified skeleton file was not found: ");
			exception.Append(skeleton_filename.getData());
			throw Ogre::InsufficientData(exception.getData());
		}
	}

	assert(mesh->Skeleton != NULL); // the bone assignments could instead be added only, if there is a skeleton

	// Build bone handle->index quick access table
	std::map<uint16_t, size_t> bone_lookup;
	for (size_t i = 0; i < mesh->GetSkeleton().GetNumBones(); ++i)
	{
		bone_lookup[mesh->GetSkeleton().GetBone(i).ID] = i;
	}

	// Read submeshes
	mesh->SubMeshes.reserve(cmesh.submeshes.size());
	for (size_t i = 0; i < cmesh.submeshes.size(); ++i)
	{
		mesh->SubMeshes.push_back(StdSubMesh());
		StdSubMesh &sm = mesh->SubMeshes.back();
		Ogre::Mesh::ChunkSubmesh &csm = cmesh.submeshes[i];
		sm.Material = mat_mgr.GetMaterial(csm.material.c_str());
		if (!sm.Material)
			throw Ogre::Mesh::InvalidMaterial();
		if (csm.operation != Ogre::Mesh::ChunkSubmesh::SO_TriList)
			throw Ogre::Mesh::NotImplemented("Submesh operations other than TriList aren't implemented yet");
		sm.Faces.resize(csm.faceVertices.size() / 3);
		for (size_t face = 0; face < sm.Faces.size(); ++face)
		{
			sm.Faces[face].Vertices[0] = csm.faceVertices[face * 3 + 0];
			sm.Faces[face].Vertices[1] = csm.faceVertices[face * 3 + 1];
			sm.Faces[face].Vertices[2] = csm.faceVertices[face * 3 + 2];
		}
		Ogre::Mesh::ChunkGeometry &geo = *(csm.hasSharedVertices ? cmesh.geometry : csm.geometry);
		sm.Vertices = ReadSubmeshGeometry(geo, filename);

		// Read bone assignments
		std::vector<Ogre::Mesh::BoneAssignment> &boneAssignments = (csm.hasSharedVertices ? cmesh.boneAssignments : csm.boneAssignments);
		assert(!csm.hasSharedVertices || csm.boneAssignments.empty());
		BOOST_FOREACH(const Ogre::Mesh::BoneAssignment &ba, boneAssignments)
		{
			if (ba.vertex >= sm.GetNumVertices())
				throw Ogre::Mesh::VertexNotFound();
			if (bone_lookup.find(ba.bone) == bone_lookup.end())
				throw Ogre::Skeleton::BoneNotFound();
			StdMeshVertexBoneAssignment assignment;
			assignment.BoneIndex = bone_lookup[ba.bone];
			assignment.Weight = ba.weight;
			sm.Vertices[ba.vertex].BoneAssignments.push_back(assignment);
		}

		// Normalize bone assignments
		BOOST_FOREACH(StdSubMesh::Vertex &vertex, sm.Vertices)
		{
			float sum = 0;
			BOOST_FOREACH(StdMeshVertexBoneAssignment &ba, vertex.BoneAssignments)
			sum += ba.Weight;
			BOOST_FOREACH(StdMeshVertexBoneAssignment &ba, vertex.BoneAssignments)
			ba.Weight /= sum;
		}
	}
	return mesh.release();
}

void StdMeshSkeletonLoader::ResolveIncompleteSkeletons()
{
	DoResetSkeletons();
	DoAppendSkeletons();
	DoIncludeSkeletons();
}

void StdMeshSkeletonLoader::DoResetSkeletons()
{
	std::map<StdCopyStrBuf, std::shared_ptr<StdMeshSkeleton>>::iterator it;
	for (it = Skeletons.begin(); it != Skeletons.end(); it++)
	{
		std::shared_ptr<StdMeshSkeleton> skeleton = it->second;

		// remove animations from destination
		std::map<StdCopyStrBuf, StdMeshAnimation>::const_iterator animations = skeleton->Animations.begin();

		while (animations != skeleton->Animations.end())
		{
			if (animations->second.OriginSkeleton != &(*(skeleton)))
			{
				DebugLogF("Erasing animation %s from %s", animations->first.getData(), it->first.getData());
				animations = skeleton->Animations.erase(animations);
				//skeleton->Animations.erase(animations++);
			}
			else
			{
				++animations;
			}
		}
	}
}

void StdMeshSkeletonLoader::DoAppendSkeletons()
{
	// handle the "appendto.<C4ID>.skeleton" files.

	std::map<std::shared_ptr<StdMeshSkeleton>, StdCopyStrBuf>::iterator it;
	for (it = AppendtoSkeletons.begin(); it != AppendtoSkeletons.end(); it++)
	{
		StdCopyStrBuf id(it->second);

		StdMeshSkeleton* destination = GetSkeletonByDefinition(id.getData());

		// append animations, if the definition has a mesh
		if (&destination == NULL)
		{
			DebugLogF("WARNING: Appending skeleton to definition '%s' failed, because the definition has no skeleton.", id.getData());
		}
		else
		{
			std::shared_ptr<StdMeshSkeleton> source = it->first;

			std::map<StdCopyStrBuf, StdMeshAnimation>::const_iterator animations;
			
			// append animations from source
			for (animations = source->Animations.begin(); animations != source->Animations.end(); animations++)
			{
				if (destination->Animations.find(animations->first) != destination->Animations.end())
				{
					DebugLogF("WARNING: Overloading animation '%s' is not allowed. This animation already exists in '%s'.", animations->first.getData(), id.getData());
				}
				else
				{
					DebugLogF("Appending animation %s to definition %s", animations->second.Name.getData(), id.getData());
					destination->InsertAnimation(*source, animations->second);
				}
			}
		}
	}
}

void StdMeshSkeletonLoader::DoIncludeSkeletons()
{
	// handle the "include.<C4ID>.skeleton" files.

	std::map<std::shared_ptr<StdMeshSkeleton>, StdCopyStrBuf>::iterator it;
	for (it = IncludeSkeletons.begin(); it != IncludeSkeletons.end(); it++)
	{
		StdCopyStrBuf id(it->second);

		StdMeshSkeleton* source = GetSkeletonByDefinition(id.getData());

		// append animations, if the definition has a mesh
		if (&source == NULL)
		{
			DebugLogF("WARNING: Including skeleton of definition '%s' failed, because the definition has no skeleton.", id.getData());
		}
		else
		{
			std::shared_ptr<StdMeshSkeleton> destination = it->first;

			std::map<StdCopyStrBuf, StdMeshAnimation>::const_iterator animations;

			// append animations from source
			for (animations = source->Animations.begin(); animations != source->Animations.end(); animations++)
			{
				if (destination->Animations.find(animations->first) != destination->Animations.end())
				{
					DebugLogF("WARNING: Animation '%s' from %s is not included. A newer version of the animation exists in the destination file.", animations->first.getData(), id.getData());
				}
				else
				{
					//DebugLogF("Including animation %s from skeleton %s", animations->second.Name.getData(), id.getData());
					destination->InsertAnimation(*source, animations->second);
				}
			}
		}
	}
}


#appendto Clonk

local idHudSandbox;
local idHudOS;

local idGuiHudOS_catselect = 100;
local idGuiHudOS_objectselect = 101;
local idGuiHudOS_switchspawndest = 102;

protected func Death(int killed_by)
{
	HideSandboxUI();
	return _inherited(killed_by, ...);
}

func ShowSandboxUI()
{
	var SandboxUI = 
	{
		Player = GetOwner(),
		Style = GUI_Multiple,
		
		ButtonBar = 
		{
			Left = "10em",
			Top = "0.5em",
			Bottom = "2.5em",
			Right = "20em",
			
			BtnObjectspawn =
			{
				Top = "0em",
				Bottom = "2em",
				Left = "0em",
				Right = "2em",
				
				Symbol = Hammer,
				Tooltip = "$TooltipObjectspawn$",
				
				BackgroundColor = { Std = RGBa(128, 128, 128, 128), Hover = RGBa(128, 255, 128, 128) },
				
				OnMouseIn = GuiAction_SetTag("Hover"),
				OnMouseOut = GuiAction_SetTag("Std"),
				OnClick = GuiAction_Call(this, "BtnObjectSpawnClick"),
			},
			
			BtnLandscapeBrush =
			{
				Top = "0em",
				Bottom = "2em",
				Left = "2em",
				Right = "4em",
				
				Symbol = SprayCan,
				Tooltip = "$TooltipLandscapeBrush$",
				
				BackgroundColor = { Std = RGBa(128, 128, 128, 128), Hover = RGBa(128, 255, 128, 128) },
				
				OnMouseIn = GuiAction_SetTag("Hover"),
				OnMouseOut = GuiAction_SetTag("Std"),
				OnClick = GuiAction_Call(this, "BtnLandscapeBrushClick"),
			},
			
			BtnMarker =
			{
				Top = "0em",
				Bottom = "2em",
				Left = "4em",
				Right = "6em",
				
				Symbol = ParkourFlag,
				Tooltip = "$TooltipMarker$",
				
				BackgroundColor = { Std = RGBa(128, 128, 128, 128), Hover = RGBa(128, 255, 128, 128) },
				
				OnMouseIn = GuiAction_SetTag("Hover"),
				OnMouseOut = GuiAction_SetTag("Std"),
				OnClick = GuiAction_Call(this, "BtnMarkerClick"),
			},
			
			BtnMapGen =
			{
				Top = "0em",
				Bottom = "2em",
				Left = "6em",
				Right = "8em",
				
				Symbol = Icon_World,
				Tooltip = "$TooltipMapGen$",
				
				BackgroundColor = { Std = RGBa(128, 128, 128, 128), Hover = RGBa(128, 255, 128, 128) },
				
				OnMouseIn = GuiAction_SetTag("Hover"),
				OnMouseOut = GuiAction_SetTag("Std"),
				OnClick = GuiAction_Call(this, "BtnMapGenClick"),
			},
			
			BtnTweaks =
			{
				Top = "0em",
				Bottom = "2em",
				Left = "8em",
				Right = "10em",
				
				Symbol = Icon_Lightbulb,
				Tooltip = "$TooltipTweaks$",
				
				BackgroundColor = { Std = RGBa(128, 128, 128, 128), Hover = RGBa(128, 255, 128, 128) },
				
				OnMouseIn = GuiAction_SetTag("Hover"),
				OnMouseOut = GuiAction_SetTag("Std"),
				OnClick = GuiAction_Call(this, "BtnTweaksClick"),
			},
		},
		
		GodsHandDisplay =
		{
			Top = "2.5em",
			Left = "10em",
			Right = "20em",
			Bottom = "3.5em",
			BackgroundColor = RGBa(96,96,96,96),
			Tooltip = "$GodsHandDisplayTT$",
			
			icon = 
			{
				Symbol = GodsHand,
				Right = "1em",
			},
			
			icon2 =
			{
				Symbol = ObjectSpawnDefinition,
				Left = "1.5em",
				Right = "2.5em",
			},
			
			text =
			{
				Text = ObjectSpawnDefinition.Name,
				Left = "2.75em",
				Style = GUI_TextVCenter,
			}
		}
	};
	
	
	idHudSandbox = GuiOpen(SandboxUI);
	
	return idHudSandbox;
}

func UpdateGodsHandDisplay()
{
	var update = 
	{
		GodsHandDisplay =
		{
			icon2 = 
			{
				Symbol = ObjectSpawnDefinition
			},
			
			text =
			{
				Text = ObjectSpawnDefinition.Name
			}
		}
	};
	
	GuiUpdate(update, idHudSandbox);
}

func HideSandboxUI()
{
	if (idHudSandbox)
	{
		GuiClose(idHudSandbox);
		idHudSandbox = nil;
	}
}

func BtnObjectSpawnClick()
{
	ShowObjectSpawnUI();
}

func BtnLandscapeBrushClick()
{
	ShowMaterialBrushUI();
}

func BtnMarkerClick()
{
	ShowMarkerUI();
}

func BtnMapGenClick()
{
	ShowMapGenUI();
}

func BtnTweaksClick()
{
	ShowTweaksUI();
}

local ObjectSpawnTarget = 1; // 1 = Spawn on Clonk/Inventory, 2 = God's Hand
local ObjectSpawnDefinition = Wood; // Definition to spawn by God's Hand

local ObjectSpawnMenuOpts =
{
	ProductionResources = 
	{
		Priority = 2,
		Caption = "$OSCatProductionResources$",
		Icon = Ore,
		Items = [Rock, Ore, Coal, Firestone, Nugget, Metal, Wood, Moss, Ruby, Amethyst, GoldBar, Firestone, Ice]
	},
	
	Foodstuff = 
	{
		Priority = 3,
		Caption = "$OSCatFoodstuff$",
		Icon = Bread,
		Items = [Bread, Mushroom, CookedMushroom, Sproutberry, Coconut]
	},
	
	Liquids =
	{
		Priority = 4,
		Caption = "$OSCatLiquids$",
		Icon = Water,
		Items = [Water, Acid, Lava, DuroLava, Oil, Concrete]
	},
	
	Tools = 
	{
		Priority = 5,
		Caption = "$OSCatTools$",
		Icon = Hammer,
		Items = [Hammer, Shovel, Axe, Pickaxe, Sickle, TeleGlove, Torch, WallKit, Ropeladder, Ropebridge, GrappleBow, Balloon, Boompack, WindBag, Lantern, Bucket, Barrel, MetalBarrel, Pipe, Crate, Dynamite, DynamiteBox, Lorry]
	},
	
	Weapons = 
	{
		Priority = 6,
		Caption = "$OSCatWeapons$",
		Icon = Sword,
		Items = [Shield, Helmet, Sword, Club, Bow, Arrow, FireArrow, BombArrow, Blunderbuss, LeadBullet, Javelin, GrenadeLauncher, IronBomb, SmokeBomb, Boompack, Lantern, Cannon, Catapult]
	},
	
	Explosives = 
	{
		Priority = 7,
		Caption = "$OSCatExplosives$",
		Icon = Dynamite,
		Items = [Firestone, Dynamite, DynamiteBox, IronBomb, PowderKeg, Lantern]
	},
	
	Vehicles =
	{
		Priority = 8,
		Caption = "$OSCatVehicles$",
		Icon = Airship,
		Items = [Airship, Airplane]
	},
	
	Animals =
	{
		Priority = 9,
		Caption = "$OSCatAnimals$",
		Icon = Wipf,
		Items = [Wipf, Bat, Butterfly, Chippie, Firefly, Fish, Piranha, Puka, Mooq, Shark, Squid, Zap, Mosquito]
	},
	
	Plants = 
	{
		Priority = 10,
		Caption = "$OSCatPlants$",
		Icon = Flower,
		Items = [Tree_Coniferous, Tree_Coniferous2, Tree_Coniferous3, Tree_Coniferous4, Tree_Deciduous, Tree_Coconut, LargeCaveMushroom, Cotton_Branch, Coral, Seaweed, SproutBerryBush, Lichen, Fern, Grass, Branch, Trunk, Vine, Wheat]
	},
	
	GodTools =
	{
		Priority = 99,
		Caption = "$OSCatGodTools$",
		Icon = GodsHand,
		Items = [GodsHand, SprayCan, Teleporter, Marker]
	}
};

func ShowObjectSpawnUI()
{
	var SpawnUI = 
	{
		Decoration = GUI_MenuDeco,
		Player = GetOwner(),
		
		CatSelect =
		{
			ID = idGuiHudOS_catselect,
			Left = "0%",
			Right = "25%",
			Top = "2em",
			Bottom = "100% - 2em",
			
			BackgroundColor = RGBa(32,32,32,224),
			Style = GUI_VerticalLayout,
		},
		
		SwitchSpawnDest =
		{
			ID = idGuiHudOS_switchspawndest,
			Top = "100% - 2em",
			Right = "25%",
			BackgroundColor = { Std = RGBa(32,32,128,128), Hover = RGBa(128,128,192,128) },
			Tooltip = GetObjectSpawnDestTooltip(),
			
			icon = 
			{
				Symbol = GetObjectSpawnDestSymbol(),
				Right = "2em",
			},
			
			text =
			{
				Text = GetObjectSpawnDest(),
				Left = "2.5em",
				Style = GUI_TextVCenter,
			},
			
			OnClick    = GuiAction_Call(this, "SwitchObjectSpawnDest"),
			OnMouseIn  = GuiAction_SetTag("Hover"),
			OnMouseOut = GuiAction_SetTag("Std"),
		},
		
		ObjectSelect = 
		{
			ID = idGuiHudOS_objectselect,
			Left = "25%",
			Right = "100%",
			Top = "2em",
			Bottom = "100% - 2em",
			BackgroundColor = RGBa(64,64,64,224),
		},
		
		HintTarget =
		{
			Left = "25%",
			Top = "100% - 2em",
			BackgroundColor = RGBa(128,128,128,224),
			
			icon =
			{
				Symbol = Icon_Arrow,
				GraphicsName = "Left",
				Right = "2em",
			},
			
			text =
			{
				Text = "$OSHintTarget$",
				Left = "2.5em",
				Style = GUI_TextVCenter,
			}
		}
		
	};
	
	var index = 0;
	for (var property in GetProperties(ObjectSpawnMenuOpts))
	{
		index++;
		var entry = ObjectSpawnMenuOpts[property];
		
		var catentry =
		{
			ID = 1000 + index,
			Priority = entry.Priority,
			Bottom = "+2em",
			Right = "100% - 1em",
			BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
			
			icon =
			{
				Symbol = entry.Icon,
				Right = "2em"
			},
			
			text =
			{
				Text = entry.Caption,
				Left = "+2.5em",
				Style = GUI_TextVCenter
			},
			
			OnClick    = GuiAction_Call(this, "ObjectSpawnSelectCat", [entry]),
			OnMouseIn  = GuiAction_SetTag("Hover"),
			OnMouseOut = GuiAction_SetTag("Std")
		};
		
		GuiAddSubwindow(catentry, SpawnUI.CatSelect);
	}
	
	GuiAddCloseButton(SpawnUI, this, "HideObjectSpawnUI");
	
	idHudOS = GuiOpen(SpawnUI);
	
	SetMenu(idHudOS);
	
	return idHudOS;
}

func HideObjectSpawnUI()
{
	if (idHudOS)
	{
		GuiClose(idHudOS);
		MenuClosed();
		idHudOS = nil;
	}
}

func GetObjectSpawnDest()
{
	if (ObjectSpawnTarget == 1) return "$OSTargetClonk$";
	if (ObjectSpawnTarget == 2) return "$OSTargetGodsHand$";
}

func GetObjectSpawnDestSymbol()
{
	if (ObjectSpawnTarget == 1) return Clonk;
	if (ObjectSpawnTarget == 2) return GodsHand;
}

func GetObjectSpawnDestTooltip()
{
	if (ObjectSpawnTarget == 1) return "$OSTargetClonkTT$";
	if (ObjectSpawnTarget == 2) return "$OSTargetGodsHandTT$";
}

func SwitchObjectSpawnDest()
{
	ObjectSpawnTarget++;
	if (ObjectSpawnTarget > 2) ObjectSpawnTarget = 1;
	
	var update = 
	{
		SwitchSpawnDest =
		{
			ID = idGuiHudOS_switchspawndest,
			Top = "100% - 2em",
			Right = "25%",
			BackgroundColor = { Std = RGBa(32,32,128,128), Hover = RGBa(128,128,192,128) },
			Tooltip = GetObjectSpawnDestTooltip(),
			
			icon = 
			{
				Symbol = GetObjectSpawnDestSymbol(),
				Right = "2em",
			},
			
			text =
			{
				Text = GetObjectSpawnDest(),
				Left = "2.5em",
				Style = GUI_TextVCenter,
			},
			
			OnClick    = GuiAction_Call(this, "SwitchObjectSpawnDest"),
			OnMouseIn  = GuiAction_SetTag("Hover"),
			OnMouseOut = GuiAction_SetTag("Std"),
		},
	};
	
	GuiClose(idHudOS, idGuiHudOS_switchspawndest);
	GuiUpdate(update, idHudOS);
}

func ObjectSpawnSelectCat(data, int player, int ID, int subwindowID, object target)
{
	GuiClose(idHudOS, idGuiHudOS_objectselect);
	var objectselect = 
	{
		ID = idGuiHudOS_objectselect,
		Left = "25%",
		Right = "100%",
		Top = "2em",
		BackgroundColor = RGBa(64,64,64,224),
		
		Style = GUI_GridLayout,
	};
	GuiUpdate({ ObjectSelect = objectselect }, idHudOS);
	
	var cat = data[0];
	
	var index = 0;
	for (var item in cat.Items)
	{
		index++;
		
		var objentry = 
		{
			ID = 2000 + index,
			Priority = index,
			Bottom = "+2em",
			Right = "+8em",
			BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
			Tooltip = item.Description,
			
			icon =
			{
				Symbol = item,
				Right = "+2em"
			},
			
			text =
			{
				Text = item.Name,
				Left = "+2.5em",
				Style = GUI_TextVCenter,
			},
			
			OnClick    = GuiAction_Call(this, "ObjectSpawnSelectObject", [item]),
			OnMouseIn  = GuiAction_SetTag("Hover"),
			OnMouseOut = GuiAction_SetTag("Std"),
		};
		
		GuiAddSubwindow(objentry, objectselect);
	}
	
	GuiUpdate(objectselect, idHudOS, idGuiHudOS_objectselect);
}

func ObjectSpawnSelectObject(data, int player, int ID, int subwindowID, object target)
{
	var clonk = GetCursor(player);
	var obj = data[0];
	
	if (ObjectSpawnTarget == 1)
	{
		// Spawn in Clonk Inventory
		
		if (obj == Marker)
		{
			PlaceNewMarker();
			return;
		}
		
		var spawnlocation = 2; // 1 = Inventory, 2 = Outside, 3 = Outside Above
		
		// Normal objects can spawn in Inventory
		if (obj->GetCategory() & C4D_Object)
			spawnlocation = 1;
		
		// Livings spawn outside the clonk
		if (obj->GetCategory() & C4D_Living)
			spawnlocation = 2;
		
		// Vehicles spawn above to avoid being stuck
		if (obj->GetCategory() & C4D_Vehicle)
			spawnlocation = 3;
		
		
		//if (spawnlocation == 1) clonk->CreateContents(obj);
		if (spawnlocation == 1) clonk->Collect(CreateObject(obj));
		if (spawnlocation == 2) clonk->CreateObject(obj);
		if (spawnlocation == 3) clonk->CreateObjectAbove(obj, 0, 0);
	}
	else if (ObjectSpawnTarget == 2)
	{
		// Select for God's Hand
		clonk.ObjectSpawnDefinition = obj;
		HideObjectSpawnUI();
		UpdateGodsHandDisplay();
	}
	
	// TODO: Prüfung wo das Objekt erzeugt werden soll (Inventar/Aussen)
	// TODO: Prüfen, was mit dem ausgewählten Objekt passieren soll (Spawn/Hand of god)
}

local SelectedBrushMaterial = "Earth-earth";
local SelectedBrushBgMaterial = "Tunnel-tunnel";
local SelectedBrushSize = 8;
local SelectedBrushMode = 1; // 1 = Draw Brush, 2 = Quad Brush, 3 = Eraser

local BrushMaterials =
{
	Earth       = { Icon = Earth,     Caption = "$MatEarth$",       Material = "Earth-earth" },
	EarthSpongy = { Icon = Earth,     Caption = "$MatEarthSpongy$", Material = "Earth-earth_spongy" },
	EarthRoot   = { Icon = Earth,     Caption = "$MatEarthRoot$",   Material = "Earth-earth_root" },
	Sand        = { Icon = Sand,      Caption = "$MatSand$",        Material = "Sand-sand" },
	SandDry     = { Icon = Sand,      Caption = "$MatSandDry$",     Material = "SandDry-sand" },
	Ice         = { Icon = Ice,       Caption = "$MatIce$",         Material = "Ice-ice" },
	Ice2        = { Icon = Ice,       Caption = "$MatIce2$",        Material = "Ice-ice2" },
	Snow        = { Icon = Snow,      Caption = "$MatSnow$",        Material = "Snow-snow1" },
	Rock        = { Icon = Rock,      Caption = "$MatRock$",        Material = "Rock-rock" },
	RockSmooth  = { Icon = Rock,      Caption = "$MatRockSmooth$",  Material = "Rock-rock_smooth" },
	Granite     = { Icon = Rock,      Caption = "$MatGranite$",     Material = "Granite-granite" },
	Ore         = { Icon = Ore,       Caption = "$MatOre$",         Material = "Ore-ore" },
	Gold        = { Icon = Nugget,    Caption = "$MatGold$",        Material = "Gold-gold" },
	Coal        = { Icon = Coal,      Caption = "$MatCoal$",        Material = "Coal-coal" },
	Firestone   = { Icon = Firestone, Caption = "$MatFirestone$",   Material = "Firestone-firestone" },
	Ruby        = { Icon = Ruby,      Caption = "$MatRuby$",        Material = "Ruby-ruby" },
	Amethyst    = { Icon = Amethyst,  Caption = "$MatAmethyst$",    Material = "Amethyst-amethyst" },
	Ashes       = { Icon = Ashes,     Caption = "$MatAshes$",       Material = "Ashes-ashes" },
	Brick       = { Icon = WallKit,   Caption = "$MatBrick$",       Material = "Brick-brick" },
	BrickSoft   = { Icon = WallKit,   Caption = "$MatBrickSoft$",   Material = "BrickSoft-brick" },
	Everrock    = { Icon = Metal,     Caption = "$MatEverrock$",    Material = "Everrock-everrock" },
	Water       = { Icon = Water,     Caption = "$MatWater$",       Material = "Water-water" },
	Acid        = { Icon = Acid,      Caption = "$MatAcid$",        Material = "Acid-acid" },
	Lava        = { Icon = Lava,      Caption = "$MatLava$",        Material = "Lava-lava_red" },
	DuroLava    = { Icon = DuroLava,  Caption = "$MatDuroLava$",    Material = "DuroLava-lava_red" },
	Oil         = { Icon = Oil,       Caption = "$MatOil$",         Material = "Oil-oil" },
	Tunnel      = { Icon = Earth,     Caption = "$MatTunnel$",      Material = "Tunnel-tunnel" },
	Backwall    = { Icon = Earth,     Caption = "$MatTunnelBrick$", Material = "Tunnel-brickback" }
};

local BrushBackgroundMaterials =
{
	Sky    = { Icon = Earth, Caption = "$MatBgSky$", Material = DMQ_Sky },
	Tunnel = { Icon = Earth, Caption = "$MatBgTunnel$", Material = "Tunnel-tunnel" },
	Bricks = { Icon = Earth, Caption = "$MatBgBricks$", Material = "Tunnel-brickback" }
};

local idHudMB;

local idGuiHudMB_MatSelect = 300;
local idGuiHudMB_MatBgSelect = 301;
local idGuiHudMB_SizeSelect = 302;
local idGuiHudMB_ModeSelect = 303;

func ShowMaterialBrushUI()
{
	var bgBrush = { Std = 0, Hover = RGBa(128,128,192,128) };
	var bgQuad  = bgBrush;
	var bgErase = bgBrush;
	
	if (SelectedBrushMode == 1) bgBrush = { Std = RGBa(128,192,128,128), Hover = RGBa(128,128,192,128) };
	if (SelectedBrushMode == 2) bgQuad  = { Std = RGBa(128,192,128,128), Hover = RGBa(128,128,192,128) };
	if (SelectedBrushMode == 3) bgErase = { Std = RGBa(128,192,128,128), Hover = RGBa(128,128,192,128) };
	
	var BrushUI = 
	{
		Decoration = GUI_MenuDeco,
		Player = GetOwner(),
		
		OptionList =
		{
			Style = GUI_VerticalLayout,
			Top = "2em",
			
			OptMaterial =
			{
				Index = 50,
				Priority = 1,
				Style = GUI_FitChildren,
				
				Caption = 
				{
					Text = "$MBMaterialFg$",
					Bottom = "2em",
					Right = "10em",
				},
				
				Selection = 
				{
					ID = idGuiHudMB_MatSelect,
					Style = GUI_FitChildren | GUI_GridLayout,
					Left = "10em"
				}
			},
			
			Spacer1 =
			{
				Bottom = "1em",
				Priority = 2,
			},
			
			OptMaterialBg =
			{
				Index = 51,
				Priority = 3,
				Bottom = "4em",
				Style = GUI_FitChildren,
				
				Caption = 
				{
					Text = "$MBMaterialBg$",
					Bottom = "2em",
					Right = "10em",
				},
				
				Selection = 
				{
					ID = idGuiHudMB_MatBgSelect,
					Style = GUI_FitChildren | GUI_GridLayout,
					Left = "10em",
				}
			},
			
			Spacer2 =
			{
				Bottom = "1em",
				Priority = 4,
			},
			
			OptBrushSize =
			{
				Index = 52,
				Priority = 5,
				Bottom = "4em",
				Style = GUI_FitChildren,
				
				Caption = 
				{
					Text = "$MBBrushSize$",
					Bottom = "2em",
					Right = "10em",
				},
				
				Selection = 
				{
					ID = idGuiHudMB_SizeSelect,
					Style = GUI_FitChildren | GUI_GridLayout,
					Left = "10em",
					
					Minus10 =
					{
						Priority = 1,
						
						Symbol = Icon_Number,
						GraphicsName = "Minus",
						Text = "10",
						Style = GUI_TextBottom | GUI_TextRight,
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						Right = "2em",
						Bottom = "2em",
						
						OnClick = GuiAction_Call(this, "BrushSizeChange", -10),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					Minus1 =
					{
						Priority = 2,
						
						Symbol = Icon_Number,
						GraphicsName = "Minus",
						Text = "1",
						Style = GUI_TextBottom | GUI_TextRight,
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						Right = "2em",
						Bottom = "2em",
						
						OnClick = GuiAction_Call(this, "BrushSizeChange", -1),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					Value =
					{
						Priority = 3,
						Style = GUI_TextHCenter | GUI_TextVCenter,
						Text = Format("<c ffff00>%d</c>", SelectedBrushSize),
						
						Right = "4em",
						Bottom = "2em",
					},
					
					Plus1 =
					{
						Priority = 4,
						
						Symbol = Icon_Number,
						GraphicsName = "Plus",
						Text = "1",
						Style = GUI_TextBottom | GUI_TextRight,
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						Right = "2em",
						Bottom = "2em",
						
						OnClick = GuiAction_Call(this, "BrushSizeChange", 1),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					Plus10 =
					{
						Priority = 5,
						
						Symbol = Icon_Number,
						GraphicsName = "Plus",
						Text = "10",
						Style = GUI_TextBottom | GUI_TextRight,
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						Right = "2em",
						Bottom = "2em",
						
						OnClick = GuiAction_Call(this, "BrushSizeChange", 10),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
				}
			},
			
			Spacer3 =
			{
				Bottom = "1em",
				Priority = 6,
			},
			
			OptBrushMode =
			{
				Index = 53,
				Priority = 7,
				Bottom = "4em",
				Style = GUI_FitChildren,
				
				Caption = 
				{
					Text = "$MBBrushMode$",
					Bottom = "2em",
					Right = "10em",
				},
				
				Selection = 
				{
					ID = idGuiHudMB_ModeSelect,
					Style = GUI_FitChildren | GUI_GridLayout,
					Left = "10em",
					
					ModeBrush =
					{
						Priority = 1,
						BackgroundColor = bgBrush,
						Bottom = "2em",
						Right = "10em",
						
						icon =
						{
							Symbol = SprayCan,
							Right = "2em"
						},
						
						text = 
						{
							Text = "$MBBrushModeBrush$",
							Left = "2.5em",
							Style = GUI_TextVCenter,
						},
						
						OnClick = GuiAction_Call(this, "SelectBrushMode", 1),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					ModeQuad =
					{
						Priority = 2,
						BackgroundColor = bgQuad,
						Bottom = "2em",
						Right = "10em",
						
						icon =
						{
							Symbol = SprayCan,
							Right = "2em"
						},
						
						text = 
						{
							Text = "$MBBrushModeQuad$",
							Left = "2.5em",
							Style = GUI_TextVCenter,
						},
						
						OnClick = GuiAction_Call(this, "SelectBrushMode", 2),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					ModeErase =
					{
						Priority = 3,
						BackgroundColor = bgErase,
						Bottom = "2em",
						Right = "10em",
						
						icon =
						{
							Symbol = SprayCan,
							Right = "2em"
						},
						
						text = 
						{
							Text = "$MBBrushModeErase$",
							Left = "2.5em",
							Style = GUI_TextVCenter,
						},
						
						OnClick = GuiAction_Call(this, "SelectBrushMode", 3),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
				}
			},
		}
	};
	
	GuiAddCloseButton(BrushUI, this, "HideMaterialBrushUI");
	
	var index = 0;
	for (var property in GetProperties(BrushMaterials))
	{
		var entry = BrushMaterials[property];
		index++;
		
		var stdcolor = 0;
		if (entry.Material == SelectedBrushMaterial) stdcolor = RGBa(128,192,128,128);
		
		var matentry = 
		{
			ID = 2000 + index,
			Priority = index,
			BackgroundColor = { Std = stdcolor, Hover = RGBa(128,128,192,128) },
			Bottom = "2em",
			Right = "10em",
			
			icon = 
			{
				Symbol = entry.Icon,
				Right = "2em"
			},
			
			text =
			{
				Text = entry.Caption,
				Left = "2.5em",
				Style = GUI_TextVCenter
			},
			
			OnClick = GuiAction_Call(this, "SelectBrushMaterial", entry.Material),
			OnMouseIn = GuiAction_SetTag("Hover"),
			OnMouseOut = GuiAction_SetTag("Std"),
		};
		
		GuiAddSubwindow(matentry, BrushUI.OptionList.OptMaterial.Selection);
	}
	
	index = 0;
	for (var property in GetProperties(BrushBackgroundMaterials))
	{
		var entry = BrushBackgroundMaterials[property];
		index++;
		
		var stdcolor = 0;
		if (entry.Material == SelectedBrushBgMaterial) stdcolor = RGBa(128,192,128,128);
		
		var matentry = 
		{
			ID = 3000 + index,
			Priority = index,
			BackgroundColor = { Std = stdcolor, Hover = RGBa(128,128,192,128) },
			Bottom = "2em",
			Right = "10em",
			
			icon = 
			{
				Symbol = entry.Icon,
				Right = "2em"
			},
			
			text =
			{
				Text = entry.Caption,
				Left = "2.5em",
				Style = GUI_TextVCenter
			},
			
			OnClick = GuiAction_Call(this, "SelectBrushBackgroundMaterial", entry.Material),
			OnMouseIn = GuiAction_SetTag("Hover"),
			OnMouseOut = GuiAction_SetTag("Std"),
		};
		
		GuiAddSubwindow(matentry, BrushUI.OptionList.OptMaterialBg.Selection);
	}
	
	idHudMB = GuiOpen(BrushUI);
	
	SetMenu(idHudMB);
	
	return idHudMB;
}

func HideMaterialBrushUI()
{
	if (idHudMB)
	{
		GuiClose(idHudMB);
		idHudMB = nil;
		MenuClosed();
	}
}

func SelectBrushMaterial(data)
{
	SelectedBrushMaterial = data;
	
	var update =
	{
		OptionList =
		{
			OptMaterial =
			{
				Selection =
				{
					ID = idGuiHudMB_MatSelect,
					Style = GUI_FitChildren | GUI_GridLayout,
					Left = "10em"
				}
			}
		}
	};
	
	var index = 0;
	for (var property in GetProperties(BrushMaterials))
	{
		var entry = BrushMaterials[property];
		index++;
		
		var stdcolor = 0;
		if (entry.Material == SelectedBrushMaterial) stdcolor = RGBa(128,192,128,128);
		
		var matentry = 
		{
			ID = 2000 + index,
			Priority = index,
			BackgroundColor = { Std = stdcolor, Hover = RGBa(128,128,192,128) },
			Bottom = "2em",
			Right = "10em",
			
			icon = 
			{
				Symbol = entry.Icon,
				Right = "2em"
			},
			
			text =
			{
				Text = entry.Caption,
				Left = "2.5em",
				Style = GUI_TextVCenter
			},
			
			OnClick = GuiAction_Call(this, "SelectBrushMaterial", entry.Material),
			OnMouseIn = GuiAction_SetTag("Hover"),
			OnMouseOut = GuiAction_SetTag("Std"),
		};
		
		GuiAddSubwindow(matentry, update.OptionList.OptMaterial.Selection);
	}
	
	GuiClose(idHudMB, idGuiHudMB_MatSelect);
	GuiUpdate(update, idHudMB);
}

func SelectBrushBackgroundMaterial(data)
{
	SelectedBrushBgMaterial = data;
	
	var update =
	{
		OptionList =
		{
			OptMaterialBg =
			{
				Selection =
				{
					ID = idGuiHudMB_MatBgSelect,
					Style = GUI_FitChildren | GUI_GridLayout,
					Left = "10em"
				}
			}
		}
	};
	
	var index = 0;
	for (var property in GetProperties(BrushBackgroundMaterials))
	{
		var entry = BrushBackgroundMaterials[property];
		index++;
		
		var stdcolor = 0;
		if (entry.Material == SelectedBrushBgMaterial) stdcolor = RGBa(128,192,128,128);
		
		var matentry = 
		{
			ID = 2000 + index,
			Priority = index,
			BackgroundColor = { Std = stdcolor, Hover = RGBa(128,128,192,128) },
			Bottom = "2em",
			Right = "10em",
			
			icon = 
			{
				Symbol = entry.Icon,
				Right = "2em"
			},
			
			text =
			{
				Text = entry.Caption,
				Left = "2.5em",
				Style = GUI_TextVCenter
			},
			
			OnClick = GuiAction_Call(this, "SelectBrushBackgroundMaterial", entry.Material),
			OnMouseIn = GuiAction_SetTag("Hover"),
			OnMouseOut = GuiAction_SetTag("Std"),
		};
		
		GuiAddSubwindow(matentry, update.OptionList.OptMaterialBg.Selection);
	}
	
	GuiClose(idHudMB, idGuiHudMB_MatBgSelect);
	GuiUpdate(update, idHudMB);
}

func BrushSizeChange(valuechange)
{
	SelectedBrushSize += valuechange;
	if (SelectedBrushSize < 1) SelectedBrushSize = 1;
	
	var update =
	{
		OptionList =
		{
			OptBrushSize =
			{
				Selection =
				{
					Value =
					{
						Text = Format("<c ffff00>%d</c>", SelectedBrushSize)
					}
				}
			}
		}
	};
	
	GuiUpdate(update, idHudMB);
}

func SelectBrushMode(data)
{
	SelectedBrushMode = data;
	
	var bgBrush = { Std = 0, Hover = RGBa(128,128,192,128) };
	var bgQuad  = bgBrush;
	var bgErase = bgBrush;
	
	if (SelectedBrushMode == 1) bgBrush = { Std = RGBa(128,192,128,128), Hover = RGBa(128,128,192,128) };
	if (SelectedBrushMode == 2) bgQuad  = { Std = RGBa(128,192,128,128), Hover = RGBa(128,128,192,128) };
	if (SelectedBrushMode == 3) bgErase = { Std = RGBa(128,192,128,128), Hover = RGBa(128,128,192,128) };
	
	var update =
	{
		OptionList =
		{
			OptBrushMode =
			{
				Selection =
				{
					ModeBrush =
					{
						BackgroundColor = bgBrush
					},
					
					ModeQuad =
					{
						BackgroundColor = bgQuad
					},
					
					ModeErase =
					{
						BackgroundColor = bgErase
					}
				}
			}
		}
	};
	
	GuiUpdate(update, idHudMB);
}


local idHudTW;

func ShowTweaksUI()
{
	var TweaksUI =
	{
		Player = GetOwner(),
		Decoration = GUI_MenuDeco,
		
		OptionList =
		{
			Top = "2em",
			Style = GUI_VerticalLayout,
			
			OptInvincible =
			{
				ID = 50,
				Priority = 1,
				Bottom = "2em",
				Style = GUI_FitChildren,
				
				Caption = 
				{
					Right = "12em",
					Text = "$TweakInvincible$",					
				},
				
				Selection = 
				{
					Style = GUI_GridLayout | GUI_FitChildren,
					Left = "12em",
					
					BtnEnable =
					{
						Priority = 1,
						Bottom = "2em",
						Right = "10em",
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						icon =
						{
							Symbol = Icon_Ok,
							Right = "2em",
						},
						
						text = 
						{
							Text = "$OptActivate$",
							Left = "2.5em",
							Style = GUI_TextVCenter,
						},
						
						OnClick = GuiAction_Call(this, "SetInvincibility", true),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					BtnDisable =
					{
						Priority = 2,
						Bottom = "2em",
						Right = "10em",
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						icon =
						{
							Symbol = Icon_Cancel,
							Right = "2em",
						},
						
						text = 
						{
							Text = "$OptDeactivate$",
							Left = "2.5em",
							Style = GUI_TextVCenter,
						},
						
						OnClick = GuiAction_Call(this, "SetInvincibility", false),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
				}
			},
			
			OptSkinChoose =
			{
				ID = 51,
				Priority = 2,
				Bottom = "2em",
				Style = GUI_FitChildren,
				
				Caption = 
				{
					Right = "12em",
					Text = "$TweakSkinChoose$",
				},
				
				Selection = 
				{
					Style = GUI_GridLayout | GUI_FitChildren,
					Left = "12em",
					
					BtnSkin1 =
					{
						Priority = 1,
						Bottom = "2em",
						Right = "10em",
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						icon =
						{
							Symbol = Clonk,
							Right = "2em",
						},
						
						text = 
						{
							Text = "$OptSkin1$",
							Left = "2.5em",
							Style = GUI_TextVCenter,
						},
						
						OnClick = GuiAction_Call(this, "SetSkin", 0),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					BtnSkin2 =
					{
						Priority = 2,
						Bottom = "2em",
						Right = "10em",
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						icon =
						{
							Symbol = Clonk,
							GraphicsName = "Steampunk",
							Right = "2em",
						},
						
						text = 
						{
							Text = "$OptSkin2$",
							Left = "2.5em",
							Style = GUI_TextVCenter,
						},
						
						OnClick = GuiAction_Call(this, "SetSkin", 1),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					BtnSkin3 =
					{
						Priority = 3,
						Bottom = "2em",
						Right = "10em",
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						icon =
						{
							Symbol = Clonk,
							GraphicsName = "Alchemist",
							Right = "2em",
						},
						
						text = 
						{
							Text = "$OptSkin3$",
							Left = "2.5em",
							Style = GUI_TextVCenter,
						},
						
						OnClick = GuiAction_Call(this, "SetSkin", 2),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					BtnSkin4 =
					{
						Priority = 4,
						Bottom = "2em",
						Right = "10em",
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						icon =
						{
							Symbol = Clonk,
							GraphicsName = "Farmer",
							Right = "2em",
						},
						
						text = 
						{
							Text = "$OptSkin4$",
							Left = "2.5em",
							Style = GUI_TextVCenter,
						},
						
						OnClick = GuiAction_Call(this, "SetSkin", 3),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					
				}
			}
		}
		
	};
	
	GuiAddCloseButton(TweaksUI, this, "HideTweaksUI");
	
	idHudTW = GuiOpen(TweaksUI);
	
	SetMenu(idHudTW);
	
	return idHudTW;
}

func HideTweaksUI()
{
	if (idHudTW)
	{
		GuiClose(idHudTW);
		MenuClosed();
		idHudTW = nil;
	}
}


local idHudMG;
static MapGenSizeWidth;
static MapGenSizeHeight;

static MapGenPreset;
static MapGenTreesAmount;

local idGuiHudMG_SizeSelectWidth = 400;
local idGuiHudMG_SizeSelectHeight = 401;
local idGuiHudMG_TypePresetList = 402;

local MapGenTypePresetOpts =
{
	// Custom =
	// {
		// Priority = 1,
		// Caption = "$MapGenTPCustom$",
		// Icon = Hammer,
		// Value = "Custom"
	// },
	
	FlatLand =
	{
		Priority = 2,
		Caption = "$MapGenTPFlatLand$",
		Icon = Earth,
		Value = "FlatLand"
	},
	
	Skylands =
	{
		Priority = 3,
		Caption = "$MapGenTPSkylands$",
		Icon = Earth,
		Value = "Skylands"
	},
	
	Caves =
	{
		Priority = 4,
		Caption = "$MapGenTPCaves$",
		Icon = Earth,
		Value = "Caves"
	}
};

func ShowMapGenUI()
{
	var MapGenUI =
	{
		Player = GetOwner(),
		Decoration = GUI_MenuDeco,
		
		OptionList = 
		{
			Top = "2em",
			Bottom = "100% - 2em",
			Style = GUI_VerticalLayout,
			
			OptMapSizeWidth =
			{
				Priority = 1,
				Style = GUI_FitChildren,
				Bottom = "2em",
				
				Caption = 
				{
					Text = "$MapGenSizeWidth$",
					Right = "12em",
				},
				
				Selection = 
				{
					ID = idGuiHudMG_SizeSelectWidth,
					Style = GUI_GridLayout | GUI_FitChildren,
					Left = "12em",
					
					Minus10 =
					{
						Priority = 1,
						
						Symbol = Icon_Number,
						GraphicsName = "Minus",
						Text = "10",
						Style = GUI_TextBottom | GUI_TextRight,
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						Right = "2em",
						Bottom = "2em",
						
						OnClick = GuiAction_Call(this, "MapGenSizeWidthChange", -10),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					Minus1 =
					{
						Priority = 2,
						
						Symbol = Icon_Number,
						GraphicsName = "Minus",
						Text = "1",
						Style = GUI_TextBottom | GUI_TextRight,
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						Right = "2em",
						Bottom = "2em",
						
						OnClick = GuiAction_Call(this, "MapGenSizeWidthChange", -1),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					Value =
					{
						Priority = 3,
						Style = GUI_TextHCenter | GUI_TextVCenter,
						Text = Format("<c ffff00>%d</c>", MapGenSizeWidth),
						
						Right = "4em",
						Bottom = "2em",
					},
					
					Plus1 =
					{
						Priority = 4,
						
						Symbol = Icon_Number,
						GraphicsName = "Plus",
						Text = "1",
						Style = GUI_TextBottom | GUI_TextRight,
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						Right = "2em",
						Bottom = "2em",
						
						OnClick = GuiAction_Call(this, "MapGenSizeWidthChange", 1),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					Plus10 =
					{
						Priority = 5,
						
						Symbol = Icon_Number,
						GraphicsName = "Plus",
						Text = "10",
						Style = GUI_TextBottom | GUI_TextRight,
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						Right = "2em",
						Bottom = "2em",
						
						OnClick = GuiAction_Call(this, "MapGenSizeWidthChange", 10),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
				}
			},
			
			OptMapSizeHeight =
			{
				Priority = 2,
				Style = GUI_FitChildren,
				Bottom = "2em",
				
				Caption = 
				{
					Text = "$MapGenSizeHeight$",
					Right = "12em",
				},
				
				Selection = 
				{
					ID = idGuiHudMG_SizeSelectWidth,
					Style = GUI_GridLayout | GUI_FitChildren,
					Left = "12em",
					
					Minus10 =
					{
						Priority = 1,
						
						Symbol = Icon_Number,
						GraphicsName = "Minus",
						Text = "10",
						Style = GUI_TextBottom | GUI_TextRight,
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						Right = "2em",
						Bottom = "2em",
						
						OnClick = GuiAction_Call(this, "MapGenSizeHeightChange", -10),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					Minus1 =
					{
						Priority = 2,
						
						Symbol = Icon_Number,
						GraphicsName = "Minus",
						Text = "1",
						Style = GUI_TextBottom | GUI_TextRight,
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						Right = "2em",
						Bottom = "2em",
						
						OnClick = GuiAction_Call(this, "MapGenSizeHeightChange", -1),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					Value =
					{
						Priority = 3,
						Style = GUI_TextHCenter | GUI_TextVCenter,
						Text = Format("<c ffff00>%d</c>", MapGenSizeHeight),
						
						Right = "4em",
						Bottom = "2em",
					},
					
					Plus1 =
					{
						Priority = 4,
						
						Symbol = Icon_Number,
						GraphicsName = "Plus",
						Text = "1",
						Style = GUI_TextBottom | GUI_TextRight,
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						Right = "2em",
						Bottom = "2em",
						
						OnClick = GuiAction_Call(this, "MapGenSizeHeightChange", 1),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					Plus10 =
					{
						Priority = 5,
						
						Symbol = Icon_Number,
						GraphicsName = "Plus",
						Text = "10",
						Style = GUI_TextBottom | GUI_TextRight,
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						Right = "2em",
						Bottom = "2em",
						
						OnClick = GuiAction_Call(this, "MapGenSizeHeightChange", 10),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
				}
			},
			
			OptPreset =
			{
				Priority = 3,
				Bottom = "2em",
				Style = GUI_FitChildren,
				
				Caption =
				{
					Text = "$MapGenMapType$",
					Right = "12em",
				},
				
				Selection =
				{
					ID = idGuiHudMG_TypePresetList,
				}
			}
		},
		
		BtnGenerate =
		{
			Top = "100% - 2em",
			Text = "$MapGenButtonGenerate$",
			Style = GUI_TextHCenter | GUI_TextVCenter,
			BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
			
			OnClick = GuiAction_Call(this, "MakeNewMap"),
			OnMouseIn = GuiAction_SetTag("Hover"),
			OnMouseOut = GuiAction_SetTag("Std"),
		}
	};
	
	GuiAddCloseButton(MapGenUI, this, "HideMapGenUI");
	
	idHudMG = GuiOpen(MapGenUI);
	
	SetMenu(idHudMG);
	
	UpdateMapGenPresetOptionList();
	
	return idHudMG;
}

func HideMapGenUI()
{
	if (idHudMG)
	{
		GuiClose(idHudMG);
		MenuClosed();
		idHudMG = nil;
	}
}

func UpdateMapGenPresetOptionList()
{	
	GuiClose(idHudMG, idGuiHudMG_TypePresetList);
	
	var update = 
	{
		OptionList =
		{
			OptPreset =
			{
				Selection =
				{
					ID = idGuiHudMG_TypePresetList,
					Left = "12em",
					Style = GUI_GridLayout | GUI_FitChildren,
				}
			}
		}
	};
	
	GuiUpdate(update, idHudMG);
	
	for (var property in GetProperties(MapGenTypePresetOpts))
	{
		var entry = MapGenTypePresetOpts[property];
		
		var bgcolor = { Std = 0, Hover = RGBa(128,128,192,128) };
		if (MapGenPreset == entry.Value) bgcolor = { Std = RGBa(128,192,128,128), Hover = RGBa(128,128,192,128) };
		
		var subentry =
		{
			Right = "10em",
			Bottom = "2em",
			Priority = entry.Priority,
			BackgroundColor = bgcolor,
			
			icon =
			{
				Symbol = entry.Icon,
				Right = "2em",
			},
			
			text =
			{
				Text = entry.Caption,
				Left = "2.5em",
				Style = GUI_TextVCenter,
			},
			
			OnClick = GuiAction_Call(this, "MapGenSelectPreset", entry.Value),
			OnMouseIn = GuiAction_SetTag("Hover"),
			OnMouseOut = GuiAction_SetTag("Std"),
		};
		
		GuiAddSubwindow(subentry, update.OptionList.OptPreset.Selection);
	}
	
	GuiUpdate(update, idHudMG);
	
	
}

func MapGenSelectPreset(data)
{
	MapGenPreset = data;
	UpdateMapGenPresetOptionList();
}

func MapGenSizeWidthChange(valuechange)
{
	MapGenSizeWidth += valuechange;
	if (MapGenSizeWidth < 1) MapGenSizeWidth = 1;
	if (MapGenSizeWidth > 1000) MapGenSizeWidth = 1000;
	
	var update =
	{
		OptionList =
		{
			OptMapSizeWidth =
			{
				Selection =
				{
					Value =
					{
						Text = Format("<c ffff00>%d</c>", MapGenSizeWidth)
					}
				}
			}
		}
	};
	
	GuiUpdate(update, idHudMG);
}

func MapGenSizeHeightChange(valuechange)
{
	MapGenSizeHeight += valuechange;
	if (MapGenSizeHeight < 1) MapGenSizeHeight = 1;
	if (MapGenSizeHeight > 1000) MapGenSizeHeight = 1000;
	
	var update =
	{
		OptionList =
		{
			OptMapSizeHeight =
			{
				Selection =
				{
					Value =
					{
						Text = Format("<c ffff00>%d</c>", MapGenSizeHeight)
					}
				}
			}
		}
	};
	
	GuiUpdate(update, idHudMG);
}

func MakeNewMap()
{
	var clonks = [];
	for (var clonk in FindObjects(Find_OCF(OCF_CrewMember)))
	{
		var container = clonk->Contained();
		if (container)
		{
			clonk->Exit();
			container->RemoveObject();
		}
		else
		{
			// Players not waiting for a relaunch get a new Clonk to prevent
			// status effects from carrying over to the next round.
			var new_clonk = CreateObject(clonk->GetID(), 0, 0, clonk->GetOwner());
			new_clonk->GrabObjectInfo(clonk);
			clonk = new_clonk;
		}
		PushBack(clonks, clonk);
		clonk->SetObjectStatus(C4OS_INACTIVE);
	}
	
	LoadScenarioSection("main");
	
	for (var clonk in clonks)
	{
		clonk->SetObjectStatus(C4OS_NORMAL);
		SetCursor(clonk->GetOwner(), clonk);
		clonk->SetPosition(LandscapeWidth() / 2, LandscapeHeight() / 2);
		clonk->Unstick(20);
	}
	
	for(var i = 0; i < GetPlayerCount(); i++)
	{
		GameCall("InitializePlayer", GetPlayerByIndex(i));
	}
	
	// Things to do after the Clonks have respawned on new map
	PostMapGen();
	
}


local idHudMK;
local idGuiHudMK_MarkerList = 610;

func ShowMarkerUI()
{
	var MarkerUI =
	{
		Player = GetOwner(),
		Decoration = GUI_MenuDeco,
		
		OptionList =
		{
			ID = idGuiHudMK_MarkerList,
			Top = "2em",
			Bottom = "100% - 2em",
			Style = GUI_VerticalLayout,
			
		},
		
		BtnPlaceMarker = 
		{
			Top = "100% - 2em",
			Text = "$PlaceMarker$",
			Style = GUI_TextHCenter | GUI_TextVCenter,
			
			BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
			
			OnClick = GuiAction_Call(this, "PlaceNewMarker"),
			OnMouseIn = GuiAction_SetTag("Hover"),
			OnMouseOut = GuiAction_SetTag("Std"),
		}
	};
	
	GuiAddCloseButton(MarkerUI, this, "HideMarkerUI");
	
	idHudMK = GuiOpen(MarkerUI);
	
	SetMenu(idHudMK);
	
	UpdateMarkerList();
	
	return idHudMK;
}

func HideMarkerUI()
{
	if (idHudMK)
	{
		GuiClose(idHudMK);
		MenuClosed();
		idHudMK = nil;
	}
}

func UpdateMarkerList()
{
	if (!idHudMK) return;
	
	GuiClose(idHudMK, idGuiHudMK_MarkerList);
	
	var update =
	{
		ID = idGuiHudMK_MarkerList,
		Top = "2em",
		Bottom = "100% - 2em",
		Style = GUI_VerticalLayout,
	};
	GuiUpdate({ OptionList = update }, idHudMK);
	
	for (var i = 0; i <= 9; i++)
	{
		var marker = GetMarkerForIndex(i, GetOwner());
		var submarker;
		
		if (marker)
		{
			submarker =
			{
				Priority = i + 1,
				Style = GUI_FitChildren,
				Bottom = "2em",
				
				Caption =
				{
					Right = "4em",
					Symbol = Icon_Number,
					GraphicsName = Format("%d", i),
				},
				
				Selection =
				{
					Bottom = "2em",
					Left = "4em",
					Style = GUI_GridLayout,
					
					BtnTeleport =
					{
						Priority = 1,
						Right = "10em",
						Bottom = "2em",
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						icon =
						{
							Symbol = Teleporter,
							Right = "2em",
						},
						
						text =
						{
							Text = "$GoToMarker$",
							Left = "2.5em",
							Style = GUI_TextVCenter,
						},
						
						OnClick = GuiAction_Call(this, "GoToMarker", marker),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					BtnRemove =
					{
						Priority = 2,
						Right = "10em",
						Bottom = "2em",
						BackgroundColor = { Std = 0, Hover = RGBa(128,128,192,128) },
						
						icon =
						{
							Symbol = Icon_Cancel,
							Right = "2em",
						},
						
						text =
						{
							Text = "$RemoveMarker$",
							Left = "2.5em",
							Style = GUI_TextVCenter,
						},
						
						OnClick = GuiAction_Call(this, "RemoveMarker", marker),
						OnMouseIn = GuiAction_SetTag("Hover"),
						OnMouseOut = GuiAction_SetTag("Std"),
					},
					
					
				}
			};
		}
		else
		{
			submarker =
			{
				Priority = i + 1,
				Style = GUI_FitChildren,
				Bottom = "2em",
				BackgroundColor = RGBa(192,0,0,128),
				
				Caption =
				{
					Right = "4em",
					Symbol = Icon_Number,
					GraphicsName = Format("%d", i),
				},
				
				Selection =
				{
					Bottom = "2em",
					Text = "$MarkerNotSet$",
					Style = GUI_TextHCenter | GUI_TextVCenter,
				}
			};
		}
		
		GuiAddSubwindow(submarker, update);
	}
	GuiUpdate(update, idHudMK, idGuiHudMK_MarkerList);
}

func PlaceNewMarker()
{
	var newindex = GetNextFreeMarkerIndex(GetOwner());
	
	if (newindex != nil)
	{
		var marker = this->CreateObject(Marker);
		
		marker->SetIcon(newindex);
		
		UpdateMarkerList();
		
		return marker;
	}
	else
	{
		Sound("UI::Error");
		UpdateMarkerList();
		return nil;
	}
}

func RemoveMarker(marker)
{
	marker->RemoveObject();
	UpdateMarkerList();
}

func GoToMarker(marker)
{
	this->SetPosition(marker->GetX(), marker->GetY());
	this->Sound("warp");
	this->Fireworks();
}
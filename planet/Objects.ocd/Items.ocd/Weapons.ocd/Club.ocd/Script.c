/*-- Club --*/

#include Library_MeleeWeapon

private func Hit()
{
	Sound("WoodHit?");
}

public func GetCarryMode() { return CARRY_HandBack; }

public func GetCarrySpecial(clonk)
{
	if(fAiming)
	{
		if(clonk->GetHandPosByItemPos(clonk->GetItemPos(this)) == 1)
			return "pos_hand1";
		else
			return "pos_hand2";
	}
}

local animation_set;

func Initialize()
{
	ClubChangeHandAnims("R");
}

private func ClubChangeHandAnims(string hand)
{
	if(hand == "R")
	{
	//Changes which (R/L) animation is used
		animation_set = {
		AimMode         = AIM_Weight,
		AnimationAim    = Format("BatAimArms.R",hand),
		AnimationAim2   = Format("BatAim2Arms.R",hand),
		AimTime         = 35*3,
		AnimationShoot  = Format("BatStrikeArms.R",hand),
		AnimationShoot2 = Format("BatStrike2Arms.R",hand),
		ShootTime       = 35/2,
		ShootTime2      = (35/2)*6/19, // At 6/19 of the shooting animation
	};
	}
	else
	{
		//Changes which (R/L) animation is used
		animation_set = {
		AimMode         = AIM_Weight,
		AnimationAim    = "BatAimArms.L",
		AnimationAim2   = "BatAim2Arms.L",
		AimTime         = 35*3,
		AnimationShoot  = "BatStrikeArms.L",
		AnimationShoot2 = "BatStrike2Arms.L",
		ShootTime       = 35/2,
		ShootTime2      = (35/2)*6/19, // At 6/19 of the shooting animation
	};
	}
}

public func GetAnimationSet() { return animation_set; }

public func HoldingEnabled() { return true; }

local fAiming;

public func RejectUse(object clonk)
{
	return !CanStrikeWithWeapon(clonk) || !clonk->HasHandAction();
}

public func ControlUseStart(object clonk, int x, int y)
{
	if(clonk->GetHandPosByItemPos(clonk->GetItemPos(this)) == 0)
		ClubChangeHandAnims("R");
	else
		ClubChangeHandAnims("L");

	fAiming = true;

	clonk->StartAim(this);
	return 1;
}

public func HoldingEnabled() { return true; }

func ControlUseHolding(object clonk, ix, iy)
{
	var angle = Angle(0,0,ix,iy);
	angle = Normalize(angle,-180);

	clonk->SetAimPosition(angle);

	return true;
}

public func ControlUseStop(object clonk, ix, iy)
{
	clonk->StopAim();
	return true;
}

// Callback from the clonk, when he actually has stopped aiming
public func FinishedAiming(object clonk, int angle)
{
	clonk->StartShoot(this);
	
	// since the Clonk internal callback is only once, we cannot use it
	// ..
	AddEffect("DuringClubShootControl", clonk, 1, 1, this, nil, angle);
	
	// aaaand, a cooldown
	AddEffect("ClubWeaponCooldown", clonk, 1, 5, this);
	return true;
}

protected func ControlUseCancel(object clonk, int x, int y)
{
	clonk->CancelAiming(this);
	return true;
}

public func Reset(clonk)
{
	fAiming = 0;
}

// Called in the half of the shoot animation (when ShootTime2 is over)
public func DuringShoot(object clonk, int angle)
{
	// called only once. We don't want it only once..
	// DoStrike(clonk, angle);
}

func FxDuringClubShootControlStart(target, effect, temp, p1)
{
	if(temp) return;
	effect.angle=p1;
}

func FxDuringClubShootControlStop(target, effect, reason, temp)
{
	if(temp) return;
	AddEffect("AfterClubShootControl", target, 1, 15, this);
}

func FxDuringClubShootControlTimer(target, effect, effect_time)
{
	if(effect_time > 16) return -1;
	if(!this) return -1;
	this->DoStrike(target, effect.angle);
}

// you are not going to be hit by objects you fling away
func FxDuringClubShootControlQueryCatchBlow(object target, effect, object obj)
{
	this->DoStrike(target, effect.angle);
	var en=Format("CannotBeHitTwiceBy%d", this->ObjectNumber());
	if(GetEffect(en, obj)) return true;
	return false;
}

func FxAfterClubShootControlTimer()
{
	return -1;
}

func FxAfterClubShootControlQueryCatchBlow(object target, effect, object obj)
{
	var en=Format("CannotBeHitTwiceBy%d", this->ObjectNumber());
	if(GetEffect(en, obj)) return true;
	return false;
}

func DoStrike(clonk, angle)
{
	var x=Sin(angle, 7);
	var y=-Cos(angle, 7);
	var found=false;
	for(var obj in FindObjects(Find_Distance(7, x, y), Find_Or(Find_OCF(OCF_Alive), Find_Category(C4D_Object), Find_Category(C4D_Vehicle)), Find_Exclude(clonk), Find_NoContainer(), Find_Layer(GetObjectLayer())))
	{
		if(obj->Stuck()) continue;
		
		// vehicles are only hit if they are pseudo vehicles. Bad system - has to be changed in the future
		if(obj->GetCategory() & C4D_Vehicle)
			if(!GetEffect("HitCheck", obj)) continue;
		
		var en=Format("CannotBeHitTwiceBy%d", this->ObjectNumber());
		if(GetEffect(en, obj)) continue;
		
		if(obj->GetOCF() & OCF_Alive)
		{
			var damage=5*1000;
			ApplyWeaponBash(obj, 400, angle);
			obj->DoEnergy(-damage, true, FX_Call_EngGetPunched, clonk->GetOwner());
		}
		else
		{
			var div=100;
			if(obj->GetContact(-1)) div*=10;
			
			// mass/size factor
			var fac1=10000/Max(2, obj->GetMass());
			var fac2=BoundBy(10-Abs(obj->GetDefCoreVal("Width", "DefCore")-obj->GetDefCoreVal("Height", "DefCore")), 1, 10);
			var speed=(3000 * fac1 * fac2) / 10 / 1000;
			obj->SetXDir((obj->GetXDir(100) + Sin(angle, speed)) / 2, div);
			obj->SetYDir((obj->GetYDir(100) - Cos(angle, speed)) / 2, div);
		}
		AddEffect(en, obj, 1, 15, nil);
		found=true;
		break;
	}
	
	if(found)
		RemoveEffect("DuringClubShoot", clonk);
}

public func IsWeapon() { return true; }
public func IsArmoryProduct() { return true; }

func Definition(def) {
	SetProperty("PictureTransformation",Trans_Rotate(-30,0,0,1),def);
}

local Collectible = 1;
local Name = "$Name$";
local Description = "$Description$";
local UsageHelp = "$UsageHelp$";
local Rebuy = true;

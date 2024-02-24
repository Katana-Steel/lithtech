// ----------------------------------------------------------------------- //
//
// MODULE  : DamageTypes.cpp
//
// PURPOSE : Implementation of damage types
//
// CREATED : 01/11/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DamageTypes.h"

// ----------------------------------------------------------------------- //

struct dt_to_flag_struct
{
	dt_to_flag_struct(uint32 d = 0, char *n = LTNULL, LTFLOAT fHP = 0.0f, LTFLOAT fAP = 0.0f, LTBOOL bDR = LTFALSE)
	{
		dwFlag	= d;
		szName	= n;
		fHealthDamagePercent = fHP;
		fArmorDamagePercent = fAP;
		bUseDamageResistance = bDR;
	}

	uint32		dwFlag;
	char		*szName;
	LTFLOAT		fHealthDamagePercent;
	LTFLOAT		fArmorDamagePercent;
	LTBOOL		bUseDamageResistance;
};

// ----------------------------------------------------------------------- //

static dt_to_flag_struct DamageTypeInfoArray[] =
{
	dt_to_flag_struct((1<<DT_UNSPECIFIED),	"Unspecified",	0.0f, 1.0f,		LTFALSE),
	dt_to_flag_struct((1<<DT_BULLET),		"Bullet",		0.0f, 1.0f,		LTTRUE),
	dt_to_flag_struct((1<<DT_BURN),			"Burn",			0.75f, 0.25f,	LTFALSE),
	dt_to_flag_struct((1<<DT_ACID),			"Acid",			0.5f, 0.5f,		LTFALSE),
	dt_to_flag_struct((1<<DT_CHOKE),		"Choke",		1.0f, 0.0f,		LTFALSE),
	dt_to_flag_struct((1<<DT_CRUSH),		"Crush",		0.0f, 1.0f,		LTFALSE),
	dt_to_flag_struct((1<<DT_ELECTRIC),		"Electric",		0.25f, 0.75f,	LTTRUE),
	dt_to_flag_struct((1<<DT_EXPLODE),		"Explode",		0.1f, 0.9f,		LTTRUE),
	dt_to_flag_struct((1<<DT_FREEZE),		"Freeze",		0.0f, 1.0f,		LTFALSE),
	dt_to_flag_struct((1<<DT_POISON),		"Poison",		1.0f, 0.0f,		LTFALSE),
	dt_to_flag_struct((1<<DT_BLEEDING),		"Bleeding",		1.0f, 0.0f,		LTFALSE),
	dt_to_flag_struct((1<<DT_STUN),			"Stun",			0.0f, 1.0f,		LTTRUE),
	dt_to_flag_struct((1<<DT_BITE),			"Bite",			0.0f, 1.0f,		LTTRUE),
	dt_to_flag_struct((1<<DT_HEADBITE),		"HeadBite",		0.0f, 1.0f,		LTFALSE),
	dt_to_flag_struct((1<<DT_ENDLESS_FALL),	"Endless Fall",	0.0f, 1.0f,		LTFALSE),
	dt_to_flag_struct((1<<DT_ALIEN_CLAW),	"Alien Claw",	0.0f, 1.0f,		LTTRUE),
	dt_to_flag_struct((1<<DT_EMP),			"EMP",			0.0f, 1.0f,		LTFALSE),
	dt_to_flag_struct((1<<DT_NAPALM),		"Napalm",		0.75f, 0.25f,	LTFALSE),
	dt_to_flag_struct((1<<DT_SPEAR),		"Spear",		0.0f, 1.0f,		LTTRUE),
	dt_to_flag_struct((1<<DT_SLICE),		"Slice",		0.0f, 1.0f,		LTTRUE),
	dt_to_flag_struct((1<<DT_FACEHUG),		"Facehug",		1.0f, 0.0f,		LTFALSE),
	dt_to_flag_struct((1<<DT_TORCH_CUT),	"TorchCut",		1.0f, 0.0f,		LTFALSE),
	dt_to_flag_struct((1<<DT_TORCH_WELD),	"TorchWeld",	1.0f, 0.0f,		LTFALSE),
	dt_to_flag_struct((1<<DT_MARINE_HACKER),"MarineHacker",	1.0f, 0.0f,		LTFALSE),
	dt_to_flag_struct((1<<DT_HIGHCALIBER),	"HighCaliber",	0.0f, 1.0f,		LTFALSE),
	dt_to_flag_struct((1<<DT_TEAR),			"Tear",			0.0f, 1.0f,		LTFALSE),
	dt_to_flag_struct((1<<DT_PRED_HOTBOMB),	"Hotbomb",		0.0f, 1.0f,		LTFALSE),
	dt_to_flag_struct((1<<DT_SHOTGUN),		"Shotgun",		0.0f, 1.0f,		LTTRUE),
	dt_to_flag_struct((1<<DT_BULLET_NOGIB),	"Bullet No Gib",0.0f, 1.0f,		LTTRUE),
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetCameraTintColor()
//
//	PURPOSE:	Retrieve the tint color and tine type for this damage
//
// ----------------------------------------------------------------------- //

uint8 GetCameraTintColor(DamageType eType, LTVector &vTint)
{
	switch(eType)
	{
	case DT_TORCH_CUT:
	case DT_TORCH_WELD:
	case DT_BURN:
	case DT_NAPALM:
	case DT_CHOKE:
		vTint = LTVector(1.0f, 0.6f, 0.25f);
		return DAMAGETYPE_TINT_LIGHTADD;
		
	case DT_ACID:
		vTint = LTVector(0.85f, 0.65f, 0.0f);
		return DAMAGETYPE_TINT_LIGHTADD;
		
	case DT_MARINE_HACKER:
	case DT_ELECTRIC:
	case DT_EMP:
		vTint = LTVector(0.0f, 0.0f, 1.0f);
		return DAMAGETYPE_TINT_LIGHTADD;
		
	case DT_FREEZE:
		vTint = LTVector(0.75f, 0.75f, 1.0f);
		return DAMAGETYPE_TINT_LIGHTADD;
		
	case DT_POISON:
		vTint = LTVector(0.0f, 1.0f, 0.0f);
		return DAMAGETYPE_TINT_LIGHTADD;
		
	case DT_STUN:
	case DT_ENDLESS_FALL:
	case DT_FACEHUG:
		vTint = LTVector(0.0f, 0.0f, 0.0f);
		return DAMAGETYPE_TINT_LIGHTSCALE;
		
	default:
		vTint = LTVector(1.0f, 0.0f, 0.0f);
		return DAMAGETYPE_TINT_LIGHTADD;
	}
	
	return DAMAGETYPE_TINT_LIGHTADD;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToFlag()
//
//	PURPOSE:	Convert the damage type to its flag equivillent
//
// ----------------------------------------------------------------------- //

uint32 DamageTypeToFlag(DamageType eType)
{
	if((eType >= DT_UNSPECIFIED) && (eType < DT_MAX))
		return DamageTypeInfoArray[eType].dwFlag;

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeHPPercent()
//
//	PURPOSE:	Get the damage percent to apply to health
//
// ----------------------------------------------------------------------- //

LTFLOAT DamageTypeHPPercent(DamageType eType)
{
	if((eType >= DT_UNSPECIFIED) && (eType < DT_MAX))
		return DamageTypeInfoArray[eType].fHealthDamagePercent;

	return 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeAPPercent()
//
//	PURPOSE:	Get the damage percent to apply to armor
//
// ----------------------------------------------------------------------- //

LTFLOAT DamageTypeAPPercent(DamageType eType)
{
	if((eType >= DT_UNSPECIFIED) && (eType < DT_MAX))
		return DamageTypeInfoArray[eType].fArmorDamagePercent;

	return 1.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeResistance()
//
//	PURPOSE:	Get the damage percent to apply to armor
//
// ----------------------------------------------------------------------- //

LTBOOL DamageTypeResistance(DamageType eType)
{
	if((eType >= DT_UNSPECIFIED) && (eType < DT_MAX))
		return DamageTypeInfoArray[eType].bUseDamageResistance;

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToName()
//
//	PURPOSE:	Convert the damage type to its string name
//
// ----------------------------------------------------------------------- //

const char* DamageTypeToName(DamageType eType)
{
	for(int i = 0; i < DT_MAX; i++)
	{
		if(i == eType)
			return DamageTypeInfoArray[i].szName;
	}

	return "Invalid";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeFromName()
//
//	PURPOSE:	Convert the damage type name to its enum equivillent
//
// ----------------------------------------------------------------------- //

DamageType DamageTypeFromName(const char *szName)
{
	for(int i = 0; i < DT_MAX; i++)
	{
		if(!_stricmp(DamageTypeInfoArray[i].szName, szName))
			return (DamageType)i;
	}

	return DT_INVALID;
}


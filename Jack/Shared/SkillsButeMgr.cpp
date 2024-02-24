// ----------------------------------------------------------------------- //
//
// MODULE  : SkillsButeMgr.cpp
//
// PURPOSE : Implementation of bute mgr to manage skill related data
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SkillsButeMgr.h"


#define SMGR_RANK_TAG				"Ranks"
#define SMGR_RANK_DATA				"Rank"

#define SMGR_COST_TAG				"Cost"
#define SMGR_MP_TAG					"Multiplayer"

namespace
{
	char s_aTagName[30];
	char s_aAttName[100];

	char *szInternalSkillNames[kNumSkills] =
	{
		"Stealth",
		"Stamina",
		"Aim",
		"Carry",
		"Armor",
		"Weapon",
		"Gadget",
		"Search",
	};

}


CSkillsButeMgr* g_pSkillsButeMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkillsButeMgr::CSkillsButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSkillsButeMgr::CSkillsButeMgr()
{
	memset(m_nCosts,0,sizeof(m_nCosts));
	m_nIntelBonus = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkillsButeMgr::~CSkillsButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CSkillsButeMgr::~CSkillsButeMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkillsButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CSkillsButeMgr::Init(const char* szAttributeFile)
{
    if (g_pSkillsButeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile)) return LTFALSE;

	// Set up global pointer...

	g_pSkillsButeMgr = this;

	// Read in the properties for each mission...

	m_nNumRanks = 0;

	while (m_nNumRanks < SMGR_MAX_NUM_RANKS )
	{
		char szStr[512] ="";
		sprintf(s_aAttName, "%s%d", SMGR_RANK_DATA, m_nNumRanks);
		m_buteMgr.GetString(SMGR_RANK_TAG, s_aAttName, "", szStr,sizeof(szStr));
		if( !m_buteMgr.Success( ))
			break;

		char *pId = strtok(szStr,",");
		char *pMin = strtok(LTNULL,"");

		if (*pId && &pMin)
		{
			m_Ranks[m_nNumRanks].nNameId = atoi(pId);
			m_Ranks[m_nNumRanks].nMinScore = atoi(pMin);
		}	
		m_nNumRanks++;
		sprintf(s_aAttName, "%s%d", SMGR_RANK_DATA, m_nNumRanks);
	}

	for (int skl = 0; skl < kNumSkills; skl++)
	{
		char szStr[512] ="";
		m_buteMgr.GetString(SMGR_COST_TAG, szInternalSkillNames[skl],szStr,sizeof(szStr));
		int lvl = 1;
		char *pCost = strtok(szStr,",");
		uint32 cost = 0;
			
		while (lvl < kNumSkillLevels && pCost)
		{
			cost = atol(pCost);
			m_nCosts[skl][lvl] = cost;
			pCost = strtok(NULL,",");
			lvl++;

		}

	}

	for (int lvl = 0; lvl < kNumSkillLevels; lvl++)
	{
		sprintf(s_aTagName, "Stealth%d", lvl);
		if (m_buteMgr.Exist(s_aTagName))
		{
			m_Stealth[lvl].fModifier[StealthModifiers::eRadius] = (float)m_buteMgr.GetDouble(s_aTagName,"Radius",1.0);
			m_Stealth[lvl].fModifier[StealthModifiers::eHideTime] = (float)m_buteMgr.GetDouble(s_aTagName,"HideTime",1.0);
			m_Stealth[lvl].fModifier[StealthModifiers::eEvasion] = (float)m_buteMgr.GetDouble(s_aTagName,"Evasion",1.0);
		}

		sprintf(s_aTagName, "Stamina%d", lvl);
		if (m_buteMgr.Exist(s_aTagName))
		{
			m_Stamina[lvl].fModifier[StaminaModifiers::eMaxHealth] = (float)m_buteMgr.GetDouble(s_aTagName,"MaxHealth",1.0);
			m_Stamina[lvl].fModifier[StaminaModifiers::eMoveDamage] = (float)m_buteMgr.GetDouble(s_aTagName,"MoveDamage",1.0);
			m_Stamina[lvl].fModifier[StaminaModifiers::eResistance] = (float)m_buteMgr.GetDouble(s_aTagName,"Resistance",1.0);
			m_Stamina[lvl].fModifier[StaminaModifiers::eHealthPickup] = (float)m_buteMgr.GetDouble(s_aTagName,"HealthPickup",1.0);
			m_Stamina[lvl].fModifier[StaminaModifiers::eDamage] = (float)m_buteMgr.GetDouble(s_aTagName,"Damage",1.0);
		}

		sprintf(s_aTagName, "Aim%d", lvl);
		if (m_buteMgr.Exist(s_aTagName))
		{
			m_Aim[lvl].fModifier[AimModifiers::eAccuracy] = (float)m_buteMgr.GetDouble(s_aTagName,"Accuracy",1.0);
			m_Aim[lvl].fModifier[AimModifiers::eZoomSway] = (float)m_buteMgr.GetDouble(s_aTagName,"ZoomSway",1.0);
			m_Aim[lvl].fModifier[AimModifiers::eCorrection] = (float)m_buteMgr.GetDouble(s_aTagName,"Correction",1.0);
		}

		sprintf(s_aTagName, "Carry%d", lvl);
		if (m_buteMgr.Exist(s_aTagName))
		{
			m_Carry[lvl].fModifier[CarryModifiers::eMaxAmmo] = (float)m_buteMgr.GetDouble(s_aTagName,"MaxAmmo",1.0);
			m_Carry[lvl].fModifier[CarryModifiers::eCarryBodyMovement] = (float)m_buteMgr.GetDouble(s_aTagName,"CarryBodyMovement",1.0);
		}

		sprintf(s_aTagName, "Armor%d", lvl);
		if (m_buteMgr.Exist(s_aTagName))
		{
			m_Armor[lvl].fModifier[ArmorModifiers::eMaxArmor] = (float)m_buteMgr.GetDouble(s_aTagName,"MaxArmor",1.0);
			m_Armor[lvl].fModifier[ArmorModifiers::eArmorPickup] = (float)m_buteMgr.GetDouble(s_aTagName,"ArmorPickup",1.0);
		}

		sprintf(s_aTagName, "Weapon%d", lvl);
		if (m_buteMgr.Exist(s_aTagName))
		{
			m_Weapon[lvl].fModifier[WeaponModifiers::eDamage] = (float)m_buteMgr.GetDouble(s_aTagName,"Damage",1.0);
			m_Weapon[lvl].fModifier[WeaponModifiers::eReload] = (float)m_buteMgr.GetDouble(s_aTagName,"Reload",1.0);
		}

		sprintf(s_aTagName, "Gadget%d", lvl);
		if (m_buteMgr.Exist(s_aTagName))
		{
			m_Gadget[lvl].fModifier[GadgetModifiers::eEffect] = (float)m_buteMgr.GetDouble(s_aTagName,"Effect",1.0);
			m_Gadget[lvl].fModifier[GadgetModifiers::eSelect] = (float)m_buteMgr.GetDouble(s_aTagName,"Select",1.0);
		}

		sprintf(s_aTagName, "Search%d", lvl);
		if (m_buteMgr.Exist(s_aTagName))
		{
			m_Search[lvl].fModifier[SearchModifiers::eSearchSpeed] = (float)m_buteMgr.GetDouble(s_aTagName,"SearchSpeed",1.0);
			m_Search[lvl].fModifier[SearchModifiers::eFindJunk] = (float)m_buteMgr.GetDouble(s_aTagName,"FindJunk",1.0);
		}


	}

	

	m_nMPPool = 0;
	for (skl = 0; skl < kNumSkills; skl++)
	{
		int l = m_buteMgr.GetInt(SMGR_MP_TAG, szInternalSkillNames[skl],0);
		if (l >= kNumSkillLevels)
			l = kNumSkillLevels - 1;
		for (int lvl = 0; lvl <= l; lvl++)
		{
			m_nMPPool += m_nCosts[skl][lvl];
		}
		m_MPSkillLevel[skl] = (eSkillLevel)l;

	}

	m_nIntelBonus = m_buteMgr.GetInt("Bonus", "Intel", 0);


    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkillsButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CSkillsButeMgr::Term()
{
    g_pSkillsButeMgr = LTNULL;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkillsButeMgr::GetRank()
//
//	PURPOSE:	Calculate a rank based on a score
//
// ----------------------------------------------------------------------- //
const RANK* CSkillsButeMgr::GetRank(uint32 nScore)
{
	int n = m_nNumRanks-1;
	while (n > 0 && m_Ranks[n].nMinScore > nScore)
		n--;

	if (n < 0)
		return LTNULL;
	return &m_Ranks[n];
}

const SkillModifiers* CSkillsButeMgr::GetModifiers(eSkill skl, eSkillLevel lvl)
{
	switch (skl)
	{
		case SKL_STEALTH:
			return &m_Stealth[lvl];
		case SKL_STAMINA:
			return &m_Stamina[lvl];
		case SKL_AIM:
			return &m_Aim[lvl];
		case SKL_CARRY:
			return &m_Carry[lvl];
		case SKL_ARMOR:
			return &m_Armor[lvl];
		case SKL_WEAPON:
			return &m_Weapon[lvl];
		case SKL_GADGET:
			return &m_Gadget[lvl];
		case SKL_SEARCH:
			return &m_Search[lvl];
	}

	ASSERT(!"Invalid Skill specified.");
	return NULL;
}

float CSkillsButeMgr::GetModifier(eSkill skl, eSkillLevel lvl, uint8 nMod)
{
	uint8 nMax = GetNumModifiers(skl);

	if (nMod > nMax)
	{
		ASSERT(!"Invalid modifier specified.");
		return 1.0f;
	}

	const SkillModifiers* pMods = GetModifiers(skl, lvl);

	return pMods->fModifier[nMod];
}

uint8 CSkillsButeMgr::GetNumModifiers(eSkill skl)
{
	switch (skl)
	{
		case SKL_STEALTH:
			return StealthModifiers::kNumModifiers;
		case SKL_STAMINA:
			return StaminaModifiers::kNumModifiers;
		case SKL_AIM:
			return AimModifiers::kNumModifiers;
		case SKL_CARRY:
			return CarryModifiers::kNumModifiers;
		case SKL_ARMOR:
			return ArmorModifiers::kNumModifiers;
		case SKL_WEAPON:
			return WeaponModifiers::kNumModifiers;
		case SKL_GADGET:
			return GadgetModifiers::kNumModifiers;
		case SKL_SEARCH:
			return SearchModifiers::kNumModifiers;
	}

	ASSERT(!"Invalid Skill specified.");
	return 0;
}



//*********************************************************************************
//*********************************************************************************
// Project:		Aliens vs. Predator 2
// Purpose:		Helper functions for characters
//*********************************************************************************
// File:		CharacterFuncs.h
// Created:		Oct. 13, 2000
// Updated:		Oct. 13, 2000
// Author:		Andy Mattingly
//*********************************************************************************
//*********************************************************************************

#ifndef __CHARACTER_FUNCS_H__
#define __CHARACTER_FUNCS_H__

//*********************************************************************************

#include "CharacterButeMgr.h"

#ifndef _CLIENTBUILD
	#include "Character.h"
#endif

//*********************************************************************************



static LTBOOL IsHuman(const t_CharacterButes &butes)
{
	return butes.m_eCharacterClass == MARINE  || butes.m_eCharacterClass == CORPORATE;
}

//-------------------------------------------------------------------------------//

static LTBOOL IsHuman(const t_CharacterButes *butes)
{
	if(!butes) return LTFALSE;
	return IsHuman(*butes);
}

//-------------------------------------------------------------------------------//

static LTBOOL IsHuman(uint32 nButeSet)
{
	return IsHuman(g_pCharacterButeMgr->GetCharacterButes(nButeSet));
}

//-------------------------------------------------------------------------------//

#ifndef _CLIENTBUILD

static LTBOOL IsHuman(HOBJECT hObj)
{
	if(!hObj) return LTFALSE;

	CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hObj));

	if(pChar)
		return IsHuman(pChar->GetButes());

	return LTFALSE;
}

#endif

//*********************************************************************************



static LTBOOL IsSynthetic(const t_CharacterButes &butes)
{
	return butes.m_eCharacterClass == SYNTHETIC;
}

//-------------------------------------------------------------------------------//

static LTBOOL IsSynthetic(uint32 nButeSet)
{
	return IsSynthetic(g_pCharacterButeMgr->GetCharacterButes(nButeSet));
}


//-------------------------------------------------------------------------------//

static LTBOOL IsSynthetic(const t_CharacterButes *butes)
{
	if(!butes) return LTFALSE;
	return IsSynthetic(*butes);
}


//-------------------------------------------------------------------------------//

#ifndef _CLIENTBUILD

static LTBOOL IsSynthetic(HOBJECT hObj)
{
	if(!hObj) return LTFALSE;

	CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hObj));

	if(pChar)
		return IsSynthetic(pChar->GetButes());

	return LTFALSE;
}

#endif

//*********************************************************************************


static LTBOOL IsPredator(const t_CharacterButes &butes)
{
	return butes.m_eCharacterClass == PREDATOR;
}

//-------------------------------------------------------------------------------//

static LTBOOL IsPredator(uint32 nButeSet)
{
	return IsPredator(g_pCharacterButeMgr->GetCharacterButes(nButeSet));
}

//-------------------------------------------------------------------------------//

static LTBOOL IsPredator(const t_CharacterButes *butes)
{
	if(!butes) return LTFALSE;
	return IsPredator(*butes);
}

//-------------------------------------------------------------------------------//

#ifndef _CLIENTBUILD

static LTBOOL IsPredator(HOBJECT hObj)
{
	if(!hObj) return LTFALSE;

	CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hObj));

	if(pChar)
		return IsPredator(pChar->GetButes());

	return LTFALSE;
}

#endif

//*********************************************************************************


static LTBOOL IsAlien(const t_CharacterButes &butes)
{
	return butes.m_eCharacterClass == ALIEN;
}

//-------------------------------------------------------------------------------//

static LTBOOL IsAlien(uint32 nButeSet)
{
	return IsAlien(g_pCharacterButeMgr->GetCharacterButes(nButeSet));
}

//-------------------------------------------------------------------------------//

static LTBOOL IsAlien(const t_CharacterButes *butes)
{
	if(!butes) return LTFALSE;
	return IsAlien(*butes);
}

//-------------------------------------------------------------------------------//

#ifndef _CLIENTBUILD

static LTBOOL IsAlien(HOBJECT hObj)
{
	if(!hObj) return LTFALSE;

	CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hObj));

	if(pChar)
		return IsAlien(pChar->GetButes());

	return LTFALSE;
}

#endif

//*********************************************************************************

static LTBOOL IsExosuit(const t_CharacterButes & butes)
{
	return   butes.m_eCharacterClass == MARINE_EXOSUIT
		  || butes.m_eCharacterClass == CORPORATE_EXOSUIT;
}

//-------------------------------------------------------------------------------//

static LTBOOL IsExosuit(uint32 nButeSet)
{
	return IsExosuit(g_pCharacterButeMgr->GetCharacterButes(nButeSet));
}

//-------------------------------------------------------------------------------//

static LTBOOL IsExosuit(const t_CharacterButes *butes)
{
	if(!butes) return LTFALSE;
	return IsExosuit(*butes);
}


//-------------------------------------------------------------------------------//

#ifndef _CLIENTBUILD

static LTBOOL IsExosuit(HOBJECT hObj)
{
	if(!hObj) return LTFALSE;

	CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hObj));

	if(pChar)
		return IsExosuit(pChar->GetButes());

	return LTFALSE;
}

#endif

//*********************************************************************************

#endif //_CHARACTER_FUNCS_H_


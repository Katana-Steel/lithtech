// ----------------------------------------------------------------------- //
//
// MODULE  : AIButeMgr.cpp
//
// PURPOSE : AIButeMgr implementation - Controls attributes of all AIButes
//
// CREATED : 12/02/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIButeMgr.h"
#include "CommonUtilities.h"

#include <algorithm>

// Globals/statics

CAIButeMgr* g_pAIButeMgr = LTNULL;
CAIWeaponButeMgr* g_pAIWeaponButeMgr = LTNULL;

extern char * CHARACTER_BUTE_WALK_SPEED;
extern char * CHARACTER_BUTE_RUN_SPEED;

namespace /*unnamed*/
{
	const char * szBidName[NUM_BIDS] =
	{
		"Script",
		"NodeCheck",
		"Alarm",
		"Snipe",
		"Lurk",
		"AttackSeeEnemy",
		"Retreat",
		"Investigate",
		"Cower",
		"AttackOther",
		"IdleScript",
		"Patrol",
		"Idle"
	};

	const char * szDifficultyName[DIFFICULTY_PC_INSANE+1] =
	{
		"Easy",
		"Medium",
		"Hard",
		"Insane"
	};

} //namespace /*unnamed*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //
static bool AddTag(const char * szTagName, void * pContainer)
{
	CAIButeMgr::TagList * pTagList = reinterpret_cast<CAIButeMgr::TagList*>(pContainer);

	if( strstr(szTagName,"AIAttrib") )
		pTagList->push_back(szTagName);

	return true;
}

LTBOOL CAIButeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	if (g_pAIButeMgr || !szAttributeFile) return LTFALSE;
	
	if (!Parse(pInterface, szAttributeFile)) 
	{
#ifndef _FINAL
		if( g_pLTServer ) g_pLTServer->CPrint( const_cast<char*>((const char*)m_buteMgr.GetErrorString()) );
#endif
		return LTFALSE;
	}

	// Set up global pointer

	g_pAIButeMgr = this;


	// Read the attribute template tags.

	m_buteMgr.GetTags(AddTag,&m_TagNames);

	if( !m_TagNames.empty() )
	{
		m_aTemplates.assign(m_TagNames.size(), AIBM_Template( ));
		m_aSenses.assign(m_TagNames.size(), AIBM_AllSenses( ));
		m_aTargeting.assign(m_TagNames.size(),AIBM_Targeting( ));
		m_aAlienAttack.assign(m_TagNames.size(), AIBM_AlienAttack( ));
		m_aPatrol.assign(m_TagNames.size(), AIBM_Patrol( ));
		m_aInvestigate.assign(m_TagNames.size(), AIBM_Investigate( ));
		m_aCower.assign(m_TagNames.size(), AIBM_Cower( ));
		m_aMovement.assign(m_TagNames.size(), AIBM_Movement( ));
		m_aApproachTarget.assign(m_TagNames.size(), AIBM_ApproachTarget( ));
		m_Attack.assign(m_TagNames.size(), AIBM_Attack( ));
		m_aInfluence.assign(m_TagNames.size(), AIBM_Influence( ));

		for ( uint32 iTemplate = 0 ; iTemplate < m_TagNames.size() ; iTemplate++ )
		{
			SetTemplate( iTemplate, m_TagNames[iTemplate] );
			SetSenses( iTemplate, m_TagNames[iTemplate] );
			SetTargeting( iTemplate, m_TagNames[iTemplate] );
			SetAlienAttack( iTemplate, m_TagNames[iTemplate] );
			SetPatrol( iTemplate, m_TagNames[iTemplate] );
			SetInvestigate( iTemplate, m_TagNames[iTemplate] );
			SetCower( iTemplate, m_TagNames[iTemplate] );
			SetMovement( iTemplate, m_TagNames[iTemplate] );
			SetApproachTarget( iTemplate, m_TagNames[iTemplate] );
			SetAttack( iTemplate, m_TagNames[iTemplate] );
			SetInfluence( iTemplate, m_TagNames[iTemplate] );
		}
	}

	{for (int i = 0; i < NUM_BIDS; ++i)
	{
		m_aDefaultBids[i] = (LTFLOAT)GetDouble("DefaultBids", szBidName[i], 0.0f);
	}}

	{for (int i = 0; i < DIFFICULTY_PC_INSANE+1; ++i)
	{
		m_aDifficultyFactor[i] = (LTFLOAT)fabs(GetDouble("DifficultyFactor", szDifficultyName[i], 1.0f));
	}}

	m_buteMgr.Term();

	// Set up the AI weapons butes

	m_AIWeaponButeMgr.Init(g_pLTServer, szAttributeFile);
	g_pAIWeaponButeMgr = &m_AIWeaponButeMgr;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::GetTemplate*()
//
//	PURPOSE:	Gets an attribute template property or information
//
// ----------------------------------------------------------------------- //

int CAIButeMgr::GetTemplateIDByName(const std::string & strName) const
{
	for ( uint32 iTemplateID = 0 ; iTemplateID < m_aTemplates.size(); iTemplateID++ )
	{
		if ( 0 == stricmp(strName.c_str(),m_aTemplates[iTemplateID].strName.c_str()) )
		{
			return iTemplateID;
		}
	}

	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::Set*
//
//	PURPOSE:	Sets an attribute
//
// ----------------------------------------------------------------------- //
namespace 
{
	class TemplateNameMatches
	{
		const char * const nameToMatch;
		
	public:

		TemplateNameMatches(const char * name_to_match)
			: nameToMatch(name_to_match) {}

		bool operator()(const AIBM_Template & x)
		{
			return 0 == stricmp(x.strName.c_str(), nameToMatch);
		}
	};
}


void CAIButeMgr::SetTemplate(uint32 iTemplate, const std::string & szTagName)
{

	CString cstrTemplateName = GetString(szTagName.c_str(), "Name");
	if(    !cstrTemplateName.IsEmpty()
		&& m_aTemplates.end() != std::find_if(m_aTemplates.begin(),m_aTemplates.end(),TemplateNameMatches(cstrTemplateName)) )
	{
#ifndef _FINAL
		if( g_pLTServer ) 
			g_pLTServer->CPrint( "Duplicate named template \"%s\" found.  Duplicate name will be ignored.",
								  cstrTemplateName.GetBuffer(0) );
#endif
	}
	else
	{
		m_aTemplates[iTemplate].strName = cstrTemplateName;
	}

	CString cstrAlignment = GetString(szTagName.c_str(), "Class");
	if ( 0 == stricmp(cstrAlignment,"MARINE") )		
	{
		m_aTemplates[iTemplate].eCharacterClass = MARINE;
	}
	else if ( 0 == stricmp(cstrAlignment,"CORPORATE") )		
	{
		m_aTemplates[iTemplate].eCharacterClass = CORPORATE;
	}
	else if ( 0 == stricmp(cstrAlignment,"CORPORATE_EXOSUIT") )		
	{
		m_aTemplates[iTemplate].eCharacterClass = CORPORATE_EXOSUIT;
	}
	else if ( 0 == stricmp(cstrAlignment,"MARINE_EXOSUIT") )		
	{
		m_aTemplates[iTemplate].eCharacterClass = MARINE_EXOSUIT;
	}
	else if ( 0 == stricmp(cstrAlignment,"SYNTHETIC") )		
	{
		m_aTemplates[iTemplate].eCharacterClass = SYNTHETIC;
	}
	else if ( 0 == stricmp(cstrAlignment,"PREDATOR") )
	{
		m_aTemplates[iTemplate].eCharacterClass = PREDATOR;
	}
	else if ( 0 == stricmp(cstrAlignment,"ALIEN") )
	{
		m_aTemplates[iTemplate].eCharacterClass = ALIEN;
	}
	else
	{
		m_aTemplates[iTemplate].eCharacterClass = UNKNOWN;
	}

	m_aTemplates[iTemplate].strWeapon[0] = GetString(szTagName.c_str(), "Weapon");
	m_aTemplates[iTemplate].strWeapon[1] = GetString(szTagName.c_str(), "SecondaryWeapon");
	m_aTemplates[iTemplate].strWeapon[2] = GetString(szTagName.c_str(), "TertiaryWeapon");

	m_aTemplates[iTemplate].strWeaponPosition = GetString(szTagName.c_str(), "WeaponPosition");
	m_aTemplates[iTemplate].fAccuracy = (LTFLOAT)GetDouble(szTagName.c_str(), "Accuracy");
	m_aTemplates[iTemplate].fLurkRange = (LTFLOAT)GetDouble(szTagName.c_str(), "LurkRange");

	// These two are also read in CAIButeMgr::SetSeeEnemySense(..)
	m_aTemplates[iTemplate].fForwardFOV = MATH_DEGREES_TO_RADIANS((LTFLOAT)GetDouble(szTagName.c_str(), "FOVAngle"));
	m_aTemplates[iTemplate].fUpNonFOV = MATH_DEGREES_TO_RADIANS((LTFLOAT)GetDouble(szTagName.c_str(), "VerticalBlindAngle"));

	m_aTemplates[iTemplate].fSwitchWeaponDelay = (LTFLOAT)GetDouble(szTagName.c_str(), "SwitchWeaponDelay");
	m_aTemplates[iTemplate].fSwitchWeaponChance = (LTFLOAT)GetDouble(szTagName.c_str(), "SwitchWeaponProbability");

	m_aTemplates[iTemplate].fSnipeTime = (LTFLOAT)GetDouble(szTagName.c_str(), "SnipeTime");
	m_aTemplates[iTemplate].fLoseTargetTime = (LTFLOAT)GetDouble(szTagName.c_str(), "LoseTargetTime");
	m_aTemplates[iTemplate].nHideFromDEdit = GetInt(szTagName.c_str(), "HideFromDEdit",0);
	m_aTemplates[iTemplate].fDamagePlayerFactor = (LTFLOAT)GetDouble(szTagName.c_str(), "DamagePlayerFactor",1.0f);
	m_aTemplates[iTemplate].bUseDumbAttack = GetInt(szTagName.c_str(), "UseDumbAttack",0);
	m_aTemplates[iTemplate].fInjuredSpeed = (LTFLOAT)GetDouble(szTagName.c_str(), "InjuredSpeed",0);
}

void CAIButeMgr::SetTargeting(uint32 iTemplate, const std::string & szTagName)
{
	m_aTargeting[iTemplate].fFullSighting = (LTFLOAT)GetDouble(szTagName.c_str(),"TargetFullSighting", 1.0f);
	m_aTargeting[iTemplate].fMaxFiredCheck = (LTFLOAT)GetDouble(szTagName.c_str(),"TargetMaxFiredCheck", 1.0f);
	m_aTargeting[iTemplate].fMinAttackingDist = (LTFLOAT)GetDouble(szTagName.c_str(),"TargetMinAttackingDist", 1.0f);
	m_aTargeting[iTemplate].fTargetDelay = (LTFLOAT)GetDouble(szTagName.c_str(), "TargetDelay", 3.0f);
	m_aTargeting[iTemplate].fMaintainDuration = (LTFLOAT)GetDouble(szTagName.c_str(), "TargetMaintainDuration", 5.0f);
	m_aTargeting[iTemplate].fMaintainChance = (LTFLOAT)GetDouble(szTagName.c_str(), "TargetMaintainProbability", 0.25f);
	m_aTargeting[iTemplate].fMaintainCloakedDuration = (LTFLOAT)GetDouble(szTagName.c_str(), "TargetMaintainCloakedDuration", 5.0f);
	m_aTargeting[iTemplate].fMaintainCloakedChance = (LTFLOAT)GetDouble(szTagName.c_str(), "TargetMaintainCloakedProbability", 0.25f);
	m_aTargeting[iTemplate].fStillAttackingDuration = (LTFLOAT)GetDouble(szTagName.c_str(), "TargetStillAttackingDuration", 1.0f);
	m_aTargeting[iTemplate].fIncreaseLagTime = (LTFLOAT)GetDouble(szTagName.c_str(), "TargetIncreaseLagTime", 0.0f);
	m_aTargeting[iTemplate].fDecreaseLagTime = (LTFLOAT)GetDouble(szTagName.c_str(), "TargetDecreaseLagTime", 0.0f);
	m_aTargeting[iTemplate].fIdleLagTime = (LTFLOAT)GetDouble(szTagName.c_str(), "TargetIdleLagTime", 0.0f);
	m_aTargeting[iTemplate].fMaxLag = (LTFLOAT)GetDouble(szTagName.c_str(), "TargetMaxLag", 0.0f);
	m_aTargeting[iTemplate].fMinLag = (LTFLOAT)GetDouble(szTagName.c_str(), "TargetMinLag", 0.0f);
}

void CAIButeMgr::SetAttack(uint32 iTemplate, const std::string & szTagName)
{
	CARange temp;

	m_Attack[iTemplate].fDodgeProbability    = (LTFLOAT)GetDouble(szTagName.c_str(),"DodgeProbability", 0.75f);

	temp = GetRange(szTagName.c_str(),"DodgeDelay" );
	m_Attack[iTemplate].fMinDodgeDelay = (LTFLOAT)temp.GetMin();
	m_Attack[iTemplate].fMaxDodgeDelay = (LTFLOAT)temp.GetMax();

	temp = GetRange(szTagName.c_str(),"DodgeDistance" );
	m_Attack[iTemplate].fMinDodgeDist = (LTFLOAT)temp.GetMin();
	m_Attack[iTemplate].fMaxDodgeDist = (LTFLOAT)temp.GetMax();

	
	m_Attack[iTemplate].fDodgeImpactDist = (LTFLOAT)GetDouble(szTagName.c_str(), "DodgeImpactDist", 250.0f);


	m_Attack[iTemplate].fBackoffProbability    = (LTFLOAT)GetDouble(szTagName.c_str(),"BackoffProbability");

	temp = GetRange(szTagName.c_str(),"BackoffDelay" );
	m_Attack[iTemplate].fMinBackoffDelay = (LTFLOAT)temp.GetMin();
	m_Attack[iTemplate].fMaxBackoffDelay = (LTFLOAT)temp.GetMax();

	temp = GetRange(szTagName.c_str(),"BackoffDistance" );
	m_Attack[iTemplate].fMinBackoffDist = (LTFLOAT)temp.GetMin();
	m_Attack[iTemplate].fMaxBackoffDist = (LTFLOAT)temp.GetMax();

	
	m_Attack[iTemplate].fRunForCoverChance =  (LTFLOAT)GetDouble(szTagName.c_str(),"RunForCoverProbability");

	temp = GetRange(szTagName.c_str(),"RunForCoverDelay" );
	m_Attack[iTemplate].fMinRunForCoverDelay = (LTFLOAT)temp.GetMin();
	m_Attack[iTemplate].fMaxRunForCoverDelay = (LTFLOAT)temp.GetMax();

	temp = GetRange(szTagName.c_str(),"CoverDuration" );
	m_Attack[iTemplate].fMinCoverDuration = (LTFLOAT)temp.GetMin();
	m_Attack[iTemplate].fMaxCoverDuration = (LTFLOAT)temp.GetMax();

	temp = GetRange(szTagName.c_str(),"StuckFireDuration" );
	m_Attack[iTemplate].fMinStuckFireDuration = (LTFLOAT)temp.GetMin();
	m_Attack[iTemplate].fMaxStuckFireDuration = (LTFLOAT)temp.GetMax();
	
	m_Attack[iTemplate].fCrouchChance =  (LTFLOAT)GetDouble(szTagName.c_str(),"CrouchProbability");

	temp = GetRange(szTagName.c_str(),"CrouchDelay" );
	m_Attack[iTemplate].fMinCrouchDelay = (LTFLOAT)temp.GetMin();
	m_Attack[iTemplate].fMaxCrouchDelay = (LTFLOAT)temp.GetMax();

	
	m_Attack[iTemplate].strMeleeWeapon = GetString(szTagName.c_str(), "MeleeWeapon", CString(""));

	temp = GetRange(szTagName.c_str(),"EnterExitMeleeDistance" );
	m_Attack[iTemplate].fEnterMeleeDist = (LTFLOAT)temp.GetMin();
	m_Attack[iTemplate].fExitMeleeDist = (LTFLOAT)temp.GetMax();
}

void CAIButeMgr::SetAlienAttack(uint32 iTemplate, const std::string & szTagName)
{
	CARange temp;

	m_aAlienAttack[iTemplate].fPounceChance  = (LTFLOAT)GetDouble(szTagName.c_str(),"AlienPounceProbability", 0.0f);

	temp = GetRange(szTagName.c_str(),"AlienPounceDelay", CARange(0.0f, 0.0f) );
	m_aAlienAttack[iTemplate].fMinPounceDelay = (LTFLOAT)temp.GetMin();
	m_aAlienAttack[iTemplate].fMaxPounceDelay = (LTFLOAT)temp.GetMax();

	temp = GetRange(szTagName.c_str(),"AlienPounceRange", CARange(0.0f, 0.0f) );
	m_aAlienAttack[iTemplate].fPounceMinRangeSqr   = LTFLOAT(temp.GetMin())*LTFLOAT(temp.GetMin());
	m_aAlienAttack[iTemplate].fPounceMaxRangeSqr   = LTFLOAT(temp.GetMax())*LTFLOAT(temp.GetMax());



	m_aAlienAttack[iTemplate].fFacehugChance  = (LTFLOAT)GetDouble(szTagName.c_str(),"AlienFacehugProbability", 0.0f);

	temp = GetRange(szTagName.c_str(),"AlienFacehugDelay", CARange(0.0f, 0.0f) );
	m_aAlienAttack[iTemplate].fMinFacehugDelay = (LTFLOAT)temp.GetMin();
	m_aAlienAttack[iTemplate].fMaxFacehugDelay = (LTFLOAT)temp.GetMax();

	temp = GetRange(szTagName.c_str(),"AlienFacehugRange", CARange(0.0f, 0.0f) );
	m_aAlienAttack[iTemplate].fFacehugMinRangeSqr   = LTFLOAT(temp.GetMin())*LTFLOAT(temp.GetMin());
	m_aAlienAttack[iTemplate].fFacehugMaxRangeSqr   = LTFLOAT(temp.GetMax())*LTFLOAT(temp.GetMax());
}

void CAIButeMgr::SetPatrol(uint32 iTemplate, const std::string & szTagName)
{
	CARange temp;

	temp = GetRange(szTagName.c_str(),"PatrolStuckDelay", CARange(0.75f, 1.5f) );
	m_aPatrol[iTemplate].fMinStuckDelay = (LTFLOAT)temp.GetMin();
	m_aPatrol[iTemplate].fMaxStuckDelay = (LTFLOAT)temp.GetMax();
}

void CAIButeMgr::SetInvestigate(uint32 iTemplate, const std::string & szTagName)
{
	CARange temp;

	temp = GetRange(szTagName.c_str(),"InvestigateAnimDelay", CARange(0.75f, 1.5f) );
	m_aInvestigate[iTemplate].fMinAnimDelay = (LTFLOAT)temp.GetMin();
	m_aInvestigate[iTemplate].fMaxAnimDelay = (LTFLOAT)temp.GetMax();

	temp = GetRange(szTagName.c_str(),"InvestigateSuspiciousDur", CARange(2.0f, 4.0f) );
	m_aInvestigate[iTemplate].fMinSuspiciousDur = (LTFLOAT)temp.GetMin();
	m_aInvestigate[iTemplate].fMaxSuspiciousDur = (LTFLOAT)temp.GetMax();

	temp = GetRange(szTagName.c_str(),"InvestigateSearchDur", CARange(3.0f, 5.0f) );
	m_aInvestigate[iTemplate].fMinSearchDur = (LTFLOAT)temp.GetMin();
	m_aInvestigate[iTemplate].fMaxSearchDur = (LTFLOAT)temp.GetMax();

	temp = GetRange(szTagName.c_str(),"InvestigateDelay", CARange(3.0f, 5.0f) );
	m_aInvestigate[iTemplate].fMinHoldOffDelay = (LTFLOAT)temp.GetMin();
	m_aInvestigate[iTemplate].fMaxHoldOffDelay = (LTFLOAT)temp.GetMax();
}

void CAIButeMgr::SetCower(uint32 iTemplate, const std::string & szTagName)
{
	CARange temp;

	temp = GetRange(szTagName.c_str(),"CowerDuration", CARange(2.0f, 5.0f) );
	m_aCower[iTemplate].fMinCowerDuration = (LTFLOAT)temp.GetMin();
	m_aCower[iTemplate].fMaxCowerDuration = (LTFLOAT)temp.GetMax();

	m_aCower[iTemplate].fRunDistSqr = (LTFLOAT)GetDouble(szTagName.c_str(), "CowerRunDistance", 100.0f);
	m_aCower[iTemplate].fRunDistSqr *= m_aCower[iTemplate].fRunDistSqr;

	m_aCower[iTemplate].fRunChance = (LTFLOAT)GetDouble(szTagName.c_str(), "CowerRunChance", 1.0f);

	temp = GetRange(szTagName.c_str(),"CowerSafeDuration", CARange(2.0f, 5.0f) );
	m_aCower[iTemplate].fMinSafeDuration = (LTFLOAT)temp.GetMin();
	m_aCower[iTemplate].fMaxSafeDuration = (LTFLOAT)temp.GetMax();

	m_aCower[iTemplate].fSafeDistSqr = (LTFLOAT)GetDouble(szTagName.c_str(), "CowerSafeDistance", 500.0f);
	m_aCower[iTemplate].fSafeDistSqr *= m_aCower[iTemplate].fSafeDistSqr;

	m_aCower[iTemplate].fNotifyDistSqr = (LTFLOAT)GetDouble(szTagName.c_str(), "CowerNotifyDistance", 500.0f);
	m_aCower[iTemplate].fNotifyDistSqr *= m_aCower[iTemplate].fNotifyDistSqr;

	m_aCower[iTemplate].fNotifyDelay = (LTFLOAT)GetDouble(szTagName.c_str(), "CowerNotifyDelay", 1.0f);

	// Consistency checks.
	if( m_aCower[iTemplate].fSafeDistSqr < m_aCower[iTemplate].fRunDistSqr )
	{
		m_aCower[iTemplate].fSafeDistSqr = m_aCower[iTemplate].fRunDistSqr*1.1f;
	}
}

void CAIButeMgr::SetMovement(uint32 iTemplate, const std::string & szTagName)
{
	CARange temp;

	// MovementCorrectionTime is not mentions in AIButes becuase it is not
	// really used in StrategyMove.  It is here in case it becomes necessary.
	temp = GetRange(szTagName.c_str(),"MovementCorrectionTime", CARange(0.0f, 0.0f) );
	m_aMovement[iTemplate].fMinCorrectionTime = (LTFLOAT)temp.GetMin();
	m_aMovement[iTemplate].fMaxCorrectionTime = (LTFLOAT)temp.GetMax();

	m_aMovement[iTemplate].fCosMaxSnapTurnAngle = (LTFLOAT)GetDouble(szTagName.c_str(),"MovementMaxSnapTurnAngle", 45.0f);
	m_aMovement[iTemplate].fCosMaxSnapTurnAngle = (LTFLOAT)cos( MATH_DEGREES_TO_RADIANS(m_aMovement[iTemplate].fCosMaxSnapTurnAngle) );

	m_aMovement[iTemplate].fMaxStuckDuration = (LTFLOAT)GetDouble(szTagName.c_str(),"MovementMaxWaitForOtherAI", 0.5f);
	m_aMovement[iTemplate].fAvoidancePathFrequency = (LTFLOAT)GetDouble(szTagName.c_str(),"MovementAvoidancePathFrequency", 0.0f);
	
}

void CAIButeMgr::SetApproachTarget(uint32 iTemplate, const std::string & szTagName)
{
	CARange temp;
	temp = GetRange(szTagName.c_str(),"ApproachTargetRepathTime", CARange(0.0f, 0.0f) );
	m_aApproachTarget[iTemplate].fMinRepath = (LTFLOAT)temp.GetMin();
	m_aApproachTarget[iTemplate].fMaxRepath = (LTFLOAT)temp.GetMax();

	temp = GetRange(szTagName.c_str(), "ApproachTargetFireTime", CARange(0.0f, 0.0f));
	m_aApproachTarget[iTemplate].fMinFireTime = (LTFLOAT)temp.GetMin();
	m_aApproachTarget[iTemplate].fMaxFireTime = (LTFLOAT)temp.GetMax();

	m_aApproachTarget[iTemplate].fFireChance = (LTFLOAT)GetDouble(szTagName.c_str(), "ApproachTargetFireChance", 0.0f);
}

void CAIButeMgr::SetInfluence(uint32 iTemplate, const std::string & szTagName)
{
	m_aInfluence[iTemplate].fUpdateTime = (LTFLOAT)GetDouble(szTagName.c_str(),"InfluenceUpdateTime", 2.0f);
	m_aInfluence[iTemplate].fCharacterRange = (LTFLOAT)GetDouble(szTagName.c_str(),"InfluenceCharacterRange", 1000.0f);
	m_aInfluence[iTemplate].nOddsLimit = GetInt(szTagName.c_str(),"InfluenceOddsLimit", 5000);
	m_aInfluence[iTemplate].fSelfDamage = (LTFLOAT)GetDouble(szTagName.c_str(),"InfluenceSelfDamage", 0.25f);
	m_aInfluence[iTemplate].fFriendDamage = (LTFLOAT)GetDouble(szTagName.c_str(),"InfluenceFriendDamage", 0.0f);
	m_aInfluence[iTemplate].fFearDecay = (LTFLOAT)GetDouble(szTagName.c_str(), "InfluenceFearDecay", 0.05f);

	CARange temp = GetRange(szTagName.c_str(),"InfluenceFearDuration");
	m_aInfluence[iTemplate].fMinDamageFearDuration = (LTFLOAT)temp.GetMin();
	m_aInfluence[iTemplate].fMaxDamageFearDuration = (LTFLOAT)temp.GetMax();

}

void CAIButeMgr::SetSenses(uint32 iTemplate, const std::string & szTagName)
{
	SetSeeEnemySense(iTemplate, szTagName);
	SetSeeDeadBodySense(iTemplate, szTagName);
	SetSeeBloodSense(iTemplate, szTagName);
	SetSeeCloakedSense(iTemplate, szTagName);
	SetHearEnemyFootstepSense(iTemplate, szTagName);
	SetHearEnemyWeaponFireSense(iTemplate, szTagName);
	SetHearAllyWeaponFireSense(iTemplate, szTagName);
	SetHearTurretWeaponFireSense(iTemplate, szTagName);
	SetHearAllyPainSense(iTemplate, szTagName);
	SetHearAllyBattleCrySense(iTemplate, szTagName);
	SetHearEnemyTauntSense(iTemplate, szTagName);
	SetHearDeathSense(iTemplate, szTagName);
	SetHearWeaponImpactSense(iTemplate, szTagName);
	SetSeeEnemyFlashlightSense(iTemplate, szTagName);
}


void CAIButeMgr::SetSeeEnemySense(uint32 iTemplate, const std::string & szTagName)
{
	m_aSenses[iTemplate].m_SeeEnemy.fRange = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeEnemyRange");
	m_aSenses[iTemplate].m_SeeEnemy.fIncreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeEnemyShortIncrStimulationTime", 1.0f);
	m_aSenses[iTemplate].m_SeeEnemy.fDecreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeEnemyDecreaseStimulationTime", 1.0f);
	m_aSenses[iTemplate].m_SeeEnemy.fPartialLevel = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeEnemyPartialLevel", 0.5f);
	m_aSenses[iTemplate].m_SeeEnemy.fAlertRateModifier = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeEnemyAlertModifier", 1.0f);

	m_aSenses[iTemplate].m_SeeEnemy.fShortTimeDistance = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeEnemyShortTimeDistance");
	m_aSenses[iTemplate].m_SeeEnemy.fLongTimeDistance = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeEnemyLongTimeDistance");
	m_aSenses[iTemplate].m_SeeEnemy.fLongIncreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeEnemyLongIncrStimulationTime");
	m_aSenses[iTemplate].m_SeeEnemy.fLightMin = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeEnemyLightMin");
	m_aSenses[iTemplate].m_SeeEnemy.fLightMax = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeEnemyLightMax");

	m_aSenses[iTemplate].m_SeeEnemy.fCloakSensitivity =    (LTFLOAT)GetDouble(szTagName.c_str(), "CloakSensitivity");

	CARange temp = GetRange(szTagName.c_str(),"CloakFalloffDistance");

	m_aSenses[iTemplate].m_SeeEnemy.fCloakFalloffMinDist = (LTFLOAT)temp.GetMin();
	m_aSenses[iTemplate].m_SeeEnemy.fCloakFalloffMaxDist = (LTFLOAT)temp.GetMax();

	temp = GetRange(szTagName.c_str(),"CloakAlphaCap");
	m_aSenses[iTemplate].m_SeeEnemy.fCloakMinAlphaAffect = (LTFLOAT)temp.GetMin();
	m_aSenses[iTemplate].m_SeeEnemy.fCloakMaxAlphaAffect = (LTFLOAT)temp.GetMax();

	// These two are also read in CAIButeMgr::SetTemplate(..)
	m_aSenses[iTemplate].m_SeeEnemy.fFOV = MATH_DEGREES_TO_RADIANS((LTFLOAT)GetDouble(szTagName.c_str(), "FOVAngle"));
	m_aSenses[iTemplate].m_SeeEnemy.fVerticalBlind = MATH_DEGREES_TO_RADIANS((LTFLOAT)GetDouble(szTagName.c_str(), "VerticalBlindAngle"));

	m_aSenses[iTemplate].m_SeeEnemy.fStartFOV = MATH_DEGREES_TO_RADIANS((LTFLOAT)GetDouble(szTagName.c_str(), "StartFOVAngle", m_aSenses[iTemplate].m_SeeEnemy.fFOV*0.75f));
	m_aSenses[iTemplate].m_SeeEnemy.fStartVerticalBlind = MATH_DEGREES_TO_RADIANS((LTFLOAT)GetDouble(szTagName.c_str(), "StartVerticalBlindAngle", m_aSenses[iTemplate].m_SeeEnemy.fVerticalBlind*1.25f));
}

void CAIButeMgr::SetSeeDeadBodySense(uint32 iTemplate, const std::string & szTagName)
{
	m_aSenses[iTemplate].m_SeeDeadBody.fRange = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeDeadBodyRange");
	m_aSenses[iTemplate].m_SeeDeadBody.fIncreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeDeadBodyShortIncrStimulationTime", 1.0f);
	m_aSenses[iTemplate].m_SeeDeadBody.fDecreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeDeadBodyDecreaseStimulationTime", 1.0f);
	m_aSenses[iTemplate].m_SeeDeadBody.fPartialLevel = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeDeadBodyPartialLevel", 0.5f);
	m_aSenses[iTemplate].m_SeeDeadBody.fAlertRateModifier = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeDeadBodyAlertModifier", 1.0f);
}

void CAIButeMgr::SetSeeBloodSense(uint32 iTemplate, const std::string & szTagName)
{
	m_aSenses[iTemplate].m_SeeBlood.fRange = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeBloodRange");
	m_aSenses[iTemplate].m_SeeBlood.fIncreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeBloodShortIncrStimulationTime", 1.0f);
	m_aSenses[iTemplate].m_SeeBlood.fDecreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeBloodDecreaseStimulationTime", 1.0f);
	m_aSenses[iTemplate].m_SeeBlood.fPartialLevel = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeBloodPartialLevel", 0.5f);
	m_aSenses[iTemplate].m_SeeBlood.fAlertRateModifier = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeBloodAlertModifier", 1.0f);
}

void CAIButeMgr::SetSeeCloakedSense(uint32 iTemplate, const std::string & szTagName)
{
	m_aSenses[iTemplate].m_SeeCloaked.fRange = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeCloakedRange");
	m_aSenses[iTemplate].m_SeeCloaked.fIncreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeCloakedShortIncrStimulationTime", 1.0f);
	m_aSenses[iTemplate].m_SeeCloaked.fDecreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeCloakedDecreaseStimulationTime", 1.0f);
	m_aSenses[iTemplate].m_SeeCloaked.fAlertRateModifier = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeCloakedAlertModifier", 1.0f);
}

void CAIButeMgr::SetHearEnemyFootstepSense(uint32 iTemplate, const std::string & szTagName)
{
	m_aSenses[iTemplate].m_HearEnemyFootstep.fRange = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyFootstepRange");
	m_aSenses[iTemplate].m_HearEnemyFootstep.fIncreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyFootstepIncreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearEnemyFootstep.fDecreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyFootstepDecreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearEnemyFootstep.fPartialLevel = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyFootstepPartialLevel",0.5f);
	m_aSenses[iTemplate].m_HearEnemyFootstep.fAlertRateModifier = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyFootstepAlertModifier", 1.0f);

	m_aSenses[iTemplate].m_HearEnemyFootstep.fMaxListeningTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyFootstepMaxListeningTime",0.5f);
	m_aSenses[iTemplate].m_HearEnemyFootstep.fMaxNoiseDistance = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyFootstepMaxNoiseDistance",200.0f);

}


void CAIButeMgr::SetHearEnemyWeaponFireSense(uint32 iTemplate, const std::string & szTagName)
{
	m_aSenses[iTemplate].m_HearEnemyWeaponFire.fRange = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyWeaponFireRange");
	m_aSenses[iTemplate].m_HearEnemyWeaponFire.fIncreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyWeaponFireIncreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearEnemyWeaponFire.fDecreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyWeaponFireDecreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearEnemyWeaponFire.fPartialLevel = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyWeaponFirePartialLevel",0.5f);
	m_aSenses[iTemplate].m_HearEnemyWeaponFire.fAlertRateModifier = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyWeaponFireAlertModifier", 1.0f);

	m_aSenses[iTemplate].m_HearEnemyWeaponFire.fMaxListeningTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyWeaponFireMaxListeningTime",0.5f);
	m_aSenses[iTemplate].m_HearEnemyWeaponFire.fMaxNoiseDistance = 1.0f; // This just needs to be set to some positive value.
}

void CAIButeMgr::SetHearAllyWeaponFireSense(uint32 iTemplate, const std::string & szTagName)
{
	m_aSenses[iTemplate].m_HearAllyWeaponFire.fRange = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyWeaponFireRange");
	m_aSenses[iTemplate].m_HearAllyWeaponFire.fIncreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyWeaponFireIncreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearAllyWeaponFire.fDecreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyWeaponFireDecreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearAllyWeaponFire.fPartialLevel = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyWeaponFirePartialLevel",0.5f);
	m_aSenses[iTemplate].m_HearAllyWeaponFire.fAlertRateModifier = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyWeaponFireAlertModifier", 1.0f);

	m_aSenses[iTemplate].m_HearAllyWeaponFire.fMaxListeningTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyWeaponFireMaxListeningTime",0.5f);
	m_aSenses[iTemplate].m_HearAllyWeaponFire.fMaxNoiseDistance = 1.0f; // This just needs to be set to some positive value.
}

void CAIButeMgr::SetHearTurretWeaponFireSense(uint32 iTemplate, const std::string & szTagName)
{
	m_aSenses[iTemplate].m_HearTurretWeaponFire.fRange = (LTFLOAT)GetDouble(szTagName.c_str(),"HearTurretWeaponFireRange");
	m_aSenses[iTemplate].m_HearTurretWeaponFire.fIncreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearTurretWeaponFireIncreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearTurretWeaponFire.fDecreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearTurretWeaponFireDecreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearTurretWeaponFire.fPartialLevel = (LTFLOAT)GetDouble(szTagName.c_str(),"HearTurretWeaponFirePartialLevel",0.5f);
	m_aSenses[iTemplate].m_HearTurretWeaponFire.fAlertRateModifier = (LTFLOAT)GetDouble(szTagName.c_str(),"HearTurretWeaponFireAlertModifier", 1.0f);

	m_aSenses[iTemplate].m_HearTurretWeaponFire.fMaxListeningTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearTurretWeaponFireMaxListeningTime",0.5f);
	m_aSenses[iTemplate].m_HearTurretWeaponFire.fMaxNoiseDistance = 1.0f; // This just needs to be set to some positive value.
}

void CAIButeMgr::SetHearAllyPainSense(uint32 iTemplate, const std::string & szTagName)
{
	m_aSenses[iTemplate].m_HearAllyPain.fRange = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyPainRange");
	m_aSenses[iTemplate].m_HearAllyPain.fIncreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyPainIncreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearAllyPain.fDecreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyPainDecreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearAllyPain.fPartialLevel = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyPainPartialLevel",0.5f);
	m_aSenses[iTemplate].m_HearAllyPain.fAlertRateModifier = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyPainAlertModifier", 1.0f);

	m_aSenses[iTemplate].m_HearAllyPain.fMaxListeningTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyPainMaxListeningTime",0.5f);
	m_aSenses[iTemplate].m_HearAllyPain.fMaxNoiseDistance = 1.0f; // This just needs to be set to some positive value.
}

void CAIButeMgr::SetHearAllyBattleCrySense(uint32 iTemplate, const std::string & szTagName)
{
	m_aSenses[iTemplate].m_HearAllyBattleCry.fRange = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyBattleCryRange");
	m_aSenses[iTemplate].m_HearAllyBattleCry.fIncreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyBattleCryIncreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearAllyBattleCry.fDecreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyBattleCryDecreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearAllyBattleCry.fPartialLevel = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyBattleCryPartialLevel",0.5f);
	m_aSenses[iTemplate].m_HearAllyBattleCry.fAlertRateModifier = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyBattleCryAlertModifier", 1.0f);

	m_aSenses[iTemplate].m_HearAllyBattleCry.fMaxListeningTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearAllyBattleCryMaxListeningTime",0.5f);
	m_aSenses[iTemplate].m_HearAllyBattleCry.fMaxNoiseDistance = 1.0f; // This just needs to be set to some positive value.
}

void CAIButeMgr::SetHearEnemyTauntSense(uint32 iTemplate, const std::string & szTagName)
{
	m_aSenses[iTemplate].m_HearEnemyTaunt.fRange = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyTauntRange");
	m_aSenses[iTemplate].m_HearEnemyTaunt.fIncreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyTauntIncreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearEnemyTaunt.fDecreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyTauntDecreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearEnemyTaunt.fPartialLevel = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyTauntPartialLevel",0.5f);
	m_aSenses[iTemplate].m_HearEnemyTaunt.fAlertRateModifier = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyTauntAlertModifier", 1.0f);

	m_aSenses[iTemplate].m_HearEnemyTaunt.fMaxListeningTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearEnemyTauntMaxListeningTime",0.5f);
	m_aSenses[iTemplate].m_HearEnemyTaunt.fMaxNoiseDistance = 1.0f; // This just needs to be set to some positive value.
}

void CAIButeMgr::SetHearDeathSense(uint32 iTemplate, const std::string & szTagName)
{
	m_aSenses[iTemplate].m_HearDeath.fRange = (LTFLOAT)GetDouble(szTagName.c_str(),"HearDeathRange");
	m_aSenses[iTemplate].m_HearDeath.fIncreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearDeathIncreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearDeath.fDecreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearDeathDecreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearDeath.fPartialLevel = (LTFLOAT)GetDouble(szTagName.c_str(),"HearDeathPartialLevel",0.5f);
	m_aSenses[iTemplate].m_HearDeath.fAlertRateModifier = (LTFLOAT)GetDouble(szTagName.c_str(),"HearDeathAlertModifier", 1.0f);

	m_aSenses[iTemplate].m_HearDeath.fMaxListeningTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearDeathMaxListeningTime",0.5f);
	m_aSenses[iTemplate].m_HearDeath.fMaxNoiseDistance = 1.0f; // This just needs to be set to some positive value.
}

void CAIButeMgr::SetHearWeaponImpactSense(uint32 iTemplate, const std::string & szTagName)
{
	m_aSenses[iTemplate].m_HearWeaponImpact.fRange = (LTFLOAT)GetDouble(szTagName.c_str(),"HearWeaponImpactRange");
	m_aSenses[iTemplate].m_HearWeaponImpact.fIncreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearWeaponImpactIncreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearWeaponImpact.fDecreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearWeaponImpactDecreaseStimulationTime",1.0f);
	m_aSenses[iTemplate].m_HearWeaponImpact.fPartialLevel = (LTFLOAT)GetDouble(szTagName.c_str(),"HearWeaponImpactPartialLevel",0.5f);
	m_aSenses[iTemplate].m_HearWeaponImpact.fAlertRateModifier = (LTFLOAT)GetDouble(szTagName.c_str(),"HearWeaponImpactAlertModifier", 1.0f);

	m_aSenses[iTemplate].m_HearWeaponImpact.fMaxListeningTime = (LTFLOAT)GetDouble(szTagName.c_str(),"HearWeaponImpactMaxListeningTime",0.5f);
	m_aSenses[iTemplate].m_HearWeaponImpact.fMaxNoiseDistance = 1.0f; // This just needs to be set to some positive value.
}

void CAIButeMgr::SetSeeEnemyFlashlightSense(uint32 iTemplate, const std::string & szTagName)
{
	m_aSenses[iTemplate].m_SeeEnemyFlashlight.fRange = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeEnemyFlashlightRange");
	m_aSenses[iTemplate].m_SeeEnemyFlashlight.fIncreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeEnemyFlashlightShortIncrStimulationTime", 1.0f);
	m_aSenses[iTemplate].m_SeeEnemyFlashlight.fDecreaseStimulationTime = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeEnemyFlashlightDecreaseStimulationTime", 1.0f);
	m_aSenses[iTemplate].m_SeeEnemyFlashlight.fPartialLevel = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeEnemyFlashlightPartialLevel", 0.5f);
	m_aSenses[iTemplate].m_SeeEnemyFlashlight.fAlertRateModifier = (LTFLOAT)GetDouble(szTagName.c_str(),"SeeEnemyFlashlightAlertModifier", 1.0f);
}

//////////////////////////////////////////////////////////////////
// CAIWeaponButeMgr
////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //
static bool WeaponAddTag(const char * szTagName, void * pContainer)
{
	CAIButeMgr::TagList * pTagList = reinterpret_cast<CAIButeMgr::TagList*>(pContainer);

	if( strstr(szTagName,"AIWeapon") )
		pTagList->push_back(szTagName);

	return true;
}

CAIWeaponButeMgr::~CAIWeaponButeMgr()
{
	g_pAIWeaponButeMgr = LTNULL;
}


LTBOOL CAIWeaponButeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	if ( !szAttributeFile) return LTFALSE;
	
	if (!Parse(pInterface, szAttributeFile)) return LTFALSE;

	// Read the attribute template tags.

	m_buteMgr.GetTags(WeaponAddTag,&m_TagNames);

	if( !m_TagNames.empty() )
	{
		m_aWeapons.assign(m_TagNames.size(), AIWBM_AIWeapon( ));

		for ( uint32 iTemplate = 0 ; iTemplate < m_TagNames.size() ; iTemplate++ )
		{
			SetAIWeapon( iTemplate, m_TagNames[iTemplate] );
		}
	}


	m_buteMgr.Term();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponButeMgr::GetTemplate*()
//
//	PURPOSE:	Gets an attribute template property or information
//
// ----------------------------------------------------------------------- //

int CAIWeaponButeMgr::GetIDByName(const std::string & strName) const
{
	for ( uint32 iTemplateID = 0 ; iTemplateID < m_TagNames.size(); iTemplateID++ )
	{
		if ( 0 == stricmp(strName.c_str(), m_aWeapons[iTemplateID].Name.c_str()) )
		{
			return iTemplateID;
		}
	}

	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::Set*
//
//	PURPOSE:	Sets an attribute
//
// ----------------------------------------------------------------------- //

void CAIWeaponButeMgr::SetAIWeapon(uint32 iTemplate, const std::string & TagName)
{
	m_aWeapons[iTemplate].Name = GetString(TagName.c_str(),"Name");

	m_aWeapons[iTemplate].WeaponName = GetString(TagName.c_str(),"WeaponName");
	m_aWeapons[iTemplate].BarrelName = GetString(TagName.c_str(),"BarrelName");

	m_aWeapons[iTemplate].AnimReference = GetString(TagName.c_str(),"WeaponAnimation");

	m_aWeapons[iTemplate].strPiece = GetString(TagName.c_str(),"NeedsPiece", "");

	m_aWeapons[iTemplate].bCanUseInjured = LTBOOL( GetInt(TagName.c_str(), "CanUseInjured", 1) );

	CString cstrRace = GetString(TagName.c_str(),"Race", "Human");

	if( 0 == stricmp(cstrRace,"Alien") )
	{
		m_aWeapons[iTemplate].eRace = AIWBM_AIWeapon::Alien;
	}
	else if( 0 == stricmp(cstrRace,"Predator") )
	{
		m_aWeapons[iTemplate].eRace = AIWBM_AIWeapon::Predator;
	}
	else if( 0 == stricmp(cstrRace,"Exosuit") )
	{
		m_aWeapons[iTemplate].eRace = AIWBM_AIWeapon::Exosuit;
	}
	else
	{
#ifndef _FINAL
		if( 0 != stricmp(cstrRace, "Human") && g_pInterface)
		{
			g_pInterface->CPrint("AIWeapon bute \"%s\" has unknown race of \"%s\", assuming Human.",
				m_aWeapons[iTemplate].Name.c_str(), cstrRace.GetBuffer(0));
		}
#endif
		m_aWeapons[iTemplate].eRace = AIWBM_AIWeapon::Human;
	}

	CARange temp;

	temp = GetRange(TagName.c_str(),"IdealRange");
	m_aWeapons[iTemplate].fMinIdealRange = (LTFLOAT)temp.GetMin();
	m_aWeapons[iTemplate].fMaxIdealRange = (LTFLOAT)temp.GetMax();

	temp = GetRange(TagName.c_str(),"FireRange");
	m_aWeapons[iTemplate].fMinFireRange = (LTFLOAT)temp.GetMin();
	m_aWeapons[iTemplate].fMaxFireRange = (LTFLOAT)temp.GetMax();

	temp = GetRange(TagName.c_str(),"BurstInterval");
	m_aWeapons[iTemplate].fMaxBurstInterval = (LTFLOAT)temp.GetMax();
	m_aWeapons[iTemplate].fMinBurstInterval = (LTFLOAT)temp.GetMin();

	temp = GetRange(TagName.c_str(),"BurstShots");
	m_aWeapons[iTemplate].nMaxBurstShots = (uint32)temp.GetMax();
	m_aWeapons[iTemplate].nMinBurstShots = (uint32)temp.GetMin();

	temp = GetRange(TagName.c_str(),"BurstDelay");
	m_aWeapons[iTemplate].fMaxBurstDelay = (LTFLOAT)temp.GetMax();
	m_aWeapons[iTemplate].fMinBurstDelay = (LTFLOAT)temp.GetMin();

	m_aWeapons[iTemplate].fBurstChance = (LTFLOAT)GetDouble(TagName.c_str(),"BurstProbability", 1.0);
	m_aWeapons[iTemplate].fHardMaxBurstDelay = (int)GetDouble(TagName.c_str(),"BurstMaxDelay", -1);


	m_aWeapons[iTemplate].fBurstSpeed = (LTFLOAT)GetDouble(TagName.c_str(),"BurstSpeed");

	m_aWeapons[iTemplate].fAltWeaponChance = (LTFLOAT)GetDouble(TagName.c_str(),"AltWeaponProbability");
	temp = GetRange(TagName.c_str(), "AltWeaponDelay");
	m_aWeapons[iTemplate].fAltWeaponMaxDelay = (LTFLOAT)temp.GetMax();
	m_aWeapons[iTemplate].fAltWeaponMinDelay = (LTFLOAT)temp.GetMin();
	m_aWeapons[iTemplate].AltWeaponName = GetString(TagName.c_str(),"AltWeaponName");


	m_aWeapons[iTemplate].fAimAtHeadChance = (LTFLOAT)GetDouble(TagName.c_str(),"AimAtHeadProbability",0);
	m_aWeapons[iTemplate].nHideFromDEdit = GetInt(TagName.c_str(),"HideFromDEdit",0);

	m_aWeapons[iTemplate].bWaitForAnimation = GetInt(TagName.c_str(),"WaitForAnimation",LTTRUE);
	m_aWeapons[iTemplate].bMeleeWeapon = GetInt(TagName.c_str(),"MeleeWeapon",LTFALSE);
	m_aWeapons[iTemplate].bUseAltFX = GetInt(TagName.c_str(),"UseAltFX",LTFALSE);

	m_aWeapons[iTemplate].bInfiniteAmmo = GetInt(TagName.c_str(),"InfiniteAmmo",LTTRUE);
	temp = GetRange(TagName.c_str(), "StartingAmmo");
	m_aWeapons[iTemplate].nMaxStartingAmmo = int(temp.GetMax());
	m_aWeapons[iTemplate].nMinStartingAmmo = int(temp.GetMin());

	m_aWeapons[iTemplate].fDamageMultiplier = (LTFLOAT)GetDouble(TagName.c_str(),"DamageMultiplier",1.0f);

	m_aWeapons[iTemplate].nStartFireSound = GetInt(TagName.c_str(),"StartFireSound",WMKS_INVALID);
	m_aWeapons[iTemplate].nLoopFireSound = GetInt(TagName.c_str(),"LoopFireSound",WMKS_INVALID);
	m_aWeapons[iTemplate].nStopFireSound = GetInt(TagName.c_str(),"StopFireSound",WMKS_INVALID);

}



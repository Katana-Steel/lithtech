// ----------------------------------------------------------------------- //
//
// MODULE  : AIEisenberg.cpp
//
// PURPOSE : AIEisenberg object - Implementation
//
// CREATED : 04/09/01
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "AIEisenberg.h"
#include "AIButeMgr.h"
#include "ObjectMsgs.h"
#include "AIAttack.h"
#include "CharacterMgr.h"
#include "AITarget.h"
#include "PlayerObj.h"
#include "SCDefs.h"


BEGIN_CLASS(AIEisenberg)

	ADD_STRINGPROP_FLAG(AttributeTemplate, "Eisenberg", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(CharacterType, "Eisenberg", PF_HIDDEN)
	ADD_COLORPROP_FLAG_HELP(DEditColor, 255.0f, 255.0f, 0.0f, 0, "Color used for object in DEdit.") 

	ADD_STRINGPROP_FLAG_HELP(Weapon, "<default>", PF_HIDDEN, "AI's primary weapon.")
	ADD_STRINGPROP_FLAG_HELP(SecondaryWeapon, "<default>", PF_HIDDEN, "If AI cannot use primary weapon, use this weapon.")
	ADD_STRINGPROP_FLAG_HELP(TertiaryWeapon, "<default>", PF_HIDDEN, "If AI cannot use primary or secondary weapon, use this weapon.")

	ADD_BOOLPROP_FLAG_HELP(UseCover, LTTRUE, PF_HIDDEN, "AI will use cover nodes if available." )
	ADD_BOOLPROP_FLAG_HELP(NeverLoseTarget,	LTTRUE,	PF_HIDDEN, "This should always be true for aliens!" )

	PROP_DEFINEGROUP(Goals, PF_GROUP5 | PF_HIDDEN)
	
END_CLASS_DEFAULT_FLAGS(AIEisenberg, CAI, NULL, NULL, 0)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIEisenberg::AIEisenberg()
//
//	PURPOSE:	Constructor - Initialize
//
// ----------------------------------------------------------------------- //

AIEisenberg::AIEisenberg()
{
	SetCharacter(g_pCharacterButeMgr->GetSetFromModelType("Eisenberg"));
}

AIEisenberg::~AIEisenberg()
{
	HMESSAGEWRITE hMsg;

	// remove health bar
	hMsg = g_pInterface->StartMessage(LTNULL, SCM_SHOW_BOSS_HEALTH);
	g_pInterface->WriteToMessageByte(hMsg, LTFALSE);
	g_pInterface->EndMessage(hMsg);

	// reset it
	hMsg = g_pInterface->StartMessage(LTNULL, SCM_UPDATE_BOSS_HEALTH);
	g_pInterface->WriteToMessageFloat(hMsg, 1.0f);
	g_pInterface->EndMessage(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIEisenberg::PostPropRead()
//
//	PURPOSE:	Initialize model
//
// ----------------------------------------------------------------------- //

void AIEisenberg::PostPropRead(ObjectCreateStruct *pStruct)
{
	CAI::PostPropRead(pStruct);

	if (!g_pLTServer || !pStruct) return;

	// Read our attributes
	if( GetTemplateId() >= 0 )
	{
		const AIBM_Template & butes = g_pAIButeMgr->GetTemplate(GetTemplateId());
	}
}

void AIEisenberg::InitialUpdate()
{
	CAI::InitialUpdate();

	// Really make sure this is set for the aliens.
	// Sorry for the redundant functionality.
	SetNeverLoseTarget(LTTRUE);
}

void AIEisenberg::ProcessDamageMsg(const DamageStruct& damage)
{
	const LTFLOAT fTotal = m_damage.GetMaxHitPoints() + m_damage.GetMaxArmorPoints();
	const LTFLOAT fCurrent = m_damage.GetHitPoints() + m_damage.GetArmorPoints();
	
	LTFLOAT fRatio;
	if (fTotal > 0.0f)
		fRatio = fCurrent / fTotal;
	else
		fRatio = 0.0f;

	// Give the client info to update the health status bar
	HMESSAGEWRITE hMsg = g_pInterface->StartMessage(LTNULL, SCM_UPDATE_BOSS_HEALTH);
	g_pInterface->WriteToMessageFloat(hMsg, fRatio);
	g_pInterface->EndMessage(hMsg);

	CCharacter::ProcessDamageMsg(damage);
}

LTBOOL AIEisenberg::ProcessCommand(const char*const* pTokens, int nArgs)
{
	if (nArgs == 1)
	{
		if (0 == _stricmp("SHOWHEALTH",pTokens[0]))
		{
			HMESSAGEWRITE hMsg = g_pInterface->StartMessage(LTNULL, SCM_SHOW_BOSS_HEALTH);
			g_pInterface->WriteToMessageByte(hMsg, LTTRUE);
			g_pInterface->EndMessage(hMsg);
			return LTTRUE;
		}
		else if (0 == stricmp("HIDEHEALTH",pTokens[0]))
		{
			HMESSAGEWRITE hMsg = g_pInterface->StartMessage(LTNULL, SCM_SHOW_BOSS_HEALTH);
			g_pInterface->WriteToMessageByte(hMsg, LTFALSE);
			g_pInterface->EndMessage(hMsg);
			return LTTRUE;
		}
	}
	
	return CAI::ProcessCommand(pTokens, nArgs);
}

Goal * AIEisenberg::CreateGoal(const char * szGoalName)
{
	ASSERT( szGoalName );

	Goal * pGoal = LTNULL;

	if( stricmp(szGoalName, "ATTACK") == 0 )
	{
		pGoal = new AttackGoal(this, szGoalName);
	}
	else
	{
		pGoal = CAI::CreateGoal(szGoalName);
	}

	return pGoal;
}



void AIEisenberg::UpdateActionAnimation()
{
	CAI::UpdateActionAnimation();
}

void AIEisenberg::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!g_pServerDE || !hWrite) return;
	
	CAI::Save(hWrite,dwSaveFlags);
}


void AIEisenberg::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!g_pServerDE || !hRead) return;

	CAI::Load(hRead,dwLoadFlags);
}

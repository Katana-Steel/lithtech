// ----------------------------------------------------------------------- //
//
// MODULE  : AIDunyaExosuit.cpp
//
// PURPOSE : AIDunyaExosuit object - Implementation
//
// CREATED : 04/09/01
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "AIDunyaExosuit.h"
#include "AIButeMgr.h"
#include "ObjectMsgs.h"
#include "AIAttack.h"
#include "CharacterMgr.h"
#include "AITarget.h"
#include "PlayerObj.h"
#include "ModelButeMgr.h"
#include "SCDefs.h"
#include "SFXMsgIds.h"
#include "MsgIDs.h"


BEGIN_CLASS(AIDunyaExosuit)

	ADD_STRINGPROP_FLAG(AttributeTemplate, "DunyaExosuit", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(CharacterType, "DunyaExosuit", PF_HIDDEN)
	ADD_COLORPROP_FLAG_HELP(DEditColor, 255.0f, 255.0f, 0.0f, 0, "Color used for object in DEdit.") 

	ADD_STRINGPROP_FLAG_HELP(Weapon, "<default>", PF_HIDDEN, "AI's primary weapon.")
	ADD_STRINGPROP_FLAG_HELP(SecondaryWeapon, "<default>", PF_HIDDEN, "If AI cannot use primary weapon, use this weapon.")
	ADD_STRINGPROP_FLAG_HELP(TertiaryWeapon, "<default>", PF_HIDDEN, "If AI cannot use primary or secondary weapon, use this weapon.")

	ADD_BOOLPROP_FLAG_HELP(UseCover, LTTRUE, PF_HIDDEN, "AI will use cover nodes if available." )
	ADD_BOOLPROP_FLAG_HELP(NeverLoseTarget,	LTTRUE,	PF_HIDDEN, "This should always be true for aliens!" )

	PROP_DEFINEGROUP(Goals, PF_GROUP5 | PF_HIDDEN)
	
END_CLASS_DEFAULT_FLAGS(AIDunyaExosuit, CAI, NULL, NULL, 0)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIDunyaExosuit::AIDunyaExosuit()
//
//	PURPOSE:	Constructor - Initialize
//
// ----------------------------------------------------------------------- //

AIDunyaExosuit::AIDunyaExosuit()
	: m_nNumHits(0),
	  m_bMaskOpen(LTFALSE),
	  m_bHealthMeterOn(LTFALSE)
{
}

AIDunyaExosuit::~AIDunyaExosuit()
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
//	ROUTINE:	AIDunyaExosuit::PostPropRead()
//
//	PURPOSE:	Initialize model
//
// ----------------------------------------------------------------------- //

void AIDunyaExosuit::PostPropRead(ObjectCreateStruct *pStruct)
{
	CAI::PostPropRead(pStruct);

	if (!g_pLTServer || !pStruct) return;

	// Read our attributes
	if( GetTemplateId() >= 0 )
	{
		const AIBM_Template & butes = g_pAIButeMgr->GetTemplate(GetTemplateId());
	}
}

void AIDunyaExosuit::InitialUpdate()
{
	AIExosuit::InitialUpdate();

	// This adds the canopy to this particular Exosuit
	AddWindshield();

	SetNeverLoseTarget(LTTRUE);
}

LTBOOL AIDunyaExosuit::IgnoreDamageMsg(const DamageStruct& damage)
{
	// Location-based damage control
	HitLocation hlTemp = g_pModelButeMgr->GetSkeletonNodeHitLocation(GetModelSkeleton(), GetModelNodeLastHit());

	// Phase 1: Dunya is only vulnerable from behind
	if (!m_bMaskOpen)
	{
		if ((hlTemp == HL_TORSO) || (hlTemp == HL_HEAD))
		{
			const LTFLOAT fDot = GetForward().Dot(damage.vDir);

			// only vulnerable in the rear torso section
			if (fDot > 0)
			{
				// register damage
				m_nNumHits++;

				if (m_nNumHits < 8)
				{
					PlaySmokeFX(LTTRUE);
				}

				// Don't allow death quite yet...
				if ((m_damage.GetHitPoints() - damage.fDamage) > 0.0f)
					return LTFALSE;
				else
				{
					m_damage.SetArmorPoints(0.0f);
					m_damage.SetHitPoints(5.0f);	// I'm not quite dead yet!
					ProcessDamageMsg(damage);		// update the health bar
					m_tmrInvulnerable.Start(2.0f);

					return LTTRUE;
				}
			}
		}
	}

	// Phase 2: Dunya removes mask to clear smoke; player must kill with a head shot
	else
	{
		if ((hlTemp == HL_HEAD) && m_tmrInvulnerable.Stopped())
		{
			const LTFLOAT fDot = GetForward().Dot(damage.vDir);

			if (fDot < -0.5f)
			{
				// TODO: handle death
				m_damage.SetHitPoints(0.0f);
				return LTFALSE;
			}
		}
	}

	// ignore all other damage
	return LTTRUE;
}

void AIDunyaExosuit::ProcessDamageMsg(const DamageStruct& damage)
{
	// Health bar update
	// Give the client info to update the health status bar
	HMESSAGEWRITE hMsg = g_pInterface->StartMessage(LTNULL, SCM_UPDATE_BOSS_HEALTH);
	g_pInterface->WriteToMessageFloat(hMsg, GetHealthRatio());
	g_pInterface->EndMessage(hMsg);

	// Phase 2: open mask (new vulnerable area)
	if (GetHealthRatio() < 0.25f)
	{
		LaunchWindshield();
		m_bMaskOpen = LTTRUE;
	}

	return;
}

// Returns the (current armor + health) / (max armor + health)
LTFLOAT AIDunyaExosuit::GetHealthRatio()
{
	const LTFLOAT fTotal = m_damage.GetMaxHitPoints() + m_damage.GetMaxArmorPoints();
	const LTFLOAT fCurrent = m_damage.GetHitPoints() + m_damage.GetArmorPoints();
	
	LTFLOAT fRatio;
	if (fTotal > 0.0f)
		fRatio = fCurrent / fTotal;
	else
		fRatio = 0.0f;

	// always display at least a sliver until death
	if (fRatio < 0.05f)
		fRatio = 0.05f;

	return fRatio;
}

LTBOOL AIDunyaExosuit::ProcessCommand(const char*const* pTokens, int nArgs)
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


Goal * AIDunyaExosuit::CreateGoal(const char * szGoalName)
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



void AIDunyaExosuit::UpdateActionAnimation()
{
	CAI::UpdateActionAnimation();
}


void AIDunyaExosuit::PlaySmokeFX(LTBOOL bOn)
{
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_BOSS_SMOKE_FX);
	g_pLTServer->WriteToMessageByte(hMessage, bOn);
	g_pLTServer->EndMessage(hMessage);
}


void AIDunyaExosuit::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!g_pServerDE || !hWrite) return;
	
	CAI::Save(hWrite,dwSaveFlags);

	*hWrite << m_nNumHits;
	*hWrite << m_bMaskOpen;
	*hWrite << m_bHealthMeterOn;
	*hWrite << m_tmrInvulnerable;
}


void AIDunyaExosuit::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!g_pServerDE || !hRead) return;

	CAI::Load(hRead,dwLoadFlags);

	*hRead >> m_nNumHits;
	*hRead >> m_bMaskOpen;
	*hRead >> m_bHealthMeterOn;
	*hRead >> m_tmrInvulnerable;
}

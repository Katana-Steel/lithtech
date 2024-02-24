// ----------------------------------------------------------------------- //
//
// MODULE  : SimpleAIAlien.cpp
//
// PURPOSE : Low tick count AIAlien object - Implementation
//
// CREATED : 03/06/01
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "SimpleAIAlien.h"
#include "AIButeMgr.h"
#include "ObjectMsgs.h"
#include "AIAlien.h"
#include "CharacterMgr.h"
#include "RCMessage.h"
#include "AIUtils.h"
#include "MsgIDs.h"
#include "ServerSoundMgr.h"
#include "SoundButeMgr.h"
#include "MusicMgr.h"
#include "CharacterFuncs.h"
#include "SimpleNodeMgr.h"

#define TRIGGER_CHECKVERT		"CHECKVERT"

static void BuildPath(SimpleAIAlien::Path * pPath, const CSimpleNode * pEndNode)
{
	const CSimpleNode * pCurrentNode = pEndNode;

	while( pCurrentNode )
	{
		pPath->push_front(pCurrentNode);
		pCurrentNode = pCurrentNode->GetPrevNode();
	}
}

class SimpleAIDrone : public SimpleAIAlien
{
public :

	virtual ~SimpleAIDrone() { }
};

class SimpleAIRunner : public SimpleAIAlien
{
public :

	virtual ~SimpleAIRunner() { }
};

BEGIN_CLASS(SimpleAIAlien)

	ADD_STRINGPROP_FLAG(CharacterType, "Drone", 0)
	ADD_COLORPROP_FLAG_HELP(DEditColor, 255.0f, 255.0f, 0.0f, 0, "Color used for object in DEdit.") 
	ADD_BOOLPROP_FLAG_HELP(RemoveOnDeactivate, LTFALSE, 0, "Removes the AI if it is auto-deactivated.") 

#ifdef _WIN32
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models\\Characters\\Drone.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#else
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models/Characters/Drone.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#endif

END_CLASS_DEFAULT_FLAGS(SimpleAIAlien, CSimpleAI, NULL, NULL, 0)

BEGIN_CLASS(SimpleAIRunner)

	ADD_STRINGPROP_FLAG(CharacterType, "Runner", 0)

#ifdef _WIN32
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models\\Characters\\runner.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#else
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models/Characters/runner.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#endif

END_CLASS_DEFAULT(SimpleAIRunner, SimpleAIAlien, NULL, NULL)

BEGIN_CLASS(SimpleAIDrone)

	ADD_STRINGPROP_FLAG(CharacterType, "Drone", 0)

#ifdef _WIN32
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models\\Characters\\Drone.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#else
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models/Characters/Drone.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#endif

END_CLASS_DEFAULT(SimpleAIDrone, SimpleAIAlien, NULL, NULL)

SimpleAIAlien::SimpleAIAlien()
{
	m_strIdleAnim = "Crouch";
	m_strChaseAnim = "Crawl_F";
	m_strAttackAnim = "Crouch_Claw_Atk1";

	m_bRemoveOnDeactivate = LTFALSE;
	m_bUsingPath = LTFALSE;
	m_pTargetsLastNode = LTNULL;

	m_vLastRecordedPos = LTVector(0,0,0);
	m_bCheckVert = LTTRUE;

	m_pAttackButes = &(g_pAIButeMgr->GetAlienAttack( g_pAIButeMgr->GetTemplateIDByName("SimpleAlien") ) );

	SetupFSM();
}

SimpleAIAlien::~SimpleAIAlien()
{
}

LTBOOL SimpleAIAlien::IgnoreDamageMsg(const DamageStruct& damage)
{
	if (!g_pLTServer) 
		return LTTRUE;

	if( damage.eType == DT_ACID)
	{
		return LTTRUE;
	}

	return CSimpleAI::IgnoreDamageMsg(damage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SimpleAIAlien::HandleInjured
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void SimpleAIAlien::HandleInjured()
{
	CSimpleAI::HandleInjured();

	// Slow them down.
	const AIBM_Template & butes = g_pAIButeMgr->GetTemplate( g_pAIButeMgr->GetTemplateIDByName("SimpleAlien") );

	if( butes.fInjuredSpeed > 0.0f )
	{
		const LTFLOAT fWalkSpeed = m_cmMovement.GetMaxSpeed(CMS_CRAWLING);

		CharacterVars * pVars = m_cmMovement.GetCharacterVars();

		if( fWalkSpeed > 0.0f  )
		{
			LTFLOAT fNewSpeedMod = butes.fInjuredSpeed / fWalkSpeed;
			pVars->m_fSpeedMod *= fNewSpeedMod;
		}
	}

	// Switch to the injured animation
	const char * const szCurrentAction = GetAnimation()->GetLayerReference(ANIMSTYLELAYER_ACTION);
	if( stricmp(szCurrentAction, "CWalkF") == 0 )
	{
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "InjuredCrawl");
	}
	else
	{
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "InjuredIdle");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SimpleAIAlien::HandleStunEffect
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void SimpleAIAlien::HandleStunEffect()
{
	CSimpleAI::HandleStunEffect();

	if (IsStunned() || IsNetted())
	{
		m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() & ~CM_FLAG_WALLWALK);
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_WEAPON, "None");
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Stunned");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SimpleAIAlien::UpdateStunEffect
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void SimpleAIAlien::UpdateStunEffect()
{
	CSimpleAI::UpdateStunEffect();

	if(!IsStunned() && !IsNetted())
	{
		m_RootFSM.SetState(Root::eStateChase);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SimpleAIAlien::HandleEMPEffect
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void SimpleAIAlien::HandleEMPEffect()
{
	CSimpleAI::HandleEMPEffect();

	if( IsStunned() )
	{
		m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() & ~CM_FLAG_WALLWALK);
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_WEAPON, "None");
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Stunned");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SimpleAIAlien::UpdateEMPEffect
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void SimpleAIAlien::UpdateEMPEffect()
{
	CSimpleAI::UpdateEMPEffect();

	if( !IsStunned() )
	{
		m_RootFSM.SetState(Root::eStateChase);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SimpleAIAlien::UpdateActionAnimation()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void SimpleAIAlien::UpdateActionAnimation()
{
	if(    GetMovement()->GetMovementState() == CMS_POUNCING 
	    || GetMovement()->GetMovementState() == CMS_FACEHUG  )
	{
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Pounce" );
		return;
	}

	if(    GetMovement()->GetMovementState() == CMS_FACEHUG_ATTACH )
	{
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Facehug" );
		return;
	}

	// Other wise, don't do nothing!
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SimpleAIAlien::EngineMessageFn()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

uint32 SimpleAIAlien::EngineMessageFn(uint32 dwID, void *pData, LTFLOAT fData)
{
	switch(dwID)
	{
		case MID_DEACTIVATING:
		{
			if( !IsFirstUpdate() && m_bRemoveOnDeactivate )
			{
				// If we have a target and we are de-activating, 
				// we can just remove ourself, cause we're done!
				if( GetTarget() )
				{
					g_pLTServer->RemoveObject(m_hObject);
				}
			}
			break;
		}
	}

	return CSimpleAI::EngineMessageFn(dwID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAIAlien::ProcessCommand()
//
//	PURPOSE:	Handles trigger commands
//
// ----------------------------------------------------------------------- //
LTBOOL SimpleAIAlien::ProcessCommand(const char* const *pTokens, int nArgs)
{
	if (!pTokens || !pTokens[0] || nArgs < 1) return LTFALSE;

	if (stricmp(TRIGGER_CHECKVERT, pTokens[0]) == 0 && nArgs > 0)
	{
		if (nArgs > 1)
		{
			if (stricmp(pTokens[1], "OFF") == 0)
			{
				m_bCheckVert = LTFALSE;
			}
			else if (stricmp(pTokens[1], "ON") == 0)
			{
				m_bCheckVert = LTTRUE;
			}
			else
			{
				m_bCheckVert = LTTRUE;
#ifndef _FINAL
				AICPrint(m_hObject, "Usage: CheckVert [ON | OFF]  -- Assuming ON");
#endif
			}
		}

#ifndef _FINAL
		if (nArgs <= 1)
		{
			AICPrint(m_hObject, "Usage: CheckVert [ON | OFF]");
		}
#endif
		return LTTRUE;
	}

#ifdef _DEBUG
	if (_stricmp("ww", pTokens[0]) == 0 && nArgs > 0)
	{
		if (nArgs > 1)
		{
			if (_stricmp(pTokens[1], "OFF") == 0)
			{
				m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() & ~CM_FLAG_WALLWALK);
			}
			if (_stricmp(pTokens[1], "ON") == 0)
			{
				m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() | CM_FLAG_WALLWALK);
			}
		}
	}
#endif

	return CSimpleAI::ProcessCommand(pTokens, nArgs);
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SimpleAIAlien::ReadProp()
//
//	PURPOSE:	Read DEdit properties
//
// ----------------------------------------------------------------------- //

LTBOOL SimpleAIAlien::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	if (!g_pInterface|| !pData) return LTFALSE;

	CSimpleAI::ReadProp(pData);

	if (g_pLTServer->GetPropGeneric("RemoveOnDeactivate", &genProp) == LT_OK)
	{
		m_bRemoveOnDeactivate = genProp.m_Bool;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//  IsTargetVertical()
//
//	This method determines if a target is in the direction of our
//	up vector.  When this returns true, wallwalking needs to be disabled.
// ----------------------------------------------------------------------- //
LTBOOL SimpleAIAlien::IsTargetVertical()
{
	if (!m_bCheckVert)
		return LTFALSE;

	if (GetTarget())
	{
		LTVector vAI, vTarget, vAIToTarget;
		LTFLOAT fDot;

		g_pInterface->GetObjectPos(m_hObject, &vAI);

		if( !m_bUsingPath || m_Path.empty())
		{
			g_pInterface->GetObjectPos(GetTarget(), &vTarget);
			
			LTVector vTargetDims(0,0,0);
			g_pInterface->GetObjectDims(GetTarget(),&vTargetDims);
			vTarget.y -= vTargetDims.y;
		}
		else
		{
			ASSERT( !m_Path.empty() );

			vTarget = m_Path.front()->GetAttractorPos();
		}

		vAIToTarget = vTarget - vAI;
		vAIToTarget.Norm();
		
		fDot = vAIToTarget.Dot(GetUp());
		if (fDot > 0.985f)		// roughly 10 degrees leeway
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

LTBOOL SimpleAIAlien::IsStuck()
{
	// Position recording and timer is handled in SimpleAIAlien::Update()

	// Can't be stuck if we're not chasing anything
	if (m_RootFSM.GetState() != Root::eStateChase)
	{
		return LTFALSE;
	}

	// Consider ourselves stuck if we're not moving while in the chase state
	LTVector vPos;
	g_pInterface->GetObjectPos(m_hObject, &vPos);

	if(vPos.Equals(m_vLastRecordedPos, 1.0f))
	{
		return LTTRUE;
	}

	// Everything else is ok
	return LTFALSE;
}
	
void SimpleAIAlien::InitialUpdate()
{
	CSimpleAI::InitialUpdate();

	m_RootFSM.SetState(Root::eStateIdle);

	return;
}


void SimpleAIAlien::Update()
{
	CSimpleAI::Update();

	if( m_nRandomSoundBute > 0 && m_tmrRandomSound.Stopped() )
	{
		if( m_RootFSM.GetState() == Root::eStateIdle )
		{
			PlayExclamationSound("RandomIdle");
		}
		else
		{
			PlayExclamationSound("RandomAttack");
		}

		if( IsPlayingVoicedSound() )
			m_tmrRandomSound.Start();
	}

	// This will be set to true in m_RootFSM.Update() if using m_Path.
	m_bUsingPath = LTFALSE;

	// Update behaviours.
	if(!IsStunned() && !IsNetted())
	{
		m_RootFSM.Update();
	}
	else
	{
	}


	if (IsTargetVertical() || IsStuck() || IsNetted())
	{
		m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() & ~CM_FLAG_WALLWALK);
	}
	else
	{
		if (m_cmMovement.IsStandingOn())
			m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() | CM_FLAG_WALLWALK);
	}

	// Record position periodically
	if (m_tmrStuck.Stopped())
	{
		g_pInterface->GetObjectPos(m_hObject, &m_vLastRecordedPos);
		m_tmrStuck.Start(2.0f);
	}
}


// ----------------------------------------------------------------------- //
//   State Machine & related methods
// ----------------------------------------------------------------------- //
void SimpleAIAlien::SetupFSM()
{
	m_RootFSM.DefineState(Root::eStateIdle, 
						  m_RootFSM.MakeCallback(*this, &SimpleAIAlien::UpdateIdle),
						  m_RootFSM.MakeCallback(*this, &SimpleAIAlien::StartIdle),
						  m_RootFSM.MakeCallback(*this, &SimpleAIAlien::EndIdle));

	m_RootFSM.DefineState(Root::eStateChase,
						  m_RootFSM.MakeCallback(*this, &SimpleAIAlien::UpdateChase),
						  m_RootFSM.MakeCallback(*this, &SimpleAIAlien::StartChase));

	m_RootFSM.DefineState(Root::eStateAttack,
						  m_RootFSM.MakeCallback(*this, &SimpleAIAlien::UpdateAttack),
						  m_RootFSM.MakeCallback(*this, &SimpleAIAlien::StartAttack));

	// Default Transition
	m_RootFSM.DefineTransition(Root::eNoTarget, Root::eStateIdle);

	// Standard Transitions
	m_RootFSM.DefineTransition(Root::eStateIdle, Root::eEnemyDetected, Root::eStateChase);
	m_RootFSM.DefineTransition(Root::eStateChase, Root::eInAttackRange, Root::eStateAttack);
	m_RootFSM.DefineTransition(Root::eStateAttack, Root::eOutOfAttackRange, Root::eStateChase);
}

void SimpleAIAlien::StartIdle(RootFSM * fsm)
{
	if (!m_hObject)
		return;

	// WARNING: This will set their dims to crouch dims.  Be sure they
	// always have a crouch animation!
	m_cmMovement.SetControlFlags((m_cmMovement.GetControlFlags() | CM_FLAG_DUCK) & ~(CM_FLAG_FORWARD | CM_FLAG_BACKWARD));

//	HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, "Idle_0");

//	g_pInterface->SetModelAnimation(m_hObject, hAnim);
//	g_pInterface->SetModelLooping(m_hObject, LTTRUE);

	if( !GetInjuredMovement() )
	{
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Crouch");
	}
	else
	{
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "InjuredIdle");
	}
	GetAnimation()->SetLayerReference(ANIMSTYLELAYER_WEAPON, "None");


	// Set-up to play a random idle sound.
	m_nRandomSoundBute = -1;
	CString strSoundButes = g_pServerSoundMgr->GetSoundSetName( g_pCharacterButeMgr->GetGeneralSoundDir(GetCharacter()), "RandomIdle" );
	if( !strSoundButes.IsEmpty() )
	{
		m_nRandomSoundBute = g_pSoundButeMgr->GetSoundSetFromName(strSoundButes);

		if( m_nRandomSoundBute > 0 )
		{
			const SoundButes & butes = g_pSoundButeMgr->GetSoundButes(m_nRandomSoundBute);
			m_tmrRandomSound.Start( butes.m_fMinDelay );
		}
	}
}

void SimpleAIAlien::UpdateIdle(RootFSM * fsm)
{
	CCharacter *pChar;

	// Be sure music is set.
	SetMusicMood( CMusicMgr::eMoodWarning );

	// Check for enemies.
	pChar = IsEnemyInRange(GetDetectionRadiusSqr());
	if (pChar/* && WithinLeash(pChar->GetPosition()) */)
	{
		LTVector vTargetPos;
		g_pInterface->GetObjectPos(pChar->m_hObject, &vTargetPos);

		// Don't "detect" an enemy unless he is inside our leash
		if (WithinLeash(vTargetPos))
		{
			SetTarget(pChar->m_hObject);
		}
	}

	if (GetTarget())
	{
		fsm->AddMessage(Root::eEnemyDetected);
		return;
	}

}

void SimpleAIAlien::EndIdle(RootFSM * fsm)
{
	// Play our initial attack sound.
	PlayExclamationSound("Attack");

	// Set-up to play a random attack sound.
	m_nRandomSoundBute = -1;
	CString strSoundButes = g_pServerSoundMgr->GetSoundSetName( g_pCharacterButeMgr->GetGeneralSoundDir(GetCharacter()), "RandomAttack" );
	if( !strSoundButes.IsEmpty() )
	{
		m_nRandomSoundBute = g_pSoundButeMgr->GetSoundSetFromName(strSoundButes);

		if( m_nRandomSoundBute > 0 )
		{
			const SoundButes & butes = g_pSoundButeMgr->GetSoundButes(m_nRandomSoundBute);
			m_tmrRandomSound.Start( butes.m_fMinDelay );
		}
	}
}

void SimpleAIAlien::StartChase(RootFSM * fsm)
{
	if (!GetTarget())
	{
		fsm->AddMessage(Root::eNoTarget);
		return;
	}

	m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() | CM_FLAG_FORWARD | CM_FLAG_RUN);
//	HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, "Run_F");

//	g_pInterface->SetModelAnimation(m_hObject, hAnim);
//	g_pInterface->SetModelLooping(m_hObject, LTTRUE);
//	m_caAnimation.PlayAnim(m_strChaseAnim.c_str(), LTTRUE);

	if( !GetInjuredMovement() )
	{
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "CWalkF");
	}
	else
	{
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "InjuredCrawl");
	}
	GetAnimation()->SetLayerReference(ANIMSTYLELAYER_WEAPON, "None");
}

void SimpleAIAlien::UpdateChase(RootFSM * fsm)
{
	// Be sure music is set.
	SetMusicMood( CMusicMgr::eMoodHostile );

	CSimpleAI *pSimpleAI = LTNULL;

	if (!GetTarget())
	{
		fsm->AddMessage(Root::eNoTarget);
		return;
	}

	// We ran past another SimpleAI, so give him our target if he doesn't have one.
	pSimpleAI = IsSimpleAIInRange(GetAlertRadiusSqr());
	if (pSimpleAI && !pSimpleAI->GetTarget())
	{
		pSimpleAI->SetTarget(GetTarget());
	}

	// Check for target in range.
	LTVector vTargetPos, vPos;

	g_pInterface->GetObjectPos(GetTarget(), &vTargetPos);
	g_pInterface->GetObjectPos(m_hObject, &vPos);


	if( !WithinLeash(vTargetPos) )
	{
		SetTarget(LTNULL);
		fsm->AddMessage(Root::eNoTarget);
		return;
	}


	LTFLOAT fDistSqr = (vTargetPos - vPos).MagSqr();
	if (fDistSqr < GetMaxMeleeRange()*GetMaxMeleeRange())
	{
		fsm->AddMessage(Root::eInAttackRange);
		return;
	}
	

	if( !GetInjuredMovement() )
	{
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "CWalkF");
	}
	else
	{
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "InjuredCrawl");
	}

	
	// Check for a new path or follow our current path.
	const CCharacter * pCharTarget = GetCharTargetPtr();
	if( pCharTarget )
	{
		// Try to find a path if we haven't recorded the target's last node,
		// or if the target has changed nodes.
		if( !m_pTargetsLastNode || m_pTargetsLastNode != pCharTarget->GetLastSimpleNode() )
		{
			m_Path.clear();

			// Only path if our target has a last node and we have a last node.
			if( pCharTarget->GetLastSimpleNode() && GetLastSimpleNode() )
			{
				// Only path if our last node is not the same as the target's last node.
				if( GetLastSimpleNode() != pCharTarget->GetLastSimpleNode() )
				{
					// Try to find a path.
					if( g_pSimpleNodeMgr->FindPath(vTargetPos, pCharTarget->GetLastSimpleNode(), GetLastSimpleNode() ) )
					{
						// Store the path in m_Path.
						BuildPath(&m_Path, pCharTarget->GetLastSimpleNode());

						m_pTargetsLastNode = pCharTarget->GetLastSimpleNode();
					}
				}
			}
		}

		// If we have a path, follow it!
		if( !m_Path.empty() )
		{
			// If we have reached the current node (or any node after it), be done with it!
			if( GetLastSimpleNode() )
			{
				Path::iterator iter = std::find(m_Path.begin(), m_Path.end(), GetLastSimpleNode());
				if( iter != m_Path.end() )
				{
					// The check to make sure iter is not end will insure
					// that we can add one to iter.
					m_Path.erase(m_Path.begin(), iter+1);
				}
			}

			// If we still have a path, go to the next node in the path.
			if( !m_Path.empty() )
			{
				m_bUsingPath = LTTRUE;
				vTargetPos = m_Path.front()->GetAttractorPos();
			}
		}
		
	}

	// Push the target's position down to their feet.
	if( !m_bUsingPath )
	{
		LTVector vTargetDims(0,0,0);
		g_pInterface->GetObjectDims(GetTarget(),&vTargetDims);
		vTargetPos.y -= vTargetDims.y;
	}

	LTVector vUp;
	LTRotation rRot;

	rRot = m_cmMovement.GetRot();
	vUp = m_cmMovement.GetUp();
	LTVector vTemp = vTargetPos - vPos;
	vTemp -= vUp* vUp.Dot(vTemp);  // remove up component

	g_pMathLT->AlignRotation(rRot, vTemp, const_cast<LTVector&>(vUp));
	m_cmMovement.SetRot(rRot);

	m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() | CM_FLAG_FORWARD | CM_FLAG_RUN);

	return;
}

void SimpleAIAlien::StartAttack(RootFSM * fsm)
{
	if (!GetTarget())
	{
		fsm->AddMessage(Root::eNoTarget);
		return;
	}
	
	// Stop moving
	m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() &  ~(CM_FLAG_FORWARD | CM_FLAG_BACKWARD));
	m_cmMovement.SetAcceleration(LTVector(0,0,0));
	m_cmMovement.SetVelocity(LTVector(0,0,0));

//	HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, "Crouch_Claw_Atk1");

//	g_pInterface->SetModelAnimation(m_hObject, hAnim);
//	g_pInterface->SetModelLooping(m_hObject, LTTRUE);



	if( !GetInjuredMovement() )
	{
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Crouch");
	}
	else
	{
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "InjuredIdle");
	}

	GetAnimation()->SetLayerReference(ANIMSTYLELAYER_WEAPON, "Claws");
}

void SimpleAIAlien::UpdateAttack(RootFSM * fsm)
{
	if (!GetTarget())
	{
		fsm->AddMessage(Root::eNoTarget);
		return;
	}

	// Be sure music is set.
	SetMusicMood( CMusicMgr::eMoodHostile );

	// Be sure target is still in range.
	LTVector vTargetPos, vMyPos;
	LTFLOAT fDistSqr;

	g_pInterface->GetObjectPos(GetTarget(), &vTargetPos);
	g_pInterface->GetObjectPos(m_hObject, &vMyPos);

	fDistSqr = (vTargetPos - vMyPos).MagSqr();
//	const LTFLOAT fAdvanceRange = GetMinMeleeRange() + 0.75f*(GetMaxMeleeRange() - GetMinMeleeRange());
//	const LTFLOAT fRetreatRange = GetMinMeleeRange();

	if (fDistSqr >= GetMaxMeleeRange()*GetMaxMeleeRange())
	{
		fsm->AddMessage(Root::eOutOfAttackRange);
		return;
	}
/*	else if( fDistSqr >= fAdvanceRange*fAdvanceRange )
	{
		m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() & ~CM_FLAG_RUN);
		m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() | CM_FLAG_FORWARD);
	}
	else if( fDistSqr < fRetreatRange*fRetreatRange )
	{
		m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() & ~CM_FLAG_RUN);
		m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() | CM_FLAG_BACKWARD);
	}
	else
	{
		m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() & ~(CM_FLAG_FORWARD | CM_FLAG_BACKWARD) );
	}
*/



	// Keep facing the target.
	LTVector vUp;
	LTRotation rRot;

	rRot = m_cmMovement.GetRot();
	vUp = m_cmMovement.GetUp();
	LTVector vTemp = vTargetPos - vMyPos;
	vTemp -= vUp* vUp.Dot(vTemp);  // remove up component

	// Update direction to face
	g_pMathLT->AlignRotation(rRot, vTemp, const_cast<LTVector&>(vUp));
	m_cmMovement.SetRot(rRot);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::Save()
//
//	PURPOSE:	Save current state
//
// ----------------------------------------------------------------------- //

void SimpleAIAlien::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite)
		return;

	CSimpleAI::Save(hWrite);

	*hWrite << m_RootFSM;

	*hWrite << m_strIdleAnim;
	*hWrite << m_strChaseAnim;
	*hWrite << m_strAttackAnim;

	*hWrite << m_bRemoveOnDeactivate;
	*hWrite << m_vLastRecordedPos;
	*hWrite << m_bCheckVert;

	hWrite->WriteDWord(m_Path.size());
	for(Path::iterator iter = m_Path.begin(); iter != m_Path.end(); ++iter )
	{
		hWrite->WriteDWord( (*iter)->GetID() );
	}

	*hWrite << m_bUsingPath;

	hWrite->WriteDWord( m_pTargetsLastNode ? m_pTargetsLastNode->GetID() : CSimpleNode::kInvalidID );

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::Load()
//
//	PURPOSE:	Load data
//
// ----------------------------------------------------------------------- //

void SimpleAIAlien::Load(HMESSAGEREAD hRead)
{
	if (!hRead)
		return;
	
	CSimpleAI::Load(hRead);

	*hRead >> m_RootFSM;

	*hRead >> m_strIdleAnim;
	*hRead >> m_strChaseAnim;
	*hRead >> m_strAttackAnim;

	*hRead >> m_bRemoveOnDeactivate;
	*hRead >> m_vLastRecordedPos;
	*hRead >> m_bCheckVert;

	m_Path.clear();
	const uint32 nPathSize = hRead->ReadDWord();
	for(uint32 i = 0; i < nPathSize; ++i )
	{
		const uint32 nNodeID = hRead->ReadDWord();
		m_Path.push_back( g_pSimpleNodeMgr->GetNode(nNodeID) );
	}

	*hRead >> m_bUsingPath;

	m_pTargetsLastNode = g_pSimpleNodeMgr->GetNode( hRead->ReadDWord() );
}



// ----------------------------------------------------------------------- //
//
//	SimpleAIFacehugger (derived from SimpleAIAlien)
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(SimpleAIFacehugger)

	ADD_STRINGPROP_FLAG(CharacterType, "Facehugger", PF_HIDDEN)

#ifdef _WIN32
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models\\Characters\\Facehugger.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#else
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models/Characters/Facehugger.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#endif

END_CLASS_DEFAULT(SimpleAIFacehugger, SimpleAIAlien, NULL, NULL)

SimpleAIFacehugger::SimpleAIFacehugger()
{
	m_strIdleAnim = "Idle_0";
	m_strChaseAnim = "Run_F";
	m_strAttackAnim = "";
	
	m_pAttackButes = &(g_pAIButeMgr->GetAlienAttack( g_pAIButeMgr->GetTemplateIDByName("BasicFaceHugger") ) );

	SetupFSM();
	m_RootFSM.SetState(Root::eStateIdle);
}

void SimpleAIFacehugger::Update()
{
	// Don't perform updates if the facehugging sequence has begun
	const CharacterMovementState cms = GetMovement()->GetMovementState();
	if (cms == CMS_FACEHUG || cms == CMS_FACEHUG_ATTACH)
		return;

	SimpleAIAlien::Update();

	if (GetMovement()->GetMovementState() != CMS_FACEHUG_ATTACH 
		&& GetMovement()->GetMovementState() != CMS_POUNCING 
		&& GetMovement()->GetMovementState() != CMS_FACEHUG)
	{
		if( m_RootFSM.GetState() != Root::eStateIdle )
		{
			PlayExclamationSound("RandomAttack");
		}

		m_RootFSM.Update();
	}
}

void SimpleAIFacehugger::StartAttack(RootFSM * fsm)
{
	if (!GetTarget())
	{
		fsm->AddMessage(Root::eNoTarget);
		return;
	}
	
	// Stop moving
	m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() &  ~(CM_FLAG_FORWARD | CM_FLAG_BACKWARD));
	m_cmMovement.SetAcceleration(LTVector(0,0,0));
	m_cmMovement.SetVelocity(LTVector(0,0,0));
//	m_cmMovement.SetControlFlags(CM_FLAG_WALLWALK);

	if( !GetInjuredMovement() )
	{
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Crouch");
	}
	else
	{
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "InjuredIdle");
	}


	m_tmrFacehugDelay.Stop();
}

void SimpleAIFacehugger::UpdateAttack(RootFSM * fsm)
{
	if (!GetTarget())
	{
		fsm->AddMessage(Root::eNoTarget);
		return;
	}

	// Start Facehug attack
	if (m_tmrFacehugDelay.Stopped())
	{
		m_tmrFacehugDelay.Start( GetRandom(m_pAttackButes->fMinFacehugDelay, m_pAttackButes->fMaxFacehugDelay) );

		if (m_pAttackButes->fFacehugChance > GetRandom(0.0f, 1.0f))
		{
			if( GetCharTargetPtr() && GetCharTargetPtr()->GetButes()->m_bCanBeFacehugged )
			{
				RCMessage pMsg;
				if (!pMsg.null())
				{
					pMsg->WriteObject(GetTarget());
					g_pLTServer->SendToObject(*pMsg, MID_FORCE_FACEHUG, m_hObject, m_hObject, 0);
					return;
				}
			}
		}
	}
	else
	{
		if( !GetInjuredMovement() )
		{
			GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Crouch");
		}
		else
		{
			GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "InjuredIdle");
		}
	}

	// Be sure target is still in range.
	LTVector vTargetPos, vMyPos;
	LTFLOAT fDistSqr;

	g_pInterface->GetObjectPos(GetTarget(), &vTargetPos);
	g_pInterface->GetObjectPos(m_hObject, &vMyPos);

	fDistSqr = (vTargetPos - vMyPos).MagSqr();
//	const LTFLOAT fAdvanceRange = GetMinMeleeRange() + 0.75f*(GetMaxMeleeRange() - GetMinMeleeRange());
//	const LTFLOAT fRetreatRange = GetMinMeleeRange();

	if (fDistSqr >= GetMaxMeleeRange()*GetMaxMeleeRange())
	{
		fsm->AddMessage(Root::eOutOfAttackRange);
		return;
	}
/*	else if( fDistSqr >= fAdvanceRange*fAdvanceRange )
	{
		m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() & ~CM_FLAG_RUN);
		m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() | CM_FLAG_FORWARD);
	}
	else if( fDistSqr < fRetreatRange*fRetreatRange )
	{
		m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() & ~CM_FLAG_RUN);
		m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() | CM_FLAG_BACKWARD);
	}
	else
	{
		m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() & ~(CM_FLAG_FORWARD | CM_FLAG_BACKWARD) );
	}
*/
	// Keep facing the target.
	LTVector vUp;
	LTRotation rRot;

	rRot = m_cmMovement.GetRot();
	vUp = m_cmMovement.GetUp();
	LTVector vTemp = vTargetPos - vMyPos;
	vTemp -= vUp* vUp.Dot(vTemp);  // remove up component

	// Update direction to face
	g_pMathLT->AlignRotation(rRot, vTemp, const_cast<LTVector&>(vUp));
	m_cmMovement.SetRot(rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::Save()
//
//	PURPOSE:	Save current state
//
// ----------------------------------------------------------------------- //

void SimpleAIFacehugger::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite)
		return;

	SimpleAIAlien::Save(hWrite);

	*hWrite << m_tmrFacehugDelay;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::Load()
//
//	PURPOSE:	Load data
//
// ----------------------------------------------------------------------- //

void SimpleAIFacehugger::Load(HMESSAGEREAD hRead)
{
	if (!hRead)
		return;
	
	SimpleAIAlien::Load(hRead);

	*hRead >> m_tmrFacehugDelay;
}

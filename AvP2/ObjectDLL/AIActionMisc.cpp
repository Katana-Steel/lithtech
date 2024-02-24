#include "stdafx.h"
#include "AIActionMisc.h"
#include "AI.h"
#include "AIUtils.h"

#include "AIButeMgr.h"
#include "Weapons.h"
#include "AITarget.h"
#include "DebugLineSystem.h"
#include "CharacterMgr.h"
#include "CVarTrack.h"
#include "CharacterAnimationDefs.h"
#include "AIVolume.h"
#include "AINodeMgr.h"
#include "AINodeCover.h"
#include "RCMessage.h"
#include "MsgIDs.h"
#include "SoundTypes.h"
#include "ServerSoundMgr.h"

static void AimAtTarget(CAI * pAI )
{
	if( !pAI->HasTarget() )
		return;

	const CWeapon * pWeapon =  pAI->GetCurrentWeaponPtr();
	if( pWeapon && pAI)
	{
		const LTVector vAimPoint = TargetAimPoint( *pAI, pAI->GetTarget(), *pWeapon);
		const LTVector vAimDir = vAimPoint - pAI->GetAimFromPosition();

		if( pAI->AllowSnapTurn() )
		{
			const LTFLOAT aim_dot_forward = vAimDir.Dot(pAI->GetForward());
			if(    aim_dot_forward < 0.0f 
				|| aim_dot_forward*aim_dot_forward < c_fCos45*c_fCos45*vAimDir.MagSqr() )
			{
				const LTVector & my_dims = pAI->GetDims();
				if( vAimDir.MagSqr() > my_dims.x*my_dims.x + my_dims.z*my_dims.z )
				{
					// Do the turn.
					pAI->SnapTurn( vAimDir );

					// If you want to do an animation to turn, you would set it here.
				}
			}
		}

		pAI->AimAt(vAimPoint);
	}
}

static void FaceTarget(CAI * pAI)
{

	if( pAI->AllowSnapTurn() && pAI->HasTarget() )
	{
		const LTVector vAimDir = pAI->GetTarget().GetPosition() - pAI->GetPosition();
		const LTFLOAT aim_dot_forward = vAimDir.Dot(pAI->GetForward());
		if(    aim_dot_forward < 0.0f 
			|| aim_dot_forward*aim_dot_forward < c_fCos45*c_fCos45*vAimDir.MagSqr() )
		{
			const LTVector & my_dims = pAI->GetDims();
			if( vAimDir.MagSqr() > my_dims.x*my_dims.x + my_dims.z*my_dims.z )
			{
				// Do the turn.
				pAI->SnapTurn( vAimDir );

				// If you want to do an animation to turn, you would set it here.
			}
		}
	}
}

static const char * GetWeaponSound(const BARREL & barrel, DBYTE nType)
{
	switch(nType)
	{
		case WMKS_FIRE:				return barrel.szFireSound;			break;
		case WMKS_ALT_FIRE:			return barrel.szAltFireSound;			break;
		case WMKS_DRY_FIRE:			return barrel.szDryFireSound;			break;

		case WMKS_MISC0:
		case WMKS_MISC1:
		case WMKS_MISC2:			return barrel.szMiscSounds[nType];	break;

		case WMKS_SELECT:
		case WMKS_DESELECT:
		case WMKS_INVALID:
		default:					return LTNULL;
	}
}

// Please insert new Actions in alphabetical order.


// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionAim
//
//	PURPOSE:	The AI will go to a weapon up position.
//
// ----------------------------------------------------------------------- //

void AIActionAim::Start()
{
	m_bDone = LTFALSE;
	m_bCompletedAim = LTFALSE;	
	GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "StandAim");
}


void AIActionAim::Update()
{
	if( !m_bCompletedAim && !GetAIPtr()->GetAnimation()->UsingTransAnim() )
	{
		m_bCompletedAim = LTTRUE;
	}

	const CAITarget & target = GetAIPtr()->GetTarget();

	m_bDone = !target.IsMaintained();

	AimAtTarget(GetAIPtr());
}

void AIActionAim::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	AIAction::Load(hRead);

	*hRead >> m_bCompletedAim;
}


void AIActionAim::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	AIAction::Save(hWrite);

	*hWrite << m_bCompletedAim;
}

// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionBackoff
//
//	PURPOSE:	Turns the AI to face a particular direction.
//
// ----------------------------------------------------------------------- //
void AIActionBackoff::Start()
{
	// Initialize our state variables.
	m_bDone = LTFALSE;
	m_bHasAnim = LTFALSE;

	// Store away our initial position.
	m_vInitialPos = GetAIPtr()->GetPosition();
	m_vFinalPos = m_vNextBackoffDest;

	m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Backoff");

	if( !m_bHasAnim )
		m_bDone = LTTRUE;
}

void AIActionBackoff::Update()
{
	// Wait for the transition animation to complete.
	if( GetAIPtr()->GetAnimation()->UsingTransAnim() )
		return;

	if( !m_bHasAnim )
	{
		m_bDone = LTTRUE;
		return;
	}

	// Be sure we are facing our target!
	FaceTarget(GetAIPtr());

	const LTFLOAT fBackoffAnimLength = (LTFLOAT)GetAIPtr()->GetAnimation()->GetAnimLength(ANIM_TRACKER_MOVEMENT);
	const LTFLOAT fBackoffAnimTime = (LTFLOAT)GetAIPtr()->GetAnimation()->GetAnimTime(ANIM_TRACKER_MOVEMENT);
	const LTFLOAT fAnimPercent = ( fBackoffAnimLength > 0.0f ) ? fBackoffAnimTime / fBackoffAnimLength : 1.0f;
	
	if( fAnimPercent < 1.0f )
	{
		const LTVector vInterpolatePos = m_vInitialPos*(1.0f - fAnimPercent) + m_vFinalPos*fAnimPercent;
		const LTVector & vCurrentPos = GetAIPtr()->GetPosition();

		GetAIPtr()->GetMovement()->SetPos( LTVector(vInterpolatePos.x, vCurrentPos.y, vInterpolatePos.z) );
	}
	else
	{
		const LTVector & vCurrentPos = GetAIPtr()->GetPosition();

		GetAIPtr()->GetMovement()->SetPos( LTVector(m_vFinalPos.x, vCurrentPos.y, m_vFinalPos.z) );
		m_bDone = LTTRUE;
	}

	// Be sure our velocity and acceleration is zero.
	GetAIPtr()->GetMovement()->UpdateAcceleration(0.0f, CM_DIR_FLAG_XZ_PLANE);
	GetAIPtr()->GetMovement()->UpdateVelocity(0.0f, CM_DIR_FLAG_XZ_PLANE);
}

void AIActionBackoff::AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo)
{
	if( nInfo == ANIMMSG_ANIM_BREAK && nTracker == ANIM_TRACKER_MOVEMENT)
	{
		const LTVector & vCurrentPos = GetAIPtr()->GetPosition();

		GetAIPtr()->GetMovement()->SetPos( LTVector(m_vFinalPos.x, vCurrentPos.y, m_vFinalPos.z) );
		m_bDone = LTTRUE;
	}
}

void AIActionBackoff::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	AIAction::Load(hRead);

	*hRead >> m_vNextBackoffDest;
	*hRead >> m_vInitialPos;
	*hRead >> m_vFinalPos;
	*hRead >> m_bHasAnim;
}


void AIActionBackoff::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	AIAction::Save(hWrite);

	*hWrite << m_vNextBackoffDest;
	*hWrite << m_vInitialPos;
	*hWrite << m_vFinalPos;
	*hWrite << m_bHasAnim;
}


// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionCrouch
//
//	PURPOSE:	Has the AI crouch (and possible reload)
//
// ----------------------------------------------------------------------- //
void AIActionCrouch::Start()
{
	// Initialize our state variables.
	m_bDone = LTFALSE;

	LTBOOL bHasAnim = LTFALSE;

	m_pCurrentCoverNode = m_pNextCoverNode;
	m_pNextCoverNode = LTNULL;

	if( !GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Crouch") )
	{
		m_bDone = LTTRUE;
		return;
	}

//	GetAIPtr()->Crouch();
}

void AIActionCrouch::Update()
{
	if( GetAIPtr()->GetAnimation()->UsingTransAnim() )
		return;

	// Stop crouching if we are no longer covered.
	if( m_pCurrentCoverNode && !m_pCurrentCoverNode->IsCoverFromTarget(*GetAIPtr()) )
	{
		m_bDone = LTTRUE;
	}
}

void AIActionCrouch::End()
{
	GetAIPtr()->StandUp();
}

void AIActionCrouch::AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo)
{
	if( nInfo == ANIMMSG_ANIM_BREAK && nTracker == ANIM_TRACKER_MOVEMENT)
	{
		m_bDone = LTTRUE;
	}
}

void AIActionCrouch::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	AIAction::Load(hRead);

	m_pNextCoverNode = dynamic_cast<CAINodeCover*>( g_pAINodeMgr->GetNode(hRead->ReadDWord()) );
	m_pCurrentCoverNode = dynamic_cast<CAINodeCover*>( g_pAINodeMgr->GetNode(hRead->ReadDWord()) );
}


void AIActionCrouch::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	AIAction::Save(hWrite);

	if( m_pNextCoverNode )
		hWrite->WriteDWord( m_pNextCoverNode->GetID() );
	else
		hWrite->WriteDWord( CAINode::kInvalidID );

	if( m_pCurrentCoverNode )
		hWrite->WriteDWord( m_pCurrentCoverNode->GetID() );
	else
		hWrite->WriteDWord( CAINode::kInvalidID );
}

// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionCrouchReload
//
//	PURPOSE:	Has the AI CrouchReload (and possible reload)
//
// ----------------------------------------------------------------------- //
void AIActionCrouchReload::Start()
{
	// Initialize our state variables.
	m_bDone = LTFALSE;

	CWeapon * pWeapon = GetAIPtr()->GetCurrentWeaponPtr();
	if( pWeapon )
	{
		pWeapon->ReloadClip();
	}

	if( !GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "CReload") )
	{
		m_bDone = LTTRUE;
	}

	if( !m_bDone )
	{
		GetAIPtr()->ClearAim();
	}

}

void AIActionCrouchReload::Update()
{
	if( GetAIPtr()->HasTarget() && GetAIPtr()->AllowSnapTurn() )
	{
		GetAIPtr()->SnapTurn( GetAIPtr()->GetTarget().GetPosition() - GetAIPtr()->GetPosition() );
	}
}

void AIActionCrouchReload::AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo)
{
	if( nInfo == ANIMMSG_ANIM_BREAK && nTracker == ANIM_TRACKER_MOVEMENT)
	{
		m_bDone = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionDodge
//
//	PURPOSE:	Turns the AI to face a particular direction.
//
// ----------------------------------------------------------------------- //
void AIActionDodge::Start()
{
	// Initialize our state variables.
	m_bDone = LTFALSE;
	m_bHasAnim = LTFALSE;

	// Store away our initial position.
	m_vInitialPos = GetAIPtr()->GetPosition();
	m_vFinalPos = m_vNextDodgeDest;

	const char * szActionName = m_bDodgeRight ? "DodgeR" : "DodgeL";

	m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, szActionName);

	if( !m_bHasAnim )
		m_bDone = LTTRUE;
}

void AIActionDodge::Update()
{
	// Wait for the transition animation to complete.
	if( GetAIPtr()->GetAnimation()->UsingTransAnim() )
		return;

	if( !m_bHasAnim )
	{
		m_bDone = LTTRUE;
		return;
	}

	// Be sure we are facing our target!
	FaceTarget(GetAIPtr());

	// If m_bHasAnim is false, the AI will interpolate based off their current animation.
	const LTFLOAT fDodgeAnimLength = (LTFLOAT)GetAIPtr()->GetAnimation()->GetAnimLength(ANIM_TRACKER_MOVEMENT);
	const LTFLOAT fDodgeAnimTime = (LTFLOAT)GetAIPtr()->GetAnimation()->GetAnimTime(ANIM_TRACKER_MOVEMENT);
	const LTFLOAT fAnimPercent = ( fDodgeAnimLength > 0.0f ) ? fDodgeAnimTime / fDodgeAnimLength : 1.0f;
	
	if( fAnimPercent < 1.0f )
	{
		const LTVector vInterpolatePos = m_vInitialPos*(1.0f - fAnimPercent) + m_vFinalPos*fAnimPercent;
		const LTVector & vCurrentPos = GetAIPtr()->GetPosition();

		GetAIPtr()->GetMovement()->SetPos( LTVector(vInterpolatePos.x, vCurrentPos.y, vInterpolatePos.z) );
	}
	else
	{
		const LTVector & vCurrentPos = GetAIPtr()->GetPosition();

		GetAIPtr()->GetMovement()->SetPos( LTVector(m_vFinalPos.x, vCurrentPos.y, m_vFinalPos.z) );
		m_bDone = LTTRUE;
	}

	// Be sure our velocity and acceleration is zero.
	GetAIPtr()->GetMovement()->UpdateAcceleration(0.0f, CM_DIR_FLAG_XZ_PLANE);
	GetAIPtr()->GetMovement()->UpdateVelocity(0.0f, CM_DIR_FLAG_XZ_PLANE);
}

void AIActionDodge::AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo)
{
	if( nInfo == ANIMMSG_ANIM_BREAK && nTracker == ANIM_TRACKER_MOVEMENT)
	{
		const LTVector & vCurrentPos = GetAIPtr()->GetPosition();

		GetAIPtr()->GetMovement()->SetPos( LTVector(m_vFinalPos.x, vCurrentPos.y, m_vFinalPos.z) );
		m_bDone = LTTRUE;
	}
}

void AIActionDodge::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	AIAction::Load(hRead);

	*hRead >> m_vNextDodgeDest;
	*hRead >> m_vInitialPos;
	*hRead >> m_vFinalPos;
	*hRead >> m_bDodgeRight;
	*hRead >> m_bHasAnim;
}


void AIActionDodge::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	AIAction::Save(hWrite);

	*hWrite << m_vNextDodgeDest;
	*hWrite << m_vInitialPos;
	*hWrite << m_vFinalPos;
	*hWrite << m_bDodgeRight;
	*hWrite << m_bHasAnim;
}


// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionDrop
//
//	PURPOSE:	Drops (stands up and waits until standing on ground).
//
// ----------------------------------------------------------------------- //

void AIActionDrop::Start()
{
	m_bDone = LTFALSE;
	
	GetAIPtr()->StandUp();
	GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Jump");
}


LTBOOL AIActionDrop::IsDone() const
{
	return GetAIPtr()->GetMovement()->GetMovementState() != CMS_WALLWALKING && GetAIPtr()->GetMovement()->GetCharacterVars()->m_bStandingOn;
}

// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionFacehug
//
//	PURPOSE:	Performs a facehug (very similar to AIActionPounce)
//
// ----------------------------------------------------------------------- //

void AIActionFacehug::Start()
{
	m_bDone = LTFALSE;

	RCMessage pMsg;
	if( !pMsg.null() )
	{
		pMsg->WriteObject(GetAIPtr()->GetTarget().GetObject());
		g_pLTServer->SendToObject(*pMsg,MID_FORCE_FACEHUG, GetAIPtr()->m_hObject, GetAIPtr()->m_hObject,0);
		return;
	}
	
}

void AIActionFacehug::Update()
{
	// If we aren't in a facehugging state, assume we are finished facehugging!
	if(    GetAIPtr()->GetMovement()->GetMovementState() != CMS_FACEHUG_ATTACH
		&& GetAIPtr()->GetMovement()->GetMovementState() != CMS_FACEHUG )
	{
		GetAIPtr()->StandUp();
		m_bDone = LTTRUE;
	}
}


// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionFireMelee
//
//	PURPOSE:	Fires a ranged Melee
//
// ----------------------------------------------------------------------- //


void AIActionFireMelee::Start()
{

	// Clear out variables
	m_bDone = LTFALSE;
	m_nBurstShotsLeft = 0;
	m_fLastShotTime = GetAIPtr()->GetLastFireInfo().fTime;

	ASSERT(GetAIPtr());
	if (!GetAIPtr())
	{
		m_bDone = LTTRUE;
		return;
	}

	if (!GetAIPtr()->GetCurrentWeaponPtr())
	{
		m_bDone = LTTRUE;
		return;
	}
	// If we switched weapons, be sure to reset our shot
	// interval timer.
	if( GetAIPtr()->GetLastFireInfo().nBarrelId != GetAIPtr()->GetCurrentWeaponPtr()->GetBarrelId() )
	{
		 m_fLastShotTime = -1000.0f;
	}


	if (GetAIPtr()->IsNetted())
	{
		m_bDone = LTTRUE;
		return;
	}

	if( !GetAIPtr()->GetCurrentWeaponButesPtr() || !GetAIPtr()->GetCurrentWeaponPtr())
	{
		m_bDone = LTTRUE;
		return;
	}

	const CWeapon & weapon = *GetAIPtr()->GetCurrentWeaponPtr();
	const AIWBM_AIWeapon & weapon_butes = *GetAIPtr()->GetCurrentWeaponButesPtr();

	// Don't use non-melee weapons!
	_ASSERT( weapon_butes.bMeleeWeapon );
	if( !weapon_butes.bMeleeWeapon )
	{
		m_bDone = LTTRUE;
		return;
	}


	// Define these for easier to read code...
	const int nMinBurstShots = weapon_butes.nMinBurstShots;
	const int nMaxBurstShots = weapon_butes.nMaxBurstShots;

	// Set-up for the burst.
	if (nMaxBurstShots > 0)
	{
		m_nBurstShotsLeft = GetRandom(nMinBurstShots, nMaxBurstShots);
	}
	else
	{
		m_nBurstShotsLeft = 1;
	}

	if( weapon_butes.eRace == AIWBM_AIWeapon::Alien  )
	{
		if( GetAIPtr()->GetInjuredMovement() )
			m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "InjuredIdle" );
		else if( (GetAIPtr()->GetMovementFlags() & CM_FLAG_WALLWALK) || GetAIPtr()->GetMustCrawl() )
			m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Crouch" );
		else
			m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Idle" );

		// Start our clawing/tailing/biting animation.
		if( !weapon_butes.AnimReference.empty()  )
			m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_WEAPON, weapon_butes.AnimReference.c_str() );
		else
			m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_WEAPON, weapon.GetWeaponData()->szName );

		if( !m_bHasAnim )
		{
			GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_WEAPON, "None");
		}
	}
	else
	{
		// Non-alien melee weapons.
		if( weapon_butes.bWaitForAnimation )
		{
			m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "StandFire");
		}

		if( !m_bHasAnim )
		{
			GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "StandAim");
		}
	}

	return;
}

void AIActionFireMelee::Update()
{
	const LTVector & vMyPos = GetAIPtr()->GetPosition();
	const LTVector & vMyRight = GetAIPtr()->GetRight();

	const CAITarget & target = GetAIPtr()->GetTarget();
	
	// Be sure we are "aiming" at the target so 
	// that the fire weapon code will hit our target!
	if( target.IsValid() )
	{
		GetAIPtr()->AimAt(target.GetPosition() + target.GetTargetOffset() );
	}

	if( !m_bHasAnim )
	{
		// Handle our own fire rate.
		const AIWBM_AIWeapon * pWeaponButes = GetAIPtr()->GetCurrentWeaponButesPtr();
		const LTFLOAT fShotInterval = pWeaponButes->fBurstSpeed > 0 ? 1.0f / pWeaponButes->fBurstSpeed : 0.5f;

		if( g_pLTServer->GetTime() - m_fLastShotTime  > fShotInterval )
		{
			// Fire our weapon.
			GetAIPtr()->FireWeapon();

			m_fLastShotTime = g_pLTServer->GetTime();

			// Decrement the shots left (even if we didn't really fire),
			--m_nBurstShotsLeft;

		}

		// If we have fired all our shots then we are done!
		if( m_nBurstShotsLeft <= 0 )
		{
			if( pWeaponButes->eRace == AIWBM_AIWeapon::Alien  )
			{
				GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_WEAPON, "None");
			}
			m_bDone = LTTRUE;
			return;
		}
	}

	// Face our target.
	if( target.IsValid() )
	{
		if( !GetAIPtr()->GetCurrentWeaponButesPtr() || !GetAIPtr()->GetCurrentWeaponPtr())
		{
			GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_WEAPON, "None");
			m_bDone = LTTRUE;
			return;
		}

		const LTVector vTargetRange = ( target.GetPosition() - vMyPos );
		const LTFLOAT  fTargetRangeSqr = vTargetRange.MagSqr();

		const CWeapon & weapon = *GetAIPtr()->GetCurrentWeaponPtr();
		const AIWBM_AIWeapon & weapon_butes = *GetAIPtr()->GetCurrentWeaponButesPtr();


		if( fTargetRangeSqr > weapon_butes.fMaxIdealRange*weapon_butes.fMaxIdealRange )
		{
			// Move toward the target.
			if( GetAIPtr()->GetInjuredMovement() )
			{
				m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "InjuredCrawl" );
			}
			else if( (GetAIPtr()->GetMovementFlags() & CM_FLAG_WALLWALK) || GetAIPtr()->GetMustCrawl() )
			{
				m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "CRunF" );
			}
			else
			{
				m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "RunF" );
			}

			GetAIPtr()->Forward();
		}
		else if( fTargetRangeSqr < weapon_butes.fMinIdealRange*weapon_butes.fMinIdealRange )
		{
			// Move away from the target.
			if( GetAIPtr()->GetInjuredMovement() )
			{
				m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "InjuredCrawl" );
			}
			else if( (GetAIPtr()->GetMovementFlags() & CM_FLAG_WALLWALK) || GetAIPtr()->GetMustCrawl() )
			{
				m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "CRunB" );
			}
			else 
			{
				m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "RunB" );
			}

			GetAIPtr()->Reverse();
		}
		else
		{
			if( weapon_butes.eRace == AIWBM_AIWeapon::Alien  )
			{
				if( GetAIPtr()->GetInjuredMovement() )
				{
					m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "InjuredIdle" );
				}
				else if( (GetAIPtr()->GetMovementFlags() & CM_FLAG_WALLWALK) || GetAIPtr()->GetMustCrawl() )
				{
					m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Crouch" );
				}
				else
				{
					m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Idle" );
				}

			}
			else
			{
				if( weapon_butes.bWaitForAnimation )
				{
					m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "StandFire");
				}

				if( !m_bHasAnim )
				{
					GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "StandAim");
				}
			}
		}

		// Move to the side if we are clipping into another character.
		CCharacter * pClipCharacter = WillClip(*GetAIPtr(),GetAIPtr()->GetPosition());
		if( pClipCharacter && pClipCharacter != target.GetCharacterPtr() )
		{
			if( (pClipCharacter->GetPosition() - vMyPos).Dot(vMyRight) > 0.0f )
			{
				GetAIPtr()->StrafeLeft();
			}
			else
			{
				GetAIPtr()->StrafeRight();
			}
		}

		// Be sure we are facing the target.
		if(  GetAIPtr()->AllowSnapTurn() )
			GetAIPtr()->SnapTurn( target.GetPosition() - GetAIPtr()->GetPosition() );

	}

	return;
}

void AIActionFireMelee::End()
{
	const AIWBM_AIWeapon * pWeaponButes = GetAIPtr()->GetCurrentWeaponButesPtr();
	if( !pWeaponButes || pWeaponButes->eRace == AIWBM_AIWeapon::Alien  )
	{
		GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_WEAPON, "None");
	}
}

void AIActionFireMelee::AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo)
{
	if( !GetAIPtr()->GetAnimation()->UsingTransAnim() )
	{
		if( nInfo == ANIMMSG_ANIM_BREAK && nTracker == ANIM_TRACKER_MOVEMENT)
		{
			--m_nBurstShotsLeft;

			if( m_nBurstShotsLeft <= 0 )
			{
				m_bDone = LTTRUE;
			}
		}
	}
}

void AIActionFireMelee::PhysicsMsg(LTVector * pNewOffset)
{
	ASSERT( pNewOffset );

	if( !GetAIPtr()->HasTarget() )
	{
		return;
	}

	// If the point would put us outside a volume, stay put!
	const CAIVolume * pVolume = GetAIPtr()->GetCurrentVolumePtr();

	LTVector vMyPos;
	g_pLTServer->GetObjectPos(GetAIPtr()->m_hObject, &vMyPos);
	
	if( pVolume )
	{
		// We have to be sure to use our flat dims.
		const LTVector vHorizDims(GetAIPtr()->GetDims().x, 0.0f, GetAIPtr()->GetDims().z);

		// The point to test against is our location, shifted to be in the middle of 
		// the volume (this elinates problems with sloped and stair-stepped volumes).
		LTVector vNewPos = vMyPos + *pNewOffset;
		vNewPos.y = pVolume->GetCenter().y;

		if( !pVolume->ContainsWithNeighbor( vNewPos, vHorizDims ) )
		{
			*pNewOffset = LTVector(0,0,0);
			return;
		}
	}

	const CAITarget & target = GetAIPtr()->GetTarget();

	LTVector vTargetPos;
	g_pLTServer->GetObjectPos(target.GetObject(), &vTargetPos);

	const LTVector vCurrentTargetRange = vTargetPos - vMyPos;
	const LTVector vFutureTargetRange = vCurrentTargetRange - *pNewOffset;

	const LTFLOAT  fCurrentTargetRangeSqr = vCurrentTargetRange.MagSqr();
	const LTFLOAT  fFutureTargetRangeSqr = vFutureTargetRange.MagSqr();

	if( !GetAIPtr()->GetCurrentWeaponButesPtr() || !GetAIPtr()->GetCurrentWeaponPtr())
	{
		m_bDone = LTTRUE;
		return;
	}

	const CWeapon & weapon = *GetAIPtr()->GetCurrentWeaponPtr();
	const AIWBM_AIWeapon & weapon_butes = *GetAIPtr()->GetCurrentWeaponButesPtr();

	const LTFLOAT fIdealRange = (weapon_butes.fMinIdealRange + weapon_butes.fMaxIdealRange)*0.5f;
	const LTFLOAT fIdealRangeSqr = fIdealRange*fIdealRange;

	if(    ( fCurrentTargetRangeSqr > fIdealRangeSqr && fFutureTargetRangeSqr < fIdealRangeSqr )
		|| ( fCurrentTargetRangeSqr < fIdealRangeSqr && fFutureTargetRangeSqr > fIdealRangeSqr ) )
	{
		// Just stay put!
		*pNewOffset = LTVector(0,0,0);
	}
	
}

void AIActionFireMelee::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead )
		return;

	AIAction::Load(hRead);   

	*hRead >> m_bHasAnim;
	m_fLastShotTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	m_nBurstShotsLeft = int(hRead->ReadDWord());
}

void AIActionFireMelee::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite )
		return;

	AIAction::Save(hWrite);   

	*hWrite << m_bHasAnim;
	hWrite->WriteFloat(m_fLastShotTime - g_pLTServer->GetTime());
	hWrite->WriteDWord(m_nBurstShotsLeft);
}


// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionFireWeapon
//
//	PURPOSE:	Fires a ranged weapon
//
// ----------------------------------------------------------------------- //

AIActionFireWeapon::~AIActionFireWeapon()
{
	if( m_hWeaponSound && g_pLTServer )
	{
		g_pLTServer->KillSound(m_hWeaponSound);
	}
}

void AIActionFireWeapon::Start()
{
	ASSERT(GetAIPtr());
	if (!GetAIPtr())
	{
		m_bDone = LTTRUE;
		return;
	}

	// unable to fire under these conditions
	if (GetAIPtr()->IsRecoiling() || GetAIPtr()->IsNetted())
	{
		m_bDone = LTTRUE;
		return;
	}

	// Clear out variables
	m_bHasAnim = LTFALSE;
	m_bDone = LTFALSE;
	m_nBurstShotsLeft = 0;
	m_fLastShotTime = GetAIPtr()->GetLastFireInfo().fTime;
	m_bTriedToPlaySounds = LTFALSE;

	if( !GetAIPtr()->GetCurrentWeaponButesPtr() || !GetAIPtr()->GetCurrentWeaponPtr() )
	{
		m_bDone = LTTRUE;
		return;
	}

	// If we switched weapons, be sure to reset our shot
	// interval timer.
	if( GetAIPtr()->GetLastFireInfo().nBarrelId != GetAIPtr()->GetCurrentWeaponPtr()->GetBarrelId() )
	{
		 m_fLastShotTime = -1000.0f;
	}

	const AIWBM_AIWeapon & weapon_butes = *GetAIPtr()->GetCurrentWeaponButesPtr();

	// Don't use alien weapons!
	_ASSERT( weapon_butes.eRace != AIWBM_AIWeapon::Alien );
	if( weapon_butes.eRace == AIWBM_AIWeapon::Alien  )
	{
		m_bDone = LTTRUE;
		return;
	}

	// Define these for easier to read code...
	const int nMinBurstShots = weapon_butes.nMinBurstShots;
	const int nMaxBurstShots = weapon_butes.nMaxBurstShots;

	// Set-up for the burst.
	if (nMaxBurstShots > 0)
	{
		m_nBurstShotsLeft = GetRandom(nMinBurstShots, nMaxBurstShots);
	}
	else
	{
		// every weapon should allow at least 1 shot to be fired
		m_nBurstShotsLeft = 1;
	}

	// Set-up our animation.
	if( weapon_butes.bWaitForAnimation )
	{
		m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "StandFire");
	}

	if( !m_bHasAnim )
	{
		GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "StandAim");
	}

	return;
}

void AIActionFireWeapon::Update()
{
	if( !GetAIPtr()->GetCurrentWeaponButesPtr() || !GetAIPtr()->GetCurrentWeaponButesPtr() )
	{
		m_bDone = LTTRUE;
		return;
	}

	if( GetAIPtr()->GetAnimation()->UsingTransAnim() )
	{
		// Try to aim while doing our transition.
		AimAtTarget(GetAIPtr());
		return;
	}
	else
	{
		if( !m_bTriedToPlaySounds )
		{
			m_bTriedToPlaySounds = LTTRUE;
			const AIWBM_AIWeapon & weapon_butes = *GetAIPtr()->GetCurrentWeaponButesPtr();

			// Play the start fire sound (if there is one).
			if( weapon_butes.nStartFireSound > WMKS_INVALID && GetAIPtr()->GetCurrentWeaponPtr() )
			{
				const BARREL * pBarrel = GetAIPtr()->GetCurrentWeaponPtr()->GetBarrelData();

				const char * szSoundName = GetWeaponSound(*pBarrel, weapon_butes.nStartFireSound);

				if( szSoundName )
				{
					g_pServerSoundMgr->PlaySoundFromObject(GetAIPtr()->m_hObject, 
										const_cast<char*>(szSoundName), pBarrel->fFireSoundRadius, 
										SOUNDPRIORITY_AI_HIGH);
				}
			}
			
			m_hWeaponSound = LTNULL;
			if( weapon_butes.nLoopFireSound > WMKS_INVALID )
			{
				const BARREL * pBarrel = GetAIPtr()->GetCurrentWeaponPtr()->GetBarrelData();

				const char * szSoundName = GetWeaponSound(*pBarrel, weapon_butes.nLoopFireSound);

				if( szSoundName )
				{
					m_hWeaponSound = g_pServerSoundMgr->PlaySoundFromObject(GetAIPtr()->m_hObject, 
										const_cast<char*>(szSoundName), pBarrel->fFireSoundRadius, 
										SOUNDPRIORITY_AI_MEDIUM, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);
				}
			}
		}
	}

	if( !m_bHasAnim )
	{
		// Handle our own fire rate.
		const AIWBM_AIWeapon * pWeaponButes = GetAIPtr()->GetCurrentWeaponButesPtr();
		const LTFLOAT fShotInterval = pWeaponButes->fBurstSpeed > 0 ? 1.0f / pWeaponButes->fBurstSpeed : 0.1f;

		if( g_pLTServer->GetTime() - m_fLastShotTime  > fShotInterval )
		{
			// Fire our weapon.
			GetAIPtr()->FireWeapon();

			m_fLastShotTime = g_pLTServer->GetTime();

			--m_nBurstShotsLeft;
		}

		if (m_nBurstShotsLeft <= 0 || GetAIPtr()->OutOfAmmo() || GetAIPtr()->EmptyClip() )
			m_bDone = LTTRUE;
	}

	// Keep aiming at our target.
	// This needs to be after the above call to CAI::FireWeapon, as that
	// check for CAI::m_bFullyAimed which is only set to true in UpdateAiming,
	// but is set to false in AimAtTarget.  Hence, we want to make sure
	// CAI::AimAt is only called after CAI::FireWeapon or before CAI::UpdateAiming.
	AimAtTarget(GetAIPtr());

	return;
}

void AIActionFireWeapon::End()
{
	GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "StandAim");

	if( m_hWeaponSound )
	{
		g_pLTServer->KillSound(m_hWeaponSound);
		m_hWeaponSound = LTNULL;
	}


	if(const AIWBM_AIWeapon * pWeaponButes = GetAIPtr()->GetCurrentWeaponButesPtr())
	{
		if( pWeaponButes->nStopFireSound > WMKS_INVALID && GetAIPtr()->GetCurrentWeaponPtr() )
		{
			const BARREL * pBarrel = GetAIPtr()->GetCurrentWeaponPtr()->GetBarrelData();

			const char * szSoundName = GetWeaponSound(*pBarrel, pWeaponButes->nStopFireSound);

			if( szSoundName )
			{
				g_pServerSoundMgr->PlaySoundFromObject(GetAIPtr()->m_hObject, 
									const_cast<char*>(szSoundName), pBarrel->fFireSoundRadius, 
									SOUNDPRIORITY_AI_HIGH);
			}
		}
	}

}

void AIActionFireWeapon::AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo)
{
	if(     nInfo == ANIMMSG_ANIM_BREAK 
		&& (nTracker == ANIM_TRACKER_AIMING_UP || nTracker == ANIM_TRACKER_AIMING_DOWN) )
	{
		--m_nBurstShotsLeft;
		if (m_nBurstShotsLeft <= 0 || GetAIPtr()->OutOfAmmo() || GetAIPtr()->EmptyClip() )
		{
			m_bDone = LTTRUE;
		}
	}
}

void AIActionFireWeapon::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead )
		return;

	AIAction::Load(hRead);   

	*hRead >> m_bHasAnim;
	m_fLastShotTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	m_nBurstShotsLeft = int(hRead->ReadDWord());

	m_hWeaponSound = LTNULL;
	m_bTriedToPlaySounds = LTFALSE;
}

void AIActionFireWeapon::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite )
		return;

	AIAction::Save(hWrite);   

	*hWrite << m_bHasAnim;
	hWrite->WriteFloat(m_fLastShotTime - g_pLTServer->GetTime());
	hWrite->WriteDWord(m_nBurstShotsLeft);

	// m_hWeaponSound will not be restarted at load time.
	// m_bTriedToPlaySound will be reset at load time.
}

// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionJumpTo
//
//	PURPOSE:	Jumps to a location
//
// ----------------------------------------------------------------------- //

void AIActionJumpTo::Start()
{
	m_bDone = LTFALSE;

	GetAIPtr()->SnapTurn(m_vDest - GetAIPtr()->GetPosition());

	GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Jump");

	GetAIPtr()->JumpTo(m_vDest);
}

void AIActionJumpTo::Update()
{
	if( GetAIPtr()->GetMovement()->GetMovementState() != CMS_SPECIAL )
	{
		// Be sure they are standing (especially wall-walkers!).
		GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Land");
		GetAIPtr()->StandUp();
		m_bDone = LTTRUE;
	}
}


void AIActionJumpTo::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead )
		return;

	AIAction::Load(hRead);   

	*hRead >> m_vDest;
}

void AIActionJumpTo::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite )
		return;

	AIAction::Save(hWrite);   

	*hWrite << m_vDest;
}



// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionPlayAnim
//
//	PURPOSE:	Plays an animation specified in the Init method
//
// ----------------------------------------------------------------------- //

void AIActionPlayAnim::Start()
{
	m_bDone = LTFALSE;
	m_bInterrupt = LTFALSE;

	m_bLoop = m_bNextLoop;
	m_bInterruptible = m_bNextInterruptible || m_bNextLoop;
	m_bFaceTarget = m_bNextFaceTarget;
	
	if( !GetAIPtr()->GetAnimation()->PlayAnim(const_cast<char *>(m_sAnim.c_str()), m_bLoop, m_bReset) )
	{
		m_bDone = LTTRUE;
	}
}

void AIActionPlayAnim::Update()
{
	if( m_bFaceTarget )
	{
		FaceTarget(GetAIPtr());
	}
}

void AIActionPlayAnim::End()
{
	if( GetAIPtr()->GetAnimation()->UsingAltAnim() )
	{
		GetAIPtr()->GetAnimation()->StopAnim(LTFALSE);
	}
}

void AIActionPlayAnim::AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo)
{
	if( nInfo == ANIMMSG_ALT_ANIM_BREAK )
	{
		// Only stop if it is a non-looping or interrupted
		// looping animation.
		if (!m_bLoop || m_bInterrupt)
		{
			GetAIPtr()->GetAnimation()->StopAnim(LTFALSE);
			m_bDone = LTTRUE;
		}
	}
}

void AIActionPlayAnim::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	AIAction::Load(hRead);

	*hRead >> m_sAnim;

	*hRead >> m_bNextLoop;
	*hRead >> m_bNextInterruptible;
	*hRead >> m_bNextFaceTarget;

	*hRead >> m_bLoop;
	*hRead >> m_bInterrupt;
	*hRead >> m_bInterruptible;
}

void AIActionPlayAnim::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	AIAction::Save(hWrite);

	*hWrite << m_sAnim;

	*hWrite << m_bNextLoop;
	*hWrite << m_bNextInterruptible;
	*hWrite << m_bNextFaceTarget;

	*hWrite << m_bLoop;
	*hWrite << m_bInterrupt;
	*hWrite << m_bInterruptible;
}


// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionPounce
//
//	PURPOSE:	Performs a pounce
//
// ----------------------------------------------------------------------- //

void AIActionPounce::Start()
{
	if( GetAIPtr()->GetInjuredMovement() )
	{
		m_bDone = LTTRUE;
		return;
	}

	m_bDone = LTFALSE;

	m_bStartedPounce = LTFALSE;

	GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_WEAPON, "None" );
	m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Pounce");
}

void AIActionPounce::Update()
{
	if( GetAIPtr()->GetAnimation()->UsingTransAnim() || m_bDone )
	{
		return; 
	}

	if( !m_bStartedPounce )
	{
		m_bStartedPounce = LTTRUE;

		RCMessage pMsg;
		if( !pMsg.null() )
		{
			pMsg->WriteObject(GetAIPtr()->GetTarget().GetObject());
			g_pLTServer->SendToObject(*pMsg,MID_FORCE_POUNCE, GetAIPtr()->m_hObject, GetAIPtr()->m_hObject,0);
			return;
		}
	}
	else
	{
		// If we aren't in the pouncing state, assume we are finished pouncing.
		if ( GetAIPtr()->GetMovement()->GetMovementState() != CMS_POUNCING )
		{
			GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Pounce_End");
			GetAIPtr()->StandUp();
			m_bDone = LTTRUE;
		}
	}
}

void AIActionPounce::AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo)
{
}

void AIActionPounce::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	AIAction::Load(hRead);

	*hRead >> m_bStartedPounce;
	*hRead >> m_bHasAnim;
}

void AIActionPounce::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	AIAction::Save(hWrite);

	*hWrite << m_bStartedPounce;
	*hWrite << m_bHasAnim;
}


// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionReload
//
//	PURPOSE:	Performs the reload animation and reloads the clip.
//
// ----------------------------------------------------------------------- //

void AIActionReload::Start()
{
	m_bDone = LTFALSE;

	// Actually reload the weapon.
	CWeapon * pWeapon = GetAIPtr()->GetCurrentWeaponPtr();
	if( pWeapon )
	{
		pWeapon->ReloadClip();
	}

	// Play the animation.
	if( !GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Reload") )
	{
		m_bDone = LTTRUE;
	}

	if( !m_bDone )
	{
		GetAIPtr()->ClearAim();
	}

}

void AIActionReload::Update()
{
	if( GetAIPtr()->HasTarget() && GetAIPtr()->AllowSnapTurn() )
	{
		GetAIPtr()->SnapTurn( GetAIPtr()->GetTarget().GetPosition() - GetAIPtr()->GetPosition() );
	}

}

void AIActionReload::AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo)
{
	if( nTracker == ANIM_TRACKER_MOVEMENT && nInfo == ANIMMSG_ANIM_BREAK )
	{
		m_bDone = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionStun
//
//	PURPOSE:	Performs the Stun animation.
//
// ----------------------------------------------------------------------- //

void AIActionStun::Start()
{
	m_bDone = LTFALSE;

	if (!GetAIPtr())
	{
		m_bDone = LTTRUE;
		return;
	}

	// Play the animation.
	const AIWBM_AIWeapon * pWeaponButes = GetAIPtr()->GetCurrentWeaponButesPtr();
	if( !pWeaponButes || pWeaponButes->eRace == AIWBM_AIWeapon::Alien  )
	{
		GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_WEAPON, "None");
	}

	if( !GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Stunned") )
	{
		GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Idle");
	}

	GetAIPtr()->ClearAim();
	GetAIPtr()->GetMovement()->AllowMovement(LTFALSE);
	GetAIPtr()->GetMovement()->AllowRotation(LTFALSE);
}

void AIActionStun::Update()
{
	const uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(GetAIPtr()->GetObject());
	if( !(dwUserFlags & USRFLG_CHAR_STUNNED) && !(dwUserFlags & USRFLG_CHAR_EMPEFFECT) )
	{
		m_bDone = LTTRUE;
	}
}

void AIActionStun::End()
{
	ASSERT(GetAIPtr());
	if (!GetAIPtr())
		return;

	GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Idle");
	GetAIPtr()->GetMovement()->AllowMovement(LTTRUE);
	GetAIPtr()->GetMovement()->AllowRotation(LTTRUE);
}

void AIActionStun::Load(HMESSAGEREAD hRead)
{
	ASSERT(hRead);
	if (!hRead) 
		return;

	AIAction::Load(hRead);
}

void AIActionStun::Save(HMESSAGEWRITE hWrite)
{
	ASSERT(hWrite);
	if (!hWrite)
		return;

	AIAction::Save(hWrite);
}

// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionTurnTo
//
//	PURPOSE:	Turns the AI to face a particular direction.
//
// ----------------------------------------------------------------------- //
void AIActionTurnTo::Start()
{
	// Initialize our state variables.
	m_bDone = LTFALSE;
	m_bHasAnim = LTFALSE;

	// Some handy constants.
	const LTVector & vMyRight = GetAIPtr()->GetRight();
	const LTVector & vMyForward = GetAIPtr()->GetForward();
	const LTVector & vMyUp = GetAIPtr()->GetUp();


	// Store away our initial rotation.
	m_rInitialRot = GetAIPtr()->GetRotation();


	// Be sure we have no up component to our turn.
	m_vDirection -= vMyUp * vMyUp.Dot(m_vDirection);
	m_vDirection.Norm();

	if( m_vDirection.MagSqr() < 0.9f )
	{
		m_vDirection = vMyForward;
	}

	if( GetAIPtr()->GetMovement()->GetControlFlags() & CM_FLAG_WALLWALK )
	{
		GetAIPtr()->SnapTurn(m_vDirection);
		m_bDone = LTTRUE;
	}

	// Set-up our final rotation.
	g_pMathLT->AlignRotation( m_rFinalRot, m_vDirection, const_cast<LTVector&>(vMyUp) );

	// Set our animation layer reference.
/*	LTBOOL bSmallTurn = vMyForward.Dot(m_vDirection) >= 0.0f;

	const LTFLOAT fRightDotDir = vMyRight.Dot(m_vDirection);
	if( fRightDotDir > FLT_EPSILON )
		m_bRightTurn = LTTRUE;
	else if( fRightDotDir < -FLT_EPSILON )
		m_bRightTurn = LTFALSE;
	else
		m_bRightTurn = GetRandom(0,10) >= 5;

	if( bSmallTurn )
	{
		if( m_bRightTurn )
			m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "TurnR90");
		else
			m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "TurnL90");
	}
	else
	{
		if( m_bRightTurn )
			m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "StepR");
		else
			m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "StepL");
	}
*/


	if( GetAIPtr()->GetMustCrawl() )
	{
		m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Crouch");
	}
	else if( GetAIPtr()->GetInjuredMovement() )
	{
		m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "InjuredTurn");
	}
	else if( GetAIPtr()->GetMovement()->GetControlFlags() & CM_FLAG_RUN )
	{
		m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "RunTurn");
	}
	else
	{
		m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "WalkTurn");
	}

	if( !m_bHasAnim )
	{
		const char * szAction = "Idle";
		if( GetAIPtr()->GetInjuredMovement() )
			szAction = "InjuredIdle";
		else if( GetAIPtr()->GetMustCrawl() )
			szAction = "Crouch";

		GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, szAction );
	}
	const LTFLOAT fTurnRate = MATH_DEGREES_TO_RADIANS(GetAIPtr()->GetVars()->m_fTurnRate);
	if( fTurnRate > 0.0f )
	{
		// Find the turn amount.  Anything beyond 90 degrees is at the same rate as 90 degrees.
		const LTFLOAT fForwardDotDirection = LTCLAMP(vMyForward.Dot(m_vDirection), -1.0f, 1.0f );
		const LTFLOAT fTurnAmount = LTMIN( (LTFLOAT) acos(fForwardDotDirection), MATH_HALFPI);

		m_fTurnLength = fTurnAmount / fTurnRate;

	}

	m_fStartTime = g_pLTServer->GetTime();
}

void AIActionTurnTo::Update()
{
	if( m_bDone )
		return;

	// Wait for the transition animation to complete.
	if( GetAIPtr()->GetAnimation()->UsingTransAnim() )
	{
		m_fStartTime = g_pLTServer->GetTime();
		return;
	}

	LTFLOAT fAnimPercent = 1.0f;
/*	if( m_bHasAnim )
	{
		const LTFLOAT fTurnAnimLength = (LTFLOAT)GetAIPtr()->GetAnimation()->GetAnimLength(ANIM_TRACKER_MOVEMENT);
		const LTFLOAT fTurnAnimTime = (LTFLOAT)GetAIPtr()->GetAnimation()->GetAnimTime(ANIM_TRACKER_MOVEMENT);
		if( fTurnAnimLength > 0.0f ) 
		{
			fAnimPercent = fTurnAnimTime / fTurnAnimLength;
		}
	}
	else
*/	{
		if( m_fTurnLength > 0.0f )
		{
			const LTFLOAT fTurnTime = g_pLTServer->GetTime() - m_fStartTime;
			fAnimPercent = fTurnTime / m_fTurnLength;
		}
	}

	if( fAnimPercent < 1.0f )
	{
		LTRotation rCurrentRot;
		g_pMathLT->InterpolateRotation(rCurrentRot, m_rInitialRot, m_rFinalRot, fAnimPercent );
		GetAIPtr()->GetMovement()->SetRot(rCurrentRot);
	}
	else
	{
		GetAIPtr()->GetMovement()->SetRot(m_rFinalRot);
		m_bDone = LTTRUE;
	}
}

void AIActionTurnTo::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	AIAction::Load(hRead);

	*hRead >> m_vDirection;
	*hRead >> m_rInitialRot;
	*hRead >> m_rFinalRot;
	*hRead >> m_bRightTurn;
	*hRead >> m_fTurnLength;
	m_fStartTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	*hRead >> m_bHasAnim;
}


void AIActionTurnTo::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	AIAction::Save(hWrite);

	*hWrite << m_vDirection;
	*hWrite << m_rInitialRot;
	*hWrite << m_rFinalRot;
	*hWrite << m_bRightTurn;
	*hWrite << m_fTurnLength;
	hWrite->WriteFloat(m_fStartTime - g_pLTServer->GetTime());
	*hWrite << m_bHasAnim;
}


// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionWWJumpTo
//
//	PURPOSE:	Jumps to a location
//
// ----------------------------------------------------------------------- //

void AIActionWWJumpTo::Start()
{
	m_bDone = LTFALSE;

	GetAIPtr()->ClipTo(m_vDest + m_vNormal*GetAIPtr()->GetMovement()->GetDimsSetting(CM_DIMS_WALLWALK).y*1.1f, m_vNormal);
}

void AIActionWWJumpTo::Update()
{
	if (GetAIPtr()->GetMovement()->GetMovementState() != CMS_SPECIAL)
		m_bDone = LTTRUE;
}

void AIActionWWJumpTo::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead )
		return;

	AIAction::Load(hRead);   

	*hRead >> m_vDest;
	*hRead >> m_vNormal;
}

void AIActionWWJumpTo::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite )
		return;

	AIAction::Save(hWrite);   

	*hWrite << m_vDest;
	*hWrite << m_vNormal;
}

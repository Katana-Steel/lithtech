// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionMoveTo.cpp
//
// PURPOSE : Implements the MoveTo action.
//
// CREATED : 2/6/01
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIActionMoveTo.h"

#include "AI.h"
#include "AIUtils.h"
#include "ContainerCodes.h"
#include "ltbeziercurve.h"

#ifdef _DEBUG
//#define BEZIER_DEBUG
//#include "DebugLineSystem.h"
#endif

static const char * GetMovementAction( uint32 nFlags, LTBOOL bBackwards, LTBOOL bCrouch, LTBOOL bInjuredMovement )
{
	if( bInjuredMovement )
		return "InjuredCrawl";

	if( nFlags & CM_FLAG_RUN )
	{
		if( !bBackwards )
		{
			if( !bCrouch )
				return "RunF";

			return "CRunF";
		}

		if( !bCrouch )
			return "RunB";

		return "CRunB";
	}

	if( !bBackwards )
	{
		if( !bCrouch ) 
			return "WalkF";

		return "CWalkF";
	}

	if( !bCrouch )
		return "WalkB";

	return "CWalkB";
}

static const char * GetWWMovementAction( uint32 nFlags )
{
	if( nFlags & CM_FLAG_RUN )
	{
		return "CRunF";
	}

	return "CWalkF";
}

// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionBezierTo
//
//	PURPOSE:	Follows a bezier curve to a location
//
// ----------------------------------------------------------------------- //

void AIActionBezierTo::Start()
{
	m_bDone = LTFALSE;
	const LTVector & vPos = GetAIPtr()->GetPosition();
	
	// Set-up our Bezier control points.
	m_vDest = m_vInitDest;
	m_vBezierDest = m_vDest + m_vInitBezierDest;
	m_vSrc = vPos;
	m_vBezierSrc = m_vSrc + m_vInitBezierSrc;

	m_vSrcNormal = GetAIPtr()->GetUp();
	m_vDestNormal = m_vInitDestNormal;


	// Determine how long it should take us.
	const LTFLOAT fSegmentLength = Bezier_SegmentLength( m_vSrc, m_vBezierSrc, m_vBezierDest, m_vDest );
	const LTFLOAT fMovementSpeed = GetAIPtr()->GetMovement()->GetMaxSpeed(CMS_WALLWALKING);

	if( fMovementSpeed > 0.0f )
	{
		m_fTotalTime = fSegmentLength / fMovementSpeed;
	}
	else
	{
		// Lets just assume we're moving at 200 units/sec.
		m_fTotalTime = fSegmentLength / 200.0f;
	}

	// We have to take some time, don't we?
	if( m_fTotalTime <= 0.0f )
		m_fTotalTime = 0.1f;

	// Be sure we know what time we started (this value will get updated if while we wait for a transition animation).
	m_fStartTime = g_pLTServer->GetTime();

	// Be sure we start with a "last position".
	m_vLastPosition = vPos;

	// Set our animation appropriately.
	GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, GetWWMovementAction(GetAIPtr()->GetMovement()->GetControlFlags()) );

	// Record that we haven't succeeded.
	GetAIPtr()->GetMovement()->GetCharacterVars()->m_bAllowJump = LTFALSE;
}

void AIActionBezierTo::Update()
{
	// Wait until we're done with any transition animations.
	if( GetAIPtr()->GetAnimation()->UsingTransAnim() )
	{
		m_fStartTime = g_pLTServer->GetTime();
		return;
	}

	// Some handy constants.
	CharacterMovement * const pMovement = GetAIPtr()->GetMovement();

	// 

	const LTFLOAT fDuration = g_pLTServer->GetTime() - m_fStartTime;

	if( fDuration >= m_fTotalTime*(1.0f - pMovement->GetCharacterVars()->m_fFrameTime) )
	{
		// Be sure we are at the correct position.
		pMovement->SetPos(m_vDest - m_vDestNormal);

		// Set our final rotation.
		LTVector vNewForward = pMovement->GetPos() - m_vLastPosition;
		vNewForward -= m_vDestNormal*m_vDestNormal.Dot(vNewForward);

		if( vNewForward.MagSqr() < 0.1f )
		{
			vNewForward = GetAIPtr()->GetForward()*3.0f;
			vNewForward -= m_vDestNormal*m_vDestNormal.Dot(vNewForward);
		}

		LTRotation rNewRot;
		g_pMathLT->AlignRotation(rNewRot, vNewForward, m_vDestNormal);
		pMovement->SetRot(rNewRot);

		// Record that we have succeeded.
		pMovement->GetCharacterVars()->m_bAllowJump = LTTRUE;

#ifdef BEZIER_DEBUG
		// Clean-up debug info.
		LineSystem::GetSystem(GetAIPtr(),"ShowRot")
			<< LineSystem::Clear();
#endif

		// And this action is done!
		m_bDone = LTTRUE;
		return;
	}

	ASSERT( m_fTotalTime > 0.0f );
	const LTFLOAT fPercent = fDuration / m_fTotalTime;

	// Set-up our flags.
	pMovement->SetObjectFlags(OFT_Flags, pMovement->GetObjectFlags(OFT_Flags) | FLAG_GOTHRUWORLD, LTFALSE);
	pMovement->SetObjectFlags(OFT_Flags, pMovement->GetObjectFlags(OFT_Flags) & ~(FLAG_GRAVITY | FLAG_SOLID | FLAG_DONTFOLLOWSTANDING | FLAG_REMOVEIFOUTSIDE) , LTFALSE);

	pMovement->SetObjectFlags(OFT_Flags2, pMovement->GetObjectFlags(OFT_Flags2) & ~(FLAG2_ORIENTMOVEMENT | FLAG2_SPHEREPHYSICS) , LTFALSE);

	// Determine our next position.
	LTVector vMoveToPos;
	Bezier_Evaluate(vMoveToPos, m_vSrc, m_vBezierSrc, m_vBezierDest, m_vDest, fPercent);

	// Set our next position.
	pMovement->SetPos(vMoveToPos);


	// Set our rotation, this must be done after the movement.
	const LTVector vInterpolatedNormal = m_vDestNormal*fPercent + m_vSrcNormal*(1.0f - fPercent);
	LTVector vNewForward = vMoveToPos - m_vLastPosition;

	vNewForward -= vInterpolatedNormal*vInterpolatedNormal.Dot(vNewForward);

	if( vNewForward.MagSqr() < 0.1f )
	{
		vNewForward = GetAIPtr()->GetForward();
		vNewForward -= vInterpolatedNormal*vInterpolatedNormal.Dot(vNewForward);
	}

#ifdef BEZIER_DEBUG
	LineSystem::GetSystem(GetAIPtr(),"ShowRot")
			<< LineSystem::Clear()
			<< LineSystem::Arrow( GetAIPtr()->GetPosition(),GetAIPtr()->GetPosition() + vNewForward*10.0f, Color::Blue )
			<< LineSystem::Arrow( GetAIPtr()->GetPosition(),GetAIPtr()->GetPosition() + vInterpolatedNormal*50.0f, Color::Green );
#endif

	LTRotation rNewRot;
	g_pMathLT->AlignRotation(rNewRot, const_cast<LTVector&>(vNewForward), const_cast<LTVector&>(vInterpolatedNormal) );
	pMovement->SetRot(rNewRot);

	// Record our current position for our next turn.
	m_vLastPosition = GetAIPtr()->GetPosition();
}

void AIActionBezierTo::End()
{
	CharacterMovement * const pMovement = GetAIPtr()->GetMovement();

	// Restore our flags.
	pMovement->SetObjectFlags(OFT_Flags, pMovement->GetCharacterVars()->m_dwFlags, LTFALSE);
	pMovement->SetObjectFlags(OFT_Flags2, pMovement->GetCharacterVars()->m_dwFlags2, LTFALSE);
}

LTBOOL AIActionBezierTo::IsInterruptible() const
{
	const CharacterMovement * const pMovement = GetAIPtr()->GetMovement();

	if( !m_bDone && m_fTotalTime > 0 )
		return ( g_pLTServer->GetTime() - m_fStartTime ) > m_fTotalTime*( 1.0f - pMovement->GetCharacterVars()->m_fFrameTime );

	return LTTRUE;
}

void AIActionBezierTo::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead )
		return;

	AIAction::Load(hRead);   

	*hRead >> m_vInitDest;
	*hRead >> m_vInitBezierSrc;
	*hRead >> m_vInitBezierDest;
	*hRead >> m_vInitDestNormal;

	*hRead >> m_vSrc;
	*hRead >> m_vDest;
	*hRead >> m_vBezierSrc;
	*hRead >> m_vBezierDest;
	*hRead >> m_vSrcNormal;
	*hRead >> m_vDestNormal;

	*hRead >> m_fTotalTime;
	m_fStartTime = hRead->ReadFloat() + g_pLTServer->GetTime();

	*hRead >> m_vLastPosition;
}

void AIActionBezierTo::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite )
		return;

	AIAction::Save(hWrite);   

	*hWrite << m_vInitDest;
	*hWrite << m_vInitBezierSrc;
	*hWrite << m_vInitBezierDest;
	*hWrite << m_vInitDestNormal;

	*hWrite << m_vSrc;
	*hWrite << m_vDest;
	*hWrite << m_vBezierSrc;
	*hWrite << m_vBezierDest;
	*hWrite << m_vSrcNormal;
	*hWrite << m_vDestNormal;

	*hWrite << m_fTotalTime;
	hWrite->WriteFloat(m_fStartTime - g_pLTServer->GetTime());

	*hWrite << m_vLastPosition;
}


// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionLadderTo
//
//	PURPOSE:	Moves to a point assuming it is in a ladder volume.
//
// ----------------------------------------------------------------------- //

void AIActionLadderTo::Start()
{
	// Initialize variables
	m_bDone = LTFALSE;

	m_vLadderDest = m_vInitLadderDest;
	m_vExitDest = m_vInitExitDest;

	const LTVector & my_dims = GetAIPtr()->GetMovement()->GetDimsSetting(CM_DIMS_DEFAULT);

	// Be sure the heights are correct,  the exit destination
	// assumes that it is on the ground.
	m_vExitDest.y += my_dims.y;
	m_vLadderDest.y = m_vExitDest.y;

	m_vDest = m_vLadderDest;

	const LTVector & my_pos = GetAIPtr()->GetPosition();
	if( LTFLOAT( fabs( m_vDest.y - my_pos.y ) ) <= my_dims.y*1.1f )
	{
		m_vDest = m_vExitDest;
	}

	// Don't aim!
	GetAIPtr()->ClearAim();

	// Force a ladder volume.
	GetAIPtr()->GetMovement()->GetCharacterVars()->m_nForceContainerType = CC_LADDER;

	// Set action layer
	const uint32 nFlags = GetAIPtr()->GetMovement()->GetControlFlags();
	const char * const szAction = GetMovementAction(nFlags, LTFALSE, GetAIPtr()->GetMustCrawl(), GetAIPtr()->GetInjuredMovement() );

	m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, szAction);
}

void AIActionLadderTo::Update()
{
	// Update should never happen if the action is done!
	_ASSERT( !m_bDone );

	// Wait for the transition animation to finish.
	if( GetAIPtr()->GetAnimation()->UsingTransAnim() )
		return;

	const LTVector & my_pos = GetAIPtr()->GetPosition();
	const LTVector & my_right = GetAIPtr()->GetRight();
	const LTVector & my_forward = GetAIPtr()->GetForward();
	const LTFLOAT horiz_epsilon = 0.5f;
	const LTFLOAT vert_epsilon = 0.5f;

	const LTVector rel_dest = m_vDest - my_pos;
	const LTVector rel_proj_dest(rel_dest.x, 0.0f, rel_dest.z);

	// Make sure there is no lingering horizontal movement.
//	GetAIPtr()->GetMovement()->UpdateVelocity( 0.001f, CM_DIR_FLAG_XZ_PLANE );

	// Be sure the movement action is set correctly.
	const uint32 nFlags = GetAIPtr()->GetMovement()->GetControlFlags();
	const char * const szAction = GetMovementAction(nFlags, LTFALSE, GetAIPtr()->GetMustCrawl(), GetAIPtr()->GetInjuredMovement() );

	m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, szAction);

	// Do our movement.
	if( !m_bHasAnim || GetAIPtr()->GetAnimation()->MovementAllowed() )
	{
		// Vertical movement.
		// We want to wait until we enter our climbing state before doing any 
		// of our vertical movement.
		if( GetAIPtr()->GetMovement()->GetMovementState() == CMS_CLIMBING )
		{
			if( rel_dest.y > vert_epsilon )
			{
				GetAIPtr()->Jump();
			}
			else if( rel_dest.y < -vert_epsilon )
			{
				GetAIPtr()->Crouch();
			}
			else
			{
				if( m_vDest.Equals(m_vExitDest,1.0f) )
				{
					// We have moved to our final destination, we're done.
					if( rel_proj_dest.MagSqr() <= horiz_epsilon*horiz_epsilon )
					{
						m_bDone = LTTRUE;
						return;
					}
					else
					{
						// Keep facing our destination.
						GetAIPtr()->SnapTurn(rel_proj_dest);
					}
				}
				else
				{
					// We need to start moving to our exit destination.
					m_vDest = m_vExitDest;
					GetAIPtr()->SnapTurn(rel_proj_dest);

					// Stop all movement and be sure we are standing like an AI should!
					GetAIPtr()->GetMovement()->UpdateVelocity( 0.001f, CM_DIR_FLAG_ALL );
					GetAIPtr()->StandUp();
				}
			}
		}

		// Horizontal movement.
		if( rel_proj_dest.Dot( my_forward ) > horiz_epsilon )
		{
			GetAIPtr()->Forward();
		}
		else if( rel_proj_dest.Dot( my_forward ) < -horiz_epsilon )
		{
			GetAIPtr()->Reverse();
		}
		else
		{
			GetAIPtr()->GetMovement()->UpdateVelocity( 0.001f, CM_DIR_FLAG_Z_AXIS );
		}

		if( rel_proj_dest.Dot( my_right ) > horiz_epsilon )
		{
			GetAIPtr()->StrafeRight();
		}
		else if( rel_proj_dest.Dot( my_right ) < -horiz_epsilon )
		{
			GetAIPtr()->StrafeLeft();
		}
		else
		{
			GetAIPtr()->GetMovement()->UpdateVelocity( 0.001f, CM_DIR_FLAG_X_AXIS );
		}
	}

}

void AIActionLadderTo::End()
{
	GetAIPtr()->GetMovement()->GetCharacterVars()->m_nForceContainerType = -1;
	GetAIPtr()->StandUp();
}

void AIActionLadderTo::AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo)
{
//	if( (nTracker == ANIM_TRACKER_MOVEMENT && nInfo == ANIMMSG_ANIM_BREAK) )
//	{
//		m_bDone = LTTRUE;
//	}
}

void AIActionLadderTo::PhysicsMsg(LTVector * pNewOffset)
{
	ASSERT( pNewOffset );

	LTVector my_pos;
	g_pLTServer->GetObjectPos(GetAIPtr()->m_hObject, &my_pos);

	LTRotation my_rot;
	g_pLTServer->GetObjectRotation(GetAIPtr()->m_hObject, &my_rot);

	LTVector my_up, my_forward, my_right;
	g_pMathLT->GetRotationVectors(my_rot, my_right, my_up, my_forward);

	const LTVector vInitialDist = (m_vDest - my_pos);
	const LTVector vFinalDist = vInitialDist - *pNewOffset;


	if( my_right.Dot( vInitialDist ) * my_right.Dot(vFinalDist) <= 0.01f )
	{
		*pNewOffset += my_right*my_right.Dot( vInitialDist - *pNewOffset );
	}

	if( my_forward.Dot( vInitialDist ) * my_forward.Dot(vFinalDist) <= 0.0f )
	{
		*pNewOffset += my_forward*my_forward.Dot( vInitialDist - *pNewOffset );
	}

	if( my_up.Dot( vInitialDist ) * my_up.Dot(vFinalDist) <= 0.0f )
	{
		*pNewOffset += my_up*my_up.Dot( vInitialDist - *pNewOffset );
	}

}

void AIActionLadderTo::Load(HMESSAGEREAD  hRead)
{
	ASSERT( hRead );
	if( !hRead )
		return;

	AIAction::Load(hRead);

	*hRead >> m_bHasAnim;

	*hRead >> m_vDest;
	*hRead >> m_vLadderDest;
	*hRead >> m_vExitDest;

	*hRead >> m_vInitLadderDest;
	*hRead >> m_vInitExitDest;
}


void AIActionLadderTo::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite )
		return;

	AIAction::Save(hWrite);

	*hWrite << m_bHasAnim;

	*hWrite << m_vDest;
	*hWrite << m_vLadderDest;
	*hWrite << m_vExitDest;

	*hWrite << m_vInitLadderDest;
	*hWrite << m_vInitExitDest;
}


// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionMoveTo
//
//	PURPOSE:	Moves to a point.
//
// ----------------------------------------------------------------------- //

void AIActionMoveTo::Start()
{
	// Initialize variables
	m_bDone = LTFALSE;
	m_bInterrupt = LTFALSE;

	m_vDest = m_vInitDest;
	m_bReverseMovement = LTFALSE;

	m_vStartDir = m_vDest - GetAIPtr()->GetPosition();
	m_vStartDir.y = 0.0f;
	const LTFLOAT fStartDist = m_vStartDir.Mag();
	
	m_vStartDir /= (fStartDist > 0.0f ? fStartDist : 1.0f);

	if( fStartDist <= GetAIPtr()->GetButes()->m_fMaxAIPopDist )
	{
		GetAIPtr()->GetMovement()->SetPos(m_vDest);
		m_bDone = LTTRUE;
		return;
	}

	// Shall we back-up?
	if( m_bAllowReverseMovement )
	{
		if( GetAIPtr()->GetForward().Dot( m_vDest - GetAIPtr()->GetPosition() ) < -0.0001f )
		{
			m_bReverseMovement = LTTRUE;
		}
	}

	if( m_bReverseMovement )
	{
		// We want to reverse our overshoot checking code for reverse movement.
		m_vStartDir = -m_vStartDir;
	}

	// Don't aim!
	GetAIPtr()->ClearAim();

	// Set action layer
	const uint32 nFlags = GetAIPtr()->GetMovement()->GetControlFlags();
	const char * const szAction = GetMovementAction(nFlags, m_bReverseMovement, GetAIPtr()->GetMustCrawl(), GetAIPtr()->GetInjuredMovement());

	m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, szAction);

	// Be sure we are not wall walking.
	if( nFlags & CM_FLAG_WALLWALK )
	{
		if( GetAIPtr()->GetMustCrawl() || GetAIPtr()->GetInjuredMovement())
		{
			GetAIPtr()->Crouch();
		}
		else
		{
			GetAIPtr()->StandUp();
		}
	}
}

void AIActionMoveTo::Update()
{
	if( m_bDone )
	{
		return;
	}

	// Wait for the transition animation to finish.
	if( GetAIPtr()->GetAnimation()->UsingTransAnim() )
		return;

	const LTVector & my_pos = GetAIPtr()->GetPosition();
	const LTVector & my_up = GetAIPtr()->GetUp();


	LTVector rel_proj_dest = m_vDest - my_pos;
	rel_proj_dest -= my_up*my_up.Dot(rel_proj_dest);

	if( m_bReverseMovement )
	{
		rel_proj_dest *= -1.0f;
	}

	// Keep facing the correct direction (even if we are not "allowed" to, this is needed for this action!)
	GetAIPtr()->SnapTurn(rel_proj_dest);

	// Make sure there is no left/right movement.
	GetAIPtr()->GetMovement()->UpdateVelocity( 0.001f, CM_DIR_FLAG_X_AXIS );

	// Be sure the movement action is set correctly.
	const uint32 nFlags = GetAIPtr()->GetMovement()->GetControlFlags();
	const char * const szAction = GetMovementAction(nFlags, m_bReverseMovement, GetAIPtr()->GetMustCrawl(), GetAIPtr()->GetInjuredMovement());

	m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, szAction);

	// And move forward.
	if( !m_bHasAnim || GetAIPtr()->GetAnimation()->MovementAllowed() )
	{
		if( !m_bReverseMovement )
			GetAIPtr()->Forward();
		else
			GetAIPtr()->Reverse();
	}
}

void AIActionMoveTo::AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo)
{
	if( (nTracker == ANIM_TRACKER_MOVEMENT && nInfo == ANIMMSG_ANIM_BREAK) )
	{
		m_bDone = LTTRUE;
	}
}

void AIActionMoveTo::PhysicsMsg(LTVector * pNewOffset)
{
	ASSERT( pNewOffset );

	LTVector vPos;
	g_pLTServer->GetObjectPos(GetAIPtr()->m_hObject,&vPos);

	const LTVector vCurrentDist = m_vDest - vPos;

	// If we are going to overshoot our destination.
	if( m_vStartDir.Dot(vCurrentDist - *pNewOffset) <= 0.01f )
	{
		// Set us at our destination.
		*pNewOffset = LTVector(vCurrentDist.x, pNewOffset->y, vCurrentDist.z);
		GetAIPtr()->Stop();

		m_bDone = LTTRUE;
//		g_pLTServer->SetNextUpdate(GetAIPtr()->m_hObject, 0.001f);
		return;
	}
}

void AIActionMoveTo::Load(HMESSAGEREAD  hRead)
{
	ASSERT( hRead );
	if( !hRead )
		return;

	AIAction::Load(hRead);

	*hRead >> m_bInterrupt;
	*hRead >> m_bHasAnim;

	*hRead >> m_vDest;
	*hRead >> m_vStartDir;
	*hRead >> m_bReverseMovement;

	*hRead >> m_vInitDest;
	*hRead >> m_bAllowReverseMovement;
}


void AIActionMoveTo::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite )
		return;

	AIAction::Save(hWrite);

	*hWrite << m_bInterrupt;
	*hWrite << m_bHasAnim;

	*hWrite << m_vDest;
	*hWrite << m_vStartDir;
	*hWrite << m_bReverseMovement;

	*hWrite << m_vInitDest;
	*hWrite << m_bAllowReverseMovement;
}


// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionMoveForward
//
//	PURPOSE:	Moves the AI forward for a bit.
//
// ----------------------------------------------------------------------- //

void AIActionMoveForward::Start()
{
	m_bDone = LTFALSE;
	m_tmrInterrupt.Stop();

	// Don't aim!
	GetAIPtr()->ClearAim();

	// Set action layer
	const uint32 nFlags = GetAIPtr()->GetMovement()->GetControlFlags();
	const char * const szAction = GetMovementAction(nFlags, LTFALSE, GetAIPtr()->GetMustCrawl(), GetAIPtr()->GetInjuredMovement());

	m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, szAction);

	if( !m_bHasAnim )
		m_tmrNoAnim.Start(0.75f);
}

void AIActionMoveForward::Update()
{
//	if( GetAIPtr()->GetAnimation()->UsingTransAnim() )
//		return;

	// Be sure the movement action is set correctly.
	const uint32 nFlags = GetAIPtr()->GetMovement()->GetControlFlags();
	const char * const szAction = GetMovementAction(nFlags, LTFALSE, GetAIPtr()->GetMustCrawl(), GetAIPtr()->GetInjuredMovement());

	m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, szAction);

	GetAIPtr()->Forward();

	if( m_tmrInterrupt.On() && m_tmrInterrupt.Stopped() )
	{
		m_bDone = LTTRUE;
		return;
	}

	if( !m_bHasAnim && m_tmrNoAnim.Stopped() )
	{
		m_bDone = LTTRUE;
	}
}

void AIActionMoveForward::AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo)
{
	if( m_bHasAnim )
	{
		if( nTracker == ANIM_TRACKER_MOVEMENT && nInfo == ANIMMSG_ANIM_BREAK )
		{
			m_bDone = LTTRUE;
		}
	}
}


void AIActionMoveForward::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	AIAction::Load(hRead);

	*hRead >> m_tmrInterrupt;
	*hRead >> m_bHasAnim;
	*hRead >> m_tmrNoAnim;

}

void AIActionMoveForward::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	AIAction::Save(hWrite);

	*hWrite << m_tmrInterrupt;
	*hWrite << m_bHasAnim;
	*hWrite << m_tmrNoAnim;

}



// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionWWForward
//
//	PURPOSE:	Wall Walks the AI forward for a bit (until animation ends or up vector changes).
//
// ----------------------------------------------------------------------- //

void AIActionWWForward::Start()
{
	if( GetAIPtr()->GetInjuredMovement() )
	{
		m_bDone = LTTRUE;
		return;
	}

	m_bDone = LTFALSE;
	m_vDest = m_vInitDest;
	m_vStartingUp = GetAIPtr()->GetUp();
	m_vLastUsedUp = m_vStartingUp;

	GetAIPtr()->SnapTurn( m_vDest - GetAIPtr()->GetPosition() );

	// Don't aim!
	GetAIPtr()->ClearAim();

	// Set action layer
	const uint32 nFlags = GetAIPtr()->GetMovement()->GetControlFlags();
	const char * const szAction = GetWWMovementAction(nFlags);

	m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, szAction);

	if( !m_bHasAnim )
		m_tmrNoAnim.Start(0.75f);
}


void AIActionWWForward::Update()
{
	if( GetAIPtr()->GetInjuredMovement() )
	{
		m_bDone = LTTRUE;
		return;
	}

	if( GetAIPtr()->GetAnimation()->UsingTransAnim() || m_bDone )
		return;

	// Always crouch.
	GetAIPtr()->WallWalk();

	if( !m_vLastUsedUp.Equals(GetAIPtr()->GetUp(), 0.05f) )
	{
		GetAIPtr()->SnapTurn( m_vDest - GetAIPtr()->GetPosition() );
		m_vLastUsedUp = GetAIPtr()->GetUp();
	}
	
	// Be sure the action is set correctly.
	const uint32 nFlags = GetAIPtr()->GetMovement()->GetControlFlags();
	const char * const szAction = GetWWMovementAction(nFlags);

	m_bHasAnim = GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, szAction);

	// And move forward.
	if( !m_bHasAnim || GetAIPtr()->GetAnimation()->MovementAllowed() )
		GetAIPtr()->Forward();

	if( !m_bHasAnim && m_tmrNoAnim.Stopped() )
	{
		m_bDone = LTTRUE;
	}
}

void AIActionWWForward::AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo)
{
	if( m_bHasAnim )
	{
		if( nTracker == ANIM_TRACKER_MOVEMENT && nInfo == ANIMMSG_ANIM_BREAK )
		{
			m_bDone = LTTRUE;
		}
	}
}

void AIActionWWForward::PhysicsMsg(LTVector * pNewOffset)
{
	ASSERT( pNewOffset );

	const LTVector & my_dims = GetAIPtr()->GetDims();

	LTVector my_pos;
	g_pLTServer->GetObjectPos(GetAIPtr()->m_hObject, &my_pos);

	LTRotation my_rot;
	g_pLTServer->GetObjectRotation(GetAIPtr()->m_hObject, &my_rot);

	LTVector my_up, my_forward, my_right;
	g_pMathLT->GetRotationVectors(my_rot, my_right, my_up, my_forward);

	const LTFLOAT fMaxDistSqr = my_dims.y*my_dims.y*2.1f;
	const LTVector vStart = my_pos - my_up*my_dims.y;

	const LTVector vLine = *pNewOffset;
	LTVector vRelDest = m_vDest - vStart;

	vRelDest -= my_right*my_right.Dot(vRelDest);

	// If the line is very short, just check if the line
	// is between the two points and if it is less than
	// max dist from the end point.
	//
	// Floating point errror would give worst results.
	if( vLine.MagSqr() <= 0.1f )
	{
		const LTFLOAT fLineDotPoint = vLine.Dot(vRelDest);
		if(    fLineDotPoint < -0.1f
			|| fLineDotPoint*fLineDotPoint > vLine.MagSqr() )
			return;

		if( (vRelDest - vStart).MagSqr() < fMaxDistSqr )
		{
			m_bDone = LTTRUE;
			GetAIPtr()->Stop();
//			g_pLTServer->SetNextUpdate(GetAIPtr()->m_hObject, 0.001f);
			return;
		}
	}

	const LTFLOAT  fT = vLine.Dot(vRelDest) / vLine.MagSqr();

	// Check to see if the point lies "between" the start and end point.
	if(    fT < 0.0f
		|| fT > 1.0f )
	{
		return;
	}


	// Okay, the point is between the two test points, check its 
	// distance from the line.
	if( (vRelDest - vLine*fT).MagSqr() < fMaxDistSqr )
	{
		m_bDone = LTTRUE;
		*pNewOffset *= fT;
		GetAIPtr()->Stop();
//		g_pLTServer->SetNextUpdate(GetAIPtr()->m_hObject, 0.001f);
		return;
	}

}

void AIActionWWForward::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	AIAction::Load(hRead);

	*hRead >> m_bHasAnim;
	*hRead >> m_tmrNoAnim;

	*hRead >> m_vStartingUp;
	*hRead >> m_vLastUsedUp;

	*hRead >> m_vInitDest;
	*hRead >> m_vDest;
}

void AIActionWWForward::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	AIAction::Save(hWrite);

	*hWrite << m_bHasAnim;
	*hWrite << m_tmrNoAnim;

	*hWrite << m_vStartingUp;
	*hWrite << m_vLastUsedUp;

	*hWrite << m_vInitDest;
	*hWrite << m_vDest;
}


/* This isn't used, it is here for reference.
static bool NearLineSegment( const LTVector & vTestPoint, const LTVector & vStart, const LTVector & vEnd, LTFLOAT fMaxDistSqr )
{
	const LTVector vLine = vEnd - vStart;
	const LTVector vRelTestPoint = vTestPoint - vStart;

	// If the line is very short, just check if the line
	// is between the two points and if it is less than
	// max dist from the end point.
	//
	// Floating point errror would give worst results.
	if( vLine.MagSqr() <= 0.1f )
	{
		const LTFLOAT fLineDotPoint = vLine.Dot(vRelTestPoint);
		if(  fLineDotPoint < 0.0f
			|| fLineDotPoint*fLineDotPoint > vLine.MagSqr() )
			return false;

		return (vRelTestPoint - vEnd).MagSqr() < fMaxDistSqr;
	}

	const LTFLOAT  fT = vLine.Dot(vRelTestPoint) / vLine.MagSqr();


	// Check to see if the point lies "between" the start and end point.
	// "Between" means projected ont
	if(    fT < 0.0f
		|| fT > 1.0f )
	{
		return false;
	}


	// Okay, the point is between the two test points, check its 
	// distance from the line.
	return (vRelTestPoint - vLine*fT).MagSqr() < fMaxDistSqr;
}
*/

// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategySetFacing.cpp
//
// PURPOSE : AI strategy class implementations
//
// CREATED : 9.9.2000
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIStrategySetFacing.h"
#include "AI.h"
#include "AITarget.h"
#include "Weapon.h"
#include "AIButeMgr.h"
#include "AIActionMisc.h"
#include "AIUtils.h"
#include "FXButeMgr.h"
#include "AIUtils.h"

#ifdef _DEBUG
//#define SET_FACING_DEBUG
//#define TARGET_PREDICTION_DEBUG
#include "DebugLineSystem.h"
#endif

void CAIStrategySetFacing::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	CAIStrategy::Load(hRead);

	*hRead >> m_bUseWeaponPosition;
	m_eFacingType = FacingType( hRead->ReadByte() );
	*hRead >> m_hFaceObject;
	*hRead >> m_vFacePos;
	*hRead >> m_vWeaponOffset;
	*hRead >> m_bFinishedFacing;
}

void CAIStrategySetFacing::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	CAIStrategy::Save(hWrite);

	*hWrite << m_bUseWeaponPosition;
	hWrite->WriteByte( m_eFacingType );  ASSERT( m_eFacingType < 256 );
	*hWrite << m_hFaceObject;
	*hWrite << m_vFacePos;
	*hWrite << m_vWeaponOffset;
	*hWrite << m_bFinishedFacing;
}




void CAIStrategySetFacing::Update()
{
	ASSERT( GetAIPtr() );
	if( !GetAIPtr() ) return;


	//
	// Set-up our facing destination (put into goal_pos).
	//
	LTVector goal_pos(0,0,0);
	switch( m_eFacingType )
	{
		case Target :
		{
			const CAITarget & target = GetAIPtr()->GetTarget();
			if( !target.IsValid() )
				return;

			if( m_bUseWeaponPosition && GetAIPtr()->GetCurrentWeaponPtr() )
			{
				goal_pos = TargetAimPoint(*GetAIPtr(), target, *GetAIPtr()->GetCurrentWeaponPtr() );
			}
			else
			{
				goal_pos = target.GetPosition();

				const CCharacter * pCharTarget = target.GetCharacterPtr();
				if( pCharTarget )
				{
					goal_pos += pCharTarget->GetHeadOffset();
				}
			}

		}
		break;

		case Object :
		{
			if( !m_hFaceObject ) return;
			g_pLTServer->GetObjectPos(m_hFaceObject, &goal_pos);
		}
		break;

		case Position :
		{
			goal_pos = m_vFacePos;
		}
		break;

		default:
		case None :
		{
			return;
		}
		break;
	}



	//
	// Get the point we are aiming from.
	//
	const LTVector & my_pos = m_bUseWeaponPosition ? GetAIPtr()->GetAimFromPosition() : GetAIPtr()->GetPosition();
	const LTVector & my_dims = GetAIPtr()->GetDims();
	const LTVector & my_up = GetAIPtr()->GetUp();
	const LTVector & my_forward = GetAIPtr()->GetForward();

#ifdef SET_FACING_DEBUG
	LineSystem::GetSystem(this,"ShowAimGoal") 
		<< LineSystem::Clear()
		<< LineSystem::Arrow( my_pos, goal_pos, Color::Cyan );
#endif

	// Be sure the turning point is far enough away.  If it
	// isn't, just claim we did turn in that direction
	LTVector me_to_goal = goal_pos - my_pos;
	LTVector proj_me_to_goal = me_to_goal - my_up*my_up.Dot(me_to_goal);

	// Only aim if we are using a weapon.``
	if(    m_bUseWeaponPosition 
		&& GetAIPtr()->GetCurrentWeaponPtr()
		&& GetAIPtr()->GetCurrentWeaponButesPtr() 
		&& !GetAIPtr()->GetCurrentWeaponButesPtr()->bMeleeWeapon )
	{
		// Check to see if we have to actually turn to aim.

		const LTFLOAT pmtg_dot_forward = proj_me_to_goal.Dot(my_forward);
		if(    pmtg_dot_forward < 0.0f 
			|| pmtg_dot_forward*pmtg_dot_forward < c_fCos35*c_fCos35*proj_me_to_goal.MagSqr() )
		{

			if( proj_me_to_goal.MagSqr() > my_dims.x*my_dims.x + my_dims.z*my_dims.z )
			{
				// Do the turn.
				// If you want to do an action turn, remove the above code 
				// and replace with this.
				AIActionTurnTo * pNewAction = dynamic_cast<AIActionTurnTo*>( GetAIPtr()->SetNextAction(CAI::ACTION_TURNTO) );
				if( pNewAction )
				{
					pNewAction->Init(me_to_goal);
					m_bFinishedFacing = false;
					return;
				}
			}
		}

		// And aim.
		GetAIPtr()->AimAt(goal_pos);

		// And assumed we are finished (the aiming takes a little time, but we will ignore that).
		m_bFinishedFacing = true;
	}
	else
	{
		const LTFLOAT pmtg_dot_forward = proj_me_to_goal.Dot(my_forward);
		if(    pmtg_dot_forward < 0.0f 
			|| pmtg_dot_forward*pmtg_dot_forward < c_fCos15*c_fCos15*proj_me_to_goal.MagSqr() )
		{
			// Only turn if the goal is far enough away (this prevents spinning when the destination is very
			// near, like when the player is right on top of the AI).
			if( proj_me_to_goal.MagSqr() > my_dims.x*my_dims.x + my_dims.z*my_dims.z )
			{
				AIActionTurnTo * pNewAction = dynamic_cast<AIActionTurnTo*>( GetAIPtr()->SetNextAction(CAI::ACTION_TURNTO) );
				if( pNewAction )
				{
					pNewAction->Init(me_to_goal);
					m_bFinishedFacing = false;
					return;
				}
			}
			else
			{
				m_bFinishedFacing = true;
			}
		}
		else
		{
			// Do a snap turn.
			GetAIPtr()->SnapTurn(me_to_goal);
			
			m_bFinishedFacing = true;
		}
	}

#ifdef SET_FACING_DEBUG
		LineSystem::GetSystem(this,"ShowAimGoal") 
			<< LineSystem::Arrow( my_pos, goal_pos, Color::Blue );
#endif

}

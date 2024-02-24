// ----------------------------------------------------------------------- //
//
// MODULE  : AIMeleeAttack.h
//
// PURPOSE : Implements the Melee Attack goal and state.
//
// CREATED : 8/1/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIMeleeAttack.h"
#include "AI.h"
#include "AITarget.h"
#include "AIButeMgr.h"
#include "AINodeMgr.h"

#ifdef _DEBUG
//#include "AIUtils.h"
//#define MELEE_ATTACK_DEBUG
#endif 

CAIStateMeleeAttack::CAIStateMeleeAttack(CAI * pAI)
	: CAIStateAttack(pAI),
	  m_pDestNode(LTNULL)
{
	ASSERT( pAI );
	if( !pAI ) return;

	const AIBM_Melee & melee_butes = g_pAIButeMgr->GetMelee( pAI->GetTemplateId() );

	m_fJumpProbability = melee_butes.fJumpProbability;

	m_rngJumpDelay = FRange( melee_butes.fMinJumpDelay,   melee_butes.fMaxJumpDelay );

	m_StrategyFollowPath.Init(pAI);
	m_StrategyFollowPath.FaceTarget();
}

void CAIStateMeleeAttack::Load(HMESSAGEREAD hRead)
{
	_ASSERT( hRead );

	CAIStateAttack::Load(hRead);

	*hRead >> m_tmrJumpDelay;
	*hRead >> m_StrategyFollowPath;
	m_pDestNode = g_pAINodeMgr->GetNode( hRead->ReadDWord() );
	*hRead >> m_fJumpProbability;
	*hRead >> m_rngJumpDelay;
}

void CAIStateMeleeAttack::Save(HMESSAGEWRITE hWrite)
{
	_ASSERT( hWrite );

	CAIStateAttack::Save(hWrite);

	*hWrite << m_tmrJumpDelay;
	*hWrite << m_StrategyFollowPath;
	hWrite->WriteDWord( m_pDestNode ? m_pDestNode->GetID() : CAINode::kInvalidID );
	*hWrite << m_fJumpProbability;
	*hWrite << m_rngJumpDelay;
}

void CAIStateMeleeAttack::Start()
{
	CAIStateAttack::Start();

	m_pDestNode = LTNULL;
	m_tmrJumpDelay.Start( GetJumpDelay() );
}

void CAIStateMeleeAttack::ApproachTarget()
{
	if( GetAIPtr()->GetButes()->m_bWallWalk )
	{
		if( !m_pDestNode )
		{
			// Closest wall-walking node.
			const LTVector target_pos = GetAIPtr()->GetTarget().GetPosition();
			const LTVector my_pos = GetAIPtr()->GetPosition();
			LTFLOAT fClosestNodeDistSqr = FLT_MAX;
			const CAINode * pClosestNode = LTNULL;

			for( CAINodeMgr::NodeList::const_iterator iter = g_pAINodeMgr->BeginNodes();
			     iter != g_pAINodeMgr->EndNodes(); ++iter )
			{
				if( (*iter)->IsWallWalkOnly() )
				{
					if( ((*iter)->GetPos() - my_pos).Dot( target_pos - my_pos ) > 0.0f )
					{
						const LTFLOAT fCurrentNodeDistSqr = ((*iter)->GetPos() - target_pos).MagSqr();
						if( fCurrentNodeDistSqr < fClosestNodeDistSqr )
						{
							fClosestNodeDistSqr = fCurrentNodeDistSqr;
							pClosestNode = (*iter);
						}
					}
				}
			}

			if( pClosestNode )
			{
				if( m_StrategyFollowPath.Set(pClosestNode) )
				{
					m_pDestNode = pClosestNode;
				}
			}
		}

		if( m_pDestNode && m_StrategyFollowPath.IsSet() )
		{
			m_StrategyFollowPath.Update();
			return;
		}
		else if( m_pDestNode )
		{
			GetAIPtr()->StandUp();

			// We know we are leaving the node, so let the AI know.
			GetAIPtr()->SetLastNodeReached(LTNULL);
			GetAIPtr()->SetNextNode(LTNULL);
		}
	}


	CAIStateAttack::ApproachTarget();

	PerformJumping();
}

void CAIStateMeleeAttack::PerformJumping()
{
//	const bool bWallWalking = (GetAIPtr()->GetCharacterClass() == ALIEN); //PLH DEBUG
//	if( bWallWalking ) return;

	if( !GetAIPtr() || !GetAIPtr()->HasTarget() ) return;

	if( m_tmrJumpDelay.Stopped() )
	{
		m_tmrJumpDelay.Start( GetJumpDelay() );

		if( GetRandom(0.0f,1.0f) < m_fJumpProbability )
		{
			GetAIPtr()->Jump();
		}
	}
}

void CAIStateMeleeAttack::FireOnTarget()
{
	CAIStateAttack::FireOnTarget();

	PerformJumping();
}

LTFLOAT CAIStateMeleeAttack::GetJumpDelay()
{
	return GetRandom( m_rngJumpDelay.Min(), m_rngJumpDelay.Max() );
}
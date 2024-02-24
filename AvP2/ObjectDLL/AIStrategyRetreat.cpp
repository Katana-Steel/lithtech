// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategyRetreat.cpp
//
// PURPOSE : This strategy causes the AI to move to the neighboring volume
//			 furthest from his target.  This continues until the enemy no 
//			 longer has LOS, or the AI is cornered.
//
// CREATED : 8.15.2000
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AI.h"
#include "AIStrategyRetreat.h"
#include "AITarget.h"
#include "AIVolume.h"
#include "CharacterHitBox.h"
#include "AIButeMgr.h"
#include "AINodeMgr.h"
#include "AINodeVolume.h"
#include "AIUtils.h"
#include "CharacterMgr.h"
#include "AIActionMisc.h"

#ifdef _DEBUG
//#include "DebugLineSystem.h"
//#include "AIUtils.h"
//#define STRATEGY_RETREAT_DEBUG
#endif

CAIStrategyRetreat::CAIStrategyRetreat(CAI * pAI)
	: CAIStrategy(pAI),
	  m_StrategyFollowPath(pAI),
	  m_StrategySetFacing(pAI, true),
	  m_pDestNode(LTNULL),
	  m_bFlee(LTFALSE)
{
//	m_StrategySetFacing.FaceTarget();

	SetupFSM();
}

void CAIStrategyRetreat::Start()
{
	m_StrategyFollowPath.Clear(); 
	m_pDestNode = LTNULL;
	m_RootFSM.SetState( Root::eStateRunning );
}

void CAIStrategyRetreat::Update()
{
	CAIStrategy::Update();

	if(!GetAIPtr())
	{
#ifndef _FINAL
		g_pInterface->CPrint("WARNING: pAI invalid");
#endif
		return;
	}

	m_RootFSM.Update();
}

void CAIStrategyRetreat::SetupFSM()
{
	m_RootFSM.DefineState( Root::eStateRunning,  
						   m_RootFSM.MakeCallback(*this, &CAIStrategyRetreat::UpdateRunning),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyRetreat::StartRunning) );

	m_RootFSM.DefineState( Root::eStateRunningToCorner,  
						   m_RootFSM.MakeCallback(*this, &CAIStrategyRetreat::UpdateRunningToCorner),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyRetreat::StartRunningToCorner) );

	m_RootFSM.DefineState( Root::eStateCornered,  
						   m_RootFSM.MakeCallback(*this, &CAIStrategyRetreat::UpdateCornered),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyRetreat::StartCornered) );

	m_RootFSM.SetInitialState( Root::eStateRunning );

	// Default Transitions.
	m_RootFSM.DefineTransition(Root::eTargetInvalid, Root::eStateCornered);

	// Standard Transitions
	m_RootFSM.DefineTransition(Root::eStateRunning, Root::eCornered, Root::eStateRunningToCorner);

	m_RootFSM.DefineTransition(Root::eStateRunningToCorner, Root::eCornered, Root::eStateCornered);
	m_RootFSM.DefineTransition(Root::eStateRunningToCorner, Root::eNotCornered, Root::eStateRunning);

	m_RootFSM.DefineTransition(Root::eStateCornered, Root::eNotCornered, Root::eStateRunning);

#ifdef STRATEGY_RETREAT_DEBUG
	m_RootFSM.SetChangingStateCallback(m_RootFSM.MakeChangingStateCallback(*this,&CAIStrategyRetreat::PrintRootFSMState));
	m_RootFSM.SetProcessingMessageCallback(m_RootFSM.MakeProcessingMessageCallback(*this,&CAIStrategyRetreat::PrintRootFSMMessage));
#endif
}


void CAIStrategyRetreat::StartRunning(RootFSM * fsm)
{
	if(!GetAIPtr() || !GetAIPtr()->HasTarget() )
	{
		fsm->AddMessage(Root::eTargetInvalid);
		return;
	}

	m_pDestNode = LTNULL;
}


static const CAINode * GetCurrentNode(const CAI & ai)
{
#ifdef STRATEGY_RETREAT_DEBUG
	LineSystem::GetSystem(&ai,"CurrentNode")
		<< LineSystem::Clear();

	if( ai.GetNextNode() )
	{
		LineSystem::GetSystem(&ai,"CurrentNode")
			<< LineSystem::Arrow(ai.GetPosition(), ai.GetNextNode()->GetPos(), Color::Yellow);
	}

	if( ai.GetLastNodeReached() )
	{
		LineSystem::GetSystem(&ai,"CurrentNode")
			<< LineSystem::Arrow(ai.GetPosition(), ai.GetLastNodeReached()->GetPos(), Color::Blue);
	}

	if( ai.GetCurrentVolumePtr() )
	{
		CAINode * pVolNode = g_pAINodeMgr->GetNode(ai.GetCurrentVolumePtr()->GetNodeID());
		if( pVolNode )
			LineSystem::GetSystem(&ai,"CurrentNode")
				<< LineSystem::Arrow(ai.GetPosition(), pVolNode->GetPos(), Color::White);
	}
#endif	

	// Get our current position in the node system.
	const CAINode * pCurNode  = ai.GetNextNode();

	if( !pCurNode )
	{
		pCurNode = ai.GetLastNodeReached();
	}

	if( !pCurNode )
	{
		// Try grabbing our volume as a node.
		const CAIVolume * const pCurVol = ai.GetCurrentVolumePtr();
		if( pCurVol )
		{
			pCurNode = g_pAINodeMgr->GetNode(pCurVol->GetNodeID());
		}

		if( !pCurNode )
		{
			// Woah!  Things are getting bad, try getting our last volume as a node.
			const CAIVolume * const pLastVol = ai.GetLastVolumePtr();
			
			if( pLastVol )
			{
				pCurNode = g_pAINodeMgr->GetNode(pLastVol->GetNodeID());
			}
		}
	}

	return pCurNode;
}

static const CAINode * FindBetterRetreatNode( const CAI & ai, const CAINode * pCurrentRetreatNode,
											  CAINode::NeighborList::const_iterator begin_nodes, CAINode::NeighborList::const_iterator end_nodes, 
									          const LTVector & vTargetPos, LTBOOL bAllowWWNode )
{
	const LTVector & my_pos = ai.GetPosition();

	LTVector vRelTargetPos = vTargetPos - my_pos;
	vRelTargetPos.y = 0;

	LTFLOAT fBestNodeDotEnemy = 0.0f;
	const CAINode * pBetterNode = LTNULL;

	if (!ai.GetCurrentVolumePtr())
	{
		if (ai.GetLastVolumePtr())
		{
			return g_pAINodeMgr->GetNode(ai.GetLastVolumePtr()->GetNodeID());
		}
		else
			return LTNULL;
	}

	if( pCurrentRetreatNode )
	{
		fBestNodeDotEnemy = vRelTargetPos.Dot(pCurrentRetreatNode->GetPos() - my_pos);

		if( fBestNodeDotEnemy > 0.0f ) fBestNodeDotEnemy = 0.0f;
	}

	for(CAINode::NeighborList::const_iterator iter = begin_nodes; iter != end_nodes; ++iter )
	{
		const CAINode * pNode = *iter;
		if( pNode && ai.CanUseNode(*pNode)  )
		{
			// Filter out wall walk nodes.
			if( !bAllowWWNode && pNode->IsWallWalkOnly() )
			{
				continue;
			}

			// No need to check our current node, if we hit it.
			if( pCurrentRetreatNode == pNode )
			{
				continue;
			}

			// only choose a destination that is farther away from the enemy
			const LTFLOAT fNodeDotEnemy = vRelTargetPos.Dot(pNode->GetPos() - my_pos);
			if (  fNodeDotEnemy < fBestNodeDotEnemy )
			{
				fBestNodeDotEnemy = fNodeDotEnemy;
				pBetterNode = pNode;
			}
			// If we are dealing with a volume neighbor node, be sure to check the end points of
			// the connection.
			else if( const CAIVolumeNeighborNode * pVolumeNeighborNode = dynamic_cast<const CAIVolumeNeighborNode*>( pNode ) )
			{
#if 0
				LTVector vEndpoint1 = (pVolumeNeighborNode->GetConnectionEndpoint1() - my_pos);
				vEndpoint1 -= my_up*my_up.Dot(vEndpoint1);
				const LTFLOAT fNode1DotEnemy = vEndpoint1.Dot(vTargetPos);
				if( fNode1DotEnemy < fBestNodeDotEnemy )
				{
					fBestNodeDotEnemy = fNode1DotEnemy;
					pBetterNode = pVolumeNeighborNode;
				}
				else 
				{
					LTVector vEndpoint2 = (pVolumeNeighborNode->GetConnectionEndpoint2() - my_pos);
					vEndpoint2 -= my_up*my_up.Dot(vEndpoint2);
					const LTFLOAT fNode2DotEnemy = vEndpoint2.Dot(vTargetPos);
					if( fNode2DotEnemy < fBestNodeDotEnemy )
					{
						fBestNodeDotEnemy = fNode2DotEnemy;
						pBetterNode = pVolumeNeighborNode;
					}
				}
#endif
			}
		}
	}

	return pBetterNode;
}


void CAIStrategyRetreat::UpdateRunning(RootFSM *)
{
	if(!GetAIPtr() || !GetAIPtr()->HasTarget()) 
	{
		m_RootFSM.AddMessage(Root::eTargetInvalid);
		return;
	}

	const CAITarget & target = GetAIPtr()->GetTarget();

	// If this is set, go find a nearby friend
	if (m_hFriend)
	{
		LTVector vDest;
		g_pInterface->GetObjectPos(m_hFriend, &vDest);

		if (!m_StrategyFollowPath.IsSet())
			m_StrategyFollowPath.Set(vDest);

		m_StrategyFollowPath.Update();
		return;
	}


	// Otherwise, find a node that takes us away from our target
	const CAINode * pCurNode = GetCurrentNode(*GetAIPtr());

	// We have no idea which we node we're at, what are we to do?
	// Exit out gracefully.
	if (!pCurNode)
	{
		// Maybe if we keep following our last path, we'll get
		// back into the volume system.
		m_StrategyFollowPath.Update();
		return;
	}


	// Find the best retreat node.
	// Exclude our destination if we have reached it.
	const CAINode * pBetterNode = FindBetterRetreatNode( *GetAIPtr(), m_pDestNode,
		                                             const_cast<CAINode*>(pCurNode)->BeginNeighbors(), const_cast<CAINode*>(pCurNode)->EndNeighbors(),
													 target.GetPosition(), m_bFlee );


	if( pBetterNode && m_StrategyFollowPath.Set(pBetterNode) )
	{
		// The target has moved, causing our destination node to change.
		m_pDestNode = pBetterNode;

		// see if we can fire off a shot to get this enemy off us
		// This doesn't look good.
		//FireOnTarget();

#ifdef STRATEGY_RETREAT_DEBUG

		LineSystem::GetSystem(this,"ShowBetterNode")
			<< LineSystem::Clear()
			<< LineSystem::Arrow( GetAIPtr()->GetPosition(), pBetterNode->GetPos(), Color::Cyan );
#endif
	}
	else if( !m_StrategyFollowPath.IsSet() )
	{
		m_RootFSM.AddMessage(Root::eCornered);
		return;
	}

	m_StrategyFollowPath.Update();

	if( m_StrategyFollowPath.IsDone() )
	{
		pCurNode = GetCurrentNode(*GetAIPtr());

		pBetterNode = FindBetterRetreatNode( *GetAIPtr(), m_pDestNode,
			                                 const_cast<CAINode*>(pCurNode)->BeginNeighbors(), const_cast<CAINode*>(pCurNode)->EndNeighbors(),
											 target.GetPosition(), m_bFlee );

		if( pBetterNode && m_StrategyFollowPath.Set(pBetterNode) )
		{
			m_pDestNode = pBetterNode;
		}
		else if( !m_StrategyFollowPath.IsSet() )
		{
			m_RootFSM.AddMessage(Root::eCornered);
			return;
		}

		m_StrategyFollowPath.Update();
	}
}


void CAIStrategyRetreat::StartRunningToCorner(RootFSM * fsm)
{
	m_StrategyFollowPath.Clear();
}

void CAIStrategyRetreat::UpdateRunningToCorner(RootFSM * fsm)
{

	if (!GetAIPtr() || !GetAIPtr()->HasTarget())
	{
		// No target... nothing to run from.
		return;
	}

	const CAITarget & target = GetAIPtr()->GetTarget();

	const CAINode * pCurNode = GetCurrentNode(*GetAIPtr());

	// We have no idea which we node we're at, what are we to do?
	// Exit out gracefully.
	if (!pCurNode)
	{
		// Maybe if we keep following our last path, we'll get
		// back into the volume system.
		m_StrategyFollowPath.Update();
		return;
	}

	// Find the best retreat node.
	const CAINode * pBetterNode = FindBetterRetreatNode( *GetAIPtr(), m_pDestNode,
		                                             const_cast<CAINode*>(pCurNode)->BeginNeighbors(), const_cast<CAINode*>(pCurNode)->EndNeighbors(),
													 target.GetPosition(), m_bFlee );

	// We found a retreat node, we're not cornered!
	// Note that the UpdateRunning code will find this node again, so there is no need to try to pass it along.
	if( pBetterNode && m_StrategyFollowPath.Set(pBetterNode) )
	{
		m_RootFSM.AddMessage(Root::eNotCornered);
		return;
	}


	// Okay, no retreat node.  Just move backward.
	LTVector vRetreatDest = target.GetPosition() - GetAIPtr()->GetPosition();
	vRetreatDest.Norm( -3.0f*GetAIPtr()->GetDims().z );
	vRetreatDest += GetAIPtr()->GetPosition();

	if( !m_StrategyFollowPath.Set(vRetreatDest) )
	{
		// If we couldn't set our destination then the destination is outside the volumes,
		// we're cornered!
		m_RootFSM.AddMessage(Root::eCornered);
		return;
	}	

	m_StrategyFollowPath.Update();

	if(    m_StrategyFollowPath.IsDone() 
		|| m_StrategyFollowPath.IsStuck() 
		|| WillClip(*GetAIPtr(), GetAIPtr()->GetPosition(), vRetreatDest) )
	{
		// If we have reached our destination or are stuck, we're cornered!
		m_RootFSM.AddMessage(Root::eCornered);
		return;
	}

	return;
}

void CAIStrategyRetreat::StartCornered(RootFSM * fsm)
{
/*
	if(!GetAIPtr() || !GetAIPtr()->HasTarget() )
	{
		fsm->AddMessage(Root::eTargetInvalid);
		return;
	}
*/
}

void CAIStrategyRetreat::UpdateCornered(RootFSM * fsm)
{

	if (!GetAIPtr() || !GetAIPtr()->HasTarget())
	{
		// No target... nothing to run from.
		return;
	}

	const CAITarget & target = GetAIPtr()->GetTarget();

	const CAINode * pCurNode = GetCurrentNode(*GetAIPtr());

	// We have no idea which we node we're at, what are we to do?
	// Exit out gracefully.
	if (!pCurNode)
	{
		FireOnTarget();
		return;
	}

	// Find the best retreat node.
	const CAINode * pBetterNode = FindBetterRetreatNode( *GetAIPtr(), m_pDestNode,
		                                             const_cast<CAINode*>(pCurNode)->BeginNeighbors(), const_cast<CAINode*>(pCurNode)->EndNeighbors(),
													 target.GetPosition(), m_bFlee );

	// We found a retreat node, we're not cornered!
	// Note that the UpdateRunning code will find this node again, so there is no need to try to pass it along.
	if( pBetterNode )
	{
		m_RootFSM.AddMessage(Root::eNotCornered);
		return;
	}

	// Face our target (even if we're fleeing, we want to face them and cower in fear!).
	const LTVector vTargetDir = GetAIPtr()->GetTarget().GetPosition() - GetAIPtr()->GetPosition();
	const LTFLOAT target_dot_forward = vTargetDir.Dot(GetAIPtr()->GetForward());
	if(    target_dot_forward > 0.0f 
		&& target_dot_forward*target_dot_forward > c_fCos45*c_fCos45*vTargetDir.MagSqr() )
	{
		const LTVector & my_dims = GetAIPtr()->GetDims();
		if( GetAIPtr()->AllowSnapTurn() && vTargetDir.MagSqr() > my_dims.x*my_dims.x + my_dims.z*my_dims.z )
		{
			// Just snap in that direction.
			GetAIPtr()->SnapTurn( vTargetDir );
		}
	}
	else
	{
		// Do a turn action.
		AIActionTurnTo * pNewAction = dynamic_cast<AIActionTurnTo*>( GetAIPtr()->SetNextAction(CAI::ACTION_TURNTO) );
		if( pNewAction )
		{
			pNewAction->Init(vTargetDir);
			return;
		}
	}

	if(    GetAIPtr()->GetCurrentWeaponPtr()
		&& GetAIPtr()->GetCurrentWeaponButesPtr()
		&& NeedReload(*GetAIPtr()->GetCurrentWeaponPtr(), *GetAIPtr()->GetCurrentWeaponButesPtr()) )
	{
		GetAIPtr()->SetNextAction(CAI::ACTION_RELOAD);
		return;
	}

	// No place to go -- try to attack
	FireOnTarget();

	return;
}

void CAIStrategyRetreat::FireOnTarget()
{
	const AIWBM_AIWeapon * pWeaponProfile = GetAIPtr()->GetCurrentWeaponButesPtr();
	const CWeapon * pCurrentWeapon = GetAIPtr()->GetCurrentWeaponPtr();
	if( pWeaponProfile && pCurrentWeapon )
	{
		// Relaod the weapon if need be.
		if( NeedReload(*pCurrentWeapon, *pWeaponProfile) )
		{
			GetAIPtr()->SetNextAction( CAI::ACTION_RELOAD );
			return;
		}


		// Only fire if within range.
		const LTFLOAT target_range_sqr 
			= ( GetAIPtr()->GetTarget().GetPosition() - GetAIPtr()->GetPosition() ).MagSqr();

		if(    target_range_sqr > pWeaponProfile->fMinFireRange*pWeaponProfile->fMinFireRange
			&& target_range_sqr < pWeaponProfile->fMaxFireRange*pWeaponProfile->fMaxFireRange )
		{
			// Must have full SeeEnemy stimulation in order to fire at a cloaked enemy
			if (GetAIPtr()->GetTarget().GetCharacterPtr()->GetCloakState() != CLOAK_STATE_INACTIVE)
			{
				if (!GetAIPtr()->GetTarget().IsFullyVisible())
					return;
			}

			// Only fire if target can be seen.
			if( GetAIPtr()->GetTarget().IsPartiallyVisible() && GetAIPtr()->BurstDelayExpired() )
			{
				if (!GetAIPtr()->WillHurtFriend())
				{
					GetAIPtr()->SetNextAction( CAI::ACTION_FIREWEAPON );
					return;
				}
			}
		}
	}

	return;
}

void CAIStrategyRetreat::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	CAIStrategy::Load(hRead);

	*hRead >> m_RootFSM;
	*hRead >> m_StrategyFollowPath;
	*hRead >> m_StrategySetFacing;
	*hRead >> m_bFlee;
	*hRead >> m_hFriend;
}

void CAIStrategyRetreat::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	CAIStrategy::Save(hWrite);

	*hWrite << m_RootFSM;
	*hWrite << m_StrategyFollowPath;
	*hWrite << m_StrategySetFacing;
	*hWrite << m_bFlee;
	*hWrite << m_hFriend;
}


#ifdef STRATEGY_RETREAT_DEBUG

void CAIStrategyRetreat::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
	const char * szMessage = 0;
	switch( state )
	{
		case Root::eStateRunning : szMessage = "eStateRunning"; break;
		case Root::eStateRunningToCorner : szMessage = "eStateRunningToCorner"; break;
		case Root::eStateCornered : szMessage = "eStateCornered"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
		" Retreat::RootFSM is entering state %s.", szMessage );
}

void CAIStrategyRetreat::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{

	const char * szMessage = 0;
	switch( message )
	{
		case Root::eCornered : szMessage = "eCornered"; break;
		case Root::eNotCornered : szMessage = "eNotCornered "; break;
		case Root::eTargetInvalid : szMessage = "eTargetInvalid"; break;
		default : szMessage = "Unknown"; break;
	}

	if( fsm.HasTransition( fsm.GetState(), message ) )
	{
		AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
					" Retreat::RootFSM is processing %s.", szMessage );
	}
}

#endif

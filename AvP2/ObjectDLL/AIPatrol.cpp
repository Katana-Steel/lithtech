// ----------------------------------------------------------------------- //
//
// MODULE  : Patrol.h
//
// PURPOSE : Implements the Patrol goal and state.
//
// CREATED : 5/26/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AIPatrol.h"
#include "AI.h"
#include "AITarget.h"
#include "AINodeMgr.h"
#include "AINodePatrol.h"
#include "AIButeMgr.h"
#include "AIActionMisc.h"

#include <vector>
#include <string>
#include <algorithm>

#ifdef _DEBUG
//#include "AIUtils.h"
//#define PATROL_DEBUG
#endif

/////////////////////////////////////////////////
//
//     Patrol State
//
/////////////////////////////////////////////////


CAIStatePatrol::CAIStatePatrol(CAI * pAI)
	: CAIState(pAI),
	  m_StrategyFollowPath(pAI),
	  m_pLastPatrolNodeReached(0),
	  m_pPatrolNode(0),
	  m_bForward(true)
{
	_ASSERT(pAI);

	SetupFSM();

	const AIBM_Patrol & butes = g_pAIButeMgr->GetPatrol( pAI->GetTemplateId() );

	m_rngStuckDelay = FRange(butes.fMinStuckDelay,butes.fMaxStuckDelay);
}




void CAIStatePatrol::SetupFSM()
{

	//
	// Set up the root FSM.
	//
	m_RootFSM.DefineState( Root::eStateFindNextNode, 
		                   m_RootFSM.MakeCallback(*this,&CAIStatePatrol::UpdateFindNextNode) );

	m_RootFSM.DefineState( Root::eStateWait, 
						   m_RootFSM.MakeCallback(*this,&CAIStatePatrol::UpdateWait),
						   m_RootFSM.MakeCallback(*this,&CAIStatePatrol::StartWait)   );

	m_RootFSM.DefineState( Root::eStateGoToNode, 
						   m_RootFSM.MakeCallback(*this,&CAIStatePatrol::UpdateGoToNode),
						   m_RootFSM.MakeCallback(*this,&CAIStatePatrol::StartGoToNode)   );

	m_RootFSM.DefineState( Root::eStateEndOfPath,
						   m_RootFSM.MakeCallback() );

	m_RootFSM.DefineTransition( Root::eEndOfPath, Root::eStateEndOfPath );

	m_RootFSM.DefineTransition( Root::eStateFindNextNode, Root::eFinishedState, Root::eStateGoToNode );
	// Ignore Root::eStuck

	m_RootFSM.DefineTransition( Root::eStateGoToNode, Root::eFinishedState, Root::eStateWait );
	// Ignore Root::eStuck
	

	m_RootFSM.DefineTransition( Root::eStateWait,     Root::eFinishedState, Root::eStateFindNextNode );
	// Ignore Root::eStuck

#ifdef PATROL_DEBUG
	m_RootFSM.SetChangingStateCallback(m_RootFSM.MakeChangingStateCallback(*this,&CAIStatePatrol::PrintRootFSMState));
	m_RootFSM.SetProcessingMessageCallback(m_RootFSM.MakeProcessingMessageCallback(*this,&CAIStatePatrol::PrintRootFSMMessage));
#endif

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStatePatrol::HandleParameter
//
//	PURPOSE:	Sets data from name/value pairs
//
// ----------------------------------------------------------------------- //

bool CAIStatePatrol::HandleParameter(const char * const * pszArgs, int nArgs)
{
	if( CAIState::HandleParameter(pszArgs, nArgs) ) return true;

	if ( !_stricmp(*pszArgs, "LINK") )
	{
		ASSERT( nArgs == 2 );
		if( nArgs != 2 ) return false;

		++pszArgs;
		--nArgs;

		if( *pszArgs )
		{
			CAINode * pNode = g_pAINodeMgr->GetNode( *pszArgs );

			if( !pNode )
			{
				// If we couldn't find a node by that name, try
				// to use the name as a base name.
				std::string strNodeName = *pszArgs;
				strNodeName += "0";

				pNode = g_pAINodeMgr->GetNode( strNodeName.c_str() );
			}

			if( pNode )
			{
				CAINodePatrol * pPathNode = dynamic_cast<CAINodePatrol*>( pNode );

				if ( pPathNode )
				{
					// The node exists and is a Patrol node.  Set it as our current path.
					SetPath(pPathNode);
				}
				else
				{
#ifndef _FINAL
					g_pServerDE->CPrint("Non-Patrol waypoint \"%s\" given to patrol state.", *pszArgs );
#endif
				}
			}
			else
			{
#ifndef _FINAL
				g_pServerDE->CPrint("Unknown patrol waypoint \"%s\".", *pszArgs );
#endif
			}

		}
		else
		{
#ifndef _FINAL
			g_pServerDE->CPrint( "State patrol given a LINK parameter with no value!"  );
#endif
		}

		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStatePatrol::Start
//
//	PURPOSE:	Called just before the state is started
//
// ----------------------------------------------------------------------- //

void CAIStatePatrol::Start()
{
	CAIState::Start();

	m_StrategyFollowPath.Clear();

	GetAIPtr()->SetMovementStyle();
	GetAIPtr()->Walk();
	GetAIPtr()->SetNextAction(CAI::ACTION_IDLE);

	// never patrol with a flashlight on
	if (GetAIPtr()->IsFlashlightOn())
		GetAIPtr()->SetFlashlight(LTFALSE);

	if(    m_pLastPatrolNodeReached 
		&& m_pPatrolNode
		&& m_pLastPatrolNodeReached == m_pPatrolNode)
	{
		if(	   m_pPatrolNode->GetNextID(m_bForward) >= g_pAINodeMgr->GetNumNodes()
			&& m_pPatrolNode->GetNextID(!m_bForward) >= g_pAINodeMgr->GetNumNodes() )
		{
			// This is a single node route, always go to the node.
			m_RootFSM.SetState(Root::eStateGoToNode);
		}
		else
		{
			// This is a multinode route and we are sitting at a node,
			// go on to the next node.
			m_RootFSM.SetState(Root::eStateFindNextNode);
		}
	}
	else
	{
		// We have a node to go to, get to it!
		m_RootFSM.SetState(Root::eStateGoToNode);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStatePatrol::Update
//
//	PURPOSE:	Update of the state
//
// ----------------------------------------------------------------------- //

void CAIStatePatrol::Update()
{
	CAIState::Update();

	if( m_pPatrolNode )	
		m_RootFSM.Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStatePatrol::SetPath
//
//	PURPOSE:	Sets the path to follow.
//
// ----------------------------------------------------------------------- //
void CAIStatePatrol::SetPath(const CAINodePatrol * pPatrolNode) 
{ 
	m_pPatrolNode = pPatrolNode;
	m_pLastPatrolNodeReached = LTNULL;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStatePatrol::xxxFindNextNode
//
//	PURPOSE:	Sets next path node to be reached.
//
// ----------------------------------------------------------------------- //
void CAIStatePatrol::UpdateFindNextNode(RootFSM * rootFSM)
{
	if( m_pPatrolNode )
	{
		// Detect single node paths and don't increment them!
		if(	   m_pPatrolNode->GetNextID(m_bForward) >= g_pAINodeMgr->GetNumNodes()
			&& m_pPatrolNode->GetNextID(!m_bForward) >= g_pAINodeMgr->GetNumNodes() )
		{

			// Orient ourselves with the patrol node.
			AIActionTurnTo * pNewAction = dynamic_cast<AIActionTurnTo *>( GetAIPtr()->SetNextAction( CAI::ACTION_TURNTO ) );
			if( pNewAction )
			{
				pNewAction->Init( m_pPatrolNode->GetForward() );
			}

			rootFSM->AddMessage(Root::eEndOfPath);
			return;
		}

		// Possibly reverse direction if our next id is invalid.
		if( m_pPatrolNode->GetNextID(m_bForward) >= g_pAINodeMgr->GetNumNodes() )
		{
			// Reverse direction if the node has cycle marked as true.
			if( m_pPatrolNode->GetCycle() )	
			{
				m_bForward = !m_bForward;
			}
			else
			{
				rootFSM->AddMessage(Root::eEndOfPath);
				return;
			}
		}

		// This should only fail if the path has only one point.  And the above check should 
		// determine that and bail out!
		_ASSERT( m_pPatrolNode->GetNextID(m_bForward) < g_pAINodeMgr->GetNumNodes() );
		if( m_pPatrolNode->GetNextID(m_bForward) < g_pAINodeMgr->GetNumNodes() )
		{
			const CAINodePatrol * pNextNode 
				= dynamic_cast<CAINodePatrol*>( g_pAINodeMgr->GetNode( m_pPatrolNode->GetNextID(m_bForward) ) );

			if( pNextNode )
			{
				// Okay, we found our next node.  Set it and move on.
				m_pPatrolNode = pNextNode;
				rootFSM->AddMessage(Root::eFinishedState);
				return;
			}
		}

	} //if( m_pPatrolNode )
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStatePatrol::xxxGoToNode
//
//	PURPOSE:	Goes to the next path node.
//
// ----------------------------------------------------------------------- //

void CAIStatePatrol::StartGoToNode(RootFSM * rootFSM)
{
	if( m_pPatrolNode )
	{
		m_StrategyFollowPath.ExactMovement( m_pPatrolNode->GetWaitTime() > 0.0f );
		if( !m_StrategyFollowPath.Set(m_pPatrolNode, LTTRUE) )
		{
			// This is a mean, nasty trick.
			// The rest of the rootFSM is run off messages, but
			// at this one point I can't use a message.  I really want
			// the engine to wait for one cycle before going on.
			rootFSM->SetNextState(Root::eStateFindNextNode);
			return;
		}
	}
}

void CAIStatePatrol::UpdateGoToNode(RootFSM * rootFSM)
{
	m_StrategyFollowPath.Update();

	if( m_StrategyFollowPath.IsDone() )
	{
		m_pLastPatrolNodeReached = m_pPatrolNode;
		rootFSM->AddMessage(Root::eFinishedState);
		return;
	}

}

void CAIStatePatrol::StartWait(RootFSM * rootFSM)
{
	if( m_pPatrolNode )
	{
		// Start the wait timer.
		m_tmrWait.Start( m_pPatrolNode->GetWaitTime() );
		if( m_pPatrolNode->GetWaitTime() > 0 )
		{
			AIActionTurnTo * pNewAction = dynamic_cast<AIActionTurnTo *>( GetAIPtr()->SetNextAction( CAI::ACTION_TURNTO ) );
			if( pNewAction )
			{
				pNewAction->Init( m_pPatrolNode->GetForward() );
			}
		}
	}
}

void CAIStatePatrol::UpdateWait(RootFSM * rootFSM)
{
	// Wait for the timer to run out.
	if( m_tmrWait.Stopped() )
	{
		// We're done with this state!
		rootFSM->AddMessage(Root::eFinishedState);
		return;
	}
}


#ifndef _FINAL


#ifdef _DEBUG
const char * CAIStatePatrol::GetDescription() const
{
	static char description[100];

	sprintf(description, "Patrol %s",
		(m_pPatrolNode ? m_pPatrolNode->GetName().c_str() : "(node not set)") );

	return description;
}
#else
const char * CAIStatePatrol::GetDescription() const
{
	return "Patrol";
}
#endif

#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStatePatrol::Load
//
//	PURPOSE:	Restores the State
//
// ----------------------------------------------------------------------- //

void CAIStatePatrol::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);

	CAIState::Load(hRead);

	*hRead >> m_RootFSM;

	*hRead >> m_StrategyFollowPath;

	m_pPatrolNode = dynamic_cast<CAINodePatrol*>( g_pAINodeMgr->GetNode(hRead->ReadDWord()) );
	m_pLastPatrolNodeReached = dynamic_cast<CAINodePatrol*>( g_pAINodeMgr->GetNode(hRead->ReadDWord()) );

	*hRead >> m_tmrWait;
	*hRead >> m_bForward;

	*hRead >> m_rngStuckDelay;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStatePatrol::Save
//
//	PURPOSE:	Saves the State
//
// ----------------------------------------------------------------------- //

void CAIStatePatrol::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);

	CAIState::Save(hWrite);

	*hWrite << m_RootFSM;

	*hWrite << m_StrategyFollowPath;
	
	hWrite->WriteDWord( m_pPatrolNode ? m_pPatrolNode->GetID() : CAINode::kInvalidID );
	hWrite->WriteDWord( m_pLastPatrolNodeReached ? m_pLastPatrolNodeReached->GetID() : CAINode::kInvalidID );

	*hWrite << m_tmrWait;
	*hWrite << m_bForward;

	*hWrite << m_rngStuckDelay;
}


#ifdef PATROL_DEBUG

void CAIStatePatrol::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
	const char * szMessage = 0;
	switch( state )
	{
		case Root::eStateFindNextNode : szMessage = "eStateFindNextNode"; break;
		case Root::eStateWait : szMessage = "eStateWait"; break;
		case Root::eStateGoToNode : szMessage = "eStateGoToNode"; break;
		case Root::eStateEndOfPath : szMessage = "eStateEndOfPath"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
		" StatePatrol::RootFSM is entering state %s.", szMessage );
}

void CAIStatePatrol::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{

	const char * szMessage = 0;
	switch( message )
	{
		case Root::eFinishedState : szMessage = "eFinishedState"; break;
		case Root::eEndOfPath  : szMessage = "eEndOfPath"; break;
		case Root::eStuck: szMessage = "eStuck"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
				" StatePatrol::RootFSM is processing %s.", szMessage );
}


#endif

/////////////////////////////////////////////////
//
//     Patrol Goal
//
/////////////////////////////////////////////////


PatrolGoal::PatrolGoal( CAI * pAI, std::string create_name)
	: Goal(pAI, create_name ),
	  m_PatrolState(pAI)
{
}


bool PatrolGoal::HandleParameter(const char * const * pszArgs, int nArgs)
{
	return m_PatrolState.HandleParameter(pszArgs,nArgs);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStatePatrol::GetBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT PatrolGoal::GetBid()
{ 
	const LTFLOAT fDefault = g_pAIButeMgr->GetDefaultBid(BID_PATROL);
	LTFLOAT fBid = 0.0f;

	if (m_PatrolState.HasValidPath())
		fBid = fDefault;
	
	return fBid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStatePatrol::GetMaximumBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT PatrolGoal::GetMaximumBid()
{ 
	return g_pAIButeMgr->GetDefaultBid(BID_PATROL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatrolGoal::Load/Save
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void PatrolGoal::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);
	if (!hRead) return;

	Goal::Load(hRead);

	*hRead >> m_PatrolState;
}


void PatrolGoal::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);
	if (!hWrite) return;

	Goal::Save(hWrite);
	
	*hWrite << m_PatrolState;
}



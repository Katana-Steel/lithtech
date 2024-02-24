// ----------------------------------------------------------------------- //
//
// MODULE  : BidMgr.cpp
//
// PURPOSE : Implementation of Bid Manager
//
// CREATED : 6-16-00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BidMgr.h"
#include "AI.h"
#include "AIUtils.h"

// This function should be const but LithTech
// uses a non-const reference for their load/save
// functionality. Hopefully in the future, the 
// LithTech engineers will have read Scott Meyer's
// and will actually program in C++.
void Bidder::Save(HMESSAGEWRITE hWrite) /*const*/
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	*hWrite << m_fModifiedBid;
	
	if( m_pGoal )
	{
		*hWrite << m_pGoal->GetCreateName();
		*hWrite << *m_pGoal;
	}
	else
	{
		*hWrite << std::string("");
	}
}


void Bidder::Load(HMESSAGEREAD hRead, CAI * pAI)
{
	ASSERT( hRead && pAI );
	if( !hRead || !pAI ) return;

	*hRead >> m_fModifiedBid;

	std::string create_name;
	*hRead >> create_name;

	if( !create_name.empty() )
	{
		m_pGoal = pAI->CreateGoal(create_name.c_str());
		ASSERT( m_pGoal );
		if( m_pGoal )
		{
			*hRead >> *m_pGoal;
		}
	}
}


BidMgr::~BidMgr()
{
	//
	// Delete all our goal pointers.
	//

	// Delete the goals still in the bid list.
	for( BidList::iterator bid_iter = bidList.begin(); bid_iter != bidList.end(); ++bid_iter )
	{
		Bidder & current_bidder = bid_iter->second;

		// This will clean-up the old goal.
		current_bidder.SetGoal(LTNULL);
	}

	// Delete the goals in the "to-be-deleted" list.
	for( GoalPtrList::iterator goal_iter = goalsToDelete.begin(); goal_iter != goalsToDelete.end(); ++goal_iter )
	{
		delete *goal_iter;
		*goal_iter = LTNULL;
	}
}

std::string BidMgr::GetSlotName(const Bidder * pBidder) const
{
	for( BidList::const_iterator iter = bidList.begin();
	     iter != bidList.end(); ++iter )
	{
		if( &iter->second == pBidder )
			return iter->first;
	}

	return std::string();
}


Bidder * BidMgr::GetBidder(const std::string & slot_name)
{
	BidList::iterator list_location = bidList.find(slot_name);
	
	if( list_location != bidList.end() )
		return &list_location->second;

	return LTNULL;
}

// Because bids can be changed in the middle of a goal update, we need
// to make sure the goals aren't actually deleted until a safe time.
// This function does the deleting and should be called somewhere in the AI update,
// outside of a goal update!
void BidMgr::CleanUpGoals()
{
	while( !goalsToDelete.empty() )
	{
		delete goalsToDelete.back();
		goalsToDelete.pop_back();
	}
}

void BidMgr::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	hWrite->WriteDWord(GetNumBids() );

	for( BidList::iterator iter = BeginBids();
	     iter != EndBids(); ++iter )
	{
		const std::string & name = iter->first;
		/*const*/ Bidder & bidder = iter->second;

		*hWrite << name;
		bidder.Save(hWrite);
	}
}



void BidMgr::Load(HMESSAGEREAD hRead, CAI * pAI)
{
	ASSERT( hRead );
	if( !hRead ) return;

	const int num_bidders = hRead->ReadDWord();

	for( int i = 0; i < num_bidders; ++i )
	{
		std::string name;
		*hRead >> name;

		if( bidList[name].GetGoalPtr() )
		{
			goalsToDelete.push_back(bidList[name].GetGoalPtr());
		}

		bidList[name].Load(hRead,pAI);
	}
}

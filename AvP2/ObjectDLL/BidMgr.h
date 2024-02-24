// ----------------------------------------------------------------------- //
//
// MODULE  : BidMgr.h
//
// PURPOSE : Implements the Bid Manager.
//
// CREATED : 5/17/00
//
// ----------------------------------------------------------------------- //

#ifndef __BIDMGR_H__
#define __BIDMGR_H__

#include <map>
#include <string>
#include <vector>

#include "Goal.h"


class Bidder
{
public:

	Bidder()
		: m_fModifiedBid(0),
		  m_pGoal(0)  {}

	~Bidder()
	{
		// be sure you delete m_pGoal!
	}

	void SetGoal(Goal * new_goal_ptr, LTFLOAT modified_bid = 0.0f )
	{
		if( m_pGoal && m_pGoal != new_goal_ptr )
		{
			delete m_pGoal;
		}
		
		m_fModifiedBid  = modified_bid;
		m_pGoal = new_goal_ptr;
	}

	void Save(HMESSAGEWRITE hWrite) /*const*/;
	void Load(HMESSAGEREAD hRead, CAI * pAI);

	Goal * GetGoalPtr() const   { return m_pGoal; }

	LTFLOAT  GetMaximumBid() const 
	{
		if( m_fModifiedBid > 0.0f )
			return m_fModifiedBid; 
	
		if( m_pGoal )
			return m_pGoal->GetMaximumBid();

		return 0.0f;
	}

	LTFLOAT  GetBid() const 
	{
		if (m_pGoal)
		{
			const LTFLOAT fGoalBid = m_pGoal->GetBid();

			if( m_fModifiedBid > 0.0f && fGoalBid > 0.0f )
				return m_fModifiedBid;

			return fGoalBid;
		}

		return 0.0f;
	}


	void SetModifiedBid(LTFLOAT val) { m_fModifiedBid = val; }

private:

	LTFLOAT    m_fModifiedBid;
	Goal   *   m_pGoal;

};

class BidMgr
{

public :
	class CaselessStrCmp
	{
	public:
		
		bool operator()(const std::string & x, const std::string & y) const
		{
			// In MSVC's implementation of std::string, c_str() is very cheap.
			// Even if this is used with other std libraries, the goals
			// names should be short enough to allow the string to be stored
			// in one buffer.
			return stricmp( x.c_str(), y.c_str() ) < 0;
		}
	};

	typedef std::map<std::string,Bidder,CaselessStrCmp> BidList;
	typedef BidList::iterator            BidIterator;
	typedef BidList::const_iterator		 const_BidIterator;

	typedef std::vector<Goal*>	GoalPtrList;

public :

	~BidMgr();

	void Save(HMESSAGEWRITE hWrite);
	void Load(HMESSAGEREAD hRead, CAI * pAI);

	void AddBid(std::string name, Goal * ptr_to_goal, LTFLOAT modified_bid  )
	{
		if( bidList[name].GetGoalPtr() )
		{
			goalsToDelete.push_back(bidList[name].GetGoalPtr());
		}

		bidList[name].SetGoal(ptr_to_goal, modified_bid);
	}

	bool RemoveBid(const std::string & name)
	{
		goalsToDelete.push_back(bidList[name].GetGoalPtr());
		return bidList.erase(name) > 0;
	}


	bool RemoveBid(const Bidder & bidder)
	{
		for( BidIterator iter = BeginBids();
		     iter != EndBids(); ++iter )
		{
			if( &(iter->second) == &bidder )
			{
				goalsToDelete.push_back(iter->second.GetGoalPtr());
				bidList.erase(iter);
				return true;
			}
		}

		return false;
	}

	void SetBid(const std::string & name, LTFLOAT modified_bid)
	{
		const BidList::iterator goal = bidList.find(name);
		if( goal != bidList.end() )
		{
			(goal->second).SetModifiedBid(modified_bid);
		}
	}

	void CleanUpGoals();

	std::string GetSlotName(const Bidder * pBidder) const;
	Bidder *    GetBidder(const std::string & slot_name);

	BidList::size_type GetNumBids() const { return bidList.size(); }

	BidIterator BeginBids()			  { return bidList.begin(); }
	const_BidIterator BeginBids() const { return bidList.begin(); }

	BidIterator EndBids()			  { return bidList.end(); }
	const_BidIterator EndBids() const { return bidList.end(); }

private :

	BidList bidList;
	GoalPtrList goalsToDelete;  // This is where goal pointers get placed until it is safe to delete them.
};

#endif // __BIDMGR_H__

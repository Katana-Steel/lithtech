// ----------------------------------------------------------------------- //
//
// MODULE  : SimpleNode.h
//
// PURPOSE : SimpleNode class definition
//
// CREATED : 7/8/01
//
// ----------------------------------------------------------------------- //

#ifndef _SIMPLE_NODE_H_
#define _SIMPLE_NODE_H_

#include "cpp_engineobjects_de.h"
#include "ltsmartlink.h"

#include <vector>
#include <string>

#include "GameBase.h"

class SimpleNode;
class ILTMessage;

class CSimpleNode
{
	public:
		enum MicrosoftHackToGetConstantsInHeader
		{
			kInvalidID = 0xFFFFFFFF
		};

		// For Pathing
		typedef std::vector<uint32> NeighborIDList;
		typedef std::vector<CSimpleNode*> NeighborList;

		typedef NeighborList::iterator NeighborIterator;
		typedef NeighborList::const_iterator const_NeighborIterator;

		typedef LTFLOAT CostType;

	public : // Public methods

		explicit CSimpleNode();
		CSimpleNode(const SimpleNode & node, uint32 dwID);
		CSimpleNode(const CSimpleNode & srcNode, const LTVector & position);
		CSimpleNode(HMESSAGEREAD hRead);
		
		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		// Simple accessors

		uint32	GetID() const { return m_dwID; }

		std::string GetName() const { return m_strName; }

		const LTVector& GetPos() const { return m_vPos; }
		const LTVector& GetAttractorPos() const { return m_vAttractorPos; }

		const LTVector& GetDims() const { return m_vDims; }

		LTFLOAT GetHalfHeight() const { return m_vDims.y; }
		LTFLOAT GetRadius() const     { return m_fRadius; }
		LTFLOAT GetRadiusSqr() const  { return m_fRadiusSqr; }

		void  AddNeighbor(uint32 dwNeighborID);

#ifndef _FINAL
		virtual void DrawDebug(int level);
		virtual void ClearDebug();
#endif
		// For Pathing

		// This should be called on the start and destination node with the current
		// path finding iteration (done in SimpleNodeMgr::FindPath).
		void ResetPathing(uint32 nPathingIteration );

		void  SetIsGoal(bool val) { m_bIsGoal = val; }
		void  SetCost(float val) { m_fCost = val; }
		void  SetInClosed()      { m_bInOpen = false; m_bInClosed = true;  }
		void  SetInOpen()        { m_bInOpen = true;  m_bInClosed = false; }
		void  SetHeuristic(float val) { m_fHeuristic = val; }
		void  SetPrevNode(CSimpleNode * prevNode) { m_pPrevNode = prevNode; }

		bool  IsGoal() const { return m_bIsGoal; }
		float GetCost() const { return m_fCost; }
		float GetWeight()   const { return m_fCost + m_fHeuristic; }
		
		bool  InClosed() const { return m_bInClosed; }
		bool  InOpen()   const { return m_bInOpen;   }

		CSimpleNode * GetPrevNode()             { return m_pPrevNode; }
		const CSimpleNode * GetPrevNode() const { return m_pPrevNode; }

		NeighborIterator BeginNeighbors();
		NeighborIterator EndNeighbors();

		const_NeighborIterator BeginNeighbors() const;
		const_NeighborIterator EndNeighbors() const;

		NeighborIDList::iterator BeginNeighborIDs() { return m_NeighborIDList.begin(); }
		NeighborIDList::iterator EndNeighborIDs()   { return m_NeighborIDList.end();   }


	private :

		void  InternalResetPathing();  // Does the actual reseting.

		uint32		m_dwID;						// Node ID
		std::string	m_strName;					// Node name

		LTVector	m_vPos;						// Node position
		LTVector	m_vAttractorPos;			// Attractor position
		
		LTVector	m_vDims;					// The SimpleNode's dimensions.
		LTFLOAT		m_fRadius;					// The SimpleNode's radius.
		LTFLOAT		m_fRadiusSqr;				// The SimpleNode's radius squared.

		LTSmartLink m_hDebugNodeBox;

		NeighborIDList m_NeighborIDList;
		mutable NeighborList   m_NeighborList;

		// For pathing.  Doesn't need to be saved.
		uint32		m_nLastPathingIteration;
		bool		m_bResetNeighbors;

		bool		m_bIsGoal;

		LTFLOAT		m_fCost;
		LTFLOAT		m_fHeuristic;

		bool		m_bInOpen;
		bool		m_bInClosed;
		CSimpleNode *	m_pPrevNode;
};

inline ILTMessage & operator<<(ILTMessage & out, CSimpleNode & node)
{
	node.Save(&out);

	return out;
}

inline ILTMessage & operator>>(ILTMessage & in, CSimpleNode & node)
{
	node.Load(&in);

	return in;
}


class SimpleNode : public GameBase
{
	friend CSimpleNode;

	public :
		
		SimpleNode()
			: m_vDims(16.0f,16.0f,16.0f),
			  m_fRadius(0.0f),
			  m_vOffset(0.0f,0.0f,0.0f) {}

		virtual ~SimpleNode() {}

		virtual void PostStartWorld(uint8 nLoadGameFlags);

		uint32 EngineMessageFn(uint32 messageID, void *p, LTFLOAT f);

		virtual void ReadProp(ObjectCreateStruct *p);


	private :

		LTVector	m_vDims;
		LTFLOAT		m_fRadius;
		LTVector	m_vOffset;
};

#endif // _SIMPLE_NODE_H_


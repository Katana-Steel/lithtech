// ----------------------------------------------------------------------- //
//
// MODULE  : AINode.h
//
// PURPOSE : AINode class definition
//
// CREATED : 12/15/98
//
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_H_
#define _AI_NODE_H_

#include "cpp_engineobjects_de.h"
#include "ltsmartlink.h"
#include "LTString.h"

#include <vector>
#include <string>

#include "GameBase.h"

class AINode;
class CAIVolume;
class CAIPath;
class CAIPathWaypoint;
class CAI;

// Use this to add the ability to link the node to other nodes or volumes.
#define ADD_NEIGHBOR_LINK(number, flags) \
	ADD_OBJECTPROP_FLAG_HELP(Neighbor##number, "", (flags), "AI can go to this point.")

#define ADD_BEZIER_LINK( flags ) \
	ADD_BOOLPROP_FLAG_HELP( UseBezier, LTFALSE, 0, "If true, the node will connect to the next node (like UseBaseName) and the AI will follow the bezier path.") \
	ADD_VECTORPROP_FLAG(BezierPrev, PF_BEZIERPREVTANGENT ) \
	ADD_VECTORPROP_FLAG(BezierNext, PF_BEZIERNEXTTANGENT )


class CAINode
{
	public:
		enum MicrosoftHackToGetConstantsInHeader
		{
			kInvalidID = 0xFFFFFFFF
		};

		// For Pathing
		typedef std::vector<uint32> NeighborIDList;
		typedef std::vector<CAINode*> NeighborList;

		typedef NeighborList::iterator NeighborIterator;
		typedef NeighborList::const_iterator const_NeighborIterator;

		typedef LTFLOAT CostType;

	public : // Public methods

		static void      SaveCAINode(ILTMessage & out, /*const*/ CAINode & pAINode);
		static CAINode * LoadCAINode(ILTMessage & in);

		CAINode(const char * name, LTBOOL wall_walk_only, uint32 dwID);
		CAINode(AINode & node, uint32 dwID);
		CAINode( const char * name, const LTVector & pos, 
				 const LTVector & right, const LTVector & up, const LTVector & forward, 
				 LTBOOL wall_walk_only, uint32 dwID );
		CAINode(const CAINode & srcNode, const LTVector & position);
		CAINode(HMESSAGEREAD hRead);
		virtual ~CAINode();
		
		// Used for type identity during load/save.
		// This _must_ be implemented per type for load/save to work!
		static const char * const szTypeName;
		virtual const char * GetTypeName() const { return CAINode::szTypeName; }

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		// Simple accessors

		uint32	GetID() const { return m_dwID; }

		const std::string & GetName() const { return m_strName; }

		const LTVector& GetPos() const { return m_vPos; }
		const LTVector& GetUp() const { return m_vUp; }
		const LTVector& GetForward() const { return m_vForward; }
		const LTVector& GetRight() const { return m_vRight; }

		const LTVector& GetDims() const { return m_vDims; }

		void  AddNeighbor(uint32 dwNeighborID);

		void	Lock(HOBJECT hOwner = LTNULL) { m_hLockOwner = hOwner; }
		void	Unlock() { m_hLockOwner = LTNULL; }
		LTBOOL	IsLocked() const { return (m_hLockOwner != LTNULL); }
		LTBOOL	IsOccupied(HOBJECT hOwner);
		HOBJECT	GetLockOwner() const { return m_hLockOwner; }

		void	SetActive(LTBOOL bActive)	{ m_bIsActive = bActive; }
		LTBOOL	IsActive() const			{ return m_bIsActive; }

		const LTSmartLink & GetTriggerObject() const { return m_hTriggerObject; }
		const std::string	& GetTriggerMsg() const	 { return m_strTriggerMsg; }
		const std::string	& GetSelfTriggerMsg() const { return m_strSelfTriggerMsg; }

		const LTSmartLink & GetAINodeGroup() const { return m_hAINodeGroup; }

		LTBOOL ConnectToVolume() const { return m_bConnectToVolume; }

		LTBOOL JumpTo(uint32 dwID) const;
		LTBOOL BezierTo(uint32 dwID) const;


		// Should be true for ladders only.
		virtual LTBOOL IsLadder() const { return LTFALSE; }


		// If false, the AI will not use the node unless it is a goal.
		// ( Things like snipe nodes and patrol nodes should return
		//   false. )
		virtual LTBOOL IsPathingNode() const { return LTTRUE; }

		LTBOOL IsWallWalkOnly() const { return m_bWallWalkOnly; }
		const LTVector & GetNormal() const { return m_vNormal; }

		LTBOOL  AllowAliens() const { return m_bAllowAliens; }
		LTBOOL  AllowMarines() const { return m_bAllowMarines; }
		LTBOOL  AllowPredators() const { return m_bAllowPredators; }
		LTBOOL  AllowCorporates() const { return m_bAllowCorporates; }

		 // If true, the pathing will use AI's GetCurrentVolume as a start point.
		virtual LTBOOL UseVolumePathing() const { return LTTRUE; }
		virtual const CAIVolume * GetContainingVolumePtr() const { return m_pContainingVolumePtr; }

		void SetContainingVolume(const CAIVolume & volume) { m_pContainingVolumePtr = &volume; }

#ifndef _FINAL
		virtual void DrawDebug(int level);
		virtual void ClearDebug();
#endif
		// For Pathing

		virtual LTFLOAT GenerateHeuristic(const LTVector & vDestination, LTFLOAT fImpassableCost) const  // Calculates the cost to move to that destination.
		{
			return ( GetPos() - vDestination ).Mag();
		}

		virtual LTFLOAT GenerateCost(const CAINode & dest_node, LTFLOAT fImpassableCost) const  // Calculates the cost to move to that destination.
		{
			return ( GetPos() - dest_node.GetPos() ).Mag();
		}

		// This should be called on the start and destination node with the current
		// path finding iteration (done in AIPathMgr).
		void ResetPathing(uint32 nPathingIteration );

		void  SetIsGoal(bool val) { m_bIsGoal = val; }
		void  SetCost(float val) { m_fCost = val; }
		void  SetInClosed()      { m_bInOpen = false; m_bInClosed = true;  }
		void  SetInOpen()        { m_bInOpen = true;  m_bInClosed = false; }
		void  SetHeuristic(float val) { m_fHeuristic = val; }
		void  SetPrevNode(CAINode * prevNode) { m_pPrevNode = prevNode; }

		bool  IsGoal() const { return m_bIsGoal; }
		float GetCost() const { return m_fCost; }
		float GetWeight()   const { return m_fCost + m_fHeuristic; }
		
		bool  InClosed() const { return m_bInClosed; }
		bool  InOpen()   const { return m_bInOpen;   }

		CAINode * GetPrevNode()             { return m_pPrevNode; }
		const CAINode * GetPrevNode() const { return m_pPrevNode; }

		virtual LTVector AddToPath(CAIPath * pPath, CAI * pAI, const LTVector & vStart, const LTVector & vDest) const;

		NeighborIterator BeginNeighbors();
		NeighborIterator EndNeighbors();

		const_NeighborIterator BeginNeighbors() const;
		const_NeighborIterator EndNeighbors() const;

		NeighborIDList::iterator BeginNeighborIDs() { return m_NeighborIDList.begin(); }
		NeighborIDList::iterator EndNeighborIDs()   { return m_NeighborIDList.end();   }


	protected :

		// This can be used to color code the node types.
#ifndef _FINAL
		virtual LTVector GetNodeColor() const { return LTVector(1,1,1); }
		virtual LTFLOAT  GetNodeAlpha() const { return 0.5f; }
#endif
		void SetPos(const LTVector & val) { m_vPos = val; }

		virtual void ResetPathingHook() {}  // Use this if you want a derivative to do something when being reset.

	private :

		void  InternalResetPathing();  // Does the actual reseting.
		void InternalLoad(HMESSAGEREAD hRead);

		uint32		m_dwID;						// Node ID
		std::string	m_strName;					// Node name

		LTVector	m_vPos;						// Node position
		LTVector	m_vUp;						// Node up vector
		LTVector	m_vForward;					// Node forward vector
		LTVector	m_vRight;					// Node right vector
		
		LTVector	m_vDims;					// The AINode's dimensions.

		LTBOOL		m_bConnectToVolume;			// Node should connect to volume containing it.

		const CAIVolume * m_pContainingVolumePtr;     // Set to the volume that contains if if m_bConnectToVolume is true.

		LTBOOL		m_bWallWalkOnly;			// Node can only be used by wall walkers.
		LTVector	m_vNormal;					// The normal of the poly node is associated with.

		NeighborIDList  m_JumpToID;			// If there is a jump connection from this node, the id of the node to jump to is stored here.

		NeighborIDList  m_BezierToID;			// If there is a Bezier connection from this node, the id of the node to Bezier to is stored here.
		LTVector	m_vBezierPrev;
		LTVector	m_vBezierNext;

		LTBOOL		m_bAllowAliens;
		LTBOOL		m_bAllowMarines;
		LTBOOL		m_bAllowPredators;
		LTBOOL		m_bAllowCorporates;

		LTSmartLink m_hLockOwner;				// who has the lock?  NULL means unlccked

		LTBOOL		m_bIsActive;				// If set to false, this node should not be used.

		LTSmartLink	m_hTriggerObject;
		std::string	m_strTriggerMsg;
		std::string	m_strSelfTriggerMsg;

		LTSmartLink	m_hAINodeGroup;				// each AINode may be assigned to 1 AINodeGroup.

#ifndef _FINAL
		LTSmartLink m_hDebugNodeBox;
#endif

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
		CAINode *	m_pPrevNode;
};


class AINode : public GameBase
{
	friend CAINode;

	public :

		struct NeighborLink
		{
			LTSmartLink hObject;
			bool		bConnectTo;
			bool		bConnectFrom;

			NeighborLink(const LTSmartLink & object = LTSmartLink(), bool connect_to = false, bool connect_from = false)
				: hObject(object),
				  bConnectTo(connect_to),
				  bConnectFrom(connect_from) {}
		};

		struct NeighborName
		{
			std::string	strNeighbor;
			bool		bTwoWay;

			NeighborName(const std::string & name, bool two_way = true)
				: strNeighbor(name),
				  bTwoWay(two_way) {}
		};

		typedef std::vector<NeighborName>  NeighborNameList;
		typedef std::vector<NeighborLink>  NeighborConnectList;

	public :
		
		AINode()
			: m_bStartActive(LTTRUE),
			  m_bWallWalkOnly(LTFALSE),
			  m_bMoveToFloor(LTFALSE),
			  m_bConnectToVolume(LTTRUE),
			  m_bUseBezier(LTFALSE),
			  m_vBezierPrev(0,0,0),
			  m_vBezierNext(0,0,0),
			  m_bAllowAliens(LTTRUE),
			  m_bAllowMarines(LTTRUE),
			  m_bAllowPredators(LTTRUE),
		 	  m_bAllowCorporates(LTTRUE),
			  m_vInitialPitchYawRoll(0.0f,0.0f,0.0f),
			  m_vDims(16,16,16),
			  m_nID(CAINode::kInvalidID) {}

		virtual ~AINode() {}

		virtual CAINode * MakeCAINode(uint32 dwID);

		void AddNeighborLink(const LTSmartLink & new_neighbor, bool bMakeCircular) { AddNeighborLink( NeighborLink(new_neighbor,true,bMakeCircular) ); }
		void AddJumpLink(const LTSmartLink & new_neighbor, bool bMakeCircular) { AddJumpLink( NeighborLink(new_neighbor,true,bMakeCircular) ); }
		void AddBezierLink(const LTSmartLink & new_neighbor, bool bMakeCircular) { AddBezierLink( NeighborLink(new_neighbor,true,bMakeCircular) ); }

		NeighborConnectList::const_iterator BeginNeighborLinks() const { return m_NeighborConnections.begin(); }
		NeighborConnectList::const_iterator EndNeighborLinks() const { return m_NeighborConnections.end(); }

		uint32	GetID() const { return m_nID; }

		virtual void PostStartWorld(uint8 nLoadGameFlags);

		uint32 EngineMessageFn(uint32 messageID, void *p, LTFLOAT f);

		virtual void ReadProp(ObjectCreateStruct *p);

		LTBOOL JumpTo(HOBJECT hNode) const;
		LTBOOL BezierTo(HOBJECT hNode) const;

	protected :

		virtual void FirstUpdate();
		virtual CAINode * CheckCAINode(CAINode * pNode);

		void SetWallWalk(LTBOOL val) { m_bWallWalkOnly = val; }

		void  AddNeighborLink(const NeighborLink & new_neighbor); 
		void  AddJumpLink(const NeighborLink & new_neighbor);
		void  AddBezierLink(const NeighborLink & new_neighbor);

	private :

		LTBOOL		m_bStartActive;
		LTBOOL		m_bWallWalkOnly;

		LTBOOL		m_bUseBezier;
		LTVector	m_vBezierPrev;
		LTVector	m_vBezierNext;

		LTBOOL		m_bAllowAliens;
		LTBOOL		m_bAllowMarines;
		LTBOOL		m_bAllowPredators;
		LTBOOL		m_bAllowCorporates;

		LTBOOL		m_bMoveToFloor;
		LTBOOL		m_bConnectToVolume;
		LTVector	m_vInitialPitchYawRoll;
		LTVector	m_vDims;

		std::string	m_strTriggerObject;
		std::string	m_strTriggerMsg;
		std::string	m_strSelfTriggerMsg;

		std::string	m_strAINodeGroup;

		uint32		m_nID;

		NeighborNameList m_NeighborNameList;
		NeighborConnectList m_NeighborConnections;

		NeighborNameList m_JumpToNameList;
		NeighborConnectList m_JumpToConnections;

		NeighborConnectList m_BezierToConnections;
};

#endif // _AI_NODE_H_


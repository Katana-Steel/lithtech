// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategyFollowPath.h
//
// PURPOSE : An AI strategy class 
//
// CREATED : 6.20.2000
//
// ----------------------------------------------------------------------- //




#ifndef __AI_STRATEGY_FOLLOW_PATH_H__
#define __AI_STRATEGY_FOLLOW_PATH_H__

#include "AIStrategy.h"
#include "AIStrategyMove.h"
#include "fsm.h"
#include "AIPath.h"

class CAINode;
class CAIVolume;
class CCharacter;

class CAIStrategyFollowPath : public CAIStrategy
{

	public :

		enum PathSetType
		{
			eNotSet,
			ePosition,
			eVolume,
			eNode,
			eCharacter,
			eSequintial
		};


	public : // Public methods

		// Ctors/Dtors/etc
		CAIStrategyFollowPath(CAI * pAI);
		virtual ~CAIStrategyFollowPath();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

		LTBOOL Set(const LTVector & vDestination);
		LTBOOL Set(const CAINode * pNodeDestination, LTBOOL bWarnIfFail = LTFALSE);
		LTBOOL Set(const CAIVolume * pVolumeDestination);
		LTBOOL Set(const CCharacter * pCharDestination);
		LTBOOL SetSequintial(CAIVolume * pCurrentVolume, CAIVolume * pPreviousVolume);

		LTBOOL Repath();

		void Update();

		void Clear();
		
		void ExactMovement(LTBOOL bVal) { m_bExactMovement = bVal; }

		// Simple accessors

		bool IsUnset() const { return m_RootFSM.GetState() == Root::eStateUnset; }
		bool IsSet() const { return m_RootFSM.GetState() == Root::eStateSet; }
		bool IsDone() const { return m_RootFSM.GetState() == Root::eStateDone; }
		bool IsStuck() const;

		const LTVector & GetWaypointPos() const;
		bool  HasValidWaypoint() const { return m_Waypoint.IsValid(); }
		const CAIPathWaypoint & GetWaypoint() const { return m_Waypoint; }

		LTVector GetDestinationPos() const;

	private : // Private enumerations

		struct Root
		{
			enum State
			{
				eStateSet,
				eStateUnset,
				eStateDone,
			};

			enum Message
			{
				eNewDestination,
				eInvalidPath,
				eReachedEndOfPath
			};

		};


		struct Path
		{
			enum State
			{
				eStateFindNextWaypoint,
				eStateMoveTo,
				eStateLadderTo,
				eStateWallWalkTo,
				eStateOpenDoor,
				eStateJumpTo,
				eStateWWJumpTo,
				eStateDrop,
				eStateBezierTo,
			};

			enum Message
			{
				eNewPath,
				eMoveToWaypoint,
				eLadderToWaypoint,
				eOpenDoorWaypoint,
				eWallWalkToWaypoint,
				eJumpToWaypoint,
				eWWJumpToWaypoint,
				eDrop,
				eBezierToWaypoint,
				eFinishedWaypoint,
			};
		};
		
		
	private :

		void SetupFSM();

	public :
		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;
		typedef FiniteStateMachine<Path::State,Path::Message> PathFSM;

	private :

		void	PrintRootFSMState(const RootFSM & fsm, Root::State state);
		void    PrintRootFSMMessage(const RootFSM & fsm, Root::Message message);

		void	PrintPathFSMState(const PathFSM & fsm, Path::State state);
		void    PrintPathFSMMessage(const PathFSM & fsm, Path::Message message);

		// Callbacks for the finite state machine
		void  UpdatePath(RootFSM * rootFSM);
		void  StartPath(RootFSM * rootFSM);
		void  EndPath(RootFSM * rootFSM);

		void  StartDone(RootFSM * pathFSM);

		void  StartNewPath(PathFSM * pathFSM);
		void  UpdateFindNextWaypoint(PathFSM * pathFSM);

		void  StartMoveTo(PathFSM * pathFSM);
		void  UpdateMoveTo(PathFSM * pathFSM);

		void  StartLadderTo(PathFSM * pathFSM);
		void  UpdateLadderTo(PathFSM * pathFSM);
		void  EndLadderTo(PathFSM * pathFSM);

		void  UpdateWallWalkTo(PathFSM * pathFSM);
		void  StartWallWalkTo(PathFSM * pathFSM);

		void  UpdateOpenDoor(PathFSM * pathFSM);

		void  UpdateJumpTo(PathFSM * pathFSM);
		void  StartJumpTo(PathFSM * pathFSM);

		void  UpdateWWJumpTo(PathFSM * pathFSM);
		void  StartWWJumpTo(PathFSM * pathFSM);

		void  UpdateDrop(PathFSM * pathFSM);
		void  StartDrop(PathFSM * pathFSM);

		void  UpdateBezierTo(PathFSM * pathFSM);
		void  StartBezierTo(PathFSM * pathFSM);

		void  RecordReachedWaypoint();  // This should be called whenever a waypoint is reached.
		
		RootFSM					m_RootFSM;
		PathFSM					m_PathFSM;

		LTBOOL					m_bExactMovement;

		bool					m_bShowingPath;

		CAIPath*				m_pCurrentPath;  // Points to the current path we are using, either m_PathA or m_PathB.
		CAIPath					m_PathA;
		CAIPath					m_PathB;         // This allows us to do test path without interfering with the current path.
		CAIPathWaypoint         m_Waypoint;

		uint32					m_nDestNodeID;    // Set to node id if Set(CAINode*) is used, set to CAINode::kInvalidID otherwise.
		uint32					m_nDestVolumeID;  // Set to volume id if Set(CAIVolume*) is used, set to CAIVolume::kInvalidID otherwise.

		PathSetType				m_eSetType;
};

#endif //__AI_STRATEGY_FOLLOW_PATH_H__

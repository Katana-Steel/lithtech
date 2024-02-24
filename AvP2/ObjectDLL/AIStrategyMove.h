// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategyMove.h  (was AIMovement.h)
//
// PURPOSE : 
//
// CREATED : 6.20.2000
//
// ----------------------------------------------------------------------- //

#ifndef __AI_STRATEGY_MOVE_H__
#define __AI_STRATEGY_MOVE_H__


#include "AIStrategy.h"

#include "fsm.h"
#include "ltvector.h"
#include "ltsmartlink.h"
#include "Timer.h"
#include "AIButeMgr.h"
#include <vector>

class CAI;
class ObstacleMgr;
class DebugLineSystem;
class CharacterMovement;
class CAIVolume;
class CAINode;
class AIAction;

struct AIBM_Movement;

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIStrategyMove
//
// PURPOSE : AI Movement manager
//
// ----------------------------------------------------------------------- //

class CAIStrategyMove : public CAIStrategy
{
	public :
	
		enum FacingType
		{
			None,
			Direction,
			Object,
			Target
		};

	public : // Public methods

		// Ctors/Dtors/etc
		CAIStrategyMove(CAI * pAI);
		~CAIStrategyMove();

		// Overloaded Methods.
		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		void Update();


		// Methods
		
		void Set(const LTVector& vDest, LTBOOL bExactMovement, const CAIVolume * pDestVolume, const CAINode * pDestNode);
		void Set(const LTVector& vDest, const LTVector & vNextDest, LTBOOL bExactMovement, const CAIVolume * pDestVolume, const CAINode * pDestNode);
		void SetWallWalk(const LTVector & vDest, const LTVector & vNormal, const CAINode * pDestNode );

		void SetExactMovement(bool bExact) { m_bExactMovement = bExact; }
		void Clear();
		

		// Simple accessors

		bool  IsSet()           const;
		bool  IsDone()          const;
		bool  IsStuck()			const;

		LTBOOL  IsAvoidingIgnorable() const;

		void  SetPathingFrequency(LTFLOAT val);

	private :

		struct Root
		{
			enum State
			{
				eStateMoveTo,
				eStateWallWalkTo,
				eStateStuck,
				eStateDone,
				eStateMoveToCorrection,
				eStateWallWalkCorrection
			};

			enum Message
			{
				eNewDest,
				eNewWWDest,
				eReachedDest,
				eNotMoving,

				// These go immediately to a state.
				eStuck,
				eClear
			};
		};

		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;

		void SetupFSM();

		void PrintRootFSMState(const RootFSM & , Root::State );
		void PrintRootFSMMessage(const RootFSM & , Root::Message );
	
		void StartMoveTo(RootFSM * fsm);
		void UpdateMoveTo(RootFSM * fsm);

		void StartWallWalkTo(RootFSM * fsm);
		void UpdateWallWalkTo(RootFSM * fsm);

		void StopMoving(RootFSM * fsm);

		void StartCorrection(RootFSM * fsm);
		void UpdateCorrection(RootFSM * fsm);
		
		void StartWWCorrection(RootFSM * fsm);
		void UpdateWWCorrection(RootFSM * fsm);
		void EndWWCorrection(RootFSM * fsm);

		void StartMoveToCorrection(RootFSM * fsm);
		void UpdateMoveToCorrection(RootFSM * fsm);

		void StartStuck(RootFSM * fsm);
		void UpdateStuck(RootFSM * fsm);

		const CharacterMovement * m_pMovement;
		ObstacleMgr	* m_pObstacleMgr;

		RootFSM		m_RootFSM;	


		LTVector	m_vDest;
		LTVector	m_vNextDest;
		LTBOOL		m_bExactMovement;
		LTVector	m_vNormal;
		const CAINode * m_pDestNode;
		const CAIVolume * m_pDestVolume;
		bool		m_bCheckNextDest;
		

		bool		m_bMovedLaterally; // Used to determine if stuck.
		bool		m_bAvoidingObstacle;
		bool		m_bMightOvershoot;
		bool		m_bFitsInVolume;
		LTVector	m_vLastPosition;
		LTFLOAT		m_fNextCheckTime;
		LTVector	m_vStartPosition;
		LTFLOAT		m_fMoveToTime;

		CTimer		m_tmrMaxStuckDuration;
		CTimer		m_tmrMinAvoidIgnoreableDelay;

		uint32		m_nNumCorrections;
		CTimer		m_tmrCorrection;
		LTVector	m_vCorrectionDir;

		const AIAction *  m_pLastAction;  // Note that this may not be a valid pointer!
		LTVector    m_vLastActionGoal;
		LTVector	m_vLastActionPos;
		LTVector	m_vStartingPos;

		// For wall walking.
		LTVector	m_vLastUsedUp;
		LTVector	m_vFacingDir;

		AIBM_Movement m_Butes;
};


#endif //__AI_STRATEGY_MOVE_H__

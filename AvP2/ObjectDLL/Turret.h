// ----------------------------------------------------------------------- //
//
// MODULE  : Turret.h
//
// PURPOSE : Implements an object to be used as a stationary gun turret.
//
// CREATED : 2/14/00
//
// ----------------------------------------------------------------------- //

#ifndef __TURRET_H__
#define __TURRET_H__


#include "fsm.h"
#include "Timer.h"
#include "Prop.h"
#include "ltsmartlink.h"
#include "TurretSense.h"
#include "Weapons.h"
#include "HierarchicalButeMgr.h"

#include <string>

class CCharacter;

class Turret : public Prop
{
	public :
		enum ObjectiveType
		{
			Ignore = -1,
			Interest = 0,
			Target,
			Prefer,
			Override
		};


	public :

		Turret();
		virtual ~Turret();

		void   Destroyed(LTBOOL val = DTRUE);
		
		LTFLOAT  GetMinPitch() const { return m_fMinPitch; }
		LTFLOAT  GetMaxPitch() const { return m_fMaxPitch; }
		LTFLOAT  GetMinYaw() const { return m_fMinYaw; }
		LTFLOAT  GetMaxYaw() const { return m_fMaxYaw; }
		LTFLOAT  GetVisualRange() const { return m_fVisualRange; }
		LTVector GetUpVector() const { return m_vInitialUp; }
		LTVector GetRightVector() const { return m_vInitialRight; }



	protected :

		virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);


		LTBOOL	ReadProp(ObjectCreateStruct *pData);
		void    ReadButes(const char * tag_name);
		void	PostPropRead(ObjectCreateStruct* pData);
		LTBOOL	InitialUpdate();

		LTBOOL	Update();

		CTurretVision	m_TurretVision;
		CWeapons		m_GunMgr;

		
		// For the finite state machines.
		struct Root
		{
			enum State
			{
				eStateIdle,
				eStateSweep,
				eStateTrack,
				eStateFire,
				eStateBeginClosing,
				eStateClosing,
				eStateClosed,
				eStateOpening,
				eStateDestroyed
			};

			enum Message
			{
				eCompleted,

				eIdleTimedOut,

				eNewInterestDetected,
				eNewTargetDetected,

				eObjectiveGone,

				eTurnOff,
				eTurnOn,

				eSleep,

				eDestroyed
			};

		};
		
		struct Sweep
		{
			enum State
			{
				eStateTurningToMin,
				eStateTurningToMax,
				eStatePausingAtMin,
				eStatePausingAtMax
			};
		
			enum Message
			{
				eComplete
			};
		};


	private :

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

		void	SetupFSM();

		void	FirstUpdate();

		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;
		typedef FiniteStateMachine<Sweep::State,Sweep::Message> SweepFSM;

		// Callbacks for the finite state machine
		void	StartIdleTimer(RootFSM * fsm);
		void	DoIdleTimer(RootFSM * fsm);

		void	DoSweep(RootFSM * fsm);

		void	DoDestroyed(RootFSM * fsm);

		void	StartTrack(RootFSM * fsm);
		void	DoTrack(RootFSM * fsm);
		void	EndTrack(RootFSM * fsm);

		void	StartFire(RootFSM * fsm);

		void	StartOpening(RootFSM * fsm);
		void	DoOpening(RootFSM * fsm);

		void	DoBeginClosing(RootFSM * fsm);

		void	StartClosing(RootFSM * fsm);
		void	DoClosing(RootFSM * fsm);

		void	StartClosed(RootFSM * fsm);
		void	EndClosed(RootFSM * fsm);

		void	DoTurningToMin(SweepFSM * fsm);
		void	DoTurningToMax(SweepFSM * fsm);
		void    StartSweepPausing(SweepFSM * fsm);
		void	DoSweepPausing(SweepFSM * fsm);

		
		// utility functions
		void	SeeCharacter(const CTurretVision & sense, const CCharacter * potential_target);
		LTBOOL	TurnTo(LTFLOAT desired_pitch, LTFLOAT desired_yaw, LTFLOAT pitch_speed, LTFLOAT yaw_speed, LTBOOL pop_to_destination = DFALSE);
		void	FireWeapon();

		Root::State GetRootState(const char * state_description);
		Root::State GetIdleRootState(const char * state_description);

		void    PrintRootFSMMessage(const RootFSM & fsm, Root::Message message);
		void	PrintRootFSMState(const RootFSM & fsm, Root::State state);

	private :

		RootFSM		m_RootFSM;	
		SweepFSM	m_SweepFSM;

		// Bute file properties.
		std::string m_strGunFilename;
		std::string m_strGunSkin;
		std::string m_strDeadGunFilename;
		std::string m_strDeadGunSkin;
		LTFLOAT  m_fGunMaxHitPoints;

		std::string m_strGunBaseSocket;
		std::string m_strTurretBaseSocket;

		std::string m_strAwakeSound;
		std::string m_strDetectSound;
		std::string m_strMoveSound;

		LTFLOAT		m_fWarningSoundDistance;
		LTFLOAT		m_fMovingSoundDistance;

		uint8		m_nWeaponId;
		LTFLOAT		m_fBurstDelay;
		LTFLOAT		m_fDamageMultiplier;

		uint8		m_nStartFireSound;
		uint8		m_nLoopFireSound;
		uint8		m_nStopFireSound;

		LTFLOAT	m_fYawSpeed;
		LTFLOAT	m_fPitchSpeed;
		LTFLOAT  m_fSweepYawSpeed;

		LTFLOAT  m_fVisualRange;

		LTBOOL	m_bStartOff;

		// Constant data derived at initialization or read from properties.
		LTBOOL		m_bHasGunModel;   // If false, m_hGunHandle is set to m_hObject.
		LTBOOL		m_bAttachGun;
		LTSmartLink m_hGunHandle;
		LTVector    m_vGunOffset;
		HATTACHMENT m_hGunAttachment; // Is non-null if the gun is attached to the base.
		LTVector    m_vAimingOffset;  // Aim from the center of the gun plus this offset.

		LTBOOL	m_bUseAwakeSound;
		LTBOOL  m_bUseDetectSound;

		LTFLOAT m_fMinYaw;
		LTFLOAT	m_fMaxYaw;

		LTFLOAT	m_fMinPitch;
		LTFLOAT	m_fMaxPitch;
		
		LTFLOAT	m_fSweepPauseTime;

		LTVector	m_vInitialPosition;
		LTRotation	m_rInitialRotation;
		LTVector	m_vInitialAngles;
		LTVector	m_vInitialUp;
		LTVector	m_vInitialRight;

		LTFLOAT  m_fMaxIdleTime;

		ObjectiveType m_aClassTypes[5]; // List of objective type for each class

		std::string m_OpeningAnimName;
		std::string m_ClosingAnimName;
		std::string m_OpenedAnimName;
		std::string m_ClosedAnimName;

		HMODELANIM m_hOpeningAni;
		HMODELANIM m_hClosingAni;
		HMODELANIM m_hOpenedAni;
		HMODELANIM m_hClosedAni;

		// Variables.
		LTSmartLink		m_hObjective;       // Our current target.
		const CCharacter * m_pCharObjective;
		LTVector		m_vObjectiveOffset;  // The offset from the target's position to shoot at.
		ObjectiveType	m_ObjectiveType;     // The type of the objective.

		HLTSOUND  m_hWarningSound;
		HLTSOUND  m_hMovingSound;
		HLTSOUND  m_hFiringSound;

		LTBOOL	m_bFirstUpdate;
		LTBOOL	m_bOn;
		
		LTFLOAT	m_fYaw;
		LTFLOAT  m_fPitch;

		LTVector m_vRight;  // Our current rotation.
		LTVector m_vUp;
		LTVector m_vForward;

		LTBOOL	m_bFiring;
		LTBOOL  m_bFiredWeapon;

		LTBOOL  m_bMoving;

		CTimer  m_tmrIdle;
		CTimer  m_tmrPause;
		CTimer	m_tmrBurstDelay;

		LTBOOL	m_bCanDamage;
};



class TurretWeapon : public Prop
{
	public :

		TurretWeapon() 
			: m_pOwner(DNULL) {}
		virtual ~TurretWeapon() {}

		void   Init( Turret * owner ) { m_pOwner = owner; }

	protected :

		virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual uint32	ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	private :
		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

		Turret * m_pOwner;
};


#endif // __TURRET_H__

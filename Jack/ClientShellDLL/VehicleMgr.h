// ----------------------------------------------------------------------- //
//
// MODULE  : CVehicleMgr.cpp
//
// PURPOSE : Client side vehicle movement mgr - Definition
//
// CREATED : 6/12/00
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VEHICLE_MGR_H__
#define __VEHICLE_MGR_H__

#include "iltclient.h"
#include "SharedMovement.h"
#include "CameraOffsetMgr.h"
#include "Timer.h"
#include "SurfaceDefs.h"
#include "LTObjRef.h"
#include "VehicleWeapon.h"

struct VEHICLE;

struct VEHICLENODE
{
	VEHICLENODE()
	{
		hNode = INVALID_MODEL_NODE;
		matTransform.Identity();
	}

	HMODELNODE	hNode;
	LTMatrix	matTransform;
};

class CVehicleMgr
{
  public:

	CVehicleMgr();
	~CVehicleMgr();

    LTBOOL      Init();

	void		UpdateModels();

    LTFLOAT		GetVehicleMovementPercent() const;

	void		OnEnterWorld();
	void		OnExitWorld();
	bool		OnModelKey( HLOCALOBJ hObj, ArgList *pArgs );

	void		OnTouchNotify(CollisionInfo *pInfo, float forceMag);

    uint32      GetControlFlags()	const { return m_dwControlFlags; }
 
	bool		CanShowCrosshair()	const;
	LTBOOL      IsVehiclePhysics();
	LTBOOL		CanDismount();
	LTBOOL      IsTurning()				const { return (m_bVehicleTurning && !m_bVehicleStopped); }
	int			GetTurnDirection()		const { return m_nVehicleTurnDirection; }
	bool		AllowHeadBobCant()		const;

	PlayerPhysicsModel GetPhysicsModel()	const { return m_ePPhysicsModel; }

	void		CalculateVehicleRotation(LTVector & vPlayerPYR, LTVector & vPYR, float fYawDelta, float fPitchDelta );

	// Allows vehicles to perterb the camera placement.
	void		CalculateVehicleCameraDisplacment( LTVector& vDisplacement );

	void		GetVehicleLightPosRot(LTVector & vPos, LTRotation & rRot);
	void		HandleNodeControl(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat);

	void		InitWorldData();

	void		UpdateControlFlags();
	void		UpdateMotion();
	void		UpdateFriction();
	void		UpdateSound();

 	void		TermLevel();
	
	CCameraOffsetMgr* GetModelOffsetMgr() { return &m_VehicleModelOffsetMgr; }

	void		SetPhysicsModel( PlayerPhysicsModel eModel, uint8 nModelId = (uint8)-1, LTBOOL bDoPreSet = LTTRUE );

	void		AdjustCameraRoll(LTFLOAT & fRoll);

	void		SetPlayerLureId( uint32 nPlayerLureId );
	uint32		GetPlayerLureId( ) { return m_nPlayerLureId; }

	float		GetVehicleContourRoll() const { return IsVehicleModel( m_ePPhysicsModel ) ? m_fContourRoll : 0.0f; }

	// Called by vehiclemgr.
	bool		MoveLocalSolidObject( );

	// Command handling functions
	LTBOOL		OnCommandOn(int command);
	LTBOOL		OnCommandOff(int command);
	void		AlignAngleToAxis(float &fAngle);
	void		SnapLightCycleToXZGrid(LTVector &vNewPos, LTVector &vOldPos, uint32 nGridSize);
	
	void		MoveVehicleObject( LTVector &vPos );

	// Serialization
	void		Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState);
	void		Load(ILTMessage_Read *pMsg, SaveDataState eLoadDataState);

	// Is it safe to save?
	bool		CanSave() const { return m_bHaveJumpVolumeVel == LTFALSE; }

	LTFLOAT     GetMaxVelMag(LTFLOAT fMult=1.0f) const;
    LTFLOAT     GetMaxAccelMag(LTFLOAT fMult=1.0f) const;

	void		SaveVehicleInfo();

	bool		HasVehicleWeapon( ) const { return m_bHasWeapon; }
	const CVehicleWeapon* GetVehicleWeapon() const { return	&m_VehicleWeapon; }

	// Weapon update function
	WeaponState	UpdateVehicleWeapon( LTRotation const &rRot, LTVector const &vPos,
									bool bFire, FireType eFireType = FT_NORMAL_FIRE );

  protected:

	void		UpdateGear();
	
	const char*	GetIdleSnd();
	const char*	GetBrakeSnd();
	const char*	GetAccelSnd();
	const char*	GetDecelSnd();
	const char*	GetImpactSnd(LTFLOAT fCurVelocityPercent, SURFACE* pSurface);
	
	LTFLOAT		GetAccelTime() const;
	LTFLOAT		GetMinTurnAccel()const;
	LTFLOAT		GetMaxDecel() const;
	LTFLOAT		GetDecelTime() const;
	LTFLOAT		GetMaxBrake() const;
	LTFLOAT		GetBrakeTime()const;

	void		UpdateVehicleMotion();
	void		UpdateVehicleSounds();
	void		UpdateVehicleControlFlags();
	void		UpdateVehicleHandleBars();

	void		UpdateVehicleGear();
	
	void		UpdateLureControlFlags();
	void		UpdateLureMotion();

	void        KillAllVehicleSounds(HLTSOUND hException=LTNULL);
	
	void		PlayVehicleAni(char* pAniName, LTBOOL bReset=LTTRUE, LTBOOL bLoop=LTFALSE);
	LTBOOL		IsCurVehicleAni(char* pAniName, LTBOOL & bIsDone);

	LTBOOL		PreSetPhysicsModel(PlayerPhysicsModel eModel);
			
	void		SetVehiclePhysicsModel( );
	void		SetPlayerLurePhysicsModel( );
	void		SetNormalPhysicsModel( );

	void		CreateVehicleModel();
	void		UpdateVehicleModel();
	void		UpdateVehicleFriction(LTFLOAT fSlideToStopTime);

	void		ShowVehicleModel(LTBOOL bShow=LTTRUE);

	LTFLOAT		GetSurfaceVelMult(SurfaceType eType) const;

	void		CalculateBankingVehicleRotation(LTVector & vPlayerPYR, LTVector & vPYR, LTFLOAT fYawDelta);
	void		CalculateLureVehicleRotation(LTVector& vPlayerPYR, LTVector& vPYR, float fYawDelta, float fPitchDelta );
	void		CalculateVehicleContourRotation( LTVector &vPlayerPYR, LTVector &vPYR );
	void		CalculateLightCycleRotation(LTVector & vPlayerPYR, LTVector & vPYR);
		
	bool		MoveToLure( );
	bool		MoveLightCycle();

	bool		HandleJumpVolumeTouch( HOBJECT hJumpVolume );
	bool		HandleCharacterTouch( HOBJECT hCharacter );

	void		UpdateInJumpVolume();
	void		EnableWeapon( bool bEnable, bool bToggleHolster = true );
	
	void		UpdateContainerMotion();
	
	void		HandleCollision();

  protected :

	HOBJECT		m_hVehicleModel;
    LTFLOAT		m_fAccelStart;
    LTFLOAT     m_fDecelStart;

	// Movement state.
    uint32      m_dwControlFlags;
    uint32      m_dwLastControlFlags;
	bool		m_bLastBrake;

	float		m_fVehicleBaseMoveAccel;

	PlayerPhysicsModel m_ePPhysicsModel;

    HLTSOUND        m_hVehicleStartSnd;
    HLTSOUND        m_hVehicleAccelSnd;
    HLTSOUND        m_hVehicleDecelSnd;
    HLTSOUND        m_hVehicleCruiseSnd;
	HLTSOUND		m_hVehicleImpactSnd;
	HLTSOUND		m_hIdleSnd;
	HLTSOUND		m_hBrakeSnd;

	VEHICLENODE		m_VehicleNode1;
	VEHICLENODE		m_VehicleNode2;
	VEHICLENODE		m_VehicleNode3;

	HMODELSOCKET	m_hVehicleAttachSocket;

	LTVector		m_vVehicleOffset;
	LTVector		m_vHeadOffset;

	int				m_nCurGear;
	int				m_nLastGear;
	bool			m_bTurned;
	bool			m_bHolsteredWeapon;

    LTBOOL			m_bVehicleStopped;
    LTBOOL          m_bVehicleAtMaxSpeed;
    LTBOOL			m_bVehicleTurning;          // Are we turning (vehicle mode)
	int				m_nVehicleTurnDirection;	// Direction we are turning
 
	CCameraOffsetMgr	m_VehicleModelOffsetMgr; // Adjust vehicle model orientation

	LTVector		m_vVehiclePYR;

	float			m_fContourRoll;

	SurfaceType		m_eSurface;
	SurfaceType		m_eLastSurface;

	LTFLOAT			m_fYawDiff;
	LTFLOAT			m_fYawDelta;	
	LTBOOL			m_bKeyboardTurning;
	LTFLOAT			m_fHandlebarRoll;
	LTFLOAT			m_fHeadYaw;

	enum TurnDirection { TD_LEFT, TD_CENTER, TD_RIGHT };
	TurnDirection	m_eMouseTurnDirection;

	uint32			m_nPlayerLureId;
	CTimer			m_LureTimeToNextBump;
	CTimer			m_LureTimeOverBump;
	float			m_fLureBumpHeight;
	float			m_fLureBobParameter;

	bool			m_bResetLure;
	bool			m_bResetLureFromSave;
	LTransform		m_ResetLureTransform;

	LTObjRef		m_hCurJumpVolume;

	LTBOOL			m_bHaveJumpVolumeVel;
	float			m_fJumpVolumeVel;

	bool			m_bSetLastAngles;
	LTVector		m_vLastVehiclePYR;
	LTVector		m_vLastPlayerAngles;
	LTVector		m_vLastCameraAngles;

	bool			m_bWasTouched;
	CollisionInfo	m_cLastTouchInfo;

	VEHICLE			*m_pVehicle;

	CVehicleWeapon	m_VehicleWeapon;
	bool			m_bHasWeapon;
};


#endif  // __VEHICLE_MGR_H__
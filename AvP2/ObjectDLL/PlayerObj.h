// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerObj.h
//
// PURPOSE : Player object definition
//
// CREATED : 9/18/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CPLAYER_OBJ_H__
#define __CPLAYER_OBJ_H__

#include "Character.h"
#include "SharedMovement.h"

#define DEFAULT_PLAYERNAME		"Player"
#define NET_NAME_LENGTH			30
#define NUM_HISTORY				20

class CPlayerAttachments;
class CWeapon;

typedef struct tagTimeStamp
{
	tagTimeStamp()
	{
		fTime	= 0.0f;
		vPos	= LTVector(0,0,0);
	}

	LTFLOAT		fTime;
	LTVector	vPos;
} HistoryStamp, *pHistoryStamp;

class CPlayerObj : public CCharacter
{
public :
	
	CPlayerObj();
	~CPlayerObj();

	// ------------------------------------------------------------------ //
	// General utility functions

	void	Respawn(uint8 nServerLoadGameFlags = LOAD_NEW_LEVEL);
	void	MPRespawn(uint8 nCharId);
	void	SendMPInterfaceUpdate(HMESSAGEWRITE hMessage);
	void	Teleport(LTVector &vPos);
	void	SetClientSaveData(HMESSAGEREAD hClientData) { m_hClientSaveData = hClientData; }
	void	BuildKeepAlives(ObjectList* pList);
	LTBOOL	ShouldDoAutoSave()	{ return (m_bDoAutoSave && m_nSaveCount++ == 2)?LTTRUE:LTFALSE; }
	void	ResetDoAutoSave()	{ m_nSaveCount=0; }

	void	StartLevel();
	void	CreateAttachments();
	void	ChangeWeapon(uint8 nCommandId, LTBOOL bAuto = LTFALSE, LTBOOL bPlaySelectSound = LTTRUE);
	void	ChangeWeapon(const char *szWeapon, LTBOOL bDeselect = LTTRUE);

	void	DoWeaponChange(uint8 nWeaponId);
	void	TeleportClientToServerPos(LTFLOAT fTime = 0.0f);
	void	HandleEMPEffect();
	void	UpdateInterface(LTBOOL bForceUpdate = LTFALSE, LTBOOL bGuaranteed = LTFALSE); 
	int		AquireDefaultWeapon(LTBOOL bChangeWeapon=LTTRUE);
	void	ResetHealthAndArmor(LTBOOL bReset=LTTRUE)		{ m_bResetHealthAndArmor = bReset; }
	void	ResetPlayerHealth();
	LTBOOL	IsResetHealthAndArmor()					{ return m_bResetHealthAndArmor; }
	void	SendScaleInformation()					{ m_PStateChangeFlags |= PSTATE_SCALE; UpdateClientPhysics(); }
	void	SendButesAndScaleInformation()			{ m_PStateChangeFlags |= PSTATE_ATTRIBUTES | PSTATE_SCALE; UpdateClientPhysics(); }
	LTBOOL	ValidatePosition(LTVector& vPos, LTFLOAT& fTime);

	void	Swap(LTBOOL bChangeToAIPlayer);
	LTBOOL   IsSwapped() const { return m_bSwapped; }

	LTBOOL	IsPlayingLoopedSound() { return m_bPlayingLoopFireSound; }
	void	SetPlayingLoopFireSound(LTBOOL bOn) { m_bPlayingLoopFireSound = bOn; }

	// ------------------------------------------------------------------ //
	// Set/Get Toggle utility functions

	LTBOOL	InGodMode() const						{ return m_bGodMode; }
	void	ToggleGodMode(LTBOOL bOn, LTFLOAT fMinHealth, LTBOOL bSound = LTFALSE, LTBOOL bLeaveSolid = LTFALSE);

	HCLIENT GetClient() const						{ return m_hClient; }
	void	SetClient(HCLIENT hClient)				{ m_hClient = hClient; }

	void	SetCharacter(int nCharacterSet, LTBOOL bReset = LTTRUE);

	void	ChangeState(PlayerState eNewState, HOBJECT hSwapObject = LTNULL);
	PlayerState GetState()							const { return m_eState; }
	CharacterClass GetCharacterClass()				const { return m_cc; }
	WEAPON*	GetPlayerWeapon();

	// ------------------------------------------------------------------ //
	// Cheat handling functions

	void FullAmmoCheat();
	void FullWeaponCheat();
	void FullModsCheat();
	void FullGearCheat();
	void RepairArmorCheat();
	void HealCheat();
	void AttachDefaultHead();

	// ------------------------------------------------------------------ //
	// Message handling functions
	
	void	HandleClientMsg(HMESSAGEREAD hRead);
	void	HandleMessage_UpdateClient(HMESSAGEREAD hRead);
	void	HandleMessage_Activate(HMESSAGEREAD hRead);
	void	HandleMessage_WeaponFire(HMESSAGEREAD hRead);
	void	HandleMessage_PounceJump(HMESSAGEREAD hRead);
	void	HandleMessage_WeaponHit(HMESSAGEREAD hRead);
	void	HandleMessage_WeaponSound(HMESSAGEREAD hRead);
	void	HandleMessage_WeaponLoopSound(HMESSAGEREAD hRead);

	LTBOOL	HandleMusicMessage(const char* szMsg);
	LTBOOL	HandleCreditsMessage(const char* szMsg);

	virtual LTBOOL IgnoreDamageMsg(const DamageStruct& damage);

	void	SendFXButes(HCLIENT hClient);

	LTBOOL	IsWaitingForAutoSave()	{ return m_bWaitingForAutoSave; }

	// ------------------------------------------------------------------------------------------ //
	// Getting and setting character information

	virtual void	ResetCharacter();


	// ------------------------------------------------------------------------------------------ //
	// Hit node damage modifiers (overloaded from CCharacter).

	virtual LTFLOAT	ComputeDamageModifier(ModelNode eModelNode) const;
	
	// ------------------------------------------------------------------ //

	LTBOOL	PlayingCinematic(LTBOOL bStopCinematic=LTFALSE);

	// ------------------------------------------------------------------------------------------ //
	// Animation functions

	void	UpdateAnimation();
	WEAPON*	UpdateWeaponAnimation();
	void	UpdateExtWeaponAnimation();
	void	UpdateActionAnimation();

	// ------------------------------------------------------------------------------------------ //
	// Observation mode control functions

	void	SetObservePoint_Prev();
	void	SetObservePoint_Next();
	void	SetObservePoint_Random();
	void	SetObservePoint_Current();

	void	SetObservePoint(int &nPoint, LTBOOL bRandom);

	int		GetObservePoint()					{ return m_nObservePoint; }

	void	HandleExosuitEject(LTBOOL bCreatePickup = LTTRUE);
	void	DoAutoSave();

	uint8	IncMoveCode() { return ++m_ClientMoveCode;}
	void	TeleFragObjects(LTVector & vPos);

private :

	uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
	uint32	ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	void	Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
	void	Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

	void	WeaponCheat(uint8 nWeaponId);
	void	Reset();
	void	StartDeath();
	void	HandleDead(LTBOOL bRemoveObj);
	void	ProcessDamageMsg(const DamageStruct & damage_msg);
	LTBOOL	ProcessCommand(const char * const * pTokens, int nArgs );
	LTBOOL	Activate(LTVector vPos, LTVector vDir);
	void	PostPropRead(ObjectCreateStruct *pStruct);
	LTBOOL	InitialUpdate(int nInfo);
	LTBOOL	Update();
	void	UpdateClientPhysics(LTBOOL bRestore=LTFALSE);	
	void	ProcessAnimationMsg(uint8 nTracker, uint8 bLooping, uint8 nMsg);
	void	SetForceUpdateList(ForceUpdate* pFU);
	void	UpdateClientViewPos();
	void	UpdateConsoleVars();
	void	Teleport(const char* pTeleportPointName, LTBOOL bRotate = LTTRUE);
	void	SetLeashLen(float fVal)								{ m_fLeashLen = fVal; }
	void	UpdateHistory();

	void	HandleGameRestore();

	void	HandleEnergySift();						
	void	HandleMedicomp();										
	void	HandleExosuitEntry(HOBJECT hSender);
	HOBJECT CreateExosuitHead(LTBOOL bDefault = LTFALSE);
	void	CacheMultispawners();
	void	CacheFiles(int nCharSet);

	
	// ------------------------------------------------------------------ //
	// Data members
	
	LTFLOAT		m_fOldHitPts;
	LTFLOAT		m_fOldMaxHitPts;
	LTFLOAT		m_fOldMaxArmor;
	LTFLOAT		m_fOldArmor;
	LTBOOL		m_bDoAutoSave;
	uint8		m_nSaveCount;
	
	// Used to determine our alignment.
	CharacterClass m_cc;
	
	// When we respawn, this is incremented so we ignore packets from an older teleport.
	uint8		m_ClientMoveCode;
	
	PlayerState m_eState;
	LTBOOL		m_bSpawnInState;
	
	LTBOOL		m_b3rdPersonView;
	uint32		m_nSavedFlags;
	
	// Info about cheats...
	LTBOOL		m_bGodMode;
	LTBOOL		m_bAllowInput;

	uint16		m_nClientChangeFlags;

	HMESSAGEREAD	m_hClientSaveData;

	HSTRING			m_hstrStartLevelTriggerTarget;
	HSTRING			m_hstrStartLevelTriggerMessage;

	HSTRING			m_hstrStartMissionTriggerTarget;
	HSTRING			m_hstrStartMissionTriggerMessage;

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...
	CPlayerAttachments*		m_pPlayerAttachments;	// Our attachments aggregate

	LTBOOL		m_bFirstUpdate;

	uint32		m_PStateChangeFlags;		// Which things need to get sent to client.

	int*		m_pnOldAmmo;

	HCLIENT		m_hClient;

	int			m_nObservePoint;			// The current observation point


	// Data members used in load/save...

	uint32		m_dwLastLoadFlags;


	// Weapon status from client

	uint8		m_nWeaponStatus;

	float		m_fLeashLen;

	LTBOOL		m_bLevelStarted;
	LTBOOL		m_bWaitingForAutoSave;

	LTBOOL		m_bSwapped;
	LTSmartLink m_hAIPlayer;

	LTFLOAT		m_fOldBatteryLevel;
	LTFLOAT		m_fIdleTime;

	LTVector	m_vSpawnPosition;
	LTFLOAT		m_fSpawnTime;
	LTBOOL		m_bResetHealthAndArmor;

	LTFLOAT		m_fMinHealth;	//does not need save/load
	LTBOOL		m_bInCinematic;	//does not need save/load
	HistoryStamp m_StampHistory[NUM_HISTORY]; //does not need save/load
	LTFLOAT		m_fLastHistory;	//does not need save/load
	LTBOOL		m_bNewGame;		//does not need save/load
	LTBOOL		m_bPlayingLoopFireSound; //does not need save/load
	DBYTE		m_nLoopKeyId; //does not need save/load
};


#endif  // __CPLAYER_OBJ_H__


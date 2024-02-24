// ----------------------------------------------------------------------- //
//
// MODULE  : Character.h
//
// PURPOSE : Base class for player and AI
//
// CREATED : 10/6/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include "GameBase.h"
#include "Destructible.h"
#include "ModelDefs.h"  // for ModelNode
#include "WeaponMgr.h"  // for WMGR_INVALID_ID
#include "Editable.h"
#include "CharacterMovement.h"
#include "CharacterAnimation.h"
#include "ltsmartlink.h"
#include "LTString.h"
#include "SurfaceDefs.h"
#include "Timer.h"
#include "ProjectileTypes.h"

#include <map>

// ----------------------------------------------------------------------- //

#define INVALID_ANI						((HMODELANIM)-1)
#define BC_DEFAULT_SOUND_RADIUS			1500.0f
#define MAX_TIMED_POWERUPS				8

#define CLOAK_STATE_INACTIVE	0
#define CLOAK_STATE_RAMPUP		1
#define CLOAK_STATE_ACTIVE		2
#define CLOAK_STATE_RAMPDOWN	3

#define EYEFLASH_COLOR_GREEN	0
#define EYEFLASH_COLOR_RED		1
#define EYEFLASH_COLOR_BLUE		2

#define TRIGGER_ATTACH			"ATTACH"
#define TRIGGER_DETACH			"DETACH"

// ----------------------------------------------------------------------- //

class BodyProp;
class CinematicTrigger;
class CAIVolume;
class CSimpleNode;
class CWeapon;
class CWeapons;
class VolumeBrush;
class CAttachments;
//class CTargetingSprite;
class PickupObject;
struct SoundButes;

// ----------------------------------------------------------------------- //

struct ExosuitData //save data structure for power-up
{
	ExosuitData::ExosuitData()
	{
		nFlamerAmmo		= 0;
 		nPlasmaAmmo		= 0;
  		nRocketAmmo		= 0;
   		nBulletAmmo		= 0;
		nEnergy			= 0;
		bDisabled		= LTFALSE;
	}

	int		nFlamerAmmo;
 	int		nPlasmaAmmo;
  	int		nRocketAmmo;
   	int		nBulletAmmo;
	int		nEnergy;
	LTBOOL	bDisabled;

	void Save(HMESSAGEWRITE hWrite);
	void Load(HMESSAGEREAD hRead);
};

// ----------------------------------------------------------------------- //

inline void ExosuitData::Save(HMESSAGEWRITE hWrite)
{
	if (!g_pServerDE || !hWrite) return;

	*hWrite << nFlamerAmmo;
	*hWrite << nPlasmaAmmo;
	*hWrite << nRocketAmmo;
	*hWrite << nBulletAmmo;
	*hWrite << nEnergy;
	*hWrite << bDisabled;
}

// ----------------------------------------------------------------------- //

inline void ExosuitData::Load(HMESSAGEREAD hRead)
{
	if (!g_pServerDE || !hRead) return;

	*hRead >> nFlamerAmmo;
 	*hRead >> nPlasmaAmmo;
  	*hRead >> nRocketAmmo;
   	*hRead >> nBulletAmmo;
	*hRead >> nEnergy;
	*hRead >> bDisabled;
}

// ----------------------------------------------------------------------- //

struct CharFootstepSoundInfo
{
	LTFLOAT			fTime;
	LTFLOAT			fVolume;
	SurfaceType		eSurfaceType;
	LTVector		vLocation;

	CharFootstepSoundInfo::CharFootstepSoundInfo()
		: fTime(-FLT_MAX),
		  fVolume(0.0f),
		  eSurfaceType(ST_UNKNOWN),
		  vLocation(0,0,0) {}

	void Save(HMESSAGEWRITE hWrite);
	void Load(HMESSAGEREAD hRead);
};

// ----------------------------------------------------------------------- //

inline void CharFootstepSoundInfo::Save(HMESSAGEWRITE hWrite)
{
	if (!g_pServerDE || !hWrite) return;

	hWrite->WriteFloat( fTime - g_pLTServer->GetTime() );
	*hWrite << fVolume;
	hWrite->WriteWord(eSurfaceType);
	*hWrite << vLocation;
}

// ----------------------------------------------------------------------- //

inline void CharFootstepSoundInfo::Load(HMESSAGEREAD hRead)
{
	if (!g_pServerDE || !hRead) return;

	fTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	*hRead >> fVolume;
	eSurfaceType = SurfaceType( hRead->ReadWord() );
	*hRead >> vLocation;
}


// ----------------------------------------------------------------------- //

struct CharFireInfo
{
	LTVector		vFiredPos;
	LTVector		vFiredDir;
	uint8			nWeaponId;
	uint8			nBarrelId;
	uint8			nAmmoId;
	LTFLOAT			fTime;
	SurfaceType		eSurface;

	CharFireInfo()
		: vFiredPos(0,0,0),
		  vFiredDir(0,0,0),
		  nWeaponId(WMGR_INVALID_ID),
		  nBarrelId(WMGR_INVALID_ID),
		  nAmmoId(WMGR_INVALID_ID),
		  fTime(-1.0f),
		  eSurface(ST_UNKNOWN) {}

	void Save(HMESSAGEWRITE hWrite)
	{
		if (!g_pServerDE || !hWrite) return;

		*hWrite << vFiredPos;
		*hWrite << vFiredDir;
		*hWrite << nWeaponId;
		*hWrite << nBarrelId;
		*hWrite << nAmmoId;
		hWrite->WriteFloat(fTime - g_pLTServer->GetTime());
		hWrite->WriteWord(eSurface);
	}

	void Load(HMESSAGEREAD hRead)
	{
		if (!g_pServerDE || !hRead) return;

		*hRead >> vFiredPos;
		*hRead >> vFiredDir;
		*hRead >> nWeaponId;
		*hRead >> nBarrelId;
		*hRead >> nAmmoId;
		fTime = hRead->ReadFloat() + g_pLTServer->GetTime();
		eSurface = SurfaceType( hRead->ReadWord() );
	}

};

struct CharImpactInfo
{
	LTVector		vImpactPos;
	uint8			nWeaponId;
	uint8			nBarrelId;
	uint8			nAmmoId;
	LTFLOAT			fTime;
	SurfaceType		eSurface;

	CharImpactInfo()
		: vImpactPos(0,0,0),
		  nWeaponId(WMGR_INVALID_ID),
		  nBarrelId(WMGR_INVALID_ID),
		  nAmmoId(WMGR_INVALID_ID),
		  fTime(-1.0f),		  
		  eSurface(ST_UNKNOWN) {}

	void Save(HMESSAGEWRITE hWrite)
	{
		if (!g_pServerDE || !hWrite) return;

		*hWrite << vImpactPos;
		*hWrite << nWeaponId;
		*hWrite << nBarrelId;
		*hWrite << nAmmoId;
		hWrite->WriteFloat(fTime - g_pLTServer->GetTime());
		hWrite->WriteByte(eSurface); ASSERT( eSurface < 256 );
	}

	void Load(HMESSAGEREAD hRead)
	{
		if (!g_pServerDE || !hRead) return;

		*hRead >> vImpactPos;
		*hRead >> nWeaponId;
		*hRead >> nBarrelId;
		*hRead >> nAmmoId;
		fTime = hRead->ReadFloat() + g_pLTServer->GetTime();
		eSurface = SurfaceType(hRead->ReadByte());
	}

};

struct SPEARSTRUCT
{
	SPEARSTRUCT::SPEARSTRUCT()
	{
		hObject		= LTNULL;
		rRot.Init();
		eModelNode	= (ModelNode)INVALID_MODEL_NODE;
	}

	LTSmartLink	hObject;
    LTRotation  rRot;
	ModelNode	eModelNode;

	void Save(HMESSAGEWRITE hWrite)
	{
		if (!g_pServerDE || !hWrite) return;

		*hWrite << hObject;
		*hWrite << rRot;
		hWrite->WriteByte(eModelNode);
	}

	void Load(HMESSAGEREAD hRead)
	{
		if (!g_pServerDE || !hRead) return;

		*hRead >> hObject;
		*hRead >> rRot;
		eModelNode = ModelNode(hRead->ReadByte());
	}

};

enum NetStage
{
	NET_STAGE_INVALID = -1,
	NET_STAGE_START = 0,
	NET_STAGE_END,
	NET_STAGE_STRUGGLE,
	NET_STAGE_GETUP,
	MAX_NET_ANIMS,
};

// ----------------------------------------------------------------------- //

class CCharacter : public GameBase
{
	public :

		typedef std::map<int,CTimer> SoundDelayMap;

	// ------------------------------------------------------------------------------------------ //
	//
	// MEMBER FUNCTIONS for CCharacter
	//
	// ------------------------------------------------------------------------------------------ //

	public :

		CCharacter();
		~CCharacter();

		LTBOOL			IsDead()			const { return m_damage.IsDead(); }
		void			CheckMissionFailed(HOBJECT hDamager);

		const CharacterMovement*	GetMovement()	const { return &m_cmMovement; }
		const CharacterButes*		GetButes()		const { return m_cmMovement.GetCharacterButes(); }
		const CharacterVars*		GetVars()		const { return m_cmMovement.GetCharacterVars(); }

		const CharacterAnimation*	GetAnimation()	const { return &m_caAnimation; }

		CharacterMovement*	GetMovement()			{ return &m_cmMovement; }
		CharacterAnimation*	GetAnimation()			{ return &m_caAnimation; }

		virtual uint32	GetMovementAnimFlags() const { return m_cmMovement.GetControlFlags(); }

		// ------------------------------------------------------------------------------------------ //
		// Movement information shortcuts.

		LTVector			GetDims()				const { return GetMovement()->GetObjectDims(); }

		const LTVector&		GetPosition()			const { return GetMovement()->GetPos(); }
		const LTRotation&	GetRotation()			const { return GetMovement()->GetRot(); }

		const LTVector&		GetUp()					const { return GetMovement()->GetUp(); }
		const LTVector&		GetRight()				const { return GetMovement()->GetRight(); }
		const LTVector&		GetForward()			const { return GetMovement()->GetForward(); }

		LTRotation GetAimRotation() const;
		LTVector GetAimForward() const;
		LTVector GetAimUp() const;

		LTBOOL	GetInjuredMovement() const { return m_bForceCrawl; }
		virtual void HandleInjured() { }

		// ------------------------------------------------------------------------------------------ //
		// Attachments

		virtual void	CreateAttachments();
		void			DestroyAttachments();
		CAttachments*	GetAttachments() { return m_pAttachments; }
		const CAttachments*	GetAttachments() const { return m_pAttachments; }
		CAttachments*	TransferAttachments(); // Responsibility of call to delete attachments
		void			RemoveNet();

		// ------------------------------------------------------------------------------------------ //

		CDestructible*	GetDestructible() { return &m_damage; }
		virtual CharacterClass  GetCharacterClass() const  { ASSERT(0); return UNKNOWN; }

		HOBJECT			GetHitBox()			const { return m_hHitBox; }

		ModelSkeleton	GetModelSkeleton()							const	{ return ModelSkeleton(m_cmMovement.GetCharacterButes()->m_nSkeleton); }
		void			SetModelNodeLastHit(ModelNode eModelNodeLastHit)	{ m_eModelNodeLastHit = eModelNodeLastHit; }
		ModelNode		GetModelNodeLastHit()						const	{ return m_eModelNodeLastHit; }

		void			SetLastFireInfo(const CharFireInfo & val);
		const CharFireInfo & GetLastFireInfo()						const	{ return m_LastFireInfo; }

		void			SetLastImpactInfo(const CharImpactInfo & val);
		const CharImpactInfo & GetLastImpactInfo()						const	{ return m_LastImpactInfo; }

		const CharFootstepSoundInfo	& GetLastFootstepSoundInfo()	const	{ return m_LastFootstepSoundInfo; }

		// ------------------------------------------------------------------------------------------ //
		// AI Volume information

		const CAIVolume*	GetCurrentVolumePtr()	const	{ return m_pCurrentVolume; }
		CAIVolume*			GetCurrentVolumePtr()			{ return m_pCurrentVolume; }
		const CAIVolume*	GetLastVolumePtr()		const	{ return m_pLastVolume;    }
		CAIVolume*			GetLastVolumePtr()				{ return m_pLastVolume;    } 
		// LastVolumePos is only valid if we have a last volume!  
		// If we have a current volume, it is just our current position.
		const LTVector&		GetLastVolumePos()		const	{ return m_vLastVolumePos; }   

		const CSimpleNode *	GetLastSimpleNode() const  { return m_pLastSimpleNode; }
		CSimpleNode *		GetLastSimpleNode()        { return m_pLastSimpleNode; }
		

		// ------------------------------------------------------------------------------------------ //

		LTFLOAT			GetHitPoints() const { return m_damage.GetHitPoints(); }
		LTFLOAT			GetMaxHitPoints() const { return m_damage.GetMaxHitPoints(); }
		virtual LTFLOAT	ComputeDamageModifier(ModelNode eModelNode) const;

		virtual void	RemoveObject();
		virtual void	SpawnItem(char* pItem, LTVector & vPos, LTRotation & rRot);

		// ------------------------------------------------------------------------------------------ //
		// Dialogue control functions

		LTBOOL			IsPlayingVoicedSound() const { return (LTBOOL)!!m_hCurVcdSnd; }
		LTBOOL			IsPlayingDialogue();


		LTBOOL			PlayDialogue(const char *szDialogue, CinematicTrigger* pCinematic, BOOL bWindow = FALSE, 
							BOOL bStayOpen = FALSE, const char *szCharOverride = NULL, const char *szDecisions = NULL, BYTE byMood=0);
		LTBOOL			PlayDialogue(DWORD dwID, CinematicTrigger* pCinematic,	BOOL bWindow = FALSE, BOOL bStayOpen = FALSE,
							const char *szCharOverride = NULL, const char *szDecisions = NULL, BYTE byMood=0);

		LTBOOL			DoDialogueWindow(CinematicTrigger* pCinematic,DWORD dwID, BOOL bStayOpen = FALSE, 
							const char *szCharOverride = NULL, const char *szDecisions = NULL);
		
		virtual void	StopDialogue(LTBOOL bCinematicDone = LTFALSE);

		// ------------------------------------------------------------------------------------------ //
		// Voiced sound functions

		void			KillVoicedSound();

		int			PlayDialogueSound(const char* szSoundFile);
		int			PlayDamageSound(DamageType eType);
		int			PlayExclamationSound(const char * szSoundType);

		// ------------------------------------------------------------------------------------------ //

		virtual LTVector	GetChestOffset() const;
		virtual LTVector	GetHeadOffset() const;

		virtual LTBOOL	HasDangerousWeapon() { return LTFALSE; }

		// ------------------------------------------------------------------------------------------ //
		// Getting and setting character information

		virtual void	SetCharacter(int nCharacterSet, LTBOOL bReset = LTTRUE)
			{ m_nCharacterSet = nCharacterSet; if(bReset) ResetCharacter(); }

		virtual int		GetCharacter()	const				{ return m_nCharacterSet; }
		virtual void	ResetCharacter();

		virtual void	SetupModelPieces();

		virtual std::string GetCharacterSetName(const std::string & root_name) const { return root_name; }

		// ------------------------------------------------------------------------------------------ //

		virtual void	HandleNetHit(LTVector vDir, HOBJECT hShooter);

		virtual void	HandleEMPEffect();
		virtual void	UpdateEMPEffect();

		virtual LTBOOL	IsStunned() const { return m_bStunEffect || m_bEMPEffect; }
		virtual void	HandleStunEffect();
		virtual void	UpdateStunEffect();

		virtual void	HandleCharacterSFXMessage(HCLIENT hSender, HMESSAGEREAD hRead);

		virtual void	HandleCloakToggle();
		virtual void	ForceCloakOff(LTBOOL bDelay=LTFALSE);
		virtual void	ResetCloak();
		virtual void	UpdateCloakEffect();
		int				GetCloakState() const { return m_nCloakState; }
		void			SetCloakState(uint8 nState) { m_nCloakState = nState; }

		void			HandleDiskRetrieve();

		virtual uint32	GetPredatorEnergy();
		virtual uint32	GetMaxPredatorEnergy();
		void			DecrementPredatorEnergy(int nAmount=1);

		void	AddSpear(HOBJECT hObject, HOBJECT hSpear, ModelNode eModelNode, const LTRotation& rRot);
		void	TransferSpears(BodyProp* pBody);
//		void	TransferNet(BodyProp* pBody);
		LTBOOL	IsNetted() const { return (m_hNet != LTNULL); }
		void	HandleNetSlash(uint32 nDamage);

		void	SetCannonChargeLevel(uint8 nLevel) { m_nCannonChargeLevel=nLevel;  }
		uint8	GetCannonChargeLevel() { return m_nCannonChargeLevel; }
		uint8	GetMaxChargeLevel() { return m_nMaxChargeLevel; }

		LTBOOL	IsFlashlightOn() const { return m_bFlashlight; }
		virtual void SetFlashlight(LTBOOL bOn);
		LTVector	GetFlashlightPos() const { return m_vFlashlightPos; }
		void	SetFlashlightPos(LTVector vPos) { m_vFlashlightPos = vPos; }
		virtual LTRotation GetFlashlightRot() const { return GetAimRotation(); }

		LTVector GetModelLighting() const { return m_vModelLighting; }

		LTBOOL	IsNightVisionOn() const { return m_bNightvision; }
		void	SetNightVision(LTBOOL bOn=LTTRUE) { m_bNightvision=bOn; }

		// Targeting sprite functions to be called from weapons class
		void	SetTargetingSprite(LTBOOL bOn=LTFALSE);
		void	SetFlameThrowerFX(LTBOOL bOn=LTFALSE);
//		void	InitTargetingSprite(char* szSprite, LTVector vScale);
//		void	RemoveTargetingSprite();

		// Inventory info
		LTBOOL				NeedsAmmo(AMMO_POOL *pPool, uint32 &nAmt);
		LTBOOL				NeedsWeapon(WEAPON *pWeapon);

		virtual CWeapon*	GetCurrentWeaponPtr();

		virtual HOBJECT		CreateBody(LTBOOL bCarryOverAttachments = LTTRUE);

		virtual	void		UpdateInfoString(char *szString);

		void	RespawnExosuit();


		// ------------------------------------------------------------------------------------------ //
		// Alien life cycle functions

		void	UpdateChestbursterCycle();
		void	SetChestburstedFrom(uint32 nCharacter)		{ m_nChestburstedFrom = nCharacter; }


		// ------------------------------------------------------------------------------------------ //
		// Animation functions

		virtual void	UpdateAnimation();
		virtual	WEAPON*	UpdateWeaponAnimation();
		virtual void	UpdateActionAnimation();

		LTBOOL	SetAnimExpression(int nExpression);

		LTBOOL  IsRecoiling()  { return ( m_caAnimation.IsValidTracker(ANIM_TRACKER_RECOIL) && m_caAnimation.GetTrackerInfo(ANIM_TRACKER_RECOIL)->m_nIndex != -1 ); }

		void	CreateMaskPickup(LTVector *vVel = LTNULL, LTBOOL bMessage = LTTRUE);
		void	EyeFlash(uint8 nColorDef = EYEFLASH_COLOR_GREEN);

		LTBOOL	HasMask() const {return m_bHasMask; }									// Do we have a mask (Predators)
		LTBOOL	HasNightVision() const {return m_bHasNightVision; }						// Do we have night vision (Humans)
		LTBOOL	HasCloakDevice() const { return m_bHasCloakDevice; }					// Do we have a cloak device
		LTBOOL	HasFlarePouch() const { return m_bHasFlarePouch; }						// Do we have a flare pouch
		LTBOOL	HasDiscAmmo();

		typedef std::list<CFlare*>		FlareList;
		typedef FlareList::iterator		FlareIterator;

		FlareIterator	BeginFlares() { return m_FlareList.begin(); }
		FlareIterator	EndFlares()   { return m_FlareList.end(); }
		void			AddFlare(CFlare *pFlare)	{ if( m_FlareList.end() == std::find(m_FlareList.begin(), m_FlareList.end(), pFlare) ) m_FlareList.push_back(pFlare); }
		void			RemoveFlare(CFlare *pFlare) { m_FlareList.remove(pFlare); }

		LTBOOL	GetNodePosition(ModelNode eNode, LTVector &vPos, ModelSkeleton eModelSkele, LTRotation* pRot=LTNULL);
		void	CacheFiles();

	protected :

		enum Constants
		{
			kMaxSpears = 16,
		};

		uint32			EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		uint32			ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		LTBOOL	ProcessTriggerMsg(const char* pMsg); // Do not over-ride this, over-ride ProcessCommand instead.

		virtual LTBOOL	IgnoreDamageMsg(const DamageStruct & damage_msg) { return LTFALSE; }
		virtual void	ProcessDamageMsg(const DamageStruct & damage_msg);
		virtual void	ProcessAnimationMsg(uint8 nTracker, uint8 bLooping, uint8 nMsg) { }
		virtual LTBOOL	ProcessPowerupMsg(char* pMsg, HOBJECT hSender, uint32* nAmt, uint32* nUsed, LTBOOL bNotifyHud=LTTRUE, LTBOOL* pPartialPU=LTNULL);
		virtual LTBOOL	ProcessCommand(const char*const* pTokens, int nArgs);

		virtual void    HandleFootstepKey();

		// ------------------------------------------------------------------------------------------ //

		virtual void	Reset();

		virtual void	UpdateBatteryLevel();

		virtual void	UpdateAirLevel();
		virtual void	UpdateOnGround();
		virtual void	UpdateSounds();
		virtual void	UpdateCharacterFX();

		void	(CCharacter::*fpUpdateExoEnergy)();

		virtual void	HandleDead(LTBOOL bRemoveBody);

		virtual void	StartDeath();

		virtual std::string GetDefaultDeathAni();
		virtual LTBOOL ForceDefaultDeathAni() { return LTFALSE; }

		virtual LTBOOL	HandlePickup(char **pToks, int nToks, HOBJECT hSender, uint32* nAmt, uint32* nUsed, LTBOOL bNotifyHud=LTTRUE, int nMainToks=0, uint32* pResidual=LTNULL);

		virtual void	HandleExosuitEntry(HOBJECT hSender);
		LTBOOL			HandleExosuitEject(LTVector &vNewPos, LTBOOL bCreatePickup = LTTRUE);
		void			SetExoPUData(PickupObject* pPickUp, LTFLOAT fHPs);
		void			GetExoPUData(PickupObject* pPickUp);
		void			SetExoInitialAmmo();

		virtual LTVector	HandHeldWeaponFirePos(const CWeapon* pWeapon) const;


		virtual void	CreateHitBox();
		virtual void	UpdateHitBox();

		virtual void	CreateSpecialFX();

		virtual void   UpdateVolumeInfo(LTBOOL bResetVolumeInfo = LTFALSE  );
		void	UpdateLastSimpleNode();

		virtual LTFLOAT GetSimpleNodeCheckDelay();  // Over-ride this to get more or less frequent simple node checks.

		void SetObjectFlag(uint32 flag)	  { m_cmMovement.SetObjectFlags( OFT_Flags, m_cmMovement.GetObjectFlags(OFT_Flags) | flag ); }
		void UnsetObjectFlag(uint32 flag)  { m_cmMovement.SetObjectFlags( OFT_Flags, m_cmMovement.GetObjectFlags(OFT_Flags) & ~flag ); }

		void SpewAcidDamage(LTFLOAT fDamage);
		LTFLOAT GetAcidDamageMod();
		virtual void HandleAlienDamage();				// AIQueen overrides this
		void ExplodeLimb(ModelNode eParentNode, ModelSkeleton eModelSkele);
		void CreateGibs(ModelNode eNode, ModelSkeleton eModelSkele);
		void CacheDebris();


		// ------------------------------------------------------------------------------------------ //
		// Inventory control functions

		void	SetHasFlarePouch(LTBOOL bHas = LTTRUE) { m_bHasFlarePouch = bHas; }		// Set the have flare pouch

		void	SetHasCloakDevice(LTBOOL bHas = LTTRUE);								// Set the have cloak device

		void	SetHasMask(LTBOOL bHas = LTTRUE, LTBOOL bMessage = LTTRUE);				// Set the have mask (Predators)
		void	SetHasNightVision(LTBOOL bHas = LTTRUE);								// Set night vision (Humans)

		void	SetMaxChargeLevel();													// set the maximum level for pre charge on the predator shoulder cannon
		virtual void	HandleEnergySift();												// update the pred energy
		virtual void	HandleMedicomp();												// handle a medicomp charage.
		void	UpdateExoEnergy();
		void	DecrementExosuitEnergy();
		uint32	GetExosuitEnergy();
		uint32  GetMaxExosuitEnergy();
		uint32	GetExoRocketAmmo();
		uint32	GetExoFlamerAmmo();
		uint32	GetExoPlasmaAmmo();
		uint32	GetExoBulletAmmo();
		void	ResetExosuitEnergy();

		void	SetExoRocketAmmo(int nAmount);
		void	SetExoFlamerAmmo(int nAmount);
		void	SetExoPlasmaAmmo(int nAmount);
		void	SetExoBulletAmmo(int nAmount);
		void	SetExosuitEnergy(int nAmount);

		// ------------------------------------------------------------------------------------------ //
		// Targeting sprite update function to be called by player or AI

//		void	UpdateTargetingSprite(LTBOOL bOn, LTVector vPos = LTVector(0,0,0), LTVector vNorm = LTVector(0,0,0), LTVector vCamF = LTVector(0,0,0));


		// ------------------------------------------------------------------------------------------- //
		// Voiced sound stuff

		LTBOOL PlayVoicedSound(int nButes, const SoundButes & butes, const char* pSound, int nImportance);

	// ------------------------------------------------------------------------------------------ //
	//
	// MEMBER VARIABLES for CCharacter
	//
	// ------------------------------------------------------------------------------------------ //

	public :

	protected :

		// Our aggregrates.
		CDestructible		m_damage;					// Handle damage/healing
		CAttachments*		m_pAttachments;				// Our attachments
		CEditable			m_editable;					// Handle editting


		// Character attribute information variables
		uint32				m_nCharacterSet;			// Id number for character attributes in CharacterButeMgr
		LTBOOL				m_bCharacterReset;			// Was the character just reset?
		uint32				m_nOldCharacterSet;			// Id number for character i once was
		LTFLOAT				m_fOldHealth;				// Amount of health I once had
		LTFLOAT				m_fOldArmor;				// Amount of aromor I once had
		HOBJECT				m_hHead;					// The head attachment for exosuits
		WEAPON*				m_pOldWeapon;				// The weapon I once had
		LTBOOL				m_bResetAttachments;		// Should we reset our attachments?

		CharacterMovement	m_cmMovement;				// The movement control class
		CharacterAnimation	m_caAnimation;				// The animation control class

		uint32				m_nPreCreateClientID;		// Tells whether this is a player character or not (used for SFX message)


		// Character creation information variables
		uint32				m_dwFlags;					// Initial flags

		// Air supply information variables
		LTFLOAT				m_fAirSupplyTime;			// The amount of time remaining for our air supply
		LTFLOAT				m_fOldAirSupplyTime;		// The amount of time remaining for our air supply I once had

		// Stored charge info for Predator Shoulder Cannon
		uint8 				m_nCannonChargeLevel;		// The level of charge for the Predator Shoulder Cannon.
		uint8				m_nMaxChargeLevel;

		// Stored battery info for Marine night vision and flashlight
		LTFLOAT				m_fBatteryChargeLevel;		// The level of charage of the Marine flashlight and night vision battery

		// Aiming information
		LTFLOAT				m_fLastAimingPitch;			// The last value of the aiming pitch from CharacterMovementVars
		LTFLOAT				m_fLastAimingYaw;			// The last value of the aiming yaw from CharacterMovementVars

		uint8				m_nLastAimingPitchSet;		// The set of aiming pitch values to use
		uint8				m_nAimingPitchSet;			// The set of aiming pitch values to use

		// Information needed to update strafe state
		uint8				m_nLastStrafeState;			// Lets us know if the client needs to update his strafe state

		// Information about the character/player's net state.
		NetStage			m_eNetStage;				// What stage of aour animation are we in.
		NetStage			m_eLastNetStage;			// What stage of our animation we were in last update.

		// Information about our exosuit start point
		LTSmartLink			m_hStartpointSpawner;		// The handle to our startpoint...

	private :

		// Character volume information
		CAIVolume *			m_pCurrentVolume;
		CAIVolume *			m_pLastVolume;
		LTVector			m_vLastVolumePos;

		CSimpleNode *		m_pLastSimpleNode;			// The last simple node we were at.
		CTimer				m_tmrNextSimpleNodeCheck;   // Check when this timer runs out.

		// Character death information variables
		LTBOOL				m_bCreateBody;				// Create body prop when dead

		// Character damage information varaibles
		LTSmartLink			m_hHitBox;					// Used to calculate weapon impacts

		LTFLOAT				m_fDefaultHitPts;			// Default hit pts
		LTFLOAT				m_fDefaultArmor;			// Default armor

		ModelNode			m_eModelNodeLastHit;		// The model node that was last hit

		CharFireInfo				m_LastFireInfo;				// Info about last fired shot
		CharImpactInfo				m_LastImpactInfo;			// Info about last impact (is not last fired shot for projectiles)
		CharFootstepSoundInfo		m_LastFootstepSoundInfo;	// Info about last foot step sound

		// Character stat information variables
		LTVector			m_vOldCharacterColor;		// Old color (for use with stealth powerup)
		LTFLOAT				m_fOldCharacterAlpha;		// Old alpha value (for use with stealth powerup)
		LTBOOL				m_bCharacterHadShadow;		// character had shadow before stealth powerup
		LTString			m_hstrSpawnItem;			// Object to spawn when dead

		// Character special FX information variables

		LTBOOL				m_bEMPEffect;				// Are we under EMP effect
		LTBOOL				m_bStunEffect;				// Are we under Stun effect

		SoundDelayMap		m_SoundDelays;				// Used to be sure particular voiced sounds aren't played too often.

		// Inventory related variables
		LTBOOL				m_bHasFlarePouch;			// Do we have a flare pouch for flare tossing?
		LTBOOL				m_bHasCloakDevice;			// Do we have a cloak device?
		LTBOOL				m_bHasMask;					// Does this character have a mask? (Predators only)
		LTBOOL				m_bHasNightVision;			// Does this character have night vision? (Humans only)
		LTBOOL				m_bNightvision;				// Is night vision turned on

		LTFLOAT				m_fCloakStartTime;			// The time we turned on our cloaking device
		uint8				m_nCloakState;				// The current state of cloaking
		LTFLOAT				m_fEnergyChange;			// How much the energy is changing
		LTBOOL				m_bExosuitDisabled;			// Is the exosuit disabled?


		LTFLOAT				m_fDialogueStartTime;		// The time we started to play a dialogue (used to delay switching)
		LTBOOL				m_bPlayingTextDialogue;		// Are we displaying text

		// Spear related stuff
		uint32				m_cSpears;					// How many spears do we have stuck in us
		SPEARSTRUCT			m_aSpears[kMaxSpears];		// Array of spear HOBJECTs

		// Flashlight / shoulder lamp
		LTBOOL				m_bFlashlight;				// is it on or off?
		LTVector			m_vFlashlightPos;			// end of the beam

		// Model lighting
		LTVector			m_vModelLighting;			// sent up to us from client.  Initialized to (-1,-1,-1).

		// Life cycle information for aliens
		uint32				m_nChestburstedFrom;		// The type of character a chestburster came from
		LTFLOAT				m_fChestbursterMutateTime;	// The time we started to mutate
		LTFLOAT				m_fChestbursterMutateDelay;	// The current delay amount for switch to a new character

		// Animation information
		int					m_nDesiredExpression;		// The expression last set with SetAnimExpression.
		LTFLOAT				m_fLastBlinkTime;			// The last time we started a blink animation
		LTFLOAT				m_fBlinkDelay;				// The time we're waiting for the next blink.
		CTimer				m_tmrMinRecoilDelay;		// Don't recoil until this timer has stopped.

		// Cloaking information
		LTFLOAT				m_fCloakDelayTime;			// This is the start time for the cloak delay
		LTFLOAT				m_fCloakEnergyUsed;			// This is the amount of accumulated energy used


		LTBOOL				m_bForceCrawl;				// Must we crawl?
		
		LTSmartLink			m_hNet;						// Our predator net attachment.
		LTFLOAT				m_fNetTime;					// The game time when were we netted.
		uint32				m_nNetDamage;				// How much damage has our net taken.
		LTBOOL				m_bFirstNetUpdate;			// Is this our first update?
		HLTSOUND			m_hNetStruggleSound;		// The looping struggle sound.

		FlareList			m_FlareList;				// List of flares belonging to this character

		// NOTE:  The following data members do not need to be saved / loaded
		// when saving games.  Any data members that don't need to be saved
		// should be added here (to keep them together)...

		HLTSOUND			m_hCurVcdSnd;				// Handle to current voiced sound
		int					m_nCurVcdSndImp;			// Importance of sound playing
		LTBOOL				m_bCurVcdSndLipSynced;		// Is the current voiced sound lip synced?
		LTBOOL				m_bCurVcdSndSubtitled;		// Is the current voiced sound subtitled?
		LTBOOL				m_bConfirmFX;

		HMODELANIM			m_NetAnims[MAX_NET_ANIMS];	// Our array of net animations


// ------------------------------------------------------------------------------------------ //
//		Private Functions
// ------------------------------------------------------------------------------------------ //
		
		HOBJECT	CreateNet();
		void	AttachNet(HOBJECT hNet);
		void	HandleNetEscape();
		LTBOOL	LoadNetAnimation();

		void	InitialUpdate(int nInfo);	
		void	Update();
		void	UpdateNet();

		LTBOOL	ReadProp(ObjectCreateStruct *pStruct);
		void    PostReadProp(ObjectCreateStruct *pStruct);
		void	HandleModelString(ArgList* pArgList);
		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
};

// ------------------------------------------------------------------------------------------ //
// ------------------------------------------------------------------------------------------ //
// ------------------------------------------------------------------------------------------ //
class CCharacterButeMgrPlugin;
class CHumanAttachmentsPlugin;

class CCharacterPlugin : public IObjectPlugin
{
	public:

		CCharacterPlugin()
			: m_pCharacterButeMgrPlugin(LTNULL),
			  m_pHumanAttachmentsPlugin(LTNULL) {}

		virtual ~CCharacterPlugin();

		virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char* const * aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

	private:
		CCharacterButeMgrPlugin * m_pCharacterButeMgrPlugin;
		CHumanAttachmentsPlugin	* m_pHumanAttachmentsPlugin;
};

// ------------------------------------------------------------------------------------------ //

#endif // __CHARACTER_H__

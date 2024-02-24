// ----------------------------------------------------------------------- //
//
// MODULE  : SharedFXStructs.h
//
// PURPOSE : Shared Special FX structs
//
// CREATED : 10/21/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SHARED_FX_STRUCTS_H__
#define __SHARED_FX_STRUCTS_H__

#include "Globals.h"
#include "SharedBaseFXStructs.h"
#include "ModelDefs.h"
#include "SFXMsgIds.h"
#include "SharedMovement.h"

/////////////////////////////////////////////////////////////////////////////
//
// CHARCREATESTRUCT class Definition 
//
// (used by client-side CCharacterFX and server-side Character classes)
//
/////////////////////////////////////////////////////////////////////////////

struct CHARCREATESTRUCT : public SFXCREATESTRUCT
{
	CHARCREATESTRUCT();

	virtual void Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite);
	virtual void Read(ILTCSBase *pInterface, HMESSAGEREAD hRead);

	uint8			nCharacterSet;
	uint8			nNumTrackers;
	uint32			nClientID;


	// These members don't get sent through the SFX message
	ModelSkeleton	eModelSkeleton;
};

inline CHARCREATESTRUCT::CHARCREATESTRUCT()
{
	nCharacterSet	= -1;
	nNumTrackers	= LTTRUE;
	nClientID		= 0;


	// These members don't get sent through the SFX message
	eModelSkeleton	= eModelSkeletonInvalid;
}

/////////////////////////////////////////////////////////////////////////////
//
// BODYPROPCREATESTRUCT class Definition 
//
// (used by client-side CCharacterFX and server-side Character classes)
//
/////////////////////////////////////////////////////////////////////////////

struct BODYPROPCREATESTRUCT : public SFXCREATESTRUCT
{
	BODYPROPCREATESTRUCT();

	virtual void Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite);
	virtual void Read(ILTCSBase *pInterface, HMESSAGEREAD hRead);

	uint8			nCharacterSet;
	HOBJECT			hPlayerObj;

	LTBOOL			bBloodTrail;
	LTBOOL			bBloodPool;

	LTBOOL			bLimbProp;

	// These members don't get sent through the SFX message
	ModelSkeleton	eModelSkeleton;
};

inline BODYPROPCREATESTRUCT::BODYPROPCREATESTRUCT()
{
	nCharacterSet	= -1;
	hPlayerObj		= LTNULL;

	bBloodTrail		= LTTRUE;
	bBloodPool		= LTFALSE;

	bLimbProp		= LTFALSE;

	// These members don't get sent through the SFX message
	eModelSkeleton	= eModelSkeletonInvalid;
}


/////////////////////////////////////////////////////////////////////////////
//
// CHARCREATESTRUCT class Definition
//
// (used by client-side CCharacterFX and server-side Character classes)
//
/////////////////////////////////////////////////////////////////////////////
/*
struct CHARCREATESTRUCT : public SFXCREATESTRUCT
{
	enum Flags
	{
		eSmokepuffs	= 0x01,
		eHearts		= 0x02,
		eChat		= 0x04,
		eSleep		= 0x08,
	};

	CHARCREATESTRUCT() { Clear(); }
	void Clear();

	void SetSleeping(LTBOOL bSleeping) { if ( bSleeping ) byFXFlags |= eSleep; else byFXFlags &= ~eSleep; }
	inline LTBOOL IsSleeping() const { return byFXFlags & eSleep ? LTTRUE : LTFALSE; }

	void SetChatting(LTBOOL bChatting) { if ( bChatting ) byFXFlags |= eChat; else byFXFlags &= ~eChat; }
	inline LTBOOL IsChatting() const { return byFXFlags & eChat ? LTTRUE : LTFALSE; }

    virtual void Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite);
    virtual void Read(ILTCSBase *pInterface, HMESSAGEREAD hRead);

    LTBOOL          bIsPlayer;
    uint8           byFXFlags;
	uint8			nCharacterSet;
	ModelSkeleton	eModelSkeleton;
//    uint8           nTrackers;
//    uint8           nDimsTracker;
    LTFLOAT         fStealthPercent;
	uint8			nClientID;
};

inline void CHARCREATESTRUCT::Clear()
{
    bIsPlayer       = LTFALSE;
	byFXFlags		= 0;
	nCharacterSet	= 0;
	eModelSkeleton	= eModelSkeletonInvalid;
//	nTrackers		= 0;
//	nDimsTracker	= 0;
	fStealthPercent	= 0.0f;
	nClientID		= (uint8)-1;
}
*/
/////////////////////////////////////////////////////////////////////////////
//
// BODYCREATESTRUCT class Definition
//
// (used by client-side CBodyFX and server-side Body classes)
//
/////////////////////////////////////////////////////////////////////////////
/*
struct BODYCREATESTRUCT : public SFXCREATESTRUCT
{
	BODYCREATESTRUCT();

    virtual void Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite);
    virtual void Read(ILTCSBase *pInterface, HMESSAGEREAD hRead);

//	ModelId			eModelId;
//	ModelSkeleton	eModelSkeleton;
//	ModelType		eModelType;
//	ModelStyle		eModelStyle;
//  uint8           nTrackers;
	BodyState		eBodyState;
	uint8			nClientId;
};

inline BODYCREATESTRUCT::BODYCREATESTRUCT()
{
//	eModelId		= eModelIdInvalid;
//	eModelSkeleton	= eModelSkeletonInvalid;
//	eModelType		= eModelTypeInvalid;
//	eModelStyle		= eModelStyleInvalid;
//	nTrackers		= 0;
	eBodyState		= eBodyStateNormal;
	nClientId		= (uint8)-1;

}
*/
/////////////////////////////////////////////////////////////////////////////
//
// STEAMCREATESTRUCT class Definition
//
// (used by client-side CSteamFX and server-side Steam classes)
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _CLIENTBUILD

#ifdef _WIN32
#define ADD_STEAM_PROPS() \
	ADD_REALPROP_FLAG(Range, 200.0f, 0) \
	ADD_REALPROP_FLAG(VolumeRadius, 10.0f, 0) \
	ADD_REALPROP_FLAG(ParticleVelocity, 75.0f, 0) \
	ADD_REALPROP_FLAG(ParticleRadius, 10000.0f, 0) \
	ADD_STRINGPROP_FLAG(Particle, "SFX\\Impact\\Spr\\Smoke.spr", PF_FILENAME) \
	ADD_REALPROP_FLAG(CreateDelta, 0.2f, 0) \
	ADD_LONGINTPROP_FLAG(NumParticles, 2, 0) \
	ADD_REALPROP_FLAG(StartAlpha, 0.5f, 0) \
	ADD_REALPROP_FLAG(EndAlpha, 0.0f, 0) \
	ADD_REALPROP_FLAG(StartScale, 1.0f, 0) \
	ADD_REALPROP_FLAG(EndScale, 2.0f, 0) \
	ADD_VECTORPROP_VAL(MinDriftVel, 0.0f, 0.0f ,0.0f) \
	ADD_VECTORPROP_VAL(MaxDriftVel, 0.0f, 20.0f ,0.0f) \
	ADD_COLORPROP(Color1, 255, 255 ,255) \
	ADD_COLORPROP(Color2, 255, 255 ,255) \
	ADD_REALPROP_FLAG(SoundRadius, 300.0f, PF_RADIUS) \
	ADD_STRINGPROP_FLAG(SoundName, "Snd\\Event\\steam.wav", PF_FILENAME)
#else
#define ADD_STEAM_PROPS() \
	ADD_REALPROP_FLAG(Range, 200.0f, 0) \
	ADD_REALPROP_FLAG(VolumeRadius, 10.0f, 0) \
	ADD_REALPROP_FLAG(ParticleVelocity, 75.0f, 0) \
	ADD_REALPROP_FLAG(ParticleRadius, 10000.0f, 0) \
	ADD_STRINGPROP_FLAG(Particle, "SFX/Impact/Spr/Smoke.spr", PF_FILENAME) \
	ADD_REALPROP_FLAG(CreateDelta, 0.2f, 0) \
	ADD_LONGINTPROP_FLAG(NumParticles, 2, 0) \
	ADD_REALPROP_FLAG(StartAlpha, 0.5f, 0) \
	ADD_REALPROP_FLAG(EndAlpha, 0.0f, 0) \
	ADD_REALPROP_FLAG(StartScale, 1.0f, 0) \
	ADD_REALPROP_FLAG(EndScale, 2.0f, 0) \
	ADD_VECTORPROP_VAL(MinDriftVel, 0.0f, 0.0f ,0.0f) \
	ADD_VECTORPROP_VAL(MaxDriftVel, 0.0f, 20.0f ,0.0f) \
	ADD_COLORPROP(Color1, 255, 255 ,255) \
	ADD_COLORPROP(Color2, 255, 255 ,255) \
	ADD_REALPROP_FLAG(SoundRadius, 300.0f, PF_RADIUS) \
	ADD_STRINGPROP_FLAG(SoundName, "Snd/Event/steam.wav", PF_FILENAME)
#endif // _WIN32

#endif

struct STEAMCREATESTRUCT : public SFXCREATESTRUCT
{
	STEAMCREATESTRUCT();
	~STEAMCREATESTRUCT();

    virtual void Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite);
    virtual void Read(ILTCSBase *pInterface, HMESSAGEREAD hRead);

	// Server-side only...

	virtual	void ReadProps();

    LTFLOAT  fRange;
    LTFLOAT  fVel;
    LTFLOAT  fSoundRadius;
    LTFLOAT  fParticleRadius;
    LTFLOAT  fStartAlpha;
    LTFLOAT  fEndAlpha;
    LTFLOAT  fStartScale;
    LTFLOAT  fEndScale;
    LTFLOAT  fCreateDelta;
    LTFLOAT  fVolumeRadius;
    uint8   nNumParticles;
	HSTRING	hstrSoundName;
	HSTRING	hstrParticle;
    LTVector vColor1;
    LTVector vColor2;
    LTVector vMinDriftVel;
    LTVector vMaxDriftVel;
};

inline STEAMCREATESTRUCT::STEAMCREATESTRUCT()
{
	fRange			= 0.0f;
	fVel			= 0.0f;
	fSoundRadius	= 0.0f;
	fParticleRadius	= 0.0f;
	fStartAlpha		= 0.0f;
	fEndAlpha		= 0.0f;
	fStartScale		= 0.0f;
	fEndScale		= 0.0f;
	fCreateDelta	= 0.0f;
	fVolumeRadius	= 0.0f;
	nNumParticles	= 0;
    hstrSoundName   = LTNULL;
    hstrParticle    = LTNULL;
	vColor1.Init();
	vColor2.Init();
	vMinDriftVel.Init();
	vMaxDriftVel.Init();
}


/////////////////////////////////////////////////////////////////////////////
//
// EXPLOSIONCREATESTRUCT class Definition
//
// (used by client-side CExplosionFX and server-side Explosion classes)
//
/////////////////////////////////////////////////////////////////////////////

struct EXPLOSIONCREATESTRUCT : public SFXCREATESTRUCT
{
	EXPLOSIONCREATESTRUCT();

    virtual void Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite);
    virtual void Read(ILTCSBase *pInterface, HMESSAGEREAD hRead);

    uint8       nImpactFX;
    LTFLOAT      fDamageRadius;
    LTVector     vPos;
    LTRotation   rRot;
	HOBJECT		 hFiredFrom; //only used if the FX is to be ignored by shooter
};

inline EXPLOSIONCREATESTRUCT::EXPLOSIONCREATESTRUCT()
{
	nImpactFX		= 0;
	fDamageRadius	= 0.0f;
	hFiredFrom		= LTNULL;
	vPos.Init();
	rRot.Init();
}

/////////////////////////////////////////////////////////////////////////////
//
// SPRINKLETYPECREATESTRUCT class Definition
//
// (used by client-side SprinklesFX and server-side Sprinkles classes)
//
/////////////////////////////////////////////////////////////////////////////

struct SPRINKLETYPECREATESTRUCT
{
    SPRINKLETYPECREATESTRUCT();
	~SPRINKLETYPECREATESTRUCT();

    virtual void Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite);
    virtual void Read(ILTCSBase *pInterface, HMESSAGEREAD hRead);

    HSTRING      m_hFilename;
    HSTRING      m_hSkinName;
    uint32       m_Count;
    float        m_Speed;
    float        m_Size;
    float        m_SpawnRadius;
    LTVector     m_AnglesVel;
    LTVector     m_ColorMin;
    LTVector     m_ColorMax;
};

inline SPRINKLETYPECREATESTRUCT::SPRINKLETYPECREATESTRUCT()
{
    m_hFilename     = LTNULL;
    m_hSkinName     = LTNULL;
	m_Count			= 0;
	m_Speed			= 0.0f;
	m_Size			= 0.0f;
	m_SpawnRadius	= 0.0f;
	m_AnglesVel.Init();
	m_ColorMin.Init();
	m_ColorMax.Init();
}

#define MAX_SPRINKLE_TYPES	8

/////////////////////////////////////////////////////////////////////////////
//
// SPRINKLESCREATESTRUCT class Definition
//
// (used by client-side SprinklesFX and server-side Sprinkles classes)
//
/////////////////////////////////////////////////////////////////////////////

struct SPRINKLESCREATESTRUCT : public SFXCREATESTRUCT
{
    SPRINKLESCREATESTRUCT();

    virtual void Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite);
    virtual void Read(ILTCSBase *pInterface, HMESSAGEREAD hRead);

    uint32                      m_nTypes;
	SPRINKLETYPECREATESTRUCT	m_Types[MAX_SPRINKLE_TYPES];

};

inline SPRINKLESCREATESTRUCT::SPRINKLESCREATESTRUCT()
{
	m_nTypes = 0;
}

/////////////////////////////////////////////////////////////////////////////
//
// LTCREATESTRUCT class Definition
//
// (used by client-side CLaserTriggerFX and server-side LaserTrigger classes)
//
/////////////////////////////////////////////////////////////////////////////

struct LTCREATESTRUCT : public SFXCREATESTRUCT
{
	LTCREATESTRUCT();
	~LTCREATESTRUCT();

    virtual void Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite);
    virtual void Read(ILTCSBase *pInterface, HMESSAGEREAD hRead);

    LTVector     vColor;
    LTVector     vDims;
    LTFLOAT      fAlpha;
    LTFLOAT      fSpriteScale;
    LTBOOL       bCreateSprite;
	HSTRING		hstrSpriteFilename;
};

inline LTCREATESTRUCT::LTCREATESTRUCT()
{
	vColor.Init();
	vDims.Init();
	fAlpha			= 0.0f;
	fSpriteScale	= 1.0f;
    bCreateSprite   = LTTRUE;
    hstrSpriteFilename = LTNULL;
}

/////////////////////////////////////////////////////////////////////////////
//
// MINECREATESTRUCT class Definition
//
// (used by client-side CMineFX and server-side Mine classes)
//
/////////////////////////////////////////////////////////////////////////////

struct MINECREATESTRUCT : public SFXCREATESTRUCT
{
	MINECREATESTRUCT();

    virtual void Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite);
    virtual void Read(ILTCSBase *pInterface, HMESSAGEREAD hRead);

    LTFLOAT  fMinRadius;
    LTFLOAT  fMaxRadius;
};

inline MINECREATESTRUCT::MINECREATESTRUCT()
{
	fMinRadius	= 0.0f;
	fMaxRadius	= 0.0f;
}


/////////////////////////////////////////////////////////////////////////////
//
// PVCREATESTRUCT class Definition
//
// (used by client-side CPlayerVehicleFX and server-side PlayerVehicle classes)
//
/////////////////////////////////////////////////////////////////////////////
/*
struct PVCREATESTRUCT : public SFXCREATESTRUCT
{
	PVCREATESTRUCT();

    virtual void Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite);
    virtual void Read(ILTCSBase *pInterface, HMESSAGEREAD hRead);

	PlayerPhysicsModel	ePhysicsModel;
};

inline PVCREATESTRUCT::PVCREATESTRUCT()
{
	ePhysicsModel = PPM_NORMAL;
}
*/
#endif  // __SHARED_FX_STRUCTS_H__

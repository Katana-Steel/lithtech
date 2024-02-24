// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterFX.h
//
// PURPOSE : Character special fx class - Definition
//
// CREATED : 8/24/98
//
// (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_FX_H__
#define __CHARACTER_FX_H__

#include "SpecialFX.h"
#include "SurfaceMgr.h"
#include "NodeController.h"
#include "ModelLT.h"
#include "SharedFXStructs.h"
#include "CharacterAnimationDefs.h"
#include "LensFlareFX.h"
#include "IKChain.h"


class CFlashLightAI;
class CLaserBeam;

#define MAX_ELEC_EFFECTS 4
#define MAX_CLOAK_EFFECTS 6
#define MAX_FLAME_SEGMENTS 11


typedef struct tagBleederData
{
	tagBleederData()
	{
		eModelNode		= eModelNodeInvalid;
		hBleeder		= LTNULL;
		fLastPoolTime	= 0.0f;
		fLastDripTime	= 0.0f;
		pFX				= LTNULL;
		vLastPos.Init();
	}

	ModelNode		eModelNode;
	HOBJECT			hBleeder;
	LTFLOAT			fLastPoolTime;
	LTFLOAT			fLastDripTime;
	CCharacterFX*	pFX;
	LTVector		vLastPos;
} BleederData;

typedef std::vector<BleederData*> BleederList;


typedef struct tagElecEffectData
{
	tagElecEffectData()
	{
		memset(eModelNode,0,sizeof(ModelNode)*MAX_ELEC_EFFECTS);
		fStartTime = 0.0f;
	}

	ModelNode	eModelNode[MAX_ELEC_EFFECTS];
	LTFLOAT		fStartTime;
} ElecEffectData;

typedef struct tagCloakEffectData
{
	tagCloakEffectData()
	{
		memset(eModelNode,0,sizeof(ModelNode)*MAX_CLOAK_EFFECTS);
		fStartTime = 0.0f;
		fEffectStartTime = 0.0f;
	}

	ModelNode	eModelNode[MAX_CLOAK_EFFECTS];
	LTFLOAT		fStartTime;
	LTFLOAT		fEffectStartTime;
} CloakEffectData;


typedef struct tagCloakNodeList
{
	struct tagCloakNode
	{
		ModelNode	eModelNode;
		LTFLOAT		fHeight;
	};

	struct CloakCompareFunction
	{
		inline bool operator()(const tagCloakNode& lhs, const tagCloakNode& rhs)
		{
			return lhs.fHeight > rhs.fHeight;
		}
	};

	tagCloakNodeList()
	{
		memset(NodeArray,0,sizeof(tagCloakNode)*40);
		nFirstNode = 0;
		fStartTime = 0.0f;
		fEffectStartTime = 0.0f;
	}

	tagCloakNode	NodeArray[40];
	uint32			nFirstNode;
	LTFLOAT			fStartTime;
	LTFLOAT			fEffectStartTime;
} CloakNodeList;

typedef struct tagSpriteData
{
	tagSpriteData() 
	{
		bResolved	= LTFALSE;
		bHide		= LTFALSE;
		vStart.Init();
		vPos.Init();
		vNorm.Init();
	}
	LTBOOL		bResolved;
	LTBOOL		bHide;
	LTVector	vPos, vNorm, vStart;
} SpriteData;


class CCharacterFX : public CSpecialFX
{
	public :

		CCharacterFX() : CSpecialFX() 
		{
			m_pBubbles			= LTNULL;
			m_pLaser			= LTNULL;
			m_pNapalm			= LTNULL;
			m_fNextBubbleTime	= -1.0f;
			m_bLeftFoot			= LTTRUE;
			m_fLastFootFallTime = 0;
			m_hAuraSystem		= LTNULL;
			m_hFireSystem[0]	= LTNULL;
			m_hFireSystem[1]	= LTNULL;
			m_hFireSystem[2]	= LTNULL;
			m_pFlashlight		= LTNULL;
			m_hFireSound		= LTNULL;
			m_hHeatAuraSystem	= LTNULL;
			m_hElecAuraSystem	= LTNULL;
			m_hCloakAuraSystem	= LTFALSE;
			m_bTargetingSprite	= LTFALSE;
			m_hFlare			= LTNULL;
			m_bIsDead			= LTFALSE;
			m_bAlphaOverride	= LTFALSE;
			m_fLastFlameImpactTime = 0.0f;
			m_fLastFlameMovementTime = 0.0f;
			m_fLastFlameFlareUpTime = 0.0f;
			m_bCanResetFlame	= LTTRUE;
			m_hChestBurstModel	= LTNULL;

			memset(m_hFlameEmitters, 0, MAX_FLAME_SEGMENTS * sizeof(HOBJECT));
			memset(m_fFlameStartTimes, 0, MAX_FLAME_SEGMENTS * sizeof(LTFLOAT));
			memset(m_fFlameEndTimes, 0, MAX_FLAME_SEGMENTS * sizeof(LTFLOAT));

			memset(m_hTargetingSprites, 0, 3 * sizeof(HOBJECT));

			m_vEyeFlashColor.Init(0.0f, 0.0f, 0.0f);
			m_fEyeFlashTime		= 0.0f;

			memset(m_szInfoString, 0, 256);
			m_bMoving	=	LTFALSE;
			m_vLastPos.Init();

			m_hBossSmokeSystem	= LTNULL;
			m_nBossSmokeLevel	= 0;
			m_fLastBossSmokeTime= 0.0f;
			m_vLastBossSmokePos.Init();
			m_fLastSparkTime	= 0.0f;
			m_hWeaponLoopSound	= LTNULL;
		}

		~CCharacterFX();

		void PlayFootstepSound(LTVector vPos, SurfaceType eSurface, LTBOOL bLeftFoot);

		virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage);
		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CreateObject(CClientDE* pClientDE);
		virtual LTBOOL Update();
		virtual void  OnModelKey(HLOCALOBJ hObj, ArgList *pArgs);
		virtual	LTBOOL OnServerMessage(HMESSAGEREAD hMessage);
		virtual void OnObjectRemove(HOBJECT hObj);
		virtual void SetMode(const std::string& mode);

		virtual void WantRemove(LTBOOL bRemove=LTTRUE);

		LTBOOL	IsMoving()	{ return m_bMoving; }

		void ForceFootstep() { DoFootStepKey(m_hServerObject, LTTRUE); }
		

		ModelSkeleton	GetModelSkeleton() const { return m_cs.eModelSkeleton; }

		LTAnimTracker		m_aAnimTrackers[ANIM_TRACKER_MAX];	// Our pre-allocated anim trackers
		CHARCREATESTRUCT	m_cs;

		const char* GetInfoString()			{ return m_szInfoString; }
		void ResetFlameThrowerFX();

	protected :

		void CreateUnderwaterFX(LTVector & vPos);
		void UpdateUnderwaterFX(LTVector & vPos);

		void UpdateNapalmFX(LTVector vPos);
		void RemoveNapalmFX();
		void CreateNapalmFX(LTVector vPos);

		void UpdateLaserFX();
		void RemoveLaserFX();

		void CreateAuraFX();
		void UpdateAuraFX();

		void CreateHeatAuraFX();
		void UpdateHeatAuraFX();

		void CreateElecAuraFX();
		void UpdateElecAuraFX();

		void CreateCloakAuraFX();
		void UpdateCloakAuraFX();
		void BuildCloakNodeList();

		void CreateChestBurstFX();
		void UpdateChestBurstFX();

		void CreateTargetingSprites();
		void UpdateTargetingSprite();
		void UpdateFlare();

		void CreateFlashlight();
		LTBOOL GetNodePos(const char* szNodeName, LTVector &vPos, LTBOOL bWorldPos=LTFALSE);
		LTVector GetAuraColor(LTBOOL bAutoTargeted=LTFALSE);

		void CreateFireFX();
		void UpdateFireFX();

		void CreateFlameThrowerFX();
		void UpdateFlameThrowerFX();
		LTBOOL TestForImpact(IntersectInfo& IInfo);
		void UpdateEmitterPositions();
		void UpdateEmitterImpact(LTFLOAT fTime);
		void UpdateEmitterFX(LTFLOAT fTime);
		void UpdateEmitterFlareUp(LTFLOAT fTime);

		void DoFootStepKey(HLOCALOBJ hObj, LTBOOL bForceSound=LTFALSE);
		void CreateFootprint(SurfaceType eType, IntersectInfo & iInfo);
		void GetFootStepSound(char* szSound, SurfaceType eSurfType, LTBOOL bLeftFoot);

		void OptimizedResolve(SpriteData* pData, IntersectInfo* pInfo, LTVector& vF, LTVector& vCamPos);
		void SingleResolve(SpriteData* pData, LTVector& vF, LTVector& vCamPos);
		void SetSprites(SpriteData* pData, LTVector& vCamF);
		void HideTargetingSprite();
		void CheckForBleederRemoval();
		void AddNewBleeder(ModelNode eNode);
		void AddBleederParticles(BleederData* pData);
		void UpdateBleeders();
		void AddBleederPool(BleederData* pData);

		void ResetSpecialFX();
		void HandleMessage_WeaponLoopSound(HMESSAGEREAD hRead);

		HOBJECT				m_hFireSystem[3];		// Fire fx
		HOBJECT				m_hAuraSystem;		// Aura fx
		HOBJECT				m_hHeatAuraSystem;	// Heat Aura fx
		HOBJECT				m_hElecAuraSystem;	// Electro Aura fx
		HOBJECT				m_hCloakAuraSystem;	// Cloak Aura fx
		HOBJECT				m_hTargetingSprites[3]; // Predator targeting sprites
		HOBJECT				m_hChestBurstModel;		// Fleshy chest burst model
		CSpecialFX*			m_pBubbles;			// Bubbles fx
		CLaserBeam*			m_pLaser;			// Laser fx
		CSpecialFX*			m_pNapalm;
		CFlashLightAI*		m_pFlashlight;
		LTFLOAT				m_fNextBubbleTime;
		LTBOOL				m_bLeftFoot;
		LTFLOAT				m_fLastFootFallTime;
		HLTSOUND			m_hFireSound;
		HLTSOUND			m_hWeaponLoopSound;
		LTBOOL				m_bTargetingSprite;
		LENSFLARECREATESTRUCT m_LFcs;
		HOBJECT				m_hFlare;
		ElecEffectData		m_ElecEffectData;
		CloakNodeList		m_CloakNodeData;
		LTBOOL				m_bIsDead;

		CNodeController		m_NodeController;	// Our node controller

		LTBOOL				m_bAlphaOverride;				// Do we override the model's alpha?

		LTVector			m_vEyeFlashColor;
		LTFLOAT				m_fEyeFlashTime;

		BleederList			m_BleederList;		// Our blood emmiters
		
		HOBJECT				m_hFlameEmitters[MAX_FLAME_SEGMENTS];
		LTFLOAT				m_fFlameStartTimes[MAX_FLAME_SEGMENTS];
		LTFLOAT				m_fFlameEndTimes[MAX_FLAME_SEGMENTS];
		LTFLOAT				m_fLastFlameImpactTime;
		LTFLOAT				m_fLastFlameMovementTime;
		LTFLOAT				m_fLastFlameFlareUpTime;
		LTBOOL				m_bCanResetFlame;
		IKChain_Constrained	m_FlameIKChain;
		LTVector			m_vLastPos;
		LTBOOL				m_bMoving;

		char				m_szInfoString[256];
		SurfaceType			m_eSurfaceType;

		// Sepcial Boss Character FX
		HOBJECT				m_hBossSmokeSystem;		// Boss fight smoke FX
		uint8				m_nBossSmokeLevel;		// Boss fight smoke intensity
		LTFLOAT				m_fLastBossSmokeTime;	// Last time we made smoke
		LTVector			m_vLastBossSmokePos;	// Last position of our system
		LTFLOAT				m_fLastSparkTime;		// Last time we made sparks

		void CreateBossFX();
		void UpdateBossFX();
		void AddBossSmokeParticles();
};

//-------------------------------------------------------------------------------------------------
// SFX_CharacterFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_CharacterFactory : public CSpecialFXFactory
{
	SFX_CharacterFactory() : CSpecialFXFactory(SFX_CHARACTER_ID) {;}
	static const SFX_CharacterFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // CHARCREATESTRUCT
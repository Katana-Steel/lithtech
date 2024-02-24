// ----------------------------------------------------------------------- //
//
// MODULE  : BodyPropFX.h
//
// PURPOSE : Body Prop special fx class - Definition
//
// CREATED : 7/11/2000
//
// (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BODY_PROP_FX_H__
#define __BODY_PROP_FX_H__

#include "SpecialFX.h"
#include "SharedFXStructs.h"
#include "NodeController.h"
#include "Fxbutemgr.h"

struct IMPACTFX;

typedef struct tagBodyBleederData
{
	tagBodyBleederData()
	{
		eModelNode		= eModelNodeInvalid;
		hBleeder		= LTNULL;
		fLastDripTime	= 0.0f;
		pFX				= LTNULL;
		vLastPos.Init();
		pTrail			= LTNULL;
		fStartTime		= 0.0f;
	}

	ModelNode		eModelNode;
	HOBJECT			hBleeder;
	LTFLOAT			fLastDripTime;
	BodyPropFX*		pFX;
	LTVector		vLastPos;
	PARTICLETRAILFX*pTrail;
	LTFLOAT			fStartTime;
} BodyBleederData;

typedef std::vector<BodyBleederData*> BodyBleederList;


class BodyPropFX : public CSpecialFX
{
	public :

		BodyPropFX() : CSpecialFX()
		{
			m_hAuraSystem	= LTNULL;
			m_hFireSystem[0]	= LTNULL;
			m_hFireSystem[1]	= LTNULL;
			m_hFireSystem[2]	= LTNULL;
			m_hChestBurstModel = LTNULL;
			m_hFireSound	= LTNULL;
			m_bBloodPoolMade= LTFALSE;
			m_hBloodPoolPoly= INVALID_HPOLY;
			m_pImpactFX		= LTNULL;
			m_fLastSteamTime= 0.0f;
		} 

		~BodyPropFX();

		BODYPROPCREATESTRUCT	m_cs;

		virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CreateObject(CClientDE* pClientDE);
		virtual LTBOOL Update();

		virtual	LTBOOL OnServerMessage(HMESSAGEREAD hMessage);

		virtual void WantRemove(LTBOOL bRemove=LTTRUE);

		ModelSkeleton GetModelSkeleton() const { return m_cs.eModelSkeleton; }

		void	SetMode(const std::string& mode);

	private:

		void CreateAuraFX();
		void UpdateAuraFX();

		void CreateFireFX();
		void UpdateFireFX();

		void CreateChestBurstFX();
		void UpdateChestBurstFX();

		void UpdateBloodPool();

		void ShowAllPieces();

		void CheckForBleederRemoval();
		void AddNewBleeder(ModelNode eNode);
		void UpdateBleeders();
		void AddBleederParticles(BodyBleederData* pData);

		LTBOOL		GetNodePos(const char* szNodeName, LTVector &vPos, LTBOOL bWorldPos=LTFALSE, LTRotation* pRot=LTNULL);
		LTVector	GetAuraColor(LTBOOL bAutoTargeted=LTFALSE);

		HOBJECT			m_hAuraSystem;			// Aura fx
		HOBJECT			m_hFireSystem[3];			// Aura fx
		HOBJECT			m_hChestBurstModel;		// Fleshy chest burst model
		HLTSOUND		m_hFireSound;			// The on fire sound
		HPOLY			m_hBloodPoolPoly;		// The poly to clip the blood pool to
		IMPACTFX*		m_pImpactFX;			// The steam FX for the alien blood
		LTFLOAT			m_fLastSteamTime;		// The last time we made steam
		IFXCS			m_ifxcs;				// The steam impact FX struct
		LTBOOL			m_bBloodPoolMade;		// Have we made our pool yet?
		BodyBleederList	m_BleederList;		// Our blood emmiters

		CNodeController		m_NodeController;	// Our node controller
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

class SFX_BodyPropFactory : public CSpecialFXFactory
{
	SFX_BodyPropFactory() : CSpecialFXFactory(SFX_BODYPROP_ID) {;}
	static const SFX_BodyPropFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __BODY_PROP_FX_H__


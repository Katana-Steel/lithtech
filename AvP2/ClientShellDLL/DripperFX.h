// ----------------------------------------------------------------------- //
//
// MODULE  : DripperFX.h
//
// PURPOSE : Dripper special fx class - Definition
//
// CREATED : 1/6/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DRIPPER_FX_H__
#define __DRIPPER_FX_H__

// ----------------------------------------------------------------------- //

#include "SpecialFX.h"
#include "TemplateList.h"
#include "FXButeMgr.h"

// ----------------------------------------------------------------------- //

#define MAX_DRIP_PARTICLES			64
#define MAX_DRIPS					128

#define DRIP_STATE_NONE				0
#define DRIP_STATE_DRIPPING			1
#define DRIP_STATE_FALLING			2

// ----------------------------------------------------------------------- //

struct DRIPFXCREATESTRUCT : public SFXCREATESTRUCT
{
    DRIPFXCREATESTRUCT();

	uint8		nDripEffect;
	LTVector	vDims;
};

// ----------------------------------------------------------------------- //

inline DRIPFXCREATESTRUCT::DRIPFXCREATESTRUCT()
{
	nDripEffect		= -1;
	vDims.Init();
}

// ----------------------------------------------------------------------- //

struct DripStruct
{
	DripStruct();

	LTParticle	*pParticles[MAX_DRIP_PARTICLES];
	int			nParticles;

	LTVector	vColor;
	LTFLOAT		fSize;
	LTFLOAT		fStartTime;
	LTFLOAT		fDripTime;
	int			nState;
};

inline DripStruct::DripStruct()
{
	memset(this, 0, sizeof(DripStruct));
}

// ----------------------------------------------------------------------- //

class CDripperFX : public CSpecialFX
{
	public:

		CDripperFX() : CSpecialFX()
		{
			m_pDripperFX = LTNULL;
		}

		~CDripperFX()
		{

		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
		virtual	LTBOOL OnServerMessage(HMESSAGEREAD hMessage);

        virtual LTBOOL Update();

	protected:

		void CreateDrip();
		void RemoveDrip(int nDrip);
		void UpdateDrips();

		void UpdateDripping(int nDrip);
		void UpdateFalling(int nDrip);

		void PlayImpactFX(LTFLOAT fR, LTFLOAT fF);

	protected:

		// Information about the effect
		DRIPPEROBJFX	*m_pDripperFX;
		IMPACTFX		*m_pImpactFX;

		// Data we need to keep track of
		LTVector		m_vServObjDims;

		LTFLOAT			m_fLastDripTime;
		LTFLOAT			m_fDripDelay;

		// Drip list data
		DripStruct		m_pDrips[MAX_DRIPS];
		int				m_nNumDrips;
};

//-------------------------------------------------------------------------------------------------
// SFX_DripperFactory
//-------------------------------------------------------------------------------------------------

#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_DripperFactory : public CSpecialFXFactory
{
	SFX_DripperFactory() : CSpecialFXFactory(SFX_DRIPPER_ID) {;}
	static const SFX_DripperFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

// ----------------------------------------------------------------------- //

#endif // __DRIPPER_FX_H__

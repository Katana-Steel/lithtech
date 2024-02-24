// ----------------------------------------------------------------------- //
//
// MODULE  : CameraFX.h
//
// PURPOSE : Camera special fx class - Definition
//
// CREATED : 5/20/98
//
// ----------------------------------------------------------------------- //

#ifndef __CAMERA_FX_H__
#define __CAMERA_FX_H__

#include "SpecialFX.h"

struct CAMCREATESTRUCT : public SFXCREATESTRUCT
{
	CAMCREATESTRUCT();

	LTBOOL		bAllowPlayerMovement;
	LTBOOL		bWidescreen;
	uint8		nXFOV;
	uint8		nYFOV;
	LTBOOL		bIsListener;
};

inline CAMCREATESTRUCT::CAMCREATESTRUCT()
{
	bAllowPlayerMovement	= LTFALSE;
	bWidescreen				= LTFALSE;
	nXFOV					= 75;
	nYFOV					= 60;
	bIsListener				= LTFALSE;
}

class CCameraFX : public CSpecialFX
{
	public :

		CCameraFX() : CSpecialFX()
		{
			m_bAllowPlayerMovement  = LTFALSE;
			m_bWidescreen			= LTFALSE;
			m_nXFOV					= 75;
			m_nYFOV					= 60;
			m_bIsListener           = LTFALSE;
		}

		virtual LTBOOL Update() { return !m_bWantRemove; }

		LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);

		LTBOOL		AllowPlayerMovement()	const { return m_bAllowPlayerMovement; }
		LTBOOL		IsWidescreen()			const { return m_bWidescreen; }
		LTBOOL		IsListener()			const { return m_bIsListener; }
		uint8		GetXFOV()				const { return m_nXFOV; }
		uint8		GetYFOV()				const { return m_nYFOV; }

	protected :

		LTBOOL		m_bAllowPlayerMovement;
		LTBOOL		m_bWidescreen;
		uint8		m_nXFOV;
		uint8		m_nYFOV;
		LTBOOL		m_bIsListener;
};


//-------------------------------------------------------------------------------------------------
// SFX_CameraFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_CameraFactory : public CSpecialFXFactory
{
	SFX_CameraFactory() : CSpecialFXFactory(SFX_CAMERA_ID) {;}
	static const SFX_CameraFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};


#endif // __CAMERA_FX_H__


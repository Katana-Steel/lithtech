// ----------------------------------------------------------------------- //
//
// MODULE  : FlashLight.h
//
// PURPOSE : FlashLight class - Definition
//
// CREATED : 07/21/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __FLASH_LIGHT_H__
#define __FLASH_LIGHT_H__

#include "ltbasedefs.h"
#include "PolyLineFX.h"
#include "LensFlareFX.h"

class CFlashLight
{
	public :

		CFlashLight();
		virtual ~CFlashLight();

		virtual void Init(HOBJECT hServerObject);

		virtual void Update();
		void	UpdateFlare(LTVector vFlarePos, LTVector vFlareF);

		virtual void Toggle()	{ (m_bOn ? TurnOff() : TurnOn());}
		virtual void TurnOn();
		virtual void TurnOff();

		virtual LTBOOL IsOn() const { return m_bOn; }

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

	protected :

		virtual void CreateLight();

		virtual void GetLightPositions(LTVector & vStartPos, LTVector & vEndPos, LTVector & vUOffset, LTVector & vROffset, LTVector& vF, LTVector& vR) = 0;

		HOBJECT GetServerObject() { return m_hServerObject; }
		HMODELSOCKET GetSocket()  { return m_hLightSocket; }

		LTBOOL			m_bLocalPlayer;

	private :

		HOBJECT			m_hServerObject;
		HMODELSOCKET	m_hLightSocket;

        LTBOOL          m_bOn;
		HOBJECT			m_hLight;
		HOBJECT			m_hSrcLight;
		CPolyLineFX		m_LightBeam;

		HLTSOUND		m_hSound;

		int				m_nFlickerPattern;
		LTFLOAT			m_fFlickerTime;

		LTFLOAT			m_fLastDist;

        LTFLOAT         m_fServerUpdateTimer;
		LENSFLARECREATESTRUCT m_cs;
		HOBJECT			m_hFlare;
};

class CFlashLightPlayer : public CFlashLight
{
	protected :

		void GetLightPositions(LTVector & vStartPos, LTVector & vEndPos, LTVector & vUOffset, LTVector & vROffset, LTVector& vF, LTVector& vR);
};

class CFlashLightAI : public CFlashLight
{
	public :

		CFlashLightAI()
			: m_rAimingRot(0,0,0,0) {}

		void Update();

		void SetRotation(const LTRotation & rRot) { m_rAimingRot = rRot; }

	protected :

		void GetLightPositions(LTVector & vStartPos, LTVector & vEndPos, LTVector & vUOffset, LTVector & vROffset, LTVector& vF, LTVector& vR);

	private :

		LTRotation m_rAimingRot;
};

#endif // __FLASH_LIGHT_H__
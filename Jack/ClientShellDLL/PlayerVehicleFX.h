// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerVehicleFX.h
//
// PURPOSE : Tracer special fx class - Definition
//
// CREATED : 6/8/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_VEHICLE_FX_H__
#define __PLAYER_VEHICLE_FX_H__

#include "SpecialFX.h"
#include "SharedFXStructs.h"

class CPlayerVehicleFX : public CSpecialFX
{
	public :

		CPlayerVehicleFX() : CSpecialFX()
		{
		}
		~CPlayerVehicleFX();

        virtual LTBOOL Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();

		virtual LTBOOL	OnServerMessage(ILTMessage_Read *pMsg);

		virtual uint32 GetSFXID() { return SFX_PLAYERVEHICLE_ID; }

	protected :
		void	StartBlink();
		void	UpdateBlink();
		void	ClearBlink();
		void	Respawn(LTVector& vOldPos, LTVector& vNewPos );

		PVCREATESTRUCT	m_cs;
		bool			m_bBlinking;
		bool			m_bFadeIn;
		float			m_fAlpha;

		CLIENTFX_LINK m_ClientFXLink;
};

#endif // __PLAYER_VEHICLE_FX_H__
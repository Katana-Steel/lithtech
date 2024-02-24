// ----------------------------------------------------------------------- //
//
// MODULE  : ChassisPiece.h
//
// PURPOSE : ChassisPiece - Definition
//
// CREATED : 5/12/03
//
// ----------------------------------------------------------------------- //

#ifndef __CHASSISPIECE_H__
#define __CHASSISPIECE_H__

#include "SpecialFX.h"
#include "ActivateTypeMgr.h"

struct CHASSISPIECECREATESTRUCT : public SFXCREATESTRUCT
{
    CHASSISPIECECREATESTRUCT();

	ChassisPieceBute* pChassisPieceBute;
	bool			bCarried;
	uint8			nTeam;
	bool			bPlanted;
	uint8			nActivateId;
	bool			bBlinking;
};

inline CHASSISPIECECREATESTRUCT::CHASSISPIECECREATESTRUCT()
{
    pChassisPieceBute = NULL;
	bCarried = false;
	nTeam = INVALID_TEAM;
	bPlanted = false;
	nActivateId = ATMGR_INVALID_ID;
	bBlinking = false;
}

class ChassisPieceFX : public CSpecialFX
{
	public :

		~ChassisPieceFX();

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_CHASSISPIECE_ID; }

		virtual LTBOOL OnServerMessage(ILTMessage_Read *pMsg);

		ChassisPieceBute const* GetChassisPieceBute( ) const { return m_pChassisPieceBute; }
		bool		IsCarried() const { return m_bCarried; }
		uint8		GetTeam()  const { return m_nTeam; }
		bool		IsPlanted() const { return m_bPlanted; }
		uint8		GetActivateId( ) const { return m_nActivateId; }

	private :
		void	StartBlink();
		void	UpdateBlink();
		void	ClearBlink();
		void	Respawn(LTVector& vOldPos, LTVector& vNewPos );
		CLIENTFX_LINK m_ClientFXLink;


        ChassisPieceBute* m_pChassisPieceBute;
		bool			m_bCarried;
		uint8			m_nTeam;
		bool			m_bPlanted;
		uint8			m_nActivateId;
		bool			m_bBlinking;
		float			m_fAlpha;
		float			m_fFadeSpeed;
};

#endif // __CHASSISPIECE_H__
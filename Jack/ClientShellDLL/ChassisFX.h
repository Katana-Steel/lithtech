// ----------------------------------------------------------------------- //
//
// MODULE  : ChassisFX.h
//
// PURPOSE : ChassisFX - Definition
//
// CREATED : 6/30/03
//
// ----------------------------------------------------------------------- //

#ifndef __CHASSISFX_H__
#define __CHASSISFX_H__

#include "SpecialFX.h"
#include "ActivateTypeMgr.h"

struct CHASSISCREATESTRUCT : public SFXCREATESTRUCT
{
    CHASSISCREATESTRUCT();

	uint8			nTeam;
	float			fDropZoneRadius;
	uint8			nActivateId;
};

inline CHASSISCREATESTRUCT::CHASSISCREATESTRUCT()
{
	nTeam = INVALID_TEAM;
	fDropZoneRadius = 64.0f;
	nActivateId = ATMGR_INVALID_ID;
}

class ChassisFX : public CSpecialFX
{
	public :

		~ChassisFX();

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_CHASSIS_ID; }

		virtual LTBOOL OnServerMessage(ILTMessage_Read *pMsg);

		uint8		GetTeam()  const { return m_nTeam; }
		float		GetDropZoneRadius( ) const { return m_fDropZoneRadius; }
		uint8		GetActivateId( ) const { return m_nActivateId; }

	private :

		uint8			m_nTeam;
		float			m_fDropZoneRadius;
		uint8			m_nActivateId;
};

#endif // __CHASSISFX_H__
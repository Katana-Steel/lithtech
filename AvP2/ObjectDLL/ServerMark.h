// ----------------------------------------------------------------------- //
//
// MODULE  : CServerMark.h
//
// PURPOSE : CServerMark definition - Server-side mark fx
//
// CREATED : 1/15/99
//
// ----------------------------------------------------------------------- //

#ifndef __SERVER_MARK_H__
#define __SERVER_MARK_H__

#include "cpp_engineobjects_de.h"
#include "ClientWeaponSFX.h"
#include "ltsmartlink.h"

class CServerMark : public BaseClass
{
	public :

		CServerMark();
		virtual ~CServerMark();

		void Setup(CLIENTWEAPONFX & theStruct);
		
	protected :

		LTSmartLink	m_AttachmentObj;

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
};

#endif // __SERVER_MARK_H__


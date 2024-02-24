// ----------------------------------------------------------------------- //
//
// MODULE  : Fire.h
//
// PURPOSE : Fire - Definition
//
// CREATED : 5/6/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __FIRE_H__
#define __FIRE_H__

#include "ClientSFX.h"
#include "SFXMsgIds.h"
#include "FXButeMgr.h"

class Fire : public CClientSFX
{
	public :

		Fire();
		~Fire();

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);
		void	HandleMsg(HOBJECT hSender, HSTRING hMsg);

	private :

        LTBOOL  m_bOn;
		uint8	m_nFireObjFXId;

        void    Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void    Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

        LTBOOL  ReadProp(ObjectCreateStruct *pStruct);
		void	InitialUpdate(int nInfo);
};

// ----------------------------------------------------------------------- //

class CFirePlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT	PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength);

	private:

		CFXButeMgrPlugin	m_FXButeMgrPlugin;
};

#endif // __FIRE_H__

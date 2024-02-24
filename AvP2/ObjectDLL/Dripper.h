// ----------------------------------------------------------------------- //
//
// MODULE  : Dripper.h
//
// PURPOSE : Dripper - Definition
//
// CREATED : 1/5/01
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DRIPPER_H__
#define __DRIPPER_H__

#include "ClientSFX.h"
#include "SFXMsgIds.h"
#include "iobjectplugin.h"
#include "FXButeMgr.h"

class Dripper : public CClientSFX
{
	public:

		Dripper();
		~Dripper();

	protected:

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		void	HandleMsg(HOBJECT hSender, HSTRING hMsg);

	private:

		LTBOOL		m_bFirstUpdate;
        LTBOOL		m_bOn;

		LTVector	m_vDims;
		uint8		m_nDripEffect;

        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

        LTBOOL ReadProp(ObjectCreateStruct *pStruct);

		void InitialUpdate();
		void Update();
};

// ----------------------------------------------------------------------- //

class CDripperPlugin : public IObjectPlugin
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

// ----------------------------------------------------------------------- //

#endif // __DRIPPER_H__

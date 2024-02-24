// ----------------------------------------------------------------------- //
//
// MODULE  : ExitTrigger.h
//
// PURPOSE : ExitTrigger - Definition
//
// CREATED : 11/9/2000
//
// ----------------------------------------------------------------------- //

#ifndef __EXIT_TRIGGER_H__
#define __EXIT_TRIGGER_H__

#include "Trigger.h"
#include "MissionMgr.h"

class ExitTrigger : public Trigger
{
	public :

		ExitTrigger();
		~ExitTrigger();

	protected :

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual LTBOOL	Activate();

	private :

		void Update();

		void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		LTBOOL	ReadProp(ObjectCreateStruct *pData);
		void	PostPropRead(ObjectCreateStruct *pData);

		uint32	m_nMissionData;
};

////////////////////////////////////////////////////////////////////////////
//
// CExitTrigerPlugin is used to help facilitate populating the DEdit object
// properties that use MissionMgr
//
////////////////////////////////////////////////////////////////////////////

#ifndef _CLIENTBUILD

#include "iobjectplugin.h"

class CExitTrigerPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT	PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength);

	protected :

			CMissionButeMgrPlugin sm_ButeMgr;
};

#endif // _CLIENTBUILD

#endif // __EXIT_TRIGGER_H__

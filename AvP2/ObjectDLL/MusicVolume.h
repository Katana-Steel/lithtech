// ----------------------------------------------------------------------- //
//
// MODULE  : MusicVolume.h
//
// PURPOSE : MusicVolume definition
//
// CREATED : 3/25/01
//
// (c) 1998-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MUSICVOLUME_H__
#define __MUSICVOLUME_H__

#include "cpp_engineobjects_de.h"
#include "MusicMgr.h"
#include "GameBase.h"
#include "iobjectplugin.h"

class CMusicVolumePlugin : public IObjectPlugin
{
	public:

		virtual DRESULT	PreHook_EditStringList(const char* szRezPath, const char* szPropName, char* const * aszStrings, DDWORD* pcStrings, const DDWORD cMaxStrings, const DDWORD cMaxStringLength);

	private :
		
};

class MusicVolume : public GameBase
{
	public :

		MusicVolume();
		~MusicVolume();

        uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	protected :

		uint32			EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);

	private :

		void			ReadProp(ObjectCreateStruct *pStruct);
		void			AffectPhysics(ContainerPhysics* pCPStruct);
		void			PostPropRead(ObjectCreateStruct *pStruct);
		void			InitialUpdate();
		void			Update();

		void TriggerMsg(HOBJECT hSender, const char* pMsg);
		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		// The mood to modify the music to when player is in volume.
		CMusicMgr::EMood	m_eMood;

		// The mood's trackset to use.
		std::string			m_sTrackSet;

		// If true, the mood won't be modified.
		LTBOOL				m_bLocked;
};

#endif // __MUSICVOLUME_H__

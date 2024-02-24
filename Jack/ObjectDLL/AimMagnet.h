// ----------------------------------------------------------------------- //
//
// MODULE  : AimMagnet.cpp
//
// PURPOSE : Invisible object that be used as an potential target by the auto-aim system
//
// CREATED : 3/11/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#ifndef __AIMMAGNET_H__
#define __AIMMAGNET_H__

#include "GameBase.h"
#include "ClientServerShared.h"
#include "CommandMgr.h"

LINKTO_MODULE( AimMagnet );

class AimMagnet : public GameBase
{
	public :

		AimMagnet();
		~AimMagnet();

		void			SetTeamId( uint8 nTeamId );

		// Get the complete list of all these objects.
		typedef std::vector< AimMagnet* > ObjectList;
		static ObjectList const& GetObjectList( ) { return m_lstObjects; }
	
	protected :

        uint32          EngineMessageFn (uint32 messageID, void *pData, LTFLOAT lData);
		bool			OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		virtual void	CreateSpecialFX( bool bUpdateClients = false );

		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );


	protected:

		std::string			m_sTargetName;
		LTObjRefNotifier	m_hTarget;
		uint8				m_nTeamId;					// Team the pickup item belongs to.

	private :

        LTBOOL	ReadProp(ObjectCreateStruct *pData);
        LTBOOL	InitialUpdate();
		LTBOOL	Update();
  		void	AssignTarget();

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		static ObjectList	m_lstObjects;
};

class CAimMagnetPlugin : public IObjectPlugin
{
	public:	

		virtual LTRESULT PreHook_EditStringList(
				const char* szRezPath,
				const char* szPropName,
				char** aszStrings,
				uint32* pcStrings,
				const uint32 cMaxStrings,
				const uint32 cMaxStringLength);

	protected:

		CCommandMgrPlugin m_CommandMgrPlugin;

};

#endif // __AimMagnet_H__
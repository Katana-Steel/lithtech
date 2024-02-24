// ----------------------------------------------------------------------- //
//
// MODULE  : Steam.h
//
// PURPOSE : Steam class definition
//
// CREATED : 10/19/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __STEAM_H__
#define __STEAM_H__

#include "ltengineobjects.h"
#include "SharedFXStructs.h"
#include "Timer.h"
#include "GameBase.h"
#include "FXButeMgr.h"

class Steam : public GameBase
{
	public :

		Steam();
		~Steam();

        void Setup(STEAMCREATESTRUCT *pSC, LTFLOAT fLifetime = -1.0f, LTBOOL bStartActive = LTFALSE);

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	private :

		void	Update();
		void	InitialUpdate();

		void	TriggerMsg(HOBJECT hSender, HSTRING hMsg);
		void	ReadProps();

        void    DoDamage(LTFLOAT fDamageAmount);

		void	CreateSFXMsg();
		void	TurnOn();
		void	TurnOff();

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

        LTBOOL		m_bOn;				// Are we currently on?
		LTFLOAT		m_fDamageRange;		// How far away can we damage stuff?
        LTFLOAT		m_fDamage;			// How much damage (per second) the steam does

        LTFLOAT		m_fLifetime;		// How long steam stays around (-1 = forever)
        LTBOOL		m_bStartActive;		// Should the steam start on?
		CTimer		m_Timer;			// Time until the steam turns off

		uint8		m_nSteamFXId;		// The attribute file ID for the smoke FX to create
};

// ----------------------------------------------------------------------- //

class CSteamPlugin : public IObjectPlugin
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

#endif // __STEAM_H__

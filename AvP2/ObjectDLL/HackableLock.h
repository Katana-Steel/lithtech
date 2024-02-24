// ----------------------------------------------------------------------- //
//
// MODULE  : HackableLock.cpp
//
// PURPOSE : Definition of the Hackable Lock object
//
// CREATED : 12/6/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HACKABLE_LOCK_H__
#define __HACKABLE_LOCK_H__

#include "Prop.h"


class HackableLock : public Prop
{
	public :

		HackableLock();
		~HackableLock();

		void		SetEnabled(LTBOOL bOn)			{ m_bEnabled = bOn; }

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

        LTBOOL   ReadProp(ObjectCreateStruct *pData);
        LTBOOL   InitialUpdate();

		void	SetupUnlockState();
		LTBOOL	Update();

	private :
		
		// Target information
		LTString	m_szMainTarget;
		LTSmartLink	m_hMainTarget;
		LTBOOL		m_bTriggerWhenUnlocked;

		uint32	m_nHitPoints;
		LTFLOAT	m_fMaxHitPoints;

		LTBOOL		m_bEnabled;
		LTString	m_strHackCmd;

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

		void	Cache();
};

#endif // __HACKABLE_LOCK_H__

// ----------------------------------------------------------------------- //
//
// MODULE  : HackableWorldmodel.cpp
//
// PURPOSE : Definition of the Hackable Lock object
//
// CREATED : 12/6/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HACKABLE_WORLDMODEL_H__
#define __HACKABLE_WORLDMODEL_H__

#include "Door.h"


class HackableWorldmodel : public Door
{
	public :

		HackableWorldmodel();
		~HackableWorldmodel();

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

#endif // __HACKABLE_WORLDMODEL_H__

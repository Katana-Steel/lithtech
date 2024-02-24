// ----------------------------------------------------------------------- //
//
// MODULE  : DisplayTimer.h
//
// PURPOSE : DisplayTimer - Definition
//
// CREATED : 10/15/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DISPLAY_TIMER_H__
#define __DISPLAY_TIMER_H__

#include "cpp_engineobjects_de.h"
#include "Timer.h"

class DisplayTimer : public BaseClass
{
	public :

		DisplayTimer();
		~DisplayTimer();

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private :

		HSTRING	m_hstrStartCmd;		// Command to send when timer starts
		HSTRING	m_hstrEndCmd;		// Command to send when timer ends
		DBOOL	m_bRemoveWhenDone;	// Remove the timer when done?

		CTimer	m_Timer;			// Timer

		void	ReadProp(ObjectCreateStruct *pData);
		void	InitialUpdate();
		void	Update();
		
		void	TriggerMsg(HOBJECT hSender, char *pMsg);

		void	HandleStart(DFLOAT fTime);
		void	HandleAdd(DFLOAT fTime);
		void	HandleEnd();
		void	HandleAbort();
		void	UpdateClients();

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
};

#endif // __DISPLAY_TIMER_H__


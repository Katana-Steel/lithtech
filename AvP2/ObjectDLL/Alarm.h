// ----------------------------------------------------------------------- //
//
// MODULE  : Alarm.h
//
// PURPOSE : Alarm object definition
//
// CREATED : 12/07/00
//
// ----------------------------------------------------------------------- //

#ifndef __ALARM_H__
#define __ALARM_H__

#include "GameBase.h"
#include <string>

class Alarm;

class Alarm : public GameBase
{
	public:

		Alarm();

	protected :

		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD		ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private :

		void		ReadProps();
		void		Activate();
		void		Deactivate();

		void		Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void		Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);


		std::string		m_strActivateTarget;
		std::string		m_strActivateMessage;
		std::string		m_strDeactivateTarget;
		std::string		m_strDeactivateMessage;

		std::string		m_strAlarmSound;
		HSOUNDDE		m_hActiveSound;

		LTBOOL			m_bPlayerUsable;
};

#endif // __ALARM_H__


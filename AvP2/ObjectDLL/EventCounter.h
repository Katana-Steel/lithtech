// ----------------------------------------------------------------------- //
//
// MODULE  : EventCounter.h
//
// PURPOSE : EventCounter - Definition
//
// CREATED : 04.23.1999
//
// ----------------------------------------------------------------------- //

#ifndef __EVENT_COUNTER_H__
#define __EVENT_COUNTER_H__

#include "GameBase.h"
#include "ServerUtilities.h"

#define EC_MAX_NUMBER_OF_EVENTS		6


struct EventData
{
    EventData()
	{
		nValue				= 0;
        hstrIncToValCmd     = LTNULL;
        hstrDecToValCmd     = LTNULL;
	}

    virtual ~EventData()
	{
		FREE_HSTRING(hstrIncToValCmd);
		FREE_HSTRING(hstrDecToValCmd);
	}

	inline void Save(HMESSAGEWRITE hWrite)
	{
		SAVE_INT(nValue);
		SAVE_HSTRING(hstrIncToValCmd);
		SAVE_HSTRING(hstrDecToValCmd);
	}

	inline void Load(HMESSAGEREAD hRead)
	{
		LOAD_INT(nValue);
		LOAD_HSTRING(hstrIncToValCmd);
		LOAD_HSTRING(hstrDecToValCmd);
	}

	// Data...

	int		nValue;
	HSTRING	hstrIncToValCmd;
	HSTRING	hstrDecToValCmd;
};


class EventCounter : public GameBase
{
	public :

		EventCounter();
		~EventCounter();

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	protected :

        LTBOOL   ReadProp(ObjectCreateStruct *pData);
		void	TriggerMsg(HOBJECT hSender, char* pMsg);

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

	private :

        LTBOOL       m_bLocked;
		int			m_nCurVal;

		EventData	m_EventData[EC_MAX_NUMBER_OF_EVENTS];

		void		Increment();
		void		Decrement();
};

#endif // __EVENT_COUNTER_H__

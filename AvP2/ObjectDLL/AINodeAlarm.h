// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeAlarm.h
//
// PURPOSE : Tactical node definition
//
// CREATED : 08/28/00
//
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_ALARM_H_
#define _AI_NODE_ALARM_H_

#include "AINode.h"

class AINodeAlarm : public AINode
{
	friend class CAINodeAlarm;

public:
	AINodeAlarm();

	virtual CAINode * MakeCAINode(DDWORD dwID);

	virtual void ReadProp(ObjectCreateStruct *p);

private :

	LTString	m_lstrAlarmObject;
	LTString	m_lstrAlarmMsg;
};

class CAINodeAlarm : public CAINode
{
	public :

		CAINodeAlarm(AINodeAlarm & node, DDWORD dwID);
		CAINodeAlarm(HMESSAGEREAD hRead);

		// Used for type identity during load/save.
		static const char * const szTypeName;
		virtual const char * GetTypeName() const { return CAINodeAlarm::szTypeName; }

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		const LTSmartLink & GetAlarmObject() const { return m_hAlarmObject; }
		const LTString & GetAlarmMsg() const { return m_lstrAlarmMsg; }

		virtual LTBOOL IsPathingNode() const { return LTFALSE; }

	protected :

#ifndef _FINAL
		virtual LTVector GetNodeColor() const { return LTVector(0.0f,0.5f,1.0f); }
#endif

	private :

		void InternalLoad(HMESSAGEREAD hRead);

		LTSmartLink	m_hAlarmObject;
		LTString	m_lstrAlarmMsg;				// Message to send (ON or OFF)
};

#endif // __AI_NODE_ALARM_H__

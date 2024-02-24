// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectiveEvent.cpp
//
// PURPOSE : ObjectiveEvent - Definition
//
// CREATED : 5/9/2001
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __OBJECTIVE_EVENT_H__
#define __OBJECTIVE_EVENT_H__

#include "GameBase.h"
#include "ObjectivesButeMgr.h"

#define MAX_OBJECTIVE_EVENTS 5

class ObjectiveEvent : public GameBase
{
public :
	
	ObjectiveEvent();
	~ObjectiveEvent();
	
private:
	
	uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
	uint32	ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);
	void	HandleTriggerMsg();
	
	void	Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
	void	Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
	
	LTBOOL	InitialUpdate();
	LTBOOL	ReadProp(ObjectCreateStruct *pData);

	uint32	m_nAddObjectives[MAX_OBJECTIVE_EVENTS];
	uint32	m_nCompleteObjectives[MAX_OBJECTIVE_EVENTS];
	uint32	m_nRemoveObjectives[MAX_OBJECTIVE_EVENTS];
	uint32	m_nCancelObjectives[MAX_OBJECTIVE_EVENTS];
};

// ----------------------------------------------------------------------- //
// Plugin class for static string lists

class CObjectiveEventPlugin : public IObjectPlugin
{
  public:

	virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath, 
		const char* szPropName, 
		char* const * aszStrings, 
		uint32* pcStrings, 
		const uint32 cMaxStrings, 
		const uint32 cMaxStringLength);

  protected :

		CObjectiveButeMgrPlugin m_ObjectiveButeMgrPlugin;
};

// ----------------------------------------------------------------------- //


#endif // __OBJECTIVE_EVENT_H__

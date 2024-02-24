// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectiveMgr.cpp
//
// PURPOSE : ObjectiveMgr - Definition
//
// CREATED : 12/21/2000
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __OBJECTIVE_MGR_H__
#define __OBJECTIVE_MGR_H__

#include "GameBase.h"
#include "ObjectivesButeMgr.h"

#define MAX_INITIAL_OBJECTIVES 6

class ObjectiveMgr : public GameBase
{
public :
	
	ObjectiveMgr();
	~ObjectiveMgr();

	void	AddObjective(uint32 nId);
	void	ChangeObjectiveState(uint32 nId, ObjectiveState eState);
	
private:
	
	uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
	uint32	ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);
	LTBOOL	HandleTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead);
	
	void	AddObjective(char* szObjective);
	void	ChangeObjectiveState(char* szObjective, ObjectiveState eState);
	
	void	Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
	void	Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
	
	ObjectiveData m_Objective[MAX_OBJECTIVES];
	
	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...
	
	LTBOOL	InitialUpdate();
	LTBOOL	ReadProp(ObjectCreateStruct *pData);

	uint32	m_nObjectives[MAX_INITIAL_OBJECTIVES];
	int		m_nOverviewID;
	LTBOOL	m_bSendInitialObjectives;

	static int m_count;
};

// ----------------------------------------------------------------------- //
// Plugin class for static string lists

class CObjectivePlugin : public IObjectPlugin
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


#endif // __OBJECTIVE_MGR_H__

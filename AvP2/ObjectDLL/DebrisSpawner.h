// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisSpawner.h
//
// PURPOSE : Debris Spawner object class definition
//
// CREATED : 1/24/2001
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DEBRIS_SPAWNER_H__
#define __DEBRIS_SPAWNER_H__

// ----------------------------------------------------------------------- //

#include "GameBase.h"
#include "DebrisFuncs.h"

// ----------------------------------------------------------------------- //

class DebrisSpawner : public GameBase
{
public:
	
	DebrisSpawner();
	virtual ~DebrisSpawner();
	
protected :
	
	uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
	uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);
	LTBOOL	Update();
	
private:
	void	TriggerMsg(HOBJECT hSender, HSTRING hMsg);
	LTBOOL	ReadProp(ObjectCreateStruct *pInfo);
	void	Save(HMESSAGEWRITE hWrite);
	void	Load(HMESSAGEREAD hRead);
	void	CreateDebris();

	HSTRING	m_strDebris0;
	HSTRING	m_strDebris1;
	HSTRING	m_strDebris2;
	HSTRING	m_strDebris3;

	HSTRING	m_strOnCmd;
	HSTRING	m_strOffCmd;
	HSTRING	m_strSpawnCmd;

	LTBOOL	m_bOn;
	LTBOOL	m_bContinuous;
	LTFLOAT	m_fInterval;
	LTFLOAT m_fSpawnTime;
};

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

class DebrisSpawnerPlugin : public IObjectPlugin
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

	CDebrisPlugin		m_DebrisPlugin;
};

#endif  // __DEBRIS_SPAWNER_H__


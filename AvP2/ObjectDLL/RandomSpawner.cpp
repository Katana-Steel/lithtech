// ----------------------------------------------------------------------- //
//
// MODULE  : RandomSpawner.h
//
// PURPOSE : RandomSpawner - Implementation
//
// CREATED : 04.23.1999
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "RandomSpawner.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"

BEGIN_CLASS(RandomSpawner)

	ADD_STRINGPROP(Spawner, "")
	ADD_LONGINTPROP(Number, 1)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(RandomSpawner, BaseClass, NULL, NULL)

// Statics

const char s_szSpawnTrigger[] = "DEFAULT";

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::RandomSpawner()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

RandomSpawner::RandomSpawner() : BaseClass()
{
	m_bFirstUpdate = DTRUE;

	m_hstrSpawner = DNULL;
	m_cSpawn = 0;

	m_ahSpawners = new HOBJECT[kMaxSpawners];

	for ( int iSpawner = 0 ; iSpawner < kMaxSpawners ; iSpawner++ )
	{
		m_ahSpawners[iSpawner] = DNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::~RandomSpawner()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

RandomSpawner::~RandomSpawner()
{
	FREE_HSTRING(m_hstrSpawner);

	for ( int iSpawner = 0 ; iSpawner < kMaxSpawners ; iSpawner++ )
	{
		if ( m_ahSpawners[iSpawner] )
		{
			g_pServerDE->BreakInterObjectLink(m_hObject, m_ahSpawners[iSpawner]);
		}
	}

	delete [] m_ahSpawners;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD RandomSpawner::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			break;
		}

		case MID_LINKBROKEN :
		{
			HOBJECT hObject = (HOBJECT)pData;

			for ( int iSpawner = 0 ; iSpawner < kMaxSpawners ; iSpawner++ )
			{
				if ( hObject == m_ahSpawners[iSpawner] )
				{
					m_ahSpawners[iSpawner] = DNULL;
				}
			}

			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD RandomSpawner::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	if (!g_pServerDE) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			TriggerMsg(hSender, hMsg);
			g_pServerDE->FreeString(hMsg);
		}
		break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::TriggerMsg()
//
//	PURPOSE:	Handler for RandomSpawner trigger messages.
//
// --------------------------------------------------------------------------- //

void RandomSpawner::TriggerMsg(HOBJECT hSender, HSTRING hMsg)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pMsg = pServerDE->GetStringData(hMsg);
	if (!pMsg) return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL RandomSpawner::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) return DFALSE;

	GenericProp genProp;

	if ( g_pServerDE->GetPropGeneric( "Spawner", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrSpawner = g_pServerDE->CreateString( genProp.m_String );

	if ( g_pServerDE->GetPropGeneric( "Number", &genProp ) == DE_OK )
		m_cSpawn = genProp.m_Long;
		
	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void RandomSpawner::PostPropRead(ObjectCreateStruct *pStruct)
{
	if ( !pStruct ) return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

DBOOL RandomSpawner::InitialUpdate()
{
	g_pServerDE->SetNextUpdate(m_hObject, 0.01f);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

DBOOL RandomSpawner::Update()
{
	if ( m_bFirstUpdate )
	{
		Setup();
		Spawn();

		m_bFirstUpdate = DFALSE;

		g_pServerDE->SetNextUpdate(m_hObject, 0.00f);
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void RandomSpawner::Save(HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	g_pServerDE->WriteToMessageDWord(hWrite, m_bFirstUpdate);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrSpawner);
	g_pServerDE->WriteToMessageDWord(hWrite, m_cSpawn);

	for ( int iSpawner = 0 ; iSpawner < kMaxSpawners ; iSpawner++ )
	{
		g_pServerDE->WriteToLoadSaveMessageObject(hWrite, m_ahSpawners[iSpawner]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void RandomSpawner::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_bFirstUpdate = (DBOOL)g_pServerDE->ReadFromMessageDWord(hRead);
	m_hstrSpawner = g_pServerDE->ReadFromMessageHString(hRead);
	m_cSpawn = g_pServerDE->ReadFromMessageDWord(hRead);

	for ( int iSpawner = 0 ; iSpawner < kMaxSpawners ; iSpawner++ )
	{
		g_pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_ahSpawners[iSpawner]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::Setup()
//
//	PURPOSE:	Setup
//
// ----------------------------------------------------------------------- //

void RandomSpawner::Setup()
{
	HCLASS  hSpawner = g_pServerDE->GetClass("Spawner");
	_ASSERT(hSpawner);
	if  ( !hSpawner ) return;

	const char* szSpawnerBase = g_pServerDE->GetStringData(m_hstrSpawner);
	_ASSERT(szSpawnerBase);
	if ( !szSpawnerBase ) return;

	int cSpawners = 0;

	while ( cSpawners <= kMaxSpawners )
	{
		char szSpawner[256];
		sprintf(szSpawner, "%s%2.2d", szSpawnerBase, cSpawners);

		ObjArray <HOBJECT, 1> objArray;
		g_pServerDE->FindNamedObjects(szSpawner,objArray);
		if(!objArray.NumObjects()) break;

		HOBJECT hObject = objArray.GetObject(0);

		if ( hObject )
		{
			m_ahSpawners[cSpawners++] = hObject;
			g_pServerDE->CreateInterObjectLink(m_hObject, hObject);
		} 
		else
		{
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::Spawn()
//
//	PURPOSE:	Spawn
//
// ----------------------------------------------------------------------- //

void RandomSpawner::Spawn() 
{
	DBOOL abSpawned[kMaxSpawners];
	memset(abSpawned, DFALSE, sizeof(abSpawned));

	//seed the rand
	srand(GetRandom(2, 255));

	for ( int iSpawn = 0 ; iSpawn < m_cSpawn ; iSpawn++ )
	{
		// Keep picking a random one until we have one that hasn't spawned yet

		int iSafety = 50000;
		int iSpawner = GetRandom(0, kMaxSpawners-1);
		while ( (abSpawned[iSpawner] || !m_ahSpawners[iSpawner]) && (--iSafety > 0) )
		{
			iSpawner = GetRandom(0, kMaxSpawners-1);
		}

		abSpawned[iSpawner] = DTRUE;

		// Trigger the spawner to spawn the default object

		SendTriggerMsgToObject(this, m_ahSpawners[iSpawner], s_szSpawnTrigger);
	}
}

// ----------------------------------------------------------------------- //
//
// MODULE  : Spawner.cpp
//
// PURPOSE : Spawner class - implementation
//
// CREATED : 12/31/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Spawner.h"
#include "cpp_server_de.h"
#include "MsgIDs.h"
#include "ServerSoundMgr.h"
#include "ObjectMsgs.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpawnObject
//
//	PURPOSE:	Spawns an object
//
// ----------------------------------------------------------------------- //

BaseClass *SpawnObject(char *pszSpawn, const LTVector& vPos, const LTRotation& rRot)
{
	if(!pszSpawn) return LTNULL;

	// Pull the class name out of the spawn string...
	char* pszClassName = strtok(pszSpawn, " ");
	if (!pszClassName) return LTNULL;

	HCLASS hClass = g_pLTServer->GetClass(pszClassName);
	if (!hClass) return LTNULL;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Pos = vPos;
	theStruct.m_Rotation = rRot;

	// Set the string to be the rest of the string...
	char* pszProps = strtok(NULL, "");

	// Allocate an object...
	return (BaseClass*)g_pLTServer->CreateObjectProps(hClass, &theStruct, pszProps);
}



BEGIN_CLASS(Spawner)

	ADD_STRINGPROP( DefaultSpawn, "" )
	ADD_STRINGPROP( InitialMessage, "" )

	ADD_STRINGPROP_FLAG( SpawnSound, "", PF_FILENAME )
	ADD_REALPROP( SoundRadius, 500.0f )

END_CLASS_DEFAULT(Spawner, GameBase, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::Spawner
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

Spawner::Spawner() : GameBase(OT_NORMAL)
{
	m_hstrDefaultSpawn = LTNULL;
	m_hstrInitialMsg = LTNULL;
	m_hstrSpawnSound = LTNULL;
	m_fSoundRadius = 500.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::~Spawner
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Spawner::~Spawner()
{
	if( m_hstrDefaultSpawn )
		g_pLTServer->FreeString( m_hstrDefaultSpawn );

	if( m_hstrInitialMsg )
		g_pLTServer->FreeString( m_hstrInitialMsg );

	if( m_hstrSpawnSound )
 		g_pLTServer->FreeString( m_hstrSpawnSound );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Spawner::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	LTBOOL bRet = GameBase::EngineMessageFn(messageID, pData, fData);

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			break;
		}
		case MID_INITIALUPDATE:
		{
			InitialUpdate();
			CacheFiles();
			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;
	
		default : break;
	}


	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::InitialUpdate
//
//	PURPOSE:	Do initial updating
//
// ----------------------------------------------------------------------- //

LTBOOL Spawner::InitialUpdate()
{
	g_pLTServer->SetNextUpdate(m_hObject, (LTFLOAT)0.0f);
	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::ReadProp
//
//	PURPOSE:	Reads properties from level info
//
// ----------------------------------------------------------------------- //

LTBOOL Spawner::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	if ( g_pLTServer->GetPropGeneric( "DefaultSpawn", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_hstrDefaultSpawn = g_pLTServer->CreateString( genProp.m_String );

	if ( g_pLTServer->GetPropGeneric( "InitialMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_hstrInitialMsg = g_pLTServer->CreateString( genProp.m_String );

	if ( g_pLTServer->GetPropGeneric( "SpawnSound", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_hstrSpawnSound = g_pLTServer->CreateString( genProp.m_String );

	if ( g_pLTServer->GetPropGeneric( "SoundRadius", &genProp ) == LT_OK )
	{
		m_fSoundRadius = genProp.m_Float;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 Spawner::ObjectMessageFn( HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead )
{
	LTVector vPos;
	LTRotation rRot;
	char szSpawn[501];

	switch( messageID )
	{
		case MID_TRIGGER:
		{
			szSpawn[0] = '\0';
			HSTRING hMsg = g_pLTServer->ReadFromMessageHString( hRead );
			if (!hMsg) break;

			char* pMsg = g_pLTServer->GetStringData(hMsg);
			if (_stricmp(pMsg, "default") == 0)
			{
				pMsg = g_pLTServer->GetStringData(m_hstrDefaultSpawn);
			}

			strncpy( szSpawn, pMsg, 500 );

			g_pLTServer->FreeString(hMsg);

			g_pLTServer->GetObjectPos( m_hObject, &vPos );
			g_pLTServer->GetObjectRotation( m_hObject, &rRot );

			// Play spawn sound...
			if ( m_hstrSpawnSound )
			{
				char* pSound = g_pLTServer->GetStringData(m_hstrSpawnSound);
				g_pServerSoundMgr->PlaySoundFromPos(vPos, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW);
			}
			
			BaseClass * pSpawnedObject = SpawnObject( szSpawn, vPos, rRot );
			if( m_hstrInitialMsg && pSpawnedObject )
			{
				SendTriggerMsgToObject(this, pSpawnedObject->m_hObject, g_pLTServer->GetStringData(m_hstrInitialMsg) );
			}

			break;
		}

		default : break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Spawner::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if(!hWrite) return;

	g_pLTServer->WriteToMessageHString(hWrite, m_hstrDefaultSpawn);	
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrInitialMsg);	
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrSpawnSound);	
	g_pLTServer->WriteToMessageFloat(hWrite, m_fSoundRadius);	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Spawner::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if(!hRead) return;

	m_hstrDefaultSpawn	= g_pLTServer->ReadFromMessageHString(hRead);	
	m_hstrInitialMsg	= g_pLTServer->ReadFromMessageHString(hRead);	
	m_hstrSpawnSound	= g_pLTServer->ReadFromMessageHString(hRead);	
	m_fSoundRadius		= g_pLTServer->ReadFromMessageFloat(hRead);	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::CacheFiles
//
//	PURPOSE:	Cache resources associated with this object
//
// ----------------------------------------------------------------------- //

void Spawner::CacheFiles()
{
	char* pFile = LTNULL;
	if (m_hstrSpawnSound)
	{
		pFile = g_pLTServer->GetStringData(m_hstrSpawnSound);
		if (pFile)
		{
			g_pLTServer->CacheFile(FT_SOUND, pFile);
		}
	}
}

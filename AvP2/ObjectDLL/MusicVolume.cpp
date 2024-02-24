// ----------------------------------------------------------------------- //
//
// MODULE  : MusicVolume.cpp
//
// PURPOSE : MusicVolume implementation
//
// CREATED : 1/29/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MusicVolume.h"
#include "cpp_server_de.h"
#include "ServerUtilities.h"
#include "ContainerCodes.h"
#include "ObjectMsgs.h"
#include "AIUtils.h"

const char g_szNone[] = "<none>";
const char g_szDefault[] = "<default>";

BEGIN_CLASS(MusicVolume)
	ADD_STRINGPROP_FLAG_HELP(Mood, const_cast<char*>(g_szNone), PF_STATICLIST, "Music mood.")
	ADD_STRINGPROP_FLAG_HELP(TrackSet, const_cast<char*>(g_szDefault), PF_STATICLIST, "TrackSet for music mood.")
	ADD_BOOLPROP_FLAG_HELP(BoxPhysics, LTFALSE, 0, "Use simple boxphysics collisions." )
	ADD_BOOLPROP_FLAG_HELP(StartLocked, LTFALSE, 0, "Starts locked if true." )
END_CLASS_DEFAULT_FLAGS_PLUGIN(MusicVolume, GameBase, NULL, NULL, CF_ALWAYSLOAD, CMusicVolumePlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MusicVolume::MusicVolume()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

MusicVolume::MusicVolume() : GameBase(OT_CONTAINER)
{
	// Default to having no mood.
	m_eMood = CMusicMgr::eMoodNone;
	m_bLocked = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MusicVolume::~MusicVolume
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

MusicVolume::~MusicVolume()
{ 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MusicVolume::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 MusicVolume::EngineMessageFn(uint32 messageID, void *pData, DFLOAT fData)
{
	uint32 dwRet;

	switch(messageID)
	{
		case MID_AFFECTPHYSICS:
		{
			AffectPhysics((ContainerPhysics*)pData);
			break;
		}

		case MID_PRECREATE:
		{
			dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);
			return dwRet;
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

		default : 
			break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MusicVolume::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 MusicVolume::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    if (!g_pLTServer) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hStr = g_pLTServer->ReadFromMessageHString(hRead);
			TriggerMsg(hSender, g_pLTServer->GetStringData(hStr));
			g_pLTServer->FreeString(hStr);

			// Make sure other people can read it...

            g_pLTServer->ResetRead(hRead);
		}
		break;

		default : break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MusicVolume::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void MusicVolume::ReadProp(ObjectCreateStruct *pStruct)
{
	// Check inputs.
	if( !pStruct )
	{
		ASSERT( FALSE );
		return;
	}

	GenericProp genProp;

	// Get the mood.
	if( g_pLTServer->GetPropGeneric( "Mood", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] && 0 != stricmp(genProp.m_String, g_szNone) )
		{
			m_eMood = CMusicMgr::ConvertStringToMood( genProp.m_String );
		}
	}

	// Get the trackset.
	if( g_pLTServer->GetPropGeneric( "TrackSet", &genProp ) == LT_OK )
	{
		// g_szNone needs to be checked to stay backwards compatible.
		if( genProp.m_String[0] && 0 != stricmp(genProp.m_String, g_szNone) && 0 != stricmp(genProp.m_String, g_szDefault) )
		{
			// Get the song, mood and trackset.
			char* pszSong = strtok( genProp.m_String, " " );
			if( pszSong )
			{
				char* pszMood = strtok( NULL, " " );
				if( pszMood )
				{
					char* pszTrackSet = strtok( NULL, " " );
					if( pszTrackSet )
					{
						m_sTrackSet = pszTrackSet;
					}
				}
			}
		}
	}

	// Get the boxphysics flag.
	if( g_pLTServer->GetPropGeneric( "BoxPhysics", &genProp ) == LT_OK )
	{
		pStruct->m_Flags |= FLAG_BOXPHYSICS;
	}

	if( g_pLTServer->GetPropGeneric( "StartLocked", &genProp ) == LT_OK )
	{
		m_bLocked = genProp.m_Bool;
	}
	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MusicVolume::PostPropRead
//
//	PURPOSE:	Set some final values.
//
// ----------------------------------------------------------------------- //

void MusicVolume::PostPropRead(ObjectCreateStruct *pStruct) 
{
	if (!pStruct) return;

	pStruct->m_Flags |= FLAG_CONTAINER | FLAG_TOUCH_NOTIFY | FLAG_GOTHRUWORLD;
	pStruct->m_ObjectType = OT_CONTAINER;
	SAFE_STRCPY(pStruct->m_Filename, pStruct->m_Name);
	pStruct->m_SkinName[0] = '\0';
	pStruct->m_ContainerCode = CC_NO_CONTAINER;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MusicVolume::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void MusicVolume::InitialUpdate()
{
	// Don't need updates.
	SetNextUpdate( 0.0f, 0.0f );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MusicVolume::AffectPhysics()
//
//	PURPOSE:	Update the physics of the passed in object
//
// ----------------------------------------------------------------------- //

void MusicVolume::AffectPhysics(ContainerPhysics* pCPStruct)
{
	// Sanity checks.
	if ( !pCPStruct || !pCPStruct->m_hObject) return;

	// Don't do anything if we are locked.
	if( m_bLocked ) return;

	// Get the type info for the objects we care about.
	HCLASS hPlayer = g_pServerDE->GetClass("CPlayerObj");
	HCLASS hClass = g_pServerDE->GetObjectClass(pCPStruct->m_hObject);

	// If the object in the volume is not the player, it can be ignored.
	if (!g_pLTServer->IsKindOf(hClass, hPlayer))
		return;

	// Tell the music to modify its mood.
	g_pMusicMgr->SetModifiedMood( m_eMood, m_sTrackSet.c_str( ));
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	MusicVolume::TriggerMsg()
//
//	PURPOSE:	Process trigger messages
//
// --------------------------------------------------------------------------- //

void MusicVolume::TriggerMsg(HOBJECT hSender, const char* szMsg)
{
	if (!szMsg) return;

	if (_stricmp(szMsg, "LOCK") == 0)
	{
		m_bLocked = LTTRUE;
	}
	else if (_stricmp(szMsg, "UNLOCK") == 0)
	{
		m_bLocked = LTFALSE;
	}
#ifndef _FINAL
	else
	{
		AIErrorPrint(m_hObject, "Did not understand message \"%s\".",
			szMsg);
	}
#endif
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MusicVolume::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void MusicVolume::Save(HMESSAGEWRITE hWrite)
{
    *hWrite << m_eMood;
	*hWrite << m_sTrackSet;
	*hWrite << m_bLocked;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void MusicVolume::Load(HMESSAGEREAD hRead)
{
    *hRead >> m_eMood;
	*hRead >> m_sTrackSet;
	*hRead >> m_bLocked;
}



static bool		s_bInitialized = false;

DRESULT	CMusicVolumePlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char* const * aszStrings, DDWORD* pcStrings, const DDWORD cMaxStrings, const DDWORD cMaxStringLength)
{
	char szFile[256];

	// Make sure we have the butes file loaded.
	if( !s_bInitialized )
	{
		// Initialize the music mgr.
		if( g_pMusicButeMgr && !g_pMusicButeMgr->IsValid( ))
		{
#ifdef _WIN32
			sprintf(szFile, "%s\\%s", szRezPath, "attributes\\music.txt" );
#else
			sprintf(szFile, "%s/%s", szRezPath, "Attributes/music.txt" );
#endif
			g_pMusicButeMgr->SetInRezFile(LTFALSE);
			g_pMusicButeMgr->Init(g_pLTServer, szFile);
		}

		s_bInitialized = true;
	}

	// Handle the music properties.
	if( stricmp("Mood",szPropName) == 0 )
	{
		// Add a blank entry.
		strcpy( *aszStrings, g_szNone );
		++aszStrings;
		++(*pcStrings);

		// Add a the moods.
		strcpy( *aszStrings, "Ambient" );
		++aszStrings;
		++(*pcStrings);

		// Add a the moods.
		strcpy( *aszStrings, "Warning" );
		++aszStrings;
		++(*pcStrings);

		// Add a the moods.
		strcpy( *aszStrings, "Hostile" );
		++aszStrings;
		++(*pcStrings);

		return LT_OK;
	}
	else if( stricmp("TrackSet",szPropName) == 0 )
	{
		// Make sure the music butes were initialized ok.
		if( g_pMusicButeMgr && g_pMusicButeMgr->IsValid( ))
		{
			CMusicButeMgr::TrackSetList lstTrackSets;

			// Add a blank entry.
			strcpy( *aszStrings, g_szDefault );
			++aszStrings;
			++(*pcStrings);

			// Get the tracksets.
			if( g_pMusicButeMgr->GetTrackSetsListForUI( lstTrackSets ))
			{
				// Add the tracksets to the listbox.
				for( CMusicButeMgr::TrackSetList::iterator iter = lstTrackSets.begin();
					 iter != lstTrackSets.end(); ++iter )
				{
					strcpy( *aszStrings, iter->c_str( ));
					++aszStrings;
					++(*pcStrings);
				}
			}
		}

		return LT_OK;
	}

	// No one wants it

	return LT_UNSUPPORTED;
}

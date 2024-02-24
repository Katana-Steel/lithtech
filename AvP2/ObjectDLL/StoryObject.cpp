// ----------------------------------------------------------------------- //
//
// MODULE  : StoryTrigger.cpp
//
// PURPOSE : StoryObject implementation
//
// CREATED : 12/8/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "StoryObject.h"
#include "cpp_server_de.h"
#include "ObjectMsgs.h"
#include "MsgIDs.h"
#include "CommandMgr.h"


BEGIN_CLASS(StoryObject)
	ADD_STRINGPROP_FLAG_HELP(Bitmap,"",PF_FILENAME, "This is the PCX files that will display on screen when this object is activated.")
	ADD_LONGINTPROP_FLAG_HELP(TextXOffset, -1.0, 0, "This is the pixel offset used to place the horizontal position of any caption from the left side of the image./n Any value less then 0 will center the text.")
	ADD_LONGINTPROP_FLAG_HELP(TextYOffset, -1.0, 0, "This is the pixel offset used to place the vertical position of any caption./n Any value less then zero will position the text 120 pixels below the center of the screen.")
	ADD_LONGINTPROP_FLAG_HELP(TextWidth, 500.0, 0, "This is the pixel width used to format the text if more then one line.")
	ADD_STRINGPROP_FLAG_HELP(Font,"Default", 0, "This is the font used for any caption./n Default = Character Font./n PDA = PDA Font./n MEMO = Memo font.")
	ADD_STRINGPROP_FLAG_HELP(Sound, "None", PF_STATICLIST,	"Special sound to play when the object is viewed.")
	ADD_REALPROP_FLAG_HELP(ActiveRange, 20.0f, 0, "This is an 'active' range that player can move around in without the PCX going away.")
	ADD_LONGINTPROP_FLAG_HELP(MaxViews, -1, 0, "This is the number of times you can view this object. -1 is infinite.")
	ADD_LONGINTPROP_FLAG_HELP(TextID, -1, 0, "This is the string resource ID that will be displayed over the PCX.")
	ADD_STRINGPROP_FLAG_HELP(OnViewCommand, "", 0, "This command string will be caled when the story object is viewed.")
END_CLASS_DEFAULT_FLAGS_PLUGIN(StoryObject, Prop, NULL, NULL, 0, CStoryPlugin)


LTRESULT CStoryPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char* const * aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (_stricmp("Sound", szPropName) == 0)
	{
		m_SoundButePlugin.PreHook_EditStringList(szRezPath,
			szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength, "BaseStoryObjectSound" );
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryObject::StoryObject()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

StoryObject::StoryObject()
{
	m_hBitmapFile	= LTNULL;
	m_hSoundFile	= LTNULL;
	m_fTime			= -1.0;	
	m_fRange		= 20.0f;		
	m_nNumViews		= -1;	
	m_strViewCmd	= LTNULL;
	m_nStringId		= -1;
	m_nFontId		= 0;		
	m_nYOffset		= -1;		
	m_nXOffset		= -1;		
	m_nWidth		= -1;		
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryObject::~StoryObject()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

StoryObject::~StoryObject()
{
	ClearStrings();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryObject::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

DDWORD StoryObject::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER :
			HandleTriggerMsg(hSender, hRead);

#ifndef _FINAL
			if(GetActivateType(m_hObject) == AT_NORMAL)
				g_pLTServer->CPrint("Yep, I'm activatable!");
#endif
			break;
	}

	return Prop::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryObject::EngineMessageFn
//
//	PURPOSE:	Handle engineEngineMessageFn messages
//
// ----------------------------------------------------------------------- //

DDWORD StoryObject::EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT lData)
{
	switch (messageID)
	{
	case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				//name our object
				SAFE_STRCPY(pStruct->m_Name, "StoryObject");
				
				//read some data
				ReadProp(pStruct);
			}
			break;
		}
	case MID_INITIALUPDATE:
		{
			//set the dims of our object
			SetActivateType(m_hObject, AT_NORMAL);

			// Tell the client to cache the PCX
			HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(DNULL, MID_PLAYER_CACHE_STORYOBJECT);
			g_pServerDE->WriteToMessageHString(hMessage, m_hBitmapFile);
			g_pServerDE->EndMessage(hMessage);


			// We don't need any more updates..
			SetNextUpdate(0.001f);

			break;
		}
		
	case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)lData);
		}
		break;
		
	case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)lData);
		}
		break;

	default: break;
	}
	
	return Prop::EngineMessageFn(messageID, pData, lData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryObject::HandleTriggerMsg
//
//	PURPOSE:	Handle object trigger messages
//
// ----------------------------------------------------------------------- //

void StoryObject::HandleTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hSender || !hRead) return;

	HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
	if (!hMsg) return;

	char* pMsg = pServerDE->GetStringData(hMsg);
	if (pMsg)
	{
		// Add the msg to the game console...

		pServerDE->RunGameConString(pMsg);
	}

	pServerDE->FreeString(hMsg);

	if(m_nNumViews!=0)
	{
		SendStoryToClient();

		// do the view command
		if (m_strViewCmd)
		{
			const char* pCmd = m_strViewCmd.CStr();

			if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
			{
				g_pCmdMgr->Process(pCmd);
			}
		}

		m_nNumViews--;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryObject::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL StoryObject::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;

	//be sure strings are empty
	ClearStrings();

	if (g_pServerDE->GetPropGeneric("Bitmap", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) 
		{
			m_hBitmapFile = g_pServerDE->CreateString(genProp.m_String);
		}
	}

	if (g_pServerDE->GetPropGeneric("Sound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) 
		{
			m_hSoundFile = g_pServerDE->CreateString(genProp.m_String);
		}
	}

	if (g_pServerDE->GetPropGeneric("ActiveRange", &genProp) == DE_OK)
	{
		m_fRange = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("MaxViews", &genProp) == DE_OK)
	{
		m_nNumViews = genProp.m_Long;
	}

    if (g_pLTServer->GetPropGeneric("OnViewCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_strViewCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	if (g_pServerDE->GetPropGeneric("TextID", &genProp) == DE_OK)
	{
		m_nStringId = genProp.m_Long;
	}

	if (g_pServerDE->GetPropGeneric("TextXOffset", &genProp) == DE_OK)
	{
		m_nXOffset = genProp.m_Long;
	}

	if (g_pServerDE->GetPropGeneric("TextYOffset", &genProp) == DE_OK)
	{
		m_nYOffset = genProp.m_Long;
	}

	if (g_pServerDE->GetPropGeneric("TextWidth", &genProp) == DE_OK)
	{
		m_nWidth = genProp.m_Long;
	}

    if (g_pLTServer->GetPropGeneric("Font", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			if(stricmp("DEFAULT",genProp.m_String) == 0)
				m_nFontId = 0;
			else if(stricmp("PDA",genProp.m_String) == 0)
				m_nFontId = 1;
			else if(stricmp("MEMO",genProp.m_String) == 0)
				m_nFontId = 2;
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryObject::ClearStrings
//
//	PURPOSE:	Clean up
//
// ----------------------------------------------------------------------- //
void StoryObject::ClearStrings()
{
	if(m_hBitmapFile)
	{
		g_pServerDE->FreeString(m_hBitmapFile);
	}

	if(m_hSoundFile)
	{
		g_pServerDE->FreeString(m_hSoundFile);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryObject::SendStoryToClient
//
//	PURPOSE:	Send a message to the client with our data
//
// ----------------------------------------------------------------------- //
void StoryObject::SendStoryToClient()
{
	HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(DNULL, MID_PLAYER_DO_STORYOBJECT);
	g_pServerDE->WriteToMessageHString(hMessage, m_hBitmapFile);
	g_pServerDE->WriteToMessageHString(hMessage, m_hSoundFile);
	g_pServerDE->WriteToMessageFloat(hMessage, m_fTime);
	g_pServerDE->WriteToMessageFloat(hMessage, m_fRange);
	g_pServerDE->WriteToMessageFloat(hMessage, (LTFLOAT)m_nStringId);
	g_pServerDE->WriteToMessageByte(hMessage, m_nFontId);
	g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)m_nYOffset);
	g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)m_nXOffset);
	g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)m_nWidth);
	g_pServerDE->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryObject::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void StoryObject::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!hWrite) return;

	g_pServerDE->WriteToMessageHString(hWrite, m_hBitmapFile);
	g_pServerDE->WriteToMessageHString(hWrite, m_hSoundFile);
	*hWrite << m_strViewCmd;
	*hWrite << m_fTime;		
	*hWrite << m_fRange;	
	*hWrite << m_nNumViews;	
	*hWrite << m_nStringId;	
	*hWrite << m_nFontId;	
	*hWrite << m_nYOffset;	
	*hWrite << m_nXOffset;	
	*hWrite << m_nWidth;	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryObject::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void StoryObject::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!hRead) return;

	m_hBitmapFile	= g_pServerDE->ReadFromMessageHString(hRead);
	m_hSoundFile	= g_pServerDE->ReadFromMessageHString(hRead);
	*hRead >> m_strViewCmd;
	*hRead >> m_fTime;		
	*hRead >> m_fRange;	
	*hRead >> m_nNumViews;	
	*hRead >> m_nStringId;	
	*hRead >> m_nFontId;	
	*hRead >> m_nYOffset;	
	*hRead >> m_nXOffset;	
	*hRead >> m_nWidth;	

	// Since the silly engine sends intital updates before it loads
	// saved data we need to do this here.
	HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(DNULL, MID_PLAYER_CACHE_STORYOBJECT);
	g_pServerDE->WriteToMessageHString(hMessage, m_hBitmapFile);
	g_pServerDE->EndMessage(hMessage);
}


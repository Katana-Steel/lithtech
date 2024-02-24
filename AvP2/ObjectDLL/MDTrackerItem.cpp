// ----------------------------------------------------------------------- //
//
// MODULE  : MDTrackerItem.cpp
//
// PURPOSE : MDTrackerItem implementation
//
// CREATED : 6/7/2000
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MDTrackerItem.h"
#include "cpp_server_de.h"
#include "ObjectMsgs.h"
#include "MsgIDs.h"


BEGIN_CLASS(MDTrackerItem)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(MDTrackerItem, BaseClass, LTNULL, LTNULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MDTrackerItem::MDTrackerItem()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

MDTrackerItem::MDTrackerItem()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MDTrackerItem::~MDTrackerItem()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

MDTrackerItem::~MDTrackerItem()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MDTrackerItem::EngineMessageFn
//
//	PURPOSE:	Handle engineEngineMessageFn messages
//
// ----------------------------------------------------------------------- //

DDWORD MDTrackerItem::EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT lData)
{
	DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, lData);
	
	switch (messageID)
	{
	case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				//name our object
				SAFE_STRCPY(pStruct->m_Name, "MDTrackerItem");
				pStruct->m_Flags |= FLAG_FORCECLIENTUPDATE | FLAG_GOTHRUWORLD;
			}
			
			return dwRet;
		}
	case MID_INITIALUPDATE:
		{
			//set the dims of our object
			g_pServerDE->SetNextUpdate(m_hObject, 1.0);
			SendPositionToClient();
			
			break;
		}
	case MID_UPDATE:
		{
			//set the dims of our object
			g_pServerDE->SetNextUpdate(m_hObject, 1.0);
			SendPositionToClient();
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
	
	return BaseClass::EngineMessageFn(messageID, pData, lData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MDTrackerItem::ObjectMessageFn
//
//	PURPOSE:	Send a message to the client with our data
//
// ----------------------------------------------------------------------- //

uint32 MDTrackerItem::ObjectMessageFn( HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead )
{
	switch(messageID)
	{
		case MID_TRIGGER:
		{
            HSTRING hMsg = g_pLTServer->ReadFromMessageHString(hRead);

			char szMsg[1024];
			strcpy(szMsg, g_pLTServer->GetStringData(hMsg));

			int nArgs;
			char* pCommand = szMsg;
			LTBOOL bMore = LTTRUE;
			char s_tokenSpace[PARSE_MAXTOKENS*PARSE_MAXTOKENSIZE];
			char *s_pTokens[PARSE_MAXTOKENS];
			char *s_pCommandPos;

			bMore = g_pLTServer->Parse(pCommand, &s_pCommandPos, s_tokenSpace, s_pTokens, &nArgs);
			if ( !_stricmp(s_pTokens[0], "REMOVE") )
			{
				HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(DNULL, MID_PLAYER_MDTRACKER_REMOVE);
				g_pLTServer->EndMessage(hMessage);

				g_pLTServer->RemoveObject(m_hObject);
			}

			g_pLTServer->FreeString(hMsg);
			return LTTRUE;
		}

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MDTrackerItem::SendStoryToClient
//
//	PURPOSE:	Send a message to the client with our data
//
// ----------------------------------------------------------------------- //
void MDTrackerItem::SendPositionToClient()
{
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(DNULL, MID_PLAYER_MDTRACKER);
	g_pLTServer->WriteToMessageVector(hMessage, &vPos);
	g_pLTServer->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MDTrackerItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void MDTrackerItem::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!hWrite) return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MDTrackerItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void MDTrackerItem::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!hRead) return;
}


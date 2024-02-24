// ----------------------------------------------------------------------- //
//
// MODULE  : TrailNode.cpp
//
// PURPOSE : TrailNode - Definition
//
// CREATED : 10/13/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "cpp_server_de.h"
#include "TrailNode.h"
#include "ObjectMsgs.h"
#include "LTString.h"
#include "CharacterMgr.h"

// ----------------------------------------------------------------------- //

static char *g_pTrailNodeTypes[TRAILNODE_TYPE_MAX] =
{
	"Default",
	"Human Blood",
	"Predator Blood",
	"Alien Blood",
	"Synth Blood",
};

// ----------------------------------------------------------------------- //

BEGIN_CLASS(TrailNode)
	ADD_STRINGPROP_FLAG_HELP(NodeType, "Default", PF_STATICLIST, "This is the type of trail this node pertains to.")
	ADD_REALPROP_FLAG_HELP(Lifetime, -1.0f, 0, "This is how long the node will hang around in seconds. -1 = infinite (until a REMOVE trigger is processed)")
END_CLASS_DEFAULT_FLAGS_PLUGIN(TrailNode, GameBase, NULL, NULL, CF_ALWAYSLOAD, CTrailNodePlugin)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TrailNode::TrailNode
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

TrailNode::TrailNode() : GameBase()
{ 
	m_nType = TRAILNODE_TYPE_DEFAULT;
	m_fLifetime = -1.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TrailNode::~TrailNode
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

TrailNode::~TrailNode()
{ 
	// Remove it from the list
	g_pCharacterMgr->RemoveTrailNode(this);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TrailNode::Setup
//
//	PURPOSE:	Setup the trail node
//
// ----------------------------------------------------------------------- //

void TrailNode::Setup(uint8 nType, LTFLOAT fLifetime)
{ 
	m_nType = nType;
	m_fLifetime = fLifetime;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TrailNode::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 TrailNode::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct *pStruct = (ObjectCreateStruct*)pData;

			if(pStruct)
			{
				if(fData == PRECREATE_WORLDFILE)
				{
					ReadProp(pStruct);
				}
			}			
		}
		break;

		case MID_INITIALUPDATE:
		{
			if(!dwRet) break;
			g_pLTServer->SetNextUpdate(m_hObject, 0.01f);

			// Add this node to the list
			g_pCharacterMgr->AddTrailNode(this);
		}
		break;

		case MID_UPDATE:
		{
			if(!dwRet) break;
			g_pLTServer->SetNextUpdate(m_hObject, 0.25f);

			// Only update the lifetime is it's not -1
			if(m_fLifetime != -1.0f)
			{
				m_fLifetime -= g_pLTServer->GetFrameTime();

				if(m_fLifetime < 0.0f)
					g_pLTServer->RemoveObject(m_hObject);
			}
		}
		break;

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

	return dwRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TrailNode::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 TrailNode::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
 		case MID_TRIGGER:
		{
			LTString msg = hRead->ReadHString();
			if(_stricmp(msg.CStr(), "REMOVE") == 0)
			{
				g_pLTServer->RemoveObject(m_hObject);
			}
		}

		default : break;
	}

	return GameBase::ObjectMessageFn (hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TrailNode::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL TrailNode::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;

	if(g_pLTServer->GetPropGeneric("NodeType", &genProp) == LT_OK)
	{
		if(genProp.m_String[0]) 
		{
			for(int i = 0; i < TRAILNODE_TYPE_MAX; i++)
			{
				if(!_stricmp(genProp.m_String, g_pTrailNodeTypes[i]))
				{
					m_nType = (uint8)i;
					break;
				}
			}
		}
	}
	
	if(g_pLTServer->GetPropGeneric("Lifetime", &genProp) == LT_OK)
	{
		m_fLifetime = genProp.m_Float;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TrailNode::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void TrailNode::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if(!hWrite) return;

	g_pLTServer->WriteToMessageByte(hWrite, m_nType);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fLifetime);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TrailNode::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void TrailNode::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if(!hRead) return;

	m_nType			= g_pLTServer->ReadFromMessageByte(hRead);
	m_fLifetime		= g_pLTServer->ReadFromMessageFloat(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrailNodePlugin::PreHook_EditStringList()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

LTRESULT CTrailNodePlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	if(_stricmp("NodeType", szPropName) == 0)
	{
		for(int i = 0; i < TRAILNODE_TYPE_MAX; i++)
		{
			_ASSERT(cMaxStrings > (*pcStrings) + 1);

			if(cMaxStrings > (*pcStrings) + 1)
			{
				strcpy(aszStrings[(*pcStrings)++], g_pTrailNodeTypes[i]);
			}
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

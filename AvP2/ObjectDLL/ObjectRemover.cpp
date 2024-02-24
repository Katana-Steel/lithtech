// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectRemover.h
//
// PURPOSE : ObjectRemover - Implementation
//
// CREATED : 04.23.1999
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ObjectRemover.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"

BEGIN_CLASS(ObjectRemover)

	PROP_DEFINEGROUP(Group00, PF_GROUP1)
		ADD_STRINGPROP_FLAG(Group00Object00, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group00Object01, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group00Object02, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group00Object03, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group00Object04, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group00Object05, "", PF_GROUP1|PF_OBJECTLINK)

	PROP_DEFINEGROUP(Group01, PF_GROUP2)
		ADD_STRINGPROP_FLAG(Group01Object00, "", PF_GROUP2|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group01Object01, "", PF_GROUP2|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group01Object02, "", PF_GROUP2|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group01Object03, "", PF_GROUP2|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group01Object04, "", PF_GROUP2|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group01Object05, "", PF_GROUP2|PF_OBJECTLINK)

	PROP_DEFINEGROUP(Group02, PF_GROUP3)
		ADD_STRINGPROP_FLAG(Group02Object00, "", PF_GROUP3|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group02Object01, "", PF_GROUP3|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group02Object02, "", PF_GROUP3|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group02Object03, "", PF_GROUP3|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group02Object04, "", PF_GROUP3|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group02Object05, "", PF_GROUP3|PF_OBJECTLINK)

	PROP_DEFINEGROUP(Group03, PF_GROUP4)
		ADD_STRINGPROP_FLAG(Group03Object00, "", PF_GROUP4|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group03Object01, "", PF_GROUP4|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group03Object02, "", PF_GROUP4|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group03Object03, "", PF_GROUP4|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group03Object04, "", PF_GROUP4|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group03Object05, "", PF_GROUP4|PF_OBJECTLINK)

	PROP_DEFINEGROUP(Group04, PF_GROUP5)
		ADD_STRINGPROP_FLAG(Group04Object00, "", PF_GROUP5|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group04Object01, "", PF_GROUP5|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group04Object02, "", PF_GROUP5|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group04Object03, "", PF_GROUP5|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group04Object04, "", PF_GROUP5|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group04Object05, "", PF_GROUP5|PF_OBJECTLINK)

	PROP_DEFINEGROUP(Group05, PF_GROUP6)
		ADD_STRINGPROP_FLAG(Group05Object00, "", PF_GROUP6|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group05Object01, "", PF_GROUP6|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group05Object02, "", PF_GROUP6|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group05Object03, "", PF_GROUP6|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group05Object04, "", PF_GROUP6|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group05Object05, "", PF_GROUP6|PF_OBJECTLINK)

	ADD_LONGINTPROP(GroupsToKeep, 1)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(ObjectRemover, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::ObjectRemover()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ObjectRemover::ObjectRemover() : BaseClass()
{
	m_cGroupsToKeep = 1;

	for ( int iGroup = 0 ; iGroup < kMaxGroups ; iGroup++ )
	{
		for ( int iObject = 0 ; iObject < kMaxObjectsPerGroup ; iObject++ )
		{
			m_ahstrObjects[iGroup][iObject] = DNULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::~ObjectRemover()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

ObjectRemover::~ObjectRemover()
{
	for ( int iGroup = 0 ; iGroup < kMaxGroups ; iGroup++ )
	{
		for ( int iObject = 0 ; iObject < kMaxObjectsPerGroup ; iObject++ )
		{
			FREE_HSTRING(m_ahstrObjects[iGroup][iObject]);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD ObjectRemover::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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

			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
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
//	ROUTINE:	ObjectRemover::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL ObjectRemover::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) return DFALSE;

	GenericProp genProp;

	for ( int iGroup = 0 ; iGroup < kMaxGroups ; iGroup++ )
	{
		for ( int iObject = 0 ; iObject < kMaxObjectsPerGroup ; iObject++ )
		{
			char szProp[128];
			sprintf(szProp, "Group%2.2dObject%2.2d", iGroup, iObject);
			if ( g_pServerDE->GetPropGeneric( szProp, &genProp ) == DE_OK )
				if ( genProp.m_String[0] )
					m_ahstrObjects[iGroup][iObject] = g_pServerDE->CreateString( genProp.m_String );
		}
	}

	if ( g_pServerDE->GetPropGeneric( "GroupsToKeep", &genProp ) == DE_OK )
		m_cGroupsToKeep = genProp.m_Long;
		
	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

DBOOL ObjectRemover::Update()
{
	HOBJECT ahObjects[kMaxGroups][kMaxObjectsPerGroup];
	memset(ahObjects, DNULL, sizeof(HOBJECT)*kMaxGroups*kMaxObjectsPerGroup);

	int cGroupsWithObjects = 0;

	{for ( int iGroup = 0 ; iGroup < kMaxGroups ; iGroup++ )
	{
		DBOOL bGroupHasObjects = DFALSE;

		{for ( int iObject = 0 ; iObject < kMaxObjectsPerGroup ; iObject++ )
		{
			ObjArray <HOBJECT, 1> objArray;
			g_pServerDE->FindNamedObjects(g_pServerDE->GetStringData(m_ahstrObjects[iGroup][iObject]),objArray);
			if(!objArray.NumObjects()) continue;

			HOBJECT hObject = objArray.GetObject(0);

			if ( hObject )
			{
				ahObjects[cGroupsWithObjects][iObject] = hObject;
				bGroupHasObjects = DTRUE;
			} 
		}}

		if ( bGroupHasObjects )
		{
			cGroupsWithObjects++;
		}
	}}

	// Remove the objects

	//seed the rand
	srand(GetRandom(2, 255));

	DBOOL abRemoved[kMaxGroups];
	memset(abRemoved, DFALSE, sizeof(DBOOL)*kMaxGroups);
	int iSafety = 50000;
	int cRemove = cGroupsWithObjects-m_cGroupsToKeep;

	while ( (cRemove > 0) && (--iSafety > 0) )
	{
		int iRemove = GetRandom(0, cGroupsWithObjects-1);
		if ( !abRemoved[iRemove] )
		{
			for ( int iObject = 0 ; iObject < kMaxObjectsPerGroup ; iObject++ )
			{
				if ( ahObjects[iRemove][iObject] )
				{
					g_pServerDE->RemoveObject(ahObjects[iRemove][iObject]);
				}
			}

			abRemoved[iRemove] = DTRUE;
			cRemove--;
		}
	}

	// Remove ourselves...

	g_pServerDE->RemoveObject(m_hObject);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ObjectRemover::Save(HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	for ( int iGroup = 0 ; iGroup < kMaxGroups ; iGroup++ )
	{
		for ( int iObject = 0 ; iObject < kMaxObjectsPerGroup ; iObject++ )
		{
			SAVE_HSTRING(m_ahstrObjects[iGroup][iObject]);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ObjectRemover::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	for ( int iGroup = 0 ; iGroup < kMaxGroups ; iGroup++ )
	{
		for ( int iObject = 0 ; iObject < kMaxObjectsPerGroup ; iObject++ )
		{
			LOAD_HSTRING(m_ahstrObjects[iGroup][iObject]);
		}
	}
}

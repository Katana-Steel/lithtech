// ----------------------------------------------------------------------- //
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "iltserver.h"
#include "Attachments.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "AttachButeMgr.h"
#include "WeaponMgr.h"
#include "iltmodel.h"
#include "ilttransform.h"
#include "HHWeaponModel.h"
#include "Spawner.h"
#include "GameServerShell.h"
#include "PlayerObj.h"
#include "Prop.h"
#include "AIUtils.h" // for AIErrorPrint
#include "MultiplayerMgr.h"


// Static constants
const char c_szNoAttachment[] = NO_ATTACHMENT;
const char c_szDefaultAttachment[] = DEFAULT_ATTACHMENT;

const int g_nAIInfiniteAmmo = INT_MAX - 123;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachment::CAttachment
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAttachment::CAttachment()
{
    m_hModel        = LTNULL;
    m_hObject       = LTNULL;
	m_nAttachmentID	= -1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachment::~CAttachment
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAttachment::~CAttachment()
{
	if ( m_hObject && m_hModel )
	{
        HATTACHMENT hAttachment = LTNULL;

        g_pLTServer->FindAttachment(m_hObject, m_hModel, &hAttachment);

        if ( hAttachment != LTNULL )
		{
            g_pLTServer->RemoveAttachment(hAttachment);
		}

        g_pLTServer->RemoveObject(m_hModel);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachment::Init
//
//	PURPOSE:	Initialilze the attachment
//
// ----------------------------------------------------------------------- //

void CAttachment::Init(HOBJECT hObj, HOBJECT hModel, int nAttachmentID)
{
	m_hObject = hObj;
	m_hModel = hModel;
	m_nAttachmentID = nAttachmentID;

    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(hModel);
    g_pLTServer->SetObjectUserFlags(hModel, dwUsrFlags | USRFLG_ATTACH_HIDE1SHOW3);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachment::Save
//
//	PURPOSE:	Save
//
// ----------------------------------------------------------------------- //

void CAttachment::Save(HMESSAGEWRITE hWrite)
{
	SAVE_HOBJECT(m_hModel);
	SAVE_HOBJECT(m_hObject);
	SAVE_INT(m_nAttachmentID);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachment::Load
//
//	PURPOSE:	Load
//
// ----------------------------------------------------------------------- //

void CAttachment::Load(HMESSAGEREAD hRead)
{
	LOAD_HOBJECT(m_hModel);
	LOAD_HOBJECT(m_hObject);
	LOAD_INT(m_nAttachmentID);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::CAttachments
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAttachments::CAttachments(LTBOOL bAutoInit) : IAggregate()
{
    m_hObject = LTNULL;

	m_cAttachmentPositions = 0;

	m_cWeapons = 0;
	m_cObjects = 0;
	m_cProps = 0;

	m_dwFlags = FLAG_VISIBLE | FLAG_GOTHRUWORLD | FLAG_PORTALVISIBLE | FLAG_FORCECLIENTUPDATE;

	m_bAutoInit		= bAutoInit;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::~CAttachments
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAttachments::~CAttachments()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //

uint32 CAttachments::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp(pObject, (ObjectCreateStruct*)pData);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				if(m_bAutoInit)
				{
					Init(pObject->m_hObject);
				}
			}
			break;
		}

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_LINKBROKEN :
		{
			HandleLinkBroken( (HOBJECT)pData );
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint8)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint8)fData);
		}
		break;
	}

    return IAggregate::EngineMessageFn(pObject, messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CAttachments::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HandleTrigger(pObject, hSender, hRead);
		}
		break;

		default : break;
	}

	{for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		m_apAttachmentPositions[iAttachmentPosition]->ObjectMessageFn(pObject, hSender, messageID, hRead);
	}}

    return IAggregate::ObjectMessageFn(pObject, hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CAttachments::ReadProp(BaseClass *pObject, ObjectCreateStruct *pStruct)
{
	_ASSERT(pObject && pStruct);
	if (!pObject || !pStruct) return LTFALSE;

	GenericProp genProp;

	{for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
        if ( g_pLTServer->GetPropGeneric( const_cast<char *>(m_apAttachmentPositions[iAttachmentPosition]->GetName().c_str()), &genProp ) == LT_OK )
		{
			if ( genProp.m_String[0] )
			{
				// We need interpret "<none>" as default.
				if( 0 != stricmp(genProp.m_String, c_szDefaultAttachment) && 0 != stricmp(genProp.m_String, "<none>") )
				{
					m_apAttachmentPositions[iAttachmentPosition]->SetAttachmentName(genProp.m_String);
					m_aAttachmentOverrides[iAttachmentPosition] = genProp.m_String;
				}
			}
		}
	}}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::HandleLinkBroken
//
//	PURPOSE:	Removes any attachment with this object as its model object.
//
// ----------------------------------------------------------------------- //
void CAttachments::HandleLinkBroken( HOBJECT hRemovedObject )
{
	{for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		ASSERT( m_apAttachmentPositions[iAttachmentPosition] );

		CAttachment * pAttachment = m_apAttachmentPositions[iAttachmentPosition]->GetAttachment();
		if( pAttachment )
		{
			if( pAttachment->GetModel() == hRemovedObject )
			{
				Detach(m_apAttachmentPositions[iAttachmentPosition], LTFALSE);
			}
		}
	}}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Attach
//
//	PURPOSE:	Dynamically add an attachment
//
// ----------------------------------------------------------------------- //

void CAttachments::Attach(const char* szAttachmentPosition, const char* szAttachment)
{
	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachmentPosition* pAttachmentPosition = m_apAttachmentPositions[iAttachmentPosition];

		if ( !_stricmp(szAttachmentPosition, pAttachmentPosition->GetName().c_str()) )
		{
			if ( pAttachmentPosition->HasAttachment() )
			{
				_ASSERT(!"CAttachments::Attach -- tried attaching to an occupied position");
			}
			else
			{
                pAttachmentPosition->SetAttachmentName((char*)szAttachment);
				CreateAttachment(pAttachmentPosition);
			}

			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Detach
//
//	PURPOSE:	Dynamically remove an attachment
//
// ----------------------------------------------------------------------- //

void CAttachments::Detach(const char* szAttachmentPosition)
{
	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachmentPosition* pAttachmentPosition = m_apAttachmentPositions[iAttachmentPosition];

		if ( !_stricmp(szAttachmentPosition, pAttachmentPosition->GetName().c_str()) )
		{
			Detach(pAttachmentPosition);
		}
	}
}

void CAttachments::Detach(CAttachmentPosition* pAttachmentPosition, LTBOOL bBreakLink /* = LTTRUE */ )
{
	CAttachment* pAttachment = pAttachmentPosition->GetAttachment();
	if ( pAttachment )
	{
		switch ( pAttachment->GetType() )
		{
			case ATTACHMENT_TYPE_WEAPON:
				m_cWeapons--;
				break;

			case ATTACHMENT_TYPE_PROP:
				m_cProps--;
				break;

			case ATTACHMENT_TYPE_OBJECT:
				m_cObjects--;
				break;
		}

		if( bBreakLink && pAttachment->GetModel() )
			g_pLTServer->BreakInterObjectLink(m_hObject,pAttachment->GetModel());

		delete pAttachment;

		pAttachmentPosition->SetAttachment(NULL);
		pAttachmentPosition->SetAttachmentName(c_szNoAttachment);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::CreateAttachment
//
//	PURPOSE:	Creates an attachment
//
// ----------------------------------------------------------------------- //

void CAttachments::CreateAttachment(CAttachmentPosition *pAttachmentPosition)
{
	_ASSERT(pAttachmentPosition);
	if ( !pAttachmentPosition || !m_hObject ) return;

	if ( !g_pWeaponMgr || !g_pAttachButeMgr ) return;

	if ( pAttachmentPosition->HasAttachment() ) return;
	if ( !pAttachmentPosition->HasAttachmentName() ) return;

    const char *szAttachmentName = pAttachmentPosition->GetAttachmentName().c_str();

    if ( !_stricmp(c_szNoAttachment, szAttachmentName) ) return;

	uint8 nWeaponID		= WMGR_INVALID_ID;
	uint8 nBarrelID		= WMGR_INVALID_ID;
	uint8 nAmmoID		= WMGR_INVALID_ID;
	uint8 nAmmoPoolID	= WMGR_INVALID_ID;

	char szAttachmentNameCopy[128];
	strcpy(szAttachmentNameCopy, szAttachmentName);

	g_pWeaponMgr->ReadWeapon(szAttachmentNameCopy, nWeaponID, nBarrelID, nAmmoID, nAmmoPoolID);

	// This is for AI's which only use Barrel instead of full weapons
	if( nWeaponID == WMGR_INVALID_ID )
	{
		g_pWeaponMgr->ReadBarrel(szAttachmentNameCopy, nBarrelID, nAmmoID, nAmmoPoolID);
		if( nBarrelID != WMGR_INVALID_ID )
		{
			nWeaponID = g_pWeaponMgr->FindWeaponWithBarrel(nBarrelID);
		}
	}


	// This verifies that it's a weapon... so attach it.
	if ( nWeaponID != WMGR_INVALID_ID )
	{
		CreateWeaponAttachment(pAttachmentPosition, szAttachmentName, nWeaponID, nBarrelID);
		return;
	}


	// So it's not a weapon huh... alright, search the attributes for something else
	int nAttachmentID = g_pAttachButeMgr->GetAttachmentIDByName(szAttachmentName);

	// Somethin' got screwed up... so throw out some debug info
	if ( -1 == nAttachmentID )
	{
#ifndef _FINAL
        AIErrorPrint(m_hObject, "Unknown attachment \"%s\".", szAttachmentName);
#endif
		return;
	}

	// Hook it up!
	switch ( g_pAttachButeMgr->GetAttachmentType(nAttachmentID) )
	{
		case ATTACHMENT_TYPE_OBJECT:
		{
			CreateObjectAttachment(pAttachmentPosition, nAttachmentID);
		}
		break;

		case ATTACHMENT_TYPE_PROP:
		{
			CreatePropAttachment(pAttachmentPosition, nAttachmentID);
		}
		break;

		default:
		{
#ifndef _FINAL
            g_pLTServer->CPrint("Illegal attachment type encountered on attachment #%d", nAttachmentID);
#endif
			return;
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::CreateWeaponAttachment
//
//	PURPOSE:	Creates a weapon attachment
//
// ----------------------------------------------------------------------- //

void CAttachments::CreateWeaponAttachment(CAttachmentPosition *pAttachmentPosition, const char* szAttachmentName, uint8 nWeaponID, uint8 nBarrelID)
{
	// Prepare the object create struct

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

#ifdef _DEBUG
	sprintf(theStruct.m_Name, "%s-%s", g_pLTServer->GetObjectName(m_hObject), szAttachmentName );
#endif

	theStruct.m_Flags = m_dwFlags;
	theStruct.m_NextUpdate = 1.0f;

	// Get the attachment skin, model, and class names

	char szModel[MAX_CS_FILENAME_LEN];
	char szSkin[MAX_CS_FILENAME_LEN];
	char szClass[128];

	szModel[0] = 0;
	szSkin[0] = 0;

	CAttachmentWeapon* pAttachment = new CAttachmentWeapon();
	strcpy(szClass, "CHHWeaponModel");

	// Parse weapon/ammo info
	// Set up the model/skin filenames

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponID);
	if (pWeapon)
	{
		if (pWeapon->szHHModel[0])
		{
			SAFE_STRCPY(theStruct.m_Filename, pWeapon->szHHModel);
		}

		for(int i = 0; i < MAX_MODEL_TEXTURES; i++)
		{
			if (pWeapon->szHHSkins[i])
			{
				SAFE_STRCPY(theStruct.m_SkinNames[i], pWeapon->szHHSkins[i]);
			}
		}
	}


	// Create the attachment

    HCLASS hClass = g_pLTServer->GetClass(szClass);
    LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
	if ( !pModel ) return;

	// Give it some sane dimensions.
	LTVector vDims(5,5,5);
	g_pLTServer->SetObjectDims(pModel->m_hObject,&vDims);

	// Be sure we're notified if the model is removed!
	g_pLTServer->CreateInterObjectLink(m_hObject, pModel->m_hObject);

	// Attach it

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, pModel->m_hObject, const_cast<char *>(pAttachmentPosition->GetName().c_str()),
												 &LTVector(0,0,0), &LTRotation(0,0,0,1), &hAttachment);

	pAttachment->Init(m_hObject, pModel->m_hObject, -1, nWeaponID, nBarrelID);

	pAttachmentPosition->SetAttachment(pAttachment);

	m_cWeapons++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::CreateObjectAttachment
//
//	PURPOSE:	Creates an object attachment
//
// ----------------------------------------------------------------------- //

void CAttachments::CreateObjectAttachment(CAttachmentPosition *pAttachmentPosition, int nAttachmentID)
{
	// Prepare the object create struct

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

#ifdef _DEBUG
	sprintf(theStruct.m_Name, "%s-Object%d", g_pLTServer->GetObjectName(m_hObject), nAttachmentID );
#endif

	theStruct.m_Flags = m_dwFlags;
	theStruct.m_NextUpdate = 1.0f;

	// Get the attachment skin, model, and class names

	int nAttachmentType = g_pAttachButeMgr->GetAttachmentType(nAttachmentID);

	char szClass[128];
	szClass[0] = '\0';

	SAFE_STRCPY(szClass, g_pAttachButeMgr->GetAttachmentClass(nAttachmentID));
	if ( !szClass[0] ) return;


	// Set up the model/skin filenames

	SAFE_STRCPY(theStruct.m_Filename, g_pAttachButeMgr->GetAttachmentModel(nAttachmentID));
	SAFE_STRCPY(theStruct.m_SkinName, g_pAttachButeMgr->GetAttachmentSkin(nAttachmentID));

	// Create the attachment

    HCLASS hClass = g_pLTServer->GetClass(szClass);
    LPBASECLASS pModel = g_pLTServer->CreateObjectProps(hClass, &theStruct, (char*)(LPCSTR)g_pAttachButeMgr->GetAttachmentProperties(nAttachmentID));
	if ( !pModel ) 
	{
#ifndef _FINAL
		AIErrorPrint(m_hObject, "Could not create attachment \"%s\".", theStruct.m_Filename );
#endif
		return;
	}

	// Give it some sane dimensions.
	LTVector vDims(5,5,5);
	g_pLTServer->SetObjectDims(pModel->m_hObject,&vDims);

	// Be sure we're notified if the model is removed!
	g_pLTServer->CreateInterObjectLink(m_hObject, pModel->m_hObject);

	// Set the alpha value
	LTFLOAT r, g, b, a;
	g_pLTServer->GetObjectColor(pModel->m_hObject, &r, &g, &b, &a);
	a = g_pAttachButeMgr->GetAttachmentAlpha(nAttachmentID);
	g_pLTServer->SetObjectColor(pModel->m_hObject, r, g, b, a);

	// Attach it

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, pModel->m_hObject, const_cast<char *>(pAttachmentPosition->GetName().c_str()),
											   &LTVector(0,0,0), &LTRotation(0,0,0,1), &hAttachment);

	CAttachment* pAttachment =  new CAttachmentObject();
	pAttachment->Init(m_hObject, pModel->m_hObject, nAttachmentID);
	pAttachmentPosition->SetAttachment(pAttachment);

	m_cObjects++;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::CreatePropAttachment
//
//	PURPOSE:	Creates a prop attachment
//
// ----------------------------------------------------------------------- //

void CAttachments::CreatePropAttachment(CAttachmentPosition *pAttachmentPosition, int nAttachmentID)
{
	// Prepare the object create struct

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

#ifdef _DEBUG
	sprintf(theStruct.m_Name, "%s-Prop%d", g_pLTServer->GetObjectName(m_hObject), nAttachmentID );
#endif

	theStruct.m_Flags = m_dwFlags;
	theStruct.m_NextUpdate = 1.0f;

	// Get the attachment skin, model, and class names

	int nAttachmentType = g_pAttachButeMgr->GetAttachmentType(nAttachmentID);

	char szModel[MAX_CS_FILENAME_LEN];
	char szSkin[MAX_CS_FILENAME_LEN];
	char szClass[128];

	CAttachment* pAttachment = new CAttachmentProp();
	strcpy(szClass, "Prop");
	strcpy(szModel, g_pAttachButeMgr->GetAttachmentModel(nAttachmentID));
	strcpy(szSkin, g_pAttachButeMgr->GetAttachmentSkin(nAttachmentID));

	// Set up the model/skin filenames

	SAFE_STRCPY(theStruct.m_Filename, szModel);
	SAFE_STRCPY(theStruct.m_SkinName, szSkin);

	// Create the attachment

    HCLASS hClass = g_pLTServer->GetClass(szClass);
    Prop* pModel = (Prop*)g_pLTServer->CreateObjectProps(hClass, &theStruct, (char*)(LPCSTR)g_pAttachButeMgr->GetAttachmentProperties(nAttachmentID));
	if ( !pModel ) 
	{
#ifndef _FINAL
		AIErrorPrint(m_hObject, "Could not create attachment \"%s\".", szModel );
#endif
		return;
	}

	// Give it some sane dimensions.
	LTVector vDims(5,5,5);
	g_pLTServer->SetObjectDims(pModel->m_hObject,&vDims);

	// Be sure we're notified if the model is removed!
	g_pLTServer->CreateInterObjectLink(m_hObject, pModel->m_hObject);

	// Set the alpha value
	LTFLOAT r, g, b, a;
	g_pLTServer->GetObjectColor(pModel->m_hObject, &r, &g, &b, &a);
	a = g_pAttachButeMgr->GetAttachmentAlpha(nAttachmentID);
	g_pLTServer->SetObjectColor(pModel->m_hObject, r, g, b, a);


	// Make it indestructible.
	pModel->GetDestructible()->SetNeverDestroy(LTTRUE);
	pModel->GetDestructible()->SetCanDamage(LTFALSE);

	LTRotation rRot;
    g_pLTServer->GetObjectRotation(pModel->m_hObject, &rRot);

	// Set the flags again, Prop has to go and add stuff to us!
	g_pLTServer->SetObjectFlags(pModel->m_hObject, m_dwFlags);

	// Attach it

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, pModel->m_hObject, const_cast<char *>(pAttachmentPosition->GetName().c_str()),
											   &LTVector(0,0,0), &rRot, &hAttachment);

	pAttachment->Init(m_hObject, pModel->m_hObject, nAttachmentID);
	pAttachmentPosition->SetAttachment(pAttachment);

	m_cProps++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::ReInit
//
//	PURPOSE:	Re-initialize the attachments with a new object
//
// ----------------------------------------------------------------------- //

void CAttachments::ReInit(HOBJECT hNewObj)
{
	if (!hNewObj || !m_hObject) return;

	HOBJECT hOldObj = m_hObject;
	m_hObject = hNewObj;

	for (int i=0; i < m_cAttachmentPositions; i++)
	{
		CAttachmentPosition *pCurPos = m_apAttachmentPositions[i];
		if (!pCurPos) break;

		CAttachment* pCur = pCurPos->GetAttachment();

		if (pCur)
		{
			HOBJECT hModel = pCur->GetModel();

			if (hModel)
			{
				HATTACHMENT hAttachment;
                if (LT_OK == g_pLTServer->FindAttachment(hOldObj, hModel, &hAttachment))
				{
					g_pLTServer->BreakInterObjectLink(hOldObj,hModel);
                    g_pLTServer->RemoveAttachment(hAttachment);
				}

				// Attach the model to us...

                g_pLTServer->CreateAttachment(m_hObject, hModel, const_cast<char *>(pCurPos->GetName().c_str()),
					&LTVector(0,0,0), &LTRotation(0,0,0,1), &hAttachment);

				// Don't forget to link to it!
				g_pLTServer->CreateInterObjectLink(m_hObject, hModel);

				// Re-init to set m_hObject...

				pCur->Init(m_hObject, hModel, pCur->GetID());
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::GetAttachmentPosition
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

CAttachmentPosition * CAttachments::GetAttachmentPosition(const char * szAttachmentPosition)
{
	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachmentPosition* pAttachmentPosition = m_apAttachmentPositions[iAttachmentPosition];

		if ( !_stricmp(szAttachmentPosition, pAttachmentPosition->GetName().c_str()) )
		{
			return pAttachmentPosition;
		}
	}

	_ASSERT(0);

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::EnumerateWeapons
//
//	PURPOSE:	Fills out an array with a list of all weapon attachments
//
// ----------------------------------------------------------------------- //

int CAttachments::EnumerateWeapons(CWeapon** apWeapons, CAttachmentPosition** apAttachmentPositions, int cMaxWeapons)
{
	int cWeapons = 0;

	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachment* pAttachment = m_apAttachmentPositions[iAttachmentPosition]->GetAttachment();
		if ( pAttachment && (pAttachment->GetType() == ATTACHMENT_TYPE_WEAPON) )
		{
			CAttachmentWeapon* pAttachmentWeapon = (CAttachmentWeapon*)pAttachment;
            CHHWeaponModel* pHHWeaponModel = dynamic_cast<CHHWeaponModel*>(g_pLTServer->HandleToObject(pAttachmentWeapon->GetModel()));
			if(!pHHWeaponModel) continue;

			apAttachmentPositions[cWeapons] = m_apAttachmentPositions[iAttachmentPosition];
			apWeapons[cWeapons] = pHHWeaponModel->GetParent();

			if ( ++cWeapons == cMaxWeapons )
			{
				break;
			}
		}
	}

	return cWeapons;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::EnumerateProps
//
//	PURPOSE:	Fills out an array with a list of all Prop attachments
//
// ----------------------------------------------------------------------- //

int CAttachments::EnumerateProps(Prop** apProps, CAttachmentPosition** apAttachmentPositions, int cMaxProps)
{
	int cProps = 0;

	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachment* pAttachment = m_apAttachmentPositions[iAttachmentPosition]->GetAttachment();
		if ( pAttachment && (pAttachment->GetType() == ATTACHMENT_TYPE_PROP) )
		{
			CAttachmentProp* pAttachmentProp = (CAttachmentProp*)pAttachment;
            Prop* pProp = dynamic_cast<Prop*>(g_pLTServer->HandleToObject(pAttachmentProp->GetModel()));
			if(!pProp) continue;

			apAttachmentPositions[cProps] = m_apAttachmentPositions[iAttachmentPosition];
			apProps[cProps] = pProp;

			if ( ++cProps == cMaxProps )
			{
				break;
			}
		}
	}

	return cProps;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::EnumerateObjects
//
//	PURPOSE:	Fills out an array with a list of all Object attachments
//
// ----------------------------------------------------------------------- //

int CAttachments::EnumerateObjects(BaseClass** apObjects, CAttachmentPosition** apAttachmentPositions, int cMaxObjects)
{
	int cObjects = 0;

	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachment* pAttachment = m_apAttachmentPositions[iAttachmentPosition]->GetAttachment();
		if ( pAttachment && (pAttachment->GetType() == ATTACHMENT_TYPE_OBJECT) )
		{
			CAttachmentObject* pAttachmentObject = (CAttachmentObject*)pAttachment;
            BaseClass* pObject = g_pLTServer->HandleToObject(pAttachmentObject->GetModel());

			apAttachmentPositions[cObjects] = m_apAttachmentPositions[iAttachmentPosition];
			apObjects[cObjects] = pObject;

			if ( ++cObjects == cMaxObjects )
			{
				break;
			}
		}
	}

	return cObjects;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::GetInfiniteAmmo
//
//	PURPOSE:	Used by the AIs to get a big mess of ammunition
//
// ----------------------------------------------------------------------- //

void CAttachments::GetInfiniteAmmo()
{
	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachment* pAttachment = m_apAttachmentPositions[iAttachmentPosition]->GetAttachment();
		if ( pAttachment && (pAttachment->GetType() == ATTACHMENT_TYPE_WEAPON) )
		{
			CAttachmentWeapon* pAttachmentWeapon = (CAttachmentWeapon*)pAttachment;
			CWeapons* pWeapons = pAttachmentWeapon->GetWeapons();

			int cAmmoPools = g_pWeaponMgr->GetNumAmmoPools();

			for ( int iAmmoPools = 0 ; iAmmoPools < cAmmoPools ; iAmmoPools++ )
			{
				pWeapons->AddAmmo(iAmmoPools, g_nAIInfiniteAmmo);
			}

			pWeapons->GetCurWeapon()->ReloadClip();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Init
//
//	PURPOSE:	Initializes us
//
// ----------------------------------------------------------------------- //

void CAttachments::Init(HOBJECT hObject)
{
	if (!hObject) return;
	if (!m_hObject) m_hObject = hObject;

	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CreateAttachment(m_apAttachmentPositions[iAttachmentPosition]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::ResetRequirements
//
//	PURPOSE:	Initializes us
//
// ----------------------------------------------------------------------- //

void CAttachments::ResetRequirements()
{
	// We should have one or zero weapons.
	_ASSERT( m_cWeapons == 0 || m_cWeapons == 1 );

	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachmentPosition* pAttachmentPosition = m_apAttachmentPositions[iAttachmentPosition];
		CAttachment* pAttachment = m_apAttachmentPositions[iAttachmentPosition]->GetAttachment();

		if ( pAttachment && pAttachment->GetType() != ATTACHMENT_TYPE_WEAPON )
		{
			Detach(pAttachmentPosition);
		}
	}

	// We should have no more props.
	_ASSERT( m_cProps == 0 );
	_ASSERT( m_cObjects == 0 );

	// We should have one or zero weapons.
	_ASSERT( m_cWeapons == 0 || m_cWeapons == 1 );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::AddRequirements
//
//	PURPOSE:	Initializes us
//
// ----------------------------------------------------------------------- //

void CAttachments::AddRequirements(int nCharacterSet)
{
	CString szModel = g_pCharacterButeMgr->GetModelType(nCharacterSet);

	int anRequirements[32];
	int cRequirements = g_pAttachButeMgr->GetRequirementIDs(szModel.GetBuffer(0), anRequirements, 32);

	for ( int iRequirement = 0 ; iRequirement < cRequirements ; iRequirement++ )
	{
		int nAttachment = g_pAttachButeMgr->GetRequirementAttachment(anRequirements[iRequirement]);
		CString sSocket = g_pAttachButeMgr->GetRequirementSocket(anRequirements[iRequirement]);
		CString sAttachment = g_pAttachButeMgr->GetAttachmentName(nAttachment);

		Attach(sSocket, sAttachment);
	}

	// Re-add the overrides.
	for( int iOverride = 0; iOverride < m_cAttachmentPositions; ++iOverride )
	{
		if( !m_aAttachmentOverrides[iOverride].empty() && m_apAttachmentPositions[iOverride] )
		{
			Detach(m_apAttachmentPositions[iOverride]);
			Attach(m_apAttachmentPositions[iOverride]->GetName().c_str(), m_aAttachmentOverrides[iOverride].c_str());
		}
	}
			
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Update
//
//	PURPOSE:	Handle object updates
//
// ----------------------------------------------------------------------- //

void CAttachments::Update()
{
	if( m_hObject )
	{
		const uint32 dwOldFlags = m_dwFlags;
		const uint32 dwOwnerFlags = g_pLTServer->GetObjectFlags(m_hObject);
		if( (dwOwnerFlags & FLAG_VISIBLE) && !(m_dwFlags & FLAG_VISIBLE) )
		{
			m_dwFlags |= FLAG_VISIBLE;
			m_dwFlags |= FLAG_PORTALVISIBLE;
		}
		else if( !(dwOwnerFlags & FLAG_VISIBLE) && (m_dwFlags & FLAG_VISIBLE) )
		{
			m_dwFlags &= ~FLAG_VISIBLE;
			m_dwFlags &= ~FLAG_PORTALVISIBLE;
		}

		if( m_dwFlags != dwOldFlags )
		{
			for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
			{
				CAttachmentPosition* pAttachmentPosition = m_apAttachmentPositions[iAttachmentPosition];

				if (   pAttachmentPosition 
					&& pAttachmentPosition->GetAttachment()
					&& pAttachmentPosition->GetAttachment()->GetModel() )
				{
					g_pLTServer->SetObjectFlags(pAttachmentPosition->GetAttachment()->GetModel(), m_dwFlags);
				}
			}
		}
	}

	/*	// Put all attachments in their place

	{for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachmentPosition* pAttachmentPosition = m_apAttachmentPositions[iAttachmentPosition];

		if ( pAttachmentPosition->GetAttachment()->HasObject() )
		{
			// Remove the attached model

			HOBJECT hModel = pAttachmentPosition->GetAttachment()->GetObject();

			HATTACHMENT hAttachment;
            if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hModel, &hAttachment) )
			{
                g_pLTServer->RemoveAttachment(hAttachment);
			}

			// Determine the direction to point

			LTRotation rRotModel;
            g_pLTServer->GetObjectRotation(hModel, &rRotModel);

			HMODELSOCKET hSocket;
			g_pModelLT->GetSocket(m_hObject, (char*)pAttachmentPosition->GetName(), hSocket);

			LTransform transform;
			g_pModelLT->GetSocketTransform(m_hObject, hSocket, transform, LTTRUE);

			LTRotation rRotSocket;
			g_pTransLT->GetRot(transform, rRotSocket);

            LTMatrix m1, m2, m3;

            g_pLTServer->SetupRotationMatrix(&m1, &rRotModel);
            g_pLTServer->SetupRotationMatrix(&m2, &rRotSocket);

			MatTranspose3x3(&m1);
			MatMul(&m3, &m2, &m1);
            g_pLTServer->SetupRotationFromMatrix(&rRotModel, &m3);

            g_pLTServer->SetObjectRotation(hModel, &rRotModel);

            LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, hModel, (char*)pAttachmentPosition->GetName(),
													   &LTVector(0,0,0), &rRotModel, &hAttachment);
		}
	}}
*/}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::HandleDeath
//
//	PURPOSE:	Handles being killed
// 
// ----------------------------------------------------------------------- //

void CAttachments::HandleDeath()
{
	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachment* pAttachment = m_apAttachmentPositions[iAttachmentPosition]->GetAttachment();
		if ( pAttachment && (pAttachment->GetType() == ATTACHMENT_TYPE_WEAPON) )
		{
			DropAttachment(m_apAttachmentPositions[iAttachmentPosition]);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::DropAttachment
//
//	PURPOSE:	Drops an attachment
// 
// ----------------------------------------------------------------------- //

void CAttachments::DropAttachment(CAttachmentPosition* pAttachmentPosition, LTBOOL bRemove /* = LTTRUE */)
{
	CAttachment* pAttachment = pAttachmentPosition->GetAttachment();

	// Get the position of the attachment

	LTVector vPos;
	LTRotation rRot;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	// TODO: possible optimzation, things that just stay as static attachments, just move instead of
	// destroying and respawning

	HMODELSOCKET hSocket;
	if ( g_pModelLT->GetSocket(m_hObject, const_cast<char *>(pAttachmentPosition->GetName().c_str()), hSocket) == LT_OK )
	{
		LTransform transform;
		if ( g_pModelLT->GetSocketTransform(m_hObject, hSocket, transform, LTTRUE) == LT_OK )
		{
			g_pTransLT->Get(transform, vPos, rRot);
		}
	}

	char szSpawn[1024];
	pAttachment->CreateSpawnString(szSpawn);

	if(szSpawn[0] != '\0')
	{
		BaseClass* pObj = SpawnObject(szSpawn, vPos, rRot);

		if(pObj && pObj->m_hObject)
		{
			LTVector vVel(GetRandom(-100.0f, 100.0f), GetRandom(400.0f, 600.0f), GetRandom(-100.0f, 100.0f));
			g_pLTServer->SetVelocity(pObj->m_hObject, &vVel);
		}
	}

	if ( bRemove )
	{
		switch ( pAttachment->GetType() )
		{
			case ATTACHMENT_TYPE_WEAPON:
				m_cWeapons--;
				break;

			case ATTACHMENT_TYPE_PROP:
				m_cProps--;
				break;

			case ATTACHMENT_TYPE_OBJECT:
				m_cObjects--;
				break;
		}

		if( pAttachment->GetModel() )
			g_pLTServer->BreakInterObjectLink(m_hObject,pAttachment->GetModel());

		delete pAttachment;

		pAttachmentPosition->SetAttachment(LTNULL);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::HandleTrigger
//
//	PURPOSE:	Handle trigger message.
//
// ----------------------------------------------------------------------- //

void CAttachments::HandleTrigger(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CAttachments::Save(HMESSAGEWRITE hWrite, uint8 nType)
{
	if (!hWrite) return;

	SAVE_HOBJECT(m_hObject);
	SAVE_INT(m_cAttachmentPositions);
	SAVE_INT(m_cWeapons);
	SAVE_INT(m_cObjects);
	SAVE_INT(m_cProps);

	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		m_apAttachmentPositions[iAttachmentPosition]->Save(hWrite);
		*hWrite << m_aAttachmentOverrides[iAttachmentPosition];
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CAttachments::Load(HMESSAGEREAD hRead, uint8 nType)
{
	if (!hRead) return;

	LOAD_HOBJECT(m_hObject);
	LOAD_INT(m_cAttachmentPositions);
	LOAD_INT(m_cWeapons);
	LOAD_INT(m_cObjects);
	LOAD_INT(m_cProps);

	{for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		m_apAttachmentPositions[iAttachmentPosition]->Load(hRead);
		*hRead >> m_aAttachmentOverrides[iAttachmentPosition];
	}}

	{for ( int iAttachmentPosition = m_cAttachmentPositions ; iAttachmentPosition < MAX_ATTACHMENT_POSITIONS ; iAttachmentPosition++ )
	{
        m_apAttachmentPositions[iAttachmentPosition] = LTNULL;
	}}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHumanAttachments::CAttachments
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CHumanAttachments::CHumanAttachments(LTBOOL bAutoInit) : CAttachments(bAutoInit)
{
	m_RightHand.SetName("RightHand");
	m_LeftHand.SetName("LeftHand");
	m_Head.SetName("Head");
	m_Helmet.SetName("Helmet");
	m_RightShoulder.SetName("RightShoulder");
	m_LeftShoulder.SetName("LeftShoulder");
	m_RightFoot.SetName("RightFoot");
	m_LeftFoot.SetName("LeftFoot");

	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_RightHand;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_LeftHand;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_Head;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_Helmet;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_RightShoulder;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_LeftShoulder;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_RightFoot;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_LeftFoot;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerAttachments::CPlayerAttachments
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPlayerAttachments::CPlayerAttachments(LTBOOL bAutoInit) : CHumanAttachments(bAutoInit)
{
    m_RightHand.SetAttachmentName("Knife");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerAttachments::HandleCheatFullAmmo
//
//	PURPOSE:	Do the full ammo cheat for the player
//
// ----------------------------------------------------------------------- //

void CPlayerAttachments::HandleCheatFullAmmo()
{
    if ( LTNULL != GetDefaultAttachmentWeapon() )
	{
		CWeapons* pWeapons = GetDefaultAttachmentWeapon()->GetWeapons();

        uint8 nNumAmmoPools = g_pWeaponMgr->GetNumAmmoTypes();

		for (int i=0; i < nNumAmmoPools; i++)
		{
			pWeapons->SetAmmo(i);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerAttachments::HandleCheatFullClips
//
//	PURPOSE:	Do the full clips cheat for the player
//
// ----------------------------------------------------------------------- //

void CPlayerAttachments::HandleCheatFullClips()
{
	if ( DNULL != GetDefaultAttachmentWeapon() )
	{
		CWeapons* pWeapons = GetDefaultAttachmentWeapon()->GetWeapons();

		pWeapons->FillAllClips();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerAttachments::HandleCheatFullWeapon
//
//	PURPOSE:	Do the full weapon cheat for the player
//
// ----------------------------------------------------------------------- //

void CPlayerAttachments::HandleCheatFullWeapon()
{
	if ( LTNULL != GetDefaultAttachmentWeapon() )
	{
		HandleCheatFullAmmo();

		CWeapons* pWeapons = GetDefaultAttachmentWeapon()->GetWeapons();

		if(!pWeapons) return;

		CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(pWeapons->GetCharacter()));
		if(!pPlayer) return;

		const CharacterButes* pButes = pPlayer->GetButes();

		WEAPON_SET* pSet = LTNULL;
		
		if(g_pGameServerShell->GetGameType().IsMultiplayer())
		{
			if(g_pGameServerShell->GetGameInfo()->m_bClassWeapons)
				pSet = g_pWeaponMgr->GetWeaponSet(pButes->m_nMPClassWeaponSet);
			else
				pSet = g_pWeaponMgr->GetWeaponSet(pButes->m_nMPWeaponSet);
		}
		else
			pSet = g_pWeaponMgr->GetWeaponSet(pButes->m_nWeaponSet);

		if(!pSet) return;
		
		int cWeapons = pSet->nNumWeapons;

		for ( int i = 0 ; i < cWeapons ; i++ )
		{
			WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(pSet->szWeaponName[i]);

			if(pWeapon && pSet->bCanPickup[i])
				pWeapons->ObtainWeapon(pWeapon->nId, BARREL_DEFAULT_ID, AMMO_DEFAULT_ID, 0, LTTRUE, LTFALSE);
		}

		HandleCheatFullClips();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerAttachments::HandleDeath
//
//	PURPOSE:	Handles being killed
// 
// ----------------------------------------------------------------------- //

void CPlayerAttachments::HandleDeath()
{
//	if ( g_pGameServerShell->GetGameType().IsSinglePlayer() )
//	{
		CAttachments::HandleDeath();
//	}
//	else
//	{
//		for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
//		{
//			CAttachment* pAttachment = m_apAttachmentPositions[iAttachmentPosition]->GetAttachment();
//			if ( pAttachment && (pAttachment->GetType() == ATTACHMENT_TYPE_WEAPON) )
//			{
//				DropAttachment(m_apAttachmentPositions[iAttachmentPosition], DFALSE);
//			}
//		}
//	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentProp::CreateSpawnString
//
//	PURPOSE:	Creates a spawn string for when we are dropped
//
// ----------------------------------------------------------------------- //

void CAttachmentProp::CreateSpawnString(char* szSpawn)
{
	sprintf(szSpawn, "Prop Filename %s; Skin %s; Gravity 0",
		g_pAttachButeMgr->GetAttachmentModel(GetID()), g_pAttachButeMgr->GetAttachmentSkin(GetID()));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentObject::CreateSpawnString
//
//	PURPOSE:	Creates a spawn string for when we are dropped
//
// ----------------------------------------------------------------------- //

void CAttachmentObject::CreateSpawnString(char* szSpawn)
{
	sprintf(szSpawn, "Prop Filename %s; Skin %s; Gravity 0",
		g_pAttachButeMgr->GetAttachmentModel(GetID()), g_pAttachButeMgr->GetAttachmentSkin(GetID()));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentWeapon::Init
//
//	PURPOSE:	Init
//
// ----------------------------------------------------------------------- //

void CAttachmentWeapon::Init(HOBJECT hObj, HOBJECT hWeaponModel, int nAttachmentID, int nWeaponID, int nBarrelID)
{
	CAttachment::Init(hObj, hWeaponModel, nAttachmentID);

	m_Weapons.Init(hObj, hWeaponModel);
	m_Weapons.ObtainWeapon(nWeaponID, nBarrelID);
	m_Weapons.ChangeWeapon(nWeaponID);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentWeapon::CreateSpawnString
//
//	PURPOSE:	Creates a spawn string for when we are dropped
//
// ----------------------------------------------------------------------- //

void CAttachmentWeapon::CreateSpawnString(char* szSpawn)
{
	szSpawn[0] = '\0';

	if(m_Weapons.GetCurWeapon())
	{
		WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_Weapons.GetCurWeapon()->GetId());

		if(pWeapon)
		{
			if(pWeapon->szPickupName[0] != '\0')
			{
				sprintf(szSpawn, "PickupObject Pickup %s; ForceNoRespawn 1", pWeapon->szPickupName);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentWeapon::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CAttachmentWeapon::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	CAttachment::ObjectMessageFn(pObject, hSender, messageID, hRead);

	m_Weapons.ObjectMessageFn(pObject, hSender, messageID, hRead);

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentWeapon::Save
//
//	PURPOSE:	Save
//
// ----------------------------------------------------------------------- //

void CAttachmentWeapon::Save(HMESSAGEWRITE hWrite)
{
	CAttachment::Save(hWrite);

	m_Weapons.Save(hWrite, 0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentWeapon::Load
//
//	PURPOSE:	Load
//
// ----------------------------------------------------------------------- //

void CAttachmentWeapon::Load(HMESSAGEREAD hRead)
{
	CAttachment::Load(hRead);
	m_Weapons.Load(hRead, 0);

	if( m_Weapons.GetCurWeapon() )
	{
		uint8 nWeaponID = m_Weapons.GetCurWeapon()->GetId();
		m_Weapons.DeselectCurWeapon();	// Deselect so we'll change to it
		m_Weapons.ChangeWeapon(nWeaponID);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentPosition::CAttachmentPosition
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAttachmentPosition::CAttachmentPosition()
{
    m_pAttachment = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentPosition::~CAttachmentPosition
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAttachmentPosition::~CAttachmentPosition()
{
	if ( m_pAttachment )
	{
		delete m_pAttachment;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentPosition::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CAttachmentPosition::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	if ( m_pAttachment )
	{
		m_pAttachment->ObjectMessageFn(pObject, hSender, messageID, hRead);
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentPosition::Save
//
//	PURPOSE:	Save
//
// ----------------------------------------------------------------------- //

void CAttachmentPosition::Save(HMESSAGEWRITE hWrite)
{
	SAVE_BOOL(!!m_pAttachment);
	if ( m_pAttachment )
	{
		SAVE_DWORD(m_pAttachment->GetType());
		m_pAttachment->Save(hWrite);
	}

	*hWrite << m_sAttachmentName;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentPosition::Load
//
//	PURPOSE:	Load
//
// ----------------------------------------------------------------------- //

void CAttachmentPosition::Load(HMESSAGEREAD hRead)
{
	LTBOOL bAttachment = LTFALSE;

	LOAD_BOOL(bAttachment);

	if ( bAttachment )
	{
        uint32  dwAttachmentType;

        m_pAttachment = LTNULL;

		LOAD_DWORD(dwAttachmentType);

		switch ( dwAttachmentType )
		{
			case ATTACHMENT_TYPE_PROP:
			{
				m_pAttachment = new CAttachmentProp();
			}
			break;
			case ATTACHMENT_TYPE_OBJECT:
			{
				m_pAttachment = new CAttachmentObject();
			}
			break;
			case ATTACHMENT_TYPE_WEAPON:
			{
				m_pAttachment = new CAttachmentWeapon();
			}
			break;
		}

		_ASSERT(m_pAttachment);

		m_pAttachment->Load(hRead);
	}

	*hRead >> m_sAttachmentName;
}




// Plugin statics
#include "AttachButeMgr.h"

static LTBOOL				sm_bInitted;
static CAttachButeMgr		sm_AttachButeMgr;
static CWeaponMgrPlugin		sm_WeaponMgrPlugin;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentsPlugin::PreHook_EditStringList
//
//	PURPOSE:	Callback handler for EditStringList plugin
//
// ----------------------------------------------------------------------- //

LTRESULT CAttachmentsPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	if ( !sm_bInitted )
	{
		char szFile[256];

#ifdef _WIN32
		sprintf(szFile, "%s\\Attributes\\Attachments.txt", szRezPath);
#else
		sprintf(szFile, "%s/Attributes/Attachments.txt", szRezPath);
#endif
		sm_AttachButeMgr.SetInRezFile(LTFALSE);
        sm_AttachButeMgr.Init(g_pLTServer, szFile);
		sm_bInitted = LTTRUE;
		sm_WeaponMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

	}

	if ( 0 )
	{
		PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		return LT_OK;
	}
	else
	{
		return LT_UNSUPPORTED;
	}
};

// ----------------------------------------------------------------------- //

void CAttachmentsPlugin::PopulateStringList(
	char* const * aszStrings,
	uint32* pcStrings,
	const uint32 cMaxStrings,
	const uint32 cMaxStringLength)
{
	// TODO: make sure we don't overflow cMaxStringLength or cMaxStrings
    uint32 cAttachments = sm_AttachButeMgr.GetNumAttachments();
	_ASSERT(cMaxStrings >= cAttachments);

	strcpy(aszStrings[(*pcStrings)++], c_szDefaultAttachment);
	strcpy(aszStrings[(*pcStrings)++], c_szNoAttachment);

    for ( uint32 iAttachment = 0 ; iAttachment < cAttachments ; iAttachment++ )
	{
		strcpy(aszStrings[(*pcStrings)++], sm_AttachButeMgr.GetAttachmentName(iAttachment));
	}

//	sm_WeaponMgrPlugin.PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentsPlugin::PreHook_EditStringList
//
//	PURPOSE:	Callback handler for EditStringList plugin
//
// ----------------------------------------------------------------------- //

LTRESULT CHumanAttachmentsPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	if ( LT_OK == CAttachmentsPlugin::PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}

    if ( !_stricmp("LeftHand",      szPropName) ||
         !_stricmp("RightHand",     szPropName) ||
         !_stricmp("LeftFoot",      szPropName) ||
         !_stricmp("RightFoot",     szPropName) ||
         !_stricmp("LeftShoulder",  szPropName) ||
         !_stricmp("RightShoulder", szPropName) ||
         !_stricmp("Head",          szPropName) ||
		 !_stricmp("Helmet",		szPropName) ||
         !_stricmp("Eyes",          szPropName) ||
         !_stricmp("Nose",          szPropName) ||
         !_stricmp("Chin",          szPropName) ||
         !_stricmp("Back",          szPropName) )
	{
		PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		return LT_OK;
	}
	else
	{
		return LT_UNSUPPORTED;
	}
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCreatureAttachmentsPlugin::PreHook_EditStringList
//
//	PURPOSE:	Callback handler for EditStringList plugin
//
// ----------------------------------------------------------------------- //

LTRESULT CCreatureAttachmentsPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	if ( LT_OK == CAttachmentsPlugin::PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}

	if ( 0 )
	{
		PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		return LT_OK;
	}
	else
	{
		return LT_UNSUPPORTED;
	}
};


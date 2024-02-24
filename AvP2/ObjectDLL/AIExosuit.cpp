// ----------------------------------------------------------------------- //
//
// MODULE  : AIExosuit.cpp
//
// PURPOSE : AIExosuit object - Implementation
//
// CREATED : 1/1/01
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "AIExosuit.h"
#include "AIButeMgr.h"
#include "Prop.h"
#include "SharedFXStructs.h"
#include "AIUtils.h" // for AICPrint
#include "AINode.h"
#include "AttachButeMgr.h"
#include "ServerSoundMgr.h"

BEGIN_CLASS(AIExosuit)
	ADD_STRINGPROP_FLAG(AttributeTemplate, "BasicExosuit", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(CharacterType, "Corporate_Exosuit", PF_STATICLIST)
	ADD_COLORPROP_FLAG_HELP(DEditColor, 255.0f, 0.0f, 255.0f, 0, "Color used for object in DEdit.") 

	// These need to be here in order to get CAIMarinePlugin to fill the static list.
	ADD_STRINGPROP_FLAG_HELP(Weapon, "", PF_STATICLIST, "AI's primary weapon.")
	ADD_STRINGPROP_FLAG_HELP(SecondaryWeapon, "", PF_STATICLIST, "If AI cannot use primary weapon, use this weapon.")
	ADD_STRINGPROP_FLAG_HELP(TertiaryWeapon, "", PF_STATICLIST, "If AI cannot use primary or secondary weapon, use this weapon.")

END_CLASS_DEFAULT_FLAGS_PLUGIN(AIExosuit, CAI, NULL, NULL, 0, CAIExosuitPlugin)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CAI()
//
//	PURPOSE:	Constructor - Initialize
//
// ----------------------------------------------------------------------- //

AIExosuit::AIExosuit()
	: m_hWindshield(LTNULL)
{
	// Default the character to the FemaleLabtech
	SetCharacter(g_pCharacterButeMgr->GetSetFromModelType("Corporate_Exosuit_AI"));
}

AIExosuit::~AIExosuit()
{
	if (m_hHead)
	{
		HATTACHMENT hAttachment;
		if (LT_OK == g_pLTServer->FindAttachment(m_hObject, m_hHead, &hAttachment))
		{
			g_pLTServer->RemoveAttachment(hAttachment);
		}

		g_pLTServer->RemoveObject(m_hHead);
		m_hHead = LTNULL;
	}

	if (m_hWindshield)
	{
		HATTACHMENT hAttachment;
		if (LT_OK == g_pLTServer->FindAttachment(m_hObject, m_hWindshield, &hAttachment))
		{
			g_pLTServer->RemoveAttachment(hAttachment);
		}

		g_pLTServer->RemoveObject(m_hWindshield);
		m_hWindshield = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIExosuit::GetAimFromPosition
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //


LTVector AIExosuit::GetAimFromPosition() const
{
	const CWeapon * pWeapon = const_GetCurrentWeaponPtr();
	
	LTVector vAimFrom = GetPosition();

	if( pWeapon )
	{
		const LTVector & vWeaponPos = GetWeaponPosition(pWeapon);
		const LTVector & vMyRight = GetRight();

		if( vWeaponPos.Dot( vMyRight ) > 0.0f )
		{
			vAimFrom += vMyRight*GetDims().x;
		}
		else
		{
			vAimFrom -= vMyRight*GetDims().x;
		}
	}

	return vAimFrom;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIExosuit::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD AIExosuit::EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_LINKBROKEN:
		{
			HOBJECT hBreaker = (HOBJECT) pData;

			if( m_hHead && hBreaker == m_hHead )
			{
				// Clear our look at, because our object just
				// went away.

				HATTACHMENT hAttachment;
				if( LT_OK == g_pLTServer->FindAttachment(m_hObject, m_hHead, &hAttachment) )
				{
					g_pLTServer->RemoveAttachment(hAttachment);
				}

				g_pLTServer->RemoveObject(m_hHead);
				m_hHead = LTNULL;
			}

			if (m_hWindshield && hBreaker == m_hWindshield)
			{
				HATTACHMENT hAttachment;
				if (LT_OK == g_pLTServer->FindAttachment(m_hObject, m_hWindshield, &hAttachment))
				{
					g_pLTServer->RemoveAttachment(hAttachment);
				}

				m_hWindshield = LTNULL;
			}
		}
		break;
	}

	return CAI::EngineMessageFn(messageID, pData, fData);
}


void AIExosuit::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!g_pServerDE || !hWrite) return;
	
	CAI::Save(hWrite,dwSaveFlags);

	*hWrite << m_hWindshield;
}


void AIExosuit::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!g_pServerDE || !hRead) return;

	CAI::Load(hRead,dwLoadFlags);

	*hRead >> m_hWindshield;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIExosuit::InitialUpdate
//
//	PURPOSE:	Initial Update
//
// ----------------------------------------------------------------------- //

void AIExosuit::InitialUpdate()
{
	CAI::InitialUpdate();

	// based on the character make and attach the head
	uint32 nRykov = g_pCharacterButeMgr->GetSetFromModelType("RykovExosuit_AI");
	uint32 nDunya = g_pCharacterButeMgr->GetSetFromModelType("DunyaExosuit_AI");

	ASSERT( nRykov < uint32(g_pCharacterButeMgr->GetNumSets()) );
	ASSERT( nDunya < uint32(g_pCharacterButeMgr->GetNumSets()) );

	// see if we need to clean up an existing head
	if(m_hHead)
	{
		// remove the head attachemnt
		HATTACHMENT hAttachment;
		if (g_pServerDE->FindAttachment(m_hObject, m_hHead, &hAttachment) == DE_OK)
		{
			g_pServerDE->RemoveAttachment(hAttachment);
		}
		g_pServerDE->RemoveObject(m_hHead);
		m_hHead = DNULL;
	}

	uint32 nHeadSet;
	
	const char * szHeadName = "Merc4_AI";

	if(m_nCharacterSet==nRykov)
		szHeadName = "Rykov_AI";
	else if(m_nCharacterSet==nDunya)
		szHeadName = "Dunya_AI";
	
	ASSERT( szHeadName && szHeadName[0]);
	
	nHeadSet = g_pCharacterButeMgr->GetSetFromModelType(szHeadName); 

	if( nHeadSet < 0 )
	{
		nHeadSet = 0;
#ifndef _FINAL
		AIErrorPrint(m_hObject, "Could not find character bute set %s.",
			szHeadName);
#endif
	}

	// Now create our body prop to use as a head in the suit
	m_hHead = CreateExosuitHead(nHeadSet);
		
	_ASSERT( m_hHead );

	if( m_hHead )
	{
		// and attach it
		HATTACHMENT pAttachment;

		// Get socket.
		HMODELSOCKET hSocket = NULL;
		LTransform tf;
		LTRotation rRot;
		g_pModelLT->GetSocket(m_hObject,"Body_Prop",hSocket);
		g_pModelLT->GetSocketTransform(m_hObject, hSocket, tf, LTFALSE);
		g_pTransLT->GetRot(tf,rRot);

		g_pLTServer->CreateAttachment(m_hObject, m_hHead, "Body_Prop", &LTVector(0,0,0), &rRot, &pAttachment);

		g_pLTServer->CreateInterObjectLink(m_hObject, m_hHead);
	}
#ifndef _FINAL
	else
	{
		AIErrorPrint(m_hObject, "Could not find head named \"%s\", bute id #%d.", szHeadName, nHeadSet);
	}
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIExosuit::CreateExosuitHead
//
//	PURPOSE:	Handle making the exosuit head attachment
//
// ----------------------------------------------------------------------- //

HOBJECT AIExosuit::CreateExosuitHead(uint32 nHeadSet)
{
	//see if this is a predator or human
	const CharacterButes & butes = g_pCharacterButeMgr->GetCharacterButes(nHeadSet);

	//only create a head if we are a human or predator
	HCLASS hClass = g_pServerDE->GetClass("Prop");
	if (!hClass) return LTNULL;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

#ifdef _DEBUG
	sprintf(theStruct.m_Name, "%s-Head", g_pLTServer->GetObjectName(m_hObject) );
#endif 

	g_pCharacterButeMgr->GetDefaultFilenames(nHeadSet,theStruct);
	theStruct.m_ObjectType = OT_MODEL;

	// Allocate an object...
	Prop* pProp;
	pProp = (Prop*)g_pServerDE->CreateObject(hClass, &theStruct);
	if (!pProp) return LTNULL;

	// Cool, now send a special FX message so we can trick the clients
	// into thinking this is a body prop.
	BODYPROPCREATESTRUCT cs;
	cs.nCharacterSet = (uint8)nHeadSet;
	cs.bBloodTrail = LTFALSE;

	HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(pProp);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_BODYPROP_ID);
	cs.Write(g_pLTServer, hMessage);
	g_pLTServer->EndMessage(hMessage);


	// Get the limb_head animation since it is centered on the head and 
	// has the proper dims we need.
	HMODELANIM	hAnim = g_pServerDE->GetAnimIndex(pProp->m_hObject, "Exosuit_Head");
	LTVector vDims;
	LTAnimTracker* pTracker;
	g_pModelLT->GetMainTracker(pProp->m_hObject, pTracker);
	g_pModelLT->SetCurAnim(pTracker, hAnim);
	g_pModelLT->SetLooping(pTracker, LTTRUE);
	g_pLTServer->Common()->GetModelAnimUserDims(pProp->m_hObject, &vDims, hAnim);
	g_pLTServer->SetObjectDims(pProp->m_hObject, &vDims);

	uint32 dwFlags = 0; 
	dwFlags |= FLAG_VISIBLE | FLAG_GOTHRUWORLD | FLAG_NOSLIDING;	// Make sure we are visible
	dwFlags &= ~(FLAG_SOLID | FLAG_TOUCH_NOTIFY);					

	uint32 dwUsrFlags = g_pServerDE->GetObjectUserFlags(pProp->m_hObject);

	// Set our surface type...
	uint32 dwHeadUsrFlags = g_pLTServer->GetObjectUserFlags(pProp->m_hObject);
	SurfaceType nHeadSurfType = UserFlagToSurface(dwHeadUsrFlags);

	dwUsrFlags |= SurfaceToUserFlag(nHeadSurfType);

	g_pServerDE->SetObjectUserFlags(pProp->m_hObject, dwUsrFlags);
	g_pServerDE->SetObjectFlags(pProp->m_hObject, dwFlags);

	//hide all pieces but the head
	HMODELPIECE hPiece;
	g_pLTServer->GetModelLT()->GetPiece(pProp->m_hObject, "Head", hPiece);

	//hide 'em all
	for(HMODELPIECE hTemp=0 ; hTemp<32 ; hTemp++)
		g_pLTServer->GetModelLT()->SetPieceHideStatus(pProp->m_hObject, hTemp, LTTRUE);

	//unhide head...
	g_pLTServer->GetModelLT()->SetPieceHideStatus(pProp->m_hObject, hPiece, LTFALSE);

	// Indestructible
	if (pProp->GetDestructible())
		pProp->GetDestructible()->SetCanDamage(LTFALSE);

	return pProp->m_hObject;
}

LTBOOL AIExosuit::AddWindshield()
{
	// Don't create over the top of an existing windshield
	if (m_hWindshield)
	{
		_ASSERT(!"Windshield not created: already present");
		return LTFALSE;
	}

	// Create and attach the windshield
	HCLASS hClass = g_pServerDE->GetClass("Prop");
	if (!hClass)
		return LTFALSE;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	uint32 nID = g_pAttachButeMgr->GetAttachmentIDByName("Windshield");
	g_pAttachButeMgr->GetAttachmentName(nID);
	g_pAttachButeMgr->GetAttachmentSkin(nID);

	strncpy(theStruct.m_Name, g_pAttachButeMgr->GetAttachmentName(nID), MAX_CS_FILENAME_LEN);
	strncpy(theStruct.m_Filename, g_pAttachButeMgr->GetAttachmentModel(nID), MAX_CS_FILENAME_LEN);
	strncpy(theStruct.m_SkinName, g_pAttachButeMgr->GetAttachmentSkin(nID), MAX_CS_FILENAME_LEN);

	theStruct.m_ObjectType = OT_MODEL;


	Prop* pProp;
	pProp = (Prop*)g_pServerDE->CreateObject(hClass, &theStruct);
	if (!pProp)
		return LTFALSE;

	// Now we have a windshield object
	m_hWindshield = pProp->m_hObject;
	
	uint32 dwFlags;
	ILTCommon *pCommon = g_pLTServer->Common();
	if (pCommon)
	{
		// Visible
		pCommon->GetObjectFlags(m_hWindshield, OFT_Flags, dwFlags);
		pCommon->SetObjectFlags(m_hWindshield, OFT_Flags, dwFlags | FLAG_VISIBLE);

		// Non-solid to AI
		pCommon->GetObjectFlags(m_hWindshield, OFT_Flags2, dwFlags);
		pCommon->SetObjectFlags(m_hWindshield, OFT_Flags2, dwFlags & ~FLAG2_SEMISOLID);
	}

	// Set alpha
	LTFLOAT r, g, b, a;
	g_pLTServer->GetObjectColor(m_hWindshield, &r, &g, &b, &a);
	a = g_pAttachButeMgr->GetAttachmentAlpha(nID);
	g_pLTServer->SetObjectColor(m_hWindshield, r, g, b, a);

	// Indestructible
	if (pProp->GetDestructible())
		pProp->GetDestructible()->SetCanDamage(LTFALSE);

	// Time to attach it
	HMODELSOCKET hSocket;
	LTransform tf;
	LTRotation rRot;
	HATTACHMENT hAttachment;

	g_pModelLT->GetSocket(m_hObject,"Windshield", hSocket);
	g_pModelLT->GetSocketTransform(m_hObject, hSocket, tf, LTFALSE);
	g_pTransLT->GetRot(tf, rRot);

	LTRESULT result = g_pLTServer->CreateAttachment(m_hObject, m_hWindshield, "Windshield", &LTVector(0,0,0), &rRot, &hAttachment);
	g_pLTServer->CreateInterObjectLink(m_hObject, m_hWindshield);

	return LTTRUE;
}

void AIExosuit::LaunchWindshield()
{
	if (!m_hWindshield)
		return;

	HATTACHMENT hAttachment;

	g_pLTServer->FindAttachment(m_hObject, m_hWindshield, &hAttachment);
	if (hAttachment)
		g_pLTServer->RemoveAttachment(hAttachment);

	uint32 dwFlags;
	ILTCommon *pCommon = g_pLTServer->Common();

	ASSERT(pCommon);
	if (pCommon)
	{
		pCommon->GetObjectFlags(m_hWindshield, OFT_Flags, dwFlags);
		pCommon->SetObjectFlags(m_hWindshield, OFT_Flags, dwFlags | FLAG_GRAVITY);
	}

	LTVector vDir = GetForward() + GetUp();
	const LTFLOAT fVel = 400.0f;
	vDir *= fVel;

	// TODO: Add sound effect here
	g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "Canopy_Blow");
	g_pLTServer->SetVelocity(m_hWindshield, &vDir);

	return;
}

bool CAIExosuitPlugin::CanUseWeapon(const AIWBM_AIWeapon & butes) const
{
	return butes.eRace == AIWBM_AIWeapon::Exosuit;
}


// ----------------------------------------------------------------------- //
//
// MODULE  : CServerMark.cpp
//
// PURPOSE : CServerMark implementation
//
// CREATED : 1/15/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerMark.h"
#include "WeaponMgr.h"
#include "transformlt.h"

BEGIN_CLASS(CServerMark)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS(CServerMark, BaseClass, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMark::CServerMark()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CServerMark::CServerMark() : BaseClass(OT_SPRITE)
{
	m_AttachmentObj = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMark::~CServerMark()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CServerMark::~CServerMark()
{
	if(m_AttachmentObj)
	{
		// Be sure to remove the attachment...
		HATTACHMENT hAttachment;
		if (g_pServerDE->FindAttachment(m_AttachmentObj, m_hObject, &hAttachment) == DE_OK)
		{
			g_pServerDE->RemoveAttachment(hAttachment);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMark::Setup()
//
//	PURPOSE:	Attach the mark to its parent
//
// ----------------------------------------------------------------------- //

void CServerMark::Setup(CLIENTWEAPONFX & theStruct)
{
	LTransform globalTransform, parentTransform, localTransform;
	TransformLT *pTransformLT;
	DVector vParentPos, vOffset;
	DRotation rParentRot, rRot;
	DRotation rOffset;

	if (!g_pWeaponMgr || 
		!g_pServerDE || 
		!theStruct.hObj ||
		!(pTransformLT = g_pServerDE->GetTransformLT()))
	{
		return;
	}

	// Attach the mark to the parent object...

	// Figure out what the rotation we want is.
	ROT_INIT(rOffset);
	g_pServerDE->AlignRotation(&rRot, &(theStruct.vSurfaceNormal), DNULL);


	// MD
	// Ok, now we have the transform in global space but attachments are specified in
	// local space (so they can move as the object moves and rotates).
	
	// Set the global LTransform.
	pTransformLT->Set(globalTransform, theStruct.vPos, rRot);

	// Get the object's transform.
	g_pServerDE->GetObjectPos(theStruct.hObj, &vParentPos);
	g_pServerDE->GetObjectRotation(theStruct.hObj, &rParentRot);
	pTransformLT->Set(parentTransform, vParentPos, rParentRot);

	// Get the offset.
	pTransformLT->Difference(localTransform, globalTransform, parentTransform);
	pTransformLT->Get(localTransform, vOffset, rOffset);

	HOBJECT hAttachList[1];
	uint32 dwListSize, dwNumAttachments;

	if (g_pLTServer->Common()->GetAttachments(theStruct.hObj, hAttachList, 
		ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
	{
		if(dwNumAttachments >= 16)
		{
			// Oops we are maxed...
			g_pServerDE->RemoveObject(m_hObject);
			return;
		}
	}

	HATTACHMENT hAttachment;

	DRESULT dRes = g_pServerDE->CreateAttachment(theStruct.hObj, m_hObject, DNULL, 
											     &vOffset, &rOffset, &hAttachment);
	if (dRes != DE_OK)
	{
		g_pServerDE->RemoveObject(m_hObject);
	}

	// Be sure to record the object...
	m_AttachmentObj = theStruct.hObj;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMark::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CServerMark::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PARENTATTACHMENTREMOVED :
		{
			g_pServerDE->RemoveObject(m_hObject);
		}
		break;

		case MID_INITIALUPDATE:
		{
			g_pLTServer->SetNextUpdate(m_hObject, 10.0f);
		}
		break;
		
		case MID_UPDATE:
		{
			if(m_AttachmentObj)
			{
				// Be sure to remove the attachment...
				HATTACHMENT hAttachment;
				if (g_pServerDE->FindAttachment(m_AttachmentObj, m_hObject, &hAttachment) == DE_OK)
				{
					g_pServerDE->RemoveAttachment(hAttachment);
				}
				m_AttachmentObj = LTNULL;
			}
			g_pServerDE->RemoveObject(m_hObject);
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

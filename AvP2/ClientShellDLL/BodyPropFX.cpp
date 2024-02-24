// ----------------------------------------------------------------------- //
//
// MODULE  : BodyPropFX.cpp
//
// PURPOSE : BodyProp special FX - Implementation
//
// CREATED : 7/11/2000
//
// (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BodyPropFX.h"
#include "FXButeMgr.h"
#include "SurfaceFunctions.h"
#include "ParticleTrailFX.h"
#include "SFXMgr.h"
#include "GameClientShell.h"
#include "CharacterFuncs.h"
#include "VisionModeButeMGR.h"
#include "DebrisFX.h"
#include "clientsoundmgr.h"
#include "VarTrack.h"
#include "ModelButeMgr.h"
#include "ClientButeMgr.h"

VarTrack g_vtBodyBleederTime;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::~BodyPropFX()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

BodyPropFX::~BodyPropFX()
{
	if(m_hAuraSystem)
	{
		//remove the aura system
		g_pLTClient->DeleteObject(m_hAuraSystem);
		m_hAuraSystem = LTNULL;
	}

	for(int i=0; i<3; ++i)
		if(m_hFireSystem[i])
		{
			//remove the aura system
			g_pLTClient->DeleteObject(m_hFireSystem[i]);
			m_hFireSystem[i] = LTNULL;
		}

	if(m_hChestBurstModel)
	{
		//remove the chest burst model
		g_pLTClient->DeleteObject(m_hChestBurstModel);
		m_hChestBurstModel = LTNULL;
	}

	if(m_hFireSound)
	{
		g_pLTClient->KillSound(m_hFireSound);
		m_hFireSound = LTNULL;
	}

	for( BodyBleederList::iterator iter = m_BleederList.begin(); iter != m_BleederList.end(); ++iter )
	{
		delete (*iter);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::Init
//
//	PURPOSE:	Init the body prop fx
//
// ----------------------------------------------------------------------- //

LTBOOL BodyPropFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
	if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
	if (!hMessage) return LTFALSE;

	BODYPROPCREATESTRUCT body;

	body.hServerObj		= hServObj;
	body.Read(g_pLTClient, hMessage);

	return Init(&body);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::Init
//
//	PURPOSE:	Init the body prop fx
//
// ----------------------------------------------------------------------- //

LTBOOL BodyPropFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	m_cs = *((BODYPROPCREATESTRUCT*)psfxCreateStruct);

	if ( !m_NodeController.Init(LTNULL, this, LTFALSE) )
	{
		return LTFALSE;
	}

	if(!g_vtBodyBleederTime.IsInitted())
		g_vtBodyBleederTime.Init(g_pLTClient, "BodyBleederTime", NULL, 0.1f);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::CreateObject
//
//	PURPOSE:	Create the various fx
//
// ----------------------------------------------------------------------- //

LTBOOL BodyPropFX::CreateObject(CClientDE* pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return LTFALSE;


	uint32 dwCFlags = g_pLTClient->GetObjectClientFlags(m_hServerObject);
	g_pLTClient->SetObjectClientFlags(m_hServerObject, dwCFlags | CF_NOTIFYMODELKEYS);

	// if we are low violence and this is just
	// a limb prop, exit here
	if(GetConsoleInt("LowViolence",0) && m_cs.bLimbProp)
		return LTTRUE;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();

	//don't do blood stuff if gore is off
	if(pSettings->Gore())
	{
		uint32 dwUserFlags;
		g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

		SurfaceType eSurfaceType = UserFlagToSurface(dwUserFlags);

		CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();

		if(m_cs.bBloodTrail)
		{
			// Get the user flags of the server object to determine the surface type

			// Pick the appropriate trails
			PARTICLETRAILFX *pTrail = LTNULL;
			PARTICLETRAILFX *pUWTrail = LTNULL;

			switch(eSurfaceType)
			{
				case ST_FLESH_PREDATOR:
					pTrail = g_pFXButeMgr->GetParticleTrailFX("Predator_Blood_Trail");
					pUWTrail = g_pFXButeMgr->GetParticleTrailFX("Predator_Blood_Trail");
					break;

				case ST_FLESH_ALIEN:
				case ST_FLESH_PRAETORIAN:
					pTrail = g_pFXButeMgr->GetParticleTrailFX("Alien_Blood_Trail");
					pUWTrail = g_pFXButeMgr->GetParticleTrailFX("Alien_Blood_Trail");
					break;

				case ST_FLESH_SYNTH:
					pTrail = g_pFXButeMgr->GetParticleTrailFX("Synth_Blood_Trail");
					pUWTrail = g_pFXButeMgr->GetParticleTrailFX("Synth_Blood_Trail");
					break;

				case ST_FLESH_HUMAN:
				default:
					pTrail = g_pFXButeMgr->GetParticleTrailFX("Human_Blood_Trail");
					pUWTrail = g_pFXButeMgr->GetParticleTrailFX("Human_Blood_Trail");
					break;
			}

			if(psfxMgr)
			{
				// Create this particle trail segment
				PTCREATESTRUCT pt;
				pt.hServerObj	= m_hServerObject;
				pt.nType		= pTrail ? pTrail->nId : -1;
				pt.nUWType		= pUWTrail ? pUWTrail->nId : -1;

				psfxMgr->CreateSFX(SFX_PARTICLETRAIL_ID, &pt);
			}

		}

		// See if we lost our head!
		HMODELPIECE hPiece = INVALID_MODEL_PIECE;
		if(g_pModelLT->GetPiece(m_hServerObject, "Torso", hPiece) == LT_OK)
		{
			LTBOOL bHidden;
			g_pModelLT->GetPieceHideStatus(m_hServerObject, hPiece, bHidden);

			if(!bHidden)
			{
				if(g_pModelLT->GetPiece(m_hServerObject, "Head", hPiece) == LT_OK)
				{
					LTBOOL bHidden;
					g_pModelLT->GetPieceHideStatus(m_hServerObject, hPiece, bHidden);

					if(bHidden)
					{
						// Sweet!  Time to create a blood fountain!
						ModelNode eNode = g_pModelButeMgr->GetSkeletonNode(GetModelSkeleton(),"Head_node");

						if(eNode != eModelNodeInvalid)
							AddNewBleeder(eNode);
					}
				}
			}
		}
	}
	else
	{
		//be sure we don't make a blood pool
		m_cs.bBloodPool = LTFALSE;
	}

	//create the aura fx
	CreateAuraFX();
	CreateFireFX();


	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::Update
//
//	PURPOSE:	Update the fx
//
// ----------------------------------------------------------------------- //

LTBOOL BodyPropFX::Update()
{
	//if we are a local player then hide us if we are in first person
	if((m_cs.hPlayerObj == g_pLTClient->GetClientObject()))
	{
		uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hServerObject);

		if(g_pGameClientShell->IsFirstPerson())
			g_pLTClient->SetObjectFlags(m_hServerObject, dwFlags & ~FLAG_VISIBLE);
		else
			g_pLTClient->SetObjectFlags(m_hServerObject, dwFlags | FLAG_VISIBLE);
	}

	// if we are low violence and this is just
	// a limb prop, hide us here
	if(GetConsoleInt("LowViolence",0) && m_cs.bLimbProp)
	{
		uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hServerObject);
		g_pLTClient->SetObjectFlags(m_hServerObject, dwFlags & ~FLAG_VISIBLE);

		return CSpecialFX::Update();
	}
	else if(GetConsoleInt("LowViolence",0))
	//  if we are a whole body, show al our parts
	{
		ShowAllPieces();
	}

	if(m_hAuraSystem)
		UpdateAuraFX();

	if(m_hFireSystem[0])
		UpdateFireFX();

	if(m_hChestBurstModel)
		UpdateChestBurstFX();

	m_NodeController.Update();

	if(m_cs.bBloodPool)
		UpdateBloodPool();

	// Update any blood emmiters we may have
	UpdateBleeders();

	return CSpecialFX::Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::CreateFireFX
//
//	PURPOSE:	Create the fire FX
//
// ----------------------------------------------------------------------- //

void BodyPropFX::CreateFireFX()
{
#ifdef _DEMO
	return;
#endif

	ObjectCreateStruct ocs;
	g_pLTClient->GetObjectPos(m_hServerObject, &ocs.m_Pos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &ocs.m_Rotation);
	ocs.m_Flags = 0;
	ocs.m_ObjectType = OT_PARTICLESYSTEM;
	ocs.m_Flags2 = FLAG2_ADDITIVE;

	for(int i=0; i<3; ++i)
	{
		m_hFireSystem[i] = g_pLTClient->CreateObject(&ocs);

		if(m_hFireSystem[i])
		{
			g_pLTClient->SetObjectColor(m_hFireSystem[i], 1.0f,1.0f,1.0f,0.1f);

			char buf[36];
			sprintf(buf, "sprites\\sfx\\particles\\onfire%d.spr", i);

			g_pLTClient->SetupParticleSystem(m_hFireSystem[i], buf, 0, 0, 500.0f);
		}
	}

	ModelSkeleton eModelSkeleton = GetModelSkeleton();

	int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
	const char* szNodeName = DNULL;

	for (int iNode = 0; iNode < cNodes; iNode++)
	{
		uint32 nIndex = iNode%3;
		ModelNode eCurrentNode = (ModelNode)iNode;
		szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);
		
		if (szNodeName)
		{
			LTFLOAT fRadius = g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, eCurrentNode);

			LTVector vDummy(0.0f, 0.0f, 0.0f);
			LTParticle *p = g_pLTClient->AddParticle(m_hFireSystem[nIndex], &ocs.m_Pos, &vDummy, &vDummy, 10000.0f);

			p->m_Size = fRadius;

			//TEMP
			p->m_Color = LTVector(125.0, 125.0, 125.0);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::CreateAuraFX
//
//	PURPOSE:	Create the aura FX
//
// ----------------------------------------------------------------------- //

void BodyPropFX::CreateAuraFX()
{
	uint32 dwFlags;
	g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwFlags);
	SurfaceType eSurfaceType = UserFlagToSurface(dwFlags);
	if(eSurfaceType == ST_FLESH_SYNTH) return;

	ObjectCreateStruct ocs;
	g_pLTClient->GetObjectPos(m_hServerObject, &ocs.m_Pos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &ocs.m_Rotation);
	ocs.m_Flags = 0;
	ocs.m_ObjectType = OT_PARTICLESYSTEM;
	ocs.m_Flags2 = FLAG2_ADDITIVE;
	m_hAuraSystem = g_pLTClient->CreateObject(&ocs);

	if(m_hAuraSystem)
	{
		g_pLTClient->SetObjectColor(m_hAuraSystem, 1.0f,1.0f,1.0f,0.1f);

		g_pLTClient->SetupParticleSystem(m_hAuraSystem, "sprites\\sfx\\particles\\aura.spr", 0, 0, 500.0f);

		ModelSkeleton eModelSkeleton = GetModelSkeleton();

		int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
		const char* szNodeName = DNULL;

		for (int iNode = 0; iNode < cNodes; iNode++)
		{
			ModelNode eCurrentNode = (ModelNode)iNode;
			szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);
			
			if (szNodeName)
			{
				LTFLOAT fRadius = g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, eCurrentNode);

				LTVector vDummy(0.0f, 0.0f, 0.0f);
				LTParticle *p = g_pLTClient->AddParticle(m_hAuraSystem, &ocs.m_Pos, &vDummy, &vDummy, 10000.0f);

				p->m_Alpha = 0.8f;
				p->m_Size = fRadius;
				p->m_Color = GetAuraColor();
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::GetAuraColor
//
//	PURPOSE:	Get the aura color
//
// ----------------------------------------------------------------------- //

LTVector BodyPropFX::GetAuraColor(LTBOOL bAutoTargeted)
{
	LTVector rVal = LTVector(0,0,0);

	uint32 dwFlags;
	g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwFlags);

	SurfaceType eSurfaceType = UserFlagToSurface(dwFlags);

	switch(eSurfaceType)
	{
		case ST_FLESH_PREDATOR:
			rVal = g_pClientButeMgr->GetDeadPredAuraColor(); 
			break;

		case ST_FLESH_ALIEN:
		case ST_FLESH_PRAETORIAN:
			rVal = g_pClientButeMgr->GetDeadAlienAuraColor(); 
			break;

		case ST_FLESH_HUMAN:
		default:
			rVal = !bAutoTargeted ?  g_pClientButeMgr->GetDeadHumanAuraColor() : g_pClientButeMgr->GetDeadSelHumanAuraColor(); 
			break;
	}

	return rVal;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::GetNodePos
//
//	PURPOSE:	Get the position of the node
//
// ----------------------------------------------------------------------- //

LTBOOL BodyPropFX::GetNodePos(const char* szNodeName, LTVector &vPos, LTBOOL bWorldPos, LTRotation* pRot)
{
	//see about head bite
	ModelLT* pModelLT   = g_pClientDE->GetModelLT();

	char pTemp[255];
	sprintf(pTemp,szNodeName);

	HMODELNODE hNode;

	if (pModelLT->GetNode(m_hServerObject, pTemp, hNode) == LT_OK)
	{
		LTransform	tTransform;

		if (pModelLT->GetNodeTransform(m_hServerObject, hNode, tTransform, bWorldPos) == LT_OK)
		{
			TransformLT* pTransLT = g_pLTClient->GetTransformLT();
			pTransLT->GetPos(tTransform, vPos);
			if(pRot)
				pTransLT->GetRot(tTransform, *pRot);

			return LTTRUE;
		}
	}
	else
	{
		HMODELSOCKET hSocket;

		if (pModelLT->GetSocket(m_hServerObject, pTemp, hSocket) == LT_OK)
		{
			LTransform	tTransform;

			if (pModelLT->GetSocketTransform(m_hServerObject, hSocket, tTransform, bWorldPos) == LT_OK)
			{
				TransformLT* pTransLT = g_pLTClient->GetTransformLT();
				pTransLT->GetPos(tTransform, vPos);
				if(pRot)
					pTransLT->GetRot(tTransform, *pRot);

				return LTTRUE;
			}
		}
	}

	// hmmm... can't find it!
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::UpdateFireFX
//
//	PURPOSE:	Update the fire FX
//
// ----------------------------------------------------------------------- //

void BodyPropFX::UpdateFireFX()
{
	if(m_hFireSystem[0])
	{
		uint32 dwUsrFlags;
		g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUsrFlags);
		uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hServerObject);
		HLOCALOBJ hPlayerObj = g_pClientDE->GetClientObject();
		LTBOOL 	bIsLocalClient = hPlayerObj == m_hServerObject;
		LTBOOL bHide = bIsLocalClient && g_pGameClientShell->IsFirstPerson();

		LTVector vPos;
		g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

		//if this character is not visible or not on fire
		if(!bHide && (dwUsrFlags & USRFLG_CHAR_NAPALM) && (dwFlags & FLAG_VISIBLE))
		{
			for(int i=0; i<3; ++i)
			{
				dwFlags = g_pLTClient->GetObjectFlags(m_hFireSystem[i]);
				g_pLTClient->SetObjectFlags(m_hFireSystem[i], dwFlags | FLAG_VISIBLE);
			}

			//play the fire sound (don't mess with sound if I am the local client)
			if(!bIsLocalClient)
			{
				if(!m_hFireSound)
					m_hFireSound = g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject, "CharOnFire");
				else
				{
					// Be sure the sound is located at the source
					g_pLTClient->SetSoundPosition(m_hFireSound, &vPos);
				}
			}
		}
		else
		{
			for(int i=0; i<3; ++i)
			{
				dwFlags = g_pLTClient->GetObjectFlags(m_hFireSystem[i]);
				g_pLTClient->SetObjectFlags(m_hFireSystem[i], dwFlags & ~FLAG_VISIBLE);
			}

			//kill the sound (don't mess with sound if I am the local client)
			if(!bIsLocalClient)
			{
				if(m_hFireSound)
				{
					g_pLTClient->KillSound(m_hFireSound);
					m_hFireSound = LTNULL;
				}
			}

			return;
		}
		LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

		for(int i=0; i<3; ++i)
		{
			g_pLTClient->SetObjectPos(m_hFireSystem[i], &vPos);
			g_pLTClient->SetObjectRotation(m_hFireSystem[i], &rRot);
		}

		//now update the particles
		ModelSkeleton eModelSkeleton = GetModelSkeleton();

		int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
		const char* szNodeName = DNULL;

		LTParticle *pHead[3];
		LTParticle *pTail[3];

		for(i=0; i<3; ++i)
			g_pLTClient->GetParticles(m_hFireSystem[i], &pHead[i], &pTail[i]);

		LTBOOL bHidden=LTTRUE;

		for (int iNode = 0; iNode < cNodes; iNode++)
		{
			uint32 nIndex = iNode%3;
			ModelNode eCurrentNode = (ModelNode)iNode;
			szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);

			const char* szPiece = g_pModelButeMgr->GetSkeletonNodePiece(eModelSkeleton, eCurrentNode);
			
			if(szPiece[0] != '\0')
			{
				HMODELPIECE hPiece = INVALID_MODEL_PIECE;

				// Make the piece invisible
				if(g_pModelLT->GetPiece(m_hServerObject, const_cast<char*>(szPiece), hPiece) == LT_OK)
				{

					g_pModelLT->GetPieceHideStatus(m_hServerObject, hPiece, bHidden);
				}
			}

			if (szNodeName)
			{
				LTVector vPos;
				GetNodePos(szNodeName, vPos);
				
				g_pLTClient->SetParticlePos(m_hFireSystem[nIndex], pHead[nIndex], &vPos);

			}

			if(bHidden)
				pHead[nIndex]->m_Size = 0.0f;

			bHidden = LTTRUE;

			pHead[nIndex] = pHead[nIndex]->m_pNext;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::UpdateAuraFX
//
//	PURPOSE:	Create the aura FX
//
// ----------------------------------------------------------------------- //

void BodyPropFX::UpdateAuraFX()
{
	if(m_hAuraSystem)
	{
		uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hServerObject);

		//if this character is not visible or we are not an alien, hide the system
		if(	(	dwFlags & FLAG_VISIBLE) && 
				IsAlien(g_pGameClientShell->GetPlayerStats()->GetButeSet()) && 
				!(VisionMode::g_pButeMgr->IsAlienHuntingVision(g_pGameClientShell->GetVisionModeMgr()->GetCurrentModeName()))
				&& g_pGameClientShell->IsFirstPerson())
		{
			dwFlags = g_pLTClient->GetObjectFlags(m_hAuraSystem);
			g_pLTClient->SetObjectFlags(m_hAuraSystem, dwFlags | FLAG_VISIBLE);
		}
		else
		{
			dwFlags = g_pLTClient->GetObjectFlags(m_hAuraSystem);
			g_pLTClient->SetObjectFlags(m_hAuraSystem, dwFlags & ~FLAG_VISIBLE);
			return;
		}

		LTVector vPos;
		g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
		g_pLTClient->SetObjectPos(m_hAuraSystem, &vPos);

		LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
		g_pLTClient->SetObjectRotation(m_hAuraSystem, &rRot);
		//now update the particles
		ModelSkeleton eModelSkeleton = GetModelSkeleton();

		int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
		const char* szNodeName = DNULL;

		LTParticle *pHead;
		LTParticle *pTail;
		g_pLTClient->GetParticles(m_hAuraSystem, &pHead, &pTail);

		LTBOOL bHidden=LTTRUE;

		for (int iNode = 0; iNode < cNodes; iNode++)
		{
			ModelNode eCurrentNode = (ModelNode)iNode;
			szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);

			const char* szPiece = g_pModelButeMgr->GetSkeletonNodePiece(eModelSkeleton, eCurrentNode);

			if(szPiece[0] != '\0')
			{
				HMODELPIECE hPiece = INVALID_MODEL_PIECE;

				// Make the piece invisible
				if(g_pModelLT->GetPiece(m_hServerObject, const_cast<char*>(szPiece), hPiece) == LT_OK)
				{
					g_pModelLT->GetPieceHideStatus(m_hServerObject, hPiece, bHidden);
				}
			}
			
			if (szNodeName)
			{
				LTVector vPos;
				GetNodePos(szNodeName, vPos);
				
				g_pLTClient->SetParticlePos(m_hAuraSystem, pHead, &vPos);

				pHead->m_Color = GetAuraColor(g_pInterfaceMgr->GetAutoTargetMgr()->GetAlienTarget() == m_hServerObject);

			}

			if(bHidden)
				pHead->m_Size = 0.0f;

			bHidden = LTTRUE;

			pHead = pHead->m_pNext;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::OnServerMessage
//
//	PURPOSE:	Handle any messages from our server object...
//
// ----------------------------------------------------------------------- //

LTBOOL BodyPropFX::OnServerMessage(HMESSAGEREAD hMessage)
{
	if (!CSpecialFX::OnServerMessage(hMessage)) return LTFALSE;

	DBYTE nMsgId = g_pClientDE->ReadFromMessageByte(hMessage);

	switch(nMsgId)
	{
		case BPFX_NODECONTROL_LIP_SYNC:
		{
			m_NodeController.HandleNodeControlMessage(nMsgId, hMessage);
		}
		break;

		case BPFX_CHESTBURST_FX:
		{
			CreateChestBurstFX();
		}
		break;

		default : break;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::WantRemove
//
//	PURPOSE:	Remove stuff from the engine.
//
// ----------------------------------------------------------------------- //

void BodyPropFX::WantRemove(LTBOOL bRemove)
{
	// Be sure to remove the trackers.
	if( bRemove && m_hServerObject )
	{
		// Remove the node controller.
		m_NodeController.Term();
	}


	CSpecialFX::WantRemove(bRemove);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::CreateChestBurstFX
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void BodyPropFX::CreateChestBurstFX()
{
	// If we already created the model... just return
	if(m_hChestBurstModel) return;

	// Check to see if this body prop has a stomach socket
	HMODELSOCKET hSocket;
	g_pLTClient->GetModelLT()->GetSocket(m_hServerObject, "Stomach", hSocket);

	if(hSocket != INVALID_MODEL_SOCKET)
	{
		// Go ahead and create the model
		ObjectCreateStruct ocs;
		ocs.m_Flags = FLAG_VISIBLE;
		ocs.m_ObjectType = OT_MODEL;

		LTransform trans;
		g_pLTClient->GetModelLT()->GetSocketTransform(m_hServerObject, hSocket, trans, LTTRUE);
		g_pLTClient->GetTransformLT()->Get(trans, ocs.m_Pos, ocs.m_Rotation);

		// Get the rotation vectors
		LTVector vUnused, vDir;
		g_pLTClient->GetMathLT()->GetRotationVectors(ocs.m_Rotation, vUnused, vUnused, vDir);

		// Get the user flags of the server object to determine the surface type
		uint32 dwUserFlags;
		g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUserFlags);
		SurfaceType eSurfaceType = UserFlagToSurface(dwUserFlags);

		// Choose the appropriate model and skin
		switch(eSurfaceType)
		{
			case ST_FLESH_PREDATOR:
				strcpy(ocs.m_Filename, "Models\\Props\\chestburst.abc");
				strcpy(ocs.m_SkinNames[0], "Skins\\Props\\chestburst2.dtx");
				break;

			default:
				strcpy(ocs.m_Filename, "Models\\Props\\chestburst.abc");
				strcpy(ocs.m_SkinNames[0], "Skins\\Props\\chestburst.dtx");
				break;
		}

		m_hChestBurstModel = g_pLTClient->CreateObject(&ocs);


		// Now create some GIB and blood FX based off the surface type
		DEBRIS *pDebris = LTNULL;
		CPShowerFX *pPShower = LTNULL;

		switch(eSurfaceType)
		{
			case ST_FLESH_PREDATOR:
				pDebris = g_pDebrisMgr->GetDebris("Predator_Medium");
				pPShower = g_pFXButeMgr->GetPShowerFX("Blood_Shower_Predator_Huge");
				break;

			default:
				pDebris = g_pDebrisMgr->GetDebris("Human_Medium");
				pPShower = g_pFXButeMgr->GetPShowerFX("Blood_Shower_Human_Huge");
				break;
		}


		// Try to create the debris FX
		CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
		if(!psfxMgr) return;

		DEBRISCREATESTRUCT dcs;
		dcs.nDebrisId = pDebris->nId;
		dcs.vPos = ocs.m_Pos;
		dcs.rRot = ocs.m_Rotation;
		psfxMgr->CreateSFX(SFX_DEBRIS_ID, &dcs);

		if(pPShower)
			g_pFXButeMgr->CreatePShowerFX(pPShower, ocs.m_Pos, vDir, vDir);


		// Play a gruesome sound!! Bwah ha ha ha!!
		g_pClientSoundMgr->PlaySoundFromPos(ocs.m_Pos, "Sounds\\Weapons\\Alien\\FaceHug\\chestburst.wav", 1000.0f, SOUNDPRIORITY_MISC_HIGH);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::UpdateChestBurstFX
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void BodyPropFX::UpdateChestBurstFX()
{
	// If we don't have a model... just get out of here
	if(!m_hChestBurstModel) return;


	// Check to see if this body prop has a stomach socket
	HMODELSOCKET hSocket;
	g_pLTClient->GetModelLT()->GetSocket(m_hServerObject, "Stomach", hSocket);

	// Delete the model if the socket doesn't exist
	if(hSocket == INVALID_MODEL_SOCKET)
	{
		g_pLTClient->DeleteObject(m_hChestBurstModel);
		m_hChestBurstModel = LTNULL;
		return;
	}


	// Now we know our model and socket are ok... set update the position and rotation
	// of the chest burst model so it follow the stomach socket
	LTransform trans;
	LTVector vPos;
	LTRotation rRot;

	g_pLTClient->GetModelLT()->GetSocketTransform(m_hServerObject, hSocket, trans, LTTRUE);
	g_pLTClient->GetTransformLT()->Get(trans, vPos, rRot);

	g_pLTClient->SetObjectPos(m_hChestBurstModel, &vPos);
	g_pLTClient->SetObjectRotation(m_hChestBurstModel, &rRot);


	// Now make sure the color is the same as the body prop... alpha too
	LTFLOAT r, g, b, a;
	g_pLTClient->GetObjectColor(m_hServerObject, &r, &g, &b, &a);
	g_pLTClient->SetObjectColor(m_hChestBurstModel, r, g, b, a);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::SetMode
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void BodyPropFX::SetMode(const std::string& mode)
{
	// Check to see if we are in the default mode
	if ("None" == mode || "Normal" == mode || "NightVision" == mode || "Hunting" == mode)
	{
		ObjectCreateStruct newSkins;
		newSkins.Clear();

		CString modelName = g_pCharacterButeMgr->GetDefaultModel(m_cs.nCharacterSet);

		SAFE_STRCPY(newSkins.m_Filename, modelName.GetBuffer());

		CString skinDir = g_pCharacterButeMgr->GetDefaultSkinDir(m_cs.nCharacterSet);

		for (int i = 0; i < MAX_MODEL_TEXTURES; ++i)
		{
			CString skinName = g_pCharacterButeMgr->GetDefaultSkin(m_cs.nCharacterSet, i, skinDir.GetBuffer());
			SAFE_STRCPY(newSkins.m_SkinNames[i], skinName.GetBuffer()); 
		}

		// Set this up to not interupt the current animation.
		newSkins.m_bResetAnimations = LTFALSE;

		g_pLTClient->Common()->SetObjectFilenames(m_hServerObject, &newSkins);
	}
	else
	{
		ObjectCreateStruct newSkins;
		newSkins.Clear();

		CString modelName = g_pCharacterButeMgr->GetDefaultModel(m_cs.nCharacterSet);

		SAFE_STRCPY(newSkins.m_Filename, modelName.GetBuffer());

		CString skinDir = g_pCharacterButeMgr->GetDefaultSkinDir(m_cs.nCharacterSet);
		skinDir += mode.c_str();
		skinDir += "\\";

		for (int i = 0; i < MAX_MODEL_TEXTURES; ++i)
		{
			CString skinName = g_pCharacterButeMgr->GetDefaultSkin(m_cs.nCharacterSet, i, skinDir.GetBuffer());
			SAFE_STRCPY(newSkins.m_SkinNames[i], skinName.GetBuffer()); 
		}
	
		// check for .spr swap
		LTBOOL bSwap=LTFALSE;

		if("ElectroVision" == mode && IsAlien(m_cs.nCharacterSet)) bSwap=LTTRUE;
		else if("NavVision" == mode && IsPredator(m_cs.nCharacterSet)) bSwap=LTTRUE;
		else if("HeatVision" == mode && (IsHuman(m_cs.nCharacterSet) || IsSynthetic(m_cs.nCharacterSet) || IsExosuit(m_cs.nCharacterSet))) bSwap=LTTRUE;

		if(bSwap)
		{
			//yoink the .dtx and replace with .spr
			for (i = 0; i < MAX_MODEL_TEXTURES; ++i)
			{
				LPTSTR pStr = strstr(newSkins.m_SkinNames[i], ".dtx");
				if(pStr)
					SAFE_STRCPY(pStr, ".spr"); 
			}
		}

		// Set this up to not interupt the current animation.
		newSkins.m_bResetAnimations = LTFALSE;

		// This is OK since if it can't find the model it will bail out.
		g_pLTClient->Common()->SetObjectFilenames(m_hServerObject, &newSkins);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::UpdateBloodPool
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void BodyPropFX::UpdateBloodPool()
{
	// If we don't yet have an effect then see about starting one..
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();

	if(!m_bBloodPoolMade)
	{
		//see if the body has stopped moving
		uint32 dwExplodeAnim	= g_pInterface->GetAnimIndex(m_hServerObject, "Death_f_Thrw_Start0");
		uint32 dwAni	= g_pInterface->GetModelAnimation(m_hServerObject);
		uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hServerObject);

		if(dwState & MS_PLAYDONE && dwExplodeAnim != dwAni)
		{
			// create the blood pool FX

			//see what kind of blood pool to make
			uint32 dwUserFlags;
			g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUserFlags);
			SurfaceType eSurfaceType = UserFlagToSurface(dwUserFlags);

			ClientIntersectQuery iQuery;
			ClientIntersectInfo  iInfo;

			// ok now get the node location and make the effect...
			LTVector vPos;
			if(!GetNodePos("Torso_u_node", vPos, LTTRUE))
			{
				m_cs.bBloodPool = LTFALSE;
				return;
			}

			iQuery.m_From = vPos + LTVector(0, 1.0f, 0) * 100.0f;
			iQuery.m_To = vPos + LTVector(0, -1.0f, 0) * 100.0f;
			iQuery.m_Flags = IGNORE_NONSOLID | INTERSECT_HPOLY;

			if(g_pLTClient->IntersectSegment(&iQuery, &iInfo))
			{
				// Create a blood pool...
				BSCREATESTRUCT sc;

				// Randomly rotate the blood splat
				g_pLTClient->AlignRotation(&(sc.rRot), &(iInfo.m_Plane.m_Normal), LTNULL);
				g_pLTClient->RotateAroundAxis(&(sc.rRot), &(iInfo.m_Plane.m_Normal), GetRandom(0.0f, MATH_CIRCLE));

				sc.vPos				= iInfo.m_Point + iInfo.m_Plane.m_Normal * 2.0f;  // Off the floor a bit
				sc.vVel				= LTVector(0.0f, 0.0f, 0.0f);
				sc.vInitialScale	= LTVector(0.1f, 0.1f, 0.1f);
				sc.vFinalScale		= LTVector(0.55f, 0.55f, 1.0f);
				sc.dwFlags			= FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_NOLIGHT;
				sc.fLifeTime		= 12.0f;
				sc.fInitialAlpha	= 1.0f;
				sc.fFinalAlpha		= 1.0f;
				sc.nType			= OT_SPRITE;
				sc.hServerObj		= m_hServerObject;
				sc.fFadeOutTime		= 2.0f;

				char* pBloodFiles[] =
				{
					"Sprites\\MarBld6.spr",
					"Sprites\\PrdBld6.spr",
					"Sprites\\AlnBld6.spr",
					"Sprites\\SynBld6.spr",
				};

				switch(eSurfaceType)
				{
					case ST_FLESH_PREDATOR:
					{
						sc.Filename = pBloodFiles[1];
						break;
					}

					case ST_FLESH_ALIEN:
					case ST_FLESH_PRAETORIAN:
					{
						sc.Filename = pBloodFiles[2];
						
						// First see if we have a valid impact FX or not
						m_pImpactFX = g_pFXButeMgr->GetImpactFX("Alien_Blood_Pool");

						if(!m_pImpactFX)
							break;

						// Fill in a create structure
						m_ifxcs.vPos = sc.vPos;
						m_ifxcs.rSurfRot = sc.rRot;
						m_ifxcs.vDir = iInfo.m_Plane.m_Normal;
						m_ifxcs.vSurfNormal = iInfo.m_Plane.m_Normal;
						m_ifxcs.bPlaySound = LTTRUE;

						// Create the effect
						g_pFXButeMgr->CreateImpactFX(m_pImpactFX, m_ifxcs);

						//record the time
						m_fLastSteamTime = g_pLTClient->GetTime();

						break;
					}	

					case ST_FLESH_SYNTH:
					{
						sc.Filename = pBloodFiles[3];
						break;
					}

					case ST_FLESH_HUMAN:
					default:
					{
						sc.Filename = pBloodFiles[0];
						break;
					}
				}

				CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_SCALE_ID, &sc);
				if(pFX) pFX->Update();

				//record the fact we made a pool
				m_bBloodPoolMade = LTTRUE;

				//save the hpoly info
				m_hBloodPoolPoly = iInfo.m_hPoly;
			}	
		}
	}
	else
	{

		CSpecialFX* pFX = psfxMgr->FindSpecialFX(SFX_SCALE_ID, m_hServerObject);

			// Now make sure the color is the same as the body prop... alpha too

		if(pFX)
		{
			HLOCALOBJ hObj = pFX->GetObject();

			//clip to the poly
			g_pLTClient->ClipSprite(hObj, m_hBloodPoolPoly);
		}

		if(m_pImpactFX)
		{
			//see if it is time to make another poof of steam
			LTFLOAT fTime = g_pLTClient->GetTime();
			if(fTime-m_fLastSteamTime > 0.5f)
			{
				// make another FX
				g_pFXButeMgr->CreateImpactFX(m_pImpactFX, m_ifxcs);

				// record the time
				m_fLastSteamTime = g_pLTClient->GetTime();
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::CheckForBleederRemoval()
//
//	PURPOSE:	See if we need to remove any of the bleeder FX...
//
// ----------------------------------------------------------------------- //

bool RemoveBleeder(BodyBleederData* bleeder)
{
	// If the parent piece is hidden then we need to go away...
	if(g_pLTClient->GetTime() - bleeder->fStartTime > 3.0f)
	{
		g_pLTClient->RemoveObject(bleeder->hBleeder);
		delete bleeder;
		bleeder = LTNULL;
		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::CheckForBleederRemoval()
//
//	PURPOSE:	See if we need to remove any of the bleeder FX...
//
// ----------------------------------------------------------------------- //

void BodyPropFX::CheckForBleederRemoval()
{
	BodyBleederList::iterator start_remove = std::remove_if(m_BleederList.begin(),m_BleederList.end(),RemoveBleeder);
	m_BleederList.erase(start_remove, m_BleederList.end());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::AddNewBleeder()
//
//	PURPOSE:	Add a new bleeder..
//
// ----------------------------------------------------------------------- //

void BodyPropFX::AddNewBleeder(ModelNode eNode)
{
	// Allocate a new object
	BodyBleederData* pData = new BodyBleederData;

	if(!pData) return;

	// Load up some data
	pData->eModelNode = eNode;
	pData->pFX = this;

	// Get our position and initialize the rotation
	LTVector vPos;
	LTRotation rRot;
	const char* szNodeName;
	szNodeName = g_pModelButeMgr->GetSkeletonNodeName(m_cs.eModelSkeleton, eNode);
	GetNodePos( szNodeName, vPos, LTTRUE);
	rRot.Init();

	// Now create the system
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_PARTICLESYSTEM;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_UPDATEUNSEEN | FLAG_FOGDISABLE | PS_DUMB;
	createStruct.m_Pos = pData->vLastPos = vPos;
	createStruct.m_Rotation = rRot;

	pData->hBleeder = m_pClientDE->CreateObject(&createStruct);

	if(!pData->hBleeder)
	{
		// Somthing went terribly wrong...
		delete pData;
		return;
	}

	// Set up the new system
	uint32 dwUserFlags;
	g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

	SurfaceType eSurfaceType = UserFlagToSurface(dwUserFlags);

	// Pick the appropriate trails
	switch(eSurfaceType)
	{
		case ST_FLESH_PREDATOR:
			pData->pTrail = g_pFXButeMgr->GetParticleTrailFX("Predator_Blood_Neck_Trail");
			break;

		case ST_FLESH_ALIEN:
		case ST_FLESH_PRAETORIAN:
			pData->pTrail = g_pFXButeMgr->GetParticleTrailFX("Alien_Blood_Neck_Trail");
			break;

		case ST_FLESH_SYNTH:
			pData->pTrail = g_pFXButeMgr->GetParticleTrailFX("Synth_Blood_Neck_Trail");
			break;

		case ST_FLESH_HUMAN:
		default:
			pData->pTrail = g_pFXButeMgr->GetParticleTrailFX("Human_Blood_Neck_Trail");
			break;
	}

	if(!pData->pTrail)
	{
		// Somthing went terribly wrong...
		delete pData;
		return;
	}

    uint32 dwWidth, dwHeight;
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	g_pLTClient->GetSurfaceDims (hScreen, &dwWidth, &dwHeight);

	LTFLOAT fRad = pData->pTrail->fRadius;
    fRad /= ((LTFLOAT)dwWidth);

	g_pLTClient->SetupParticleSystem(pData->hBleeder, pData->pTrail->szTexture,
									 pData->pTrail->fGravity, 0, fRad);

	// Set blend modes if applicable...
    uint32 dwFlags2;
    g_pLTClient->Common()->GetObjectFlags(pData->hBleeder, OFT_Flags2, dwFlags2);

	if (pData->pTrail->bAdditive)
	{
		dwFlags2 |= FLAG2_ADDITIVE;
	}
	else if (pData->pTrail->bMultiply)
	{
		dwFlags2 |= FLAG2_MULTIPLY;
	}
    g_pLTClient->Common()->SetObjectFlags(pData->hBleeder, OFT_Flags2, dwFlags2);

	//Time stamp
	pData->fStartTime = g_pLTClient->GetTime(); 

	// Now add some particls...
	AddBleederParticles(pData);

	// Add the emmitter to the list
	m_BleederList.push_back(pData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::AddBleederParticles()
//
//	PURPOSE:	Add new bleeder particles
//
// ----------------------------------------------------------------------- //

void BodyPropFX::AddBleederParticles(BodyBleederData* pData)
{
	// Mark the time
	pData->fLastDripTime = g_pLTClient->GetTime() + GetRandom(0.0f, g_vtBodyBleederTime.GetFloat());

	// Get the rotation
	const char* szNodeName = g_pModelButeMgr->GetSkeletonNodeName(m_cs.eModelSkeleton, pData->eModelNode);

	LTRotation	rRot;
	LTVector	vPos;
	GetNodePos(szNodeName, vPos, LTFALSE, &rRot);

	LTVector vR, vU, vF;
	g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);
	
	// Null vector
	LTVector vVelMin = vR*-30 + vU*100 + vF*-30;
	LTVector vVelMax = vR*30 + vU*100 + vF*30;

	g_pLTClient->AddParticles(pData->hBleeder, pData->pTrail->nEmitAmount,
							  &pData->pTrail->vMinDrift, &pData->pTrail->vMaxDrift,	// Position offset
							  &vVelMin, &vVelMax,											// Velocity
							  &pData->pTrail->vMinColor, &pData->pTrail->vMaxColor,	// Color
							  pData->pTrail->fLifetime + pData->pTrail->fFadetime, pData->pTrail->fLifetime + pData->pTrail->fFadetime);

	// Go through and set sizes
	LTParticle *pHead = LTNULL, *pTail = LTNULL;
	LTFLOAT	fAlpha = 1 - ((g_pLTClient->GetTime() - pData->fStartTime) / pData->pTrail->fLifetime);
	
	if(g_pLTClient->GetParticles(pData->hBleeder, &pHead, &pTail))
	{
		LTFLOAT fSize = pData->pTrail->fEndScale - fAlpha*(pData->pTrail->fEndScale - pData->pTrail->fStartScale);
		while(pHead && pHead != pTail)
		{
			pHead->m_Alpha = fAlpha;
			pHead->m_Size = fSize;
			pHead = pHead->m_pNext;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::UpdateBleeders()
//
//	PURPOSE:	Update the bleeders...
//
// ----------------------------------------------------------------------- //

void BodyPropFX::UpdateBleeders()
{
	// See if we need to remove a bleeder
	CheckForBleederRemoval();

	for( BodyBleederList::iterator iter = m_BleederList.begin(); iter != m_BleederList.end(); ++iter )
	{
		// First set our position
		LTVector vPos;
		const char* szNodeName;
		szNodeName = g_pModelButeMgr->GetSkeletonNodeName(m_cs.eModelSkeleton, (*iter)->eModelNode);
		GetNodePos( szNodeName, vPos, LTTRUE);

		g_pLTClient->SetObjectPos((*iter)->hBleeder, &vPos);

		// Go through and offset their positions
		LTParticle *pHead = LTNULL, *pTail = LTNULL;
		LTVector vOffset= vPos - (*iter)->vLastPos;

		if(g_pLTClient->GetParticles((*iter)->hBleeder, &pHead, &pTail))
		{
			while(pHead && pHead != pTail)
			{
				//get particle position
				LTVector vPartPos(0,0,0);
				g_pLTClient->GetParticlePos((*iter)->hBleeder, pHead, &vPartPos);

				//adjust for new pos
				vPartPos -= vOffset;

				//re-set pos
				g_pLTClient->SetParticlePos((*iter)->hBleeder, pHead, &vPartPos);
				
				pHead = pHead->m_pNext;
			}
		}

		// Now record the new pos
		(*iter)->vLastPos = vPos;
	
		// Now see if it is time to make more particles..
		if(g_pLTClient->GetTime() - (*iter)->fLastDripTime > g_vtBodyBleederTime.GetFloat())
		{
			AddBleederParticles((*iter));
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyPropFX::ShowAllPieces()
//
//	PURPOSE:	Show all the pieces.
//
// ----------------------------------------------------------------------- //

void BodyPropFX::ShowAllPieces()
{
	// Lithtech assumes that a model has no more than 32 pieces
	for(int i = 0; i < 32; i++)
	{
		g_pModelLT->SetPieceHideStatus(m_hServerObject, (HMODELPIECE)i, LTFALSE);
	}
}

//-------------------------------------------------------------------------------------------------
// SFX_CharacterFactory
//-------------------------------------------------------------------------------------------------

const SFX_BodyPropFactory SFX_BodyPropFactory::registerThis;

CSpecialFX* SFX_BodyPropFactory::MakeShape() const
{
	return new BodyPropFX();
}


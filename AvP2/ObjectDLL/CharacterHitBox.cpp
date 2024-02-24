// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterHitBox.cpp
//
// PURPOSE : Character hit box object class implementation
//
// CREATED : 01/05/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "CharacterHitBox.h"
#include "Character.h"
#include "BodyProp.h"
#include "Projectile.h"
#include "ServerUtilities.h"
#include "CVarTrack.h"
#include "MsgIDs.h"
#include "ObjectMsgs.h"
#include "ModelButeMgr.h"
#include "Attachments.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"
#include "GameServerShell.h"
#include "Turret.h"


#ifndef _FINAL

static CVarTrack g_vtShowNodeRadii;
static CVarTrack g_vtNodeRadiusUseOverride;
static CVarTrack g_vtHeadNodeRadius;
static CVarTrack g_vtTorsoNodeRadius;
static CVarTrack g_vtArmNodeRadius;
static CVarTrack g_vtLegNodeRadius;

const float g_fDefaultNodeRadius = 12.5f;

#endif

const float g_fScalingFactor = 1.2f;

BEGIN_CLASS(CCharacterHitBox)
END_CLASS_DEFAULT_FLAGS(CCharacterHitBox, GameBase, NULL, NULL, CF_HIDDEN)


LTBOOL WorldModelsOnlyFilterFn(HOBJECT hObj, void *pUserData)
{
	BaseClass * pObject = g_pInterface->HandleToObject(hObj);
	if( pObject && OT_WORLDMODEL == pObject->GetType() )
	{
		return LTTRUE;
	}

	return LTFALSE;
}


static LTBOOL HitWorldModel(IntersectInfo * piOrigInfo, const LTVector & vSource, const LTVector & vDest)
{
	IntersectQuery iQuery;
	IntersectInfo iInfo;

	// It is better to test from destination to source, as source may be inside
	// blocking geometry (whereas destination better not be!).
	iQuery.m_From = vDest;
	iQuery.m_To = vSource;

	iQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	iQuery.m_FilterFn = WorldModelsOnlyFilterFn;

	if( g_pInterface->IntersectSegment(&iQuery, &iInfo) )
	{
		if( piOrigInfo )
		{
			piOrigInfo->m_hObject = iInfo.m_hObject;
			piOrigInfo->m_hPoly   = iInfo.m_hPoly;
		}
		
		return LTTRUE;
	}

	return LTFALSE;
}

static bool FindNodeOrSocket(HOBJECT hModel, const char * szNodeName,
							  LTVector * pvPos)
{
	HMODELNODE hNode = INVALID_MODEL_NODE;
	HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
	LTransform transform;

	ModelLT* const pModelLT = g_pLTServer->GetModelLT();

	if( LT_OK == pModelLT->GetNode(hModel, const_cast<char*>(szNodeName), hNode) )
	{
		if( LT_OK != pModelLT->GetNodeTransform(hModel, hNode, transform, LTTRUE) )
		{
			return false;
		}
	}
	else if( LT_OK == pModelLT->GetSocket(hModel, const_cast<char*>(szNodeName), hSocket) )
	{
		if( LT_OK != pModelLT->GetSocketTransform(hModel, hSocket, transform, LTTRUE) )
		{
			return false;
		}
	}
	else
	{
		// Couldn't find a node or socket by this name, so ignore it.
		return false;
	}

	if( pvPos )
	{
		LTRotation rRot;
		TransformLT* pTransLT = g_pLTServer->GetTransformLT();
		pTransLT->Get(transform, *pvPos, rRot);
	}
	
	return true;
}

static LTVector GetHitNodePos(HOBJECT hModel, const CCharacterHitBox::HitNode & hit_node)
{
	LTransform transform;

	ModelLT* const pModelLT = g_pLTServer->GetModelLT();

	if( hit_node.hNode != INVALID_MODEL_NODE )
	{
		pModelLT->GetNodeTransform(hModel, hit_node.hNode, transform, LTTRUE);
	}
	else if( hit_node.hSocket != INVALID_MODEL_SOCKET )
	{
		pModelLT->GetSocketTransform(hModel, hit_node.hSocket, transform, LTTRUE);
	}
	else
	{
		_ASSERT( 0 );

		g_pLTServer->GetObjectPos(hModel, &transform.m_Pos);
	}

	return transform.m_Pos;
}

static LTBOOL RayIntersectsBox( const LTVector & vRayStart, const LTVector & vDir, 
								const LTVector & vBoxPos, LTVector vBoxDims)
{
	// This algorithm is taken from 3D Graphics by Watt (3rd edition), pg. 22.
	const LTVector vRayFar = vRayStart + vDir*5000.0f;

	LTVector vBoxMin = vBoxPos - vBoxDims;
	LTVector vBoxMax = vBoxPos + vBoxDims;

	if( vBoxPos.x - vRayStart.x < 0.0f )
	{
		const LTFLOAT fTemp = vBoxMin.x;
		vBoxMin.x = vBoxMax.x;
		vBoxMax.x = fTemp;
		vBoxDims.x = -vBoxDims.x;
	}

	if( vBoxPos.y - vRayStart.y < 0.0f )
	{
		const LTFLOAT fTemp = vBoxMin.y;
		vBoxMin.y = vBoxMax.y;
		vBoxMax.y = fTemp;
		vBoxDims.y = -vBoxDims.y;
	}

	if( vBoxPos.z - vRayStart.z < 0.0f )
	{
		const LTFLOAT fTemp = vBoxMin.z;
		vBoxMin.z = vBoxMax.z;
		vBoxMax.z = fTemp;
		vBoxDims.z = -vBoxDims.z;
	}

	LTFLOAT t_near = -5000.0f;
	LTFLOAT t_far = 5000.0f;

	if( vBoxDims.x > 0 )
	{
		const LTFLOAT tx_near = (vBoxMin.x - vRayStart.x)/vBoxDims.x;
		const LTFLOAT tx_far = (vBoxMax.x - vRayStart.x)/vBoxDims.x;

		if( tx_near > t_near )
		{
			t_near = tx_near;
		}

		if( tx_far < t_far )
		{
			t_far = tx_far;
		}
	}

	if( vBoxDims.y > 0 )
	{
		const LTFLOAT ty_near = (vBoxMin.y - vRayStart.y)/vBoxDims.y;
		const LTFLOAT ty_far = (vBoxMax.y - vRayStart.y)/vBoxDims.y;

		if( ty_near > t_near )
		{
			t_near = ty_near;
		}

		if( ty_far < t_far )
		{
			t_far = ty_far;
		}
	}

	if( vBoxDims.z > 0 )
	{
		const LTFLOAT tz_near = (vBoxMin.z - vRayStart.z)/vBoxDims.z;
		const LTFLOAT tz_far = (vBoxMax.z - vRayStart.z)/vBoxDims.z;

		if( tz_near > t_near )
		{
			t_near = tz_near;
		}

		if( tz_far < t_far )
		{
			t_far = tz_far;
		}
	}

	return t_near < t_far;
}


// This is really a function which returns multiple values.  Use the second constructor
// as a function.

// FindNodeHit finds the nearest node hit from a list of hit nodes.  If a node was not hit, eModelHitNode will
// be set to eModelNodeInvalid.
struct FindNodeHit
{

	LTFLOAT fNearestRayDist;
	LTVector vNodePos;
	ModelNode eModelNodeHit;

	FindNodeHit()
		: fNearestRayDist(1000.0f), 
		  vNodePos(0,0,0),
		  eModelNodeHit(eModelNodeInvalid) {}

	FindNodeHit(HOBJECT hModel, LTVector vStartPoint, LTVector vRayDir, CCharacterHitBox::HitNodeList::const_iterator beginHitNodes, CCharacterHitBox::HitNodeList::const_iterator endHitNodes, LTFLOAT fDistSqr)
		: fNearestRayDist(1000.0f), // Initialize to an "obviously" too large value.
		  vNodePos(0,0,0),
		  eModelNodeHit(eModelNodeInvalid)
	{
		// Only did hit detection if our model is still in engine, otherwise propagate
		// shot through hit box.
		if( hModel == LTNULL )
		{
			return;
		}

		// Some temporaries.
		LTBOOL bHidden = LTFALSE;

		// See if we hit a node.
		for( CCharacterHitBox::HitNodeList::const_iterator iter = beginHitNodes; iter != endHitNodes; ++iter )
		{
			const CCharacterHitBox::HitNode & currentHitNode = *iter;

			// If the piece is hidden... just skip this node
			if(currentHitNode.hPiece != INVALID_MODEL_PIECE)
			{
				g_pModelLT->GetPieceHideStatus(hModel, currentHitNode.hPiece, bHidden);
				if(bHidden) continue;
			}

			// Determine the node's position.
			const LTVector vCurrentNodePos = GetHitNodePos(hModel, currentHitNode);
			const LTVector vRelativeNodePos = vCurrentNodePos - vStartPoint;

			// Distance along ray to point of closest approach to node point
			const LTFLOAT fRayDist = vRayDir.Dot(vRelativeNodePos); 

			// If this value is negitive then the vector gets no closer
			// to the node then the start position
			// Unless we are using a line rather than a ray (a line extends in both directions, a ray extends only forward).
			if( fRayDist >= 0.0f || fDistSqr > 2500.0f)
			{
				const LTFLOAT fDistSqr = (vRayDir*fRayDist - vRelativeNodePos).MagSqr();

				if( fDistSqr < currentHitNode.fRadiusSqr )
				{
					// Aha!  The node has been hit!  
					// Check to see if the node is closer to
					// the ray's origin.  Record it as the hit
					// if it is.
					if( fRayDist < fNearestRayDist )
					{
						eModelNodeHit = ModelNode(iter - beginHitNodes);
						fNearestRayDist = fRayDist;
						vNodePos = vCurrentNodePos;
					}
				}
			}
		}
	}
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::CCharacterHitBox()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CCharacterHitBox::CCharacterHitBox() : GameBase(OT_NORMAL) 
{ 
	m_hModel = LTNULL;
	m_pCharacter = LTNULL;
	m_vOffset.Init();
	m_vModelDims.Init();
	m_bCanBeHit = LTTRUE;

	m_bOverrideDims = LTFALSE;
	m_vOverrideDims.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::~CCharacterHitBox()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CCharacterHitBox::~CCharacterHitBox() 
{
#ifndef _FINAL
	RemoveNodeRadiusModels();
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Setup()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::Init(HOBJECT hModel) 
{ 
	if (!m_hObject || !hModel || 
		(!IsCharacter(hModel) && !IsBodyProp(hModel))) return LTFALSE;

	m_hModel = hModel;
	m_pCharacter = dynamic_cast<CCharacter*>( m_hModel ? g_pLTServer->HandleToObject(m_hModel) : LTNULL );

	ResetSkeleton();

	// Set my flag...Should only need flag rayhit...

	g_pLTServer->SetObjectFlags(m_hObject, FLAG_RAYHIT | FLAG_TOUCH_NOTIFY | FLAG_GOTHRUWORLD );

#ifndef _FINAL
	if (!g_vtShowNodeRadii.IsInitted())
	{
		g_vtShowNodeRadii.Init(g_pLTServer, "HitBoxShowNodeRadii", LTNULL, 0.0f);
	}

	if (!g_vtNodeRadiusUseOverride.IsInitted())
	{
		g_vtNodeRadiusUseOverride.Init(g_pLTServer, "HitBoxNodeRadiusOverride", LTNULL, 0.0f);
	}

	if (!g_vtHeadNodeRadius.IsInitted())
	{
		g_vtHeadNodeRadius.Init(g_pLTServer, "HitBoxHeadNodeRadius", LTNULL, g_fDefaultNodeRadius);
	}
	
	if (!g_vtTorsoNodeRadius.IsInitted())
	{
		g_vtTorsoNodeRadius.Init(g_pLTServer, "HitBoxTorsoNodeRadius", LTNULL, g_fDefaultNodeRadius);
	}
	
	if (!g_vtArmNodeRadius.IsInitted())
	{
		g_vtArmNodeRadius.Init(g_pLTServer, "HitBoxArmNodeRadius", LTNULL, g_fDefaultNodeRadius);
	}
	
	if (!g_vtLegNodeRadius.IsInitted())
	{
		g_vtLegNodeRadius.Init(g_pLTServer, "HitBoxLegNodeRadius", LTNULL, g_fDefaultNodeRadius);
	}
#endif

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::ResetSkeleton
//
//	PURPOSE:	Resets the hit nodes.
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::ResetSkeleton()
{
#ifndef _FINAL
	RemoveNodeRadiusModels();
#endif

	if( m_hModel.GetHOBJECT() != LTNULL )
	{
		const ModelSkeleton eModelSkeleton = GetModelSkeleton();

		// Iterate through each node and/or socket.  If the ray passes 
		// with that node's radius, consider the AI hit at that location.
		// The algorithm goes through all the nodes, returning the node 
		// which was "first" hit (the hit node closest to the rays origin).
		const int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
		
		// Re-size our self to be the same size as the number of nodes (use swap to keep the memory trimmed down).
		HitNodeList tempList = HitNodeList(cNodes);
		m_HitNodes.swap( tempList );

		// Now fill in the hit node list.
		for( HitNodeList::iterator iter = m_HitNodes.begin(); iter != m_HitNodes.end(); ++iter )
		{
			// Gather the node's information from the Skeleton bute file.
			HitNode & currentHitNode = *iter;

			ModelNode eCurrentNode = ModelNode( iter - m_HitNodes.begin() );

			// Get the node's name.
			currentHitNode.szName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);

			// Get the node or socket id.
			if( currentHitNode.szName )
			{
				ModelLT* pModelLT = g_pLTServer->GetModelLT();

				// First try a node.
				if( LT_OK != pModelLT->GetNode(m_hModel, const_cast<char*>(currentHitNode.szName), currentHitNode.hNode) )
				{
					// If that failed, try a socket.
					if( LT_OK != pModelLT->GetSocket(m_hModel, const_cast<char*>(currentHitNode.szName), currentHitNode.hSocket) )
					{
#ifndef _FINAL
						char model_filename[256];
						char junk[1];
						g_pLTServer->GetModelFilenames(m_hModel,model_filename,256,junk,1);

						g_pLTServer->CPrint(
							"CCharacterHitBox : Model %s, set to skeleton %s,",
							model_filename,
							g_pModelButeMgr->GetNameFromSkeleton(eModelSkeleton) );
						g_pLTServer->CPrint(
							"                      does not have a node or socket named \"%s\".",
							currentHitNode.szName );
#endif
					}
				}
			}

			// Get the model piece related to this node
			const char *szPiece = g_pModelButeMgr->GetSkeletonNodePiece(eModelSkeleton, eCurrentNode);
			if( szPiece && szPiece[0] )
				g_pModelLT->GetPiece(m_hModel, const_cast<char*>(szPiece), currentHitNode.hPiece );

			// Get the node's hit radius.
			currentHitNode.fRadiusSqr = GetNodeRadius(eModelSkeleton, eCurrentNode);
			currentHitNode.fRadiusSqr *= currentHitNode.fRadiusSqr;
		}
	}
	else
	{
		// Release our memory, and be empty.
		HitNodeList tempList = HitNodeList();
		m_HitNodes.swap(tempList);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CCharacterHitBox::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			g_pLTServer->SetNextUpdate(m_hObject, 0.0f);
			g_pLTServer->SetDeactivationTime(m_hObject, 0.001f);
			g_pLTServer->SetObjectState(m_hObject, OBJSTATE_AUTODEACTIVATE_NOW);
		}
		break;

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

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

uint32 CCharacterHitBox::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	if (!g_pLTServer) return 0;


	switch(messageID)
	{
		case MID_TRIGGER:
		{
			// pass on all trigger messages to our owner (if they exist).
			if( m_hModel.GetHOBJECT() != LTNULL )
			{
				LPBASECLASS pOwner = g_pLTServer->HandleToObject(m_hModel);
				return pOwner->ObjectMessageFn(hSender, messageID, hRead);
			}
		}
		break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::GetBoundingBoxColor()
//
//	PURPOSE:	Get the color of the bounding box
//
// ----------------------------------------------------------------------- //
	
#ifndef _FINAL
LTVector CCharacterHitBox::GetBoundingBoxColor()
{
	return LTVector(1.0, 1.0, 0.0);
}
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::HandleBoxImpact()
//
//	PURPOSE:	Handle a vector impacting on us
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::HandleBoxImpact(CProjectile* pProj, const IntersectInfo& iInfo, const LTVector& vDir,
										   LTVector * pvFrom, ModelNode* peModelNode, LTVector* pvNodePos)
{
	if (!pProj || !m_hModel.GetHOBJECT()) return LTFALSE;


	LTVector vModelPos;
	g_pLTServer->GetObjectPos(m_hModel,&vModelPos);

	if( RayIntersectsBox( iInfo.m_Point, vDir, vModelPos, m_vModelDims ) )
	{
		const ModelSkeleton eModelSkeleton = GetModelSkeleton();

		// Pick a random node to hit.
		ModelNode eNodeHit = ModelNode( GetRandom( 0, g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton) - 1) );

		if( g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, eNodeHit) <= 0.0f )
		{
			eNodeHit = g_pModelButeMgr->GetSkeletonDefaultHitNode(eModelSkeleton);
		}

		if( peModelNode )
		{
			*peModelNode = eNodeHit;
		}

		if( pvNodePos )
		{
			*pvNodePos = vModelPos;

//				const char* szNodeName =  g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eNodeHit);
//				if( szNodeName && m_hModel)
//					FindNodeOrSocket( m_hModel, szNodeName, pvNodePos );
		}

		return LTTRUE;
	}

	//
	// Shot didn't hit box, propagate through box and return that point in 
	// vFrom
	//

	if( pvFrom )
	{
		// just move the point to the edge and one unit in
		*pvFrom = iInfo.m_Point + vDir;
	}

	return LTFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::HandleImpact()
//
//	PURPOSE:	Handle a vector impacting on us
//
// ----------------------------------------------------------------------- //
static LTBOOL ShouldUseBoxImpact(const CCharacter * pDamagee, HOBJECT hDamager)
{
	if( !g_pGameServerShell->GetGameType().IsSinglePlayer() )
		return LTFALSE;

	// The player always gets box physics when attacked (for speed).
	if( dynamic_cast<const CPlayerObj*>( pDamagee ) )
	{
		return LTTRUE;
	}
	// Use box hit detection for allies, unless the firer is the player. (for speed).
	else if( hDamager && pDamagee)
	{
		CCharacter * pCharDamager = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject( hDamager ) );
		CPlayerObj * pPlayerDamager = dynamic_cast<CPlayerObj*>( pCharDamager );

		if( !pPlayerDamager )
		{
			if( pCharDamager )
			{
				// Allies should use box impacts with each other.
				if( g_pCharacterMgr->AreAllies(*pCharDamager,*pDamagee) )
				{
					return LTTRUE;
				}
			}
			else if( !pPlayerDamager )
			{
				// Turrets want to use box impacts!
				TurretWeapon * pTurretWeapon = dynamic_cast<TurretWeapon*>( g_pLTServer->HandleToObject( hDamager ) );
				Turret *       pTurret = dynamic_cast<Turret*>( g_pLTServer->HandleToObject( hDamager ) );

				if( pTurretWeapon || pTurret )
				{
					return LTTRUE;
				}
			}
		}
			
	}

	return LTFALSE;
}

LTBOOL CCharacterHitBox::HandleImpact(CProjectile* pProj, IntersectInfo * piInfo, 
									 LTVector vDir, LTVector * pvFrom, LTVector* pvNodePos)
{
	if (!pProj || !m_hModel.GetHOBJECT()) return LTFALSE;

	if( !m_bCanBeHit )
		return LTFALSE;

	LTBOOL bHitSomething = LTTRUE;
	ModelNode eModelNode = eModelNodeInvalid;

	if (UsingHitDetection() && piInfo)
	{

#ifndef _FINAL
		StartTimingCounter();
#endif		
		if( !ShouldUseBoxImpact(m_pCharacter, pProj->GetFiredFrom()) )
		{
			bHitSomething = HandleVectorImpact(pProj, *piInfo, vDir, pvFrom, &eModelNode, pvNodePos);
		}
		else
		{
			bHitSomething = HandleBoxImpact(pProj,*piInfo,vDir,pvFrom,&eModelNode, pvNodePos );
		}
		
#ifndef _FINAL
		EndTimingCounter("CCharacterHitBox::HandleImpact()");
#endif
		// Did we hit something?

		if (bHitSomething)
		{
			// If there is a world model between the shooter and the node,
			// hit that instead.
			if( HitWorldModel(piInfo, piInfo->m_Point, *pvNodePos) )
			{
				return bHitSomething;
			}

			// This is the object that *really* got hit...

			piInfo->m_hObject = m_hModel;

			if (eModelNode != eModelNodeInvalid)
			{
				// Be sure everyone knows this is the last node hit.
				SetModelNodeLastHit(eModelNode);

				// Adjust the damage based on this node.
				if (IsCharacter(m_hModel))
				{
					CCharacter* pChar = (CCharacter*) g_pLTServer->HandleToObject(m_hModel);
					if (pChar)
					{
						pProj->AdjustDamage(*pChar, eModelNode);
					}
				}
				else if (IsBodyProp(m_hModel))
				{
					BodyProp* pProp = (BodyProp*) g_pLTServer->HandleToObject(m_hModel);
					if (pProp)
					{
						pProj->AdjustDamage(*pProp, eModelNode);
					}
				}
			
			}
		}
	}

	return bHitSomething;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::HandleImpact()
//
//	PURPOSE:	Handle a vector impacting on us
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::HandleForcedImpact(CProjectile* pProj, ModelNode& eNodeHit, LTVector* pvNodePos)
{
	// More sanity!  Is all this checking and re-checking really needed?  Yeah.
	if (!pProj || !m_hModel.GetHOBJECT() || !m_bCanBeHit) return;


	// Remember where we last were hit...
	SetModelNodeLastHit(eNodeHit);


	// Adjust the damage based on this node.  And determine our skeleton.
	ModelSkeleton eModelSkeleton = eModelSkeletonInvalid;

	if (IsCharacter(m_hModel))
	{
		CCharacter* pChar = (CCharacter*) g_pLTServer->HandleToObject(m_hModel);
		if (pChar)
		{
			pProj->AdjustDamage(*pChar, eNodeHit);

			eModelSkeleton = pChar->GetModelSkeleton();
		}
	}
	else if (IsBodyProp(m_hModel))
	{
		BodyProp* pProp = (BodyProp*) g_pLTServer->HandleToObject(m_hModel);
		if (pProp)
		{
			pProj->AdjustDamage(*pProp, eNodeHit);

			eModelSkeleton = pProp->GetModelSkeleton();
		}
	}

	const char* szNodeName =  g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eNodeHit);
	if( szNodeName && m_hModel)
		FindNodeOrSocket( m_hModel, szNodeName, pvNodePos );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::HandleVectorImpact()
//
//	PURPOSE:	Handle being hit by a vector
//
// Peter Higley method...
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::HandleVectorImpact(CProjectile* pProj, const IntersectInfo& iInfo, const LTVector& vDir,
										   LTVector * pvFrom, ModelNode* peModelNode, LTVector* pvNodePos)
{
	if( !m_bCanBeHit )
		return LTFALSE;

	// Only did hit detection if our model is still in engine, otherwise propagate
	// shot through hit box.
	if( m_hModel )
	{
		LTFLOAT fDist = (iInfo.m_Point-(*pvFrom)).MagSqr();
		FindNodeHit result( m_hModel, iInfo.m_Point, vDir, m_HitNodes.begin(), m_HitNodes.end(), fDist );

		// Did we hit something?

		if (result.eModelNodeHit != eModelNodeInvalid)
		{
			// finally record the point info
			if(pvNodePos)
				*pvNodePos = result.vNodePos;
			if(peModelNode)
				*peModelNode = result.eModelNodeHit;

			return LTTRUE;
		}

	} //if( m_hModel != LTNULL )
	
	//
	// Shot didn't hit box, propagate through box and return that point in 
	// vFrom
	//

	if( pvFrom )
	{
		// just move the point to the edge and one unit in
		*pvFrom = iInfo.m_Point + vDir;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::DidProjectileImpact()
//
//	PURPOSE:	See if the projectile actually hit us
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::DidProjectileImpact(const CProjectile* pProjectile)
{
    if (!m_hModel || !pProjectile) return LTFALSE;

	if( !m_bCanBeHit )
		return LTFALSE;

	ModelSkeleton eModelSkeleton = GetModelSkeleton();

	int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
    const char* szNodeName = LTNULL;

	// This is the position which will be used to make sure there is no
	// blocking geometry.
	LTVector vClosestPos;
	g_pLTServer->GetObjectPos(m_hModel, &vClosestPos);
	LTFLOAT fClosestRelDistSqr = FLT_MAX;
	ModelNode aHitNodes[64];
	int cHitNodes = 0;

	//get the projectile's position
	//cant use start point since some projectiles turn
	LTVector vFirePos;
	g_pLTServer->GetObjectPos(pProjectile->m_hObject, &vFirePos);

	//get its velocity
	LTVector vFireDir;
	g_pLTServer->GetVelocity(pProjectile->m_hObject, &vFireDir);

	//only need the direction
	vFireDir.Norm();

	//move back our fire point a bit
	vFirePos -= vFireDir*100;

	{for (int iNode = 0; iNode < cNodes; iNode++)
	{
		ModelNode eCurrentNode = (ModelNode)iNode;
		szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);

		if (szNodeName)
		{
            ILTModel* pModelLT = g_pLTServer->GetModelLT();

            LTVector vPos;
			if(FindNodeOrSocket(m_hModel, szNodeName, &vPos))
			{
				LTFLOAT fNodeRadius = GetNodeRadius(eModelSkeleton, eCurrentNode);

				const LTVector vRelativeNodePos = vPos - vFirePos;

				// Distance along ray to point of closest approach to node point

				const LTFLOAT fRayDist = vFireDir.Dot(vRelativeNodePos);
				const LTFLOAT fDistSqr = (vFireDir*fRayDist - vRelativeNodePos).MagSqr();

				if (fDistSqr < fNodeRadius*fNodeRadius)
				{
					aHitNodes[cHitNodes++] = (ModelNode)iNode;

					if( fDistSqr < fClosestRelDistSqr )
					{
						fClosestRelDistSqr = fDistSqr;
						vClosestPos = vPos;
					}
				}
			}
		}
	}}

	// Find highest priority node we hit

    LTFLOAT fMaxPriority = (LTFLOAT)(-INT_MAX);

//	if ( g_HitDebugTrack.GetFloat(0.0f) == 1.0f )
//      g_pLTServer->CPrint("Checking hit nodes..................");

	ModelNode eModelNode = eModelNodeInvalid;

	{for ( int iNode = 0 ; iNode < cHitNodes ; iNode++ )
	{
//		if ( g_HitDebugTrack.GetFloat(0.0f) == 1.0f )
//          g_pLTServer->CPrint("Hit ''%s'' node", g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, (ModelNode)aHitNodes[iNode]));

			//TODO: Adde node priority or just go with the first node on the list.
			LTFLOAT fPriority = 1.0f;//g_pModelButeMgr->GetSkeletonNodeHitPriority(eModelSkeleton, (ModelNode)aHitNodes[iNode]);
			if ( fPriority > fMaxPriority )
			{
				eModelNode = (ModelNode)aHitNodes[iNode];
				fMaxPriority = fPriority;
			}
	}}

	// Did we hit something?

	if (eModelNode != eModelNodeInvalid)
	{
		if( !HitWorldModel(LTNULL,vFirePos,vClosestPos) )
		{
//			if ( g_HitDebugTrack.GetFloat(0.0f) == 1.0f )
//				g_pLTServer->CPrint("...........using ''%s''", g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eModelNode));

			// Set our last hit node

			SetModelNodeLastHit(eModelNode);

			return LTTRUE;
		}
	}

//	if ( g_HitDebugTrack.GetFloat(0.0f) == 1.0f )
//      g_pLTServer->CPrint("....................hit nothing");

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::GetNodeRadius()
//
//	PURPOSE:	Get the model node's radius
//
// ----------------------------------------------------------------------- //

LTFLOAT CCharacterHitBox::GetNodeRadius(ModelSkeleton eModelSkeleton, 
									   ModelNode eModelNode)
{	
	LTFLOAT fRadius = g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, eModelNode);

	// See if we're overriding the radius...

#ifndef _FINAL
	if (g_vtNodeRadiusUseOverride.GetFloat())
	{
		HitLocation eLocation =	g_pModelButeMgr->GetSkeletonNodeHitLocation(eModelSkeleton, eModelNode);
		switch (eLocation)
		{
			case HL_HEAD :
				fRadius = g_vtHeadNodeRadius.GetFloat();
			break;

			case HL_TORSO :
				fRadius = g_vtTorsoNodeRadius.GetFloat();
			break;

			case HL_ARM :
				fRadius = g_vtArmNodeRadius.GetFloat();
			break;

			case HL_LEG :
				fRadius = g_vtLegNodeRadius.GetFloat();
			break;

			default :
				fRadius = g_fDefaultNodeRadius;
			break;
		}
	}
#endif

	return fRadius;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::GetModelSkeleton()
//
//	PURPOSE:	Get our model's skeleton
//
// ----------------------------------------------------------------------- //

ModelSkeleton CCharacterHitBox::GetModelSkeleton()
{
	if (!m_hModel) return eModelSkeletonInvalid;

	ModelSkeleton eModelSkeleton = eModelSkeletonInvalid;

	if (m_pCharacter)
	{
		eModelSkeleton = m_pCharacter->GetModelSkeleton();
	}
	else if (IsBodyProp(m_hModel))
	{
		BodyProp* pProp = (BodyProp*) g_pLTServer->HandleToObject(m_hModel);
		if (pProp)
		{
			eModelSkeleton = pProp->GetModelSkeleton();
		}
	}

	return eModelSkeleton;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::SetModelNodeLastHit()
//
//	PURPOSE:	Set our model's last hit node
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::SetModelNodeLastHit(ModelNode eModelNode)
{
	if(!m_hModel) return;

	if(m_pCharacter)
	{
		m_pCharacter->SetModelNodeLastHit(eModelNode);
	}
	else if(IsBodyProp(m_hModel))
	{
		BodyProp* pBody = (BodyProp*) g_pLTServer->HandleToObject(m_hModel);
		if(pBody)
		{
			pBody->SetModelNodeLastHit(eModelNode);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::SetModelNodeLastHit()
//
//	PURPOSE:	Set our model's last hit node
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::UsingHitDetection()
{
	if (!m_hModel) return LTFALSE;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::CreateNodeRadiusModels()
//
//	PURPOSE:	Create models representing the model node radii
//
// ----------------------------------------------------------------------- //
#ifndef _FINAL

void CCharacterHitBox::CreateNodeRadiusModels()
{
	if (!m_hModel) return;

	RemoveNodeRadiusModels();

	for( HitNodeList::const_iterator iter = m_HitNodes.begin(); iter != m_HitNodes.end(); ++iter )
	{
		const HitNode & currentHitNode = *iter;

		// Create the radius model...

		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);

		sprintf(theStruct.m_Name, "%s-RadiusModel", g_pLTServer->GetObjectName(m_hObject) );

		theStruct.m_Pos = GetHitNodePos(m_hModel, currentHitNode);

#ifdef _WIN32
		SAFE_STRCPY(theStruct.m_Filename, "Models\\10x10sphere.abc");
		SAFE_STRCPY(theStruct.m_SkinName, "Skins\\10x10sphere.dtx");
#else
		SAFE_STRCPY(theStruct.m_Filename, "Models/10x10sphere.abc");
		SAFE_STRCPY(theStruct.m_SkinName, "Skins/10x10sphere.dtx");
#endif

		theStruct.m_Flags = FLAG_VISIBLE;
		theStruct.m_ObjectType = OT_MODEL;

		HCLASS hClass = g_pLTServer->GetClass("GameBase");
		LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
		if (!pModel) return;

		LTFLOAT fNodeRadius = currentHitNode.fRadiusSqr > 0.0f ? (LTFLOAT)sqrt(currentHitNode.fRadiusSqr) : 0.5f;

		LTVector vScale(fNodeRadius/10.0f, fNodeRadius/10.0f, fNodeRadius/10.0f);
		g_pLTServer->ScaleObject(pModel->m_hObject, &vScale);

		LTFLOAT r, g, b, a;
		g_pLTServer->GetObjectColor(pModel->m_hObject, &r, &g, &b, &a);
		g_pLTServer->SetObjectColor(pModel->m_hObject, r, g, b, 0.5f);

		// Add the model to our list...
		m_NodeRadiusList.push_back(NodeRadiusStruct(pModel->m_hObject,*iter));
	}
}

#endif
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::RemoveNodeRadiusModels()
//
//	PURPOSE:	Remove models representing the model node radii
//
// ----------------------------------------------------------------------- //
#ifndef _FINAL

void CCharacterHitBox::RemoveNodeRadiusModels()
{
	for( NodeRadiusList::iterator iter = m_NodeRadiusList.begin(); iter != m_NodeRadiusList.end(); ++iter )
	{
		if( iter->hModel )
			g_pLTServer->RemoveObject(iter->hModel);
	}

	m_NodeRadiusList.clear();
}
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::UpdateNodeRadiusModels()
//
//	PURPOSE:	Update models representing the model node radii
//
// ----------------------------------------------------------------------- //

#ifndef _FINAL
void CCharacterHitBox::UpdateNodeRadiusModels()
{
	if (!m_hModel) return;

	// Create the models if necessary...

	if( m_NodeRadiusList.size() != m_HitNodes.size() )
	{
		CreateNodeRadiusModels();
	}


	for( NodeRadiusList::const_iterator iter = m_NodeRadiusList.begin(); iter != m_NodeRadiusList.end(); ++iter )
	{
		if( iter->hModel )
		{
			const LTVector vPos = GetHitNodePos(m_hModel, iter->hitNode);
			g_pLTServer->SetObjectPos(iter->hModel, const_cast<LTVector*>(&vPos) );
		}
	}
}

#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Update()
//
//	PURPOSE:	Update our position
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::Update()
{
	if (!m_hModel || !m_hObject) return;

	// Follow our model's position.
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hModel, &vPos);
	vPos += m_vOffset;
	g_pLTServer->SetObjectPos(m_hObject, &vPos);

	// Follow our model's dimensions.
	LTVector vDims;

	if(m_bOverrideDims)
		vDims = m_vOverrideDims;
	else
		g_pLTServer->GetObjectDims(m_hModel, &vDims);

	if( vDims != m_vModelDims )
	{
		m_vModelDims = vDims;

		//don't adjust the Y beacuse you will then be able to hit
		//character standing on thin "grate" brushes (bug 1013) from below
		if( !m_hModel || !m_pCharacter )
		{
			vDims.x *= g_fScalingFactor;
			vDims.z *= g_fScalingFactor;
		}
		else
		{
			vDims.x *= m_pCharacter->GetButes()->m_fHitBoxScale;
			vDims.z *= m_pCharacter->GetButes()->m_fHitBoxScale;
		}

		g_pLTServer->SetObjectDims(m_hObject, &vDims);
	}

	// See if we should show our model node radii...

#ifndef _FINAL
	if (g_vtShowNodeRadii.GetFloat())
	{
		UpdateNodeRadiusModels();
	}
	else if( !m_NodeRadiusList.empty() )
	{
		RemoveNodeRadiusModels();
	}
#endif
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

	g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hModel);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vOffset);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vModelDims);

	g_pLTServer->WriteToMessageByte(hWrite, m_bOverrideDims);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vOverrideDims);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

	m_hModel = hRead->ReadObject();
	m_pCharacter = dynamic_cast<CCharacter*>( m_hModel ? g_pLTServer->HandleToObject(m_hModel) : LTNULL );

	g_pLTServer->ReadFromMessageVector(hRead, &m_vOffset);
	g_pLTServer->ReadFromMessageVector(hRead, &m_vModelDims);

	m_bOverrideDims = g_pLTServer->ReadFromMessageByte(hRead);
	g_pLTServer->ReadFromMessageVector(hRead, &m_vOverrideDims);
}


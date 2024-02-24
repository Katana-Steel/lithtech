// ----------------------------------------------------------------------- //
//
// MODULE  : PredDisk.h
//
// PURPOSE : Predator Disk Weapon class - implementation
//
// CREATED : 8/8/2000
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PredDisk.h"
#include "SurfaceFunctions.h"
#include "CharacterHitBox.h"
#include "ObjectMsgs.h"
#include "Character.h"
#include "SFXMsgIds.h"
#include "ServerSoundMgr.h"
#include "FXButeMgr.h"
#include "PlayerObj.h"
#include "CVarTrack.h"
#include "GameServerShell.h"

BEGIN_CLASS(CPredDisk)
END_CLASS_DEFAULT_FLAGS(CPredDisk, CProjectile, LTNULL, LTNULL, CF_HIDDEN)

static CVarTrack g_vtDiskSpeed;
static CVarTrack g_vtDiskTurnRate;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredDisk::Setup()
//
//	PURPOSE:	Setup the grenade...
//
// ----------------------------------------------------------------------- //

void CPredDisk::Setup(const ProjectileSetup & setup, const WFireInfo & info)
{
	// See that our console vars are set up...
	if (!g_vtDiskSpeed.IsInitted())
	{
		g_vtDiskSpeed.Init(g_pLTServer, "DiskSpeed", LTNULL, 900.0f);
	}

	if (!g_vtDiskTurnRate.IsInitted())
	{
		g_vtDiskTurnRate.Init(g_pLTServer, "DiskTurnRate", LTNULL, 100.0f);
	}

	CProjectile::Setup(setup, info);

	if ( !GetAmmoClassData() ) return;

	m_hFiredFrom = GetFiredFrom();

	m_pClassData = dynamic_cast<PREDDISKCLASSDATA*>(GetAmmoClassData());

	uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	dwFlags |= (FLAG_TOUCH_NOTIFY | FLAG_VISIBLE);
	dwFlags &= ~FLAG_GRAVITY;
	g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

	g_pLTServer->SetVelocity(m_hObject, &(info.vPath * g_vtDiskSpeed.GetFloat()));

	//play powerup animation
	HMODELANIM nAni	= g_pServerDE->GetAnimIndex(m_hObject, "PowerUp");
	g_pServerDE->SetModelAnimation(m_hObject, nAni);
	g_pServerDE->SetModelLooping(m_hObject, LTFALSE);
	g_pServerDE->ResetModelAnimation(m_hObject);  // Start from beginning

	// See that we cannot be damaged
	SetCanDamage(LTFALSE);

	// Assigin our target
	m_hTarget = info.hTestObj;

	// Pick a cool node to target...
	ModelLT* pModelLT   = g_pLTServer->GetModelLT();
	pModelLT->GetNode(m_hTarget, GetRandomNode(), m_hTargetNode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredDisk::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CPredDisk::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				SAFE_STRCPY(pStruct->m_Name, "PredatorDisk");
			}			
		}
		break;

		default : break;
	}

	return CProjectile::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredDisk::UpdateGrenade()
//
//	PURPOSE:	Update the grenade...
//
// ----------------------------------------------------------------------- //

void CPredDisk::Update()
{
	// see if we are still inside the world...
	LTVector vPos, vMin, vMax;
	g_pLTServer->GetWorldBox(vMin, vMax);
	g_pLTServer->GetObjectPos(m_hObject, &vPos);	

	if (vPos.x < vMin.x || vPos.y < vMin.y || vPos.z < vMin.z ||
		vPos.x > vMax.x || vPos.y > vMax.y || vPos.z > vMax.z)
	{
		HandleDiskRetrieve(LTTRUE);
		return;
	}

	//check to see if we are moving
	LTVector vVel;
	g_pLTServer->GetVelocity(m_hObject, &vVel);

	//set movement flag
	LTFLOAT fVel = vVel.MagSqr();
	m_bMoving = fVel != 0;

	// Cap the velocity...
	if(fVel > g_vtDiskSpeed.GetFloat() * g_vtDiskSpeed.GetFloat())
	{
		vVel.Norm();
		vVel *= g_vtDiskSpeed.GetFloat();
		g_pLTServer->SetVelocity(m_hObject, &vVel);
	}

	// Keep moving...
	if(m_bMoving)
	{
		// Be sure we get updated...
		g_pLTServer->SetNextUpdate(m_hObject, 0.001f);

		Pursue();
	}
	else
	{
//		if(m_bStickHack)
//		{
//			g_pLTServer->TeleportObject(m_hObject, &m_vStickHackPos);
//			m_bStickHack = LTFALSE;
//		}
		
		// If our owner is gone then remove us...
		if(!m_hFiredFrom)
		{
			g_pLTServer->SetNextUpdate(m_hObject, 0);
			g_pLTServer->RemoveObject(m_hObject);
			return;
		}

		// Ok, owner is not gone so check some other stuff...
		CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(m_hFiredFrom));
		if(pChar)
		{
			// See if our owner has died...
			if(pChar->IsDead() || pChar->HasDiscAmmo())
			{
				g_pLTServer->SetNextUpdate(m_hObject, 0);
				g_pLTServer->RemoveObject(m_hObject);
				return;
			}
		}
	
		// Be sure we get updated...
		g_pLTServer->SetNextUpdate(m_hObject, UPDATE_DELTA);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredDisk::HandleDiskRetrieve
//
//	PURPOSE:	Handle heading back to our owner...
//
// ----------------------------------------------------------------------- //

void CPredDisk::HandleDiskRetrieve(LTBOOL bForce)
{
//	if(!m_bMoving)
//	{
		CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(m_hFiredFrom));
		if(!pChar) return;

		//see if we have LOS to our master
		if(!bForce && TestLOSIgnoreCharacters(m_hFiredFrom, m_hObject, LTVector(0,0,0), LTTRUE))
		{
			//see if we got the juice to retrieve
			if(pChar->GetPredatorEnergy() < (uint32)m_pClassData->nNormRetrieveCost)
				//not enuf juice
				return;

			//ok lets fly home
			pChar->DecrementPredatorEnergy(m_pClassData->nNormRetrieveCost);

			//we have LOS so lets head out!
			LTVector vDiskPos, vOwnerPos;

			//get positions
			g_pLTServer->GetObjectPos(m_hObject, &vDiskPos);
			g_pLTServer->GetObjectPos(m_hFiredFrom, &vOwnerPos);

			LTVector vToOwner = vOwnerPos - vDiskPos;
			vToOwner.Norm();

			LTRotation rRot;
			g_pLTServer->AlignRotation(&rRot, &vToOwner, LTNULL);
			g_pLTServer->SetObjectRotation (m_hObject, &rRot);

			//set velocity
			vToOwner *= g_vtDiskSpeed.GetFloat();
			g_pLTServer->SetVelocity(m_hObject, &vToOwner);

			//be sure we track ourself
			ModelLT* pModelLT   = g_pLTServer->GetModelLT();
			m_hTarget = m_hFiredFrom.GetHOBJECT();
			pModelLT->GetNode(m_hTarget, "Torso_u_node", m_hTargetNode);

			//play the retrieve sound
			g_pServerSoundMgr->PlaySoundFromObject(m_hFiredFrom, "Discget");
		}
		else
		{
			//see if we got the juice to retrieve
			if(!bForce && pChar->GetPredatorEnergy() < (uint32)m_pClassData->nTeleRetrieveCost)
				//not enuf juice
				return;

			//ok lets teleport
			if(!bForce)
				pChar->DecrementPredatorEnergy(m_pClassData->nTeleRetrieveCost);

			//play the retrieve sound
			g_pServerSoundMgr->PlaySoundFromObject(m_hFiredFrom, "Discget");

			//no LOS so lets teleport
			HandlePickup();
		}
//	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredDisk::IsShooter
//
//	PURPOSE:	See if we are hitting ourself
//
// ----------------------------------------------------------------------- //

LTBOOL CPredDisk::IsShooter(HOBJECT &hObj)
{
	//get collision info
	CollisionInfo info;
	g_pLTServer->GetLastCollision(&info);

	//see if this is a hitbox
	if (IsCharacterHitBox(info.m_hObject))
	{
		//see if this is our hitbox
		CCharacterHitBox* pHitBox = LTNULL;
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(info.m_hObject);
		if (!pHitBox) return LTFALSE;;

		return (pHitBox->GetModelObject() == m_hFiredFrom);
	}

	//see if this is a character
	if (IsCharacter(info.m_hObject))
	{
		//see if this is us
		return (info.m_hObject == m_hFiredFrom);
	}

	//nope not us
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredDisk::HandlePickup
//
//	PURPOSE:	Handle picking up the disk
//
// ----------------------------------------------------------------------- //

void CPredDisk::HandlePickup()
{
	//add to inventory
	HMESSAGEWRITE hMessage = g_pServerDE->StartMessageToObject(this, m_hFiredFrom, MID_AMMO);
	g_pServerDE->WriteToMessageByte(hMessage, g_pWeaponMgr->GetAmmoPool("Discs")->nId);
	g_pServerDE->WriteToMessageFloat(hMessage, 1.0f);
	g_pLTServer->WriteToMessageByte(hMessage, LTTRUE);
	g_pServerDE->EndMessage(hMessage);

	CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(m_hFiredFrom);
	CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(pChar);

	if(pPlayer)
	{
		if(g_pGameServerShell->GetGameType().IsSinglePlayer())
			pPlayer->ChangeWeapon("Disc");
		else
			pPlayer->ChangeWeapon("Disc_MP");
	}

	//remove the disk from play
	RemoveObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredDisk::DamageTarget
//
//	PURPOSE:	Handle damaging the target
//
// ----------------------------------------------------------------------- //

void CPredDisk::DamageTarget(HOBJECT &hObj)
{
	// Be sure we are not double dipping...
	if(hObj == m_hLastVictim)
		return;

	//fill in the damage struct
	DamageStruct damage;

	damage.hDamager = m_hFiredFrom;
	damage.vDir		= GetDir();

	// Do Instant damage...

	if (GetInstantDamage() > 0.0f)
	{
		damage.eType	= GetInstantDamageType();
		damage.fDamage	= GetInstantDamage();

		damage.DoDamage(this, hObj);

		// Record our victim
		m_hLastVictim = hObj;
	}

	//re-set our tracking if we killed our target
	if(hObj == m_hTarget)
		m_hTarget = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShouldDiskStick::HandleWallImpact
//
//	PURPOSE:	Should we stick or bounce....
//
// ----------------------------------------------------------------------- //

LTBOOL CPredDisk::ShouldDiskStick(CollisionInfo &info)
{
	//see if this is a world poly
	if(info.m_hPoly == INVALID_HPOLY)
	{
		DamageTarget(info.m_hObject);
		return LTFALSE;
	}

	// see if we hit a blocker brush
	if(LT_YES == g_pLTServer->Physics()->IsWorldObject(info.m_hObject) && GetSurfaceType(info) == ST_INVISIBLE)
		return LTFALSE;

	if((g_pLTServer->GetObjectFlags(info.m_hObject) & FLAG_SOLID) && GetSurfaceType(info) == ST_INVISIBLE)
		return LTFALSE;



	//direction vector
	LTVector	vDir;

	//get my rotation
	g_pLTServer->GetVelocity(m_hObject, &vDir);
	vDir.Norm();

	LTFLOAT fAngle	= vDir.Dot(info.m_Plane.m_Normal);

	// Bounds check
	fAngle = fAngle < -1.0f ? -1.0f : fAngle;
	fAngle = fAngle > 1.0f ? 1.0f : fAngle;

	fAngle	= (LTFLOAT) acos(fAngle);

	LTFLOAT fChance = ((fAngle - MATH_HALFPI)/MATH_HALFPI*0.9f);

	if(GetRandom(0.0f, 1.0f) > fChance)
	{
		//we hit a wall so set our target to the player
		//if our primary target is dead but only after
		//we have bounced once.
		if(!m_hTarget.GetHOBJECT() && m_nBounceCount)
		{
			ModelLT* pModelLT   = g_pLTServer->GetModelLT();
			m_hTarget = m_hFiredFrom.GetHOBJECT();
			pModelLT->GetNode(m_hTarget, "Torso_u_node", m_hTargetNode);
			m_nBounceCount = 0;
		}

		//time to BOUNCE!
		return LTFALSE;
	}

	// If we get to here we are gonna stick so be sure
	// to set our target back to the shooter or the 
	// Touch Notify loop will be foobared.
	m_hTarget = m_hFiredFrom.GetHOBJECT();

	//time to STICK!
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredDisk::HandleWallImpact
//
//	PURPOSE:	Handle hitting walls or other objects
//
// ----------------------------------------------------------------------- //

void CPredDisk::HandleWallImpact(HOBJECT &hObj)
{
	//be sure this is not a character
	if(IsCharacter(hObj) || IsCharacterHitBox(hObj)) return;
	
	//get collision info
	CollisionInfo info;
	g_pLTServer->GetLastCollision(&info);
	

	if(GetSurfaceType(info) == ST_SKY)
	{
		//play the retrieve sound
		g_pServerSoundMgr->PlaySoundFromObject(m_hFiredFrom, "Discget");

		//no LOS so lets teleport
		HandlePickup();

		return;
	}

	// See if we should stick to the object we just hit...
	//	m_eLastHitSurface = GetSurfaceType(info); 
	
	if (GetSurfaceType(info) != ST_LIQUID)
	{
		// Need to set velocity to 0.0f but account for stoping vel
		// being added back in...
		
		LTVector vVel;
		g_pPhysicsLT->GetVelocity(m_hObject, &vVel);
		
		if(vVel.MagSqr() != 0.0f)
		{
			//to stick or not to stick
			if(ShouldDiskStick(info))
			{
				//play the stick sound
				LTVector vPos;
//				vVel.Norm();
//				g_pLTServer->GetObjectPos(m_hObject, &vPos);
				
//				IntersectQuery IQuery;
//				IntersectInfo IInfo;
				
//				IQuery.m_From	= vPos-(vVel*100.0f);
//				IQuery.m_To		= vPos+(vVel*100.0f);
				
//				IQuery.m_Flags = INTERSECT_HPOLY | IGNORE_NONSOLID;
				
//				if(g_pServerDE->IntersectSegment(&IQuery, &IInfo))
//				{
//					m_bStickHack	= LTTRUE;
//					m_vStickHackPos = IInfo.m_Point + IInfo.m_Plane.m_Normal;
//				}
//				else
//					MoveObjectToFloor(m_hObject);
				
				vVel.Init();
				vVel -= info.m_vStopVel;
				
				g_pServerSoundMgr->PlaySoundFromPos(vPos, "DiscstickinWall");
				
				g_pLTServer->SetVelocity(m_hObject, &vVel);
			}
			else
			{
				//set bounce velocity
				vVel += info.m_vStopVel;

				//align our disk to the new trajectory
				LTVector vTemp = vVel;
				vTemp.y = 0;
				vTemp.Norm();
				
				
				LTRotation rRot;
				g_pLTServer->AlignRotation(&rRot, &vTemp, LTNULL);
				g_pLTServer->SetObjectRotation (m_hObject, &rRot);
				
				//play the bounce sound
				LTVector vPos;
				g_pLTServer->GetObjectPos(m_hObject, &vPos);
				g_pServerSoundMgr->PlaySoundFromPos(vPos, "DiscBounce");
				g_pLTServer->SetVelocity(m_hObject, &vVel);
				
				// Okay we bounced...  Only count bounces if
				// we don't have a target
				if(!m_hTarget.GetHOBJECT())
					++m_nBounceCount;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredDisk::HandleTouch
//
//	PURPOSE:	Handle bouncing off of things
//
// ----------------------------------------------------------------------- //

void CPredDisk::HandleTouch(HOBJECT hObj)
{
	//see if we are the hObj
	if(IsShooter(hObj))
	{
		//see if we are the current
		if(m_hTarget == m_hFiredFrom)
		{
			//we either picked up the weapon or it came back to us
			HandlePickup();
		}

		return;
	}


	//see if we hit a character
	if(IsCharacter(hObj) && hObj != m_hLastVictim)
	{
		if(m_bMoving)
		{
			CCharacter* pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hObj));

			if(pChar)
			{
				CCharacterHitBox* pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(pChar->GetHitBox()));

				if(pHitBox && pHitBox->DidProjectileImpact(this))
				{
					//time to die!
					DamageTarget(hObj);

					//play the impact sound
					LTVector vPos;
					g_pLTServer->GetObjectPos(m_hObject, &vPos);
					g_pServerSoundMgr->PlaySoundFromPos(vPos, "Dischitbody");
				}
			}
		}

		return;
	}

	//see if we are hitting a corpse
	if(IsBodyProp(hObj))
		return;

	if(IsPredatorDisk(hObj))
		return;


	// see if it's something non-solid
	uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);

	if(!(dwFlags & FLAG_SOLID))
		return;

	//handle hitting a wall
	HandleWallImpact(hObj);
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredDisk::IsPredatorDisk()
//
//	PURPOSE:	Check to see if this is a predator disk....
//
// ----------------------------------------------------------------------- //

LTBOOL CPredDisk::IsPredatorDisk(HOBJECT &hObj)
{
	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;

	g_pLTServer->FindNamedObjects("PredatorDisk",objArray);

	int numObjects = objArray.NumObjects();
    if (!numObjects) return LTFALSE;

	for (int i = 0; i < numObjects; i++)
	{
		if(objArray.GetObject(i) == hObj)
			return LTTRUE;
	}
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredDisk::Pursue()
//
//	PURPOSE:	Update the grenade in pursue state...
//
// ----------------------------------------------------------------------- //

void CPredDisk::Pursue()
{
	if(m_hTarget)
	{
		//time to update our status
		LTVector vMyPos, vTargetPos;

		//get positions
		g_pLTServer->GetObjectPos(m_hObject, &vMyPos);

		ModelLT* pModelLT   = g_pLTServer->GetModelLT();
		if(m_hTargetNode != eModelNodeInvalid)
		{
			LTransform	tTransform;

			if (pModelLT->GetNodeTransform(m_hTarget, m_hTargetNode, tTransform, LTTRUE) == LT_OK)
			{
				TransformLT* pTransLT = g_pLTServer->GetTransformLT();
				pTransLT->GetPos(tTransform, vTargetPos);
			}
		}
		else
			g_pLTServer->GetObjectPos(m_hTarget, &vTargetPos);

		//get my velocity
		LTVector vVel;
		g_pPhysicsLT->GetVelocity(m_hObject, &vVel);

		//get my rotation
		LTRotation rRot;
		g_pLTServer->GetObjectRotation(m_hObject, &rRot);

		LTVector vR, vU, vF;
		g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);

		//get vector to target
		LTVector vToTarget = vTargetPos-vMyPos;

		//normalize the vectors
		vVel.Norm();
		vToTarget.Norm();
		
		//get the angle to the target
		LTFLOAT fDot = vVel.Dot(vToTarget);

		if(fDot < 0)
		{
			m_hTarget = LTNULL;
			return;
		}

		// Bounds check
		fDot = fDot < -1.0f ? -1.0f : fDot;
		fDot = fDot > 1.0f ? 1.0f : fDot;

		fDot = (LTFLOAT) acos(fDot);

		LTFLOAT fMaxTurn = MATH_DEGREES_TO_RADIANS(g_vtDiskTurnRate.GetFloat() * g_pLTServer->GetFrameTime());

		//get my desired rotation
		LTRotation rDesired, rDest;
		g_pMathLT->AlignRotation(rDesired, vToTarget, vU);

		LTFLOAT fRatio = (LTFLOAT)fabs(fMaxTurn/fDot);

		g_pMathLT->InterpolateRotation(rDest, rRot, rDesired, fRatio > 1.0f ? 1.0f : fRatio);

		g_pLTServer->SetObjectRotation(m_hObject, &rDest);

		g_pMathLT->GetRotationVectors(rDest, vR, vU, vF);

		vF *= g_vtDiskSpeed.GetFloat();
		g_pLTServer->SetVelocity(m_hObject, &vF);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredDisk::GetRandomNode()
//
//	PURPOSE:	Get a random node to target...
//
// ----------------------------------------------------------------------- //

char* CPredDisk::GetRandomNode()
{
	const uint8 n = GetRandom(0,5);

	switch (n)
	{
	default:
	case (0): return "Torso_u_node";
	case (1): return "Head_node";
	case (2): return "Left_arml_node";
	case (3): return "Right_arml_node";
	case (4): return "Left_legl_node";
	case (5): return "Right_legl_node";
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredDisk::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CPredDisk::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	CProjectile::Save(hWrite, dwSaveFlags);

	m_ActivateTime.Save(hWrite);

	*hWrite << m_bMoving;
	*hWrite << m_hTarget;
	*hWrite << m_hFiredFrom;
	*hWrite << m_hTargetNode;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredDisk::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CPredDisk::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	CProjectile::Load(hRead, dwLoadFlags);

	m_ActivateTime.Load(hRead);
	
	*hRead >> m_bMoving;
	*hRead >> m_hTarget;
	*hRead >> m_hFiredFrom;
	*hRead >> m_hTargetNode;

	m_pClassData = dynamic_cast<PREDDISKCLASSDATA*>(GetAmmoClassData());
}

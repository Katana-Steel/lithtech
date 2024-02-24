// ----------------------------------------------------------------------- //
//
// MODULE  : Explosion.cpp
//
// PURPOSE : Explosion - Definition
//
// CREATED : 11/25/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Explosion.h"
#include "cpp_server_de.h"
#include "ServerUtilities.h"
#include "WeaponFXTypes.h"
#include "ObjectMsgs.h"
#include "DamageTypes.h"
#include "SharedFXStructs.h"
#include "SFXMsgIds.h"
#include "Character.h"
#include "LTString.h"
#include "SurfaceFunctions.h"

#define MIN_RADIUS_PERCENT				0.25f

BEGIN_CLASS(Explosion)

	ADD_STRINGPROP_FLAG(ImpactFXName, "", PF_STATICLIST)
	ADD_REALPROP_FLAG(DamageRadius, 200.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(MaxDamage, 200.0f, 0)
	ADD_BOOLPROP_FLAG(RemoveWhenDone, LTTRUE, 0)

END_CLASS_DEFAULT_FLAGS_PLUGIN(Explosion, GameBase, NULL, NULL, 0, CExplosionPlugin)

LTBOOL ExplosionFilterFn(HOBJECT hObj, void *pUserData)
{
	uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);
	if (!(dwFlags & FLAG_SOLID))
	{
		return LTFALSE;
	}
	else if ( (g_pPhysicsLT->IsWorldObject(hObj) == LT_YES) || 
		      (OT_WORLDMODEL == g_pLTServer->GetObjectType(hObj)) )
	{
		// If we made it this far, test for blockers...
		if (GetSurfaceType(hObj) != ST_INVISIBLE) 
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Explosion()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Explosion::Explosion()
	: m_fDamageRadius(0.0f),
	  m_fMaxDamage(0.0f),
	  m_eDamageType(DT_UNSPECIFIED),
	  m_hFiredFrom(LTNULL),
	  m_vPos(0,0,0),
	  m_bRemoveWhenDone(LTTRUE),
	  m_nImpactFXId(FXBMGR_INVALID_ID),
	  m_eProgDamType(DT_UNSPECIFIED),
	  m_fProgDamage(0.0f),
	  m_fProgDamageDur(0.0f),
	  m_bHideFromShooter(LTFALSE)  {}

void Explosion::Setup(const EXPLOSION_CREATESTRUCT &expCS)
{
	if(!expCS.hFiredFrom) return;

	m_fDamageRadius = expCS.fDamageRadius;
	m_fMaxDamage = expCS.fMaxDamage;
	m_eDamageType = expCS.eDamageType;

	m_eProgDamType = expCS.eProgDamageType;
	m_fProgDamage = expCS.fProgDamage;
	m_fProgDamageDur = expCS.fProgDamageDuration;

	m_vPos = expCS.vPos;
//	m_hFiredFrom = expCS.hFiredFrom;
	m_bRemoveWhenDone = expCS.bRemoveWhenDone;
	m_nImpactFXId = expCS.nImpactFXId;
	m_bHideFromShooter = expCS.bHideFromShooter;

	if(m_hObject)
		g_pLTServer->SetObjectPos(m_hObject, &m_vPos);

	Start(expCS.hFiredFrom);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Start()
//
//	PURPOSE:	Start the Explosion 
//
// ----------------------------------------------------------------------- //

void Explosion::Start(HOBJECT hFiredFrom)
{
	if (!m_hObject) return;

	g_pLTServer->GetObjectPos(m_hObject, &m_vPos);

	// Do special fx (on the client) if applicable...

	if (m_nImpactFXId != FXBMGR_INVALID_ID)
	{
		LTRotation rRot;
		g_pLTServer->GetObjectRotation(m_hObject, &rRot);

		EXPLOSIONCREATESTRUCT cs;
		cs.nImpactFX	 = m_nImpactFXId;
		cs.rRot			 = rRot;
		cs.vPos			 = m_vPos;
		cs.fDamageRadius = m_fDamageRadius;
		cs.hFiredFrom	 = m_bHideFromShooter?LTNULL:hFiredFrom;

		HMESSAGEWRITE hMessage = g_pLTServer->StartInstantSpecialEffectMessage(&m_vPos);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_EXPLOSION_ID);
		cs.Write(g_pLTServer, hMessage);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
	}
	
	if(!m_bHideFromShooter)
		m_hFiredFrom = hFiredFrom;

	// Damage objects caught in the blast...

	if (m_fDamageRadius > 0.0f && m_fMaxDamage > 0.0f)
	{
		DamageObjectsInSphere();
	}


	if (m_bRemoveWhenDone)
	{
		g_pLTServer->RemoveObject(m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Explosion::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			SetNextUpdate(0.0f);
		}
		break;

		case MID_PRECREATE :
		{
			if (fData == INITIALUPDATE_WORLDFILE || fData == INITIALUPDATE_STRINGPROP)
			{
				ReadProp();
			}

			CacheFiles();
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

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 Explosion::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
 		case MID_TRIGGER:
		{
			LTString msg = hRead->ReadHString();
			if (_stricmp(msg.CStr(), "REMOVE") == 0)
			{
				g_pLTServer->RemoveObject(m_hObject);
			}
			else
			{
				Start();
			}
		}

		default : break;
	}

	return GameBase::ObjectMessageFn (hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

void Explosion::ReadProp()
{
	GenericProp genProp;

	if (g_pLTServer->GetPropGeneric("DamageRadius", &genProp) == DE_OK)
	{
		m_fDamageRadius = genProp.m_Float;
	}

	if (g_pLTServer->GetPropGeneric("MaxDamage", &genProp) == DE_OK)
	{
		m_fMaxDamage = genProp.m_Float;
	}

	if (g_pLTServer->GetPropGeneric("RemoveWhenDone", &genProp) == DE_OK)
	{
		m_bRemoveWhenDone = genProp.m_Bool;
	}

	g_pFXButeMgr->ReadImpactFXProp("ImpactFXName", m_nImpactFXId);

	// Maybe allow this to be set via a property in the future...

	m_eDamageType = DT_EXPLODE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::DamageObject()
//
//	PURPOSE:	Damage the object...
//
// ----------------------------------------------------------------------- //

void Explosion::DamageObject(HOBJECT hObj)
{
	if (!hObj) return;

#ifdef __LINUX
	HOBJECT hDamager;

	if (m_hFiredFrom)
		hDamager = m_hFiredFrom;
	else
		hDamager = m_hObject;
#else
	HOBJECT hDamager = m_hFiredFrom ? m_hFiredFrom : m_hObject;
#endif

	LTVector vObjPos;
	g_pLTServer->GetObjectPos(hObj, &vObjPos);
	
	LTVector vDir = vObjPos - m_vPos;
	LTFLOAT fDist = vDir.Mag();

	if (fDist <= m_fDamageRadius) 
	{
		// Make sure that Characters don't take damage if another object
		// is blocking them from the explosion...
	
		LTBOOL bIntersect1 = LTFALSE;
		LTBOOL bIntersect2 = LTFALSE;

		if (IsCharacter(hObj) || IsCharacterHitBox(hObj))
		{
			// To do this test, do an intersect segment both directions
			// (from the object to the explosion and from the explosion
			// to the object).  This will ensure that neither point
			// is inside a wall and that nothing is blocking the damage...

			IntersectInfo iInfo;
			IntersectQuery qInfo;

			qInfo.m_Flags	  = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
			qInfo.m_FilterFn  = ExplosionFilterFn;

			qInfo.m_From = m_vPos + vDir/fDist;
			qInfo.m_To   = vObjPos;
			
			bIntersect1 = g_pLTServer->IntersectSegment(&qInfo, &iInfo);

			qInfo.m_From = vObjPos;
			qInfo.m_To   = m_vPos + vDir/fDist;

			bIntersect2 = g_pLTServer->IntersectSegment(&qInfo, &iInfo);
		}

		if (!bIntersect1 && !bIntersect2)
		{
			//do normal damage
			DamageStruct damage;

			damage.eType	= m_eDamageType;
			damage.hDamager = hDamager;
			damage.vDir		= vDir;
			damage.fDamage	= m_fMaxDamage;

			LTFLOAT fMinRadius = m_fDamageRadius * MIN_RADIUS_PERCENT;
			LTFLOAT fRange	  = m_fDamageRadius - fMinRadius;

			// Scale damage if necessary...

			if (fDist > fMinRadius)
			{ 
				LTFLOAT fPercent = (fDist - fMinRadius) / (m_fDamageRadius - fMinRadius);
				fPercent = fPercent > 1.0f ? 1.0f : (fPercent < 0.0f ? 0.0f : fPercent);

				damage.fDamage *= (1.0f - fPercent);
			}
	
			damage.DoDamage(this, hObj, m_hObject);

			//do progressive damage
			if(m_eProgDamType != DT_UNSPECIFIED)
			{
				DamageStruct damage;

				damage.eType		= m_eProgDamType;
				damage.hDamager		= hDamager;
				damage.vDir			= vDir;
				damage.fDamage		= m_fProgDamage;
				damage.fDuration	= m_fProgDamageDur;

				damage.DoDamage(this, hObj, m_hObject);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::DamageObjectsInSphere()
//
//	PURPOSE:	Damage all the objects in our radius
//
// ----------------------------------------------------------------------- //

void Explosion::DamageObjectsInSphere()
{
	if (m_fDamageRadius <= 0.0f || m_fMaxDamage <= 0.0f) return;

	ObjectList* pList = g_pLTServer->FindObjectsTouchingSphere(&m_vPos, m_fDamageRadius);
	if (!pList) return;

	ObjectLink* pLink = pList->m_pFirstLink;
	while (pLink)
	{
		DamageObject(pLink->m_hObject);
		pLink = pLink->m_pNext;
	}

	g_pLTServer->RelinquishList(pList);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::GetBoundingBoxColor()
//
//	PURPOSE:	Get the color of the bounding box
//
// ----------------------------------------------------------------------- //
	
#ifndef _FINAL
LTVector Explosion::GetBoundingBoxColor()
{
	return LTVector(0, 0, 1);
}
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::CacheFiles()
//
//	PURPOSE:	Cache impact fx resources
//
// ----------------------------------------------------------------------- //
	
void Explosion::CacheFiles()
{
	// Only cache if necessary...

	if (m_nImpactFXId == FXBMGR_INVALID_ID) return;

	// Too late to cache files?

	if (!(g_pLTServer->GetServerFlags() & SS_CACHING)) return;

	IMPACTFX* pImpactFX = g_pFXButeMgr->GetImpactFX(m_nImpactFXId);
	if (pImpactFX)
	{
		pImpactFX->Cache(g_pFXButeMgr);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Explosion::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if(!hWrite) return;

	*hWrite << m_fDamageRadius;
	*hWrite << m_fMaxDamage;
	hWrite->WriteWord(m_eDamageType);
	*hWrite << m_hFiredFrom;
	*hWrite << m_vPos;
	*hWrite << m_bRemoveWhenDone;
	*hWrite << m_nImpactFXId;
	hWrite->WriteWord(m_eProgDamType);
	*hWrite << m_fProgDamage;
	*hWrite << m_fProgDamageDur;
	*hWrite << m_bHideFromShooter;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Explosion::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if(!hRead) return;

	*hRead >> m_fDamageRadius;
	*hRead >> m_fMaxDamage;
	m_eDamageType = DamageType(hRead->ReadWord());
	*hRead >> m_hFiredFrom;
	*hRead >> m_vPos;
	*hRead >> m_bRemoveWhenDone;
	*hRead >> m_nImpactFXId;
	m_eProgDamType = DamageType(hRead->ReadWord());
	*hRead >> m_fProgDamage;
	*hRead >> m_fProgDamageDur;
	*hRead >> m_bHideFromShooter;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

LTRESULT	CExplosionPlugin::PreHook_EditStringList(const char* szRezPath, 
												 const char* szPropName, 
												 char* const * aszStrings, 
												 uint32* pcStrings, 
												 const uint32 cMaxStrings, 
												 const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	if (_stricmp("ImpactFXName", szPropName) == 0)
	{
		m_FXButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		if (!m_FXButeMgrPlugin.PopulateImpactFXStringList(aszStrings, pcStrings, 
			 cMaxStrings, cMaxStringLength)) return LT_UNSUPPORTED;

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

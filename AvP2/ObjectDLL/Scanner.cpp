// ----------------------------------------------------------------------- //
//
// MODULE  : CScanner.cpp
//
// PURPOSE : Implementation of CScanner class
//
// CREATED : 6/7/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Scanner.h"
#include "cpp_server_de.h"
#include "ServerUtilities.h"
#include "SoundMgr.h"
#include "CharacterMgr.h"
#include "SurfaceFunctions.h"
#include "VolumeBrushTypes.h"
#include "ObjectMsgs.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CScanner
//
//	PURPOSE:	Scans for players
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(CScanner)

	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedFilename, "", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedSkin, "", PF_FILENAME)
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(MoveToFloor, DFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Translucency, 1.0f, PF_HIDDEN)

	ADD_REALPROP_FLAG(HitPoints, 1.0f, PF_GROUP1)
	ADD_REALPROP_FLAG(MaxHitPoints, 10.0f, PF_GROUP1)
	ADD_REALPROP_FLAG(Armor, 0.0f, PF_GROUP1)
	ADD_REALPROP_FLAG(MaxArmor, 0.0f, PF_GROUP1)

	ADD_REALPROP(FOV, 160.0)
	ADD_REALPROP_FLAG(VisualRange, 1000.0, PF_RADIUS)
	ADD_REALPROP(DetectInterval, 0.0)

	ADD_STRINGPROP_FLAG(SpotTarget, "", PF_OBJECTLINK)
	ADD_STRINGPROP(SpotMessage, "")

END_CLASS_DEFAULT_FLAGS(CScanner, Prop, NULL, NULL, CF_HIDDEN)

// Filter functions

DBOOL CScanner::DefaultFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj || !g_pServerDE) return DFALSE;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hObj));
	if ( pSurf && pSurf->bCanSeeThrough )
	{
		return DFALSE;
	}

	HCLASS hBody = g_pServerDE->GetClass("BodyProp");

	if (g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hObj), hBody))
	{
		return DFALSE;
	}

	return LiquidFilterFn(hObj, pUserData);
}

DBOOL CScanner::BodyFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj || !g_pServerDE) return DFALSE;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hObj));
	if ( pSurf && pSurf->bCanSeeThrough )
	{
		return DFALSE;
	}

	return LiquidFilterFn(hObj, pUserData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::CScanner()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CScanner::CScanner() : Prop()
{
	m_fFOV				= 0.0f;
	m_fVisualRange		= 1000.0f;

	m_fDetectTimer		= 0.0f;
	m_fDetectInterval	= 0.0f;

	m_vPos.Init(0, 0, 0);
	m_vInitialPitchYawRoll.Init(0, 0, 0);

	m_hstrDestroyedFilename = DNULL;
	m_hstrDestroyedSkin = DNULL;

	m_hstrSpotMessage	= DNULL;
	m_hstrSpotTarget	= DNULL;

	m_hLastDetectedEnemy = DNULL;
	m_vLastDetectedDeathPos.Init(0, 0, 0);

	// We do not want our object removed when we die

	m_damage.SetRemoveOnDeath(DFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::~CScanner()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

CScanner::~CScanner()
{
	FREE_HSTRING(m_hstrDestroyedFilename);
	FREE_HSTRING(m_hstrDestroyedSkin);
	FREE_HSTRING(m_hstrSpotMessage);
	FREE_HSTRING(m_hstrSpotTarget);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CScanner::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch (messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData; 
				ReadProp(pStruct);

				// Don't stand on the player...

				pStruct->m_Flags |= FLAG_DONTFOLLOWSTANDING;
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				g_pServerDE->GetObjectPos(m_hObject, &m_vPos);
			}
			
			CacheFiles();
		}
		break;

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (hLink)
			{
				if (hLink == m_hLastDetectedEnemy)
				{
					m_hLastDetectedEnemy = DNULL;
				}
			}
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

	return Prop::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL CScanner::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) return DFALSE;

	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("DetectInterval", &genProp) == DE_OK)
	{
		m_fDetectInterval = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("FOV", &genProp) == DE_OK)
	{
		m_fFOV = genProp.m_Float;

		// Change FOV into the value we're going to compare in the dot product 
		// for fov

		m_fFOV = (float)sin(MATH_DEGREES_TO_RADIANS(90.0f-m_fFOV/2.0f));
	}
	
	if (g_pServerDE->GetPropGeneric("VisualRange", &genProp) == DE_OK)
	{
		m_fVisualRange = genProp.m_Float;

		// All values compared against VisualRange will be squared, so square 
		// it too

		m_fVisualRange = m_fVisualRange*m_fVisualRange;	
	}

	if (g_pServerDE->GetPropGeneric("DestroyedFilename", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrDestroyedFilename = g_pServerDE->CreateString(genProp.m_String);
		}
	}

	if (g_pServerDE->GetPropGeneric("DestroyedSkin", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrDestroyedSkin = g_pServerDE->CreateString(genProp.m_String);
		}
	}

	if (g_pServerDE->GetPropGeneric("SpotMessage", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrSpotMessage = g_pServerDE->CreateString(genProp.m_String);
		}
	}

	if (g_pServerDE->GetPropGeneric("SpotTarget", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrSpotTarget = g_pServerDE->CreateString(genProp.m_String);
		}
	}

	DVector vAngles;
	if (g_pServerDE->GetPropRotationEuler("Rotation", &vAngles) == DE_OK)
	{
		m_vInitialPitchYawRoll = vAngles;
	}
	
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::UpdateDetect()
//
//	PURPOSE:	Checks to see if we can see anything
//
// ----------------------------------------------------------------------- //

DBOOL CScanner::UpdateDetect()
{
	ASSERT(0); // This code is non-functioning!

	DBOOL bRet = DFALSE;

	m_fDetectTimer += g_pLTServer->GetFrameTime();

	if (m_fDetectTimer < m_fDetectInterval)
	{
		// Too early to Detect again

		return DFALSE;
	}

	m_fDetectTimer = 0.0f;

	// If you need this code, some functionality needs to be restored to
	// CCharacterMgr.  Let me know, I can get it back in (or you can
	// find it in SS).   PLH 6-9-00
/*	CCharacter* pChar   = g_pCharacterMgr->LookForEnemy(this);
	CDeathScene* pScene = g_pCharacterMgr->LookForDeathScene(this);

	if (pChar || pScene)
	{
		if (pChar)
		{
			SetLastDetectedEnemy(pChar->m_hObject);
		}
		else if (pScene)
		{
			SetLastDetectedDeathPos(pScene->GetPosition());
		}

		SendTriggerMsgToObjects(this, m_hstrSpotTarget, m_hstrSpotMessage);
		bRet = DTRUE;
	}
*/
	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::GetScanRotation()
//
//	PURPOSE:	Get the scan rotation (sub-class may want to use something
//				other than the object's rotation)
//
// ----------------------------------------------------------------------- //

DRotation CScanner::GetScanRotation()
{
	DRotation rRot;
	ROT_INIT(rRot);

	if (m_hObject)
	{
		g_pServerDE->GetObjectRotation(m_hObject, &rRot);
	}

	return rRot;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::SetDestroyedModel()
//
//	PURPOSE:	Set our model to the destroyed version
//
// ----------------------------------------------------------------------- //

void CScanner::SetDestroyedModel()
{
	if (!m_hstrDestroyedFilename || !m_hstrDestroyedSkin) return;

	char* szDestroyedFilename = g_pServerDE->GetStringData(m_hstrDestroyedFilename);
	char* szDestroyedSkin	  = g_pServerDE->GetStringData(m_hstrDestroyedSkin);

	DRESULT hResult = g_pServerDE->SetObjectFilenames(m_hObject, szDestroyedFilename, szDestroyedSkin);
	_ASSERT(hResult == DE_OK);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::SetLastDetectedEnemy()
//
//	PURPOSE:	Set our last detected enemy
//
// ----------------------------------------------------------------------- //

void CScanner::SetLastDetectedEnemy(HOBJECT hObj)
{
	if (m_hLastDetectedEnemy)
	{
		g_pServerDE->BreakInterObjectLink(m_hObject, m_hLastDetectedEnemy);
	}

	m_hLastDetectedEnemy = hObj;

	if (m_hLastDetectedEnemy)
	{
		g_pServerDE->CreateInterObjectLink(m_hObject, m_hLastDetectedEnemy);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::CanSeeObject()
//
//	PURPOSE:	Is this object visible to us?
//
// ----------------------------------------------------------------------- //

DBOOL CScanner::CanSeeObject(ObjectFilterFn ofn, HOBJECT hObject)
{
	_ASSERT(hObject);
	if (!hObject) return DFALSE;

	DVector vPos;
	g_pServerDE->GetObjectPos(hObject, &vPos);

	DVector vDir;
	vDir = vPos - m_vPos;

	if (VEC_MAGSQR(vDir) >= m_fVisualRange)
	{
		return DFALSE;
	}

	vDir.Norm();

	DRotation rRot = GetScanRotation();

	DVector vUp, vRight, vForward;
	g_pServerDE->GetRotationVectors(&rRot, &vUp, &vRight, &vForward);

	DFLOAT fDp = vDir.Dot(vForward);

	if (fDp < m_fFOV)
	{
		return DFALSE;
	}

	// See if we can see the position in question

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, m_vPos);
	VEC_COPY(IQuery.m_To, vPos);
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = ofn;

	if (g_pServerDE->IntersectSegment(&IQuery, &IInfo))
	{
		if (IInfo.m_hObject == hObject)
		{
			return DTRUE;
		}
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::CanSeePos()
//
//	PURPOSE:	Is this position visible to us?
//
// ----------------------------------------------------------------------- //

DBOOL CScanner::CanSeePos(ObjectFilterFn ofn, const DVector& vPos)
{
	DVector vDir;
	vDir = vPos - m_vPos;

	if (VEC_MAGSQR(vDir) >= m_fVisualRange)
	{
		return DFALSE;
	}

	vDir.Norm();

	DRotation rRot = GetScanRotation();

	DVector vUp, vRight, vForward;
	g_pServerDE->GetRotationVectors(&rRot, &vUp, &vRight, &vForward);

	DFLOAT fDp = vDir.Dot(vForward);

	if (fDp < m_fFOV)
	{
		return DFALSE;
	}

	// See if we can see the position in question

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, m_vPos);
	VEC_COPY(IQuery.m_To, vPos);
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = ofn;

	if (!g_pServerDE->IntersectSegment(&IQuery, &IInfo))
	{
		return DTRUE;
	}

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CScanner::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

	*hWrite << m_fFOV;
	*hWrite << m_fVisualRange;
	*hWrite << m_fDetectInterval;
	*hWrite << m_fDetectTimer;

	*hWrite << m_vPos;
	*hWrite << m_vInitialPitchYawRoll;

	g_pServerDE->WriteToMessageHString(hWrite, m_hstrDestroyedFilename);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrDestroyedSkin);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrSpotMessage);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrSpotTarget);

	*hWrite << m_hLastDetectedEnemy;
	*hWrite << m_vLastDetectedDeathPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CScanner::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

	*hRead >> m_fFOV;
	*hRead >> m_fVisualRange;
	*hRead >> m_fDetectInterval;
	*hRead >> m_fDetectTimer;

	*hRead >> m_vPos;
	*hRead >> m_vInitialPitchYawRoll;

	m_hstrDestroyedFilename = g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrDestroyedSkin		= g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrSpotMessage		= g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrSpotTarget		= g_pServerDE->ReadFromMessageHString(hRead);

	*hRead >> m_hLastDetectedEnemy;
	*hRead >> m_vLastDetectedDeathPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::CacheFiles
//
//	PURPOSE:	Cache whatever resources this object uses
//
// ----------------------------------------------------------------------- //

void CScanner::CacheFiles()
{
	if (m_hstrDestroyedFilename)
	{
		char* pFile = g_pServerDE->GetStringData(m_hstrDestroyedFilename);
		if (pFile && pFile[0])
		{
			g_pServerDE->CacheFile(FT_MODEL, pFile);
		}
	}

	if (m_hstrDestroyedSkin)
	{
		char* pFile	= g_pServerDE->GetStringData(m_hstrDestroyedSkin);
		if (pFile && pFile[0])
		{
			g_pServerDE->CacheFile(FT_TEXTURE, pFile);
		}
	}
}

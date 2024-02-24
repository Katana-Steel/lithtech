// ----------------------------------------------------------------------- //
//
// MODULE  : DestructibleModel.cpp
//
// PURPOSE : DestructibleModel aggregate
//
// CREATED : 4/23/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>

#include "DestructibleModel.h"
#include "ClientServerShared.h"
#include "ServerUtilities.h"
#include "WorldModelDebris.h"
#include "DebrisFuncs.h"
#include "Weapons.h"
#include "Spawner.h"
#include "ObjectMsgs.h"
#include "Globals.h"
#include "Explosion.h"
#include "DebrisMgr.h"
#include "Beetle.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModelPlugin::PreHook_EditStringList()
//
//	PURPOSE:	Fill in property string list
//
// ----------------------------------------------------------------------- //
static CFXButeMgrPlugin	m_FXButeMgrPlugin;
static CDebrisPlugin		m_DebrisPlugin;
static CSurfaceMgrPlugin	m_SurfaceMgrPlugin;

LTRESULT CDestructibleModelPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	// Check for the debris string props here
	if (m_DebrisPlugin.PreHook_EditStringList(szRezPath, szPropName, 
		aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
	{
		return LT_OK;
	}

	// Otherwise, check the local string props
	if(_stricmp("ImpactFXName", szPropName) == 0)
	{
		m_FXButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		if(!m_FXButeMgrPlugin.PopulateImpactFXStringList(aszStrings, pcStrings, 
			 cMaxStrings, cMaxStringLength)) return LT_UNSUPPORTED;

		return LT_OK;
	}

	// Damage type string props
	if(_stricmp("DamageType", szPropName) == 0)
	{
		for(int i = 0; i < DT_MAX; i++)
		{
			if(cMaxStrings > (*pcStrings) + 1)
			{
				strcpy(aszStrings[(*pcStrings)++], DamageTypeToName((DamageType)i));
			}
		}

		return LT_OK;
	}

	// Surface override string props
	if(_stricmp("SurfaceOverride", szPropName) == 0)
	{
		m_SurfaceMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		if(m_SurfaceMgrPlugin.PopulateStringList(aszStrings, pcStrings,
			 cMaxStrings, cMaxStringLength))
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::CDestructibleModel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CDestructibleModel::CDestructibleModel() : CDestructible()
{
	m_bRemoveOnDeath		= LTTRUE;

	// Explosion stuff..
	m_bCreateExplosion		= LTFALSE;
	m_nImpactFXId			= 0;
	m_fDamageRadius			= 200;
	m_fMaxDamage			= 200;
	m_eDamageType			= DT_UNSPECIFIED;

	// Debris stuff...
	m_nDebrisId				= DEBRISMGR_INVALID_ID;
	m_bCreatedDebris		= LTFALSE;

	m_ltstrSpawn			= LTNULL;
	m_ltstrSurfaceOverride	= LTNULL;

	m_dwOriginalFlags		= 0;
	m_bSaveNeverDestroy		= 0;
	m_bSaveCanDamage		= 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::~CDestructibleModel()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CDestructibleModel::~CDestructibleModel()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

uint32 CDestructibleModel::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT fData)
{
	switch (messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			uint32 dwRet = CDestructible::EngineMessageFn(pObject, messageID, pData, fData);

			int nInfo = (int)fData;
			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				SetSurfaceType();
			}

			CacheFiles();
			return dwRet;
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

	return CDestructible::EngineMessageFn(pObject, messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::SetSurfaceType
//
//	PURPOSE:	Set the object's surface type...
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::SetSurfaceType(const char *szSurf)
{
	m_ltstrSurfaceOverride = szSurf;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::SetSurfaceType
//
//	PURPOSE:	Set the object's surface type...
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::SetSurfaceType()
{
	Beetle* pBeetle = dynamic_cast<Beetle*>(g_pLTServer->HandleToObject(m_hObject));
	if(pBeetle) return;

	uint32 dwUsrFlgs = g_pLTServer->GetObjectUserFlags(m_hObject);

	uint32 dwSurfUsrFlgs = 0;
	SurfaceType eSurfType = ST_UNKNOWN;

	// See if this object is a world model...

	if (g_pLTServer->GetObjectType(m_hObject) == OT_WORLDMODEL)
	{
        LTBOOL bDoBruteForce = LTTRUE;

		// See if we have a surface override...
		if (m_ltstrSurfaceOverride)
		{
			char* pSurfName = g_pLTServer->GetStringData(m_ltstrSurfaceOverride);
			if (pSurfName)
			{
				SURFACE* pSurf = g_pSurfaceMgr->GetSurface(pSurfName);
				if (pSurf && pSurf->eType != ST_UNKNOWN)
				{
					eSurfType = pSurf->eType;
					bDoBruteForce = LTFALSE;
				}
			}
		}


		// Determine our surface...the hard way...

		if (bDoBruteForce)
		{
			IntersectQuery qInfo;
			IntersectInfo iInfo;

			LTVector vPos, vDims, vF, vU, vR;
			LTRotation rRot;

			g_pLTServer->GetObjectPos(m_hObject, &vPos);
			g_pLTServer->GetObjectRotation(m_hObject, &rRot);
			g_pLTServer->GetRotationVectors(&rRot, &vU, &vR, &vF);
			g_pLTServer->GetObjectDims(m_hObject, &vDims);

			LTFLOAT fMaxDims = vDims.x;
			fMaxDims = Max(fMaxDims, vDims.y);
			fMaxDims = Max(fMaxDims, vDims.z);

			qInfo.m_From = vPos + (vF * (fMaxDims + 1));
			qInfo.m_To   = vPos;

			qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

			SurfaceType eType = ST_UNKNOWN;

			if (g_pLTServer->IntersectSegment(&qInfo, &iInfo))
			{
				if (iInfo.m_hObject == m_hObject)
				{
					eSurfType = GetSurfaceType(iInfo);
				}
			}
		}
	}
	else
	{
		if (m_ltstrSurfaceOverride)
		{
			char* pSurfName = g_pLTServer->GetStringData(m_ltstrSurfaceOverride);
			if (pSurfName)
			{
				SURFACE* pSurf = g_pSurfaceMgr->GetSurface(pSurfName);
				if (pSurf && pSurf->eType != ST_UNKNOWN)
				{
					eSurfType = pSurf->eType;
				}
			}
		}
		else
		{
			DEBRIS* pDebris = g_pDebrisMgr->GetDebris(m_nDebrisId);
			if (pDebris)
			{
				eSurfType = pDebris->eSurfaceType;
			}
		}
	}

	dwSurfUsrFlgs = SurfaceToUserFlag(eSurfType);
	g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlgs | dwSurfUsrFlgs);

    m_dwOriginalFlags = g_pLTServer->GetObjectFlags(m_hObject);
	m_bSaveCanDamage = GetCanDamage();
	m_bSaveNeverDestroy = GetNeverDestroy();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CDestructibleModel::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	uint32 dwRet = CDestructible::ObjectMessageFn(pObject, hSender, messageID, hRead);

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			if (IsDead() && !m_bCreatedDebris)
			{
                ILTServer* pServerDE = BaseClass::GetServerDE();
				if (!pServerDE) break;

				SpawnItem();
				CreateDebris();
				CreateWorldModelDebris();


				// NOTE!! This MUST be above the create explosion... otherwise it'll get back in
				// this code and recursively call it until the stack overflows
				m_bCreatedDebris = LTTRUE;


				if (m_bCreateExplosion)
				{
					DoExplosion(GetLastDamager());
				}

				if (m_bRemoveOnDeath)
				{
					pServerDE->RemoveObject(m_hObject);
				}
			}
		}
		break;

		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pLTServer->ReadFromMessageHString(hRead);

			char* pMsg = g_pLTServer->GetStringData(hMsg);
			if (!pMsg) break;

			ConParse parse;
			parse.Init(pMsg);

			while (g_pLTServer->Common()->Parse(&parse) == LT_OK)
			{
				if (parse.m_nArgs > 0 && parse.m_Args[0])
				{
					if (!IsDead())
					{
						if (_stricmp(parse.m_Args[0], "FIRE") == 0)
						{
							char* pTargetName = parse.m_nArgs > 1 ? parse.m_Args[1] : LTNULL;
							DoExplosion(hSender);
						}
						else if (_stricmp(parse.m_Args[0], "HIDDEN") == 0)
						{
							uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);

							if (parse.m_nArgs > 1 && parse.m_Args[1])
							{
								if ((_stricmp(parse.m_Args[1], "1") == 0) ||
									(_stricmp(parse.m_Args[1], "TRUE") == 0))
								{
									dwFlags = 0;

									m_bSaveCanDamage = GetCanDamage();
									m_bSaveNeverDestroy = GetNeverDestroy();

									SetCanDamage(LTFALSE);
									SetNeverDestroy(LTTRUE);
								}
								else
								{
									if ((_stricmp(parse.m_Args[1], "0") == 0) ||
										(_stricmp(parse.m_Args[1], "FALSE") == 0))
									{
										dwFlags = m_dwOriginalFlags;
										dwFlags |= FLAG_VISIBLE;

										SetCanDamage(m_bSaveCanDamage);
										SetNeverDestroy(m_bSaveNeverDestroy);
									}
								}

								g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
							}
						}
					}
				}
			}

			g_pLTServer->FreeString(hMsg);
		}
		break;

		default : break;
	}

	return dwRet;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::ReadProp()
//
//	PURPOSE:	Reads CDestructibleModel properties
//
// --------------------------------------------------------------------------- //

LTBOOL CDestructibleModel::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return LTFALSE;

	GenericProp genProp;

	// ----------------------------------------------------------------------- //
	// Setup the explosion properties

	if (g_pLTServer->GetPropGeneric("CreateExplosion", &genProp) == LT_OK)
	{
		m_bCreateExplosion = genProp.m_Bool;
	}

	g_pFXButeMgr->ReadImpactFXProp("ImpactFXName", m_nImpactFXId);

	if (g_pLTServer->GetPropGeneric("DamageRadius", &genProp) == LT_OK)
	{
		m_fDamageRadius = genProp.m_Float;
	}

	if (g_pLTServer->GetPropGeneric("MaxDamage", &genProp) == LT_OK)
	{
		m_fMaxDamage = genProp.m_Float;
	}

	if (g_pLTServer->GetPropGeneric("DamageType", &genProp) == LT_OK)
	{
		if(genProp.m_String[0])
			m_eDamageType = DamageTypeFromName(genProp.m_String);
	}


    if (g_pLTServer->GetPropGeneric("Spawn", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			 m_ltstrSpawn = genProp.m_String;
		}
	}

    if (g_pLTServer->GetPropGeneric("SurfaceOverride", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			 m_ltstrSurfaceOverride = genProp.m_String;
		}
	}

	// ----------------------------------------------------------------------- //
	// Setup the properties for the debris
	GetDebrisProperties(m_nDebrisId);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::CreateWorldModelDebris()
//
//	PURPOSE:	Create world model debris...
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::CreateWorldModelDebris()
{
    ILTServer* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	char* pName = pServerDE->GetObjectName(m_hObject);
	if (!pName || !pName[0]) return;


	// Find all the debris objects...

	int nNum = 0;

	char strKey[128]; memset(strKey, 0, 128);
	char strNum[18];  memset(strNum, 0, 18);

	HCLASS hWMDebris = pServerDE->GetClass("WorldModelDebris");

	while (1)
	{
		// Create the keyname string...

		sprintf(strKey, "%sDebris%d", pName, nNum);

		// Find any debris with that name...

		ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
		pServerDE->FindNamedObjects(strKey, objArray);

		int numObjects = objArray.NumObjects();



		if (!numObjects) return;

		for (int i = 0; i < numObjects; i++)
		{
			HOBJECT hObject = objArray.GetObject(i);

			if (pServerDE->IsKindOf(pServerDE->GetObjectClass(hObject), hWMDebris))
			{
				WorldModelDebris* pDebris = (WorldModelDebris*)pServerDE->HandleToObject(hObject);
				if (!pDebris) break;

				LTVector vVel, vRotPeriods;
				vVel.Init(GetRandom(-200.0f, 200.0f),
					GetRandom(100.0f, 300.0f), GetRandom(-200.0f, 200.0f));


				vRotPeriods.Init(GetRandom(-1.0f, 1.0f),
					GetRandom(-1.0f, 1.0f), GetRandom(-1.0f, 1.0f));

				pDebris->Start(&vRotPeriods, &vVel);
			}
		}

		// Increment the counter...

		nNum++;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::CreateDebris()
//
//	PURPOSE:	Create debris...
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::CreateDebris()
{
	DEBRIS* pDebris = g_pDebrisMgr->GetDebris(m_nDebrisId);
	if (pDebris)
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		LTVector vDir = GetDeathDir();
		::CreatePropDebris(vPos, vDir, m_nDebrisId);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::DoExplosion()
//
//	PURPOSE:	Handle doing explosion
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::DoExplosion(HOBJECT hDamager)
{
	if(!m_hObject) return;

	if(!hDamager)
		hDamager = m_hObject;

	// Setup an object create structure
	ObjectCreateStruct ocs;
	ocs.Clear();
	ocs.m_ObjectType = OT_NORMAL;

#ifdef _DEBUG
	sprintf(ocs.m_Name,  "%s-Explosion", g_pLTServer->GetObjectName(m_hObject) );
#endif

	// Create the explosion object
	HCLASS hClass = g_pLTServer->GetClass("Explosion");
	Explosion *pExpObj = (Explosion*)g_pLTServer->CreateObject(hClass, &ocs);

	// If the explosion object got created ok... call the setup function to make it explode
	if(pExpObj)
	{
		// Setup an explosion create structure
		EXPLOSION_CREATESTRUCT expCS;

		expCS.hFiredFrom = hDamager;
		expCS.nImpactFXId = m_nImpactFXId;
		expCS.fDamageRadius = m_fDamageRadius;
		expCS.fMaxDamage = m_fMaxDamage;
		expCS.eDamageType = m_eDamageType;
		g_pLTServer->GetObjectPos(m_hObject, &expCS.vPos);
		expCS.bRemoveWhenDone = LTTRUE;
		expCS.bHideFromShooter = LTFALSE;

		pExpObj->Setup(expCS);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::SpawnItem()
//
//	PURPOSE:	Spawn an item
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::SpawnItem()
{
	if (m_ltstrSpawn.IsNull()) return;

	LTVector vPos;
	LTRotation rRot;
	char szSpawn[256];

	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	strncpy(szSpawn, m_ltstrSpawn.CStr(), ARRAY_LEN(szSpawn));
	szSpawn[255] = '\0';
	SpawnObject(szSpawn, vPos, rRot);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::CacheFiles
//
//	PURPOSE:	Caches files used by object.
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::CacheFiles()
{
	// Don't cache if the world already loaded...

	if (!(g_pLTServer->GetServerFlags() & SS_CACHING)) return;

	DEBRIS* pDebris = g_pDebrisMgr->GetDebris(m_nDebrisId);
	if (pDebris)
	{
		pDebris->Cache(g_pDebrisMgr);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!g_pLTServer || !hWrite) return;

	*hWrite << m_bRemoveOnDeath;

	*hWrite << m_bCreateExplosion;
	*hWrite << m_nImpactFXId;
	*hWrite << m_fDamageRadius;
	*hWrite << m_fMaxDamage;
	hWrite->WriteDWord(m_eDamageType);

	*hWrite << m_ltstrSpawn;
	*hWrite << m_ltstrSurfaceOverride;

	*hWrite << m_bCreatedDebris;
	*hWrite << m_nDebrisId;

	*hWrite << m_dwOriginalFlags;
	*hWrite << m_bSaveCanDamage;
	*hWrite << m_bSaveNeverDestroy;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!g_pLTServer || !hRead) return;

	*hRead >> m_bRemoveOnDeath;

	*hRead >> m_bCreateExplosion;
	*hRead >> m_nImpactFXId;
	*hRead >> m_fDamageRadius;
	*hRead >> m_fMaxDamage;
	m_eDamageType = DamageType(hRead->ReadDWord());

	*hRead >> m_ltstrSpawn;
	*hRead >> m_ltstrSurfaceOverride;

	*hRead >> m_bCreatedDebris;
	*hRead >> m_nDebrisId;

	*hRead >> m_dwOriginalFlags;
	*hRead >> m_bSaveCanDamage;
	*hRead >> m_bSaveNeverDestroy;
}

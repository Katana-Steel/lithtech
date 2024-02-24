// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisMgr.cpp
//
// PURPOSE : DebrisMgr - Implementation
//
// CREATED : 3/17/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DebrisMgr.h"
#include "CommonUtilities.h"
#include "FXButeMgr.h"

// ----------------------------------------------------------------------- //

#define DEBRISMGR_DEBRIS_TAG					"Debris"

#define DEBRISMGR_DEBRIS_NAME					"Name"
#define DEBRISMGR_DEBRIS_SURFACEID				"SurfaceId"

#define DEBRISMGR_DEBRIS_MODEL					"Model%d"
#define DEBRISMGR_DEBRIS_SKIN					"Skin%d"
#define DEBRISMGR_DEBRIS_FLAGS					"Flags%d"

#define DEBRISMGR_DEBRIS_MINVEL					"MinVel"
#define DEBRISMGR_DEBRIS_MAXVEL					"MaxVel"
#define DEBRISMGR_DEBRIS_MINDOFFSET				"MinDebrisOffset"
#define DEBRISMGR_DEBRIS_MAXDOFFSET				"MaxDebrisOffset"
#define DEBRISMGR_DEBRIS_MINSCALE				"MinScale"
#define DEBRISMGR_DEBRIS_MAXSCALE				"MaxScale"
#define DEBRISMGR_DEBRIS_MINLIFETIME			"MinLifetime"
#define DEBRISMGR_DEBRIS_MAXLIFETIME			"MaxLifetime"
#define DEBRISMGR_DEBRIS_FADETIME				"Fadetime"
#define DEBRISMGR_DEBRIS_NUMBER					"Number"
#define DEBRISMGR_DEBRIS_ROTATE					"Rotate"
#define DEBRISMGR_DEBRIS_MINBOUNCE				"MinBounce"
#define DEBRISMGR_DEBRIS_MAXBOUNCE				"MaxBounce"
#define DEBRISMGR_DEBRIS_GRAVITYSCALE			"GravityScale"
#define DEBRISMGR_DEBRIS_ALPHA					"Alpha"

#define DEBRISMGR_DEBRIS_BOUNCESND				"BounceSnd%d"
#define DEBRISMGR_DEBRIS_EXPLODESND				"ExplodeSnd%d"

#define DEBRISMGR_DEBRIS_PSHOWERFX				"PShowerFX"

// ----------------------------------------------------------------------- //

#define DEBRISMGR_MULTI_DEBRIS_TAG				"MultiDebris"

#define DEBRISMGR_MULTI_DEBRIS_NAME				"Name"
#define DEBRISMGR_MULTI_DEBRIS_EFFECT			"DebrisName%d"

// ----------------------------------------------------------------------- //

static char s_aTagName[30];
static char s_aAttName[100];

// Global pointer to surface mgr...

CDebrisMgr* g_pDebrisMgr = LTNULL;

// ----------------------------------------------------------------------- //

#ifndef _CLIENTBUILD

// Plugin statics

CDebrisMgr CDebrisMgrPlugin::sm_DebrisMgr;

#else

#include "ParticleTrailFX.h"
#include "GameClientShell.h"

#endif // _CLIENTBUILD

static CAVector ToCAVector(const LTVector & vec)
{
	return CAVector(vec.x,vec.y,vec.z);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::CDebrisMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CDebrisMgr::CDebrisMgr()
{
    m_DebrisList.Init(LTTRUE);
    m_MultiDebrisList.Init(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::~CDebrisMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CDebrisMgr::~CDebrisMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (g_pDebrisMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;

	g_pDebrisMgr = this;


	// Read in the properties for each debis record...

	int nNum = 0;
	sprintf(s_aTagName, "%s%d", DEBRISMGR_DEBRIS_TAG, nNum);

	while(m_buteMgr.Exist(s_aTagName))
	{
		DEBRIS* pDebris = new DEBRIS;

		if (pDebris && pDebris->Init(m_buteMgr, s_aTagName))
		{
			pDebris->nId = nNum;
			m_DebrisList.AddTail(pDebris);
		}
		else
		{
			delete pDebris;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", DEBRISMGR_DEBRIS_TAG, nNum);
	}

	// No read in the multi-debris stuff...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", DEBRISMGR_MULTI_DEBRIS_TAG, nNum);

	while(m_buteMgr.Exist(s_aTagName))
	{
		MULTI_DEBRIS* pMultiDebris = new MULTI_DEBRIS;

		if (pMultiDebris && pMultiDebris->Init(m_buteMgr, s_aTagName))
		{
			pMultiDebris->nId = nNum;
			m_MultiDebrisList.AddTail(pMultiDebris);
		}
		else
		{
			delete pMultiDebris;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", DEBRISMGR_MULTI_DEBRIS_TAG, nNum);
	}



    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::GetDebris
//
//	PURPOSE:	Get the specified debris record
//
// ----------------------------------------------------------------------- //

DEBRIS* CDebrisMgr::GetDebris(uint8 nId)
{
    DEBRIS** pCur  = LTNULL;

	pCur = m_DebrisList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nId)
		{
			return *pCur;
		}

		pCur = m_DebrisList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::GetMultiDebris
//
//	PURPOSE:	Get the specified multi-debris record
//
// ----------------------------------------------------------------------- //

MULTI_DEBRIS* CDebrisMgr::GetMultiDebris(uint8 nId)
{
    MULTI_DEBRIS** pCur  = LTNULL;

	pCur = m_MultiDebrisList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nId)
		{
			return *pCur;
		}

		pCur = m_MultiDebrisList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::GetDebris
//
//	PURPOSE:	Get the specified debris record
//
// ----------------------------------------------------------------------- //

DEBRIS* CDebrisMgr::GetDebris(char* pName)
{
    if (!pName) return LTNULL;

    DEBRIS** pCur  = LTNULL;

	pCur = m_DebrisList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_DebrisList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::GetMultiDebris
//
//	PURPOSE:	Get the specified debris record
//
// ----------------------------------------------------------------------- //

MULTI_DEBRIS* CDebrisMgr::GetMultiDebris(char* pName)
{
    if (!pName) return LTNULL;

    MULTI_DEBRIS** pCur  = LTNULL;

	pCur = m_MultiDebrisList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_MultiDebrisList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CDebrisMgr::Term()
{
    g_pDebrisMgr = LTNULL;

	m_DebrisList.Clear();
	m_MultiDebrisList.Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::CacheAll
//
//	PURPOSE:	Cache all the debris related resources
//
// ----------------------------------------------------------------------- //

void CDebrisMgr::CacheAll()
{
#ifndef _CLIENTBUILD  // Server-side only

	// Cache all the debris data...

    DEBRIS** pCurDebris  = LTNULL;
	pCurDebris = m_DebrisList.GetItem(TLIT_FIRST);

	while (pCurDebris)
	{
		if (*pCurDebris)
		{
			(*pCurDebris)->Cache(this);
		}

		pCurDebris = m_DebrisList.GetItem(TLIT_NEXT);
	}

#endif
}


#ifdef _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::CreateDebris
//
//	PURPOSE:	Create the specified debris object
//
// ----------------------------------------------------------------------- //

void CDebrisMgr::CreateDebris(uint8 nDebrisId, HOBJECT *hObjects, uint8 nNumObjects, LTVector vPos, uint16* pFlags)
{
	DEBRIS* pDebris = GetDebris(nDebrisId);
	if(!pDebris || pDebris->nModels < 1 || nNumObjects < 1 || !hObjects) return;

	// A list of how many were created for each model
	_ASSERT(pDebris->nModels <= 32);
	uint8 pCreateAmount[32];
	memset(pCreateAmount, 0, 32);

	DEBRIS_MODEL *pModel = LTNULL;
	int i, j;

	// Go through and see if any models HAVE to be created
	for(i = 0; i < pDebris->nModels; i++)
	{
		pModel = &pDebris->pModels[i];

		if(pModel->nFlags & DEBRISMGR_FLAG_ALWAYSCREATE)
		{
			// Don't force one here or it will get missed
			// in the debrisFX update cycle...
			if (nNumObjects > 0)
			{
				pCreateAmount[i]++;
				nNumObjects--;
			}
			else
			{
				_ASSERT(LTFALSE);
				break;
			}
		}
	}

	_ASSERT(nNumObjects >= 0);

	// Now go through and fill in the reset of the create amounts if needed
	while(nNumObjects > 0)
	{
		int nModel = GetRandom(0, pDebris->nModels - 1);
		pModel = &pDebris->pModels[nModel];

		if(pModel->nFlags & DEBRISMGR_FLAG_CREATEONLYONE)
		{
			if(pCreateAmount[nModel] < 1)
			{
				pCreateAmount[nModel]++;
				nNumObjects--;
			}
		}
		else
		{
			pCreateAmount[nModel]++;
			nNumObjects--;
		}
	}


	// Now go through and create all the proper objects
	nNumObjects = 0;

	for(i = 0; i < pDebris->nModels; i++)
	{
		for(j = 0; j < pCreateAmount[i]; j++)
		{
			hObjects[nNumObjects] = CreateDebrisObject(nDebrisId, i, vPos);
			if(pFlags)
			{
				DEBRIS* pDebris = GetDebris(nDebrisId);
				if(pDebris)
					pFlags[nNumObjects] = pDebris->pModels[i].nFlags;
			}
			nNumObjects++;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::CreateDebrisObject
//
//	PURPOSE:	Create the specified debris object
//
// ----------------------------------------------------------------------- //

HOBJECT CDebrisMgr::CreateDebrisObject(uint8 nDebrisId, uint8 nModel, LTVector vPos)
{
	DEBRIS* pDebris = GetDebris(nDebrisId);
	if(!pDebris || pDebris->nModels < 1) return LTNULL;
	if(nModel < 0 || nModel >= pDebris->nModels) return LTNULL;


	// Get the model properties
	char *pFilename = pDebris->pModels[nModel].szModel;
	char *pSkin = pDebris->pModels[nModel].szSkin;
	uint16 nFlags = pDebris->pModels[nModel].nFlags;

	if(!pFilename) return LTNULL;


	// Create the debris model
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_MODEL;
	SAFE_STRCPY(createStruct.m_Filename, pFilename);
	SAFE_STRCPY(createStruct.m_SkinName, pSkin);
	createStruct.m_Flags = FLAG_VISIBLE;
	createStruct.m_Pos = vPos;

	HOBJECT hObj = g_pLTClient->CreateObject(&createStruct);
	if(!hObj) return LTNULL;


	// Scale the object
	LTVector vScale(1.0f, 1.0f, 1.0f);

	if(!(nFlags & DEBRISMGR_FLAG_DONTSCALE))
		vScale *= GetRandom(pDebris->fMinScale, pDebris->fMaxScale);

	g_pLTClient->SetObjectScale(hObj, &vScale);


	// Set the object alpha
	LTFLOAT r, g, b, a;
	g_pLTClient->GetObjectColor(hObj, &r, &g, &b, &a);
	g_pLTClient->SetObjectColor(hObj, r, g, b, pDebris->fAlpha);


	// Add a blood trail if so desired
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();

	PARTICLETRAILFX *pTrail = LTNULL;
	PARTICLETRAILFX *pUWTrail = LTNULL;

	if(nFlags & DEBRISMGR_FLAG_HUMAN_PTRAIL)
	{
		pTrail = g_pFXButeMgr->GetParticleTrailFX("Human_Blood_Trail");
		pUWTrail = g_pFXButeMgr->GetParticleTrailFX("Human_Blood_Trail");
	}
	else if(nFlags & DEBRISMGR_FLAG_PRED_PTRAIL)
	{
		pTrail = g_pFXButeMgr->GetParticleTrailFX("Predator_Blood_Trail");
		pUWTrail = g_pFXButeMgr->GetParticleTrailFX("Predator_Blood_Trail");
	}
	else if(nFlags & DEBRISMGR_FLAG_ALIEN_TRAIL)
	{
		pTrail = g_pFXButeMgr->GetParticleTrailFX("Alien_Blood_Trail");
		pUWTrail = g_pFXButeMgr->GetParticleTrailFX("Alien_Blood_Trail");
	}
	else if(nFlags & DEBRISMGR_FLAG_SYNTH_TRAIL)
	{
		pTrail = g_pFXButeMgr->GetParticleTrailFX("Synth_Blood_Trail");
		pUWTrail = g_pFXButeMgr->GetParticleTrailFX("Synth_Blood_Trail");
	}

	if(psfxMgr && pTrail && pUWTrail)
	{
		// Create this particle trail segment
		PTCREATESTRUCT pt;
		pt.hServerObj	= hObj;
		pt.nType		= pTrail ? pTrail->nId : -1;
		pt.nUWType		= pUWTrail ? pUWTrail->nId : -1;

		psfxMgr->CreateSFX(SFX_PARTICLETRAIL_ID, &pt);
	}

	return hObj;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::GetExplodeSound
//
//	PURPOSE:	Get an explode sound
//
// ----------------------------------------------------------------------- //

char* CDebrisMgr::GetExplodeSound(uint8 nDebrisId)
{
	DEBRIS* pDebris = GetDebris(nDebrisId);
    if(!pDebris || pDebris->nExplodeSnds < 1) return LTNULL;

	int nIndex = GetRandom(0, pDebris->nExplodeSnds - 1);
	return pDebris->szExplodeSnds[nIndex].szString;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::GetBounceSound
//
//	PURPOSE:	Get a bounce sound
//
// ----------------------------------------------------------------------- //

char* CDebrisMgr::GetBounceSound(uint8 nDebrisId)
{
	DEBRIS* pDebris = GetDebris(nDebrisId);
    if(!pDebris || pDebris->nBounceSnds < 1) return LTNULL;

	int nIndex = GetRandom(0, pDebris->nBounceSnds - 1);
	return pDebris->szBounceSnds[nIndex].szString;
}


#endif  // _CLIENTBUILD


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DEBRIS_MODEL::DEBRIS_MODEL
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DEBRIS_MODEL::DEBRIS_MODEL()
{
	szModel[0]		= '\0';
	szSkin[0]		= '\0';
	nFlags			= 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DEBRIS_STRING::DEBRIS_STRING
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DEBRIS_STRING::DEBRIS_STRING()
{
	szString[0]		= '\0';
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DEBRIS::DEBRIS
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DEBRIS::DEBRIS()
{
	nId				= 0;

	szName[0]		= '\0';
	eSurfaceType	= ST_UNKNOWN;

	nModels			= 0;
	pModels			= LTNULL;

	vMinVel.Init();
	vMaxVel.Init();
	vMinDOffset.Init();
	vMaxDOffset.Init();
	fMinScale		= 1.0f;
	fMaxScale		= 1.0f;
	fMinLifetime	= 0.0f;
	fMaxLifetime	= 0.0f;
	fFadetime		= 0.0f;
	nNumber			= 0;
    bRotate         = LTFALSE;
	nMinBounce		= 0;
	nMaxBounce		= 0;
	fGravityScale	= 1.0f;
	fAlpha			= 1.0f;

	nBounceSnds		= 0;
	szBounceSnds	= LTNULL;

	nExplodeSnds	= 0;
	szExplodeSnds	= LTNULL;

	szPShowerFX[0]	= '\0';
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DEBRIS::~DEBRIS
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

DEBRIS::~DEBRIS()
{
	if(pModels)
	{
		delete[] pModels;
		pModels = LTNULL;
	}

	if(szBounceSnds)
	{
		delete[] szBounceSnds;
		szBounceSnds = LTNULL;
	}

	if(szExplodeSnds)
	{
		delete[] szExplodeSnds;
		szExplodeSnds = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DEBRIS::Init
//
//	PURPOSE:	Build the surface struct
//
// ----------------------------------------------------------------------- //

LTBOOL DEBRIS::Init(CButeMgr & buteMgr, char* aTagName)
{
    if(!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, DEBRISMGR_DEBRIS_NAME);
	if(!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	eSurfaceType = (SurfaceType) buteMgr.GetInt(aTagName, DEBRISMGR_DEBRIS_SURFACEID);

	// Count the number of models
	nModels = 0;
	sprintf(s_aAttName, DEBRISMGR_DEBRIS_MODEL, nModels);

	while(buteMgr.Exist(aTagName, s_aAttName))
	{
		nModels++;
		sprintf(s_aAttName, DEBRISMGR_DEBRIS_MODEL, nModels);
	}

	// Get the data for each model
	if(nModels)
	{
		pModels = new DEBRIS_MODEL[nModels];

		for(int i = 0; i < nModels; i++)
		{
			sprintf(s_aAttName, DEBRISMGR_DEBRIS_MODEL, i);
			str = buteMgr.GetString(aTagName, s_aAttName,CString(""));

			if(!str.IsEmpty())
				strncpy(pModels[i].szModel, (char*)(LPCSTR)str, ARRAY_LEN(pModels[i].szModel));

			sprintf(s_aAttName, DEBRISMGR_DEBRIS_SKIN, i);
			str = buteMgr.GetString(aTagName, s_aAttName,CString(""));

			if(!str.IsEmpty())
				strncpy(pModels[i].szSkin, (char*)(LPCSTR)str, ARRAY_LEN(pModels[i].szSkin));

			sprintf(s_aAttName, DEBRISMGR_DEBRIS_FLAGS, i);
			pModels[i].nFlags = (uint8)buteMgr.GetInt(aTagName, s_aAttName);
		}
	}

	// Get the more general debris information
	CAVector vTemp;
	
	vTemp = ToCAVector(vMinVel);
	vMinVel		 = buteMgr.GetVector(aTagName, DEBRISMGR_DEBRIS_MINVEL,vTemp);
	
	vTemp = ToCAVector(vMaxVel);
	vMaxVel		 = buteMgr.GetVector(aTagName, DEBRISMGR_DEBRIS_MAXVEL,vTemp);

	vTemp = ToCAVector(vMinDOffset);
	vMinDOffset	 = buteMgr.GetVector(aTagName, DEBRISMGR_DEBRIS_MINDOFFSET,vTemp);

	vTemp = ToCAVector(vMaxDOffset);
	vMaxDOffset	 = buteMgr.GetVector(aTagName, DEBRISMGR_DEBRIS_MAXDOFFSET,vTemp);

	fMinScale	 = (float) buteMgr.GetDouble(aTagName, DEBRISMGR_DEBRIS_MINSCALE,fMinScale);
	fMaxScale	 = (float) buteMgr.GetDouble(aTagName, DEBRISMGR_DEBRIS_MAXSCALE,fMaxScale);
	fMinLifetime = (float) buteMgr.GetDouble(aTagName, DEBRISMGR_DEBRIS_MINLIFETIME,fMinLifetime);
	fMaxLifetime = (float) buteMgr.GetDouble(aTagName, DEBRISMGR_DEBRIS_MAXLIFETIME,fMaxLifetime);
	fFadetime	 = (float) buteMgr.GetDouble(aTagName, DEBRISMGR_DEBRIS_FADETIME,fFadetime);
	nNumber		 = buteMgr.GetInt(aTagName, DEBRISMGR_DEBRIS_NUMBER,nNumber);
    bRotate      = (LTBOOL) buteMgr.GetInt(aTagName, DEBRISMGR_DEBRIS_ROTATE,bRotate);
	nMinBounce	 = buteMgr.GetInt(aTagName, DEBRISMGR_DEBRIS_MINBOUNCE,nMinBounce);
	nMaxBounce	 = buteMgr.GetInt(aTagName, DEBRISMGR_DEBRIS_MAXBOUNCE,nMaxBounce);
	fGravityScale = (float) buteMgr.GetDouble(aTagName, DEBRISMGR_DEBRIS_GRAVITYSCALE,fGravityScale);
	fAlpha		  = (float) buteMgr.GetDouble(aTagName, DEBRISMGR_DEBRIS_ALPHA,fAlpha);


	// Count the number of bounce sounds
	nBounceSnds = 0;
	sprintf(s_aAttName, DEBRISMGR_DEBRIS_BOUNCESND, nBounceSnds);

	while(buteMgr.Exist(s_aTagName, s_aAttName))
	{
		nBounceSnds++;
		sprintf(s_aAttName, DEBRISMGR_DEBRIS_BOUNCESND, nBounceSnds);
	}

	// Get the data for each bounce sound
	if(nBounceSnds)
	{
		szBounceSnds = new DEBRIS_STRING[nBounceSnds];

		for(int i = 0; i < nBounceSnds; i++)
		{
			sprintf(s_aAttName, DEBRISMGR_DEBRIS_BOUNCESND, i);
			str = buteMgr.GetString(aTagName, s_aAttName,CString(""));

			if(!str.IsEmpty())
				strncpy(szBounceSnds[i].szString, (char*)(LPCSTR)str, ARRAY_LEN(szBounceSnds[i].szString));
		}
	}


	// Count the number of explode sounds
	nExplodeSnds = 0;
	sprintf(s_aAttName, DEBRISMGR_DEBRIS_EXPLODESND, nExplodeSnds);

	while(buteMgr.Exist(s_aTagName, s_aAttName))
	{
		nExplodeSnds++;
		sprintf(s_aAttName, DEBRISMGR_DEBRIS_EXPLODESND, nExplodeSnds);
	}

	// Get the data for each explode sound
	if(nExplodeSnds)
	{
		szExplodeSnds = new DEBRIS_STRING[nExplodeSnds];

		for(int i = 0; i < nExplodeSnds; i++)
		{
			sprintf(s_aAttName, DEBRISMGR_DEBRIS_EXPLODESND, i);
			str = buteMgr.GetString(aTagName, s_aAttName,CString(""));

			if(!str.IsEmpty())
				strncpy(szExplodeSnds[i].szString, (char*)(LPCSTR)str, ARRAY_LEN(szExplodeSnds[i].szString));
		}
	}

	// Get the explode impact FX
	str = buteMgr.GetString(aTagName, DEBRISMGR_DEBRIS_PSHOWERFX,CString(""));
	if(!str.IsEmpty())
	{
		strncpy(szPShowerFX, (char*)(LPCSTR)str, ARRAY_LEN(szPShowerFX));
	}


    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DEBRIS::Cache
//
//	PURPOSE:	Cache all the resources associated with the surface
//
// ----------------------------------------------------------------------- //

void DEBRIS::Cache(CDebrisMgr* pDebrisMgr)
{
#ifndef _CLIENTBUILD

	int i = 0;

	if (!pDebrisMgr) return;

	// Cache the model data...
	for(i = 0; i < nModels; i++)
	{
		if(pModels[i].szModel[0] != '\0')
            g_pLTServer->CacheFile(FT_MODEL, pModels[i].szModel);

		if(pModels[i].szSkin[0] != '\0')
	        g_pLTServer->CacheFile(FT_TEXTURE, pModels[i].szSkin);
	}

	// Cache the sounds...
	for(i = 0; i < nBounceSnds; i++)
	{
		if(szBounceSnds[i].szString)
            g_pLTServer->CacheFile(FT_SOUND, szBounceSnds[i].szString);
	}

	for(i = 0; i < nExplodeSnds; i++)
	{
		if(szExplodeSnds[i].szString)
            g_pLTServer->CacheFile(FT_SOUND, szExplodeSnds[i].szString);
	}

#endif  // _CLIENTBUILD
}


#ifndef _CLIENTBUILD

////////////////////////////////////////////////////////////////////////////
//
// CDebrisMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CDebrisMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CDebrisMgrPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char* const * aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (!g_pDebrisMgr)
	{
		// This will set the g_pDebrisMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];

#ifdef _WIN32
		sprintf(szFile, "%s\\%s", szRezPath, DEBRISMGR_DEFAULT_FILE);
#else
		sprintf(szFile, "%s/%s", szRezPath, DEBRISMGR_DEFAULT_FILE);
#endif

        sm_DebrisMgr.SetInRezFile(LTFALSE);
        sm_DebrisMgr.Init(g_pLTServer, szFile);
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisMgrPlugin::PopulateStringList(char* const * aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	_ASSERT(aszStrings && pcStrings && g_pDebrisMgr);
    if (!aszStrings || !pcStrings || !g_pDebrisMgr) return LTFALSE;

	// Add an entry for each debris type

	int nNumDebris = g_pDebrisMgr->GetNumDebris();
	_ASSERT(nNumDebris > 0);

    DEBRIS* pDebris = LTNULL;

	for (int i=0; i < nNumDebris; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pDebris = g_pDebrisMgr->GetDebris(i);
		if (pDebris && pDebris->szName[0])
		{
            uint32 dwDebrisNameLen = strlen(pDebris->szName);

			if (dwDebrisNameLen < cMaxStringLength && ((*pcStrings) + 1) < cMaxStrings)
			{
				strcpy(aszStrings[(*pcStrings)++], pDebris->szName);
			}
		}
	}

    return LTTRUE;
}

#endif // #ifndef _CLIENTBUILD

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MULTI_DEBRIS::MULTI_DEBRIS
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

MULTI_DEBRIS::MULTI_DEBRIS()
{
	nId				= 0;

	szName[0]		= '\0';

	nDebrisFX		= 0;
	szDebrisFX		= LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MULTI_DEBRIS::~MULTI_DEBRIS
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

MULTI_DEBRIS::~MULTI_DEBRIS()
{
	if(szDebrisFX)
	{
		delete[] szDebrisFX;
		szDebrisFX = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MULTI_DEBRIS::Init
//
//	PURPOSE:	Build the surface struct
//
// ----------------------------------------------------------------------- //

LTBOOL MULTI_DEBRIS::Init(CButeMgr & buteMgr, char* aTagName)
{
    if(!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, DEBRISMGR_MULTI_DEBRIS_NAME);
	if(!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	// Count the number of FX
	nDebrisFX = 0;
	sprintf(s_aAttName, DEBRISMGR_MULTI_DEBRIS_EFFECT, nDebrisFX);

	while(buteMgr.Exist(aTagName, s_aAttName))
	{
		nDebrisFX++;
		sprintf(s_aAttName, DEBRISMGR_MULTI_DEBRIS_EFFECT, nDebrisFX);
	}

	// Get the name for each FX
	if(nDebrisFX)
	{
		szDebrisFX = new DEBRIS_STRING[nDebrisFX];

		for(int i = 0; i < nDebrisFX; i++)
		{
			sprintf(s_aAttName, DEBRISMGR_MULTI_DEBRIS_EFFECT, i);
			str = buteMgr.GetString(aTagName, s_aAttName);

			if(!str.IsEmpty())
				strncpy(szDebrisFX[i].szString, (char*)(LPCSTR)str, ARRAY_LEN(szDebrisFX[i].szString));
		}
	}

    return LTTRUE;
}


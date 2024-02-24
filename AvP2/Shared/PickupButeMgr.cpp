//*********************************************************************************
//*********************************************************************************
// Project:		Aliens vs. Predator 2
// Purpose:		Retrieves attributes from the CharacterButes.txt
//*********************************************************************************
// File:		CharacterButeMgr.cpp
// Created:		Jan 24, 2000
// Updated:		May 29, 2000
// Author:		Andy Mattingly
//*********************************************************************************
//*********************************************************************************

#include "stdafx.h"
#include "PickupButeMgr.h"
#include "LTString.h"

//*********************************************************************************

#define PICKUP_BUTES_TAG					"Pickup"

#define PICKUP_BUTE_NAME					"Name"
#define PICKUP_BUTE_MODEL					"Model"
#define PICKUP_BUTE_SKIN					"Skin%d"

#define PICKUP_BUTE_DIMS					"Dims"
#define PICKUP_BUTE_SCALE					"Scale"
#define PICKUP_BUTE_HIT_POINTS				"HitPoints"
#define PICKUP_BUTE_MESSAGE					"Message"
#define PICKUP_BUTE_RESPAWN_TIME			"RespawnTime"
#define PICKUP_BUTE_PICKUP_DELAY			"PickupDelay"
#define PICKUP_BUTE_NUM_RESPAWNS			"NumRespawns"
#define PICKUP_BUTE_CAN_TOUCH				"CanTouch"
#define PICKUP_BUTE_CAN_ACTIVATE			"CanActivate"
#define PICKUP_BUTE_CAN_DAMAGE				"CanDamage"
#define PICKUP_BUTE_PLAYER_ONLY				"PlayerOnly"

#define PICKUP_BUTE_FX						"FX"
#define PICKUP_BUTE_PICKUPFX				"PickupFX"
#define PICKUP_BUTE_RESPAWNFX				"RespawnFX"
#define PICKUP_BUTE_DESTROYFX				"DestroyFX"

#define PICKUP_BUTE_MPFX					"MPFX"
#define PICKUP_BUTE_MPPICKUPFX				"MPPickupFX"
#define PICKUP_BUTE_MPRESPAWNFX				"MPRespawnFX"
#define PICKUP_BUTE_MPDESTROYFX				"MPDestroyFX"

#define PICKUP_BUTE_PICKUP_SOUND			"PickupSound"
#define PICKUP_BUTE_RESPAWN_SOUND			"RespawnSound"
#define PICKUP_BUTE_MOVETOGROUND			"MoveToGround"
#define PICKUP_BUTE_SOLID					"Solid"

#define PICKUP_BUTE_TYPE					"Type"

//*********************************************************************************

#define PICKUPFX_BUTES_TAG					"PickupFX"

#define PICKUPFX_BUTES_NAME					"Name"
#define PICKUPFX_BUTES_PARTICLE				"Particle"
#define PICKUPFX_BUTES_NUM_PARTICLES		"NumParticles"
#define PICKUPFX_BUTES_MAX_PARTICLES		"MaxParticles"
#define PICKUPFX_BUTES_LIFETIME				"ParticleLifetime"
#define PICKUPFX_BUTES_SIZE					"ParticleSize"
#define PICKUPFX_BUTES_EMIT_TYPE			"EmitType"
#define PICKUPFX_BUTES_LIGHT				"Light"
#define PICKUPFX_BUTES_LIGHT_RADIUS			"LightRadius"
#define PICKUPFX_BUTES_GLOW					"Glow"
#define PICKUPFX_BUTES_SPIN					"Spin"
#define PICKUPFX_BUTES_BOUNCE				"Bounce"

//*********************************************************************************

CPickupButeMgr* g_pPickupButeMgr = LTNULL;

//*********************************************************************************

static char s_aTagName[30];

//*********************************************************************************
// Saving and loading of the pickup bute structure

void t_PickupButes::Write(HMESSAGEWRITE hWrite)
{
	g_pInterface->WriteToMessageString(hWrite, m_szParent);

	g_pInterface->WriteToMessageString(hWrite, m_szName);
	g_pInterface->WriteToMessageString(hWrite, m_szModel);

	for(int i = 0; i < MAX_MODEL_TEXTURES; i++)
	{
		LTString ltStr = m_szSkins[i];
		*hWrite << ltStr;
	}

	g_pInterface->WriteToMessageVector(hWrite, &m_vDims);
	g_pInterface->WriteToMessageFloat(hWrite, m_fScale);
	g_pInterface->WriteToMessageFloat(hWrite, m_fHitPoints);
	g_pInterface->WriteToMessageString(hWrite, m_szMessage);
	g_pInterface->WriteToMessageFloat(hWrite, m_fRespawnTime);
	g_pInterface->WriteToMessageFloat(hWrite, m_fPickupDelay);
	g_pInterface->WriteToMessageDWord(hWrite, m_nNumRespawns);
	g_pInterface->WriteToMessageByte(hWrite, m_bCanTouch);
	g_pInterface->WriteToMessageByte(hWrite, m_bCanActivate);
	g_pInterface->WriteToMessageByte(hWrite, m_bCanDamage);
	g_pInterface->WriteToMessageByte(hWrite, m_bPlayerOnly);

	g_pInterface->WriteToMessageDWord(hWrite, m_nFXType);
	g_pInterface->WriteToMessageString(hWrite, m_szPickupFX);
	g_pInterface->WriteToMessageString(hWrite, m_szRespawnFX);
	g_pInterface->WriteToMessageString(hWrite, m_szDestroyFX);

	g_pInterface->WriteToMessageDWord(hWrite, m_nMPFXType);
	g_pInterface->WriteToMessageString(hWrite, m_szMPPickupFX);
	g_pInterface->WriteToMessageString(hWrite, m_szMPRespawnFX);
	g_pInterface->WriteToMessageString(hWrite, m_szMPDestroyFX);

	g_pInterface->WriteToMessageString(hWrite, m_szPickupSound);
	g_pInterface->WriteToMessageString(hWrite, m_szRespawnSound);
	g_pInterface->WriteToMessageByte(hWrite, m_bMoveToGround);
	g_pInterface->WriteToMessageByte(hWrite, m_bSolid);
	g_pInterface->WriteToMessageDWord(hWrite, m_nType);
}

//*********************************************************************************

void t_PickupButes::Read(HMESSAGEREAD hRead)
{
	hRead->ReadStringFL(m_szParent, sizeof(m_szParent));

	hRead->ReadStringFL(m_szName, sizeof(m_szName));
	hRead->ReadStringFL(m_szModel, sizeof(m_szModel));

	for(int i = 0; i < MAX_MODEL_TEXTURES; i++)
	{
		LTString ltStr;
		*hRead >> ltStr;
		strcpy(m_szSkins[i], ltStr.CStr());
	}

	g_pInterface->ReadFromMessageVector(hRead, &m_vDims);
	m_fScale = g_pInterface->ReadFromMessageFloat(hRead);
	m_fHitPoints = g_pInterface->ReadFromMessageFloat(hRead);
	hRead->ReadStringFL(m_szMessage, sizeof(m_szMessage));
	m_fRespawnTime = g_pInterface->ReadFromMessageFloat(hRead);
	m_fPickupDelay = g_pInterface->ReadFromMessageFloat(hRead);
	m_nNumRespawns = g_pInterface->ReadFromMessageDWord(hRead);
	m_bCanTouch = g_pInterface->ReadFromMessageByte(hRead);
	m_bCanActivate = g_pInterface->ReadFromMessageByte(hRead);
	m_bCanDamage = g_pInterface->ReadFromMessageByte(hRead);
	m_bPlayerOnly = g_pInterface->ReadFromMessageByte(hRead);

	m_nFXType = g_pInterface->ReadFromMessageDWord(hRead);
	hRead->ReadStringFL(m_szPickupFX, sizeof(m_szPickupFX));
	hRead->ReadStringFL(m_szRespawnFX, sizeof(m_szRespawnFX));
	hRead->ReadStringFL(m_szDestroyFX, sizeof(m_szDestroyFX));

	m_nMPFXType = g_pInterface->ReadFromMessageDWord(hRead);
	hRead->ReadStringFL(m_szPickupFX, sizeof(m_szMPPickupFX));
	hRead->ReadStringFL(m_szRespawnFX, sizeof(m_szMPRespawnFX));
	hRead->ReadStringFL(m_szDestroyFX, sizeof(m_szMPDestroyFX));

	hRead->ReadStringFL(m_szPickupSound, sizeof(m_szPickupSound));
	hRead->ReadStringFL(m_szRespawnSound, sizeof(m_szRespawnSound));
	m_bMoveToGround = g_pInterface->ReadFromMessageByte(hRead);
	m_bSolid = g_pInterface->ReadFromMessageByte(hRead);
	m_nType = g_pInterface->ReadFromMessageDWord(hRead);
}

//*********************************************************************************
// Saving and loading of the pickupFX bute structure

void t_PickupFXButes::Write(HMESSAGEWRITE hWrite)
{
	g_pInterface->WriteToMessageString(hWrite, m_szParent);

	g_pInterface->WriteToMessageString(hWrite, m_szParticle);

	g_pInterface->WriteToMessageDWord(hWrite, m_nNumParticles);
	g_pInterface->WriteToMessageDWord(hWrite, m_nMaxParticles);
	g_pInterface->WriteToMessageFloat(hWrite, m_fParticleLifetime);
	g_pInterface->WriteToMessageFloat(hWrite, m_fParticleSize);
	g_pInterface->WriteToMessageDWord(hWrite, m_nEmitType);
	g_pInterface->WriteToMessageByte(hWrite, m_bLight);
	g_pInterface->WriteToMessageDWord(hWrite, m_nLightRadius);
	g_pInterface->WriteToMessageByte(hWrite, m_bGlow);
	g_pInterface->WriteToMessageByte(hWrite, m_bSpin);
	g_pInterface->WriteToMessageByte(hWrite, m_bBounce);
}

//*********************************************************************************

void t_PickupFXButes::Read(HMESSAGEREAD hRead)
{
	hRead->ReadStringFL(m_szParent, sizeof(m_szParent));

	hRead->ReadStringFL(m_szParticle, sizeof(m_szParticle));

	m_nNumParticles = g_pInterface->ReadFromMessageDWord(hRead);
	m_nMaxParticles = g_pInterface->ReadFromMessageDWord(hRead);
	m_fParticleLifetime = g_pInterface->ReadFromMessageFloat(hRead);
	m_fParticleSize = g_pInterface->ReadFromMessageFloat(hRead);
	m_nEmitType = g_pInterface->ReadFromMessageDWord(hRead);
	m_bLight = g_pInterface->ReadFromMessageByte(hRead);
	m_nLightRadius = g_pInterface->ReadFromMessageDWord(hRead);
	m_bGlow = g_pInterface->ReadFromMessageByte(hRead);
	m_bSpin = g_pInterface->ReadFromMessageByte(hRead);
	m_bBounce = g_pInterface->ReadFromMessageByte(hRead);
}

//*********************************************************************************
// Construction/Destruction
//*********************************************************************************

CPickupButeMgr::CPickupButeMgr()
{
	m_nNumPickupButes = 0;
	m_pPickupButes = LTNULL;

	m_nNumPickupFXButes = 0;
	m_pPickupFXButes = LTNULL;
}

CPickupButeMgr::~CPickupButeMgr()
{
	Term();
}

//*********************************************************************************
//
//	ROUTINE:	CPickupButeMgr::Init()
//	PURPOSE:	Init mgr
//
//*********************************************************************************

LTBOOL CPickupButeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	if (g_pPickupButeMgr || !szAttributeFile) return LTFALSE;
	if (!Parse(pInterface, szAttributeFile)) return LTFALSE;


	// Set up global pointer...
	g_pPickupButeMgr = this;


	// Calculate the number of pickupFX attribute sets
	m_nNumPickupFXButes = 0;
	sprintf(s_aTagName, "%s0", PICKUPFX_BUTES_TAG);

	while(Exist(s_aTagName))
	{
		m_nNumPickupFXButes++;
		sprintf(s_aTagName, "%s%d", PICKUPFX_BUTES_TAG, m_nNumPickupFXButes);
	}

	// Calculate the number of pickup attribute sets
	m_nNumPickupButes = 0;
	sprintf(s_aTagName, "%s0", PICKUP_BUTES_TAG);

	while(Exist(s_aTagName))
	{
		m_nNumPickupButes++;
		sprintf(s_aTagName, "%s%d", PICKUP_BUTES_TAG, m_nNumPickupButes);
	}

	// Create the pickupFX attribute list
	m_pPickupFXButes = new PickupFXButes[m_nNumPickupFXButes];
	ASSERT( m_pPickupFXButes );
	if( m_pPickupFXButes )
	{
		for( int i = 0; i < m_nNumPickupFXButes; ++i )
		{
			LoadPickupFXButes(i, m_pPickupFXButes[i]);
		}
	}

	// Create the pickup attribute list
	m_pPickupButes = new PickupButes[m_nNumPickupButes];
	ASSERT( m_pPickupButes );
	if( m_pPickupButes )
	{
		for( int i = 0; i < m_nNumPickupButes; ++i )
		{
			LoadPickupButes(i, m_pPickupButes[i]);
		}
	}

	// Free up butemgr's memory and what-not.

	m_buteMgr.Term();

	return LTTRUE;
}


//*********************************************************************************
//
//	ROUTINE:	CPickupButeMgr::Term()
//	PURPOSE:	Clean up.
//
//*********************************************************************************

void CPickupButeMgr::Term()
{
	g_pPickupButeMgr = LTNULL;

	if(m_pPickupFXButes)
	{
		delete[] m_pPickupFXButes;
		m_pPickupFXButes = LTNULL;
	}

	if(m_pPickupButes)
	{
		delete[] m_pPickupButes;
		m_pPickupButes = LTNULL;
	}
}

//*********************************************************************************

int CPickupButeMgr::GetPickupSetFromName(char *szName) const
{
	// Find out which character we're looking for
	for(int i = 0; i < m_nNumPickupButes; i++)
	{
		if(!stricmp(szName, m_pPickupButes[i].m_szName))
			return i;
	}

	g_pInterface->CPrint("PickupButeMgr: Could not find set %s! Defaulting to index zero!", szName);

	return -1;
}

//*********************************************************************************

int CPickupButeMgr::GetPickupFXSetFromName(char *szName) const
{
	// Find out which character we're looking for
	for(int i = 0; i < m_nNumPickupFXButes; i++)
	{
		if(!stricmp(szName, m_pPickupFXButes[i].m_szName))
			return i;
	}

	g_pInterface->CPrint("PickupFXButeMgr: Could not find set %s! Defaulting to index zero!", szName);

	return -1;
}

//*********************************************************************************

const PickupButes & CPickupButeMgr::GetPickupButes(char *szName) const
{
	int nSet = GetPickupSetFromName(szName);

	if( nSet >= 0 && nSet < m_nNumPickupButes )
		return m_pPickupButes[nSet];

	return m_pPickupButes[0];
}

//*********************************************************************************

const PickupFXButes & CPickupButeMgr::GetPickupFXButes(char *szName) const
{
	int nSet = GetPickupFXSetFromName(szName);

	if( nSet >= 0 && nSet < m_nNumPickupFXButes )
		return m_pPickupFXButes[nSet];

	return m_pPickupFXButes[0];
}

//*********************************************************************************

void CPickupButeMgr::LoadPickupButes(int nSet, PickupButes &butes)
{
	SAFE_STRCPY(butes.m_szParent, GetStringAttrib(nSet, "Parent"));

	SAFE_STRCPY(butes.m_szName, GetStringAttrib(nSet, PICKUP_BUTE_NAME));
	SAFE_STRCPY(butes.m_szModel, GetStringAttrib(nSet, PICKUP_BUTE_MODEL));

	// Calculate the number of pickup attribute sets
	char aAttribName[64];
	sprintf(s_aTagName, "%s%d", PICKUP_BUTES_TAG, nSet);

	for(int i = 0; i < MAX_MODEL_TEXTURES; i++)
	{
		sprintf(aAttribName, PICKUP_BUTE_SKIN, i);

		if(Exist(s_aTagName, aAttribName))
		{
			SAFE_STRCPY(butes.m_szSkins[i], GetStringAttrib(nSet, aAttribName));
		}
		else
		{
			butes.m_szSkins[i][0] = '\0';
		}
	}

	butes.m_vDims = GetVectorAttrib(nSet, PICKUP_BUTE_DIMS);
	butes.m_fScale = (LTFLOAT)GetDoubleAttrib(nSet, PICKUP_BUTE_SCALE);
	butes.m_fHitPoints = (LTFLOAT)GetDoubleAttrib(nSet, PICKUP_BUTE_HIT_POINTS);
	SAFE_STRCPY(butes.m_szMessage, GetStringAttrib(nSet, PICKUP_BUTE_MESSAGE));
	butes.m_fRespawnTime = (LTFLOAT)GetDoubleAttrib(nSet, PICKUP_BUTE_RESPAWN_TIME);
	butes.m_fPickupDelay = (LTFLOAT)GetDoubleAttrib(nSet, PICKUP_BUTE_PICKUP_DELAY);
	butes.m_nNumRespawns = GetIntAttrib(nSet, PICKUP_BUTE_NUM_RESPAWNS);
	butes.m_bCanTouch = (LTBOOL)GetIntAttrib(nSet, PICKUP_BUTE_CAN_TOUCH);
	butes.m_bCanActivate = (LTBOOL)GetIntAttrib(nSet, PICKUP_BUTE_CAN_ACTIVATE);
	butes.m_bCanDamage = (LTBOOL)GetIntAttrib(nSet, PICKUP_BUTE_CAN_DAMAGE);
	butes.m_bPlayerOnly = (LTBOOL)GetIntAttrib(nSet, PICKUP_BUTE_PLAYER_ONLY);

	butes.m_nFXType = GetPickupFXSetFromName(GetStringAttrib(nSet, PICKUP_BUTE_FX).GetBuffer(0));
	SAFE_STRCPY(butes.m_szPickupFX, GetStringAttrib(nSet, PICKUP_BUTE_PICKUPFX));
	SAFE_STRCPY(butes.m_szRespawnFX, GetStringAttrib(nSet, PICKUP_BUTE_RESPAWNFX));
	SAFE_STRCPY(butes.m_szDestroyFX, GetStringAttrib(nSet, PICKUP_BUTE_DESTROYFX));

	butes.m_nMPFXType = GetPickupFXSetFromName(GetStringAttrib(nSet, PICKUP_BUTE_MPFX).GetBuffer(0));
	SAFE_STRCPY(butes.m_szMPPickupFX, GetStringAttrib(nSet, PICKUP_BUTE_MPPICKUPFX));
	SAFE_STRCPY(butes.m_szMPRespawnFX, GetStringAttrib(nSet, PICKUP_BUTE_MPRESPAWNFX));
	SAFE_STRCPY(butes.m_szMPDestroyFX, GetStringAttrib(nSet, PICKUP_BUTE_MPDESTROYFX));

	SAFE_STRCPY(butes.m_szPickupSound, GetStringAttrib(nSet, PICKUP_BUTE_PICKUP_SOUND));
	SAFE_STRCPY(butes.m_szRespawnSound, GetStringAttrib(nSet, PICKUP_BUTE_RESPAWN_SOUND));
	butes.m_bMoveToGround = (LTBOOL)GetIntAttrib(nSet, PICKUP_BUTE_MOVETOGROUND);
	butes.m_bSolid = (LTBOOL)GetIntAttrib(nSet, PICKUP_BUTE_SOLID);
	butes.m_nType = GetIntAttrib(nSet, PICKUP_BUTE_TYPE);
}

//*********************************************************************************

void CPickupButeMgr::LoadPickupFXButes(int nSet, PickupFXButes &butes)
{
	SAFE_STRCPY(butes.m_szParent, GetFXStringAttrib(nSet, "Parent"));

	SAFE_STRCPY(butes.m_szName, GetFXStringAttrib(nSet, PICKUP_BUTE_NAME));
	SAFE_STRCPY(butes.m_szParticle, GetFXStringAttrib(nSet, PICKUPFX_BUTES_PARTICLE));

	butes.m_nNumParticles = GetFXIntAttrib(nSet, PICKUPFX_BUTES_NUM_PARTICLES);
	butes.m_nMaxParticles = GetFXIntAttrib(nSet, PICKUPFX_BUTES_MAX_PARTICLES);
	butes.m_fParticleLifetime = (LTFLOAT)GetFXDoubleAttrib(nSet, PICKUPFX_BUTES_LIFETIME);
	butes.m_fParticleSize = (LTFLOAT)GetFXDoubleAttrib(nSet, PICKUPFX_BUTES_SIZE);
	butes.m_nEmitType = GetFXIntAttrib(nSet, PICKUPFX_BUTES_EMIT_TYPE);
	butes.m_bLight = (LTBOOL)GetFXIntAttrib(nSet, PICKUPFX_BUTES_LIGHT);
	butes.m_nLightRadius = GetFXIntAttrib(nSet, PICKUPFX_BUTES_LIGHT_RADIUS);
	butes.m_bGlow = (LTBOOL)GetFXIntAttrib(nSet, PICKUPFX_BUTES_GLOW);
	butes.m_bSpin = (LTBOOL)GetFXIntAttrib(nSet, PICKUPFX_BUTES_SPIN);
	butes.m_bBounce = (LTBOOL)GetFXIntAttrib(nSet, PICKUPFX_BUTES_BOUNCE);
}

//*********************************************************************************

CString CPickupButeMgr::GetStringAttrib(int nSet, char *szAttrib)
{
	if(m_nNumPickupButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", PICKUP_BUTES_TAG, nSet);
	return GetString(s_aTagName, szAttrib);
}

//*********************************************************************************

DVector CPickupButeMgr::GetVectorAttrib(int nSet, char *szAttrib)
{
	if(m_nNumPickupButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", PICKUP_BUTES_TAG, nSet);
	return GetVector(s_aTagName, szAttrib);
}

//*********************************************************************************

CPoint CPickupButeMgr::GetPointAttrib(int nSet, char *szAttrib)
{
	if(m_nNumPickupButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", PICKUP_BUTES_TAG, nSet);
	return GetPoint(s_aTagName, szAttrib);
}

//*********************************************************************************

int CPickupButeMgr::GetIntAttrib(int nSet, char *szAttrib)
{
	if(m_nNumPickupButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", PICKUP_BUTES_TAG, nSet);
	return GetInt(s_aTagName, szAttrib);
}

//*********************************************************************************

double CPickupButeMgr::GetDoubleAttrib(int nSet, char *szAttrib)
{
	if(m_nNumPickupButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", PICKUP_BUTES_TAG, nSet);
	return GetDouble(s_aTagName, szAttrib);
}

//*********************************************************************************

CString CPickupButeMgr::GetFXStringAttrib(int nSet, char *szAttrib)
{
	if(m_nNumPickupFXButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", PICKUPFX_BUTES_TAG, nSet);
	return GetString(s_aTagName, szAttrib);
}

//*********************************************************************************

DVector CPickupButeMgr::GetFXVectorAttrib(int nSet, char *szAttrib)
{
	if(m_nNumPickupFXButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", PICKUPFX_BUTES_TAG, nSet);
	return GetVector(s_aTagName, szAttrib);
}

//*********************************************************************************

CPoint CPickupButeMgr::GetFXPointAttrib(int nSet, char *szAttrib)
{
	if(m_nNumPickupFXButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", PICKUPFX_BUTES_TAG, nSet);
	return GetPoint(s_aTagName, szAttrib);
}

//*********************************************************************************

int CPickupButeMgr::GetFXIntAttrib(int nSet, char *szAttrib)
{
	if(m_nNumPickupFXButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", PICKUPFX_BUTES_TAG, nSet);
	return GetInt(s_aTagName, szAttrib);
}

//*********************************************************************************

double CPickupButeMgr::GetFXDoubleAttrib(int nSet, char *szAttrib)
{
	if(m_nNumPickupFXButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", PICKUPFX_BUTES_TAG, nSet);
	return GetDouble(s_aTagName, szAttrib);
}



//*********************************************************************************
//
// CPickupButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use WeaponMgr
//
//*********************************************************************************

#ifndef _CLIENTBUILD  // Server-side only

// Plugin statics

LTBOOL CPickupButeMgrPlugin::sm_bInitted = LTFALSE;
CPickupButeMgr CPickupButeMgrPlugin::sm_ButeMgr;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupButeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT	CPickupButeMgrPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	if (!sm_bInitted)
	{
		char szFile[256];

		// Create the bute mgr if necessary
		if(!g_pPickupButeMgr)
		{
#ifdef _WIN32
			sprintf(szFile, "%s\\%s", szRezPath, PICKUP_BUTES_DEFAULT_FILE);
#else
			sprintf(szFile, "%s/%s", szRezPath, PICKUP_BUTES_DEFAULT_FILE);
#endif
			sm_ButeMgr.SetInRezFile(LTFALSE);
			sm_ButeMgr.Init(g_pLTServer, szFile);
			sm_bInitted = LTTRUE;
		}
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupButeMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

void CPickupButeMgrPlugin::PopulateStringList(char* const * aszStrings, uint32* pcStrings, 
	const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if (!aszStrings || !pcStrings) return;

	// Add an entry for each weapon...

	uint32 dwNumSets = sm_ButeMgr.GetNumPickupButes();
	PickupButes puButes;

	for(uint32 i = 0; i < dwNumSets; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		puButes = sm_ButeMgr.GetPickupButes(i);
		uint32 dwNameLen = strlen(puButes.m_szName);

		if(puButes.m_szName[0] && (dwNameLen < cMaxStringLength) && ((*pcStrings) + 1 < cMaxStrings))
		{
			// Just use the pickup name...
			strcpy(*aszStrings, puButes.m_szName);
			++aszStrings;
			++(*pcStrings);
		}
	}
}


#endif //_CLIENTBUILD

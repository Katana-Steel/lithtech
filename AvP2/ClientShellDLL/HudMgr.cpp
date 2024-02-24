// ----------------------------------------------------------------------- //
//
// MODULE  : HudMgr.cpp
//
// PURPOSE : HudMgr implimentation
//
// CREATED : 01.21.2000
//
// ----------------------------------------------------------------------- //

//external dependencies
#include "stdafx.h"
#include "HudMgr.h"
#include "LayoutMgr.h"
#include "GameClientShell.h"
#include "SoundMgr.h"
#include "VarTrack.h"
#include "CharacterFuncs.h"
#include "PlayerStats.h"
#include "MotionDetector.h"

VarTrack	g_cvarIconOffset;
VarTrack	g_cvarNumIcons;
VarTrack	g_cvarHorizIcons;
VarTrack	g_cvarFlashCycleTime;
VarTrack	g_cvarMeterRate;

//external globals
extern CLayoutMgr* g_pLayoutMgr;
extern CGameClientShell* g_pGameClientShell;
extern CSoundMgr* g_pSoundMgr;

//constants
const LTFLOAT FLASH_CYCLE_TIME = 2.0f;
const char g_szSquelchSound[] = "dialogsquelch";  // This is the sound bute to be played when SetCaption and StopCaption are called.

// -----------------------------------------------/
//
// void CHudMgr::CHudMgr Implimentation
//
// Constructor
//
// -----------------------------------------------/
CHudMgr::CHudMgr()
{
	m_bHudEnabled			= LTFALSE;		
	m_bMDEnabled			= LTFALSE;		
	m_bMDDraw				= LTFALSE;		
	m_nNumHudElements		= 0;	
	m_nNumHudFonts			= 0;		
	m_paHudElements			= LTNULL;	
	m_aplfFonts				= LTNULL;		
	m_pPlayerStats			= LTNULL;
	m_pMotionDetector		= LTNULL;
	m_pBaseBarrel			= LTNULL;
	m_nNumBarrels			= 0;
	m_aClipsString			= LTNULL;
	m_aClipString			= LTNULL;
	m_aBarrelLabel			= LTNULL;
	m_nActiveBarrel			= 0;
	m_bShowPickupList		= LTFALSE;
	m_hAmmoIcon				= LTNULL;
	m_pAmmoCounterFunction	= NoCounter;
	m_pAmmoDrawFunction		= NoDrawCounter;
	m_bBossHealthOn			= LTFALSE;
	m_fBossHealthRatio		= 1.0f;
	m_WeaponID				= -1;
	m_nHudId				= -1;
}

// -----------------------------------------------/
//
// void CHudMgr::Init Implimentation
//
// Sets member variables to initial states
//
// -----------------------------------------------/
LTBOOL CHudMgr::Init(uint32 nIndex, LTBOOL bRestore, LTBOOL bForce)
{
	char		s_aTagName[30];	//temp var for bute info

	//sanity check
	if(!g_pGameClientShell || !g_pLayoutMgr) return LTFALSE;
	
	//get pointer to player stats
	m_pPlayerStats = g_pGameClientShell->GetPlayerStats();
	
	//test
	if(!m_pPlayerStats)
		return LTFALSE;

	//get the default config index for this character
	uint32 nCfgNum;
	
	if(nIndex)
	{
		nCfgNum = g_pLayoutMgr->GetHUDCfg(m_pPlayerStats->GetHUDType(), nIndex);
	}
	else
	{
		nCfgNum = g_pLayoutMgr->GetDefaultCfg(m_pPlayerStats->GetHUDType());
	}

	//test
	if(nCfgNum == HUD_ERROR)
	{
		Term();
		return LTFALSE;	
	}

	// see if this is a new hud
	if(m_nHudId != nCfgNum || bForce)
	{
		// clean up the old hud
		Term();

		// record the new hud ID
		m_nHudId = nCfgNum;
	}
	else
	{
		// no need to reload the hud
		return LTTRUE;
	}
		
	//catenate tag and layout number
	sprintf(s_aTagName, "%s%d", HUD_BASIC_TAG, nCfgNum);

	//set initial variables (only if this is not a restore, load will have set these)
	if(!bRestore)
	{
		m_bHudEnabled		= DTRUE;
		m_bMDEnabled		= DTRUE;
		m_bMDDraw			= DTRUE;
	}
	
	//get initial data from bute file
	m_nNumHudElements	= g_pLayoutMgr->GetNumHudElements(s_aTagName);

	if(g_pLayoutMgr->GetHasMotionDetector(s_aTagName))
	{
		m_pMotionDetector = new CMotionDetector;

		if(m_pMotionDetector)
			m_pMotionDetector->Init();
	}

	//be sure to re-set this member
	m_bShowPickupList = LTFALSE;

	// Be sure to clean up any old lists...
	for( int i=0; i<MAX_PICKUP_LIST; i++)
		m_PickupList[i].Reset();

	for( i=0; i<MAX_COUNTER_ITEMS; i++)
		m_CounterItems[i].Reset();
	
	//if there are elements to load, new the array
	if(m_nNumHudElements)
	{
		m_paHudElements	= new CHudElement[m_nNumHudElements];
		
		//now initialize the elements
		for(uint32 i=0 ; i<m_nNumHudElements ; i++)
		{
			m_paHudElements[i].Init(s_aTagName, i);
			
			SetFunctionPointers (s_aTagName, i);
		}
	}
	else
		m_paHudElements = LTNULL;

	//load up font resources
	LoadFonts(s_aTagName);

	m_nNumBarrels = 0;

	g_cvarIconOffset.Init	(g_pLTClient, "IconOffset", NULL, 2.0f);
	g_cvarNumIcons.Init		(g_pLTClient, "NumIcons", NULL, 10.0f);
	g_cvarHorizIcons.Init	(g_pLTClient, "HorizIcons", NULL, 0.0f);
	g_cvarFlashCycleTime.Init	(g_pLTClient, "FlashCycleTime", NULL, 2.0f);
	g_cvarMeterRate.Init	(g_pLTClient, "MeterRate", NULL, 1.0f);

	// Make sure our ammo counter is set up again.
	if(bRestore)
		NewWeapon(g_pWeaponMgr->GetWeapon(m_WeaponID));

	return DTRUE;
}

// -----------------------------------------------/
//
// void CHudMgr::ResetCounter Implimentation
//
// Resets the ammo counter
//
// -----------------------------------------------/
void CHudMgr::ResetCounter ()
{
	NewWeapon(g_pWeaponMgr->GetWeapon(m_WeaponID));
}

// -----------------------------------------------/
//
// void CHudMgr::SetFunctionPointers Implimentation
//
// Sets function pointers for hud elements
//
// -----------------------------------------------/
void CHudMgr::SetFunctionPointers (char * pTag, uint32 nIndex)
{
	//set drawing function pointer
	uint32 nDrawIndex = g_pLayoutMgr->GetDrawIndex(pTag, nIndex);
	
	switch (nDrawIndex)
	{
	case 0:  {m_paHudElements[nIndex].m_pDrawFunction = NormalDrawTransparent; break;}
	case 1:  {m_paHudElements[nIndex].m_pDrawFunction = NormalDrawNonTransparent; break;}
	case 2:  {m_paHudElements[nIndex].m_pDrawFunction = NullFunction0; break;}
	case 3:  {m_paHudElements[nIndex].m_pDrawFunction = DrawBmFontText; break;}
	case 4:  {m_paHudElements[nIndex].m_pDrawFunction = DrawHealthArmorFill; break;}
	case 5:  {m_paHudElements[nIndex].m_pDrawFunction = DrawAlienHealth; break;}
	case 6:  {m_paHudElements[nIndex].m_pDrawFunction = DrawPredatorHealth; break;}
	case 7:  {m_paHudElements[nIndex].m_pDrawFunction = DrawPredatorEnergy; break;}
	case 8:  {m_paHudElements[nIndex].m_pDrawFunction = DrawMarineEKG; break;}
	case 9:  {m_paHudElements[nIndex].m_pDrawFunction = DrawMarineAmmoCounter; break;}
	case 10: {m_paHudElements[nIndex].m_pDrawFunction = DrawAirMeter; break; }
	case 11: {m_paHudElements[nIndex].m_pDrawFunction = DrawMarineAirMeterFill; break; }
	case 12: {m_paHudElements[nIndex].m_pDrawFunction = DrawPredatorAirMeterFill; break; }
	case 13: {m_paHudElements[nIndex].m_pDrawFunction = DrawMarinePickupList; m_bShowPickupList=LTTRUE; break; }
	case 14: {m_paHudElements[nIndex].m_pDrawFunction = DrawChestbursterHealth; break; }
	case 15: {m_paHudElements[nIndex].m_pDrawFunction = DrawPredatorPickupList; m_bShowPickupList=LTTRUE; break; }
	case 16: {m_paHudElements[nIndex].m_pDrawFunction = DrawExoArmor; break;}
	case 17: {m_paHudElements[nIndex].m_pDrawFunction = DrawExoEnergy; break;}
	case 18: {m_paHudElements[nIndex].m_pDrawFunction = DrawBatteryMeter; break;}
	case 19: {m_paHudElements[nIndex].m_pDrawFunction = DrawBatteryMeterFill; break;}
	case 20: {m_paHudElements[nIndex].m_pDrawFunction = DrawBossHealth; break;}
	case 21: {m_paHudElements[nIndex].m_pDrawFunction = DrawBossHealthFill; break;}
	case 22: {m_paHudElements[nIndex].m_pDrawFunction = DrawSkullCounter; break;}
	default: {m_paHudElements[nIndex].m_pDrawFunction = NormalDrawTransparent; break;}
	}

	//set update function pointer
	uint32 nUpdateIndex = g_pLayoutMgr->GetUpdateIndex(pTag, nIndex);
	
	switch (nUpdateIndex)
	{
	case 0:  {m_paHudElements[nIndex].m_pUpdateFunction = NoUpdate; break;}
	case 1:	 {m_paHudElements[nIndex].m_pUpdateFunction = UpdateHealthStr; break;}
	case 2:	 {m_paHudElements[nIndex].m_pUpdateFunction = UpdateArmorStr; break;}
	case 3:	 {m_paHudElements[nIndex].m_pUpdateFunction = UpdateHealthFill; break;}
	case 4:	 {m_paHudElements[nIndex].m_pUpdateFunction = UpdateArmorFill; break;}
	case 5:	 {m_paHudElements[nIndex].m_pUpdateFunction = UpdateAlienHealth; break;}
	case 6:	 {m_paHudElements[nIndex].m_pUpdateFunction = UpdateMarineEKG; break;}
	case 7:	 {m_paHudElements[nIndex].m_pUpdateFunction = UpdateMarineAmmoCount; break;}
	case 8:	 {m_paHudElements[nIndex].m_pUpdateFunction = UpdatePickupList; break;}
	case 9:	 {m_paHudElements[nIndex].m_pUpdateFunction = UpdateExoArmor; break;}
	case 10: {m_paHudElements[nIndex].m_pUpdateFunction = UpdateExoEnergy; break;}
	case 11: {m_paHudElements[nIndex].m_pUpdateFunction = UpdateFlareStr; break;}
	default: {m_paHudElements[nIndex].m_pUpdateFunction = NoUpdate; break;}
	}
}

// -----------------------------------------------/
//
// void CHudMgr::LoadFonts Implimentation
//
// Sets member variables to initial states
//
// -----------------------------------------------/
void CHudMgr::LoadFonts(char * pTag)
{
	LITHFONTCREATESTRUCT	lfCreateStruct;
	char					s_aSurfaceName[60];

	//set create struct fixed elements
	lfCreateStruct.hTransColor	= SETRGB_T(0,0,0);
	lfCreateStruct.bChromaKey	= LTTRUE;

	//get number of fonts
	m_nNumHudFonts = g_pLayoutMgr->GetNumHudFonts(pTag);

	//if there are fonts to load, new the array
	if(m_nNumHudFonts)
	{
		m_aplfFonts	= new LithFont*[m_nNumHudFonts];
		
		//now initialize the fonts
		for(uint32 i=0 ; i<m_nNumHudFonts ; i++)
		{
			//get font pitch
			lfCreateStruct.nFixedPitch = (unsigned char)g_pLayoutMgr->GetFontPitch(pTag, i);

			if(lfCreateStruct.nFixedPitch == 0)
				lfCreateStruct.nGroupFlags = LTF_INCLUDE_ALL;
			else
				lfCreateStruct.nGroupFlags = LTF_INCLUDE_NUMBERS;

			//get surface path and name
			g_pLayoutMgr->GetFontName(pTag, i, s_aSurfaceName, 60);

			//set name elelment for font
			lfCreateStruct.szFontBitmap = s_aSurfaceName;

			//load up the font
			m_aplfFonts[i] = CreateLithFont(g_pLTClient, &lfCreateStruct, DTRUE);

			if(!m_aplfFonts[i])
				g_pLTClient->ShutdownWithMessage("ERROR: Could not initialize font in HUDMgr");
		}
	}
}


// -----------------------------------------------/
//
// void CHudMgr::DrawHud Implimentation
//
// Cycles thru the hud elements, calling the draw function
//
// -----------------------------------------------/
void CHudMgr::DrawHud ()
{
	//check to see if HUD is enabled
	if(m_bHudEnabled)
	{
		//draw the elements
		for(uint32 i=0 ; i<m_nNumHudElements ; i++)
		{
			(this->*(m_paHudElements[i].m_pDrawFunction))(&m_paHudElements[i]);
		}
	}

	if(m_bHudEnabled && m_bMDEnabled && m_pMotionDetector && m_bMDDraw)
	{
		uint32 dwFlags;
		g_pLTClient->GetObjectUserFlags(g_pLTClient->GetClientObject(), &dwFlags);

		m_pMotionDetector->Draw(dwFlags & USRFLG_CHAR_EMPEFFECT);
	}

}

// -----------------------------------------------/
//
// void CHudMgr::DrawCloseCaption Implimentation
//
// Draws the close captioning
//
// -----------------------------------------------/
void CHudMgr::DrawCloseCaption ()
{
	ASSERT(0);
}


// -----------------------------------------------/
//
// void CHudMgr::DrawHud Implimentation
//
// Cycles thru the hud elements, calling the draw function
//
// -----------------------------------------------/
void CHudMgr::UpdateHud ()
{
	//check to see if HUD is enabled
	if(m_bHudEnabled)
	{
		//draw the elements
		for(uint32 i=0 ; i<m_nNumHudElements ; i++)
		{
			(this->*(m_paHudElements[i].m_pUpdateFunction))(&m_paHudElements[i]);
		}
	}

}

// -----------------------------------------------/
//
// void CHudMgr::Term Implimentation
//
// Clen-up routine
//
// -----------------------------------------------/
void CHudMgr::Term()
{
	m_nNumHudElements = 0;

	//clean up HudElement array
	if(m_paHudElements)
	{
		//free the array
		delete [] m_paHudElements;
		m_paHudElements = LTNULL;

		//call termination function for all fonts
		for(uint32 i=0 ; i<m_nNumHudFonts ; i++)
		{
			FreeLithFont(m_aplfFonts[i]);
			m_aplfFonts[i] = LTNULL;
		}

		//free the array
		delete [] m_aplfFonts;
		m_aplfFonts = LTNULL;
	}

	if(m_pMotionDetector)
	{
		m_pMotionDetector->Term();
		delete m_pMotionDetector;
		m_pMotionDetector = LTNULL;
	}

	if(m_aClipsString)
	{
		for(uint32 i=0; i < m_nNumBarrels; i++)
		{
			if(m_aClipsString[i])
			{
				g_pLTClient->FreeString(m_aClipsString[i]);
				m_aClipsString[i] = LTNULL;
			}
		}
		delete m_aClipsString;
		m_aClipsString = LTNULL;
	}

	if(m_aClipString)
	{
		for(uint32 i=0; i < m_nNumBarrels; i++)
		{
			if(m_aClipString[i])
			{
				g_pLTClient->FreeString(m_aClipString[i]);
				m_aClipString[i] = LTNULL;
			}
		}
		delete m_aClipString;
		m_aClipString = LTNULL;
	}

	if(m_aBarrelLabel)
	{
		for(uint32 i=0; i < m_nNumBarrels; i++)
		{
			if(m_aBarrelLabel[i])
			{
				g_pLTClient->FreeString(m_aBarrelLabel[i]);
				m_aBarrelLabel[i] = LTNULL;
			}
		}
		delete m_aBarrelLabel;
		m_aBarrelLabel = LTNULL;
	}

}

// -----------------------------------------------/
//
// void CHudMgr::LoadAmmoIcon Implimentation
//
// Load the ammo icon for pred ammo counter
//
// -----------------------------------------------/
void CHudMgr::LoadAmmoIcon (WEAPON* pNewWeapon)
{
	if(!pNewWeapon) return;

	BARREL* pBarrel = g_pWeaponMgr->GetBarrel(pNewWeapon->aBarrelTypes[0]);
	AMMO*	pAmmo	= g_pWeaponMgr->GetAmmo(pBarrel->nAmmoType);
	AMMO_POOL*	pPool	= g_pWeaponMgr->GetAmmoPool(pAmmo->szAmmoPool);

	m_hAmmoIcon = g_pInterfaceResMgr->GetSharedSurface(pPool->szCounterIcon);

	if(!m_hAmmoIcon) return;

	g_pClientDE->SetSurfaceAlpha (m_hAmmoIcon, 1.0f);
}

// -----------------------------------------------/
//
// void CHudMgr::NewWeapon Implimentation
//
// Updates the info needed when a new weapon is selected
//
// -----------------------------------------------/
void CHudMgr::NewWeapon (WEAPON* pNewWeapon)
{
	if(!pNewWeapon)
	{
		//handle no weapon
		m_pAmmoCounterFunction	= NoCounter; 
		m_pAmmoDrawFunction		= NoDrawCounter;
		return;
	}

	// Save the weapon for load/save purposes...
	m_WeaponID = pNewWeapon->nId;

	//clean up the old strings
	if(m_aClipString)
	{
		for(uint32 i=0; i < m_nNumBarrels; i++)
		{
			if(m_aClipString[i])
			{
				g_pLTClient->FreeString(m_aClipString[i]);
				m_aClipString[i] = LTNULL;
			}
		}
		delete m_aClipString;
		m_aClipString = LTNULL;
	}

	if(m_aBarrelLabel)
	{
		for(uint32 i=0; i < m_nNumBarrels; i++)
		{
			if(m_aBarrelLabel[i])
			{
				g_pLTClient->FreeString(m_aBarrelLabel[i]);
				m_aBarrelLabel[i] = LTNULL;
			}
		}
		delete m_aBarrelLabel;
		m_aBarrelLabel = LTNULL;
	}

	if(m_aClipsString)
	{
		for(uint32 i=0; i < m_nNumBarrels; i++)
		{
			if(m_aClipsString[i])
			{
				g_pLTClient->FreeString(m_aClipsString[i]);
				m_aClipsString[i] = LTNULL;
			}
		}
		delete m_aClipsString;
		m_aClipsString = LTNULL;
	}

	//set base barrel pointer
	m_pBaseBarrel = g_pWeaponMgr->GetBarrel(pNewWeapon->aBarrelTypes[PRIMARY_BARREL]);

	//set barrel count
	if(m_pBaseBarrel)
	{
		//get next barrel
		BARREL* pTemp = m_pBaseBarrel;

		//reset counter
		m_nNumBarrels = 0;

		//count
		do
		{
			m_nNumBarrels++;
			if(pTemp)
				pTemp = g_pWeaponMgr->GetBarrel(pTemp->szNextBarrel);
		}while(pTemp && pTemp != m_pBaseBarrel);
	}

	//set function pointers for counter function
	switch(pNewWeapon->nCounterType)
	{
	case (0): 
		{
			m_pAmmoCounterFunction	= NoCounter; 
			m_pAmmoDrawFunction		= NoDrawCounter;
			m_WeaponID				= -1;
			break;
		}
	case (1): 
		{
			InitNormalCounter (pNewWeapon);
			m_pAmmoCounterFunction	= NormalCounter; 
			m_pAmmoDrawFunction		= NormalDrawCounter;
			break;
		}
	case (2): 
		{
			InitMultiCounter (pNewWeapon);
			m_pAmmoCounterFunction	= MultiAmmoCounter; 
			m_pAmmoDrawFunction		= NormalDrawCounter;
			break;
		}
	case (3): 
		{
			m_pAmmoCounterFunction	= NoCounter; 
			m_pAmmoDrawFunction		= SpearAmmoDrawCounter;
			LoadAmmoIcon(pNewWeapon);
			break;
		}
	case (4): 
		{
			InitExoCounter ();
			m_pAmmoCounterFunction	= ExosuitAmmoCounter; 
			m_pAmmoDrawFunction		= NormalDrawCounter;
			break;
		}
	default: 		
		{
			m_pAmmoCounterFunction	= NoCounter; 
			m_pAmmoDrawFunction		= NoDrawCounter;
			break;
		}

	}

	//create new strings
	m_aClipsString	= new HSTRING[m_nNumBarrels];
	m_aClipString	= new HSTRING[m_nNumBarrels];
	m_aBarrelLabel	= new HSTRING[m_nNumBarrels];

	for(uint32 i=0; i<m_nNumBarrels; i++)
	{
		m_aClipsString[i] = LTNULL;
		m_aClipString[i] = LTNULL;
		m_aBarrelLabel[i] = LTNULL;
	}
}

// -----------------------------------------------/
//
// void CHudMgr::UpdateHealthFill Implimentation
//
// Updates the health icon fill
//
// -----------------------------------------------/
void CHudMgr::UpdateHealthFill	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() || !g_pGameClientShell) return;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	static DFLOAT nDisHealth = (DFLOAT)pStats->GetHealthCount();

	// see if it is time to reset...
	if(pStats->GetResetHealth())
		nDisHealth = (DFLOAT)pStats->GetHealthCount();

	uint32	nHealth = pStats->GetHealthCount();
	uint32	nMaxHealth = pStats->GetMaxHealthCount();
	DRect   rcFill  = pHudElement->GetImgSrcRect();
	LTFLOAT fRate = nMaxHealth / g_cvarMeterRate.GetFloat();

	//incriment based on time
	if(nDisHealth < nHealth) 
	{
		nDisHealth += g_pGameClientShell->GetFrameTime()*fRate;

		// Check for overshoot
		if(nDisHealth > nHealth)
			nDisHealth = (LTFLOAT)nHealth;
	}
	if(nDisHealth > nHealth)
	{
		nDisHealth -= g_pGameClientShell->GetFrameTime()*fRate;

		// Check for overshoot
		if(nDisHealth < nHealth)
			nDisHealth = (LTFLOAT)nHealth;
	}
	
	//reset fill top
	rcFill.top += (uint32) ((1-nDisHealth/nMaxHealth)*rcFill.bottom);

	//save result
	pHudElement->SetDisplayRect(rcFill);
}

// -----------------------------------------------/
//
// void CHudMgr::UpdateArmorFill Implimentation
//
// Updates the armor icon fill
//
// -----------------------------------------------/
void CHudMgr::UpdateArmorFill	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() || !g_pGameClientShell) return;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	static DFLOAT nDisArmor = (DFLOAT)pStats->GetArmorCount();

	// see if it is time to reset...
	if(pStats->GetResetArmor())
		nDisArmor = (DFLOAT)pStats->GetArmorCount();

	uint32	nArmor = pStats->GetArmorCount();
	uint32	nMaxArmor = pStats->GetMaxArmorCount();
	if(nMaxArmor == 0) nMaxArmor = 1;
	DRect   rcFill  = pHudElement->GetImgSrcRect();
	LTFLOAT fRate = nMaxArmor / g_cvarMeterRate.GetFloat();

	//incriment based on time
	if(nDisArmor < nArmor) 
	{
		nDisArmor += g_pGameClientShell->GetFrameTime()*fRate;

		// Check for overshoot
		if(nDisArmor > nArmor)
			nDisArmor = (LTFLOAT)nArmor;
	}
	if(nDisArmor > nArmor)
	{
		nDisArmor -= g_pGameClientShell->GetFrameTime()*fRate;

		// Check for overshoot
		if(nDisArmor < nArmor)
			nDisArmor = (LTFLOAT)nArmor;
	}

	//reset fill top
	rcFill.top += (uint32) ((1-nDisArmor/nMaxArmor)*rcFill.bottom);

	//save result
	pHudElement->SetDisplayRect(rcFill);
}

// -----------------------------------------------/
//
// void CHudMgr::UpdateHealthStr Implimentation
//
// Updates the health string for numeric HUD dispaly
//
// -----------------------------------------------/
void CHudMgr::UpdateHealthStr	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() || !g_pGameClientShell) return;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	static	DFLOAT nDisHealth = (DFLOAT)pStats->GetHealthCount();

	// see if it is time to reset...
	if(pStats->GetResetHealth())
		nDisHealth = (DFLOAT)pStats->GetHealthCount();

	if(pHudElement->GetString())
	{
		//create new string
		char	szBuffer[12];

		uint32	nHealth = pStats->GetHealthCount();
		LTFLOAT fRate = pStats->GetMaxHealthCount() / g_cvarMeterRate.GetFloat();

		//incriment based on time
		if(nDisHealth < nHealth) 
		{
			nDisHealth += g_pGameClientShell->GetFrameTime()*fRate;

			// Check for overshoot
			if(nDisHealth > nHealth)
				nDisHealth = (LTFLOAT)nHealth;
		}
		if(nDisHealth > nHealth)
		{
			nDisHealth -= g_pGameClientShell->GetFrameTime()*fRate;

			// Check for overshoot
			if(nDisHealth < nHealth)
				nDisHealth = (LTFLOAT)nHealth;
		}

		//take integer value of result
		nHealth = (uint32)nDisHealth;

		//print string
		sprintf(szBuffer,"%.3d",nHealth);

		char *szStr = g_pLTClient->GetStringData(pHudElement->GetString());

		//save the results
		if(_stricmp(szBuffer, szStr) != 0)
		{
			HSTRING hStr = g_pLTClient->CreateString(szBuffer);
			pHudElement->SetString(hStr);
			g_pLTClient->FreeString(hStr);
		}
	}
	else
	{
		//first time through, set the initial string
		HSTRING hStr = g_pLTClient->CreateString("");
		pHudElement->SetString(hStr);
		g_pLTClient->FreeString(hStr);
	}
}

// -----------------------------------------------/
//
// void CHudMgr::UpdateFlareStr Implimentation
//
// Updates the flare string for numeric HUD dispaly
//
// -----------------------------------------------/
void CHudMgr::UpdateFlareStr	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() || !g_pGameClientShell) return;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	if(pHudElement->GetString())
	{
		//create new string
		char	szBuffer[12];
		//print string
		sprintf(szBuffer,"%.2d",pStats->GetMarineFlares());

		char *szStr = g_pLTClient->GetStringData(pHudElement->GetString());

		if(_stricmp(szBuffer, szStr) != 0)
		{
			HSTRING hStr = g_pLTClient->CreateString(szBuffer);
			pHudElement->SetString(hStr);
			g_pLTClient->FreeString(hStr);
		}
	}
	else
	{
		//first time through, set the initial string
		HSTRING hStr = g_pLTClient->CreateString("");
		pHudElement->SetString(hStr);
		g_pLTClient->FreeString(hStr);
	}
}

// -----------------------------------------------/
//
// void CHudMgr::UpdateArmorStr Implimentation
//
// Updates the armor string for numeric HUD dispaly
//
// -----------------------------------------------/
void CHudMgr::UpdateArmorStr	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() || !g_pGameClientShell) return;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	static DFLOAT nDisArmor = (DFLOAT)pStats->GetArmorCount();

	// see if it is time to reset...
	if(pStats->GetResetArmor())
		nDisArmor = (DFLOAT)pStats->GetArmorCount();

	if(pHudElement->GetString())
	{
		//create new string
		char szBuffer[12];

		uint32	nArmor = pStats->GetArmorCount();
		LTFLOAT fRate = pStats->GetMaxArmorCount() / g_cvarMeterRate.GetFloat();

		//incriment based on time
		if(nDisArmor < nArmor) 
		{
			nDisArmor += g_pGameClientShell->GetFrameTime()*fRate;

			// Check for overshoot
			if(nDisArmor > nArmor)
				nDisArmor = (LTFLOAT)nArmor;
		}
		if(nDisArmor > nArmor)
		{
			nDisArmor -= g_pGameClientShell->GetFrameTime()*fRate;

			// Check for overshoot
			if(nDisArmor < nArmor)
				nDisArmor = (LTFLOAT)nArmor;
		}

		//take integer value of result
		nArmor = (uint32)nDisArmor;

		//print string
		sprintf(szBuffer,"%.3d",nArmor);

		char *szStr = g_pLTClient->GetStringData(pHudElement->GetString());

		if(_stricmp(szBuffer, szStr) != 0)
		{
			HSTRING hStr = g_pLTClient->CreateString(szBuffer);
			pHudElement->SetString(hStr);
			g_pLTClient->FreeString(hStr);
		}
	}
	else
	{
		//first time through, set the initial string
		HSTRING hStr = g_pLTClient->CreateString("");
		pHudElement->SetString(hStr);
		g_pLTClient->FreeString(hStr);
	}
}

// -----------------------------------------------/
//
// void CHudMgr::UpdateMarineAmmoCount Implimentation
//
// Updates the ammo string for numeric HUD dispaly
//
// -----------------------------------------------/
void CHudMgr::UpdateMarineAmmoCount	(CHudElement *pHudElement)
{
	if (pHudElement)
		(this->*(m_pAmmoCounterFunction))(pHudElement);
}

// -----------------------------------------------/
//
// void CHudMgr::NormalCounter Implimentation
//
// Normal ammo counter HUD dispaly
//
// -----------------------------------------------/
void CHudMgr::NormalCounter	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() || !g_pGameClientShell || !m_pPlayerStats) return;

	for(int i=0 ; i<MAX_COUNTER_ITEMS ; i++)
	{
		char szTemp[10];

		switch (m_CounterItems[i].eType)
		{
		case (CT_INVALID): break;
		case(CT_AMMO_ONLY):
			{
				sprintf(szTemp, "%.*d",m_CounterItems[i].nNumDigits, m_pPlayerStats->GetAmmoCount(m_CounterItems[i].pAmmo->nId));
			} break;
		case(CT_CLIPS):
			{
				uint32 nAmmo		= m_pPlayerStats->GetAmmoCount(m_CounterItems[i].pAmmo->nId);
				uint32 nAmmoInClip	= m_pPlayerStats->GetAmmoInClip(m_CounterItems[i].pBarrel->nId);
				uint32 nClips		= nAmmo > nAmmoInClip?(nAmmo-nAmmoInClip) / m_CounterItems[i].pBarrel->nShotsPerClip:0;
				if( (nAmmo-nAmmoInClip) && ((nAmmo-nAmmoInClip) % m_CounterItems[i].pBarrel->nShotsPerClip) )
					nClips++;
				sprintf(szTemp, "%.*d",m_CounterItems[i].nNumDigits, nClips);
			} break;
		case(CT_CLIP):
			{
				sprintf(szTemp, "%.*d",m_CounterItems[i].nNumDigits, m_pPlayerStats->GetAmmoInClip(m_CounterItems[i].pBarrel->nId));
			} break;
		}

		char *szStr = g_pLTClient->GetStringData(m_CounterItems[i].hStr);

		if((m_CounterItems[i].eType != CT_INVALID) && (_stricmp(szTemp, szStr) != 0) )
		{
			//clean up the old string
			if(m_CounterItems[i].hStr)
			{
				g_pLTClient->FreeString(m_CounterItems[i].hStr);
				m_CounterItems[i].hStr = LTNULL;
			}

			m_CounterItems[i].hStr = g_pLTClient->CreateString(szTemp);
		}
	}
}

// -----------------------------------------------/
//
// void CHudMgr::MultiAmmoCounter Implimentation
//
// Multiple-ammo counter HUD dispaly
//
// -----------------------------------------------/
void CHudMgr::MultiAmmoCounter	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() || !g_pGameClientShell ) return;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	if(!pStats)
		return;

	BARREL* pBarrel = m_pBaseBarrel;
	BARREL* pActiveBarrel = LTNULL;

	//reset active barrel
	m_nActiveBarrel	= 0;

	CWeaponModel*	pWeap = g_pGameClientShell->GetWeaponModel();
	BARREL*			pPrimBarrel = pWeap->GetPrimBarrel();

	if(!m_nNumBarrels)
	{
		//set barrel count
		if(m_pBaseBarrel)
		{
			//get next barrel
			BARREL* pTemp = m_pBaseBarrel;

			//reset counter
			m_nNumBarrels = 0;

			//count
			do
			{
				m_nNumBarrels++;
				if(pTemp)
					pTemp = g_pWeaponMgr->GetBarrel(pTemp->szNextBarrel);
			}while(pTemp && pTemp != m_pBaseBarrel);
		}
	}

	//clean up old strings and set new ones
	for(uint32 i=0; i < m_nNumBarrels; i++)
	{
		//set active barrel
		if(pBarrel == pPrimBarrel)
			pActiveBarrel = pBarrel;

		//advance to the next barrel
		pBarrel = g_pWeaponMgr->GetBarrel(pBarrel->szNextBarrel);
	}

	for( i=0 ; i<MAX_COUNTER_ITEMS ; i++)
	{
		//clean up the onld string
		if(m_CounterItems[i].hStr)
		{
			if(m_CounterItems[i].pBarrel == pActiveBarrel)
				m_CounterItems[i].pFont = 1;
			else
				m_CounterItems[i].pFont = 3;
		}
	}

	NormalCounter(pHudElement);
}

// -----------------------------------------------/
//
// void CHudMgr::ExosuitAmmoCounter Implimentation
//
// Exosuit ammo counter HUD dispaly
//
// -----------------------------------------------/
void CHudMgr::ExosuitAmmoCounter(CHudElement *pHudElement)
{
	CWeaponModel* pWepModel = g_pGameClientShell->GetWeaponModel();

	WEAPON* pWep	= pWepModel->GetWeapon();
	WEAPON* pWep0	= g_pWeaponMgr->GetWeapon("Exosuit_Flame_Thrower");
	WEAPON* pWep1	= g_pWeaponMgr->GetWeapon("Exosuit_Minigun");

	if(pWep == pWep0)
	{
		m_CounterItems[0].pFont = 0;
		m_CounterItems[1].pFont = m_CounterItems[2].pFont = 1;
	}
	else
	{
		if(pWep == pWep1)
		{
			m_CounterItems[0].pFont = 1;
			m_CounterItems[1].pFont = m_CounterItems[2].pFont = 0;
		}
		else
		{
			m_CounterItems[0].pFont = m_CounterItems[1].pFont = m_CounterItems[2].pFont = 1;
		}
	}
	NormalCounter(pHudElement);
}


// -----------------------------------------------/
//
// void CHudMgr::UpdateAlienHealth Implimentation
//
// Updates the alien health HUD dispaly
//
// -----------------------------------------------/
void CHudMgr::UpdateAlienHealth	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() || !g_pGameClientShell) return;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	//local variables
	static DFLOAT nDisHealth = (DFLOAT)pStats->GetHealthCount();

	// see if it is time to reset...
	if(pStats->GetResetHealth())
		nDisHealth = (DFLOAT)pStats->GetHealthCount();

	uint32	nHealth = pStats->GetHealthCount();
	uint32  nMaxHealth = pStats->GetMaxHealthCount();
	uint32	nScreenWidth, nScreenHeight;
	DIntPt	ptDest;
	LTFLOAT fRate = nMaxHealth / g_cvarMeterRate.GetFloat();

	//get the dims of the screen
	g_pLTClient->GetSurfaceDims(	g_pLTClient->GetScreenSurface(), 
									&nScreenWidth, &nScreenHeight );

	//set new offset
	ptDest.x = (nScreenWidth>>1) - (pHudElement->GetImgSrcRect().right>>1);
	ptDest.y = nScreenHeight - (nScreenHeight>>4);
	pHudElement->SetDestPt(ptDest);

	//incriment based on time
	if(nDisHealth < nHealth) 
	{
		nDisHealth += g_pGameClientShell->GetFrameTime()*fRate;

		// Check for overshoot
		if(nDisHealth > nHealth)
			nDisHealth = (LTFLOAT)nHealth;
	}
	if(nDisHealth > nHealth)
	{
		nDisHealth -= g_pGameClientShell->GetFrameTime()*fRate;

		// Check for overshoot
		if(nDisHealth < nHealth)
			nDisHealth = (LTFLOAT)nHealth;
	}
	
	//set new scaling factors
	DFLOAT  fRatio = nDisHealth/nMaxHealth;
	pHudElement->SetScaleX(fRatio);
	pHudElement->SetScaleY(fRatio);
}

// -----------------------------------------------/
//
// void CHudMgr::UpdateMarineEKG Implimentation
//
// Marine Ekg display update
//
// -----------------------------------------------/
void CHudMgr::UpdateMarineEKG (CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() || !g_pGameClientShell) return;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	static uint32	nEKGHeight	= pHudElement->GetImgSrcRect().bottom;
	static uint32	nEKGWidth	= pHudElement->GetImgSrcRect().right;
	static uint32	nWindow		= nEKGWidth >> 1;
	static DFLOAT	fOffset		= 0;
	DFLOAT			fSpeed;
	uint32			nDisOffset;
	DRect			rcSrc;

	//calculate speed of animation based on player's health
	fSpeed = 100 - 85 * ((DFLOAT)pStats->GetHealthCount() / pStats->GetMaxHealthCount()) ;

	//update our offset
	nDisOffset = uint32 (fOffset += g_pGameClientShell->GetFrameTime() * fSpeed);
	
	//test for limits
	if(nDisOffset+nWindow >= nEKGWidth)
	{
		nDisOffset	= 0;
		fOffset		= 0;
	}

	//set our rect
	rcSrc.left	= nDisOffset;
	rcSrc.top	= 0;
	rcSrc.right = nDisOffset + nWindow;
	rcSrc.bottom= nEKGHeight;

	pHudElement->SetDisplayRect(rcSrc);
}

// -----------------------------------------------/
//
// void CHudMgr::NormalDrawTransparent Implimentation
//
// Normal Drawing runtine, using transparency
//
// -----------------------------------------------/
void CHudMgr::NormalDrawTransparent	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	//draw our element to the screen
	g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen, 
													pHudElement->GetSurface(), 
													&pHudElement->GetDisplayRect(),
													pHudElement->GetDestX(), 
													pHudElement->GetDestY(), 
													hTransColor);
}

// -----------------------------------------------/
//
// void CHudMgr::GetEKGSurface Implimentation
//
// Gets the proper drawing surface from the animation array.
//
// -----------------------------------------------/
HSURFACE CHudMgr::GetEKGSurface	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() || !g_pGameClientShell) return LTNULL;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	uint32	nHealth = pStats->GetHealthCount();
	uint32	nMaxHealth = pStats->GetMaxHealthCount();

	if(nMaxHealth != 0)
	{
		if(nHealth > ((nMaxHealth>>2)*3))
			return pHudElement->GetAnimSurf()[0];
		else if(nHealth > (nMaxHealth>>1))
			return pHudElement->GetAnimSurf()[1];
		else if(nHealth > (nMaxHealth>>2))
			return pHudElement->GetAnimSurf()[2];
	}
	return pHudElement->GetAnimSurf()[3];
}

// -----------------------------------------------/
//
// void CHudMgr::DrawMarineEKG Implimentation
//
// DrawMarineEKG Drawing runtine, using transparency
//
// -----------------------------------------------/
void CHudMgr::DrawMarineEKG	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	//draw our element to the screen
	g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen, 
													GetEKGSurface(pHudElement),//pHudElement->GetSurface(), 
													&pHudElement->GetDisplayRect(),
													pHudElement->GetDestX(), 
													pHudElement->GetDestY(), 
													hTransColor);

}

// -----------------------------------------------/
//
// void CHudMgr::NormalDrawNonTransparent Implimentation
//
// Normal Drawing runtine, not using transparency
//
// -----------------------------------------------/
void CHudMgr::NormalDrawNonTransparent	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//draw our element to the screen
	g_pLTClient->DrawSurfaceToSurface(	hScreen, 
										pHudElement->GetSurface(), 
										&pHudElement->GetDisplayRect(),
										pHudElement->GetDestX(), 
										pHudElement->GetDestY() );
}

// -----------------------------------------------/
//
// void CHudMgr::NormalDrawTransparent Implimentation
//
// Normal Drawing runtine, using transparency
//
// -----------------------------------------------/
void CHudMgr::DrawHealthArmorFill	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	//adjust draw offset
	int nDestY = pHudElement->GetDestY() + pHudElement->GetDisplayRect().top;

	//draw our element to the screen
	g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen, 
													pHudElement->GetSurface(), 
													&pHudElement->GetDisplayRect(),
													pHudElement->GetDestX(), 
													nDestY, 
													hTransColor);
}

// -----------------------------------------------/
//
// void CHudMgr::DrawBmFontText Implimentation
//
// Bitmap Font drawing routine
//
// -----------------------------------------------/
void CHudMgr::DrawBmFontText	(CHudElement *pHudElement)
{
	LITHFONTDRAWDATA lfDD;

	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set drawdata elements
	lfDD.byJustify		= LTF_JUSTIFY_LEFT;
	lfDD.nLetterSpace	= 1;
	lfDD.bForceFormat	= LTTRUE;

	//draw our text to the screen
	m_aplfFonts[pHudElement->GetFontIndex()]->Draw(	hScreen,
													pHudElement->GetString(),
													&lfDD, 
													pHudElement->GetDestX(), 
													pHudElement->GetDestY(),
													LTNULL);
}

// -----------------------------------------------/
//
// void CHudMgr::DrawMarineAmmoCounter Implimentation
//
// Bitmap Font drawing routine for ammo counter
//
// -----------------------------------------------/
void CHudMgr::DrawMarineAmmoCounter	(CHudElement *pHudElement)
{
	if(pHudElement)
		(this->*(m_pAmmoDrawFunction))(pHudElement);
}

// -----------------------------------------------/
//
// void CHudMgr::NormalDrawCounter Implimentation
//
// Bitmap Font drawing routine for normal ammo counter
//
// -----------------------------------------------/
void CHudMgr::NormalDrawCounter	(CHudElement *pHudElement)
{
	LITHFONTDRAWDATA lfDD;

	//sanity check
	if ( !g_pClientDE || !pHudElement || !pHudElement->GetEnabled() || !m_aplfFonts) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	//set drawdata elements
	lfDD.byJustify		= LTF_JUSTIFY_RIGHT;
	lfDD.nLetterSpace	= 1;
	lfDD.bForceFormat	= LTTRUE;

	for(int i=0 ; i<MAX_COUNTER_ITEMS ; i++)
	{
		if(m_CounterItems[i].hStr)
		{
			//draw our text to the screen
			m_aplfFonts[m_CounterItems[i].pFont]->Draw(	hScreen,
														m_CounterItems[i].hStr,
														&lfDD, 
														pHudElement->GetDestX()-m_CounterItems[i].ptStrLoc.x, 
														pHudElement->GetDestY()-m_CounterItems[i].ptStrLoc.y+1,
														LTNULL);
		}

		if(m_CounterItems[i].hIcon)
		{
			//draw the icon
			g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen, 
															m_CounterItems[i].hIcon, 
															LTNULL,
															pHudElement->GetDestX()-m_CounterItems[i].ptIconLoc.x, 
															pHudElement->GetDestY()-m_CounterItems[i].ptIconLoc.y,
															hTransColor);
		}
	}
}

// -----------------------------------------------/
//
// void CHudMgr::SpearAmmoDrawCounter Implimentation
//
// Bitmap drawing routine for Predator spear ammo counter
//
// -----------------------------------------------/
void CHudMgr::SpearAmmoDrawCounter	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pClientDE || !pHudElement || !pHudElement->GetEnabled() ) return;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	if(!pStats)	return;

	//test
	if(!m_pBaseBarrel) return;
	
	
	if(!pHudElement->GetAnimSurf()) return;


	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(m_pBaseBarrel->nAmmoType);

	if(!pAmmo) return;
		
	//get the ammo count
//	int nTotAmmo = pStats->GetAmmoCount(pAmmo->nId);

	//get handle to screen surface
	HSURFACE hScreen = g_pClientDE->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	//local variables
	uint32	nAmmo = pStats->GetAmmoCount(pAmmo->nId);

	int		nXPos = pHudElement->GetDestX();
	uint32	nWidth, nHeight;

	//get the dims of our surface
	g_pClientDE->GetSurfaceDims(pHudElement->GetAnimSurf()[0], &nWidth, &nHeight);

	g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen, 
													m_hAmmoIcon, 
													LTNULL,
													nXPos-40,
													pHudElement->GetDestY(), 
													hTransColor);

	for(uint32 i=0 ; i < nAmmo/9 ; i++)
	{
		//display number icon 10
		g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen, 
														pHudElement->GetAnimSurf()[8], 
														LTNULL,
														nXPos,
														pHudElement->GetDestY(), 
														hTransColor);

		//increment position by one icon height
		nXPos += nWidth+4;
	}

	//check for remainder
	uint32 nRem = nAmmo%9;

	if(nRem)
	{
		//display number icon 10
		g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen, 
														pHudElement->GetAnimSurf()[nRem-1], 
														LTNULL,
														nXPos,
														pHudElement->GetDestY(), 
														hTransColor);

	}
}


// -----------------------------------------------/
//
// void CHudMgr::DrawSkullCounter Implimentation
//
// Bitmap drawing routine for Predator skull counter
//
// -----------------------------------------------/
void CHudMgr::DrawSkullCounter	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pClientDE || !pHudElement || !pHudElement->GetEnabled() ) return;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	if(!pStats)	return;
	
	if(!pHudElement->GetAnimSurf()) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pClientDE->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	//local variables
	uint32	nCount = pStats->GetSkullCount();

	int		nXPos = pHudElement->GetDestX();
	uint32	nWidth, nHeight;

	//get the dims of our surface
	g_pClientDE->GetSurfaceDims(pHudElement->GetAnimSurf()[0], &nWidth, &nHeight);

	g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen, 
													pHudElement->GetSurface(), 
													LTNULL,
													nXPos-40,
													pHudElement->GetDestY(), 
													hTransColor);

	for(uint32 i=0 ; i < nCount/9 ; i++)
	{
		//display number icon 10
		g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen, 
														pHudElement->GetAnimSurf()[8], 
														LTNULL,
														nXPos,
														pHudElement->GetDestY(), 
														hTransColor);

		//increment position by one icon height
		nXPos += nWidth+4;
	}

	//check for remainder
	uint32 nRem = nCount%9;

	if(nRem)
	{
		//display number icon 10
		g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen, 
														pHudElement->GetAnimSurf()[nRem-1], 
														LTNULL,
														nXPos,
														pHudElement->GetDestY(), 
														hTransColor);

	}
}


// -----------------------------------------------/
//
// void CHudMgr::DrawBossHealth Implimentation
//
// Boss Health Bar drawing routine
//
// -----------------------------------------------/
void CHudMgr::DrawBossHealth (CHudElement *pHudElement)
{
	//sanity check
	if (!g_pLTClient || !pHudElement || !pHudElement->GetEnabled())
		return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	if (m_bBossHealthOn)
	{
		g_pLTClient->TransformSurfaceToSurfaceTransparent(	hScreen,
															pHudElement->GetSurface(),
															LTNULL,
															pHudElement->GetDestX(),
															pHudElement->GetDestY(),
															0,
															pHudElement->GetScaleX(),
															pHudElement->GetScaleY(),
															hTransColor);
	}
}

// -----------------------------------------------/
//
// void CHudMgr::DrawBossHealthFill Implimentation
//
// Boss Health Display drawing routine
//
// -----------------------------------------------/

void CHudMgr::DrawBossHealthFill(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	if (m_bBossHealthOn)
	{
		//get the dims of our surface
		uint32	nWidth, nHeight;
		g_pLTClient->GetSurfaceDims(pHudElement->GetSurface(), &nWidth, &nHeight);

		LTRect srcRect;
		srcRect.left = srcRect.top = 0;
		srcRect.right = nWidth;
		srcRect.bottom = nHeight;

		uint32 xOffset = nWidth  - (int)(nWidth * m_fBossHealthRatio);
		srcRect.right -= xOffset;

		g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen,
														pHudElement->GetSurface(),
														&srcRect,
														pHudElement->GetDestX(),
														pHudElement->GetDestY(),
														hTransColor);
	}
}

// -----------------------------------------------/
//
// void CHudMgr::DrawAlienHealth Implimentation
//
// Alien Health Bar drawing routine
//
// -----------------------------------------------/
void CHudMgr::DrawAlienHealth	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	g_pLTClient->TransformSurfaceToSurfaceTransparent(	hScreen,
														pHudElement->GetSurface(),
														LTNULL,
														pHudElement->GetDestX(),
														pHudElement->GetDestY(),
														0,
														pHudElement->GetScaleX(),
														pHudElement->GetScaleY(),
														hTransColor);
}

// -----------------------------------------------/
//
// void CHudMgr::DrawChestbursterHealth Implimentation
//
// Alien Health Bar drawing routine
//
// -----------------------------------------------/
void CHudMgr::DrawChestbursterHealth	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	static LTFLOAT fFlashStartTime = 0.0f;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	if(fFlashStartTime == 0.0f)
	{
		//see if it's time to start flashing
		HOBJECT hObj = g_pLTClient->GetClientObject();
		LTFLOAT fScale;

		g_pLTClient->GetSConValueFloat("ChestbursterMaxScale", fScale);

		LTVector vScale;
		g_pLTClient->GetObjectScale(hObj, &vScale);

		if(vScale.x == fScale)
			fFlashStartTime = g_pLTClient->GetTime();


		g_pLTClient->TransformSurfaceToSurfaceTransparent(	hScreen,
															pHudElement->GetSurface(),
															LTNULL,
															pHudElement->GetDestX(),
															pHudElement->GetDestY(),
															0,
															pHudElement->GetScaleX(),
															pHudElement->GetScaleY(),
															hTransColor);
	}
	else
	{
		// If we're in multiplayer... let the character know we're about to mutate
		if(g_pGameClientShell->GetGameType()->IsMultiplayer())
		{
			//flashing
			LTFLOAT fCurTime = g_pLTClient->GetTime();
			LTFLOAT fTimeDelta = fCurTime - fFlashStartTime;

			if(fTimeDelta > g_cvarFlashCycleTime.GetFloat())
			{
				//reset the flashing
				fFlashStartTime = fCurTime;

				//re-set alpha
				g_pClientDE->SetSurfaceAlpha (pHudElement->GetSurface(), 1.0f);
			}
			else
			{
				LTFLOAT fAlpha;
				LTFLOAT fRatio = fTimeDelta / g_cvarFlashCycleTime.GetFloat();

				if(fRatio > 0.5f)
				{
					//fading in
					fAlpha = (fRatio * 1.5f) - 0.75f;
				}
				else
				{
					//fading out
					fAlpha = 0.75f - (fRatio * 1.5f);
				}

				g_pClientDE->SetSurfaceAlpha (pHudElement->GetSurface(), fAlpha);
			}
		}
		else
		{
			g_pClientDE->SetSurfaceAlpha (pHudElement->GetSurface(), 0.75f);
		}

		g_pLTClient->TransformSurfaceToSurfaceTransparent(	hScreen,
															pHudElement->GetSurface(),
															LTNULL,
															pHudElement->GetDestX(),
															pHudElement->GetDestY(),
															0,
															pHudElement->GetScaleX(),
															pHudElement->GetScaleY(),
															hTransColor);
	}
}

// -----------------------------------------------/
//
// void CHudMgr::DrawPredatorHealth Implimentation
//
// Predator Health Display drawing routine
//
// -----------------------------------------------/
void CHudMgr::DrawPredatorHealth	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	//local variables
	static DFLOAT nDisHealth = (DFLOAT)pStats->GetHealthCount();

	// see if it is time to reset...
	if(pStats->GetResetHealth())
		nDisHealth = (DFLOAT)pStats->GetHealthCount();

	uint32	nHealth = pStats->GetHealthCount();
	uint32  nMaxHealth = pStats->GetMaxHealthCount();
	if(nMaxHealth == 0) nMaxHealth=1;
	LTFLOAT fRate = nMaxHealth / g_cvarMeterRate.GetFloat();

	//incriment based on time
	if(nDisHealth < nHealth) 
	{
		nDisHealth += g_pGameClientShell->GetFrameTime()*fRate;

		// Check for overshoot
		if(nDisHealth > nHealth)
			nDisHealth = (LTFLOAT)nHealth;
	}
	if(nDisHealth > nHealth)
	{
		nDisHealth -= g_pGameClientShell->GetFrameTime()*fRate;

		// Check for overshoot
		if(nDisHealth < nHealth)
			nDisHealth = (LTFLOAT)nHealth;
	}

	//TEMP convert to base 60
	uint32 nTemp = uint32(60 * nDisHealth/nMaxHealth);

	int		nYPos = pHudElement->GetDestY();
	uint32	nWidth, nHeight;

	//get the dims of our surface
	g_pLTClient->GetSurfaceDims(pHudElement->GetAnimSurf()[0], &nWidth, &nHeight);


	for(uint32 i=0 ; i < nTemp/10 ; i++)
	{
		//display number icon 10
		g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen, 
														pHudElement->GetAnimSurf()[8], 
														LTNULL,
														pHudElement->GetDestX(), 
														nYPos, 
														hTransColor);

		//increment position by one icon height
		nYPos += nHeight+4;
	}

	//check for remainder
	uint32 nRem = nTemp%10;

	if(nRem)
	{
		//display number icon 10
		g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen, 
														pHudElement->GetAnimSurf()[nRem-1], 
														LTNULL,
														pHudElement->GetDestX(), 
														nYPos, 
														hTransColor);

	}

}

// -----------------------------------------------/
//
// void CHudMgr::DrawPredatorEnergy Implimentation
//
// Predator Energy Display drawing routine
//
// -----------------------------------------------/
//TEMP change from armor display to energy when stats available
void CHudMgr::DrawPredatorEnergy	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	//local variables
	static DFLOAT nDisEnergy = (DFLOAT)pStats->GetPredatorEnergy();
	uint32	nEnergy = pStats->GetPredatorEnergy();
	uint32  nMaxEnergy = pStats->GetMaxPredatorEnergy();
	LTFLOAT fRate = nMaxEnergy / g_cvarMeterRate.GetFloat();

	//incriment based on time
	if(nDisEnergy < nEnergy) 
	{
		nDisEnergy += g_pGameClientShell->GetFrameTime()*fRate;

		// Check for overshoot
		if(nDisEnergy > nEnergy)
			nDisEnergy = (LTFLOAT)nEnergy;
	}
	if(nDisEnergy > nEnergy)
	{
		nDisEnergy -= g_pGameClientShell->GetFrameTime()*fRate;

		// Check for overshoot
		if(nDisEnergy < nEnergy)
			nDisEnergy = (LTFLOAT)nEnergy;
	}

	uint32 nTemp = (uint32)nDisEnergy;

	int		nYPos = pHudElement->GetDestY();
	uint32	nWidth, nHeight;

	//get the dims of our surface
	g_pLTClient->GetSurfaceDims(pHudElement->GetAnimSurf()[0], &nWidth, &nHeight);


	for(uint32 i=0 ; i < nTemp/9 ; i++)
	{
		//display number icon 10
		g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen, 
														pHudElement->GetAnimSurf()[8], 
														LTNULL,
														pHudElement->GetDestX(), 
														nYPos, 
														hTransColor);

		//increment position by one icon height
		nYPos += nHeight+4;
	}

	//check for remainder
	uint32 nRem = nTemp%9;

	if(nRem)
	{
		//display number icon 10
		g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen, 
														pHudElement->GetAnimSurf()[nRem-1], 
														LTNULL,
														pHudElement->GetDestX(), 
														nYPos, 
														hTransColor);

	}

}

// -----------------------------------------------/
//
// void CHudMgr::DrawBatteryMeter Implimentation
//
// Marine Battery Display drawing routine
//
// -----------------------------------------------/

void CHudMgr::DrawBatteryMeter(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	//local variables
	LTFLOAT fBattLevel = pStats->GetBatteryLevel();

	if(fBattLevel != MARINE_MAX_BATTERY_LEVEL)
	{
		g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen,
														pHudElement->GetSurface(),
														LTNULL,
														pHudElement->GetDestX(),
														pHudElement->GetDestY(),
														hTransColor);
	}
}

// -----------------------------------------------/
//
// void CHudMgr::DrawBatteryMeterFill Implimentation
//
// Marine Battery Display drawing routine
//
// -----------------------------------------------/

void CHudMgr::DrawBatteryMeterFill(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	//local variables
	LTFLOAT fRatio = pStats->GetBatteryLevel() / MARINE_MAX_BATTERY_LEVEL;

	if(fRatio != 1.0f)
	{
		static LTFLOAT	fStartTime = 0.0f;
		static LTBOOL	bRed = LTFALSE;

		//get the dims of our surface
		HSURFACE hSurf;

		if(fRatio > 0.5f)
			hSurf = pHudElement->GetAnimSurf()[0];
		else if(fRatio > 0.25)
			hSurf = pHudElement->GetAnimSurf()[1];
		else
		{
			hSurf = bRed?pHudElement->GetAnimSurf()[2]:pHudElement->GetAnimSurf()[3];

			//see if it's time to swap
			if(g_pLTClient->GetTime() - fStartTime > 0.2f)
			{
				bRed = !bRed;
				fStartTime = g_pLTClient->GetTime();
			}
		}

		uint32	nWidth, nHeight;
		g_pLTClient->GetSurfaceDims(hSurf, &nWidth, &nHeight);

		LTRect srcRect;
		srcRect.left = srcRect.top = 0;
		srcRect.right = nWidth;
		srcRect.bottom = nHeight;

		uint32 yOffset = nHeight - (int)(nHeight * fRatio);
		srcRect.top += yOffset;

		//display number icon 10
		g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen,
														hSurf,
														&srcRect,
														pHudElement->GetDestX(),
														pHudElement->GetDestY() + yOffset,
														hTransColor);
	}
}

// -----------------------------------------------/
//
// void CHudMgr::DrawAirMeter Implimentation
//
// Marine Air Display drawing routine
//
// -----------------------------------------------/

void CHudMgr::DrawAirMeter(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	//local variables
	LTFLOAT fAirPercent = pStats->GetAirPercent();

	if(fAirPercent < 1.0f)
	{
		g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen,
														pHudElement->GetSurface(),
														LTNULL,
														pHudElement->GetDestX(),
														pHudElement->GetDestY(),
														hTransColor);
	}
}

// -----------------------------------------------/
//
// void CHudMgr::DrawMarineAirMeterFill Implimentation
//
// Marine Air Display drawing routine
//
// -----------------------------------------------/

void CHudMgr::DrawMarineAirMeterFill(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	//local variables
	LTFLOAT fAirPercent = pStats->GetAirPercent();

	if(fAirPercent < 1.0f)
	{
		//get the dims of our surface
		uint32	nWidth, nHeight;
		g_pLTClient->GetSurfaceDims(pHudElement->GetSurface(), &nWidth, &nHeight);

		LTRect srcRect;
		srcRect.left = srcRect.top = 0;
		srcRect.right = nWidth;
		srcRect.bottom = nHeight;

		uint32 yOffset = nHeight - (int)(nHeight * fAirPercent);
		srcRect.top += yOffset;

		//display number icon 10
		g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen,
														pHudElement->GetSurface(),
														&srcRect,
														pHudElement->GetDestX(),
														pHudElement->GetDestY() + yOffset,
														hTransColor);
	}
}

// -----------------------------------------------/
//
// void CHudMgr::DrawPredatorAirMeterFill Implimentation
//
// Marine Air Display drawing routine
//
// -----------------------------------------------/

void CHudMgr::DrawPredatorAirMeterFill(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	//local variables
	LTFLOAT fAirPercent = pStats->GetAirPercent();

	if(fAirPercent < 1.0f)
	{
		//get the dims of our surface
		uint32	nWidth, nHeight;
		g_pLTClient->GetSurfaceDims(pHudElement->GetSurface(), &nWidth, &nHeight);

		LTRect srcRect;
		srcRect.left = srcRect.top = 0;
		srcRect.right = nWidth;
		srcRect.bottom = nHeight;

		uint32 xOffset = nWidth - (int)(nWidth * fAirPercent);
		srcRect.left += xOffset;

		//display number icon 10
		g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen,
														pHudElement->GetSurface(),
														&srcRect,
														pHudElement->GetDestX() + xOffset,
														pHudElement->GetDestY(),
														hTransColor);
	}
}

// -----------------------------------------------/
//
// void CHudMgr::DrawMarinePickupList Implimentation
//
// Draw the pickup list
//
// -----------------------------------------------/

void CHudMgr::DrawMarinePickupList(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	int nNumToDisplay = g_cvarNumIcons.GetFloat() > MAX_PICKUP_LIST ? 0 : (int)g_cvarNumIcons.GetFloat();

	for(int i=0; i<nNumToDisplay ; i++)
	{
		if(m_PickupList[i].bUsed)
			//display number icon
			g_pLTClient->DrawSurfaceToSurfaceTransparent
				(	hScreen,
					m_PickupList[i].hSurface,
					LTNULL,
					m_PickupList[i].ptLocation.x,
					m_PickupList[i].ptLocation.y,
					hTransColor);
	}
}

// -----------------------------------------------/
//
// void CHudMgr::DrawMarinePickupList Implimentation
//
// Draw the pickup list
//
// -----------------------------------------------/

void CHudMgr::DrawPredatorPickupList(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	int nNumToDisplay = g_cvarNumIcons.GetFloat() > MAX_PICKUP_LIST ? MAX_PICKUP_LIST : (int)g_cvarNumIcons.GetFloat();

	for(int i=0; i<nNumToDisplay ; i++)
	{
		if(m_PickupList[i].bUsed)
			//display number icon
			g_pLTClient->DrawSurfaceToSurfaceTransparent
				(	hScreen,
					m_PickupList[i].hSurface,
					LTNULL,
					m_PickupList[i].ptLocation.x,
					m_PickupList[i].ptLocation.y,
					hTransColor);
	}
}

// -----------------------------------------------/
//
// void CHudMgr::AddHealthPickup Implimentation
//
// Add a pickup item to the list
//
// -----------------------------------------------/

void CHudMgr::AddHealthPickup ()
{
	//add the icon to the list
	AddPickupIcon("Interface\\StatusBar\\Marine\\health_pickup.pcx");
}

// -----------------------------------------------/
//
// void CHudMgr::AddMaskPickup Implimentation
//
// Add a pickup item to the list
//
// -----------------------------------------------/

void CHudMgr::AddMaskPickup ()
{
	//add the icon to the list
	AddPickupIcon("Interface\\StatusBar\\Predator\\mask_pickup.pcx");
}

// -----------------------------------------------/
//
// void CHudMgr::AddNightVisionPickup Implimentation
//
// Add a pickup item to the list
//
// -----------------------------------------------/

void CHudMgr::AddNightVisionPickup ()
{
	//add the icon to the list
	AddPickupIcon("Interface\\StatusBar\\Marine\\nightvision_pickup.pcx");
}

// -----------------------------------------------/
//
// void CHudMgr::AddHealthPickup Implimentation
//
// Add a pickup item to the list
//
// -----------------------------------------------/

void CHudMgr::AddCloakPickup ()
{
	//add the icon to the list
	AddPickupIcon("Interface\\StatusBar\\Predator\\cloak_pickup.pcx");
}

// -----------------------------------------------/
//
// void CHudMgr::AddArmorPickup Implimentation
//
// Add a pickup item to the list
//
// -----------------------------------------------/

void CHudMgr::AddArmorPickup ()
{
	//add the icon to the list
	AddPickupIcon("Interface\\StatusBar\\Marine\\armor_pickup.pcx");
}

// -----------------------------------------------/
//
// void CHudMgr::AddAmmoPickup Implimentation
//
// Add a pickup item to the list
//
// -----------------------------------------------/

void CHudMgr::AddAmmoPickup (uint32 nAmmoPoolId)
{
	//get the ammo pool info
	AMMO_POOL *pPool = g_pWeaponMgr->GetAmmoPool(nAmmoPoolId);

	if(!pPool) return;

	//add the icon to the list
	AddPickupIcon(pPool->szPickupIcon);
}

// -----------------------------------------------/
//
// void CHudMgr::AddWeaponPickup Implimentation
//
// Add a pickup item to the list
//
// -----------------------------------------------/

void CHudMgr::AddWeaponPickup (uint32 nWepId)
{
	//get the ammo pool info
	WEAPON *pWep = g_pWeaponMgr->GetWeapon(nWepId);

	if(!pWep) return;

	//add the icon to the list
	AddPickupIcon(pWep->szPickupIcon);
}

// -----------------------------------------------/
//
// void CHudMgr::AddPickupIcon Implimentation
//
// Add a pickup item to the list
//
// -----------------------------------------------/

void CHudMgr::AddPickupIcon (char *szIcon)
{
	if(!m_pPlayerStats)
		return;

	//set some info
	PickupList NewItem;

	LTBOOL bIsPred = IsPredator(m_pPlayerStats->GetButeSet());

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	NewItem.hSurface = g_pInterfaceResMgr->GetSharedSurface(szIcon);

	if(!NewItem.hSurface) return;

//	g_pClientDE->OptimizeSurface (NewItem.hSurface, hTransColor);
	g_pClientDE->SetSurfaceAlpha (NewItem.hSurface, 1.0f);

	NewItem.bUsed = LTTRUE;
	NewItem.fStartTime = g_pLTClient->GetTime();

	//get info on the screen and the icon
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	uint32 nScreenWidth, nScreenHeight, nIconWidth, nIconHeight;

	g_pClientDE->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);
	g_pClientDE->GetSurfaceDims (NewItem.hSurface, &nIconWidth, &nIconHeight);

	if(g_cvarHorizIcons.GetFloat() != 0)
		//horizontal layout
		NewItem.ptLocation.x = (nScreenWidth>>2);
	else
		//vertical layout
		NewItem.ptLocation.x = (nScreenWidth>>1) - (nIconWidth>>1);

	if(bIsPred)
		NewItem.ptLocation.y = nScreenHeight - (int)g_cvarIconOffset.GetFloat() - nIconHeight;
	else
		NewItem.ptLocation.y = (int)g_cvarIconOffset.GetFloat() + 40;

	//now update the list
	for(int i=(MAX_PICKUP_LIST-1) ; i ; --i)
	{
		m_PickupList[i] = m_PickupList[i-1];

		if(m_PickupList[i].bUsed)
		{
			if(g_cvarHorizIcons.GetFloat() != 0)
			{
				//update for horizontal layout
				m_PickupList[i].ptLocation.x += nIconWidth+(int)g_cvarIconOffset.GetFloat();
			}
			else
			{
				if(bIsPred)
					//update for vertical layout
					m_PickupList[i].ptLocation.y -= nIconHeight+(int)g_cvarIconOffset.GetFloat();
				else
					//update for vertical layout
					m_PickupList[i].ptLocation.y += nIconHeight+(int)g_cvarIconOffset.GetFloat();
			}
		}
	}
	m_PickupList[0] = NewItem;
}

// -----------------------------------------------/
//
// void CHudMgr::UpdateMarinePickupList Implimentation
//
// Update the pickup list
//
// -----------------------------------------------/

void CHudMgr::UpdatePickupList(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	for(int i=0 ; i<MAX_PICKUP_LIST ; i++)
	{
		if(m_PickupList[i].bUsed)
		{
			LTFLOAT fFadeTime	= GetConsoleFloat("PickupIconDuration",5.0f);
			LTFLOAT	fTimeDelta	= g_pLTClient->GetTime() - m_PickupList[i].fStartTime;
			//see if it's time to go away
			if( fTimeDelta > fFadeTime )
			{
				m_PickupList[i].Reset();
			}
			else
			{
				//set the alpha
				g_pClientDE->SetSurfaceAlpha (m_PickupList[i].hSurface, 1-fTimeDelta/fFadeTime);
			}
		}
	}
}

// -----------------------------------------------/
//
// void CHudMgr::OnObjectMove Implimentation
//
// Updates the moving object list for the marine MD
//
// -----------------------------------------------/
void CHudMgr::OnObjectMove(HOBJECT hObj, LTBOOL bTeleport, DVector vPos)
{
	if(m_pMotionDetector)
		m_pMotionDetector->OnObjectMove(hObj, bTeleport, vPos);
}


// -----------------------------------------------/
//
// void CHudMgr::OnObjectRemove Implimentation
//
// Handles removing objects
//
// -----------------------------------------------/
void CHudMgr::OnObjectRemove(HLOCALOBJ hObj)
{
	if(m_pMotionDetector)
		m_pMotionDetector->OnObjectRemove(hObj);
}

// -----------------------------------------------/
//
// void CHudMgr::SetTrackerObject Implimentation
//
// Sets Up the tracker object
//
// -----------------------------------------------/
void CHudMgr::SetTrackerObject(HMESSAGEREAD hMessage)
{
	LTVector vPos;
	g_pLTClient->ReadFromMessageVector(hMessage, &vPos);

	if(m_pMotionDetector)
		m_pMotionDetector->SetTrackerObject(vPos);
}

// -----------------------------------------------/
//
// void CHudMgr::RemoveTracker Implimentation
//
// Removes the tracker object
//
// -----------------------------------------------/
void CHudMgr::RemoveTracker()
{
	if(m_pMotionDetector)
		m_pMotionDetector->RemoveTracker();
}

// utility function
HSURFACE CreateCounterIcon(char* szImage)
{
	//set transparent color
	HSURFACE rVal = g_pInterfaceResMgr->GetSharedSurface(szImage);

	if(rVal)
		g_pClientDE->SetSurfaceAlpha (rVal, 1.0f);

	return rVal;
}

// -----------------------------------------------/
//
// void CHudMgr::InitMultiCounter Implimentation
//
// Sets Up the ammo counter
//
// -----------------------------------------------/
void CHudMgr::InitMultiCounter(WEAPON* pNewWeapon)
{
	//clean up the old counter items
	for(uint32 i=0; i<MAX_COUNTER_ITEMS ; ++i)
		m_CounterItems[i].Reset();

	BARREL* pBarrel = m_pBaseBarrel;

	//reset active barrel
	m_nActiveBarrel	= 0;

	int j=0;

	int nOffset = m_nNumBarrels*40;

	//clean up old strings and set new ones
	for( i = 0; i < m_nNumBarrels; i++)
	{
		AMMO*		pAmmo = g_pWeaponMgr->GetAmmo(pBarrel->nAmmoType);
		AMMO_POOL*	pPool = g_pWeaponMgr->GetAmmoPool(pAmmo->szAmmoPool);

		if(pBarrel->nShotsPerClip)
		{
			//record the pointers
			m_CounterItems[j+1].pBarrel	= m_CounterItems[j].pBarrel	= pBarrel;
			m_CounterItems[j+1].pAmmo	= m_CounterItems[j].pAmmo	= pAmmo;
			m_CounterItems[j+1].pPool	= m_CounterItems[j].pPool	= pPool;
			m_CounterItems[j].eType		= CT_CLIPS;
			m_CounterItems[j+1].eType	= CT_CLIP;

			//yep lets set up a clip counter
			m_CounterItems[j].hIcon		= CreateCounterIcon(pBarrel->szClipIcon);
			m_CounterItems[j+1].hIcon	= CreateCounterIcon(pPool->szCounterIcon);

			uint32 nMaxClips = pPool->GetMaxAmount(g_pLTClient->GetClientObject())/pBarrel->nShotsPerClip;

			char temp[10];

			sprintf(temp,"%d", nMaxClips);
			m_CounterItems[j].nNumDigits = strlen(temp);

			sprintf(temp,"%d", pBarrel->nShotsPerClip);
			m_CounterItems[j+1].nNumDigits = strlen(temp);
			
			if(pBarrel == m_pBaseBarrel)
				m_CounterItems[j].pFont = m_CounterItems[j+1].pFont = 1;
			else
				m_CounterItems[j].pFont = m_CounterItems[j+1].pFont = 3;

			m_CounterItems[j].ptIconLoc = LTIntPt(75+15*(m_CounterItems[j].nNumDigits+1)/2,60+(i*50));
			m_CounterItems[j].ptStrLoc	= LTIntPt(75,36+(i*50));
			m_CounterItems[j+1].ptIconLoc	= LTIntPt(15+15*(m_CounterItems[j+1].nNumDigits+1)/2,60+(i*50));
			m_CounterItems[j+1].ptStrLoc	= LTIntPt(15,36+(i*50));

			
//			m_CounterItems[j].ptIconLoc = LTIntPt((nOffset-(i*50))+15*(m_CounterItems[j].nNumDigits+1)/2,120);
//			m_CounterItems[j].ptStrLoc	= LTIntPt((nOffset-(i*50)),97);
//			m_CounterItems[j+1].ptIconLoc	= LTIntPt((nOffset-(i*50))+15*(m_CounterItems[j+1].nNumDigits+1)/2,60);
//			m_CounterItems[j+1].ptStrLoc	= LTIntPt((nOffset-(i*50)),36);
			j+=2;
		}
		else
		{
			//record the pointers
			m_CounterItems[j].pBarrel	= pBarrel;
			m_CounterItems[j].pAmmo		= pAmmo;
			m_CounterItems[j].pPool		= pPool;

			m_CounterItems[j].eType		= CT_AMMO_ONLY;

			//alt parrel doesnt use clips
			m_CounterItems[j].hIcon = CreateCounterIcon(pPool->szCounterIcon);

			uint32 nMaxAmmo = pPool->GetMaxAmount(g_pLTClient->GetClientObject());

			char temp[10];

			sprintf(temp,"%d", nMaxAmmo);
			m_CounterItems[j].nNumDigits = strlen(temp);

			if(pBarrel == m_pBaseBarrel)
				m_CounterItems[j].pFont = 1;
			else
				m_CounterItems[j].pFont = 3;

			m_CounterItems[j].ptIconLoc	= LTIntPt((nOffset-(i*50))+15*(m_CounterItems[j].nNumDigits+1)/2,60);
			m_CounterItems[j].ptStrLoc	= LTIntPt((nOffset-(i*50)),36);
			j++;
		}

		//advance to the next barrel
		pBarrel = g_pWeaponMgr->GetBarrel(pBarrel->szNextBarrel);
	}
}

// -----------------------------------------------/
//
// void CHudMgr::InitExoCounter Implimentation
//
// Sets Up the ammo counter
//
// -----------------------------------------------/
void CHudMgr::InitExoCounter()
{
	//clean up the old counter items
	for(int i=0; i<MAX_COUNTER_ITEMS ; ++i)
		m_CounterItems[i].Reset();

	//reset active barrel
	m_nActiveBarrel	= 0;

	BARREL*		pBarrel0	= g_pWeaponMgr->GetBarrel("Exosuit_Flame_Thrower_Barrel");
	AMMO*		pAmmo0		= g_pWeaponMgr->GetAmmo(pBarrel0->nAmmoType);
	AMMO_POOL*	pPool0		= g_pWeaponMgr->GetAmmoPool(pAmmo0->szAmmoPool);

	BARREL*		pBarrel1	= g_pWeaponMgr->GetBarrel("Exosuit_Minigun_Barrel");
	AMMO*		pAmmo1		= g_pWeaponMgr->GetAmmo(pBarrel1->nAmmoType);
	AMMO_POOL*	pPool1		= g_pWeaponMgr->GetAmmoPool(pAmmo1->szAmmoPool);

	BARREL*		pBarrel2	= g_pWeaponMgr->GetBarrel("Exosuit_Rocket_Barrel");
	AMMO*		pAmmo2		= g_pWeaponMgr->GetAmmo(pBarrel2->nAmmoType);
	AMMO_POOL*	pPool2		= g_pWeaponMgr->GetAmmoPool(pAmmo2->szAmmoPool);


	//record the pointers
	m_CounterItems[0].pBarrel = pBarrel0;
	m_CounterItems[1].pBarrel = pBarrel1;
	m_CounterItems[2].pBarrel = pBarrel2;

	m_CounterItems[0].pAmmo	= pAmmo0;
	m_CounterItems[1].pAmmo	= pAmmo1;
	m_CounterItems[2].pAmmo	= pAmmo2;

	m_CounterItems[0].pPool	= pPool0;
	m_CounterItems[1].pPool	= pPool1;
	m_CounterItems[2].pPool	= pPool2;

	m_CounterItems[0].eType	= CT_AMMO_ONLY;
	m_CounterItems[1].eType	= CT_AMMO_ONLY;
	m_CounterItems[2].eType	= CT_AMMO_ONLY;

	m_CounterItems[0].hIcon	= CreateCounterIcon(pPool0->szCounterIcon);
	m_CounterItems[1].hIcon	= CreateCounterIcon(pPool1->szCounterIcon);
	m_CounterItems[2].hIcon	= CreateCounterIcon(pPool2->szCounterIcon);

	m_CounterItems[0].pFont = 0;
	m_CounterItems[1].pFont = 1;
	m_CounterItems[2].pFont = 1;

	uint32 nMaxAmmo0 = pPool0->GetMaxAmount(g_pLTClient->GetClientObject());
	uint32 nMaxAmmo1 = pPool1->GetMaxAmount(g_pLTClient->GetClientObject());
	uint32 nMaxAmmo2 = pPool2->GetMaxAmount(g_pLTClient->GetClientObject());

	char temp[10];

	sprintf(temp,"%d", nMaxAmmo0); m_CounterItems[0].nNumDigits = strlen(temp);
	sprintf(temp,"%d", nMaxAmmo1); m_CounterItems[1].nNumDigits = strlen(temp);
	sprintf(temp,"%d", nMaxAmmo2); m_CounterItems[2].nNumDigits = strlen(temp);

	for(i=0 ; i<3 ; i++)
	{
		m_CounterItems[i].ptIconLoc = LTIntPt((-i*60)+15*(m_CounterItems[0].nNumDigits+1)/2,30);
		m_CounterItems[i].ptStrLoc	= LTIntPt((-i*60),0);
	}
}

// -----------------------------------------------/
//
// void CHudMgr::InitNormalCounter Implimentation
//
// Sets Up the ammo counter
//
// -----------------------------------------------/
void CHudMgr::InitNormalCounter	(WEAPON* pNewWeapon)
{
	//clean up the old counter items
	for(int i=0; i<MAX_COUNTER_ITEMS ; ++i)
		m_CounterItems[i].Reset();

	//get the barrel info
	BARREL* pPrimBarrel = g_pWeaponMgr->GetBarrel(pNewWeapon->aBarrelTypes[PRIMARY_BARREL]);
	BARREL* pAltBarrel	= g_pWeaponMgr->GetBarrel(pNewWeapon->aBarrelTypes[ALT_BARREL]);

	//ok let's reset and re-use i
	i=0;

	if(pAltBarrel)
	{
		AMMO*		pAmmo = g_pWeaponMgr->GetAmmo(pAltBarrel->nAmmoType);
		AMMO_POOL*	pPool = g_pWeaponMgr->GetAmmoPool(pAmmo->szAmmoPool);

		//looks like we have an alt barrel so this goes on the bottom
		//lets see if it uses clips
		if(pAltBarrel->nShotsPerClip)
		{
			//record the pointers
			m_CounterItems[i+1].pBarrel	= m_CounterItems[i].pBarrel	= pAltBarrel;
			m_CounterItems[i+1].pAmmo	= m_CounterItems[i].pAmmo	= pAmmo;
			m_CounterItems[i+1].pPool	= m_CounterItems[i].pPool	= pPool;
			m_CounterItems[i].eType		= CT_CLIPS;
			m_CounterItems[i+1].eType	= CT_CLIP;

			//yep lets set up a clip counter
			m_CounterItems[i].hIcon		= CreateCounterIcon(pAltBarrel->szClipIcon);
			m_CounterItems[i+1].hIcon	= CreateCounterIcon(pPool->szCounterIcon);

			uint32 nMaxClips = pPool->GetMaxAmount(g_pLTClient->GetClientObject())/pAltBarrel->nShotsPerClip;

			char temp[10];

			sprintf(temp,"%d", nMaxClips);
			m_CounterItems[i].nNumDigits = strlen(temp);

			sprintf(temp,"%d", pAltBarrel->nShotsPerClip);
			m_CounterItems[i+1].nNumDigits = strlen(temp);
			
			m_CounterItems[i].pFont = m_CounterItems[i+1].pFont = 1;
			m_CounterItems[i].ptIconLoc = LTIntPt(75+15*(m_CounterItems[i].nNumDigits+1)/2,60);
			m_CounterItems[i].ptStrLoc	= LTIntPt(75,36);
			m_CounterItems[i+1].ptIconLoc	= LTIntPt(15+15*(m_CounterItems[i+1].nNumDigits+1)/2,60);
			m_CounterItems[i+1].ptStrLoc	= LTIntPt(15,36);
			i+=2;
		}
		else
		{
			//record the pointers
			m_CounterItems[i].pBarrel	= pAltBarrel;
			m_CounterItems[i].pAmmo		= pAmmo;
			m_CounterItems[i].pPool		= pPool;

			m_CounterItems[i].eType		= CT_AMMO_ONLY;

			//alt parrel doesnt use clips
			m_CounterItems[i].hIcon = CreateCounterIcon(pPool->szCounterIcon);

			uint32 nMaxAmmo = pPool->GetMaxAmount(g_pLTClient->GetClientObject());

			char temp[10];

			sprintf(temp,"%d", nMaxAmmo);
			m_CounterItems[i].nNumDigits = strlen(temp);

			m_CounterItems[i].pFont		= 1;
			m_CounterItems[i].ptIconLoc	= LTIntPt(15+15*(m_CounterItems[i].nNumDigits+1)/2,60);
			m_CounterItems[i].ptStrLoc	= LTIntPt(15,36);
			++i;
		}
	}

	if(pPrimBarrel)
	{
		AMMO*		pAmmo = g_pWeaponMgr->GetAmmo(pPrimBarrel->nAmmoType);
		AMMO_POOL*	pPool = g_pWeaponMgr->GetAmmoPool(pAmmo->szAmmoPool);

		//looks like we have an alt barrel so tihs goes on the bottom
		//lets see if it uses clips
		if(pPrimBarrel->nShotsPerClip)
		{
			//record the pointers
			m_CounterItems[i+1].pBarrel	= m_CounterItems[i].pBarrel	= pPrimBarrel;
			m_CounterItems[i+1].pAmmo	= m_CounterItems[i].pAmmo	= pAmmo;
			m_CounterItems[i+1].pPool	= m_CounterItems[i].pPool	= pPool;

			m_CounterItems[i].eType		= CT_CLIPS;
			m_CounterItems[i+1].eType	= CT_CLIP;

			//yep lets set up a clip counter
			m_CounterItems[i].hIcon		= CreateCounterIcon(pPrimBarrel->szClipIcon);
			m_CounterItems[i+1].hIcon	= CreateCounterIcon(pPool->szCounterIcon);

			uint32 nMaxClips = pPool->GetMaxAmount(g_pLTClient->GetClientObject())/pPrimBarrel->nShotsPerClip;

			char temp[10];

			sprintf(temp,"%d", nMaxClips);
			m_CounterItems[i].nNumDigits = strlen(temp);

			sprintf(temp,"%d", pPrimBarrel->nShotsPerClip);
			m_CounterItems[i+1].nNumDigits = strlen(temp);
			
			m_CounterItems[i].pFont		= m_CounterItems[i+1].pFont = 1;

			if(pAltBarrel)
			{
				m_CounterItems[i].ptIconLoc = LTIntPt(75+15*(m_CounterItems[i].nNumDigits+1)/2,120);
				m_CounterItems[i].ptStrLoc	= LTIntPt(75,97);
				m_CounterItems[i+1].ptIconLoc	= LTIntPt(15+15*(m_CounterItems[i+1].nNumDigits+1)/2,120);
				m_CounterItems[i+1].ptStrLoc	= LTIntPt(15,97);
			}
			else
			{
				m_CounterItems[i].ptIconLoc = LTIntPt(75+15*(m_CounterItems[i].nNumDigits+1)/2,60);
				m_CounterItems[i].ptStrLoc	= LTIntPt(75,36);
				m_CounterItems[i+1].ptIconLoc	= LTIntPt(15+15*(m_CounterItems[i+1].nNumDigits+1)/2,60);
				m_CounterItems[i+1].ptStrLoc	= LTIntPt(15,36);
			}
			i+=2;
		}
		else
		{
			//record the pointers
			m_CounterItems[i].pBarrel	= pPrimBarrel;
			m_CounterItems[i].pAmmo		= pAmmo;
			m_CounterItems[i].pPool		= pPool;

			m_CounterItems[i].eType		= CT_AMMO_ONLY;

			//alt parrel doesnt use clips
			m_CounterItems[i].hIcon = CreateCounterIcon(pPool->szCounterIcon);

			uint32 nMaxAmmo = pPool->GetMaxAmount(g_pLTClient->GetClientObject());

			char temp[10];

			sprintf(temp,"%d", nMaxAmmo);
			m_CounterItems[i].nNumDigits = strlen(temp);

			m_CounterItems[i].pFont		= 1;
			if(pAltBarrel)
			{
				m_CounterItems[i].ptIconLoc	= LTIntPt(15+15*(m_CounterItems[i].nNumDigits+1)/2,120);
				m_CounterItems[i].ptStrLoc	= LTIntPt(15,97);
			}
			else
			{
				m_CounterItems[i].ptIconLoc	= LTIntPt(15+15*(m_CounterItems[i].nNumDigits+1)/2,60);
				m_CounterItems[i].ptStrLoc	= LTIntPt(15,36);
			}
			++i;
		}
	}
}

// -----------------------------------------------/
//
// void CHudMgr::DrawExoArmor Implimentation
//
// Exosuit Armor Drawing runtine
//
// -----------------------------------------------/
void CHudMgr::DrawExoArmor	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	//draw our element to the screen
	g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen, 
													pHudElement->GetSurface(), 
													&pHudElement->GetDisplayRect(),
													pHudElement->GetDestX(), 
													pHudElement->GetDestY(), 
													hTransColor);
}

// -----------------------------------------------/
//
// void CHudMgr::DrawExoEnergy Implimentation
//
// Exosuit Energy Drawing runtine
//
// -----------------------------------------------/
void CHudMgr::DrawExoEnergy	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	//adjust draw offset
	int nDestX = pHudElement->GetDestX() + pHudElement->GetDisplayRect().left;

	//draw our element to the screen
	g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen, 
													pHudElement->GetSurface(), 
													&pHudElement->GetDisplayRect(),
													nDestX, 
													pHudElement->GetDestY(), 
													hTransColor);
}

// -----------------------------------------------/
//
// void CHudMgr::UpdateExoArmor Implimentation
//
// Exosuit Armor Update runtine
//
// -----------------------------------------------/
void CHudMgr::UpdateExoArmor	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() || !g_pGameClientShell) return;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	static DFLOAT nDisHealth = 0;

	// see if it is time to reset...
	if(pStats->GetResetHealth())
		nDisHealth = (LTFLOAT)pStats->GetHealthCount();

	uint32	nHealth = pStats->GetHealthCount();
	uint32	nMaxHealth = pStats->GetMaxHealthCount();
	DRect   rcFill  = pHudElement->GetImgSrcRect();
	LTFLOAT fRate = nMaxHealth / g_cvarMeterRate.GetFloat();

	//incriment based on time
	if(nDisHealth < nHealth) 
	{
		nDisHealth += g_pGameClientShell->GetFrameTime()*fRate;

		// Check for overshoot
		if(nDisHealth > nHealth)
			nDisHealth = (LTFLOAT)nHealth;
	}
	if(nDisHealth > nHealth)
	{
		nDisHealth -= g_pGameClientShell->GetFrameTime()*fRate;

		// Check for overshoot
		if(nDisHealth < nHealth)
			nDisHealth = (LTFLOAT)nHealth;
	}
	
	//reset fill top
	rcFill.right -= (uint32) ((1-nDisHealth/nMaxHealth)*rcFill.right);

	//save result
	pHudElement->SetDisplayRect(rcFill);}

// -----------------------------------------------/
//
// void CHudMgr::UpdateExoEnergy Implimentation
//
// Exosuit Energy Update runtine
//
// -----------------------------------------------/
void CHudMgr::UpdateExoEnergy	(CHudElement *pHudElement)
{
	//sanity check
	if ( !g_pLTClient || !pHudElement || !pHudElement->GetEnabled() || !g_pGameClientShell) return;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	static DFLOAT nDisEnergy = 0;

	uint32	nEnergy = pStats->GetExoEnergyCount();
	uint32	nMaxEnergy = pStats->GetMaxExoEnergyCount();
	DRect   rcFill  = pHudElement->GetImgSrcRect();
	LTFLOAT fRate = nMaxEnergy / g_cvarMeterRate.GetFloat();

	//incriment based on time
	if(nDisEnergy < nEnergy) 
	{
		nDisEnergy += g_pGameClientShell->GetFrameTime()*fRate;

		// Check for overshoot
		if(nDisEnergy > nEnergy)
			nDisEnergy = (LTFLOAT)nEnergy;
	}
	if(nDisEnergy > nEnergy)
	{
		nDisEnergy -= g_pGameClientShell->GetFrameTime()*fRate;

		// Check for overshoot
		if(nDisEnergy < nEnergy)
			nDisEnergy = (LTFLOAT)nEnergy;
	}
	
	//reset fill top
	rcFill.left += (uint32) ((1-nDisEnergy/nMaxEnergy)*rcFill.right);

	//save result
	pHudElement->SetDisplayRect(rcFill);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CHudMgr::Save
//
//	PURPOSE:	Save the interface info
//
// --------------------------------------------------------------------------- //

void CHudMgr::Save(HMESSAGEWRITE hWrite)
{
	*hWrite << m_bHudEnabled;		
	*hWrite << m_bMDEnabled;		
	*hWrite << m_bMDDraw;		
	*hWrite << m_bBossHealthOn;		
	*hWrite << m_fBossHealthRatio;
	*hWrite << m_WeaponID;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CHudMgr::Load
//
//	PURPOSE:	Load the interface info
//
// --------------------------------------------------------------------------- //

void CHudMgr::Load(HMESSAGEREAD hRead)
{
	*hRead >> m_bHudEnabled;		
	*hRead >> m_bMDEnabled;		
	*hRead >> m_bMDDraw;		
	*hRead >> m_bBossHealthOn;		
	*hRead >> m_fBossHealthRatio;	
	*hRead >> m_WeaponID;

	NewWeapon(g_pWeaponMgr->GetWeapon(m_WeaponID));
}

// ----------------------------------------------------------------------- //
//
// PURPOSE : HudElement implimentation
//
// CREATED : 01.21.2000
//
// ----------------------------------------------------------------------- //
// -----------------------------------------------/
//
// void CHudElement::CHudElement Implimentation
//
// Constructor
//
// -----------------------------------------------/
CHudElement::CHudElement()
{
	m_nID			= 0;			
	m_bEnabled		= LTFALSE;
	m_bTransparent	= LTFALSE; 
	m_bTranslucent	= LTFALSE; 
	m_fTranslucency	= 0.0f;
	m_nFontIndex	= 0;	
	m_nNumAnim		= 0;		
	m_hSurface		= LTNULL;		
	m_ahAnimSurf	= LTNULL;	
	m_ahAltSurf		= LTNULL;
	m_hszString		= LTNULL;	
	m_hszAltString	= LTNULL;	
	m_fHorScale		= 0.0f;	
	m_fVerScale		= 0.0f;	
	m_nOffsetType	= 0;
}
// -----------------------------------------------/
//
// void CHudElement::Init Implimentation
//
// Initialization routine
//
// -----------------------------------------------/
void CHudElement::Init(char * pTag, uint32 nIndex)
{
	char		s_aSurfaceName[60];

	//set element index number
	m_nID		= nIndex;
	
	//set alternate surface array pointer to null
	m_ahAltSurf = LTNULL;

	//set animation array pointer to null
	m_ahAnimSurf = LTNULL;

	//get number of animation frames
	m_nNumAnim	= g_pLayoutMgr->GetNumAnim(pTag, nIndex);
	
	if(m_nNumAnim)
		LoadAnimation (pTag, nIndex);
	
	//set element enabled boolean 
	m_bEnabled	= g_pLayoutMgr->GetElementEnabled(pTag, nIndex);	

	//set image offset
	m_ptOffset		= g_pLayoutMgr->GetElementOffset(pTag, nIndex);	
	m_nOffsetType	= g_pLayoutMgr->GetElementOffsetType(pTag, nIndex);	

	//set transparancy boolean
	m_bTransparent = g_pLayoutMgr->GetElementTransparent(pTag, nIndex);	

	//set translucency boolean
	m_bTranslucent = g_pLayoutMgr->GetElementTranslucent(pTag, nIndex);

	//get translucency value
	m_fTranslucency = g_pLayoutMgr->GetElementTranslucency(pTag, nIndex);

	//get surface path and name
	g_pLayoutMgr->GetSurfaceName(pTag, nIndex, s_aSurfaceName, 60);

	//load up surface
	m_hSurface = g_pInterfaceResMgr->GetSharedSurface(s_aSurfaceName);

	//set translucency
	if(m_bTranslucent)
		g_pLTClient->SetSurfaceAlpha (m_hSurface, m_fTranslucency);

	//get font index
	m_nFontIndex = g_pLayoutMgr->GetFontIndex(pTag, nIndex);

	//set string pointer to null
	m_hszString		= LTNULL;
	m_hszAltString	= LTNULL;
	
	//set destination rectangle
	SetRects();

	//set initial scaling factors
	m_fHorScale = 1;
	m_fVerScale = 1;
}

// -----------------------------------------------/
//
// void CHudElement::Term Implimentation
//
// Termination routine
//
// -----------------------------------------------/
void CHudElement::Term()
{
	//clean up surfaces and strings
//	if(m_hSurface)
//	{
//		g_pLTClient->DeleteSurface(m_hSurface);
//		m_hSurface = LTNULL;
//	}

	if(m_hszString)
	{
		g_pLTClient->FreeString(m_hszString);
		m_hszString = LTNULL;
	}

	if(m_hszAltString)
	{
		g_pLTClient->FreeString(m_hszAltString);
		m_hszAltString = LTNULL;
	}

	if(m_ahAnimSurf)
	{
//		for(uint32 i=0 ; i<m_nNumAnim ; i++)
//		{
//			g_pLTClient->DeleteSurface(m_ahAnimSurf[i]);
//			m_ahAnimSurf[i] = LTNULL;
//		}

		//now free the array
		delete [] m_ahAnimSurf;
		m_ahAnimSurf = LTNULL;
	}

	if(m_ahAltSurf)
	{
//		for(uint32 i=0 ; i<46 ; i++)
//		{
//			g_pLTClient->DeleteSurface(m_ahAltSurf[i]);
//			m_ahAltSurf[i] = LTNULL;
//		}

		//now free the array
		delete [] m_ahAltSurf;
		m_ahAltSurf = LTNULL;
	}
}

// -----------------------------------------------/

void CHudElement::SetString(HSTRING hStr)
{
	if(m_hszString)
	{
		g_pLTClient->FreeString(m_hszString);
		m_hszString = LTNULL;
	}

	m_hszString = g_pLTClient->CopyString(hStr);
}

// -----------------------------------------------/

void CHudElement::SetAltString(HSTRING hStr)
{
	if(m_hszAltString)
	{
		g_pLTClient->FreeString(m_hszAltString);
		m_hszAltString = LTNULL;
	}

	m_hszAltString = g_pLTClient->CopyString(hStr);
}

// -----------------------------------------------/
//
// void CHudElement::LoadAnimation Implimentation
//
// This routine loads the animation frames is required
//
// -----------------------------------------------/
void CHudElement::LoadAnimation(char * pTag, uint32 nIndex)
{
	char		aAnimName[60];
	char		aTemp[60];
	
	//new the array of pointers
	m_ahAnimSurf	= new HSURFACE[m_nNumAnim];
	
	//get surface path and name
	g_pLayoutMgr->GetAnimName(pTag, nIndex, aAnimName, 60);

	//now load the surfaces
	for(uint32 i=0 ; i<m_nNumAnim ; i++)
	{
		//parse name
		sprintf(aTemp,"%s%d%s",aAnimName,i,".pcx");

		//create surface
		m_ahAnimSurf[i] = g_pInterfaceResMgr->GetSharedSurface(aTemp);

		DFLOAT fTrans = g_pLayoutMgr->GetAmimTranslucency(pTag,nIndex);
		
		//set translucency
		g_pLTClient->SetSurfaceAlpha (m_ahAnimSurf[i], fTrans);
	}
}


// -----------------------------------------------/
//
// void CHudElement::SetRects Implimentation
//
// Routine that sets various rectangles for drawing
//
// -----------------------------------------------/
void CHudElement::SetRects()
{
	//local variables
	uint32		nWidth, nHeight, nScreenWidth, nScreenHeight;

	//get the dims of our surface
	g_pLTClient->GetSurfaceDims(m_hSurface, &nWidth, &nHeight);

	//get the dims of the screen
	g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(),
								&nScreenWidth, &nScreenHeight);
	
	//set the dest point (neg offsets are from right and bottom)
	switch(m_nOffsetType)
	{
	case(0):
		{
			if(m_ptOffset.x > 0)
				m_ptDest.x	= m_ptOffset.x;
			else
				m_ptDest.x	= nScreenWidth + m_ptOffset.x;

			if(m_ptOffset.y > 0)
				m_ptDest.y	= m_ptOffset.y;
			else
				m_ptDest.y	= nScreenHeight + m_ptOffset.y;
			break;
		}
	case(1):
		{
			m_ptDest.x	= nScreenWidth - m_ptOffset.x;
			m_ptDest.y	= (nScreenHeight>>1) + m_ptOffset.y;
			break;
		}
	case(2):
		{
			m_ptDest.x	= m_ptOffset.x;
			m_ptDest.y	= (nScreenHeight>>1) + m_ptOffset.y;
			break;
		}
	case(3):
		{
			m_ptDest.x	= (nScreenWidth>>1) + m_ptOffset.x;
			m_ptDest.y	= nScreenHeight-m_ptOffset.y;
			break;
		}
	case(4):
		{
			m_ptDest.x	= (nScreenWidth>>1) + m_ptOffset.x;
			m_ptDest.y	= m_ptOffset.y;
			break;
		}
	}

	//set entire src rect
	m_rcImgSrc.left		= 0;
	m_rcImgSrc.top		= 0;
	m_rcImgSrc.right	= nWidth;
	m_rcImgSrc.bottom	= nHeight;

	//set half height rect
	m_rcHalfHeight			= m_rcImgSrc;
	m_rcHalfHeight.bottom	= (nHeight >> 1) + 1;

	//set initial display rectangle
	SetDisplayRect(m_rcImgSrc);
}


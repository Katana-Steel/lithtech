// CMarineWepChooser.cpp: implementation of the CMarineWepChooser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WeaponChooser.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "SoundMgr.h"
#include "PlayerStats.h"

extern CGameClientShell* g_pGameClientShell;


namespace
{
	const int kIconSpacing  = 2;
	const float kfDelayTime	= 10.0f;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMarineWepChooser::CMarineWepChooser()
{
	memset(m_ahSmallIcon, DNULL, sizeof(m_ahSmallIcon));
	m_ahWeaponSurf	= DNULL;
	m_bIsOpen		= DFALSE;
	m_fStartTime	= 0.0f;
	m_fFinishTime	= 0.0f;
	m_fResetTime	= 0.0f;
	m_hSelectBox	= DNULL;
	m_nCurWeapon	= WMGR_INVALID_ID;
	m_nLastWeapon	= 0;
	m_nFirstWeapon	= 0;
	m_nIconWidth	= 0;
	m_nIconHeight	= 0;
	m_nSmIconWidth	= 0;
	m_nSmIconHeight = 0;
	m_plfIconFontWht= DNULL;
	m_plfIconFontBlk= DNULL;
	m_pPlayerStats	= DNULL;
	m_bClosing		= LTFALSE;
}

CMarineWepChooser::~CMarineWepChooser()
{
	Term();
}

// -----------------------------------------------/
//
// void CMarineWepChooser::Init Implimentation
//
// Sets member variables to initial states
//
// -----------------------------------------------/
void CMarineWepChooser::Init()
{
	//get pointer to player stats
	m_pPlayerStats = g_pGameClientShell->GetPlayerStats();
	
	//set variables
	if(m_pPlayerStats)
		m_nMaxWeapons = m_pPlayerStats->GetNumWeapons();
	
	//new the array of icon surface pointers
	if(m_nMaxWeapons)
	{
		m_ahWeaponSurf = new HSURFACE[m_nMaxWeapons];
		memset(m_ahWeaponSurf, 0, sizeof(HSURFACE) * m_nMaxWeapons);
	}

	//load icon surfaces
	LoadImages();

	//load icon fonts
	LoadFonts();
}

// -----------------------------------------------/
//
// void CMarineWepChooser::LoadFonts Implimentation
//
// Loads the font resources for the weapon chooser
//
// -----------------------------------------------/
void CMarineWepChooser::LoadFonts()
{
	LITHFONTCREATESTRUCT	lfCreateStruct;

	//set create struct fixed elements
	lfCreateStruct.nGroupFlags = LTF_INCLUDE_ALL;
	lfCreateStruct.hTransColor = SETRGB_T(0,0,0);
	lfCreateStruct.bChromaKey	= LTTRUE;

	//set name elelment for font
	lfCreateStruct.szFontBitmap = "\\Interface\\Fonts\\font_weapon.pcx";

	//load up the font
	m_plfIconFontWht = CreateLithFont(g_pClientDE, &lfCreateStruct, DTRUE);

	if(!m_plfIconFontWht)
		g_pClientDE->ShutdownWithMessage("ERROR: Could not initialize font in Weapon Chooser");

	//set name elelment for font
	lfCreateStruct.szFontBitmap = "\\Interface\\Fonts\\font_weapon2.pcx";

	//load up the font
	m_plfIconFontBlk = CreateLithFont(g_pClientDE, &lfCreateStruct, DTRUE);

	if(!m_plfIconFontBlk)
		g_pClientDE->ShutdownWithMessage("ERROR: Could not initialize font in Weapon Chooser");
}

// -----------------------------------------------/
//
// void CMarineWepChooser::LoadImages Implimentation
//
// Loads the icon images into the array
//
// -----------------------------------------------/
void CMarineWepChooser::LoadImages()
{
	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	//get the player's weapon set info
	WEAPON_SET *pSet = g_pWeaponMgr->GetWeaponSet(m_pPlayerStats->GetWeaponSet());

	//load up surfaces
	if(m_ahWeaponSurf)
	{
		for(int i=0 ; i<pSet->nNumWeapons ; i++)
		{
			WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(pSet->szWeaponName[i]);

			if(pWeapon)
			{
				//create the surface
				m_ahWeaponSurf[i] = g_pClientDE->CreateSurfaceFromBitmap(pWeapon->szIcon);
			
				//set transparency
				g_pClientDE->OptimizeSurface (m_ahWeaponSurf[i], hTransColor);

				//set translucency
				g_pClientDE->SetSurfaceAlpha (m_ahWeaponSurf[i], 0.5f);
			}
			else
				m_ahWeaponSurf[i] = DNULL;
		}
	}

	//load up the small icons and selection box
	m_ahSmallIcon[0] = g_pClientDE->CreateSurfaceFromBitmap("interface\\statusbar\\marine\\inv_greenbox.pcx");
	m_ahSmallIcon[1] = g_pClientDE->CreateSurfaceFromBitmap("interface\\statusbar\\marine\\inv_redbox.pcx");
	m_hSelectBox = g_pClientDE->CreateSurfaceFromBitmap("interface\\statusbar\\marine\\inv_frame.pcx");

	//store dims
	g_pClientDE->GetSurfaceDims (m_ahWeaponSurf[0], &m_nIconWidth, &m_nIconHeight);
	g_pClientDE->GetSurfaceDims (m_ahSmallIcon[0], &m_nSmIconWidth, &m_nSmIconHeight);

	g_pClientDE->OptimizeSurface (m_hSelectBox, hTransColor);

	g_pClientDE->SetSurfaceAlpha (m_ahSmallIcon[0], 0.5f);
	g_pClientDE->SetSurfaceAlpha (m_ahSmallIcon[1], 0.5f);
	g_pClientDE->SetSurfaceAlpha (m_hSelectBox, 0.5f);
}

// -----------------------------------------------/
//
// void CMarineWepChooser::Term Implimentation
//
// Clean-up routine
//
// -----------------------------------------------/
void CMarineWepChooser::Term()
{
	if(m_ahWeaponSurf)
	{
		//clean up icon surfaces
		for( DDWORD i=0 ; i<m_nMaxWeapons ; i++ )
		{
			if(m_ahWeaponSurf[i])
			{
				g_pClientDE->DeleteSurface (m_ahWeaponSurf[i]);
				m_ahWeaponSurf[i] = DNULL;
			}
		}

		delete [] m_ahWeaponSurf;
		m_ahWeaponSurf = DNULL;
	}

	if(m_ahSmallIcon[0])
	{
		g_pClientDE->DeleteSurface (m_ahSmallIcon[0]);
		m_ahSmallIcon[0] = DNULL;
	}

	if(m_ahSmallIcon[1])
	{
		g_pClientDE->DeleteSurface (m_ahSmallIcon[1]);
		m_ahSmallIcon[1] = DNULL;
	}

	if(m_hSelectBox)
	{
		g_pClientDE->DeleteSurface (m_hSelectBox);
		m_hSelectBox = DNULL;
	}

	if(m_plfIconFontWht)
	{
		FreeLithFont(m_plfIconFontWht);
		m_plfIconFontWht = DNULL;
	}

	if(m_plfIconFontBlk)
	{
		FreeLithFont(m_plfIconFontBlk);
		m_plfIconFontBlk = DNULL;
	}
}

// -----------------------------------------------/
//
// void CMarineWepChooser::Open Implimentation
//
// Chooser open routine
//
// -----------------------------------------------/
DBOOL CMarineWepChooser::Open()
{
	if (m_bIsOpen)
		return DTRUE;

	//play the open sound
	g_pClientSoundMgr->PlaySoundLocal("MarineInterfaceOpen");

	CWeaponModel*	pWeap = g_pGameClientShell->GetWeaponModel();

	if(!pWeap) 
		return DFALSE;

	WEAPON* pWep = pWeap->GetWeapon();

	if(!pWep)
		return LTFALSE;

	int nWepIndex = m_pPlayerStats->GetWeaponIndex(pWep->nId);

	//special case for blowtorch and hacking device
	LTBOOL bIsTorch		= (strcmp(pWep->szName, "Blowtorch")==0);
	LTBOOL bIsHacker	= (strcmp(pWep->szName, "MarineHackingDevice")==0);

	if(bIsTorch || bIsHacker) nWepIndex=0;

	if(nWepIndex == WMGR_INVALID_ID)
		return DFALSE;

	if(m_nCurWeapon != WMGR_INVALID_ID)
		g_pClientDE->SetSurfaceAlpha(m_ahWeaponSurf[m_nCurWeapon],0.5f);

	m_nCurWeapon = nWepIndex;
		
	g_pClientDE->SetSurfaceAlpha(m_ahWeaponSurf[nWepIndex],1.0f);

	for(DBYTE i=0 ; i<m_nMaxWeapons ; i++)
	{
		WEAPON* pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(i));

		if(pWep)
		{
			if(m_pPlayerStats->HaveWeapon(pWep->nId))
				m_nLastWeapon = i;
		}
	}
	
	for(i=(DBYTE) m_nMaxWeapons ; i>0 ; i--)
	{
		WEAPON* pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(i-1));

		if(pWep)
		{
			if(m_pPlayerStats->HaveWeapon(pWep->nId))
				m_nFirstWeapon = i-1;
		}
	}

	m_fResetTime = m_fStartTime = g_pClientDE->GetTime();

	m_bIsOpen = DTRUE;
	m_bClosing = LTFALSE;

	return DTRUE;
}

// -----------------------------------------------/
//
// void CMarineWepChooser::Close Implimentation
//
// Chooser close routine
//
// -----------------------------------------------/
void CMarineWepChooser::Close()
{
	if(m_bClosing)
		return;

	//play the close sound
	if(!g_pGameClientShell->IsCinematic())
		g_pClientSoundMgr->PlaySoundLocal("MarineInterfaceClose");

	m_fFinishTime = g_pClientDE->GetTime();
	m_fStartTime = 0.0f;
	m_bClosing = LTTRUE;
}

// -----------------------------------------------/
//
// void CMarineWepChooser::NextWeapon Implimentation
//
// Increments the current weapon index
//
// -----------------------------------------------/

void CMarineWepChooser::NextWeapon()
{
	//play the next sound
	g_pClientSoundMgr->PlaySoundLocal("MarineInterfaceScroll");
	
	g_pClientDE->SetSurfaceAlpha(m_ahWeaponSurf[m_nCurWeapon],0.5f);
	
	m_nCurWeapon++;
	
	WEAPON* pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(m_nCurWeapon));
	
	for(int i=m_nCurWeapon; pWep && i<=(int)m_nLastWeapon && !m_pPlayerStats->HaveWeapon(pWep->nId); i++)
	{
		pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(i+1));
	}
	
	m_nCurWeapon = i;
	
	if(m_nCurWeapon > (int)m_nLastWeapon)
		m_nCurWeapon = m_nFirstWeapon;
	
	g_pClientDE->SetSurfaceAlpha(m_ahWeaponSurf[m_nCurWeapon],1.0f);
	
	m_fResetTime = g_pClientDE->GetTime();	
	
}

void CMarineWepChooser::PrevWeapon()
{
	//play the next sound
	g_pClientSoundMgr->PlaySoundLocal("MarineInterfaceScroll");
	
	g_pClientDE->SetSurfaceAlpha(m_ahWeaponSurf[m_nCurWeapon],0.5f);
	
	m_nCurWeapon--;
	
	WEAPON* pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(m_nCurWeapon));
	
	for(int i=m_nCurWeapon; pWep && i>(int)m_nFirstWeapon && !m_pPlayerStats->HaveWeapon(pWep->nId); i--)
	{
		pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(i-1));
	}
	
	m_nCurWeapon = i;
	
	if(m_nCurWeapon < (int)m_nFirstWeapon)
		m_nCurWeapon = m_nLastWeapon;
	
	g_pClientDE->SetSurfaceAlpha(m_ahWeaponSurf[m_nCurWeapon],1.0f);
	
	m_fResetTime = g_pClientDE->GetTime();	
	
}

void CMarineWepChooser::Draw()
{
	float fTime = g_pClientDE->GetTime() - m_fResetTime;
	
	if (m_fStartTime > 0.0f && fTime > kfDelayTime)
	{
		Close();
		return;
	}
	
	// Need to make sure our list has not changed.
	for(DBYTE i=0 ; i<m_nMaxWeapons ; i++)
	{
		WEAPON* pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(i));
		
		if(pWep)
		{
			if(m_pPlayerStats->HaveWeapon(pWep->nId))
				m_nLastWeapon = i;
		}
	}
	
	for(i=(DBYTE) m_nMaxWeapons ; i>0 ; i--)
	{
		WEAPON* pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(i-1));
		
		if(pWep)
		{
			if(m_pPlayerStats->HaveWeapon(pWep->nId))
				m_nFirstWeapon = i-1;
		}
	}
	
	//set transparent color
	static HDECOLOR hTransColor = SETRGB_T(0,0,0);
	
	//get screen handle and dims
	HSURFACE	hScreen = g_pClientDE->GetScreenSurface();
	DDWORD		nScreenHeight, nScreenWidth;
	g_pClientDE->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);
	
	//set default X and Y drawing offsets
	DDWORD	nXOffset = nScreenWidth-(m_nIconWidth+2);
	DDWORD	nYOffset = 50;
	DFLOAT	fXDisplay;
	
	//calculate animation position for starting and ending
	if(m_fFinishTime)
	{
		//we are closing, move chooser off screen
		fXDisplay= nXOffset + (g_pClientDE->GetTime()-m_fFinishTime) * 600;
		
		nXOffset = (DDWORD)fXDisplay;
		
		//test to see if we are off screen and end
		if(fXDisplay > nScreenWidth)
		{
			m_bIsOpen = DFALSE;
			m_fFinishTime = 0.0f;
		}
	}
	else
	{
		//we are opening, move choser on screen
		fXDisplay= nScreenWidth - (g_pClientDE->GetTime()-m_fStartTime) * 600;
		
		if(fXDisplay > nXOffset)
			nXOffset = (DDWORD)fXDisplay;
	}
	
	//set drawing sequence type
	DDWORD	nDrawIndex;
	
	//selected weapon is at the top of the list
	if(m_nCurWeapon == 0)
		nDrawIndex = 0;
	//selected weapon is at the end of the list
	else if(m_nCurWeapon == (int)m_nLastWeapon)
		nDrawIndex = 1;
	//selected weapon is in the middle of the list
	else nDrawIndex = 2;
	
	LITHFONTDRAWDATA lfDD;
	char str[3];
	HSTRING hStr = DNULL;
	
	//set drawdata elements
	lfDD.byJustify		= LTF_JUSTIFY_LEFT;
	lfDD.nLetterSpace	= 1;
	
	if(!g_pGameClientShell->IsCinematic())
	{
		for( i=0 ; i<m_nMaxWeapons ; i++ )
		{
			WEAPON* pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(i));
			
			if(pWep)
			{
				if(m_pPlayerStats->HaveWeapon(pWep->nId))
				{
					sprintf(str,"%d",(i+1)%10);
					
					hStr = g_pClientDE->CreateString(str);
					
					if(i==m_nCurWeapon)
					{
						g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen,
							m_hSelectBox, 
							DNULL, 
							nXOffset, nYOffset,
							hTransColor);
						
						g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen,
							m_ahWeaponSurf[i], 
							DNULL, 
							nXOffset, nYOffset,
							hTransColor);
						m_plfIconFontWht->Draw(	hScreen,
							hStr,
							&lfDD, 
							nXOffset+2, 
							nYOffset+1,
							DNULL);
						
						DrawAmmoBars(hScreen, pWep->nId, nXOffset, nYOffset);
						
						nYOffset += m_nIconHeight + kIconSpacing;
					}
					else
					{
						//need to check for ammo availability here
						if(HaveAmmo(pWep->nId))
						{
							g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen,
								m_ahSmallIcon[0], 
								DNULL, 
								nXOffset+120, nYOffset,
								hTransColor);
						}
						else
						{
							g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen,
								m_ahSmallIcon[1], 
								DNULL, 
								nXOffset+120, nYOffset,
								hTransColor);
						}
						
						m_plfIconFontBlk->Draw(	hScreen,
							hStr,
							&lfDD, 
							nXOffset+120, 
							nYOffset,
							DNULL);
						
						nYOffset += m_nSmIconHeight + kIconSpacing;
					}
					
					g_pClientDE->FreeString( hStr );
					
				}
			}
		}
	}
}

DBOOL CMarineWepChooser::HaveAmmo(DBYTE nWeaponID)
{
	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponID);

	if(pWeapon)
	{
		//loop thru the barrels
		for(int j=0 ; j<NUM_BARRELS ; j++)
		{
			BARREL* pBarrel = g_pWeaponMgr->GetBarrel(pWeapon->aBarrelTypes[j]);

			BARREL* pBaseBarrel = pBarrel;

			if(pBarrel)
				do
				{
					if(pBarrel->bInfiniteAmmo || (m_pPlayerStats->GetAmmoCount(pBarrel->nAmmoType) > 0))
						return DTRUE;

					pBarrel = g_pWeaponMgr->GetBarrel(pBarrel->szNextBarrel);

				}while(pBarrel && pBarrel != pBaseBarrel);
		}
	}
	return DFALSE;
}

void CMarineWepChooser::DrawAmmoBars(HSURFACE hScreen, DBYTE nWeaponID, DWORD nXOffset, DWORD nYOffset)
{
	//sanity check
	if(!g_pWeaponMgr || !m_pPlayerStats || !g_pClientDE) return;
	
	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponID);

	//set fill color
	static HDECOLOR hFillColor = SETRGB_T(0,88,250);

	if(pWeapon)
	{
		int counter=0;

		for ( int i=0 ; i < NUM_BARRELS ; i++)
		{
			//get barrel info
			BARREL* pFirstBarrel = g_pWeaponMgr->GetBarrel(pWeapon->aBarrelTypes[i]);
			BARREL* pCurBarrel = pFirstBarrel;
		
			//loop thru barrels
			do
			{

				if(pCurBarrel && pCurBarrel->bDrawChooserBars)
				{
					DFLOAT	fRatio		= 0.0f;
					AMMO*	pAmmo		= g_pWeaponMgr->GetAmmo(pCurBarrel->nAmmoType);
					DDWORD	nAmmoCount	= m_pPlayerStats->GetAmmoCount(pCurBarrel->nAmmoType);
					DDWORD	nMaxAmmo	= pAmmo->GetMaxPoolAmount(NULL);

					if(nMaxAmmo)
						fRatio = (DFLOAT)nAmmoCount / nMaxAmmo;

					DRect rect;

					rect.right	= nXOffset+38+counter++*23;
					rect.left	= rect.right - (int)(fRatio*20);
					rect.top	= nYOffset+29;
					rect.bottom	= rect.top+2;

					g_pClientDE->FillRect(hScreen,&rect,hFillColor);

					pCurBarrel = g_pWeaponMgr->GetBarrel(pCurBarrel->szNextBarrel);
				}
			}while(pCurBarrel && pCurBarrel != pFirstBarrel);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPredatorWepChooser::CPredatorWepChooser()
{
	memset(m_ahEnergyIcon, DNULL, sizeof(m_ahEnergyIcon));
	m_ahWeaponSurf	= DNULL;
	m_bIsOpen		= DFALSE;
	m_fStartTime	= 0.0f;
	m_fFinishTime	= 0.0f;
	m_fResetTime	= 0.0f;
	m_hSelectBox	= DNULL;
	m_nCurWeapon	= WMGR_INVALID_ID;
	m_nLastWeapon	= 0;
	m_nFirstWeapon	= 0;
	m_nIconWidth	= 0;
	m_nIconHeight	= 0;
	m_nSmIconWidth	= 0;
	m_nSmIconHeight = 0;
	m_plfIconFontWht= DNULL;
	m_plfIconFontBlk= DNULL;
	m_pPlayerStats	= DNULL;
	m_bClosing		= LTFALSE;
}

CPredatorWepChooser::~CPredatorWepChooser()
{
	Term();
}

// -----------------------------------------------/
//
// void CPredatorWepChooser::Init Implimentation
//
// Sets member variables to initial states
//
// -----------------------------------------------/
void CPredatorWepChooser::Init()
{
	//get pointer to player stats
	m_pPlayerStats = g_pGameClientShell->GetPlayerStats();
	
	//set variables
	if(m_pPlayerStats)
		m_nMaxWeapons = m_pPlayerStats->GetNumWeapons();
	
	//new the array of icon surface pointers
	if(m_nMaxWeapons)
	{
		m_ahWeaponSurf = new HSURFACE[m_nMaxWeapons];
		memset(m_ahWeaponSurf, 0, sizeof(HSURFACE) * m_nMaxWeapons);
	}

	//load icon surfaces
	LoadImages();

	//load icon fonts
	LoadFonts();
}

// -----------------------------------------------/
//
// void CPredatorWepChooser::LoadFonts Implimentation
//
// Loads the font resources for the weapon chooser
//
// -----------------------------------------------/
void CPredatorWepChooser::LoadFonts()
{
	LITHFONTCREATESTRUCT	lfCreateStruct;

	//set create struct fixed elements
	lfCreateStruct.nGroupFlags = LTF_INCLUDE_NUMBERS;
	lfCreateStruct.hTransColor = SETRGB_T(0,0,0);
	lfCreateStruct.bChromaKey	= LTTRUE;
	lfCreateStruct.nFixedPitch = 20;

	//set name elelment for font
	lfCreateStruct.szFontBitmap = "\\Interface\\statusbar\\predator\\inv_numbers.pcx";

	//load up the font
	m_plfIconFontWht = CreateLithFont(g_pClientDE, &lfCreateStruct, DTRUE);

	if(!m_plfIconFontWht)
		g_pClientDE->ShutdownWithMessage("ERROR: Could not initialize font in Weapon Chooser");

	//set name elelment for font
	lfCreateStruct.szFontBitmap = "\\Interface\\statusbar\\predator\\inv_numbers.pcx";

	//load up the font
	m_plfIconFontBlk = CreateLithFont(g_pClientDE, &lfCreateStruct, DTRUE);

	if(!m_plfIconFontBlk)
		g_pClientDE->ShutdownWithMessage("ERROR: Could not initialize font in Weapon Chooser");
}

// -----------------------------------------------/
//
// void CPredatorWepChooser::LoadImages Implimentation
//
// Loads the icon images into the array
//
// -----------------------------------------------/
void CPredatorWepChooser::LoadImages()
{
	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	//get the player's weapon set info
	WEAPON_SET *pSet = g_pWeaponMgr->GetWeaponSet(m_pPlayerStats->GetWeaponSet());

	//load up surfaces
	if(m_ahWeaponSurf)
	{
		for(int i=0 ; i<pSet->nNumWeapons ; i++)
		{
			WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(pSet->szWeaponName[i]);

			if(pWeapon)
			{
				//create the surface
				m_ahWeaponSurf[i] = g_pClientDE->CreateSurfaceFromBitmap(pWeapon->szIcon);
			
				//set transparency
				g_pClientDE->OptimizeSurface (m_ahWeaponSurf[i], hTransColor);

				//set translucency
				g_pClientDE->SetSurfaceAlpha (m_ahWeaponSurf[i], 0.5f);
			}
			else
				m_ahWeaponSurf[i] = DNULL;
		}
	}

	//load up the small icons and selection box
	m_ahEnergyIcon[0] = g_pClientDE->CreateSurfaceFromBitmap("interface\\statusbar\\predator\\energyicon1.pcx");
	m_ahEnergyIcon[1] = g_pClientDE->CreateSurfaceFromBitmap("interface\\statusbar\\predator\\energyicon2.pcx");
	m_hSelectBox = g_pClientDE->CreateSurfaceFromBitmap("interface\\statusbar\\predator\\inv_frame.pcx");

	//store dims
	if( m_ahWeaponSurf && m_ahWeaponSurf[1] )
		g_pClientDE->GetSurfaceDims (m_ahWeaponSurf[1], &m_nIconWidth, &m_nIconHeight);

	if( m_ahEnergyIcon && m_ahEnergyIcon[0] )
	{
		g_pClientDE->GetSurfaceDims (m_ahEnergyIcon[0], &m_nSmIconWidth, &m_nSmIconHeight);
		g_pClientDE->OptimizeSurface (m_ahEnergyIcon[0], hTransColor);
		g_pClientDE->OptimizeSurface (m_ahEnergyIcon[1], hTransColor);
	}

	g_pClientDE->OptimizeSurface (m_hSelectBox, hTransColor);

	g_pClientDE->SetSurfaceAlpha (m_hSelectBox, 0.5f);
}

// -----------------------------------------------/
//
// void CPredatorWepChooser::Term Implimentation
//
// Clean-up routine
//
// -----------------------------------------------/
void CPredatorWepChooser::Term()
{
	if(m_ahWeaponSurf)
	{
		//clean up icon surfaces
		for( DDWORD i=0 ; i<m_nMaxWeapons ; i++ )
		{
			if(m_ahWeaponSurf[i])
			{
				g_pClientDE->DeleteSurface (m_ahWeaponSurf[i]);
				m_ahWeaponSurf[i] = DNULL;
			}
		}

		delete [] m_ahWeaponSurf;
		m_ahWeaponSurf = DNULL;
	}

	if(m_ahEnergyIcon[0])
	{
		g_pClientDE->DeleteSurface (m_ahEnergyIcon[0]);
		m_ahEnergyIcon[0] = DNULL;
	}

	if(m_ahEnergyIcon[1])
	{
		g_pClientDE->DeleteSurface (m_ahEnergyIcon[1]);
		m_ahEnergyIcon[1] = DNULL;
	}

	if(m_hSelectBox)
	{
		g_pClientDE->DeleteSurface (m_hSelectBox);
		m_hSelectBox = DNULL;
	}

	if(m_plfIconFontWht)
	{
		FreeLithFont(m_plfIconFontWht);
		m_plfIconFontWht = DNULL;
	}

	if(m_plfIconFontBlk)
	{
		FreeLithFont(m_plfIconFontBlk);
		m_plfIconFontBlk = DNULL;
	}
}

// -----------------------------------------------/
//
// void CPredatorWepChooser::Open Implimentation
//
// Chooser open routine
//
// -----------------------------------------------/
DBOOL CPredatorWepChooser::Open()
{
	if (m_bIsOpen)
		return DTRUE;

	//play the open sound
	g_pClientSoundMgr->PlaySoundLocal("PredatorInterfaceOpen");

	CWeaponModel*	pWeap = g_pGameClientShell->GetWeaponModel();

	if(!pWeap) 
		return DFALSE;

	WEAPON* pWep = pWeap->GetWeapon();

	if(!pWep)
		return LTFALSE;

	int nWepIndex = m_pPlayerStats->GetWeaponIndex(pWep->nId);

	//special case for blowtorch and hacking device
	LTBOOL bIsSift		= (strcmp(pWep->szName, "Energy_Sift")==0);
	LTBOOL bIsMediComp	= (strcmp(pWep->szName, "Medicomp")==0);
	LTBOOL bIsHacker	= (strcmp(pWep->szName, "PredatorHackingDevice")==0);

	if(bIsSift || bIsMediComp || bIsHacker) nWepIndex=0;

	if(nWepIndex == WMGR_INVALID_ID)
		return DFALSE;

	if(m_nCurWeapon != WMGR_INVALID_ID)
		g_pClientDE->SetSurfaceAlpha(m_ahWeaponSurf[m_nCurWeapon],0.5f);

	m_nCurWeapon = nWepIndex;
		
	g_pClientDE->SetSurfaceAlpha(m_ahWeaponSurf[nWepIndex],1.0f);

	for(DBYTE i=0 ; i<m_nMaxWeapons ; i++)
	{
		WEAPON* pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(i));

		if(pWep)
		{
			if(m_pPlayerStats->HaveWeapon(pWep->nId))
				m_nLastWeapon = i;
		}
	}
	
	for(i=(DBYTE) m_nMaxWeapons ; i>0 ; i--)
	{
		WEAPON* pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(i-1));

		if(pWep)
		{
			if(m_pPlayerStats->HaveWeapon(pWep->nId))
				m_nFirstWeapon = i-1;
		}
	}

	m_fResetTime = m_fStartTime = g_pClientDE->GetTime();

	m_bIsOpen = DTRUE;
	m_bClosing = LTFALSE;

	return DTRUE;
}

// -----------------------------------------------/
//
// void CPredatorWepChooser::Close Implimentation
//
// Chooser close routine
//
// -----------------------------------------------/
void CPredatorWepChooser::Close()
{
	if(m_bClosing)
		return;

	//play the open sound
	if(!g_pGameClientShell->IsCinematic())
		g_pClientSoundMgr->PlaySoundLocal("PredatorInterfaceClose");

	m_fFinishTime = g_pClientDE->GetTime();
	m_fStartTime = 0.0f;
	m_bClosing = LTTRUE;
}

// -----------------------------------------------/
//
// void CPredatorWepChooser::NextWeapon Implimentation
//
// Increments the current weapon index
//
// -----------------------------------------------/

void CPredatorWepChooser::NextWeapon()
{
	//play the next sound
	g_pClientSoundMgr->PlaySoundLocal("PredatorInterfaceScroll");

	g_pClientDE->SetSurfaceAlpha(m_ahWeaponSurf[m_nCurWeapon],0.5f);


	m_nCurWeapon++;
	
	WEAPON* pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(m_nCurWeapon));

	for(int i=m_nCurWeapon; pWep && i<=(int)m_nLastWeapon && !m_pPlayerStats->HaveWeapon(pWep->nId); i++)
	{
		pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(i+1));
	}

	m_nCurWeapon = i;

	if(m_nCurWeapon > (int)m_nLastWeapon)
		m_nCurWeapon = m_nFirstWeapon;

	g_pClientDE->SetSurfaceAlpha(m_ahWeaponSurf[m_nCurWeapon],1.0f);
	
	m_fResetTime = g_pClientDE->GetTime();	

}

void CPredatorWepChooser::PrevWeapon()
{
	//play the next sound
	g_pClientSoundMgr->PlaySoundLocal("PredatorInterfaceScroll");

	g_pClientDE->SetSurfaceAlpha(m_ahWeaponSurf[m_nCurWeapon],0.5f);

	m_nCurWeapon--;
	
	WEAPON* pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(m_nCurWeapon));

	for(int i=m_nCurWeapon; pWep && i>(int)m_nFirstWeapon && !m_pPlayerStats->HaveWeapon(pWep->nId); i--)
	{
		pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(i-1));
	}

	m_nCurWeapon = i;

	if(m_nCurWeapon < (int)m_nFirstWeapon)
		m_nCurWeapon = m_nLastWeapon;

	g_pClientDE->SetSurfaceAlpha(m_ahWeaponSurf[m_nCurWeapon],1.0f);

	m_fResetTime = g_pClientDE->GetTime();	

}

void CPredatorWepChooser::Draw()
{
	float fTime = g_pClientDE->GetTime() - m_fResetTime;

	if (m_fStartTime > 0.0f && fTime > kfDelayTime)
	{
		Close();
		return;
	}

	// Need to make sure our list has not changed.
	for(DBYTE i=0 ; i<m_nMaxWeapons ; i++)
	{
		WEAPON* pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(i));

		if(pWep)
		{
			if(m_pPlayerStats->HaveWeapon(pWep->nId))
				m_nLastWeapon = i;
		}
	}
	
	for(i=(DBYTE) m_nMaxWeapons ; i>0 ; i--)
	{
		WEAPON* pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(i-1));

		if(pWep)
		{
			if(m_pPlayerStats->HaveWeapon(pWep->nId))
				m_nFirstWeapon = i-1;
		}
	}

	//set transparent color
	static HDECOLOR hTransColor = SETRGB_T(0,0,0);

	//get screen handle and dims
	HSURFACE	hScreen = g_pClientDE->GetScreenSurface();
	DDWORD		nScreenHeight, nScreenWidth;
	g_pClientDE->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);

	//set default X and Y drawing offsets
	DDWORD	nXOffset = (nScreenWidth>>1) - ((m_nMaxWeapons*(m_nIconWidth+kIconSpacing))>>1);
	DDWORD	nYOffset = 2;
	DFLOAT	fYDisplay;

	//calculate animation position for starting and ending
	if(m_fFinishTime)
	{
		//we are closing, move chooser off screen
		fYDisplay = nYOffset - (g_pClientDE->GetTime()-m_fFinishTime) * 600;

		nYOffset = (DDWORD)fYDisplay;

		//test to see if we are off screen and end
		if(fYDisplay + m_nIconHeight < 0.0f)
		{
			m_bIsOpen = DFALSE;
			m_fFinishTime = 0.0f;
		}
	}
	else
	{
		//we are opening, move choser on screen
		fYDisplay= (g_pClientDE->GetTime()-m_fStartTime) * 600;

		if(fYDisplay < nYOffset)
			nYOffset = (DDWORD)fYDisplay;
	}
	
	//set drawing sequence type
	DDWORD	nDrawIndex;
	
	//selected weapon is at the top of the list
	if(m_nCurWeapon == 0)
		nDrawIndex = 0;
	//selected weapon is at the end of the list
	else if(m_nCurWeapon == (int)m_nLastWeapon)
		nDrawIndex = 1;
	//selected weapon is in the middle of the list
	else nDrawIndex = 2;

	LITHFONTDRAWDATA lfDD;
	char str[3];
	HSTRING hStr = DNULL;

	//set drawdata elements
	lfDD.byJustify		= LTF_JUSTIFY_LEFT;
	lfDD.nLetterSpace	= 1;

	if(!g_pGameClientShell->IsCinematic())
	{
		for( i=0 ; i<m_nMaxWeapons ; i++ )
		{
			WEAPON* pWep = g_pWeaponMgr->GetWeapon(m_pPlayerStats->GetWeaponFromSet(i));

			if(pWep)
			{
				if(m_pPlayerStats->HaveWeapon(pWep->nId))
				{
					sprintf(str,"%d",(i+1)%10);

					hStr = g_pClientDE->CreateString(str);

					if(i==m_nCurWeapon)
					{
						g_pClientDE->SetSurfaceAlpha (m_hSelectBox, 1.0f);

						g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen,
																		m_hSelectBox, 
																		DNULL, 
																		nXOffset, nYOffset,
																		hTransColor);
					}
					else
					{
						g_pClientDE->SetSurfaceAlpha (m_hSelectBox, 0.5f);

						g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen,
																		m_hSelectBox, 
																		DNULL, 
																		nXOffset, nYOffset,
																		hTransColor);

					}

					g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen,
																	m_ahWeaponSurf[i], 
																	DNULL, 
																	nXOffset, nYOffset,
																	hTransColor);
					if(i==m_nCurWeapon)
						lfDD.fAlpha = 1.0f;
					else
						lfDD.fAlpha = 0.5f;

					m_plfIconFontWht->Draw(	hScreen,
											hStr,
											&lfDD, 
											nXOffset+2, 
											nYOffset+1,
											DNULL);

					DrawAmmoBars(hScreen, pWep->nId, nXOffset, nYOffset, i);

					g_pClientDE->FreeString( hStr );

				}

				nXOffset += m_nIconWidth + kIconSpacing;
			}
		}
	}
}

DBOOL CPredatorWepChooser::HaveAmmo(DBYTE nWeaponID)
{
	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponID);

	if(pWeapon)
	{
		//loop thru the barrels
		for(int j=0 ; j<NUM_BARRELS ; j++)
		{
			BARREL* pBarrel = g_pWeaponMgr->GetBarrel(pWeapon->aBarrelTypes[j]);

			BARREL* pBaseBarrel = pBarrel;

			if(pBarrel)
				do
				{
					if(pBarrel->bInfiniteAmmo || (m_pPlayerStats->GetAmmoCount(pBarrel->nAmmoType) > 0))
						return DTRUE;

					pBarrel = g_pWeaponMgr->GetBarrel(pBarrel->szNextBarrel);

				}while(pBarrel && pBarrel != pBaseBarrel);
		}
	}
	return DFALSE;
}

void CPredatorWepChooser::DrawAmmoBars(HSURFACE hScreen, DBYTE nWeaponID, DWORD nXOffset, DWORD nYOffset, DBYTE nId)
{
	//sanity check
	if(!g_pWeaponMgr || !m_pPlayerStats || !g_pClientDE) return;
	
	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponID);

	if(pWeapon)
	{
		int counter=0;

		for ( int i=0 ; i < NUM_BARRELS ; i++)
		{
			//get barrel info
			BARREL* pBarrel = g_pWeaponMgr->GetBarrel(pWeapon->aBarrelTypes[i]);
		
			if(pBarrel && pBarrel->bDrawChooserBars)
			{
				DFLOAT	fRatio		= 0.0f;
				AMMO*	pAmmo		= g_pWeaponMgr->GetAmmo(pBarrel->nAmmoType);
				AMMO_POOL*	pPool	= g_pWeaponMgr->GetAmmoPool(pAmmo->szAmmoPool);
				DDWORD	nAmmoCount	= m_pPlayerStats->GetAmmoCount(pBarrel->nAmmoType);
				DDWORD	nMaxAmmo	= pAmmo->GetMaxPoolAmount(NULL);

				if(strcmp(pPool->szName, "Predator_Energy") == 0)
				{
					int nIndex = (int)nAmmoCount < pAmmo->nAmmoPerShot?1:0;

					//set transparent color
					HDECOLOR hTransColor = SETRGB_T(0,0,0);

					if(nId == m_nCurWeapon)
						g_pClientDE->SetSurfaceAlpha (m_ahEnergyIcon[nIndex], 1.0f);
					else
						g_pClientDE->SetSurfaceAlpha (m_ahEnergyIcon[nIndex], 0.5f);

					g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen,
																	m_ahEnergyIcon[nIndex], 
																	DNULL, 
																	nXOffset+m_nIconWidth-20, nYOffset+5,
																	hTransColor);
				}
				else
				{
					if(nMaxAmmo)
						fRatio = (DFLOAT)nAmmoCount / nMaxAmmo;

					//set fill color
					HDECOLOR hFillColor = SETRGB_T(0,44,125);

					if(nId == m_nCurWeapon)
						hFillColor = SETRGB_T(0,88,250);

					DRect rect;

					rect.right	= nXOffset+m_nIconWidth-5;
					rect.left	= rect.right - (int)(fRatio*20);
					rect.top	= nYOffset+5+(i*5);
					rect.bottom	= rect.top+2;

					g_pClientDE->FillRect(hScreen,&rect,hFillColor);
				}
			}
		}
	}
}

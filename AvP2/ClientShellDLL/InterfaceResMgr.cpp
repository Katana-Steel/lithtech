// InterfaceResMgr.cpp: implementation of the CInterfaceResMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ClientRes.h"
#include "gameclientshell.h"
#include "InterfaceResMgr.h"
#include "ClientButeMgr.h"

CInterfaceResMgr*	g_pInterfaceResMgr = LTNULL;

namespace
{
	char g_szFontNames[3][128];
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInterfaceResMgr::CInterfaceResMgr()
{
	g_pInterfaceResMgr = this;

	m_hSurfUpArrow = NULL;
	m_hSurfDownArrow = NULL;
	m_hSurfLeftArrow = NULL;
	m_hSurfRightArrow = NULL;
	m_hSurfLoading	 = NULL;
	m_hBlank		 = NULL;
	m_hTransColor = SETRGB(0,0,0);

	m_pTitleFont = LTNULL;
	m_pLargeFont = LTNULL;
	m_pSmallFont = LTNULL;
	m_pTinyFont = LTNULL;
	m_pLargeFixedFont = LTNULL;
	m_pSmallFixedFont = LTNULL;
	m_pTinyFixedFont = LTNULL;
	m_pHelpFont = LTNULL;
	m_pMessageFont = LTNULL;
	m_pObjectiveFont = LTNULL;

	m_pAlienFont = LTNULL;
	m_pMarineFont = LTNULL;
	m_pPredatorFont = LTNULL;

	m_pPDAFont = LTNULL;
	m_pMEMOFont = LTNULL;

	m_Offset.x =  -1;
	m_Offset.y =  -1;

	m_fXRatio = 1.0f;
	m_fYRatio = 1.0f;
					
}

CInterfaceResMgr::~CInterfaceResMgr()
{
	g_pInterfaceResMgr = LTNULL;
}

//////////////////////////////////////////////////////////////////////
// Function name	: CInterfaceResMgr::Init
// Description	    : 
// Return type		: DBOOL 
// Argument         : CClientDE* pClientDE
// Argument         : CGameClientShell* pClientShell
//////////////////////////////////////////////////////////////////////
DBOOL CInterfaceResMgr::Init(CClientDE* pClientDE, CGameClientShell* pClientShell)
{
	if (!pClientDE)
	{
		return LTFALSE;
	}
	

	// Set the English flag
	HSTRING hString=g_pClientDE->FormatString(IDS_GAME_LANGUAGE);
	if (hString && _mbsicmp((const unsigned char*)"english", (const unsigned char*)g_pClientDE->GetStringData(hString)) != 0)
	{
		m_bEnglish=LTFALSE;
	}
	else
	{
		m_bEnglish=LTTRUE;
	}
	g_pClientDE->FreeString(hString);
	hString=LTNULL;

	// Load the virtual key codes for yes responses
	hString=g_pClientDE->FormatString(IDS_MENU_VKEY_YES);
	if (hString)
	{
		m_nYesVKeyCode=atoi(g_pClientDE->GetStringData(hString));
		g_pClientDE->FreeString(hString);
		hString=LTNULL;
	}

	// Load the virtual key codes for no responses
	hString=g_pClientDE->FormatString(IDS_MENU_VKEY_NO);
	if (hString)
	{
		m_nNoVKeyCode=atoi(g_pClientDE->GetStringData(hString));
		g_pClientDE->FreeString(hString);
		hString=LTNULL;
	}

	// Init the InterfaceSurfMgr class
	m_InterfaceSurfMgr.Init(g_pClientDE);


	ScreenDimsChanged();

	// Initialize the fonts
	if (!InitFonts())
	{
		return LTFALSE;
	}

	m_hBlank = g_pClientDE->CreateSurface(2, 2);
	if (m_hBlank)
	{
		LTRect rcSrc(0,0,2,2);
		g_pClientDE->SetSurfaceAlpha(m_hBlank, 0.75f);
		g_pClientDE->FillRect(m_hBlank, &rcSrc, kNearBlack);
		g_pClientDE->OptimizeSurface(m_hBlank, kBlack);
	}
	else
		return LTFALSE;

	return LTTRUE;
}

//////////////////////////////////////////////////////////////////////
// Function name	: CInterfaceResMgr::Term
// Description	    : 
// Return type		: void 
//////////////////////////////////////////////////////////////////////

void CInterfaceResMgr::Term()
{
	Clean();

	// Terminate the InterfaceSurfMgr class
	m_InterfaceSurfMgr.Term();

	if (m_hBlank)
	{
		g_pClientDE->DeleteSurface(m_hBlank);
		m_hBlank = NULL;
	}

	if ( m_pHelpFont )
	{
		m_pHelpFont->Term();
		delete m_pHelpFont;
		m_pHelpFont=LTNULL;
	}

	if ( m_pSmallFont )
	{
		m_pSmallFont->Term();
		delete m_pSmallFont;
		m_pSmallFont=LTNULL;
	}
	if ( m_pTinyFont )
	{
		m_pTinyFont->Term();
		delete m_pTinyFont;
		m_pTinyFont=LTNULL;
	}
	if ( m_pLargeFont )
	{
		m_pLargeFont->Term();
		delete m_pLargeFont;
		m_pLargeFont=LTNULL;
	}

	if ( m_pSmallFixedFont )
	{
		m_pSmallFixedFont->Term();
		delete m_pSmallFixedFont;
		m_pSmallFixedFont=LTNULL;
	}
	if ( m_pLargeFixedFont )
	{
		m_pLargeFixedFont->Term();
		delete m_pLargeFixedFont;
		m_pLargeFixedFont=LTNULL;
	}
	if ( m_pTinyFixedFont )
	{
		m_pTinyFixedFont->Term();
		delete m_pTinyFixedFont;
		m_pTinyFixedFont=LTNULL;
	}


	if ( m_pTitleFont )
	{
		m_pTitleFont->Term();
		delete m_pTitleFont;
		m_pTitleFont=LTNULL;
	}

	if ( m_pMessageFont )
	{
		m_pMessageFont->Term();
		delete m_pMessageFont;
		m_pMessageFont=LTNULL;
	}

	if ( m_pObjectiveFont )
	{
		m_pObjectiveFont->Term();
		delete m_pObjectiveFont;
		m_pObjectiveFont=LTNULL;
	}


	if ( m_pAlienFont )
	{
		m_pAlienFont->Term();
		delete m_pAlienFont;
		m_pAlienFont=LTNULL;
	}
	if ( m_pMarineFont )
	{
		m_pMarineFont->Term();
		delete m_pMarineFont;
		m_pMarineFont=LTNULL;
	}
	if ( m_pPredatorFont )
	{
		m_pPredatorFont->Term();
		delete m_pPredatorFont;
		m_pPredatorFont=LTNULL;
	}

	if ( m_pPDAFont )
	{
		m_pPDAFont->Term();
		delete m_pPDAFont;
		m_pPDAFont=LTNULL;
	}

	if ( m_pMEMOFont )
	{
		m_pMEMOFont->Term();
		delete m_pMEMOFont;
		m_pMEMOFont=LTNULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Function name	: CInterfaceResMgr::Setup
// Description	    : 
// Return type		: DBOOL
//////////////////////////////////////////////////////////////////////

DBOOL CInterfaceResMgr::Setup()
{
	return LTTRUE;
}

//////////////////////////////////////////////////////////////////////
// Function name	: CInterfaceResMgr::Clean
// Description	    : 
// Return type		: void 
//////////////////////////////////////////////////////////////////////

void CInterfaceResMgr::Clean()
{
	if (g_pClientDE)
	{
		if (m_hSurfLoading)
		{
			g_pClientDE->DeleteSurface(m_hSurfLoading);
			m_hSurfLoading = NULL;
		}
		if (m_hSurfUpArrow)
		{
			g_pClientDE->DeleteSurface(m_hSurfUpArrow);
			m_hSurfUpArrow = NULL;
		}
		if (m_hSurfDownArrow)
		{
			g_pClientDE->DeleteSurface(m_hSurfDownArrow);
			m_hSurfDownArrow = NULL;
		}
		if (m_hSurfLeftArrow)
		{
			g_pClientDE->DeleteSurface(m_hSurfLeftArrow);
			m_hSurfLeftArrow = NULL;
		}
		if (m_hSurfRightArrow)
		{
			g_pClientDE->DeleteSurface(m_hSurfRightArrow);
			m_hSurfRightArrow = NULL;
		}

		// free shared surfaces
		m_InterfaceSurfMgr.FreeAllSurfaces();


	}
}


//////////////////////////////////////////////////////////////////////
// Function name	: CInterfaceResMgr::DrawFolder
// Description	    :
// Return type		: void
//////////////////////////////////////////////////////////////////////
void CInterfaceResMgr::DrawFolder()
{
    _ASSERT(g_pLTClient);
    if (!g_pLTClient) return;

	if (m_Offset.x < 0)
		ScreenDimsChanged();

	// The screen surface
    HSURFACE hScreen = g_pLTClient->GetScreenSurface();

	// Render the current folder
	g_pInterfaceMgr->GetFolderMgr()->Render(hScreen);

	return;
}


void CInterfaceResMgr::DrawMessage(CLTGUIFont* pFont, int nMessageId)
{
	_ASSERT(g_pClientDE && pFont);
	if (!g_pClientDE || !pFont) return;

	// The screen surface
	HSURFACE hScreen = g_pClientDE->GetScreenSurface();
	DDWORD dwScreenWidth=0;
	DDWORD dwScreenHeight=0;
	DDWORD dwWidth=0;
	DDWORD dwHeight=0;


	// Get the string...
	HSTRING hStr = g_pClientDE->FormatString(nMessageId);

	_ASSERT(hStr);
	if (!hStr)
	{
		return;
	}

	// Get the dims of the screen and the working surface
	g_pClientDE->GetSurfaceDims(hScreen, &dwScreenWidth, &dwScreenHeight);	

	// Center the string on the surface...
	DIntPt size = pFont->GetTextExtentsFormat(hStr,(dwScreenWidth/2));
	dwWidth = size.x + 16;
	dwHeight = size.y + 16;
	if (dwHeight < dwWidth/2)
		dwHeight = dwWidth/2;

	// Center the image and blit it to the screen
	int nLeft= (dwScreenWidth-dwWidth)/2;
	int nTop= (dwScreenHeight-dwHeight)/2;
	LTRect rcDest(nLeft,nTop,nLeft+dwWidth,nTop+dwHeight);
	g_pLTClient->ScaleSurfaceToSurface(	hScreen, m_hBlank, &rcDest, LTNULL);

	pFont->DrawFormat(hStr, hScreen, (dwScreenWidth - size.x)/2, (dwScreenHeight - size.y)/2, (dwScreenWidth/2));
	g_pClientDE->FreeString(hStr);

	return;
}


DBOOL CInterfaceResMgr::InitFonts()
{
	
	m_pLargeFont = new CLTGUIFont;
	m_pSmallFont = new CLTGUIFont;
	m_pTinyFont = new CLTGUIFont;
	m_pLargeFixedFont = new CLTGUIFont;
	m_pSmallFixedFont = new CLTGUIFont;
	m_pTinyFixedFont = new CLTGUIFont;

	m_pMessageFont = new CLTGUIFont;
	m_pObjectiveFont = new CLTGUIFont;
	m_pHelpFont = new CLTGUIFont;
	m_pTitleFont = new CLTGUIFont;

	m_pAlienFont = new CLTGUIFont;
	m_pMarineFont = new CLTGUIFont;
	m_pPredatorFont = new CLTGUIFont;

	m_pPDAFont = new CLTGUIFont;
	m_pMEMOFont = new CLTGUIFont;

	// Initialize the bitmap fonts if we are in english
	if (IsEnglish())
	{

		//************* help font
		g_pLayoutMgr->GetHelpFont(g_szFontNames[0],sizeof(g_szFontNames[0]));
		if (!SetupFont(m_pHelpFont))
		{
			delete m_pHelpFont;
			m_pHelpFont=LTNULL;
			return LTFALSE;
		}

		char szName[128] = "";		

		//*********** small fixed font
		g_pLayoutMgr->GetSmallFontBase(szName,sizeof(szName));
		sprintf(g_szFontNames[0],"%sn.pcx",szName);
		if (!SetupFont(m_pSmallFixedFont))
		{
			delete m_pSmallFixedFont;
			m_pSmallFixedFont=LTNULL;
			return LTFALSE;
		}

		//*********** small font
		g_pLayoutMgr->GetSmallFontBase(szName,sizeof(szName));
		sprintf(g_szFontNames[0],"%sn.pcx",szName);
		sprintf(g_szFontNames[1],"%sh.pcx",szName);
		sprintf(g_szFontNames[2],"%sd.pcx",szName);
		if (!SetupFont(m_pSmallFont,LTFALSE))
		{
			delete m_pSmallFont;
			m_pSmallFont=LTNULL;
			return LTFALSE;
		}

		//*********** tiny fixed font
		g_pLayoutMgr->GetTinyFontBase(szName,sizeof(szName));
		sprintf(g_szFontNames[0],"%sn.pcx",szName);
		if (!SetupFont(m_pTinyFixedFont))
		{
			delete m_pTinyFixedFont;
			m_pTinyFixedFont=LTNULL;
			return LTFALSE;
		}

		//*********** tiny font
		g_pLayoutMgr->GetTinyFontBase(szName,sizeof(szName));
		sprintf(g_szFontNames[0],"%sn.pcx",szName);
		sprintf(g_szFontNames[1],"%sh.pcx",szName);
		sprintf(g_szFontNames[2],"%sd.pcx",szName);
		if (!SetupFont(m_pTinyFont,LTFALSE))
		{
			delete m_pTinyFont;
			m_pTinyFont=LTNULL;
			return LTFALSE;
		}

		//*********** large fixed font
		g_pLayoutMgr->GetLargeFontBase(szName,sizeof(szName));
		sprintf(g_szFontNames[0],"%sn.pcx",szName);
		if (!SetupFont(m_pLargeFixedFont))
		{
			delete m_pLargeFixedFont;
			m_pLargeFixedFont=LTNULL;
			return LTFALSE;
		}

		//*********** large  font
		g_pLayoutMgr->GetLargeFontBase(szName,sizeof(szName));
		sprintf(g_szFontNames[0],"%sn.pcx",szName);
		sprintf(g_szFontNames[1],"%sh.pcx",szName);
		sprintf(g_szFontNames[2],"%sd.pcx",szName);
		if (!SetupFont(m_pLargeFont, LTFALSE))
		{
			delete m_pLargeFont;
			m_pLargeFont=LTNULL;
			return LTFALSE;
		}


		//************* Message font
		g_pLayoutMgr->GetMessageFont(g_szFontNames[0],sizeof(g_szFontNames[0]));
		if (!SetupFont(m_pMessageFont))
		{
			delete m_pMessageFont;
			m_pMessageFont=LTNULL;
			return LTFALSE;
		}

		//************* Objective font
		g_pLayoutMgr->GetObjectiveFont(g_szFontNames[0],sizeof(g_szFontNames[0]));
		if (!SetupFont(m_pObjectiveFont))
		{
			delete m_pObjectiveFont;
			m_pObjectiveFont=LTNULL;
			return LTFALSE;
		}

		//************* Title font
		g_pLayoutMgr->GetTitleFont(g_szFontNames[0],sizeof(g_szFontNames[0]));
		if (!SetupFont(m_pTitleFont))
		{
			delete m_pTitleFont;
			m_pTitleFont=LTNULL;
			return LTFALSE;
		}

#ifndef _DEMO
		//*********** Alien font
		g_pLayoutMgr->GetAlienFontBase(szName,sizeof(szName));
		sprintf(g_szFontNames[0],"%sn.pcx",szName);
		sprintf(g_szFontNames[1],"%sh.pcx",szName);
		sprintf(g_szFontNames[2],"%sd.pcx",szName);
		if (!SetupFont(m_pAlienFont,LTFALSE))
		{
			delete m_pAlienFont;
			m_pAlienFont=LTNULL;
			return LTFALSE;
		}
#endif

		//*********** Marine font
		g_pLayoutMgr->GetMarineFontBase(szName,sizeof(szName));
		sprintf(g_szFontNames[0],"%sn.pcx",szName);
		sprintf(g_szFontNames[1],"%sh.pcx",szName);
		sprintf(g_szFontNames[2],"%sd.pcx",szName);
		if (!SetupFont(m_pMarineFont,LTFALSE))
		{
			delete m_pMarineFont;
			m_pMarineFont=LTNULL;
			return LTFALSE;
		}

#ifndef _DEMO
		//*********** Predator font
		g_pLayoutMgr->GetPredatorFontBase(szName,sizeof(szName));
		sprintf(g_szFontNames[0],"%sn.pcx",szName);
		sprintf(g_szFontNames[1],"%sh.pcx",szName);
		sprintf(g_szFontNames[2],"%sd.pcx",szName);
		if (!SetupFont(m_pPredatorFont,LTFALSE))
		{
			delete m_pPredatorFont;
			m_pPredatorFont=LTNULL;
			return LTFALSE;
		}
#endif

		//*********** PDA font
		g_pLayoutMgr->GetPDAFontBase(szName,sizeof(szName));
		sprintf(g_szFontNames[0],"%sn.pcx",szName);
		sprintf(g_szFontNames[1],"%sh.pcx",szName);
		sprintf(g_szFontNames[2],"%sd.pcx",szName);
		if (!SetupFont(m_pPDAFont,LTFALSE))
		{
			delete m_pPDAFont;
			m_pPDAFont=LTNULL;
			return LTFALSE;
		}

		//*********** MEMO font
		g_pLayoutMgr->GetMEMOFontBase(szName,sizeof(szName));
		sprintf(g_szFontNames[0],"%sn.pcx",szName);
		sprintf(g_szFontNames[1],"%sh.pcx",szName);
		sprintf(g_szFontNames[2],"%sd.pcx",szName);
		if (!SetupFont(m_pMEMOFont,LTFALSE))
		{
			delete m_pMEMOFont;
			m_pMEMOFont=LTNULL;
			return LTFALSE;
		}

		DDWORD dwFlags = LTF_INCLUDE_SYMBOLS_1 | LTF_INCLUDE_NUMBERS | LTF_INCLUDE_SYMBOLS_2;
	}
	else
	{
		InitEngineFont(m_pLargeFont,		IDS_FONT_LARGE,		IDS_FONT_LARGE_WIDTH,		IDS_FONT_LARGE_HEIGHT,		IDS_FONT_LARGE_NORMAL,		IDS_FONT_LARGE_SELECTED,		IDS_FONT_LARGE_DISABLED,	LTFALSE);
		InitEngineFont(m_pSmallFont,		IDS_FONT_MEDIUM,	IDS_FONT_MEDIUM_WIDTH,		IDS_FONT_MEDIUM_HEIGHT,		IDS_FONT_MEDIUM_NORMAL,		IDS_FONT_MEDIUM_SELECTED,		IDS_FONT_MEDIUM_DISABLED,	LTFALSE);
		InitEngineFont(m_pTinyFont,			IDS_FONT_SMALL,		IDS_FONT_SMALL_WIDTH,		IDS_FONT_SMALL_HEIGHT,		IDS_FONT_SMALL_NORMAL,		IDS_FONT_SMALL_SELECTED,		IDS_FONT_SMALL_DISABLED,	LTFALSE);

		InitEngineFont(m_pLargeFixedFont,	IDS_FONT_LARGE,		IDS_FONT_LARGE_WIDTH,		IDS_FONT_LARGE_HEIGHT,		IDS_FONT_LARGE_SELECTED,	IDS_FONT_LARGE_SELECTED,		IDS_FONT_LARGE_SELECTED,	LTTRUE);
		InitEngineFont(m_pSmallFixedFont,	IDS_FONT_MEDIUM,	IDS_FONT_MEDIUM_WIDTH,		IDS_FONT_MEDIUM_HEIGHT,		IDS_FONT_MEDIUM_SELECTED,	IDS_FONT_MEDIUM_SELECTED,		IDS_FONT_MEDIUM_SELECTED,	LTTRUE);
		InitEngineFont(m_pTinyFixedFont,	IDS_FONT_SMALL,		IDS_FONT_SMALL_WIDTH,		IDS_FONT_SMALL_HEIGHT,		IDS_FONT_SMALL_SELECTED,	IDS_FONT_SMALL_SELECTED,		IDS_FONT_SMALL_SELECTED,	LTTRUE);

		InitEngineFont(m_pMessageFont,		IDS_FONT_MESSAGE,	IDS_FONT_MESSAGE_WIDTH,		IDS_FONT_MESSAGE_HEIGHT,	IDS_FONT_LARGE_SELECTED,	IDS_FONT_LARGE_SELECTED,		IDS_FONT_LARGE_SELECTED,	LTTRUE);
		InitEngineFont(m_pObjectiveFont,	IDS_FONT_OBJECTIVE,	IDS_FONT_OBJECTIVE_WIDTH,	IDS_FONT_OBJECTIVE_HEIGHT,	IDS_FONT_OBJECTIVE_SELECTED,IDS_FONT_OBJECTIVE_SELECTED,	IDS_FONT_OBJECTIVE_SELECTED,LTTRUE);
		InitEngineFont(m_pHelpFont,			IDS_FONT_SMALL,		IDS_FONT_SMALL_WIDTH,		IDS_FONT_SMALL_HEIGHT,		IDS_FONT_SMALL_SELECTED,	IDS_FONT_SMALL_SELECTED,		IDS_FONT_SMALL_SELECTED,	LTTRUE);
		InitEngineFont(m_pTitleFont,		IDS_FONT_TITLE,		IDS_FONT_TITLE_WIDTH,		IDS_FONT_TITLE_HEIGHT,		IDS_FONT_TITLE_SELECTED,	IDS_FONT_TITLE_SELECTED,		IDS_FONT_TITLE_SELECTED,	LTTRUE);

		InitEngineFont(m_pAlienFont,		IDS_FONT_ALIEN,		IDS_FONT_ALIEN_WIDTH,		IDS_FONT_ALIEN_HEIGHT,		IDS_FONT_ALIEN_NORMAL,		IDS_FONT_ALIEN_SELECTED,		IDS_FONT_ALIEN_DISABLED,	LTFALSE);
		InitEngineFont(m_pMarineFont,		IDS_FONT_MARINE,	IDS_FONT_MARINE_WIDTH,		IDS_FONT_MARINE_HEIGHT,		IDS_FONT_MARINE_NORMAL,		IDS_FONT_MARINE_SELECTED,		IDS_FONT_MARINE_DISABLED,	LTFALSE);
		InitEngineFont(m_pPredatorFont,		IDS_FONT_PREDATOR,	IDS_FONT_PREDATOR_WIDTH,	IDS_FONT_PREDATOR_HEIGHT,	IDS_FONT_PREDATOR_NORMAL,	IDS_FONT_PREDATOR_SELECTED,		IDS_FONT_PREDATOR_DISABLED,	LTFALSE);

		InitEngineFont(m_pPDAFont,			IDS_FONT_MESSAGE,	IDS_FONT_MESSAGE_WIDTH,		IDS_FONT_MESSAGE_HEIGHT,	IDS_FONT_MESSAGE_NORMAL,	IDS_FONT_MESSAGE_SELECTED,		IDS_FONT_MESSAGE_DISABLED,	LTFALSE);
		InitEngineFont(m_pMEMOFont,			IDS_FONT_MESSAGE,	IDS_FONT_MESSAGE_WIDTH,		IDS_FONT_MESSAGE_HEIGHT,	IDS_FONT_MESSAGE_NORMAL,	IDS_FONT_MESSAGE_SELECTED,		IDS_FONT_MESSAGE_DISABLED,	LTFALSE);
	}


	return LTTRUE;
}

//*******************************************************************

// Initialize an engine font from string IDs that represent the name, width, and height
DBOOL CInterfaceResMgr::InitEngineFont(CLTGUIFont *pFont, int nNameID, int nWidthID, int nHeightID, int nN, int nH, int nD, LTBOOL bFixed)
{
	if (!pFont)
	{
		return LTFALSE;
	}


	LTBOOL bResult = LTTRUE;

	// Get the font name, width, and height
	HSTRING hName=g_pClientDE->FormatString(nNameID);
	HSTRING hWidth=g_pClientDE->FormatString(nWidthID);
	HSTRING hHeight=g_pClientDE->FormatString(nHeightID);
	HSTRING hNormal=g_pClientDE->FormatString(nN);
	HSTRING hHigh=g_pClientDE->FormatString(nH);
	HSTRING hDisabled=g_pClientDE->FormatString(nD);

	int nWidth=atoi(g_pClientDE->GetStringData(hWidth));
	int nHeight=atoi(g_pClientDE->GetStringData(hHeight));

	HLTCOLOR nColors[3];
	
	nColors[0]=atoi(g_pClientDE->GetStringData(hNormal));
	nColors[1]=atoi(g_pClientDE->GetStringData(hHigh));
	nColors[2]=atoi(g_pClientDE->GetStringData(hDisabled));

	if(bFixed)
	{
		//set create params
		LITHFONTCREATESTRUCT lfCreateParams;

		lfCreateParams.szFontName	= g_pClientDE->GetStringData(hName);
		lfCreateParams.nWidth		= nWidth;
		lfCreateParams.nHeight		= nHeight;
		lfCreateParams.hDefaultColor= nColors[0];

		// Initialize the font
		bResult = pFont->Init(g_pClientDE, &lfCreateParams);//g_pClientDE->GetStringData(hName), nWidth, nHeight, LTFALSE, LTFALSE, LTFALSE);

		if (!bResult)
		{				
			char szString[1024];
			sprintf(szString, "Cannot initialize font: %s", g_pClientDE->GetStringData(hName));
			g_pClientDE->CPrint(szString);		
		}
	}
	else
	{
		LITHFONTCREATESTRUCT lithFonts[3];

		for (int i=0; i < 3; i++)
		{
			lithFonts[i].szFontName		= g_pClientDE->GetStringData(hName);
			lithFonts[i].nWidth			= nWidth;
			lithFonts[i].nHeight		= nHeight;
			lithFonts[i].hDefaultColor	= nColors[i];
		}

		if ( !pFont || !pFont->Init(g_pClientDE, &lithFonts[0], &lithFonts[1], &lithFonts[2]) )
		{
			char szString[512];
			sprintf(szString, "Cannot load font: %s", g_szFontNames[0]);
			g_pClientDE->CPrint(szString);

			bResult = LTFALSE;
		}
	}

	// Free the strings
	g_pClientDE->FreeString(hName);
	g_pClientDE->FreeString(hWidth);
	g_pClientDE->FreeString(hHeight);
	g_pClientDE->FreeString(hNormal);
	g_pClientDE->FreeString(hHigh);
	g_pClientDE->FreeString(hDisabled);

	return bResult;
}

// -------------------------------------------------------------------------------- //

DBOOL CInterfaceResMgr::InitEngineFont(CLTGUIFont *pFont, char *lpszName, int nWidth, int nHeight, HLTCOLOR hColor)
{
	if (!pFont)
	{
		return LTFALSE;
	}

	LITHFONTCREATESTRUCT lfCreateParams;

	// Get the font name, width, and height
	HSTRING hName=g_pClientDE->CreateString(lpszName);

	//set create params
	lfCreateParams.szFontName	= g_pClientDE->GetStringData(hName);
	lfCreateParams.nWidth		= nWidth;
	lfCreateParams.nHeight		= nHeight;
	lfCreateParams.hDefaultColor= hColor;

	// Initialize the font
	DBOOL bResult; 
	bResult=pFont->Init(g_pClientDE, &lfCreateParams);//g_pClientDE->GetStringData(hName), nWidth, nHeight, LTFALSE, LTFALSE, LTFALSE);

	if (!bResult)
	{				
		char szString[1024];
		sprintf(szString, "Cannot initialize font: %s", g_pClientDE->GetStringData(hName));
		g_pClientDE->CPrint(szString);		
	}

	// Free the strings
	g_pClientDE->FreeString(hName);

	return bResult;

}

void CInterfaceResMgr::ScreenDimsChanged()
{
	if (!g_pClientDE) return;

	RMode currentMode;
	g_pClientDE->GetRenderMode(&currentMode);

	m_Offset.x = (int)(currentMode.m_Width - 640) / 2;
	m_Offset.y = (int)(currentMode.m_Height - 480) / 2;

	m_fXRatio = (float)currentMode.m_Width / 640.0f;
	m_fYRatio = (float)currentMode.m_Height / 480.0f;

	m_dwScreenWidth = currentMode.m_Width;
	m_dwScreenHeight = currentMode.m_Height;

}


const char *CInterfaceResMgr::GetSoundSelect()
{
	if (m_csSoundSelect.IsEmpty())
	{
		m_csSoundSelect = g_pClientButeMgr->GetInterfaceAttributeString("SelectSound");
	}
	_ASSERT(!m_csSoundSelect.IsEmpty());
	return m_csSoundSelect;
};

const char *CInterfaceResMgr::GetSoundChoose()
{
	if (m_csSoundChoose.IsEmpty())
	{
		m_csSoundChoose = g_pClientButeMgr->GetInterfaceAttributeString("ChooseSound");
	}
	_ASSERT(!m_csSoundChoose.IsEmpty());
	return m_csSoundChoose;
};

const char *CInterfaceResMgr::GetSoundFolderChange()
{
	if (m_csSoundFolderChange.IsEmpty())
	{
		m_csSoundFolderChange = g_pClientButeMgr->GetInterfaceAttributeString("FolderChangeSound");
	}
	_ASSERT(!m_csSoundFolderChange.IsEmpty());
	return m_csSoundFolderChange;
};

const char *CInterfaceResMgr::GetSoundFolderOpen()
{
	if (m_csSoundFolderOpen.IsEmpty())
	{
		m_csSoundFolderOpen = g_pClientButeMgr->GetInterfaceAttributeString("FolderOpenSound");
	}
	_ASSERT(!m_csSoundFolderOpen.IsEmpty());
	return m_csSoundFolderOpen;
};


const char *CInterfaceResMgr::GetSoundPageChange()
{
	if (m_csSoundPageChange.IsEmpty())
	{
		m_csSoundPageChange = g_pClientButeMgr->GetInterfaceAttributeString("PageChangeSound");
	}
	_ASSERT(!m_csSoundPageChange.IsEmpty());
	return m_csSoundPageChange;
};


// Creates a surface just large enough for the string.
// You can make the surface a little larger with extraPixelsX and extraPixelsY.
HSURFACE CInterfaceResMgr::CreateSurfaceFromString(CLTGUIFont *pFont, HSTRING hString, HDECOLOR hBackColor, 
												int extraPixelsX, int extraPixelsY, int nWidth)
{
	DIntPt sz(0,0);// =	{0,0};

	if( pFont )
	{
		if (nWidth > 0)
			sz = pFont->GetTextExtentsFormat(hString,nWidth);
		else
			sz = pFont->GetTextExtents(hString);
	}

	DRect rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = sz.x+extraPixelsX;
	rect.bottom = sz.y+extraPixelsY;

	HSURFACE hSurf	= g_pClientDE->CreateSurface(sz.x+extraPixelsX,sz.y+extraPixelsY);
	g_pClientDE->FillRect(hSurf,&rect,hBackColor);

	if( pFont )
		pFont->Draw(hString, hSurf,extraPixelsX/2,extraPixelsY/2,LTF_JUSTIFY_LEFT);

	return hSurf;

}
HSURFACE CInterfaceResMgr::CreateSurfaceFromString(CLTGUIFont *pFont, int nStringId, HDECOLOR hBackColor, 
												int extraPixelsX, int extraPixelsY, int nWidth)
{
	HSTRING hString = g_pClientDE->FormatString(nStringId);
	return CreateSurfaceFromString(pFont, hString, hBackColor, extraPixelsX, extraPixelsY, nWidth);
	g_pClientDE->FreeString(hString);
}
HSURFACE CInterfaceResMgr::CreateSurfaceFromString(CLTGUIFont *pFont, char *lpszString, HDECOLOR hBackColor, 
												int extraPixelsX, int extraPixelsY, int nWidth)
{
	HSTRING hString = g_pClientDE->CreateString(lpszString);
	return CreateSurfaceFromString(pFont, hString, hBackColor, extraPixelsX, extraPixelsY, nWidth);
	g_pClientDE->FreeString(hString);
}



DBOOL CInterfaceResMgr::SetupFont(CLTGUIFont *pFont, DBOOL bFixed, DDWORD dwFlags)
{
	if (bFixed)
	{
		LITHFONTCREATESTRUCT lithFont;
		lithFont.szFontBitmap = g_szFontNames[0];
		lithFont.nGroupFlags = dwFlags;
		lithFont.hTransColor = SETRGB(0,0,0);
		lithFont.bChromaKey	= LTTRUE;
		if ( !pFont || !pFont->Init(g_pClientDE, &lithFont) )
		{
			char szString[512];
			sprintf(szString, "Cannot load font: %s", g_szFontNames[0]);
			g_pClientDE->CPrint(szString);

			return LTFALSE;
		}
	}
	else
	{
		LITHFONTCREATESTRUCT lithFonts[3];
		for (int i=0; i < 3; i++)
		{
			lithFonts[i].szFontBitmap = g_szFontNames[i];
			lithFonts[i].nGroupFlags = dwFlags;
			lithFonts[i].hTransColor = SETRGB(0,0,0);
			lithFonts[i].bChromaKey	= LTTRUE;
		}

		if ( !pFont || !pFont->Init(g_pClientDE, &lithFonts[0], &lithFonts[1], &lithFonts[2]) )
		{
			char szString[512];
			sprintf(szString, "Cannot load font: %s", g_szFontNames[0]);
			g_pClientDE->CPrint(szString);

			return LTFALSE;
		}
	}

	return LTTRUE;
}



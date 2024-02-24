// BaseFolder.cpp: implementation of the CBaseFolder class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "BaseFolder.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "SoundMgr.h"
#include "InterfaceMgr.h"
#include "VKDefs.h"
#include "GameClientShell.h"
#include "ClientRes.h"
//#include "AttachButeMGR.h"
#include "CharacterButeMgr.h"
#include "ServerUtilities.h"		// For DEG2RAD()
#include "ServerOptionMgr.h"
#include "FolderJoin.h"
#include "FolderJoinInfo.h"
#include "FolderMulti.h"
#include "FXButeMgr.h"

#include <algorithm>

extern CGameClientShell* g_pGameClientShell;

LTBOOL      CBaseFolder::m_bReadLayout = LTFALSE;
LTIntPt     CBaseFolder::m_BackPos;
LTIntPt     CBaseFolder::m_ContinuePos;
LTIntPt     CBaseFolder::m_MainPos;
char		CBaseFolder::m_sArrowBack[128];
char		CBaseFolder::m_sArrowNext[128];
LTIntPt		CBaseFolder::m_ArrowBackPos;
LTIntPt     CBaseFolder::m_ArrowNextPos;
HSURFACE	CBaseFolder::m_hHelpSurf = LTNULL;
HSURFACE	CBaseFolder::m_hTabSurf = LTNULL;

char		CBaseFolder::m_sDefaultChangeSound[128] = "Sounds\\Menu\\selectchange.wav";
char		CBaseFolder::m_sDefaultSelectSound[128] = "Sounds\\Menu\\select.wav";

static int	m_nLogoScaleFX;


namespace
{
	enum LocalCommands
	{
		CMD_HOSTCONFIRM = FOLDER_CMD_CUSTOM + 50,
	};

	void HostConfirmCallBack(LTBOOL bReturn, void *pData)
	{
		CBaseFolder *pThisFolder = (CBaseFolder*)pData;

		if (bReturn && pThisFolder)
			pThisFolder->SendCommand(CMD_HOSTCONFIRM,0,0);
	}

	LTIntPt offscreen(-2000,0);
    LTVector g_vPos, g_vU, g_vR, g_vF;
    LTRotation g_rRot;

	HSTRING hDemoBuildVersion;
	LTIntPt DemoBuildPos;

	const int kTabHeight = 25;

	LTIntPt ptDlgPWSize;
	LTIntPt ptDlgPWEditPW;
	int ptDlgPWOffset;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBaseFolder::CBaseFolder()
{
    m_bInit = LTFALSE;
    m_bBuilt = LTFALSE;
    m_bHasFocus = LTFALSE;

    m_pFolderMgr = LTNULL;

    m_hTitleString = LTNULL;

	m_hTransparentColor = SETRGB_T(0,0,0);
    m_hTitleColor = kWhite;

	m_titlePos.x = 0;
	m_titlePos.y = 0;
	m_nTitleAlign = LTF_JUSTIFY_LEFT;

	m_nFolderID = FOLDER_ID_NONE;
	m_nContinueID = FOLDER_ID_NONE;

	m_dwCurrHelpID = 0;

	m_nSelection = kNoSelection;
	m_nGroup = kNoSelection;
    m_pCaptureCtrl = LTNULL;
	m_nFirstDrawn = 0;
	m_nLastDrawn = -1;
	m_nRMouseDownItemSel =  kNoSelection;
	m_nRMouseDownItemSel =  kNoSelection;

	m_nItemSpacing = 0;
    m_bScrollWrap = LTTRUE;


	m_hSelectedColor		= kWhite;
	m_hNormalColor		= kBlack;
	m_hDisabledColor		= kGray;
	m_nAlignment			= LTF_JUSTIFY_LEFT;

    m_pUpArrow = LTNULL;
    m_pDownArrow = LTNULL;
    m_pBack = LTNULL;
    m_pContinue = LTNULL;
    m_pMain = LTNULL;

	m_nNumAttachments = 0;

	m_nNumTabs = 0;
	m_nNextLink = 0;
	m_nCurTab  = -1;

	m_hHighlightColor		= kWhite;

	memset(m_TabPos,0,sizeof(m_TabPos));
	memset(m_LinkPos,0,sizeof(m_LinkPos));

	m_nLogoScaleFX = LOGO_SCALEFX_NONE;
	m_bUseLogo = LTFALSE;

	m_szPassword[0] = LTNULL;

	m_pPWTitle = LTNULL;
	m_pPWPassword = LTNULL;
	m_pPWDlgGroup = LTNULL;
}

CBaseFolder::~CBaseFolder()
{
	if ( m_bInit )
	{
		Term();
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseFolder::Init
//
//	PURPOSE:	initialize the folder
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseFolder::Init(int nFolderID)
{
	if (!m_bReadLayout)
	{
        m_bReadLayout   = LTTRUE;
		m_BackPos		= g_pLayoutMgr->GetBackPos();
		m_ContinuePos	= g_pLayoutMgr->GetContinuePos();
		m_MainPos		= g_pLayoutMgr->GetMainPos();
		g_pLayoutMgr->GetArrowBackBmp(m_sArrowBack,128);
		g_pLayoutMgr->GetArrowNextBmp(m_sArrowNext,128);
		m_ArrowBackPos	= g_pLayoutMgr->GetArrowBackPos();
		m_ArrowNextPos	= g_pLayoutMgr->GetArrowNextPos();

		g_pLayoutMgr->GetChangeSound(m_sDefaultChangeSound,sizeof(m_sDefaultChangeSound));
		g_pLayoutMgr->GetSelectSound(m_sDefaultSelectSound,sizeof(m_sDefaultSelectSound));

/*		
#ifdef _DEMO
	    hDemoBuildVersion=g_pLTClient->FormatString(IDS_DEMOVERSION);
		DemoBuildPos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_MAIN,"VersionPos");
#endif
*/
	}

	int nNumTabs = g_pLayoutMgr->GetNumTabs();
	for (int tab = 0; tab < nNumTabs; tab++)
	{
		m_TabPos[tab] = g_pLayoutMgr->GetTabPos((eFolderID)nFolderID,tab);
	}

	int nNumLinks = g_pLayoutMgr->GetNumLinks();
	for (int Link = 0; Link < nNumLinks; Link++)
	{
		m_LinkPos[Link] = g_pLayoutMgr->GetLinkPos((eFolderID)nFolderID,Link);
	}

	m_UpArrowPos	= offscreen;
	m_DownArrowPos	= offscreen;

	m_nFolderID=nFolderID;
	m_pFolderMgr = g_pInterfaceMgr->GetFolderMgr();

	SetTitleColor(kWhite);

	m_HelpRect = g_pLayoutMgr->GetFolderHelpRect((eFolderID)nFolderID);


	//set up layout variables
    LTIntPt tpos = g_pLayoutMgr->GetFolderTitlePos((eFolderID)nFolderID);
	SetTitlePos(tpos.x,tpos.y);

	SetTitleAlignment(g_pLayoutMgr->GetFolderTitleAlign((eFolderID)nFolderID));

	m_PageRect  = g_pLayoutMgr->GetFolderPageRect((eFolderID)nFolderID);

	SetItemSpacing(g_pLayoutMgr->GetFolderItemSpacing((eFolderID)nFolderID));

	m_nAlignment = g_pLayoutMgr->GetFolderItemAlign((eFolderID)nFolderID);

	m_hNormalColor = g_pLayoutMgr->GetFolderNormalColor((eFolderID)nFolderID);
	m_hSelectedColor = g_pLayoutMgr->GetFolderSelectedColor((eFolderID)nFolderID);
	m_hDisabledColor = g_pLayoutMgr->GetFolderDisabledColor((eFolderID)nFolderID);
	m_hHighlightColor = g_pLayoutMgr->GetFolderHighlightColor((eFolderID)nFolderID);

	g_pLayoutMgr->GetFolderAmbient((eFolderID)nFolderID,m_sAmbientSound,sizeof(m_sAmbientSound));

	if (!m_hTabSurf)
	{
		m_hTabSurf = g_pLTClient->CreateSurface(2,2);
        LTRect rect(0,0,2,2);
		g_pLTClient->FillRect(m_hTabSurf,&rect,SETRGB(10,10,10));
		g_pLTClient->SetSurfaceAlpha(m_hTabSurf,0.75f);
		g_pLTClient->OptimizeSurface(m_hTabSurf,kTransBlack);
	}

	m_bInit=TRUE;
    return LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseFolder::Term
//
//	PURPOSE:	Terminate the folder
//
// ----------------------------------------------------------------------- //

void CBaseFolder::Term()
{
	RemoveAll();

	if (!m_bBuilt)
	{
		if (m_pUpArrow)
		{
			delete m_pUpArrow;
            m_pUpArrow = LTNULL;
		}
		if (m_pDownArrow)
		{
			delete m_pDownArrow;
            m_pDownArrow = LTNULL;
		}
		if (m_pBack)
		{
			delete m_pBack;
            m_pBack = LTNULL;
		}
	}

	// Free the title string
	if (m_hTitleString)
	{
        g_pLTClient->FreeString(m_hTitleString);
        m_hTitleString=LTNULL;
	}

	if (m_hHelpSurf)
	{
        g_pLTClient->DeleteSurface (m_hHelpSurf);
        m_hHelpSurf = LTNULL;
	}

	m_bInit=FALSE;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseFolder::SetLogo
//
//	PURPOSE:	Set the logo scale FX to use
//
// ----------------------------------------------------------------------- //

void CBaseFolder::SetLogo(int nLogo)
{
	m_nLogoScaleFX = nLogo;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseFolder::Escape
//
//	PURPOSE:	back out of the folder
//
// ----------------------------------------------------------------------- //

void CBaseFolder::Escape()
{
	if(GetCapture() == m_pPWPassword)
	{
		HidePasswordDlg();
		return;
	}

	if(m_pFolderMgr->PreviousFolder())
	{
		g_pInterfaceMgr->PlayInterfaceSound(IS_ESCAPE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseFolder::Render
//
//	PURPOSE:	Renders the folder to a surface
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseFolder::Render(HSURFACE hDestSurf)
{
	if (!hDestSurf)
	{
        return LTFALSE;
	}

	int xo = g_pInterfaceResMgr->GetXOffset();
	int yo = g_pInterfaceResMgr->GetYOffset();


	// Render the title
	CLTGUIFont *pTitleFont=GetTitleFont();

	if (pTitleFont && m_hTitleString)
	{
		int xPos=m_titlePos.x + xo;
		int yPos=m_titlePos.y + yo;

		// Align the text as needed
		switch (m_nTitleAlign)
		{
		case LTF_JUSTIFY_CENTER:
			{
                LTIntPt size=pTitleFont->GetTextExtents(m_hTitleString);
				xPos-=size.x/2;
			} break;
		case LTF_JUSTIFY_RIGHT:
			{
                LTIntPt size=pTitleFont->GetTextExtents(m_hTitleString);
				xPos-=size.x;
			} break;
		}
//		pTitleFont->Draw(m_hTitleString, hDestSurf, xPos+2, yPos+2, LTF_JUSTIFY_LEFT,kBlack);
		pTitleFont->Draw(m_hTitleString, hDestSurf, xPos, yPos, LTF_JUSTIFY_LEFT,m_hTitleColor);
	}


	if (m_pBack && m_pBack->IsSelected())
	{
		g_pLTClient->DrawSurfaceToSurfaceTransparent(hDestSurf,g_pInterfaceResMgr->GetSharedSurface(m_sArrowBack), LTNULL, m_ArrowBackPos.x + xo, m_ArrowBackPos.y + yo, m_hTransparentColor);
	}
	if (m_pContinue && m_pContinue->IsSelected())
	{
		g_pLTClient->DrawSurfaceToSurfaceTransparent(hDestSurf,g_pInterfaceResMgr->GetSharedSurface(m_sArrowNext), LTNULL, m_ArrowNextPos.x + xo, m_ArrowNextPos.y + yo, m_hTransparentColor);
	}

	if (m_nCurTab >= 0)
	{
		LTIntPt base = m_TabPos[m_nCurTab]; 
		LTRect rcDest(xo+base.x,yo+base.y,xo+GetPageLeft()-2,yo+base.y+kTabHeight);
		g_pLTClient->ScaleSurfaceToSurface(	hDestSurf, m_hTabSurf, &rcDest, LTNULL);

	}

//render list of ctrls
	int x= GetPageLeft();
	int y= GetPageTop();
	unsigned int i;
	for ( i = 0; i < m_fixedControlArray.size(); i++ )
	{
        LTIntPt oldPos = m_fixedControlArray[i]->GetPos();
		m_fixedControlArray[i]->SetPos(oldPos.x + xo, oldPos.y + yo);
		m_fixedControlArray[i]->Render ( hDestSurf );
		m_fixedControlArray[i]->SetPos(oldPos);
	}

    LTIntPt  size;
	for ( i = m_nFirstDrawn; i < m_controlArray.size(); ++i )
	{
		if (m_controlArray[i]->GetID() == FOLDER_CMD_BREAK)
		{
			++i;
			break;
		}
		size.x=m_controlArray[i]->GetWidth();
		size.y=m_controlArray[i]->GetHeight();

		if ( y+size.y <= GetPageBottom() )
		{

			// Set the position for the control
			m_controlArray[i]->SetPos(x+ xo, y+ yo);
			y+=size.y+m_nItemSpacing;
		}
		else
		{
			break;
		}
	}
	m_nLastDrawn = i-1;

	if (m_nFirstDrawn <= m_nLastDrawn)
	{

		if (m_nCurTab >= 0)
		{
			LTIntPt base = m_TabPos[m_nCurTab]; 
			LTRect rcDest(xo+GetPageLeft()-2,yo+GetPageTop()-2,xo+GetPageRight(),Max(yo+y+2,yo+base.y+kTabHeight));
			g_pLTClient->ScaleSurfaceToSurface(	hDestSurf, m_hTabSurf, &rcDest, LTNULL);
		}

		
		for ( i = m_nFirstDrawn; i <= (unsigned int)m_nLastDrawn; ++i )
		{

			m_controlArray[i]->Render ( hDestSurf );
			LTIntPt pos = m_controlArray[i]->GetPos();
			m_controlArray[i]->SetPos(pos.x-xo, pos.y-yo);
		}
	}


	if (m_hHelpSurf && m_dwCurrHelpID)
	{
//		g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_MASK);
        g_pLTClient->DrawSurfaceToSurfaceTransparent(hDestSurf, m_hHelpSurf, LTNULL, m_HelpRect.left+xo,m_HelpRect.top+yo,kTransBlack);
//		g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ALPHA);
	}
/*
#ifdef _DEMO

	if (hDemoBuildVersion)
	{
		CLTGUIFont* pFont = g_pInterfaceResMgr->GetHelpFont();
		pFont->Draw(hDemoBuildVersion, hDestSurf, DemoBuildPos.x+g_pInterfaceResMgr->GetXOffset(), 
			DemoBuildPos.y+g_pInterfaceResMgr->GetYOffset(), LTF_JUSTIFY_RIGHT, kBlack);
	}
#endif
*/
    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseFolder::CreateTitle
//
//	PURPOSE:	Creates the string to display as the folders title
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseFolder::CreateTitle(HSTRING hString)
{
	if (!hString)
        return LTFALSE;

	if (m_hTitleString)
	{
        g_pLTClient->FreeString(m_hTitleString);
        m_hTitleString=LTNULL;
	}

    m_hTitleString=g_pLTClient->CopyString(hString);


    return LTTRUE;
}

LTBOOL CBaseFolder::CreateTitle(char *lpszTitle)
{
    HSTRING hString=g_pLTClient->CreateString(lpszTitle);

    LTBOOL created = CreateTitle(hString);

    g_pLTClient->FreeString(hString);
	return created;
}

LTBOOL CBaseFolder::CreateTitle(int nStringID)
{
    HSTRING hString=g_pLTClient->FormatString(nStringID);

    LTBOOL created = CreateTitle(hString);

    g_pLTClient->FreeString(hString);
	return created;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseFolder::Build
//
//	PURPOSE:	Construct the basic folder elements
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseFolder::Build()
{
	m_UpArrowPos		= g_pLayoutMgr->GetUpArrowPos((eFolderID)m_nFolderID);
	m_DownArrowPos		= g_pLayoutMgr->GetDownArrowPos((eFolderID)m_nFolderID);

	m_bBuilt=LTTRUE;

	UseArrows(LTFALSE);
	UseBack(LTTRUE);
	UseMain(LTFALSE);

	BuildPasswordDlg();

	return TRUE;
}

// Handles a user entered character
LTBOOL CBaseFolder::HandleChar(char c)
{
    LTBOOL handled = LTFALSE;

	if (m_pCaptureCtrl)
	{
		if (m_pCaptureCtrl->HandleChar(c))
            handled = LTTRUE;
	}
	return handled;
}


// Handles a key press.  Returns FALSE if the key was not processed through this method.
// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
LTBOOL CBaseFolder::HandleKeyDown(int key, int rep)
{
    LTBOOL handled = LTFALSE;

	if (m_pCaptureCtrl)
	{
		if (key == VK_RETURN)
			handled = m_pCaptureCtrl->OnEnter();
		else if (m_pCaptureCtrl->HandleKeyDown(key,rep))
            handled = LTTRUE;
		return handled;
	}


	switch (key)
	{
	case VK_LEFT:
		{
			handled = OnLeft();
			break;
		}
	case VK_RIGHT:
		{
			handled = OnRight();
			break;
		}
	case VK_UP:
		{
			handled = OnUp();
			break;
		}
	case VK_DOWN:
		{
			handled = OnDown();
			break;
		}
	case VK_TAB:
		{
			handled = OnTab();
			break;
		}
	case VK_RETURN:
		{
			handled = OnEnter();
			break;
		}
	case VK_PRIOR:
		{
			handled = OnPageUp();
			break;
		}
	case VK_NEXT:
		{
			handled = OnPageDown();
			break;
		}
	default:
		{
            handled = LTFALSE;
			break;
		}
	}

	// Handled the key
	return handled;
}


/******************************************************************/


/******************************************************************/

LTBOOL CBaseFolder::OnUp()
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnUp())
		return LTTRUE;

	int sel = m_nSelection;
	return (sel != PreviousSelection());
}

/******************************************************************/

LTBOOL CBaseFolder::OnDown()
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnDown())
		return LTTRUE;

	int sel = m_nSelection;
	return (sel != NextSelection());
}

/******************************************************************/

LTBOOL CBaseFolder::OnTab()
{
	int sel = m_nSelection;
	if (IsKeyDown(VK_SHIFT))
		return (sel != PreviousTab());
	else
		return (sel != NextTab());
}

/******************************************************************/

LTBOOL CBaseFolder::OnLeft()
{
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	char *pSnd = GetSelectSound();
	if (pCtrl)
		handled = pCtrl->OnLeft();
	if (handled)
		g_pInterfaceMgr->PlayInterfaceSound(IS_SELECT,pSnd);
	return handled;
}


/******************************************************************/

LTBOOL CBaseFolder::OnRight()
{
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	char *pSnd = GetSelectSound();
	if (pCtrl)
		handled = pCtrl->OnRight();
	if (handled)
		g_pInterfaceMgr->PlayInterfaceSound(IS_SELECT,pSnd);
	return handled;
}


/******************************************************************/

LTBOOL CBaseFolder::OnPageUp()
{
	if (!IsFirstPage())
	{
		PreviousPage();

		if (m_nSelection >= 0)
			SetSelection(m_nFirstDrawn);

        return LTTRUE;
	}
	else
        return LTFALSE;
}

/******************************************************************/

LTBOOL CBaseFolder::OnPageDown()
{
	if (!IsLastPage())
	{
		NextPage();

		if (m_nSelection >= 0)
			SetSelection(m_nFirstDrawn);

        return LTTRUE;
	}
	else
        return LTFALSE;
}


/******************************************************************/

LTBOOL CBaseFolder::OnEnter()
{
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	char *pSnd = GetSelectSound();
	if (pCtrl)
	{
		handled = pCtrl->OnEnter();
		if (handled)
		{
			g_pInterfaceMgr->PlayInterfaceSound(IS_SELECT,pSnd);
		}
	}
	return handled;
}


CLTGUIFont* CBaseFolder::GetSmallFont()
{
    if (!g_pInterfaceResMgr) return LTNULL;
	return g_pInterfaceResMgr->GetSmallFont();
}

CLTGUIFont* CBaseFolder::GetTinyFont()
{
    if (!g_pInterfaceResMgr) return LTNULL;
	return g_pInterfaceResMgr->GetTinyFont();
}

CLTGUIFont* CBaseFolder::GetTinyFixedFont()
{
    if (!g_pInterfaceResMgr) return LTNULL;
	return g_pInterfaceResMgr->GetTinyFixedFont();
}


CLTGUIFont* CBaseFolder::GetHelpFont()
{
    if (!g_pInterfaceResMgr) return LTNULL;
	return g_pInterfaceResMgr->GetHelpFont();
}

CLTGUIFont* CBaseFolder::GetLargeFont( )
{
    if (!g_pInterfaceResMgr) return LTNULL;
	return g_pInterfaceResMgr->GetLargeFont();
}


CLTGUIFont* CBaseFolder::GetSmallFixedFont( )
{
    if (!g_pInterfaceResMgr) return LTNULL;
	return g_pInterfaceResMgr->GetSmallFixedFont();
}

CLTGUIFont* CBaseFolder::GetLargeFixedFont( )
{
    if (!g_pInterfaceResMgr) return LTNULL;
	return g_pInterfaceResMgr->GetLargeFixedFont();
}

CLTGUIFont* CBaseFolder::GetTitleFont( )
{
    if (!g_pInterfaceResMgr) return LTNULL;
	return g_pInterfaceResMgr->GetTitleFont();
}


CLTGUIFont* CBaseFolder::GetDefaultFont(LTBOOL bFixed)
{
	
	CLTGUIFont *pFont = LTNULL;

	if (bFixed)
	{
		pFont = GetLargeFixedFont();
		if (g_pLayoutMgr->GetFolderFontSize((eFolderID)m_nFolderID) == 0)
			pFont = GetSmallFixedFont();
	}
	else
	{
		pFont = GetLargeFont();
		if (g_pLayoutMgr->GetFolderFontSize((eFolderID)m_nFolderID) == 0)
			pFont = GetSmallFont();
	}

	return pFont;
}

int CBaseFolder::NextSelection()
{
	int select = m_nSelection;
	if (select == kNoSelection)
		select = 0;
	int oldSelect = select;
	LTBOOL bFixed = (m_nSelection < 0);

	int nGroup = kNoSelection;
	if (GetSelectedControl())
		nGroup = FindTab(GetSelectedControl());


	
	CLTGUICtrl* pCtrl = LTNULL;	
	do
	{
		if (bFixed)
		{
			select--;
			int fixSel = FixedIndex(select);
			if (fixSel >= (int)m_fixedControlArray.size())
			{
				if (m_controlArray.size())
				{
					select = 0;
					bFixed = LTFALSE;
				}
				else
					select = -1;
			}
	
		}
		else
		{
			select++;
			if (select >= (int)m_controlArray.size())
			{
				if (m_fixedControlArray.size())
				{
					select = -1;
					bFixed = LTTRUE;
				}
				else
					select = 0;
			}
	
		}
	
		pCtrl = GetControl(select);	
		if (nGroup == kNoSelection)
			nGroup = FindTab(pCtrl);



	} while (select != oldSelect && pCtrl && (!pCtrl->IsEnabled() || SkipControl(pCtrl) /*|| nGroup != FindTab(pCtrl)*/  ));


	if (!pCtrl || !pCtrl->IsEnabled() || SkipControl(pCtrl))
		select = m_nSelection;

	return SetSelection(select);

}

int CBaseFolder::PreviousSelection()
{
	int select = m_nSelection;
	if (select == kNoSelection)
		select = 0;
	int oldSelect = select;
	LTBOOL bFixed = (m_nSelection < 0);
	int nGroup = kNoSelection;
	if (GetSelectedControl())
		nGroup = FindTab(GetSelectedControl());
	
	CLTGUICtrl* pCtrl = LTNULL;	
	do
	{
		if (bFixed)
		{
			select++;
			if (select == 0)
			{
				if (m_controlArray.size())
				{
					select = m_controlArray.size()-1;
					bFixed = LTFALSE;
				}
				else
					select = -1 * (int)m_fixedControlArray.size();
			}
	
		}
		else
		{
			select--;
			if (select < 0)
			{
				if (m_fixedControlArray.size())
				{
					select = -1 * (int)m_fixedControlArray.size();
					bFixed = LTTRUE;
				}
				else
					select = m_controlArray.size()-1;

			}
	
		}
		pCtrl = GetControl(select);	
		if (nGroup == kNoSelection)
			nGroup = FindTab(pCtrl);

	} while (select != oldSelect && pCtrl && (!pCtrl->IsEnabled() || SkipControl(pCtrl) /*|| nGroup != FindTab(pCtrl) */ ));


	if (!pCtrl || !pCtrl->IsEnabled() || SkipControl(pCtrl))
		select = m_nSelection;

	return SetSelection(select);

}

int CBaseFolder::NextTab()
{
	int nGroup = 0;
	if (GetSelectedControl())
		nGroup = FindTab(GetSelectedControl());
	int nTest = (nGroup+1) % MAX_TAB_GROUPS;

	CLTGUICtrl *pCtrl = LTNULL;
	LTBOOL bFound = LTFALSE;
	//look at the each group starting with the next
	while((nTest >= 0) && (nTest != nGroup))
	{
		uint32 nIndex = 0;
		//look at each control in the group
		while (!bFound && nIndex < m_tabArray[nTest].size())
		{
			//if the we've found an enabled control we're done
			pCtrl = m_tabArray[nTest].at(nIndex);
			if (pCtrl && pCtrl->IsEnabled())
				bFound = LTTRUE;
			else
				nIndex++;
		}

		//if we found a control stop looking, otherwise check the next group
		if (bFound)
			break;
		else
			nTest = (nTest+1) % MAX_TAB_GROUPS;
	}

	int nIndex = m_nSelection;
	if (nGroup != nTest)
	{
		nIndex = GetIndex(pCtrl);
		SetSelection(nIndex);
	}
	
	return nIndex;	
};

int CBaseFolder::PreviousTab()
{
	int nGroup = 0;
	if (GetSelectedControl())
		nGroup = FindTab(GetSelectedControl());
	int nTest = (nGroup+MAX_TAB_GROUPS-1) % MAX_TAB_GROUPS;

	CLTGUICtrl *pCtrl = LTNULL;
	LTBOOL bFound = LTFALSE;
	//look at the each group starting with the next
	while (nTest != nGroup)
	{
		uint32 nIndex = 0;
		//look at each control in the group
		while (!bFound && nIndex < m_tabArray[nTest].size())
		{
			//if the we've found an enabled control we're done
			pCtrl = m_tabArray[nTest].at(nIndex);
			if (pCtrl && pCtrl->IsEnabled())
				bFound = LTTRUE;
			else
				nIndex++;
		}

		//if we found a control stop looking, otherwise check the next group
		if (bFound)
			break;
		else
			nTest = (nTest+MAX_TAB_GROUPS-1) % MAX_TAB_GROUPS;
	}

	int nIndex = m_nSelection;
	if (nGroup != nTest)
	{
		nIndex = GetIndex(pCtrl);
		SetSelection(nIndex);
	}
	
	return nIndex;	
}


/******************************************************************/
LTBOOL CBaseFolder::OnLButtonDown(int x, int y)
{
	// Get the control that the click was on
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
            return LTFALSE;

		// Select the control
		SetSelection(nControlIndex);

		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.
		m_nLMouseDownItemSel=nControlIndex;
		return pCtrl->OnLButtonDown(x,y);

	}
	else
		m_nLMouseDownItemSel=kNoSelection;

    return LTFALSE;
}

/******************************************************************/
LTBOOL CBaseFolder::OnLButtonUp(int x, int y)
{
	// Get the control that the click was on
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
            return LTFALSE;
		// If the mouse is over the same control now as it was when the down message was called
		// then send the "enter" message to the control.
		if (nControlIndex == m_nLMouseDownItemSel)
		{
			if (pCtrl->IsEnabled() )
			{
				SetSelection(nControlIndex);
				char *pSnd = GetSelectSound();
				LTBOOL bHandled = pCtrl->OnLButtonUp(x,y);
				if (bHandled)
				{
					g_pInterfaceMgr->PlayInterfaceSound(IS_SELECT,pSnd);
				}
				return bHandled;

			}
		}
	}
	else
	{
		m_nLMouseDownItemSel= kNoSelection;
	}
    return LTFALSE;
}



/******************************************************************/
LTBOOL CBaseFolder::OnRButtonDown(int x, int y)
{
	// Get the control that the click was on
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
            return LTFALSE;

		// Select the control
		SetSelection(nControlIndex);

		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.
		m_nRMouseDownItemSel=nControlIndex;

		return pCtrl->OnRButtonDown(x,y);
	}
	else
		m_nRMouseDownItemSel=kNoSelection;

    return LTFALSE;
}

/******************************************************************/
LTBOOL CBaseFolder::OnRButtonUp(int x, int y)
{
	// Get the control that the click was on
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
            return LTFALSE;

		// If the mouse is over the same control now as it was when the down message was called
		// then send the "left" message to the control.
		if (nControlIndex == m_nRMouseDownItemSel)
		{
			if (pCtrl->IsEnabled())
			{
				SetSelection(nControlIndex);
				char *pSnd = GetSelectSound();
				LTBOOL bHandled = pCtrl->OnRButtonUp(x,y);
				if (bHandled)
					g_pInterfaceMgr->PlayInterfaceSound(IS_SELECT,pSnd);
				return bHandled;
			}
		}
	}
	else
	{
		m_nRMouseDownItemSel= kNoSelection;
	}
    return LTFALSE;
}


/******************************************************************/
LTBOOL CBaseFolder::OnLButtonDblClick(int x, int y)
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
        return LTFALSE;

	if (pCtrl)
		return pCtrl->OnLButtonDblClick(x, y);
	else
        return LTFALSE;
}


/******************************************************************/
LTBOOL CBaseFolder::OnRButtonDblClick(int x, int y)
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
        return LTFALSE;

	if (pCtrl)
		return pCtrl->OnRButtonDblClick(x, y);
	else
        return LTFALSE;
}

HSTRING CBaseFolder::GetHelpString(uint32 dwHelpId, int nControlIndex)
{
    return g_pLTClient->FormatString(dwHelpId);
}

/******************************************************************/
LTBOOL CBaseFolder::OnMouseMove(int x, int y)
{
	int nControlUnderPoint=kNoSelection;
    LTBOOL onCtrl = GetControlUnderPoint(x,y,&nControlUnderPoint);
	if (onCtrl)
	{
		CLTGUICtrl* pCtrl = GetControl(nControlUnderPoint);
		if (m_pCaptureCtrl && m_pCaptureCtrl != pCtrl)
            return LTFALSE;

		pCtrl->OnMouseMove(x,y);
	}
	else if (m_pCaptureCtrl)
        return LTFALSE;

	if (onCtrl)
	{
		if (GetSelection() != nControlUnderPoint)
		{

			if (GetControl(nControlUnderPoint)->IsEnabled())
			{
				SetSelection(nControlUnderPoint);
			}
			else
			{
//				SetSelection(kNoSelection);
			}
		}
        return LTTRUE;
	}
//	SetSelection(kNoSelection);
    return LTFALSE;
}

LTBOOL CBaseFolder::OnMouseWheel(int x, int y, int delta)
{
	return LTFALSE;
}


int CBaseFolder::SetSelection(int select)
{
	if (select == m_nSelection) return select;

	int nOldSelect=m_nSelection;

	if (select == kNoSelection)
	{
		if (nOldSelect != kNoSelection)
			GetControl(nOldSelect)->Select(FALSE);
		m_nSelection = kNoSelection;
		m_nGroup = kNoSelection;
		UpdateHelpText();
		return kNoSelection;
	}



	int fixedSel = FixedIndex(select);
	CLTGUICtrl *pSelCtrl;


	if (select >= 0)
	{
		if ((uint32)select >= m_controlArray.size())
			select = m_controlArray.size()-1;
	}
	else
	{
		if (fixedSel >= (int)m_fixedControlArray.size())
			select = FixedIndex((int)m_fixedControlArray.size()-1);
	}


	pSelCtrl = GetControl(select);
	if (!pSelCtrl || !pSelCtrl->IsEnabled())
	{
		UpdateHelpText();
		return nOldSelect;
	}


	if (nOldSelect != kNoSelection)
		GetControl(nOldSelect)->Select(LTFALSE);


	m_nSelection=select;

	if (m_nSelection == kNoSelection)
	{
		m_nGroup = kNoSelection;
		UpdateHelpText();
		return nOldSelect;
	}

	pSelCtrl->Select(TRUE);
	m_nGroup = FindTab(pSelCtrl);


	if (m_nSelection != nOldSelect && m_bHasFocus)
		g_pInterfaceMgr->PlayInterfaceSound(IS_CHANGE);


	if (select >= 0)
	{
		// Figure out if we should change the page
		while ( m_nSelection < m_nFirstDrawn )
		{
            PreviousPage(LTFALSE);
		}
		if (m_nLastDrawn < m_nFirstDrawn)
			CalculateLastDrawn();

		while (m_nLastDrawn < m_nSelection )
		{
            NextPage(LTFALSE);
			if (m_nLastDrawn < 0)
			{
				//no items are drawn
				_ASSERT(0);
				break;
			}
		}
	}

	CheckArrows();

	UpdateHelpText();
	return m_nSelection;
}

int CBaseFolder::FindTab(CLTGUICtrl *pCtrl)
{
	int nGroup = 0;
	LTBOOL bFound = LTFALSE;
	while (nGroup < MAX_TAB_GROUPS && !bFound)
	{
		ControlArray::iterator iter = std::find(m_tabArray[nGroup].begin(), m_tabArray[nGroup].end(), pCtrl);
		bFound = (iter != m_tabArray[nGroup].end());
		if (!bFound)
			nGroup++;
	}

	if (bFound)
		return nGroup;
	else
		return kNoSelection;
}

void CBaseFolder::CalculateLastDrawn()
{
    LTIntPt  size;
	int x= GetPageLeft();
	int y= GetPageTop();
	unsigned int i;
	for ( i = m_nFirstDrawn; i < m_controlArray.size(); i++ )
	{
		if (m_controlArray[i]->GetID() == FOLDER_CMD_BREAK)
		{
			i++;
			break;
		}

		size.x=m_controlArray[i]->GetWidth();
		size.y=m_controlArray[i]->GetHeight();

		if ( y+size.y <= GetPageBottom() )
		{

			// Set the position for the control
			m_controlArray[i]->SetPos(x, y);
			y+=size.y+m_nItemSpacing;
		}
		else
		{
			break;
		}
	}
	m_nLastDrawn = i-1;



}

// Gets the index of the control that is under the specific screen point.
// Returns FALSE if there isn't one under the specified point.
LTBOOL CBaseFolder::GetControlUnderPoint(int xPos, int yPos, int *pnIndex)
{
	_ASSERT(pnIndex);

	// if we have free controls check them
	if (m_nFirstDrawn >= 0 || (uint32)m_nFirstDrawn < m_controlArray.size())
	{
		// See if the user clicked on any of the controls.
		int i;
		for (i=m_nLastDrawn; i >= m_nFirstDrawn; i--)
		{
			int nLeft=m_controlArray[i]->GetPos().x;
			int nTop=m_controlArray[i]->GetPos().y;
			int nRight=nLeft+m_controlArray[i]->GetWidth();
			int nBottom=nTop+m_controlArray[i]->GetHeight();

			// Check to see if the click is in the bounding box for the control
			if (m_controlArray[i]->IsEnabled() && xPos >= nLeft && xPos <= nRight && yPos >= nTop && yPos <= nBottom)
			{
				*pnIndex=i;

				return LTTRUE;
			}
		}
	}

	//then check fixed controls
	for (int i=(int)m_fixedControlArray.size()-1 ; i >= 0 ; i--)
	{
		int nLeft=m_fixedControlArray[i]->GetPos().x;
		int nTop=m_fixedControlArray[i]->GetPos().y;
		int nRight=nLeft+m_fixedControlArray[i]->GetWidth();
		int nBottom=nTop+m_fixedControlArray[i]->GetHeight();

		// Check to see if the click is in the bounding box for the control
		if (m_fixedControlArray[i]->IsEnabled() && xPos >= nLeft && xPos <= nRight && yPos >= nTop && yPos <= nBottom)
		{
			*pnIndex=-1-i;

            return LTTRUE;
		}
	}

    return LTFALSE;
}

// Return a control at a specific index
CLTGUICtrl *CBaseFolder::GetControl ( int nIndex )
{
	if (nIndex == kNoSelection) return LTNULL;
	if (nIndex >= 0 && (uint32)nIndex < m_controlArray.size() )
		return m_controlArray[nIndex];
	if (nIndex < 0)
	{
		int nFix = FixedIndex(nIndex);
		if (nFix < (int)m_fixedControlArray.size() )
			return m_fixedControlArray[nFix];
	}
    return LTNULL;
}





void CBaseFolder::RemoveAll()
{
	// Terminate the ctrls
	RemoveFree();
	RemoveFixed();
}
void CBaseFolder::RemoveFree()
{
	// Terminate the ctrls
	unsigned int i;
	for (i=0; i < m_controlArray.size(); i++)
	{
		ControlArray::iterator iter = std::find(m_tabArray[0].begin(), m_tabArray[0].end(), m_controlArray[i]);
		if (iter != m_tabArray[0].end())
		{
			m_tabArray[0].erase(iter);
		}
		m_controlArray[i]->Destroy();
		delete m_controlArray[i];
	}
	m_controlArray.clear();
	if (m_nSelection >= 0)
		m_nSelection = kNoSelection;
	m_nFirstDrawn = 0;
	m_nLastDrawn = -1;

}
void CBaseFolder::RemoveFixed()
{
	// Terminate the ctrls
	unsigned int i;
	for (i=0; i < m_fixedControlArray.size(); i++)
	{
		LTBOOL bFound = LTFALSE;
		int nGroup = 0;
		while (!bFound && nGroup < MAX_TAB_GROUPS)
		{
			ControlArray::iterator iter = std::find(m_tabArray[nGroup].begin(), m_tabArray[nGroup].end(), m_fixedControlArray[i]);
			if (iter == m_tabArray[nGroup].end())
			{
				nGroup++;
			}
			else
			{
				m_tabArray[nGroup].erase(iter);
				bFound = LTTRUE;
			}
		}

		m_fixedControlArray[i]->Destroy();
		delete m_fixedControlArray[i];
	}
	m_skipControlArray.clear();
	m_fixedControlArray.clear();
	if (m_nSelection < 0)
		m_nSelection = kNoSelection;

}

LTBOOL CBaseFolder::NextPage(LTBOOL bChangeSelection)
{
	int oldFirst = m_nFirstDrawn;

	if(!IsLastPage())
	{
		int nCanDraw = m_nLastDrawn - m_nFirstDrawn;
		m_nFirstDrawn += nCanDraw;

		if((m_nFirstDrawn + nCanDraw) > (int)m_controlArray.size())
		{
			m_nFirstDrawn -= (m_nFirstDrawn + nCanDraw) - m_controlArray.size();
			m_nFirstDrawn -= 1;
		}

		CalculateLastDrawn();
		CheckArrows();

		if (bChangeSelection)
		{
            LTIntPt cPos = g_pInterfaceMgr->GetCursorPos();
			OnMouseMove(cPos.x,cPos.y);
		}
	}

	return (oldFirst != m_nFirstDrawn);
}

LTBOOL CBaseFolder::PreviousPage(LTBOOL bChangeSelection)
{
	int oldFirst = m_nFirstDrawn;

	if(!IsFirstPage())
	{
		int nCanDraw = m_nLastDrawn - m_nFirstDrawn;
		m_nFirstDrawn -= nCanDraw;

		if(m_nFirstDrawn < 0)
		{
			m_nFirstDrawn = 0;
		}

		CalculateLastDrawn();
		CheckArrows();

		if (bChangeSelection)
		{
            LTIntPt cPos = g_pInterfaceMgr->GetCursorPos();
			OnMouseMove(cPos.x,cPos.y);
		}
	}

	return (oldFirst != m_nFirstDrawn);
}

int CBaseFolder::AddFixedControl(CLTGUICtrl* pCtrl, eNavGroups eTabGroup, LTIntPt pos, LTBOOL bSelectable)
{
	if (!pCtrl) return 0;

	// Make sure the control wasn't already added...

	for (int i = (int)m_fixedControlArray.size()-1; i >=0; i--)
	{
		if (m_fixedControlArray[i] == pCtrl)
		{
			return 0;
		}
	}

	pCtrl->SetPos(pos);
	m_fixedControlArray.push_back(pCtrl);
	if (bSelectable && eTabGroup != kNoGroup)
	{
		m_tabArray[eTabGroup].push_back(pCtrl);
	}
	else
		m_skipControlArray.push_back(pCtrl);
	int num = m_fixedControlArray.size();


	return FixedIndex(num-1);

}

int	CBaseFolder::AddFreeControl(CLTGUICtrl* pCtrl)
{
	m_controlArray.push_back(pCtrl);
	m_tabArray[kFreeGroup].push_back(pCtrl);
    
	int num = m_controlArray.size();
	if (num == m_nSelection+1)
        pCtrl->Select(LTTRUE);


	return num-1;

}


// Calls UpdateData on each control in the menu
void CBaseFolder::UpdateData(LTBOOL bSaveAndValidate)
{
	unsigned int i;
	for (i=0; i < m_fixedControlArray.size(); i++)
	{
		m_fixedControlArray[i]->UpdateData(bSaveAndValidate);
	}
	for (i=0; i < m_controlArray.size(); i++)
	{
		m_controlArray[i]->UpdateData(bSaveAndValidate);
	}
}


CLTGUITextItemCtrl* CBaseFolder::CreateTextItem(HSTRING hString, uint32 commandID, int helpID, LTBOOL bFixed, CLTGUIFont *pFont, int *pnValue)
{
	CLTGUITextItemCtrl* pCtrl=new CLTGUITextItemCtrl;

    if (pFont == LTNULL)
	{
		pFont = GetDefaultFont(bFixed);
	}

    if (!pCtrl->Create(g_pLTClient, commandID, hString, pFont, this, pnValue))
	{
		delete pCtrl;
        return LTNULL;
	}

	SetFontColors(pCtrl, (bFixed != 0));

	pCtrl->SetHelpID(helpID);
	pCtrl->SetTransparentColor(m_hTransparentColor);
	pCtrl->Enable(!bFixed);

	return pCtrl;
}

CLTGUITextItemCtrl* CBaseFolder::CreateTextItem(int stringID, uint32 commandID, int helpID, LTBOOL bFixed, CLTGUIFont *pFont, int *pnValue)
{
    HSTRING hStr=g_pLTClient->FormatString(stringID);
	CLTGUITextItemCtrl* pCtrl=CreateTextItem(hStr, commandID, helpID, bFixed, pFont, pnValue);
    g_pLTClient->FreeString(hStr);

	return pCtrl;

}

CLTGUITextItemCtrl* CBaseFolder::CreateTextItem(const char *pString, uint32 commandID, int helpID, LTBOOL bFixed, CLTGUIFont *pFont, int *pnValue)
{
    HSTRING hStr=g_pLTClient->CreateString(const_cast<char*>(pString));
	CLTGUITextItemCtrl* pCtrl=CreateTextItem(hStr, commandID, helpID, bFixed, pFont, pnValue);
    g_pLTClient->FreeString(hStr);

	return pCtrl;

}

CStaticTextCtrl* CBaseFolder::CreateStaticTextItem(HSTRING hString, uint32 commandID, int helpID, int width, int height, LTBOOL bFixed, CLTGUIFont *pFont)
{
	CStaticTextCtrl* pCtrl=new CStaticTextCtrl;

    if (pFont == LTNULL)
	{
		pFont = GetDefaultFont(bFixed);
	}

    if (!pCtrl->Create(g_pLTClient,commandID,hString,pFont,this,width,height))
	{
		delete pCtrl;
        return LTNULL;
	}


	SetFontColors(pCtrl, (bFixed != 0));

	pCtrl->SetHelpID(helpID);
	pCtrl->SetTransparentColor(SETRGB_T(0,0,0));

	return pCtrl;
}

CStaticTextCtrl* CBaseFolder::CreateStaticTextItem(int stringID, uint32 commandID, int helpID, int width, int height, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->FormatString(stringID);
	CStaticTextCtrl* pCtrl=CreateStaticTextItem(hStr, commandID, helpID, width, height, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;

}

CStaticTextCtrl* CBaseFolder::CreateStaticTextItem(char *pString, uint32 commandID, int helpID, int width, int height, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->CreateString(pString);
	CStaticTextCtrl* pCtrl=CreateStaticTextItem(hStr, commandID, helpID, width, height, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;

}




CLTGUIEditCtrl* CBaseFolder::CreateEditCtrl(HSTRING hDescription, uint32 commandID, int helpID, char *pBuffer, int nBufferSize,
									int	nTextOffset, LTBOOL bFixed, CLTGUIFont *pFont)
{

	CLTGUIEditCtrl* pCtrl=new CLTGUIEditCtrl;

    if (pFont == LTNULL)
	{
		pFont = GetDefaultFont(bFixed);
	}

    if (!pCtrl->Create(g_pLTClient, commandID, hDescription, pFont, nTextOffset, nBufferSize, this, pBuffer))
	{
		delete pCtrl;
        return LTNULL;
	}

	SetFontColors(pCtrl, (bFixed != 0));

	pCtrl->SetHelpID(helpID);
	pCtrl->SetTransparentColor(m_hTransparentColor);

	return pCtrl;
}

CLTGUIEditCtrl* CBaseFolder::CreateEditCtrl(int nDescriptionID, uint32 commandID, int helpID, char *pBuffer, int nBufferSize,
									int	nTextOffset, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->FormatString(nDescriptionID);
	CLTGUIEditCtrl* pCtrl=CreateEditCtrl(hStr, commandID, helpID, pBuffer, nBufferSize, nTextOffset, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;

}
CLTGUIEditCtrl* CBaseFolder::CreateEditCtrl(char *pszDescription, uint32 commandID, int helpID, char *pBuffer, int nBufferSize,
									int	nTextOffset, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->CreateString(pszDescription);
	CLTGUIEditCtrl* pCtrl=CreateEditCtrl(hStr, commandID, helpID, pBuffer, nBufferSize, nTextOffset, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;

}


// Adds a Cycle control
CCycleCtrl *CBaseFolder::CreateCycleItem(HSTRING hText, int helpID, int nHeaderWidth, int nSpacerWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
	// Create the new menu option
	CCycleCtrl *pCtrl=new CCycleCtrl;

    if (pFont == LTNULL)
	{
		pFont = GetDefaultFont(bFixed);
	}

    if ( !pCtrl->Create(g_pLTClient, hText, pFont, nHeaderWidth, nSpacerWidth ,pnValue, m_nAlignment ) )
	{
		delete pCtrl;
        return LTNULL;
	}

	SetFontColors(pCtrl, (bFixed != 0));

	pCtrl->SetHelpID(helpID);
	pCtrl->SetTransparentColor(m_hTransparentColor);

	return pCtrl;

}

CCycleCtrl *CBaseFolder::CreateCycleItem(int stringID, int helpID, int nHeaderWidth, int nSpacerWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->FormatString(stringID);
	CCycleCtrl* pCtrl=CreateCycleItem(hStr, helpID, nHeaderWidth, nSpacerWidth, pnValue, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;
}

CCycleCtrl *CBaseFolder::CreateCycleItem(char *pString, int helpID, int nHeaderWidth, int nSpacerWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->CreateString(pString);
	CCycleCtrl* pCtrl=CreateCycleItem(hStr, helpID, nHeaderWidth, nSpacerWidth, pnValue, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;
}



// Adds an on/off control
CToggleCtrl *CBaseFolder::CreateToggle(HSTRING hText, int helpID, int nRightColumnOffset, LTBOOL *pbValue, LTBOOL bFixed, CLTGUIFont *pFont)
{

    if (pFont == LTNULL)
	{
		pFont = GetDefaultFont(bFixed);
	}

	// Create the new menu control
	CToggleCtrl *pCtrl=new CToggleCtrl;
    if ( !pCtrl->Create(g_pLTClient, hText, pFont, nRightColumnOffset, pbValue, m_nAlignment ) )
	{
		delete pCtrl;
        return LTNULL;
	}

	SetFontColors(pCtrl, (bFixed != 0));

	pCtrl->SetHelpID(helpID);
	pCtrl->SetTransparentColor(m_hTransparentColor);

	return pCtrl;
}

CToggleCtrl *CBaseFolder::CreateToggle(int stringID, int helpID, int nRightColumnOffset, LTBOOL *pbValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->FormatString(stringID);
	CToggleCtrl* pCtrl=CreateToggle(hStr, helpID, nRightColumnOffset, pbValue, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;
}

CToggleCtrl *CBaseFolder::CreateToggle(char *pString, int helpID, int nRightColumnOffset, LTBOOL *pbValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->CreateString(pString);
	CToggleCtrl* pCtrl=CreateToggle(hStr, helpID, nRightColumnOffset, pbValue, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;
}


CSliderCtrl* CBaseFolder::CreateSlider(HSTRING hText, int helpID, int nSliderOffset, int nSliderWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont)
{

    if (pFont == LTNULL)
	{
		pFont = GetDefaultFont(bFixed);
	}

	CSliderCtrl *pCtrl=new CSliderCtrl;
    if ( !pCtrl->Create(hText, pFont, nSliderOffset, nSliderWidth, LTFALSE, pnValue) )
	{
		delete pCtrl;
        return LTNULL;
	}

	SetFontColors(pCtrl, (bFixed != 0));

	pCtrl->SetHelpID(helpID);
	pCtrl->SetTransparentColor(m_hTransparentColor);

	return pCtrl;
}

CSliderCtrl* CBaseFolder::CreateSlider(int stringID, int helpID, int nSliderOffset, int nSliderWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->FormatString(stringID);
	CSliderCtrl* pCtrl=CreateSlider(hStr, helpID, nSliderOffset, nSliderWidth, pnValue, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;
}

CSliderCtrl* CBaseFolder::CreateSlider(char *pString, int helpID, int nSliderOffset, int nSliderWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->CreateString(pString);
	CSliderCtrl* pCtrl=CreateSlider(hStr, helpID, nSliderOffset, nSliderWidth, pnValue, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;
}

CLTGUIColumnTextCtrl*	CBaseFolder::CreateColumnText(DWORD dwCommandID, int helpID, LTBOOL bFixed, CLTGUIFont *pFont, DWORD dwParam1, DWORD dwParam2)
{

    if (pFont == LTNULL)
	{
		pFont = GetDefaultFont(bFixed);
	}

	CLTGUIColumnTextCtrl *pCtrl=new CLTGUIColumnTextCtrl;
    if ( !pCtrl->Create(g_pLTClient, dwCommandID, pFont, this, dwParam1, dwParam2) )
	{
		delete pCtrl;

        return LTNULL;
	}

	SetFontColors(pCtrl, (bFixed != 0));


	pCtrl->SetHelpID(helpID);
	pCtrl->SetTransparentColor(m_hTransparentColor);
	pCtrl->Enable(!bFixed);


	return pCtrl;
}

CGroupCtrl*	CBaseFolder::CreateGroup(int nWidth, int nHeight, int helpID)
{
	// Create the new menu option
	CGroupCtrl *pCtrl=new CGroupCtrl;
	if ( !pCtrl->Create(nWidth,nHeight))
	{
		delete pCtrl;
        return LTNULL;
	}

	pCtrl->SetHelpID(helpID);

	return pCtrl;
}

void CBaseFolder::AddPageBreak()
{
	CPageBreakCtrl* pCtrl = new CPageBreakCtrl;
	pCtrl->Create(FOLDER_CMD_BREAK);
    pCtrl->Enable(LTFALSE);
	AddFreeControl(pCtrl);
}

void CBaseFolder::AddBlankLine()
{
    CStaticTextCtrl* pCtrl = AddStaticTextItem(" ",LTNULL,LTNULL, 2, GetHelpFont()->GetHeight(), LTTRUE, GetHelpFont());
    pCtrl->Enable(LTFALSE);
}

void CBaseFolder::UseArrows(LTBOOL bArrows, LTBOOL bLeft, LTBOOL bRight)
{
	if (bArrows)
	{
		if (bLeft)
		{
			CreateUpArrow();
		}
		else
		{
			RemoveFixedControl(m_pUpArrow);
			delete m_pUpArrow;
            m_pUpArrow = LTNULL;
		}

		if (bRight)
		{
			CreateDownArrow();
		}
		else
		{
			RemoveFixedControl(m_pDownArrow);
			delete m_pDownArrow;
            m_pDownArrow = LTNULL;
		}

		if (IsBuilt())
		{
            AddFixedControl(m_pUpArrow,kNoGroup,m_UpArrowPos,LTFALSE);
            AddFixedControl(m_pDownArrow,kNoGroup,m_DownArrowPos,LTFALSE);
		}
	}
	else
	{
		if (m_pUpArrow)
		{
			RemoveFixedControl(m_pUpArrow);
			delete m_pUpArrow;
            m_pUpArrow = LTNULL;
		}
		if (m_pDownArrow)
		{
			RemoveFixedControl(m_pDownArrow);
			delete m_pDownArrow;
            m_pDownArrow = LTNULL;
		}
	}
}
void CBaseFolder::UseBack(LTBOOL bBack,LTBOOL bOK,LTBOOL bReturn)
{
	if (bBack)
	{
		CreateBack(bOK,bReturn);

		if (IsBuilt())
		{
            AddFixedControl(m_pBack,kBackGroup,m_BackPos,LTTRUE);
		}
	}
	else
	{
		if (m_pBack)
		{
			RemoveFixedControl(m_pBack);
			delete m_pBack;
            m_pBack = LTNULL;
		}
	}
}
void CBaseFolder::UseMain(LTBOOL bMain)
{
	if (bMain)
	{
		CreateMain();

		if (IsBuilt())
		{
            AddFixedControl(m_pMain,kBackGroup,m_MainPos,LTTRUE);
		}
	}
	else
	{
		if (m_pMain)
		{
			RemoveFixedControl(m_pMain);
			delete m_pMain;
            m_pMain = LTNULL;
		}
	}
}
void CBaseFolder::UseContinue(int nContinueID, int nHelpID, int nStringID)
{
	if (nContinueID != FOLDER_ID_NONE)
	{
		CreateContinue(nStringID,nHelpID);
		m_nContinueID = nContinueID;

		if (IsBuilt())
		{
			LTBOOL bBack = (m_pBack != LTNULL);
			if (bBack) UseBack(LTFALSE);
	           AddFixedControl(m_pContinue,kNextGroup,m_ContinuePos,LTTRUE);
			if (bBack) UseBack(LTTRUE);
		}
	}
	else
	{
		if (m_pContinue)
		{
			RemoveFixedControl(m_pContinue);
			delete m_pContinue;
            m_pContinue = LTNULL;
		}
	}
}


void CBaseFolder::CreateUpArrow()
{
	if (!m_pUpArrow)
	{
		m_pUpArrow = new CBitmapCtrl;
        m_pUpArrow->Create(g_pLTClient,"interface\\menus\\slider_up.pcx","interface\\menus\\slider_up_h.pcx","interface\\menus\\slider_up_d.pcx", this, FOLDER_CMD_LEFT_ARROW);
		m_pUpArrow->SetHelpID(IDS_HELP_LEFT);
	}
}

void CBaseFolder::CreateDownArrow()
{
	if (!m_pDownArrow)
	{
		m_pDownArrow = new CBitmapCtrl;
        m_pDownArrow->Create(g_pLTClient,"interface\\menus\\slider_down.pcx","interface\\menus\\slider_down_h.pcx","interface\\menus\\slider_down_d.pcx", this, FOLDER_CMD_RIGHT_ARROW);
		m_pDownArrow->SetHelpID(IDS_HELP_RIGHT);
	}
}

void CBaseFolder::CreateBack(LTBOOL bOK, LTBOOL bReturn)
{
	if (m_pBack)
	{
        HSTRING hStr = LTNULL;
		if (bOK)
			hStr = g_pLTClient->FormatString(IDS_OK);
		else
			hStr = g_pLTClient->FormatString(IDS_BACK);
		m_pBack->RemoveAll();
		m_pBack->AddString(hStr);
		if (bReturn)
			m_pBack->SetHelpID(IDS_HELP_RETURN);
		else if (bOK)
			m_pBack->SetHelpID(IDS_HELP_OK);
		else
			m_pBack->SetHelpID(IDS_HELP_BACK);
        g_pLTClient->FreeString(hStr);
	}
	else
	{
		CLTGUIFont *pFont = GetLargeFont();
        HSTRING hStr = LTNULL;
		if (bOK)
			hStr = g_pLTClient->FormatString(IDS_OK);
		else
			hStr = g_pLTClient->FormatString(IDS_BACK);
		m_pBack = new CLTGUITextItemCtrl;
        m_pBack->Create(g_pLTClient, FOLDER_CMD_BACK, hStr, pFont, this);

		SetFontColors(m_pBack, LTFALSE);

		if (bOK)
			m_pBack->SetHelpID(IDS_HELP_OK);
		else
			m_pBack->SetHelpID(IDS_HELP_BACK);
        g_pLTClient->FreeString(hStr);
	}
}

void CBaseFolder::CreateContinue(int nStringID, int nHelpID)
{
	if (!nStringID)
		nStringID = IDS_CONTINUE;
    HSTRING hStr=g_pLTClient->FormatString(nStringID);

	if (!nHelpID)
		nHelpID = IDS_HELP_CONTINUE;
	if (m_pContinue)
	{
		m_pContinue->RemoveAll();
		m_pContinue->AddString(hStr);
	}
	else
	{
		CLTGUIFont *pFont = GetLargeFont();
		m_pContinue = new CLTGUITextItemCtrl;
        m_pContinue->Create(g_pLTClient, FOLDER_CMD_CONTINUE, hStr, pFont, this);

		SetFontColors(m_pContinue, LTFALSE);

        g_pLTClient->FreeString(hStr);
	}
	if (!nHelpID)
		nHelpID = IDS_HELP_CONTINUE;
	m_pContinue->SetHelpID(nHelpID);
}

void CBaseFolder::CreateMain()
{
	if (!m_pMain)
	{
		CLTGUIFont *pFont = GetLargeFont();
        HSTRING hStr=g_pLTClient->FormatString(IDS_MAIN);
		m_pMain = new CLTGUITextItemCtrl;
        m_pMain->Create(g_pLTClient, FOLDER_CMD_MAIN, hStr, pFont, this);

		SetFontColors(m_pMain, LTFALSE);

		m_pMain->SetHelpID(IDS_HELP_MAIN);
        g_pLTClient->FreeString(hStr);
	}
}


void CBaseFolder::RemoveFixedControl(CLTGUICtrl* pControl)
{
	if (!IsBuilt() || !pControl) return;

	ControlArray::iterator iter = std::find(m_fixedControlArray.begin(), m_fixedControlArray.end(), pControl);
	if (iter  != m_fixedControlArray.end())
	{
		m_fixedControlArray.erase(iter);
	}
	iter = std::find(m_skipControlArray.begin(), m_skipControlArray.end(), pControl);
	if (iter != m_skipControlArray.end())
	{
		m_skipControlArray.erase(iter);
	}

//	m_tabGroup.erase(pControl);


}

uint32 CBaseFolder::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
		case FOLDER_CMD_BACK:
		{
			m_pFolderMgr->EscapeCurrentFolder();
			break;
		}

		case FOLDER_CMD_MAIN:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_MAIN);
			break;
		}

		case FOLDER_CMD_CONTINUE:
		{
			if (m_nContinueID != FOLDER_ID_NONE)
			{
				m_pFolderMgr->SetCurrentFolder((eFolderID)m_nContinueID);
				return 1;
			}
			else
				return 0;

			break;
		}

		case FOLDER_CMD_LEFT_ARROW:
		{
			PreviousPage();
			break;
		}

		case FOLDER_CMD_RIGHT_ARROW:
		{
			NextPage();
			break;
		}

		case FOLDER_CMD_OPTIONS:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_OPTIONS);
			break;
		}

		case FOLDER_CMD_DISPLAY:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_DISPLAY);
			break;
		}

		case FOLDER_CMD_AUDIO:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_AUDIO);
			break;
		}

		case FOLDER_CMD_GAME:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_GAME);
			break;
		}

		case FOLDER_CMD_PROFILE:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_PROFILE);
			break;
		}

		case FOLDER_CMD_CONTROLS:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_CUST_CONTROLS);
			break;
		}

		case FOLDER_CMD_MOUSE:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_MOUSE);
			break;
		}

		case FOLDER_CMD_KEYBOARD:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_KEYBOARD);
			break;
		}

		case FOLDER_CMD_JOYSTICK:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_JOYSTICK);
			break;
		}

		case FOLDER_CMD_MULTIPLAYER:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_MULTI);
			break;
		}

		case FOLDER_CMD_MULTIPLAYER_LAN:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_MULTI);
			break;
		}

		case FOLDER_CMD_MP_HOST:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_HOST);
			break;
		}

		case FOLDER_CMD_MP_SETUP:
		{
			switch(g_pServerOptionMgr->GetCurrentCfg()->nGameType)
			{
				case MP_DM:			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_HOST_SETUP_DM);		break;
				case MP_TEAMDM:		m_pFolderMgr->SetCurrentFolder(FOLDER_ID_HOST_SETUP_TEAMDM);	break;
				case MP_HUNT:		m_pFolderMgr->SetCurrentFolder(FOLDER_ID_HOST_SETUP_HUNT);		break;
				case MP_SURVIVOR:	m_pFolderMgr->SetCurrentFolder(FOLDER_ID_HOST_SETUP_SURVIVOR);	break;
				case MP_OVERRUN:	m_pFolderMgr->SetCurrentFolder(FOLDER_ID_HOST_SETUP_OVERRUN);	break;
				case MP_EVAC:		m_pFolderMgr->SetCurrentFolder(FOLDER_ID_HOST_SETUP_EVAC);		break;
			}

			break;
		}

		case FOLDER_CMD_MP_ADVANCED:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_HOST_OPTS);
			break;
		}

		case FOLDER_CMD_MP_MAPS:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_HOST_MAPS);
			break;
		}

		case FOLDER_CMD_MP_PLAYER:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_PLAYER);
			break;
		}

		case FOLDER_CMD_MP_PLAYER_JOIN:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_PLAYER_JOIN);
			break;
		}

		case FOLDER_CMD_MP_CONFIG:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_HOST_CONFIG);
			break;
		}

		case FOLDER_CMD_LAUNCH:
		{
			HSTRING hString = LTNULL;
			CServerOptions *pOptions = g_pServerOptionMgr->GetCurrentCfg();

			// Check to make sure it's a valid game type so far...
			uint8 nGameType = pOptions->nGameType;

			// Call the OnFocus FALSE here so the settings will get saved before moving on...
			FocusSetup(LTFALSE);

			// Check for a non-dedicated server with more than 8 players... and warn them
			int nMaxPlayers = 0;

			switch(pOptions->nGameType)
			{
				case MP_DM:			nMaxPlayers = pOptions->nDM_MaxTotalPlayers;	break;
				case MP_TEAMDM:		nMaxPlayers = pOptions->nTeam_MaxTotalPlayers;	break;
				case MP_HUNT:		nMaxPlayers = pOptions->nHunt_MaxTotalPlayers;	break;
				case MP_SURVIVOR:	nMaxPlayers = pOptions->nSurv_MaxTotalPlayers;	break;
				case MP_OVERRUN:	nMaxPlayers = pOptions->nOver_MaxTotalPlayers;	break;
				case MP_EVAC:		nMaxPlayers = pOptions->nEvac_MaxTotalPlayers;	break;
			}

			if(!pOptions->bDedicated && (nMaxPlayers > 8))
			{
				HSTRING hString = g_pLTClient->FormatString(IDS_HOSTCONFIRM);
				g_pInterfaceMgr->ShowMessageBox(hString, LTMB_YESNO, HostConfirmCallBack, this);
				g_pLTClient->FreeString(hString);
			}
			else
			{
				LTRESULT result = g_pGameClientShell->HostMultiplayerGame(pOptions);

				if(result != LT_OK)
				{
					switch(result)
					{
						case MP_HOST_ERROR_INVALID_OPTIONS:
							hString = g_pLTClient->FormatString(IDS_HOST_ERROR_1);
							break;

						case MP_HOST_ERROR_NO_MAPS:
							hString = g_pLTClient->FormatString(IDS_HOST_ERROR_2);
							break;

						case MP_HOST_ERROR_STARTGAME_FAILED:
							hString = g_pLTClient->FormatString(IDS_HOST_ERROR_3);
							break;

						case MP_HOST_ERROR_UNKNOWN:
						default:
							hString = g_pLTClient->FormatString(IDS_HOST_ERROR_4);
							break;
					}

					g_pInterfaceMgr->ShowMessageBox(hString, LTMB_OK, LTNULL, LTNULL);
					g_pLTClient->FreeString(hString);
				}
			}

			break;
		}

		case CMD_HOSTCONFIRM:
		{
			HSTRING hString = LTNULL;
			CServerOptions *pOptions = g_pServerOptionMgr->GetCurrentCfg();

			// Check to make sure it's a valid game type so far...
			uint8 nGameType = pOptions->nGameType;

			// Call the OnFocus FALSE here so the settings will get saved before moving on...
			FocusSetup(LTFALSE);

			LTRESULT result = g_pGameClientShell->HostMultiplayerGame(pOptions);

			if(result != LT_OK)
			{
				switch(result)
				{
					case MP_HOST_ERROR_INVALID_OPTIONS:
						hString = g_pLTClient->FormatString(IDS_HOST_ERROR_1);
						break;

					case MP_HOST_ERROR_NO_MAPS:
						hString = g_pLTClient->FormatString(IDS_HOST_ERROR_2);
						break;

					case MP_HOST_ERROR_STARTGAME_FAILED:
						hString = g_pLTClient->FormatString(IDS_HOST_ERROR_3);
						break;

					case MP_HOST_ERROR_UNKNOWN:
					default:
						hString = g_pLTClient->FormatString(IDS_HOST_ERROR_4);
						break;
				}

				g_pInterfaceMgr->ShowMessageBox(hString, LTMB_OK, LTNULL, LTNULL);
				g_pLTClient->FreeString(hString);
			}

			break;
		}

		case FOLDER_CMD_MP_JOIN:
		{
			CFolderJoin *pJoin = (CFolderJoin*)m_pFolderMgr->GetFolderFromID(FOLDER_ID_JOIN);

			switch(dwParam1)
			{
				case FOLDERJOIN_NOCHANGE:	break;
				case FOLDERJOIN_LANONLY:	pJoin->SetLANOnly(LTTRUE);			break;
				case FOLDERJOIN_INTERNET:	pJoin->SetLANOnly(LTFALSE);			break;
			}

			switch(dwParam2)
			{
				case FOLDERJOIN_NOREFRESH:	pJoin->SetRefreshOnFocus(LTFALSE);	break;
				case FOLDERJOIN_REFRESH:	pJoin->SetRefreshOnFocus(LTTRUE);	break;
			}

			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_JOIN);

			break;
		}

		case FOLDER_CMD_MP_INFO:
		{
			// Make sure the folders exist
			CFolderJoin *pJoin = (CFolderJoin*)m_pFolderMgr->GetFolderFromID(FOLDER_ID_JOIN);
			CFolderJoinInfo *pInfo = (CFolderJoinInfo*)m_pFolderMgr->GetFolderFromID(FOLDER_ID_JOIN_INFO);
			CFolderMulti *pMulti = (CFolderMulti*)m_pFolderMgr->GetFolderFromID(FOLDER_ID_MULTI);

			if(!pJoin || !pInfo || !pMulti) return 0;


			// Make sure the needed interfaces exist
			CListCtrl *pServerList = pJoin->GetServerList();
			CGameSpyClientMgr *pGameSpy = pMulti->GetGameSpyMgr();

			if(!pServerList || ! pGameSpy) return 0;


			// Get the currently selected game server...
			int nIndex = pServerList->GetSelectedItem();
			void *pHandle = LTNULL;

			if(nIndex == CListCtrl::kNoSelection)
			{
				// TODO!! Display a message box here saying there's no server selected
				return 0;
			}


			// Grab the game information
			pHandle = (void*)pServerList->GetControl(nIndex)->GetParam1();
			CGameSpyServer* pGame = pGameSpy->GetServerFromHandle(pHandle);
			if (!pGame) return 0;


			// Setup the info folder and switch to it
			pInfo->SetGameSpyInfo(pGameSpy, pGame);
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_JOIN_INFO);

			break;
		}

		case FOLDER_CMD_MP_JOIN_GAME:
		{
			CFolderJoin *pJoin = (CFolderJoin*)m_pFolderMgr->GetFolderFromID(FOLDER_ID_JOIN);
			CGameSpyServer *pServer = pJoin->GetSelectedServer();

			if(pServer)
			{
				if(pServer->GetIntValue("lock"))
				{
					ShowPasswordDlg();
				}
				else
				{
					pJoin->JoinServer(pServer);
				}
			}

			break;
		}

		case FOLDER_CMD_PASSWORD:
		{
			if(GetCapture() == m_pPWPassword)
			{
				HidePasswordDlg();

				if(m_szPassword[0])
				{
					CFolderJoin *pJoin = (CFolderJoin*)m_pFolderMgr->GetFolderFromID(FOLDER_ID_JOIN);
					CGameSpyServer *pServer = pJoin->GetSelectedServer();
					pJoin->JoinServer(pServer, m_szPassword);
				}
			}
			break;
		}

		default:
			return 0;
	}

	return 1;
}

void CBaseFolder::ForceMouseUpdate()
{
//	SetSelection(kNoSelection);
	m_dwCurrHelpID = 0;
    LTIntPt cPos = g_pInterfaceMgr->GetCursorPos();
	OnMouseMove(cPos.x,cPos.y);
}

// This is called when the folder gets or loses focus
void CBaseFolder::OnFocus(LTBOOL bFocus)
{
    m_pCaptureCtrl = LTNULL;
	if (bFocus)
	{
		// Setup the console commands for the menu.
		SetupMenuConsoleCommands( LTTRUE );

		int nWidth = m_HelpRect.right - m_HelpRect.left;
		int nHeight = m_HelpRect.bottom - m_HelpRect.top;

		if (m_hHelpSurf)
		{
			uint32 dwW,dwH;
			g_pLTClient->GetSurfaceDims(m_hHelpSurf,&dwW,&dwH);

			if (dwW != (uint32)nWidth || dwH != (uint32)nHeight)
			{
				g_pLTClient->DeleteSurface (m_hHelpSurf);
				m_hHelpSurf = LTNULL;
			}
		}

		if (!m_hHelpSurf)
			m_hHelpSurf = g_pLTClient->CreateSurface((uint32)nWidth,(uint32)nHeight);

		if (m_pUpArrow)
			m_pUpArrow->Enable(LTFALSE);
		if (m_pDownArrow)
			m_pDownArrow->Enable(LTFALSE);

		if (m_nSelection == kNoSelection)
		{
			LTBOOL bSet = LTFALSE;
			int sel = 0;
			while (!bSet && sel < (int)m_controlArray.size())
			{
				CLTGUICtrl *pCtrl = GetControl(0);
				bSet = pCtrl->IsEnabled();
				if (!bSet)
					++sel;
			}
			if (bSet)
			{
				SetSelection(sel);
			}
			else
			{
				sel = 0;
				while (!bSet && sel < (int)m_fixedControlArray.size())
				{
					CLTGUICtrl *pCtrl = GetControl(FixedIndex(sel));
					bSet = pCtrl->IsEnabled();
					if (!bSet)
						++sel;
				}
				if (bSet)
					SetSelection(FixedIndex(sel));

			}

		}

		
		if (m_nLastDrawn < m_nFirstDrawn)
			CalculateLastDrawn();
		CheckArrows();
		ForceMouseUpdate();
		UpdateHelpText();
		CreateInterfaceSFX();
		m_bHasFocus = LTTRUE;

	}
	else
	{
		// Restore the console commands for the menu.
		SetupMenuConsoleCommands( LTFALSE );

		m_bHasFocus = LTFALSE;
		SetSelection(kNoSelection);
		m_nLastDrawn = -1;
		m_nFirstDrawn = 0;
		RemoveInterfaceSFX();

	}
}

void CBaseFolder::CheckArrows()
{
	if (m_pUpArrow)
	{
		m_pUpArrow->Enable(!IsFirstPage());
		if (m_pUpArrow->IsEnabled())
		{
			m_pUpArrow->SetHelpID(IDS_HELP_LEFT);
			m_pUpArrow->SetPos(m_UpArrowPos);
		}
		else
		{
			m_pUpArrow->SetHelpID(0);
            m_pUpArrow->SetPos(offscreen);
		}
	}

	if (m_pDownArrow)
	{
		m_pDownArrow->Enable(!IsLastPage());
		if (m_pDownArrow->IsEnabled())
		{
			m_pDownArrow->SetHelpID(IDS_HELP_RIGHT);
			m_pDownArrow->SetPos(m_DownArrowPos);
		}
		else
		{
			m_pDownArrow->SetHelpID(0);
            m_pDownArrow->SetPos(offscreen);
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseFolder::CreateInterfaceSFX
//
//	PURPOSE:	Create the SFX to render on this screen
//
// ----------------------------------------------------------------------- //

void CBaseFolder::CreateInterfaceSFX()
{

	int n = 0;
	char szAttName[30];
	char szFXName[128];


	HOBJECT hCamera = g_pGameClientShell->GetInterfaceCamera();
	if (!hCamera) return;


    g_pLTClient->GetObjectPos(hCamera, &g_vPos);
    g_pLTClient->GetObjectRotation(hCamera, &g_rRot);
    g_pLTClient->GetRotationVectors(&g_rRot, &g_vU, &g_vR, &g_vF);


	sprintf(szAttName,"ScaleName%d",n);
	while (g_pLayoutMgr->HasCustomValue((eFolderID)m_nFolderID,szAttName))
	{
		g_pLayoutMgr->GetFolderCustomString((eFolderID)m_nFolderID,szAttName,szFXName,128);
		if (strlen(szFXName))
		{
			CreateScaleFX(szFXName);
		}

		n++;
		sprintf(szAttName,"ScaleName%d",n);

	}

	if (m_pBack)
	{
		g_pLayoutMgr->GetArrowBackSFX(szFXName,128);
		if (strlen(szFXName))
		{
			CreateScaleFX(szFXName);
		}
	}
	if (m_pContinue)
	{
		g_pLayoutMgr->GetArrowNextSFX(szFXName,128);
		if (strlen(szFXName))
		{
			CreateScaleFX(szFXName);
		}
	}


	INT_CHAR *pChar = g_pLayoutMgr->GetFolderCharacter((eFolderID)m_nFolderID);
	if (pChar)
	{
		CreateCharFX(pChar);

// Turn off attachments for now
/*
		if (m_CharSFX.GetObject())
		{
			int reqID[MAX_INT_ATTACHMENTS];
			int numReq = g_pAttachButeMgr->GetRequirementIDs(pChar->szModel,pChar->szStyle,reqID,MAX_INT_ATTACHMENTS);
			for (int i = 0; i < numReq; i++)
			{
				INT_ATTACH acs;
				acs.fScale = pChar->fScale;
				acs.nAttachmentID = g_pAttachButeMgr->GetRequirementAttachment(reqID[i]);
				CString socket = g_pAttachButeMgr->GetRequirementSocket(reqID[i]);
				acs.pszSocket = (char *)(LPCTSTR)socket;

				CreateAttachFX(&acs);
			}

			int numAtt = g_pLayoutMgr->GetFolderNumAttachments((eFolderID)m_nFolderID);
			for (int i = 0; i < numAtt; i++)
			{
				char szTemp[128];
				char *pName = LTNULL;
				char *pSocket = LTNULL;
				g_pLayoutMgr->GetFolderAttachment( (eFolderID)m_nFolderID, i, szTemp, 128);

				pName = strtok(szTemp,";");
				pSocket = strtok(NULL,";");

				INT_ATTACH acs;

				acs.fScale = pChar->fScale;
				acs.nAttachmentID = g_pAttachButeMgr->GetAttachmentIDByName(pName);
				acs.pszSocket = pSocket;

				CreateAttachFX(&acs);
			}
		}
*/
	}


	// Create the logo scale FX
	if(m_bUseLogo)
	{
		switch(m_nLogoScaleFX)
		{
			case LOGO_SCALEFX_SIERRA:
			{
				CreateScaleFX("SierraLogo");
				break;
			}

			case LOGO_SCALEFX_GAMESPY:
			{
				CreateScaleFX("GameSpyLogo");
				break;
			}

			case LOGO_SCALEFX_NONE:
			default:
			{
				break;
			}
		}
	}


	// Create the logo scale FX
	if(m_bUseLogo)
	{
		switch(m_nLogoScaleFX)
		{
			case LOGO_SCALEFX_SIERRA:
			{
				CreateScaleFX("SierraLogo");
				break;
			}

			case LOGO_SCALEFX_GAMESPY:
			{
				CreateScaleFX("GameSpyLogo");
				break;
			}

			case LOGO_SCALEFX_NONE:
			default:
			{
				break;
			}
		}
	}
}

void CBaseFolder::RemoveInterfaceSFX()
{
	for(SFXArray::iterator iter = m_SFXArray.begin(); iter != m_SFXArray.end(); ++iter)
	{
		g_pInterfaceMgr->RemoveInterfaceSFX(*iter);
		delete *iter;
	}
	m_SFXArray.clear();

//	while (m_SFXArray.size() > 0)
//	{
//		CSpecialFX *pSFX = m_SFXArray[0];
//		g_pInterfaceMgr->RemoveInterfaceSFX(pSFX);
//		delete pSFX;
//		m_SFXArray.Remove(0);
//	}

	g_pInterfaceMgr->RemoveInterfaceSFX(&m_CharSFX);
	m_CharSFX.Reset();
	m_CharSFX.Term();

	ClearAttachFX();

}

void CBaseFolder::ClearAttachFX()
{
/*
	for (int i = 0; i < MAX_INT_ATTACHMENTS; i++)
	{
		g_pInterfaceMgr->RemoveInterfaceSFX(&m_aAttachment[i].sfx);
		m_aAttachment[i].sfx.Reset();
		m_aAttachment[i].sfx.Term();
		m_aAttachment[i].socket = INVALID_MODEL_SOCKET;
	}
	m_nNumAttachments = 0;
*/
}

void CBaseFolder::UpdateInterfaceSFX()
{

	for (int i = 0; i < m_nNumAttachments; i++)
	{
		CBaseScaleFX *pSFX = &m_aAttachment[i].sfx;
		
		HMODELSOCKET hSocket = m_aAttachment[i].socket;
		LTVector vPos;
		LTRotation rRot;
		LTransform transform;
		if (g_pModelLT->GetSocketTransform(m_CharSFX.GetObject(), hSocket, transform, LTTRUE) == LT_OK)
		{
			g_pTransLT->Get(transform, vPos, rRot);
            g_pLTClient->SetObjectPos(pSFX->GetObject(), &vPos, LTTRUE);
            g_pLTClient->SetObjectRotation(pSFX->GetObject(), &rRot);

		}
	}


}


int CBaseFolder::GetIndex(CLTGUICtrl* pCtrl)
{
	// Is it in the control array?
	ControlArray::iterator iter = std::find(m_controlArray.begin(), m_controlArray.end(), pCtrl);
	int dwIndex = iter - m_controlArray.begin();
	if ((uint32)dwIndex < m_controlArray.size())
	{
		return dwIndex;
	}

	// How about in the fixed array?
	iter = std::find(m_fixedControlArray.begin(), m_fixedControlArray.end(), pCtrl);
	dwIndex = iter - m_fixedControlArray.begin();
	if ((uint32)dwIndex < m_fixedControlArray.size())
	{
		return FixedIndex(dwIndex);
	}

	// No? Oh well.
	return kNoSelection;
}


CBaseScaleFX* CBaseFolder::CreateScaleFX(char *szFXName)
{

	CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX(szFXName);
	if (pScaleFX)
	{
		CBaseScaleFX *pSFX = new CBaseScaleFX;
		g_pFXButeMgr->CreateScaleFX(pScaleFX,g_vPos, g_vF, LTNULL, &g_rRot, pSFX);
		m_SFXArray.push_back(pSFX);
		g_pInterfaceMgr->AddInterfaceSFX(pSFX, IFX_NORMAL);				

		//adjust the object's position based on screen res
		HOBJECT hSFX = pSFX->GetObject();
		if (hSFX)
		{
			LTVector vNewPos;
			g_pLTClient->GetObjectPos(hSFX, &vNewPos);
			vNewPos.z *= g_pInterfaceResMgr->GetXRatio();
			g_pLTClient->SetObjectPos(hSFX, &vNewPos);
		}

		return pSFX;

	}
	return LTNULL;
}

void CBaseFolder::CreateCharFX(INT_CHAR *pChar)
{
	if (pChar)
	{
		BSCREATESTRUCT bcs;
	    LTVector vPos, vTemp, vScale(1.0f,1.0f,1.0f);
	    LTRotation rRot = g_rRot;

		int characterSet = g_pCharacterButeMgr->GetSetFromModelType(pChar->szModel);
		if( characterSet < 0 )
			characterSet = 0;

		bcs.Filename = g_pCharacterButeMgr->GetDefaultModel(characterSet);

		for(int i = 0; i < MAX_MODEL_TEXTURES; ++i)
		{
			bcs.Skins[i] = g_pCharacterButeMgr->GetDefaultSkin(characterSet, i);
		}

		VEC_COPY(vPos,g_vPos);
		VEC_SET(vScale,1.0f,1.0f,1.0f);
		VEC_MULSCALAR(vScale, vScale, pChar->fScale);

		LTVector vModPos = pChar->vPos;
	    LTFLOAT fRot = pChar->fRot;
		fRot  = MATH_PI + DEG2RAD(fRot);
	    g_pLTClient->RotateAroundAxis(&rRot, &g_vU, fRot);

		VEC_MULSCALAR(vTemp, g_vF, vModPos.z);
		VEC_MULSCALAR(vTemp, vTemp, g_pInterfaceResMgr->GetXRatio());
		VEC_ADD(vPos, vPos, vTemp);

		VEC_MULSCALAR(vTemp, g_vR, vModPos.x);
		VEC_ADD(vPos, vPos, vTemp);

		VEC_MULSCALAR(vTemp, g_vU, vModPos.y);
		VEC_ADD(vPos, vPos, vTemp);

		VEC_COPY(bcs.vPos, vPos);
		bcs.rRot = rRot;
		VEC_COPY(bcs.vInitialScale, vScale);
		VEC_COPY(bcs.vFinalScale, vScale);
		VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
		VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
		bcs.bUseUserColors = LTTRUE;

		bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE | FLAG_NOLIGHT;

		bcs.nType = OT_MODEL;
		bcs.fInitialAlpha = 1.0f;
		bcs.fFinalAlpha = 1.0f;
		bcs.fLifeTime = 1000000.0f;
		bcs.bLoop = LTTRUE;


		if (m_CharSFX.Init(&bcs))
		{
			m_CharSFX.CreateObject(g_pLTClient);
			if (m_CharSFX.GetObject())
			{
				HMODELANIM hmodelAnim = g_pLTClient->GetAnimIndex(m_CharSFX.GetObject(), pChar->szStyle);
				g_pLTClient->SetModelAnimation(m_CharSFX.GetObject(), hmodelAnim);
				g_pInterfaceMgr->AddInterfaceSFX(&m_CharSFX, IFX_WORLD);
			}
		}

	}
}

void CBaseFolder::CreateAttachFX(INT_ATTACH *pAttach)
{
/*
	if (m_nNumAttachments < MAX_INT_ATTACHMENTS)
	{

		BSCREATESTRUCT bcs;
	    LTVector vPos, vTemp, vScale(1.0f,1.0f,1.0f);
	    LTRotation rRot = g_rRot;

		CString str = "";
		char szModel[128];
		char szSkin[128];

		str = g_pAttachButeMgr->GetAttachmentModel(pAttach->nAttachmentID);
		strncpy(szModel, (char*)(LPCSTR)str, 128);

		str = g_pAttachButeMgr->GetAttachmentSkin(pAttach->nAttachmentID);
		strncpy(szSkin, (char*)(LPCSTR)str, 128);

		VEC_SET(vScale,1.0f,1.0f,1.0f);
		VEC_MULSCALAR(vScale, vScale, pAttach->fScale);

		VEC_COPY(bcs.vInitialScale, vScale);
		VEC_COPY(bcs.vFinalScale, vScale);
		VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
		VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
		bcs.bUseUserColors = LTTRUE;

		bcs.pFilename = szModel;
		bcs.pSkin = szSkin;
		bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE | FLAG_NOLIGHT;

		bcs.nType = OT_MODEL;
		bcs.fInitialAlpha = 1.0f;
		bcs.fFinalAlpha = 1.0f;
		bcs.fLifeTime = 1000000.0f;
		bcs.bLoop = LTTRUE;

		CBaseScaleFX *pSFX = &m_aAttachment[m_nNumAttachments].sfx;

		if (!pSFX->Init(&bcs)) return;
		
		pSFX->CreateObject(g_pLTClient);
		if (!pSFX->GetObject()) return;

		HOBJECT hChar = m_CharSFX.GetObject();
		if (!hChar) return;
		if (g_pModelLT->GetSocket(hChar, pAttach->pszSocket, m_aAttachment[m_nNumAttachments].socket) != LT_OK)
			return;


		g_pInterfaceMgr->AddInterfaceSFX(pSFX, IFX_ATTACH);
		m_nNumAttachments++;


	}
*/
}

void CBaseFolder::UpdateHelpText()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
    uint32 dwID = 0;
	if (pCtrl)
		dwID = pCtrl->GetHelpID();

	if (dwID != m_dwCurrHelpID)
	{
		m_dwCurrHelpID = dwID;

		int nWidth = m_HelpRect.right - m_HelpRect.left;
		int nHeight = m_HelpRect.bottom - m_HelpRect.top;
        LTRect rect(0,0,nWidth,nHeight);
        g_pLTClient->FillRect(m_hHelpSurf,&rect,kTransBlack);

		if (m_dwCurrHelpID)
		{
			HSTRING hHelpTxt = GetHelpString(m_dwCurrHelpID,m_nSelection);

            GetHelpFont()->DrawFormat(hHelpTxt,m_hHelpSurf,0,0,(uint32)nWidth,kWhite);
			g_pLTClient->OptimizeSurface(m_hHelpSurf,kTransBlack);
            g_pLTClient->FreeString(hHelpTxt);
		}
	}
}

CLTGUITextItemCtrl* CBaseFolder::AddTextItem(HSTRING hString, uint32 commandID, int helpID, LTBOOL bFixed, CLTGUIFont *pFont, int *pnValue)
{
	CLTGUITextItemCtrl* pCtrl = CreateTextItem(hString, commandID, helpID, bFixed, pFont, pnValue);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}
CLTGUITextItemCtrl* CBaseFolder::AddTextItem(int stringID, uint32 commandID, int helpID, LTBOOL bFixed, CLTGUIFont *pFont, int *pnValue)
{
	CLTGUITextItemCtrl* pCtrl = CreateTextItem(stringID, commandID, helpID, bFixed, pFont, pnValue);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}
CLTGUITextItemCtrl* CBaseFolder::AddTextItem(const char *pString, uint32 commandID, int helpID, LTBOOL bFixed, CLTGUIFont *pFont, int *pnValue)
{
	CLTGUITextItemCtrl* pCtrl = CreateTextItem(pString, commandID, helpID, bFixed, pFont, pnValue);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}

CStaticTextCtrl* CBaseFolder::AddStaticTextItem(HSTRING hString, uint32 commandID, int helpID, int width, int height, LTBOOL bFixed, CLTGUIFont *pFont)
{
	CStaticTextCtrl* pCtrl = CreateStaticTextItem(hString, commandID, helpID, width, height, bFixed, pFont);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}
CStaticTextCtrl* CBaseFolder::AddStaticTextItem(int stringID, uint32 commandID, int helpID, int width, int height, LTBOOL bFixed, CLTGUIFont *pFont)
{
	CStaticTextCtrl* pCtrl = CreateStaticTextItem(stringID, commandID, helpID, width, height, bFixed, pFont);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}
CStaticTextCtrl* CBaseFolder::AddStaticTextItem(char *pString, uint32 commandID, int helpID, int width, int height, LTBOOL bFixed, CLTGUIFont *pFont)
{
	CStaticTextCtrl* pCtrl = CreateStaticTextItem(pString, commandID, helpID, width, height, bFixed, pFont);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}

CLTGUIEditCtrl* CBaseFolder::AddEditCtrl(HSTRING hDescription, uint32 commandID, int helpID, char *pBuffer, int nBufferSize, int nTextOffset, LTBOOL bFixed, CLTGUIFont *pFont)
{
	CLTGUIEditCtrl*	pCtrl = CreateEditCtrl(hDescription, commandID, helpID, pBuffer, nBufferSize, nTextOffset, bFixed, pFont);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}
CLTGUIEditCtrl* CBaseFolder::AddEditCtrl(int nDescriptionID, uint32 commandID, int helpID, char *pBuffer, int nBufferSize, int nTextOffset, LTBOOL bFixed, CLTGUIFont *pFont)
{
	CLTGUIEditCtrl*	pCtrl = CreateEditCtrl(nDescriptionID, commandID, helpID, pBuffer, nBufferSize, nTextOffset, bFixed, pFont);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}
CLTGUIEditCtrl* CBaseFolder::AddEditCtrl(char *pszDescription, uint32 commandID, int helpID, char *pBuffer, int nBufferSize, int nTextOffset, LTBOOL bFixed, CLTGUIFont *pFont)
{
	CLTGUIEditCtrl*	pCtrl = CreateEditCtrl(pszDescription, commandID, helpID, pBuffer, nBufferSize, nTextOffset, bFixed, pFont);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}

CCycleCtrl*	CBaseFolder::AddCycleItem(HSTRING hText, int helpID, int nHeaderWidth, int nSpacerWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
	CCycleCtrl*	pCtrl = CreateCycleItem(hText, helpID, nHeaderWidth, nSpacerWidth, pnValue, bFixed, pFont);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}
CCycleCtrl*	CBaseFolder::AddCycleItem(int stringID,	int helpID, int nHeaderWidth, int nSpacerWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
	CCycleCtrl*	pCtrl = CreateCycleItem(stringID, helpID, nHeaderWidth, nSpacerWidth, pnValue, bFixed, pFont);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}
CCycleCtrl*	CBaseFolder::AddCycleItem(char *pString, int helpID, int nHeaderWidth, int nSpacerWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
	CCycleCtrl*	pCtrl = CreateCycleItem(pString, helpID, nHeaderWidth, nSpacerWidth, pnValue, bFixed, pFont);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}

CToggleCtrl* CBaseFolder::AddToggle(HSTRING hText, int helpID, int nRightColumnOffset, LTBOOL *pbValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
	CToggleCtrl* pCtrl = CreateToggle(hText, helpID, nRightColumnOffset, pbValue, bFixed, pFont);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}
CToggleCtrl* CBaseFolder::AddToggle(int stringID, int helpID, int nRightColumnOffset, LTBOOL *pbValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
	CToggleCtrl* pCtrl = CreateToggle(stringID, helpID, nRightColumnOffset, pbValue, bFixed, pFont);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}
CToggleCtrl* CBaseFolder::AddToggle(char *pString, int helpID, int nRightColumnOffset, LTBOOL *pbValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
	CToggleCtrl* pCtrl = CreateToggle(pString, helpID, nRightColumnOffset, pbValue, bFixed, pFont);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}

CSliderCtrl* CBaseFolder::AddSlider(HSTRING hText, int helpID, int nSliderOffset, int nSliderWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont )
{
	CSliderCtrl* pCtrl = CBaseFolder::CreateSlider(hText, helpID, nSliderOffset, nSliderWidth, pnValue, bFixed, pFont );
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}
CSliderCtrl* CBaseFolder::AddSlider(int stringID,  int helpID, int nSliderOffset, int nSliderWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont )
{
	CSliderCtrl* pCtrl = CBaseFolder::CreateSlider(stringID, helpID, nSliderOffset, nSliderWidth, pnValue, bFixed, pFont );
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}
CSliderCtrl* CBaseFolder::AddSlider(char *pString, int helpID, int nSliderOffset, int nSliderWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont )
{
	CSliderCtrl* pCtrl = CBaseFolder::CreateSlider(pString, helpID, nSliderOffset, nSliderWidth, pnValue, bFixed, pFont );
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}

CLTGUIColumnTextCtrl* CBaseFolder::AddColumnText(DWORD dwCommandID, int helpID, LTBOOL bFixed, CLTGUIFont *pFont, DWORD dwParam1, DWORD dwParam2)
{
	CLTGUIColumnTextCtrl* pCtrl = CBaseFolder::CreateColumnText(dwCommandID, helpID, bFixed, pFont, dwParam1, dwParam2);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}

CGroupCtrl* CBaseFolder::AddGroup(int nWidth , int nHeight, int helpID)
{
	CGroupCtrl* pCtrl = CBaseFolder::CreateGroup(nWidth,nHeight,helpID);
	if (pCtrl)
		AddFreeControl(pCtrl);
	return pCtrl;
}

LTBOOL CBaseFolder::SkipControl(CLTGUICtrl* pControl)
{
	if (!pControl) return  LTFALSE;
	ControlArray::iterator iter = std::find(m_skipControlArray.begin(), m_skipControlArray.end(), pControl);
	return (iter != m_skipControlArray.end());
}

CLTGUITextItemCtrl* CBaseFolder::AddLink(int stringID, uint32 commandID, int helpID, int xpos, int ypos)
{
    HSTRING hTxt=g_pLTClient->FormatString(stringID);
	CLTGUITextItemCtrl* pCtrl= CreateTextItem(stringID,commandID,helpID, LTFALSE, GetLargeFont());

    LTIntPt pos(xpos ,ypos);
	AddFixedControl(pCtrl,kLinkGroup,pos);

	return pCtrl;

}

CLTGUITextItemCtrl* CBaseFolder::AddLink(int stringID, uint32 commandID, int helpID)
{
    HSTRING hTxt=g_pLTClient->FormatString(stringID);
	CLTGUITextItemCtrl* pCtrl= CreateTextItem(stringID,commandID,helpID, LTFALSE, GetLargeFont());

    LTIntPt pos = m_LinkPos[m_nNextLink];
	m_nNextLink++;
	AddFixedControl(pCtrl,kLinkGroup,pos);

	return pCtrl;

}

CLTGUITextItemCtrl* CBaseFolder::AddTab(int stringID, uint32 commandID, int helpID)
{
    HSTRING hTxt=g_pLTClient->FormatString(stringID);
	CLTGUITextItemCtrl* pCtrl= CreateTextItem(stringID,commandID,helpID, LTFALSE, GetSmallFont());

    LTIntPt pos = m_TabPos[m_nNumTabs];
	m_nNumTabs++;
	AddFixedControl(pCtrl,kTabGroup,pos);

	return pCtrl;

}

void CBaseFolder::SetFontColors(CLTGUIFadeColorCtrl *control, LTBOOL fixed)
{
	if (fixed)
	{
		control->SetColor(m_hNormalColor,m_hNormalColor,m_hNormalColor);
	}
	else
	{
		control->SetColor(m_hSelectedColor,m_hNormalColor,m_hDisabledColor);
	}
}


void CBaseFolder::SetupMenuConsoleCommands( LTBOOL bMenuValues )
{
	// Check if the menu is being entered.  If so, we need to 
	// save all the current settings for the console commands we
	// are about to change.  Then change them to the values we need.
	if( bMenuValues )
	{
		// Save the current console values.
		m_fFarZ = GetConsoleFloat( "FarZ", 10000.0f );
		m_bFogEnable = GetConsoleInt( "FogEnable", FALSE );

		// Set the farz to very far so the 3D special effects in the menus show up.
		WriteConsoleFloat( "FarZ", 10000.0f );

		// Don't need fog in the menus.
		WriteConsoleInt( "FogEnable", FALSE );
	}
	// If we are not entering the menu, we need to restore the console
	// commands to the values they were before we entered the menu.
	else
	{
		// Restore the console variables we changed.
		WriteConsoleFloat( "FarZ", m_fFarZ );
		WriteConsoleInt( "FogEnable", m_bFogEnable );
	}
}


void CBaseFolder::AddFrame(int x, int y, int width, int height, int headerHeight, LTBOOL bOuterFrame)
{
	CFrameCtrl *pFrame = LTNULL;

	LTIntPt pos(x,y);
	if (headerHeight)
	{
		//header
		pFrame = new CFrameCtrl;
		pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, width, headerHeight);
		AddFixedControl(pFrame, kNoGroup, pos, LTFALSE);
	}

	pos.y += headerHeight;
	height -= headerHeight;

	//black frame
	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(10,10,10), 0.75f, width, height);
	AddFixedControl(pFrame, kNoGroup, pos, LTFALSE);

	if (bOuterFrame)
	{
		//blue outer frame
		pFrame = new CFrameCtrl;
		pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, width, height, LTFALSE);
		AddFixedControl(pFrame, kNoGroup, pos, LTFALSE);
	}

}


void CBaseFolder::BuildPasswordDlg()
{
	LTIntPt	pt(0, 0);

	ptDlgPWSize			= g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_JOIN, "DlgPWSize");
	ptDlgPWEditPW		= g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_JOIN, "DlgPWEditPW");
	ptDlgPWOffset		= g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_JOIN, "DlgPWOffset");


	// Create the group that will contain all the appropriate controls
	m_pPWDlgGroup = CreateGroup(ptDlgPWSize.x, ptDlgPWSize.y, LTNULL);
	m_pPWDlgGroup->AllowSubSelection(LTTRUE);
	m_pPWDlgGroup->Enable(LTTRUE);


	// Create the frame of the dialog
	CFrameCtrl *pFrame;

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(10,10,10), 0.75f, ptDlgPWSize.x, ptDlgPWSize.y);
	m_pPWDlgGroup->AddControl(pFrame, pt);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, ptDlgPWSize.x, ptDlgPWSize.y, LTFALSE);
	m_pPWDlgGroup->AddControl(pFrame, pt);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, ptDlgPWSize.x, GetHelpFont()->GetHeight() + 6);
	m_pPWDlgGroup->AddControl(pFrame, pt);


	// Now create the title for the dialog
	m_pPWTitle = CreateStaticTextItem(IDS_PASSWORD_DIALOG, LTNULL, LTNULL, ptDlgPWSize.x, ptDlgPWSize.y, LTTRUE, GetHelpFont());
	m_pPWDlgGroup->AddControl(m_pPWTitle, LTIntPt(3, 3));


	// Create the controls for entering the password
	CLTGUITextItemCtrl*	pCtrl = CreateTextItem(IDS_JOIN_PASSWORD, LTNULL, LTNULL, LTTRUE);
	m_pPWDlgGroup->AddControl(pCtrl, ptDlgPWEditPW, LTFALSE);

	m_pPWPassword = CreateEditCtrl(" ", FOLDER_CMD_PASSWORD, LTNULL, m_szPassword, sizeof(m_szPassword), 0, LTTRUE);
	m_pPWPassword->EnableCursor();
    m_pPWPassword->Enable(LTFALSE);
	m_pPWDlgGroup->AddControl(m_pPWPassword, LTIntPt(ptDlgPWEditPW.x + ptDlgPWOffset, ptDlgPWEditPW.y), LTTRUE);
}

void CBaseFolder::ShowPasswordDlg()
{
	memset(m_szPassword, 0, sizeof(m_szPassword));
	m_pPWPassword->UpdateData(LTFALSE);

	LTIntPt pos(0,0);

	pos.x = 320 - ptDlgPWSize.x / 2;
	pos.y = 240 - ptDlgPWSize.y / 2;

	AddFixedControl(m_pPWDlgGroup, kFreeGroup, pos, LTTRUE);

	SetSelection(GetIndex(m_pPWDlgGroup));

	SetCapture(m_pPWPassword);
	m_pPWPassword->SetColor(m_hSelectedColor, m_hSelectedColor, m_hSelectedColor);
	m_pPWPassword->Select(LTTRUE);
}

void CBaseFolder::HidePasswordDlg()
{
	m_pPWPassword->UpdateData(LTTRUE);

	SetSelection(kNoSelection);

	RemoveFixedControl(m_pPWDlgGroup);

	SetCapture(LTNULL);
	ForceMouseUpdate();
}


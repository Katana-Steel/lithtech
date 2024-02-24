// FolderJoin.cpp: implementation of the CFolderJoin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderJoin.h"
#include "FolderJoinInfo.h"
#include "FolderMgr.h"
#include "LayoutMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "GameClientShell.h"

#include "MultiplayerMgrDefs.h"
#include "FolderMulti.h"


namespace
{
	int kIconWidth = 0;
	int kNameWidth = 0;
	int kPingWidth = 0;
	int kNumWidth = 0;
	int kTypeWidth = 0;
	int kMapWidth = 0;
	int nOldIndex = 0;
	int nOldSelection = -1;

	int nLAN = 0;

	LTIntPt ptCountPos;
	LTIntPt ptStatusPos;
	LTIntPt ptListSize;

	LTIntPt ptDlgJSIPSize;
	LTIntPt ptDlgJSIPEditIP;
	LTIntPt ptDlgJSIPEditPort;
	LTIntPt ptDlgJSIPEditPW;
	int ptDlgJSIPOffset;

	enum LocalCommands
	{
		CMD_JOIN_IP = FOLDER_CMD_CUSTOM,
		CMD_REFRESH,
		CMD_SORT_NAME,
		CMD_SORT_PING,
		CMD_SORT_NUM,
		CMD_SORT_TYPE,
		CMD_SORT_MAP,
		CMD_FILTER_POP,
		CMD_FILTER_VERSION,
		CMD_FILTER_TYPE,
		CMD_FILTER_PING,
		CMD_FILTER_DEDICATED,
		CMD_EDIT_IP,
		CMD_EDIT_PORT,
		CMD_EDIT_PASSWORD,
	};

	const int kNumPopFilters = 4;
	int anPopFilter[kNumPopFilters] = 
	{
		IDS_POP_ANYNUM,
		IDS_POP_NOTFULL,
		IDS_POP_NOTEMPTY,
		IDS_POP_NEITHER,
	};

	const int kNumTypeFilters = 7;
	int anTypeFilter[kNumTypeFilters] = 
	{
		IDS_FILTER_ANYTYPE,
		IDS_FILTER_DM,
		IDS_FILTER_TEAMDM,
		IDS_FILTER_HUNT,
		IDS_FILTER_SURVIVOR,
		IDS_FILTER_OVERRUN,
		IDS_FILTER_EVAC,
	};

	const int kNumPingFilters = 4;
	int anPingFilterNum[kNumPingFilters] = 
	{
		10000,
		100,
		200,
		400,
	};
	int anPingFilter[kNumPingFilters] = 
	{
		IDS_FILTER_ANYPING,
		IDS_FILTER_PING100,
		IDS_FILTER_PING200,
		IDS_FILTER_PING400,
	};

	const int kNumDedicatedFilters = 3;
	int anDedicatedFilter[kNumDedicatedFilters] =
	{
		IDS_DEDICATED_ANYTYPE,
		IDS_DEDICATED_YES,
		IDS_DEDICATED_NO,
	};

	// Find Servers State
	enum eFSState
	{
		FSS_IDLE = 0,
		FSS_GETSERVICES,
		FSS_GETPINGS,
		FSS_DUMMYSTATUS,
	};

	uint32 timeLastFullUpdate = 0;

	const int kDlgWidth = 240;
	const int kDlgHeight = 120;
	char  szOldStr[64];
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderJoin::CFolderJoin()
{
	m_pServerList = LTNULL;
	m_nPopFilter = 0;
	m_nVersionFilter = 0;
	m_nTypeFilter = 0;
	m_nPingFilter = 0;
	m_nDedicatedFilter = 0;

	m_nServerSort = 0;

	m_bForceUpdate = LTFALSE;

	m_pServerListTitle = LTNULL;
	m_pFilterTitle = LTNULL;

	m_pJoinIP = LTNULL;

	m_pPopFilter = LTNULL;
	m_pVersionFilter = LTNULL;
	m_pTypeFilter = LTNULL;
	m_pPingFilter = LTNULL;
	m_pDedicatedFilter = LTNULL;

	m_hServersShown = LTNULL;
	m_hDummyStatus = LTNULL;
	m_hStatus = LTNULL;

	m_pSortName = LTNULL;
	m_pSortPing = LTNULL;
	m_pSortNum = LTNULL;
	m_pSortType = LTNULL;
	m_pSortMap = LTNULL;
	m_pSortFrame = LTNULL;

	m_bLANOnly = LTFALSE;
	m_bRefreshOnFocus = LTTRUE;

	m_szIP[0] = LTNULL;
	m_szPort[0] = LTNULL;

	m_pJSIPTitle = LTNULL;
	m_pJSIPIPEdit = LTNULL;
	m_pJSIPPortEdit = LTNULL;
	m_pJSIPPassword = LTNULL;
	m_pJSIPDlgGroup = LTNULL;

	m_pIPCtrl = LTNULL;

	m_bEscape = LTFALSE;

	m_hLock = LTNULL;
}


CFolderJoin::~CFolderJoin()
{
	if(m_hLock)
	{
		g_pLTClient->DeleteSurface(m_hLock);
		m_hLock = LTNULL;
	}
}


CGameSpyClientMgr* CFolderJoin::GetGameSpyMgr()
{
	CFolderMulti *pMulti = (CFolderMulti*)m_pFolderMgr->GetFolderFromID(FOLDER_ID_MULTI);
	return pMulti->GetGameSpyMgr();
}


// Build the folder
LTBOOL CFolderJoin::Build()
{
	// Some temp variables for this function...
	CLTGUITextItemCtrl *pCtrl = LTNULL;
	CFrameCtrl *pFrame = LTNULL;
	int i, nWidth, nHeight;
	LTIntPt pos;


	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;

	// Setup the title of this folder
	CreateTitle(IDS_TITLE_JOIN);
	UseLogo(LTTRUE);


	// Create the lock graphic
	m_hLock = g_pLTClient->CreateSurfaceFromBitmap("Interface\\Menus\\lock.pcx");
	g_pLTClient->OptimizeSurface(m_hLock, kTransBlack);


	// Add the links to other menus...
	m_pServerListTitle = AddLink(IDS_SERVER_LIST, FOLDER_CMD_MP_JOIN, IDS_HELP_SERVER_LIST);
	m_pServerListTitle->Enable(LTFALSE);

	AddLink(IDS_SERVER_INFO, FOLDER_CMD_MP_INFO, IDS_HELP_SERVER_INFO);
	AddLink(IDS_PLAYER, FOLDER_CMD_MP_PLAYER_JOIN, IDS_HELP_PLAYER);


	// Grab some custom values out of the attributes
	kIconWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "IconWidth");
	kNameWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "NameWidth");
	kPingWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "PingWidth");
	kNumWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "NumWidth");
	kTypeWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "TypeWidth");
	kMapWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "MapWidth");

	ptCountPos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "CountPos");
	ptStatusPos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "StatusPos");
	ptListSize = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "ListSize");


	// -----------------------------------------------------------------
	// Setup all the frames

	pos = LTIntPt(GetPageLeft(), GetPageTop());
	nWidth = GetPageRight() - GetPageLeft();
	nHeight = GetPageBottom() - GetPageTop();

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(10,10,10), 0.75f, nWidth, nHeight);
	AddFixedControl(pFrame, kNoGroup, pos, LTFALSE);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, nWidth, GetTinyFont()->GetHeight() + 6);
	AddFixedControl(pFrame, kNoGroup, pos, LTFALSE);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, nWidth, nHeight, LTFALSE);
	AddFixedControl(pFrame, kNoGroup, pos, LTFALSE);

	pos.y += nHeight - (GetTinyFont()->GetHeight() + 6);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, nWidth, GetTinyFont()->GetHeight() + 6);
	AddFixedControl(pFrame, kNoGroup, pos, LTFALSE);

	pos.y -= (GetTinyFont()->GetHeight() + 4);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, (nWidth / 4) + 1, GetTinyFont()->GetHeight() + 6, LTFALSE);
	AddFixedControl(pFrame, kNoGroup, pos, LTFALSE);

	pos.x += (nWidth / 4) - 1;

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, (nWidth / 2) + 1, GetTinyFont()->GetHeight() + 6, LTFALSE);
	AddFixedControl(pFrame, kNoGroup, pos, LTFALSE);

	pos.x += (nWidth / 2) - 1;

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, (nWidth / 4) + 1, GetTinyFont()->GetHeight() + 6, LTFALSE);
	AddFixedControl(pFrame, kNoGroup, pos, LTFALSE);

	// Create the sort selection frame
	m_pSortFrame = new CFrameCtrl;
	m_pSortFrame->Create(g_pLTClient, kWhite, 0.75f, 16, 16, LTFALSE);
	AddFixedControl(m_pSortFrame, kNoGroup, LTIntPt(0, 0), LTFALSE);


	// -----------------------------------------------------------------
	// Create the controls for this folder

	// Reset the position
	pos.y = GetPageTop() + 3;
	pos.x = GetPageLeft() + (2 * kIconWidth);

	m_pSortName = CreateTextItem(IDS_FIND_SERVERNAME, CMD_SORT_NAME, IDS_HELP_FIND_SERVERNAME, LTFALSE, GetTinyFont());
	m_pSortName->SetParam1(1);
	AddFixedControl(m_pSortName,kCustomGroup0,pos);

	m_pSortFrame->SetSize(m_pSortName->GetWidth() + 10, m_pSortName->GetHeight() + 4);
	m_pSortFrame->SetPos(LTIntPt(pos.x - 5, pos.y - 2));

	pos.x += kNameWidth;

	m_pSortPing = CreateTextItem(IDS_PING, CMD_SORT_PING, IDS_HELP_FIND_PING, LTFALSE, GetTinyFont());
	m_pSortPing->SetParam1(1);
	AddFixedControl(m_pSortPing,kCustomGroup0,pos);
	pos.x += kPingWidth;

	m_pSortNum = CreateTextItem(IDS_FIND_NUMPLAYERS, CMD_SORT_NUM, IDS_HELP_FIND_NUMPLAYERS, LTFALSE, GetTinyFont());
	m_pSortNum->SetParam1(1);
	AddFixedControl(m_pSortNum,kCustomGroup0,pos);
	pos.x += kNumWidth;

	m_pSortType = CreateTextItem(IDS_FIND_TYPE, CMD_SORT_TYPE, IDS_HELP_FIND_TYPE, LTFALSE, GetTinyFont());
	m_pSortType->SetParam1(1);
	AddFixedControl(m_pSortType,kCustomGroup0,pos);
	pos.x += kTypeWidth;

	m_pSortMap = CreateTextItem(IDS_FIND_MAP, CMD_SORT_MAP, IDS_HELP_FIND_MAP, LTFALSE, GetTinyFont());
	m_pSortMap->SetParam1(1);
	AddFixedControl(m_pSortMap,kCustomGroup0,pos);

	pos.y += GetSmallFont()->GetHeight();
	pos.x = GetPageLeft();

	m_pServerList = new CListCtrl;
    m_pServerList->Create(ptListSize.y, LTTRUE, ptListSize.x);
	m_pServerList->SetItemSpacing(0);
    m_pServerList->EnableMouseClickSelect(LTTRUE);
    m_pServerList->EnableMouseMoveSelect(LTFALSE);
    m_pServerList->SetIndent(LTIntPt(2,4));
    AddFixedControl(m_pServerList,kFreeGroup,pos,LTTRUE);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"CurrentIPPos");
	m_pIPCtrl = CreateTextItem(IDS_SPACER, LTNULL, LTNULL, LTTRUE, GetHelpFont());
	AddFixedControl(m_pIPCtrl,kNoGroup,pos,LTFALSE);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"FilterPos");
	m_pFilterTitle = CreateTextItem(IDS_FIND_FILTERS, LTNULL, LTNULL, LTTRUE, GetTinyFont());
	m_pFilterTitle->Enable(LTFALSE);
	AddFixedControl(m_pFilterTitle,kNoGroup,pos);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"PopFilterPos");
	m_pPopFilter = CreateTextItem(anPopFilter[0], CMD_FILTER_POP, IDS_HELP_FILTER_POP, LTFALSE, GetTinyFont(), &m_nPopFilter);
	m_pPopFilter->AddString(anPopFilter[1]);
	m_pPopFilter->AddString(anPopFilter[2]);
	m_pPopFilter->AddString(anPopFilter[3]);
	m_pPopFilter->SetParam1(1);
	AddFixedControl(m_pPopFilter,kCustomGroup1,pos);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"VersionFilterPos");
	m_pVersionFilter = CreateTextItem(IDS_FILTER_ANYVERSION, CMD_FILTER_VERSION, IDS_HELP_FILTER_VERSION, LTFALSE, GetTinyFont(), &m_nVersionFilter);

	HSTRING hTemp = g_pLTClient->FormatString(IDS_VERSION);
	HSTRING hTemp2 = g_pLTClient->FormatString(IDS_FILTER_VERSION, g_pLTClient->GetStringData(hTemp));
	g_pLTClient->FreeString(hTemp);

	m_pVersionFilter->AddString(hTemp2);
	g_pLTClient->FreeString(hTemp2);
	m_pVersionFilter->SetParam1(1);
	AddFixedControl(m_pVersionFilter,kCustomGroup1,pos);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"TypeFilterPos");
	m_pTypeFilter = CreateTextItem(anTypeFilter[0], CMD_FILTER_TYPE, IDS_HELP_FILTER_TYPE, LTFALSE, GetTinyFont(), &m_nTypeFilter);

	for(i = 1; i < kNumTypeFilters; i++)
		m_pTypeFilter->AddString(anTypeFilter[i]);

	m_pTypeFilter->SetParam1(1);
	AddFixedControl(m_pTypeFilter,kCustomGroup1,pos);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"PingFilterPos");
	m_pPingFilter = CreateTextItem(IDS_FILTER_ANYPING, CMD_FILTER_PING, IDS_HELP_FILTER_PING, LTFALSE, GetTinyFont(), &m_nPingFilter);

	for(i = 1; i < kNumPingFilters; i++)
		m_pPingFilter->AddString(anPingFilter[i]);

	m_pPingFilter->SetParam1(1);
	AddFixedControl(m_pPingFilter,kCustomGroup1,pos);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"DedicatedFilterPos");
	m_pDedicatedFilter = CreateTextItem(IDS_DEDICATED_ANYTYPE, CMD_FILTER_DEDICATED, IDS_HELP_FILTER_DEDICATED, LTFALSE, GetTinyFont(), &m_nDedicatedFilter);

	for(i = 1; i < kNumDedicatedFilters; i++)
		m_pDedicatedFilter->AddString(anDedicatedFilter[i]);

	AddFixedControl(m_pDedicatedFilter,kCustomGroup1,pos);


	// join ip button	
	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"JoinIPPos");
	m_pJoinIP = CreateTextItem(IDS_JOIN_IP, CMD_JOIN_IP, IDS_HELP_JOIN_IP, LTFALSE, GetTinyFont());
	AddFixedControl(m_pJoinIP,kLinkGroup,pos);

	BuildSpecificIPDlg();

	// refresh button	
	CBitmapCtrl *pBMPCtrl = new CBitmapCtrl;
    pBMPCtrl->Create(g_pLTClient,"interface\\menus\\refresh.pcx","interface\\menus\\refresh_h.pcx","interface\\menus\\refresh_d.pcx", this, CMD_REFRESH);
	pBMPCtrl->SetHelpID(IDS_HELP_REFRESH);
	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"RefreshPos");
	AddFixedControl(pBMPCtrl,kLinkGroup,pos);


//	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"RefreshPos");
//	pCtrl = CreateTextItem(IDS_REFRESH, CMD_REFRESH, IDS_HELP_REFRESH, LTFALSE, GetSmallFont());
//	AddFixedControl(pCtrl,kLinkGroup,pos);

	// Create the Join button
	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"JoinPos");
	pCtrl = CreateTextItem(IDS_JOIN, FOLDER_CMD_MP_JOIN_GAME, IDS_HELP_JOIN, LTFALSE, GetLargeFont());
	AddFixedControl(pCtrl,kLinkGroup,pos);


	return LTTRUE;
}

void CFolderJoin::Escape()
{
	if(GetGameSpyMgr()->GetState() != sl_idle)
	{
		GetGameSpyMgr()->KillGetServersOp();
		return;
	}

	if(m_nState != FSS_IDLE)
		return;

	if(GetCapture() == m_pPWPassword)
	{
		HidePasswordDlg();
		return;
	}
	if (GetCapture() == m_pJSIPIPEdit || GetCapture() == m_pJSIPPortEdit || GetCapture() == m_pJSIPPassword )
	{
		HideSpecificIPDlg();
	}
	else
	{
		m_bEscape = LTTRUE;

		g_pInterfaceMgr->PlayInterfaceSound(IS_ESCAPE);
		g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_MULTI);
	}
}

uint32 CFolderJoin::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
		case FOLDER_CMD_MULTIPLAYER:
		{
			m_bEscape = LTTRUE;
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_MULTI);
			break;
		}

		case CMD_REFRESH:
		{
			RefreshServers();
			break;
		}

		case CMD_JOIN_IP:
		{
			ShowSpecificIPDlg();
			break;
		}

		case CMD_EDIT_IP:
		{
			if (GetCapture() == m_pJSIPIPEdit)
			{
				m_pJSIPIPEdit->SetColor(m_hNormalColor,m_hNormalColor,m_hNormalColor);
				m_pJSIPIPEdit->Select(LTFALSE);

				strcpy(szOldStr,m_szPort);
				SetCapture(m_pJSIPPortEdit);

				m_pJSIPPortEdit->SetColor(m_hSelectedColor,m_hSelectedColor,m_hSelectedColor);
				m_pJSIPPortEdit->Select(LTTRUE);
			}
			break;
		}

		case CMD_EDIT_PORT:
		{
			if (GetCapture() == m_pJSIPPortEdit)
			{
				m_pJSIPPortEdit->SetColor(m_hNormalColor,m_hNormalColor,m_hNormalColor);
				m_pJSIPPortEdit->Select(LTFALSE);

				strcpy(szOldStr,m_szPassword);
				SetCapture(m_pJSIPPassword);

				m_pJSIPPassword->SetColor(m_hSelectedColor,m_hSelectedColor,m_hSelectedColor);
				m_pJSIPPassword->Select(LTTRUE);
			}
			break;
		}

		case CMD_EDIT_PASSWORD:
		{
			if (GetCapture() == m_pJSIPPassword)
			{
				HideSpecificIPDlg();
				JoinSpecificIP();
			}
			break;
		}

		case CMD_SORT_NAME:
		case CMD_SORT_PING:
		case CMD_SORT_NUM:
		case CMD_SORT_TYPE:
		case CMD_SORT_MAP:
		{
            m_bNeedServerSorting = LTTRUE;
			m_nServerSort = (int)dwCommand;
            ForceNextUpdate();
			
			break;
		}

		case CMD_FILTER_POP:
		{
			m_nPopFilter++;
			if (m_nPopFilter >= kNumPopFilters)
				m_nPopFilter = 0;
			m_pPopFilter->UpdateData(LTFALSE);
			ForceNextUpdate();
			break;
		}

		case CMD_FILTER_VERSION:
		{
			m_nVersionFilter++;
			if (m_nVersionFilter > 1)
				m_nVersionFilter = 0;
			m_pVersionFilter->UpdateData(LTFALSE);
			ForceNextUpdate();
			break;
		}

		case CMD_FILTER_TYPE:
		{
			m_nTypeFilter++;
			if (m_nTypeFilter >= kNumTypeFilters)
				m_nTypeFilter = 0;

			m_pTypeFilter->UpdateData(LTFALSE);
			ForceNextUpdate();
			break;
		}

		case CMD_FILTER_PING:
		{
			m_nPingFilter++;
			if (m_nPingFilter >= kNumPingFilters)
				m_nPingFilter = 0;
			m_pPingFilter->UpdateData(LTFALSE);
			ForceNextUpdate();
			break;
		}

		case CMD_FILTER_DEDICATED:
		{
			m_nDedicatedFilter++;
			if (m_nDedicatedFilter >= kNumDedicatedFilters)
				m_nDedicatedFilter = 0;
			m_pDedicatedFilter->UpdateData(LTFALSE);
			ForceNextUpdate();
			break;
		}

		default:
			return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}

	return 1;
}

void CFolderJoin::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		if (m_bLANOnly)
			CreateTitle(IDS_TITLE_JOIN_LAN);
		else
			CreateTitle(IDS_TITLE_JOIN);


		m_bEscape = LTFALSE;

		if(m_bRefreshOnFocus)
		{
			m_nNumServers       = 0;
			m_nNumServersListed = 0;

			m_nPopFilter = GetConsoleInt("JoinPopFilter",0);
			m_nVersionFilter = GetConsoleInt("JoinVersionFilter",1);
			m_nTypeFilter = GetConsoleInt("JoinTypeFilter",0);
			m_nPingFilter = GetConsoleInt("JoinPingFilter",3);
			m_nDedicatedFilter = GetConsoleInt("JoinDedicatedFilter",0);

			m_nServerSort = GetConsoleInt("JoinSort",CMD_SORT_PING);

			nOldIndex = -1;
			nOldSelection = -1;
			timeLastFullUpdate = 0;


			HSTRING hStr = LTNULL;
			if (m_bLANOnly)
				hStr = g_pLTClient->FormatString(IDS_STATUS_SEARCH_LAN);
			else
				hStr = g_pLTClient->FormatString(IDS_STATUS_CONNECTING);

			SetUpdateControls(LTFALSE);
			SetDummyStatusState(hStr, 500, FSS_GETSERVICES);

			g_pLTClient->FreeString(hStr);
		}

		UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		if (m_bEscape)
		{
			SetState(FSS_IDLE);
		
			WriteConsoleInt("JoinPopFilter",m_nPopFilter);
			WriteConsoleInt("JoinVersionFilter",m_nVersionFilter);
			WriteConsoleInt("JoinTypeFilter",m_nTypeFilter );
			WriteConsoleInt("JoinPingFilter",m_nPingFilter);
			WriteConsoleInt("JoinDedicatedFilter",m_nDedicatedFilter);
			WriteConsoleInt("JoinSort",m_nServerSort);
			
			m_pServerList->RemoveAllControls();
		}
	}
	CBaseFolder::OnFocus(bFocus);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::UpdateServers
//
//	PURPOSE:	Updates the servers
//
// ----------------------------------------------------------------------- //

void CFolderJoin::UpdateServers(LTBOOL bForce)
{
	// Check for a forced update...

	if (m_bForceUpdate)
	{
        bForce             = LTTRUE;
        m_bForceUpdate = LTFALSE;
	}


	// Check if we got a new server...

	static LTBOOL bDelayNewServer = LTFALSE;

	LTBOOL bNewServer = LTFALSE;

	if (GetGameSpyMgr()->ExistNewServer())
	{
//		g_pLTClient->CPrint("Found new server.");
		bNewServer = LTTRUE;
	}

	if (bDelayNewServer)
	{
		bNewServer = LTTRUE;
	}


	// Make sure we don't do a full update too often...


	if (!bForce && bNewServer)
	{
		if ((GetTickCount() - timeLastFullUpdate) < 1000)
		{
			bDelayNewServer = LTTRUE;
		}
		else
		{
			bForce = LTTRUE;
		}
	}

	if (bForce)
	{
		bDelayNewServer    = LTFALSE;
	}
	else
		return;

//	if (timeLastFullUpdate)
//	{
//		uint32 elapsed = GetTickCount() - timeLastFullUpdate;
//		g_pLTClient->CPrint("Time between updates: %d",elapsed);
//	}

	timeLastFullUpdate = GetTickCount();

	// Rebuild the entire list...

	// Get the currently selected game server...
	int nIndex = m_pServerList->GetSelectedItem();
    void* pHandle = LTNULL;

	if((nIndex >= 0) && (nIndex < CListCtrl::kNoSelection))
	{
		pHandle = (void*)m_pServerList->GetControl(nIndex)->GetParam1();
	}

	m_pServerList->RemoveAllControls();

    CGameSpyServer* pGame = GetGameSpyMgr()->GetFirstServer();

	int i = 0;
	int nDeadServers = 0;

	while (pGame && pGame->IsInitialized())
	{
		// Add a new control for this game service...

		// Check our filters...

		LTBOOL bPass = LTTRUE;

		//filter for dead servers
		if (0 == pGame->GetMaxPlayers())
		{
			nDeadServers++;
			bPass = LTFALSE;
		}


		if (m_nVersionFilter)
		{
			char* sGameVer = pGame->GetStringValue("gamever");

			HSTRING hTemp = g_pLTClient->FormatString(IDS_VERSION);

			if (sGameVer && stricmp(sGameVer, g_pLTClient->GetStringData(hTemp))!=0)
				bPass = LTFALSE;

			g_pLTClient->FreeString(hTemp);
		}

		if (bPass && m_nTypeFilter)
		{
			char* sType = pGame->GetGameType();
			int   nType = 0;
			while (nType < g_nDescGameTypes && (stricmp(sType, g_pDescGameTypes[nType]) != 0))
				nType++;


			switch(anTypeFilter[m_nTypeFilter])
			{
				case IDS_FILTER_DM:			if(nType != MP_DM) bPass = LTFALSE;			break;
				case IDS_FILTER_TEAMDM:		if(nType != MP_TEAMDM) bPass = LTFALSE;		break;
				case IDS_FILTER_HUNT:		if(nType != MP_HUNT) bPass = LTFALSE;		break;
				case IDS_FILTER_SURVIVOR:	if(nType != MP_SURVIVOR) bPass = LTFALSE;	break;
				case IDS_FILTER_OVERRUN:	if(nType != MP_OVERRUN) bPass = LTFALSE;	break;
				case IDS_FILTER_EVAC:		if(nType != MP_EVAC) bPass = LTFALSE;		break;
			}
		}

		if (bPass && m_nPopFilter)
		{
			int nPlrs = pGame->GetNumPlayers();
			int nMax  = pGame->GetMaxPlayers();

			switch (anPopFilter[m_nPopFilter])
			{
                case IDS_POP_NOTEMPTY: if (nPlrs == 0) bPass = LTFALSE; break;
                case IDS_POP_NOTFULL:  if (nPlrs >= nMax) bPass = LTFALSE; break;
                case IDS_POP_NEITHER:  if (nPlrs == 0 || nPlrs >= nMax) bPass = LTFALSE; break;
			}
		}

		if (bPass && m_nDedicatedFilter)
		{
			LTBOOL bDedicated = pGame->GetDedicated();

			switch (anDedicatedFilter[m_nDedicatedFilter])
			{
				case IDS_DEDICATED_ANYTYPE:		break;
				case IDS_DEDICATED_YES:			if (!bDedicated) bPass = LTFALSE; break;
				case IDS_DEDICATED_NO:			if (bDedicated) bPass = LTFALSE; break;
			}
		}

		if (bPass)
		{
			if (pGame->GetPing() >= anPingFilterNum[m_nPingFilter])
				bPass = LTFALSE;
		}


		if (!bPass)
		{
            pGame = GetGameSpyMgr()->GetNextServer();
			continue;
		}

		AddServerCtrl(pGame);

		// Get the next game service...

        pGame = GetGameSpyMgr()->GetNextServer();
	}

	i = 0;
	while (i < m_pServerList->GetNum() && pHandle != (void*)m_pServerList->GetControl(i)->GetParam1())
		i++;

	if (i < m_pServerList->GetNum())
		m_pServerList->SelectItem(i);



	m_nNumServers = GetGameSpyMgr()->GetNumServers() - nDeadServers;
	m_nNumServersListed = m_pServerList->GetNum();
}


CGameSpyServer* CFolderJoin::GetSelectedServer()
{
	// Get the currently selected game server...
	int nIndex = m_pServerList->GetSelectedItem();
    void* pHandle = LTNULL;
	if (nIndex != CListCtrl::kNoSelection)
	{
		pHandle = (void*)m_pServerList->GetControl(nIndex)->GetParam1();
	}


	CGameSpyServer* pGame = GetGameSpyMgr()->GetServerFromHandle(pHandle);

	return pGame;
}


void CFolderJoin::JoinServer(CGameSpyServer *pGame, const char *szPassword)
{
    if (!pGame) return;


	LTRESULT result = g_pGameClientShell->JoinMultiplayerGame(pGame, szPassword);

	if(result != LT_OK)
	{
		HSTRING hString = LTNULL;

		switch(result)
		{
			case MP_JOIN_ERROR_INVALID_SERVER:
				hString = g_pLTClient->FormatString(IDS_JOIN_ERROR_1);
				break;

			case MP_JOIN_ERROR_GAME_FULL:
				hString = g_pLTClient->FormatString(IDS_JOIN_ERROR_2);
				break;

			case MP_JOIN_ERROR_STARTGAME_FAILED:
				hString = g_pLTClient->FormatString(IDS_JOIN_ERROR_3);
				break;

			case MP_JOIN_ERROR_WRONG_VERSION:
				hString = g_pLTClient->FormatString(IDS_JOIN_ERROR_4);
				break;

			case MP_JOIN_ERROR_UNKNOWN:
			default:
				hString = g_pLTClient->FormatString(IDS_JOIN_ERROR_5);
				break;
		}

		g_pInterfaceMgr->ShowMessageBox(hString, LTMB_OK, LTNULL, LTNULL);
		g_pLTClient->FreeString(hString);
	}
}


void CFolderJoin::JoinSpecificIP()
{
	uint16 nPort = atoi(m_szPort);
	if (!nPort) return;

	// Look for game server?
	LTRESULT result = g_pGameClientShell->JoinMultiplayerGameAtIP(m_szIP, nPort, m_szPassword);

	if(result != LT_OK)
	{
		HSTRING hString = LTNULL;

		switch(result)
		{
			case MP_JOIN_ERROR_INVALID_SERVER:
				hString = g_pLTClient->FormatString(IDS_JOIN_ERROR_1);
				break;

			case MP_JOIN_ERROR_STARTGAME_FAILED:
				hString = g_pLTClient->FormatString(IDS_JOIN_ERROR_3);
				break;

			case MP_JOIN_ERROR_UNKNOWN:
			default:
				hString = g_pLTClient->FormatString(IDS_JOIN_ERROR_4);
				break;
		}

		g_pInterfaceMgr->ShowMessageBox(hString, LTMB_OK, LTNULL, LTNULL);
		g_pLTClient->FreeString(hString);
	}
}


void CFolderJoin::AddServerCtrl(CGameSpyServer* pGame)
{
	CLTGUIColumnTextCtrl* pColCtrl = CreateColumnText(FOLDER_CMD_MP_JOIN_GAME, LTNULL, LTFALSE, GetTinyFont());
	pColCtrl->SetParam1((uint32)pGame->GetHandle());
	pColCtrl->SetParam2((uint32)pGame->GetIntValue("lock"));

	char szTemp[128];

	// Create the unknown string
	HSTRING hUnknown = g_pLTClient->FormatString(IDS_UNKNOWN);

	//add dummy columns for icons
	HSTRING hText = g_pLTClient->CreateString(" ");
	pColCtrl->AddColumn(hText, kIconWidth, LTF_JUSTIFY_CENTER);
	pColCtrl->AddColumn(hText, kIconWidth, LTF_JUSTIFY_CENTER);
	g_pLTClient->FreeString(hText);

	// mission name
	sprintf(szTemp,"%s",pGame->GetName());
	szTemp[30] = '\0';

	if(!szTemp[0])
		sprintf(szTemp,"%s",g_pLTClient->GetStringData(hUnknown));

	hText = g_pLTClient->CreateString(szTemp);
	pColCtrl->AddColumn(hText, kNameWidth, LTF_JUSTIFY_LEFT);
	g_pLTClient->FreeString(hText);

	// ping
	if(pGame->GetPing() >= 9999)
		strcpy(szTemp, "???");
	else
		sprintf(szTemp,"%d",pGame->GetPing());

	hText = g_pLTClient->CreateString(szTemp);
	pColCtrl->AddColumn(hText, kPingWidth, LTF_JUSTIFY_CENTER);
	g_pLTClient->FreeString(hText);

	// players
	sprintf(szTemp,"%d/%d",pGame->GetNumPlayers(), pGame->GetMaxPlayers());
	hText = g_pLTClient->CreateString(szTemp);
	pColCtrl->AddColumn(hText, kNumWidth, LTF_JUSTIFY_CENTER);
	g_pLTClient->FreeString(hText);

	// type
	sprintf(szTemp,"%s",pGame->GetGameType());

	if(!szTemp[0])
		sprintf(szTemp,"%s",g_pLTClient->GetStringData(hUnknown));

	hText = g_pLTClient->CreateString(szTemp);
	pColCtrl->AddColumn(hText, kTypeWidth, LTF_JUSTIFY_LEFT);
	g_pLTClient->FreeString(hText);
	
	// map
	sprintf(szTemp,"%s",pGame->GetMap());

	if(!szTemp[0])
		sprintf(szTemp,"%s",g_pLTClient->GetStringData(hUnknown));

	hText = g_pLTClient->CreateString(szTemp);
	pColCtrl->AddColumn(hText, kMapWidth, LTF_JUSTIFY_LEFT);
	g_pLTClient->FreeString(hText);
	m_pServerList->AddControl(pColCtrl);


	g_pLTClient->FreeString(hUnknown);
}


LTBOOL CFolderJoin::Render(HSURFACE hDestSurf)
{
    LTBOOL bOK = CBaseFolder::Render(hDestSurf);


	// There's no update pump, so we'll have to do it here...

	if (GetGameSpyMgr()->IsInitialized())
	{
		Update(hDestSurf);
	}
	else
	{
		UpdateDummyStatus(hDestSurf);
	}

	int xo = g_pInterfaceResMgr->GetXOffset();
	int yo = g_pInterfaceResMgr->GetYOffset();

//	if (m_pServerList->GetLastDisplayedIndex() >= 0)
//	{
		if (m_pServerList->GetNum() != nOldIndex)
		{
			if (m_hServersShown)
			{
				g_pLTClient->FreeString(m_hServersShown);
				m_hServersShown = LTNULL;
			}

			if (m_nNumServers)
			{
				m_hServersShown = g_pLTClient->FormatString(IDS_NUM_SHOWN, m_nNumServersListed, m_nNumServers);
			}

			nOldIndex = m_pServerList->GetNum();
		}

		if(m_hServersShown)
		{
			GetTinyFont()->DrawMenu(m_hServersShown, hDestSurf, xo+ptCountPos.x, yo+ptCountPos.y, LTF_JUSTIFY_LEFT, LGCS_SELECTED);
		}
//	}

	if (m_hStatus)
	{
		if (m_nState == FSS_IDLE)
			GetTinyFont()->DrawMenu(m_hStatus, hDestSurf, xo+ptStatusPos.x, yo+ptStatusPos.y, LTF_JUSTIFY_CENTER, LGCS_DISABLED);
		else
			GetTinyFont()->DrawMenu(m_hStatus, hDestSurf, xo+ptStatusPos.x, yo+ptStatusPos.y, LTF_JUSTIFY_CENTER, LGCS_SELECTED);
	}


	// Render the lock graphics into place...
	int nStart = m_pServerList->GetStartIndex();
	int nEnd = m_pServerList->GetLastDisplayedIndex();
	CLTGUICtrl *pCtrl;
	LTIntPt pt;
	int nOffset = (kIconWidth / 2);

	for(int i = nStart; i <= nEnd; i++)
	{
		pCtrl = (CLTGUICtrl*)m_pServerList->GetControl(i);

		if(pCtrl->GetParam2())
		{
			pt = pCtrl->GetPos();

			g_pLTClient->DrawSurfaceToSurface(hDestSurf, m_hLock, LTNULL, pt.x + nOffset, pt.y);
		}
	}


	return bOK;
}


void CFolderJoin::Update(HSURFACE hDestSurf)
{
	// Let the GameSpy client manager update...
	GetGameSpyMgr()->Update();


	// Update based on our current state...
	switch (m_nState)
	{
		case FSS_IDLE:
		{
			UpdateIdle(hDestSurf);
			break;
		}

		case FSS_GETSERVICES:
		{
			UpdateGetServices(hDestSurf);
			break;
		}

		case FSS_GETPINGS:
		{
			UpdateGetPings(hDestSurf);
			break;
		}

		case FSS_DUMMYSTATUS:
		{
			UpdateDummyStatus(hDestSurf);
			break;
		}
	}

	if (m_pServerList->GetSelectedItem() != nOldSelection)
	{
		UpdateIPText();
		nOldSelection = m_pServerList->GetSelectedItem();
	}


	// Check for required sorting...

	if (m_bNeedServerSorting)
	{
        m_bNeedServerSorting = LTFALSE;
		SortServers();
	}

	UpdateServers(LTFALSE);


	// Enable or disable the join control as necessary...

//    m_pJoin->Enable((GetCurGameServerHandle() != LTNULL));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::UpdateDummyStatus
//
//	PURPOSE:	Updates the FSS_DUMMYSTATUS state
//
// ----------------------------------------------------------------------- //

void CFolderJoin::UpdateDummyStatus(HSURFACE hDestSurf)
{
	// Set our dummy status text...

	SetStatusText(m_hDummyStatus);


	// Check for ending...

	if (GetTickCount() >= m_timeDummyEnd)
	{
		SetState(m_nNextDummyState);
		return;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::UpdateGetServices
//
//	PURPOSE:	Updates the FSS_GETSERVICES state
//
// ----------------------------------------------------------------------- //

void CFolderJoin::UpdateGetServices(HSURFACE hDestSurf)
{
	int nState = GetGameSpyMgr()->GetState();


	switch (nState)
	{
		case sl_idle:		// done updating
		{
			SortServers();
			m_bNeedServerSorting = FALSE;
			HSTRING hStr = g_pLTClient->FormatString(IDS_STATUS_QUERYDONE);
			SetDummyStatusState(hStr, 1000, FSS_IDLE);
			g_pLTClient->FreeString(hStr);

			SetUpdateControls(LTTRUE);
			break;
		}

		case sl_listxfer:	// getting list
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_STATUS_GETLIST);
			SetStatusText(hStr);
			g_pLTClient->FreeString(hStr);
			break;
		}

		case sl_lanlist:	// searching lan
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_STATUS_GETLIST);
			SetStatusText(hStr);
			g_pLTClient->FreeString(hStr);
			break;
		}

		case sl_querying:	// querying servers
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_STATUS_QUERY, GetGameSpyMgr()->GetProgress(), m_nNumServersListed);
			SetStatusText(hStr);
			g_pLTClient->FreeString(hStr);
			break;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::UpdateGetPings
//
//	PURPOSE:	Updates the FSS_GETPINGS state
//
// ----------------------------------------------------------------------- //

void CFolderJoin::UpdateGetPings(HSURFACE hDestSurf)
{
	HSTRING hStr = g_pLTClient->FormatString(IDS_STATUS_PINGDONE, m_nNumServersListed);
	SetDummyStatusState(hStr, 1000, FSS_IDLE);
	g_pLTClient->FreeString(hStr);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::UpdateIdle
//
//	PURPOSE:	Updates the FSS_IDLE state
//
// ----------------------------------------------------------------------- //

void CFolderJoin::UpdateIdle(HSURFACE hDestSurf)
{
	// If we're still getting the server list, go back to the proper state...

	if (GetGameSpyMgr()->GetState() != sl_idle)
	{
		UpdateGetServices(hDestSurf);
		return;
	}


	// Draw the status info...

	HSTRING hStr= LTNULL;

	if (m_nNumServers == 0)
	{
		hStr = g_pLTClient->FormatString(IDS_STATUS_IDLENONE);
	}
	else
	{
		hStr = g_pLTClient->FormatString(IDS_STATUS_IDLE, m_nNumServersListed);
	}

	SetStatusText(hStr);
	g_pLTClient->FreeString(hStr);
}



void CFolderJoin::SetStatusText(HSTRING hStr)
{

	if (m_hStatus)
	{
		g_pLTClient->FreeString(m_hStatus);
		m_hStatus = LTNULL;
	}
	m_hStatus = g_pLTClient->CopyString(hStr);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::SetDummyStatusState
//
//	PURPOSE:	Sets the dummy status state for just displing status
//				text for a short amount of time.  Otherwise, some status
//				messages go by too quickly to read!
//
// ----------------------------------------------------------------------- //

void CFolderJoin::SetDummyStatusState(HSTRING hStr, uint32 dwWaitTime, int nNextState)
{
	// Set dummy status values...

	m_timeDummyEnd    = GetTickCount() + dwWaitTime;
	m_nNextDummyState = nNextState;

	if (m_hDummyStatus)
	{
		g_pLTClient->FreeString(m_hDummyStatus);
		m_hDummyStatus = LTNULL;
	}
	m_hDummyStatus = g_pLTClient->CopyString(hStr);

	// Set the state...

	SetState(FSS_DUMMYSTATUS);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::SortServers
//
//	PURPOSE:	Sorts the servers
//
// ----------------------------------------------------------------------- //

void CFolderJoin::SortServers()
{
	// Set our sort dir based on the sort type...

	switch (m_nServerSort)
	{
		case CMD_SORT_NAME:
		{
			LTIntPt pos = m_pSortName->GetPos();
			m_pSortFrame->SetPos(LTIntPt(pos.x - 5, pos.y - 2));
			m_pSortFrame->SetSize(m_pSortName->GetWidth() + 10, m_pSortName->GetHeight() + 4);

			GetGameSpyMgr()->SortServersByName();
			break;
		}

		case CMD_SORT_PING:
		{
			LTIntPt pos = m_pSortPing->GetPos();
			m_pSortFrame->SetPos(LTIntPt(pos.x - 5, pos.y - 2));
			m_pSortFrame->SetSize(m_pSortPing->GetWidth() + 10, m_pSortPing->GetHeight() + 4);

			GetGameSpyMgr()->SortServersByPing();
			break;
		}

		case CMD_SORT_NUM:
		{
			LTIntPt pos = m_pSortNum->GetPos();
			m_pSortFrame->SetPos(LTIntPt(pos.x - 5, pos.y - 2));
			m_pSortFrame->SetSize(m_pSortNum->GetWidth() + 10, m_pSortNum->GetHeight() + 4);

			GetGameSpyMgr()->SortServersByPlayers();
			break;
		}

		case CMD_SORT_TYPE:
		{
			LTIntPt pos = m_pSortType->GetPos();
			m_pSortFrame->SetPos(LTIntPt(pos.x - 5, pos.y - 2));
			m_pSortFrame->SetSize(m_pSortType->GetWidth() + 10, m_pSortType->GetHeight() + 4);

			GetGameSpyMgr()->SortServersByGameType();
			break;
		}

		case CMD_SORT_MAP:
		{
			LTIntPt pos = m_pSortMap->GetPos();
			m_pSortFrame->SetPos(LTIntPt(pos.x - 5, pos.y - 2));
			m_pSortFrame->SetSize(m_pSortMap->GetWidth() + 10, m_pSortMap->GetHeight() + 4);

			GetGameSpyMgr()->SortServersByMap();
			break;
		}
	}


	// Force the next update...

	ForceNextUpdate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::SetState
//
//	PURPOSE:	Sets the server finding state
//
// ----------------------------------------------------------------------- //

void CFolderJoin::SetState(int nNewState)
{
	// Set the new state accordingly...

	switch (nNewState)
	{
		case FSS_GETSERVICES:
		{
			SetUpdateControls(LTFALSE);

//			SetCurGameServerHandle(LTNULL);
            m_bNeedServerSorting = LTTRUE;

			if (m_bLANOnly)
				GetGameSpyMgr()->RefreshLANServers();
			else
				GetGameSpyMgr()->RefreshServers(LTTRUE);

			// ALM (5/1/01) Having this line here was causing a crash when you are already
			// joined in a multiplayer game, and then return to this folder.
//			UpdateIPText();

			ForceNextUpdate();

			break;
		}

		case FSS_GETPINGS:
		{
			SetUpdateControls(LTFALSE);
            ForceNextUpdate();
			break;
		}

		case FSS_IDLE:
		{
			SetUpdateControls(LTTRUE);
			break;
		}

		case FSS_DUMMYSTATUS:
		{
			break;
		}

		default:
		{
			return;
		}
	}

	// Set the new state value...

	m_nState = nNewState;
}


LTBOOL	CFolderJoin::OnLeft()
{
	if(m_nState != FSS_IDLE)
		return LTFALSE;

	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->GetParam1())
		return CBaseFolder::OnUp();

	return CBaseFolder::OnLeft();
}

LTBOOL	CFolderJoin::OnRight()
{
	if(m_nState != FSS_IDLE)
		return LTFALSE;

	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->GetParam1())
		return CBaseFolder::OnDown();

	return CBaseFolder::OnRight();
}

LTBOOL	CFolderJoin::OnPageUp()
{
	if(m_nState != FSS_IDLE)
		return LTFALSE;

	if(m_pServerList)
		m_pServerList->ScrollUp();

	return CBaseFolder::OnPageUp();
}

LTBOOL	CFolderJoin::OnPageDown()
{
	if(m_nState != FSS_IDLE)
		return LTFALSE;

	if(m_pServerList)
		m_pServerList->ScrollDown();

	return CBaseFolder::OnPageDown();
}

LTBOOL CFolderJoin::OnMouseWheel(int x, int y, int delta)
{
	if(m_nState != FSS_IDLE)
		return LTFALSE;

	if(m_pServerList)
	{
		if(delta > 0)
			m_pServerList->PreviousSelection();
		else
			m_pServerList->NextSelection();
	}

	return CBaseFolder::OnMouseWheel(x, y, delta);
}


LTBOOL CFolderJoin::HandleKeyDown(int key, int rep)
{
	if(m_nState != FSS_IDLE)
		return LTFALSE;

	if (GetCapture() && VK_TAB == key)
	{
		return GetCapture()->OnEnter();
	}

	if (VK_F5 == key)
	{
		RefreshServers();
	}

	return CBaseFolder::HandleKeyDown(key,rep);
}


void CFolderJoin::BuildSpecificIPDlg()
{
	LTIntPt	pt(0, 0);

	ptDlgJSIPSize		= g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DlgJSIPSize");
	ptDlgJSIPEditIP		= g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DlgJSIPEditIP");
	ptDlgJSIPEditPort	= g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DlgJSIPEditPort");
	ptDlgJSIPEditPW		= g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DlgJSIPEditPW");
	ptDlgJSIPOffset		= g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "DlgJSIPOffset");


	// Create the group that will contain all the appropriate controls
	m_pJSIPDlgGroup = CreateGroup(ptDlgJSIPSize.x, ptDlgJSIPSize.y, LTNULL);
	m_pJSIPDlgGroup->AllowSubSelection(LTTRUE);
	m_pJSIPDlgGroup->Enable(LTTRUE);


	// Create the frame of the dialog
	CFrameCtrl *pFrame;

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(10,10,10), 0.75f, ptDlgJSIPSize.x, ptDlgJSIPSize.y);
	m_pJSIPDlgGroup->AddControl(pFrame, pt);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, ptDlgJSIPSize.x, ptDlgJSIPSize.y, LTFALSE);
	m_pJSIPDlgGroup->AddControl(pFrame, pt);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, ptDlgJSIPSize.x, GetHelpFont()->GetHeight() + 6);
	m_pJSIPDlgGroup->AddControl(pFrame, pt);


	// Now create the title for the dialog
	m_pJSIPTitle = CreateStaticTextItem(IDS_JOIN_IP_DIALOG, LTNULL, LTNULL, ptDlgJSIPSize.x, ptDlgJSIPSize.y, LTTRUE, GetHelpFont());
	m_pJSIPDlgGroup->AddControl(m_pJSIPTitle, LTIntPt(3, 3));


	// Create the controls for entering the IP address
	CLTGUITextItemCtrl*	pCtrl = CreateTextItem(IDS_INFO_IP, LTNULL, LTNULL, LTTRUE);
	m_pJSIPDlgGroup->AddControl(pCtrl, ptDlgJSIPEditIP, LTFALSE);

	m_pJSIPIPEdit = CreateEditCtrl(" ", CMD_EDIT_IP, LTNULL, m_szIP, sizeof(m_szIP), 0, LTTRUE);
	m_pJSIPIPEdit->EnableCursor();
    m_pJSIPIPEdit->Enable(LTFALSE);
	m_pJSIPDlgGroup->AddControl(m_pJSIPIPEdit, LTIntPt(ptDlgJSIPEditIP.x + ptDlgJSIPOffset, ptDlgJSIPEditIP.y), LTTRUE);


	// Create the controls for entering the port
	pCtrl = CreateTextItem(IDS_HOST_PORT, LTNULL, LTNULL, LTTRUE);
	m_pJSIPDlgGroup->AddControl(pCtrl, ptDlgJSIPEditPort, LTFALSE);

	m_pJSIPPortEdit = CreateEditCtrl(" ", CMD_EDIT_PORT, LTNULL, m_szPort, sizeof(m_szPort), 0, LTTRUE);
	m_pJSIPPortEdit->EnableCursor();
    m_pJSIPPortEdit->Enable(LTFALSE);
	m_pJSIPPortEdit->SetInputMode(CLTGUIEditCtrl::kInputNumberOnly);
	m_pJSIPDlgGroup->AddControl(m_pJSIPPortEdit, LTIntPt(ptDlgJSIPEditPort.x + ptDlgJSIPOffset, ptDlgJSIPEditPort.y), LTTRUE);


	// Create the controls for entering the password
	pCtrl = CreateTextItem(IDS_JOIN_PASSWORD, LTNULL, LTNULL, LTTRUE);
	m_pJSIPDlgGroup->AddControl(pCtrl, ptDlgJSIPEditPW, LTFALSE);

	m_pJSIPPassword = CreateEditCtrl(" ", CMD_EDIT_PASSWORD, LTNULL, m_szPassword, sizeof(m_szPassword), 0, LTTRUE);
	m_pJSIPPassword->EnableCursor();
    m_pJSIPPassword->Enable(LTFALSE);
	m_pJSIPDlgGroup->AddControl(m_pJSIPPassword, LTIntPt(ptDlgJSIPEditPW.x + ptDlgJSIPOffset, ptDlgJSIPEditPW.y), LTTRUE);
}

void CFolderJoin::ShowSpecificIPDlg()
{
	m_pJSIPIPEdit->UpdateData(LTFALSE);
	m_pJSIPPortEdit->UpdateData(LTFALSE);
	m_pJSIPPassword->UpdateData(LTFALSE);

	LTIntPt pos(0,0);

	pos.x = 320 - ptDlgJSIPSize.x / 2;
	pos.y = 240 - ptDlgJSIPSize.y / 2;

	AddFixedControl(m_pJSIPDlgGroup, kFreeGroup, pos, LTTRUE);

	SetSelection(GetIndex(m_pJSIPDlgGroup));

	SetCapture(m_pJSIPIPEdit);
	m_pJSIPIPEdit->SetColor(m_hSelectedColor, m_hSelectedColor, m_hSelectedColor);
	m_pJSIPIPEdit->Select(LTTRUE);
}

void CFolderJoin::HideSpecificIPDlg()
{
	m_pJSIPIPEdit->UpdateData(LTTRUE);
	m_pJSIPPortEdit->UpdateData(LTTRUE);
	m_pJSIPPassword->UpdateData(LTTRUE);

	SetSelection(kNoSelection);

	RemoveFixedControl(m_pJSIPDlgGroup);

	SetCapture(LTNULL);
	ForceMouseUpdate();
}

void CFolderJoin::UpdateIPText()
{
	m_pIPCtrl->RemoveAll();

	// Get the currently selected game server...
	int nIndex = m_pServerList->GetSelectedItem();

	void* pHandle = LTNULL;
	if((nIndex < 0) || (nIndex >= CListCtrl::kNoSelection))
	{
		return;
	}
	else
	{
		pHandle = (void*)m_pServerList->GetControl(nIndex)->GetParam1();
	}


	CGameSpyServer* pGame = GetGameSpyMgr()->GetServerFromHandle(pHandle);
	if (!pGame) return;


	char szTemp[32];
	sprintf(m_szIP,"%s",pGame->GetAddress());
	sprintf(m_szPort,"%d",pGame->GetIntValue("hostport",0));
	sprintf(szTemp,"%s:%s",m_szIP,m_szPort);
	HSTRING hTemp = g_pLTClient->CreateString(szTemp);
	m_pIPCtrl->AddString(hTemp);
	g_pLTClient->FreeString(hTemp);
}


// Sets certain controls to enabled or disabled depending on whether the list
// is being refreshed or not.

void CFolderJoin::SetUpdateControls(LTBOOL bEnabled)
{
	uint32 i;

	for(i = 0; i < m_fixedControlArray.size(); ++i)
		((CLTGUICtrl*)m_fixedControlArray[i])->Enable(bEnabled);

	for(i = 0; i < m_skipControlArray.size(); ++i)
		((CLTGUICtrl*)m_skipControlArray[i])->Enable(bEnabled);

	for(i = 0; i < m_controlArray.size(); ++i)
		((CLTGUICtrl*)m_controlArray[i])->Enable(bEnabled);


	// Make sure certain controls stay enabled or disabled
	m_pServerListTitle->Enable(LTFALSE);
	m_pFilterTitle->Enable(LTFALSE);


	m_pJoinIP->Enable(!m_bLANOnly);
}


void CFolderJoin::RefreshServers()
{
	HSTRING hStr = LTNULL;
	if (m_bLANOnly)
		hStr = g_pLTClient->FormatString(IDS_STATUS_SEARCH_LAN);
	else
		hStr = g_pLTClient->FormatString(IDS_STATUS_CONNECTING);

	SetUpdateControls(LTFALSE);
	SetDummyStatusState(hStr, 500, FSS_GETSERVICES);

	g_pLTClient->FreeString(hStr);
}

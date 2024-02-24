// FolderMulti.cpp: implementation of the CFolderMulti class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderMulti.h"
#include "FolderMgr.h"
#include "LayoutMgr.h"
#include "FolderCommands.h"
#include "FolderJoin.h"
#include "ClientRes.h"
#include "WinUtil.h"
#include "GameClientShell.h"
#include "ModelButeMgr.h"
#include "ProfileMgr.h"

#include "MultiplayerMgrDefs.h"
#include "ServerOptionMgr.h"


#define DIALOGSTATE_NONE			0
#define DIALOGSTATE_CDKEY			1
#define DIALOGSTATE_MOTD			2
#define DIALOGSTATE_PROGRESS		3
#define DIALOGSTATE_DECISION		4

#define VERIFYSTATE_NONE			0
#define VERIFYSTATE_CDKEY			1
#define VERIFYSTATE_USER			2
#define VERIFYSTATE_SERVERS			3
#define VERIFYSTATE_SETUP_MOTD		4
#define VERIFYSTATE_SYS_MOTD		5
#define VERIFYSTATE_GAME_MOTD		6
#define VERIFYSTATE_PRE_VERSION		7
#define VERIFYSTATE_VERSION			8
#define VERIFYSTATE_POST_VERSION	9
#define VERIFYSTATE_COMPLETE		10
#define VERIFYSTATE_READY			11

#define MIN_PROGRESS_TIME			2.0f

#define DCS_TYPE_STANDARD			0
#define DCS_TYPE_OK					1
#define DCS_TYPE_OKCANCEL			2


namespace
{
	int kGap = 0;

	LTIntPt ptDlgCDKSize;
	LTIntPt	ptDlgCDKEdit;

	LTIntPt	ptDlgMOTDSize;
	LTIntPt ptDlgMOTDUp;
	LTIntPt ptDlgMOTDDown;
	LTIntPt ptDlgMOTDOk;
	LTRect	rtDlgMOTDText;

	LTIntPt	ptDlgPRGSize;
	LTIntPt	ptDlgPRGUpdate;
	int		nDlgPRGMax;

	LTIntPt	ptDlgDCSSize;
	LTIntPt	ptDlgDCSOk;
	LTIntPt	ptDlgDCSCancel;
	LTIntPt	ptDlgDCSOkOnly;

	enum LocalCommands
	{
		CMD_CDKEY_UPDATE = FOLDER_CMD_CUSTOM,
		CMD_CDKEY_ENTER,
		CMD_MOTD_UP,
		CMD_MOTD_DOWN,
		CMD_MOTD_OK,
		CMD_DCS_OK,
		CMD_DCS_CANCEL,
	};
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderMulti::CFolderMulti()
{
	m_pHost = LTNULL;
	m_pHostLAN = LTNULL;
	m_pJoinNet = LTNULL;
	m_pJoinLAN = LTNULL;
	m_pUpdateCDK = LTNULL;
	m_pMain = LTNULL;

	m_pJoinSpeed = LTNULL;
	m_nConnection = 0;

	m_pCDKDialog = LTNULL;
	m_pCDKTitle = LTNULL;
	m_pCDKDesc = LTNULL;
	m_pCDKEdit = LTNULL;

	m_pMOTDDialog = LTNULL;
	m_pMOTDTitle = LTNULL;
	m_pMOTDUp = LTNULL;
	m_pMOTDDown = LTNULL;
	m_pMOTDOk = LTNULL;
	m_hMOTDDesc = LTNULL;

	m_nMOTDLines = 0;
	m_nMOTDDispLines = 0;
	m_nMOTDCurLine = 0;

	m_szMOTDSys[0] = '\0';
	m_szMOTDGame[0] = '\0';

	m_szCDK[0] = '\0';

	m_pPRGDialog = LTNULL;
	m_pPRGTitle = LTNULL;
	m_pPRGDesc = LTNULL;
	m_pPRGUpdate = LTNULL;

	m_nCurProgress = 0;

	m_pDCSDialog = LTNULL;
	m_pDCSTitle = LTNULL;
	m_pDCSDesc = LTNULL;
	m_pDCSOk = LTNULL;
	m_pDCSCancel = LTNULL;

	m_nDCSType = DCS_TYPE_STANDARD;
	m_bDecisionOk = LTFALSE;

	m_nDialogState = DIALOGSTATE_NONE;
	m_nVerifyState = VERIFYSTATE_NONE;

	m_bLAN = LTFALSE;
}

CFolderMulti::~CFolderMulti()
{
	// Delete the description surface if it exists...
	if(m_hMOTDDesc)
	{
		g_pLTClient->DeleteSurface(m_hMOTDDesc);
		m_hMOTDDesc = LTNULL;
	}
}


//////////////////////////////////////////////////////////////////////
// Build the folder
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::Build()
{
	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;

	CreateTitle(IDS_TITLE_MULTI);

	if (g_pLayoutMgr->HasCustomValue((eFolderID)m_nFolderID,"ColumnWidth"))
		kGap = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID,"ColumnWidth");


	m_pHost = AddLink(IDS_HOST, FOLDER_CMD_MP_HOST, IDS_HELP_HOST);
	m_pHostLAN = AddLink(IDS_HOST_LAN, FOLDER_CMD_MP_HOST, IDS_HELP_HOST_LAN);
	m_pJoinNet = AddLink(IDS_FIND_INTERNET, FOLDER_CMD_MP_JOIN, IDS_HELP_FIND_INTERNET);
	m_pJoinLAN = AddLink(IDS_FIND_LAN, FOLDER_CMD_MP_JOIN_LAN, IDS_HELP_FIND_LAN);

	// Connect speed (affects the 'UpdateRate' console variable)
	m_pJoinSpeed = CreateCycleItem(IDS_PLAYER_CONNECTION, IDS_HELP_PLAYER_CONNECTION, kGap, 0, &m_nConnection, LTTRUE, GetLargeFont());
	m_pJoinSpeed->AddString(IDS_CONNECT_VSLOW);
	m_pJoinSpeed->AddString(IDS_CONNECT_SLOW);
	m_pJoinSpeed->AddString(IDS_CONNECT_MEDIUM);
	m_pJoinSpeed->AddString(IDS_CONNECT_FAST);
	m_pJoinSpeed->AddString(IDS_CONNECT_VFAST);
	AddFixedControl(m_pJoinSpeed, kLinkGroup, m_LinkPos[2]);


	m_pUpdateCDK = AddLink(IDS_UPDATE_CDKEY, CMD_CDKEY_UPDATE, IDS_HELP_UPDATE_CDKEY);
	m_pMain = CreateTextItem(IDS_BACK, FOLDER_CMD_BACK, IDS_HELP_MAIN, LTFALSE, GetLargeFont());

	AddFixedControl(m_pMain, kBackGroup, m_BackPos);
	UseBack(LTFALSE);

	m_pHost->Enable(LTFALSE);
	m_pHostLAN->Enable(LTFALSE);
	m_pJoinNet->Enable(LTFALSE);
	m_pJoinLAN->Enable(LTFALSE);
	m_pJoinSpeed->Enable(LTFALSE);
	m_pUpdateCDK->Enable(LTFALSE);
	m_pMain->Enable(LTFALSE);

	// Build the dialog boxes
	BuildDialog_CDK();
	BuildDialog_MOTD();
	BuildDialog_PRG();
	BuildDialog_DCS();


	// Start the initialization process for the WON API
	HSTRING hVersion = g_pLTClient->FormatString(IDS_VERSION);
	m_GameSpyClientMgr.Init(GAMESPY_GAME_NAME, GAMESPY_SECRET_KEY, g_pLTClient->GetStringData(hVersion));
	g_pLTClient->FreeString(hVersion);


	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////
// Handle menu commands
//////////////////////////////////////////////////////////////////////

uint32 CFolderMulti::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
		case FOLDER_CMD_MP_HOST:
		{
			CServerOptions *pOpt = g_pServerOptionMgr->GetCurrentCfg();
			pOpt->bLANConnection = m_bLAN;
			g_pServerOptionMgr->SaveCurrentCfg();

			return CBaseFolder::OnCommand(dwCommand, dwParam1, dwParam2);
		}

		case FOLDER_CMD_MP_JOIN:
		{
			char szFailedMsg[256];

			if(!m_GameSpyClientMgr.SetupAuthentication(szFailedMsg, 256))
			{
				SetDialogText_DCS(IDS_AUTH_ERROR, 0, szFailedMsg);
				ShowDialog_DCS(LTTRUE, DCS_TYPE_OK);

				m_nVerifyState = VERIFYSTATE_COMPLETE;

				return 1;
			}

			SetLogo(LOGO_SCALEFX_SIERRA);

			return CBaseFolder::OnCommand(FOLDER_CMD_MP_JOIN, FOLDERJOIN_INTERNET, FOLDERJOIN_REFRESH);
		}

		case FOLDER_CMD_MP_JOIN_LAN:
		{
			SetLogo(LOGO_SCALEFX_GAMESPY);

			return CBaseFolder::OnCommand(FOLDER_CMD_MP_JOIN, FOLDERJOIN_LANONLY, FOLDERJOIN_REFRESH);
		}

		case CMD_CDKEY_UPDATE:
		{
			SetDialogText_CDK(IDS_CDKEY_TITLE, IDS_CDKEY_DESC);
			ShowDialog_CDK(LTTRUE);
			m_nVerifyState = VERIFYSTATE_CDKEY;
			break;
		}

		case CMD_CDKEY_ENTER:
		{
			if(GetCapture() == m_pCDKEdit)
			{
				if(m_GameSpyClientMgr.SetupCDKey(m_pCDKEdit->GetText()))
				{
					ShowDialog_CDK(LTFALSE);
				}
				else
				{
					SetDialogText_CDK(IDS_CDKEY_TITLE_ERROR, IDS_CDKEY_DESC_ERROR);
					g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\bind_error.wav", SOUNDPRIORITY_MISC_MEDIUM);
				}
			}

			break;
		}

		case CMD_MOTD_UP:
		{
			if(m_nMOTDCurLine > 0)
				m_nMOTDCurLine--;

			m_pMOTDUp->Enable(m_nMOTDCurLine != 0);
			m_pMOTDDown->Enable((m_nMOTDCurLine + m_nMOTDDispLines) < m_nMOTDLines);

			break;
		}

		case CMD_MOTD_DOWN:
		{
			if((m_nMOTDCurLine + m_nMOTDDispLines) < m_nMOTDLines)
				m_nMOTDCurLine++;

			m_pMOTDUp->Enable(m_nMOTDCurLine != 0);
			m_pMOTDDown->Enable((m_nMOTDCurLine + m_nMOTDDispLines) < m_nMOTDLines);

			break;
		}

		case CMD_MOTD_OK:
		{
			ShowDialog_MOTD(LTFALSE);
			break;
		}

		case CMD_DCS_OK:
		{
			m_bDecisionOk = LTTRUE;
			ShowDialog_DCS(LTFALSE);
			break;
		}

		case CMD_DCS_CANCEL:
		{
			m_bDecisionOk = LTFALSE;
			ShowDialog_DCS(LTFALSE);
			break;
		}

		default:
		{
			break;
		}
	}

	return CBaseFolder::OnCommand(dwCommand, dwParam1, dwParam2);
}


//////////////////////////////////////////////////////////////////////
// Handles the Escape key
//////////////////////////////////////////////////////////////////////

void CFolderMulti::Escape()
{
	if(GetCapture() == m_pCDKEdit)
	{
		m_pCDKEdit->SetText("\0");
		ShowDialog_CDK(LTFALSE);
		return;
	}

	if(m_nDialogState == DIALOGSTATE_NONE)
	{
		CBaseFolder::Escape();
	}
	else if(m_nDialogState == DIALOGSTATE_MOTD)
	{
		m_pMOTDOk->OnEnter();
	}
	else if(m_nDialogState == DIALOGSTATE_DECISION)
	{
		if(m_nDCSType == DCS_TYPE_OK)
			m_pDCSOk->OnEnter();
		else if(m_nDCSType == DCS_TYPE_OKCANCEL)
			m_pDCSCancel->OnEnter();
	}
}


//////////////////////////////////////////////////////////////////////
// Handles the Enter key
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::OnEnter()
{
	if(m_nDialogState == DIALOGSTATE_MOTD)
	{
		m_pMOTDOk->OnEnter();
	}
	else if(m_nDialogState == DIALOGSTATE_DECISION)
	{
		if(m_nDCSType != DCS_TYPE_STANDARD)
			m_pDCSOk->OnEnter();
	}

	return CBaseFolder::OnEnter();
}


//////////////////////////////////////////////////////////////////////
// Handles the Up key
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::OnUp()
{
	if(m_nDialogState == DIALOGSTATE_MOTD)
	{
		m_pMOTDUp->OnEnter();
	}

	return CBaseFolder::OnUp();
}


//////////////////////////////////////////////////////////////////////
// Handles the Down key
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::OnDown()
{
	if(m_nDialogState == DIALOGSTATE_MOTD)
	{
		m_pMOTDDown->OnEnter();
	}

	return CBaseFolder::OnDown();
}


//////////////////////////////////////////////////////////////////////
// Handles a key down event
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::HandleKeyDown(int key, int rep)
{
	if((m_nDialogState == DIALOGSTATE_CDKEY) && (key == VK_TAB))
	{
		return LTTRUE;
	}

	return CBaseFolder::HandleKeyDown(key,rep);
}


//////////////////////////////////////////////////////////////////////
// Constant update for rendering (also used for constant updates)
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::Render(HSURFACE hDestSurf)
{
    LTBOOL bOK = CBaseFolder::Render(hDestSurf);

	LTBOOL bBypass = LTFALSE;

#ifdef _DEMO
	bBypass = LTTRUE;
#endif


	// If we're in LAN mode, don't do the rest of this junk...
	if(m_bLAN)
	{
		m_pHostLAN->Enable(LTTRUE);
		m_pJoinLAN->Enable(LTTRUE);
		m_pJoinSpeed->Enable(LTTRUE);
		m_pMain->Enable(LTTRUE);

		return bOK;
	}


	// Turn off the menu controls if we're displaying a dialog
	if(m_nDialogState != DIALOGSTATE_NONE)
	{
		m_pHost->Enable(LTFALSE);
		m_pHostLAN->Enable(LTFALSE);
		m_pJoinNet->Enable(LTFALSE);
		m_pJoinLAN->Enable(LTFALSE);
		m_pJoinSpeed->Enable(LTFALSE);
		m_pUpdateCDK->Enable(LTFALSE);
		m_pMain->Enable(LTFALSE);
	}


	// Handle the current verification state
	switch(m_nVerifyState)
	{
		case VERIFYSTATE_NONE:
		{
			if(bBypass)
			{
				m_nVerifyState = VERIFYSTATE_USER;

				SetDialogText_DCS(IDS_DIRSERVER_TITLE, IDS_DIRSERVER_TEXT, LTNULL);
				ShowDialog_DCS(LTTRUE, DCS_TYPE_STANDARD);
			}
			else if(!m_GameSpyClientMgr.SetupCDKey())
			{
				SetDialogText_CDK(IDS_CDKEY_TITLE, IDS_CDKEY_DESC);
				ShowDialog_CDK(LTTRUE);
				m_nVerifyState = VERIFYSTATE_CDKEY;
			}
			else
			{
				m_nVerifyState = VERIFYSTATE_USER;

				SetDialogText_DCS(IDS_DIRSERVER_TITLE, IDS_DIRSERVER_TEXT, LTNULL);
				ShowDialog_DCS(LTTRUE, DCS_TYPE_STANDARD);
			}

			break;
		}

		case VERIFYSTATE_CDKEY:
		{
			if(m_nDialogState != DIALOGSTATE_CDKEY)
			{
				if(m_GameSpyClientMgr.IsCDKeyValid())
				{
					SetDialogText_DCS(IDS_DIRSERVER_TITLE, IDS_DIRSERVER_TEXT, LTNULL);
					ShowDialog_DCS(LTTRUE, DCS_TYPE_STANDARD);

					m_nVerifyState = VERIFYSTATE_USER;
				}
				else
				{
					m_nVerifyState = VERIFYSTATE_COMPLETE;
				}
			}

			break;
		}

		case VERIFYSTATE_USER:
		{
			m_GameSpyClientMgr.SetupUserIdentity();
			m_nVerifyState = VERIFYSTATE_SERVERS;

			break;
		}

		case VERIFYSTATE_SERVERS:
		{
			m_GameSpyClientMgr.SetupDirServers();
			m_nVerifyState = VERIFYSTATE_SETUP_MOTD;

			ShowDialog_DCS(LTFALSE);

			break;
		}

		case VERIFYSTATE_SETUP_MOTD:
		{
			m_GameSpyClientMgr.SetupMOTD(m_szMOTDSys, m_szMOTDGame, MOTD_STRING_SIZE);

			if(m_szMOTDSys[0])
			{
				SetDialogText_MOTD(IDS_MOTD_SYS_TITLE, m_szMOTDSys);
				ShowDialog_MOTD(LTTRUE);
				m_nVerifyState = VERIFYSTATE_SYS_MOTD;
			}
			else if(m_szMOTDGame[0])
			{
				SetDialogText_MOTD(IDS_MOTD_GAME_TITLE, m_szMOTDGame);
				ShowDialog_MOTD(LTTRUE);
				m_nVerifyState = VERIFYSTATE_GAME_MOTD;
			}
			else
			{
				m_nVerifyState = VERIFYSTATE_PRE_VERSION;
			}

			break;
		}

		case VERIFYSTATE_SYS_MOTD:
		{
			if(m_nDialogState != DIALOGSTATE_MOTD)
			{
				if(m_szMOTDGame[0])
				{
					SetDialogText_MOTD(IDS_MOTD_GAME_TITLE, m_szMOTDGame);
					ShowDialog_MOTD(LTTRUE);
					m_nVerifyState = VERIFYSTATE_GAME_MOTD;
				}
				else
				{
					m_nVerifyState = VERIFYSTATE_PRE_VERSION;
				}
			}

			break;
		}

		case VERIFYSTATE_GAME_MOTD:
		{
			if(m_nDialogState != DIALOGSTATE_MOTD)
			{
				m_nVerifyState = VERIFYSTATE_PRE_VERSION;
			}

			break;
		}

		case VERIFYSTATE_PRE_VERSION:
		{
			SetDialogText_PRG(IDS_VERSION_CHECK_TITLE, IDS_VERSION_CHECK_DESC);
			ShowDialog_PRG(LTTRUE);
			m_nVerifyState = VERIFYSTATE_VERSION;

			m_GameSpyClientMgr.SetupVersionCheck();

			break;
		}

		case VERIFYSTATE_VERSION:
		{
			int nVersionCheck = m_GameSpyClientMgr.GetVersionCheckState();

			if(nVersionCheck != VERSIONCHECKSTATE_INPROGRESS)
			{
				if((g_pLTClient->GetTime() - m_fProgessStart) >= MIN_PROGRESS_TIME)
				{
					ShowDialog_PRG(LTFALSE);

#ifndef _DEMO
					if(nVersionCheck == VERSIONCHECKSTATE_FAILED)
					{
						SetDialogText_DCS(IDS_VC_FAILED_TITLE, IDS_VC_FAILED_DESC);
						ShowDialog_DCS(LTTRUE, DCS_TYPE_OK);
						m_nVerifyState = VERIFYSTATE_POST_VERSION;
					}
					else if(nVersionCheck == VERSIONCHECKSTATE_REQUIREPATCH)
					{
						SetDialogText_DCS(IDS_VC_RPATCH_TITLE, IDS_VC_RPATCH_DESC);
						ShowDialog_DCS(LTTRUE, DCS_TYPE_OKCANCEL);
						m_nVerifyState = VERIFYSTATE_POST_VERSION;
					}
					else if(nVersionCheck == VERSIONCHECKSTATE_OPTIONALPATCH)
					{
						m_nVerifyState = VERIFYSTATE_COMPLETE;
//						SetDialogText_DCS(IDS_VC_OPATCH_TITLE, IDS_VC_OPATCH_DESC);
//						ShowDialog_DCS(LTTRUE, DCS_TYPE_OKCANCEL);
//						m_nVerifyState = VERIFYSTATE_POST_VERSION;
					}
					else if(nVersionCheck == VERSIONCHECKSTATE_SUCCESS)
#endif
					{
						m_nVerifyState = VERIFYSTATE_COMPLETE;
					}
				}
			}

			break;
		}

		case VERIFYSTATE_POST_VERSION:
		{
			if(m_nDialogState != DIALOGSTATE_DECISION)
			{
				int nVersionCheck = m_GameSpyClientMgr.GetVersionCheckState();

				if(nVersionCheck == VERSIONCHECKSTATE_FAILED)
				{
					m_GameSpyClientMgr.SetLANOnly(LTTRUE);
				}
				else if(nVersionCheck == VERSIONCHECKSTATE_REQUIREPATCH)
				{
					if(m_bDecisionOk)
					{
						LaunchSierraUp();
					}
					else
					{
						m_GameSpyClientMgr.SetLANOnly(LTTRUE);
					}
				}
				else if(nVersionCheck == VERSIONCHECKSTATE_OPTIONALPATCH)
				{
					if(m_bDecisionOk)
					{
						LaunchSierraUp();
					}
				}

				m_nVerifyState = VERIFYSTATE_COMPLETE;
			}

			break;
		}

		case VERIFYSTATE_COMPLETE:
		{
			if(bBypass || m_GameSpyClientMgr.IsCDKeyValid())
			{
				m_pHost->Enable(!m_GameSpyClientMgr.GetLANOnly());
				m_pJoinNet->Enable(!m_GameSpyClientMgr.GetLANOnly());
				m_pHostLAN->Enable(LTTRUE);
				m_pJoinLAN->Enable(LTTRUE);
				m_pJoinSpeed->Enable(LTTRUE);
			}

			m_pUpdateCDK->Enable(LTTRUE);
			m_pMain->Enable(LTTRUE);

			// Move on automatically if everything for GameSpy Arcade is setup right...
			if(m_pJoinNet->IsEnabled() && m_pFolderMgr->GetGSA())
			{
				JoinGameSpyArcadeServer();
			}
			else
			{
				// Always turn it off if it didn't get used
				m_pFolderMgr->SetGSA(LTFALSE);
			}

			m_nVerifyState = VERIFYSTATE_READY;

			break;
		}

		case VERIFYSTATE_READY:
		{
			// Don't need to do anything... we're done

			if(m_nDialogState == DIALOGSTATE_NONE)
			{
				if(bBypass || m_GameSpyClientMgr.IsCDKeyValid())
				{
					m_pHost->Enable(!m_GameSpyClientMgr.GetLANOnly());
					m_pJoinNet->Enable(!m_GameSpyClientMgr.GetLANOnly());
					m_pHostLAN->Enable(LTTRUE);
					m_pJoinLAN->Enable(LTTRUE);
					m_pJoinSpeed->Enable(LTTRUE);
				}

				m_pUpdateCDK->Enable(LTTRUE);
				m_pMain->Enable(LTTRUE);
			}

			break;
		}
	}


	// Handle any special rendering needed for a specific dialog state
	switch(m_nDialogState)
	{
		case DIALOGSTATE_MOTD:
		{
			// Render the description surface to the screen...
			if(m_hMOTDDesc)
			{
				LTRect srcRect, destRect;

				// Figure out the area to render from...
				srcRect.left = 0;
				srcRect.right = rtDlgMOTDText.right - rtDlgMOTDText.left;
				srcRect.top = m_nMOTDCurLine * GetHelpFont()->GetHeight();
				srcRect.bottom = srcRect.top + (m_nMOTDDispLines * GetHelpFont()->GetHeight());

				// Calculate the destination
				int xo = g_pInterfaceResMgr->GetXOffset();
				int yo = g_pInterfaceResMgr->GetYOffset();

				LTIntPt pt(320 - (ptDlgMOTDSize.x / 2) + xo + rtDlgMOTDText.left, 240 - (ptDlgMOTDSize.y / 2) + yo + rtDlgMOTDText.top);

				// Do the rendering...
				g_pLTClient->DrawSurfaceToSurfaceTransparent(hDestSurf, m_hMOTDDesc, &srcRect, pt.x, pt.y, kBlack);
			}

			break;
		}

		case DIALOGSTATE_PROGRESS:
		{
			// Update the progess ticks
			Update_PRG();

			break;
		}

		default:
		{
			break;
		}
	}


	return bOK;
}


//////////////////////////////////////////////////////////////////////

void CFolderMulti::JoinGameSpyArcadeServer()
{
	uint16 nPort = atoi(m_pFolderMgr->GetGSA_Port());
	if (!nPort) return;

	// Set the player name
	if(m_pFolderMgr->GetGSA_Name()[0])
	{
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		pProfile->m_sPlayerName = m_pFolderMgr->GetGSA_Name();
	}

	// Always turn it off if it didn't get used
	m_pFolderMgr->SetGSA(LTFALSE);


	// Look for game server?
	LTRESULT result = g_pGameClientShell->JoinMultiplayerGameAtIP(m_pFolderMgr->GetGSA_IP(), nPort, m_pFolderMgr->GetGSA_PW());

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


//////////////////////////////////////////////////////////////////////
// Gaining and losing focus of the menu
//////////////////////////////////////////////////////////////////////

void CFolderMulti::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	if(bFocus)
	{
		pProfile->SetMultiplayer();
		m_nConnection = pProfile->m_nConnection;

		// Remove all the controls
		RemoveFixedControl(m_pHost);
		RemoveFixedControl(m_pHostLAN);
		RemoveFixedControl(m_pJoinNet);
		RemoveFixedControl(m_pJoinLAN);
		RemoveFixedControl(m_pUpdateCDK);

		// Add the controls back depending on if we're in LAN mode
		if(m_bLAN)
		{
			AddFixedControl(m_pHostLAN, kLinkGroup, m_LinkPos[0]);
			AddFixedControl(m_pJoinLAN, kLinkGroup, m_LinkPos[1]);
		}
		else
		{
			AddFixedControl(m_pHost, kLinkGroup, m_LinkPos[0]);
			AddFixedControl(m_pJoinNet, kLinkGroup, m_LinkPos[1]);

#ifndef _DEMO
			AddFixedControl(m_pUpdateCDK, kLinkGroup, m_LinkPos[3]);
#endif
		}

		UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		// Delete the description surface if it exists...
		if(m_hMOTDDesc)
		{
			g_pLTClient->DeleteSurface(m_hMOTDDesc);
			m_hMOTDDesc = LTNULL;
		}

		pProfile->m_nConnection = (uint8)m_nConnection;
		pProfile->ApplyMultiplayer();
		pProfile->Save();
	}

	CBaseFolder::OnFocus(bFocus);
}


//////////////////////////////////////////////////////////////////////
// Build the dialog for the CD Key input
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::BuildDialog_CDK()
{
	LTIntPt	pt(0, 0);

	ptDlgCDKSize	= g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DlgCDKSize");
	ptDlgCDKEdit	= g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DlgCDKEdit");


	// Create the group that will contain all the appropriate controls
	m_pCDKDialog = CreateGroup(ptDlgCDKSize.x, ptDlgCDKSize.y, LTNULL);
	m_pCDKDialog->AllowSubSelection(LTTRUE);
	m_pCDKDialog->Enable(LTTRUE);


	// Create the frame of the dialog
	CFrameCtrl *pFrame;

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(10,10,10), 0.75f, ptDlgCDKSize.x, ptDlgCDKSize.y);
	m_pCDKDialog->AddControl(pFrame, pt);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, ptDlgCDKSize.x, ptDlgCDKSize.y, LTFALSE);
	m_pCDKDialog->AddControl(pFrame, pt);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, ptDlgCDKSize.x, GetHelpFont()->GetHeight() + 6);
	m_pCDKDialog->AddControl(pFrame, pt);


	// Now create the title and description for the dialog
	m_pCDKTitle = CreateStaticTextItem("---", LTNULL, LTNULL, ptDlgCDKSize.x, ptDlgCDKSize.y, LTTRUE, GetHelpFont());
	m_pCDKDesc = CreateStaticTextItem("<>", LTNULL, LTNULL, ptDlgCDKSize.x - 20, ptDlgCDKSize.y - GetHelpFont()->GetHeight() - 15, LTTRUE, GetHelpFont());

	m_pCDKDialog->AddControl(m_pCDKTitle, LTIntPt(3, 3));
	m_pCDKDialog->AddControl(m_pCDKDesc, LTIntPt(10, GetHelpFont()->GetHeight() + 15));


	// Make the edit control needed to type in the CD key
	m_pCDKEdit = CreateEditCtrl(" ", CMD_CDKEY_ENTER, LTNULL, m_szCDK, 26, 0, LTTRUE);
	m_pCDKEdit->EnableCursor();
    m_pCDKEdit->Enable(LTFALSE);


	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////
// Setup the title and description for the dialog
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::SetDialogText_CDK(uint32 nTitle, uint32 nDesc)
{
	m_pCDKTitle->SetString(nTitle);
	m_pCDKDesc->SetString(nDesc);

	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////
// Show or hide the dialog for the CD Key input
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::ShowDialog_CDK(LTBOOL bShow)
{
	if(bShow)
	{
		// Set the dialog state so we can know that we're displaying this dialog
		m_nDialogState = DIALOGSTATE_CDKEY;

		m_pCDKEdit->UpdateData(LTFALSE);
		char null[CDKEY_STRING_SIZE];
		null[0] = '\0';
		m_pCDKEdit->SetText(null);

		LTIntPt pt(320 - (ptDlgCDKSize.x / 2), 240 - (ptDlgCDKSize.y / 2));

		AddFixedControl(m_pCDKDialog, kFreeGroup, pt, LTTRUE);
		AddFixedControl(m_pCDKEdit, kFreeGroup, LTIntPt(pt.x + ptDlgCDKEdit.x, pt.y + ptDlgCDKEdit.y), LTTRUE);

		SetSelection(GetIndex(m_pCDKDialog));
		SetCapture(m_pCDKEdit);

		m_pCDKEdit->SetColor(m_hSelectedColor, m_hSelectedColor, m_hSelectedColor);
		m_pCDKEdit->Select(LTTRUE);
	}
	else
	{
		// Go back to the normal dialog state (none displayed)
		m_nDialogState = DIALOGSTATE_NONE;

		m_pCDKEdit->UpdateData(LTTRUE);

		SetSelection(kNoSelection);

		RemoveFixedControl(m_pCDKDialog);
		RemoveFixedControl(m_pCDKEdit);

		SetCapture(LTNULL);
		ForceMouseUpdate();

		// Re-enable the controls that we need to
		m_GameSpyClientMgr.SetupCDKey(m_pCDKEdit->GetText());
	}

	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////
// Build the dialog for the message of the day display
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::BuildDialog_MOTD()
{
	LTIntPt	pt(0, 0);

	ptDlgMOTDSize	= g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DlgMOTDSize");
	ptDlgMOTDUp		= g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DlgMOTDUp");
	ptDlgMOTDDown	= g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DlgMOTDDown");
	ptDlgMOTDOk		= g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DlgMOTDOk");
	rtDlgMOTDText	= g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID, "DlgMOTDText");


	// Create the group that will contain all the appropriate controls
	m_pMOTDDialog = CreateGroup(ptDlgMOTDSize.x, ptDlgMOTDSize.y, LTNULL);
	m_pMOTDDialog->AllowSubSelection(LTTRUE);
	m_pMOTDDialog->Enable(LTTRUE);


	// Create the frame of the dialog
	CFrameCtrl *pFrame;

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(10,10,10), 0.75f, ptDlgMOTDSize.x, ptDlgMOTDSize.y);
	m_pMOTDDialog->AddControl(pFrame, pt);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, ptDlgMOTDSize.x, ptDlgMOTDSize.y, LTFALSE);
	m_pMOTDDialog->AddControl(pFrame, pt);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, ptDlgMOTDSize.x, GetHelpFont()->GetHeight() + 6);
	m_pMOTDDialog->AddControl(pFrame, pt);


	// Now create the title
	m_pMOTDTitle = CreateStaticTextItem("---", LTNULL, LTNULL, ptDlgCDKSize.x, ptDlgCDKSize.y, LTTRUE, GetHelpFont());
	m_pMOTDDialog->AddControl(m_pMOTDTitle, LTIntPt(3, 3));


	// Make the up and down arrows
	m_pMOTDUp = new CBitmapCtrl;
	m_pMOTDUp->Create(g_pLTClient, "interface\\menus\\slider_up.pcx", "interface\\menus\\slider_up_h.pcx", "interface\\menus\\slider_up_d.pcx", this, CMD_MOTD_UP);
	m_pMOTDUp->SetHelpID(IDS_MOTD_UP);

	m_pMOTDDown = new CBitmapCtrl;
	m_pMOTDDown->Create(g_pLTClient, "interface\\menus\\slider_down.pcx", "interface\\menus\\slider_down_h.pcx", "interface\\menus\\slider_down_d.pcx", this, CMD_MOTD_DOWN);
	m_pMOTDDown->SetHelpID(IDS_MOTD_DOWN);


	// Make the Ok button
	m_pMOTDOk = CreateTextItem(IDS_OK, CMD_MOTD_OK, IDS_MOTD_OK);


	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////
// Setup the title and description for the dialog
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::SetDialogText_MOTD(uint32 nTitle, char *szDesc)
{
	// Set the title
	m_pMOTDTitle->SetString(nTitle);

	// Delete the description surface if it exists...
	if(m_hMOTDDesc)
	{
		g_pLTClient->DeleteSurface(m_hMOTDDesc);
		m_hMOTDDesc = LTNULL;
	}


	// Reinit the line values
	m_nMOTDLines = 0;
	m_nMOTDDispLines = 0;
	m_nMOTDCurLine = 0;

	m_pMOTDUp->Enable(LTFALSE);


	if(szDesc)
	{
		int nWidth = rtDlgMOTDText.right - rtDlgMOTDText.left;

		// Grab the dimensions of the string
		LTIntPt strDims = GetHelpFont()->GetTextExtentsFormat(szDesc, nWidth);

		// Create the surface with these dimensions and clear it to black
		m_hMOTDDesc = g_pLTClient->CreateSurface(strDims.x, strDims.y);
		g_pLTClient->FillRect(m_hMOTDDesc, LTNULL, kBlack);

		// Draw the text onto this surface and optimize it
		GetHelpFont()->DrawFormat(szDesc, m_hMOTDDesc, 0, 0, nWidth);
		g_pLTClient->OptimizeSurface(m_hMOTDDesc, kBlack);


		// Calculate the line values needed to render to the proper area
		m_nMOTDLines = strDims.y / GetHelpFont()->GetHeight();
		m_nMOTDDispLines = (rtDlgMOTDText.bottom - rtDlgMOTDText.top) / GetHelpFont()->GetHeight();


		// See if the down arrow should be enabled...
		m_pMOTDDown->Enable(m_nMOTDDispLines < m_nMOTDLines);
	}


	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////
// Show or hide the dialog for the message of the day
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::ShowDialog_MOTD(LTBOOL bShow)
{
	if(bShow)
	{
		// Set the dialog state so we can know that we're displaying this dialog
		m_nDialogState = DIALOGSTATE_MOTD;

		LTIntPt pt(320 - (ptDlgMOTDSize.x / 2), 240 - (ptDlgMOTDSize.y / 2));

		AddFixedControl(m_pMOTDDialog, kFreeGroup, pt, LTTRUE);
		AddFixedControl(m_pMOTDUp, kFreeGroup, LTIntPt(pt.x + ptDlgMOTDUp.x, pt.y + ptDlgMOTDUp.y), LTTRUE);
		AddFixedControl(m_pMOTDDown, kFreeGroup, LTIntPt(pt.x + ptDlgMOTDDown.x, pt.y + ptDlgMOTDDown.y), LTTRUE);
		AddFixedControl(m_pMOTDOk, kFreeGroup, LTIntPt(pt.x + ptDlgMOTDOk.x, pt.y + ptDlgMOTDOk.y), LTTRUE);

		SetSelection(GetIndex(m_pMOTDDialog));
	}
	else
	{
		// Go back to the normal dialog state (none displayed)
		m_nDialogState = DIALOGSTATE_NONE;

		SetSelection(kNoSelection);

		RemoveFixedControl(m_pMOTDDialog);
		RemoveFixedControl(m_pMOTDUp);
		RemoveFixedControl(m_pMOTDDown);
		RemoveFixedControl(m_pMOTDOk);

		ForceMouseUpdate();
	}

	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////
// Build the dialog for the progress meter
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::BuildDialog_PRG()
{
	LTIntPt	pt(0, 0);

	ptDlgPRGSize	= g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DlgPRGSize");
	ptDlgPRGUpdate	= g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DlgPRGUpdate");
	nDlgPRGMax		= g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "DlgPRGMax");


	// Create the group that will contain all the appropriate controls
	m_pPRGDialog = CreateGroup(ptDlgPRGSize.x, ptDlgPRGSize.y, LTNULL);
	m_pPRGDialog->AllowSubSelection(LTTRUE);
	m_pPRGDialog->Enable(LTTRUE);


	// Create the frame of the dialog
	CFrameCtrl *pFrame;

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(10,10,10), 0.75f, ptDlgPRGSize.x, ptDlgPRGSize.y);
	m_pPRGDialog->AddControl(pFrame, pt);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, ptDlgPRGSize.x, ptDlgPRGSize.y, LTFALSE);
	m_pPRGDialog->AddControl(pFrame, pt);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, ptDlgPRGSize.x, GetHelpFont()->GetHeight() + 6);
	m_pPRGDialog->AddControl(pFrame, pt);


	// Now create the static text items...
	m_pPRGTitle = CreateStaticTextItem("---", LTNULL, LTNULL, ptDlgPRGSize.x, ptDlgPRGSize.y, LTTRUE, GetHelpFont());
	m_pPRGDesc = CreateStaticTextItem("<>", LTNULL, LTNULL, ptDlgPRGSize.x - 20, ptDlgPRGSize.y - GetHelpFont()->GetHeight() - 15, LTTRUE, GetHelpFont());
	m_pPRGUpdate = CreateStaticTextItem("...", LTNULL, LTNULL, ptDlgPRGSize.x, ptDlgPRGSize.y, LTTRUE, GetHelpFont());

	m_pPRGDialog->AddControl(m_pPRGTitle, LTIntPt(3, 3));
	m_pPRGDialog->AddControl(m_pPRGDesc, LTIntPt(10, GetHelpFont()->GetHeight() + 15));
	m_pPRGDialog->AddControl(m_pPRGUpdate, LTIntPt(ptDlgPRGUpdate.x , ptDlgPRGUpdate.y));


	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////
// Setup the title and description for the dialog
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::SetDialogText_PRG(uint32 nTitle, uint32 nDesc)
{
	m_pPRGTitle->SetString(nTitle);
	m_pPRGDesc->SetString(nDesc);

	m_fProgessStart = g_pLTClient->GetTime();
	m_nCurProgress = 0;

	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////
// Show or hide the dialog for the progress
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::ShowDialog_PRG(LTBOOL bShow)
{
	if(bShow)
	{
		// Set the dialog state so we can know that we're displaying this dialog
		m_nDialogState = DIALOGSTATE_PROGRESS;

		LTIntPt pt(320 - (ptDlgPRGSize.x / 2), 240 - (ptDlgPRGSize.y / 2));

		AddFixedControl(m_pPRGDialog, kFreeGroup, pt, LTTRUE);

		SetSelection(GetIndex(m_pPRGDialog));
	}
	else
	{
		// Go back to the normal dialog state (none displayed)
		m_nDialogState = DIALOGSTATE_NONE;

		SetSelection(kNoSelection);

		RemoveFixedControl(m_pPRGDialog);

		ForceMouseUpdate();
	}

	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////
// Update for the dialog
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::Update_PRG()
{
	// Get the number of seconds we've been checking progress
	LTFLOAT fTime = (g_pLTClient->GetTime() - m_fProgessStart);
	int nTicks = (int)(fTime / 0.04f); // 25 ticks per second

	if(nTicks > nDlgPRGMax)
	{
		m_fProgessStart = g_pLTClient->GetTime();
		nTicks = nDlgPRGMax;
	}

	// Create a buffer for the text...
	char szBuffer[64];
	szBuffer[0] = '\0';

	for(int i = 0; i < nTicks; i++)
	{
		strcat(szBuffer, "*");
	}

	m_pPRGUpdate->SetString(szBuffer);

	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////
// Build the dialog for basic decision making
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::BuildDialog_DCS()
{
	LTIntPt	pt(0, 0);

	ptDlgDCSSize	= g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DlgDCSSize");
	ptDlgDCSOk		= g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DlgDCSOk");
	ptDlgDCSCancel	= g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DlgDCSCancel");
	ptDlgDCSOkOnly	= g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DlgDCSOkOnly");


	// Create the group that will contain all the appropriate controls
	m_pDCSDialog = CreateGroup(ptDlgDCSSize.x, ptDlgDCSSize.y, LTNULL);
	m_pDCSDialog->AllowSubSelection(LTTRUE);
	m_pDCSDialog->Enable(LTTRUE);


	// Create the frame of the dialog
	CFrameCtrl *pFrame;

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(10,10,10), 0.75f, ptDlgDCSSize.x, ptDlgDCSSize.y);
	m_pDCSDialog->AddControl(pFrame, pt);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, ptDlgDCSSize.x, ptDlgDCSSize.y, LTFALSE);
	m_pDCSDialog->AddControl(pFrame, pt);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, ptDlgDCSSize.x, GetHelpFont()->GetHeight() + 6);
	m_pDCSDialog->AddControl(pFrame, pt);


	// Now create the static text items...
	m_pDCSTitle = CreateStaticTextItem("---", LTNULL, LTNULL, ptDlgDCSSize.x, ptDlgDCSSize.y, LTTRUE, GetHelpFont());
	m_pDCSDesc = CreateStaticTextItem("<>", LTNULL, LTNULL, ptDlgDCSSize.x - 20, ptDlgDCSSize.y - GetHelpFont()->GetHeight() - 15, LTTRUE, GetHelpFont());

	m_pDCSDialog->AddControl(m_pDCSTitle, LTIntPt(3, 3));
	m_pDCSDialog->AddControl(m_pDCSDesc, LTIntPt(10, GetHelpFont()->GetHeight() + 15));


	// Make the Ok button
	m_pDCSOk = CreateTextItem(IDS_OK, CMD_DCS_OK, IDS_HELP_OK);
	m_pDCSCancel = CreateTextItem(IDS_CANCEL, CMD_DCS_CANCEL, IDS_HELP_CANCEL);


	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////
// Setup the title and description for the dialog
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::SetDialogText_DCS(uint32 nTitle, uint32 nDesc, char *szDesc)
{
	m_pDCSTitle->SetString(nTitle);

	if(szDesc)
	{
		m_pDCSDesc->SetString(szDesc);
	}
	else
	{
		m_pDCSDesc->SetString(nDesc);
	}

	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////
// Show or hide the dialog for the decision
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::ShowDialog_DCS(LTBOOL bShow, uint8 nType)
{
	if(bShow)
	{
		// Set the dialog state so we can know that we're displaying this dialog
		m_nDialogState = DIALOGSTATE_DECISION;
		m_nDCSType = nType;

		LTIntPt pt(320 - (ptDlgDCSSize.x / 2), 240 - (ptDlgDCSSize.y / 2));

		AddFixedControl(m_pDCSDialog, kFreeGroup, pt, LTTRUE);

		if(nType == DCS_TYPE_OK)
		{
			AddFixedControl(m_pDCSOk, kFreeGroup, LTIntPt(pt.x + ptDlgDCSOkOnly.x, pt.y + ptDlgDCSOkOnly.y), LTTRUE);
		}
		else if(nType == DCS_TYPE_OKCANCEL)
		{
			AddFixedControl(m_pDCSOk, kFreeGroup, LTIntPt(pt.x + ptDlgDCSOk.x, pt.y + ptDlgDCSOk.y), LTTRUE);
			AddFixedControl(m_pDCSCancel, kFreeGroup, LTIntPt(pt.x + ptDlgDCSCancel.x, pt.y + ptDlgDCSCancel.y), LTTRUE);
		}

		SetSelection(GetIndex(m_pDCSDialog));
	}
	else
	{
		// Go back to the normal dialog state (none displayed)
		m_nDialogState = DIALOGSTATE_NONE;

		SetSelection(kNoSelection);

		RemoveFixedControl(m_pDCSDialog);
		RemoveFixedControl(m_pDCSOk);
		RemoveFixedControl(m_pDCSCancel);

		ForceMouseUpdate();
	}

	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////
// Run the game update utility
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMulti::LaunchSierraUp()
{
	PROCESS_INFORMATION procInfo;
	STARTUPINFO startInfo;
	CString sCmdLine;
	RMode rMode;

	// Save the current render mode.  We'll need to restore it if the serverapp
	// launching fails.
	g_pLTClient->GetRenderMode( &rMode );

	// Shutdown the renderer, minimize it, and hide it...
	g_pLTClient->ShutdownRender( RSHUTDOWN_MINIMIZEWINDOW | RSHUTDOWN_HIDEWINDOW );

	// Initialize the startup info.
	memset( &startInfo, 0, sizeof( STARTUPINFO ));
	startInfo.cb = sizeof( STARTUPINFO );


	HSTRING hVersion = g_pLTClient->FormatString(IDS_VERSION);

	// Setup the command line.
#ifdef _DEMO
	sCmdLine.Format("SierraUp.exe ProductName \"AvP2Demo\" CurrentVersion \"%s\" ResourceDllFile \"AVP2Up.dll\"", g_pLTClient->GetStringData(hVersion));
#else
	sCmdLine.Format("SierraUp.exe ProductName \"AliensVsPredator2\" CurrentVersion \"%s\" ResourceDllFile \"AVP2Up.dll\"", g_pLTClient->GetStringData(hVersion));
#endif

	g_pLTClient->FreeString(hVersion);


	// Start the server app.
	if(!CreateProcess("SierraUp.exe", ( char* )( char const* )sCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &procInfo))
	{
		// Serverapp failed.  Restore the render mode.
		g_pLTClient->SetRenderMode( &rMode );
		return LTFALSE;
	}

	// We're done with this process.
	g_pLTClient->Shutdown();

	return LTTRUE;
}


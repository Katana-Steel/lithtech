// FolderGame.cpp: implementation of the CFolderGame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderGame.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
#include "GameSettings.h"
#include "ProfileMgr.h"


namespace
{
	int kGap = 0;
	int kWidth = 0;

	enum LocalCommands
	{
		CMD_GENERAL = FOLDER_CMD_CUSTOM,
		CMD_MULTIPLAYER,
	};
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderGame::CFolderGame()
{
	m_nPickupMsgDur		= 0;
	m_bAlwaysRun		= LTFALSE;
	m_bSubtitles		= LTFALSE;
	m_bObjectives		= LTFALSE;
	m_bHeadBob			= LTFALSE;
	m_bHeadCanting		= LTFALSE;
	m_bWeaponSway		= LTFALSE;
	m_bOrientOverlay	= LTFALSE;
	m_bHint				= LTFALSE;
	m_bAutoswitch		= LTFALSE;
	m_pObjToggle		= LTNULL;

	m_nMessageDisplay	= 0;
	m_nMessageTime		= 0;
	m_nConnection		= 0;
	m_bIgnoreTaunts		= LTFALSE;
}

//////////////////////////////////////////////////////////////////////

CFolderGame::~CFolderGame()
{

}

//////////////////////////////////////////////////////////////////////
// Build the folder

LTBOOL CFolderGame::Build()
{
	CreateTitle(IDS_TITLE_GAME);
	UseBack(LTFALSE);

	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;


	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_GAME,"ColumnWidth"))
		kGap = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_GAME,"ColumnWidth");
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_GAME,"SliderWidth"))
		kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_GAME,"SliderWidth");

	m_pGeneral = AddTab(IDS_OPT_GENERAL, CMD_GENERAL, IDS_HELP_OPT_GENERAL);
	m_pMultiplayer = AddTab(IDS_OPT_MULTIPLAYER, CMD_MULTIPLAYER, IDS_HELP_OPT_MULTIPLAYER);


	return LTTRUE;
}

//////////////////////////////////////////////////////////////////////

uint32 CFolderGame::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
		case CMD_GENERAL:
		{
			SetTab(0);

			m_pGeneral->Enable(LTFALSE);
			m_pMultiplayer->Enable(LTTRUE);

			UpdateData(LTTRUE);
			BuildControlList();
			UpdateData(LTFALSE);
			break;
		}

		case CMD_MULTIPLAYER:
		{
			SetTab(1);

			m_pGeneral->Enable(LTTRUE);
			m_pMultiplayer->Enable(LTFALSE);

			UpdateData(LTTRUE);
			BuildControlList();
			UpdateData(LTFALSE);
			break;
		}

		default:
			return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}

	return 1;
};

//////////////////////////////////////////////////////////////////////

void CFolderGame::OnFocus(LTBOOL bFocus)
{
	FocusSetup(bFocus);
}

//////////////////////////////////////////////////////////////////////

void CFolderGame::FocusSetup(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	if (bFocus)
	{
		SetTab(0);

		m_pGeneral->Enable(LTFALSE);
		m_pMultiplayer->Enable(LTTRUE);

		BuildControlList();

		pProfile->SetGame();
		pProfile->SetMultiplayer();


		m_nPickupMsgDur = (int)pProfile->m_fPickupDuration;
		m_bHint = pProfile->m_bHints;
		m_bAlwaysRun = pProfile->m_bRunLock;
		m_bSubtitles = pProfile->m_bSubtitles;
		m_bHeadBob = pProfile->m_bHeadBob;
		m_bHeadCanting = pProfile->m_bHeadCanting;
		m_bWeaponSway = pProfile->m_bWeaponSway;
		m_bOrientOverlay = pProfile->m_bOrientOverlay;
		m_bAutoswitch = pProfile->m_bAutoswitch;

		m_nMessageDisplay = (int)(10.0f * pProfile->m_fMessageDisplay);
		m_nMessageTime = (int)((pProfile->m_fMessageTime - 0.15f) * 5.0f);
		m_nConnection = pProfile->m_nConnection;
		m_bIgnoreTaunts = pProfile->m_bIgnoreTaunts;


		//if 4th difficulty level added, disable objective messages
		if (g_pGameClientShell->IsInWorld() && g_pGameClientShell->GetGameType()->GetDifficulty() > DIFFICULTY_PC_HARD)
		{
			m_bObjectives = 0;
			m_pObjToggle->Enable(LTFALSE);
		}
		else
		{
			m_bObjectives = pProfile->m_bObjectives;
			m_pObjToggle->Enable(LTTRUE);
		}

		UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		pProfile->m_fPickupDuration = (LTFLOAT)m_nPickupMsgDur;
		pProfile->m_bHints = m_bHint;
		pProfile->m_bRunLock = m_bAlwaysRun;
		pProfile->m_bSubtitles = m_bSubtitles;
		pProfile->m_bHeadBob = m_bHeadBob;
		pProfile->m_bHeadCanting = m_bHeadCanting;
		pProfile->m_bWeaponSway = m_bWeaponSway;
		pProfile->m_bOrientOverlay = m_bOrientOverlay;
		pProfile->m_bAutoswitch = m_bAutoswitch;

		pProfile->m_fMessageDisplay = (LTFLOAT)m_nMessageDisplay / 10.0f;
		pProfile->m_fMessageTime = ((LTFLOAT)m_nMessageTime / 5.0f) + 0.15f;
		pProfile->m_nConnection = (uint8)m_nConnection;
		pProfile->m_bIgnoreTaunts = m_bIgnoreTaunts;

		//if 4th difficulty level added, disable objective messages
		if (!g_pGameClientShell->IsInWorld() || g_pGameClientShell->GetGameType()->GetDifficulty() <= DIFFICULTY_PC_HARD)
		{
			pProfile->m_bObjectives = m_bObjectives;
		}

		pProfile->ApplyGame();
		pProfile->ApplyMultiplayer();
		pProfile->Save();

	}

	CBaseFolder::OnFocus(bFocus);
}

//////////////////////////////////////////////////////////////////////

void CFolderGame::BuildControlList()
{
	RemoveFree();

	switch(m_nCurTab)
	{
		// General
		case 0:
		{
			CSliderCtrl *pSlider=AddSlider(IDS_PICKUP_MSG_DUR, IDS_HELP_PICKUP_MSG_DUR, kGap, kWidth, &m_nPickupMsgDur);
			pSlider->SetSliderRange(0, 10);
			pSlider->SetSliderIncrement(1);

			CToggleCtrl *pToggle = AddToggle(IDS_AUTOSWITCH_WEAPONS, IDS_HELP_AUTOSWITCH_WEAPONS, kGap, &m_bAutoswitch);
			pToggle->SetOnString(IDS_YES);
			pToggle->SetOffString(IDS_NO);

			pToggle = AddToggle(IDS_ADVCONTROLS_RUNLOCK, IDS_HELP_RUNLOCK, kGap, &m_bAlwaysRun);
			pToggle->SetOnString(IDS_YES);
			pToggle->SetOffString(IDS_NO);

			pToggle = AddToggle(IDS_HEADBOB, IDS_HELP_HEADBOB, kGap, &m_bHeadBob);
			pToggle->SetOnString(IDS_YES);
			pToggle->SetOffString(IDS_NO);

			pToggle = AddToggle(IDS_HEAD_CANTING, IDS_HELP_HEAD_CANTING, kGap, &m_bHeadCanting);
			pToggle->SetOnString(IDS_YES);
			pToggle->SetOffString(IDS_NO);

			pToggle = AddToggle(IDS_WEAPONSWAY, IDS_HELP_WEAPONSWAY, kGap, &m_bWeaponSway);
			pToggle->SetOnString(IDS_YES);
			pToggle->SetOffString(IDS_NO);

			m_pObjToggle = AddToggle(IDS_OBJECTIVES,IDS_HELP_OBJECTIVES, kGap, &m_bObjectives);
			m_pObjToggle->SetOnString(IDS_YES);
			m_pObjToggle->SetOffString(IDS_NO);

			pToggle = AddToggle(IDS_ENABLE_HINTS,IDS_HELP_HINT,kGap,&m_bHint);
			pToggle->SetOnString(IDS_YES);
			pToggle->SetOffString(IDS_NO);

			pToggle = AddToggle(IDS_SUBTITLES,IDS_HELP_SUBTITLES, kGap, &m_bSubtitles);
			pToggle->SetOnString(IDS_YES);
			pToggle->SetOffString(IDS_NO);

			pToggle = AddToggle(IDS_ORIENT_OVERLAY, IDS_HELP_ORIENT_OVERLAY, kGap, &m_bOrientOverlay);
			pToggle->SetOnString(IDS_YES);
			pToggle->SetOffString(IDS_NO);

			break;
		}

		// Multiplayer
		case 1:
		{
			CSliderCtrl *pSlider=AddSlider(IDS_MESSAGE_DISPLAY, IDS_HELP_MESSAGE_DISPLAY, kGap, kWidth, &m_nMessageDisplay);
			pSlider->SetSliderRange(0, 10);
			pSlider->SetSliderIncrement(1);

			pSlider=AddSlider(IDS_MESSAGE_TIME, IDS_HELP_MESSAGE_TIME, kGap, kWidth, &m_nMessageTime);
			pSlider->SetSliderRange(0, 10);
			pSlider->SetSliderIncrement(1);

			// Connect speed (affects the 'UpdateRate' console variable)
			CCycleCtrl *pCycle = AddCycleItem(IDS_PLAYER_CONNECTION, IDS_HELP_PLAYER_CONNECTION, kGap, 0, &m_nConnection);
			pCycle->AddString(IDS_CONNECT_VSLOW);
			pCycle->AddString(IDS_CONNECT_SLOW);
			pCycle->AddString(IDS_CONNECT_MEDIUM);
			pCycle->AddString(IDS_CONNECT_FAST);
			pCycle->AddString(IDS_CONNECT_VFAST);

			// Taunt toggle control
			CToggleCtrl *pToggle = AddToggle(IDS_IGNORE_TAUNTS, IDS_HELP_IGNORE_TAUNTS, kGap, &m_bIgnoreTaunts);
			pToggle->SetOnString(IDS_YES);
			pToggle->SetOffString(IDS_NO);

			break;
		}
	}
}


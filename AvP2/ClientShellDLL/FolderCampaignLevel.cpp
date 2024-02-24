// FolderCampaignLevel.cpp: implementation of the CFolderCampaignLevel class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderCampaignLevel.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "MissionMgr.h"
#include "profileMgr.h"

#include "GameClientShell.h"


#include <algorithm>

extern CGameClientShell* g_pGameClientShell;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderCampaignLevel::CFolderCampaignLevel(Species species) 
	: m_Species(species), 
	  m_pListCtrl(LTNULL),
	  m_bShowAllMissions(LTFALSE)
{
}

CFolderCampaignLevel::~CFolderCampaignLevel()
{
}

// Build the folder
LTBOOL CFolderCampaignLevel::Build()
{

	CLTGUIFont *pFont = LTNULL;

	// Set the title's text
	switch (m_Species)
	{
	case Alien:
//		CreateTitle(IDS_TITLE_ALIEN);
		pFont = g_pInterfaceResMgr->GetAlienFont();
		break;
	case Marine:
//		CreateTitle(IDS_TITLE_MARINE);
		pFont = g_pInterfaceResMgr->GetMarineFont();
		break;
	case Predator:
//		CreateTitle(IDS_TITLE_PREDATOR);
		pFont = g_pInterfaceResMgr->GetPredatorFont();
		break;
	}

	g_pLayoutMgr->GetFolderCustomString((eFolderID)m_nFolderID,"ListSound",m_szListSound,sizeof(m_szListSound));
	g_pLayoutMgr->GetFolderCustomString((eFolderID)m_nFolderID,"LaunchSound",m_szLaunchSound,sizeof(m_szLaunchSound));

	LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"BackPos");
	CLTGUITextItemCtrl* pCtrl = CreateTextItem(IDS_BACK, FOLDER_CMD_BACK, LTNULL, LTFALSE, pFont);
	AddFixedControl(pCtrl,kBackGroup,pos, LTTRUE);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"MissionsPos");
	pCtrl = CreateTextItem(IDS_SELECT_MISSION, LTNULL, LTNULL, LTTRUE, pFont);
	AddFixedControl(pCtrl,kLinkGroup,pos, LTFALSE);

	// Make a list controller
	LTRect rect = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID,"MissionsRect");
	m_pListCtrl = new CListCtrl();
	m_pListCtrl->Create(rect.bottom-rect.top, LTTRUE, rect.right-rect.left);
	m_pListCtrl->EnableMouseClickSelect(LTTRUE);
	m_pListCtrl->UseHighlight(LTTRUE);
	m_pListCtrl->SetHighlightColor(m_hHighlightColor);
	m_pListCtrl->SetIndent(LTIntPt(25,4));


	pos = LTIntPt(rect.left,rect.top);
	AddFixedControl(m_pListCtrl,kCustomGroup0,pos);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"DifficultyPos");
	pCtrl = CreateTextItem(IDS_DIFFICULTY, LTNULL, LTNULL, LTTRUE,pFont);
	AddFixedControl(pCtrl,kNoGroup,pos, LTFALSE);


	// Make a Diff controller
	rect = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID,"DifficultyRect");
	m_pDiffCtrl = new CListCtrl();
	m_pDiffCtrl->Create(rect.bottom-rect.top, LTTRUE, rect.right-rect.left);
	m_pDiffCtrl->EnableMouseClickSelect(LTTRUE);
	m_pDiffCtrl->UseHighlight(LTTRUE);
	m_pDiffCtrl->SetHighlightColor(m_hHighlightColor);
	m_pDiffCtrl->SetIndent(LTIntPt(25,4));


	pCtrl = CreateTextItem(IDS_DIFF_EASY,FOLDER_CMD_CUSTOM,LTNULL,LTFALSE,pFont);
	m_pDiffCtrl->AddControl(pCtrl);

	pCtrl = CreateTextItem(IDS_DIFF_MEDIUM,FOLDER_CMD_CUSTOM,LTNULL,LTFALSE,pFont);
	m_pDiffCtrl->AddControl(pCtrl);

	pCtrl = CreateTextItem(IDS_DIFF_HARD,FOLDER_CMD_CUSTOM,LTNULL,LTFALSE,pFont);
	m_pDiffCtrl->AddControl(pCtrl);

	pCtrl = CreateTextItem(IDS_DIFF_INSANE,FOLDER_CMD_CUSTOM,LTNULL,LTFALSE,pFont);
	m_pDiffCtrl->AddControl(pCtrl);


	pos = LTIntPt(rect.left,rect.top);
	AddFixedControl(m_pDiffCtrl,kCustomGroup1,pos);

	// Add Launch button	
	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"LaunchPos");
	m_pLaunchCtrl = CreateTextItem(IDS_LAUNCH, FOLDER_CMD_LAUNCH, IDS_HELP_LAUNCH, LTFALSE, pFont);
	AddFixedControl(m_pLaunchCtrl,kNextGroup,pos);


	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;
	
	UseBack(LTFALSE);

	return LTTRUE;
}

void CFolderCampaignLevel::OnFocus(LTBOOL bFocus)
{
	CBaseFolder::OnFocus(bFocus);

	if (bFocus)
	{
		g_pProfileMgr->SetSpecies(m_Species);

		FillMissionList();

		m_pListCtrl->SelectItem(0);
		m_pDiffCtrl->SelectItem(g_pGameClientShell->GetGameType()->GetDifficulty());

		// Make the launch button the first one active
		int sel = GetIndex(m_pLaunchCtrl);
		SetSelection(sel);
	}
	else
	{
		ClearMissionList();
	}
}

uint32 CFolderCampaignLevel::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case FOLDER_CMD_CUSTOM:
		return 1;
		break;
	case FOLDER_CMD_LAUNCH:
		{
			const CampaignButes & campaign = g_pMissionMgr->GetCampaignButes(m_Species);

			g_pInterfaceMgr->PlayInterfaceSound(IS_SELECT);
			g_pGameClientShell->GetGameType()->SetDifficulty(m_pDiffCtrl->GetSelectedItem());

			g_pInterfaceMgr->GetPlayerStats()->ResetInventory();
			int missionNum = m_pListCtrl->GetSelectedItem();

			int nPrevMissionId = g_pMissionMgr->GetSetFromNextWorld( const_cast<char*>(campaign.m_Missions[missionNum].m_szWorld) );
			if( nPrevMissionId < 0 )
			{
				g_pInterface->CPrint("FolderCampaignLevel: Could not find set %s! Defaulting to index zero!", campaign.m_Missions[missionNum].m_szWorld);
				nPrevMissionId = 0;
			}

			g_pInterfaceMgr->StartMission(nPrevMissionId);

			break;
		}
	case FOLDER_CMD_SINGLE_PLAYER:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_SINGLE);
			break;
		}
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}

	return 1;
};


void CFolderCampaignLevel::FillMissionList()
{
	CLTGUIFont *pFont = LTNULL;
	CLTGUITextItemCtrl *pCtrl = LTNULL;
	int nCompleted;
	DWORD dwVal, dwDigit;

	const CampaignButes & campaign = g_pMissionMgr->GetCampaignButes(m_Species);

	ClearMissionList();

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (!pProfile)
	{
		ASSERT(!"User profile not found - level menu not loaded");
		return;
	}

	// Set the title's text
	switch (m_Species)
	{
	case Alien:
		dwVal = g_pInterfaceMgr->DecodeStatus(pProfile->m_LevelStatus.Alien);
		pFont = g_pInterfaceResMgr->GetAlienFont();
		break;
	case Marine:
		dwVal = g_pInterfaceMgr->DecodeStatus(pProfile->m_LevelStatus.Marine);
		pFont = g_pInterfaceResMgr->GetMarineFont();
		break;
	case Predator:
		dwVal = g_pInterfaceMgr->DecodeStatus(pProfile->m_LevelStatus.Predator);
		pFont = g_pInterfaceResMgr->GetPredatorFont();
		break;
	}

	// First mission is always available
	pCtrl = CreateTextItem(campaign.m_Missions[0].m_nNameId,FOLDER_CMD_CUSTOM,LTNULL,LTFALSE,pFont);
	m_pListCtrl->AddControl(pCtrl);

	// Find the number of missions completed so far
	for (nCompleted = 0; nCompleted < campaign.m_nNumMissions - 1; nCompleted++)
	{
		dwDigit = g_pInterfaceMgr->GetDigit(dwVal, nCompleted);
		if ((dwDigit <= 3) || m_bShowAllMissions)
		{
			// looks ok... add it
			// TODO: possibly add special difficulty-based icons here
			pCtrl = CreateTextItem(campaign.m_Missions[nCompleted+1].m_nNameId,FOLDER_CMD_CUSTOM,LTNULL,LTFALSE,pFont);
			m_pListCtrl->AddControl(pCtrl);
		}
		else
		{
			// can't complete missions out of sequence
			break;
		}
	}
}

void CFolderCampaignLevel::ClearMissionList()
{
	m_pListCtrl->RemoveAllControls();
}

char* CFolderCampaignLevel::GetChangeSound()	
{
	if (GetSelectedControl() == m_pListCtrl || GetSelectedControl() == m_pDiffCtrl)
		return m_szListSound;
	return m_sDefaultChangeSound;
}
char* CFolderCampaignLevel::GetSelectSound()	
{
	if (GetSelectedControl() == m_pListCtrl || GetSelectedControl() == m_pDiffCtrl)
		return m_szListSound;
	if (GetSelectedControl() == m_pLaunchCtrl)
		return m_szLaunchSound;
	return m_sDefaultSelectSound;
}

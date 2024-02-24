// FolderProfile.cpp: implementation of the CFolderProfile class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderProfile.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
#include "profileMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace
{
	enum eLocalCommands
	{
		CMD_LOAD = FOLDER_CMD_CUSTOM+1,
		CMD_CONFIRM,
		CMD_DELETE,
		CMD_CREATE,
		CMD_RENAME,
		CMD_EDIT,
	};

	const int kDlgWidth = 360;
	const int kDlgHeight = 60;

	LTBOOL	bCreate;
	char	szOldFN[MAX_PROFILE_LEN];

	void DeleteCallBack(LTBOOL bReturn, void *pData)
	{
		CFolderProfile *pThisFolder = (CFolderProfile *)pData;
		if (bReturn && pThisFolder)
			pThisFolder->SendCommand(CMD_DELETE,0,0);
	}

}

CFolderProfile::CFolderProfile()
{
	m_pEdit = LTNULL;
	m_pCurrent = LTNULL;
	m_pLoad = LTNULL;
	m_pDelete = LTNULL;
	m_pRename = LTNULL;
	m_pProfile = LTNULL;
	m_pDlgBack = LTNULL;
	m_pDlgGroup = LTNULL;

}

CFolderProfile::~CFolderProfile()
{
}

// Build the folder
LTBOOL CFolderProfile::Build()
{
	// Set the title's text
	CreateTitle(IDS_TITLE_PROFILE);

	// Get edit controls position and create it.
	LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"ProfileNamePos");
	CLTGUITextItemCtrl* pCtrl = CreateTextItem(IDS_CURRENT_PROFILE, LTNULL, LTNULL, LTTRUE);
	AddFixedControl(pCtrl,kNoGroup,pos, LTFALSE);

	pos.y += pCtrl->GetHeight() + 4;
	pos.x += 40;

	m_pCurrent = CreateTextItem(" ", LTNULL, LTNULL, LTTRUE);
	AddFixedControl(m_pCurrent,kNoGroup,pos, LTFALSE);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"ProfileListPos");
	LTRect rect = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID,"ProfileListRect");

	AddFrame(pos.x,(pos.y-2),(rect.right-rect.left) + 22,(rect.bottom-pos.y)+2,(rect.top-pos.y)+2);

	pCtrl = CreateTextItem(IDS_PROFILE_LIST, LTNULL, LTNULL, LTTRUE);
	AddFixedControl(pCtrl,kNoGroup,pos, LTFALSE);

	// Make a list controller
	m_pListCtrl = new CListCtrl();
	m_pListCtrl->Create(rect.bottom-rect.top, LTTRUE, rect.right-rect.left);
	m_pListCtrl->EnableMouseClickSelect(LTTRUE);
	m_pListCtrl->UseHighlight(LTTRUE);
	m_pListCtrl->SetHighlightColor(m_hHighlightColor);
	m_pListCtrl->SetIndent(LTIntPt(8,8));

	pos = LTIntPt(rect.left,rect.top);
	AddFixedControl(m_pListCtrl,kCustomGroup0,pos);

	rect = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID,"ControlRect");
	AddFrame(rect.left,rect.top,(rect.right-rect.left),(rect.bottom-rect.top));

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"CreatePos");
	pCtrl = CreateTextItem(IDS_CREATE, CMD_CREATE, IDS_HELP_PROFILE_CREATE);
	AddFixedControl(pCtrl,kCustomGroup1,pos, LTTRUE);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"LoadPos");
	m_pLoad = CreateTextItem(IDS_LOAD, CMD_LOAD, IDS_HELP_PROFILE_LOAD);
	AddFixedControl(m_pLoad,kCustomGroup1,pos, LTTRUE);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"RenamePos");
	m_pRename = CreateTextItem(IDS_RENAME, CMD_RENAME, IDS_HELP_PROFILE_RENAME);
	AddFixedControl(m_pRename,kCustomGroup1,pos, LTTRUE);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"DeletePos");
	m_pDelete = CreateTextItem(IDS_DELETE, CMD_CONFIRM, IDS_HELP_PROFILE_DELETE);
	AddFixedControl(m_pDelete,kCustomGroup1,pos, LTTRUE);


	BuildDlg();

	// Make sure to call the base class
	return CBaseFolder::Build();
}

uint32 CFolderProfile::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch (dwCommand)
	{
	case CMD_LOAD:
		if (m_pListCtrl->GetSelectedItem() != CListCtrl::kNoSelection) 
		{
			StringSet::iterator iter = m_ProfileList.begin();
			for (int i = 0; i < m_pListCtrl->GetSelectedItem() && iter != m_ProfileList.end(); ++i, ++iter);

			if (iter != m_ProfileList.end())
			{
				std::string profile = *iter;
				g_pProfileMgr->NewProfile(profile);
				m_pProfile = g_pProfileMgr->GetCurrentProfile();

				SAFE_STRCPY(m_szProfile, profile.c_str());
				UpdateProfileName();
			}
		}
		break;
	case CMD_CONFIRM:
		{
            HSTRING hString = g_pLTClient->FormatString(IDS_CONFIRM_DELETE);
			g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,DeleteCallBack,this);
			g_pLTClient->FreeString(hString);
			break;
		}
	case CMD_DELETE:
		if (m_pListCtrl->GetSelectedItem() != CListCtrl::kNoSelection) 
		{
			StringSet::iterator iter = m_ProfileList.begin();
			for (int i = 0; i < m_pListCtrl->GetSelectedItem() && iter != m_ProfileList.end(); ++i, ++iter);

			if (iter != m_ProfileList.end())
			{
				std::string profile = *iter;
				g_pProfileMgr->DeleteProfile(profile);

			}
			m_pProfile = g_pProfileMgr->GetCurrentProfile();
			SAFE_STRCPY(m_szProfile, m_pProfile->m_sName.c_str());
			UpdateProfileName();

		}
		break;
	case CMD_CREATE:
		{
			SAFE_STRCPY(m_szOldProfile,m_szProfile);
			bCreate = LTTRUE;
			ShowDlg();
		}
		break;
	case CMD_RENAME:
		{
			SAFE_STRCPY(m_szOldProfile,m_szProfile);
			if (m_pListCtrl->GetSelectedItem() != CListCtrl::kNoSelection) 
			{
				StringSet::iterator iter = m_ProfileList.begin();
				for (int i = 0; i < m_pListCtrl->GetSelectedItem() && iter != m_ProfileList.end(); ++i, ++iter);

				if (iter != m_ProfileList.end())
				{
					std::string profile = *iter;
					SAFE_STRCPY(szOldFN,profile.c_str());
					SAFE_STRCPY(m_szProfile, profile.c_str());
					bCreate = LTFALSE;
					ShowDlg();
				}
			

			}

		}
		break;
	case CMD_EDIT:
		{
			HideDlg(LTTRUE);

			std::string profileName = m_szProfile;
			StringSet::iterator iter = m_ProfileList.find(profileName);
			if (iter != m_ProfileList.end())
				profileName = (*iter);

			if (bCreate)
			{

				g_pProfileMgr->NewProfile(profileName);

			}
			else
			{
				g_pProfileMgr->RenameProfile(szOldFN,profileName);
			}
			m_pProfile = g_pProfileMgr->GetCurrentProfile();
			SAFE_STRCPY(m_szProfile,m_pProfile->m_sName.c_str());
			UpdateProfileName();
		}
		break;
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}

	return 1;
};

void CFolderProfile::Escape()
{
	if (GetCapture() == m_pEdit)
	{
		HideDlg(LTFALSE);
		SAFE_STRCPY(m_szProfile,m_szOldProfile);
	}
	else
		CBaseFolder::Escape();

}

LTBOOL	CFolderProfile::OnLButtonUp(int x, int y)
{
	if (GetCapture())
	{
		Escape();
		return LTTRUE;
	}
	return CBaseFolder::OnLButtonUp(x,y);
}

LTBOOL	CFolderProfile::OnRButtonUp(int x, int y)
{
	if (GetCapture())
	{
		Escape();
		return LTTRUE;
	}
	return CBaseFolder::OnRButtonUp(x,y);
}

void CFolderProfile::CreateProfileList()
{
	// Empty the list
	m_pListCtrl->RemoveAllControls();

	// Get new stuff
	m_ProfileList.clear();
	g_pProfileMgr->GetProfileList(m_ProfileList);

	// Profiles to the list
	for (StringSet::iterator iter = m_ProfileList.begin(); iter != m_ProfileList.end(); ++iter)
	{
		CLTGUITextItemCtrl* pTextCtrl = CreateTextItem(iter->c_str(), LTNULL, LTNULL, LTFALSE, GetTinyFont());
		int ndx = m_pListCtrl->AddControl(pTextCtrl);

		if (stricmp(iter->c_str(),m_pProfile->m_sName.c_str()) == 0)
			m_pListCtrl->SelectItem(ndx);
	}

}

void CFolderProfile::UpdateProfileName()
{
	m_pCurrent->RemoveAll();
	HSTRING hTemp = g_pLTClient->CreateString(m_szProfile);
	m_pCurrent->AddString(hTemp);
	g_pLTClient->FreeString(hTemp);

	CreateProfileList();

}

void CFolderProfile::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_pProfile = g_pProfileMgr->GetCurrentProfile();
		SAFE_STRCPY(m_szProfile,m_pProfile->m_sName.c_str());
		UpdateProfileName();

	}
	CBaseFolder::OnFocus(bFocus);
}



void CFolderProfile::BuildDlg()
{
	LTIntPt	pos(0,0);

	m_pDlgGroup = CreateGroup(kDlgWidth,kDlgHeight,LTNULL);
	m_pDlgGroup->AllowSubSelection(LTTRUE);
	m_pDlgGroup->Enable(LTTRUE);

	m_pDlgBack = new CFrameCtrl();
	m_pDlgBack->Create(g_pLTClient,kNearBlack,0.75f,kDlgWidth,kDlgHeight);
	m_pDlgGroup->AddControl(m_pDlgBack,pos);


	pos.x = 8;
	pos.y += 8;
	CLTGUITextItemCtrl*	pCtrl = CreateTextItem(IDS_PROFILE_NAME, LTNULL, LTNULL, LTTRUE);
	m_pDlgGroup->AddControl(pCtrl,pos, LTFALSE);

	pos.x = 8;
	pos.y += pCtrl->GetHeight();
	m_pEdit = CreateEditCtrl(" ", CMD_EDIT, LTNULL, m_szProfile, sizeof(m_szProfile), 24, LTTRUE);
	m_pEdit->EnableCursor();
    m_pEdit->Enable(LTFALSE);
	m_pEdit->SetInputMode(CLTGUIEditCtrl::kInputFileFriendly);
	m_pDlgGroup->AddControl(m_pEdit,pos, LTTRUE);

}

void CFolderProfile::ShowDlg()
{
	m_pEdit->UpdateData(LTFALSE);

	LTIntPt pos(0,0);

	pos.x = 320 - kDlgWidth / 2;
	pos.y = 240 - kDlgHeight / 2;

	AddFixedControl(m_pDlgGroup,kFreeGroup,pos,LTTRUE);

	SetSelection(GetIndex(m_pDlgGroup));

	SetCapture(m_pEdit);
	m_pEdit->SetColor(m_hSelectedColor,m_hSelectedColor,m_hSelectedColor);
	m_pEdit->Select(LTTRUE);

}

void CFolderProfile::HideDlg(LTBOOL bOK)
{
	m_pEdit->UpdateData(bOK);

	SetSelection(kNoSelection);

	RemoveFixedControl(m_pDlgGroup);

	SetCapture(LTNULL);
	ForceMouseUpdate();
}


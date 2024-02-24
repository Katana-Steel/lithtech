// FolderHostConfig.cpp: implementation of the CFolderHostConfig class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderHostConfig.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
#include "ServerOptionMgr.h"

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
	char	szOldFN[MAX_CONFIG_LEN];

	void DeleteCallBack(LTBOOL bReturn, void *pData)
	{
		CFolderHostConfig *pThisFolder = (CFolderHostConfig *)pData;
		if (bReturn && pThisFolder)
			pThisFolder->SendCommand(CMD_DELETE,0,0);
	}


}

CFolderHostConfig::CFolderHostConfig()
{
	m_pEdit = LTNULL;
	m_pCurrent = LTNULL;
	m_pLoad = LTNULL;
	m_pDelete = LTNULL;
	m_pRename = LTNULL;
	m_pConfig = LTNULL;
	m_pDlgBack = LTNULL;
	m_pDlgGroup = LTNULL;
}

CFolderHostConfig::~CFolderHostConfig()
{

}

// Build the folder
LTBOOL CFolderHostConfig::Build()
{
	// Set the title's text
	CreateTitle(IDS_TITLE_CONFIG);
	UseLogo(LTFALSE);

	// Get edit controls position and create it.
	LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"ConfigNamePos");
	m_pCurrent = CreateTextItem(" ", LTNULL, LTNULL, LTTRUE);
	AddFixedControl(m_pCurrent,kNoGroup,pos, LTFALSE);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"ConfigListPos");
	CLTGUITextItemCtrl* pCtrl = CreateTextItem(IDS_CONFIG_LIST, LTNULL, LTNULL, LTTRUE);
	AddFixedControl(pCtrl,kNoGroup,pos, LTFALSE);

	// Make a list controller
	LTRect rect = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID,"ConfigListRect");
	m_pListCtrl = new CListCtrl();
	m_pListCtrl->Create(rect.bottom-rect.top, LTTRUE, rect.right-rect.left);
	m_pListCtrl->EnableMouseClickSelect(LTTRUE);
	m_pListCtrl->UseHighlight(LTTRUE);
	m_pListCtrl->SetHighlightColor(m_hHighlightColor);
	m_pListCtrl->SetIndent(LTIntPt(25,4));

	pos = LTIntPt(rect.left,rect.top);
	AddFixedControl(m_pListCtrl,kCustomGroup0,pos);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"CreatePos");
	pCtrl = CreateTextItem(IDS_CREATE, CMD_CREATE, IDS_HELP_CONFIG_CREATE);
	AddFixedControl(pCtrl,kCustomGroup1,pos, LTTRUE);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"LoadPos");
	m_pLoad = CreateTextItem(IDS_LOAD, CMD_LOAD, IDS_HELP_CONFIG_LOAD);
	AddFixedControl(m_pLoad,kCustomGroup1,pos, LTTRUE);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"RenamePos");
	m_pRename = CreateTextItem(IDS_RENAME, CMD_RENAME, IDS_HELP_CONFIG_RENAME);
	AddFixedControl(m_pRename,kCustomGroup1,pos, LTTRUE);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"DeletePos");
	m_pDelete = CreateTextItem(IDS_DELETE, CMD_CONFIRM, IDS_HELP_CONFIG_DELETE);
	AddFixedControl(m_pDelete,kCustomGroup1,pos, LTTRUE);


	BuildDlg();

	// Make sure to call the base class
	return CBaseFolder::Build();
}

uint32 CFolderHostConfig::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch (dwCommand)
	{
	case CMD_LOAD:
		if (m_pListCtrl->GetSelectedItem() != CListCtrl::kNoSelection) 
		{
			StringSet::iterator iter = m_ConfigNames.begin();
			for (int i = 0; i < m_pListCtrl->GetSelectedItem() && iter != m_ConfigNames.end(); ++i, ++iter);

			if (iter != m_ConfigNames.end())
			{
				std::string config = *iter;
				g_pServerOptionMgr->SetCurrentCfg(config.c_str());
				m_pConfig = g_pServerOptionMgr->GetCurrentCfg();

				SAFE_STRCPY(m_szConfig, config.c_str());
				UpdateConfigName();
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
			StringSet::iterator iter = m_ConfigNames.begin();
			for (int i = 0; i < m_pListCtrl->GetSelectedItem() && iter != m_ConfigNames.end(); ++i, ++iter);

			if (iter != m_ConfigNames.end())
			{
				std::string config = *iter;
				g_pServerOptionMgr->DeleteCfg(config.c_str());

			}
			m_pConfig = g_pServerOptionMgr->GetCurrentCfg();
			SAFE_STRCPY(m_szConfig, m_pConfig->szCfgName);
			UpdateConfigName();

		}
		break;
	case CMD_CREATE:
		{
			SAFE_STRCPY(m_szOldConfig,m_szConfig);
			bCreate = LTTRUE;
			ShowDlg();
		}
		break;
	case CMD_RENAME:
		{
			SAFE_STRCPY(m_szOldConfig,m_szConfig);
			if (m_pListCtrl->GetSelectedItem() != CListCtrl::kNoSelection) 
			{
				StringSet::iterator iter = m_ConfigNames.begin();
				for (int i = 0; i < m_pListCtrl->GetSelectedItem() && iter != m_ConfigNames.end(); ++i, ++iter);

				if (iter != m_ConfigNames.end())
				{
					std::string config = *iter;
					SAFE_STRCPY(szOldFN,config.c_str());
					SAFE_STRCPY(m_szConfig, config.c_str());
					bCreate = LTFALSE;
					ShowDlg();
				}
			

			}

		}
		break;
	case CMD_EDIT:
		{
			HideDlg(LTTRUE);

			std::string ConfigName = m_szConfig;
			StringSet::iterator iter = m_ConfigNames.find(ConfigName);
			if (iter != m_ConfigNames.end())
				ConfigName = (*iter);

			if (bCreate)
			{
				g_pServerOptionMgr->SetCurrentCfg(ConfigName.c_str());

			}
			else
			{
				g_pServerOptionMgr->RenameCfg(szOldFN,ConfigName.c_str());
			}
			m_pConfig = g_pServerOptionMgr->GetCurrentCfg();
			SAFE_STRCPY(m_szConfig,m_pConfig->szCfgName);
			UpdateConfigName();
		}
		break;
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}

	return 1;
};

void CFolderHostConfig::Escape()
{
	if (GetCapture() == m_pEdit)
	{
		HideDlg(LTFALSE);
		SAFE_STRCPY(m_szConfig,m_szOldConfig);
	}
	else
		CBaseFolder::Escape();

}

LTBOOL	CFolderHostConfig::OnLButtonUp(int x, int y)
{
	if (GetCapture())
	{
		Escape();
		return LTTRUE;
	}
	return CBaseFolder::OnLButtonUp(x,y);
}

LTBOOL	CFolderHostConfig::OnRButtonUp(int x, int y)
{
	if (GetCapture())
	{
		Escape();
		return LTTRUE;
	}
	return CBaseFolder::OnRButtonUp(x,y);
}

void CFolderHostConfig::CreateConfigList()
{
	// Empty the list
	m_pListCtrl->RemoveAllControls();

	// Get new stuff
	m_ConfigNames.clear();
	g_pServerOptionMgr->GetConfigNames(m_ConfigNames);

	// Configs to the list
	for (StringSet::iterator iter = m_ConfigNames.begin(); iter != m_ConfigNames.end(); ++iter)
	{
		CLTGUITextItemCtrl* pTextCtrl = CreateTextItem(iter->c_str(), LTNULL, LTNULL, LTFALSE);
		int ndx = m_pListCtrl->AddControl(pTextCtrl);
		if (stricmp(iter->c_str(),m_pConfig->szCfgName) == 0)
			m_pListCtrl->SelectItem(ndx);

	}

}

void CFolderHostConfig::UpdateConfigName()
{
	m_pCurrent->RemoveAll();
	HSTRING hTemp = g_pLTClient->FormatString(IDS_CURRENT_CONFIG,m_szConfig);
	m_pCurrent->AddString(hTemp);
	g_pLTClient->FreeString(hTemp);

	CreateConfigList();

}

void CFolderHostConfig::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_pConfig = g_pServerOptionMgr->GetCurrentCfg();
		SAFE_STRCPY(m_szConfig,m_pConfig->szCfgName);
		UpdateConfigName();
	}
	CBaseFolder::OnFocus(bFocus);
}



void CFolderHostConfig::BuildDlg()
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
	CLTGUITextItemCtrl*	pCtrl = CreateTextItem(IDS_CONFIG_NAME, LTNULL, LTNULL, LTTRUE);
	m_pDlgGroup->AddControl(pCtrl,pos, LTFALSE);

	pos.x = 8;
	pos.y += pCtrl->GetHeight();
	m_pEdit = CreateEditCtrl(" ", CMD_EDIT, LTNULL, m_szConfig, sizeof(m_szConfig), 24, LTTRUE);
	m_pEdit->EnableCursor();
    m_pEdit->Enable(LTFALSE);
	m_pEdit->SetInputMode(CLTGUIEditCtrl::kInputAlphaNumeric);
	m_pDlgGroup->AddControl(m_pEdit,pos, LTTRUE);

}

void CFolderHostConfig::ShowDlg()
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

void CFolderHostConfig::HideDlg(LTBOOL bOK)
{
	m_pEdit->UpdateData(bOK);

	SetSelection(kNoSelection);

	RemoveFixedControl(m_pDlgGroup);

	SetCapture(LTNULL);
	ForceMouseUpdate();
}


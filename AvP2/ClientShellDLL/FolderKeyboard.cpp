// FolderKeyboard.cpp: implementation of the CFolderKeyboard class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderKeyboard.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "FolderCustomControls.h"
#include "ClientRes.h"
#include "GameSettings.h"
#include "InterfaceMgr.h"
#include "GameSettings.h"
#include "profileMgr.h"

namespace
{
	int kGap = 200;
	int kWidth = 200;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderKeyboard::CFolderKeyboard()
{
    m_pLookCtrl = LTNULL;

    m_bAutoCenter = LTFALSE;

    m_bMouseLook = LTFALSE;
	m_nNormalTurn	= 15;
	m_nFastTurn		= 23;
	m_nLookUp		= 25;

}

CFolderKeyboard::~CFolderKeyboard()
{

}

// Build the folder
LTBOOL CFolderKeyboard::Build()
{
	CreateTitle(IDS_TITLE_CONTROLS);
	
	kGap = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_KEYBOARD,"ColumnWidth");
	kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_KEYBOARD,"SliderWidth");
	g_pLayoutMgr->GetFolderCustomString((eFolderID)m_nFolderID,"TabSound",m_szTabSound,sizeof(m_szTabSound));

	//turn speed
	CSliderCtrl *pSlider = AddSlider(IDS_NORMALTURN, IDS_HELP_NORMALTURN, kGap, kWidth, &m_nNormalTurn);
	pSlider->SetSliderRange(5, 50);
	pSlider->SetSliderIncrement(5);

	pSlider = AddSlider(IDS_FASTTURN, IDS_HELP_FASTTURN, kGap, kWidth, &m_nFastTurn);
	pSlider->SetSliderRange(8, 53);
	pSlider->SetSliderIncrement(5);

	// look speed
	pSlider = AddSlider(IDS_LOOKUP, IDS_HELP_LOOKUP, kGap, kWidth, &m_nLookUp);
	pSlider->SetSliderRange(5, 50);
	pSlider->SetSliderIncrement(5);

	// auto center
	m_pLookCtrl = AddToggle(IDS_MOUSE_AUTOCENTER, IDS_HELP_AUTOCENTER, kGap, &m_bAutoCenter );
	m_pLookCtrl->SetOnString(IDS_YES);
	m_pLookCtrl->SetOffString(IDS_NO);

	m_nNextLink = 2;
	AddLink(IDS_CUSTOM_CONTROLS, FOLDER_CMD_CONTROLS, IDS_HELP_CUSTOMCONTROLS);
	AddLink(IDS_MOUSE, FOLDER_CMD_MOUSE, IDS_HELP_MOUSE);

	CLTGUITextItemCtrl* pCtrl = AddLink(IDS_KEYBOARD, LTNULL, LTNULL);
	pCtrl->Enable(LTFALSE);

	AddLink(IDS_JOYSTICK, FOLDER_CMD_JOYSTICK, IDS_HELP_JOYSTICK);


	LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_KEYBOARD,"RestorePos");
	pCtrl = CreateTextItem(IDS_RESTOREDEFAULTS, FOLDER_CMD_RESET_DEFAULTS, IDS_HELP_RESTORE, LTFALSE, GetSmallFont());
	AddFixedControl(pCtrl,kNextGroup,pos);

	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	return LTTRUE;

}

uint32 CFolderKeyboard::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
		case FOLDER_CMD_UPDATE:
		{
			UpdateData();
			m_pLookCtrl->Enable(!m_bMouseLook);
			break;
		}

		case FOLDER_CMD_RESET_DEFAULTS:
		{
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

			if (pProfile)
			{
				pProfile->RestoreDefaults(RESTORE_KEYBOARD);
				pProfile->SetKeyboard();

				m_pLookCtrl->Enable(!pProfile->m_bMouseLook);
				m_bAutoCenter = pProfile->m_bAutoCenter;
				m_nNormalTurn = pProfile->m_nNormalTurn;
				m_nFastTurn = pProfile->m_nFastTurn;
				m_nLookUp = pProfile->m_nLookUp;

				UpdateData(LTFALSE);
			}

			break;
		}

		default:
			return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);

	}
	return 1;
};



// Change in focus
void CFolderKeyboard::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{

		if (pProfile)
		{
			pProfile->SetKeyboard();
			m_pLookCtrl->Enable(!pProfile->m_bMouseLook);
			m_bAutoCenter = pProfile->m_bAutoCenter;
			m_nNormalTurn = pProfile->m_nNormalTurn;
			m_nFastTurn = pProfile->m_nFastTurn;
			m_nLookUp = pProfile->m_nLookUp;
		}

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		if (pProfile)
		{
			pProfile->m_bAutoCenter = m_bAutoCenter;
			pProfile->m_nNormalTurn = m_nNormalTurn;
			pProfile->m_nFastTurn = m_nFastTurn;
			pProfile->m_nLookUp = m_nLookUp;

			pProfile->ApplyKeyboard();
		}
	}
	CBaseFolder::OnFocus(bFocus);
}

void CFolderKeyboard::Escape()
{
	g_pInterfaceMgr->PlayInterfaceSound(IS_ESCAPE);
	g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_OPTIONS);
}

char* CFolderKeyboard::GetSelectSound()	
{
	uint32 dwID = 0;
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl)
		dwID = pCtrl->GetHelpID();
	if (dwID == IDS_HELP_CUSTOMCONTROLS || dwID == IDS_HELP_MOUSE || dwID == IDS_HELP_KEYBOARD || dwID == IDS_HELP_JOYSTICK)
		return m_szTabSound;
	return m_sDefaultSelectSound;
}

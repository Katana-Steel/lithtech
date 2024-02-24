// FolderMouse.cpp: implementation of the CFolderMouse class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderMouse.h"
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

CFolderMouse::CFolderMouse()
{

    m_bInvertY = LTFALSE;
    m_bMouseLook = LTFALSE;
	m_nInputRate = 0;
	m_nSensitivity = 0;

    m_pSensitivityCtrl = LTNULL;
    m_pInputCtrl = LTNULL;


}

CFolderMouse::~CFolderMouse()
{

}

// Build the folder
LTBOOL CFolderMouse::Build()
{
	CreateTitle(IDS_TITLE_CONTROLS);
	
	kGap = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_MOUSE,"ColumnWidth");
	kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_MOUSE,"SliderWidth");
	g_pLayoutMgr->GetFolderCustomString((eFolderID)m_nFolderID,"TabSound",m_szTabSound,sizeof(m_szTabSound));

	//Always mouse look
	CToggleCtrl* pToggle = AddToggle(IDS_MOUSE_MOUSELOOK, IDS_HELP_MOUSELOOK, kGap, &m_bMouseLook );
	pToggle->SetOnString(IDS_YES);
	pToggle->SetOffString(IDS_NO);

	//invert mouse axis
	pToggle = AddToggle(IDS_MOUSE_INVERTYAXIS, IDS_HELP_INVERTY, kGap, &m_bInvertY );
	pToggle->SetOnString(IDS_YES);
	pToggle->SetOffString(IDS_NO);

	//mouse sensitivity
	m_pSensitivityCtrl=AddSlider(IDS_MOUSE_SENSITIVITY, IDS_HELP_MOUSE_SENSE, kGap, kWidth, &m_nSensitivity);
	m_pSensitivityCtrl->SetSliderRange(0, 20);
	m_pSensitivityCtrl->SetSliderIncrement(1);

	//mouse responsiveness
	m_pInputCtrl=AddSlider(IDS_MOUSE_INPUTRATE, IDS_HELP_MOUSE_INPUT, kGap, kWidth, &m_nInputRate);
	m_pInputCtrl->SetSliderRange(0, 100);
	m_pInputCtrl->SetSliderIncrement(5);

	m_nNextLink = 2;
	AddLink(IDS_CUSTOM_CONTROLS, FOLDER_CMD_CONTROLS, IDS_HELP_CUSTOMCONTROLS);

	CLTGUITextItemCtrl* pCtrl = AddLink(IDS_MOUSE, LTNULL, LTNULL);
	pCtrl->Enable(LTFALSE);

	AddLink(IDS_KEYBOARD, FOLDER_CMD_KEYBOARD, IDS_HELP_KEYBOARD);
	AddLink(IDS_JOYSTICK, FOLDER_CMD_JOYSTICK, IDS_HELP_JOYSTICK);


	LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_MOUSE,"RestorePos");
	pCtrl = CreateTextItem(IDS_RESTOREDEFAULTS, FOLDER_CMD_RESET_DEFAULTS, IDS_HELP_RESTORE, LTFALSE, GetSmallFont());
	AddFixedControl(pCtrl,kNextGroup,pos);


	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	return LTTRUE;

}

uint32 CFolderMouse::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
		case FOLDER_CMD_RESET_DEFAULTS:
		{
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

			if (pProfile)
			{
				pProfile->RestoreDefaults(RESTORE_MOUSE);
				pProfile->SetMouse();

				m_bMouseLook = pProfile->m_bMouseLook;
				m_bInvertY = pProfile->m_bInvertY;
				m_nInputRate = pProfile->m_nInputRate;
				m_nSensitivity = pProfile->m_nSensitivity;

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
void CFolderMouse::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		if (pProfile)
		{
			pProfile->SetMouse();
			m_bMouseLook = pProfile->m_bMouseLook;
			m_bInvertY = pProfile->m_bInvertY;
			m_nInputRate = pProfile->m_nInputRate;
			m_nSensitivity = pProfile->m_nSensitivity;
		}

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		if (pProfile)
		{
			pProfile->m_bMouseLook = m_bMouseLook;
			pProfile->m_bInvertY = m_bInvertY;
			pProfile->m_nInputRate = m_nInputRate;
			pProfile->m_nSensitivity = m_nSensitivity;

			pProfile->ApplyMouse();
		}

	}
	CBaseFolder::OnFocus(bFocus);
}

void CFolderMouse::Escape()
{
	g_pInterfaceMgr->PlayInterfaceSound(IS_ESCAPE);
	g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_OPTIONS);
}

char* CFolderMouse::GetSelectSound()	
{
	uint32 dwID = 0;
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl)
		dwID = pCtrl->GetHelpID();
	if (dwID == IDS_HELP_CUSTOMCONTROLS || dwID == IDS_HELP_MOUSE || dwID == IDS_HELP_KEYBOARD || dwID == IDS_HELP_JOYSTICK)
		return m_szTabSound;
	return m_sDefaultSelectSound;
}

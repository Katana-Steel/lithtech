// FolderJoystick.cpp: implementation of the CFolderJoystick class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderJoystick.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "ProfileMgr.h"


#include "InterfaceMgr.h"
#include "GameSettings.h"

namespace
{
	int kWidth = 0;
	int kSlider = 0;
	LTBOOL bOldUse = LTFALSE;

	enum eLocalCommands
	{
		CMD_STRAFE = FOLDER_CMD_CUSTOM+1,
		CMD_MOVE,
		CMD_LOOK,
		CMD_TURN,
		CMD_ACTIVE
	};
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderJoystick::CFolderJoystick()
{
    m_bUseJoystick=0;
	m_pAxisCycle = LTNULL;
	m_pInvertToggle = LTNULL;
	m_pDeadZoneSlider = LTNULL;
	m_pAnalogToggle = LTNULL;
	m_pSensitivitySlider = LTNULL;
	m_pCenterToggle = LTNULL;
	m_pFixedToggle = LTNULL;
	m_bFixedPositionLook = LTFALSE;
	m_dwCurrentAxis = 0;
}

CFolderJoystick::~CFolderJoystick()
{
}

// Build the folder
LTBOOL CFolderJoystick::Build()
{
	kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_JOYSTICK,"ColumnWidth");
	kSlider = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_JOYSTICK,"SliderWidth");
	g_pLayoutMgr->GetFolderCustomString((eFolderID)m_nFolderID,"TabSound",m_szTabSound,sizeof(m_szTabSound));


	CreateTitle(IDS_TITLE_CONTROLS);

	int nWidth = GetPageLeft() - m_TabPos[0].x;
	CToggleCtrl *pToggle = CreateToggle(IDS_JOYSTICK_USE, IDS_HELP_USEJOYSTICK, nWidth, &m_bUseJoystick);
	pToggle->SetOnString(IDS_YES);
	pToggle->SetOffString(IDS_NO);
	AddFixedControl(pToggle,kTabGroup,m_TabPos[0]);

	m_nNumTabs++;
	AddTab(IDS_JOYSTICK_STRAFE,CMD_STRAFE,IDS_HELP_JOYSTICK_STRAFE);
	AddTab(IDS_JOYSTICK_MOVE,CMD_MOVE,IDS_HELP_JOYSTICK_MOVE);
	AddTab(IDS_JOYSTICK_LOOK,CMD_LOOK,IDS_HELP_JOYSTICK_LOOK);
	AddTab(IDS_JOYSTICK_TURN,CMD_TURN,IDS_HELP_JOYSTICK_TURN);


	m_nNextLink = 2;
	AddLink(IDS_CUSTOM_CONTROLS, FOLDER_CMD_CONTROLS, IDS_HELP_CUSTOMCONTROLS);
	AddLink(IDS_MOUSE, FOLDER_CMD_MOUSE, IDS_HELP_MOUSE);
	AddLink(IDS_KEYBOARD, FOLDER_CMD_KEYBOARD, IDS_HELP_KEYBOARD);
	

	CLTGUITextItemCtrl *pCtrl = AddLink(IDS_JOYSTICK, FOLDER_CMD_JOYSTICK, IDS_HELP_JOYSTICK);
	pCtrl->Enable(LTFALSE);


	LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_KEYBOARD,"RestorePos");
	pCtrl = CreateTextItem(IDS_RESTOREDEFAULTS, FOLDER_CMD_RESET_DEFAULTS, IDS_HELP_RESTORE, LTFALSE, GetSmallFont());
	AddFixedControl(pCtrl,kNextGroup,pos);

	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	return LTTRUE;
}

uint32 CFolderJoystick::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
		case CMD_STRAFE:
		case CMD_MOVE:
		case CMD_TURN:
		case CMD_LOOK:
			SetCurrentAxis(dwCommand);
			break;

		case FOLDER_CMD_RESET_DEFAULTS:
		{
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

			if (pProfile)
			{
				pProfile->RestoreDefaults(RESTORE_JOYSTICK);
				pProfile->SetJoystick();

				m_bUseJoystick = pProfile->m_bUseJoystick;
				
				m_AxisData[0] = *pProfile->m_AxisStrafe.GetBindingData();
				m_AxisData[1] = *pProfile->m_AxisMove.GetBindingData();
				m_AxisData[2] = *pProfile->m_AxisLook.GetBindingData();
				m_AxisData[3] = *pProfile->m_AxisTurn.GetBindingData();

				m_bFixedPositionLook = pProfile->m_AxisLook.GetFixedPosition();

				m_dwCurrentAxis = 0;
				SetCurrentAxis(CMD_STRAFE);

				UpdateData(LTFALSE);
			}

			break;
		}

		default:
			return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


void CFolderJoystick::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{

		if (pProfile)
		{
			pProfile->SetJoystick();

			m_bUseJoystick = pProfile->m_bUseJoystick;
			bOldUse = m_bUseJoystick;


			m_AxisData[0] = *pProfile->m_AxisStrafe.GetBindingData();
			m_AxisData[1] = *pProfile->m_AxisMove.GetBindingData();
			m_AxisData[2] = *pProfile->m_AxisLook.GetBindingData();
			m_AxisData[3] = *pProfile->m_AxisTurn.GetBindingData();

			m_bFixedPositionLook = pProfile->m_AxisLook.GetFixedPosition();

			m_dwCurrentAxis = 0;
			SetCurrentAxis(CMD_STRAFE);

		}

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		if (pProfile)
		{

			pProfile->m_bUseJoystick = m_bUseJoystick;

			pProfile->m_AxisLook.SetFixedPosition(m_bFixedPositionLook);

			pProfile->m_AxisStrafe.SetBindingData(&m_AxisData[0]);
			pProfile->m_AxisMove.SetBindingData(&m_AxisData[1]);
			pProfile->m_AxisLook.SetBindingData(&m_AxisData[2]);
			pProfile->m_AxisTurn.SetBindingData(&m_AxisData[3]);

			pProfile->ApplyJoystick();

			if (bOldUse != m_bUseJoystick)
				pProfile->ApplyBindings();
		}
	}
	CBaseFolder::OnFocus(bFocus);
}


void CFolderJoystick::SetCurrentAxis(uint32 dwAxisCmd)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	//if we've already on a tab
	if (m_dwCurrentAxis)
	{
		//save the values on this tab
		UpdateData(LTTRUE);
		//clear any other axis that share the one I just set
		if (m_dwCurrentAxis != dwAxisCmd)
		{
			int nCurrAxis = m_dwCurrentAxis - CMD_STRAFE;
			for (int a = 0; a < 4;a++)
			{

				if (a != nCurrAxis && m_AxisData[a].m_nAxis == m_AxisData[nCurrAxis].m_nAxis)
					m_AxisData[a].m_nAxis = 0;
			}
		}
	}
	m_dwCurrentAxis = dwAxisCmd;

	RemoveFree();
	m_pAxisCycle = LTNULL;
	m_pInvertToggle = LTNULL;
	m_pDeadZoneSlider = LTNULL;
	m_pAnalogToggle = LTNULL;
	m_pSensitivitySlider = LTNULL;
	m_pCenterToggle = LTNULL;
	m_pFixedToggle = LTNULL;

	m_pAxisCycle = LTNULL;

	int nAxis = dwAxisCmd - CMD_STRAFE;
	if (nAxis < 0 || nAxis > 3) nAxis = 0;;

	SetTab(nAxis+1);
	m_pAxisCycle = AddCycleItem(IDS_JOYSTICK_AXIS,IDS_HELP_JOYSTICK_AXIS,kWidth-25,25,&m_AxisData[nAxis].m_nAxis);

	if (m_AxisData[nAxis].m_nAxis < 0 || m_AxisData[nAxis].m_nAxis >= g_pProfileMgr->GetNumAxis())
		m_AxisData[nAxis].m_nAxis = 0;

	for (int i = 0; i < g_pProfileMgr->GetNumAxis();i++)
	{
		CDeviceAxisData *pAxis = g_pProfileMgr->GetAxisData(i);
		HSTRING hTemp = g_pLTClient->CreateString(pAxis->m_sName);
		m_pAxisCycle->AddString(hTemp);
		g_pLTClient->FreeString(hTemp);
	}

	m_pInvertToggle = AddToggle(IDS_JOYSTICK_INVERT, IDS_HELP_INVERTAXIS, kWidth, &m_AxisData[nAxis].m_bInvertAxis);
	m_pInvertToggle->SetOnString(IDS_ON);
	m_pInvertToggle->SetOffString(IDS_OFF);

	m_pDeadZoneSlider = AddSlider(IDS_JOYSTICK_DEADZONE, IDS_HELP_DEADZONE, kWidth, kSlider, &m_AxisData[nAxis].m_nDeadZone);
	m_pDeadZoneSlider->SetSliderRange(kDeadZoneLow, kDeadZoneHigh);
	m_pDeadZoneSlider->SetSliderIncrement(1);

	m_pAnalogToggle=AddToggle(IDS_JOYSTICK_ANALOG, IDS_HELP_ANALOG, kWidth, &m_AxisData[nAxis].m_bAnalog);
	m_pAnalogToggle->SetOnString(IDS_ON);
	m_pAnalogToggle->SetOffString(IDS_OFF);

	m_pSensitivitySlider = AddSlider(IDS_JOYSTICK_SENSITIVITY, IDS_HELP_JOY_SENSE, kWidth, kSlider, &m_AxisData[nAxis].m_nSensitivity);
	m_pSensitivitySlider->SetSliderRange(kSensitivityLow, kSensitivityHigh);
	m_pSensitivitySlider->SetSliderIncrement(1);

	m_pCenterToggle=AddToggle(IDS_JOYSTICK_CENTERCORRECTION, IDS_HELP_JOY_CENTER, kWidth, &m_AxisData[nAxis].m_bCenterOffset);
	m_pCenterToggle->SetOnString(IDS_ON);
	m_pCenterToggle->SetOffString(IDS_OFF);

	if (dwAxisCmd == CMD_LOOK)
	{
		m_pFixedToggle=AddToggle(IDS_JOYSTICK_FIXEDBYPOSITION, IDS_HELP_FIXEDBYPOSITION, kWidth, &m_bFixedPositionLook);
		m_pFixedToggle->SetOnString(IDS_ON);
		m_pFixedToggle->SetOffString(IDS_OFF);
	}

	if (!m_AxisData[nAxis].m_nAxis)
	{
		m_pInvertToggle->Enable(LTFALSE);
		m_pDeadZoneSlider->Enable(LTFALSE);
		m_pAnalogToggle->Enable(LTFALSE);
		m_pSensitivitySlider->Enable(LTFALSE);
		m_pCenterToggle->Enable(LTFALSE);
		if (m_pFixedToggle)
			m_pFixedToggle->Enable(LTFALSE);
	}
	else if (!m_AxisData[nAxis].m_bAnalog)
	{
		m_pSensitivitySlider->Enable(LTFALSE);
		m_pCenterToggle->Enable(LTFALSE);
		if (m_pFixedToggle)
			m_pFixedToggle->Enable(LTFALSE);
	}


	UpdateData(LTFALSE);
}

void CFolderJoystick::Escape()
{
	g_pInterfaceMgr->PlayInterfaceSound(IS_ESCAPE);
	g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_OPTIONS);
}


LTBOOL CFolderJoystick::OnLeft()
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl == m_pAxisCycle)
	{
		m_pAxisCycle->OnLeft();
		LTBOOL bActive = (m_pAxisCycle->GetSelIndex());
		m_pInvertToggle->Enable(bActive);
		m_pDeadZoneSlider->Enable(bActive);
		m_pAnalogToggle->Enable(bActive);

		bActive = bActive && (m_pAnalogToggle->IsOn());
		m_pSensitivitySlider->Enable(bActive);
		m_pCenterToggle->Enable(bActive);
		if (m_pFixedToggle)
			m_pFixedToggle->Enable(bActive);

		return LTTRUE;

	}
	else if (pCtrl == m_pAnalogToggle)
	{
		m_pAnalogToggle->OnLeft();

		LTBOOL bActive = m_pAnalogToggle->IsOn();
		m_pSensitivitySlider->Enable(bActive);
		m_pCenterToggle->Enable(bActive);
		if (m_pFixedToggle)
			m_pFixedToggle->Enable(bActive);

		return LTTRUE;
	}

	return CBaseFolder::OnLeft();
}

LTBOOL CFolderJoystick::OnRight()
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl == m_pAxisCycle)
	{
		m_pAxisCycle->OnRight();
		LTBOOL bActive = (m_pAxisCycle->GetSelIndex());
		m_pInvertToggle->Enable(bActive);
		m_pDeadZoneSlider->Enable(bActive);
		m_pAnalogToggle->Enable(bActive);

		bActive = bActive && (m_pAnalogToggle->IsOn());
		m_pSensitivitySlider->Enable(bActive);
		m_pCenterToggle->Enable(bActive);
		if (m_pFixedToggle)
			m_pFixedToggle->Enable(bActive);

		return LTTRUE;
	}
	else if (pCtrl == m_pAnalogToggle)
	{
		m_pAnalogToggle->OnRight();

		LTBOOL bActive = m_pAnalogToggle->IsOn();
		m_pSensitivitySlider->Enable(bActive);
		m_pCenterToggle->Enable(bActive);
		if (m_pFixedToggle)
			m_pFixedToggle->Enable(bActive);

		return LTTRUE;
	}

	return CBaseFolder::OnRight();
	
}


LTBOOL CFolderJoystick::OnLButtonUp(int x, int y)
{
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pAxisCycle)
		{
			m_pAxisCycle->OnLButtonUp(x, y);
			LTBOOL bActive = (m_pAxisCycle->GetSelIndex());
			m_pInvertToggle->Enable(bActive);
			m_pDeadZoneSlider->Enable(bActive);
			m_pAnalogToggle->Enable(bActive);

			bActive = bActive && (m_pAnalogToggle->IsOn());
			m_pSensitivitySlider->Enable(bActive);
			m_pCenterToggle->Enable(bActive);
			if (m_pFixedToggle)
				m_pFixedToggle->Enable(bActive);

			return LTTRUE;
		}
		else if (pCtrl == m_pAnalogToggle)
		{
			m_pAnalogToggle->OnLButtonUp(x, y);

			LTBOOL bActive = m_pAnalogToggle->IsOn();
			m_pSensitivitySlider->Enable(bActive);
			m_pCenterToggle->Enable(bActive);
			if (m_pFixedToggle)
				m_pFixedToggle->Enable(bActive);

			return LTTRUE;
		}
	}
	return CBaseFolder::OnLButtonUp(x, y);
}

LTBOOL CFolderJoystick::OnRButtonUp(int x, int y)
{
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pAxisCycle)
		{
			m_pAxisCycle->OnRButtonUp(x, y);
			LTBOOL bActive = (m_pAxisCycle->GetSelIndex());
			m_pInvertToggle->Enable(bActive);
			m_pDeadZoneSlider->Enable(bActive);
			m_pAnalogToggle->Enable(bActive);

			bActive = bActive && (m_pAnalogToggle->IsOn());
			m_pSensitivitySlider->Enable(bActive);
			m_pCenterToggle->Enable(bActive);
			if (m_pFixedToggle)
				m_pFixedToggle->Enable(bActive);

			return LTTRUE;
		}
		else if (pCtrl == m_pAnalogToggle)
		{
			m_pAnalogToggle->OnRButtonUp(x, y);

			LTBOOL bActive = m_pAnalogToggle->IsOn();
			m_pSensitivitySlider->Enable(bActive);
			m_pCenterToggle->Enable(bActive);
			if (m_pFixedToggle)
				m_pFixedToggle->Enable(bActive);

			return LTTRUE;
		}
	}
	return CBaseFolder::OnRButtonUp(x, y);
}

char* CFolderJoystick::GetSelectSound()	
{
	uint32 dwID = 0;
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl)
		dwID = pCtrl->GetHelpID();
	if (dwID == IDS_HELP_CUSTOMCONTROLS || dwID == IDS_HELP_MOUSE || dwID == IDS_HELP_KEYBOARD || dwID == IDS_HELP_JOYSTICK)
		return m_szTabSound;
	return m_sDefaultSelectSound;
}


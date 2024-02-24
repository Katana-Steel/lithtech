// FolderCustomControls.cpp: implementation of the CFolderCustomControls class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderCustomControls.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "GameClientShell.h"
#include "dinput.h"

#ifndef DIKEYBOARD_1
	#define DIKEYBOARD_1		 0x81000402
#endif

namespace
{
	enum eLocalCommands
	{
		CMD_SHARED = FOLDER_CMD_CUSTOM,
		CMD_MARINE,
		CMD_ALIEN,
		CMD_PREDATOR,
	};

	enum eStates
	{
		STATE_NORMAL = 0,
		STATE_RESTORE,
	};

	enum eColumns
	{
		ActionColumn,
		BufferColumn,
		EqualsColumn,
		KeyColumn,
	};

    uint32 devices[3] =
	{
		DEVICETYPE_KEYBOARD,
		DEVICETYPE_MOUSE,
		DEVICETYPE_JOYSTICK
	};

	char strDeviceName[3][256] =
	{
		"","",""
	};

	char strDeviceNiceName[3][256] =
	{
		"","",""
	};

	LTBOOL bEatMouseButtonUp = LTFALSE;

	char szWheelUp[32] = "";
	char szWheelDown[32] = "";

}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderCustomControls::CFolderCustomControls()
{
	m_filterType = COM_SHARED;
	m_pProfile = LTNULL;
    m_bWaitingForKey = LTFALSE;
	m_fInputPauseTimeLeft = 0.0f;

	m_pMainTabCtrl = LTNULL;

	m_pShared = LTNULL;
	m_pMarine = LTNULL;
	m_pPredator = LTNULL;
	m_pAlien = LTNULL;

	m_bDummyState = LTFALSE;
	m_nDummyTime = 0;
	m_nNextDummyState = 0;
	m_nTabStatus = 0;
}

CFolderCustomControls::~CFolderCustomControls()
{

}

// Build the folder
LTBOOL CFolderCustomControls::Build()
{
	CreateTitle(IDS_TITLE_CONTROLS);

	m_pShared = AddTab(IDS_SHARED_CONTROLS,CMD_SHARED,LTNULL);
	m_pAlien = AddTab(IDS_ALIEN,CMD_ALIEN,LTNULL);
	m_pMarine = AddTab(IDS_MARINE,CMD_MARINE,LTNULL);
	m_pPredator = AddTab(IDS_PREDATOR,CMD_PREDATOR,LTNULL);

	g_pLayoutMgr->GetFolderCustomString((eFolderID)m_nFolderID,"TabSound",m_szTabSound,sizeof(m_szTabSound));


	m_nNextLink = 2;

	m_pMainTabCtrl = AddLink(IDS_CUSTOM_CONTROLS, LTNULL, LTNULL);
	m_pMainTabCtrl->Enable(LTFALSE);


	AddLink(IDS_MOUSE, FOLDER_CMD_MOUSE, IDS_HELP_MOUSE);
	AddLink(IDS_KEYBOARD, FOLDER_CMD_KEYBOARD, IDS_HELP_KEYBOARD);
	AddLink(IDS_JOYSTICK, FOLDER_CMD_JOYSTICK, IDS_HELP_JOYSTICK);


	m_nActionWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_CUST_CONTROLS,"ActionWidth");
	m_nBufferWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_CUST_CONTROLS,"BufferWidth");
	m_nEqualsWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_CUST_CONTROLS,"EqualsWidth");
	m_nBindingWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_CUST_CONTROLS,"BindingWidth");

	for (int dev = 0; dev < 3; dev++)
	{
		g_pLTClient->GetDeviceName (devices[dev],strDeviceName[dev], sizeof(strDeviceName[dev]));
	}

	HSTRING hStr = g_pLTClient->FormatString(IDS_DEVICE_WHEEL_UP);
	SAFE_STRCPY(szWheelUp,g_pLTClient->GetStringData(hStr));
	g_pLTClient->FreeString(hStr);

	hStr = g_pLTClient->FormatString(IDS_DEVICE_WHEEL_DOWN);
	SAFE_STRCPY(szWheelDown,g_pLTClient->GetStringData(hStr));
	g_pLTClient->FreeString(hStr);

	hStr = g_pLTClient->FormatString(IDS_DEVICE_MOUSE);
	SAFE_STRCPY(strDeviceNiceName[1],g_pLTClient->GetStringData(hStr));
	g_pLTClient->FreeString(hStr);

	hStr = g_pLTClient->FormatString(IDS_DEVICE_JOYSTICK);
	SAFE_STRCPY(strDeviceNiceName[2],g_pLTClient->GetStringData(hStr));
	g_pLTClient->FreeString(hStr);


	LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_CUST_CONTROLS,"RestorePos");
	CLTGUITextItemCtrl *pCtrl = CreateTextItem(IDS_RESTOREDEFAULTS, FOLDER_CMD_RESET_DEFAULTS, IDS_HELP_RESTORE, LTFALSE, GetSmallFont());
	AddFixedControl(pCtrl,kNextGroup,pos);

	if (! CBaseFolder::Build()) return LTFALSE;

	UseArrows(LTTRUE);

	return LTTRUE;
}

void CFolderCustomControls::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_pProfile = g_pProfileMgr->GetCurrentProfile();
		m_pProfile->SetBindings();
		OnCommand(CMD_SHARED,0,0);
	}
	else
	{
		m_pProfile->ApplyBindings();
		m_pProfile->Save();
		m_pProfile = LTNULL;
	}
	CBaseFolder::OnFocus(bFocus);
}

uint32 CFolderCustomControls::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
		case CMD_SHARED:
		{
			SetFilter(COM_SHARED);
			SetTab(0);

			m_pShared->Enable(LTFALSE);
			m_pMarine->Enable(LTTRUE);
			m_pPredator->Enable(LTTRUE);
			m_pAlien->Enable(LTTRUE);

			SetSelection(m_nFirstDrawn);

			break;
		}

		case CMD_ALIEN:
		{
			SetFilter(COM_ALIEN);
			SetTab(1);
			CheckArrows();

			m_pShared->Enable(LTTRUE);
			m_pMarine->Enable(LTTRUE);
			m_pPredator->Enable(LTTRUE);
			m_pAlien->Enable(LTFALSE);

			SetSelection(m_nFirstDrawn);

			break;
		}

		case CMD_MARINE:
		{
			SetFilter(COM_MARINE);
			SetTab(2);
			CheckArrows();

			m_pShared->Enable(LTTRUE);
			m_pMarine->Enable(LTFALSE);
			m_pPredator->Enable(LTTRUE);
			m_pAlien->Enable(LTTRUE);

			SetSelection(m_nFirstDrawn);

			break;
		}

		case CMD_PREDATOR:
		{
			SetFilter(COM_PREDATOR);
			SetTab(3);
			CheckArrows();

			m_pShared->Enable(LTTRUE);
			m_pMarine->Enable(LTTRUE);
			m_pPredator->Enable(LTFALSE);
			m_pAlien->Enable(LTTRUE);

			SetSelection(m_nFirstDrawn);

			break;
		}

		case FOLDER_CMD_CHANGE_CONTROL:
		{
			if (m_bWaitingForKey)
				break;
			
			int nCommand = (int)dwParam1;
			int nIndex = (int)dwParam2;

			CLTGUIColumnTextCtrl *pCtrl=(CLTGUIColumnTextCtrl *)GetControl(nIndex);

            HSTRING hEquals=g_pLTClient->CreateString("=");
			pCtrl->SetString(EqualsColumn, hEquals);
            g_pLTClient->FreeString(hEquals);

            HSTRING hEmpty=g_pLTClient->CreateString("");
			pCtrl->SetString(KeyColumn, hEmpty);
            g_pLTClient->FreeString(hEmpty);

			// see if we can track the input devices
            LTRESULT hResult = g_pLTClient->StartDeviceTrack (DEVICETYPE_KEYBOARD | DEVICETYPE_MOUSE | DEVICETYPE_JOYSTICK, kTrackBufferSize);
			if (hResult != LT_OK)
			{
                g_pLTClient->EndDeviceTrack();
			}
			else
			{
                m_bWaitingForKey = LTTRUE;
			}

			break;
		}

		case FOLDER_CMD_RESET_DEFAULTS:
		{
			SetUpdateControls(LTFALSE);
			SetDummyStatus(500, STATE_RESTORE);
			break;
		}

		default:
			return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
}


void CFolderCustomControls::SetFilter(CommandType comType)
{
	if (GetSelection() >= 0)
		SetSelection(kNoSelection);

	Species s = Unknown;
	switch (m_filterType)
	{
	case COM_MARINE:
		s = Marine;
		break;
	case COM_ALIEN:
		s = Alien;
		break;
	case COM_PREDATOR:
		s = Predator;
		break;
	}
	m_pProfile->ApplyBindings(s);
	m_pProfile->Save();

	m_filterType = comType;

	switch (m_filterType)
	{
	case COM_MARINE:
		s = Marine;
		break;
	case COM_ALIEN:
		s = Alien;
		break;
	case COM_PREDATOR:
		s = Predator;
		break;
	}
	m_pProfile->ApplyBindings(s);
	m_pProfile->SetBindings(s);


	RemoveFree();
	BuildList();
	if (GetSelection() == kNoSelection)
		SetSelection(m_nFirstDrawn);
}

void CFolderCustomControls::BuildList()
{
	HSTRING hEmpty = g_pLTClient->CreateString(" ");
	for (int c = 0; c < g_kNumCommands; c++)
	{
		if (g_CommandArray[c].nType == m_filterType)
		{
			// The initial column (contains the action)
			CLTGUIColumnTextCtrl *pCtrl=AddColumnText(FOLDER_CMD_CHANGE_CONTROL, IDS_HELP_SETCONTROL, LTFALSE, GetTinyFont());

			// The "action" column
			pCtrl->AddColumn(g_CommandArray[c].nStringID, m_nActionWidth, LTF_JUSTIFY_LEFT);

			// This is a buffer column to space things out on the screen
			pCtrl->AddColumn(hEmpty, m_nBufferWidth, LTF_JUSTIFY_LEFT);

			// The equals column.  Changes from "" to "=" when the user is binding the key
			pCtrl->AddColumn(hEmpty, m_nEqualsWidth, LTF_JUSTIFY_LEFT);

			// The column that contains the key which is assigned to the control!
			pCtrl->AddWordWrapColumn(IDS_CONTROL_UNASSIGNED, m_nBindingWidth);

			int index = GetIndex(pCtrl);
			pCtrl->SetParam1(c);
			pCtrl->SetParam2(index);
			pCtrl->Enable( (g_CommandArray[c].nCommandID >= 0) );
			SetControlText(pCtrl);

		}
	}
	g_pLTClient->FreeString(hEmpty);
}


void CFolderCustomControls::SetControlText(CLTGUIColumnTextCtrl *pCtrl)
{
	if (!pCtrl) 
	{
		_ASSERT(0);
		return;
	}
	int com = pCtrl->GetParam1();


	// If the key is unassigned, then just set the text to the unassigned message
	if ( g_CommandArray[com].nCommandID < 0 )
	{
        HSTRING hString=g_pLTClient->CreateString("");
		pCtrl->SetString(KeyColumn, hString);
        g_pLTClient->FreeString(hString);
		return;
	}


	char strControls[256] = "";
	int numControls = 0;

	CBindingData *pData = m_pProfile->FindBinding(m_filterType,com);
	if (pData)
	{
		if (pData->strTriggerName[0][0] && pData->strTriggerName[0][0] != ' ' )
		{
			SAFE_STRCPY(strControls,pData->strTriggerName[0]);
			++numControls;
		}

		if (pData->strTriggerName[1][0] && pData->strTriggerName[1][0] != ' ' )
		{
			if (numControls)
			{
				strcat(strControls,", ");
			}

			// The device name is copied from the CRes.
			strcat(strControls,strDeviceNiceName[1]);
			strcat(strControls," ");

			// Handle wheel up, wheel down, or key.
			if (stricmp("#U",pData->strTriggerName[1]) == 0)
			{
				strcat(strControls,szWheelUp);
			}
			else if (stricmp("#D",pData->strTriggerName[1]) == 0)
			{
				strcat(strControls,szWheelDown);
			}
			else
			{
				strcat(strControls,pData->strTriggerName[1]);
			}

			++numControls;
		}

		if (m_pProfile->m_bUseJoystick && pData->strTriggerName[2][0] && pData->strTriggerName[2][0] != ' ' )
		{
			if (numControls)
				strcat(strControls,", ");

			strcat(strControls,strDeviceNiceName[2]);
			strcat(strControls," ");
			strcat(strControls,pData->strTriggerName[2]);
		}
	}


	// If the key is unassigned, then just set the text to the unassigned message
	if ( _mbstrlen(strControls) == 0 )
	{
		pCtrl->SetString(KeyColumn, IDS_CONTROL_UNASSIGNED);
	}
	else
	{
		// Set the text in the control
        HSTRING hString=g_pLTClient->CreateString(strControls);
		pCtrl->SetString(KeyColumn, hString);
        g_pLTClient->FreeString(hString);
	}


}

LTBOOL CFolderCustomControls::Render(HSURFACE hDestSurf)
{
	//no actual rendering here... just wait for keypress
	Update(hDestSurf);

	// see if we are pausing input
	if (m_fInputPauseTimeLeft)
	{
        m_fInputPauseTimeLeft -= g_pGameClientShell->GetFrameTime();
		if (m_fInputPauseTimeLeft < 0.0f) m_fInputPauseTimeLeft = 0.0f;
	}

	// see if we are waiting for a keypress
	if (m_bWaitingForKey)
	{
        uint32 nArraySize = kTrackBufferSize;
        g_pLTClient->TrackDevice (m_pInputArray, &nArraySize);
		if (nArraySize > 0)
		{
			// find the first key down event
            for (uint32 i = 0; i < nArraySize; i++)
			{
				if (m_pInputArray[i].m_InputValue)
				{
					if (SetCurrentSelection (&m_pInputArray[i]))
					{
						if (m_pInputArray[i].m_ControlType == CONTROLTYPE_BUTTON)
                            bEatMouseButtonUp = LTTRUE;
                        m_bWaitingForKey = LTFALSE;
                        g_pLTClient->EndDeviceTrack();
						m_fInputPauseTimeLeft = 0.2f;
					}
				}
			}
		}
	}
	LTBOOL bRend = CBaseFolder::Render(hDestSurf);


	return bRend;
}

LTBOOL CFolderCustomControls::SetCurrentSelection (DeviceInput* pInput)
{
	int nCommand=GetSelectedControl()->GetParam1();
	int nIndex=GetSelectedControl()->GetParam2();

	if (pInput->m_DeviceType == DEVICETYPE_MOUSE && pInput->m_ControlType == CONTROLTYPE_ZAXIS)
	{
		return CheckMouseWheel(pInput, nCommand, nIndex);
	}

	if (pInput->m_ControlType != CONTROLTYPE_BUTTON &&
		pInput->m_ControlType != CONTROLTYPE_KEY)
        return LTFALSE;

	char sNewKey[256];
	SAFE_STRCPY(sNewKey,pInput->m_ControlName);
	if (pInput->m_DeviceType == DEVICETYPE_KEYBOARD && (_mbstrlen(pInput->m_ControlName) == 1))
	{
		strncpy(sNewKey, pInput->m_ControlName, 256);
		sNewKey[255] = '\0';
	}

	char sControlCode[32];

	// see if this key is bound to something not in the keyboard configuration folder...
	if (KeyRemappable(pInput))
	{
		sprintf(sControlCode, "##%d", pInput->m_ControlCode);

		UnBind(nCommand, sControlCode, pInput->m_DeviceType);

		if (pInput->m_DeviceType == DEVICETYPE_KEYBOARD)
		{
			if (_mbstrlen(pInput->m_ControlName) != 1)
			{
				Bind(nCommand, pInput->m_ControlCode, pInput->m_ControlName, pInput->m_DeviceType);
			}
			else
			{
				Bind(nCommand, pInput->m_ControlCode, sNewKey, pInput->m_DeviceType);
			}
		}
		else
		{
			Bind(nCommand, pInput->m_ControlCode, pInput->m_ControlName, pInput->m_DeviceType);
		}

	};

	CLTGUIColumnTextCtrl *pCtrl=(CLTGUIColumnTextCtrl *)GetControl(nIndex);
    HSTRING hEmpty=g_pLTClient->CreateString("");
	pCtrl->SetString(EqualsColumn, hEmpty);
    g_pLTClient->FreeString(hEmpty);

	UpdateAllControls();

    return LTTRUE;
}


LTBOOL CFolderCustomControls::OnUp()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnUp();
	else
        return LTTRUE;
}

LTBOOL CFolderCustomControls::OnDown()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnDown();
	else
        return LTTRUE;
}

LTBOOL CFolderCustomControls::OnLeft()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnLeft();
	else
        return LTTRUE;
}

LTBOOL CFolderCustomControls::OnRight()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnRight();
	else
        return LTTRUE;
}
LTBOOL CFolderCustomControls::OnEnter()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnEnter();
	else
        return LTTRUE;
}
LTBOOL CFolderCustomControls::OnTab()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnTab();
	else
        return LTTRUE;
}

LTBOOL CFolderCustomControls::HandleKeyDown(int key, int rep)
{
	if (m_bWaitingForKey || m_fInputPauseTimeLeft)
		return LTTRUE;

	if (key != VK_DELETE)
		return CBaseFolder::HandleKeyDown(key,rep);

	int nCommand=GetSelectedControl()->GetParam1();

	CBindingData *pData = m_pProfile->FindBinding(m_filterType,nCommand);
	if (pData)
	{
		g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\bind_error.wav", SOUNDPRIORITY_MISC_MEDIUM);
		for (int dev = 0; dev < 3; dev++)
		{
			strcpy(pData->strTriggerName[dev]," ");
			strcpy(pData->strRealName[dev]," ");
		}
	}

	SetControlText((CLTGUIColumnTextCtrl *)GetSelectedControl());

	return LTFALSE;
}


LTBOOL CFolderCustomControls::OnLButtonDown(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnLButtonDown(x,y);
	else
        return LTTRUE;
}

LTBOOL CFolderCustomControls::OnLButtonUp(int x, int y)
{
	if (bEatMouseButtonUp)
	{
        bEatMouseButtonUp = LTFALSE;
        return LTTRUE;
	}
	else
		return CBaseFolder::OnLButtonUp(x,y);
}

LTBOOL CFolderCustomControls::OnLButtonDblClick(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnLButtonDblClick(x,y);
	else
        return LTTRUE;
}

LTBOOL CFolderCustomControls::OnRButtonDown(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnRButtonDown(x,y);
	else
        return LTTRUE;
}

LTBOOL CFolderCustomControls::OnRButtonUp(int x, int y)
{
	if (bEatMouseButtonUp)
	{
        bEatMouseButtonUp = LTFALSE;
        return LTTRUE;
	}
	else
		return CBaseFolder::OnRButtonUp(x,y);
}

LTBOOL CFolderCustomControls::OnRButtonDblClick(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnRButtonDblClick(x,y);
	else
        return LTTRUE;
}


LTBOOL CFolderCustomControls::OnMouseMove(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnMouseMove(x, y);
	else
        return LTTRUE;
}

void CFolderCustomControls::Escape()
{
	if (m_bWaitingForKey || m_fInputPauseTimeLeft)
	{
		return;
	}

	g_pInterfaceMgr->PlayInterfaceSound(IS_ESCAPE);
	g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_OPTIONS);
}



LTBOOL CFolderCustomControls::KeyRemappable (DeviceInput* pInput)
{
	// Check for invalid key types...

	if (pInput->m_ControlType == CONTROLTYPE_KEY)
	{
		uint16 nDIK = pInput->m_ControlCode;

		if (nDIK == DIK_ESCAPE)
			return LTFALSE;
		if (nDIK == DIK_PAUSE)
			return LTFALSE;
		if (nDIK >= DIK_F1 && nDIK <= DIK_F10)
			return LTFALSE;
		if (nDIK >= DIK_F11 && nDIK <= DIK_F15)
			return LTFALSE;

		char sNewKey[256];
		strncpy(sNewKey, pInput->m_ControlName, 256);
		sNewKey[255] = '\0';

		if (sNewKey[0] == ';')
			return LTFALSE;
	}

	//*********************************************************************************

    DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (DEVICETYPE_KEYBOARD);
    if (!pBindings) return LTTRUE;

	// see if this input object is already bound to something...

	DeviceBinding* ptr = pBindings;
	while (ptr)
	{
		if (strcmp (ptr->strTriggerName, pInput->m_ControlName) == 0)
		{
			// see if this binding is in the list of mappable ones
			GameAction* pAction = ptr->pActionHead;
			while (pAction)
			{
                LTBOOL bFound = LTFALSE;
				for (int i = 0; i < g_kNumCommands; i++)
				{
					if (g_CommandArray[i].nCommandID == pAction->nActionCode)
					{
                        bFound = LTTRUE;
						break;
					}
				}

				if (!bFound)
				{
					// this key is already bound to something but is not remappable
                    g_pLTClient->FreeDeviceBindings (pBindings);
                    return LTFALSE;
				}

				pAction = pAction->pNext;
			}

			// binding must already exist in the folders...
			break;
		}
		ptr = ptr->pNext;
	}

	// either the binding exists in the folders or this key is not currently bound...therefore it's remappable

    g_pLTClient->FreeDeviceBindings (pBindings);
    return LTTRUE;
}


// Binds a key to a action
void CFolderCustomControls::Bind(int com, uint16 diCode, char *lpszControlName, uint32 deviceType)
{
	_ASSERT(lpszControlName || diCode);

	int dev = 0;
	while (dev < 3 && devices[dev] != deviceType)
		++dev;


	CBindingData *pData = m_pProfile->FindBinding(g_CommandArray[com].nType,com);
	if (pData)
	{
		if (diCode && deviceType != DEVICETYPE_JOYSTICK)
		{
			sprintf(pData->strRealName[dev],"##%d",diCode);
			SAFE_STRCPY(pData->strTriggerName[dev],lpszControlName)
		}
		else
		{
			SAFE_STRCPY(pData->strRealName[dev],lpszControlName)
			SAFE_STRCPY(pData->strTriggerName[dev],lpszControlName)
		}
	}


}


void CFolderCustomControls::UnBind(int newCom, char *lpszControlName, uint32 deviceType)
{
	_ASSERT(lpszControlName);

	int dev = 0;
	while (dev < 3 && devices[dev] != deviceType)
		++dev;

	for (int com = 0; com < g_kNumCommands; com++)
	{
		if (g_CommandArray[com].nType == COM_SHARED || g_CommandArray[newCom].nType == COM_SHARED || g_CommandArray[newCom].nType == g_CommandArray[com].nType)
		{
			CBindingData *pData = m_pProfile->FindBinding(g_CommandArray[com].nType,com);
			if (pData)
			{
				if (!stricmp(pData->strRealName[dev], lpszControlName))
				{
					g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\bind_error.wav", SOUNDPRIORITY_MISC_MEDIUM);
					strcpy(pData->strTriggerName[dev]," ");
					strcpy(pData->strRealName[dev]," ");
				}
			}
		}
	}

}

void CFolderCustomControls::UpdateAllControls()
{
	for (uint32 i = 0; i < m_controlArray.size(); ++i )
	{
		SetControlText((CLTGUIColumnTextCtrl *)m_controlArray[i]);
	}
}



LTBOOL CFolderCustomControls::CheckMouseWheel (DeviceInput* pInput, int nCommand, int nIndex)
{
    if (!g_pLTClient) return LTFALSE;

	if (pInput->m_DeviceType != DEVICETYPE_MOUSE || pInput->m_ControlType != CONTROLTYPE_ZAXIS)
		return LTFALSE;

	LTBOOL bWheelUp = ((int)pInput->m_InputValue > 0);
	char szCommand[4];
	
	if (bWheelUp)
		strcpy(szCommand, "#U");
	else
		strcpy(szCommand, "#D");
	
	UnBind(nCommand, szCommand, pInput->m_DeviceType);

	Bind(nCommand, 0, szCommand, pInput->m_DeviceType);


	CLTGUIColumnTextCtrl *pCtrl=(CLTGUIColumnTextCtrl *)GetControl(nIndex);
    HSTRING hEmpty=g_pLTClient->CreateString("");
	pCtrl->SetString(EqualsColumn, hEmpty);
    g_pLTClient->FreeString(hEmpty);

	UpdateAllControls();

    return LTTRUE;

}

char* CFolderCustomControls::GetSelectSound()	
{
	uint32 dwID = 0;
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl)
		dwID = pCtrl->GetHelpID();
	if (dwID == IDS_HELP_MOUSE || dwID == IDS_HELP_KEYBOARD || dwID == IDS_HELP_JOYSTICK)
		return m_szTabSound;
	return m_sDefaultSelectSound;
}


void CFolderCustomControls::Update(HSURFACE hDestSurf)
{
	// Update based on our current state...
	if(m_bDummyState)
	{
		UpdateDummyStatus();
	}
}


void CFolderCustomControls::SetDummyStatus(uint32 nDelay, int nNextState)
{
	m_bDummyState = LTTRUE;
	m_nDummyTime = GetTickCount() + nDelay;
	m_nNextDummyState = nNextState;
}


void CFolderCustomControls::UpdateDummyStatus()
{
	// Check for ending...
	if (GetTickCount() >= m_nDummyTime)
	{
		m_bDummyState = LTFALSE;

		if(m_nNextDummyState == STATE_RESTORE)
		{
			m_pProfile->RestoreDefaults(RESTORE_BINDINGS);
			SetFilter(m_filterType);

			SetDummyStatus(500, STATE_NORMAL);
		}
		else if(m_nNextDummyState == STATE_NORMAL)
		{
			SetUpdateControls(LTTRUE);
		}
	}
}

// Sets certain controls to enabled or disabled depending on whether the list
// is being refreshed or not.

void CFolderCustomControls::SetUpdateControls(LTBOOL bEnabled)
{
	if(!bEnabled)
	{
		if(!m_nTabStatus)
		{
			if(!m_pShared->IsEnabled())
				m_nTabStatus |= 0x01;
			if(!m_pMarine->IsEnabled())
				m_nTabStatus |= 0x02;
			if(!m_pPredator->IsEnabled())
				m_nTabStatus |= 0x04;
			if(!m_pAlien->IsEnabled())
				m_nTabStatus |= 0x08;
		}
	}


	uint32 i;

	for(i = 0; i < m_fixedControlArray.size(); ++i)
		m_fixedControlArray[i]->Enable(bEnabled);

	for(i = 0; i < m_skipControlArray.size(); ++i)
		m_skipControlArray[i]->Enable(bEnabled);

	for(i = 0; i < m_controlArray.size(); ++i)
	{
		uint32 c = m_controlArray[i]->GetParam1();
		m_controlArray[i]->Enable( bEnabled && (g_CommandArray[c].nCommandID >= 0) );
	}

	m_pMainTabCtrl->Enable(LTFALSE);


	if(bEnabled)
	{
		if(m_nTabStatus)
		{
			if(m_nTabStatus & 0x01)
				m_pShared->Enable(LTFALSE);
			if(m_nTabStatus & 0x02)
				m_pMarine->Enable(LTFALSE);
			if(m_nTabStatus & 0x04)
				m_pPredator->Enable(LTFALSE);
			if(m_nTabStatus & 0x08)
				m_pAlien->Enable(LTFALSE);

			m_nTabStatus = 0;
		}
	}
}


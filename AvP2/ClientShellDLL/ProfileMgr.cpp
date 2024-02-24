#include "stdafx.h"

#include "profileMgr.h"
#include "clientutilities.h"
#include "commandids.h"
#include "interfacemgr.h"
#include "winutil.h"
#include "clientres.h"
#include "GameClientShell.h"
#include "CharacterButeMgr.h"
#include "MultiplayerClientMgr.h"

extern CGameClientShell* g_pGameClientShell;

#include <Direct.h>			// For _rmdir


const std::string PROFILE_DIR("Profiles\\");
const std::string PROFILE_EXT(".txt");
const std::string DEFAULT_PROFILE("Player_0");


CProfileMgr* g_pProfileMgr = NULL;

namespace
{
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




	//56k,ISDN, DSL, T1, T3
	const int kConnectSpeeds[5] = {4, 6, 9, 13, 19};

	char szWheelUp[32] = "";
	char szWheelDown[32] = "";
}

	char s_aTagName[30];
	char s_aAttName[30];

CUserProfile::CUserProfile()
{
	m_sName			= "";		
	m_bInitted		= LTFALSE;
	m_pCryptKey		= LTNULL;

    m_bInvertY = LTFALSE;
    m_bMouseLook = LTFALSE;
	m_nInputRate = 0;
	m_nSensitivity = 0;

	m_LevelStatus.Marine = 0;
	m_LevelStatus.Predator = 0;
	m_LevelStatus.Alien = 0;

	m_nConnection = 0;
}


LTBOOL CUserProfile::Init(const std::string& profileName, LTBOOL bCreate)
{
	std::string fn = PROFILE_DIR + profileName + PROFILE_EXT;
	std::string dfn = PROFILE_DIR + "default.prf";

	LTBOOL bRet = LTFALSE;

	if (m_bInitted)
		m_buteMgr.Term();

	if (m_pCryptKey)
	{
		bRet = m_buteMgr.Parse(fn.c_str(), m_pCryptKey);
	}
	else
	{
		bRet = m_buteMgr.Parse(fn.c_str());
	}

	if (bRet)
	{
		m_sName = profileName;
	}
	else
	{
		if (!bCreate)
			return LTFALSE;
		m_sName = profileName;
		if (m_pCryptKey)
		{
			bRet = m_buteMgr.Parse(dfn.c_str(), m_pCryptKey);
		}
		else
		{
			bRet = m_buteMgr.Parse(dfn.c_str());
		}

		if (!bRet) return LTFALSE;

	}

	LoadBindings();
	LoadMouse();
	LoadKeyboard();
	LoadJoystick();


	// Game options
	strcpy(s_aTagName,"Game");

	m_fPickupDuration = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,"PickupDuration",5.0f);
	m_bAutoswitch = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"AutoswitchWeapons",1);
	m_bRunLock = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"RunLock",1);
	m_bHeadBob = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"HeadBob",1);
	m_bHeadCanting = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"HeadCanting",1);
	m_bWeaponSway = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"WeaponSway",1);
	m_bObjectives = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"Objectives",1);
	m_bHints = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"Hints",1);
	m_bSubtitles = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"Subtitles",1);
	m_bOrientOverlay = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"OrientOverlay",1);


	// Multiplayer options
	strcpy(s_aTagName,"Multiplayer");

	CString str = m_buteMgr.GetString(s_aTagName, "PlayerName",m_sName.c_str());
	const char* szName = str.GetBuffer();

	for(int i=0 ; szName[i] != '\0'; i++)
	{
		if(szName[i] < ' ' || szName[i] == '`' || szName[i] == '~')
			str[i] = 'X';
	}

	m_sPlayerName = (LPCSTR)str;
	m_nRace = (uint8)m_buteMgr.GetInt(s_aTagName,"Race",1);
	m_nChar[0] = (uint8)m_buteMgr.GetInt(s_aTagName,"Char0",g_pCharacterButeMgr->GetSetFromModelType("Drone_MP"));
	m_nChar[1] = (uint8)m_buteMgr.GetInt(s_aTagName,"Char1",g_pCharacterButeMgr->GetSetFromModelType("Harrison_MP"));
	m_nChar[2] = (uint8)m_buteMgr.GetInt(s_aTagName,"Char2",g_pCharacterButeMgr->GetSetFromModelType("Predator_MP"));
	m_nChar[3] = (uint8)m_buteMgr.GetInt(s_aTagName,"Char3",g_pCharacterButeMgr->GetSetFromModelType("Rykov_MP"));
	m_fMessageDisplay = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,"MessageDisplay",0.2f);
	m_fMessageTime = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,"MessageTime",1.0f);
	m_nConnection = (uint8)m_buteMgr.GetInt(s_aTagName,"Connection",0);
	m_bIgnoreTaunts = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"IgnoreTaunts",0);


	// Store the mission completion status per species
	m_LevelStatus.Alien = m_buteMgr.GetDword("Alien", "LevelStatus");
	m_LevelStatus.Marine = m_buteMgr.GetDword("Marine", "LevelStatus");
	m_LevelStatus.Predator = m_buteMgr.GetDword("Predator", "LevelStatus");


	// Apply everything

	ApplyBindings();
	ApplyMouse();
	ApplyKeyboard();
	ApplyJoystick();
	ApplyGame();
	ApplyMultiplayer();


	m_bInitted = LTTRUE;
	return LTTRUE;
}


void CUserProfile::Save()
{
	std::string fn = PROFILE_DIR + m_sName + PROFILE_EXT;


	for (int c = 0; c < g_kNumCommands; c++)
	{
		if ( g_CommandArray[c].nCommandID < 0 )
			continue;
		switch (g_CommandArray[c].nType)
		{
		case COM_SHARED:
			strcpy(s_aTagName,"Shared");
			break;
		case COM_ALIEN:
			strcpy(s_aTagName,"Alien");
			break;
		case COM_MARINE:
			strcpy(s_aTagName,"Marine");
			break;
		case COM_PREDATOR:
			strcpy(s_aTagName,"Predator");
			break;
		}

		CBindingData *pData = FindBinding(g_CommandArray[c].nType,c);
		if (pData)
		{
			HSTRING hTmp = g_pLTClient->FormatString(pData->nAction);
			strcpy(s_aAttName,g_pLTClient->GetStringData(hTmp));
			g_pLTClient->FreeString(hTmp);
			
			CString str = "";
			for (int d = 0; d < 3; d++)
			{
				if (pData->strRealName[d] && pData->strRealName[d][0])
				{
					str += pData->strRealName[d];
				}
				else
				{
					str += " ";
				}
				if (d < 2)
					str += "@";
			}
	
			m_buteMgr.SetString(s_aTagName, s_aAttName, str);

		}
	}


	// Basic control options
	strcpy(s_aTagName,"Basic");
	m_buteMgr.SetInt(s_aTagName,"InvertMouse",(int)m_bInvertY);
	m_buteMgr.SetInt(s_aTagName,"MouseLook",(int)m_bMouseLook);
    m_buteMgr.SetInt(s_aTagName,"InputRate",m_nInputRate);
    m_buteMgr.SetInt(s_aTagName,"Sensitivity",m_nSensitivity);

	m_buteMgr.SetInt(s_aTagName,"NormalTurn",m_nNormalTurn);
	m_buteMgr.SetInt(s_aTagName,"FastTurn",m_nFastTurn);
	m_buteMgr.SetInt(s_aTagName,"LookUp",m_nLookUp);
	m_buteMgr.SetInt(s_aTagName,"AutoCenter",(int)m_bAutoCenter);

	m_buteMgr.SetInt(s_aTagName,"UseJoystick",(int)m_bUseJoystick);
	m_AxisLook.WriteToProfile(&m_buteMgr);
	m_AxisTurn.WriteToProfile(&m_buteMgr);
	m_AxisStrafe.WriteToProfile(&m_buteMgr);
	m_AxisMove.WriteToProfile(&m_buteMgr);


	// Game options
	strcpy(s_aTagName,"Game");

	m_buteMgr.SetDouble(s_aTagName,"PickupDuration",m_fPickupDuration);
	m_buteMgr.SetInt(s_aTagName,"AutoswitchWeapons",m_bAutoswitch);
	m_buteMgr.SetInt(s_aTagName,"RunLock",m_bRunLock);
	m_buteMgr.SetInt(s_aTagName,"HeadBob",m_bHeadBob);
	m_buteMgr.SetInt(s_aTagName,"HeadCanting",m_bHeadCanting);
	m_buteMgr.SetInt(s_aTagName,"WeaponSway",m_bWeaponSway);
	m_buteMgr.SetInt(s_aTagName,"Objectives",m_bObjectives);
	m_buteMgr.SetInt(s_aTagName,"Hints",m_bHints);
	m_buteMgr.SetInt(s_aTagName,"Subtitles",m_bSubtitles);
	m_buteMgr.SetInt(s_aTagName,"OrientOverlay",m_bOrientOverlay);


	// Multiplayer options
	strcpy(s_aTagName,"Multiplayer");

	m_buteMgr.SetString(s_aTagName, "PlayerName",m_sPlayerName.c_str());
	m_buteMgr.SetInt(s_aTagName,"Race",m_nRace);
	m_buteMgr.SetInt(s_aTagName,"Char0",m_nChar[0]);
	m_buteMgr.SetInt(s_aTagName,"Char1",m_nChar[1]);
	m_buteMgr.SetInt(s_aTagName,"Char2",m_nChar[2]);
	m_buteMgr.SetInt(s_aTagName,"Char3",m_nChar[3]);
	m_buteMgr.SetDouble(s_aTagName,"MessageDisplay",m_fMessageDisplay);
	m_buteMgr.SetDouble(s_aTagName,"MessageTime",m_fMessageTime);
	m_buteMgr.SetInt(s_aTagName,"IgnoreTaunts",m_bIgnoreTaunts);
	m_buteMgr.SetInt(s_aTagName,"Connection",m_nConnection);


	// Store the mission completion status per species
	m_buteMgr.SetDword("Alien", "LevelStatus", m_LevelStatus.Alien);
	m_buteMgr.SetDword("Marine", "LevelStatus", m_LevelStatus.Marine);
	m_buteMgr.SetDword("Predator", "LevelStatus", m_LevelStatus.Predator);


	m_buteMgr.Save(fn.c_str());
}


void CUserProfile::Term()
{
	m_buteMgr.Term();
	m_bInitted = LTFALSE;
}


CBindingData* CUserProfile::FindBinding(int nType, int commandIndex)
{
	int n = 0;
	while (n < m_nNumBindings[nType] && m_bindings[nType][n].nAction != g_CommandArray[commandIndex].nActionStringID)
		n++;

	if (n >= m_nNumBindings[nType])
		return LTNULL;
	return &m_bindings[nType][n];
}


void CUserProfile::LoadBindings()
{

	char temp[256];
	memset(m_bindings,0,sizeof(m_bindings));
	memset(m_nNumBindings,0,sizeof(m_nNumBindings));


	for (int c = 0; c < g_kNumCommands; c++)
	{

		int g = g_CommandArray[c].nType;
		int n = m_nNumBindings[g];
		m_nNumBindings[g]++;
		switch (g)
		{
		case COM_SHARED:
			strcpy(s_aTagName,"Shared");
			break;
		case COM_ALIEN:
			strcpy(s_aTagName,"Alien");
			break;
		case COM_MARINE:
			strcpy(s_aTagName,"Marine");
			break;
		case COM_PREDATOR:
			strcpy(s_aTagName,"Predator");
			break;
		}

		m_bindings[g][n].nStringID = g_CommandArray[c].nStringID;
		m_bindings[g][n].nAction = g_CommandArray[c].nActionStringID;


		strcpy(m_bindings[g][n].strTriggerName[0],"");

		if (g_CommandArray[c].nCommandID < 0)
		{
			strcpy(m_bindings[g][n].strRealName[0]," ");
		}
		else
		{
			HSTRING hTmp = g_pLTClient->FormatString(m_bindings[g][n].nAction);
			strcpy(s_aAttName,g_pLTClient->GetStringData(hTmp));
			g_pLTClient->FreeString(hTmp);
			CString str = m_buteMgr.GetString(s_aTagName, s_aAttName);
			strncpy(temp, (char*)(LPCSTR)str, 256);

			char *pTok = strtok(temp,"@");
			if (pTok)
				strcpy(m_bindings[g][n].strRealName[0],pTok);

			pTok = strtok(LTNULL,"@");
			if (pTok)
				strcpy(m_bindings[g][n].strRealName[1],pTok);

			pTok = strtok(LTNULL,"@");
			if (pTok)
				strcpy(m_bindings[g][n].strRealName[2],pTok);
		}

	}
}

void CUserProfile::LoadMouse()
{
	strcpy(s_aTagName,"Basic");

    m_bInvertY = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"InvertMouse",0);
    m_bMouseLook = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"MouseLook",1);
	m_nInputRate = m_buteMgr.GetInt(s_aTagName,"InputRate",20);
	m_nSensitivity = m_buteMgr.GetInt(s_aTagName,"Sensitivity",10);
}

void CUserProfile::LoadKeyboard()
{
	strcpy(s_aTagName,"Basic");

	m_nNormalTurn	= m_buteMgr.GetInt(s_aTagName,"NormalTurn",15);
	m_nFastTurn		= m_buteMgr.GetInt(s_aTagName,"FastTurn",23);
	m_nLookUp		= m_buteMgr.GetInt(s_aTagName,"LookUp",25);
	m_bAutoCenter	= (LTBOOL)m_buteMgr.GetInt(s_aTagName,"AutoCenter",0);
}

void CUserProfile::LoadJoystick()
{
	strcpy(s_aTagName,"Basic");

	m_bUseJoystick = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"UseJoystick",0);

	m_AxisLook.Init();
	m_AxisTurn.Init();
	m_AxisStrafe.Init();
	m_AxisMove.Init();

	m_AxisLook.ReadFromProfile(&m_buteMgr);
	m_AxisTurn.ReadFromProfile(&m_buteMgr);
	m_AxisStrafe.ReadFromProfile(&m_buteMgr);
	m_AxisMove.ReadFromProfile(&m_buteMgr);
}


//take bindings from profile and apply them to the game
void CUserProfile::ApplyBindings(Species s)
{
	g_pProfileMgr->ClearBindings();
	if (s == Unknown)
		s = g_pProfileMgr->GetSpecies();
	uint32 sf = COM_SHARED;
	switch (s)
	{
	case Alien:
		sf = COM_ALIEN;
		break;
	case Marine:
		sf = COM_MARINE;
		break;
	case Predator:
		sf = COM_PREDATOR;
		break;
	}

	for (int d = 0; d < 3; d++)
	{
		CBindingData *pWheelUp = LTNULL;
		CBindingData *pWheelDown = LTNULL;
		for (int c = 0; c < g_kNumCommands; c++)
		{
			if (g_CommandArray[c].nType != sf && g_CommandArray[c].nType != COM_SHARED)
				continue;
			CBindingData *pData = FindBinding(g_CommandArray[c].nType,c);
			if (pData)
			{

				if (devices[d] == DEVICETYPE_MOUSE && stricmp(pData->strRealName[d],"#U") == 0)
				{
					pWheelUp = pData;
				}
				else if (devices[d] == DEVICETYPE_MOUSE && stricmp(pData->strRealName[d],"#D") == 0)
				{
					pWheelDown = pData;
				}
				else if (pData->strRealName[d][0] && pData->strRealName[d][0] != ' ')
				{
					if (devices[d] == DEVICETYPE_JOYSTICK && !m_bUseJoystick)
					{
						char str[256];
						sprintf (str, "unbind \"%s\" \"%s\" ", strDeviceName[d], pData->strRealName[d]);
						g_pLTClient->RunConsoleString (str);

					}
					else
					{
						HSTRING hTmp = g_pLTClient->FormatString(pData->nAction);
						strcpy(s_aAttName,g_pLTClient->GetStringData(hTmp));
						g_pLTClient->FreeString(hTmp);
						char tempStr[256];

						// Set the binding
						sprintf(tempStr, "bind \"%s\" \"%s\" \"%s\"", strDeviceName[d], pData->strRealName[d],s_aAttName);
						g_pLTClient->RunConsoleString(tempStr);
					}

				}
			}


		}
		if (devices[d] == DEVICETYPE_MOUSE)
		{
			char tempStr[512] = "";
			if (pWheelUp || pWheelDown)
			{
				// Set the mouse wheel binding
				//rangebind "##mouse" "##z-axis" 0.100000 255.000000 "Forward" -255.000000 -0.100000 "Backward" 
				char upStr[64] = "";
				if (pWheelUp)
				{
					HSTRING hTmp = g_pLTClient->FormatString(pWheelUp->nAction);
					strcpy(s_aAttName,g_pLTClient->GetStringData(hTmp));
					g_pLTClient->FreeString(hTmp);

					sprintf(upStr,"0.10 255.0 \"%s\"",s_aAttName);
				}
				char downStr[64] = "";
				if (pWheelDown)
				{
					HSTRING hTmp = g_pLTClient->FormatString(pWheelDown->nAction);
					strcpy(s_aAttName,g_pLTClient->GetStringData(hTmp));
					g_pLTClient->FreeString(hTmp);

					sprintf(downStr,"-0.10 -255.0 \"%s\"",s_aAttName);
				}

				sprintf(tempStr, "rangebind \"%s\" \"##z-axis\" %s %s", strDeviceName[d], upStr, downStr);
				g_pLTClient->RunConsoleString(tempStr);
			}

		}
				
	}


	g_pLTClient->WriteConfigFile("autoexec.cfg");
}


void CUserProfile::ApplyMouse()
{
	CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();
	pSettings->SetBoolVar("MouseInvertY",m_bInvertY);
	pSettings->SetBoolVar("MouseLook",m_bMouseLook);
	pSettings->SetFloatVar("MouseSensitivity",(float)m_nSensitivity);
	pSettings->ImplementMouseSensitivity();
	pSettings->SetFloatVar("InputRate",(float)m_nInputRate);

	g_pLTClient->WriteConfigFile("autoexec.cfg");
}


void CUserProfile::ApplyKeyboard()
{
	CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();
	if (!m_bMouseLook)
		pSettings->SetBoolVar("AutoCenter",m_bAutoCenter);

	float fTemp = (float)m_nNormalTurn / 10.0f;
	pSettings->SetFloatVar("NormalTurnRate",fTemp);

	fTemp = (float)m_nFastTurn / 10.0f;
	pSettings->SetFloatVar("FastTurnRate",fTemp);

	fTemp = (float)m_nLookUp / 10.0f;
	pSettings->SetFloatVar("LookUpRate",fTemp);

	g_pLTClient->WriteConfigFile("autoexec.cfg");
}


void CUserProfile::ApplyGame()
{
	WriteConsoleFloat("PickupIconDuration",m_fPickupDuration);

	WriteConsoleInt("AutoswitchWeapons",(int)m_bAutoswitch);
	WriteConsoleInt("RunLock",(int)m_bRunLock);
	WriteConsoleInt("HeadBob",(int)m_bHeadBob);
	WriteConsoleInt("HeadCanting",(int)m_bHeadCanting);
	WriteConsoleInt("WeaponSway",(int)m_bWeaponSway);
	WriteConsoleInt("ObjectiveMessages",m_bObjectives);
	WriteConsoleInt("EnableHints",(int)m_bHints);
	WriteConsoleInt("Subtitles",(int)m_bSubtitles);
	WriteConsoleInt("OrientOverlay",(int)m_bOrientOverlay);

	g_pLTClient->WriteConfigFile("autoexec.cfg");
}


void CUserProfile::ApplyMultiplayer()
{
	// filter our any garbage
	for(int i=0 ; m_sPlayerName[i] != '\0'; i++)
	{
		if(m_sPlayerName[i] < ' ' || m_sPlayerName[i] == '`' || m_sPlayerName[i] == '~')
			m_sPlayerName[i] = 'X';
	}

	WriteConsoleFloat("MsgMgrMaxHeight",m_fMessageDisplay);
	WriteConsoleFloat("MsgMgrTimeScale",m_fMessageTime);

	// limit the update rate to match the server...
	if(g_pGameClientShell->GetMultiplayerMgr())
	{
		uint32 nBandwidth = g_pGameClientShell->GetMultiplayerMgr()->GetServerBandwidth();

		if(nBandwidth == 0)
			nBandwidth = m_nConnection;
		else if(nBandwidth <= 15999)
			nBandwidth = 0;
		else if (nBandwidth <= 31999)
			nBandwidth = 0;
		else if (nBandwidth <= 999999)
			nBandwidth = 1;
		else if (nBandwidth <= 9999999)
			nBandwidth = 2;
		else
			nBandwidth = 3;

		if(m_nConnection <= nBandwidth)
			WriteConsoleInt("UpdateRate",kConnectSpeeds[m_nConnection]);
		else
			WriteConsoleInt("UpdateRate",kConnectSpeeds[nBandwidth]);
	}
	else
	{
		WriteConsoleInt("UpdateRate",kConnectSpeeds[m_nConnection]);
	}

	WriteConsoleInt("IgnoreTaunts",(int)m_bIgnoreTaunts);

	g_pLTClient->WriteConfigFile("autoexec.cfg");
}


void CUserProfile::ApplyJoystick()
{
	//clear all joystick actions bound to an axis
    DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (DEVICETYPE_JOYSTICK);
	char str[128];
	//step through each binding
	DeviceBinding* ptr = pBindings;
	while (ptr)
	{
		uint32 contType = g_pProfileMgr->GetControlType(DEVICETYPE_JOYSTICK, ptr->strTriggerName);
		if (contType != CONTROLTYPE_BUTTON && contType != CONTROLTYPE_KEY)							 
		{
			sprintf (str, "unbind \"%s\" \"%s\" ", ptr->strDeviceName, ptr->strTriggerName);
			g_pLTClient->RunConsoleString (str);
		}
		ptr = ptr->pNext;
	}

    g_pLTClient->FreeDeviceBindings (pBindings);

	if (m_bUseJoystick)
	{
		g_pGameClientShell->EnableJoystick();

		m_AxisLook.SaveToConsole();
		m_AxisTurn.SaveToConsole();
		m_AxisStrafe.SaveToConsole();
		m_AxisMove.SaveToConsole();
	}

	g_pLTClient->WriteConfigFile("autoexec.cfg");
}


//read bindings from the game and save them in the profile
void CUserProfile::SetBindings(Species s)
{

	if (s == Unknown)
		s = g_pProfileMgr->GetSpecies();
	uint32 sf = COM_SHARED;
	switch (s)
	{
	case Alien:
		sf = COM_ALIEN;
		break;
	case Marine:
		sf = COM_MARINE;
		break;
	case Predator:
		sf = COM_PREDATOR;
		break;
	}
	for (int dev = 0; dev < 3; ++dev)
	{
		if (devices[dev] == DEVICETYPE_JOYSTICK && !m_bUseJoystick)
		{
			continue;
		}

		for (int i = 0; i < m_nNumBindings[sf]; i++)
		{
			SAFE_STRCPY (m_bindings[sf][i].strRealName[dev], " ");
			SAFE_STRCPY (m_bindings[sf][i].strTriggerName[dev], " ");
			m_bindings[sf][i].nCode = 0;
		}
        uint32 devType = devices[dev];
        DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (devType);
		if (!pBindings)
		{
			continue;
		}
		DeviceBinding* ptr = pBindings;
		while (ptr)
		{
			GameAction* pAction = ptr->pActionHead;
			while (pAction)
			{
				int com = 0;
				while (com < g_kNumCommands && (pAction->nActionCode != g_CommandArray[com].nCommandID || (g_CommandArray[com].nType != COM_SHARED && g_CommandArray[com].nType != sf) ))
					com++;
				if (com >= g_kNumCommands)
					break;


				CBindingData *pData = FindBinding(g_CommandArray[com].nType,com);
                uint32 contType = g_pProfileMgr->GetControlType(devType, ptr->strTriggerName);
				if (contType == CONTROLTYPE_BUTTON || contType == CONTROLTYPE_KEY)							 
				{
					SAFE_STRCPY (pData->strTriggerName[dev], ptr->strTriggerName);
					SAFE_STRCPY (pData->strRealName[dev], ptr->strRealName);
				}
				else if (devType == DEVICETYPE_MOUSE && contType == CONTROLTYPE_ZAXIS)
				{
					if (pAction->nRangeHigh > 0)
					{
						strcpy(pData->strRealName[dev],"#U");
						strcpy(pData->strTriggerName[dev],szWheelUp);
					}
					else if (pAction->nRangeHigh < 0)
					{
						strcpy(pData->strRealName[dev],"#D");
						strcpy(pData->strTriggerName[dev],szWheelDown);
					}
				}
	
				pAction = pAction->pNext;
			}

			ptr = ptr->pNext;
		}
        g_pLTClient->FreeDeviceBindings (pBindings);
	}


}


void CUserProfile::SetMouse()
{
	CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();
	//mouse settings
	m_bMouseLook = pSettings->MouseLook();
	m_bInvertY = pSettings->MouseInvertY();

	m_nInputRate = (int)pSettings->GetFloatVar("InputRate");
	if (m_nInputRate < 0) m_nInputRate = 0;
	if (m_nInputRate > 100) m_nInputRate = 100;

	m_nSensitivity = (int)pSettings->MouseSensitivity();
	if (m_nSensitivity < 0) m_nSensitivity = 0;
	if (m_nSensitivity > 20) m_nSensitivity = 20;
}


void CUserProfile::SetKeyboard()
{
	CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();
	//keyboard settings
	if (m_bMouseLook)
	{
		m_bAutoCenter = LTFALSE;
	}
	else
		m_bAutoCenter = pSettings->AutoCenter();

	float fTemp = pSettings->GetFloatVar("NormalTurnRate");
	m_nNormalTurn = (int)(10.0f * fTemp);

	fTemp = pSettings->GetFloatVar("FastTurnRate");
	m_nFastTurn = (int)(10.0f * fTemp);

	fTemp = pSettings->GetFloatVar("LookUpRate");
	m_nLookUp = (int)(10.0f * fTemp);
}


void CUserProfile::SetJoystick()
{
	m_AxisLook.Init();
	m_AxisTurn.Init();
	m_AxisStrafe.Init();
	m_AxisMove.Init();

	if (g_pProfileMgr->GetNumAxis() <= 1)
	{
		m_bUseJoystick = 0;
		return;
	}

	m_bUseJoystick = (LTBOOL)m_buteMgr.GetInt("Basic","UseJoystick",0);

	if (m_bUseJoystick)
	{
		// Get the bindings
		DeviceBinding *pBinding=g_pLTClient->GetDeviceBindings(DEVICETYPE_JOYSTICK);

		// Iterate through each binding look for the one that matches one of our actions
		DeviceBinding* pCurrentBinding = pBinding;
		while (pCurrentBinding != NULL)
		{
			if (stricmp(pCurrentBinding->strDeviceName, strDeviceName[2]) == 0)
			{
				int i = 0;
				LTBOOL bFound = LTFALSE;
				while (i < 4 && !bFound)
				{
					switch (i)
					{
					case 0:
						if (!m_AxisLook.IsBound())
						{
							bFound = m_AxisLook.CheckBinding(pCurrentBinding);
							m_AxisLook.LoadFromConsole();
						}
						break;
					case 1:
						if (!m_AxisTurn.IsBound())
						{
							bFound = m_AxisTurn.CheckBinding(pCurrentBinding);
							m_AxisTurn.LoadFromConsole();
						}
						break;
					case 2:
						if (!m_AxisStrafe.IsBound())
						{
							bFound = m_AxisStrafe.CheckBinding(pCurrentBinding);
							m_AxisStrafe.LoadFromConsole();
						}
						break;
					case 3:
						if (!m_AxisMove.IsBound())
						{
							bFound = m_AxisMove.CheckBinding(pCurrentBinding);
							m_AxisMove.LoadFromConsole();
						}
						break;
					}
					++i;
				}
			}
			pCurrentBinding = pCurrentBinding->pNext;
			if (pCurrentBinding == pBinding) break;
		}

		// Free the device bindings
		g_pLTClient->FreeDeviceBindings(pBinding);
	}
	else
	{
		m_AxisLook.ReadFromProfile(&m_buteMgr);
		m_AxisTurn.ReadFromProfile(&m_buteMgr);
		m_AxisStrafe.ReadFromProfile(&m_buteMgr);
		m_AxisMove.ReadFromProfile(&m_buteMgr);
	}

}


void CUserProfile::SetGame()
{
	m_fPickupDuration = GetConsoleFloat("PickupIconDuration",m_fPickupDuration);

	m_bAutoswitch = GetConsoleInt("AutoswitchWeapons",(int)m_bAutoswitch);
	m_bRunLock = GetConsoleInt("RunLock",(int)m_bRunLock);
	m_bHeadBob = GetConsoleInt("HeadBob",(int)m_bHeadBob);
	m_bHeadCanting = GetConsoleInt("HeadCanting",(int)m_bHeadCanting);
	m_bWeaponSway = GetConsoleInt("WeaponSway",(int)m_bWeaponSway);
	m_bObjectives = GetConsoleInt("ObjectiveMessages",m_bObjectives);
	m_bHints = GetConsoleInt("EnableHints",(int)m_bHints);
	m_bSubtitles = GetConsoleInt("Subtitles",(int)m_bSubtitles);
	m_bOrientOverlay = GetConsoleInt("OrientOverlay",(int)m_bOrientOverlay);
}


void CUserProfile::SetMultiplayer()
{
	m_fMessageDisplay = GetConsoleFloat("MsgMgrMaxHeight",m_fMessageDisplay);
	m_fMessageTime = GetConsoleFloat("MsgMgrTimeScale",m_fMessageTime);

	m_bIgnoreTaunts = GetConsoleInt("IgnoreTaunts",(int)m_bIgnoreTaunts);

	if(m_nConnection == 0)
	{
		int nUpdateRate = GetConsoleInt("UpdateRate",kConnectSpeeds[m_nConnection]);

		m_nConnection = 4;

		while (m_nConnection > 0 && nUpdateRate < kConnectSpeeds[m_nConnection])
			m_nConnection--;
	}
}


LTBOOL CProfileMgr::Init()
{

	SetDeviceData();

	std::string currentSpecies;
	HCONSOLEVAR	hVar = g_pLTClient->GetConsoleVar("SpeciesName");
	if (hVar)
	{
		currentSpecies = g_pLTClient->GetVarValueString(hVar);
	}
	else
	{
		currentSpecies = "Marine";
		g_pLTClient->RunConsoleString("+SpeciesName Marine");
	}
	SetSpecies(currentSpecies);


	std::string currentProfile;
	char szProfileName[128];
	GetConsoleString("ProfileName",szProfileName,(char *)DEFAULT_PROFILE.c_str());

	//since console variable can't contain spaces so we replace them with "/"
	//this code will replace the slashes with spaces
	char *pSpace = strchr(szProfileName,'/');
	while (pSpace)
	{
		*pSpace = ' ';
		pSpace = strchr(pSpace,'/');
	};
	currentProfile = szProfileName;
	if (m_profile.Init(currentProfile, LTFALSE))
		return LTTRUE;

	NewProfile(DEFAULT_PROFILE);

	return m_profile.IsInitted();


}


void CProfileMgr::Term()
{
	if (m_profile.IsInitted())
	{
		m_profile.SetBindings();

		m_profile.Save();
		m_profile.Term();
	}
}


void CProfileMgr::GetProfileList(StringSet& profileList)
{
	struct _finddata_t file;
	long hFile;

	std::string directory = PROFILE_DIR + "*" + PROFILE_EXT;

	// find first file
	if((hFile = _findfirst(directory.c_str(), &file)) != -1L)
	{
		do
		{
			char *pName = strtok(file.name,".");
			profileList.insert(pName);
		}
		while(_findnext(hFile, &file) == 0);
	}
	_findclose(hFile);
}


void CProfileMgr::ClearBindings()
{
	//step through each device
	for (int i = 0; i < 3; ++i)
	{
        DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (devices[i]);
		if (!pBindings)
		{
			continue;
		}

		char str[128];
		//step through each binding
		DeviceBinding* ptr = pBindings;
		while (ptr)
		{
			//step through each action on that binding checking to see if we should clear it
			GameAction* pAction = ptr->pActionHead;
			while (pAction)
			{
				int com = 0;
				while (com < g_kNumCommands && pAction->nActionCode != g_CommandArray[com].nCommandID)
					com++;
				//if it is a bindable command, clear it
				if (com < g_kNumCommands)
				{
					uint32 contType = GetControlType(devices[i], ptr->strTriggerName);
					if (contType == CONTROLTYPE_BUTTON || contType == CONTROLTYPE_KEY)							 
					{
						sprintf (str, "unbind \"%s\" \"%s\" ", ptr->strDeviceName, ptr->strTriggerName);
						g_pLTClient->RunConsoleString (str);
					}
					else if (devices[i] == DEVICETYPE_MOUSE && contType == CONTROLTYPE_ZAXIS)
					{
						sprintf (str, "unbind \"%s\" \"%s\" ", ptr->strDeviceName, ptr->strTriggerName);
						g_pLTClient->RunConsoleString (str);
					}

				}
				pAction = pAction->pNext;
			}

			ptr = ptr->pNext;
		}

        g_pLTClient->FreeDeviceBindings (pBindings);
	}

	g_pLTClient->WriteConfigFile("autoexec.cfg");

}


uint32 CProfileMgr::GetControlType(uint32 deviceType, char *controlName)
{
    DeviceObject* pObj = g_pLTClient->GetDeviceObjects(deviceType);
    uint32 type = CONTROLTYPE_UNKNOWN;
	bool bFound = false;
	while (pObj != NULL && !bFound)
	{
		if (pObj->m_ObjectName != NULL)
		{
			bFound = (strcmp (controlName, pObj->m_ObjectName) == 0);
			if (bFound)
				type = pObj->m_ObjectType;
		}
		pObj = pObj->m_pNext;
	}
	return type;
}


void CProfileMgr::SetSpecies(const std::string& speciesName)
{
	
	if (stricmp(speciesName.c_str(),"Alien") == 0)
		SetSpecies(Alien);
	else if (stricmp(speciesName.c_str(),"Predator") == 0)
		SetSpecies(Predator);
	else
		SetSpecies(Marine);
}


void CProfileMgr::SetSpecies(Species species)  
{
	if (m_eSpecies == species) return;

	//write the current bindings to the profile
	if (m_profile.IsInitted())
	{
		m_profile.SetBindings();
		m_profile.Save();

	}
	ClearBindings();

	//change species
	m_eSpecies = species;
	switch (species)
	{
	case Alien:
		g_pLTClient->RunConsoleString("+SpeciesName Alien");
		break;
	case Marine:
		g_pLTClient->RunConsoleString("+SpeciesName Marine");
		break;
	case Predator:
		g_pLTClient->RunConsoleString("+SpeciesName Predator");
		break;

	}
	


	if (m_profile.IsInitted())
	{
		//read the bindings for the new species
		m_profile.ApplyBindings();
	}
}


void CProfileMgr::NewProfile(const std::string& profileName)
{
	if (profileName.length() == 0)
		return;
	if (m_profile.IsInitted())
	{
		m_profile.Save();
		m_profile.Term();
	}

	m_profile.Init(profileName, LTTRUE);

	char saveDir[128];
	sprintf(saveDir,"%s\\%s",SAVE_DIR,profileName.c_str());

	if (!CWinUtil::DirExist(saveDir))
	{
		CWinUtil::CreateDir(saveDir);
	}

	m_profile.Save();
	
	char str[128];
	SAFE_STRCPY(str,m_profile.m_sName.c_str());
	//since console variable can't contain spaces, we replace them with "/"
	char *pSpace = strchr(str,' ');
	while (pSpace)
	{
		*pSpace = '/';
		pSpace = strchr(pSpace,' ');
	};

	WriteConsoleString("ProfileName",str);

}


void CProfileMgr::DeleteProfile(const std::string& profileName)
{
	LTBOOL bNeedNew = LTFALSE;
	if (m_profile.IsInitted() && m_profile.m_sName.compare(profileName) == 0)
	{
		m_profile.Term();
		bNeedNew = LTTRUE;
	}
		
	std::string fn = PROFILE_DIR + profileName + PROFILE_EXT;
	remove(fn.c_str());

	char saveDir[128];
	sprintf(saveDir,"%s\\%s",SAVE_DIR,profileName.c_str());
	_rmdir(saveDir);

	if (bNeedNew)
	{
		StringSet	profileList;
		GetProfileList(profileList);

		StringSet::iterator iter = profileList.begin();
		if (iter == profileList.end())
		{
			//empty list
			NewProfile(DEFAULT_PROFILE);
		}
		else
			NewProfile(*iter);


	}

}


void CProfileMgr::RenameProfile(const std::string& oldName,const std::string& newName)
{
	if (newName.length() == 0)
		return;
	LTBOOL bIsCurrent = (m_profile.IsInitted() && m_profile.m_sName.compare(oldName) == 0);

	if (bIsCurrent && m_profile.IsInitted())
	{
		m_profile.Save();
		m_profile.Term();
	}
		
	std::string ofn = PROFILE_DIR + oldName + PROFILE_EXT;
	std::string nfn = PROFILE_DIR + newName + PROFILE_EXT;
	rename(ofn.c_str(),nfn.c_str());
	Sleep(100);

	char oldDir[128];
	char newDir[128];
	sprintf(oldDir,"%s\\%s",SAVE_DIR,oldName.c_str());
	sprintf(newDir,"%s\\%s",SAVE_DIR,newName.c_str());
	rename(oldDir,newDir);

	if (bIsCurrent)
	{
		m_profile.Init(newName,LTFALSE);

		char str[128];
		SAFE_STRCPY(str,m_profile.m_sName.c_str());
		//since console variable can't contain spaces, we replace them with "/"
		char *pSpace = strchr(str,' ');
		while (pSpace)
		{
			*pSpace = '/';
			pSpace = strchr(pSpace,' ');
		};

		WriteConsoleString("ProfileName",str);
	}



}


LTBOOL CUserProfile::RestoreDefaults(int nFlags)
{
	std::string dfn = PROFILE_DIR + "default.prf";

	LTBOOL bRet = LTFALSE;

	if (m_bInitted)
		m_buteMgr.Term();

	if (m_pCryptKey)
	{
		bRet = m_buteMgr.Parse(dfn.c_str(), m_pCryptKey);
	}
	else
	{
		bRet = m_buteMgr.Parse(dfn.c_str());
	}

	if (!bRet) return LTFALSE;


	// Handle all general bindings...
	if(nFlags & RESTORE_BINDINGS)
	{
		m_bInitted =  LTTRUE;
		// Apply the settings
		LoadBindings();
		ApplyBindings();
	}



	// Handle all mouse settings...
	if(nFlags & RESTORE_MOUSE)
	{
		LoadMouse();
		ApplyMouse();
	}

	// Handle all keyboard settings...
	if(nFlags & RESTORE_KEYBOARD)
	{
		LoadKeyboard();
		ApplyKeyboard();
	}

	// Handle all joystick settings...
	if(nFlags & RESTORE_JOYSTICK)
	{
		LoadJoystick();
		ApplyJoystick();
	}


	return LTTRUE;

}


void CProfileMgr::SetDeviceData()
{
	for (int dev = 0; dev < 3; dev++)
	{
		g_pLTClient->GetDeviceName (devices[dev],strDeviceName[dev], sizeof(strDeviceName[dev]));
	}

	HSTRING hTmp = g_pLTClient->FormatString(IDS_WHEEL_UP);
	SAFE_STRCPY(szWheelUp,g_pLTClient->GetStringData(hTmp));
	g_pLTClient->FreeString(hTmp);

	hTmp = g_pLTClient->FormatString(IDS_WHEEL_DOWN);
	SAFE_STRCPY(szWheelDown,g_pLTClient->GetStringData(hTmp));
	g_pLTClient->FreeString(hTmp);


	// the first axis is always the none axis
	HSTRING hAxisName = g_pLTClient->FormatString(IDS_JOYSTICK_AXISNONE);
	SAFE_STRCPY(m_aDeviceData[0].m_sName,g_pLTClient->GetStringData(hAxisName));
	g_pLTClient->FreeString(hAxisName);

	m_nNumDeviceAxis = 1;

	DeviceObject* pObjects = g_pLTClient->GetDeviceObjects(DEVICETYPE_JOYSTICK);
	DeviceObject* pObj = pObjects;
    LTBOOL bFoundIt = LTFALSE;

	// loop through all joystick objects and store the axis ones with our devicename the m_aDeviceData array
	while ((pObj != NULL) && (m_nNumDeviceAxis < kMaxDeviceAxis))
	{
		if ((pObj->m_ObjectName != NULL) && (pObj->m_DeviceName != NULL))
		{
			if ((pObj->m_DeviceType == DEVICETYPE_JOYSTICK) &&
				(stricmp(pObj->m_DeviceName, strDeviceName[2]) == 0) &&
				((pObj->m_ObjectType == CONTROLTYPE_XAXIS) ||
				 (pObj->m_ObjectType == CONTROLTYPE_YAXIS) ||
				 (pObj->m_ObjectType == CONTROLTYPE_ZAXIS) ||
				 (pObj->m_ObjectType == CONTROLTYPE_RXAXIS) ||
				 (pObj->m_ObjectType == CONTROLTYPE_RYAXIS) ||
				 (pObj->m_ObjectType == CONTROLTYPE_RZAXIS) ||
				 (pObj->m_ObjectType == CONTROLTYPE_SLIDER)))
			{
				m_aDeviceData[m_nNumDeviceAxis].Init(pObj->m_ObjectName, pObj->m_ObjectType, pObj->m_RangeLow, pObj->m_RangeHigh);
				m_nNumDeviceAxis++;
			}

		}
		pObj = pObj->m_pNext;
	}

	// free the device objects
	if (pObjects != NULL) g_pLTClient->FreeDeviceObjects (pObjects);

	if (m_nNumDeviceAxis > 1)
	{
		g_pLTClient->RunConsoleString("AddAction AxisYaw -10001");
		g_pLTClient->RunConsoleString("AddAction AxisPitch -10002");
		g_pLTClient->RunConsoleString("AddAction AxisLeftRight -10003");
		g_pLTClient->RunConsoleString("AddAction AxisForwardBackward -10004");

	}

}

void CProfileMgr::ApplyBandwidthThrottle()
{
	// limit the update rate to match the server...
	if(g_pGameClientShell->GetMultiplayerMgr())
	{
		uint32 nBandwidth = g_pGameClientShell->GetMultiplayerMgr()->GetServerBandwidth();

		if(nBandwidth == 0)
			nBandwidth = m_profile.m_nConnection;
		else if(nBandwidth <= 15999)
			nBandwidth = 0;
		else if (nBandwidth <= 31999)
			nBandwidth = 0;
		else if (nBandwidth <= 999999)
			nBandwidth = 1;
		else if (nBandwidth <= 9999999)
			nBandwidth = 2;
		else
			nBandwidth = 3;

		if(m_profile.m_nConnection > nBandwidth)
			WriteConsoleInt("UpdateRate",kConnectSpeeds[nBandwidth]);
	}
}


CDeviceAxisData* CProfileMgr::GetAxisData(int ndx)
{
	if (ndx < 0 || ndx >= m_nNumDeviceAxis) 
		return LTNULL;
	return &m_aDeviceData[ndx];
}




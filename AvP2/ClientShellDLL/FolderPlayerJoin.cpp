// FolderPlayer.cpp
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderPlayerJoin.h"
#include "FolderMgr.h"
#include "LayoutMgr.h"
#include "ProfileMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "CharacterButeMgr.h"
#include "GameClientShell.h"
#include "ServerUtilities.h"		// For DEG2RAD()
#include "WeaponMgr.h"
#include "MultiplayerMgrDefs.h"

extern CGameClientShell* g_pGameClientShell;

namespace
{
	int kNameWidth = 0;
	int kWidth = 0;
	int kSlider = 0;

	enum LocalCommands
	{
		CMD_NAME = FOLDER_CMD_CUSTOM,
		CMD_PREVTEAM,
		CMD_NEXTTEAM,
		CMD_PREVCLASS,
		CMD_NEXTCLASS,
	};

	INT_CHAR sChar;

	LTIntPt kTeamPos;
	LTIntPt kClassPos;
	LTRect kDescription;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderPlayerJoin::CFolderPlayerJoin()
{
	m_pNameGroup = LTNULL;
	m_pEdit = LTNULL;
	m_pLabel = LTNULL;

	m_pTeam = LTNULL;
	m_pClass = LTNULL;

	m_szPlayerName[0] = LTNULL;
	m_szOldName[0] = LTNULL;
	m_nRace = 0;
	memset(m_nChar,0,sizeof(m_nChar));

	memset(m_nCharList,0,sizeof(m_nCharList));
	memset(m_nCharCount,0,sizeof(m_nCharCount));

	m_pDescription = LTNULL;
}

//////////////////////////////////////////////////////////////////////

CFolderPlayerJoin::~CFolderPlayerJoin()
{

}

//////////////////////////////////////////////////////////////////////

void CFolderPlayerJoin::Term()
{
	CBaseFolder::Term();
}

//////////////////////////////////////////////////////////////////////
// Build the folder

LTBOOL CFolderPlayerJoin::Build()
{
	// Some temp variables for this function...
	CLTGUITextItemCtrl* pCtrl = LTNULL;
	CToggleCtrl *pToggle = LTNULL;
	LTIntPt pos;


	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;

	// Setup the title of this folder
	CreateTitle(IDS_TITLE_PLAYER);
	UseLogo(LTTRUE);


	// Add the links to other menus...
	AddLink(IDS_SERVER_LIST, FOLDER_CMD_MP_JOIN, IDS_HELP_SERVER_LIST);
	AddLink(IDS_SERVER_INFO, FOLDER_CMD_MP_INFO, IDS_HELP_SERVER_INFO);
	AddLink(IDS_PLAYER, FOLDER_CMD_MP_PLAYER_JOIN, IDS_HELP_PLAYER)->Enable(LTFALSE);


	// Grab some custom values out of the attributes
	kNameWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_PLAYER_JOIN, "NameWidth");
	kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_PLAYER_JOIN, "ColumnWidth");
	kSlider = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_PLAYER_JOIN, "SliderWidth");

	kTeamPos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_PLAYER_JOIN, "TeamPos");
	kClassPos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_PLAYER_JOIN, "ClassPos");
	kDescription = g_pLayoutMgr->GetFolderCustomRect(FOLDER_ID_PLAYER_JOIN, "DescriptionRect");


	// -----------------------------------------------------------------
	// Create the controls for this folder

	// Name control
	m_pEdit = CreateEditCtrl("", CMD_NAME, IDS_HELP_PLAYER_NAME, m_szPlayerName, 21+1);
	m_pEdit->EnableCursor();

	m_pLabel = CreateTextItem(IDS_PLAYER_NAME, CMD_NAME, IDS_HELP_PLAYER_NAME);

	m_pNameGroup = CreateGroup(345, m_pLabel->GetHeight(), IDS_HELP_PLAYER_NAME);
    m_pNameGroup->AddControl(m_pLabel, LTIntPt(0, 0), LTTRUE);
    m_pNameGroup->AddControl(m_pEdit, LTIntPt(kNameWidth, 0), LTFALSE);
	AddFreeControl(m_pNameGroup);


	CFrameCtrl *pFrame = LTNULL;

	// Now create the description field
	int nWidth, nHeight;
	nWidth = kDescription.right - kDescription.left;
	nHeight = kDescription.bottom - kDescription.top;

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(10,10,10), 0.75f, nWidth, nHeight);
	AddFixedControl(pFrame, kNoGroup, LTIntPt(kDescription.left, kDescription.top), LTFALSE);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, nWidth, GetHelpFont()->GetHeight() + 6);
	AddFixedControl(pFrame, kNoGroup, LTIntPt(kDescription.left, kDescription.top), LTFALSE);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, nWidth, nHeight, LTFALSE);
	AddFixedControl(pFrame, kNoGroup, LTIntPt(kDescription.left, kDescription.top), LTFALSE);


	// Create the static text items
	m_pTeam = CreateStaticTextItem(" ", LTNULL, LTNULL, 0, 0, LTTRUE, GetHelpFont());
	m_pClass = CreateStaticTextItem(" ", LTNULL, LTNULL, 0, 0, LTTRUE, GetHelpFont());
	m_pDescription = CreateStaticTextItem(" ", LTNULL, LTNULL, kDescription.right - kDescription.left - 10, 0, LTTRUE, GetHelpFont());

	m_pTeam->SetJustification(LTF_JUSTIFY_CENTER);
	m_pClass->SetJustification(LTF_JUSTIFY_CENTER);

	m_pTeam->SetString("---");
	m_pClass->SetString("---");

	AddFixedControl(m_pTeam, kNoGroup, kTeamPos, LTFALSE);
	AddFixedControl(m_pClass, kNoGroup, kClassPos, LTFALSE);
	AddFixedControl(m_pDescription, kNoGroup, LTIntPt(kDescription.left + 5, kDescription.top + 3), LTFALSE);


	// Team selection buttons
	CBitmapCtrl *pBMPCtrl = LTNULL;

	pBMPCtrl = new CBitmapCtrl;
    pBMPCtrl->Create(g_pLTClient,"interface\\menus\\slider_left.pcx","interface\\menus\\slider_left_h.pcx","interface\\menus\\slider_left_d.pcx", this, CMD_PREVTEAM);
	pBMPCtrl->SetHelpID(IDS_HELP_PREV_TEAM);
	pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_PLAYER_JOIN,"PrevTeam");
	AddFixedControl(pBMPCtrl, kCustomGroup0, pos, LTTRUE);

	pBMPCtrl = new CBitmapCtrl;
    pBMPCtrl->Create(g_pLTClient,"interface\\menus\\slider_right.pcx","interface\\menus\\slider_right_h.pcx","interface\\menus\\slider_right_d.pcx", this, CMD_NEXTTEAM);
	pBMPCtrl->SetHelpID(IDS_HELP_NEXT_TEAM);
	pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_PLAYER_JOIN,"NextTeam");
	AddFixedControl(pBMPCtrl, kCustomGroup0, pos, LTTRUE);


	// Class selection buttons
	pBMPCtrl = new CBitmapCtrl;
    pBMPCtrl->Create(g_pLTClient,"interface\\menus\\slider_left.pcx","interface\\menus\\slider_left_h.pcx","interface\\menus\\slider_left_d.pcx", this, CMD_PREVCLASS);
	pBMPCtrl->SetHelpID(IDS_HELP_PREV_CLASS);
	pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_PLAYER_JOIN,"PrevClass");
	AddFixedControl(pBMPCtrl, kCustomGroup0, pos, LTTRUE);

	pBMPCtrl = new CBitmapCtrl;
    pBMPCtrl->Create(g_pLTClient,"interface\\menus\\slider_right.pcx","interface\\menus\\slider_right_h.pcx","interface\\menus\\slider_right_d.pcx", this, CMD_NEXTCLASS);
	pBMPCtrl->SetHelpID(IDS_HELP_NEXT_CLASS);
	pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_PLAYER_JOIN,"NextClass");
	AddFixedControl(pBMPCtrl, kCustomGroup0, pos, LTTRUE);


	// Setup the model display information
	sChar.vPos = g_pLayoutMgr->GetFolderCustomVector(FOLDER_ID_PLAYER_JOIN,"CharBasePos");


	// Build the lists of available characters
	BuildClassLists();


	// Create the Join button
	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"JoinPos");
	pCtrl = CreateTextItem(IDS_JOIN, FOLDER_CMD_MP_JOIN_GAME, IDS_HELP_JOIN, LTFALSE, GetLargeFont());
	AddFixedControl(pCtrl, kNextGroup, pos);


	return LTTRUE;
}

//////////////////////////////////////////////////////////////////////

uint32 CFolderPlayerJoin::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
		case FOLDER_CMD_MULTIPLAYER:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_MULTI);
			break;
		}

		case CMD_NAME:
		{
			if(GetCapture())
			{
				SetCapture(LTNULL);
				m_pEdit->SetColor(m_hNormalColor,m_hNormalColor,m_hNormalColor);
				m_pEdit->Select(LTFALSE);
				m_pLabel->Select(LTTRUE);
				ForceMouseUpdate();
			}
			else
			{
				strcpy(m_szOldName,m_szPlayerName);
				SetCapture(m_pEdit);
				m_pEdit->SetColor(m_hSelectedColor,m_hSelectedColor,m_hSelectedColor);
				m_pEdit->Select(LTTRUE);
				m_pLabel->Select(LTFALSE);
			}

			break;
		}

		case CMD_PREVTEAM:
		{
			if(m_nRace > 0)
				m_nRace--;
			else
				m_nRace = MAX_TEAMS - 1;

			UpdateCurrentSelection();
			break;
		}

		case CMD_NEXTTEAM:
		{
			if(m_nRace < (MAX_TEAMS - 1))
				m_nRace++;
			else
				m_nRace = 0;

			UpdateCurrentSelection();
			break;
		}

		case CMD_PREVCLASS:
		{
			if(m_nChar[m_nRace] > 0)
				m_nChar[m_nRace]--;
			else
				m_nChar[m_nRace] = m_nCharCount[m_nRace] - 1;

			UpdateCurrentSelection();
			break;
		}

		case CMD_NEXTCLASS:
		{
			if(m_nChar[m_nRace] < (m_nCharCount[m_nRace] - 1))
				m_nChar[m_nRace]++;
			else
				m_nChar[m_nRace] = 0;

			UpdateCurrentSelection();
			break;
		}

		default:
		{
			return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
		}
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////
// Change in focus

void CFolderPlayerJoin::OnFocus(LTBOOL bFocus)
{
	FocusSetup(bFocus);

	CBaseFolder::OnFocus(bFocus);
}


void CFolderPlayerJoin::FocusSetup(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	if(bFocus)
	{
		if(pProfile)
		{
			pProfile->SetMultiplayer();

			SAFE_STRCPY(m_szPlayerName,pProfile->m_sPlayerName.c_str());
			SAFE_STRCPY(m_szOldName,m_szPlayerName);

			m_nRace = pProfile->m_nRace;

			if(m_nRace != Marine && m_nRace != Alien && m_nRace != Predator && m_nRace != Corporate)
				m_nRace = Marine;

			int i, j;

			for(i = 0; i < 4; i++)
			{
				m_nChar[i] = 0;

				for(j = m_nCharCount[i] - 1; j > 0; j--)
				{
					if(m_nCharList[i][j] == pProfile->m_nChar[i])
					{
						m_nChar[i] = j;
						break;
					}
				}
			}
		}

		UpdateCurrentSelection();
		UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		if(pProfile)
		{
			pProfile->m_sPlayerName = m_szPlayerName;

			pProfile->m_nRace = (uint8)m_nRace;

			for(int i = 0; i < 4; i++)
				pProfile->m_nChar[i] = m_nCharList[i][m_nChar[i]];

			pProfile->ApplyMultiplayer();
			pProfile->Save();
		}
	}
}

//////////////////////////////////////////////////////////////////////

void CFolderPlayerJoin::Escape()
{
	if(GetCapture() == m_pPWPassword)
	{
		HidePasswordDlg();
		return;
	}

	// If edit control is happening... then stop it
	if(GetCapture())
	{
        SetCapture(LTNULL);
		strcpy(m_szPlayerName,m_szOldName);
        m_pEdit->UpdateData(LTFALSE);
		m_pEdit->SetColor(m_hNormalColor,m_hNormalColor,m_hNormalColor);
        m_pEdit->Select(LTFALSE);
        m_pLabel->Select(LTTRUE);
		ForceMouseUpdate();
	}
	// Otherwise, handle the base functionality (return to the previous menu)
	else
	{
		g_pInterfaceMgr->PlayInterfaceSound(IS_ESCAPE);
		g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_MULTI);
	}

}

//////////////////////////////////////////////////////////////////////

LTBOOL CFolderPlayerJoin::OnEnter()
{
	if(GetCapture() == m_pEdit)
	{
		char *szText = m_pEdit->GetText();

		if(strlen(szText) < 1)
			m_pEdit->SetText(m_szOldName);
	}

	return CBaseFolder::OnEnter();
}

//////////////////////////////////////////////////////////////////////

LTBOOL	CFolderPlayerJoin::OnLButtonUp(int x, int y)
{
	if (GetCapture())
	{
		OnEnter();
		return LTTRUE;
	}
	else
	{
		return CBaseFolder::OnLButtonUp(x,y);
	}
}

//////////////////////////////////////////////////////////////////////

LTBOOL	CFolderPlayerJoin::OnRButtonUp(int x, int y)
{
	if (GetCapture())
	{
		OnEnter();
		return LTTRUE;
	}
	else
	{
		return CBaseFolder::OnRButtonUp(x,y);
	}
}

//////////////////////////////////////////////////////////////////////

void CFolderPlayerJoin::BuildClassLists()
{
	for(int s = 0; s < 4; s++)
	{
		m_nCharCount[s] = 0;
		for(int c = 0; c < 32;  c++)
		{
			m_nCharList[s][c] = 0;
		}
	}

	int nSets = g_pCharacterButeMgr->GetNumSets();

	for(int i = 0; i < nSets; i++)
	{
		CString str = g_pCharacterButeMgr->GetMultiplayerModel(i);
		if(!str.IsEmpty() && str.Find(".abc") >= 0)
		{
			CharacterClass c = g_pCharacterButeMgr->GetClass(i);
			Species eSpec;
			LTBOOL bValid = LTTRUE;

			switch(c)
			{
				case ALIEN:			eSpec = Alien;				break;
				case MARINE:		eSpec = Marine;				break;
				case PREDATOR:		eSpec = Predator;			break;
				case CORPORATE:		eSpec = Corporate;			break;
				default:			bValid = LTFALSE;			break;
			}

			if(bValid)
			{
				m_nCharList[eSpec][m_nCharCount[eSpec]] = i;
				m_nCharCount[eSpec]++;
			}
		}
	}

}

//////////////////////////////////////////////////////////////////////

void CFolderPlayerJoin::UpdateCurrentSelection()
{
	// Update the team string
	CharacterClass c = g_pCharacterButeMgr->GetClass(m_nCharList[m_nRace][m_nChar[m_nRace]]);

	switch(c)
	{
		case ALIEN:			m_pTeam->SetString(IDS_MP_TEAM_ALIENS);			break;
		case MARINE:		m_pTeam->SetString(IDS_MP_TEAM_MARINES);		break;
		case PREDATOR:		m_pTeam->SetString(IDS_MP_TEAM_PREDATORS);		break;
		case CORPORATE:		m_pTeam->SetString(IDS_MP_TEAM_CORPORATES);		break;
	}

	// Update the class string
	m_pClass->SetString(g_pCharacterButeMgr->GetDisplayName(m_nCharList[m_nRace][m_nChar[m_nRace]]));

	// Update the player model
	CreatePlayerModel();

	// Update the class descriptions
	UpdateDescriptions();
}

//////////////////////////////////////////////////////////////////////

void CFolderPlayerJoin::CreatePlayerModel()
{
	// Get information about the camera that's rendering this model
	HOBJECT hCamera = g_pGameClientShell->GetInterfaceCamera();
	if (!hCamera) return;

	LTVector vPos, vU, vR, vF;
	LTRotation rRot;

	g_pLTClient->GetObjectPos(hCamera, &vPos);
	g_pLTClient->GetObjectRotation(hCamera, &rRot);
	g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);


	// Get the set of character attributes that we need to use
	int characterSet = m_nCharList[m_nRace][m_nChar[m_nRace]];


	// Fill in some variables for the special FX
	BSCREATESTRUCT bcs;

	// Filenames
	bcs.Filename = g_pCharacterButeMgr->GetDefaultModel(characterSet, LTNULL, LTTRUE);

	for(int i = 0; i < MAX_MODEL_TEXTURES; ++i)
		bcs.Skins[i] = g_pCharacterButeMgr->GetDefaultSkin(characterSet, i);

	// Position
	LTVector vOffset = g_pCharacterButeMgr->GetSelectPos(characterSet);
	bcs.vPos = vPos;
	bcs.vPos += vR * (sChar.vPos.x + vOffset.x);
	bcs.vPos += vU * (sChar.vPos.y + vOffset.y);
	bcs.vPos += vF * (sChar.vPos.z + vOffset.z) * g_pInterfaceResMgr->GetXRatio();

	// Rotation
	LTFLOAT fRot = MATH_PI + DEG2RAD(g_pCharacterButeMgr->GetSelectRot(characterSet));
	g_pLTClient->RotateAroundAxis(&rRot, &vU, fRot);
	bcs.rRot = rRot;

	// Scale
	LTFLOAT fScale = g_pCharacterButeMgr->GetSelectScale(characterSet);
	bcs.vInitialScale = LTVector(fScale, fScale, fScale);
	bcs.vFinalScale = bcs.vInitialScale;

	// Color information
	bcs.vInitialColor = LTVector(1.0f, 1.0f, 1.0f);
	bcs.vFinalColor = bcs.vInitialColor;
	bcs.bUseUserColors = LTTRUE;

	// Misc data
	bcs.nType = OT_MODEL;
	bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE | FLAG_NOLIGHT;
	bcs.fInitialAlpha = 1.0f;
	bcs.fFinalAlpha = 1.0f;
	bcs.fLifeTime = 1000000.0f;
	bcs.bLoop = LTTRUE;


	// Create the effect
	if(m_CharSFX.Init(&bcs))
	{
		m_CharSFX.CreateObject(g_pLTClient);

		if(m_CharSFX.GetObject())
		{
			HMODELANIM hAnim = g_pLTClient->GetAnimIndex(m_CharSFX.GetObject(), const_cast<char*>(g_pCharacterButeMgr->GetSelectAnim(characterSet)));
			g_pLTClient->SetModelAnimation(m_CharSFX.GetObject(), hAnim);
			g_pInterfaceMgr->AddInterfaceSFX(&m_CharSFX, IFX_WORLD);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderPlayerJoin::UpdateDescriptions
//
//	PURPOSE:	Update the character description text
//
// ----------------------------------------------------------------------- //

void CFolderPlayerJoin::UpdateDescriptions()
{
	// A temp string
	std::string szDescription;
	HSTRING hTemp = LTNULL;
	char szTemp[128];

	int characterSet = m_nCharList[m_nRace][m_nChar[m_nRace]];


	// Fill in the title of the field...
	hTemp = g_pLTClient->FormatString(IDS_CLASS_DESCRIPTION);
	sprintf(szTemp,"--- %s ---\n\n",g_pLTClient->GetStringData(hTemp));
	szDescription = szTemp;
	g_pLTClient->FreeString(hTemp);


	// Setup the scoring class string
	hTemp = g_pLTClient->FormatString(IDS_MP_SCORING_CLASS);
	sprintf(szTemp,"%s: ",g_pLTClient->GetStringData(hTemp));
	szDescription += szTemp;
	g_pLTClient->FreeString(hTemp);

	// Now fill in the actual name of the scoring class
	hTemp = g_pLTClient->FormatString(g_pCharacterButeMgr->GetScoringClass(characterSet));
	sprintf(szTemp,"%s\n\n",g_pLTClient->GetStringData(hTemp));
	szDescription += szTemp;
	g_pLTClient->FreeString(hTemp);


	// Grab the weapon set for this character
	WEAPON_SET *pSet1 = g_pWeaponMgr->GetWeaponSet(g_pCharacterButeMgr->GetMPWeaponSet(characterSet));
	WEAPON_SET *pSet2 = g_pWeaponMgr->GetWeaponSet(g_pCharacterButeMgr->GetMPClassWeaponSet(characterSet));

	if(!pSet1 || !pSet2)
	{
		m_pDescription->SetString(" ");
		return;
	}


	// Setup some temp variables
	WEAPON *pWeap = LTNULL;
	int i;


	// Setup the available and default weapon strings
	char szAvailable[256];
	char szDefault[256];

	memset(szAvailable, 0, 256);
	memset(szDefault, 0, 256);

	for(i = 0; i < pSet1->nNumWeapons; i++)
	{
		pWeap = g_pWeaponMgr->GetWeapon(pSet1->szWeaponName[i]);

		if(pWeap)
		{
			if(pSet1->bCanPickup[i])
			{
				if(szAvailable[0] == '\0')
					sprintf(szAvailable, "%s", g_pLTClient->GetStringData(pWeap->hName));
				else
				{
					sprintf(szTemp, ", %s", g_pLTClient->GetStringData(pWeap->hName));
					strcat(szAvailable, szTemp);
				}
			}
		}
	}

	for(i = 0; i < pSet2->nNumWeapons; i++)
	{
		pWeap = g_pWeaponMgr->GetWeapon(pSet2->szWeaponName[i]);

		if(pWeap)
		{
			if(pSet2->bCanPickup[i])
			{
				if(szDefault[0] == '\0')
					sprintf(szDefault, "%s", g_pLTClient->GetStringData(pWeap->hName));
				else
				{
					sprintf(szTemp, ", %s", g_pLTClient->GetStringData(pWeap->hName));
					strcat(szDefault, szTemp);
				}
			}
		}
	}


	if(!szAvailable[0])		g_pLTClient->CreateString("Error is Weapon.TXT with available weapon setup!");
	if(!szDefault[0])		g_pLTClient->CreateString("Error is Weapon.TXT with default weapon setup!");


	// Now setup the final special text string
	hTemp = g_pLTClient->FormatString(g_pCharacterButeMgr->GetMPWeaponStringRes(characterSet), szAvailable, szDefault);
	szDescription += g_pLTClient->GetStringData(hTemp);
	g_pLTClient->FreeString(hTemp);


	// Now create the final description string
	m_pDescription->SetString((char*)szDescription.c_str());
}


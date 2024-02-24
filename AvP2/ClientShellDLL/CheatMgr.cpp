// ----------------------------------------------------------------------- //
//
// MODULE  : CheatMgr.cpp
//
// PURPOSE : General player cheat code handling
//
// CREATED : 4/7/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "cheatmgr.h"
#include "characterbutemgr.h"
#include "modelbutemgr.h"
#include "gameclientshell.h"
#include "msgids.h"
#include "vartrack.h"
#include "foldercampaignlevel.h"
#include "FXButeMgr.h"

// ----------------------------------------------------------------------- //
// Tracked console variables

VarTrack	g_vtCheatWPOSAdjust;
VarTrack	g_vtCheatWMPOSAdjust;
VarTrack	g_vtCheatBREACHAdjust;
VarTrack	g_vtCheatLIGHTSCALEAdjust;
VarTrack	g_vtCheatLIGHTADDAdjust;
VarTrack	g_vtCheatVERTEXTINTAdjust;
VarTrack	g_vtCheatFOVAdjust;

// ----------------------------------------------------------------------- //
// Cheat code update functions

LTBOOL Update_POS(char *szDebug);
LTBOOL Update_ROT(char *szDebug);
LTBOOL Update_DIMS(char *szDebug);
LTBOOL Update_VEL(char *szDebug);
LTBOOL Update_FRAMERATE(char *szDebug);
LTBOOL Update_WPOS(char *szDebug);
LTBOOL Update_WMPOS(char *szDebug);
LTBOOL Update_BREACH(char *szDebug);
LTBOOL Update_LIGHTSCALE(char *szDebug);
LTBOOL Update_LIGHTADD(char *szDebug);
LTBOOL Update_VERTEXTINT(char *szDebug);
LTBOOL Update_FOV(char *szDebug);

// ----------------------------------------------------------------------- //

CheatMgr*		g_pCheatMgr			= DNULL;
LTBOOL			g_bInfiniteAmmo		= LTFALSE;

static LTFLOAT	g_fUpdateVel_LastTime = -1.0f;
static LTVector	g_vUpdateVel_LastPos(0,0,0);

static LTFLOAT	g_fUpdateFramerate_LastTime = -1.0f;
static LTFLOAT	g_fUpdateFramerate_LastFPS = -1.0f;
static int		g_nUpdateFramerate_Count = 0;

// ----------------------------------------------------------------------- //

LTBOOL CheatMgr::m_bPlayerCheated	= LTFALSE;

// ----------------------------------------------------------------------- //

CheatInfo CheatMgr::s_CheatInfo[] = {

	// Player modification cheats
	{ "LPCKNPH\\MUHZRf",LTFALSE, LTNULL },	// mpdoctordoctor	- gives the player max health
	{ "LPPITPSa",		LTFALSE, LTNULL },	// mpsmithy			- gives the player max armor
	{ "LPPRJMTZPTJ",	LTFALSE, LTNULL },	// mpstockpile		- gives the player all the available weapons
	{ "LPHKUHN\\",		LTFALSE, LTNULL },	// mpkohler			- gives the player max ammo
	{ "LPAQKUN\\",		LTFALSE, LTNULL },	// mpbunker			- All weapons, infinite ammo
	{ "LP@MKPS][\\RI",	LTFALSE, LTNULL },	// mpcanthurtme		- makes the player take no damage
	{ "LPPOU_LSP\\",	LTFALSE, LTNULL },	// mpschuckit		- gives the player max health, max armor, and all weapons
	{ "LPAALWVM",		LTFALSE, LTNULL },	// mpbeamme			- teleports to the spawn point, or to a specific location
	{ "LPPUePS[LVXI",	LTFALSE, LTNULL },	// mpsixthsense		- allows the player to fly throughout the world with no collision
	{ "LPJK_\\S",		LTFALSE, LTNULL },	// mpmorph			- change the player character
	{ "LPQAIIJNK[[If",	LTFALSE, LTNULL },	// mpreloadbutes	- reload an attribute file
	{ "LPVOP",			LTFALSE, LTNULL },	// mpicu			- toggle 3rd person camera

	// Debug information cheats
	{ "LPD^^",			LTFALSE, Update_POS },			// mpgps		- displays debug position information
	{ "LPDP^",			LTFALSE, Update_ROT },			// mpgrs		- displays debug rotation information
	{ "LPPUgOVM",		LTFALSE, Update_DIMS },			// mpsizeme		- displays debug dims information
	{ "LPSMNTHUL\\JX",	LTFALSE, Update_VEL  },			// mptachometer	- displays debug velocity information
	{ "LPJ^U",			LTFALSE, Update_FRAMERATE },	// mpmph		- displays the game framerate
	{ "LPSPTA@M[Y",		LTFALSE, LTNULL },				// mptriggers	- Toggle trigger boxes on/off

	// Position control cheats
	{ "LPT^J]",			LTFALSE, Update_WPOS },			// mpwpos		- toggle position weapon adjustment
	{ "LPTI]I\\",		LTFALSE, Update_WMPOS },		// mpwmpos		- toggle position weapon muzzle adjustment
	{ "LPAP@KLR",		LTFALSE, Update_BREACH },		// mpbreach		- Toggle breach adjust on/off
	{ "LPKUBT_[JWSI",	LTFALSE, Update_LIGHTSCALE },	// mplightscale	- toggle light scale adjustment
	{ "LPKUBT_IML",		LTFALSE, Update_LIGHTADD },		// mplightadd	- toggle light add adjustment
	{ "LPUA_PNb]_UZ",	LTFALSE, Update_VERTEXTINT },	// mpvertextint - toggle vertex tint adjustment
	{ "LPEKS",			LTFALSE, Update_FOV },			// mpfov		- toggle fov adjustment

	// AI cheats
	{ "LPCALPS^VWSR",	LTFALSE, LTNULL },	// mpdeathtoall		- Remove all AI in the level

	// Misc cheats
	{ "LP@KKBRO",		LTFALSE, LTNULL },	// mpconfig			- load in a new cfg file
	{ "LPgDTHN[",		LTFALSE, LTNULL },	// mpxfiles			- reveal all missions

	// Easter egg cheats
	{ "LPJUIHN\\]_RI",	LTFALSE, LTNULL },	// mpmillertime		- the 'Miller' easter egg
};	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::CheatMgr()
//
//	PURPOSE:	Initializes the cheat manager
//
// ----------------------------------------------------------------------- //

CheatMgr::CheatMgr()
{
	g_pCheatMgr = LTNULL;
	m_argc = 0;
	m_argv = LTNULL;
	m_pFont = LTNULL;
	m_pCursor = LTNULL;
	m_hDebugSurf = LTNULL;
	strcpy(m_szDebugString, "\0");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::~CheatMgr()
//
//	PURPOSE:	Initializes the cheat manager
//
// ----------------------------------------------------------------------- //

CheatMgr::~CheatMgr()
{
	g_pCheatMgr = LTNULL;
	m_argc = 0;
	m_argv = LTNULL;

	// Destroy the debug font
	if(m_pFont)
		FreeLithFont(m_pFont);

	// Destroy the debug cursor
	if(m_pCursor)
		FreeLithCursor(m_pCursor);

	if(m_hDebugSurf)
		g_pLTClient->DeleteSurface(m_hDebugSurf);

	strcpy(m_szDebugString, "\0");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Init()
//
//	PURPOSE:	Initializes the cheat manager
//
// ----------------------------------------------------------------------- //

LTBOOL CheatMgr::Init()
{
	g_pCheatMgr = this;
	m_argc = 0;
	m_argv = LTNULL;

	// Init the console tracking variables
	g_vtCheatWPOSAdjust.Init(g_pLTClient, "CheatWPOSAdjust", NULL, 0.005f);
	g_vtCheatWMPOSAdjust.Init(g_pLTClient, "CheatWMPOSAdjust", NULL, 0.005f);
	g_vtCheatBREACHAdjust.Init(g_pLTClient, "CheatBREACHAdjust", NULL, 0.01f);
	g_vtCheatLIGHTSCALEAdjust.Init(g_pLTClient, "CheatLIGHTSCALEAdjust", NULL, 0.005f);
	g_vtCheatLIGHTADDAdjust.Init(g_pLTClient, "CheatLIGHTADDAdjust", NULL, 0.005f);
	g_vtCheatVERTEXTINTAdjust.Init(g_pLTClient, "CheatVERTEXTINTAdjust", NULL, 0.005f);
	g_vtCheatFOVAdjust.Init(g_pLTClient, "CheatFOVAdjust", NULL, 0.5f);

	// Create the debug font
	LITHFONTCREATESTRUCT lfCS;
	lfCS.bChromaKey = LTTRUE;
	m_pFont = CreateLithFont(g_pLTClient, &lfCS);

	// Create the cursor for debug output
	LITHCURSORCREATESTRUCT lcCS;
	lcCS.szRefName = "Cursor_DebugText";
	lcCS.pFont = m_pFont;
	lcCS.pDest = g_pLTClient->GetScreenSurface();
	lcCS.lfDD.hColor = SETRGB(0,255,0);
	lcCS.lfDD.byJustify = LTF_JUSTIFY_RIGHT;
	m_pCursor = CreateLithCursor(g_pLTClient, &lcCS);

	m_hDebugSurf = LTNULL;
	strcpy(m_szDebugString, "\0");

	return (m_pFont && m_pCursor);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::HandleCheat()
//
//	PURPOSE:	See if a string is a cheat code
//
// ----------------------------------------------------------------------- //

LTBOOL CheatMgr::HandleCheat(int argc, char **argv)
{
#ifdef _DEMO
	return LTFALSE;		// Don't do cheats in the demo...
#endif

	if(argc <= 0) return LTFALSE;

	char buf[128];


	// Copy their text
	strncpy(buf, argv[0], sizeof(buf)-1);

	// It should start with "MP"
	if ( strncmp(buf, "mp", 2) != 0 )
		return LTFALSE;

	// Convert it to cheat compatible text
	for ( unsigned i = 0; i < strlen(argv[0]); i++ )
		buf[i] = ((buf[i] ^ 38) + i) ^ 7;

	// Then compare the converted text
	for ( i = 0; i < CHEAT_MAX; i++ )
	{
		if ( strcmp( buf, s_CheatInfo[i].szText ) == 0)
		{
			m_argc = argc;
			m_argv = argv;

			Process((CheatCode)i);

			m_argc = 0;
			m_argv = DNULL;

			// Play a cheat activation sound
			PlayCheatSound();

			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::UpdateCheats()
//
//	PURPOSE:	Do any cheats that require constant updates
//
// ----------------------------------------------------------------------- //

LTBOOL CheatMgr::UpdateCheats()
{
	LTBOOL bValue = LTFALSE;

	// Clear out the debug string...
	strcpy(m_szDebugString, "\0");

	// Go through the list of cheats and see if any active ones need to call
	// their update function
	for(int i = 0; i < CHEAT_MAX; i++)
	{
		if(s_CheatInfo[i].bActive && s_CheatInfo[i].fnUpdate)
			if((*s_CheatInfo[i].fnUpdate)(m_szDebugString))
				bValue = LTTRUE;
	}

	return bValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::UpdateDebugText()
//
//	PURPOSE:	Updates the debug text surface to the screen
//
// ----------------------------------------------------------------------- //

LTBOOL CheatMgr::UpdateDebugText()
{
	// If the length of our debug string is 0, return
	if(strlen(m_szDebugString) < 1)
		return LTFALSE;

	// If everything is valid... draw the string
	if(m_pFont && m_pCursor)
	{
		uint32 nWidth, nHeight;
		uint32 nSizeX, nSizeY;
		HSURFACE hScreen = g_pLTClient->GetScreenSurface();
		g_pLTClient->GetSurfaceDims(hScreen, &nWidth, &nHeight);

		m_pCursor->GetTextExtents(m_szDebugString, LTNULL, nSizeX, nSizeY);
		m_pCursor->SetDest(hScreen);
		m_pCursor->SetLoc(nWidth, nHeight - nSizeY);
		m_pCursor->Draw(m_szDebugString);
	}

	// Get the dimensions of our current debug string

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Process()
//
//	PURPOSE:	Calls the appropriate cheat function
//
// ----------------------------------------------------------------------- //

void CheatMgr::Process(CheatCode nCheatCode)
{
	if ( nCheatCode <= CHEAT_NONE || nCheatCode >= CHEAT_MAX ) return;

	LTBOOL bValidCode = LTTRUE;

	if(g_pGameClientShell->IsMultiplayerGame())
	{
#ifdef _FINAL
		return;
#endif

		bValidCode = IsValidMPCheat(nCheatCode);
	}

	// Process cheat codes
	if(bValidCode)
	{
		switch ( nCheatCode )
		{
			// Player modification cheats
			case CHEAT_HEALTH:				Cheat_Health();				break;
			case CHEAT_ARMOR:				Cheat_Armor();				break;
			case CHEAT_FULL_WEAPONS:		Cheat_FullWeapons();		break;
			case CHEAT_AMMO:				Cheat_Ammo();				break;
			case CHEAT_TEARS:				Cheat_Tears();				break;
			case CHEAT_GOD:					Cheat_God();				break;
			case CHEAT_KFA:					Cheat_KFA();				break;
			case CHEAT_TELEPORT:			Cheat_Teleport();			break;
			case CHEAT_CLIP:				Cheat_Clip();				break;
			case CHEAT_CHARACTER:			Cheat_Character();			break;
			case CHEAT_RELOADBUTES:			Cheat_ReloadButes();		break;
			case CHEAT_3RDPERSON:			Cheat_3rdPerson();			break;

			// Debug information cheats
			case CHEAT_POS:					Cheat_Pos();				break;
			case CHEAT_ROT:					Cheat_Rot();				break;
			case CHEAT_DIMS:				Cheat_Dims();				break;
			case CHEAT_VEL:					Cheat_Vel();				break;
//			case CHEAT_FRAMERATE:			Cheat_Framerate();			break;
			case CHEAT_TRIGGERBOX:			Cheat_TriggerBox();			break;

			// Position control cheats
			case CHEAT_POSWEAPON:			Cheat_PosWeapon();			break;
			case CHEAT_POSWEAPON_MUZZLE:	Cheat_PosWeaponMuzzle();	break;
			case CHEAT_WEAPON_BREACH:		Cheat_WeaponBreach();		break;
			case CHEAT_LIGHTSCALE:			Cheat_LightScale();			break;
			case CHEAT_LIGHTADD:			Cheat_LightAdd();			break;
			case CHEAT_VERTEXTINT:			Cheat_VertexTint();			break;
			case CHEAT_FOV:					Cheat_FOV();				break;

			// AI cheats
			case CHEAT_REMOVEAI:			Cheat_RemoveAI();			break;

			// Misc cheats
			case CHEAT_CONFIG:				Cheat_Config();				break;
			case CHEAT_MISSIONS:			Cheat_Missions();			break;

			// Easter egg cheats
			case CHEAT_MILLERTIME:			Cheat_MillerTime();			break;

			// Skip setting the global cheat indicator for unhandled cheats
			default:
				return;
		}

		m_bPlayerCheated = LTTRUE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::IsValidMPCheat()
//
//	PURPOSE:	Is this code okay in MP?
//
// ----------------------------------------------------------------------- //

LTBOOL CheatMgr::IsValidMPCheat(CheatCode nCheatCode)
{
	switch ( nCheatCode )
	{
		// Player modification cheats
		case CHEAT_HEALTH:
		case CHEAT_ARMOR:
		case CHEAT_TEARS:
		case CHEAT_FULL_WEAPONS:
		case CHEAT_AMMO:
		case CHEAT_KFA:
		case CHEAT_TELEPORT:
			return LTTRUE;

		case CHEAT_GOD:
		case CHEAT_CLIP:
		case CHEAT_CHARACTER:
		case CHEAT_RELOADBUTES:
			g_pLTClient->CPrint("No, no, no... that just wouldn't be fair!");
			return LTFALSE;

		// Debug information cheats
		case CHEAT_3RDPERSON:
		case CHEAT_POS:
		case CHEAT_ROT:
		case CHEAT_DIMS:
		case CHEAT_VEL:
		case CHEAT_FRAMERATE:
		case CHEAT_TRIGGERBOX:

		// Position control cheats
		case CHEAT_POSWEAPON:
		case CHEAT_POSWEAPON_MUZZLE:
		case CHEAT_WEAPON_BREACH:
		case CHEAT_LIGHTSCALE:
		case CHEAT_LIGHTADD:
		case CHEAT_VERTEXTINT:
		case CHEAT_FOV:

		// AI cheats
		case CHEAT_REMOVEAI:

		// Misc cheats
		case CHEAT_CONFIG:

		case CHEAT_MISSIONS:

		// Easter egg cheats
		case CHEAT_MILLERTIME:
			return LTTRUE;

		// Skip setting the global cheat indicator for unhandled cheats
		default:
			return LTFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::SendCheatMessage()
//
//	PURPOSE:	Sends a cheat to the server
//
// ----------------------------------------------------------------------- //

void CheatMgr::SendCheatMessage(CheatCode nCheatCode, uint8 nData)
{
	// Send the Message to the server
	HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_PLAYER_CHEAT);
	g_pLTClient->WriteToMessageByte(hMsg, (uint8)nCheatCode);
	g_pLTClient->WriteToMessageByte(hMsg, nData);
	g_pLTClient->EndMessage(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::PlayCheatSound()
//
//	PURPOSE:	Plays the sound that tells when a cheat was used
//
// ----------------------------------------------------------------------- //

void CheatMgr::PlayCheatSound()
{
	if(g_pClientSoundMgr)
		g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\Cheat.wav", SOUNDPRIORITY_MISC_MEDIUM);
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
//***************************************************************************

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_Health()
//
//	PURPOSE:	Gives full health
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_Health()
{
	s_CheatInfo[CHEAT_HEALTH].bActive = LTTRUE;

	// Tell the server
	SendCheatMessage(CHEAT_HEALTH, LTTRUE);

	g_pLTClient->CPrint("You feel good...real good!");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_Armor()
//
//	PURPOSE:	Gives full armor
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_Armor()
{
	s_CheatInfo[CHEAT_ARMOR].bActive = LTTRUE;

	// Tell the server
	SendCheatMessage(CHEAT_ARMOR, LTTRUE);

	g_pLTClient->CPrint("You feel tough...real tough!");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_FullWeapons()
//
//	PURPOSE:	Give us all the weapons
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_FullWeapons()
{
	s_CheatInfo[CHEAT_FULL_WEAPONS].bActive = LTTRUE;

	// Tell the server
	SendCheatMessage(CHEAT_FULL_WEAPONS, LTTRUE);

	g_pGameClientShell->GetWeaponModel()->Cheat();

	g_pLTClient->CPrint("Mi Mi Mi - Can't find any weapons? <sniff>");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_Ammo()
//
//	PURPOSE:	Gives full ammo
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_Ammo()
{
	s_CheatInfo[CHEAT_AMMO].bActive = LTTRUE;

	// Tell the server
	SendCheatMessage(CHEAT_AMMO, LTTRUE);

	g_pGameClientShell->GetWeaponModel()->Cheat();

	g_pLTClient->CPrint("You feel powerful...real powerful!");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_Tears()
//
//	PURPOSE:	Toggle tears cheat on/off
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_Tears()
{
	LTBOOL bMode = !s_CheatInfo[CHEAT_TEARS].bActive;
	s_CheatInfo[CHEAT_TEARS].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_TEARS, bMode);

	if (bMode)
	{
		g_bInfiniteAmmo = LTTRUE;
		g_pLTClient->CPrint("<sniff>...Here, have some tissue...");
	}
	else
	{
		g_bInfiniteAmmo = LTFALSE;
		g_pLTClient->CPrint("All better now?");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_God()
//
//	PURPOSE:	Sets/resets God mode
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_God()
{
	LTFLOAT fMinHealth = -1.0f;

	if(m_argc > 1)
		fMinHealth = (LTFLOAT)atof(m_argv[1]);

	LTBOOL bMode = (fMinHealth <= 0.0f) ? !s_CheatInfo[CHEAT_GOD].bActive : LTTRUE;
	s_CheatInfo[CHEAT_GOD].bActive = bMode;

	// Send the message to the server to set god mode
	HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_PLAYER_CHEAT);
	g_pLTClient->WriteToMessageByte(hMsg, CHEAT_GOD);
	g_pLTClient->WriteToMessageByte(hMsg, bMode);
	g_pLTClient->WriteToMessageFloat(hMsg, fMinHealth);
	g_pLTClient->EndMessage(hMsg);


	if(bMode)
	{
		g_pLTClient->CPrint("God Mode: ON (%f)... Sucks to be you!", fMinHealth);
	}
	else
	{
		g_pLTClient->CPrint("God Mode: OFF... Yikes!");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_KFA()
//
//	PURPOSE:	Give us all weapons, ammo, armor, and health...
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_KFA()
{
	s_CheatInfo[CHEAT_KFA].bActive = LTTRUE;

	// Tell the server
	SendCheatMessage(CHEAT_KFA, LTTRUE);

	g_pGameClientShell->GetWeaponModel()->Cheat();

	g_pLTClient->CPrint("I'm coming out of the BOOOOOOTH!");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_Teleport()
//
//	PURPOSE:	Teleports to beginning of level
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_Teleport()
{
	if(m_argc == 4)
	{
		// Teleport to a location
		LTVector vPos;
		vPos.x = (LTFLOAT)atof(m_argv[1]);
		vPos.y = (LTFLOAT)atof(m_argv[2]);
		vPos.z = (LTFLOAT)atof(m_argv[3]);

		// Send the Message to the server
		HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_PLAYER_CHEAT);
		g_pLTClient->WriteToMessageByte(hMsg, CHEAT_TELEPORT);
		g_pLTClient->WriteToMessageByte(hMsg, LTTRUE);
		g_pLTClient->WriteToMessageVector(hMsg, &vPos);
		g_pLTClient->EndMessage(hMsg);
	}
	else
	{
		// Teleport to a start point
		SendCheatMessage(CHEAT_TELEPORT, LTFALSE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_Clip()
//
//	PURPOSE:	Sets/resets Clip mode
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_Clip()
{
	LTBOOL bMode = !s_CheatInfo[CHEAT_CLIP].bActive;
	s_CheatInfo[CHEAT_CLIP].bActive = bMode;
	SendCheatMessage(CHEAT_CLIP, bMode);

	g_pGameClientShell->GetPlayerMovement()->SetMovementState(bMode ? CMS_CLIPPING : CMS_POST_CLIPPING);

	g_pLTClient->CPrint("Spectator mode: %s", bMode ? "ENABLED" : "DISABLED");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_Character()
//
//	PURPOSE:	Changes the player character
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_Character()
{
	if(m_argc != 2 || !g_pGameClientShell)
		return;

	// Change the player character
	int nPlayerSet = 0;
	uint8 bReload = LTFALSE;

	// Get a pointer to the player movement class
	PlayerMovement *pMovement = g_pGameClientShell->GetPlayerMovement();
	if(!pMovement) return;

	// See if we're supposed to reload the player bute file
	if(!stricmp(m_argv[1], "reload"))
		bReload = LTTRUE;
	else if(!stricmp(m_argv[1], "prev"))
		nPlayerSet = g_pCharacterButeMgr->GetSetFromModelType(pMovement->GetCharacterButes()->m_szName) - 1;
	else if(!stricmp(m_argv[1], "next"))
		nPlayerSet = g_pCharacterButeMgr->GetSetFromModelType(pMovement->GetCharacterButes()->m_szName) + 1;
	else
		nPlayerSet = g_pCharacterButeMgr->GetSetFromModelType(m_argv[1]);

	// Reload the file if we're suppose to
	if(bReload)
		g_pCharacterButeMgr->Reload(g_pLTClient);
	else
	{
		// Cap off the player set just in case the prev and next options put it out of bounds
		if(nPlayerSet < 0)
			nPlayerSet = g_pCharacterButeMgr->GetNumSets() - 1;
		else if(nPlayerSet >= g_pCharacterButeMgr->GetNumSets())
			nPlayerSet = 0;

		g_pLTClient->CPrint("Character cheat set to: %s", g_pCharacterButeMgr->GetCharacterButes(nPlayerSet).m_szName);
	}

	// Send the message to the server to change the player dims
	HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_PLAYER_CHEAT);
	g_pLTClient->WriteToMessageByte(hMsg, CHEAT_CHARACTER);
	g_pLTClient->WriteToMessageByte(hMsg, bReload);
	g_pLTClient->WriteToMessageByte(hMsg, nPlayerSet);
	g_pLTClient->EndMessage(hMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_ReloadButes()
//
//	PURPOSE:	Updates an attribute file
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_ReloadButes()
{
	if(m_argc != 2 || !g_pGameClientShell)
		return;

	// See if we're supposed to reload an attribute file
	if(!stricmp(m_argv[1], "character"))
	{
		g_pCharacterButeMgr->Reload(g_pLTClient);
	}
	else if(!stricmp(m_argv[1], "model"))
	{
		g_pModelButeMgr->Reload(g_pLTClient);
	}
	else if(!stricmp(m_argv[1], "fx"))
	{
		g_pFXButeMgr->Reload(g_pLTClient);
	}

	// Send the message to the server to change the player dims
	HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_PLAYER_CHEAT);
	g_pLTClient->WriteToMessageByte(hMsg, CHEAT_RELOADBUTES);
	g_pLTClient->WriteToMessageByte(hMsg, 0);
	g_pLTClient->WriteToMessageString(hMsg, m_argv[1]);
	g_pLTClient->EndMessage(hMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_3rdPerson()
//
//	PURPOSE:	Updates an attribute file
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_3rdPerson()
{
	if(g_pGameClientShell->Is3rdPerson())
	{
		g_pGameClientShell->SetExternalCamera(LTFALSE);
		s_CheatInfo[CHEAT_3RDPERSON].bActive = LTFALSE;
		g_pLTClient->CPrint("See ya later!");
	}
	else
	{
		g_pGameClientShell->SetExternalCamera(LTTRUE);
		s_CheatInfo[CHEAT_3RDPERSON].bActive = LTTRUE;
		g_pLTClient->CPrint("You see me!");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_Pos()
//
//	PURPOSE:	Toggle displaying of position on/off
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_Pos()
{
	LTBOOL bMode = !s_CheatInfo[CHEAT_POS].bActive;
	s_CheatInfo[CHEAT_POS].bActive = bMode;
	SendCheatMessage(CHEAT_POS, bMode);
	g_pLTClient->CPrint("Show player position: %s", bMode ? "ENABLED" : "DISABLED");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_Rot()
//
//	PURPOSE:	Toggle displaying of rotation on/off
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_Rot()
{
	LTBOOL bMode = !s_CheatInfo[CHEAT_ROT].bActive;
	s_CheatInfo[CHEAT_ROT].bActive = bMode;
	SendCheatMessage(CHEAT_ROT, bMode);
	g_pLTClient->CPrint("Show player rotation: %s", bMode ? "ENABLED" : "DISABLED");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_Dims()
//
//	PURPOSE:	Toggle displaying of dims on/off
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_Dims()
{
	LTBOOL bMode = !s_CheatInfo[CHEAT_DIMS].bActive;
	s_CheatInfo[CHEAT_DIMS].bActive = bMode;
	SendCheatMessage(CHEAT_DIMS, bMode);
	g_pLTClient->CPrint("Show player dims: %s", bMode ? "ENABLED" : "DISABLED");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_Vel()
//
//	PURPOSE:	Toggle displaying of velocity on/off
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_Vel()
{
	LTBOOL bMode = !s_CheatInfo[CHEAT_VEL].bActive;

	if( !bMode )
	{
		// Insure that the next time this is turned on, 
		// the velocity will wait one frame before displaying.
		g_fUpdateVel_LastTime = -1.0f;
	}

	s_CheatInfo[CHEAT_VEL].bActive = bMode;
	SendCheatMessage(CHEAT_VEL, bMode);
	g_pLTClient->CPrint("Show player velocity: %s", bMode ? "ENABLED" : "DISABLED");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_Framerate()
//
//	PURPOSE:	Toggle displaying of velocity on/off
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_Framerate()
{
	LTBOOL bMode = !s_CheatInfo[CHEAT_FRAMERATE].bActive;

	s_CheatInfo[CHEAT_FRAMERATE].bActive = bMode;
	SendCheatMessage(CHEAT_FRAMERATE, bMode);
	g_pLTClient->CPrint("Show framerate: %s", bMode ? "ENABLED" : "DISABLED");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_TriggerBox()
//
//	PURPOSE:	Toggle trigger boxes on/off
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_TriggerBox()
{
	// Currently unsupported.
	return;

	LTBOOL bMode = !s_CheatInfo[CHEAT_TRIGGERBOX].bActive;
	s_CheatInfo[CHEAT_TRIGGERBOX].bActive = bMode;
	SendCheatMessage(CHEAT_TRIGGERBOX, bMode);

	if (bMode)
	{
		g_pLTClient->CPrint("Ah shucks, that takes all the fun out of it...");
	}
	else
	{
		g_pLTClient->CPrint("That's better sport!");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_PosWeapon()
//
//	PURPOSE:	Toggle positioning of player view weapon on/off
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_PosWeapon()
{
	LTBOOL bMode = !s_CheatInfo[CHEAT_POSWEAPON].bActive;
	s_CheatInfo[CHEAT_POSWEAPON].bActive = bMode;
	SendCheatMessage(CHEAT_POSWEAPON, bMode);

	if(g_pGameClientShell)
		g_pGameClientShell->GetPlayerMovement()->AllowMovement(!bMode);

	if(!bMode)
		g_pLTClient->CPrint("WEAPON.TXT attribute file: %s", g_pWeaponMgr->WriteFile() ? "SAVED" : "FAILED TO SAVE");

	g_pLTClient->CPrint("Adjust player-view weapon position: %s", bMode ? "ON" : "OFF");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_PosWeaponMuzzle()
//
//	PURPOSE:	Toggle positioning of player view weapon muzzle on/off
//
// ----------------------------------------------------------------------- //

int g_nWMPOSBarrel = 0;

void CheatMgr::Cheat_PosWeaponMuzzle()
{
	LTBOOL bMode = LTFALSE;

	if(m_argc == 2)
	{
		// Get the barrel number to adjust
		g_nWMPOSBarrel = (uint16)atoi(m_argv[1]);

		if(!s_CheatInfo[CHEAT_POSWEAPON_MUZZLE].bActive)
		{
			bMode = LTTRUE;
			s_CheatInfo[CHEAT_POSWEAPON_MUZZLE].bActive = LTTRUE;
			SendCheatMessage(CHEAT_POSWEAPON_MUZZLE, LTTRUE);
		}

		g_pLTClient->CPrint("Adjust player-view weapon muzzle position: ON");
	}
	else
	{
		g_nWMPOSBarrel = 0;

		bMode = !s_CheatInfo[CHEAT_POSWEAPON_MUZZLE].bActive;
		s_CheatInfo[CHEAT_POSWEAPON_MUZZLE].bActive = bMode;
		SendCheatMessage(CHEAT_POSWEAPON_MUZZLE, bMode);

		if(!bMode)
			g_pLTClient->CPrint("WEAPON.TXT attribute file: %s", g_pWeaponMgr->WriteFile() ? "SAVED" : "FAILED TO SAVE");

		g_pLTClient->CPrint("Adjust player-view weapon muzzle position: %s", bMode ? "ON" : "OFF");
	}

	if(g_pGameClientShell)
		g_pGameClientShell->GetPlayerMovement()->AllowMovement(!bMode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_WeaponBreach()
//
//	PURPOSE:	Toggle adjusting hand-held weapon breach on/off
//
// ----------------------------------------------------------------------- //

int g_nBREACHBarrel = 0;

void CheatMgr::Cheat_WeaponBreach()
{
	LTBOOL bMode = LTFALSE;

	if(m_argc == 2)
	{
		// Get the barrel number to adjust
		g_nBREACHBarrel = (uint16)atoi(m_argv[1]);

		if(!s_CheatInfo[CHEAT_WEAPON_BREACH].bActive)
		{
			bMode = LTTRUE;
			s_CheatInfo[CHEAT_WEAPON_BREACH].bActive = LTTRUE;
			SendCheatMessage(CHEAT_WEAPON_BREACH, LTTRUE);
		}

		g_pLTClient->CPrint("Adjust hand-held weapon breach offset: ON");
	}
	else
	{
		g_nBREACHBarrel = 0;

		bMode = !s_CheatInfo[CHEAT_WEAPON_BREACH].bActive;
		s_CheatInfo[CHEAT_WEAPON_BREACH].bActive = bMode;
		SendCheatMessage(CHEAT_WEAPON_BREACH, bMode);

		if(!bMode)
			g_pLTClient->CPrint("WEAPON.TXT attribute file: %s", g_pWeaponMgr->WriteFile() ? "SAVED" : "FAILED TO SAVE");

		g_pLTClient->CPrint("Adjust hand-held weapon breach offset: %s", bMode ? "ON" : "OFF");
	}

	if(g_pGameClientShell)
		g_pGameClientShell->GetPlayerMovement()->AllowMovement(!bMode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_LightScale()
//
//	PURPOSE:	Toggle adjustment of light scale offset on/off
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_LightScale()
{
	LTBOOL bMode = !s_CheatInfo[CHEAT_LIGHTSCALE].bActive;
	s_CheatInfo[CHEAT_LIGHTSCALE].bActive = bMode;
	SendCheatMessage(CHEAT_LIGHTSCALE, bMode);

	if(g_pGameClientShell)
		g_pGameClientShell->GetPlayerMovement()->AllowMovement(!bMode);

	g_pLTClient->CPrint("Adjust Light Scale: %s", bMode ? "ON" : "OFF");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_LightAdd()
//
//	PURPOSE:	Toggle adjustment of light add offset on/off
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_LightAdd()
{
	LTBOOL bMode = !s_CheatInfo[CHEAT_LIGHTADD].bActive;
	s_CheatInfo[CHEAT_LIGHTADD].bActive = bMode;
	SendCheatMessage(CHEAT_LIGHTADD, bMode);

	if(g_pGameClientShell)
		g_pGameClientShell->GetPlayerMovement()->AllowMovement(!bMode);

	g_pLTClient->CPrint("Adjust Light Add: %s", bMode ? "ON" : "OFF");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_VertexTint()
//
//	PURPOSE:	Toggle adjustment of light add offset on/off
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_VertexTint()
{
	LTBOOL bMode = !s_CheatInfo[CHEAT_VERTEXTINT].bActive;
	s_CheatInfo[CHEAT_VERTEXTINT].bActive = bMode;
	SendCheatMessage(CHEAT_VERTEXTINT, bMode);

	if(g_pGameClientShell)
		g_pGameClientShell->GetPlayerMovement()->AllowMovement(!bMode);

	g_pLTClient->CPrint("Vertex Tint Adjust: %s", bMode ? "ON" : "OFF");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_FOV()
//
//	PURPOSE:	Toggle adjustment of FOV on/off
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_FOV()
{
	if(!g_pGameClientShell) return;

	if(m_argc == 3)
	{
		LTFLOAT fFOVX = (LTFLOAT)atof(m_argv[1]);
		LTFLOAT fFOVY = (LTFLOAT)atof(m_argv[2]);

		HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
		g_pGameClientShell->GetCameraMgr()->SetCameraFOV(hCamera, fFOVX, fFOVY);
	}
	else
	{
		LTBOOL bMode = !s_CheatInfo[CHEAT_FOV].bActive;
		s_CheatInfo[CHEAT_FOV].bActive = bMode;
		SendCheatMessage(CHEAT_FOV, bMode);

		if(g_pGameClientShell)
			g_pGameClientShell->GetPlayerMovement()->AllowMovement(!bMode);

		g_pLTClient->CPrint("Adjust FOV: %s", bMode ? "ON" : "OFF");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_RemoveAI()
//
//	PURPOSE:	Remove all AI in the level
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_RemoveAI()
{
	LTBOOL bMode = !s_CheatInfo[CHEAT_REMOVEAI].bActive;
	s_CheatInfo[CHEAT_REMOVEAI].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_REMOVEAI, bMode);

	if (bMode)
	{
		g_pLTClient->CPrint("Cheaters never prosper...");
	}
	else
	{
		g_pLTClient->CPrint("Sheeze, you really ARE pathetic...");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_Config()
//
//	PURPOSE:	Loads in a new CFG file
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_Config()
{
	if(m_argc != 2 || !g_pLTClient)
		return;

	// See if we should just clear the bindings
	if(!stricmp(m_argv[1], "clear"))
	{
		uint32 devices[3] =
		{
			DEVICETYPE_KEYBOARD,
			DEVICETYPE_MOUSE,
			DEVICETYPE_JOYSTICK
		};

		for (int i = 0; i < 3; ++i)
		{
			DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings(devices[i]);
			if(!pBindings)
				continue;
			
			char str[128];
			DeviceBinding* ptr = pBindings;
			while(ptr)
			{
				sprintf(str, "rangebind \"%s\" \"%s\" 0 0 \"\"", ptr->strDeviceName, ptr->strTriggerName);
				g_pLTClient->RunConsoleString (str);
				ptr = ptr->pNext;
			}

			g_pLTClient->FreeDeviceBindings(pBindings);
		}

		g_pLTClient->CPrint("Device bindings have been cleared!");
	}
	// Otherwise, load in the new config
	else
	{
		LTRESULT result = g_pLTClient->ReadConfigFile (m_argv[1]);

		if (result != LT_ERROR)
			g_pLTClient->CPrint("New config file has been loaded!");
		else
			g_pLTClient->CPrint("Error loading the config file!");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_Missions()
//
//	PURPOSE:	Toggles mission availability in the menu
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_Missions()
{
	LTBOOL bShow = LTTRUE;
	CFolderCampaignLevel *pAlienMissions = LTNULL, *pMarineMissions = LTNULL, *pPredatorMissions = LTNULL;

	if (!g_pLTClient)
		return;

	pAlienMissions = dynamic_cast<CFolderCampaignLevel *>(g_pInterfaceMgr->GetFolderMgr()->GetFolderFromID(FOLDER_ID_ALIEN_MISSIONS));
	pMarineMissions = dynamic_cast<CFolderCampaignLevel *>(g_pInterfaceMgr->GetFolderMgr()->GetFolderFromID(FOLDER_ID_MARINE_MISSIONS));
	pPredatorMissions = dynamic_cast<CFolderCampaignLevel *>(g_pInterfaceMgr->GetFolderMgr()->GetFolderFromID(FOLDER_ID_PREDATOR_MISSIONS));

	if (pAlienMissions && pMarineMissions && pPredatorMissions)
	{
		bShow = !pAlienMissions->GetShowMissions();

		pAlienMissions->SetShowMissions(bShow);
		pMarineMissions->SetShowMissions(bShow);
		pPredatorMissions->SetShowMissions(bShow);

		if (bShow)
			g_pLTClient->CPrint("All missions revealed!");
		else
			g_pLTClient->CPrint("Mission cheat deactivated!");

		return;
	}

	g_pLTClient->CPrint("Error attempting to reveal missions!");

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheatMgr::Cheat_MillerTime()
//
//	PURPOSE:	Toggles the 'Miller' easter egg
//
// ----------------------------------------------------------------------- //

void CheatMgr::Cheat_MillerTime()
{
	LTBOOL bMode = !s_CheatInfo[CHEAT_MILLERTIME].bActive;
	s_CheatInfo[CHEAT_MILLERTIME].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_MILLERTIME, bMode);

	if (bMode)
	{
		g_pLTClient->CPrint("You gotta love this guy!");
	}
	else
	{
		g_pLTClient->CPrint("What!? It got annoying already?");
	}
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
//***************************************************************************

LTBOOL Update_POS(char *szDebug)
{
	HOBJECT hPhysicsObj = g_pGameClientShell->GetPlayerMovement()->GetObject();
	HOBJECT hClientObj = g_pLTClient->GetClientObject();
	LTVector vPos, vPos2;

	g_pLTClient->GetObjectPos(hPhysicsObj, &vPos);
	g_pLTClient->GetObjectPos(hClientObj, &vPos2);

	LTBOOL bOutSide = (g_pLTClient->GetPointStatus(&vPos) == LT_OUTSIDE) ? LTTRUE : LTFALSE;
	LTBOOL bOutSide2 = (g_pLTClient->GetPointStatus(&vPos2) == LT_OUTSIDE) ? LTTRUE : LTFALSE;

	char szBuffer[256];
	sprintf(szBuffer, "Physics Obj Pos (%.0f, %.0f, %.0f) [%s]\nClient Obj Pos (%.0f, %.0f, %.0f) [%s]\n",
			vPos.x, vPos.y, vPos.z, bOutSide ? "OUTSIDE" : "INSIDE",
			vPos2.x, vPos2.y, vPos2.z, bOutSide2 ? "OUTSIDE" : "INSIDE");

	strcat(szDebug, szBuffer);

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

LTBOOL Update_ROT(char *szDebug)
{
	HOBJECT hPhysicsObj = g_pGameClientShell->GetPlayerMovement()->GetObject();
	HOBJECT hCameraObj = g_pGameClientShell->GetCameraMgr()->GetCameraObject(g_pGameClientShell->GetPlayerCamera());
	LTRotation rRot, rRot2;
	LTVector vR, vU, vF, vR2, vU2, vF2;

	g_pLTClient->GetObjectRotation(hPhysicsObj, &rRot);
	g_pLTClient->GetObjectRotation(hCameraObj, &rRot2);

	g_pLTClient->GetMathLT()->GetRotationVectors(rRot, vR, vU, vF);
	g_pLTClient->GetMathLT()->GetRotationVectors(rRot2, vR2, vU2, vF2);

	char szBuffer[256];
	sprintf(szBuffer, "Player Rot: F = (%.2f, %.2f, %.2f), U = (%.2f, %.2f, %.2f)\nCamera Rot: F = (%.2f, %.2f, %.2f), U = (%.2f, %.2f, %.2f)\n",
			vF.x, vF.y, vF.z, vU.x, vU.y, vU.z,
			vF2.x, vF2.y, vF2.z, vU2.x, vU2.y, vU2.z);

	strcat(szDebug, szBuffer);

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

LTBOOL Update_DIMS(char *szDebug)
{
	HOBJECT hPhysicsObj = g_pGameClientShell->GetPlayerMovement()->GetObject();
	HOBJECT hClientObj = g_pLTClient->GetClientObject();

	LTVector vDims, vDims2;

	g_pLTClient->Physics()->GetObjectDims(hPhysicsObj, &vDims);
	g_pLTClient->Physics()->GetObjectDims(hClientObj, &vDims2);

	char szBuffer[256];
	sprintf(szBuffer, "Physics Obj Dims (%.1f, %.1f, %.1f)\nClient Obj Dims (%.1f, %.1f, %.1f)\n",
			vDims.x, vDims.y, vDims.z, vDims2.x, vDims2.y, vDims2.z);

	strcat(szDebug, szBuffer);

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

LTBOOL Update_VEL(char *szDebug)
{
	HOBJECT hClientObj = g_pLTClient->GetClientObject();

	const LTFLOAT  fCurrentTime = g_pLTClient->GetTime();
	LTVector vCurrentPos(0,0,0);
	g_pLTClient->GetObjectPos(hClientObj, &vCurrentPos);


	const LTFLOAT fDeltaTime = fCurrentTime - g_fUpdateVel_LastTime;
	const LTVector vDeltaPos  = vCurrentPos - g_vUpdateVel_LastPos;

	if( fDeltaTime > 0.0f && fDeltaTime <= 0.2f )
	{
		const LTVector vVel = vDeltaPos / fDeltaTime;

		char szBuffer[256];
		sprintf(szBuffer, "Client Obj Vel (%.2f, %.2f, %.2f) [%.1f]\n",
				vVel.x, vVel.y, vVel.z, vVel.Mag() );

		strcat(szDebug, szBuffer);
	}
	else
	{
		strcat(szDebug, "Client Obj Vel Unknown\n");
	}

	g_fUpdateVel_LastTime = fCurrentTime;
	g_vUpdateVel_LastPos  = vCurrentPos;

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

LTBOOL Update_FRAMERATE(char *szDebug)
{
	const LTFLOAT fCurrentTime = g_pLTClient->GetTime();
	g_nUpdateFramerate_Count++;

	// Only update every 1/2 second so it's readable...
	if(fCurrentTime >= (g_fUpdateFramerate_LastTime + 0.5f))
	{
		g_fUpdateFramerate_LastFPS = 1.0f / ((fCurrentTime - g_fUpdateFramerate_LastTime) / (LTFLOAT)g_nUpdateFramerate_Count);
		g_fUpdateFramerate_LastTime = fCurrentTime;
		g_nUpdateFramerate_Count = 0;
	}

	char szBuffer[256];
	sprintf(szBuffer, "Framerate (%.2f fps)\n", g_fUpdateFramerate_LastFPS);
	strcat(szDebug, szBuffer);

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

LTBOOL Update_WPOS(char *szDebug)
{
	if(!g_pGameClientShell) return LTFALSE;

	LTFLOAT fAdjustAmount = g_vtCheatWPOSAdjust.GetFloat();
	LTVector vWeaponOffset = g_pGameClientShell->GetWeaponModel()->GetWeaponOffset();

	if(g_pLTClient->IsCommandOn(COMMAND_ID_RUN))
		fAdjustAmount *= 2.0f;

	if(g_pLTClient->IsCommandOn(COMMAND_ID_FORWARD))		vWeaponOffset.z += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_BACKWARD))		vWeaponOffset.z -= fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_RIGHT))	vWeaponOffset.x += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_LEFT))	vWeaponOffset.x -= fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_JUMP))			vWeaponOffset.y += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_DUCK))			vWeaponOffset.y -= fAdjustAmount;

	g_pGameClientShell->GetWeaponModel()->SetWeaponOffset(vWeaponOffset);

	char szBuffer[256];
	sprintf(szBuffer, "Weapon Offset (%.3f, %.3f, %.3f)\n", vWeaponOffset.x, vWeaponOffset.y, vWeaponOffset.z);
	strcat(szDebug, szBuffer);

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

LTBOOL Update_WMPOS(char *szDebug)
{
	if(!g_pGameClientShell || !g_pWeaponMgr) return LTFALSE;

	LTFLOAT fAdjustAmount = g_vtCheatWMPOSAdjust.GetFloat();
	WEAPON *pWeapon = g_pGameClientShell->GetWeaponModel()->GetWeapon();

	char szBuffer[256];

	if(!pWeapon) return LTFALSE;

	if((g_nWMPOSBarrel < 0) || (g_nWMPOSBarrel >= NUM_BARRELS))//pWeapon->nNumBarrelTypes))
	{
		sprintf(szBuffer, "Weapon Barrel #%d Invalid (Muzzle Offset)\n", g_nWMPOSBarrel);
		strcat(szDebug, szBuffer);
		return LTFALSE;
	}

	BARREL *pBarrel = g_pWeaponMgr->GetBarrel(pWeapon->aBarrelTypes[g_nWMPOSBarrel]);
	LTVector vBarrelOffset = pBarrel->vMuzzlePos;

	if(g_pLTClient->IsCommandOn(COMMAND_ID_RUN))
		fAdjustAmount *= 2.0f;

	if(g_pLTClient->IsCommandOn(COMMAND_ID_FORWARD))		vBarrelOffset.z += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_BACKWARD))		vBarrelOffset.z -= fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_RIGHT))	vBarrelOffset.x += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_LEFT))	vBarrelOffset.x -= fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_JUMP))			vBarrelOffset.y += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_DUCK))			vBarrelOffset.y -= fAdjustAmount;

	pBarrel->vMuzzlePos = vBarrelOffset;

	sprintf(szBuffer, "Weapon Barrel #%d Muzzle Offset (%.3f, %.3f, %.3f)\n", g_nWMPOSBarrel, vBarrelOffset.x, vBarrelOffset.y, vBarrelOffset.z);
	strcat(szDebug, szBuffer);

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

LTBOOL Update_BREACH(char *szDebug)
{
	if(!g_pGameClientShell || !g_pWeaponMgr) return LTFALSE;

	LTFLOAT fAdjustAmount = g_vtCheatBREACHAdjust.GetFloat();
	WEAPON *pWeapon = g_pGameClientShell->GetWeaponModel()->GetWeapon();

	char szBuffer[256];

	if(!pWeapon) return LTFALSE;

	if((g_nBREACHBarrel < 0) || (g_nBREACHBarrel >= NUM_BARRELS))//pWeapon->nNumBarrelTypes))
	{
		sprintf(szBuffer, "Weapon Barrel #%d Invalid (Breach Offset)\n", g_nBREACHBarrel);
		strcat(szDebug, szBuffer);
		return LTFALSE;
	}

	BARREL *pBarrel = g_pWeaponMgr->GetBarrel(pWeapon->aBarrelTypes[g_nBREACHBarrel]);
	LTFLOAT fBreachOffset = pBarrel->fHHBreachOffset;

	if(g_pLTClient->IsCommandOn(COMMAND_ID_RUN))
		fAdjustAmount *= 2.0f;

	if(g_pLTClient->IsCommandOn(COMMAND_ID_FORWARD))		fBreachOffset += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_BACKWARD))		fBreachOffset -= fAdjustAmount;

	pBarrel->fHHBreachOffset = fBreachOffset;

	sprintf(szBuffer, "Weapon Barrel #%d Breach Offset (%.3f)\n", g_nBREACHBarrel, fBreachOffset);
	strcat(szDebug, szBuffer);

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

LTBOOL Update_LIGHTSCALE(char *szDebug)
{
	LTVector vScale;
	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
	g_pGameClientShell->GetCameraMgr()->GetCameraLightScale(hCamera, vScale);

	LTFLOAT fAdjustAmount = g_vtCheatLIGHTSCALEAdjust.GetFloat();

	if(g_pLTClient->IsCommandOn(COMMAND_ID_RUN))
		fAdjustAmount *= 2.0f;

	if(g_pLTClient->IsCommandOn(COMMAND_ID_FORWARD))		vScale.z += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_BACKWARD))		vScale.z -= fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_RIGHT))	vScale.x += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_LEFT))	vScale.x -= fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_JUMP))			vScale.y += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_DUCK))			vScale.y -= fAdjustAmount;

	if(vScale.x < 0.0f) vScale.x = 0.0f;
	else if(vScale.x > 1.0f) vScale.x = 1.0f;

	if(vScale.y < 0.0f) vScale.y = 0.0f;
	else if(vScale.y > 1.0f) vScale.y = 1.0f;

	if(vScale.z < 0.0f) vScale.z = 0.0f;
	else if(vScale.z > 1.0f) vScale.z = 1.0f;

	char szBuffer[256];
	sprintf(szBuffer, "Global Light Scale (%.3f, %.3f, %.3f)\n", vScale.x, vScale.y, vScale.z);
	strcat(szDebug, szBuffer);

	g_pGameClientShell->GetCameraMgr()->SetCameraLightScale(hCamera, vScale);

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

LTBOOL Update_LIGHTADD(char *szDebug)
{
	if(!g_pGameClientShell) return LTFALSE;

	LTVector vLight;
	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
	g_pGameClientShell->GetCameraMgr()->GetCameraLightAdd(hCamera, vLight);

	LTFLOAT fAdjustAmount = g_vtCheatLIGHTADDAdjust.GetFloat();

	if(g_pLTClient->IsCommandOn(COMMAND_ID_RUN))
		fAdjustAmount *= 2.0f;

	if(g_pLTClient->IsCommandOn(COMMAND_ID_FORWARD))		vLight.z += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_BACKWARD))		vLight.z -= fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_RIGHT))	vLight.x += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_LEFT))	vLight.x -= fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_JUMP))			vLight.y += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_DUCK))			vLight.y -= fAdjustAmount;

	if(vLight.x < 0.0f) vLight.x = 0.0f;
	else if(vLight.x > 1.0f) vLight.x = 1.0f;

	if(vLight.y < 0.0f) vLight.y = 0.0f;
	else if(vLight.y > 1.0f) vLight.y = 1.0f;

	if(vLight.z < 0.0f) vLight.z = 0.0f;
	else if(vLight.z > 1.0f) vLight.z = 1.0f;

	char szBuffer[256];
	sprintf(szBuffer, "Player Camera Light Add (%.3f, %.3f, %.3f)\n", vLight.x, vLight.y, vLight.z);
	strcat(szDebug, szBuffer);

	g_pGameClientShell->GetCameraMgr()->SetCameraLightAdd(hCamera, vLight);

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

LTBOOL Update_VERTEXTINT(char *szDebug)
{
	if(!g_pGameClientShell) return LTFALSE;

	LTVector vTint;
	g_pLTClient->GetGlobalVertexTint(&vTint);

	LTFLOAT fAdjustAmount = g_vtCheatVERTEXTINTAdjust.GetFloat();

	if(g_pLTClient->IsCommandOn(COMMAND_ID_RUN))
		fAdjustAmount *= 2.0f;

	if(g_pLTClient->IsCommandOn(COMMAND_ID_FORWARD))		vTint.z += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_BACKWARD))		vTint.z -= fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_RIGHT))	vTint.x += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_LEFT))	vTint.x -= fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_JUMP))			vTint.y += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_DUCK))			vTint.y -= fAdjustAmount;

	if(vTint.x < 0.0f) vTint.x = 0.0f;
	else if(vTint.x > 1.0f) vTint.x = 1.0f;

	if(vTint.y < 0.0f) vTint.y = 0.0f;
	else if(vTint.y > 1.0f) vTint.y = 1.0f;

	if(vTint.z < 0.0f) vTint.z = 0.0f;
	else if(vTint.z > 1.0f) vTint.z = 1.0f;

	char szBuffer[256];
	sprintf(szBuffer, "Vertex Tint (%.3f, %.3f, %.3f)\n", vTint.x, vTint.y, vTint.z);
	strcat(szDebug, szBuffer);

	g_pLTClient->SetGlobalVertexTint(&vTint);

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

LTBOOL Update_FOV(char *szDebug)
{
	if(!g_pGameClientShell) return LTFALSE;

	LTFLOAT fFOVX, fFOVY;
	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
	g_pGameClientShell->GetCameraMgr()->GetCameraFOV(hCamera, fFOVX, fFOVY);

	LTFLOAT fAdjustAmount = g_vtCheatFOVAdjust.GetFloat();

	if(g_pLTClient->IsCommandOn(COMMAND_ID_RUN))
		fAdjustAmount *= 2.0f;

	if(g_pLTClient->IsCommandOn(COMMAND_ID_FORWARD))		fFOVY += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_BACKWARD))		fFOVY -= fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_RIGHT))	fFOVX += fAdjustAmount;
	if(g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_LEFT))	fFOVX -= fAdjustAmount;

	if(fFOVX < 0.0f) fFOVX = 0.0f;
	else if(fFOVX > 180.0f) fFOVX = 180.0f;

	if(fFOVY < 0.0f) fFOVY = 0.0f;
	else if(fFOVY > 180.0f) fFOVY = 180.0f;

	char szBuffer[256];
	sprintf(szBuffer, "Player Camera FOV (%.2f, %.2f)\n", fFOVX, fFOVY);
	strcat(szDebug, szBuffer);

	g_pGameClientShell->GetCameraMgr()->SetCameraFOV(hCamera, fFOVX, fFOVY);

	return LTFALSE;
}

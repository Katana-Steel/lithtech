// LayoutMgr.cpp: implementation of the CLayoutMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LayoutMgr.h"
#include "FolderMgr.h"

//attribute (bute) file defines
;
const char* const LO_BASIC_TAG				=	"BasicLayout";
const char* const LO_DEFAULT_TAG			=	"GenericFolder";
const char* const LO_MISC_TAG				=	"Miscellaneous";

const char* const LO_BASIC_BACK_POS			=	"BackPos";
const char* const LO_BASIC_CHANGE_SND		=	"ChangeSound";
const char* const LO_BASIC_SELECT_SND		=	"SelectSound";
const char* const LO_BASIC_ESCAPE_SND		=	"EscapeSound";
const char* const LO_BASIC_NO_SELECT_SND	=	"UnselectableSound";
const char* const LO_BASIC_LEFT_SND			=	"SliderLeftSound";
const char* const LO_BASIC_RIGHT_SND		=	"SliderRightSound";

const char* const LO_FOLDER_TITLE_POS		=	"TitlePos";
const char* const LO_FOLDER_TITLE_ALIGN		=	"TitleAlign";
const char* const LO_FOLDER_BACKGROUND		=	"Background";
const char* const LO_FOLDER_PAGE_RECT		=	"PageRect";
const char* const LO_FOLDER_ITEM_SPACE		=	"ItemSpace";
const char* const LO_FOLDER_ITEM_ALIGN		=	"ItemAlign";
const char* const LO_FOLDER_NORMAL			=	"NormalColor";
const char* const LO_FOLDER_SELECTED		=	"SelectedColor";
const char* const LO_FOLDER_DISABLED		=	"DisabledColor";
const char* const LO_FOLDER_HIGHLIGHT		=	"HighlightColor";
const char* const LO_FOLDER_HELP_RECT		=	"HelpRect";
const char* const LO_FOLDER_FONT_SIZE		=	"FontSize";
const char* const LO_FOLDER_TAB_POS			=	"TabPos";
const char* const LO_FOLDER_LINK_POS		=	"LinkPos";
const char* const LO_FOLDER_AMBIENT_LOOP	=	"AmbientLoop";

const char* const LO_MISC_FLASHSPEED		=	"FlashSpeed";
const char* const LO_MISC_FLASHDUR			=	"FlashDuration";
const char* const LO_MISC_WPN_COLOR			=	"WeaponPickupColor";
const char* const LO_MISC_AMMO_COLOR		=	"AmmoPickupColor";
const char* const LO_MISC_TINTTIME			=	"TintTime";
const char* const LO_MISC_GAPMIN			=	"CrosshairGapMin";
const char* const LO_MISC_GAPMAX			=	"CrosshairGapMax";
const char* const LO_MISC_BARMIN			=	"CrosshairBarMin";
const char* const LO_MISC_BARMAX			=	"CrosshairBarMax";
const char* const LO_MISC_ROTEFFECT			=	"PerturbRotationEffect";
const char* const LO_MISC_PERTURBINC		=	"PerturbIncreaseSpeed";
const char* const LO_MISC_PERTURBDEC		=	"PerturbDecreaseSpeed";
const char* const LO_MISC_WALKPER			=	"PerturbWalkPercent";
const char* const LO_MISC_SCOPESPRITE		=	"ScopeMaskSprite";
const char* const LO_MISC_SCOPESCALE		=	"ScopeMaskScale";
const char* const LO_MISC_SCUBASPRITE		=	"ScubaMaskSprite";
const char* const LO_MISC_SCUBASCALE		=	"ScubaMaskScale";
const char* const LO_MISC_MB_BACK			=	"MessageBoxBackground";
const char* const LO_MISC_FAILBACK			=	"FailScreenBackground";
const char* const LO_MISC_FAILPOS			=	"FailStringPos";
const char* const LO_MISC_FAILDELAY			=	"FailScreenDelay";
const char* const LO_MISC_DEATHDELAY		=	"DeathDelay";
const char* const LO_MISC_HELPFONT			=	"HelpFont";
const char* const LO_MISC_TINYFONT			=	"TinyFontBase";
const char* const LO_MISC_SMALLFONT			=	"SmallFontBase";
const char* const LO_MISC_LARGEFONT			=	"LargeFontBase";
const char* const LO_MISC_TITLEFONT			=	"TitleFont";
const char* const LO_MISC_OBJFONT			=	"ObjectiveFont";
const char* const LO_MISC_MSGFONT			=	"HUDMessageFont";
const char* const LO_MISC_HEALTHFONT		=	"HUDHealthFont";
const char* const LO_MISC_ARMORFONT			=	"HUDArmorFont";
const char* const LO_MISC_AMMOFONT			=	"HUDAmmoFont";
const char* const LO_MISC_AIRFONT			=	"HUDAirFont";
const char* const LO_MISC_CHOOSERFONT		=	"HUDChooserFont";
const char* const LO_MISC_CHOOSER_HCOLOR	=	"HUDChooserHColor";
const char* const LO_MISC_ALIENFONT			=	"AlienFontBase";
const char* const LO_MISC_MARINEFONT		=	"MarineFontBase";
const char* const LO_MISC_PREDATORFONT		=	"PredatorFontBase";

const char* const LO_MISC_PDAFONT			=	"PDAFontBase";
const char* const LO_MISC_MEMOFONT			=	"MEMOFontBase";

const char* const LO_MT_WIDTH				=	"Width";
const char* const LO_MT_NUM_LINES			=	"NumLines";
const char* const LO_MT_LETTER_DELAY		=	"LetterDelay";
const char* const LO_MT_LINE_DELAY			=	"LineDelay";
const char* const LO_MT_LINE_SCROLL			=	"LineScrollTime";
const char* const LO_MT_FADE_DELAY			=	"FadeDelay";
const char* const LO_MT_FADE_TIME			=	"FadeTime";
const char* const LO_MT_POS					=	"Pos";
const char* const LO_MT_TYPE_SOUND			=	"TypeSound";
const char* const LO_MT_SCROLL_SOUND		=	"ScrollSound";

//attribute (bute) file defines=
const char* const HUD_MD_TAG				=	"HUDMotionDetector";
const char* const HUD_MD_SMALL_NUMS			=	"SmallNums";
const char* const HUD_MD_LARGE_NUMS			=	"LargeNums";
const char* const HUD_MD_GRID_OFFSET		=	"GridOffset";
const char* const HUD_MD_BASE_OFFSET		=	"BaseOffset";
const char* const HUD_MD_GRID_TRANS			=	"GridTranslucency";
const char* const HUD_MD_ARC_TRANS			=	"ArcTranslucency";
const char* const HUD_MD_BASE_TRANS			=	"BaseTranslucency";
const char* const HUD_MD_BASE_FILE			=	"BaseFile";
const char* const HUD_MD_ARC_FILE			=	"ArcFile";
const char* const HUD_MD_GRID_FILE			=	"GridFile";
const char* const HUD_MD_BLIP_FILE			=	"BlipFile";
const char* const HUD_MD_NUM_BLIP_IMAGES	=	"NumBlipImages";
const char* const HUD_MD_NUM_GRID_IMAGES	=	"NumGridImages";
const char* const HUD_MD_TRACKER_BLIP_FILE	=	"TrackerBlipFile";
const char* const HUD_MD_TRACKER_ARROW_FILE	=	"TrackerArrowFile";

const char* const HUD_CHARACTER_INDEX		=	"CharacterIndex";
const char* const HUD_NUM_ELEMENTS			=	"NumElements";
const char* const HUD_NUM_FONTS				=	"NumFonts";
const char* const HUD_HAS_MD				=	"HasMotionDetector";
const char* const HUD_ENABLE_ELEMENT		=	"Enabled";
const char* const HUD_OFFSET_ELEMENT		=	"DestOffset";
const char* const HUD_OFFSET_TYPE			=	"DestOffsetType";
const char* const HUD_SURF_NAME_ELEMENT		=	"ImageFile";
const char* const HUD_FONT_NAME_ELEMENT		=	"FontFile";
const char* const HUD_DRAW_INDEX_ELEMENT	=	"DrawFunctionIndex";
const char* const HUD_TRANSPAR_ELEMENT		=	"IsTransparent";
const char* const HUD_TRANSLU_ELEMENT		=	"IsTranslucent";
const char* const HUD_TRANSLUCY_ELEMENT		=	"Translucency";
const char* const HUD_FONT_INDEX_ELEMENT	=	"FontIndex";
const char* const HUD_UPDATE_INDEX_ELEMENT	=	"UpdateFunctionIndex";
const char* const HUD_NUM_ANIM_ELEMENT		=	"NumAnimFrames";
const char* const HUD_ANIM_NAME_ELEMENT		=	"AnimationFile";
const char* const HUD_ANIM_TRANSL_ELEMENT	=	"AnimTranslucency";
const char* const HUD_FONT_PITCH_ELEMENT	=	"FontPitch";

const char* const FRAME_SURF_NAME			=	"Background";
const char* const FRAME_TRANSPARENT			=	"IsTransparent";
const char* const FRAME_TRANSLUCENT			=	"IsTranslucent";
const char* const FRAME_TRANSLUCENCY		=	"Translucency";
const char* const FRAME_HAS_BACKGROUND		=	"HasBackground";
const char* const FRAME_WIDTH				=	"Width";
const char* const FRAME_HEIGHT				=	"Height";
const char* const FRAME_ENABLED_POS			=	"EnabledPos";
const char* const FRAME_DISABLED_POS		=	"DisabledPos";
const char* const FRAME_HAS_TITLE			=	"HasTitle";
const char* const FRAME_TITLE				=	"Title";
const char* const FRAME_NUM_CONTROLS		=	"NumControls";
const char* const FRAME_CONTROL_INDEX		=	"ControlIndex";

const char* const PAGE_NUM_FRAMES			=	"NumFrames";
const char* const PAGE_FRAME_INDEX			=	"FrameIndex";
const char* const PAGE_HAS_BACKGROUND		=	"HasBackground";
const char* const PAGE_SURF_NAME			=	"Background";
const char* const PAGE_HAS_TITLE			=	"HasTitle";
const char* const PAGE_TITLE				=	"Title";
const char* const PAGE_TITLE_OFFSET			=	"TitleOffset";
const char* const PAGE_PARENT				=	"Parent";

const char* const MENU_FONT_TAG				=	"MenuFonts";
const char* const MENU_NUM_FONT				=	"NumFonts";
const char* const MENU_FONT					=	"Font";
const char* const MENU_FONT_TYPE			=	"FontType";

const char* const CTRL_TAG					=	"Control";
const char* const CTRL_TYPE					=	"ControlType";
const char* const CTRL_STRING				=	"String";
const char* const CTRL_CMD					=	"CommandID";
const char* const CTRL_HELP					=	"HelpID";
const char* const CTRL_FONT					=	"FontIndex";
const char* const CTRL_OFFSET				=	"ControlOffset";
const char* const CTRL_BMP_STRING			=	"ButtonFile";
const char* const CTRL_TEXT_OFFSET			=	"LabelOffset";

const char* const CTRL_SCROLL_TAB			=	"ScrollTabImg";
const char* const CTRL_SCROLL_BAR			=	"ScrollBarImg";
const char* const CTRL_UP_ARROW				=	"UpArrow";
const char* const CTRL_DN_ARROW				=	"DownArrow";
const char* const CTRL_UP_ARROW_PRESS		=	"UpArrowPressed";
const char* const CTRL_DN_ARROW_PRESS		=	"DownArrowPressed";
const char* const CTRL_SCROLL_WIDTH			=	"ScrollWidth";
const char* const CTRL_SCROLL_HEIGHT		=	"ScrollHeight";

// NOLF Stuff
const char* const LO_SUBTITLE_TAG			=	"Subtitle";
const char* const LO_MASK_TAG				=	"Overlay";
const char* const LO_DIALOGUE_TAG			=	"DialogueWindow";
const char* const LO_DECISION_TAG			=	"DecisionWindow";
const char* const LO_MENU_TAG				=	"MenuWindow";
const char* const LO_CHAR_TAG				=	"Character";
const char* const LO_CREDITS_TAG			=	"Credits";

const char* const LO_BASIC_CONT_POS			=	"ContinuePos";
const char* const LO_BASIC_MAIN_POS			=	"MainPos";
const char* const LO_BASIC_BACK_SPRITE		=	"BackSprite";
const char* const LO_BASIC_BACK_SCALE		=	"BackSpriteScale";
const char* const LO_BASIC_ARROW_BACK		=	"ArrowBackSFX";
const char* const LO_BASIC_ARROW_NEXT		=	"ArrowNextSFX";
const char* const LO_BASIC_ARROW_BACK_BMP	=	"ArrowBackBitmap";
const char* const LO_BASIC_ARROW_NEXT_BMP	=	"ArrowNextBitmap";
const char* const LO_BASIC_ARROW_BACK_POS	=	"ArrowBackPos";
const char* const LO_BASIC_ARROW_NEXT_POS	=	"ArrowNextPos";

const char* const LO_SELECT_SLOT_POS		=	"SlotOffset";
const char* const LO_SELECT_UP_POS			=	"UpArrowOffset";
const char* const LO_SELECT_DOWN_POS		=	"DownArrowOffset";

const char* const LO_FOLDER_UP_POS			=	"UpArrowPos";
const char* const LO_FOLDER_DOWN_POS		=	"DownArrowPos";
const char* const LO_FOLDER_CHARACTER		=	"Character";
const char* const LO_FOLDER_ATTACH			=	"Attachment";

const char* const LO_FOLDER_MUSIC_INTENSITY	=	"MusicIntensity";

const char* const LO_MISC_NV_MODEL			=	"NightVisionModelColor";
const char* const LO_MISC_NV_SCREEN			=	"NightVisionScreenTint";
const char* const LO_MISC_IR_MODEL			=	"InfraredModelColor";
const char* const LO_MISC_IR_LIGHT			=	"InfraredLightScale";
const char* const LO_MISC_MD_SCREEN			=	"MineDetectScreenTint";

const char* const LO_MISC_MB_ALPHA			=	"MessageBoxAlpha";

const char* const LO_MISC_MEDFONT			=	"MediumFontBase";

const char* const LO_CHAR_NAME				=	"Name";
const char* const LO_CHAR_MOD				=	"Model";
const char* const LO_CHAR_STYLE				=	"Style";
const char* const LO_CHAR_POS				=	"Pos";
const char* const LO_CHAR_SCALE				=	"Scale";
const char* const LO_CHAR_ROT				=	"Rotation";

const char* const LO_MISC_MSGFOREFONT		=	"MsgForeFont";
const char* const LO_MISC_MSGBACKFONT		=	"MsgBackFont";
const char* const LO_MISC_HUDFOREFONT		=	"HUDForeFont";
const char* const LO_MISC_HUDBACKFONT		=	"HUDBackFont";

const char* const LO_MISC_MSG_FADE			=	"MessageFade";
const char* const LO_MISC_MSG_TIME			=	"MessageTime";
const char* const LO_MISC_MSG_NUM			=	"MaxMessages";
const char* const LO_MISC_OBJ_RECT			=	"ObjectiveRect";
const char* const LO_MISC_POPUP_RECT		=	"PopupTextRect";
const char* const LO_MISC_SUB_TINT			=	"SubtitleTint";
const char* const LO_MISC_HEALTH_TINT		=	"HealthTint";
const char* const LO_MISC_ARMOR_TINT		=	"ArmorTint";
const char* const LO_MISC_AMMO_TINT			=	"AmmoTint";
const char* const LO_MISC_POPUP_TINT		=	"PopupTint";
const char* const LO_MISC_TEAM1_COLOR		=	"Team1Color";
const char* const LO_MISC_TEAM2_COLOR		=	"Team2Color";

const char* const LO_SUBTITLE_NUM_LINES		=	"NumLines";
const char* const LO_SUBTITLE_1ST_WIDTH		=	"Width1stPerson";
const char* const LO_SUBTITLE_1ST_POS		=	"Pos1stPerson";
const char* const LO_SUBTITLE_3RD_WIDTH		=	"Width3rdPerson";
const char* const LO_SUBTITLE_3RD_POS		=	"Pos3rdPerson";

const char* const	LO_WINDOW_POS			=	"Position";
const char* const	LO_WINDOW_SIZE			=	"Size";
const char* const	LO_WINDOW_TEXT_OFFSET	=	"TextOffset";
const char* const	LO_WINDOW_SPACING		=	"Spacing";
const char* const	LO_WINDOW_BACK			=	"Background";
const char* const	LO_WINDOW_ALPHA			=	"Alpha";
const char* const	LO_WINDOW_FRAME			=	"Frame";
const char* const	LO_WINDOW_FONT			=	"Font";
const char* const	LO_WINDOW_OPEN			=	"OpenTime";
const char* const	LO_WINDOW_CLOSE			=	"CloseTime";
const char* const	LO_WINDOW_SND_OPEN		=	"OpenSound";
const char* const	LO_WINDOW_SND_CLOSE		=	"CloseSound";

const char* const	LO_CREDITS_FADEIN		=	"FadeInTime";
const char* const	LO_CREDITS_HOLD			=	"HoldTime";
const char* const	LO_CREDITS_FADEOUT		=	"FadeOutTime";
const char* const	LO_CREDITS_DELAY		=	"DelayTime";
const char* const	LO_CREDITS_POS_UL		=	"PositionUL";
const char* const	LO_CREDITS_POS_UR		=	"PositionUR";
const char* const	LO_CREDITS_POS_LR		=	"PositionLR";
const char* const	LO_CREDITS_POS_LL		=	"PositionLL";

CLayoutMgr* g_pLayoutMgr = LTNULL;

const char* const s_aFolderTag[FOLDER_ID_UNASSIGNED+1] =
{
	LO_DEFAULT_TAG,			//FOLDER_ID_NONE,
	"FolderMain",			//FOLDER_ID_MAIN,
	"FolderSingle",			//FOLDER_ID_SINGLE,
	"FolderEscape",			//FOLDER_ID_ESCAPE,
	"FolderOptions",		//FOLDER_ID_OPTIONS,
	"FolderProfile",		//FOLDER_ID_PROFILE,
	"FolderDisplay",		//FOLDER_ID_DISPLAY,
	"FolderAudio",			//FOLDER_ID_AUDIO,
	"FolderGame",			//FOLDER_ID_GAME,
	"FolderCredits",		//FOLDER_ID_CREDITS,
	"FolderPerformance",	//FOLDER_ID_PERFORMANCE,
	"FolderCustomControls",	//FOLDER_ID_CUST_CONTROLS,
	"FolderJoystick",		//FOLDER_ID_JOYSTICK,
	"FolderMouse",			//FOLDER_ID_MOUSE,
	"FolderKeyboard",		//FOLDER_ID_KEYBOARD,
	"FolderCustomLevel",    //FOLDER_ID_CUSTOM_LEVEL,
	"FolderMarineMissions",	//FOLDER_ID_MARINE_MISSIONS,
	"FolderPredatorMissions",	//FOLDER_ID_PREDATOR_MISSIONS,
	"FolderAlienMissions",	//FOLDER_ID_ALIEN_MISSIONS,
	"FolderLoad",			//FOLDER_ID_LOAD,
	"FolderSave",			//FOLDER_ID_SAVE,
	"FolderMulti",			//FOLDER_ID_MULTIPLAYER,
	"FolderPlayer",			//FOLDER_ID_PLAYER,
	"FolderPlayerJoin",		//FOLDER_ID_PLAYER_JOIN,
	"FolderJoin",			//FOLDER_ID_JOIN,
	"FolderJoinInfo",		//FOLDER_ID_JOIN_INFO,
	"FolderHost",			//FOLDER_ID_HOST,
	"FolderSetupDM",		//FOLDER_ID_HOST_SETUP_DM,
	"FolderSetupTeamDM",	//FOLDER_ID_HOST_SETUP_TEAMDM,
	"FolderSetupHunt",		//FOLDER_ID_HOST_SETUP_HUNT,
	"FolderSetupSurvivor",	//FOLDER_ID_HOST_SETUP_SURVIVOR,
	"FolderSetupOverrun",	//FOLDER_ID_HOST_SETUP_OVERRUN,
	"FolderSetupEvac",		//FOLDER_ID_HOST_SETUP_EVAC,
	"FolderHostOptions",	//FOLDER_ID_HOST_OPTS,
	"FolderHostMaps",		//FOLDER_ID_HOST_MAPS,
	"FolderHostConfig",		//FOLDER_ID_HOST_CONFIG,
	"LoadScreenAlien",		//FOLDER_ID_LOADSCREEN_ALIEN,
	"LoadScreenMarine",		//FOLDER_ID_LOADSCREEN_MARINE,
	"LoadScreenPredator",	//FOLDER_ID_LOADSCREEN_PREDATOR,
	"LoadScreenMulti",		//FOLDER_ID_LOADSCREEN_MULTI,

	LO_DEFAULT_TAG			//FOLDER_ID_UNASSIGNED,

};

static char s_aTagName[30];
static char s_aAttName[30];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

INT_CHAR::INT_CHAR()
{
	szName[0] = LTNULL;

	szModel[0] = LTNULL;
	szStyle[0] = LTNULL;

    vPos.Init();
    fScale = 1.0f;
	fRot = 0.0f;
}

LTBOOL INT_CHAR::Init(CButeMgr & buteMgr, char* aTagName)
{

	CString str = "";
	str = buteMgr.GetString(aTagName, LO_CHAR_NAME);
	if (str.GetLength() == 0) return LTFALSE;
	strncpy(szName, (char*)(LPCSTR)str, sizeof(szName));


	str = "";
	str = buteMgr.GetString(aTagName, LO_CHAR_MOD);
	if (str.GetLength() == 0) return LTFALSE;
	strncpy(szModel, (char*)(LPCSTR)str, sizeof(szModel));

	str = "";
	str = buteMgr.GetString(aTagName, LO_CHAR_STYLE);
	if (str.GetLength() == 0) return LTFALSE;
	strncpy(szStyle, (char*)(LPCSTR)str, sizeof(szStyle));


	vPos = buteMgr.GetVector(aTagName, LO_CHAR_POS, CAVector(0.0,0.0,0.0));
	fScale = (LTFLOAT)buteMgr.GetDouble(aTagName, LO_CHAR_SCALE, 1.0);
	fRot = (LTFLOAT)buteMgr.GetDouble(aTagName, LO_CHAR_ROT, 1.0);

	return LTTRUE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLayoutMgr::CLayoutMgr()
{

}

CLayoutMgr::~CLayoutMgr()
{
	Term();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLayoutMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //
	
LTBOOL CLayoutMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	if (g_pLayoutMgr || !szAttributeFile) return LTFALSE;
	if (!Parse(pInterface, szAttributeFile)) return LTFALSE;


	// Set up global pointer...

	g_pLayoutMgr = this;

	sprintf(s_aTagName, "%s0", LO_CHAR_TAG);
	int numChar = 0;
	while (m_buteMgr.Exist(s_aTagName))
	{
		INT_CHAR *pChar = new INT_CHAR;
		if (pChar->Init(m_buteMgr,s_aTagName))
			m_CharacterArray.Add(pChar);
		else
		{
			delete pChar;
		}
		numChar++;
		sprintf(s_aTagName, "%s%d", LO_CHAR_TAG, numChar);
	}


	m_nNumTabs = 0;
	sprintf(s_aAttName, "%s0", LO_FOLDER_TAB_POS);
	while (m_buteMgr.Exist(LO_DEFAULT_TAG,s_aAttName) && m_nNumTabs < MAX_TABS)
	{
		m_TabPos[m_nNumTabs] = GetPoint(LO_DEFAULT_TAG, s_aAttName);
		m_nNumTabs++;
		
		sprintf(s_aAttName, "%s%d", LO_FOLDER_TAB_POS, m_nNumTabs);
	}

	m_nNumLinks = 0;
	sprintf(s_aAttName, "%s0", LO_FOLDER_LINK_POS);
	while (m_buteMgr.Exist(LO_DEFAULT_TAG,s_aAttName) && m_nNumTabs < MAX_TABS)
	{
		m_LinkPos[m_nNumLinks] = GetPoint(LO_DEFAULT_TAG, s_aAttName);
		m_nNumLinks++;
		sprintf(s_aAttName, "%s%d", LO_FOLDER_LINK_POS, m_nNumLinks);
	}
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLayoutMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CLayoutMgr::Term()
{
	g_pLayoutMgr = LTNULL;

	while (m_CharacterArray.GetSize())
	{
		delete m_CharacterArray[0];
		m_CharacterArray.Remove(0);

	}
}




// ------------------------------------------------------------------------//
//
//	Miscellaneous Layout 
//
// ------------------------------------------------------------------------//

//DFLOAT	CLayoutMgr::GetFlashSpeed()
//{
//	return (DFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_FLASHSPEED, 0.0f);
//}

//DFLOAT	CLayoutMgr::GetFlashDuration()
//{
//	return (DFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_FLASHDUR, 0.0f);
//}

DVector CLayoutMgr::GetWeaponPickupColor()
{
	CAVector vRet(0.0,0.0,0.0);

	return m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_WPN_COLOR, vRet);
}


DVector CLayoutMgr::GetAmmoPickupColor()
{
	CAVector vRet(0.0,0.0,0.0);

	return m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_AMMO_COLOR, vRet);
}

//DVector CLayoutMgr::GetChooserHighlightColor()
//{
//	CAVector vRet(0.0,0.0,0.0);
//
//	return m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_CHOOSER_HCOLOR, vRet);
//}

DFLOAT CLayoutMgr::GetTintTime()
{
	return (DFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_TINTTIME, 0.0f);
}

DFLOAT CLayoutMgr::GetPerturbRotationEffect()
{
	return (DFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_ROTEFFECT, 0.0f);
}

DFLOAT CLayoutMgr::GetPerturbIncreaseSpeed()
{
	return (DFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_PERTURBINC, 0.0f);
}

DFLOAT CLayoutMgr::GetPerturbDecreaseSpeed()
{
	return (DFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_PERTURBDEC, 0.0f);
}

DFLOAT CLayoutMgr::GetPerturbWalkPercent()
{
	return (DFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_WALKPER, 0.0f);
}






void CLayoutMgr::GetScopeMaskSprite(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_SCOPESPRITE, pBuf, nBufLen);
}


DFLOAT CLayoutMgr::GetScopeMaskScale()
{
	return (DFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_SCOPESCALE, 0.0f);
}

//void CLayoutMgr::GetScubaMaskSprite(char *pBuf, int nBufLen)
//{
//	GetString(LO_MISC_TAG, LO_MISC_SCUBASPRITE, pBuf, nBufLen);
//}

//DFLOAT CLayoutMgr::GetScubaMaskScale()
//{
//	return (DFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_SCUBASCALE, 0.0f);
//}


void CLayoutMgr::GetMessageBoxBackground(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_MB_BACK, pBuf, nBufLen);
}

void CLayoutMgr::GetFailScreenBackground(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_FAILBACK, pBuf, nBufLen);
}


DIntPt	CLayoutMgr::GetFailStringPos()
{
	return GetPoint(LO_MISC_TAG, LO_MISC_FAILPOS);
}

DFLOAT	CLayoutMgr::GetFailScreenDelay()
{
	return (DFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_FAILDELAY, 0.0f);
}

DFLOAT	CLayoutMgr::GetDeathDelay()
{
	return (DFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_DEATHDELAY, 0.0f);
}


void CLayoutMgr::GetSmallFontBase(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_SMALLFONT, pBuf, nBufLen);
}

void CLayoutMgr::GetTinyFontBase(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_TINYFONT, pBuf, nBufLen);
}

void CLayoutMgr::GetLargeFontBase(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_LARGEFONT, pBuf, nBufLen);
}


void CLayoutMgr::GetMessageFont(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_MSGFONT, pBuf, nBufLen);
}

void CLayoutMgr::GetObjectiveFont(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_OBJFONT, pBuf, nBufLen);
}

void CLayoutMgr::GetAlienFontBase(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_ALIENFONT, pBuf, nBufLen);
}

void CLayoutMgr::GetMarineFontBase(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_MARINEFONT, pBuf, nBufLen);
}

void CLayoutMgr::GetPredatorFontBase(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_PREDATORFONT, pBuf, nBufLen);
}

void CLayoutMgr::GetPDAFontBase(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_PDAFONT, pBuf, nBufLen);
}

void CLayoutMgr::GetMEMOFontBase(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_MEMOFONT, pBuf, nBufLen);
}


// ------------------------------------------------------------------------//
//
//	Private Helper functions
//
// ------------------------------------------------------------------------//

LTBOOL CLayoutMgr::GetBool(const char *pTag,const char *pAttribute)
{
	return (LTBOOL) m_buteMgr.GetInt(pTag,pAttribute, 0);
}

DFLOAT CLayoutMgr::GetFloat(const char *pTag,const char *pAttribute)
{
	return (DFLOAT)m_buteMgr.GetDouble(pTag, pAttribute, 0.0f);
}

int	CLayoutMgr::GetInt(const char *pTag,const char *pAttribute)
{
	return m_buteMgr.GetInt(pTag, pAttribute, 0);
}

DIntPt CLayoutMgr::GetPoint(const char *pTag,const char *pAttribute)
{
	CPoint tmp = m_buteMgr.GetPoint(pTag, pAttribute, CPoint(0,0));
	DIntPt pt(tmp.x,tmp.y);
	return pt;
}

DRect CLayoutMgr::GetRect(const char *pTag,const char *pAttribute)
{
	CRect tmp = m_buteMgr.GetRect(pTag, pAttribute, CRect(0,0,0,0) );
	DRect rect (tmp.left,tmp.top,tmp.right,tmp.bottom);
	return rect;

}

void CLayoutMgr::GetString(const char *pTag,const char *pAttribute,char *pBuf, int nBufLen)
{
	CString str = "";
	str = m_buteMgr.GetString(pTag, pAttribute);
	strncpy(pBuf, (char*)(LPCSTR)str, nBufLen);
}

DVector CLayoutMgr::GetVector(const char *pTag,const char *pAttribute)
{
	CAVector vRet(0.0,0.0,0.0);
	return m_buteMgr.GetVector(pTag, pAttribute, vRet);
}

/*****************************************************************/

//new function for AvP3

/*****************************************************************/

DDWORD	CLayoutMgr::GetNumHudElements(char *pTag)
{
	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
		return m_buteMgr.GetInt(pTag, HUD_NUM_ELEMENTS);

	//layout does not exist so return 0
	return 0;
}

DDWORD	CLayoutMgr::GetNumHudFonts(char *pTag)
{
	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
		return m_buteMgr.GetInt(pTag, HUD_NUM_FONTS);

	//layout does not exist so return 0
	return 0;
}

LTBOOL CLayoutMgr::GetHasMotionDetector(char *pTag)
{
	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
		return GetBool(pTag, HUD_HAS_MD);

	//layout does not exist so return 0
	return 0;
}


LTBOOL CLayoutMgr::GetElementEnabled(char *pTag, DDWORD nIndex)
{
	char		s_aTagName[30];	//temp var for bute info

	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
	{
		//catenate tag and layout number
		sprintf(s_aTagName, "%s%d", HUD_ENABLE_ELEMENT, nIndex);

		return GetBool(pTag, s_aTagName);
	}

	//layout does not exist so return 0
	return 0;
}

DIntPt CLayoutMgr::GetElementOffset(char *pTag, DDWORD nIndex)
{
	char		s_aTagName[30];	//temp var for bute info
	DIntPt		nullPt(0,0);// = {0,0};

	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
	{
		//catenate tag and layout number
		sprintf(s_aTagName, "%s%d", HUD_OFFSET_ELEMENT, nIndex);

		return GetPoint(pTag, s_aTagName);
	}

	//layout does not exist so return nullPt
	return nullPt;
}

DDWORD CLayoutMgr::GetElementOffsetType(char *pTag, DDWORD nIndex)
{
	char		s_aTagName[30];	//temp var for bute info

	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
	{
		//catenate tag and layout number
		sprintf(s_aTagName, "%s%d", HUD_OFFSET_TYPE, nIndex);

		return m_buteMgr.GetInt(pTag, s_aTagName);
	}

	//layout does not exist so return 0
	return 0;
}

void CLayoutMgr::GetSurfaceName(char *pTag, DDWORD nIndex, char *pBuf, int nBufLen)
{
	char		s_aTagName[30];	//temp var for bute info

	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
	{
		//catenate tag and layout number
		sprintf(s_aTagName, "%s%d", HUD_SURF_NAME_ELEMENT, nIndex);

		GetString(pTag, s_aTagName, pBuf, nBufLen);
	}
}

void CLayoutMgr::GetFontName(char *pTag, DDWORD nIndex, char *pBuf, int nBufLen)
{
	char		s_aTagName[30];	//temp var for bute info

	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
	{
		//catenate tag and layout number
		sprintf(s_aTagName, "%s%d", HUD_FONT_NAME_ELEMENT, nIndex);

		GetString(pTag, s_aTagName, pBuf, nBufLen);
	}
}

DDWORD CLayoutMgr::GetDrawIndex(char *pTag, DDWORD nIndex)
{
	char		s_aTagName[30];	//temp var for bute info

	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
	{
		//catenate tag and layout number
		sprintf(s_aTagName, "%s%d", HUD_DRAW_INDEX_ELEMENT, nIndex);

		return m_buteMgr.GetInt(pTag, s_aTagName);
	}

	//layout does not exist so return 0
	return 0;
}

DDWORD CLayoutMgr::GetUpdateIndex(char *pTag, DDWORD nIndex)
{
	char		s_aTagName[30];	//temp var for bute info

	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
	{
		//catenate tag and layout number
		sprintf(s_aTagName, "%s%d", HUD_UPDATE_INDEX_ELEMENT, nIndex);

		return m_buteMgr.GetInt(pTag, s_aTagName);
	}

	//layout does not exist so return 0
	return 0;
}

LTBOOL CLayoutMgr::GetElementTransparent(char *pTag, DDWORD nIndex)
{
	char		s_aTagName[30];	//temp var for bute info

	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
	{
		//catenate tag and layout number
		sprintf(s_aTagName, "%s%d", HUD_TRANSPAR_ELEMENT, nIndex);

		return GetBool(pTag, s_aTagName);
	}

	//layout does not exist so return 0
	return 0;
}

LTBOOL CLayoutMgr::GetElementTranslucent(char *pTag, DDWORD nIndex)
{
	char		s_aTagName[30];	//temp var for bute info

	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
	{
		//catenate tag and layout number
		sprintf(s_aTagName, "%s%d", HUD_TRANSLU_ELEMENT, nIndex);

		return GetBool(pTag, s_aTagName);
	}

	//layout does not exist so return 0
	return 0;
}

DFLOAT CLayoutMgr::GetElementTranslucency(char *pTag, DDWORD nIndex)
{
	char		s_aTagName[30];	//temp var for bute info

	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
	{
		//catenate tag and layout number
		sprintf(s_aTagName, "%s%d", HUD_TRANSLUCY_ELEMENT, nIndex);

		return GetFloat(pTag, s_aTagName);
	}

	//layout does not exist so return 1
	return 1.0f;
}

DFLOAT CLayoutMgr::GetAmimTranslucency(char *pTag, DDWORD nIndex)
{
	char		s_aTagName[30];	//temp var for bute info

	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
	{
		//catenate tag and layout number
		sprintf(s_aTagName, "%s%d", HUD_ANIM_TRANSL_ELEMENT, nIndex);

		return GetFloat(pTag, s_aTagName);
	}

	//layout does not exist so return 1
	return 1.0f;
}

DDWORD CLayoutMgr::GetFontIndex(char *pTag, DDWORD nIndex)
{
	char		s_aTagName[30];	//temp var for bute info

	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
	{
		//catenate tag and layout number
		sprintf(s_aTagName, "%s%d", HUD_FONT_INDEX_ELEMENT, nIndex);

		return m_buteMgr.GetInt(pTag, s_aTagName);
	}

	//layout does not exist so return 0
	return 0;
}

DDWORD CLayoutMgr::GetFontPitch(char *pTag, DDWORD nIndex)
{
	char		s_aTagName[30];	//temp var for bute info

	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
	{
		//catenate tag and layout number
		sprintf(s_aTagName, "%s%d", HUD_FONT_PITCH_ELEMENT, nIndex);

		return m_buteMgr.GetInt(pTag, s_aTagName);
	}

	//layout does not exist so return 0
	return 0;
}

DDWORD CLayoutMgr::GetNumAnim(char *pTag, DDWORD nIndex)
{
	char		s_aTagName[30];	//temp var for bute info

	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
	{
		//catenate tag and layout number
		sprintf(s_aTagName, "%s%d", HUD_NUM_ANIM_ELEMENT, nIndex);

		return m_buteMgr.GetInt(pTag, s_aTagName);
	}

	//layout does not exist so return 0
	return 0;
}

void CLayoutMgr::GetAnimName(char *pTag, DDWORD nIndex, char *pBuf, int nBufLen)
{
	char		s_aTagName[30];	//temp var for bute info

	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
	{
		//catenate tag and layout number
		sprintf(s_aTagName, "%s%d", HUD_ANIM_NAME_ELEMENT, nIndex);

		GetString(pTag, s_aTagName, pBuf, nBufLen);
	}
}

/****************************************************************
//
//	New Menu Bute Functions
//
//
*/

//
void CLayoutMgr::GetFrameBackground(char *pTag, char *pBuf, int nBufLen)
{
	//check to see that the frame exists
	if(m_buteMgr.Exist(pTag))
		GetString(pTag, FRAME_SURF_NAME, pBuf, nBufLen);
}

LTBOOL CLayoutMgr::GetFrameTransparent(char *pTag)
{
	//check to see that the frame exists
	if(m_buteMgr.Exist(pTag))
		return GetBool(pTag, FRAME_TRANSPARENT);

	//frame does not exist so return 0
	return 0;
}

LTBOOL CLayoutMgr::GetFrameTranslucent(char *pTag)
{
	//check to see that the frame exists
	if(m_buteMgr.Exist(pTag))
		return GetBool(pTag, FRAME_TRANSLUCENT);

	//frame does not exist so return 0
	return 0;
}

DFLOAT CLayoutMgr::GetFrameTranslucency(char *pTag)
{
	//check to see that the layout exists
	if(m_buteMgr.Exist(pTag))
		return GetFloat(pTag, FRAME_TRANSLUCENCY);

	//layout does not exist so return 1
	return 1.0f;
}

LTBOOL CLayoutMgr::GetFrameHasBackground(char *pTag)
{
	//check to see that the frame exists
	if(m_buteMgr.Exist(pTag))
		return GetBool(pTag, FRAME_HAS_BACKGROUND);

	//frame does not exist so return 0
	return 0;
}

DDWORD CLayoutMgr::GetFrameWidth(char *pTag)
{
	//check to see that the frame exists
	if(m_buteMgr.Exist(pTag))
		return m_buteMgr.GetInt(pTag, FRAME_WIDTH);

	//frame does not exist so return 0
	return 0;
}

DDWORD CLayoutMgr::GetFrameHeight(char *pTag)
{
	//check to see that the frame exists
	if(m_buteMgr.Exist(pTag))
		return m_buteMgr.GetInt(pTag, FRAME_HEIGHT);

	//frame does not exist so return 0
	return 0;
}

DIntPt CLayoutMgr::GetFrameEnabledPos(char *pTag)
{
	DIntPt		nullPt(0,0);

	//check to see that the frame exists
	if(m_buteMgr.Exist(pTag))
		return GetPoint(pTag, FRAME_ENABLED_POS);

	//frame does not exist so return nullPt
	return nullPt;
}

DIntPt CLayoutMgr::GetFrameDisabledPos(char *pTag)
{
	DIntPt		nullPt(0,0);

	//check to see that the frame exists
	if(m_buteMgr.Exist(pTag))
		return GetPoint(pTag, FRAME_DISABLED_POS);

	//frame does not exist so return nullPt
	return nullPt;
}

LTBOOL CLayoutMgr::GetFrameHasTitle(char *pTag)
{
	//check to see that the frame exists
	if(m_buteMgr.Exist(pTag))
		return GetBool(pTag, FRAME_HAS_TITLE);

	//frame does not exist so return 0
	return 0;
}

void CLayoutMgr::GetFrameTitle(char *pTag, char *pBuf, int nBufLen)
{
	//check to see that the frame exists
	if(m_buteMgr.Exist(pTag))
		GetString(pTag, FRAME_TITLE, pBuf, nBufLen);
}

DDWORD CLayoutMgr::GetNumFrames(char *pTag)
{
	//check to see that the page exists
	if(m_buteMgr.Exist(pTag))
		return m_buteMgr.GetInt(pTag, PAGE_NUM_FRAMES);

	//page does not exist so return 0
	return 0;
}

DDWORD CLayoutMgr::GetFrameIndex(char *pTag, DDWORD nIndex)
{
	char		aTagName[30];	//temp var for bute info

	//check to see that the page exists
	if(m_buteMgr.Exist(pTag))
	{
		//catenate tag and frame number
		sprintf(aTagName, "%s%d", PAGE_FRAME_INDEX, nIndex);

		return m_buteMgr.GetInt(pTag, aTagName);
	}

	//page does not exist so return 0
	return 0;
}

LTBOOL CLayoutMgr::GetPageHasBackground(char *pTag)
{
	//check to see that the page exists
	if(m_buteMgr.Exist(pTag))
		return GetBool(pTag, PAGE_HAS_BACKGROUND);

	//page does not exist so return 0
	return 0;
}

void CLayoutMgr::GetPageBackground(char *pTag, char *pBuf, int nBufLen)
{
	//check to see that the frame exists
	if(m_buteMgr.Exist(pTag))
		GetString(pTag, PAGE_SURF_NAME, pBuf, nBufLen);
}

LTBOOL CLayoutMgr::GetPageHasTitle(char *pTag)
{
	//check to see that the frame exists
	if(m_buteMgr.Exist(pTag))
		return GetBool(pTag, PAGE_HAS_TITLE);

	//frame does not exist so return 0
	return 0;
}

void CLayoutMgr::GetPageTitle(char *pTag, char *pBuf, int nBufLen)
{
	//check to see that the frame exists
	if(m_buteMgr.Exist(pTag))
		GetString(pTag, PAGE_TITLE, pBuf, nBufLen);
}

DIntPt CLayoutMgr::GetPageTitleOffset(char *pTag)
{
	DIntPt		nullPt(0,0);

	//check to see that the frame exists
	if(m_buteMgr.Exist(pTag))
		return GetPoint(pTag, PAGE_TITLE_OFFSET);

	//frame does not exist so return nullPt
	return nullPt;
}

DDWORD CLayoutMgr::GetNumMenuFonts()
{
	//check to see that the page exists
	if(m_buteMgr.Exist(MENU_FONT_TAG))
		return m_buteMgr.GetInt(MENU_FONT_TAG, MENU_NUM_FONT);

	//font tag does not exist so return 0
	return 0;
}

void CLayoutMgr::GetMenuFontFile(char *pBuf, int nBufLen, DDWORD nIndex)
{
	//check to see that the font tag exists
	if(m_buteMgr.Exist(MENU_FONT_TAG))
	{
		char aTemp[60];

		sprintf(aTemp,"%s%d",MENU_FONT,nIndex);

		GetString(MENU_FONT_TAG, aTemp, pBuf, nBufLen);
	}
}

DDWORD CLayoutMgr::GetMenuFontType(DDWORD nIndex)
{
	char		aTagName[30];	//temp var for bute info

	//check to see that the page exists
	if(m_buteMgr.Exist(MENU_FONT_TAG))
	{
		//catenate tag and frame number
		sprintf(aTagName, "%s%d", MENU_FONT_TYPE, nIndex);

		return m_buteMgr.GetInt(MENU_FONT_TAG, aTagName);
	}

	//font tag does not exist so return 0
	return 0;
}

DDWORD CLayoutMgr::GetFrameNumControls(char* aTag)
{
	//check to see that the page exists
	if(m_buteMgr.Exist(aTag))
		return m_buteMgr.GetInt(aTag, FRAME_NUM_CONTROLS);

	//font tag does not exist so return 0
	return 0;
}

DDWORD CLayoutMgr::GetFrameControlIndex(char* aTag, DDWORD nIndex)
{
	char		aTagName[30];	//temp var for bute info

	//check to see that the page exists
	if(m_buteMgr.Exist(aTag))
	{
		//catenate tag and frame number
		sprintf(aTagName, "%s%d", FRAME_CONTROL_INDEX, nIndex);

		return m_buteMgr.GetInt(aTag, aTagName);
	}

	//font tag does not exist so return 0
	return 0;
}

DDWORD CLayoutMgr::GetControlType(DDWORD nIndex)
{
	//check to see that the font tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",CTRL_TAG,nIndex);

	if(m_buteMgr.Exist(aTemp))
		return m_buteMgr.GetInt(aTemp, CTRL_TYPE);

	//font tag does not exist so return 0
	return 0;
}

void CLayoutMgr::GetControlString(char *pBuf, int nBufLen, DDWORD nIndex)
{
	//check to see that the font tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",CTRL_TAG,nIndex);

	if(m_buteMgr.Exist(aTemp))
		GetString(aTemp, CTRL_STRING, pBuf, nBufLen);
}

DDWORD CLayoutMgr::GetControlCmd(DDWORD nIndex)
{
	//check to see that the font tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",CTRL_TAG,nIndex);

	if(m_buteMgr.Exist(aTemp))
		return m_buteMgr.GetInt(aTemp, CTRL_CMD);

	//font tag does not exist so return 0
	return 0;
}

DDWORD CLayoutMgr::GetControlHlp(DDWORD nIndex)
{
	//check to see that the font tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",CTRL_TAG,nIndex);

	if(m_buteMgr.Exist(aTemp))
		return m_buteMgr.GetInt(aTemp, CTRL_HELP);

	//font tag does not exist so return 0
	return 0;
}

DDWORD CLayoutMgr::GetFontIndex(DDWORD nIndex)
{
	//check to see that the font tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",CTRL_TAG,nIndex);

	if(m_buteMgr.Exist(aTemp))
		return m_buteMgr.GetInt(aTemp, CTRL_FONT);

	//font tag does not exist so return 0
	return 0;
}

DIntPt CLayoutMgr::GetPageTitleOffset(DDWORD nIndex)
{
	DIntPt		nullPt(0,0);

	//check to see that the font tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",CTRL_TAG,nIndex);

	//check to see that the frame exists
	if(m_buteMgr.Exist(aTemp))
		return GetPoint(aTemp, CTRL_OFFSET);

	//frame does not exist so return nullPt
	return nullPt;
}

DDWORD CLayoutMgr::GetParentPage(char* aTag)
{
	//check to see that the page exists
	if(m_buteMgr.Exist(aTag))
		return m_buteMgr.GetInt(aTag, PAGE_PARENT);

	//page data does not exist so return 0
	return 0;
}

void CLayoutMgr::GetBitmapControl(char *pBuf, int nBufLen, DDWORD nIndex)
{
	//check to see that the font tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",CTRL_TAG,nIndex);

	if(m_buteMgr.Exist(aTemp))
		GetString(aTemp, CTRL_BMP_STRING, pBuf, nBufLen);
}

DIntPt CLayoutMgr::GetTextLabelOffset(DDWORD nIndex)
{
	DIntPt		nullPt(0,0);

	//check to see that the font tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",CTRL_TAG,nIndex);

	//check to see that the frame exists
	if(m_buteMgr.Exist(aTemp))
		return GetPoint(aTemp, CTRL_TEXT_OFFSET);

	//frame does not exist so return nullPt
	return nullPt;
}

void CLayoutMgr::GetScrollTab(char *pBuf, int nBufLen, DDWORD nIndex)
{
	//check to see that the font tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",CTRL_TAG,nIndex);

	if(m_buteMgr.Exist(aTemp))
		GetString(aTemp, CTRL_SCROLL_TAB, pBuf, nBufLen);
}

void CLayoutMgr::GetScrollBar(char *pBuf, int nBufLen, DDWORD nIndex)
{
	//check to see that the font tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",CTRL_TAG,nIndex);

	if(m_buteMgr.Exist(aTemp))
		GetString(aTemp, CTRL_SCROLL_BAR, pBuf, nBufLen);
}

void CLayoutMgr::GetScrollUpArrow(char *pBuf, int nBufLen, DDWORD nIndex)
{
	//check to see that the font tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",CTRL_TAG,nIndex);

	if(m_buteMgr.Exist(aTemp))
		GetString(aTemp, CTRL_UP_ARROW, pBuf, nBufLen);
}

void CLayoutMgr::GetScrollDownArrow(char *pBuf, int nBufLen, DDWORD nIndex)
{
	//check to see that the font tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",CTRL_TAG,nIndex);

	if(m_buteMgr.Exist(aTemp))
		GetString(aTemp, CTRL_DN_ARROW, pBuf, nBufLen);
}

void CLayoutMgr::GetScrollDownArrowPressed(char *pBuf, int nBufLen, DDWORD nIndex)
{
	//check to see that the font tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",CTRL_TAG,nIndex);

	if(m_buteMgr.Exist(aTemp))
		GetString(aTemp, CTRL_DN_ARROW_PRESS, pBuf, nBufLen);
}

void CLayoutMgr::GetScrollUpArrowPressed(char *pBuf, int nBufLen, DDWORD nIndex)
{
	//check to see that the font tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",CTRL_TAG,nIndex);

	if(m_buteMgr.Exist(aTemp))
		GetString(aTemp, CTRL_UP_ARROW_PRESS, pBuf, nBufLen);
}

DDWORD CLayoutMgr::GetScrollWidth(DDWORD nIndex)
{
	//check to see that the font tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",CTRL_TAG,nIndex);

	if(m_buteMgr.Exist(aTemp))
		return m_buteMgr.GetInt(aTemp, CTRL_SCROLL_WIDTH);

	//font tag does not exist so return 0
	return 0;
}

DDWORD CLayoutMgr::GetScrollHeight(DDWORD nIndex)
{
	//check to see that the font tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",CTRL_TAG,nIndex);

	if(m_buteMgr.Exist(aTemp))
		return m_buteMgr.GetInt(aTemp, CTRL_SCROLL_HEIGHT);

	//font tag does not exist so return 0
	return 0;
}

DDWORD CLayoutMgr::GetDefaultCfg(DDWORD nIndex)
{
	//check to see that the tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",HUD_BASIC_TAG,0);

	for(int i=1; m_buteMgr.Exist(aTemp); i++)
	{
		if(m_buteMgr.GetInt(aTemp, HUD_CHARACTER_INDEX) == (int)nIndex)
			return i-1;
		
		sprintf(aTemp,"%s%d",HUD_BASIC_TAG,i);
	}

	//font tag does not exist so return 0
	return HUD_ERROR;
}

int	CLayoutMgr::GetNumHUDLayouts(DDWORD nType)
{
	//check to see that the tag exists
	char aTemp[60];

	int nRval=0;

	sprintf(aTemp,"%s%d",HUD_BASIC_TAG,0);

	for(int i=1; m_buteMgr.Exist(aTemp); i++)
	{
		if(m_buteMgr.GetInt(aTemp, HUD_CHARACTER_INDEX) == (int)nType)
			nRval++;
		
		sprintf(aTemp,"%s%d",HUD_BASIC_TAG,i);
	}

	//return number of layouts
	return nRval;
}

DDWORD	CLayoutMgr::GetHUDCfg(DDWORD nType, DDWORD nIndex)
{
	//check to see that the tag exists
	char aTemp[60];

	sprintf(aTemp,"%s%d",HUD_BASIC_TAG,0);

	for(int i=1; m_buteMgr.Exist(aTemp); i++)
	{
		if(m_buteMgr.GetInt(aTemp, HUD_CHARACTER_INDEX) == (int)nType)
		{
			if(!(--nIndex+1))
				return i-1;
		}
		
		sprintf(aTemp,"%s%d",HUD_BASIC_TAG,i);
	}

	//tag does not exist so return 0
	return HUD_ERROR;
}

void CLayoutMgr::GetMDLargeFontName(char *pBuf, int nBufLen)
{
	if(m_buteMgr.Exist(HUD_MD_TAG))
		GetString(HUD_MD_TAG, HUD_MD_LARGE_NUMS, pBuf, nBufLen);
}

void CLayoutMgr::GetMDSmallFontName(char *pBuf, int nBufLen)
{
	if(m_buteMgr.Exist(HUD_MD_TAG))
		GetString(HUD_MD_TAG, HUD_MD_SMALL_NUMS, pBuf, nBufLen);
}

DDWORD		CLayoutMgr::GetNumMDBlipImages()
{
	//check to see that the page exists
	if(m_buteMgr.Exist(HUD_MD_TAG))
		return m_buteMgr.GetInt(HUD_MD_TAG, HUD_MD_NUM_BLIP_IMAGES);

	return 0;
}

DDWORD		CLayoutMgr::GetNumMDGridImages()
{
	//check to see that the page exists
	if(m_buteMgr.Exist(HUD_MD_TAG))
		return m_buteMgr.GetInt(HUD_MD_TAG, HUD_MD_NUM_GRID_IMAGES);

	return 0;
}

DIntPt		CLayoutMgr::GetMDGridOffset()
{
	return GetPoint(HUD_MD_TAG, HUD_MD_GRID_OFFSET);
}

DIntPt		CLayoutMgr::GetMDBaseOffset()
{
	return GetPoint(HUD_MD_TAG, HUD_MD_BASE_OFFSET);
}

DFLOAT CLayoutMgr::GetMDGridTranslucency()
{
	return (DFLOAT)m_buteMgr.GetDouble(HUD_MD_TAG, HUD_MD_GRID_TRANS, 0.0f);
}

DFLOAT CLayoutMgr::GetMDBaseTranslucency()
{
	return (DFLOAT)m_buteMgr.GetDouble(HUD_MD_TAG, HUD_MD_BASE_TRANS, 0.0f);
}

DFLOAT CLayoutMgr::GetMDArcTranslucency()
{
	return (DFLOAT)m_buteMgr.GetDouble(HUD_MD_TAG, HUD_MD_ARC_TRANS, 0.0f);
}

void CLayoutMgr::GetArcSurfaceName(char *pBuf, int nBufLen)
{
	if(m_buteMgr.Exist(HUD_MD_TAG))
		GetString(HUD_MD_TAG, HUD_MD_ARC_FILE, pBuf, nBufLen);
}

void CLayoutMgr::GetBaseSurfaceName(char *pBuf, int nBufLen)
{
	if(m_buteMgr.Exist(HUD_MD_TAG))
		GetString(HUD_MD_TAG, HUD_MD_BASE_FILE, pBuf, nBufLen);
}

void CLayoutMgr::GetGridSurfaceName(char *pBuf, int nBufLen)
{
	if(m_buteMgr.Exist(HUD_MD_TAG))
		GetString(HUD_MD_TAG, HUD_MD_GRID_FILE, pBuf, nBufLen);
}

void CLayoutMgr::GetBlipSurfaceName(char *pBuf, int nBufLen)
{	
	if(m_buteMgr.Exist(HUD_MD_TAG))
		GetString(HUD_MD_TAG, HUD_MD_BLIP_FILE, pBuf, nBufLen);
}

void CLayoutMgr::GetTrackerBlipSurfaceName(char *pBuf, int nBufLen)
{
	if(m_buteMgr.Exist(HUD_MD_TAG))
		GetString(HUD_MD_TAG, HUD_MD_TRACKER_BLIP_FILE, pBuf, nBufLen);
}

void CLayoutMgr::GetTrackerArrowSurfaceName(char *pBuf, int nBufLen)
{	
	if(m_buteMgr.Exist(HUD_MD_TAG))
		GetString(HUD_MD_TAG, HUD_MD_TRACKER_ARROW_FILE, pBuf, nBufLen);
}





// ------------------------------------------------------------------------//
//
//	Basic Folder Layout
//
// ------------------------------------------------------------------------//

LTIntPt  CLayoutMgr::GetBackPos()
{
	return GetPoint(LO_BASIC_TAG, LO_BASIC_BACK_POS);
}

LTIntPt CLayoutMgr::GetContinuePos()
{
	return GetPoint(LO_BASIC_TAG, LO_BASIC_CONT_POS);
}

LTIntPt CLayoutMgr::GetMainPos()
{
	return GetPoint(LO_BASIC_TAG, LO_BASIC_MAIN_POS);
}

void CLayoutMgr::GetBackSprite(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_BACK_SPRITE, pBuf, nBufLen);
}

LTFLOAT CLayoutMgr::GetBackSpriteScale()
{
	return GetFloat(LO_BASIC_TAG, LO_BASIC_BACK_SCALE);
}

void CLayoutMgr::GetArrowBackSFX(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_ARROW_BACK, pBuf, nBufLen);
}
void CLayoutMgr::GetArrowNextSFX(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_ARROW_NEXT, pBuf, nBufLen);
}

void CLayoutMgr::GetArrowBackBmp(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_ARROW_BACK_BMP, pBuf, nBufLen);
}
void CLayoutMgr::GetArrowNextBmp(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_ARROW_NEXT_BMP, pBuf, nBufLen);
}

LTIntPt  CLayoutMgr::GetArrowBackPos()
{
	return GetPoint(LO_BASIC_TAG, LO_BASIC_ARROW_BACK_POS);
}

LTIntPt CLayoutMgr::GetArrowNextPos()
{
	return GetPoint(LO_BASIC_TAG, LO_BASIC_ARROW_NEXT_POS);
}

LTIntPt CLayoutMgr::GetTabPos(eFolderID folderId, int nTab)
{
	if (nTab >= MAX_TABS)
	{
		_ASSERT(0);
		return LTIntPt(0,0);
	}
	sprintf(s_aAttName, "%s%d", LO_FOLDER_TAB_POS, nTab);

	const char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,s_aAttName))
	{
		return GetPoint(pTag, s_aAttName);
	}
	else
		return m_TabPos[nTab];
}

LTIntPt CLayoutMgr::GetLinkPos(eFolderID folderId, int nLink)
{
	if (nLink >= MAX_TABS)
	{
		_ASSERT(0);
		return LTIntPt(0,0);
	}
	sprintf(s_aAttName, "%s%d", LO_FOLDER_LINK_POS, nLink);

	const char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,s_aAttName))
	{
		return GetPoint(pTag, s_aAttName);
	}
	else
		return m_LinkPos[nLink];

}


void CLayoutMgr::GetChangeSound(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_CHANGE_SND, pBuf, nBufLen);
}
void CLayoutMgr::GetSelectSound(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_SELECT_SND, pBuf, nBufLen);
}

void CLayoutMgr::GetEscapeSound(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_ESCAPE_SND, pBuf, nBufLen);
}

void CLayoutMgr::GetUnselectableSound(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_NO_SELECT_SND, pBuf, nBufLen);
}

void CLayoutMgr::GetSliderLeftSound(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_LEFT_SND, pBuf, nBufLen);
}

void CLayoutMgr::GetSliderRightSound(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_RIGHT_SND, pBuf, nBufLen);
}



// ------------------------------------------------------------------------//
//
//	Specific Folder Layouts
//
// ------------------------------------------------------------------------//
LTRect   CLayoutMgr::GetFolderHelpRect(eFolderID folderId)
{
	const char* pTag = s_aFolderTag[folderId];

	if (m_buteMgr.Exist(pTag,LO_FOLDER_HELP_RECT))
	{
		return GetRect(pTag, LO_FOLDER_HELP_RECT);
	}
	else
		return GetRect(LO_DEFAULT_TAG, LO_FOLDER_HELP_RECT);
}

LTIntPt CLayoutMgr::GetUpArrowPos(eFolderID folderId)
{
	const char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_UP_POS))
	{
		return GetPoint(pTag, LO_FOLDER_UP_POS);
	}
	return GetPoint(LO_DEFAULT_TAG, LO_FOLDER_UP_POS);
}

LTIntPt CLayoutMgr::GetDownArrowPos(eFolderID folderId)
{
	const char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_DOWN_POS))
	{
		return GetPoint(pTag, LO_FOLDER_DOWN_POS);
	}
	return GetPoint(LO_DEFAULT_TAG, LO_FOLDER_DOWN_POS);
}


LTIntPt  CLayoutMgr::GetFolderTitlePos(eFolderID folderId)
{
	const char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_TITLE_POS))
	{
		return GetPoint(pTag, LO_FOLDER_TITLE_POS);
	}
	else
		return GetPoint(LO_DEFAULT_TAG, LO_FOLDER_TITLE_POS);
}

int  CLayoutMgr::GetFolderFontSize(eFolderID folderId)
{
	const char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_FONT_SIZE))
	{
		return GetInt(pTag, LO_FOLDER_FONT_SIZE);
	}
	else
		return GetInt(LO_DEFAULT_TAG, LO_FOLDER_FONT_SIZE);
}

int		CLayoutMgr::GetFolderTitleAlign(eFolderID folderId)
{
	const char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_TITLE_ALIGN))
	{
		return GetInt(pTag, LO_FOLDER_TITLE_ALIGN);
	}
	else
		return GetInt(LO_DEFAULT_TAG, LO_FOLDER_TITLE_ALIGN);
}


LTRect   CLayoutMgr::GetFolderPageRect(eFolderID folderId)
{
	const char* pTag = s_aFolderTag[folderId];

	if (m_buteMgr.Exist(pTag,LO_FOLDER_PAGE_RECT))
	{
		return GetRect(pTag, LO_FOLDER_PAGE_RECT);
	}
	else
		return GetRect(LO_DEFAULT_TAG, LO_FOLDER_PAGE_RECT);
}

int		CLayoutMgr::GetFolderItemSpacing(eFolderID folderId)
{
	const char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_ITEM_SPACE))
	{
		return GetInt(pTag, LO_FOLDER_ITEM_SPACE);
	}
	else
		return GetInt(LO_DEFAULT_TAG, LO_FOLDER_ITEM_SPACE);
}

int		CLayoutMgr::GetFolderItemAlign(eFolderID folderId)
{
	const char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_ITEM_ALIGN))
	{
		return GetInt(pTag, LO_FOLDER_ITEM_ALIGN);
	}
	else
		return GetInt(LO_DEFAULT_TAG, LO_FOLDER_ITEM_ALIGN);
}

int		CLayoutMgr::GetFolderMusicIntensity(eFolderID folderId)
{
	const char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_MUSIC_INTENSITY))
	{
		return GetInt(pTag, LO_FOLDER_MUSIC_INTENSITY);
	}
	else
		return GetInt(LO_DEFAULT_TAG, LO_FOLDER_MUSIC_INTENSITY);
}


INT_CHAR* CLayoutMgr::GetFolderCharacter(eFolderID folderId)
{
	const char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_CHARACTER))
	{
		char szTest[128];
		GetString(pTag,LO_FOLDER_CHARACTER,szTest,sizeof(szTest));
		int i = 0;
		while (i < (int)m_CharacterArray.GetSize())
		{
			if (stricmp(szTest,m_CharacterArray[i]->szName) == 0)
				return m_CharacterArray[i];
			i++;
		}

	}
	return LTNULL;
}

int	CLayoutMgr::GetFolderNumAttachments(eFolderID folderId)
{
	const char* pTag = s_aFolderTag[folderId];
	int nNum = 0;
	sprintf(s_aAttName, "%s%d", LO_FOLDER_ATTACH, nNum);
	while (m_buteMgr.Exist(pTag,s_aAttName))
	{
		nNum++;
		sprintf(s_aAttName, "%s%d", LO_FOLDER_ATTACH, nNum);
	}
	return nNum;

}


void CLayoutMgr::GetFolderAttachment(eFolderID folderId, int num, char *pBuf, int nBufLen)
{
	const char* pTag = s_aFolderTag[folderId];
	sprintf(s_aAttName, "%s%d", LO_FOLDER_ATTACH, num);
	if (m_buteMgr.Exist(pTag,s_aAttName))
	{
		GetString(pTag,s_aAttName,pBuf,nBufLen);
	}
}

HLTCOLOR CLayoutMgr::GetFolderNormalColor(eFolderID folderId)
{
	const char* pTag = s_aFolderTag[folderId];
	CAVector vDef(0.0,0.0,0.0);
	LTVector vColor;
	if (m_buteMgr.Exist(pTag,LO_FOLDER_NORMAL))
	{
		vColor = m_buteMgr.GetVector(pTag, LO_FOLDER_NORMAL);
	}
	else
		vColor = m_buteMgr.GetVector(LO_DEFAULT_TAG, LO_FOLDER_NORMAL);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

HLTCOLOR CLayoutMgr::GetFolderSelectedColor(eFolderID folderId)
{
	const char* pTag = s_aFolderTag[folderId];
	CAVector vDef(0.0,0.0,0.0);
	LTVector vColor;
	if (m_buteMgr.Exist(pTag,LO_FOLDER_SELECTED))
	{
		vColor = m_buteMgr.GetVector(pTag, LO_FOLDER_SELECTED);
	}
	else
		vColor = m_buteMgr.GetVector(LO_DEFAULT_TAG, LO_FOLDER_SELECTED);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

HLTCOLOR CLayoutMgr::GetFolderDisabledColor(eFolderID folderId)
{
	const char* pTag = s_aFolderTag[folderId];
	CAVector vDef(0.0,0.0,0.0);
	LTVector vColor;
	if (m_buteMgr.Exist(pTag,LO_FOLDER_DISABLED))
	{
		vColor = m_buteMgr.GetVector(pTag, LO_FOLDER_DISABLED);
	}
	else
		vColor = m_buteMgr.GetVector(LO_DEFAULT_TAG, LO_FOLDER_DISABLED);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

HLTCOLOR CLayoutMgr::GetFolderHighlightColor(eFolderID folderId)
{
	const char* pTag = s_aFolderTag[folderId];
	CAVector vDef(0.0,0.0,0.0);
	LTVector vColor;
	if (m_buteMgr.Exist(pTag,LO_FOLDER_HIGHLIGHT))
	{
		vColor = m_buteMgr.GetVector(pTag, LO_FOLDER_HIGHLIGHT);
	}
	else
		vColor = m_buteMgr.GetVector(LO_DEFAULT_TAG, LO_FOLDER_HIGHLIGHT);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

void CLayoutMgr::GetFolderAmbient(eFolderID folderId, char *pBuf, int nBufLen)
{
	const char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_AMBIENT_LOOP))
	{
		GetString(pTag,LO_FOLDER_AMBIENT_LOOP,pBuf,nBufLen);
	}
	else
	{
		GetString(LO_DEFAULT_TAG,LO_FOLDER_AMBIENT_LOOP,pBuf,nBufLen);
	}
}

//* Custom Folder values
LTBOOL CLayoutMgr::HasCustomValue(eFolderID folderId, char *pAttribute)
{
	return m_buteMgr.Exist(s_aFolderTag[folderId], pAttribute);
}

LTIntPt  CLayoutMgr::GetFolderCustomPoint(eFolderID folderId, char *pAttribute)
{
	return GetPoint(s_aFolderTag[folderId], pAttribute);
}

LTRect   CLayoutMgr::GetFolderCustomRect(eFolderID folderId, char *pAttribute)
{
	return GetRect(s_aFolderTag[folderId], pAttribute);
}

int		CLayoutMgr::GetFolderCustomInt(eFolderID folderId, char *pAttribute)
{
	return GetInt(s_aFolderTag[folderId], pAttribute);
}

LTFLOAT CLayoutMgr::GetFolderCustomFloat(eFolderID folderId, char *pAttribute)
{
	return GetFloat(s_aFolderTag[folderId], pAttribute);
}

void	CLayoutMgr::GetFolderCustomString(eFolderID folderId, char *pAttribute, char *pBuf, int nBufLen)
{
	GetString(s_aFolderTag[folderId], pAttribute, pBuf, nBufLen);
}

LTVector CLayoutMgr::GetFolderCustomVector(eFolderID folderId, char *pAttribute)
{
	return GetVector(s_aFolderTag[folderId], pAttribute);
}




// ------------------------------------------------------------------------//
//
//	Miscellaneous Layout
//
// ------------------------------------------------------------------------//

LTFLOAT  CLayoutMgr::GetFlashSpeed()
{
    return (LTFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_FLASHSPEED, 0.0f);
}

LTFLOAT  CLayoutMgr::GetFlashDuration()
{
    return (LTFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_FLASHDUR, 0.0f);
}

LTVector CLayoutMgr::GetNightVisionModelColor()
{
	CAVector vRet(0.0,0.0,0.0);

	return m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_NV_MODEL, vRet);
}

LTVector CLayoutMgr::GetNightVisionScreenTint()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vRet = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_NV_SCREEN, vDef);
	return (vRet * MATH_ONE_OVER_255);
}

LTVector CLayoutMgr::GetInfraredModelColor()
{
	CAVector vRet(0.0,0.0,0.0);

	return m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_IR_MODEL, vRet);
}

LTVector CLayoutMgr::GetInfraredLightScale()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vRet = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_IR_LIGHT, vDef);
	return (vRet * MATH_ONE_OVER_255);
}


LTVector CLayoutMgr::GetMineDetectScreenTint()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vRet = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_MD_SCREEN, vDef);
	return (vRet * MATH_ONE_OVER_255);
}


LTVector CLayoutMgr::GetChooserHighlightColor()
{
	CAVector vRet(0.0,0.0,0.0);

	return m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_CHOOSER_HCOLOR, vRet);
}

LTBOOL CLayoutMgr::IsMaskSprite(eOverlayMask eMask)
{
	LTBOOL bFound = LTFALSE;
	
/*
	switch (eMask)
	{
	case OVM_SUNGLASS:
		bFound = m_buteMgr.Exist(LO_MASK_TAG, LO_MASK_SUNSPRITE);
		break;
	case OVM_SCOPE:
		bFound = m_buteMgr.Exist(LO_MASK_TAG, LO_MASK_SCOPESPRITE);
		break;
	case OVM_SCUBA:
		bFound = m_buteMgr.Exist(LO_MASK_TAG, LO_MASK_SCUBASPRITE);
		break;
	case OVM_SPACE:
		bFound = m_buteMgr.Exist(LO_MASK_TAG, LO_MASK_SPACESPRITE);
		break;
	case OVM_STATIC:
		bFound = m_buteMgr.Exist(LO_MASK_TAG, LO_MASK_STATICSPRITE);
		break;
	case OVM_CAMERA:
		bFound = m_buteMgr.Exist(LO_MASK_TAG, LO_MASK_CAMERASPRITE);
		break;
	case OVM_ZOOM_IN:
		bFound = m_buteMgr.Exist(LO_MASK_TAG, LO_MASK_ZOOMIN_SPRITE);
		break;
	case OVM_ZOOM_OUT:
		bFound = m_buteMgr.Exist(LO_MASK_TAG, LO_MASK_ZOOMOUT_SPRITE);
		break;
	}
*/
	return bFound;
}


void CLayoutMgr::GetMaskSprite(eOverlayMask eMask, char *pBuf, int nBufLen)
{
/*
	switch (eMask)
	{
	case OVM_SUNGLASS:
		GetString(LO_MASK_TAG, LO_MASK_SUNSPRITE, pBuf, nBufLen);
		break;
	case OVM_SCOPE:
		GetString(LO_MASK_TAG, LO_MASK_SCOPESPRITE, pBuf, nBufLen);
		break;
	case OVM_SCUBA:
		GetString(LO_MASK_TAG, LO_MASK_SCUBASPRITE, pBuf, nBufLen);
		break;
	case OVM_SPACE:
		GetString(LO_MASK_TAG, LO_MASK_SPACESPRITE, pBuf, nBufLen);
		break;
	case OVM_STATIC:
		GetString(LO_MASK_TAG, LO_MASK_STATICSPRITE, pBuf, nBufLen);
		break;
	case OVM_CAMERA:
		GetString(LO_MASK_TAG, LO_MASK_CAMERASPRITE, pBuf, nBufLen);
		break;
	case OVM_ZOOM_IN:
		GetString(LO_MASK_TAG, LO_MASK_ZOOMIN_SPRITE, pBuf, nBufLen);
		break;
	case OVM_ZOOM_OUT:
		GetString(LO_MASK_TAG, LO_MASK_ZOOMOUT_SPRITE, pBuf, nBufLen);
		break;
	default:
        pBuf[0] = LTNULL;
		break;
	}
*/
}

void CLayoutMgr::GetMaskModel(eOverlayMask eMask, char *pBuf, int nBufLen)
{
/*
	switch (eMask)
	{
	case OVM_SUNGLASS:
		GetString(LO_MASK_TAG, LO_MASK_SUNMODEL, pBuf, nBufLen);
		break;
	case OVM_SCOPE:
		GetString(LO_MASK_TAG, LO_MASK_SCOPEMODEL, pBuf, nBufLen);
		break;
	case OVM_SCUBA:
		GetString(LO_MASK_TAG, LO_MASK_SCUBAMODEL, pBuf, nBufLen);
		break;
	case OVM_SPACE:
		GetString(LO_MASK_TAG, LO_MASK_SPACEMODEL, pBuf, nBufLen);
		break;
	case OVM_STATIC:
		GetString(LO_MASK_TAG, LO_MASK_STATICMODEL, pBuf, nBufLen);
		break;
	case OVM_CAMERA:
		GetString(LO_MASK_TAG, LO_MASK_CAMERAMODEL, pBuf, nBufLen);
		break;
	case OVM_ZOOM_IN:
		GetString(LO_MASK_TAG, LO_MASK_ZOOMIN_MODEL, pBuf, nBufLen);
		break;
	case OVM_ZOOM_OUT:
		GetString(LO_MASK_TAG, LO_MASK_ZOOMOUT_MODEL, pBuf, nBufLen);
		break;
	default:
        pBuf[0] = LTNULL;
		break;
	}
*/
}


void CLayoutMgr::GetMaskSkin(eOverlayMask eMask, char *pBuf, int nBufLen)
{
/*
	switch (eMask)
	{
	case OVM_SUNGLASS:
		GetString(LO_MASK_TAG, LO_MASK_SUNSKIN, pBuf, nBufLen);
		break;
	case OVM_SCOPE:
		GetString(LO_MASK_TAG, LO_MASK_SCOPESKIN, pBuf, nBufLen);
		break;
	case OVM_SCUBA:
		GetString(LO_MASK_TAG, LO_MASK_SCUBASKIN, pBuf, nBufLen);
		break;
	case OVM_SPACE:
		GetString(LO_MASK_TAG, LO_MASK_SPACESKIN, pBuf, nBufLen);
		break;
	case OVM_STATIC:
		GetString(LO_MASK_TAG, LO_MASK_STATICSKIN, pBuf, nBufLen);
		break;
	case OVM_CAMERA:
		GetString(LO_MASK_TAG, LO_MASK_CAMERASKIN, pBuf, nBufLen);
		break;
	case OVM_ZOOM_IN:
		GetString(LO_MASK_TAG, LO_MASK_ZOOMIN_SKIN, pBuf, nBufLen);
		break;
	case OVM_ZOOM_OUT:
		GetString(LO_MASK_TAG, LO_MASK_ZOOMOUT_SKIN, pBuf, nBufLen);
		break;
	default:
        pBuf[0] = LTNULL;
		break;
	}
*/
}


LTFLOAT CLayoutMgr::GetMaskScale(eOverlayMask eMask)
{
    LTFLOAT fRet = 1.0f;
/*
	switch (eMask)
	{
	case OVM_SUNGLASS:
        fRet = (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, LO_MASK_SUNSCALE, 0.0f);
		break;
	case OVM_SCOPE:
        fRet = (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, LO_MASK_SCOPESCALE, 0.0f);
		break;
	case OVM_SCUBA:
        fRet = (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, LO_MASK_SCUBASCALE, 0.0f);
		break;
	case OVM_SPACE:
        fRet = (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, LO_MASK_SPACESCALE, 0.0f);
		break;
	case OVM_STATIC:
        fRet = (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, LO_MASK_STATICSCALE, 0.0f);
		break;
	case OVM_CAMERA:
        fRet = (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, LO_MASK_CAMERASCALE, 0.0f);
		break;
	case OVM_ZOOM_IN:
        fRet = (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, LO_MASK_ZOOMIN_SCALE, 0.0f);
		break;
	case OVM_ZOOM_OUT:
        fRet = (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, LO_MASK_ZOOMOUT_SCALE, 0.0f);
		break;
	}
*/
	return fRet;
}


LTFLOAT CLayoutMgr::GetMessageBoxAlpha()
{
	return GetFloat(LO_MISC_TAG, LO_MISC_MB_ALPHA);
}

void CLayoutMgr::GetHelpFont(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_HELPFONT, pBuf, nBufLen);
}

void CLayoutMgr::GetTitleFont(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_TITLEFONT, pBuf, nBufLen);
}



LTFLOAT CLayoutMgr::GetMessageFade()
{
	return GetFloat(LO_MISC_TAG, LO_MISC_MSG_FADE);
}

LTFLOAT CLayoutMgr::GetMessageTime()
{
	return GetFloat(LO_MISC_TAG, LO_MISC_MSG_TIME);
}

int CLayoutMgr::GetMaxNumMessages()
{
	return GetInt(LO_MISC_TAG, LO_MISC_MSG_NUM);
}


LTRect   CLayoutMgr::GetObjectiveRect()
{
	return GetRect(LO_MISC_TAG, LO_MISC_OBJ_RECT);
}

LTRect   CLayoutMgr::GetPopupTextRect()
{
	return GetRect(LO_MISC_TAG, LO_MISC_POPUP_RECT);
}

HLTCOLOR CLayoutMgr::GetSubtitleTint()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vColor = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_SUB_TINT, vDef);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

HLTCOLOR CLayoutMgr::GetHealthTint()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vColor = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_HEALTH_TINT, vDef);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

HLTCOLOR CLayoutMgr::GetArmorTint()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vColor = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_ARMOR_TINT, vDef);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

HLTCOLOR CLayoutMgr::GetAmmoTint()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vColor = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_AMMO_TINT, vDef);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

HLTCOLOR CLayoutMgr::GetPopupTextTint()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vColor = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_POPUP_TINT, vDef);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}


int CLayoutMgr::GetSubtitleNumLines()
{
	return GetInt(LO_SUBTITLE_TAG, LO_SUBTITLE_NUM_LINES);
}

int CLayoutMgr::GetSubtitle1stPersonWidth()
{
	return GetInt(LO_SUBTITLE_TAG, LO_SUBTITLE_1ST_WIDTH);
}

LTIntPt CLayoutMgr::GetSubtitle1stPersonPos()
{
	return GetPoint(LO_SUBTITLE_TAG, LO_SUBTITLE_1ST_POS);
}

int CLayoutMgr::GetSubtitle3rdPersonWidth()
{
	return GetInt(LO_SUBTITLE_TAG, LO_SUBTITLE_3RD_WIDTH);
}

LTIntPt CLayoutMgr::GetSubtitle3rdPersonPos()
{
	return GetPoint(LO_SUBTITLE_TAG, LO_SUBTITLE_3RD_POS);
}


int CLayoutMgr::GetDialoguePosition()
{
	return GetInt(LO_DIALOGUE_TAG, LO_WINDOW_POS);
}

LTIntPt CLayoutMgr::GetDialogueSize()
{
	return GetPoint(LO_DIALOGUE_TAG, LO_WINDOW_SIZE);
}
LTIntPt CLayoutMgr::GetDialogueTextOffset()
{
	return GetPoint(LO_DIALOGUE_TAG, LO_WINDOW_TEXT_OFFSET);
}

void CLayoutMgr::GetDialogueBackground(char *pBuf, int nBufLen)
{
	GetString(LO_DIALOGUE_TAG, LO_WINDOW_BACK, pBuf, nBufLen);
}

LTFLOAT CLayoutMgr::GetDialogueAlpha()
{
	return GetFloat(LO_DIALOGUE_TAG, LO_WINDOW_ALPHA);
}

void CLayoutMgr::GetDialogueFrame(char *pBuf, int nBufLen)
{
	GetString(LO_DIALOGUE_TAG, LO_WINDOW_FRAME, pBuf, nBufLen);
}

int CLayoutMgr::GetDialogueFont()
{
	return GetInt(LO_DIALOGUE_TAG, LO_WINDOW_FONT);
}

LTFLOAT CLayoutMgr::GetDialogueOpenTime()
{
	return GetFloat(LO_DIALOGUE_TAG, LO_WINDOW_OPEN);
}

LTFLOAT CLayoutMgr::GetDialogueCloseTime()
{
	return GetFloat(LO_DIALOGUE_TAG, LO_WINDOW_CLOSE);
}

void CLayoutMgr::GetDialogueOpenSound(char *pBuf, int nBufLen)
{
	GetString(LO_DIALOGUE_TAG, LO_WINDOW_SND_OPEN, pBuf, nBufLen);
}

void CLayoutMgr::GetDialogueCloseSound(char *pBuf, int nBufLen)
{
	GetString(LO_DIALOGUE_TAG, LO_WINDOW_SND_CLOSE, pBuf, nBufLen);
}



int CLayoutMgr::GetDecisionPosition()
{
	return GetInt(LO_DECISION_TAG, LO_WINDOW_POS);
}


LTIntPt CLayoutMgr::GetDecisionTextOffset()
{
	return GetPoint(LO_DECISION_TAG, LO_WINDOW_TEXT_OFFSET);
}

int CLayoutMgr::GetDecisionSpacing()
{
	return GetInt(LO_DECISION_TAG, LO_WINDOW_SPACING);
}

void CLayoutMgr::GetDecisionBackground(char *pBuf, int nBufLen)
{
	GetString(LO_DECISION_TAG, LO_WINDOW_BACK, pBuf, nBufLen);
}

LTFLOAT CLayoutMgr::GetDecisionAlpha()
{
	return GetFloat(LO_DECISION_TAG, LO_WINDOW_ALPHA);
}


int CLayoutMgr::GetDecisionFont()
{
	return GetInt(LO_DECISION_TAG, LO_WINDOW_FONT);
}


LTFLOAT CLayoutMgr::GetDecisionOpenTime()
{
	return GetFloat(LO_DECISION_TAG, LO_WINDOW_OPEN);
}

LTFLOAT CLayoutMgr::GetDecisionCloseTime()
{
	return GetFloat(LO_DECISION_TAG, LO_WINDOW_CLOSE);
}



int CLayoutMgr::GetMenuPosition()
{
	return GetInt(LO_MENU_TAG, LO_WINDOW_POS);
}


LTIntPt CLayoutMgr::GetMenuTextOffset()
{
	return GetPoint(LO_MENU_TAG, LO_WINDOW_TEXT_OFFSET);
}

int CLayoutMgr::GetMenuSpacing()
{
	return GetInt(LO_MENU_TAG, LO_WINDOW_SPACING);
}

void CLayoutMgr::GetMenuBackground(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_WINDOW_BACK, pBuf, nBufLen);
}

void CLayoutMgr::GetMenuFrame(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_WINDOW_FRAME, pBuf, nBufLen);
}

LTFLOAT CLayoutMgr::GetMenuAlpha()
{
	return GetFloat(LO_MENU_TAG, LO_WINDOW_ALPHA);
}


LTFLOAT CLayoutMgr::GetMenuOpenTime()
{
	return GetFloat(LO_MENU_TAG, LO_WINDOW_OPEN);
}

LTFLOAT CLayoutMgr::GetMenuCloseTime()
{
	return GetFloat(LO_MENU_TAG, LO_WINDOW_CLOSE);
}

void CLayoutMgr::GetMenuOpenSound(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_WINDOW_SND_OPEN, pBuf, nBufLen);
}

void CLayoutMgr::GetMenuCloseSound(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_WINDOW_SND_CLOSE, pBuf, nBufLen);
}

//-------------------------------------------------------------------------
LTFLOAT CLayoutMgr::GetCreditsFadeInTime()
{
	return GetFloat(LO_CREDITS_TAG, LO_CREDITS_FADEIN);
}

LTFLOAT CLayoutMgr::GetCreditsHoldTime()
{
	return GetFloat(LO_CREDITS_TAG, LO_CREDITS_HOLD);
}

LTFLOAT CLayoutMgr::GetCreditsFadeOutTime()
{
	return GetFloat(LO_CREDITS_TAG, LO_CREDITS_FADEOUT);
}

LTFLOAT CLayoutMgr::GetCreditsDelayTime()
{
	return GetFloat(LO_CREDITS_TAG, LO_CREDITS_DELAY);
}

LTIntPt CLayoutMgr::GetCreditsPositionUL()
{
	return GetPoint(LO_CREDITS_TAG, LO_CREDITS_POS_UL);
}

LTIntPt CLayoutMgr::GetCreditsPositionUR()
{
	return GetPoint(LO_CREDITS_TAG, LO_CREDITS_POS_UR);
}

LTIntPt CLayoutMgr::GetCreditsPositionLL()
{
	return GetPoint(LO_CREDITS_TAG, LO_CREDITS_POS_LL);
}

LTIntPt CLayoutMgr::GetCreditsPositionLR()
{
	return GetPoint(LO_CREDITS_TAG, LO_CREDITS_POS_LR);
}


// ------------------------------------------------------------------------//
//
//	Private Helper functions
//
// ------------------------------------------------------------------------//

LTBOOL CLayoutMgr::GetBool(char *pTag,char *pAttribute)
{
    return (LTBOOL) m_buteMgr.GetInt(pTag,pAttribute, 0);
}

LTFLOAT CLayoutMgr::GetFloat(char *pTag,char *pAttribute)
{
    return (LTFLOAT)m_buteMgr.GetDouble(pTag, pAttribute, 0.0f);
}

int	CLayoutMgr::GetInt(char *pTag,char *pAttribute)
{
	return m_buteMgr.GetInt(pTag, pAttribute, 0);
}

LTIntPt CLayoutMgr::GetPoint(char *pTag,char *pAttribute)
{
	CPoint tmp = m_buteMgr.GetPoint(pTag, pAttribute, CPoint(0,0));
    LTIntPt pt(tmp.x,tmp.y);
	return pt;
}

LTRect CLayoutMgr::GetRect(char *pTag,char *pAttribute)
{
	CRect tmp = m_buteMgr.GetRect(pTag, pAttribute, CRect(0,0,0,0) );
    LTRect rect(tmp.left,tmp.top,tmp.right,tmp.bottom);
	return rect;

}

void CLayoutMgr::GetString(char *pTag,char *pAttribute,char *pBuf, int nBufLen)
{
	CString str = "";
	str = m_buteMgr.GetString(pTag, pAttribute);
	strncpy(pBuf, (char*)(LPCSTR)str, nBufLen);
}

LTVector CLayoutMgr::GetVector(char *pTag,char *pAttribute)
{
	CAVector vRet(0.0,0.0,0.0);
	return m_buteMgr.GetVector(pTag, pAttribute, vRet);
}

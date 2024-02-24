// LayoutMgr.h: interface for the CLayoutMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LAYOUTMGR_H__D238C1C0_8635_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_LAYOUTMGR_H__D238C1C0_8635_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "FolderMgr.h"
#include "Overlays.h"
#include "HudMgr.h"


const char* const	LO_DEFAULT_FILE = "Attributes\\Layout.txt";
const DDWORD		HUD_ERROR		= 255;
const char* const	HUD_BASIC_TAG	= "HUDConfig";
const int			MAX_TABS		= 6;


class CLayoutMgr;
extern CLayoutMgr* g_pLayoutMgr;

struct INT_CHAR
{
	INT_CHAR();

	LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);

	char	szName[128];

	char	szModel[128];
	char	szStyle[128];

	LTVector vPos;
	LTFLOAT	 fScale;
	LTFLOAT	 fRot;

};

class CLayoutMgr : public CGameButeMgr  
{
public:
	CLayoutMgr();
	virtual ~CLayoutMgr();

	LTBOOL		Init(ILTCSBase *pInterface, const char* szAttributeFile=LO_DEFAULT_FILE);
	void		Term();

	LTBOOL		WriteFile() { return m_buteMgr.Save(); }
	void		Reload()    { m_buteMgr.Parse(m_strAttributeFile); }

	int			GetNumHUDLayouts(DDWORD nType);
	DVector		GetWeaponPickupColor();
	DVector		GetAmmoPickupColor();
	DFLOAT		GetTintTime();
	DFLOAT		GetPerturbRotationEffect();
	DFLOAT		GetPerturbIncreaseSpeed();
	DFLOAT		GetPerturbDecreaseSpeed();
	DFLOAT		GetPerturbWalkPercent();
	void		GetScopeMaskSprite(char *pBuf, int nBufLen);
	DFLOAT		GetScopeMaskScale();
	void		GetMessageBoxBackground(char *pBuf, int nBufLen);
	void		GetFailScreenBackground(char *pBuf, int nBufLen);
	DIntPt		GetFailStringPos();
	DFLOAT		GetFailScreenDelay();
	DFLOAT		GetDeathDelay();
	void		GetSmallFontBase(char *pBuf, int nBufLen);
	void		GetLargeFontBase(char *pBuf, int nBufLen);
	void		GetMessageFont(char *pBuf, int nBufLen);
	void		GetObjectiveFont(char *pBuf, int nBufLen);

	void		GetAlienFontBase(char *pBuf, int nBufLen);
	void		GetPredatorFontBase(char *pBuf, int nBufLen);
	void		GetMarineFontBase(char *pBuf, int nBufLen);

	void		GetPDAFontBase(char *pBuf, int nBufLen);
	void		GetMEMOFontBase(char *pBuf, int nBufLen);

	//new functions for AvP3
	DDWORD		GetNumHudElements(char *pTag);
	DDWORD		GetNumHudFonts(char *pTag);
	LTBOOL		GetHasMotionDetector(char *pTag);
	LTBOOL		GetElementEnabled(char *pTag, DDWORD nIndex);
	DIntPt		GetElementOffset(char *pTag, DDWORD nIndex);
	DDWORD		GetElementOffsetType(char *pTag, DDWORD nIndex);
	void		GetSurfaceName(char *pTag, DDWORD nIndex, char *pBuf, int nBufLen);
	void		GetFontName(char *pTag, DDWORD nIndex, char *pBuf, int nBufLen);
	DDWORD		GetDrawIndex(char *pTag, DDWORD nIndex);
	LTBOOL		GetElementTransparent(char *pTag, DDWORD nIndex);
	LTBOOL		GetElementTranslucent(char *pTag, DDWORD nIndex);
	DFLOAT		GetElementTranslucency(char *pTag, DDWORD nIndex);
	DDWORD		GetFontIndex(char *pTag, DDWORD nIndex);
	DDWORD		GetUpdateIndex(char *pTag, DDWORD nIndex);
	DDWORD		GetNumAnim(char *pTag, DDWORD nIndex);
	void		GetAnimName(char *pTag, DDWORD nIndex, char *pBuf, int nBufLen);
	DFLOAT		GetAmimTranslucency(char *pTag, DDWORD nIndex);
	DDWORD		GetFontPitch(char *pTag, DDWORD nIndex);
	void		GetFrameBackground(char *pTag, char *pBuf, int nBufLen);
	LTBOOL		GetFrameTransparent(char *pTag);
	LTBOOL		GetFrameTranslucent(char *pTag);
	DFLOAT		GetFrameTranslucency(char *pTag);
	LTBOOL		GetFrameHasBackground(char *pTag);
	DDWORD		GetFrameWidth(char *pTag);
	DDWORD		GetFrameHeight(char *pTag);
	DIntPt		GetFrameEnabledPos(char *pTag);
	DIntPt		GetFrameDisabledPos(char *pTag);
	LTBOOL		GetFrameHasTitle(char *pTag);
	void		GetFrameTitle(char *pTag, char *pBuf, int nBufLen);
	DDWORD		GetNumFrames(char *pTag);
	DDWORD		GetFrameIndex(char *pTag, DDWORD nIndex);
	LTBOOL		GetPageHasBackground(char *pTag);
	void		GetPageBackground(char *pTag, char *pBuf, int nBufLen);
	LTBOOL		GetPageHasTitle(char *pTag);
	void		GetPageTitle(char *pTag, char *pBuf, int nBufLen);
	DIntPt		GetPageTitleOffset(char *pTag);
	DDWORD		GetNumMenuFonts();
	void		GetMenuFontFile(char *pBuf, int nBufLen, DDWORD nIndex);
	DDWORD		GetMenuFontType(DDWORD nIndex);
	DDWORD		GetFrameNumControls(char* aTag);
	DDWORD		GetFrameControlIndex(char* aTag, DDWORD nIndex);
	DDWORD		GetControlType(DDWORD nIndex);
	void		GetControlString(char *pBuf, int nBufLen, DDWORD nIndex);
	DDWORD		GetControlCmd(DDWORD nIndex);
	DDWORD		GetControlHlp(DDWORD nIndex);
	DDWORD		GetFontIndex(DDWORD nIndex);
	DIntPt		GetPageTitleOffset(DDWORD nIndex);
	DDWORD		GetParentPage(char* aTag);
	void		GetBitmapControl(char *pBuf, int nBufLen, DDWORD nIndex);
	DIntPt		GetTextLabelOffset(DDWORD nIndex);
	DDWORD		GetScrollHeight(DDWORD nIndex);
	DDWORD		GetScrollWidth(DDWORD nIndex);
	void		GetScrollUpArrowPressed(char *pBuf, int nBufLen, DDWORD nIndex);
	void		GetScrollDownArrowPressed(char *pBuf, int nBufLen, DDWORD nIndex);
	void		GetScrollDownArrow(char *pBuf, int nBufLen, DDWORD nIndex);
	void		GetScrollUpArrow(char *pBuf, int nBufLen, DDWORD nIndex);
	void		GetScrollBar(char *pBuf, int nBufLen, DDWORD nIndex);
	void		GetScrollTab(char *pBuf, int nBufLen, DDWORD nIndex);
	DDWORD		GetDefaultCfg(DDWORD nIndex);
	DDWORD		GetHUDCfg(DDWORD nType, DDWORD nIndex);
	void		GetMDLargeFontName(char *pBuf, int nBufLen);
	void		GetMDSmallFontName(char *pBuf, int nBufLen);
	DDWORD		GetNumMDBlipImages();
	DDWORD		GetNumMDGridImages();
	DIntPt		GetMDGridOffset();
	DIntPt		GetMDBaseOffset();
	DFLOAT		GetMDGridTranslucency();
	DFLOAT		GetMDBaseTranslucency();
	DFLOAT		GetMDArcTranslucency();
	void		GetArcSurfaceName(char *pBuf, int nBufLen);
	void		GetBaseSurfaceName(char *pBuf, int nBufLen);
	void		GetGridSurfaceName(char *pBuf, int nBufLen);
	void		GetBlipSurfaceName(char *pBuf, int nBufLen);
	void		GetTrackerArrowSurfaceName(char *pBuf, int nBufLen);
	void		GetTrackerBlipSurfaceName(char *pBuf, int nBufLen);

	// NOLF Stuff
    LTIntPt     GetBackPos();
    LTIntPt     GetContinuePos();
    LTIntPt     GetMainPos();
	void		GetBackSprite(char *pBuf, int nBufLen);
    LTFLOAT     GetBackSpriteScale();
	void		GetArrowBackSFX(char *pBuf, int nBufLen);
	void		GetArrowNextSFX(char *pBuf, int nBufLen);
	void		GetArrowBackBmp(char *pBuf, int nBufLen);
	void		GetArrowNextBmp(char *pBuf, int nBufLen);
    LTIntPt     GetArrowBackPos();
    LTIntPt     GetArrowNextPos();
	void		GetChangeSound(char *pBuf, int nBufLen);
	void		GetSelectSound(char *pBuf, int nBufLen);
	void		GetEscapeSound(char *pBuf, int nBufLen);
	void		GetUnselectableSound(char *pBuf, int nBufLen);
	void		GetSliderLeftSound(char *pBuf, int nBufLen);
	void		GetSliderRightSound(char *pBuf, int nBufLen);


    LTRect      GetFolderHelpRect(eFolderID folderId);
    LTIntPt     GetFolderTitlePos(eFolderID folderId);
	int			GetFolderTitleAlign(eFolderID folderId);
    LTRect      GetFolderPageRect(eFolderID folderId);
	int			GetFolderItemSpacing(eFolderID folderId);
	int			GetFolderItemAlign(eFolderID folderId);
	int			GetFolderMusicIntensity(eFolderID folderId);
    LTIntPt     GetUpArrowPos(eFolderID folderId);
    LTIntPt     GetDownArrowPos(eFolderID folderId);
    int			GetFolderFontSize(eFolderID folderId);

	INT_CHAR   *GetFolderCharacter(eFolderID folderId);
	int			GetFolderNumAttachments(eFolderID folderId);
	void		GetFolderAttachment(eFolderID folderId, int num, char *pBuf, int nBufLen);
	HLTCOLOR	GetFolderNormalColor(eFolderID folderId);
	HLTCOLOR	GetFolderSelectedColor(eFolderID folderId);
	HLTCOLOR	GetFolderDisabledColor(eFolderID folderId);
	HLTCOLOR	GetFolderHighlightColor(eFolderID folderId);

	void		GetFolderAmbient(eFolderID folderId, char *pBuf, int nBufLen);


    LTBOOL      HasCustomValue(eFolderID folderId, char *pAttribute);
    LTIntPt     GetFolderCustomPoint(eFolderID folderId, char *pAttribute);
    LTRect      GetFolderCustomRect(eFolderID folderId, char *pAttribute);
	int			GetFolderCustomInt(eFolderID folderId, char *pAttribute);
    LTFLOAT     GetFolderCustomFloat(eFolderID folderId, char *pAttribute);
	void		GetFolderCustomString(eFolderID folderId, char *pAttribute, char *pBuf, int nBufLen);
    LTVector    GetFolderCustomVector(eFolderID folderId, char *pAttribute);


	int			GetNumTabs()			{return m_nNumTabs;}
	LTIntPt     GetTabPos(eFolderID folderId, int nTab);
	int			GetNumLinks()			{return m_nNumLinks;}
	LTIntPt     GetLinkPos(eFolderID folderId, int nLink);

    LTFLOAT     GetFlashSpeed();
    LTFLOAT     GetFlashDuration();
    LTVector    GetNightVisionModelColor();
    LTVector    GetNightVisionScreenTint();
    LTVector    GetInfraredModelColor();
    LTVector    GetInfraredLightScale();
    LTVector    GetMineDetectScreenTint();
    LTVector    GetChooserHighlightColor();

	LTBOOL		IsMaskSprite(eOverlayMask eMask);
	void		GetMaskSprite(eOverlayMask eMask, char *pBuf, int nBufLen);
	void		GetMaskModel(eOverlayMask eMask, char *pBuf, int nBufLen);
	void		GetMaskSkin(eOverlayMask eMask, char *pBuf, int nBufLen);
    LTFLOAT     GetMaskScale(eOverlayMask eMask);

    LTFLOAT     GetMessageBoxAlpha();

	void		GetHelpFont(char *pBuf, int nBufLen);
	void		GetTinyFontBase(char *pBuf, int nBufLen);
	void		GetTitleFont(char *pBuf, int nBufLen);

    LTFLOAT     GetMessageFade();
    LTFLOAT     GetMessageTime();
	int			GetMaxNumMessages();

    LTRect      GetObjectiveRect();
    LTRect      GetPopupTextRect();

	HLTCOLOR	GetSubtitleTint();
	HLTCOLOR	GetHealthTint();
	HLTCOLOR	GetArmorTint();
	HLTCOLOR	GetAmmoTint();
	HLTCOLOR	GetPopupTextTint();

	int			GetSubtitleNumLines();
	int			GetSubtitle3rdPersonWidth();
    LTIntPt     GetSubtitle3rdPersonPos();
	int			GetSubtitle1stPersonWidth();
    LTIntPt     GetSubtitle1stPersonPos();

	int			GetDialoguePosition();
    LTIntPt     GetDialogueSize();
    LTIntPt     GetDialogueTextOffset();
	void		GetDialogueBackground(char *pBuf, int nBufLen);
    LTFLOAT     GetDialogueAlpha();
	void		GetDialogueFrame(char *pBuf, int nBufLen);
	int			GetDialogueFont();
    LTFLOAT     GetDialogueOpenTime();
    LTFLOAT     GetDialogueCloseTime();
	void		GetDialogueOpenSound(char *pBuf, int nBufLen);
	void		GetDialogueCloseSound(char *pBuf, int nBufLen);

	int			GetDecisionPosition();
    LTIntPt     GetDecisionTextOffset();
	int			GetDecisionSpacing();
	void		GetDecisionBackground(char *pBuf, int nBufLen);
    LTFLOAT     GetDecisionAlpha();
	int			GetDecisionFont();
    LTFLOAT     GetDecisionOpenTime();
    LTFLOAT     GetDecisionCloseTime();

	int			GetMenuPosition();
    LTIntPt     GetMenuTextOffset();
	int			GetMenuSpacing();
	void		GetMenuBackground(char *pBuf, int nBufLen);
	void		GetMenuFrame(char *pBuf, int nBufLen);
    LTFLOAT     GetMenuAlpha();
    LTFLOAT     GetMenuOpenTime();
    LTFLOAT     GetMenuCloseTime();
	void		GetMenuOpenSound(char *pBuf, int nBufLen);
	void		GetMenuCloseSound(char *pBuf, int nBufLen);

	LTFLOAT		GetCreditsFadeInTime();
	LTFLOAT		GetCreditsHoldTime();
	LTFLOAT		GetCreditsFadeOutTime();
	LTFLOAT		GetCreditsDelayTime();
	LTIntPt		GetCreditsPositionUL();
	LTIntPt		GetCreditsPositionUR();
	LTIntPt		GetCreditsPositionLL();
	LTIntPt		GetCreditsPositionLR();

protected:
    LTBOOL      GetBool(char *pTag,char *pAttribute);
    LTFLOAT     GetFloat(char *pTag,char *pAttribute);
	int			GetInt(char *pTag,char *pAttribute);
    LTIntPt     GetPoint(char *pTag,char *pAttribute);
    LTRect      GetRect(char *pTag,char *pAttribute);
	void		GetString(char *pTag,char *pAttribute, char *pBuf, int nBufLen);
    LTVector    GetVector(char *pTag,char *pAttribute);


protected:
	CMoArray<INT_CHAR *> m_CharacterArray;
	int			m_nNumTabs;
	int			m_nNumLinks;
	LTIntPt		m_TabPos[MAX_TABS];
	LTIntPt		m_LinkPos[MAX_TABS];

protected:
	LTBOOL		GetBool(const char *pTag,const char *pAttribute);
	DFLOAT		GetFloat(const char *pTag,const char *pAttribute);
	int			GetInt(const char *pTag,const char *pAttribute);
	DIntPt		GetPoint(const char *pTag,const char *pAttribute);
	DRect		GetRect(const char *pTag,const char *pAttribute);
	void		GetString(const char *pTag,const char *pAttribute, char *pBuf, int nBufLen);
	DVector		GetVector(const char *pTag,const char *pAttribute);

};

#endif // !defined(AFX_LAYOUTMGR_H__D238C1C0_8635_11D3_B2DB_006097097C7B__INCLUDED_)

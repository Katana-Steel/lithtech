// InterfaceResMgr.h: interface for the CInterfaceResMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTERFACERESMGR_H__44498A80_ECE2_11D2_B2DB_006097097C7B__INCLUDED_)
#define AFX_INTERFACERESMGR_H__44498A80_ECE2_11D2_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LTGUIMgr.h"
#include "InterfaceSurfMgr.h"

#include <string>

class CGameClientShell;
class CInterfaceResMgr;
extern CInterfaceResMgr* g_pInterfaceResMgr;


class CInterfaceResMgr  
{
public:
	CInterfaceResMgr();
	virtual ~CInterfaceResMgr();

	LTBOOL				Init (ILTClient* pClientDE, CGameClientShell* pClientShell);
	void				Term();
	HSURFACE			GetSharedSurface(char *lpszPath)	{ return m_InterfaceSurfMgr.GetSurface(lpszPath); }
	void				FreeSharedSurface(char *lpszPath)	{ m_InterfaceSurfMgr.FreeSurface(lpszPath); }
	void				FreeSharedSurface(HSURFACE hSurf)	{ m_InterfaceSurfMgr.FreeSurface(hSurf); }
	
	CLTGUIFont			*GetTitleFont()						{return m_pTitleFont;}
	CLTGUIFont			*GetLargeFont()						{return m_pLargeFont;}
	CLTGUIFont			*GetSmallFont()						{return m_pSmallFont;}
	CLTGUIFont			*GetTinyFont()						{return m_pTinyFont;}
	CLTGUIFont			*GetLargeFixedFont()				{return m_pLargeFixedFont;}
	CLTGUIFont			*GetSmallFixedFont()				{return m_pSmallFixedFont;}
	CLTGUIFont			*GetTinyFixedFont()					{return m_pTinyFixedFont;}
	CLTGUIFont			*GetHelpFont()						{return m_pHelpFont;}
	CLTGUIFont			*GetMessageFont()					{return m_pMessageFont;}
	CLTGUIFont			*GetObjectiveFont()					{return m_pObjectiveFont;}

	CLTGUIFont			*GetAlienFont()						{return m_pAlienFont;}
	CLTGUIFont			*GetMarineFont()					{return m_pMarineFont;}
	CLTGUIFont			*GetPredatorFont()					{return m_pPredatorFont;}

	CLTGUIFont			*GetPDAFont()					{return m_pPDAFont;}
	CLTGUIFont			*GetMEMOFont()					{return m_pMEMOFont;}

	// Creates a surface just large enough for the string.
	// You can make the surface a little larger with extraPixelsX and extraPixelsY.
	// if a Width > 0 is specified, the text is wrapped to fit that width
	HSURFACE			CreateSurfaceFromString(CLTGUIFont *pFont, HSTRING hString, HDECOLOR hBackColor, 
												int extraPixelsX = 0, int extraPixelsY = 0, int nWidth = 0);
	HSURFACE			CreateSurfaceFromString(CLTGUIFont *pFont, int	nStringId, HDECOLOR hBackColor, 
												int extraPixelsX = 0, int extraPixelsY = 0, int nWidth = 0);
	HSURFACE			CreateSurfaceFromString(CLTGUIFont *pFont, char *lpszString, HDECOLOR hBackColor, 
												int extraPixelsX = 0, int extraPixelsY = 0, int nWidth = 0);


	const char			*GetSoundSelect();
	const char			*GetSoundChoose();
	const char			*GetSoundFolderChange();
	const char			*GetSoundFolderOpen();
	const char			*GetSoundPageChange();

	void				ScreenDimsChanged();


	DBOOL				IsEnglish()							{ return m_bEnglish; }

	int					GetXOffset()						{return m_Offset.x;}
	int					GetYOffset()						{return m_Offset.y;}
    LTFLOAT				GetXRatio()							{return m_fXRatio;}
    LTFLOAT				GetYRatio()							{return m_fYRatio;}

    uint32              GetScreenWidth();
    uint32              GetScreenHeight();

	
	void				DrawMessage(CLTGUIFont* pFont, int nMessageId);


	//call Setup() before entering a 2-d state (folder)
	//call Clean() before returning to the game
	DBOOL				Setup();
	void				Clean();

	// NOLF
	void				DrawFolder();

protected:
	// More initialization

	DBOOL				InitFonts();
	DBOOL				InitEngineFont(CLTGUIFont *pFont, int nNameID, int nWidthID, int nHeightID, int nN, int nH, int nD, LTBOOL bFixed = LTTRUE);
	DBOOL				InitEngineFont(CLTGUIFont *pFont, char *lpszName, int nWidth, int nHeight, HLTCOLOR hColor);
	DBOOL				SetupFont(CLTGUIFont *pFont, DBOOL bFixed = LTTRUE, DDWORD dwFlags = LTF_INCLUDE_ALL);

protected:
	DBOOL				m_bEnglish;				// True if the resource file has English as the specified language

	CLTGUIFont			*m_pTitleFont;			// Title font 
	CLTGUIFont			*m_pLargeFont;			// Large fading font
	CLTGUIFont			*m_pSmallFont;			// Small fading font
	CLTGUIFont			*m_pTinyFont;			// Tiny fading font
	CLTGUIFont			*m_pHelpFont;			// Help font
	CLTGUIFont			*m_pSmallFixedFont;		// Small non-fading font
	CLTGUIFont			*m_pLargeFixedFont;		// Large non-fading font
	CLTGUIFont			*m_pTinyFixedFont;		// Tiny non-fading font
	CLTGUIFont			*m_pMessageFont;		// Font used in HUD for messages
	CLTGUIFont			*m_pObjectiveFont;		// Font used in objectives display
					
	CLTGUIFont			*m_pAlienFont;			// Alien fading font
	CLTGUIFont			*m_pMarineFont;			// Marine fading font
	CLTGUIFont			*m_pPredatorFont;		// Predator fading font

	CLTGUIFont			*m_pPDAFont;			// PDA font
	CLTGUIFont			*m_pMEMOFont;			// Memo font

	HSURFACE			m_hSurfLoading;			// Loading level surface
	HSURFACE			m_hSurfUpArrow;			// The up arrow surface
	HSURFACE			m_hSurfDownArrow;		// The down arrow surface
	HSURFACE			m_hSurfLeftArrow;		// The left arrow surface
	HSURFACE			m_hSurfRightArrow;		// The right arrow surface
	HSURFACE			m_hBlank;

	int					m_nYesVKeyCode;			// The virtual key code for "yes" responses
	int					m_nNoVKeyCode;			// The virtual key code for "no" responses

	CInterfaceSurfMgr	m_InterfaceSurfMgr;	// Used to share title graphics

	HDECOLOR			m_hTransColor;

	DIntPt				m_Offset;
	DFLOAT				m_fXRatio;
	DFLOAT				m_fYRatio;

	CString				m_csSoundSelect;
	CString				m_csSoundChoose;
	CString				m_csSoundFolderChange;
	CString				m_csSoundFolderOpen;
	CString				m_csSoundPageChange;

    int					m_dwScreenWidth;
    int					m_dwScreenHeight;
};

inline uint32 CInterfaceResMgr::GetScreenWidth()
{
	if (m_dwScreenWidth < 0)
		ScreenDimsChanged();
	return m_dwScreenWidth;
}
inline uint32 CInterfaceResMgr::GetScreenHeight()
{
	if (m_dwScreenHeight < 0)
		ScreenDimsChanged();
	return m_dwScreenHeight;
}

#endif // !defined(AFX_INTERFACERESMGR_H__44498A80_ECE2_11D2_B2DB_006097097C7B__INCLUDED_)

// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceResMgr.h
//
// PURPOSE : Manager for resources associated with the interface
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_INTERFACERESMGR_H_)
#define _INTERFACERESMGR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ltguimgr.h"
#include "InterfaceSurfMgr.h"
#include "iltfontmanager.h"

class CGameClientShell;
class CInterfaceResMgr;
extern CInterfaceResMgr* g_pInterfaceResMgr;

typedef std::vector<CUIFont*> FontArray;
typedef std::set<HTEXTURE> TextureSet;

const int kMaxSoundFilename = 64;

class CInterfaceResMgr
{
public:
	CInterfaceResMgr();
	virtual ~CInterfaceResMgr();

    LTBOOL              Init();
	void				Term();
	HSURFACE			GetSharedSurface(const char *lpszPath)	{ return m_InterfaceSurfMgr.GetSurface(lpszPath); }
	void				FreeSharedSurface(const char *lpszPath)	{ m_InterfaceSurfMgr.FreeSurface(lpszPath); }
	void				FreeSharedSurface(HSURFACE hSurf)	{ m_InterfaceSurfMgr.FreeSurface(hSurf); }

	HTEXTURE			GetTexture(const char *szTexName);

	CUIFont*			GetFont(uint32 nIndex);

	HSURFACE			GetSurfaceCursor();

	const char			*GetSoundSelect();
	const char			*GetSoundChange();
	const char			*GetSoundPageChange();
	const char			*GetSoundUnselectable();
	const char			*GetSoundArrowUp();
	const char			*GetSoundArrowDown();
	const char			*GetSoundArrowLeft();
	const char			*GetSoundArrowRight();
	const char			*GetObjectiveAddedSound();
	const char			*GetObjectiveRemovedSound();
	const char			*GetObjectiveCompletedSound();

	void				ScreenDimsChanged();


    LTBOOL              IsEnglish()                         { return m_bEnglish; }

	void				DrawScreen();


	void				ConvertScreenRect(LTRect &rect);
	void				ConvertScreenPos(LTIntPt &pos) {ConvertScreenPos(pos.x,pos.y);}
	void				ConvertScreenPos(int &x, int &y);
    LTFLOAT             GetXRatio()                         {return m_fXRatio;}
    LTFLOAT             GetYRatio()                         {return m_fYRatio;}
    const LTFLOAT				Get4x3Ratio()						{ return 640.0f / 480.0f; }
    int					Get4x3Offset(int w, int h);
    int					Get640x480Offset();
    const LTFLOAT		GetAspectRatio()					{ return m_fAspectRatio; }
    const LTFLOAT		GetInvAspectRatio()					{ return m_fInvAspectRatio;  }

    uint32              GetScreenWidth();
    uint32              GetScreenHeight();

	void				DrawMessage(int nMessageId, uint8 nFontSize = 0);
	void				DrawMessage(const char *pString, uint8 nFontSize = 0);


	//call Setup() before entering a 2-d state (screen)
	//call Clean() before returning to the game
    LTBOOL               Setup();
	void				Clean();

	const LTFLOAT GetHorizontalFOV(LTFLOAT vFOV)
	{
		LTFLOAT vRadFov = DEG2RAD(vFOV);
		LTFLOAT fov = 2 * (
			std::atan(
				std::tan(vRadFov / 2) * GetAspectRatio()
			)
		);

		return RAD2DEG(fov);
	}

	const LTFLOAT Get4x3HorizontalFOV(LTFLOAT vFOV)
	{
		LTFLOAT vRadFov = DEG2RAD(vFOV);
		LTFLOAT fov = 2 * (
			std::atan(
				std::tan(vRadFov / 2) * Get4x3Ratio()
			)
		);

		return RAD2DEG(fov);
	}

	const LTFLOAT GetVerticalFOV(LTFLOAT fFOV)
	{
		LTFLOAT fRadFov = DEG2RAD(fFOV);
		LTFLOAT fov = 2 * (
			std::atan(
				std::tan(fRadFov / 2) * GetInvAspectRatio()
			)
			);

		return RAD2DEG(fov);
	}

protected:
	// More initialization

    LTBOOL              InitFonts();
	CUIFont*			CreateFont(char const* pszFontFile, char const* pszFontFace, uint8 ptSize);

protected:
    LTBOOL				m_bEnglish;             // True if the resource file has English as the specified language

	TextureSet			m_TextureSet;			// list of used textures

	FontArray			m_FontArray;

	HSURFACE			m_hSurfCursor;			// The software cursor surface

	int					m_nYesVKeyCode;			// The virtual key code for "yes" responses
	int					m_nNoVKeyCode;			// The virtual key code for "no" responses

	CInterfaceSurfMgr	m_InterfaceSurfMgr;	// Used to share title graphics

    LTFLOAT              m_fXRatio;
    LTFLOAT              m_fYRatio;
    LTFLOAT				 m_fAspectRatio;
    LTFLOAT				 m_fInvAspectRatio;
    uint32              m_dwScreenWidth;
    uint32              m_dwScreenHeight;

	char				m_szSoundSelect[kMaxSoundFilename];
	char				m_szSoundChange[kMaxSoundFilename];
	char				m_szSoundPageChange[kMaxSoundFilename];
	char				m_szSoundUnselectable[kMaxSoundFilename];
	char				m_szSoundArrowUp[kMaxSoundFilename];
	char				m_szSoundArrowDown[kMaxSoundFilename];
	char				m_szSoundArrowLeft[kMaxSoundFilename];
	char				m_szSoundArrowRight[kMaxSoundFilename];
	char				m_szSoundObjAdd[kMaxSoundFilename];
	char				m_szSoundObjRemove[kMaxSoundFilename];
	char				m_szSoundObjComplete[kMaxSoundFilename];

	CLTGUIWindow	    m_MsgDlg;
	CLTGUITextCtrl*		m_pMsgText;

};

#define TERMSHAREDSURF(surf) if(surf) { g_pInterfaceResMgr->FreeSharedSurface(surf); surf = NULL; }

inline int CInterfaceResMgr::Get4x3Offset(int w = 0, int h = 0)
{
	LTFLOAT width = (LTFLOAT)GetScreenWidth();
	LTFLOAT height = (LTFLOAT)GetScreenHeight();
	LTFLOAT ratio = Get4x3Ratio();

	if (w) {
		width = (LTFLOAT)w;
	}

	if (h) {
		height = (LTFLOAT)h;
	}

	return (int)(0.5f * ( width - ( height * ratio ) ));
}

//
// I hate this name, but it's the only thing that came to mind.
// All the interfaces seem to be designed for 640x480 then scaled up.
//
inline int CInterfaceResMgr::Get640x480Offset()
{
	int adjustedScreenWidth = 480 * g_pInterfaceResMgr->GetAspectRatio();
	return g_pInterfaceResMgr->Get4x3Offset(adjustedScreenWidth, 480);
}

inline uint32 CInterfaceResMgr::GetScreenWidth()
{
	RMode currentMode;
    g_pLTClient->GetRenderMode(&currentMode);

	if (m_dwScreenWidth != currentMode.m_Width)
		ScreenDimsChanged();
	return m_dwScreenWidth;
}
inline uint32 CInterfaceResMgr::GetScreenHeight()
{
	RMode currentMode;
    g_pLTClient->GetRenderMode(&currentMode);

	if (m_dwScreenHeight != currentMode.m_Height)
		ScreenDimsChanged();
	return m_dwScreenHeight;
}


#endif // !defined(_INTERFACERESMGR_H_)

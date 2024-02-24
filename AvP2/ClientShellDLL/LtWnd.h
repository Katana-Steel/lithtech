/****************************************************************************
;
;	 MODULE:		LTWnd (.h)
;
;	PURPOSE:		LithTech Window class
;
;	HISTORY:		11/30/98 [kml] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/

#ifndef _LTWND_H_
#define _LTWND_H_

// Includes
#include "basedefs_de.h"
#include "cpp_clientshell_de.h"
#include "de_client.h"

// Defines
#define LTWF_NORMAL				0
#define LTWF_VDRAG				1
#define LTWF_HDRAG				2
#define LTWF_DRAGGABLE			(LTWF_VDRAG | LTWF_HDRAG)
#define LTWF_TRANSPARENT		4
#define LTWF_SIZEABLE			8
#define LTWF_FIXEDCHILD			16
#define LTWF_PARENTNOTIFY		32
#define LTWF_DISABLED			64
#define LTWF_TOPMOST			128
#define LTWF_PARENTDRAG			256
#define LTWF_NOCLOSE			512
#define LTWF_MANUALDRAW			1024
#define LTWF_NOUPDATE			2048
#define LTWF_TERM				4096

// Window messages
#define LTWM_COMMAND			1001

// Window commands
// These need to be defined if you are using the associated windows
#define LTWC_MINIMIZE			1
#define LTWC_CLOSE				2
#define LTWC_DOMENU				3
#define LTWC_SELECT_SPELL		4
#define LTWC_SELECT_TEXT		5
#define LTWC_QUIT				10
#define LTWC_LOAD				11
#define LTWC_REPLAY				12
#define LTWC_CONTINUE			13
#define LTWC_KEYPAD_1			100
#define LTWC_KEYPAD_2			101
#define LTWC_KEYPAD_3			102
#define LTWC_KEYPAD_4			103
#define LTWC_KEYPAD_5			104
#define LTWC_KEYPAD_6			105
#define LTWC_KEYPAD_7			106
#define LTWC_KEYPAD_8			107
#define LTWC_KEYPAD_9			108
#define LTWC_KEYPAD_STAR		109
#define LTWC_KEYPAD_0			110
#define LTWC_KEYPAD_POUND		111
#define LTWC_KEYPAD_ENTER		112
#define LTWC_KEYPAD_CANCEL		113

// States
#define LTWS_NORMAL			0
#define LTWS_CLOSED			1
#define LTWS_MINIMIZED		2

typedef CArray<HSURFACE,HSURFACE> CLTSurfaceArray;

extern HDECOLOR g_hColorTransparent;

// Classes
class CLTWnd
{

public:

	CLTWnd();
	virtual ~CLTWnd() { Term(); }

	// Initialization and termination
	virtual BOOL			Init(int nControlID, char* szWndName, CLTWnd* pParentWnd, HSURFACE hSurf, int xPos = 0, int yPos = 0, DWORD dwFlags = LTWF_NORMAL, DWORD dwState = LTWS_NORMAL);
	virtual BOOL			InitFromBitmap(int nControlID, char* szWndName, CLTWnd* pParentWnd, char* szBitmap, int xPos = 0, int yPos = 0, DWORD dwFlags = LTWF_NORMAL, DWORD dwState = LTWS_NORMAL);
	virtual void			Term();
	void					TermNext() { m_dwFlags |= LTWF_TERM; }
	virtual void			FreeAllSurfaces(CLTSurfaceArray* pcollSurfs = NULL);
	BOOL					IsInitialized() { return m_bInitialized; }

	HSURFACE				GetSurface() { return m_hSurf; }
	POSITION				GetPos() { return(m_pos); }
	CString&				GetName() { return m_sWndName; }
	DWORD					GetFlags() { return m_dwFlags; }
	CLTWnd*					GetParent() { return m_pParentWnd; }
	int						GetCursorXClick() { return m_xCursorClick; }
	int						GetCursorYClick() { return m_yCursorClick; }
	BOOL					IsFlagSet(DWORD dwFlag) { return (m_dwFlags & dwFlag); }
	//void					GetTransparentColor(float* r, float* g, float* b);
	HDECOLOR				GetTransparentColor();
	DWORD					GetState() { return m_dwState; }
	int						GetWidth() { return m_nWidth; }
	int						GetHeight() { return m_nHeight; }
	void					SetWidth(int nWidth) { m_nWidth = nWidth; }
	void					SetHeight(int nHeight) { m_nHeight = nHeight; }
	void					SetWidthHeight(int nWidth, int nHeight) { m_nWidth = nWidth; m_nHeight = nHeight; }
	void					SetSurfaceOffsets(int x, int y, int w, int h);

	void					SetControlID(int nControlID) { m_nControlID = nControlID; }

	void					SetFlag(DWORD dwFlag) { m_dwFlags |= dwFlag; }
	void					ClearFlag(DWORD dwFlag) { m_dwFlags &= ~dwFlag; }
	void					SetPos(POSITION pos) { m_pos = pos; }
	HSURFACE				SetSurface(HSURFACE hSurf, BOOL bDeleteSurf = FALSE);
	void					SetTransparentColor(float r = 1.0, float g = 0.0, float b = 1.0);
	void					DeleteTransparentColor();
	void					SetTransparentColor(HDECOLOR hColor) { m_hColorTransparent = hColor; }
	void					SetDeleteSurf(BOOL bDelete) { m_bDeleteSurfOnTerm = bDelete; }

	virtual BOOL			Update(float fTimeDelta);
	virtual BOOL			Draw(HSURFACE hSurf);
	CLTWnd*					SetFocus();
	BOOL					IsActiveWindow() { return (this == s_pWndActive); }
	BOOL					SetWindowPos(CLTWnd* pInsertAfter, int xPos, int yPos, int nWidth, int nHeight, DWORD dwFlags);

	virtual BOOL			MoveWindow(int xPos, int yPos, int nWidth, int nHeight, BOOL bReDraw = TRUE);
	virtual BOOL			MoveWindow(CRect& rcPos, BOOL bReDraw = TRUE);
	virtual BOOL 			MoveWindow(int xPos, int yPos, BOOL bReDraw = TRUE) { m_xPos = xPos; m_yPos = yPos; return TRUE; }
	virtual BOOL			MoveWindowX(int xPos, BOOL bReDraw = TRUE) { m_xPos = xPos; return TRUE; }
	virtual BOOL			MoveWindowY(int yPos, BOOL bReDraw = TRUE) { m_yPos = yPos; return TRUE; }
							
	virtual BOOL			ShowWindow(BOOL bShow = TRUE, BOOL bPlaySound = TRUE, BOOL bAnimate = TRUE);
	BOOL					IsVisible() { return m_bVisible; }
	BOOL					EnableWindow(BOOL bEnable = TRUE);
	BOOL					IsWindowEnabled() const { return m_bEnabled; }
	void					ShowAllChildren(BOOL bShow = TRUE);

	virtual BOOL			PtInWnd(int x, int y);  // x,y are screen coords
	CLTWnd*					GetWndFromPt(int x, int y);

	void					ArrangeAllChildren(DWORD dwWidth, DWORD dwHeight, DWORD dwOldWidth, DWORD dwOldHeight);

	void					SetCapture(CLTWnd* pWnd) { s_pWndCapture = pWnd; }

	// Absolute screen coordinates
	int						GetWindowLeft();
	int						GetWindowTop();
	void					GetWindowRect(CRect* pRect);
	void					GetClipRect(CRect* pRect);
	virtual void			GetSurfaceRect(CRect *pRect);

	virtual BOOL			SendMessage(int nMsg, int nParam1 = 0,int nParam2 = 0) { return FALSE; }

	// These will get passed down to the appropriate window
	BOOL					HandleMouseMove(int xPos, int yPos);
	BOOL					HandleLButtonDown(int xPos, int yPos);
	BOOL					HandleLButtonUp(int xPos, int yPos);
	BOOL					HandleLButtonDblClick(int xPos, int yPos);
	BOOL					HandleRButtonDown(int xPos, int yPos);
	BOOL					HandleRButtonUp(int xPos, int yPos);
	void					RemoveAllChildren();
	void					RemoveFromParent();

	void					AddChild(CLTWnd* pWnd, BOOL bIgnoreTopmost = FALSE);
	CPtrList				m_lstChildren;			// List of windows that cannot go outside of our borders

protected:

	virtual BOOL			DrawToSurface(HSURFACE hSurfDest);
	// NOTE: return TRUE on these if you are capturing the message.
	// i.e. if you do not want the game to process them.
	virtual BOOL			OnLButtonUp(int xPos, int yPos);
	virtual BOOL			OnLButtonDown(int xPos, int yPos);
	virtual BOOL			OnLButtonDblClick(int xPos, int yPos) { return FALSE; }
					
	virtual BOOL			OnRButtonUp(int xPos, int yPos) { return FALSE; }
	virtual BOOL			OnRButtonDown(int xPos, int yPos);
	virtual BOOL			OnRButtonDblClick(int xPos, int yPos) { return FALSE; }
							
	virtual BOOL			OnMouseMove(int xPos, int yPos);
							
	virtual BOOL			OnSetFocus(CLTWnd* pOldWnd) { return FALSE; }
	virtual BOOL			OnKillFocus(CLTWnd* pNewWnd) { return FALSE; }
	virtual BOOL			OnDrag(int xPos, int yPos);

	BOOL					m_bEnabled;				// Are we enabled (accepting clicks/commands)
	BOOL					m_bVisible;				// Are we visible
	int						m_nControlID;			// ID of this window
	int						m_xPos;					// x pos relative to parent's top/left
	int						m_yPos;					// y pos relative to parent's top/left
	int						m_nWidth;				// Window width
	int						m_nHeight;				// Window height
						
	int						m_xSurfPos;				// Surface pos relative to top/left
	int						m_ySurfPos;				// Surface pos relative to top/left
	int						m_nSurfWidth;			// Surface width
	int						m_nSurfHeight;			// Surface height
						
	int						m_xCursor;				// Cursor relative to top/left
	int						m_yCursor;				// Cursor relative to top/left
	int						m_xCursorClick;			// Where they clicked relative to top/left
	int						m_yCursorClick;			// Where they clicked relative to top/left
						
	HSURFACE				m_hSurf;				// Main surface

	CLTWnd*					m_pParentWnd;			// Parent window
	CString					m_sWndName;				// Name of the window as string
	BOOL					m_bInitialized;			// Are we initialized
	POSITION				m_pos;					// Position in paren't list

	HDECOLOR				m_hColorTransparent;	// Transparent color (handle)
	DWORD					m_dwState;				// Normal, minimized, closed, etc...

	/*struct
	{
		float r;
		float g;
		float b;
	}						m_colorTransparent;		// Transparent color as (r,g,b)*/
	BOOL					m_bDeleteSurfOnTerm;	// Should we delete the surface?
	DWORD					m_dwFlags;				// Window flags

	static CLTWnd*			s_pWndActive;			// Window that has the input focus
	static CLTWnd*			s_pMainWnd;				// Main screen window
	static CLTWnd*			s_pWndCapture;			// Window that has the mouse capture
};

// Inlines
inline CLTWnd::CLTWnd()
{
	m_pos					= NULL;
	m_dwFlags				= 0;
	m_xPos					= 0;
	m_yPos					= 0;
	m_nWidth				= 0;
	m_nHeight				= 0;
	m_xSurfPos				= 0;
	m_ySurfPos				= 0;
	m_nSurfWidth			= 0;
	m_nSurfHeight			= 0;
	m_bEnabled				= TRUE;
	m_xCursor				= 0;
	m_yCursor				= 0;
	m_xCursorClick			= 0;
	m_yCursorClick			= 0;
	m_pos					= NULL;
	m_bInitialized			= FALSE;
	m_bVisible				= TRUE;
	m_hColorTransparent		= g_hColorTransparent;//g_pInterface->CreateColor(0.0, 0.0, 0.0, FALSE);
	m_bDeleteSurfOnTerm		= FALSE;
	m_nControlID			= 0;
	m_dwState				= LTWS_NORMAL;
}

inline void CLTWnd::AddChild(CLTWnd* pWnd, BOOL bIgnoreTopmost)
{
	ASSERT(m_bInitialized);
	ASSERT(pWnd);

	if(bIgnoreTopmost)
	{
		// Stick it into our list of children and set its position variable
		pWnd->SetPos(m_lstChildren.AddTail(pWnd));
	}
	else
	{
		POSITION pos = m_lstChildren.GetTailPosition();
		while (pos)
		{
			CLTWnd* pWndTest = (CLTWnd*)m_lstChildren.GetPrev(pos);
			ASSERT(pWndTest);
			if(!pWndTest->IsFlagSet(LTWF_TOPMOST))
			{
				pWnd->SetPos(m_lstChildren.InsertAfter(pWndTest->GetPos(),pWnd));
				return;
			}
		}
		pWnd->SetPos(m_lstChildren.AddHead(pWnd));
	}
}

/*inline void CLTWnd::GetTransparentColor(float* r, float* g, float* b)
{
	*r = m_colorTransparent.r;
	*g = m_colorTransparent.g;
	*b = m_colorTransparent.b;
}*/

inline HDECOLOR CLTWnd::GetTransparentColor()
{
	return m_hColorTransparent;
}

inline void CLTWnd::RemoveFromParent()
{
	if(m_pParentWnd)
	{
		ASSERT(m_pos);
		m_pParentWnd->m_lstChildren.RemoveAt(m_pos);
		m_pParentWnd = NULL;
		m_pos = NULL;
	}
}

inline int CLTWnd::GetWindowLeft()
{
	if (m_pParentWnd)
		return m_xPos + m_pParentWnd->GetWindowLeft();
	return 0; // root window so offset is 0
}

inline int CLTWnd::GetWindowTop()
{
	if (m_pParentWnd)
		return m_yPos + m_pParentWnd->GetWindowTop();
	return 0; // root window so offset is 0
}

inline void CLTWnd::SetSurfaceOffsets(int x, int y, int w, int h)
{
	m_xSurfPos = x;
	m_ySurfPos = y;
	m_nSurfWidth = w;
	m_nSurfHeight = h;
}

#endif

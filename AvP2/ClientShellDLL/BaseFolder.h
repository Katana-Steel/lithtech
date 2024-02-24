// BaseFolder.h: interface for the CBaseFolder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASEFOLDER_H__88EE6E21_1515_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_BASEFOLDER_H__88EE6E21_1515_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LTGUIMgr.h"
#include "BitmapCtrl.h"
#include "FrameCtrl.h"
#include "GroupCtrl.h"
#include "ListCtrl.h"
#include "SliderCtrl.h"
#include "CycleCtrl.h"
#include "stdlith.h"
#include "BaseScaleFX.h"
#include "StaticTextCtrl.h"
#include "LayoutMgr.h"
#include "MultiplayerMgrDefs.h"

#pragma warning (disable : 4503)
#include <vector>

#define MAX_INT_ATTACHMENTS			5
#define MAX_TAB_GROUPS				8

#define LOGO_SCALEFX_NONE			0
#define LOGO_SCALEFX_SIERRA			1
#define LOGO_SCALEFX_GAMESPY		2

struct INT_ATTACH
{
	INT_ATTACH() { fScale = 1.0f;nAttachmentID = -1;pszSocket=LTNULL;}

	LTFLOAT	fScale;
	int		nAttachmentID;
	char	*pszSocket;
};
struct AttachmentData
{
	AttachmentData() { socket = INVALID_MODEL_SOCKET; }

	CBaseScaleFX	sfx;
	HMODELSOCKET	socket;
};

class CFolderMgr;

class CPageBreakCtrl : public CLTGUICtrl
{
public:
	void	Render ( HSURFACE hDestSurf ) {}
	int		GetWidth ( ) {return 0; }
	int		GetHeight ( ) {return 0; }
    uint32  GetID() { return m_dwCommandID; }

};

const int kNoSelection = -999;
enum eNavGroups
{
	kNoGroup = -1,
	kLinkGroup,
	kTabGroup,
	kFreeGroup,
	kCustomGroup0,
	kCustomGroup1,
	kCustomGroup2,
	kBackGroup,
	kNextGroup,
};


class CBaseFolder : public CLTGUICommandHandler
{
public:
	CBaseFolder();
	virtual ~CBaseFolder();

	// Initialization/Termination
    virtual LTBOOL   Init(int nFolderID);
	virtual void	Term();

	// Builds the folder
    virtual LTBOOL   Build();
    inline  LTBOOL   IsBuilt() const { return m_bBuilt; }

    virtual LTBOOL   IsAvailable() { return LTTRUE; }  //used for folders that may or may not be available

	virtual void	Escape();

    virtual uint32 OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	// Get the folder ID
	inline int	GetFolderID() const	{ return m_nFolderID; }


	// Renders the folder to a surface
    virtual LTBOOL   Render(HSURFACE hDestSurf);
	virtual void	UpdateInterfaceSFX();

	// Creates the title for the folder
    LTBOOL           CreateTitle(HSTRING hString);
    LTBOOL           CreateTitle(int nStringID);
    LTBOOL           CreateTitle(char *lpszTitle);

	// Logo information
	void		SetLogo(int nLogo);
	void		UseLogo(LTBOOL bLogo)		{ m_bUseLogo = bLogo; }


	//set the background for the folder
    void         UseArrows(LTBOOL bArrows, LTBOOL bLeft=LTTRUE, LTBOOL bRight=LTTRUE);
    void         UseBack(LTBOOL	bBack=LTTRUE,LTBOOL	bOK=LTFALSE, LTBOOL bReturn = LTFALSE );
    void         UseContinue(int nContinueID, int nHelpID = LTNULL, int nStringID = LTNULL);
    void         UseMain(LTBOOL bMain=LTTRUE);

	// This is called when the folder gets or loses focus
    virtual void    OnFocus(LTBOOL bFocus);
	virtual void	FocusSetup(LTBOOL bFocus)	{ }

	// Handles a key press.  Returns FALSE if the key was not processed through this method.
	// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
    virtual LTBOOL   HandleKeyDown(int key, int rep);
    virtual LTBOOL   HandleKeyUp(int key) { return LTFALSE; }
    virtual LTBOOL   HandleChar(char c);
    virtual LTBOOL   HandleForceUpdate() { return LTFALSE; }

	//reset selection and help display
	virtual void	ForceMouseUpdate();
	virtual void	UpdateHelpText();

	// Mouse messages
    virtual LTBOOL   OnLButtonDown(int x, int y);
    virtual LTBOOL   OnLButtonUp(int x, int y);
    virtual LTBOOL   OnLButtonDblClick(int x, int y);
    virtual LTBOOL   OnRButtonDown(int x, int y);
    virtual LTBOOL   OnRButtonUp(int x, int y);
    virtual LTBOOL   OnRButtonDblClick(int x, int y);
    virtual LTBOOL   OnMouseMove(int x, int y);
    virtual LTBOOL   OnMouseWheel(int x, int y, int delta);

	inline int 		GetPageLeft() const {return m_PageRect.left;}
	inline int 		GetPageRight() const {return m_PageRect.right;}
	inline int 		GetPageTop() const {return m_PageRect.top;}
	inline int 		GetPageBottom() const {return m_PageRect.bottom;}

	CLTGUIFont*		GetTitleFont();
	CLTGUIFont*		GetHelpFont();
	CLTGUIFont*		GetTinyFont();
	CLTGUIFont*		GetSmallFont();
	CLTGUIFont*		GetLargeFont();
	CLTGUIFont*		GetTinyFixedFont();
	CLTGUIFont*		GetSmallFixedFont();
	CLTGUIFont*		GetLargeFixedFont();
	CLTGUIFont*		GetDefaultFont(LTBOOL bFixed = LTFALSE);

	virtual char*	GetChangeSound()	{return m_sDefaultChangeSound;}
	virtual char*	GetSelectSound()	{return m_sDefaultSelectSound;}
	virtual char*	GetAmbientSound()	{return m_sAmbientSound;}

    inline void     SetTitleColor(HLTCOLOR titleColor)  { m_hTitleColor = titleColor; }

	//these return the index of the added control
    int             AddFixedControl(CLTGUICtrl* pCtrl, eNavGroups eTabGroup, LTIntPt pos, LTBOOL bSelectable = LTTRUE);
	int				AddFreeControl(CLTGUICtrl* pCtrl);

	int				SetSelection(int select);
	int				NextSelection();
	int				PreviousSelection();
	int				NextTab();
	int				PreviousTab();
	inline int		GetSelection() const		{return m_nSelection;}

	void			SetCapture(CLTGUICtrl *pCtrl)	{m_pCaptureCtrl = pCtrl;}
	CLTGUICtrl*		GetCapture()					{return m_pCaptureCtrl;}

	inline CLTGUICtrl*		GetSelectedControl()  {return GetControl(m_nSelection);}
	CLTGUICtrl*				GetControl(int nIndex);
	int						GetIndex(CLTGUICtrl* pCtrl);

    inline LTBOOL    IsFirstPage() const {return (m_nFirstDrawn == 0);}
    inline LTBOOL    IsLastPage() const  {return (m_nLastDrawn >= (int)m_controlArray.size()-1);}

    virtual LTBOOL   NextPage(LTBOOL bChangeSelection = LTTRUE);
    virtual LTBOOL   PreviousPage(LTBOOL bChangeSelection = LTTRUE);
	void			CheckArrows();

	inline void		SetItemSpacing(int space)	{m_nItemSpacing = space;}

	//this function creates a string, the caller must free it
    virtual         HSTRING GetHelpString(uint32 dwHelpId, int nControlIndex);

	//default font for items is the large font
	// These AddXXX() functions call CreateXXX() and then add the control to the FreeControl list
    CLTGUITextItemCtrl*     AddTextItem(HSTRING hString, uint32 commandID, int helpID, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL, int *pnValue=LTNULL);
    CLTGUITextItemCtrl*     AddTextItem(int stringID, uint32 commandID, int helpID, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL, int *pnValue=LTNULL);
    CLTGUITextItemCtrl*     AddTextItem(const char *pString, uint32 commandID, int helpID, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL, int *pnValue=LTNULL);
    CStaticTextCtrl*        AddStaticTextItem(HSTRING hString, uint32 commandID, int helpID, int width, int height, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL);
    CStaticTextCtrl*        AddStaticTextItem(int stringID, uint32 commandID, int helpID, int width, int height, LTBOOL bFixed = LTFALSE,  CLTGUIFont *pFont = LTNULL);
    CStaticTextCtrl*        AddStaticTextItem(char *pString, uint32 commandID, int helpID, int width, int height, LTBOOL bFixed = LTFALSE,  CLTGUIFont *pFont = LTNULL);
    CLTGUIEditCtrl*         AddEditCtrl(HSTRING hDescription, uint32 commandID, int helpID, char *pBuffer, int nBufferSize, int nTextOffset = 25, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL);
    CLTGUIEditCtrl*         AddEditCtrl(int nDescriptionID, uint32 commandID, int helpID, char *pBuffer, int nBufferSize, int nTextOffset = 25, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL);
    CLTGUIEditCtrl*         AddEditCtrl(char *pszDescription, uint32 commandID, int helpID, char *pBuffer, int nBufferSize, int nTextOffset = 25, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL);
    CCycleCtrl*             AddCycleItem(HSTRING hText, int helpID, int nHeaderWidth, int nSpacerWidth=25, int *pnValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont=LTNULL);
    CCycleCtrl*             AddCycleItem(int stringID,  int helpID, int nHeaderWidth, int nSpacerWidth=25, int *pnValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont=LTNULL);
    CCycleCtrl*             AddCycleItem(char *pString, int helpID, int nHeaderWidth, int nSpacerWidth=25, int *pnValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont=LTNULL);
    CToggleCtrl*            AddToggle(HSTRING hText, int helpID, int nRightColumnOffset, LTBOOL *pbValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont=LTNULL);
    CToggleCtrl*            AddToggle(int stringID, int helpID, int nRightColumnOffset, LTBOOL *pbValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont=LTNULL);
    CToggleCtrl*            AddToggle(char *pString, int helpID, int nRightColumnOffset, LTBOOL *pbValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont=LTNULL);
    CSliderCtrl*            AddSlider(HSTRING hText, int helpID, int nSliderOffset, int nSliderWidth, int *pnValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont =LTNULL);
    CSliderCtrl*            AddSlider(int stringID,  int helpID, int nSliderOffset, int nSliderWidth, int *pnValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont =LTNULL);
    CSliderCtrl*            AddSlider(char *pString, int helpID, int nSliderOffset, int nSliderWidth, int *pnValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont =LTNULL);
    CLTGUIColumnTextCtrl*   AddColumnText(DWORD dwCommandID, int helpID, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL, DWORD dwParam1 = 0, DWORD dwParam2 = 0);
	CGroupCtrl*				AddGroup(int nWidth , int nHeight, int helpID);

	// These CreateXXX() create controls but do not add them to any list
    CLTGUITextItemCtrl*     CreateTextItem(HSTRING hString, uint32 commandID, int helpID, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL, int *pnValue=LTNULL);
    CLTGUITextItemCtrl*     CreateTextItem(int stringID, uint32 commandID, int helpID, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL, int *pnValue=LTNULL);
    CLTGUITextItemCtrl*     CreateTextItem(const char *pString, uint32 commandID, int helpID, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL, int *pnValue=LTNULL);
    CStaticTextCtrl*        CreateStaticTextItem(HSTRING hString, uint32 commandID, int helpID, int width, int height = 0, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL);
    CStaticTextCtrl*        CreateStaticTextItem(int stringID, uint32 commandID, int helpID, int width, int height = 0, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL);
    CStaticTextCtrl*        CreateStaticTextItem(char *pString, uint32 commandID, int helpID, int width, int height = 0, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL);
    CLTGUIEditCtrl*         CreateEditCtrl(HSTRING hDescription, uint32 commandID, int helpID, char *pBuffer, int nBufferSize, int nTextOffset = 25, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL);
    CLTGUIEditCtrl*         CreateEditCtrl(int nDescriptionID, uint32 commandID, int helpID, char *pBuffer, int nBufferSize, int nTextOffset = 25, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL);
    CLTGUIEditCtrl*         CreateEditCtrl(char *pszDescription, uint32 commandID, int helpID, char *pBuffer, int nBufferSize, int nTextOffset = 25, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL);
    CCycleCtrl*             CreateCycleItem(HSTRING hText, int helpID, int nHeaderWidth, int nSpacerWidth=25, int *pnValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont=LTNULL);
    CCycleCtrl*             CreateCycleItem(int stringID,   int helpID, int nHeaderWidth, int nSpacerWidth=25, int *pnValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont=LTNULL);
    CCycleCtrl*             CreateCycleItem(char *pString, int helpID, int nHeaderWidth, int nSpacerWidth=25, int *pnValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont=LTNULL);
    CToggleCtrl*            CreateToggle(HSTRING hText, int helpID, int nRightColumnOffset, LTBOOL *pbValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont=LTNULL);
    CToggleCtrl*            CreateToggle(int stringID, int helpID, int nRightColumnOffset, LTBOOL *pbValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont=LTNULL);
    CToggleCtrl*            CreateToggle(char *pString, int helpID, int nRightColumnOffset, LTBOOL *pbValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont=LTNULL);
    CSliderCtrl*            CreateSlider(HSTRING hText, int helpID, int nSliderOffset, int nSliderWidth, int *pnValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont =LTNULL);
    CSliderCtrl*            CreateSlider(int stringID,  int helpID, int nSliderOffset, int nSliderWidth, int *pnValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont =LTNULL);
    CSliderCtrl*            CreateSlider(char *pString, int helpID, int nSliderOffset, int nSliderWidth, int *pnValue=LTNULL, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont =LTNULL);
    CLTGUIColumnTextCtrl*   CreateColumnText(DWORD dwCommandID, int helpID, LTBOOL bFixed = LTFALSE, CLTGUIFont *pFont = LTNULL, DWORD dwParam1 = 0, DWORD dwParam2 = 0);
	CGroupCtrl*				CreateGroup(int nWidth , int nHeight, int helpID);


	void	AddPageBreak();
	void	AddBlankLine();
	void	AddFrame(int x, int y, int width, int height, int headerHeight = 0, LTBOOL bOuterFrame = LTTRUE);


	void	BuildPasswordDlg();
	void	ShowPasswordDlg();
	void	HidePasswordDlg();


protected:
	// Calls UpdateData on each control in the folder
    void            UpdateData(LTBOOL bSaveAndValidate=LTTRUE);

	// Handle input
    virtual LTBOOL   OnUp();
    virtual LTBOOL   OnDown();
    virtual LTBOOL   OnLeft();
    virtual LTBOOL   OnRight();
    virtual LTBOOL   OnEnter();
    virtual LTBOOL   OnTab();
    virtual LTBOOL   OnPageUp();
    virtual LTBOOL   OnPageDown();

	// Sets the position of the title
	inline void		SetTitlePos(int x, int y)				{ m_titlePos.x=x; m_titlePos.y=y; }
    inline void     SetTitlePos(LTIntPt pt)                  { SetTitlePos(pt.x, pt.y); }
	inline void		SetTitleAlignment(int nAlignment)		{ m_nTitleAlign = nAlignment; }

	// Gets the index of the control that is under the specific screen point.
	// Returns FALSE if there isn't one under the specified point.
    LTBOOL       GetControlUnderPoint(int xPos, int yPos, int *pnIndex);

    CLTGUITextItemCtrl* AddLink(int stringID, uint32 commandID, int helpID, int xpos, int ypos);
	CLTGUITextItemCtrl* AddLink(int stringID, uint32 commandID, int helpID);
	CLTGUITextItemCtrl* AddTab(int stringID, uint32 commandID, int helpID);

protected:

	void			RemoveAll();
	void			RemoveFree();
	void			RemoveFixed();
	void			CalculateLastDrawn();

	void			CreateUpArrow();
	void			CreateDownArrow();
	void			CreateBack(LTBOOL bOK = LTFALSE, LTBOOL bReturn = LTFALSE);
	void			CreateMain();
	void			CreateContinue(int nStringID, int nHelpID);
	void			RemoveFixedControl(CLTGUICtrl* pControl);
	LTBOOL			SkipControl(CLTGUICtrl* pControl);

	virtual void	CreateInterfaceSFX();
	virtual void	RemoveInterfaceSFX();

	CBaseScaleFX*	CreateScaleFX(char *szFXName);
	void			CreateCharFX(INT_CHAR *pChar);
	void			CreateAttachFX(INT_ATTACH *pAttach);
	void			ClearAttachFX();

	//converts to and from the negative indexes used for fixed controls
	int				FixedIndex(int x) {return -1 - x;}

	// Set fonts
	void			SetFontColors(CLTGUIFadeColorCtrl *control, LTBOOL fixed = LTFALSE);

	void			SetTab(int n) {if (n>=0 && n<m_nNumTabs) m_nCurTab = n;}

	// Sets or restores console commands to menu needs.
	void			SetupMenuConsoleCommands( LTBOOL bEnter );
	
protected:

    LTBOOL          m_bInit;
    LTBOOL          m_bBuilt;
	LTBOOL			m_bHasFocus;

	CFolderMgr*		m_pFolderMgr;

	int				m_nFolderID;		// The ID of this folder
	int				m_nContinueID;		// The ID of the folder to show when continue is clicked

	HSTRING			m_hTitleString;		// The title string

    LTIntPt         m_titlePos;         // The title position
	int				m_nTitleAlign;		// the title alignment

	typedef std::vector<CLTGUICtrl*>	ControlArray;
	// Array of SFX owned by this folder
	typedef std::vector<CSpecialFX*>	SFXArray;
	SFXArray							m_SFXArray;

	// Array of fixed controls that this folder owns
	ControlArray						m_fixedControlArray;

	// Array of fixed controls that should be skipped by next/previous selection
	ControlArray						m_skipControlArray;

	// Array of free controls that this folder owns
//	CMoArray<CLTGUICtrl *>	m_controlArray;
	ControlArray						m_controlArray;

	ControlArray						m_tabArray[MAX_TAB_GROUPS];
	int									FindTab(CLTGUICtrl *pCtrl);


	int				m_nGroup;
	int				m_nSelection;
	int				m_nFirstDrawn;
	int				m_nLastDrawn;
	int				m_nLMouseDownItemSel;
	int				m_nRMouseDownItemSel;
	int				m_nItemSpacing;

	int				m_nNextLink;

	int				m_nNumTabs;
	int				m_nCurTab;

	CLTGUICtrl*		m_pCaptureCtrl;

    LTBOOL           m_bScrollWrap;

    LTRect           m_PageRect;
	int				m_nAlignment;
    HLTCOLOR        m_hSelectedColor;
    HLTCOLOR        m_hNormalColor;
    HLTCOLOR        m_hDisabledColor;
	HLTCOLOR		m_hHighlightColor;

    HLTCOLOR        m_hTransparentColor; // The transparent color
    HLTCOLOR        m_hTitleColor;       // Color to use for title

    uint32          m_dwCurrHelpID;
    LTRect			m_HelpRect;
	static HSURFACE		m_hHelpSurf;
	static HSURFACE		m_hTabSurf;

	CBitmapCtrl			*m_pUpArrow;
	CBitmapCtrl			*m_pDownArrow;
	CLTGUITextItemCtrl	*m_pBack;
	CLTGUITextItemCtrl	*m_pMain;
	CLTGUITextItemCtrl	*m_pContinue;
	

    LTIntPt      m_UpArrowPos;
    LTIntPt      m_DownArrowPos;

    static  LTBOOL      m_bReadLayout;
    static  LTIntPt     m_BackPos;
    static  LTIntPt     m_ContinuePos;
    static  LTIntPt     m_MainPos;
	static	char		m_sArrowBack[128];
	static	char		m_sArrowNext[128];
    static  LTIntPt     m_ArrowBackPos;
    static  LTIntPt     m_ArrowNextPos;
	
	static	char		m_sDefaultChangeSound[128];
	static	char		m_sDefaultSelectSound[128];
	
	char		m_sAmbientSound[128];
	
	LTIntPt		m_TabPos[MAX_TABS];
	LTIntPt		m_LinkPos[MAX_TABS];


	CBaseScaleFX	m_CharSFX;
	int				m_nNumAttachments;
	AttachmentData	m_aAttachment[MAX_INT_ATTACHMENTS];


	// Environment variables controlled during folder focus
	float			m_fFarZ;
	LTBOOL			m_bFogEnable;
	

	// Scale FX logo
	LTBOOL			m_bUseLogo;


	// Pasword input variables
	char			m_szPassword[MAX_MP_PASSWORD_LENGTH+2];

	CStaticTextCtrl	*m_pPWTitle;
	CLTGUIEditCtrl	*m_pPWPassword;
	CGroupCtrl		*m_pPWDlgGroup;
};


#endif // !defined(AFX_BASEFOLDER_H__88EE6E21_1515_11D3_B2DB_006097097C7B__INCLUDED_)
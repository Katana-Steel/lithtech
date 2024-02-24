// FolderCustomControls.h: interface for the CFolderCustomControls class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_CUSTOM_CONTROLS_H_
#define _FOLDER_CUSTOM_CONTROLS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"
#include "ClientUtilities.h"
#include "ProfileMgr.h"

const int kTrackBufferSize = 8;

class CFolderCustomControls : public CBaseFolder
{
public:

	CFolderCustomControls();
	virtual ~CFolderCustomControls();

	// Build the folder
    virtual LTBOOL  Build();
	virtual void  OnFocus(LTBOOL bFocus);
	virtual LTBOOL  Render(HSURFACE hDestSurf);

	// Handle input
    virtual LTBOOL   OnUp();
    virtual LTBOOL   OnDown();
    virtual LTBOOL   OnLeft();
    virtual LTBOOL   OnRight();
    virtual LTBOOL   OnEnter();
    virtual LTBOOL   OnTab();
    virtual LTBOOL   OnLButtonDown(int x, int y);
    virtual LTBOOL   OnLButtonUp(int x, int y);
    virtual LTBOOL   OnLButtonDblClick(int x, int y);
    virtual LTBOOL   OnRButtonDown(int x, int y);
    virtual LTBOOL   OnRButtonUp(int x, int y);
    virtual LTBOOL   OnRButtonDblClick(int x, int y);
    virtual LTBOOL   OnMouseMove(int x, int y);

	virtual LTBOOL	HandleKeyDown(int key, int rep);

	virtual void	Escape();

	virtual char*	GetSelectSound();

	LTBOOL	WaitingForKey()			{ return m_bWaitingForKey; }

protected:

    virtual uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	void	Update(HSURFACE hDestSurf);

	void	SetDummyStatus(uint32 nDelay, int nNextState);
	void	UpdateDummyStatus();

	void	SetUpdateControls(LTBOOL bEnabled);

	void	SetFilter(CommandType comType);
	void	BuildList();
	void	SetControlText(CLTGUIColumnTextCtrl *pCtrl);
    LTBOOL  SetCurrentSelection (DeviceInput* pInput);
	LTBOOL	CheckMouseWheel (DeviceInput* pInput,int nCommand,int nIndex);

	LTBOOL	KeyRemappable (DeviceInput* pInput);
	void	Bind(int com, uint16 diCode, char *lpszControlName, uint32 deviceType);
	void	UnBind(int newcom, char *lpszControlName, uint32 deviceType);
	void	UpdateAllControls();

	CommandType		m_filterType;
	uint32			m_filterFlag;
	CUserProfile*	m_pProfile;

	int 			m_nActionWidth;
	int 			m_nBufferWidth;
	int 			m_nEqualsWidth;
	int 			m_nBindingWidth;

    LTBOOL          m_bWaitingForKey;
    LTFLOAT         m_fInputPauseTimeLeft;
	DeviceInput		m_pInputArray[kTrackBufferSize];

	char			m_szTabSound[64];

	LTBOOL			m_bDummyState;
	uint32			m_nDummyTime;
	int				m_nNextDummyState;
	int				m_nTabStatus;

	CLTGUITextItemCtrl	*m_pMainTabCtrl;

	CLTGUITextItemCtrl	*m_pShared;
	CLTGUITextItemCtrl	*m_pMarine;
	CLTGUITextItemCtrl	*m_pPredator;
	CLTGUITextItemCtrl	*m_pAlien;
};

#endif // _FOLDER_CUSTOM_CONTROLS_H_
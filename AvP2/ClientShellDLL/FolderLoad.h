// FolderLoad.h: interface for the CFolderLoad class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_FOLDERLOAD_H_)
#define _FOLDERLOAD_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"
#include "ClientUtilities.h"


class CFolderLoad : public CBaseFolder
{
public:

	CFolderLoad();
	virtual ~CFolderLoad();

    LTBOOL Build();
    void OnFocus(LTBOOL bFocus);

	LTBOOL HandleKeyDown(int key, int rep);

protected:

    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void	BuildSavedLevelList();
	void	ClearSavedLevelList();
    virtual LTBOOL   OnLButtonUp(int x, int y);
    virtual LTBOOL   OnMouseMove(int x, int y);
    virtual LTBOOL   OnUp();
    virtual LTBOOL   OnDown();
    virtual LTBOOL   OnEnter();
	void	CheckSelection();

	CLTGUITextItemCtrl*		m_pQuick;
	CLTGUITextItemCtrl*		m_pBakQuick;
	CLTGUITextItemCtrl*		m_pReload;
	CLTGUITextItemCtrl*		m_pBakQuickName;
	CLTGUITextItemCtrl*		m_pQuickName;
	CLTGUITextItemCtrl*		m_pReloadName;
	CLTGUITextItemCtrl*		m_pLoad;
	CLTGUITextItemCtrl*		m_pDelete;
	CListCtrl*				m_pList;

	CLTGUIFont				m_TextFont;
};

#endif // _FOLDERLOAD_H_
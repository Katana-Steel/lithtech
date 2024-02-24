// FolderHostMaps.h: interface for the CFolderHostMaps class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_HOST_MAPS_H_
#define _FOLDER_HOST_MAPS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderHostMaps : public CBaseFolder
{
	public:

		CFolderHostMaps();
		virtual ~CFolderHostMaps();

		// Build the folder
		LTBOOL  Build();

		void	Escape();
		void	OnFocus(LTBOOL bFocus);
		void	FocusSetup(LTBOOL bFocus);


	protected:

		uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
		virtual LTBOOL   OnLButtonUp(int x, int y);
		virtual LTBOOL   OnMouseMove(int x, int y);
		virtual LTBOOL   OnUp();
		virtual LTBOOL   OnDown();
		virtual LTBOOL   OnEnter();
		void	CheckSelection();

		void	AppendToMapList(char *szDir);
		void	FillLevelList(CListCtrl *pList);
		void	LoadLevelList(CListCtrl *pList);

		CListCtrl	*m_pAvailList;
		CListCtrl	*m_pSelectList;

		CLTGUITextItemCtrl *m_pAdd;
		CLTGUITextItemCtrl *m_pRemove;
		CLTGUITextItemCtrl *m_pAddAll;
		CLTGUITextItemCtrl *m_pRemoveAll;

		StringList	m_slMaps;
};

#endif // _FOLDER_HOST_MAPS_H_

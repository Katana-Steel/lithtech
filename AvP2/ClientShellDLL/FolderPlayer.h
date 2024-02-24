//////////////////////////////////////////////////////////////////////
//
// FolderPlayer.h: interface for the CFolderPlayer class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_PLAYER_H_
#define _FOLDER_PLAYER_H_

//////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////

#include "BaseFolder.h"

//////////////////////////////////////////////////////////////////////

class CFolderPlayer : public CBaseFolder
{
	public:

		CFolderPlayer();
		virtual ~CFolderPlayer();

		// Build the folder
		LTBOOL	Build();
		void	OnFocus(LTBOOL bFocus);
		void	FocusSetup(LTBOOL bFocus);
		void	Escape();
		LTBOOL	OnEnter();
		void	Term();

		LTBOOL	OnLButtonUp(int x, int y);
		LTBOOL	OnRButtonUp(int x, int y);

	protected:

		uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

		void	BuildClassLists();
		void	UpdateCurrentSelection();

		void	CreatePlayerModel();
		void	UpdateDescriptions();


		// Folder controls
		CGroupCtrl			*m_pNameGroup;
		CLTGUIEditCtrl		*m_pEdit;
		CLTGUITextItemCtrl	*m_pLabel;

		CStaticTextCtrl		*m_pTeam;
		CStaticTextCtrl		*m_pClass;


		// Temp variables for this folder
		char	m_szPlayerName[21];
		char	m_szOldName[21];
		int		m_nRace;
		int		m_nChar[4];


		// The available players to choose from
		int		m_nCharList[4][32];
		int		m_nCharCount[4];


		// The player description strings
		CStaticTextCtrl		*m_pDescription;
};

//////////////////////////////////////////////////////////////////////

#endif // _FOLDER_PLAYER_H_


// FolderGame.h: interface for the CFolderGame class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_GAME_H_
#define _FOLDER_GAME_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderGame : public CBaseFolder
{
	public:

		CFolderGame();
		virtual ~CFolderGame();

		// Build the folder
		LTBOOL	Build();
		void	OnFocus(LTBOOL bFocus);
		void	FocusSetup(LTBOOL bFocus);

	protected:

		uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
		void	BuildControlList();

	protected:

		CLTGUITextItemCtrl *m_pGeneral;
		CLTGUITextItemCtrl *m_pMultiplayer;

		CToggleCtrl* m_pObjToggle;

		int		m_nPickupMsgDur;

		LTBOOL	m_bHint;
		LTBOOL	m_bAlwaysRun;
		LTBOOL	m_bSubtitles;
		LTBOOL	m_bObjectives;
		LTBOOL	m_bHeadBob;
		LTBOOL	m_bHeadCanting;
		LTBOOL	m_bWeaponSway;
		LTBOOL	m_bOrientOverlay;
		LTBOOL	m_bAutoswitch;

		int		m_nMessageDisplay;
		int		m_nMessageTime;
		int		m_nConnection;

		LTBOOL	m_bIgnoreTaunts;
};

#endif // _FOLDER_GAME_H_

//////////////////////////////////////////////////////////////////////
//
// FolderSetupOverrun.h
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_SETUP_OVERRUN_H_
#define _FOLDER_SETUP_OVERRUN_H_

//////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////

#include "BaseFolder.h"

//////////////////////////////////////////////////////////////////////

class CFolderSetupOverrun : public CBaseFolder
{
	public:

		CFolderSetupOverrun();
		virtual ~CFolderSetupOverrun();

		// Build the folder
		LTBOOL	Build();
		void	OnFocus(LTBOOL bFocus);
		void	FocusSetup(LTBOOL bFocus);
		void	Escape();

	protected:

		uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);


	protected:

		// Folder controls
		int		m_nMaxPlayers;
		int		m_nRoundLimit;
		int		m_nTimeLimit;

		int		m_nDefenderRace;
		int		m_nDefenderLives;

		int		m_nAttackerRace;
		int		m_nAttackerLives;
};

//////////////////////////////////////////////////////////////////////

#endif


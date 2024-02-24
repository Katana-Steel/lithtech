//////////////////////////////////////////////////////////////////////
//
// FolderSetupSurvivor.h
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_SETUP_SURVIVOR_H_
#define _FOLDER_SETUP_SURVIVOR_H_

//////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////

#include "BaseFolder.h"

//////////////////////////////////////////////////////////////////////

class CFolderSetupSurvivor : public CBaseFolder
{
	public:

		CFolderSetupSurvivor();
		virtual ~CFolderSetupSurvivor();

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
		int		m_nScoreLimit;
		int		m_nRoundLimit;
		int		m_nTimeLimit;

		int		m_nSurvivorRace;
		int		m_nMutateRace;
};

//////////////////////////////////////////////////////////////////////

#endif


//////////////////////////////////////////////////////////////////////
//
// FolderSetupDM.h
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_SETUP_DM_H_
#define _FOLDER_SETUP_DM_H_

//////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////

#include "BaseFolder.h"

//////////////////////////////////////////////////////////////////////

class CFolderSetupDM : public CBaseFolder
{
	public:

		CFolderSetupDM();
		virtual ~CFolderSetupDM();

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
		int		m_nMaxTeams[4];
		int		m_nFragLimit;
		int		m_nScoreLimit;
		int		m_nTimeLimit;
		LTBOOL	m_bLifecycle;
};

//////////////////////////////////////////////////////////////////////

#endif


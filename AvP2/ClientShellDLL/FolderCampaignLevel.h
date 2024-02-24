// FolderCampaignLevel.h: interface for the CFolderCustomLevel class.
//
//////////////////////////////////////////////////////////////////////

#ifndef FOLDER_CAMPAIGN_LEVEL_H
#define FOLDER_CAMPAIGN_LEVEL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

#include <vector>
#include <string>

class CFolderCampaignLevel : public CBaseFolder
{
public:
	CFolderCampaignLevel(Species);
	virtual ~CFolderCampaignLevel();

	// Build the folder
    LTBOOL   Build();

    void    OnFocus(LTBOOL bFocus);

	virtual char*	GetSelectSound();
	virtual char*	GetChangeSound();

	// These are intended for use by CheatMgr to make all missions available
	virtual void	SetShowMissions(LTBOOL bShow) { m_bShowAllMissions = bShow; }
	virtual LTBOOL	GetShowMissions() const { return m_bShowAllMissions; }

private:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void	FillMissionList();
	void	ClearMissionList();

	Species		m_Species;
	CListCtrl*	m_pListCtrl;
	CListCtrl*	m_pDiffCtrl;
	CLTGUITextItemCtrl* m_pLaunchCtrl;

	char		m_szLaunchSound[64];	
	char		m_szListSound[64];
	LTBOOL		m_bShowAllMissions;
};

#endif // FOLDER_CAMPAIGN_LEVEL_H
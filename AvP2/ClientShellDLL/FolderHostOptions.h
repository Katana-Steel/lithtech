// FolderHostOptions.h: interface for the CFolderHostOptions class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_HOST_OPTS_H_
#define _FOLDER_HOST_OPTS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"
#include "ServerOptionMgr.h"

class CFolderHostOptions : public CBaseFolder
{
public:

	CFolderHostOptions();
	virtual ~CFolderHostOptions();

	// Build the folder
    LTBOOL  Build();

	void	Escape();
	void	OnFocus(LTBOOL bFocus);
	void	FocusSetup(LTBOOL bFocus);

	LTBOOL	OnEnter();

	LTBOOL	OnRButtonUp(int x, int y);
	LTBOOL	OnLButtonUp(int x, int y);

protected:

    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void	BuildControlList();

	void	UpdateBandwidth();
	LTBOOL	IsValidBandwidth(uint32 nBandwidth);

protected:

	CLTGUITextItemCtrl *m_pPreferences;
	CLTGUITextItemCtrl *m_pScoring;

	CServerOptions		m_Options;

	CGroupCtrl			*m_pPortGroup;
	CLTGUIEditCtrl		*m_pPortEdit;
	CLTGUITextItemCtrl	*m_pPortLabel;

	char				m_szPort[8];
	char				m_szOldPort[8];

	CCycleCtrl			*m_pBandwidthCycle;
	CGroupCtrl			*m_pBandwidthGroup;
	CLTGUIEditCtrl		*m_pBandwidthEdit;
	CLTGUITextItemCtrl	*m_pBandwidthLabel;

	int					m_nBandwidth;
	char				m_szBandwidth[16];
	char				m_szOldBandwidth[16];

	int	m_nSpeed;
	int	m_nRespawn;
	int	m_nDamage;
	int	m_nExosuit;
	int	m_nQueenMolt;

	int m_nScoring[MAX_MP_SCORING_CLASSES];
};

inline LTBOOL CFolderHostOptions::IsValidBandwidth(uint32 nBandwidth) 
{
	return (nBandwidth >= 1000 && nBandwidth <= 1000000);
}

#endif // _FOLDER_HOST_OPTS_H_
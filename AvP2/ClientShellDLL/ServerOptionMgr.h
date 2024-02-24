// ServerOptionMgr.h: interface for the CServerOptionMgr class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SERVEROPT_MGR_H
#define SERVEROPT_MGR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ClientUtilities.h"
#include "ButeMgr.h"
#include "MultiplayerMgrDefs.h"
#include "ServerOptions.h"

#pragma warning (disable : 4503)
#include <vector>

class CServerOptionMgr
{
public:

	LTBOOL	Init();
	void	Term();

	CServerOptions *GetCurrentCfg();
	void			SetCurrentCfg(const char *szName, LTBOOL bCreate = LTTRUE);
	void			DeleteCfg(const char *szName);
	void			RenameCfg(const char *oldName,const char *newName);

	void			SaveCurrentCfg();

	uint32	GetNumScoringClasses() {return m_nNumScoringClasses;};
	uint32	GetScoringClass(uint8 i) {return (i < m_nNumScoringClasses) ? m_nScoringClass[i] : 0;}

	void	GetConfigNames(StringSet& configNames);

protected:
	void	BuildScoringClassInfo();
	void	BuildCfgList();
	void	SetCurrentCfg(uint16 nCfg);
	void	DeleteCfg(uint16 nCfg);
	CServerOptions* CreateCfg(const char *szName);

	typedef std::vector<CServerOptions *> ConfigList;
	ConfigList	m_ConfigList;

	uint16	m_nCurrentCfg;
	uint32	m_nNumScoringClasses;
	uint32	m_nScoringClass[MAX_MP_SCORING_CLASSES];

	CButeMgr	m_buteMgr;
};

extern CServerOptionMgr* g_pServerOptionMgr;



#endif	// SERVEROPT_MGR_H

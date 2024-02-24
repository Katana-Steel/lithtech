// PerformanceMgr.h: interface for the CPerformanceMgr class.
//
//////////////////////////////////////////////////////////////////////

#ifndef PERFORM_MGR_H
#define PERFORM_MGR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ClientUtilities.h"
#include "ButeMgr.h"
#pragma warning (disable : 4503)
#include <vector>

class CPerformanceMgr;
extern CPerformanceMgr* g_pPerformanceMgr;


const int kNumGroups = 10;
typedef struct sDetailSetting_t
{
	char		szVar[32];
	char		szName[32];
	int			nSetting[5];
} sDetailSetting;

const int kNumSettings = 16;
typedef struct sConfig_t
{
	char		szName[64];
	char		szNiceName[64];
	int			nSettings[kNumSettings];
	int			nDetails[kNumGroups];
} sConfig;



class CPerformanceMgr
{
public:

	LTBOOL	Init();
	void	Term();


	int		GetPerformanceCfg();

	void	SetPerformanceCfg(char *szName);
	void	SetPerformanceCfg(int nCfg);

	typedef std::vector<sConfig *>	ConfigList;
	ConfigList	m_ConfigList;

	int		GetDetailLevel(const int* pOffsetArray, int nGroups = kNumGroups);
	void	SetDetailLevels(int nLevel, int* pOffsetArray);

protected:
	void	BuildConfigList();
	LTBOOL	IsCurrentConfig(int nCfg);


	CButeMgr	m_buteMgr;


};


#endif	// PERFORM_MGR_H

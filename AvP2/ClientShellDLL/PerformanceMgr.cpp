#include "stdafx.h"

#include "PerformanceMgr.h"
#include "interfacemgr.h"
#include "ClientRes.h"
#include "GameClientShell.h"

#include <iostream>			// For input and output
#include <fstream>			// For the files
#include <IO.h>				// Find first, find next, etc.


namespace
{
	const std::string CONFIG_DIR("Config\\");
	const std::string CONFIG_EXT(".txt");

	int g_nTriple = 99;
	int g_nDetail = 99;


	sDetailSetting sDetailGroups[kNumGroups] =
	{
		"GroupOffset0",	"WorldTex",		2,	1,	0,	0,	-1,
		"GroupOffset1",	"SkyTex",		2,	1,	0,	0,	-1,
		"GroupOffset2",	"PropTex",		2,	0,	-1,	-1,	-1,
		"GroupOffset3",	"WeaponTex",	2,	1,	1,	0,	-1,
		"GroupOffset4",	"CharTex",		2,	1,	1,	0,	-1,
		"GroupOffset5",	"SFXTex",		2,	2,	1,	0,	-1,
		"GroupOffset6",	"EnvTex",		0,	0,	-1,	-1,	-1,
		"GroupOffset7",	"MiscTex1",		0,	0,	-1,	-1,	-1,
		"GroupOffset8",	"MiscTex2",		0,	0,	-1,	-1,	-1,
		"GroupOffset9",	"SignTex",		0,	0,	-1,	-1,	-1,
	};

	char szSettings[kNumSettings][64] =
	{
		"FogPerformanceScale",
		"32BitTextures",
		"32BitLightMaps",
		"Shadows",
		"DetailTextures",
		"WorldEnvironmentMap",
		"ModelEnvironmentMap",
		"Trilinear",
		"TripleBuffer",
		"ShellCasings",
		"MuzzleLight",
		"ImpactFXLevel",
		"DebrisFXLevel",
		"DetailLevel",
		"EnableSky",
		"DrawViewModel",
	};

	char szVars[kNumSettings][64] =
	{
		"FogPerformanceScale",
		"32BitTextures",
		"32BitLightMaps",
		"MaxModelShadows",
		"DetailTextures",
		"EnvMapWorld",
		"EnvMapEnable",
		"Trilinear",
		"TripleBuffer",
		"ShellCasings",
		"MuzzleLight",
		"ImpactFXLevel",
		"DebrisFXLevel",
		"DetailLevel",
		"EnableSky",
		"DrawViewModel",
	};


	sConfig cfgDefaultExtra =
	{
		".DefaultExtraLow",
		"",
		100,//"FogPerformanceScale"
		1, //"32BitTextures",
		1, //"32BitLightMaps",
		1, //"MaxModelShadows",
		1, //"DetailTextures",
		1, //"EnvMapWorld",
		1, //"EnvMapEnable",
		1, //"Trilinear",
		1, //"TripleBuffer",
		1, //"ShellCasings",
		1, //"MuzzleLight",
		2, //"ImpactFXLevel",
		2, //"DebrisFXLevel",
		4, //"DetailLevel",
		1, //"EnableSky",
		1, //"DrawViewModel",
	};

	sConfig cfgDefaultLow =
	{
		".DefaultLow",
		"",
		100,//"FogPerformanceScale"
		1, //"32BitTextures",
		1, //"32BitLightMaps",
		1, //"MaxModelShadows",
		1, //"DetailTextures",
		1, //"EnvMapWorld",
		1, //"EnvMapEnable",
		1, //"Trilinear",
		1, //"TripleBuffer",
		1, //"ShellCasings",
		1, //"MuzzleLight",
		2, //"ImpactFXLevel",
		2, //"DebrisFXLevel",
		3, //"DetailLevel",
		1, //"EnableSky",
		1, //"DrawViewModel",
	};

	sConfig cfgDefaultMid =
	{
		".DefaultMid",
		"",
		60,//"FogPerformanceScale"
		1, //"32BitTextures",
		0, //"32BitLightMaps",
		1, //"MaxModelShadows",
		1, //"DetailTextures",
		0, //"EnvMapWorld",
		1, //"EnvMapEnable",
		0, //"Trilinear",
		0, //"TripleBuffer",
		1, //"ShellCasings",
		1, //"MuzzleLight",
		1, //"ImpactFXLevel",
		1, //"DebrisFXLevel",
		2, //"DetailLevel",
		1, //"EnableSky",
		1, //"DrawViewModel",
	};

	sConfig cfgDefaultHigh =
	{
		".DefaultHigh",
		"",
		20,//"FogPerformanceScale"
		0, //"32BitTextures",
		0, //"32BitLightMaps",
		0, //"MaxModelShadows",
		0, //"DetailTextures",
		0, //"EnvMapWorld",
		0, //"EnvMapEnable",
		0, //"Trilinear",
		0, //"TripleBuffer",
		0, //"ShellCasings",
		0, //"MuzzleLight",
		0, //"ImpactFXLevel",
		0, //"DebrisFXLevel",
		1, //"DetailLevel",
		1, //"EnableSky",
		1, //"DrawViewModel",
	};

	sConfig cfgDefaultExtraHigh =
	{
		".DefaultExtraHigh",
		"",
		20,//"FogPerformanceScale"
		0, //"32BitTextures",
		0, //"32BitLightMaps",
		0, //"MaxModelShadows",
		0, //"DetailTextures",
		0, //"EnvMapWorld",
		0, //"EnvMapEnable",
		0, //"Trilinear",
		0, //"TripleBuffer",
		0, //"ShellCasings",
		0, //"MuzzleLight",
		0, //"ImpactFXLevel",
		0, //"DebrisFXLevel",
		0, //"DetailLevel",
		0, //"EnableSky",
		0, //"DrawViewModel",
	};
}

CPerformanceMgr* g_pPerformanceMgr = NULL;

LTBOOL CPerformanceMgr::Init()
{
	g_pPerformanceMgr = this;

	g_nTriple = 0;
	while (g_nTriple < kNumSettings && stricmp("TripleBuffer",szVars[g_nTriple]) != 0)
		g_nTriple++;

	g_nDetail = 0;
	while (g_nDetail < kNumSettings && stricmp("DetailLevel",szVars[g_nDetail]) != 0)
		g_nDetail++;

	HSTRING hTemp = g_pLTClient->FormatString(IDS_PERFORMANCE_EXTRA_HIGH);
	SAFE_STRCPY(cfgDefaultExtraHigh.szNiceName,g_pLTClient->GetStringData(hTemp));
	
	hTemp = g_pLTClient->FormatString(IDS_PERFORMANCE_HIGH);
	SAFE_STRCPY(cfgDefaultHigh.szNiceName,g_pLTClient->GetStringData(hTemp));

	hTemp = g_pLTClient->FormatString(IDS_PERFORMANCE_MID);
	SAFE_STRCPY(cfgDefaultMid.szNiceName,g_pLTClient->GetStringData(hTemp));

	hTemp = g_pLTClient->FormatString(IDS_PERFORMANCE_LOW);
	SAFE_STRCPY(cfgDefaultLow.szNiceName,g_pLTClient->GetStringData(hTemp));

	hTemp = g_pLTClient->FormatString(IDS_PERFORMANCE_EXTRA);
	SAFE_STRCPY(cfgDefaultExtra.szNiceName,g_pLTClient->GetStringData(hTemp));


	char szCfg[64];
	GetConsoleString("PerformanceConfig",szCfg,".DefaultLow");

	BuildConfigList();

	SetPerformanceCfg(szCfg);

	return LTTRUE;
}

void CPerformanceMgr::Term()
{
	unsigned int i;
	for (i=0; i < m_ConfigList.size(); i++)
	{
		delete m_ConfigList[i];
	}
	m_ConfigList.clear();

	g_pPerformanceMgr = NULL;
}


int CPerformanceMgr::GetPerformanceCfg()
{
	uint32 nCfg = 0;

	while (nCfg < m_ConfigList.size() && !IsCurrentConfig(nCfg))
		nCfg++;

	if (nCfg >= m_ConfigList.size())
		return -1;
	return nCfg;
}

void CPerformanceMgr::SetPerformanceCfg(char *szName)
{
	uint32 i = 0;

	while (i < m_ConfigList.size() && stricmp(m_ConfigList[i]->szName,szName) != 0)
		i++;

	if (i >= m_ConfigList.size()) 
		i = -1;

	SetPerformanceCfg(i);
}

void CPerformanceMgr::SetPerformanceCfg(int nCfg)
{

	if (nCfg < 0 || (uint32)nCfg >= m_ConfigList.size())
	{
		WriteConsoleString("PerformanceConfig",".CustomConfig");
		g_pLTClient->WriteConfigFile("autoexec.cfg");
		return;
	}

	sConfig *pCfg = m_ConfigList[nCfg];

	for (int i = 0; i < kNumSettings; i++)
	{
		WriteConsoleInt(szVars[i],pCfg->nSettings[i]);
	}

	uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();
	//disable triple buffer 
	if (g_nTriple < kNumSettings && !(dwAdvancedOptions & AO_TRIPLEBUFFER))
	{
		WriteConsoleInt(szVars[g_nTriple],0);
	}

	int nDetail = pCfg->nSettings[g_nDetail];
	if ( nDetail >= 0 && nDetail < 5)
	{
		if (g_nDetail < kNumSettings)
		{
			for (int grp = 0; grp < kNumGroups; grp++)
			{
				WriteConsoleInt(sDetailGroups[grp].szVar,sDetailGroups[grp].nSetting[nDetail]);
			}
		}
	}
	else
	{
		for (int grp = 0; grp < kNumGroups; grp++)
		{
			WriteConsoleInt(sDetailGroups[grp].szVar,pCfg->nDetails[grp]);
		}
	}

	WriteConsoleString("PerformanceConfig",pCfg->szName);

	g_pLTClient->WriteConfigFile("autoexec.cfg");

	if(g_pGameClientShell->GetVisionModeMgr())
		g_pGameClientShell->GetVisionModeMgr()->ResetEnvMap();
}

void CPerformanceMgr::BuildConfigList()
{
	unsigned int i;

	if (m_ConfigList.size() >= 3)
	{
		m_ConfigList.erase(m_ConfigList.begin());
		m_ConfigList.erase(m_ConfigList.begin());
		m_ConfigList.erase(m_ConfigList.begin());
	};
	for (i=0; i < m_ConfigList.size(); i++)
	{
		delete m_ConfigList[i];
	}
	m_ConfigList.clear();

	m_ConfigList.push_back(&cfgDefaultExtraHigh);
	m_ConfigList.push_back(&cfgDefaultHigh);
	m_ConfigList.push_back(&cfgDefaultMid);
	m_ConfigList.push_back(&cfgDefaultLow);
	m_ConfigList.push_back(&cfgDefaultExtra);

	struct _finddata_t file;
	long hFile;

	std::string directory = CONFIG_DIR + "*" + CONFIG_EXT;

	// find first file
	if((hFile = _findfirst(directory.c_str(), &file)) != -1L)
	{
		do
		{
			std::string fn = CONFIG_DIR + file.name;

			m_buteMgr.Term();
			if (m_buteMgr.Parse(fn.c_str()))
			{
				sConfig* pCfg = new sConfig;
				char *pName = strtok(file.name,".");
				SAFE_STRCPY(pCfg->szName,pName);

				std::string str = m_buteMgr.GetString("General","Name");
				SAFE_STRCPY(pCfg->szNiceName,str.c_str());

				for (int i = 0; i < kNumSettings; i++)
				{
					pCfg->nSettings[i] = m_buteMgr.GetInt("Settings",szSettings[i], 0);
				}

				int nDetail = pCfg->nSettings[g_nDetail];
				if ( nDetail >= 0 && nDetail < 5)
				{
					for (int grp = 0; grp < kNumGroups; grp++)
					{
						pCfg->nDetails[grp] = sDetailGroups[grp].nSetting[nDetail];
					}

				}
				else
				{
					for (int grp = 0; grp < kNumGroups; grp++)
					{
						pCfg->nDetails[grp] = m_buteMgr.GetInt("Details",sDetailGroups[grp].szName, 0);
					}
				}

				m_ConfigList.push_back(pCfg);
			}
		}
		while(_findnext(hFile, &file) == 0);
	}
	_findclose(hFile);
		

}


LTBOOL	CPerformanceMgr::IsCurrentConfig(int nCfg)
{
	uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();

	if (nCfg < 0 || (uint32)nCfg >= m_ConfigList.size())
	{
		return LTFALSE;
	}

	sConfig *pCfg = m_ConfigList[nCfg];

	for (int i = 0; i < kNumSettings; i++)
	{
		if (g_nTriple == i && !(dwAdvancedOptions & AO_TRIPLEBUFFER))
			continue;
		int n = GetConsoleInt(szVars[i],0);
		if (n != pCfg->nSettings[i])
			return LTFALSE;
	}

	int nDetail = pCfg->nSettings[g_nDetail];
	if ( nDetail < 0 || nDetail >= 5)
	{
		for (int grp = 0; grp < kNumGroups; grp++)
		{
			if (pCfg->nDetails[grp] != GetConsoleInt(sDetailGroups[grp].szVar,0) )
				return LTFALSE;
		}
	}

	return LTTRUE;


}

int CPerformanceMgr::GetDetailLevel(const int* pOffsetArray, int nGroups)
{
	if(nGroups > kNumGroups)
		nGroups = kNumGroups;

	for (int level = 0; level < 5; level++)
	{
		for (int grp = 0; grp < nGroups; grp++)
		{
			if (pOffsetArray[grp] != sDetailGroups[grp].nSetting[level])
				break;
		}

		if (grp >= nGroups)
			break;
	}

	return level;
}

void CPerformanceMgr::SetDetailLevels(int nLevel, int* pOffsetArray)
{
	if (nLevel < 0 || nLevel >= 5) return;

	for (int grp = 0; grp < kNumGroups; grp++)
	{
		pOffsetArray[grp] = sDetailGroups[grp].nSetting[nLevel];
	}
}
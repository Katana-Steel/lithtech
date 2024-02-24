// ----------------------------------------------------------------------------------------------
// 
// ServerOptionsMgr.cpp
// 
// ----------------------------------------------------------------------------------------------


#include "stdafx.h"

#include "ServerOptionMgr.h"
#include "ServerOptions.h"
#include "interfacemgr.h"
#include "ClientRes.h"
#include "GameType.h"
#include "WinUtil.h"
#include "CharacterButeMgr.h"
#include "MultiplayerMgrDefs.h"

#include <iostream>			// For input and output
#include <fstream>			// For the files
#include <IO.h>				// Find first, find next, etc.

// ----------------------------------------------------------------------------------------------

namespace
{
	const std::string CONFIG_DIR( SERVER_OPTIONS_CONFIG_DIRECTORY );
	const std::string CONFIG_EXT( SERVER_OPTIONS_CONFIG_EXTENSION );

}

CServerOptionMgr* g_pServerOptionMgr = NULL;

LTBOOL CServerOptionMgr::Init()
{
	g_pServerOptionMgr = this;

	BuildScoringClassInfo();

	if (!CWinUtil::DirExist("ServerData"))
	{
		CWinUtil::CreateDir("ServerData");
	}

	m_nCurrentCfg = (uint16)-1;


	HSTRING hUnnamed = g_pLTClient->FormatString(IDS_UNNAMED);

	char szCfg[64];
	GetConsoleString("ServerConfig",szCfg,g_pLTClient->GetStringData(hUnnamed));

	BuildCfgList();
	SetCurrentCfg(szCfg,LTFALSE);

	if (m_nCurrentCfg >= m_ConfigList.size())
		SetCurrentCfg(g_pLTClient->GetStringData(hUnnamed),LTTRUE);

	g_pLTClient->FreeString(hUnnamed);

	return LTTRUE;
}

void CServerOptionMgr::Term()
{
	uint16 i;
	for (i=0; i < m_ConfigList.size(); i++)
	{
		delete m_ConfigList[i];
	}
	m_ConfigList.clear();

	g_pServerOptionMgr = NULL;
}

CServerOptions *CServerOptionMgr::GetCurrentCfg()
{
	if (m_nCurrentCfg >= m_ConfigList.size())
		return LTNULL;
	return m_ConfigList[m_nCurrentCfg];
}

void CServerOptionMgr::SetCurrentCfg(const char *szName, LTBOOL bCreate)
{
	if (!szName || strlen(szName) == 0)
		return;

	uint16 i = 0;

	while (i < m_ConfigList.size() && stricmp(m_ConfigList[i]->szCfgName,szName) != 0)
		i++;

	
	if (bCreate && i >= m_ConfigList.size())
	{
		CServerOptions *pNew = CreateCfg(szName);
		i = m_ConfigList.size()-1;
	}

	if (i < m_ConfigList.size())
		SetCurrentCfg(i);
}

void CServerOptionMgr::SetCurrentCfg(uint16 nCfg)
{
	SaveCurrentCfg();
	if (nCfg < m_ConfigList.size())
	{
		m_nCurrentCfg = nCfg;
		WriteConsoleString("ServerConfig",m_ConfigList[m_nCurrentCfg]->szCfgName);
	}
}

void CServerOptionMgr::SaveCurrentCfg()
{
	// Make sure the current config is valid.
	if( m_nCurrentCfg >= m_ConfigList.size( ))
		return;

	// Get the current config bute.
	CServerOptions* pCurrentCfg = m_ConfigList[m_nCurrentCfg];

	// Save the config to the butemgr object.
	pCurrentCfg->SaveToBute( m_buteMgr );

	// Save the attributes...
	std::string fn = CONFIG_DIR + pCurrentCfg->szCfgName + CONFIG_EXT;
	m_buteMgr.Save( fn.c_str( ));

	// Reset the bute mgr so it doesn't make multiple copies of the settings in the file
	m_buteMgr.Term();
	m_buteMgr.Init();
	m_buteMgr.Parse(fn.c_str());
}


void CServerOptionMgr::BuildCfgList()
{

	unsigned int i;

	for (i=0; i < m_ConfigList.size(); i++)
	{
		delete m_ConfigList[i];
	}
	m_ConfigList.clear();


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
			m_buteMgr.Init();
			if (m_buteMgr.Parse(fn.c_str()))
			{
				CServerOptions* pCfg = new CServerOptions;
				char *pName = strtok(file.name,".");
				SAFE_STRCPY(pCfg->szCfgName,pName);

				pCfg->LoadFromBute(m_buteMgr);

				m_ConfigList.push_back(pCfg);
			}
		}
		while(_findnext(hFile, &file) == 0);
	}
	_findclose(hFile);

	if (m_ConfigList.size() == 0)
	{
		CServerOptions* pCfg = new CServerOptions;
		m_ConfigList.push_back(pCfg);
	}

}

CServerOptions* CServerOptionMgr::CreateCfg(const char *szName)
{
	CServerOptions* pNew = new CServerOptions;

	SAFE_STRCPY(pNew->szCfgName,szName);

	m_ConfigList.push_back(pNew);

	pNew->SaveToBute(m_buteMgr);

	return pNew;
}



void CServerOptionMgr::BuildScoringClassInfo()
{
	uint32 nClasses = 0;

	int nSets = g_pCharacterButeMgr->GetNumSets();
	for (uint8 i = 0; i < nSets; i++)
	{
		uint32 nClass = g_pCharacterButeMgr->GetScoringClass(i);
		uint32 nTest = 0; 
		while (nTest < nClasses && nClass != m_nScoringClass[nTest])
			nTest++;

		if (nTest == nClasses && nClass != 0)
		{
			m_nScoringClass[nClasses] = nClass;
			nClasses++;
		}
	}

	m_nNumScoringClasses = nClasses;


	while (nClasses < MAX_MP_SCORING_CLASSES)
	{
		m_nScoringClass[nClasses] = 0;

		nClasses++;
	}

	
}

void CServerOptionMgr::GetConfigNames(StringSet& configNames)
{
	configNames.clear();
	unsigned int i;

	for (i=0; i < m_ConfigList.size(); i++)
	{
		if (m_ConfigList[i]->szCfgName[0] && m_ConfigList[i]->szCfgName[0] != '.')
			configNames.insert( m_ConfigList[i]->szCfgName );
	}
}


void CServerOptionMgr::DeleteCfg(const char *szName)
{

	LTBOOL bFound = LTFALSE;
	ConfigList::iterator iter = m_ConfigList.begin();

	while (iter != m_ConfigList.end() && !bFound)
	{
		bFound = (stricmp((*iter)->szCfgName,szName) == 0);
		if (!bFound)
			iter++;
	}

	if (!bFound) return;

	LTBOOL bNeedNew = ((*iter) == m_ConfigList[m_nCurrentCfg]);

	char szCurName[MAX_MP_CFG_NAME_LENGTH];
	SAFE_STRCPY(szCurName,m_ConfigList[m_nCurrentCfg]->szCfgName);

	CServerOptions *pCfg = (*iter);
	m_ConfigList.erase(iter);
	delete pCfg;


	std::string fn = CONFIG_DIR + szName + CONFIG_EXT;
	remove(fn.c_str());

	if (bNeedNew)
	{

		if (m_ConfigList.size() > 1)
		{
			uint16 nCfg = 1;
			SetCurrentCfg(nCfg);
		}
		else
		{
			HSTRING hUnnamed = g_pLTClient->FormatString(IDS_UNNAMED);

			//empty list
			SetCurrentCfg(g_pLTClient->GetStringData(hUnnamed),LTTRUE);

			g_pLTClient->FreeString(hUnnamed);
		}
	}
	else
	{
		SetCurrentCfg(szCurName);
	}

}


void CServerOptionMgr::RenameCfg(const char *oldName,const char *newName)
{
	
	
	std::string ofn = CONFIG_DIR + oldName + CONFIG_EXT;
	std::string nfn = CONFIG_DIR + newName + CONFIG_EXT;
	rename(ofn.c_str(),nfn.c_str());


	//find and rename CFG in list
	uint16 i = 0;
	while (i < m_ConfigList.size() && stricmp(m_ConfigList[i]->szCfgName,oldName) != 0)
		i++;

	

	if (i < m_ConfigList.size())
	{
		SAFE_STRCPY(m_ConfigList[i]->szCfgName,newName);
	}

}

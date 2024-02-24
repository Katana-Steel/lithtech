
#ifndef __SAVEGAMEDATA_H
#define __SAVEGAMEDATA_H


#include "ClientServerShared.h"

class CLTGUIColumnTextCtrl;
class CLTGUITextItemCtrl;

const int g_nLoadSaveTextWidth = 6;

struct CSaveGameData
{
	CSaveGameData() {szWorldName[0] = szTime[0] = LTNULL; nPrevMission = 0; nTime = 0; eSpecies = Marine;}
	void ParseSaveString(char *pszString);
	void BuildSaveString(char *pszString);

	char	szWorldName[128];
	char	szDate[64];
	char	szTime[64];
	int		nPrevMission;
	Species eSpecies;
	int		nDifficulty;
	long	nTime;

};


uint32 BuildSaveGameEntry( CLTGUIColumnTextCtrl* pCtrl, const CSaveGameData & saveData, 
				   int nMissionWidth, int nDateWidth, int nTimeWidth, int nDifficultyWidth );

void SetSaveGameText( CLTGUITextItemCtrl* pCtrl, const CSaveGameData & saveData);


void FillSavedGameData( CSaveGameData * pData);

#endif //__SAVEGAMEDATA_H
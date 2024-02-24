
#include "stdafx.h"
#include "SaveGameData.h"
#include "GameType.h"
#include "InterfaceMgr.h"
#include "ClientRes.h"
#include "MissionMgr.h"
#include "GameClientShell.h"

extern HLTCOLOR * g_clrSpecies;

#include <stdlib.h>
#include <time.h>

static HLTCOLOR g_clrNonSelSpeciesTextColor[5] =
{
	SETRGB_T(192, 16, 0),		// Alien
	SETRGB_T(64, 64, 192),	// Marine
	SETRGB_T(0, 192, 0),		// Predator
	SETRGB_T(192, 32, 32),		// Corporate
	SETRGB_T(192,192,192)      // Unknown
};

static HLTCOLOR g_clrSelSpeciesTextColor[5] =
{
	SETRGB_T(255, 32, 0),		// Alien
	SETRGB_T(128, 128, 255),	// Marine
	SETRGB_T(0, 255, 0),		// Predator
	SETRGB_T(255, 96, 96),		// Corporate
	SETRGB_T(255,255,255)      // Unknown
};

void CSaveGameData::ParseSaveString(char* pszSaveStr)
{
	if (!pszSaveStr || strlen (pszSaveStr) == 0) return;

	char* pWorldName = strtok(pszSaveStr,"|");
	char* pMission = strtok(NULL,"|");
	char* pSpecies = strtok(NULL,"|");
	char* pDifficulty = strtok(NULL,"|");
	char* pTimeStr = strtok(NULL,"|");

	SAFE_STRCPY(szWorldName,pWorldName);

	if( pMission && strlen(pMission) )
		nPrevMission = atoi(pMission);
	else
		nPrevMission = -1;

	if( pSpecies && strlen(pSpecies) )
		eSpecies = (Species)atoi(pSpecies);
	else
		eSpecies = Marine;


	if( pDifficulty && strlen(pDifficulty) )
		nDifficulty = atoi(pDifficulty);
	else
		nDifficulty = DIFFICULTY_PC_MEDIUM;
	
	if( pTimeStr && strlen(pTimeStr) )
	{
		nTime = atol(pTimeStr);
		
		
		time_t nSeconds = time_t(nTime);
		struct tm* pTimeDate = localtime(&nSeconds);

		if (pTimeDate)
		{
			if (g_pInterfaceResMgr->IsEnglish())
			{
				sprintf (szDate, "%02d.%02d.%02d", pTimeDate->tm_mon + 1, pTimeDate->tm_mday, (pTimeDate->tm_year + 1900) % 100);
			}
			else
			{
				sprintf (szDate, "%02d.%02d.%02d", pTimeDate->tm_mday, pTimeDate->tm_mon + 1, (pTimeDate->tm_year + 1900) % 100);
			}
			sprintf (szTime, "%02d:%02d:%02d", pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec);

		}
	}
	else
	{
		nTime = 0;
		szTime[0] = 0;
		szDate[0] = 0;
	}	
};

void CSaveGameData::BuildSaveString(char* pszSaveStr)
{
	if (!pszSaveStr) return;
	sprintf (pszSaveStr, "%s|%d|%d|%d|%ld",szWorldName, nPrevMission, (int)eSpecies, nDifficulty, nTime);
}

uint32 CreateSavedGameIndex(const CSaveGameData & saveData)
{
	return saveData.nTime;

/*	const uint32 nStartSpeciesBits		= 30;
	const uint32 nSpeciesBitMask		= 0xC0000000;
	const uint32 nStartMissionBits		= 24;
	const uint32 nMissionBitMask		= 0x3F000000;
	const uint32 nStartDifficultyBits	= 22;
	const uint32 nDifficultyBitMask		= 0x00C00000;
	const uint32 nStartTimeBits			= 0;
	const uint32 nTimeBitMask			= 0x003FFFFF;

	uint32 nResult = 0;

	nResult |= (saveData.eSpecies << nStartSpeciesBits);

	if( saveData.nPrevMission > 0 )
	{
		nResult |= (saveData.nPrevMission << nStartMissionBits);
	}

	nResult |= (saveData.nDifficulty << nStartDifficultyBits);
	nResult |= (saveData.nTime & nTimeBitMask);

	return nResult;
*/
}


uint32 BuildSaveGameEntry( CLTGUIColumnTextCtrl* pCtrl, const CSaveGameData & saveData, 
				   int nMissionWidth, int nDateWidth, int nTimeWidth, int nDifficultyWidth )
{
	if (!pCtrl || !g_pMissionMgr || !g_pLTClient) 
	{
		_ASSERT( 0 );
		return INT_MAX;
	}

	// 
	// Set-up our color.
	//

	if( saveData.eSpecies != Unknown )
	{
		pCtrl->SetColor( g_clrSelSpeciesTextColor[saveData.eSpecies], g_clrNonSelSpeciesTextColor[saveData.eSpecies], g_clrNonSelSpeciesTextColor[saveData.eSpecies] );
	}
	else
	{
		pCtrl->SetColor( g_clrSelSpeciesTextColor[4], g_clrNonSelSpeciesTextColor[4], g_clrNonSelSpeciesTextColor[4] );
	}


	//
	// The column that contains the description.
	//
	HSTRING hText = LTNULL;
	if (saveData.nPrevMission >= 0 )
	{
		const int nMissionDesc = g_pMissionMgr->GetMissionDescription(saveData.nPrevMission);
		if( nMissionDesc >= 0 )
		{
			hText = g_pLTClient->FormatString( nMissionDesc );
		}
	}
	
	// If we don't have a description, asssume it is a custom level.
	if( !hText )
	{
		if( !saveData.szWorldName || !saveData.szWorldName[0] )
		{
			hText = g_pLTClient->CreateString( "" );
		}
		else
		{
			char szTemp[256];
			if( !g_pGameClientShell->GetNiceWorldName(const_cast<char*>(saveData.szWorldName), szTemp, sizeof(szTemp) ) )
			{
				const char * szStartPrettyName = saveData.szWorldName + strlen(saveData.szWorldName) - 1;
				const char cDirectoryBreak = '\\';

				while( szStartPrettyName > saveData.szWorldName && *szStartPrettyName != cDirectoryBreak) 
				{
					--szStartPrettyName;
				}
				++szStartPrettyName;
					
				ASSERT( szStartPrettyName[0] );

				SAFE_STRCPY( szTemp, szStartPrettyName );
			}

			hText = g_pLTClient->CreateString( szTemp );
		}
	}
	pCtrl->AddColumn(hText, nMissionWidth, LTF_JUSTIFY_LEFT);
	g_pLTClient->FreeString(hText);
		
	//
	// The column that contains the date/time.
	//
	hText = g_pLTClient->CreateString( const_cast<char*>(saveData.szDate) );
	pCtrl->AddColumn(hText, nDateWidth, LTF_JUSTIFY_LEFT);
	g_pLTClient->FreeString(hText);

	//
	// The column that contains the elapsed time.
	//
	hText = g_pLTClient->CreateString( const_cast<char*>(saveData.szTime) );
	pCtrl->AddColumn(hText, nTimeWidth, LTF_JUSTIFY_LEFT);
	g_pLTClient->FreeString(hText);

	//
	// The column that contains the difficulty.
	//
	int nDifficultyID = IDS_DIFF_MEDIUM;
	switch( saveData.nDifficulty )
	{
	case DIFFICULTY_PC_EASY :
		nDifficultyID = IDS_DIFF_EASY;
		break;

	case DIFFICULTY_PC_MEDIUM :
		nDifficultyID = IDS_DIFF_MEDIUM;
		break;

	case DIFFICULTY_PC_HARD :
		nDifficultyID = IDS_DIFF_HARD;
		break;
	
	case DIFFICULTY_PC_INSANE :
		nDifficultyID = IDS_DIFF_INSANE;
		break;
	}

	hText = g_pLTClient->FormatString(nDifficultyID);
	pCtrl->AddColumn(hText, nDifficultyWidth, LTF_JUSTIFY_LEFT);
	g_pLTClient->FreeString(hText);

	return CreateSavedGameIndex(saveData);
}

void FillSavedGameData( CSaveGameData * pData)
{
	if( !pData )
		return;

	if( g_pGameClientShell )
	{
		SAFE_STRCPY(pData->szWorldName,g_pGameClientShell->GetCurrentWorldName());
	}
	else
	{
		pData->szWorldName[0] = 0;
	}

	if( g_pGameClientShell )
		pData->eSpecies = g_pGameClientShell->GetPlayerSpecies();
	else
		pData->eSpecies = Marine;

	if( g_pGameClientShell && g_pGameClientShell->GetGameType() )
		pData->nDifficulty = g_pGameClientShell->GetGameType()->GetDifficulty();
	else
		pData->nDifficulty = DIFFICULTY_PC_MEDIUM;

	if( g_pInterfaceMgr )
		pData->nPrevMission = g_pInterfaceMgr->GetPrevMissionIndex();
	else
		pData->nPrevMission = -1;

	time_t seconds;
	time(&seconds);
	pData->nTime	   = (long)seconds;
}



void SetSaveGameText( CLTGUITextItemCtrl* pCtrl, const CSaveGameData & saveData)
{

	if (!pCtrl || !g_pMissionMgr || !g_pLTClient) 
	{
		_ASSERT( 0 );
		return;
	}

	// 
	// Set-up our color.
	//

	if( saveData.eSpecies != Unknown )
	{
		pCtrl->SetColor( g_clrSelSpeciesTextColor[saveData.eSpecies], g_clrNonSelSpeciesTextColor[saveData.eSpecies], g_clrNonSelSpeciesTextColor[saveData.eSpecies] );
	}
	else
	{
		pCtrl->SetColor( g_clrSelSpeciesTextColor[4], g_clrNonSelSpeciesTextColor[4], g_clrNonSelSpeciesTextColor[4]);
	}


	bool bFoundDescription = false;
	HSTRING hText = LTNULL;
	char szTemp[256] = "[";
	if (saveData.nPrevMission >= 0 )
	{
		const int nMissionDesc = g_pMissionMgr->GetMissionDescription(saveData.nPrevMission);
		if( nMissionDesc >= 0 )
		{
			hText = g_pLTClient->FormatString( nMissionDesc );
			if( hText )
			{
				bFoundDescription = true;
				strcat( szTemp, g_pLTClient->GetStringData(hText));
			}
			g_pLTClient->FreeString(hText);
		}
	}
	
	// If we don't have a description, asssume it is a custom level.
	if( !bFoundDescription )
	{
		if( saveData.szWorldName && saveData.szWorldName[0] )
		{
			if( !g_pGameClientShell->GetNiceWorldName(const_cast<char*>(saveData.szWorldName), szTemp + 1, sizeof(szTemp - 1) ) )
			{
				const char * szStartPrettyName = saveData.szWorldName + strlen(saveData.szWorldName) - 1;
				const char cDirectoryBreak = '\\';

				while( szStartPrettyName > saveData.szWorldName && *szStartPrettyName != cDirectoryBreak) 
				{
					--szStartPrettyName;
				}
				++szStartPrettyName;
			
				bFoundDescription = true;
				ASSERT( szStartPrettyName[0] );

				strcat( szTemp, szStartPrettyName );
			}
			else
			{
				bFoundDescription = true;
			}
		}
	}

	if( saveData.szDate && saveData.szDate[0] )
	{
		strcat(szTemp,",  ");
		strcat(szTemp,saveData.szDate);
	}
	if( saveData.szTime && saveData.szTime[0] )
	{
		strcat(szTemp,",  ");
		strcat(szTemp,saveData.szTime);
	}

	int nDifficultyID = IDS_DIFF_MEDIUM;
	switch( saveData.nDifficulty )
	{
	case DIFFICULTY_PC_EASY :
		nDifficultyID = IDS_DIFF_EASY;
		break;

	case DIFFICULTY_PC_MEDIUM :
		nDifficultyID = IDS_DIFF_MEDIUM;
		break;

	case DIFFICULTY_PC_HARD :
		nDifficultyID = IDS_DIFF_HARD;
		break;
	
	case DIFFICULTY_PC_INSANE :
		nDifficultyID = IDS_DIFF_INSANE;
		break;
	}

	hText = g_pLTClient->FormatString(nDifficultyID);
	strcat(szTemp,",  ");
	strcat(szTemp,g_pLTClient->GetStringData(hText));
	g_pLTClient->FreeString(hText);

	strcat(szTemp,"]");	

	hText = g_pLTClient->CreateString( szTemp );
	pCtrl->RemoveAll();
	pCtrl->AddString(hText);
	g_pLTClient->FreeString(hText);
}
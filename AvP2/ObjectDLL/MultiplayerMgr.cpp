// -------------------------------------------------------------------- //
//
// MODULE  : MultiplayerMgr.cpp
//
// PURPOSE : A server-side multiplayer statistics manager
//
// CREATED : 3/27/01
//
// -------------------------------------------------------------------- //

#include "stdafx.h"
#include "MultiplayerMgr.h"
#include "CharacterButeMgr.h"
#include "MsgIDs.h"
#include "ServerRes.h"

#include "PlayerObj.h"
#include "ContainerCodes.h"

// include files for AI taunt notification.
#include "GameServerShell.h"
#include "ServerSoundMgr.h"
#include "SoundButeMgr.h"
#include "AI.h"
#include "AISenseMgr.h"
#include "CharacterFuncs.h"

#include "ServerOptions.h"

// -------------------------------------------------------------------- //

#define MPMGR_FORCE_VERIFY_TIME		15.0f
#define MPMGR_FORCE_LEVEL_LOAD		30.0f

#define MPMGR_MUTATE_TIME			2.0f

#define MPMGR_SURVIVOR_MKILL_PTS	10
#define MPMGR_SURVIVOR_FKILL_PTS	-10

#define MPMGR_OVERRUN_PTS			1
#define MPMGR_EVAC_PTS				1

#define MPMGR_ROUND_DELAY			15.0f
#define MPMGR_ROUND_START_DELAY		20.0f

#define MPMGR_EVAC_DELAY			11.5f

// -------------------------------------------------------------------- //

static char* GetBaseMapName(char *szMap)
{
	char *szTemp;

	if(!szMap) return LTNULL;

	int size = strlen(szMap);

	for(szTemp = szMap + size; szTemp > szMap; szTemp--)
	{
		if(*szTemp == '\\')
		{
			szTemp = szTemp + 1;
			break;
		}
	}

	return szTemp;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPMgrSender::SendTo()
//
// PURPOSE:		Sender class for GameSpy support
//
// -------------------------------------------------------------------- //

void MPMgrSender::SendTo(const void *pData, unsigned long len, const char *sAddr, unsigned long port)
{
	g_pLTServer->SendTo(pData, len, sAddr, port);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::MultiplayerMgr()
//
// PURPOSE:		Initial setup
//
// -------------------------------------------------------------------- //

MultiplayerMgr::MultiplayerMgr()
{
	// Initialize any variables that need it
	UpdateGameInfo();

	m_nCurrentLevel = 0;
	m_nNextLevel = 0;

	m_nClients = 0;

	m_bUsingGameSpy = LTFALSE;

	m_fStartTime = 0.0f;

	m_nTimeRemaining = 0;
	m_nServerState = MPMGR_STATE_PLAYING;
	m_nServerSubState = MPMGR_SUBSTATE_NONE;
	m_nWaitSeconds = 0xFF;

	m_nServerController = 0xFF;

	m_fVerifyTime = 0.0f;
	m_fMiscTime = 0.0f;

	m_nCurrentRound = 0;

	m_bGameServHosted = LTFALSE;
	m_bUpdateGameServ = LTFALSE;

	m_nEvacState = 0;

	m_bGameInfoChanged = LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::~MultiplayerMgr()
//
// PURPOSE:		Mass Destruction!! Bwah ha ha!
//
// -------------------------------------------------------------------- //

MultiplayerMgr::~MultiplayerMgr()
{
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::Setup()
//
// PURPOSE:		Connect up to GameSpy
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::Setup()
{
	// If our game type is not of a multiplayer variety... then skip this
	if(!GetGameType().IsMultiplayer())
		return LTFALSE;


	// Get our ip address and port...
	char szBuf[32];
	uint16 nPort = 0;
    g_pLTServer->GetTcpIpAddress(szBuf, 30, nPort);


	// Init the GameSpy manager
	LTBOOL bReturn = LTFALSE;

#ifdef _WIN32
	HSTRING hStr = g_pLTServer->FormatString(IDS_VERSION);
#else
	HSTRING hStr = g_pLTServer->CreateString("1.0.9.6");
#endif

	if( CGameSpyMgr::Init(GAMESPY_GAME_NAME, g_pLTServer->GetStringData(hStr), GAMESPY_SECRET_KEY, 0,
		szBuf, nPort, (nPort + 166), m_GameInfo.m_bLANConnection, GSMF_USEGAMEPORTFORHEARTBEAT))
	{
		CGameSpyMgr::SetSendHandler(&m_Sender);
		bReturn = LTTRUE;

		if(!m_GameInfo.m_bLANConnection)
		{
			m_bUsingGameSpy = LTTRUE;
		}
	}

	g_pLTServer->FreeString(hStr);
	hStr = LTNULL;

	return bReturn;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::CleanUp()
//
// PURPOSE:		Terminate the GameSpy support
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::CleanUp()
{
	// Only terminate the GameSpy manager if it was used
	CGameSpyMgr::Term();

	return LTTRUE;
}

 
// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ProcessPacket()
//
// PURPOSE:		Handle the packets from ILTServer
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ProcessPacket(char* sData, uint32 dataLen, uint8 senderAddr[4], uint16 senderPort)
{
	if(dataLen > 0 && sData[0] == '\\')
	{
		char szAddr[128];
		sprintf(szAddr, "%d.%d.%d.%d", senderAddr[0], senderAddr[1], senderAddr[2], senderAddr[3]);
		CGameSpyMgr::OnQuery(szAddr, senderPort, sData, dataLen);
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::AddClient()
//
// PURPOSE:		Add a new client into the list
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::AddClient(HCLIENT hClient, MPClientData *pData)
{
	// Make sure this client is even valid
	if(!hClient) return LTFALSE;


	// See if this client is already on the VIP list
	int nIndex = FindClient(hClient);

	// If this client is already here... just return
	if(nIndex != -1) return LTTRUE;


	// Otherwise, find a spot to add us in
	for(uint8 i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient == LTNULL)
		{
			m_ClientData[i].m_hClient = hClient;
			m_ClientData[i].m_nClientID = g_pLTServer->GetClientID(hClient);

			m_ClientData[i].m_fJoinedTime = g_pLTServer->GetTime();
			m_ClientData[i].m_fPickupMsgTime = 0.0f;
			m_ClientData[i].m_bVerified = LTFALSE;

			m_ClientData[i].m_bFirstRespawn = LTTRUE;
			m_ClientData[i].m_bSpawnAsQueen = LTFALSE;
			m_ClientData[i].m_nFrags = 0;
			m_ClientData[i].m_nConsecutiveFrags = 0;
			m_ClientData[i].m_nScore = 0;

			if(pData)
				memcpy(&m_ClientData[i].m_Data, pData, sizeof(MPClientData));

			// Create a unique name for this client
			LTBOOL bRenamed = CreateUniqueClientName(hClient);

			// Send the current game information to this new client
			SendGameInfo(hClient);

			m_nClients++;

			// Now send this new client's information to everyone
			HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_ADD_CLIENT);
			uint8 nClass = m_ClientData[i].m_Data.m_nCharacterClass;

			hWrite->WriteString(m_ClientData[i].m_Data.m_szName);
			hWrite->WriteByte(nClass);
			hWrite->WriteByte(m_ClientData[i].m_Data.m_nCharacterSet[nClass]);
			hWrite->WriteDWord(m_ClientData[i].m_nClientID);

			g_pLTServer->EndMessage(hWrite);

			// If this is not a dedicated server then make the
			// first client the controler.
			if(!m_GameInfo.m_bDedicated)
			{
				if(m_nClients == 1)
				{
					SetServerController(i);
				}
			}
			else
			{
				// Grab the client data stored in Lithtech and update the character settings...
				void *pData;
				uint32 nLength;
				g_pLTServer->GetClientData(hClient, pData, nLength);

				if(nLength == sizeof(MPClientData))
				{
					MPClientData *pMPData = (MPClientData*)pData;
					
					// if we had server control before
					// give it back now.
					if(pMPData->m_bServerControl)
					{
						SetServerController(i);
					}
				}
			}


			// If the server state is waiting for the next level verification... send
			// that info to this new client
			if(m_nServerState == MPMGR_STATE_NEXT_LEVEL_VERIFY)
			{
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_MPMGR_NEXT_LEVEL);
				hMessage->WriteByte(MPMGR_NEXT_LEVEL_IN_PROGRESS);
				hMessage->WriteString(GetBaseMapName(m_GameInfo.m_szLevels[m_nNextLevel]));
				g_pLTServer->EndMessage(hMessage);
			}
			// If the server state is waiting on load level verification...
			else if(m_nServerState == MPMGR_STATE_LOAD_LEVEL_VERIFY)
			{
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_MPMGR_LOAD_LEVEL);
				g_pLTServer->EndMessage(hMessage);
			}


			// Now tell the client if he's been renamed or not
			if(bRenamed)
			{
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_MPMGR_SERVER_MESSAGE);
				hMessage->WriteByte(MP_CLIENT_RENAMED);
				hMessage->WriteString(m_ClientData[i].m_Data.m_szName);
				g_pLTServer->EndMessage(hMessage);

				// Save the new name so the next level won't need to rename us again
				strncpy(pData->m_szName, m_ClientData[i].m_Data.m_szName, MAX_MP_CLIENT_NAME_LENGTH);
			}


			// Tell the player that he got auto-switched
			if(pData && pData->m_bAutoTeamSwitched)
			{
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_MPMGR_SERVER_MESSAGE);
				hMessage->WriteByte(MP_AUTO_TEAM_SWITCH);
				g_pLTServer->EndMessage(hMessage);

				pData->m_bAutoTeamSwitched = LTFALSE;
			}


			// Tell serverapp about it.
			ServerAppAddClient( hClient );

			return LTTRUE;
		}
	}


	// We must be out of slots... sorry bud!
	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::RemoveClient()
//
// PURPOSE:		Remove a client from the list
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::RemoveClient(HCLIENT hClient)
{
	// Make sure this client is even valid
	if(!hClient) return LTFALSE;

	// Find the client
	int nIndex = FindClient(hClient);
	if(nIndex == -1) return LTFALSE;


	// Otherwise, get rid of his ass! He's been drinking all the beer!
	m_ClientData[nIndex].m_hClient = LTNULL;
	m_nClients--;


	// Now make sure all the clients know that he left
	HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_REMOVE_CLIENT);
	hWrite->WriteDWord(m_ClientData[nIndex].m_nClientID);
	g_pLTServer->EndMessage(hWrite);


	// Update who has control over the server
	if(nIndex == m_nServerController)
		m_nServerController = 0xff;

	// Tell serverapp about it.
	ServerAppRemoveClient( hClient );

	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::CreateUniqueClientName()
//
// PURPOSE:		Sets this client's name to something unique and returns
//				true if the name was changed
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::CreateUniqueClientName(HCLIENT hClient)
{
	// Make sure this client is even valid
	if(!hClient) return LTFALSE;

	// Find the client
	int nIndex = FindClient(hClient);
	if(nIndex == -1) return LTFALSE;


	// Trim off any ending spaces first...
	int nLength = strlen(m_ClientData[nIndex].m_Data.m_szName);
	char *szTemp = m_ClientData[nIndex].m_Data.m_szName + ((nLength > 0) ? (nLength - 1) : 0);

	while((*szTemp == ' ') && (szTemp != m_ClientData[nIndex].m_Data.m_szName))
		szTemp--;

	szTemp[1] = '\0';

	if((szTemp == m_ClientData[nIndex].m_Data.m_szName) && ((*szTemp == ' ') || (*szTemp == '\0')))
	{
		strcpy(m_ClientData[nIndex].m_Data.m_szName, "X");
	}
	else
	{
		// Now trim off any leading spaces...
		szTemp = m_ClientData[nIndex].m_Data.m_szName;

		while(*szTemp == ' ')
			szTemp++;

		strcpy(m_ClientData[nIndex].m_Data.m_szName, szTemp);
	}


	// Loop through to see if we match any of the other names
	LTBOOL bContinue = LTTRUE;
	LTBOOL bChanged = LTFALSE;

	while(bContinue)
	{
		bContinue = LTFALSE;

		for(uint8 i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_ClientData[i].m_hClient != LTNULL)
			{
				LTBOOL bMakeUnique = LTFALSE;

				if(i != nIndex)
				{
					if(!_stricmp(m_ClientData[nIndex].m_Data.m_szName, m_ClientData[i].m_Data.m_szName))
					{
						bMakeUnique = LTTRUE;
					}
				}
				else
				{
					 if(!_stricmp(m_ClientData[nIndex].m_Data.m_szName, "name") ||
						!_stricmp(m_ClientData[nIndex].m_Data.m_szName, "serv") ||
						!_stricmp(m_ClientData[nIndex].m_Data.m_szName, "server"))
					{
						bMakeUnique = LTTRUE;
					}
				}

				if(bMakeUnique)
				{
					char *szName = m_ClientData[nIndex].m_Data.m_szName;

					// Get the length of the current name
					int nLength = strlen(szName);

					// Figure out an index for the numeric portion of the name
					int nNumIndex = nLength - 1;

					while((nNumIndex > 0) && isdigit(szName[nNumIndex]))
						nNumIndex--;

					// Get the actual number
					int nNum = (nNumIndex == (nLength - 1)) ? 0 : atoi(szName + nNumIndex + 1) + 1;

					// Get the remaining part of the string
					char szBuffer[MAX_MP_CLIENT_NAME_LENGTH];
					strncpy(szBuffer, szName, nNumIndex + 1);
					szBuffer[nNumIndex + 1] = 0;

					// Set the new name and continue checking...
					sprintf(szName, "%s%d", szBuffer, nNum);

					bContinue = LTTRUE;
					bChanged = LTTRUE;
					break;
				}
			}
		}
	}


	// Grab the client data stored in Lithtech and update the character settings...
	void *pData;
	uint32 nDataLength;
	g_pLTServer->GetClientData(hClient, pData, nDataLength);

	if(nDataLength == sizeof(MPClientData))
	{
		MPClientData *pMPData = (MPClientData*)pData;

		memset(pMPData->m_szName, 0, MAX_MP_CLIENT_NAME_LENGTH);
		strncpy(pMPData->m_szName, m_ClientData[nIndex].m_Data.m_szName, MAX_MP_CLIENT_NAME_LENGTH - 1);
	}


	// Flag a serverapp update if the name changed.
	if( bChanged )
	{
		// Flag an update to happen.
		SetUpdateGameServ( );
	}


	return bChanged;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::FindClient()
//
// PURPOSE:		Returns the index of a client... or -1 if not found
//
// -------------------------------------------------------------------- //

int MultiplayerMgr::FindClient(HCLIENT hClient)
{
	// Make sure this client is even valid
	if(!hClient) return -1;

	// Go through the list and look for this guy
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient == hClient)
		{
			return i;
		}
	}

	return -1;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::FindClient()
//
// PURPOSE:		Returns the index of a client... or -1 if not found
//
// -------------------------------------------------------------------- //

int MultiplayerMgr::FindClient(uint32 nClient)
{
	// Go through the list and look for this guy
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient != LTNULL)
		{
			if(m_ClientData[i].m_nClientID == nClient)
			{
				return i;
			}
		}
	}

	return -1;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::FindClient()
//
// PURPOSE:		Returns the index of a client... or -1 if not found
//
// -------------------------------------------------------------------- //

int MultiplayerMgr::FindClient(char *szName)
{
	// Go through the list and look for this guy
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient != LTNULL)
		{
			if(!_stricmp(m_ClientData[i].m_Data.m_szName, szName))
			{
				return i;
			}
		}
	}

	return -1;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::FindRandomClient()
//
// PURPOSE:		Returns the index of a random client in the game
//
// -------------------------------------------------------------------- //

int MultiplayerMgr::FindRandomClient()
{
	// If there are no clients... return invalid
	if(m_nClients < 1)
		return -1;


	// Get the random client number to retrieve
	int nRandom = GetRandom(1, m_nClients);
	int nNumber = 1;

	// Go through the list and look for this guy
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient != LTNULL)
		{
			if(nRandom == nNumber)
				return i;

			nNumber++;
		}
	}

	return -1;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::FindLowestFragsClient()
//
// PURPOSE:		Finds the client with the lowest frags
//
// -------------------------------------------------------------------- //

int MultiplayerMgr::FindLowestFragsClient(uint8 nTeam)
{
	// If there are no clients... return invalid
	if(m_nClients < 1)
		return -1;


	// Get the random client number to retrieve
	int nFrags = 100000;
	int nNumber = -1;

	// Go through the list and look for this guy
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient != LTNULL)
		{
			if((nTeam > MAX_TEAMS) || (nTeam == m_ClientData[i].m_Data.m_nCharacterClass))
			{
				if(m_ClientData[i].m_nFrags < nFrags)
				{
					nNumber = i;
					nFrags = m_ClientData[i].m_nFrags;
				}
			}
		}
	}

	return nNumber;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::FindLowestFragsClient()
//
// PURPOSE:		Finds the client with the highest frags
//
// -------------------------------------------------------------------- //

int MultiplayerMgr::FindHighestFragsClient(uint8 nTeam)
{
	// If there are no clients... return invalid
	if(m_nClients < 1)
		return -1;


	// Get the random client number to retrieve
	int nFrags = -100000;
	int nNumber = -1;

	// Go through the list and look for this guy
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient != LTNULL)
		{
			if((nTeam > MAX_TEAMS) || (nTeam == m_ClientData[i].m_Data.m_nCharacterClass))
			{
				if(m_ClientData[i].m_nFrags > nFrags)
				{
					nNumber = i;
					nFrags = m_ClientData[i].m_nFrags;
				}
			}
		}
	}

	return nNumber;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::FindLowestFragsClient()
//
// PURPOSE:		Finds the client with the lowest score
//
// -------------------------------------------------------------------- //

int MultiplayerMgr::FindLowestScoreClient(uint8 nTeam)
{
	// If there are no clients... return invalid
	if(m_nClients < 1)
		return -1;


	// Get the random client number to retrieve
	int nScore = 100000;
	int nNumber = -1;

	// Go through the list and look for this guy
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient != LTNULL)
		{
			if((nTeam > MAX_TEAMS) || (nTeam == m_ClientData[i].m_Data.m_nCharacterClass))
			{
				if(m_ClientData[i].m_nFrags < nScore)
				{
					nNumber = i;
					nScore = m_ClientData[i].m_nScore;
				}
			}
		}
	}

	return nNumber;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::FindLowestFragsClient()
//
// PURPOSE:		Finds the client with the highest score
//
// -------------------------------------------------------------------- //

int MultiplayerMgr::FindHighestScoreClient(uint8 nTeam)
{
	// If there are no clients... return invalid
	if(m_nClients < 1)
		return -1;


	// Get the random client number to retrieve
	int nScore = -100000;
	int nNumber = -1;

	// Go through the list and look for this guy
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient != LTNULL)
		{
			if((nTeam > MAX_TEAMS) || (nTeam == m_ClientData[i].m_Data.m_nCharacterClass))
			{
				if(m_ClientData[i].m_nFrags > nScore)
				{
					nNumber = i;
					nScore = m_ClientData[i].m_nScore;
				}
			}
		}
	}

	return nNumber;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::FindPlayerObj()
//
// PURPOSE:		Finds a pointer to the player object from a client handle
//
// -------------------------------------------------------------------- //

CPlayerObj* MultiplayerMgr::FindPlayerObj(HCLIENT hClient)
{
	HOBJECT hPlayerObj = LTNULL;
	CPlayerObj *pPlayer = LTNULL;
	ObjArray<HOBJECT, MAX_CLIENTS> objArray;


	// Every player should have 'Player' as it's object name
	g_pLTServer->FindNamedObjects("Player", objArray);
	if(!objArray.NumObjects()) return LTNULL;


	// Go through the list one by one and compare the handles...
	for(uint32 i = 0; i < objArray.NumObjects(); i++)
	{
		// Get this object in the list
		hPlayerObj = objArray.GetObject(i);
		if(!hPlayerObj) continue;

		pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(hPlayerObj));
		if(!pPlayer) continue;

		// Now see if this is our guy...
		if(pPlayer->GetClient() == hClient)
			return pPlayer;
	}

	// If all else fails... return NULL
	return LTNULL;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::GetClientData()
//
// PURPOSE:		Get information on a particular client
//
// -------------------------------------------------------------------- //

MPMgrData* MultiplayerMgr::GetClientData(HCLIENT hClient)
{
	// Find the client
	int nIndex = FindClient(hClient);
	if(nIndex == -1) return LTNULL;

	return &m_ClientData[nIndex];
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::GetClientScore()
//
// PURPOSE:		Returns the score value for this client
//
// -------------------------------------------------------------------- //

uint8 MultiplayerMgr::GetClientScore(HCLIENT hClient)
{
	// Find the client
	int nIndex = FindClient(hClient);
	if(nIndex == -1) return 0;


	// Go through the whole score list
	for(int i = 0; i < MAX_MP_SCORING_CLASSES; i++)
	{
		uint8 nClass = m_ClientData[nIndex].m_Data.m_nCharacterClass;
		uint8 nSet = m_ClientData[nIndex].m_Data.m_nCharacterSet[nClass];

		if(m_GameInfo.m_nScoreClasses[i] == g_pCharacterButeMgr->GetScoringClass(nSet))
		{
			return m_GameInfo.m_nScoreValues[i];
		}
	}

	return 0;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::GetClientScore()
//
// PURPOSE:		Returns the score value for this client
//
// -------------------------------------------------------------------- //

uint8 MultiplayerMgr::GetClientScore(uint32 nClient)
{
	// Find the client
	int nIndex = FindClient(nClient);
	if(nIndex == -1) return 0;


	// Go through the whole score list
	for(int i = 0; i < MAX_MP_SCORING_CLASSES; i++)
	{
		uint8 nClass = m_ClientData[nIndex].m_Data.m_nCharacterClass;
		uint8 nSet = m_ClientData[nIndex].m_Data.m_nCharacterSet[nClass];

		if(m_GameInfo.m_nScoreClasses[i] == g_pCharacterButeMgr->GetScoringClass(nSet))
		{
			return m_GameInfo.m_nScoreValues[i];
		}
	}

	return 0;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ClientsOnSameTeam()
//
// PURPOSE:		Are these clients on the same team?
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::ClientsOnSameTeam(HCLIENT hClient1, HCLIENT hClient2)
{
	// Get the indexes for each of these clients...
	int nIndex1 = FindClient(hClient1);
	int nIndex2 = FindClient(hClient2);

	// If either of these clients is invalid... just leave
	if((nIndex1 == -1) || (nIndex2 == -1))
		return LTFALSE;


	if(m_ClientData[nIndex1].m_Data.m_nCharacterClass == m_ClientData[nIndex2].m_Data.m_nCharacterClass)
	{
		return LTTRUE;
	}

	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ResetClient()
//
// PURPOSE:		Reset the client information
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::ResetClient(HCLIENT hClient)
{
	// Return if the client is invalid
	if(!hClient)
		return LTFALSE;


	// Go through the list and reset any information that needs to be
	// reset for the valid clients
	for(uint8 i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient == hClient)
		{
			m_ClientData[i].m_nUpdateFlags = 0xFF;

			m_ClientData[i].m_nFrags = 0;
			m_ClientData[i].m_nConsecutiveFrags = 0;
			m_ClientData[i].m_nScore = 0;

			m_ClientData[i].m_fPickupMsgTime = 0.0f;
			m_ClientData[i].m_bVerified = LTFALSE;

			m_ClientData[i].m_fTauntTimer = 0.0f;

			m_ClientData[i].m_bFirstRespawn = LTTRUE;

			m_ClientData[i].m_fMiscTimer = 0.0f;
			m_ClientData[i].m_nNextClass = -1;

			m_ClientData[i].m_nLives = 1;
			m_ClientData[i].m_bDisplayEvacZone = LTFALSE;

			m_ClientData[i].m_bObserving = LTFALSE;
			m_ClientData[i].m_nObserveMode = 0;

			m_ClientData[i].m_bSpawnAsQueen = LTFALSE;

			return LTTRUE;
		}
	}

	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ResetAllClients()
//
// PURPOSE:		Reset the client list
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::ResetAllClients()
{
	uint8 i = 0;

	// Go ahead and reset some general game variables too
	m_fStartTime = 0.0f;

	m_nServerState = MPMGR_STATE_PLAYING;
	m_nServerSubState = MPMGR_SUBSTATE_NONE;

	m_fVerifyTime = 0.0f;
	m_fMiscTime = 0.0f;

	m_nCurrentRound = 0;

	m_nEvacState = 0;


	// Go through the list and reset any information that needs to be
	// reset for the valid clients
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		m_ClientData[i].m_nUpdateFlags = 0xFF;

		m_ClientData[i].m_nFrags = 0;
		m_ClientData[i].m_nConsecutiveFrags = 0;
		m_ClientData[i].m_nScore = 0;

		m_ClientData[i].m_fPickupMsgTime = 0.0f;
		m_ClientData[i].m_bVerified = LTFALSE;

		m_ClientData[i].m_fTauntTimer = 0.0f;

		m_ClientData[i].m_bFirstRespawn = LTTRUE;

		m_ClientData[i].m_fMiscTimer = 0.0f;
		m_ClientData[i].m_nNextClass = -1;

		m_ClientData[i].m_nLives = 1;
		m_ClientData[i].m_bDisplayEvacZone = LTFALSE;

		m_ClientData[i].m_bObserving = LTFALSE;
		m_ClientData[i].m_nObserveMode = 0;
		m_ClientData[i].m_bSpawnAsQueen = LTFALSE;
	}


	// Now reset all the team data
	for(i = 0; i < MAX_TEAMS; i++)
	{
		m_TeamData[i].m_nUpdateFlags = 0xFF;

		m_TeamData[i].m_nFrags = 0;
		m_TeamData[i].m_nScore = 0;
	}


	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ClientObserveCommand()
//
// PURPOSE:		Set observe command of a particular client
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ClientObserveCommand(HCLIENT hClient, uint8 nCommand)
{
	// Find this player
	CPlayerObj *pPlayer = FindPlayerObj(hClient);
	int nIndex = FindClient(hClient);
	HMESSAGEWRITE hMessage;

	LTBOOL bUpdatePosition = LTTRUE;

	if(pPlayer && (nIndex != -1))
	{
		if(m_ClientData[nIndex].m_nNextClass >= MAX_TEAMS)
		{
			switch(nCommand)
			{
				case MP_OBSERVE_ON:
				{
					pPlayer->SetObservePoint_Random();

					// Tell everyone that this client is in observe mode...
					hMessage = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_OBSERVE_MODE);
					hMessage->WriteByte(MP_OBSERVE_ON);
					hMessage->WriteDWord(m_ClientData[nIndex].m_nClientID);
					g_pLTServer->EndMessage(hMessage);

					break;
				}

				case MP_OBSERVE_OFF:
				{
					pPlayer->Respawn();
					break;
				}

				case MP_OBSERVE_PREV:
				{
					if(pPlayer->GetState() == PS_OBSERVING)
						pPlayer->SetObservePoint_Prev();
					else
						bUpdatePosition = LTFALSE;

					break;
				}

				case MP_OBSERVE_NEXT:
				{
					if(pPlayer->GetState() == PS_OBSERVING)
						pPlayer->SetObservePoint_Next();
					else
						bUpdatePosition = LTFALSE;

					break;
				}

				case MP_OBSERVE_RANDOM:
				{
					if(pPlayer->GetState() == PS_OBSERVING)
						pPlayer->SetObservePoint_Random();
					else
						bUpdatePosition = LTFALSE;

					break;
				}

				case MP_OBSERVE_CHANGE_MODE:
				{
					break;
				}
			}


			if(bUpdatePosition)
			{
				// Make sure we store the observe state
				m_ClientData[nIndex].m_bObserving = LTTRUE;

				// Send down the current observe point for the client who sent this change...
				hMessage = g_pLTServer->StartMessage(hClient, MID_MPMGR_OBSERVE_MODE);
				hMessage->WriteByte(MP_OBSERVE_POINT);
				hMessage->WriteByte((uint8)pPlayer->GetObservePoint());
				hMessage->WriteByte(m_ClientData[nIndex].m_nObserveMode);
				g_pLTServer->EndMessage(hMessage);
			}
		}
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::UpdateClients()
//
// PURPOSE:		Go through all the clients and check to see if updates are needed
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::UpdateClients()
{
	for(uint8 i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient != LTNULL && m_ClientData[i].m_nUpdateFlags)
		{
			// Start an update message
			HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_UPDATE_CLIENT);

			// Fill in the basic information about the game
			hWrite->WriteDWord(m_ClientData[i].m_nClientID);
			hWrite->WriteByte(m_ClientData[i].m_nUpdateFlags);

			if(m_ClientData[i].m_nUpdateFlags & MP_CLIENT_UPDATE_CLIENTDATA)
			{
				uint8 nClass = m_ClientData[i].m_Data.m_nCharacterClass;

				hWrite->WriteString(m_ClientData[i].m_Data.m_szName);
				hWrite->WriteByte(m_ClientData[i].m_Data.m_nCharacterClass);
				hWrite->WriteByte(m_ClientData[i].m_Data.m_nCharacterSet[nClass]);
			}

			if(m_ClientData[i].m_nUpdateFlags & MP_CLIENT_UPDATE_PING)
			{
				hWrite->WriteWord(m_ClientData[i].m_nPing);
			}

			if(m_ClientData[i].m_nUpdateFlags & MP_CLIENT_UPDATE_FRAGS)
			{
				hWrite->WriteWord(m_ClientData[i].m_nFrags);
			}

			if(m_ClientData[i].m_nUpdateFlags & MP_CLIENT_UPDATE_CONFRAGS)
			{
				if(m_ClientData[i].m_nConsecutiveFrags > 100)
					m_ClientData[i].m_nConsecutiveFrags = 100;

				hWrite->WriteByte(m_ClientData[i].m_nConsecutiveFrags);
			}

			if(m_ClientData[i].m_nUpdateFlags & MP_CLIENT_UPDATE_SCORE)
			{
				hWrite->WriteWord(m_ClientData[i].m_nScore);
			}

			if(m_ClientData[i].m_nUpdateFlags & MP_CLIENT_UPDATE_LIVES)
			{
				hWrite->WriteByte(m_ClientData[i].m_nLives);
			}

			// Send it!
			g_pLTServer->EndMessage2(hWrite, MESSAGE_NAGGLE);

			// Clear out the flags so we don't get in here again...
			m_ClientData[i].m_nUpdateFlags = 0;
		}
	}
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::VerifyClientData()
//
// PURPOSE:		Go through all the clients and verify who they are
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::VerifyClientData(HCLIENT hClient)
{
	for(uint8 i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient != LTNULL && m_ClientData[i].m_hClient != hClient)
		{
			// Start an update message
			HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(hClient, MID_MPMGR_UPDATE_CLIENT);

			// Fill in the basic information about the game
			hWrite->WriteDWord(m_ClientData[i].m_nClientID);
			hWrite->WriteByte(MP_CLIENT_UPDATE_CLIENTDATA);

			uint8 nClass = m_ClientData[i].m_Data.m_nCharacterClass;

			hWrite->WriteString(m_ClientData[i].m_Data.m_szName);
			hWrite->WriteByte(m_ClientData[i].m_Data.m_nCharacterClass);
			hWrite->WriteByte(m_ClientData[i].m_Data.m_nCharacterSet[nClass]);

			// Send it!
			g_pLTServer->EndMessage2(hWrite, MESSAGE_NAGGLE);
		}
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::UpdateClientPings()
//
// PURPOSE:		Go through all the client and update the current Pings
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::UpdateClientPings()
{
	// Go through the list and reset any information that needs to be
	// reset for the valid clients
	for(uint8 i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient != LTNULL)
		{
			LTFLOAT fPing;
			uint16 nPing;

			if(g_pLTServer->GetClientPing(m_ClientData[i].m_hClient, fPing) == LT_OK)
			{
				nPing = (uint16)(fPing * 1000.0f);

				if(nPing != m_ClientData[i].m_nPing)
				{
					m_ClientData[i].m_nPing = nPing;
					m_ClientData[i].m_nUpdateFlags |= MP_CLIENT_UPDATE_PING;
				}
			}
		}
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::UpdateTeams()
//
// PURPOSE:		Go through all the teams and check to see if updates are needed
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::UpdateTeams()
{
	for(uint8 i = 0; i < MAX_TEAMS; i++)
	{
		if(m_TeamData[i].m_nUpdateFlags)
		{
			// Start an update message
			HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_UPDATE_TEAM);

			// Fill in the basic information about the team
			hWrite->WriteByte(i);
			hWrite->WriteByte(m_TeamData[i].m_nUpdateFlags);

			if(m_TeamData[i].m_nUpdateFlags & MP_CLIENT_UPDATE_FRAGS)
			{
				hWrite->WriteWord(m_TeamData[i].m_nFrags);
			}

			if(m_TeamData[i].m_nUpdateFlags & MP_CLIENT_UPDATE_SCORE)
			{
				hWrite->WriteWord(m_TeamData[i].m_nScore);
			}

			// Send it!
			g_pLTServer->EndMessage2(hWrite, MESSAGE_NAGGLE);

			// Clear out the flags so we don't get in here again...
			m_TeamData[i].m_nUpdateFlags = 0;
		}
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::NumberOfClass()
//
// PURPOSE:		Counts the number of client of a specific class... which
//				can also include counting the next class variable.
//
// -------------------------------------------------------------------- //

int MultiplayerMgr::NumberOfClass(uint8 nClass, LTBOOL bIncludeNextClass, LTBOOL bIgnoreObserving)
{
	int nNum = 0;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient != LTNULL)
		{
			if(bIncludeNextClass && (m_ClientData[i].m_nNextClass < MAX_TEAMS))
			{
				if(m_ClientData[i].m_nNextClass == nClass)
				{
					if(!bIgnoreObserving || !m_ClientData[i].m_bObserving)
						nNum++;
				}
			}
			else
			{
				if(m_ClientData[i].m_Data.m_nCharacterClass == nClass)
				{
					if(!bIgnoreObserving || !m_ClientData[i].m_bObserving)
						nNum++;
				}
			}
		}
	}

	return nNum;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ValidateClientData()
//
// PURPOSE:		Make sure the selected characters are supported by the server
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::ValidateClientData(HCLIENT hClient, MPClientData *pData)
{
	// Go through each character and make sure the server supports it...
	for(int i = 0; i < MAX_TEAMS; i++)
	{
		CString str = g_pCharacterButeMgr->GetMultiplayerModel(pData->m_nCharacterSet[i]);
		CharacterClass eClass = g_pCharacterButeMgr->GetClass(pData->m_nCharacterSet[i]);
		LTBOOL bValid = LTFALSE;

		switch(eClass)
		{
			case ALIEN:			if(i == Alien) bValid = LTTRUE; 		break;
			case MARINE:		if(i == Marine) bValid = LTTRUE; 		break;
			case PREDATOR:		if(i == Predator) bValid = LTTRUE; 		break;
			case CORPORATE:		if(i == Corporate) bValid = LTTRUE;		break;
		}

		if(str.IsEmpty() || str.Find(".abc") < 0 || !bValid)
		{
			switch(i)
			{
				case Marine:	pData->m_nCharacterSet[i] = (uint8)g_pCharacterButeMgr->GetSetFromModelType("Harrison_MP");	break;
				case Alien:		pData->m_nCharacterSet[i] = (uint8)g_pCharacterButeMgr->GetSetFromModelType("Drone_MP");	break;
				case Predator:	pData->m_nCharacterSet[i] = (uint8)g_pCharacterButeMgr->GetSetFromModelType("Predator_MP");	break;
				case Corporate:	pData->m_nCharacterSet[i] = (uint8)g_pCharacterButeMgr->GetSetFromModelType("Rykov_MP");	break;
			}
		}
	}

	// Now make sure the class selection is ok...
	if(pData->m_nCharacterClass >= MAX_TEAMS)
		pData->m_nCharacterClass = Marine;

	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ValidateClass()
//
// PURPOSE:		Make sure the selected class is available, and change
//				the character if it needs to be for a specific game type.
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::ValidateClassAndCharacter(HCLIENT hClient, MPClientData *pData)
{
	switch(m_GameInfo.m_nGameType.Get())
	{
		case MP_DM:
		case MP_TEAMDM:
		{
			// Make sure all aliens get spawned as a facehugger if it's a lifecycle game...
			if((pData->m_nCharacterClass == Alien) && m_GameInfo.m_bLifecycle)
			{
				pData->m_nCharacterSet[Alien] = g_pCharacterButeMgr->GetSetFromModelType("Facehugger_MP");
			}

			// Now check the team limits to see if this player needs to get put on a different team
			if(NumberOfClass(pData->m_nCharacterClass, LTTRUE) >= m_GameInfo.m_nTeamLimits[pData->m_nCharacterClass])
			{
				// Alright... there's too many of this class... move this player to another team.
				uint8 nNewClass = Alien; // Alien will be our last resort

				if(NumberOfClass(Marine, LTTRUE) < m_GameInfo.m_nTeamLimits[Marine])
					nNewClass = Marine;
				else if(NumberOfClass(Predator, LTTRUE) < m_GameInfo.m_nTeamLimits[Predator])
					nNewClass = Predator;
				else if(NumberOfClass(Corporate, LTTRUE) < m_GameInfo.m_nTeamLimits[Corporate])
					nNewClass = Corporate;

				// If things are different (which they should be)... then tell the player what happened
				pData->m_bAutoTeamSwitched = (pData->m_nCharacterClass != nNewClass);

				// Ok... we should have found something by now... so switch 'em!
				pData->m_nCharacterClass = nNewClass;
			}

			return LTTRUE;
		}

		case MP_HUNT:
		{
			// Count the number of hunters and prey in the level
			int nHunters = NumberOfClass(m_GameInfo.m_nHunt_HunterRace, LTTRUE);
			int nPrey = NumberOfClass(m_GameInfo.m_nHunt_PreyRace, LTTRUE);

			// Now figure out what this new player should start as...
			if(nHunters < 1)
			{
				pData->m_nCharacterClass = m_GameInfo.m_nHunt_HunterRace;
			}
			else
			{
				if(((nHunters + 1) * m_GameInfo.m_nHunt_Ratio) > nPrey)
					pData->m_nCharacterClass = m_GameInfo.m_nHunt_PreyRace;
				else
					pData->m_nCharacterClass = m_GameInfo.m_nHunt_HunterRace;
			}

			return LTTRUE;
		}

		case MP_SURVIVOR:
		{
			// Everyone starts as a Survivor for this mode...
			pData->m_nCharacterClass = m_GameInfo.m_nSurv_SurvivorRace;
			return LTTRUE;
		}

		case MP_OVERRUN:
		case MP_EVAC:
		{
			// If this client tries to start as a class that's not supported... then
			// switch them to the class that has the least amount of clients
			if((pData->m_nCharacterClass != m_GameInfo.m_nTM_DefenderRace) && (pData->m_nCharacterClass != m_GameInfo.m_nTM_AttackerRace))
			{
				int nDefenders = NumberOfClass(m_GameInfo.m_nTM_DefenderRace, LTTRUE);
				int nAttackers = NumberOfClass(m_GameInfo.m_nTM_AttackerRace, LTTRUE);

				if(nDefenders <= nAttackers)
					pData->m_nCharacterClass = m_GameInfo.m_nTM_DefenderRace;
				else
					pData->m_nCharacterClass = m_GameInfo.m_nTM_AttackerRace;
			}

			return LTTRUE;
		}
	}

	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ValidateRespawn()
//
// PURPOSE:		Make sure the player respawns as an appropriate character.
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::ValidateRespawn(CPlayerObj *pPlayer, int* pCharId)
{
	// If the player is invalid... just return
	if(!pPlayer)
		return LTTRUE;

	// Get the handle to this client
	HCLIENT hClient = pPlayer->GetClient();

	if(!hClient)
		return LTTRUE;

	// Now find the data for this player
	int nIndex = FindClient(hClient);

	if(nIndex == -1)
		return LTTRUE;

	// If we are an Exosuit, make sure we get reset...
	if(IsExosuit(pPlayer->m_hObject))
	{
		if(pCharId)
			*pCharId = (int)m_ClientData[nIndex].m_Data.m_nCharacterSet[m_ClientData[nIndex].m_Data.m_nCharacterClass];
		else
			pPlayer->SetCharacter(m_ClientData[nIndex].m_Data.m_nCharacterSet[m_ClientData[nIndex].m_Data.m_nCharacterClass], LTTRUE);
	}

	// If we are a queen make sure we get reset...
	int nQueenCharSet = g_pCharacterButeMgr->GetSetFromModelType("queen");

	if(pPlayer->GetCharacter() == nQueenCharSet)
	{
		if(pCharId)
			*pCharId = (int)m_ClientData[nIndex].m_Data.m_nCharacterSet[m_ClientData[nIndex].m_Data.m_nCharacterClass];
		else
			pPlayer->SetCharacter(m_ClientData[nIndex].m_Data.m_nCharacterSet[m_ClientData[nIndex].m_Data.m_nCharacterClass], LTTRUE);
	}


	switch(m_GameInfo.m_nGameType.Get())
	{
		case MP_DM:
		case MP_TEAMDM:
		{
			// Handle switching aliens back to Facehuggers if the lifecycle option is on...
			if((m_ClientData[nIndex].m_Data.m_nCharacterClass == Alien) && m_GameInfo.m_bLifecycle)
			{
				// only reset us to the facehugger if we are not about
				// to spawn as a queen.
				if(!m_ClientData[nIndex].m_bSpawnAsQueen)
				{
					int nCharacterSet = g_pCharacterButeMgr->GetSetFromModelType("Facehugger_MP");

					if(pCharId)
						*pCharId = nCharacterSet;
					else
						pPlayer->SetCharacter(nCharacterSet, LTTRUE);

					ChangeClientCharacter(hClient, (uint8)nCharacterSet, LTFALSE);
				}
			}

			break;
		}

		case MP_OVERRUN:
		{
			// Do some special things if this is the first respawn into this game type...
			if(m_ClientData[nIndex].m_bFirstRespawn && !m_ClientData[nIndex].m_bObserving)
			{
				// Start with no lives until we get put into the game...
				m_ClientData[nIndex].m_nLives = 0;
				m_ClientData[nIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_LIVES;

				if(m_nServerSubState == MPMGR_SUBSTATE_TM_GOING)
					m_ClientData[nIndex].m_nObserveMode = MP_OBSERVE_MODE_WAIT_NEXT_ROUND;
				else
					m_ClientData[nIndex].m_nObserveMode = MP_OBSERVE_MODE_WAIT_START_GAME;

				// Always put us in observe mode when first starting the game...
				ClientObserveCommand(hClient, MP_OBSERVE_ON);
				return LTFALSE;
			}


			// If we're out of lives... let them observe until the next round or level
			if(m_ClientData[nIndex].m_nLives <= 0)
			{
				m_ClientData[nIndex].m_nObserveMode = MP_OBSERVE_MODE_WAIT_NEXT_ROUND;
				ClientObserveCommand(hClient, MP_OBSERVE_ON);
				return LTFALSE;
			}

			break;
		}

		case MP_EVAC:
		{
			// Do some special things if this is the first respawn into this game type...
			if(m_ClientData[nIndex].m_bFirstRespawn && !m_ClientData[nIndex].m_bObserving)
			{
				// Start with no lives until we get put into the game...
				m_ClientData[nIndex].m_nLives = 0;

				if(m_nServerSubState == MPMGR_SUBSTATE_TM_GOING)
					m_ClientData[nIndex].m_nObserveMode = MP_OBSERVE_MODE_WAIT_NEXT_ROUND;
				else
					m_ClientData[nIndex].m_nObserveMode = MP_OBSERVE_MODE_WAIT_START_GAME;

				// Always put us in observe mode when first starting the game...
				ClientObserveCommand(hClient, MP_OBSERVE_ON);
				return LTFALSE;
			}

			// If we're out of lives... let them observe until the next round or level
			if(m_ClientData[nIndex].m_nLives <= 0)
			{
				m_ClientData[nIndex].m_nObserveMode = MP_OBSERVE_MODE_WAIT_NEXT_ROUND;
				ClientObserveCommand(hClient, MP_OBSERVE_ON);
				return LTFALSE;
			}

			break;
		}
	}


	// We've respawned at least once...
	m_ClientData[nIndex].m_bFirstRespawn = LTFALSE;


	// Make sure we store the observe state
	m_ClientData[nIndex].m_bObserving = LTFALSE;


	// Inform all clients that this player has respawned... so they can reset
	// anything that might need to be reset
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_RESPAWN);
	hMessage->WriteDWord(m_ClientData[nIndex].m_nClientID);
	g_pLTServer->EndMessage(hMessage);


	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ClientKilledMsg()
//
// PURPOSE:		Handle the death of a client
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ClientKilledMsg(HCLIENT hVictim, HCLIENT hKiller, uint8 nFlags)
{
	// Get the indexes for each of these clients...
	int nVIndex = FindClient(hVictim);
	int nKIndex = FindClient(hKiller);

	// If either of these clients is invalid... just leave
	if((nVIndex == -1) || (nKIndex == -1))
		return;


	// Always clear out the consecutive frags for the victim
	m_ClientData[nVIndex].m_nConsecutiveFrags = 0;
	m_ClientData[nVIndex].m_bSpawnAsQueen = 0;
	m_ClientData[nVIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_CONFRAGS;

	// See if the victim was a queen..
	{
		CPlayerObj *pPlayer = FindPlayerObj(m_ClientData[nVIndex].m_hClient);

		if(pPlayer)
		{
			int nQueenCharSet = g_pCharacterButeMgr->GetSetFromModelType("queen");

			if(pPlayer->GetCharacter() == nQueenCharSet)
			{
				// Tell all the clients that the Queen is dead
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
				hMessage->WriteByte(MP_QUEEN_DEATH);
				hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
				g_pLTServer->EndMessage(hMessage);
			}
		}
	}

	switch(m_GameInfo.m_nGameType.Get())
	{
		case MP_DM:			ClientKilledMsg_DM(nVIndex, nKIndex, nFlags);			break;
		case MP_TEAMDM:		ClientKilledMsg_TeamDM(nVIndex, nKIndex, nFlags);		break;
		case MP_HUNT:		ClientKilledMsg_Hunt(nVIndex, nKIndex, nFlags);			break;
		case MP_SURVIVOR:	ClientKilledMsg_Survivor(nVIndex, nKIndex, nFlags);		break;
		case MP_OVERRUN:	ClientKilledMsg_Overrun(nVIndex, nKIndex, nFlags);		break;
		case MP_EVAC:		ClientKilledMsg_Evac(nVIndex, nKIndex, nFlags);			break;
	}


	// ALM (6/24/01) -- Just put this here since just about everything causes a need
	// for an update.  And this way it doesn't have to be in all the sub functions.

	// Flag an update to happen.
	SetUpdateGameServ( );
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ClientKilledMsg_DM()
//
// PURPOSE:		Handle the death of a client
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ClientKilledMsg_DM(int nVIndex, int nKIndex, uint8 nFlags)
{
	// If this client killed himself... punish the poor bastard
	if(nVIndex == nKIndex)
	{
		m_ClientData[nKIndex].m_nFrags--;
		m_ClientData[nKIndex].m_nScore -= GetClientScore(m_ClientData[nVIndex].m_nClientID);
		m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_FRAGS | MP_CLIENT_UPDATE_SCORE;

		// Now tell all the clients that you fucked up
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SERV_MSG_KILLED_HIMSELF);
		hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

		return;
	}


	// If this client killed a team member... brand the traitor!
	if(m_GameInfo.m_nGameType.IsTeamMode())
	{
		if(m_ClientData[nVIndex].m_Data.m_nCharacterClass == m_ClientData[nKIndex].m_Data.m_nCharacterClass)
		{
			m_ClientData[nKIndex].m_nFrags--;
			m_ClientData[nKIndex].m_nConsecutiveFrags = 0;
			m_ClientData[nKIndex].m_nScore -= GetClientScore(m_ClientData[nVIndex].m_nClientID);
			m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_FRAGS | MP_CLIENT_UPDATE_SCORE | MP_CLIENT_UPDATE_CONFRAGS;

			// Now tell all the clients that you're a traitor
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
			hMessage->WriteByte(MP_SERV_MSG_KILLED_TEAMMATE);
			hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
			hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

			return;
		}
	}


	// Good kill bloke!!
	m_ClientData[nKIndex].m_nFrags++;
	IncrementConsecutiveFrags(nKIndex);
	m_ClientData[nKIndex].m_nScore += GetClientScore(m_ClientData[nVIndex].m_nClientID);
	m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_FRAGS | MP_CLIENT_UPDATE_SCORE | MP_CLIENT_UPDATE_CONFRAGS;

	// Now tell all the clients that you owned his ass
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
	hMessage->WriteByte(MP_SERV_MSG_KILLED_SOMEONE);
	hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
	hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ClientKilledMsg_TeamDM()
//
// PURPOSE:		Handle the death of a client
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ClientKilledMsg_TeamDM(int nVIndex, int nKIndex, uint8 nFlags)
{
	int nVTeam = m_ClientData[nVIndex].m_Data.m_nCharacterClass;
	int nKTeam = m_ClientData[nKIndex].m_Data.m_nCharacterClass;

	// If this client killed himself... punish the poor bastard
	if(nVIndex == nKIndex)
	{
		m_ClientData[nKIndex].m_nFrags--;
		m_ClientData[nKIndex].m_nScore -= GetClientScore(m_ClientData[nVIndex].m_nClientID);
		m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_FRAGS | MP_CLIENT_UPDATE_SCORE;

		m_TeamData[nKTeam].m_nFrags--;
		m_TeamData[nKTeam].m_nScore -= GetClientScore(m_ClientData[nVIndex].m_nClientID);
		m_TeamData[nKTeam].m_nUpdateFlags |= MP_CLIENT_UPDATE_FRAGS | MP_CLIENT_UPDATE_SCORE;

		// Now tell all the clients that you fucked up
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SERV_MSG_KILLED_HIMSELF);
		hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

		return;
	}


	// If this client killed a team member... brand the traitor!
	if(m_GameInfo.m_nGameType.IsTeamMode())
	{
		if(m_ClientData[nVIndex].m_Data.m_nCharacterClass == m_ClientData[nKIndex].m_Data.m_nCharacterClass)
		{
			m_ClientData[nKIndex].m_nFrags--;
			m_ClientData[nKIndex].m_nConsecutiveFrags = 0;
			m_ClientData[nKIndex].m_nScore -= GetClientScore(m_ClientData[nVIndex].m_nClientID);
			m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_FRAGS | MP_CLIENT_UPDATE_SCORE | MP_CLIENT_UPDATE_CONFRAGS;

			m_TeamData[nKTeam].m_nFrags--;
			m_TeamData[nKTeam].m_nScore -= GetClientScore(m_ClientData[nVIndex].m_nClientID);
			m_TeamData[nKTeam].m_nUpdateFlags |= MP_CLIENT_UPDATE_FRAGS | MP_CLIENT_UPDATE_SCORE;

			// Now tell all the clients that you're a traitor
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
			hMessage->WriteByte(MP_SERV_MSG_KILLED_TEAMMATE);
			hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
			hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

			return;
		}
	}


	// Good kill bloke!!
	m_ClientData[nKIndex].m_nFrags++;
	IncrementConsecutiveFrags(nKIndex);
	m_ClientData[nKIndex].m_nScore += GetClientScore(m_ClientData[nVIndex].m_nClientID);
	m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_FRAGS | MP_CLIENT_UPDATE_SCORE | MP_CLIENT_UPDATE_CONFRAGS;

	m_TeamData[nKTeam].m_nFrags++;
	m_TeamData[nKTeam].m_nScore += GetClientScore(m_ClientData[nVIndex].m_nClientID);
	m_TeamData[nKTeam].m_nUpdateFlags |= MP_CLIENT_UPDATE_FRAGS | MP_CLIENT_UPDATE_SCORE;


	// Now tell all the clients that you owned his ass
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
	hMessage->WriteByte(MP_SERV_MSG_KILLED_SOMEONE);
	hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
	hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ClientKilledMsg_Hunt()
//
// PURPOSE:		Handle the death of a client
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ClientKilledMsg_Hunt(int nVIndex, int nKIndex, uint8 nFlags)
{
	// If this client killed himself... punish the poor bastard
	if(nVIndex == nKIndex)
	{
		m_ClientData[nKIndex].m_nFrags--;
		m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_FRAGS;

		// Now tell all the clients that you fucked up
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SERV_MSG_KILLED_HIMSELF);
		hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

		return;
	}


	// If this client killed a team member... brand the traitor!
	if(m_ClientData[nVIndex].m_Data.m_nCharacterClass == m_ClientData[nKIndex].m_Data.m_nCharacterClass)
	{
		m_ClientData[nKIndex].m_nFrags--;
		m_ClientData[nKIndex].m_nConsecutiveFrags = 0;
		m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_FRAGS | MP_CLIENT_UPDATE_CONFRAGS;

		// Now tell all the clients that you're a traitor
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SERV_MSG_KILLED_TEAMMATE);
		hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
		hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

		return;
	}


	// Now finally... if one of the Prey killed a Hunter... switch 'em
	if((m_ClientData[nKIndex].m_Data.m_nCharacterClass == m_GameInfo.m_nHunt_PreyRace) && (m_ClientData[nVIndex].m_Data.m_nCharacterClass == m_GameInfo.m_nHunt_HunterRace))
	{
		// Now tell all the clients that someone is mutating
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_HUNT_MUTATE_SWITCH);
		hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
		hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

		// Setup the players to switch
		m_ClientData[nKIndex].m_nNextClass = m_GameInfo.m_nHunt_HunterRace;
		m_ClientData[nKIndex].m_fMiscTimer = g_pLTServer->GetTime();

		m_ClientData[nVIndex].m_nNextClass = m_GameInfo.m_nHunt_PreyRace;
		m_ClientData[nVIndex].m_fMiscTimer = g_pLTServer->GetTime();
	}
	// If the killer was a Hunter... give him a biscuit and scratch behind his ear.
	else if((m_ClientData[nKIndex].m_Data.m_nCharacterClass == m_GameInfo.m_nHunt_HunterRace) && (m_ClientData[nVIndex].m_Data.m_nCharacterClass == m_GameInfo.m_nHunt_PreyRace))
	{
		m_ClientData[nKIndex].m_nFrags++;
		IncrementConsecutiveFrags(nKIndex);
		m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_FRAGS | MP_CLIENT_UPDATE_CONFRAGS;
	}


	// Now tell all the clients that you owned his ass
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
	hMessage->WriteByte(MP_SERV_MSG_KILLED_SOMEONE);
	hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
	hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ClientKilledMsg_Survivor()
//
// PURPOSE:		Handle the death of a client
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ClientKilledMsg_Survivor(int nVIndex, int nKIndex, uint8 nFlags)
{
	// If this client killed himself... punish the poor bastard
	if(nVIndex == nKIndex)
	{
		// Now tell all the clients that you fucked up
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SERV_MSG_KILLED_HIMSELF);
		hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

		// If a Survivor killed himself... mutate him!
		if(m_ClientData[nVIndex].m_Data.m_nCharacterClass == m_GameInfo.m_nSurv_SurvivorRace)
		{
			// Tell all the clients that someone is mutating
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
			hMessage->WriteByte(MP_SURVIVOR_MUTATE);
			hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

			m_ClientData[nVIndex].m_nNextClass = m_GameInfo.m_nSurv_MutateRace;
			m_ClientData[nVIndex].m_fMiscTimer = g_pLTServer->GetTime();
		}

		return;
	}


	// If the killer and victim were the same race...
	if(m_ClientData[nVIndex].m_Data.m_nCharacterClass == m_ClientData[nKIndex].m_Data.m_nCharacterClass)
	{
		// Make sure we're in tag mode
		if(m_nServerSubState == MPMGR_SUBSTATE_SURVIVOR_TAG)
		{
			IncrementConsecutiveFrags(nKIndex);
			m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_CONFRAGS;

			// Tell all the clients that someone is mutating
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
			hMessage->WriteByte(MP_SURVIVOR_MUTATE);
			hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

			// Now convert the victim to the mutate race
			m_ClientData[nVIndex].m_nNextClass = m_GameInfo.m_nSurv_MutateRace;
			m_ClientData[nVIndex].m_fMiscTimer = g_pLTServer->GetTime();

			// Now tell all the clients that you owned his ass
			hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
			hMessage->WriteByte(MP_SERV_MSG_KILLED_SOMEONE);
			hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
			hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
		}
		else
		{
			m_ClientData[nKIndex].m_nConsecutiveFrags = 0;
			m_ClientData[nKIndex].m_nScore += MPMGR_SURVIVOR_FKILL_PTS;
			m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_SCORE | MP_CLIENT_UPDATE_CONFRAGS;

			// Now tell all the clients that you're a traitor
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
			hMessage->WriteByte(MP_SERV_MSG_KILLED_TEAMMATE);
			hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
			hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
		}

		return;
	}


	// Make sure we're in cut throat mode for the rest of the frag checks
	if(m_nServerSubState == MPMGR_SUBSTATE_SURVIVOR_CUT)
	{
		// If the killer was an mutant, and the victim a survivor...
		if((m_ClientData[nKIndex].m_Data.m_nCharacterClass == m_GameInfo.m_nSurv_MutateRace) && (m_ClientData[nVIndex].m_Data.m_nCharacterClass == m_GameInfo.m_nSurv_SurvivorRace))
		{
			// Give the mutant a biscuit and scratch behind his ear...
			IncrementConsecutiveFrags(nKIndex);
			m_ClientData[nKIndex].m_nScore += MPMGR_SURVIVOR_MKILL_PTS;
			m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_SCORE | MP_CLIENT_UPDATE_CONFRAGS;

			// Now tell all the clients that someone is mutating
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
			hMessage->WriteByte(MP_SURVIVOR_MUTATE);
			hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

			// Now convert the survivor
			m_ClientData[nVIndex].m_nNextClass = m_GameInfo.m_nSurv_MutateRace;
			m_ClientData[nVIndex].m_fMiscTimer = g_pLTServer->GetTime();
		}
		else
		{
			IncrementConsecutiveFrags(nKIndex);
			m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_CONFRAGS;
		}


		// Now tell all the clients that you owned his ass
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SERV_MSG_KILLED_SOMEONE);
		hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
		hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ClientKilledMsg_Overrun()
//
// PURPOSE:		Handle the death of a client
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ClientKilledMsg_Overrun(int nVIndex, int nKIndex, uint8 nFlags)
{
	// Decrement the lives of our victim
	m_ClientData[nVIndex].m_nLives--;
	m_ClientData[nVIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_LIVES;


	// If this client killed himself... punish the poor bastard
	if(nVIndex == nKIndex)
	{
		// Now tell all the clients that you fucked up
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SERV_MSG_KILLED_HIMSELF);
		hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

		return;
	}


	// If this client killed a team member... brand the traitor!
	if(m_ClientData[nVIndex].m_Data.m_nCharacterClass == m_ClientData[nKIndex].m_Data.m_nCharacterClass)
	{
		m_ClientData[nKIndex].m_nConsecutiveFrags = 0;
		m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_CONFRAGS;

		// Now tell all the clients that you're a traitor
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SERV_MSG_KILLED_TEAMMATE);
		hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
		hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

		return;
	}


	IncrementConsecutiveFrags(nKIndex);
	m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_CONFRAGS;

	// Now tell all the clients that you owned his ass
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
	hMessage->WriteByte(MP_SERV_MSG_KILLED_SOMEONE);
	hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
	hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ClientKilledMsg_Evac()
//
// PURPOSE:		Handle the death of a client
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ClientKilledMsg_Evac(int nVIndex, int nKIndex, uint8 nFlags)
{
	// Decrement the lives of our victim
	m_ClientData[nVIndex].m_nLives--;
	m_ClientData[nVIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_LIVES;


	// If this client killed himself... punish the poor bastard
	if(nVIndex == nKIndex)
	{
		// Now tell all the clients that you fucked up
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SERV_MSG_KILLED_HIMSELF);
		hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

		return;
	}


	// If this client killed a team member... brand the traitor!
	if(m_ClientData[nVIndex].m_Data.m_nCharacterClass == m_ClientData[nKIndex].m_Data.m_nCharacterClass)
	{
		m_ClientData[nKIndex].m_nConsecutiveFrags = 0;
		m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_CONFRAGS;

		// Now tell all the clients that you're a traitor
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SERV_MSG_KILLED_TEAMMATE);
		hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
		hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

		return;
	}


	IncrementConsecutiveFrags(nKIndex);
	m_ClientData[nKIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_CONFRAGS;

	// Now tell all the clients that you owned his ass
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
	hMessage->WriteByte(MP_SERV_MSG_KILLED_SOMEONE);
	hMessage->WriteDWord(m_ClientData[nKIndex].m_nClientID);
	hMessage->WriteDWord(m_ClientData[nVIndex].m_nClientID);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ClientPickupMsg()
//
// PURPOSE:		Handle a special pickup message
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ClientPickupMsg(HCLIENT hClient, uint8 nMsg)
{
	// Get the index of the client...
	int nIndex = FindClient(hClient);

	// If the client is invalid... just leave
	if(nIndex == -1)
		return;

	// If the time is too early... also leave
	if(g_pLTServer->GetTime() < m_ClientData[nIndex].m_fPickupMsgTime)
		return;

	// Make sure this is a valid pickup message
	switch(nMsg)
	{
		case MP_PICKUP_MSG_CANT_USE_WEAPON:
		case MP_PICKUP_MSG_CANT_USE_AMMO:
			break;

		default:
			return;
	}

	// Now tell the client the message
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_ClientData[nIndex].m_hClient, MID_MPMGR_SERVER_MESSAGE);
	hMessage->WriteByte(nMsg);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

	m_ClientData[nIndex].m_fPickupMsgTime = g_pLTServer->GetTime() + 5.0f;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ChangeClientCharacter()
//
// PURPOSE:		Change the character type of this client...
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ChangeClientCharacter(HCLIENT hClient, uint8 nCharacterSet, LTBOOL bDisplay)
{
	// Find the client
	int nIndex = FindClient(hClient);
	if(nIndex == -1) return;


	// Set this player to the new character type
	uint8 nCharacterClass = Marine;
	CharacterClass eClass = g_pCharacterButeMgr->GetClass(nCharacterSet);

	switch(eClass)
	{
		case ALIEN:			nCharacterClass = Alien;		break;
		case MARINE:		nCharacterClass = Marine;		break;
		case PREDATOR:		nCharacterClass = Predator;		break;
		case CORPORATE:		nCharacterClass = Corporate;	break;
	}


	m_ClientData[nIndex].m_Data.m_nCharacterClass = nCharacterClass;
	m_ClientData[nIndex].m_Data.m_nCharacterSet[nCharacterClass] = nCharacterSet;


	// Send a message to everyone telling that this player has changed...
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_CHANGE_PLAYER);
	hMessage->WriteDWord(m_ClientData[nIndex].m_nClientID);
	hMessage->WriteByte(nCharacterClass);
	hMessage->WriteByte(nCharacterSet);
	hMessage->WriteByte(bDisplay);
	g_pLTServer->EndMessage(hMessage);


	// Grab the client data stored in Lithtech and update the character settings...
	void *pData;
	uint32 nLength;
	g_pLTServer->GetClientData(hClient, pData, nLength);

	if(nLength == sizeof(MPClientData))
	{
		MPClientData *pMPData = (MPClientData*)pData;

		pMPData->m_nCharacterClass = nCharacterClass;
		pMPData->m_nCharacterSet[nCharacterClass] = nCharacterSet;
	}


	SetUpdateGameServ( );
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ChangeClientClass()
//
// PURPOSE:		Change the class / team of this client...
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ChangeClientClass(HCLIENT hClient, uint8 nCharacterClass, LTBOOL bDisplay)
{
	// Find the client
	int nIndex = FindClient(hClient);
	if(nIndex == -1) return;


	// Set this player to the new character
	m_ClientData[nIndex].m_Data.m_nCharacterClass = nCharacterClass;


	// Send a message to everyone telling that this player has changed...
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_CHANGE_PLAYER);
	hMessage->WriteDWord(m_ClientData[nIndex].m_nClientID);
	hMessage->WriteByte(nCharacterClass);
	hMessage->WriteByte(m_ClientData[nIndex].m_Data.m_nCharacterSet[nCharacterClass]);
	hMessage->WriteByte(bDisplay);
	g_pLTServer->EndMessage(hMessage);


	// Grab the client data stored in Lithtech and update the character settings...
	void *pData;
	uint32 nLength;
	g_pLTServer->GetClientData(hClient, pData, nLength);

	if(nLength == sizeof(MPClientData))
	{
		MPClientData *pMPData = (MPClientData*)pData;

		pMPData->m_nCharacterClass = nCharacterClass;
	}


	SetUpdateGameServ( );
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ClientCheating()
//
// PURPOSE:		Notify all other clients that someone is cheating...
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ClientCheating(HCLIENT hClient)
{
	// If our game type is not of a multiplayer variety... then skip this
	if(!GetGameType().IsMultiplayer())
		return;


	// Find the client who sent it...
	int nIndex = FindClient(hClient);

	if(nIndex != -1)
	{
		for(uint8 i = 0; i < MAX_CLIENTS; i++)
		{
			if((m_ClientData[i].m_hClient != LTNULL) && (m_ClientData[i].m_hClient != hClient))
			{
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_ClientData[i].m_hClient, MID_MPMGR_SERVER_MESSAGE);
				hMessage->WriteByte(MP_CLIENT_CHEATING);
				hMessage->WriteDWord(m_ClientData[nIndex].m_nClientID);
				g_pLTServer->EndMessage(hMessage);
			}
		}
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ServerAppMessageFn()
//
// PURPOSE:		Dedicated server control...
//
// -------------------------------------------------------------------- //

LTRESULT MultiplayerMgr::ServerAppMessageFn(char *pMsg, int nLen)
{
	// Check inputs.
	if( !pMsg || nLen < 1 )
	{
		ASSERT( FALSE );
		return LT_ERROR;
	}

	// Convert to a stream so we can work with the data.
	istrstream memStream( pMsg, nLen );

	// Get the message id.
	char nVal;
	memStream >> nVal;

	// Handle the message.
	switch( nVal )
	{
		case SERVERSHELL_INIT:
		{
			// We're being hosted by the serverapp.
			m_bGameServHosted = LTTRUE;

			// Start off just playing.
			m_nServerState = MPMGR_STATE_PLAYING;

			// Start with the first level.
			m_nCurrentLevel = 0;

			// Load the level.
			LTRESULT res = LoadLevel( m_GameInfo.m_szLevels[m_nCurrentLevel] );
			if( res != LT_OK )
				return res;

			break;
		}
		case SERVERSHELL_NEXTWORLD:
		{
			ProcessServerCommand( "nextlevel");
			break;
		}
		case SERVERSHELL_SETWORLD:
		{
			// Read in the world index.
			uint32 nNextLevel;
			memStream >> nNextLevel;
			m_nNextLevel = ( uint8 )( Min( nNextLevel, ( uint32 )255 ));
			
			// The nextlevel command advances the m_nNextLevel by one, so
			// we need to undo that here.
			m_nNextLevel--;

			ProcessServerCommand( "nextlevel");
			break;
		}
		case SERVERSHELL_MESSAGE:
		{
			char szMsg[256];
			memStream.read(szMsg, 256);

			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_MESSAGE);
			hMessage->WriteDWord(-1);
			hMessage->WriteString(szMsg);
			g_pLTServer->EndMessage(hMessage);

			break;
		}
		// Invalid message.
		default:
		{
			ASSERT( FALSE );
			break;
		}
	}

	return LT_OK;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::OnMessage()
//
// PURPOSE:		You recieved a message! <play that little Outlook message sound here>
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::OnMessage(HCLIENT hSender, uint8 nMessage, HMESSAGEREAD hRead)
{
	switch(nMessage)
	{
		case MID_MPMGR_MESSAGE:
		{
			// Retrieve the string they sent
			char szString[256];
			szString[0] = 0;
			hRead->ReadStringFL(szString, sizeof(szString));

			// Find the client who sent it...
			int nIndex = FindClient(hSender);

			if(nIndex != -1)
			{
				// Now send the string to all clients
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_MESSAGE);
				hMessage->WriteDWord(m_ClientData[nIndex].m_nClientID);
				hMessage->WriteString(szString);
				g_pLTServer->EndMessage(hMessage);

				ServerAppMessage(szString, nIndex);
			}

			return LTTRUE;
		}

		case MID_MPMGR_TEAM_MESSAGE:
		{
			// Retrieve the string they sent
			char szString[256];
			szString[0] = 0;
			hRead->ReadStringFL(szString, sizeof(szString));

			// Find the client who sent it...
			int nIndex = FindClient(hSender);

			if(nIndex != -1)
			{
				// Go through all the clients on this persons team and send the message to them
				for(uint8 i = 0; i < MAX_CLIENTS; i++)
				{
					if(m_ClientData[i].m_hClient != LTNULL)
					{
						if(m_ClientData[i].m_Data.m_nCharacterClass == m_ClientData[nIndex].m_Data.m_nCharacterClass)
						{
							// Now send the string to this client
							HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_ClientData[i].m_hClient, MID_MPMGR_TEAM_MESSAGE);
							hMessage->WriteDWord(m_ClientData[nIndex].m_nClientID);
							hMessage->WriteString(szString);
							g_pLTServer->EndMessage(hMessage);
						}
					}
				}
			}

			return LTTRUE;
		}

		case MID_MPMGR_PRIVATE_MESSAGE:
		{
			// Retrieve the string they sent
			char szString[256];
			szString[0] = 0;
			uint32 nSendToID;

			hRead->ReadDWordFL(nSendToID);
			hRead->ReadStringFL(szString, sizeof(szString));

			// Find the client who sent it...
			int nIndex1 = FindClient(hSender);
			int nIndex2 = FindClient(nSendToID);

			if((nIndex1 != -1) && (nIndex2 != -1))
			{
				// Now send the string to all clients
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_ClientData[nIndex2].m_hClient, MID_MPMGR_PRIVATE_MESSAGE);
				hMessage->WriteDWord(m_ClientData[nIndex1].m_nClientID);
				hMessage->WriteString(szString);
				g_pLTServer->EndMessage(hMessage);
			}

			return LTTRUE;
		}

		case MID_MPMGR_NAME_CHANGE:
		{
			// Find the client who sent it...
			int nIndex = FindClient(hSender);

			// Read in the string
			char szString[256];
			szString[0] = 0;

			hRead->ReadStringFL(szString, sizeof(szString));


			if(nIndex != -1)
			{
				// Use 21 since that's what the name interface allows...
				memset(m_ClientData[nIndex].m_Data.m_szName, 0, MAX_MP_CLIENT_NAME_LENGTH);
				strncpy(m_ClientData[nIndex].m_Data.m_szName, szString, 21);

				// Make sure the name is unique
				CreateUniqueClientName(m_ClientData[nIndex].m_hClient);
				SetUpdateGameServ();


				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
				hMessage->WriteByte(MP_NAME_CHANGE);
				hMessage->WriteDWord(m_ClientData[nIndex].m_nClientID);
				hMessage->WriteString(m_ClientData[nIndex].m_Data.m_szName);
				g_pLTServer->EndMessage(hMessage);
			}

			return LTTRUE;
		}

		case MID_MPMGR_SERVER_COMMAND:
		{
			// Find the client who sent it...
			int nIndex = FindClient(hSender);

			// Read in the string
			char szString[256];
			szString[0] = 0;

			hRead->ReadStringFL(szString, sizeof(szString));

			ProcessServerCommand(szString, LTTRUE, nIndex);

			return LTTRUE;
		}

		case MID_MPMGR_NEXT_LEVEL:
		{
//			// Find the client who sent it...
//			int nIndex = FindClient(hSender);
//
//			if(nIndex != -1)
//			{
//				// Set the verification value of this client
//				m_ClientData[nIndex].m_bVerified = LTTRUE;
//
//				// Now send a message down to everyone telling who verified
//				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_NEXT_LEVEL);
//				hMessage->WriteByte(MPMGR_NEXT_LEVEL_ACCEPTED);
//				hMessage->WriteDWord(m_ClientData[nIndex].m_nClientID);
//				g_pLTServer->EndMessage(hMessage);
//			}
//
			return LTTRUE;
		}

		case MID_MPMGR_LOAD_LEVEL:
		{
			// Find the client who sent it...
			int nIndex = FindClient(hSender);

			if(nIndex != -1)
			{
				// Set the verification value of this client
				m_ClientData[nIndex].m_bVerified = LTTRUE;
			}

			return LTTRUE;
		}

		case MID_MPMGR_CHANGE_PLAYER:
		{
			// Read in the new character setting
			uint8 nCharacterSet;
			hRead->ReadByteFL(nCharacterSet);


			// Get the team character class value from the Bute class value (yes, yes... confusing I know)
			uint8 nCharacterClass = Marine;
			CharacterClass eClass = g_pCharacterButeMgr->GetClass(nCharacterSet);

			switch(eClass)
			{
				case ALIEN:			nCharacterClass = Alien;		break;
				case MARINE:		nCharacterClass = Marine;		break;
				case PREDATOR:		nCharacterClass = Predator;		break;
				case CORPORATE:		nCharacterClass = Corporate;	break;
			}


			// Find the client who sent it...
			int nIndex = FindClient(hSender);

			if(nIndex != -1)
			{
				// If we are about to be a queen then ignore this...
				if(m_ClientData[nIndex].m_bSpawnAsQueen)
					return LTTRUE;

				// If there's too many of this class... don't allow them to switch
				if(	(NumberOfClass(nCharacterClass, LTTRUE) >= m_GameInfo.m_nTeamLimits[nCharacterClass]) &&
					(nCharacterClass != m_ClientData[nIndex].m_Data.m_nCharacterClass))
				{
					HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hSender, MID_MPMGR_SERVER_MESSAGE);
					hMessage->WriteByte(MP_TEAM_NOT_AVAILABLE);
					g_pLTServer->EndMessage(hMessage);
				}
				else
				{
					LTBOOL bBypass = LTFALSE;

					// Make sure to respawn as a facehugger if it's a lifecycle game
					if((nCharacterClass == Alien) && m_GameInfo.m_bLifecycle)
					{
						nCharacterSet = g_pCharacterButeMgr->GetSetFromModelType("Facehugger_MP");
						bBypass = LTTRUE;
					}


					// Make sure this is a valid character type for multiplayer... that way people can't
					// cheat to new characters from the client
					CString str = g_pCharacterButeMgr->GetDefaultModel(nCharacterSet);

					if(!bBypass && (str.IsEmpty() || str.Find(".abc") < 0))
					{
						// Send an error message down to the client who sent this message...
					}
					else
					{
						// Find this player
						CPlayerObj *pPlayer = FindPlayerObj(hSender);

						if(pPlayer)
						{
							// Get the user flags...
							uint32 nUserFlags = g_pLTServer->GetObjectUserFlags(pPlayer->m_hObject);

							if(!(nUserFlags & USRFLG_CHAR_FACEHUG))
							{
								// If we're in a team game... this will take away all our lives.
								if((m_GameInfo.m_nGameType == MP_OVERRUN) || (m_GameInfo.m_nGameType == MP_EVAC))
								{
									m_ClientData[nIndex].m_nLives = 0;
									m_ClientData[nIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_LIVES;
								}


								// Reset this player's frags and score...
								if(m_ClientData[nIndex].m_nFrags > 0) m_ClientData[nIndex].m_nFrags = 0;
								if(m_ClientData[nIndex].m_nScore > 0) m_ClientData[nIndex].m_nScore = 0;
								m_ClientData[nIndex].m_nConsecutiveFrags = 0;

								m_ClientData[nIndex].m_nUpdateFlags |= MP_CLIENT_UPDATE_FRAGS | MP_CLIENT_UPDATE_SCORE;

								// If this player was an exosuit then make sure the powerup 
								// gets restored
								if(IsExosuit(pPlayer->m_hObject))
									pPlayer->RespawnExosuit();

								// [KLS] 9/27/01 Moved this call before respawn so aliens 
								// can change teams when lifecycle is true.  If not respawn
								// below would call ValidateRespawn which resets the
								// character back to the alien on the server while the
								// client (interface) thinks the character change was
								// successful...

								// Only do the respawn if we're not in observe mode...
								if(!m_ClientData[nIndex].m_bObserving)
								{
									// Setup the multiplayer stats with this character change
									ChangeClientCharacter(hSender, nCharacterSet);
									pPlayer->MPRespawn(nCharacterSet);
								}
								else
								{
									// Setup the multiplayer stats with this character change
									ChangeClientCharacter(hSender, nCharacterSet);
									// Set the character and respawn
									pPlayer->SetCharacter(nCharacterSet, LTTRUE);
								}

								// [KLS] 9/27/01 Commented this out and moved it before
								// the repawn call above...
								// Setup the multiplayer stats with this character change
								// ChangeClientCharacter(hSender, nCharacterSet);
							}
						}
					}
				}
			}

			return LTTRUE;
		}

		case MID_MPMGR_OBSERVE_MODE:
		{
			// Read in the command value
			uint8 nCommand;
			hRead->ReadByteFL(nCommand);

			ClientObserveCommand(hSender, nCommand);

			return LTTRUE;
		}

		case MID_MPMGR_RESPAWN:
		{
			// Find this player
			CPlayerObj *pPlayer = FindPlayerObj(hSender);

			if(pPlayer)
			{
				pPlayer->Respawn();
			}

			return LTTRUE;
		}

		case MID_MPMGR_START_GAME:
		{
			// This should only get used for Overrun and Evac games
			if((m_GameInfo.m_nGameType == MP_OVERRUN) || (m_GameInfo.m_nGameType == MP_EVAC))
			{
				// This also should only do something if the game isn't going yet...
				if(m_nServerSubState != MPMGR_SUBSTATE_TM_GOING)
				{
					// Make sure the number of defenders and attackers is ok...
					int nDefenders = NumberOfClass(m_GameInfo.m_nTM_DefenderRace, LTTRUE);
					int nAttackers = NumberOfClass(m_GameInfo.m_nTM_AttackerRace, LTTRUE);

					if((nDefenders > 0) && (nAttackers > 0))
					{
						// Set the new state
						m_nServerSubState = MPMGR_SUBSTATE_TM_GOING;
						m_fStartTime = g_pLTServer->GetTime();

						// Give everyone the appropriate number of lives... and respawn them
						for(int i = 0; i < MAX_CLIENTS; i++)
						{
							if(m_ClientData[i].m_hClient != LTNULL)
							{
								if(m_ClientData[i].m_Data.m_nCharacterClass == m_GameInfo.m_nTM_DefenderRace)
									m_ClientData[i].m_nLives = m_GameInfo.m_nTM_DefenderLives;
								else
									m_ClientData[i].m_nLives = m_GameInfo.m_nTM_AttackerLives;

								m_ClientData[i].m_nUpdateFlags |= MP_CLIENT_UPDATE_LIVES;

								CPlayerObj *pPlayer = FindPlayerObj(m_ClientData[i].m_hClient);

								if(pPlayer)
								{
									pPlayer->Respawn();
								}
							}
						}
					}
				}
			}

			return LTTRUE;
		}

		case MID_MPMGR_TAUNT:
		{
			// Find the client who sent the message
			int nIndex = FindClient(hSender);

			if(nIndex != -1)
			{
				// Make sure to not let people go crazy with taunts...
				if(g_pLTServer->GetTime() > (m_ClientData[nIndex].m_fTauntTimer + 5.0f))
				{
					m_ClientData[nIndex].m_fTauntTimer = g_pLTServer->GetTime();

					// Send the taunt to everyone who has taunts enabled.
					for(int i = 0; i < MAX_CLIENTS; i++)
					{
						if((m_ClientData[i].m_hClient != LTNULL) && !m_ClientData[i].m_Data.m_bIgnoreTaunts)
						{
							HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_ClientData[i].m_hClient, MID_MPMGR_TAUNT);
							hMessage->WriteDWord(m_ClientData[nIndex].m_nClientID);
							g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
						}
					}
				}


				// Notify all the AI's about our taunting.
				if( g_pGameServerShell->GetGameType().IsSinglePlayer() )
				{
					// Find this player
					CPlayerObj *pPlayer = FindPlayerObj(hSender);

					if( pPlayer )
					{
						// Be sure we actually played a sound.
						// Currently, we can only confirm that a sound bute exists and assume
						// it was played.
						const CString strSoundSetName = g_pServerSoundMgr->GetSoundSetName( g_pCharacterButeMgr->GetGeneralSoundDir(pPlayer->GetCharacterClass()), "TauntSP" );
						const int nButes = g_pSoundButeMgr->GetSoundSetFromName( strSoundSetName );
						

						if( nButes >= 0 )
						{
							// Tell the other AI's of our battle cry!
							const SoundButes & sound_bute = g_pSoundButeMgr->GetSoundButes(nButes);
							for( CCharacterMgr::AIIterator iter = g_pCharacterMgr->BeginAIs(); iter != g_pCharacterMgr->EndAIs(); ++iter )
							{
								(*iter)->GetSenseMgr().Taunt(sound_bute,pPlayer);
							}

						} // if( nButes >= 0 && pPlayer )

					} // if( pPlayer )

				} // if( g_pGameServerShell->GetGameType().IsSinglePlayer() )

			} // if(nIndex != -1)

			return LTTRUE;
		}
	}


	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ProcessServerCommand()
//
// PURPOSE:		Handles a command from the server controller
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::ProcessServerCommand(char *szMsg, LTBOOL bFromMsg, int senderID)
{
	if(!szMsg) return LTFALSE;

	// Temp variables
	LTBOOL bStartNewLevel = LTFALSE;
	LTBOOL bCommandHandled = LTFALSE;

	if(bFromMsg)
	{
		if((uint8)senderID != -1)
		{
			// If this guy isn't in control of the server, see if this is a control change command
			if((uint8)senderID != m_nServerController)
			{
				if(!_strnicmp(szMsg, "control", 7))
				{
					// see if this is a dedicated server
					if(!m_GameInfo.m_bDedicated)
					{
						HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_ClientData[senderID].m_hClient, MID_MPMGR_SERVER_MESSAGE);
						hMessage->WriteByte(MP_SERVER_CONTROLLER_NOT_DEDICATED);
						g_pLTServer->EndMessage(hMessage);
						return LTTRUE;
					}
					// see if the PW matches...
					char *szPW = szMsg + 7;

					// Get rid of leading spaces
					while(*szPW == ' ')
						szPW++;

					if(!_stricmp(szPW, m_GameInfo.m_szSCPassword))
					{
						// we have a winner!
						SetServerController(senderID);;
					}
					// wrong PW!
					else
					{
						HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_ClientData[senderID].m_hClient, MID_MPMGR_SERVER_MESSAGE);
						hMessage->WriteByte(MP_SERVER_CONTROLLER_PW_INVAL);
						g_pLTServer->EndMessage(hMessage);
					}
				}
				else
				{
					// this guy is not in control...  let him know
					HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_ClientData[senderID].m_hClient, MID_MPMGR_SERVER_MESSAGE);
					hMessage->WriteByte(MP_SERVER_CONTROLLER);
					hMessage->WriteDWord(m_ClientData[m_nServerController].m_nClientID);
					hMessage->WriteByte(LTFALSE);
					g_pLTServer->EndMessage(hMessage);
				}

				// in any of these events, the process stops here
				return LTTRUE;
			}
		}
		// invalid client index...
		else
		{
			return LTTRUE;
		}
	}

	if(!_strnicmp(szMsg, "lifecycle", 9))
	{
		// time to change the game option
		SetGameInfoChanged();
		m_PendingGameInfo.m_bLifecycle = !m_PendingGameInfo.m_bLifecycle;

		// tell everyone about this change
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SC_ALIEN_LIFECYCLE);
		hMessage->WriteByte(m_PendingGameInfo.m_bLifecycle);
		g_pLTServer->EndMessage(hMessage);
	}
	else if(!_strnicmp(szMsg, "queen", 5))
	{
		if(strlen(szMsg) > 6)
		{
			// Finds the next non-space character
			char *szValue = szMsg + 5;

			// Get rid of leading spaces
			while(*szValue == ' ')
				szValue++;

			int value = atoi(szValue);

			if(value == 0 && szValue[0] != '0')
			{
				// invalid string entered
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_ClientData[m_nServerController].m_hClient, MID_MPMGR_SERVER_MESSAGE);
				hMessage->WriteByte(MP_SC_INVALID);
				g_pLTServer->EndMessage(hMessage);
			}
			else if(value > -1 && value < 11)
			{
				// time to change the game option
				SetGameInfoChanged();
				m_PendingGameInfo.m_nQueenMolt = value;

				// tell everyone about this change
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_SERVER_MESSAGE);
				hMessage->WriteByte(MP_SC_QUEEN);
				hMessage->WriteByte(value);
				g_pLTServer->EndMessage(hMessage);
			}
			else
			{
				// out of bounds value entered
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_ClientData[m_nServerController].m_hClient, MID_MPMGR_SERVER_MESSAGE);
				hMessage->WriteByte(MP_SC_BOUNDS);
				g_pLTServer->EndMessage(hMessage);
			}
		}
	}
	else if(!_strnicmp(szMsg, "exosuit", 7))
	{
		if(strlen(szMsg) > 8)
		{
			// Finds the next non-space character
			char *szValue = szMsg + 7;

			// Get rid of leading spaces
			while(*szValue == ' ')
				szValue++;

			int value = atoi(szValue);

			if(value == 0 && szValue[0] != '0')
			{
				// invalid string entered
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_ClientData[m_nServerController].m_hClient, MID_MPMGR_SERVER_MESSAGE);
				hMessage->WriteByte(MP_SC_INVALID);
				g_pLTServer->EndMessage(hMessage);
			}
			else if(value > -1 && value < 11)
			{
				// time to change the game option
				SetGameInfoChanged();
				m_PendingGameInfo.m_nExosuit = value;

				// tell everyone about this change
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_SERVER_MESSAGE);
				hMessage->WriteByte(MP_SC_EXOSUIT);
				hMessage->WriteByte(value);
				g_pLTServer->EndMessage(hMessage);
			}
			else
			{
				// out of bounds value entered
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_ClientData[m_nServerController].m_hClient, MID_MPMGR_SERVER_MESSAGE);
				hMessage->WriteByte(MP_SC_BOUNDS);
				g_pLTServer->EndMessage(hMessage);
			}
		}
	}
	else if(!_strnicmp(szMsg, "predmask", 8))
	{
		// time to change the game option
		SetGameInfoChanged();
		m_PendingGameInfo.m_bMaskLoss = !m_PendingGameInfo.m_bMaskLoss;

		// tell everyone about this change
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SC_PRED_MASK);
		hMessage->WriteByte(m_PendingGameInfo.m_bMaskLoss);
		g_pLTServer->EndMessage(hMessage);
	}
	else if(!_strnicmp(szMsg, "friendlyfire", 12))
	{
		// time to change the game option
		SetGameInfoChanged();
		m_PendingGameInfo.m_bFriendlyFire = !m_PendingGameInfo.m_bFriendlyFire;

		// tell everyone about this change
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SC_FF);
		hMessage->WriteByte(m_PendingGameInfo.m_bFriendlyFire);
		g_pLTServer->EndMessage(hMessage);
	}
	else if(!_strnicmp(szMsg, "popupnames", 10))
	{
		// time to change the game option
		SetGameInfoChanged();
		m_PendingGameInfo.m_bFriendlyNames = !m_PendingGameInfo.m_bFriendlyNames;

		// tell everyone about this change
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SC_FN);
		hMessage->WriteByte(!m_PendingGameInfo.m_bFriendlyNames);
		g_pLTServer->EndMessage(hMessage);
	}
	else if(!_strnicmp(szMsg, "locationdamage", 14))
	{
		// time to change the game option
		SetGameInfoChanged();
		m_PendingGameInfo.m_bLocationDamage = !m_PendingGameInfo.m_bLocationDamage;

		// tell everyone about this change
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SC_LOC_DAM);
		hMessage->WriteByte(m_PendingGameInfo.m_bLocationDamage);
		g_pLTServer->EndMessage(hMessage);
	}
	else if(!_strnicmp(szMsg, "classweapons", 12))
	{
		// time to change the game option
		SetGameInfoChanged();
		m_PendingGameInfo.m_bClassWeapons = !m_PendingGameInfo.m_bClassWeapons;

		// tell everyone about this change
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SC_CLASS_WEAPONS);
		hMessage->WriteByte(m_PendingGameInfo.m_bClassWeapons);
		g_pLTServer->EndMessage(hMessage);
	}
	else if(!_strnicmp(szMsg, "kick", 4))
	{
		if(strlen(szMsg) > 5)
		{
			// Finds the next non-space character
			char *szName = szMsg + 4;

			// Get rid of leading spaces
			while(*szName == ' ')
				szName++;


			// Find the named client
			int nIndex = FindClient(szName);

			if(nIndex != -1)
			{
				g_pLTServer->KickClient(m_ClientData[nIndex].m_hClient);
			}

			bCommandHandled = LTTRUE;
		}
	}
	else if(!_strnicmp(szMsg, "prevlevel", 9))
	{
		SetPrevLevel();
		bStartNewLevel = LTTRUE;
		bCommandHandled = LTTRUE;
	}
	else if(!_strnicmp(szMsg, "nextlevel", 9))
	{
		SetNextLevel();
		bStartNewLevel = LTTRUE;
		bCommandHandled = LTTRUE;
	}
	else if(!_strnicmp(szMsg, "restart", 7))
	{
		m_nNextLevel = m_nCurrentLevel;
		bStartNewLevel = LTTRUE;
		bCommandHandled = LTTRUE;
	}
	else if(!_strnicmp(szMsg, "forceverify", 11))
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			m_ClientData[i].m_bVerified = LTTRUE;
		}
	}
	else if(!_strnicmp(szMsg, "control", 7))
	{
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_ClientData[m_nServerController].m_hClient, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SERVER_CONTROLLER_REDUND);
		g_pLTServer->EndMessage(hMessage);
	}
	else
	{
		// Send a message saying it's an invalid command
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_ClientData[m_nServerController].m_hClient, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SERVER_COMMAND_INVALID);
		g_pLTServer->EndMessage(hMessage);
	}


	// If we're going to start a new level... then do so
	if(bStartNewLevel)
	{
		// Set the new state
		m_nServerState = MPMGR_STATE_NEXT_LEVEL_VERIFY;
		m_fVerifyTime = g_pLTServer->GetTime();

		// See if we need to reset the rules..
		if(m_bGameInfoChanged)
		{
			// copy the rules and reset the flag
			m_GameInfo = m_PendingGameInfo;
			m_bGameInfoChanged = LTFALSE;
		}

		// Clear the current verification states
		ClearVerification();

		// Send a message to let everyone know they should be in a waiting state
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_NEXT_LEVEL);
		hMessage->WriteByte(MPMGR_NEXT_LEVEL_RESET);
		hMessage->WriteString(GetBaseMapName(m_GameInfo.m_szLevels[m_nNextLevel]));
		g_pLTServer->EndMessage(hMessage);
	}


	return bCommandHandled;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::AllowFriendlyFire()
//
// PURPOSE:		Determines if friend fire should be used or not...
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::AllowFriendlyFire()
{
	if(m_GameInfo.m_bFriendlyFire)
		return LTTRUE;

	if(m_GameInfo.m_nGameType == MP_DM)
		return LTTRUE;

	if(m_nServerSubState == MPMGR_SUBSTATE_SURVIVOR_TAG)
		return LTTRUE;

	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::Update()
//
// PURPOSE:		Do any constant updates for the game type
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::Update()
{
	// Determine what to do based off the server state
	switch(m_nServerState)
	{
		case MPMGR_STATE_PLAYING:
		{
			// Check to see if the end conditions are met... if so, goto the
			// next level verification state
			if(CheckEndConditions())
			{
				// Set the new state
				m_nServerState = MPMGR_STATE_NEXT_LEVEL_VERIFY;
				m_fVerifyTime = g_pLTServer->GetTime();

				// See if we need to reset the rules..
				if(m_bGameInfoChanged)
				{
					// copy the rules and reset the flag
					m_GameInfo = m_PendingGameInfo;
					m_bGameInfoChanged = LTFALSE;
				}

				// Clear the current verification states
				ClearVerification();

				// Set the next level
				SetNextLevel();

				// Send a message to let everyone know they should be in a waiting state
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_NEXT_LEVEL);
				hMessage->WriteByte(MPMGR_NEXT_LEVEL_RESET);
				hMessage->WriteString(GetBaseMapName(m_GameInfo.m_szLevels[m_nNextLevel]));
				g_pLTServer->EndMessage(hMessage);

				break;
			}

			// Do special player updates...
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if((m_ClientData[i].m_hClient != LTNULL) && m_ClientData[i].m_bSpawnAsQueen)
				{
					if(g_pLTServer->GetTime() > m_ClientData[i].m_fMiscTimer + (MPMGR_MUTATE_TIME*2))
					{
						// ok change this client into a queen then respawn the bitch!
						CPlayerObj *pPlayer = FindPlayerObj(m_ClientData[i].m_hClient);

						if(pPlayer)
						{
							int nCharacterSet = g_pCharacterButeMgr->GetSetFromModelType("queen");

							pPlayer->SetCharacter(nCharacterSet, LTTRUE);
							pPlayer->ResetPlayerHealth();
							pPlayer->AquireDefaultWeapon();
//							pPlayer->Respawn();

							m_ClientData[i].m_bSpawnAsQueen = LTFALSE;
						}
					}
				}
			}


			// Do special game updates
			switch(m_GameInfo.m_nGameType.Get())
			{
				case MP_DM:			break;
				case MP_TEAMDM:		break;
				case MP_HUNT:		Update_Hunt();		break;
				case MP_SURVIVOR:	Update_Survivor();	break;
				case MP_OVERRUN:	Update_Overrun();	break;
				case MP_EVAC:		Update_Evac();		break;
			}

			break;
		}

		case MPMGR_STATE_NEXT_LEVEL_VERIFY:
		{
			// Check to see if everone has verified the next level
			if(CheckVerification() || (g_pLTServer->GetTime() > (m_fVerifyTime + MPMGR_FORCE_VERIFY_TIME)))
			{
				// Clear the current verification states
				ClearVerification();

				// Set the new state
				m_nServerState = MPMGR_STATE_LOAD_LEVEL_VERIFY;
				m_fVerifyTime = g_pLTServer->GetTime();

				// Send a message to let everyone know they should be in a waiting state
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_LOAD_LEVEL);
				g_pLTServer->EndMessage(hMessage);
			}
			else
			{
				// Send the remaining amount of time for the start round delay
				LTFLOAT fWaitTime = MPMGR_FORCE_VERIFY_TIME - (g_pLTServer->GetTime() - m_fVerifyTime);

				if(fWaitTime < 0.0f)
					fWaitTime = 0.0f;

				uint8 nWait = (uint8)(fWaitTime);

				if(nWait != m_nWaitSeconds)
				{
					m_nWaitSeconds = nWait;

					HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_WAIT_SECONDS);
					hWrite->WriteByte(m_nWaitSeconds);
					g_pLTServer->EndMessage(hWrite);
				}
			}

			break;
		}

		case MPMGR_STATE_LOAD_LEVEL_VERIFY:
		{
			// If everyone has verified the loading of the next level... then load it!
			if(CheckVerification() || (g_pLTServer->GetTime() > (m_fVerifyTime + MPMGR_FORCE_LEVEL_LOAD)))
			{
				// Kick out anyone who hasn't verified yet...
				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					if((m_ClientData[i].m_hClient != LTNULL) && (!m_ClientData[i].m_bVerified))
					{
						g_pLTServer->KickClient(m_ClientData[i].m_hClient);
					}
				}


				// Clear the current verification states
				ClearVerification();

				// Set the new state
				m_nServerState = MPMGR_STATE_PLAYING;

				m_nCurrentLevel = m_nNextLevel;

				if(LoadLevel(m_GameInfo.m_szLevels[m_nCurrentLevel]) != LT_OK)
				{
					// Do some kind of error handling??
				}
			}

			break;
		}
	}


	// See if any client information needs to be updated
	UpdateClientPings();
	UpdateClients();
	UpdateTeams();


	// Update the server app.
	ServerAppShellUpdate( );

	// Only update the GameSpy stuff if it got initialized properly
	if(m_bUsingGameSpy)
		CGameSpyMgr::Update();

}



// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::Update_Hunt()
//
// PURPOSE:		Do any constant updates for the game type
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::Update_Hunt()
{
	// Count the number of hunters and prey in the level
	int nHunters = NumberOfClass(m_GameInfo.m_nHunt_HunterRace, LTTRUE);
	int nPrey = NumberOfClass(m_GameInfo.m_nHunt_PreyRace, LTTRUE);
	int i;


	// Now make sure we have an appropriate number of hunters at all times...
	int nDestHunters = m_nClients / (m_GameInfo.m_nHunt_Ratio + 1);

	if(nDestHunters < 1)
		nDestHunters = 1;

	if(nHunters < nDestHunters)
	{
		// If we're short on hunters... find the prey with the lowest score and make them a hunter
		int nIndex = FindLowestFragsClient(m_GameInfo.m_nHunt_PreyRace);

		if(nIndex != -1)
		{
			// Now tell all the clients that someone is mutating
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
			hMessage->WriteByte(MP_HUNT_MUTATE_HUNTER);
			hMessage->WriteDWord(m_ClientData[nIndex].m_nClientID);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

			m_ClientData[nIndex].m_nNextClass = m_GameInfo.m_nHunt_HunterRace;
			m_ClientData[nIndex].m_fMiscTimer = g_pLTServer->GetTime();
		}
	}
	else if(nHunters > nDestHunters)
	{
		// If we've got too many hunters... find the hunter with the highest score and make them a prey
		int nIndex = FindHighestFragsClient(m_GameInfo.m_nHunt_HunterRace);

		if(nIndex != -1)
		{
			// Now tell all the clients that someone is mutating
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
			hMessage->WriteByte(MP_HUNT_MUTATE_PREY);
			hMessage->WriteDWord(m_ClientData[nIndex].m_nClientID);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);

			m_ClientData[nIndex].m_nNextClass = m_GameInfo.m_nHunt_PreyRace;
			m_ClientData[nIndex].m_fMiscTimer = g_pLTServer->GetTime();
		}
	}

	LTBOOL	bRespawn[MAX_CLIENTS];
	uint8	nChar[MAX_CLIENTS];
	memset(bRespawn, LTFALSE, MAX_CLIENTS*sizeof(LTBOOL));

	// Make sure everyone is the correct class
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient != LTNULL)
		{
			if((m_ClientData[i].m_nNextClass < MAX_TEAMS) && (m_ClientData[i].m_Data.m_nCharacterClass != m_ClientData[i].m_nNextClass))
			{
				if((m_ClientData[i].m_fMiscTimer < 0.0f) || (g_pLTServer->GetTime() >= m_ClientData[i].m_fMiscTimer + MPMGR_MUTATE_TIME))
				{
					CPlayerObj *pPlayer = FindPlayerObj(m_ClientData[i].m_hClient);

					if(pPlayer)
					{
						// If this player was an exosuit then make sure the powerup 
						// gets restored
						if(IsExosuit(pPlayer->m_hObject))
							pPlayer->RespawnExosuit();

						ChangeClientClass(m_ClientData[i].m_hClient, m_ClientData[i].m_nNextClass, LTFALSE);

						nChar[i]	= m_ClientData[i].m_Data.m_nCharacterSet[m_ClientData[i].m_nNextClass];
						bRespawn[i] = LTTRUE;
					}

					m_ClientData[i].m_fMiscTimer = 0.0f;
					m_ClientData[i].m_nNextClass = -1;
				}
			}
			else
			{
				m_ClientData[i].m_nNextClass = -1;
			}
		}
	}

	for(i=0;i<MAX_CLIENTS;i++)
	{
		if(bRespawn[i])
		{
			CPlayerObj *pPlayer = FindPlayerObj(m_ClientData[i].m_hClient);

			if(pPlayer)
				pPlayer->MPRespawn(nChar[i]);
		}
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::Update_Survivor()
//
// PURPOSE:		Do any constant updates for the game type
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::Update_Survivor()
{
	// Count the number of survivors and mutants in the level
	int nSurvivors = NumberOfClass(m_GameInfo.m_nSurv_SurvivorRace, LTTRUE);
	int nMutants = NumberOfClass(m_GameInfo.m_nSurv_MutateRace, LTTRUE);

	LTBOOL bUpdateIntermTime = LTFALSE;
	LTBOOL bTimeUpdated = LTFALSE;
	int i;


	// Set the current sub state of the game
	uint8 nOldSubState = m_nServerSubState;


	// If we're waiting on the next round... check the timer...
	if(m_nServerSubState == MPMGR_SUBSTATE_SURVIVOR_ROUND)
	{
		if(g_pLTServer->GetTime() >= (m_fMiscTime + MPMGR_ROUND_DELAY))
		{
			// Increment the round counter and send the data to the clients...
			m_nCurrentRound++;
			m_fStartTime = g_pLTServer->GetTime();

			LTBOOL bLastRound = !(!(m_GameInfo.m_nEndConditionFlags & MP_END_CONDITION_ROUNDS) || (m_nCurrentRound < m_GameInfo.m_nRoundLimit));

			if(!bLastRound)
			{
				HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_ROUND);
				hWrite->WriteByte(m_nCurrentRound + 1); // Add 1 to give the non-zero based round value
				g_pLTServer->EndMessage(hWrite);


				// Put us back in 'tag mode'
				m_nServerSubState = MPMGR_SUBSTATE_NONE;
			}


			// Now respawn everyone as survivors again for the new round
			for(i = 0; i < MAX_CLIENTS; i++)
			{
				if(m_ClientData[i].m_hClient != LTNULL)
				{
					CPlayerObj *pPlayer = FindPlayerObj(m_ClientData[i].m_hClient);

					if(pPlayer)
					{
						// If this player was an exosuit then make sure the powerup 
						// gets restored
						if(IsExosuit(pPlayer->m_hObject))
							pPlayer->RespawnExosuit();

						if(!bLastRound)
							pPlayer->MPRespawn(m_ClientData[i].m_Data.m_nCharacterSet[m_GameInfo.m_nSurv_SurvivorRace]);

						ChangeClientClass(m_ClientData[i].m_hClient, m_GameInfo.m_nSurv_SurvivorRace, LTFALSE);
					}

					m_ClientData[i].m_nNextClass = -1;
					m_ClientData[i].m_nConsecutiveFrags = 0;
				}
			}
		}
		else
		{
			// Send the remaining amount of time for the start round delay
			LTFLOAT fWaitTime = MPMGR_ROUND_DELAY - (g_pLTServer->GetTime() - m_fMiscTime);

			if(fWaitTime < 0.0f)
				fWaitTime = 0.0f;

			uint8 nWait = (uint8)(fWaitTime);

			if(nWait != m_nWaitSeconds)
			{
				m_nWaitSeconds = nWait;

				HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_WAIT_SECONDS);
				hWrite->WriteByte(m_nWaitSeconds);
				g_pLTServer->EndMessage(hWrite);
			}
		}
	}
	// Otherwise, continue as usual...
	else
	{
		// Make sure the sub state is set properly
		if(m_nClients < 2)
		{
			m_nServerSubState = MPMGR_SUBSTATE_NONE;
		}
		else if(nMutants < 1)
		{
			m_nServerSubState = MPMGR_SUBSTATE_SURVIVOR_TAG;
		}
		else if(m_nServerSubState == MPMGR_SUBSTATE_SURVIVOR_INTERM)
		{
			if(g_pLTServer->GetTime() >= (m_fMiscTime + MPMGR_SURVIVOR_INTERM_TIME))
			{
				m_nServerSubState = MPMGR_SUBSTATE_SURVIVOR_CUT;
			}
			else
			{
				bUpdateIntermTime = LTTRUE;
			}
		}
		else if(m_nServerSubState != MPMGR_SUBSTATE_SURVIVOR_CUT)
		{
			m_nServerSubState = MPMGR_SUBSTATE_SURVIVOR_INTERM;
			m_fMiscTime = g_pLTServer->GetTime();
		}


		// See if it's time to start the next round...
		LTFLOAT fTime = g_pLTServer->GetTime() - m_fStartTime;

		if(	(m_GameInfo.m_fTimeLimit && (fTime >= m_GameInfo.m_fTimeLimit)) ||
			((m_nClients > 1) && (nMutants > 0) && (nSurvivors < 1)))
		{
			// Set the sub state for the next round waiting period...
			m_nServerSubState = MPMGR_SUBSTATE_SURVIVOR_ROUND;
			m_fMiscTime = g_pLTServer->GetTime();
		}
		else
		{
			// Update the clients with the new time
			uint16 nRemaining = (uint16)(m_GameInfo.m_fTimeLimit - fTime);

			if(nRemaining != m_nTimeRemaining)
			{
				m_nTimeRemaining = nRemaining;
				bTimeUpdated = LTTRUE;

				if(m_GameInfo.m_fTimeLimit)
				{
					HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_TIME);
					hWrite->WriteWord(m_nTimeRemaining);
					g_pLTServer->EndMessage2(hWrite, MESSAGE_NAGGLE);
				}

				// Update the time for the interm mode...
				if(bUpdateIntermTime)
				{
					int8 nTime = (int8)(MPMGR_SURVIVOR_INTERM_TIME - (g_pLTServer->GetTime() - m_fMiscTime));

					HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
					hMessage->WriteByte(MP_SURVIVOR_INTERM_TIME);
					hMessage->WriteByte(nTime);
					g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
				}
			}


			// Make sure everyone is the correct class
			for(i = 0; i < MAX_CLIENTS; i++)
			{
				if(m_ClientData[i].m_hClient != LTNULL)
				{
					if(	(m_ClientData[i].m_nNextClass < MAX_TEAMS) &&
						(m_ClientData[i].m_Data.m_nCharacterClass != m_ClientData[i].m_nNextClass))
					{
						if(	(m_ClientData[i].m_fMiscTimer < 0.0f) ||
							(g_pLTServer->GetTime() >= m_ClientData[i].m_fMiscTimer + MPMGR_MUTATE_TIME))
						{
							CPlayerObj *pPlayer = FindPlayerObj(m_ClientData[i].m_hClient);

							if(pPlayer)
							{
								// If this player was an exosuit then make sure the powerup 
								// gets restored
								if(IsExosuit(pPlayer->m_hObject))
									pPlayer->RespawnExosuit();

								ChangeClientClass(m_ClientData[i].m_hClient, m_ClientData[i].m_nNextClass, LTFALSE);

								pPlayer->MPRespawn(m_ClientData[i].m_Data.m_nCharacterSet[m_ClientData[i].m_nNextClass]);

							}

							// This time setting is a stamp for when this character mutated... this will be
							// used to determine score when the round is over
							m_ClientData[i].m_fMiscTimer = g_pLTServer->GetTime();
							m_ClientData[i].m_nNextClass = -1;
						}
					}
					else
					{
						m_ClientData[i].m_nNextClass = -1;
					}
				}
			}
		}
	}


	// Now give all the appropriate players some points...
	if(bTimeUpdated && (m_nServerSubState == MPMGR_SUBSTATE_SURVIVOR_CUT))
	{
		for(i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_ClientData[i].m_hClient != LTNULL)
			{
				if(m_ClientData[i].m_Data.m_nCharacterClass == m_GameInfo.m_nSurv_SurvivorRace)
				{
					CPlayerObj *pPlayer = FindPlayerObj(m_ClientData[i].m_hClient);

					if(pPlayer && !pPlayer->InGodMode() && (pPlayer->GetState() == PS_ALIVE))
					{
						m_ClientData[i].m_nScore++;
						m_ClientData[i].m_nUpdateFlags |= MP_CLIENT_UPDATE_SCORE;
					}
				}
			}
		}
	}


	// Tell everyone that the sub state has changed...
	if(nOldSubState != m_nServerSubState)
	{
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SUBSTATE_CHANGED);
		hMessage->WriteByte(m_nServerSubState);
		g_pLTServer->EndMessage(hMessage);
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::Update_Overrun()
//
// PURPOSE:		Do any constant updates for the game type
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::Update_Overrun()
{
	// Count the number of defenders and attackers in the level
	int nDefenders = NumberOfClass(m_GameInfo.m_nTM_DefenderRace, LTTRUE);
	int nAttackers = NumberOfClass(m_GameInfo.m_nTM_AttackerRace, LTTRUE);
	int i;


	// Set the current sub state of the game
	uint8 nOldSubState = m_nServerSubState;


	// Handle the pre-game state...
	if(m_nServerSubState == MPMGR_SUBSTATE_NONE)
	{
		LTBOOL bCanStart = ((nDefenders > 0) && (nAttackers > 0));

		// Go through all the clients and make sure their observe states are correct
		for(i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_ClientData[i].m_hClient != LTNULL)
			{
				// If the teams are unbalanced... make everyone wait
				if(!bCanStart)
				{
					if(m_ClientData[i].m_nObserveMode != MP_OBSERVE_MODE_WAIT_TEAMS)
					{
						m_ClientData[i].m_nObserveMode = MP_OBSERVE_MODE_WAIT_TEAMS;
						ClientObserveCommand(m_ClientData[i].m_hClient, MP_OBSERVE_CHANGE_MODE);
					}

					m_fMiscTime = 0.0f;
					m_nWaitSeconds = 0xFF;
				}
				else
				{
					// The server controller can have a different observer state
					if(i == m_nServerController)
					{
						if(m_ClientData[i].m_nObserveMode != MP_OBSERVE_MODE_START_GAME)
						{
							m_ClientData[i].m_nObserveMode = MP_OBSERVE_MODE_START_GAME;
							ClientObserveCommand(m_ClientData[i].m_hClient, MP_OBSERVE_CHANGE_MODE);
						}
					}
					else
					{
						if(m_ClientData[i].m_nObserveMode != MP_OBSERVE_MODE_WAIT_START_GAME)
						{
							m_ClientData[i].m_nObserveMode = MP_OBSERVE_MODE_WAIT_START_GAME;
							ClientObserveCommand(m_ClientData[i].m_hClient, MP_OBSERVE_CHANGE_MODE);
						}
					}


					// Check to see if we need to set our wait time
					if(!m_fMiscTime)
					{
						m_fMiscTime = g_pLTServer->GetTime();
					}


					// Send the remaining amount of time for the start round delay
					LTFLOAT fWaitTime = MPMGR_ROUND_START_DELAY - (g_pLTServer->GetTime() - m_fMiscTime);

					if(fWaitTime < 0.0f)
						fWaitTime = 0.0f;

					uint8 nWait = (uint8)(fWaitTime);

					if(nWait != m_nWaitSeconds)
					{
						m_nWaitSeconds = nWait;

						HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_WAIT_SECONDS);
						hWrite->WriteByte(m_nWaitSeconds);
						g_pLTServer->EndMessage(hWrite);
					}

					// Go ahead and start the game if we're out of time...
					if(!m_nWaitSeconds)
					{
						OnMessage(LTNULL, MID_MPMGR_START_GAME, LTNULL);
					}
				}
			}
		}
	}
	// Handle waiting on the next round...
	else if(m_nServerSubState == MPMGR_SUBSTATE_TM_ROUND)
	{
		if(g_pLTServer->GetTime() >= (m_fMiscTime + MPMGR_ROUND_DELAY))
		{
			// Increment the round counter and send the data to the clients...
			m_nCurrentRound++;
			m_fStartTime = g_pLTServer->GetTime();

			LTBOOL bLastRound = !(!(m_GameInfo.m_nEndConditionFlags & MP_END_CONDITION_ROUNDS) || (m_nCurrentRound < m_GameInfo.m_nRoundLimit));

			if(!bLastRound)
			{
				HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_ROUND);
				hWrite->WriteByte(m_nCurrentRound + 1); // Add 1 to give the non-zero based round value
				g_pLTServer->EndMessage(hWrite);

				// Now respawn everyone again for the new round
				for(i = 0; i < MAX_CLIENTS; i++)
				{
					if(m_ClientData[i].m_hClient != LTNULL)
					{
						CPlayerObj *pPlayer = FindPlayerObj(m_ClientData[i].m_hClient);

						if(pPlayer)
						{
							// If this player was an exosuit then make sure the powerup 
							// gets restored
							if(IsExosuit(pPlayer->m_hObject))
								pPlayer->RespawnExosuit();

							// Reset the number of lives, consecutive frags and race...
							uint8 nChar;
							if(m_ClientData[i].m_Data.m_nCharacterClass == m_GameInfo.m_nTM_DefenderRace)
							{
								m_ClientData[i].m_nLives = m_GameInfo.m_nTM_DefenderLives;
								nChar = m_ClientData[i].m_Data.m_nCharacterSet[m_GameInfo.m_nTM_DefenderRace];
							}
							else
							{
								m_ClientData[i].m_nLives = m_GameInfo.m_nTM_AttackerLives;
								nChar = m_ClientData[i].m_Data.m_nCharacterSet[m_GameInfo.m_nTM_AttackerRace];
							}

							m_ClientData[i].m_nUpdateFlags |= MP_CLIENT_UPDATE_LIVES;
							m_ClientData[i].m_nConsecutiveFrags = 0;

							pPlayer->MPRespawn(nChar);
						}
					}
				}

				// Make sure the level gets reset...
				g_pGameServerShell->ProcessMPStartPointMessages();

				// Put us back in normal mode
				m_nServerSubState = MPMGR_SUBSTATE_TM_GOING;
			}
		}
		else
		{
			// Send the remaining amount of time for the start round delay
			LTFLOAT fWaitTime = MPMGR_ROUND_DELAY - (g_pLTServer->GetTime() - m_fMiscTime);

			if(fWaitTime < 0.0f)
				fWaitTime = 0.0f;

			uint8 nWait = (uint8)(fWaitTime);

			if(nWait != m_nWaitSeconds)
			{
				m_nWaitSeconds = nWait;

				HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_WAIT_SECONDS);
				hWrite->WriteByte(m_nWaitSeconds);
				g_pLTServer->EndMessage(hWrite);
			}
		}
	}
	// Handle the during-game state
	else if(m_nServerSubState == MPMGR_SUBSTATE_TM_GOING)
	{
		// Go through and count the number of people who still have lives...
		int nDefendersAlive = 0;
		int nAttackersAlive = 0;

		// Go through all the clients and add them up
		for(i = 0; i < MAX_CLIENTS; i++)
		{
			if((m_ClientData[i].m_hClient != LTNULL) && (m_ClientData[i].m_nLives > 0))
			{
				if(m_ClientData[i].m_Data.m_nCharacterClass == m_GameInfo.m_nTM_DefenderRace)
					nDefendersAlive++;
				else
					nAttackersAlive++;
			}
		}


		// See if we might need to start the next round...
		LTBOOL bStartNextRound = LTFALSE;

		if(!nDefendersAlive || !nAttackersAlive)
		{
			bStartNextRound = LTTRUE;
		}
		else
		{
			if(m_GameInfo.m_fTimeLimit > 0.0f)
			{
				LTFLOAT fTime = g_pLTServer->GetTime() - m_fStartTime;

				if(fTime >= m_GameInfo.m_fTimeLimit)
				{
					bStartNextRound = LTTRUE;
				}
				else
				{
					uint16 nRemaining = (uint16)(m_GameInfo.m_fTimeLimit - fTime);

					if(nRemaining != m_nTimeRemaining)
					{
						m_nTimeRemaining = nRemaining;

						// Now update the clients with the new time
						HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_TIME);
						hWrite->WriteWord(m_nTimeRemaining);
						g_pLTServer->EndMessage2(hWrite, MESSAGE_NAGGLE);
					}
				}
			}
		}


		// Start the next round if the conditions were met...
		if(bStartNextRound)
		{
			// Set the sub state for the next round waiting period...
			m_nServerSubState = MPMGR_SUBSTATE_TM_ROUND;
			m_fMiscTime = g_pLTServer->GetTime();
			m_nWaitSeconds = 0xFF;


			// Handle the scoring...
			for(i = 0; i < MAX_CLIENTS; i++)
			{
				if((m_ClientData[i].m_hClient != LTNULL) && (m_ClientData[i].m_nLives > 0))
				{
					if(nDefendersAlive > 0)
					{
						if(m_ClientData[i].m_Data.m_nCharacterClass == m_GameInfo.m_nTM_DefenderRace)
						{
							m_ClientData[i].m_nScore += MPMGR_OVERRUN_PTS;
							m_ClientData[i].m_nUpdateFlags |= MP_CLIENT_UPDATE_SCORE;

							m_TeamData[m_GameInfo.m_nTM_DefenderRace].m_nScore += MPMGR_OVERRUN_PTS;
							m_TeamData[m_GameInfo.m_nTM_DefenderRace].m_nUpdateFlags |= MP_CLIENT_UPDATE_SCORE;
						}
					}
					else
					{
						if(m_ClientData[i].m_Data.m_nCharacterClass == m_GameInfo.m_nTM_AttackerRace)
						{
							m_ClientData[i].m_nScore += MPMGR_OVERRUN_PTS;
							m_ClientData[i].m_nUpdateFlags |= MP_CLIENT_UPDATE_SCORE;

							m_TeamData[m_GameInfo.m_nTM_AttackerRace].m_nScore += MPMGR_OVERRUN_PTS;
							m_TeamData[m_GameInfo.m_nTM_AttackerRace].m_nUpdateFlags |= MP_CLIENT_UPDATE_SCORE;
						}
					}
				}
			}
		}
	}


	// Tell everyone that the sub state has changed...
	if(nOldSubState != m_nServerSubState)
	{
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SUBSTATE_CHANGED);
		hMessage->WriteByte(m_nServerSubState);
		g_pLTServer->EndMessage(hMessage);
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::Update_Evac()
//
// PURPOSE:		Do any constant updates for the game type
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::Update_Evac()
{
	// Count the number of defenders and attackers in the level
	int nDefenders = NumberOfClass(m_GameInfo.m_nTM_DefenderRace, LTTRUE);
	int nAttackers = NumberOfClass(m_GameInfo.m_nTM_AttackerRace, LTTRUE);
	int i;


	// Set the current sub state of the game
	uint8 nOldSubState = m_nServerSubState;


	// Handle the pre-game state...
	if(m_nServerSubState == MPMGR_SUBSTATE_NONE)
	{
		LTBOOL bCanStart = ((nDefenders > 0) && (nAttackers > 0));

		// Go through all the clients and make sure their observe states are correct
		for(i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_ClientData[i].m_hClient != LTNULL)
			{
				// If the teams are unbalanced... make everyone wait
				if(!bCanStart)
				{
					if(m_ClientData[i].m_nObserveMode != MP_OBSERVE_MODE_WAIT_TEAMS)
					{
						m_ClientData[i].m_nObserveMode = MP_OBSERVE_MODE_WAIT_TEAMS;
						ClientObserveCommand(m_ClientData[i].m_hClient, MP_OBSERVE_CHANGE_MODE);
					}

					m_fMiscTime = 0.0f;
					m_nWaitSeconds = 0xFF;
				}
				else
				{
					// The server controller can have a different observer state
					if(i == m_nServerController)
					{
						if(m_ClientData[i].m_nObserveMode != MP_OBSERVE_MODE_START_GAME)
						{
							m_ClientData[i].m_nObserveMode = MP_OBSERVE_MODE_START_GAME;
							ClientObserveCommand(m_ClientData[i].m_hClient, MP_OBSERVE_CHANGE_MODE);
						}
					}
					else
					{
						if(m_ClientData[i].m_nObserveMode != MP_OBSERVE_MODE_WAIT_START_GAME)
						{
							m_ClientData[i].m_nObserveMode = MP_OBSERVE_MODE_WAIT_START_GAME;
							ClientObserveCommand(m_ClientData[i].m_hClient, MP_OBSERVE_CHANGE_MODE);
						}
					}


					// Check to see if we need to set our wait time
					if(!m_fMiscTime)
					{
						m_fMiscTime = g_pLTServer->GetTime();
					}


					// Send the remaining amount of time for the start round delay
					LTFLOAT fWaitTime = MPMGR_ROUND_START_DELAY - (g_pLTServer->GetTime() - m_fMiscTime);

					if(fWaitTime < 0.0f)
						fWaitTime = 0.0f;

					uint8 nWait = (uint8)(fWaitTime);

					if(nWait != m_nWaitSeconds)
					{
						m_nWaitSeconds = nWait;

						HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_WAIT_SECONDS);
						hWrite->WriteByte(m_nWaitSeconds);
						g_pLTServer->EndMessage(hWrite);
					}

					// Go ahead and start the game if we're out of time...
					if(!m_nWaitSeconds)
					{
						OnMessage(LTNULL, MID_MPMGR_START_GAME, LTNULL);
					}
				}
			}
		}
	}
	// Handle waiting on the next round...
	else if(m_nServerSubState == MPMGR_SUBSTATE_TM_ROUND)
	{
		if(g_pLTServer->GetTime() >= (m_fMiscTime + MPMGR_ROUND_DELAY))
		{
			// Increment the round counter and send the data to the clients...
			m_nCurrentRound++;
			m_fStartTime = g_pLTServer->GetTime();

			LTBOOL bLastRound = !(!(m_GameInfo.m_nEndConditionFlags & MP_END_CONDITION_ROUNDS) || (m_nCurrentRound < m_GameInfo.m_nRoundLimit));

			if(!bLastRound)
			{
				HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_ROUND);
				hWrite->WriteByte(m_nCurrentRound + 1); // Add 1 to give the non-zero based round value
				g_pLTServer->EndMessage(hWrite);

				// Now respawn everyone again for the new round
				for(i = 0; i < MAX_CLIENTS; i++)
				{
					if(m_ClientData[i].m_hClient != LTNULL)
					{
						CPlayerObj *pPlayer = FindPlayerObj(m_ClientData[i].m_hClient);

						if(pPlayer)
						{
							// If this player was an exosuit then make sure the powerup 
							// gets restored
							if(IsExosuit(pPlayer->m_hObject))
								pPlayer->RespawnExosuit();

							// Reset the number of lives and respawn...
							uint8 nChar;
							if(m_ClientData[i].m_Data.m_nCharacterClass == m_GameInfo.m_nTM_DefenderRace)
							{
								m_ClientData[i].m_nLives = m_GameInfo.m_nTM_DefenderLives;
								nChar = m_ClientData[i].m_Data.m_nCharacterSet[m_GameInfo.m_nTM_DefenderRace];
							}
							else
							{
								m_ClientData[i].m_nLives = m_GameInfo.m_nTM_AttackerLives;
								nChar = m_ClientData[i].m_Data.m_nCharacterSet[m_GameInfo.m_nTM_AttackerRace];
							}

							m_ClientData[i].m_nUpdateFlags |= MP_CLIENT_UPDATE_LIVES;
							m_ClientData[i].m_nConsecutiveFrags = 0;

							pPlayer->MPRespawn(nChar);
						}
					}
				}

				// Make sure the level gets reset...
				g_pGameServerShell->ProcessMPStartPointMessages();

				// Put us back in normal mode
				m_nServerSubState = MPMGR_SUBSTATE_TM_GOING;
				m_nEvacState = 0;
				m_fMiscTime = 0.0f;
			}
		}
		else
		{
			// Send the remaining amount of time for the start round delay
			LTFLOAT fWaitTime = MPMGR_ROUND_DELAY - (g_pLTServer->GetTime() - m_fMiscTime);

			if(fWaitTime < 0.0f)
				fWaitTime = 0.0f;

			uint8 nWait = (uint8)(fWaitTime);

			if(nWait != m_nWaitSeconds)
			{
				m_nWaitSeconds = nWait;

				HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_WAIT_SECONDS);
				hWrite->WriteByte(m_nWaitSeconds);
				g_pLTServer->EndMessage(hWrite);
			}
		}
	}
	// Handle the during-game state
	else if(m_nServerSubState == MPMGR_SUBSTATE_TM_GOING)
	{
		// See if we might need to start the next round...
		LTBOOL bStartNextRound = LTFALSE;
		LTBOOL bTimeExpired = LTFALSE;


		// Go through and count the number of people who still have lives...
		int nDefendersAlive = 0;
		int nAttackersAlive = 0;

		// Go through all the clients and add them up
		for(i = 0; i < MAX_CLIENTS; i++)
		{
			if((m_ClientData[i].m_hClient != LTNULL) && (m_ClientData[i].m_nLives > 0))
			{
				if(m_ClientData[i].m_Data.m_nCharacterClass == m_GameInfo.m_nTM_DefenderRace)
					nDefendersAlive++;
				else
					nAttackersAlive++;
			}
		}


		// Check the number of people in the Evac zone here...
		int nEvacAmount = 0;
		LTBOOL bOldDisplayEvacZone;

		for(i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_ClientData[i].m_hClient != LTNULL)
			{
				bOldDisplayEvacZone = m_ClientData[i].m_bDisplayEvacZone;

				if(m_ClientData[i].m_Data.m_nCharacterClass == m_GameInfo.m_nTM_DefenderRace)
				{
					CPlayerObj *pPlayer = FindPlayerObj(m_ClientData[i].m_hClient);

					if(pPlayer && (pPlayer->GetState() == PS_ALIVE))
					{
						m_ClientData[i].m_bDisplayEvacZone = LTTRUE;

						LTVector vPos;
						g_pLTServer->GetObjectPos(pPlayer->m_hObject, &vPos);

						// Setup some temporary container storage
						HOBJECT hContainers[CM_MAX_CONTAINERS];
						uint32 nNumContainers = 0;

						// Retrieve the position containers
						nNumContainers = g_pLTServer->GetPointContainers(&vPos, hContainers, CM_MAX_CONTAINERS);

						// Check our volume stuff..
						uint16 nCode;

						for(uint32 i = 0; i < nNumContainers; i++)
						{
							g_pLTServer->GetContainerCode(hContainers[i], &nCode);

							if((ContainerCode)nCode == CC_EVACZONE)
							{
								nEvacAmount++;
								m_ClientData[i].m_bDisplayEvacZone = LTFALSE;

								break;
							}
						}
					}
				}
				else
					m_ClientData[i].m_bDisplayEvacZone = LTFALSE;


				if(bOldDisplayEvacZone != m_ClientData[i].m_bDisplayEvacZone)
				{
					// Send the evac zone state to the individual client
					HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_ClientData[i].m_hClient, MID_MPMGR_SERVER_MESSAGE);
					hMessage->WriteByte(MP_EVAC_STATE);
					hMessage->WriteByte(m_ClientData[i].m_bDisplayEvacZone);
					g_pLTServer->EndMessage(hMessage);
				}
			}
		}


		// Update the state of the evacuation...
		uint8 nOldEvacState = m_nEvacState;

		switch(nOldEvacState)
		{
			case 0:
			{
				if(nEvacAmount > 0)
				{
					m_fMiscTime = g_pLTServer->GetTime();
					m_nEvacState = MP_EVAC_COUNTDOWN;
				}

				break;
			}

			case MP_EVAC_COUNTDOWN:
			{
				if((nEvacAmount < 1) || !nDefendersAlive)
				{
					m_nEvacState = MP_EVAC_CANCEL;
					m_fMiscTime = 0.0f;
				}
				else if(g_pLTServer->GetTime() >= m_fMiscTime + MPMGR_EVAC_DELAY)
				{
					m_nEvacState = MP_EVAC_COMPLETE;
					m_fMiscTime = 0.0f;
				}

				break;
			}

			case MP_EVAC_CANCEL:
			{
				m_nEvacState = 0;
				m_fMiscTime = 0.0f;
				break;
			}
		}


		// If all the defenders or attackers are gone... start the next round.
		if((m_nEvacState == MP_EVAC_COMPLETE) || !nDefendersAlive || !nAttackersAlive)
		{
			bStartNextRound = LTTRUE;
		}
		else
		{
			if(m_GameInfo.m_fTimeLimit > 0.0f)
			{
				LTFLOAT fTime = g_pLTServer->GetTime() - m_fStartTime;

				if(fTime >= m_GameInfo.m_fTimeLimit)
				{
					bStartNextRound = LTTRUE;
					bTimeExpired = LTTRUE;

					if(m_nEvacState == MP_EVAC_COUNTDOWN)
						m_nEvacState = MP_EVAC_CANCEL;
				}
				else
				{
					uint16 nRemaining = (uint16)(m_GameInfo.m_fTimeLimit - fTime);

					if(nRemaining != m_nTimeRemaining)
					{
						m_nTimeRemaining = nRemaining;

						// Now update the clients with the new time
						HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_TIME);
						hWrite->WriteWord(m_nTimeRemaining);
						g_pLTServer->EndMessage2(hWrite, MESSAGE_NAGGLE);
					}
				}
			}
		}


		// Send this here because of the potential setting in the time check above...
		if(nOldEvacState != m_nEvacState)
		{
			if(m_nEvacState)
			{
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
				hMessage->WriteByte(m_nEvacState);
				g_pLTServer->EndMessage(hMessage);
			}
		}


		// Start the next round if the conditions were met...
		if(bStartNextRound)
		{
			// Set the sub state for the next round waiting period...
			m_nServerSubState = MPMGR_SUBSTATE_TM_ROUND;
			m_fMiscTime = g_pLTServer->GetTime();
			m_nWaitSeconds = 0xFF;


			// Handle the scoring...
			if(bTimeExpired || !nDefendersAlive)
			{
				m_TeamData[m_GameInfo.m_nTM_AttackerRace].m_nScore += MPMGR_EVAC_PTS;
				m_TeamData[m_GameInfo.m_nTM_AttackerRace].m_nUpdateFlags |= MP_CLIENT_UPDATE_SCORE;
			}
			else
			{
				m_TeamData[m_GameInfo.m_nTM_DefenderRace].m_nScore += MPMGR_EVAC_PTS;
				m_TeamData[m_GameInfo.m_nTM_DefenderRace].m_nUpdateFlags |= MP_CLIENT_UPDATE_SCORE;
			}
		}
	}


	// Tell everyone that the sub state has changed...
	if(nOldSubState != m_nServerSubState)
	{
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
		hMessage->WriteByte(MP_SUBSTATE_CHANGED);
		hMessage->WriteByte(m_nServerSubState);
		g_pLTServer->EndMessage(hMessage);
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::OnInfoQuery()
//
// PURPOSE:		Update basic game information
//
// -------------------------------------------------------------------- //

BOOL MultiplayerMgr::OnInfoQuery()
{
	HSTRING hDesc = LTNULL;
	char *szMap = GetBaseMapName(m_GameInfo.m_szLevels[m_nCurrentLevel]);

	switch(m_GameInfo.m_nGameType.Get())
	{
#ifdef _WIN32
		case MP_DM:			hDesc = g_pLTServer->FormatString(IDS_GAMETYPE_DM);			break;
		case MP_TEAMDM:		hDesc = g_pLTServer->FormatString(IDS_GAMETYPE_TEAMDM);		break;
		case MP_HUNT:		hDesc = g_pLTServer->FormatString(IDS_GAMETYPE_HUNT);		break;
		case MP_SURVIVOR:	hDesc = g_pLTServer->FormatString(IDS_GAMETYPE_SURVIVOR);	break;
		case MP_OVERRUN:	hDesc = g_pLTServer->FormatString(IDS_GAMETYPE_OVERRUN);	break;
		case MP_EVAC:		hDesc = g_pLTServer->FormatString(IDS_GAMETYPE_EVAC);		break;
#else
		case MP_DM:			hDesc = g_pLTServer->CreateString("DM");					break;
		case MP_TEAMDM:		hDesc = g_pLTServer->CreateString("Team DM");				break;
		case MP_HUNT:		hDesc = g_pLTServer->CreateString("Hunt");					break;
		case MP_SURVIVOR:	hDesc = g_pLTServer->CreateString("Survivor");				break;
		case MP_OVERRUN:	hDesc = g_pLTServer->CreateString("Overrun");				break;
		case MP_EVAC:		hDesc = g_pLTServer->CreateString("Evac");					break;
#endif
	}

	char szName[128];
	LTBOOL bIsLV = g_pGameServerShell->LowViolence();
	sprintf(szName, "%s%s%s", bIsLV?"[LowViol] ":"",m_GameInfo.m_szName, m_GameInfo.m_bDedicated ? " [D]" : "");

	SendResponseInfo("hostname", szName);
	SendResponseInfo("hostport", GetGamePort());
	SendResponseInfo("mapname", szMap);
	SendResponseInfo("gametype", g_pLTServer->GetStringData(hDesc));
	SendResponseInfo("gamemode", "openplaying");
	SendResponseInfo("numplayers", m_nClients);
	SendResponseInfo("maxplayers", m_GameInfo.m_nPlayerLimit);
	SendResponseInfo("lock", m_GameInfo.m_bLocked);
	SendResponseInfo("ded", m_GameInfo.m_bDedicated);
	SendResponseInfo("bandwidth", m_GameInfo.m_nBandwidth);

	if(hDesc)
	{
		g_pLTServer->FreeString(hDesc);
		hDesc = LTNULL;
	}

	return TRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::OnRulesQuery()
//
// PURPOSE:		Update extended game information
//
// -------------------------------------------------------------------- //

BOOL MultiplayerMgr::OnRulesQuery()
{
	SendResponseInfo("maxa", m_GameInfo.m_nTeamLimits[Alien]);
	SendResponseInfo("maxm", m_GameInfo.m_nTeamLimits[Marine]);
	SendResponseInfo("maxp", m_GameInfo.m_nTeamLimits[Predator]);
	SendResponseInfo("maxc", m_GameInfo.m_nTeamLimits[Corporate]);

	SendResponseInfo("frags", m_GameInfo.m_nFragLimit);
	SendResponseInfo("score", m_GameInfo.m_nScoreLimit);
	SendResponseInfo("time", (int)(m_GameInfo.m_fTimeLimit));
	SendResponseInfo("rounds", m_GameInfo.m_nRoundLimit);

	SendResponseInfo("lc", (int)m_GameInfo.m_bLifecycle);

	SendResponseInfo("hrace", m_GameInfo.m_nHunt_HunterRace);
	SendResponseInfo("prace", m_GameInfo.m_nHunt_PreyRace);
	SendResponseInfo("ratio", m_GameInfo.m_nHunt_Ratio);

	SendResponseInfo("srace", m_GameInfo.m_nSurv_SurvivorRace);
	SendResponseInfo("mrace", m_GameInfo.m_nSurv_MutateRace);

	SendResponseInfo("drace", m_GameInfo.m_nTM_DefenderRace);
	SendResponseInfo("dlive", m_GameInfo.m_nTM_DefenderLives);
	SendResponseInfo("arace", m_GameInfo.m_nTM_AttackerRace);
	SendResponseInfo("alive", m_GameInfo.m_nTM_AttackerLives);

	SendResponseInfo("speed", (int)(m_GameInfo.m_fSpeedScale * 100.0f));
	SendResponseInfo("respawn", (int)(m_GameInfo.m_fPowerupRespawnScale * 100.0f));
	SendResponseInfo("damage", (int)(m_GameInfo.m_fDamageScale * 100.0f));
	SendResponseInfo("hitloc", (int)(m_GameInfo.m_bLocationDamage));
	SendResponseInfo("ff", (int)(m_GameInfo.m_bFriendlyFire));
	SendResponseInfo("fn", (int)(m_GameInfo.m_bFriendlyNames));
	SendResponseInfo("mask", (int)(m_GameInfo.m_bMaskLoss));
	SendResponseInfo("class", (int)(m_GameInfo.m_bClassWeapons));
	SendResponseInfo("exosuit", (int)(m_GameInfo.m_nExosuit));
	SendResponseInfo("queen", (int)(m_GameInfo.m_nQueenMolt));


	// Go through the whole score list
	int nCustom = 0;

	for(int i = 0; i < MAX_MP_SCORING_CLASSES; i++)
	{
		if((m_GameInfo.m_nScoreValues[i] > 0) && (m_GameInfo.m_nScoreValues[i] != CServerOptions::eDefaultScoringValue))
		{
			nCustom = 1;
			break;
		}
	}

	SendResponseInfo("cscore", nCustom);


	return TRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::OnPlayersQuery()
//
// PURPOSE:		Update player information
//
// -------------------------------------------------------------------- //

BOOL MultiplayerMgr::OnPlayersQuery()
{
	// Send the info for each player...
	char sPlayer[32];
	char sRace[32];
//	char sFrags[32];
//	char sScore[32];
	char sPing[32];

	int nClient = 0;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient)
		{
			sprintf(sPlayer, "player_%i", nClient);
			SendResponseInfo(sPlayer, m_ClientData[i].m_Data.m_szName);

			sprintf(sRace, "race_%i", nClient);
			SendResponseInfo(sRace, m_ClientData[i].m_Data.m_nCharacterClass);

/*			sprintf(sFrags, "frags_%i", nClient);
			SendResponseInfo(sFrags, m_ClientData[i].m_nFrags);

			sprintf(sScore, "score_%i", nClient);
			SendResponseInfo(sScore, m_ClientData[i].m_nScore);
*/
			sprintf(sPing, "ping_%i", nClient);
			int nPing = m_ClientData[i].m_nPing;
			if (nPing <= 0) nPing = 1;
			SendResponseInfo(sPing, nPing);

			nClient++;
		}
	}

	return TRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::StartSinglePlayer()
//
// PURPOSE:		Should be called at the start of a single player level.
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::StartSinglePlayer()
{
	// Get the new game info.
	UpdateGameInfo();
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::SendGameInfo()
//
// PURPOSE:		Send all the current game data to a particular client
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::SendGameInfo(HCLIENT hClient)
{
	uint8 i = 0;

	// Start a game info message
	HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(hClient, MID_MPMGR_GAME_INFO);


	// Fill in the basic information about the game
	hWrite->WriteString(m_GameInfo.m_szName);
	hWrite->WriteString(GetBaseMapName(m_GameInfo.m_szLevels[m_nCurrentLevel]));
	hWrite->WriteByte((uint8)m_GameInfo.m_nGameType.Get());


	// Fill in the ending conditions
	hWrite->WriteByte(m_GameInfo.m_nFragLimit);
	hWrite->WriteWord(m_GameInfo.m_nScoreLimit);
	hWrite->WriteWord((uint16)m_GameInfo.m_fTimeLimit);
	hWrite->WriteByte(m_GameInfo.m_nRoundLimit);
	hWrite->WriteByte(m_nCurrentRound + 1);

	// Fill in the current sub state
	hWrite->WriteByte(m_nServerSubState);

	// Fill in the special game parameters that the client needs to know
	hWrite->WriteFloat(m_GameInfo.m_fSpeedScale);
	hWrite->WriteByte(m_GameInfo.m_bClassWeapons);
	hWrite->WriteByte(m_GameInfo.m_nExosuit);
	hWrite->WriteByte(m_GameInfo.m_nQueenMolt);
	hWrite->WriteDWord(m_GameInfo.m_nBandwidth);


	// Fill in the current information about all the other clients
	hWrite->WriteByte(m_GameInfo.m_nPlayerLimit);

	for(i = 0; i < MAX_TEAMS; i++)
	{
		hWrite->WriteByte(m_GameInfo.m_nTeamLimits[i]);
		hWrite->WriteWord(m_TeamData[i].m_nFrags);
		hWrite->WriteWord(m_TeamData[i].m_nScore);
	}

	hWrite->WriteByte(m_nClients);

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if((m_ClientData[i].m_hClient != LTNULL) && (m_ClientData[i].m_hClient != hClient))
		{
			uint8 nClass = m_ClientData[i].m_Data.m_nCharacterClass;

			hWrite->WriteString(m_ClientData[i].m_Data.m_szName);
			hWrite->WriteByte(nClass);
			hWrite->WriteByte(m_ClientData[i].m_Data.m_nCharacterSet[nClass]);

			hWrite->WriteDWord(m_ClientData[i].m_nClientID);

			hWrite->WriteWord(m_ClientData[i].m_nPing);
			hWrite->WriteWord(m_ClientData[i].m_nFrags);
			hWrite->WriteByte(m_ClientData[i].m_nConsecutiveFrags);
			hWrite->WriteWord(m_ClientData[i].m_nScore);
			hWrite->WriteByte(m_ClientData[i].m_nLives);

			hWrite->WriteByte(m_ClientData[i].m_bVerified);

			hWrite->WriteByte(m_ClientData[i].m_bObserving);
		}
	}

	if(m_nServerController != 0xFF)
		hWrite->WriteDWord(m_ClientData[m_nServerController].m_nClientID);
	else
		hWrite->WriteDWord(-1);


	// Only send the evac zone if it's the proper game type...
	if(m_GameInfo.m_nGameType == MP_EVAC)
	{
		int nEvacZoneObjects = g_pGameServerShell->GetEvacZoneObjects();
		LTVector vEvacZonePos = g_pGameServerShell->GetEvacZonePos();

#ifndef _FINAL
		// Make sure there's the proper amount
		if(nEvacZoneObjects != 1)
			g_pLTServer->CPrint("Warning!! Improper number of EvacZone objects! %d", nEvacZoneObjects);
#endif
		hWrite->WriteVector(vEvacZonePos);
	}

	// Now send some more goodies for the client's rulez display
	*hWrite << m_GameInfo.m_nHunt_HunterRace;							// The race that the Hunters play
	*hWrite << m_GameInfo.m_nHunt_PreyRace;								// The race that the Prey play
	*hWrite << m_GameInfo.m_nHunt_Ratio;								// The ratio of hunters to prey
																		
	*hWrite << m_GameInfo.m_nSurv_SurvivorRace;							// The race that the Survivors play
	*hWrite << m_GameInfo.m_nSurv_MutateRace;							// The race that the Mutants play
																		
	*hWrite << m_GameInfo.m_nTM_DefenderRace;							// The race that the Defending team plays
	*hWrite << m_GameInfo.m_nTM_DefenderLives;							// The number of lives the Defenders get
	*hWrite << m_GameInfo.m_nTM_AttackerRace;							// The race that the Attacking team plays
	*hWrite << m_GameInfo.m_nTM_AttackerLives;							// The number of lives the Attackers get

	*hWrite << m_GameInfo.m_bLifecycle;									// Is the alien lifecycle option on?

	*hWrite << m_GameInfo.m_fPowerupRespawnScale;						// Powerup spawn scale
	*hWrite << m_GameInfo.m_fDamageScale;								// Damage scale

	*hWrite << m_GameInfo.m_bLocationDamage;							// Location damage?
	*hWrite << m_GameInfo.m_bFriendlyFire;								// Friendly fire?
	*hWrite << m_GameInfo.m_bFriendlyNames;								// Display friendly names only?
	*hWrite << m_GameInfo.m_bMaskLoss;									// Can pred lose his mask?

	// Go through the whole score list
	{
		LTBOOL bCustom = LTFALSE;

		for(int i = 0; i < MAX_MP_SCORING_CLASSES; i++)
		{
			if((m_GameInfo.m_nScoreValues[i] > 0) && (m_GameInfo.m_nScoreValues[i] != CServerOptions::eDefaultScoringValue))
			{
				bCustom = LTTRUE;
				break;
			}
		}

		*hWrite << bCustom;									// Is there custom scoring?		
	}

	// Send it!
	g_pLTServer->EndMessage(hWrite);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::UpdateGameInfo()
//
// PURPOSE:		Mass Destruction!! Bwah ha ha!
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::UpdateGameInfo()
{
	// Fill in the current game information
	MPGameInfo *pGameInfo;
	uint32 dwLen = sizeof(MPGameInfo);

	g_pLTServer->GetGameInfo((void**)&pGameInfo, &dwLen);

	if(pGameInfo)
	{
		if(dwLen == sizeof(GameType))
		{
			// Just clear the game info and copy the game type over
			memset(&m_GameInfo, 0, sizeof(MPGameInfo));
			memcpy(&m_GameInfo.m_nGameType, pGameInfo, sizeof(GameType));
		}
		else
		{
			// Now setup specific game type information
			SetupGameInfo(pGameInfo);

			// Copy over the data sent up from the client... we'll need
			// to reference it quite a bit.
			memcpy(&m_GameInfo, pGameInfo, sizeof(MPGameInfo));
		}

		return LTTRUE;
	}


	// If our game info didn't exist... we'll just assume
	// it's a single player story game.
	memset(&m_GameInfo, 0, sizeof(MPGameInfo));
	m_GameInfo.m_nGameType = GameType(SP_STORY);

	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::SetupGameInfo()
//
// PURPOSE:		Setup type specific game info... this is mainly to make
//				sure that a client can't cheat and setup the game with
//				values that we don't want them to use
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::SetupGameInfo(MPGameInfo *pInfo)
{
	// Set the max server bandwidth
	char szTemp[64];
	sprintf(szTemp, "SendBandwidth %d", pInfo->m_nBandwidth);
	g_pLTServer->RunGameConString(szTemp);


	// Do the advanced options
	pInfo->m_fSpeedScale = 1.0f;
	pInfo->m_fPowerupRespawnScale = 1.0f;

	if(pInfo->m_fDamageScale < 0.25f)
		pInfo->m_fDamageScale = 0.25f;
	else if(pInfo->m_fDamageScale > 2.0f)
		pInfo->m_fDamageScale = 2.0f;


	// Check some general values
	if(pInfo->m_nPlayerLimit < 2)
		pInfo->m_nPlayerLimit = 2;

	if(pInfo->m_fTimeLimit < 0.0f)
		pInfo->m_fTimeLimit = 0.0f;


	// Double check demo settings...
#ifdef _DEMO
	if((pInfo->m_nGameType.Get() != MP_DM) && (pInfo->m_nGameType.Get() != MP_TEAMDM))
		pInfo->m_nGameType.Set(MP_DM);

	pInfo->m_bLifecycle = LTFALSE;
#endif


	// Check the game type specifics
	switch(pInfo->m_nGameType.Get())
	{
		case MP_DM:
		case MP_TEAMDM:
		{
			pInfo->m_nRoundLimit = 0;

			pInfo->m_nEndConditionFlags = 0;
			if(pInfo->m_nFragLimit > 0)		pInfo->m_nEndConditionFlags |= MP_END_CONDITION_FRAGS;
			if(pInfo->m_nScoreLimit > 0)	pInfo->m_nEndConditionFlags |= MP_END_CONDITION_SCORE;
			if(pInfo->m_fTimeLimit > 0)		pInfo->m_nEndConditionFlags |= MP_END_CONDITION_TIME;

			int nMaxTotal = 0;

			for(int i = 0; i < MAX_TEAMS; i++)
				nMaxTotal += pInfo->m_nTeamLimits[i];

			if(nMaxTotal < pInfo->m_nPlayerLimit)
				pInfo->m_nTeamLimits[Marine] += (pInfo->m_nPlayerLimit - nMaxTotal);

			return LTTRUE;
		}

		case MP_HUNT:
		{
			pInfo->m_nScoreLimit = 0;
			pInfo->m_nRoundLimit = 0;

			pInfo->m_nEndConditionFlags = 0;
			if(pInfo->m_nFragLimit > 0)		pInfo->m_nEndConditionFlags |= MP_END_CONDITION_FRAGS;
			if(pInfo->m_fTimeLimit > 0)		pInfo->m_nEndConditionFlags |= MP_END_CONDITION_TIME;

			if( (pInfo->m_nHunt_HunterRace == pInfo->m_nHunt_PreyRace) ||
				(pInfo->m_nHunt_HunterRace >= MAX_TEAMS) || (pInfo->m_nHunt_PreyRace >= MAX_TEAMS))
			{
				pInfo->m_nHunt_HunterRace = Predator;
				pInfo->m_nHunt_PreyRace = Marine;
			}

			if(pInfo->m_nHunt_Ratio > MAX_CLIENTS)
				pInfo->m_nHunt_Ratio = MAX_CLIENTS;

			for(int i = 0; i < MAX_TEAMS; i++)
				pInfo->m_nTeamLimits[i] = 0;

			pInfo->m_nTeamLimits[pInfo->m_nHunt_HunterRace] = MAX_CLIENTS;
			pInfo->m_nTeamLimits[pInfo->m_nHunt_PreyRace] = MAX_CLIENTS;

			pInfo->m_bLifecycle = LTFALSE;

			return LTTRUE;
		}

		case MP_SURVIVOR:
		{
			pInfo->m_nFragLimit = 0;

			pInfo->m_nEndConditionFlags = 0;
			if(pInfo->m_nScoreLimit > 0)	pInfo->m_nEndConditionFlags |= MP_END_CONDITION_SCORE;
			if(pInfo->m_nRoundLimit > 0)	pInfo->m_nEndConditionFlags |= MP_END_CONDITION_ROUNDS;

			if(	(pInfo->m_nSurv_SurvivorRace == pInfo->m_nSurv_MutateRace) ||
				(pInfo->m_nSurv_SurvivorRace >= MAX_TEAMS) || (pInfo->m_nSurv_MutateRace >= MAX_TEAMS))
			{
				pInfo->m_nSurv_SurvivorRace = Marine;
				pInfo->m_nSurv_MutateRace = Alien;
			}

			for(int i = 0; i < MAX_TEAMS; i++)
				pInfo->m_nTeamLimits[i] = 0;

			pInfo->m_nTeamLimits[pInfo->m_nSurv_SurvivorRace] = MAX_CLIENTS;
			pInfo->m_nTeamLimits[pInfo->m_nSurv_MutateRace] = MAX_CLIENTS;

			pInfo->m_bLifecycle = LTFALSE;

			return LTTRUE;
		}

		case MP_OVERRUN:
		{
			pInfo->m_nEndConditionFlags = 0;
			if(pInfo->m_nRoundLimit > 0)	pInfo->m_nEndConditionFlags |= MP_END_CONDITION_ROUNDS;

			if(	(pInfo->m_nTM_DefenderRace == pInfo->m_nTM_AttackerRace) ||
				(pInfo->m_nTM_DefenderRace >= MAX_TEAMS) || (pInfo->m_nTM_AttackerRace >= MAX_TEAMS))
			{
				pInfo->m_nTM_DefenderRace = Corporate;
				pInfo->m_nTM_AttackerRace = Alien;
			}

			for(int i = 0; i < MAX_TEAMS; i++)
				pInfo->m_nTeamLimits[i] = 0;

			pInfo->m_nTeamLimits[pInfo->m_nTM_DefenderRace] = MAX_CLIENTS;
			pInfo->m_nTeamLimits[pInfo->m_nTM_AttackerRace] = MAX_CLIENTS;

			if(pInfo->m_nTM_DefenderLives < 1)
				pInfo->m_nTM_DefenderLives = 1;

			if(pInfo->m_nTM_AttackerLives < 1)
				pInfo->m_nTM_AttackerLives = 1;

			pInfo->m_bLifecycle = LTFALSE;

			return LTTRUE;
		}

		case MP_EVAC:
		{
			pInfo->m_nEndConditionFlags = 0;
			if(pInfo->m_nRoundLimit > 0)	pInfo->m_nEndConditionFlags |= MP_END_CONDITION_ROUNDS;

			if(	(pInfo->m_nTM_DefenderRace == pInfo->m_nTM_AttackerRace) ||
				(pInfo->m_nTM_DefenderRace >= MAX_TEAMS) || (pInfo->m_nTM_AttackerRace >= MAX_TEAMS))
			{
				pInfo->m_nTM_DefenderRace = Marine;
				pInfo->m_nTM_AttackerRace = Alien;
			}

			for(int i = 0; i < MAX_TEAMS; i++)
				pInfo->m_nTeamLimits[i] = 0;

			pInfo->m_nTeamLimits[pInfo->m_nTM_DefenderRace] = MAX_CLIENTS;
			pInfo->m_nTeamLimits[pInfo->m_nTM_AttackerRace] = MAX_CLIENTS;

			if(pInfo->m_nTM_DefenderLives < 1)
				pInfo->m_nTM_DefenderLives = 1;

			if(pInfo->m_nTM_AttackerLives < 1)
				pInfo->m_nTM_AttackerLives = 1;

			pInfo->m_bLifecycle = LTFALSE;

			return LTTRUE;
		}
	}


	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::CheckEndConditions()
//
// PURPOSE:		Returns TRUE if one of the end conditions is met
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::CheckEndConditions()
{
	switch(m_GameInfo.m_nGameType.Get())
	{
		case MP_DM:			return CheckEndConditions_DM();
		case MP_TEAMDM:		return CheckEndConditions_TeamDM();
		case MP_HUNT:		return CheckEndConditions_Hunt();
		case MP_SURVIVOR:	return CheckEndConditions_Survivor();
		case MP_OVERRUN:	return CheckEndConditions_Overrun();
		case MP_EVAC:		return CheckEndConditions_Evac();
	}

	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::CheckEndConditions_DM()
//
// PURPOSE:		Returns TRUE if one of the end conditions is met
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::CheckEndConditions_DM()
{
	// Check for a client with the max frag count
	if(m_GameInfo.m_nEndConditionFlags & MP_END_CONDITION_FRAGS)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_ClientData[i].m_hClient != LTNULL)
			{
				if(m_ClientData[i].m_nFrags >= (int16)m_GameInfo.m_nFragLimit)
					return LTTRUE;
			}
		}
	}

	// Check for a client with the max score
	if(m_GameInfo.m_nEndConditionFlags & MP_END_CONDITION_SCORE)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_ClientData[i].m_hClient != LTNULL)
			{
				if(m_ClientData[i].m_nScore >= (int16)m_GameInfo.m_nScoreLimit)
					return LTTRUE;
			}
		}
	}

	// Check for an expired time
	if(m_GameInfo.m_nEndConditionFlags & MP_END_CONDITION_TIME)
	{
		if(g_pLTServer->GetTime() >= m_GameInfo.m_fTimeLimit)
			return LTTRUE;

		uint16 nRemaining = (uint16)(m_GameInfo.m_fTimeLimit - g_pLTServer->GetTime());

		if(nRemaining != m_nTimeRemaining)
		{
			m_nTimeRemaining = nRemaining;

			// Now update the clients with the new time
			HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_TIME);
			hWrite->WriteWord(m_nTimeRemaining);
			g_pLTServer->EndMessage2(hWrite, MESSAGE_NAGGLE);
		}
	}


	// Ok... just keep going then
	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::CheckEndConditions_TeamDM()
//
// PURPOSE:		Returns TRUE if one of the end conditions is met
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::CheckEndConditions_TeamDM()
{
	// Check for a client with the max frag count
	if(m_GameInfo.m_nEndConditionFlags & MP_END_CONDITION_FRAGS)
	{
		for(int i = 0; i < MAX_TEAMS; i++)
		{
			if(m_TeamData[i].m_nFrags >= (int16)m_GameInfo.m_nFragLimit)
				return LTTRUE;
		}
	}

	// Check for a client with the max score
	if(m_GameInfo.m_nEndConditionFlags & MP_END_CONDITION_SCORE)
	{
		for(int i = 0; i < MAX_TEAMS; i++)
		{
			if(m_TeamData[i].m_nScore >= (int16)m_GameInfo.m_nScoreLimit)
				return LTTRUE;
		}
	}

	// Check for an expired time
	if(m_GameInfo.m_nEndConditionFlags & MP_END_CONDITION_TIME)
	{
		if(g_pLTServer->GetTime() >= m_GameInfo.m_fTimeLimit)
			return LTTRUE;

		uint16 nRemaining = (uint16)(m_GameInfo.m_fTimeLimit - g_pLTServer->GetTime());

		if(nRemaining != m_nTimeRemaining)
		{
			m_nTimeRemaining = nRemaining;

			// Now update the clients with the new time
			HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_TIME);
			hWrite->WriteWord(m_nTimeRemaining);
			g_pLTServer->EndMessage2(hWrite, MESSAGE_NAGGLE);
		}
	}


	// Ok... just keep going then
	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::CheckEndConditions_Hunt()
//
// PURPOSE:		Returns TRUE if one of the end conditions is met
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::CheckEndConditions_Hunt()
{
	// Check for a client with the max frag count
	if(m_GameInfo.m_nEndConditionFlags & MP_END_CONDITION_FRAGS)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_ClientData[i].m_hClient != LTNULL)
			{
				if(m_ClientData[i].m_nFrags >= (int16)m_GameInfo.m_nFragLimit)
					return LTTRUE;
			}
		}
	}

	// Check for an expired time
	if(m_GameInfo.m_nEndConditionFlags & MP_END_CONDITION_TIME)
	{
		if(g_pLTServer->GetTime() >= m_GameInfo.m_fTimeLimit)
			return LTTRUE;

		uint16 nRemaining = (uint16)(m_GameInfo.m_fTimeLimit - g_pLTServer->GetTime());

		if(nRemaining != m_nTimeRemaining)
		{
			m_nTimeRemaining = nRemaining;

			// Now update the clients with the new time
			HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_MPMGR_TIME);
			hWrite->WriteWord(m_nTimeRemaining);
			g_pLTServer->EndMessage2(hWrite, MESSAGE_NAGGLE);
		}
	}


	// Ok... just keep going then
	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::CheckEndConditions_Survivor()
//
// PURPOSE:		Returns TRUE if one of the end conditions is met
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::CheckEndConditions_Survivor()
{
	// Check for a client with the max score
	if(m_GameInfo.m_nEndConditionFlags & MP_END_CONDITION_SCORE)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_ClientData[i].m_hClient != LTNULL)
			{
				if(m_ClientData[i].m_nScore >= (int16)m_GameInfo.m_nScoreLimit)
					return LTTRUE;
			}
		}
	}

	// Check for the max rounds
	if(m_GameInfo.m_nEndConditionFlags & MP_END_CONDITION_ROUNDS)
	{
		if(m_nCurrentRound >= m_GameInfo.m_nRoundLimit)
			return LTTRUE;
	}


	// Ok... just keep going then
	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::CheckEndConditions_Overrun()
//
// PURPOSE:		Returns TRUE if one of the end conditions is met
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::CheckEndConditions_Overrun()
{
	// Make sure we're actually playing the game...
	if((m_nServerSubState != MPMGR_SUBSTATE_TM_GOING) && (m_nServerSubState != MPMGR_SUBSTATE_TM_ROUND))
		return LTFALSE;


	// The game is over if an entire team quits.
	int nDefenders = NumberOfClass(m_GameInfo.m_nTM_DefenderRace, LTTRUE);
	int nAttackers = NumberOfClass(m_GameInfo.m_nTM_AttackerRace, LTTRUE);

	if(!nDefenders || !nAttackers)
		return LTTRUE;


	// Check for the max rounds
	if(m_GameInfo.m_nEndConditionFlags & MP_END_CONDITION_ROUNDS)
	{
		if(m_nCurrentRound >= m_GameInfo.m_nRoundLimit)
			return LTTRUE;
	}


	// Ok... just keep going then
	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::CheckEndConditions_Evac()
//
// PURPOSE:		Returns TRUE if one of the end conditions is met
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::CheckEndConditions_Evac()
{
	// Make sure we're actually playing the game...
	if((m_nServerSubState != MPMGR_SUBSTATE_TM_GOING) && (m_nServerSubState != MPMGR_SUBSTATE_TM_ROUND))
		return LTFALSE;


	// The game is over if an entire team quits.
	int nDefenders = NumberOfClass(m_GameInfo.m_nTM_DefenderRace, LTTRUE);
	int nAttackers = NumberOfClass(m_GameInfo.m_nTM_AttackerRace, LTTRUE);

	if(!nDefenders || !nAttackers)
		return LTTRUE;


	// Check for the max rounds
	if(m_GameInfo.m_nEndConditionFlags & MP_END_CONDITION_ROUNDS)
	{
		if(m_nCurrentRound >= m_GameInfo.m_nRoundLimit)
			return LTTRUE;
	}


	// Ok... just keep going then
	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::CheckVerification()
//
// PURPOSE:		Return true if all clients have verified
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerMgr::CheckVerification()
{
	LTBOOL bOk = LTTRUE;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_hClient != LTNULL)
		{
			if(!m_ClientData[i].m_bVerified)
				bOk = LTFALSE;
		}
	}

	return bOk;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ClearVerification()
//
// PURPOSE:		Clear all clients verification values
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ClearVerification()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		m_ClientData[i].m_bVerified = LTFALSE;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::LoadLevel()
//
// PURPOSE:		Load a specific level
//
// -------------------------------------------------------------------- //

LTRESULT MultiplayerMgr::LoadLevel(char *szLevel)
{
	// Reset all the stuff that needs to be reset for each client
	ResetAllClients();

#ifdef _DEMO
#ifdef _WIN32
	szLevel = "Worlds\\Multi\\DM\\DMDemo_ALesserFate";
#else
	szLevel = "Worlds/Multi/DM/DMDemo_ALesserFate";
#endif	// _WIN32
#endif

	// Load the next level...
	LTRESULT ltResult = LT_ERROR;

	if(szLevel)
	{
		ltResult = g_pLTServer->LoadWorld(szLevel, LOADWORLD_LOADWORLDOBJECTS | LOADWORLD_RUNWORLD);

		if(ltResult != LT_OK)
		{
			g_pLTServer->BPrint("Error loading multiplayer level '%s'", szLevel);
		}
	}

	return ltResult;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::SetPrevLevel
//
// PURPOSE:		Go back to the previous level in the list
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::SetPrevLevel()
{
	// Find what our next level will be
	m_nNextLevel = m_nNextLevel - 1;

	if(m_nNextLevel > m_GameInfo.m_nNumLevels - 1)
		m_nNextLevel = m_GameInfo.m_nNumLevels - 1;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::SetNextLevel()
//
// PURPOSE:		Go on to the next level in the list
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::SetNextLevel()
{
	// Find what our next level will be
	m_nNextLevel = m_nNextLevel + 1;

	if(m_nNextLevel >= m_GameInfo.m_nNumLevels)
		m_nNextLevel = 0;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ServerAppAddClient()
//
// PURPOSE:		Called when a new client is added to the server.
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ServerAppAddClient( HCLIENT hClient )
{
	// Check if we don't have a serverapp.
	if( !m_bGameServHosted )
		return;

	char szBuf[1024];
	ostrstream memStream( szBuf, sizeof( szBuf ));

	// Write out the player information.
	memStream << (( BYTE )SERVERAPP_ADDCLIENT ) << endl;
	memStream << (( int )g_pLTServer->GetClientID( hClient )) << endl;

	// Send to the server app.
	g_pLTServer->SendToServerApp( szBuf, sizeof( szBuf ));

	// Flag an update to happen.
	SetUpdateGameServ( );
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ServerAppRemoveClient()
//
// PURPOSE:		Called when a client is removed from the server.
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ServerAppRemoveClient( HCLIENT hClient )
{
	// Check if we don't have a serverapp.
	if( !m_bGameServHosted )
		return;

	char szBuf[32];
	ostrstream memStream( szBuf, sizeof( szBuf ));

	// Write out the player information.
	memStream << (( BYTE )SERVERAPP_REMOVECLIENT ) << endl;
	memStream << (( int )g_pLTServer->GetClientID( hClient )) << endl;

	// Send to the server app.
	g_pLTServer->SendToServerApp( szBuf, sizeof( szBuf ));

	// Flag an update to happen.
	SetUpdateGameServ( );
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ServerAppShellUpdate()
//
// PURPOSE:		Called every frame so that the serverapp remains updated.
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ServerAppShellUpdate( )
{
	// Check if we don't have a serverapp.
	if( !m_bGameServHosted )
		return;

	// Check if serverapp doesn't need updating.
	if( !m_bUpdateGameServ )
		return;

	// Clear the update flag.
	m_bUpdateGameServ = LTFALSE;

	char szBuf[1024];
	ostrstream memStream( szBuf, sizeof( szBuf ));

	// Write the message id.
	memStream << (( BYTE )SERVERAPP_SHELLUPDATE ) << endl;

	// Add info for each player.
	for(uint8 i = 0; i < MAX_CLIENTS; i++)
	{
		// Make sure the client is valid.
		if( m_ClientData[i].m_hClient == LTNULL )
		{
			continue;
		}

		// Write the client id.
		memStream << (( int )g_pLTServer->GetClientID( m_ClientData[i].m_hClient )) << endl;

		// Write the client name.
		memStream << m_ClientData[i].m_Data.m_szName << endl;

		// Write the frag count.
		memStream << ( int )m_ClientData[i].m_nFrags << endl;

		// Write the score.
		memStream << ( int )m_ClientData[i].m_nScore << endl;

		// Write the lives.
		memStream << ( int )m_ClientData[i].m_nLives << endl;

		// Write the race.
		memStream << ( char )m_ClientData[i].m_Data.m_nCharacterClass << endl;

		// Write the controller state.
		memStream << ( char )(i == m_nServerController) << endl;
	}

	// Signal end of player list.
	memStream << (( int )-1 ) << endl;

	// Send to the server app.
	g_pLTServer->SendToServerApp( szBuf, sizeof( szBuf ));
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ServerAppPreStartWorld()
//
// PURPOSE:		Called before a world is loaded.
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ServerAppPreStartWorld( )
{
	// Check if we don't have a serverapp.
	if( !m_bGameServHosted )
		return;

	char szBuf[32];
	ostrstream memStream( szBuf, sizeof( szBuf ));

	memStream << (( BYTE )SERVERAPP_PRELOADWORLD ) << endl;

	// Send to the server app.
	g_pLTServer->SendToServerApp( szBuf, sizeof( szBuf ));
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ServerAppPreStartWorld()
//
// PURPOSE:		Called after world is loaded.
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ServerAppPostStartWorld( )
{
	// Check if we don't have a serverapp.
	if( !m_bGameServHosted )
		return;

	char szBuf[32];
	ostrstream memStream( szBuf, sizeof( szBuf ));

	memStream << (( BYTE )SERVERAPP_POSTLOADWORLD ) << endl;
	memStream << (( int )m_nCurrentLevel ) << endl;

	// Send to the server app.
	g_pLTServer->SendToServerApp( szBuf, sizeof( szBuf ));
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ServerAppPreStartWorld()
//
// PURPOSE:		Add a player's message to the server app console
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ServerAppMessage( const char *szChar, int nClientIndex )
{
	// Check if we don't have a serverapp.
	if( !m_bGameServHosted )
		return;

	char szMsg[512];
	sprintf(szMsg, "<< %s >>  %s", m_ClientData[nClientIndex].m_Data.m_szName, szChar);

	char szBuf[512];
	ostrstream memStream( szBuf, sizeof( szBuf ));

	memStream << (( BYTE )SERVERAPP_CONSOLEMESSAGE ) << szMsg << ends;

	// Send to the server app.
	g_pLTServer->SendToServerApp( szBuf, sizeof( szBuf ));
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::SetServerController()
//
// PURPOSE:		Set the new server controler info.
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::SetServerController(uint8 nId)
{
	// sanity check
	if(nId >= MAX_CLIENTS)
		return;

	// first see if we need to free any old info
	if(m_nServerController < MAX_CLIENTS)
	{
		// un-mark us...
		void *pData;
		uint32 nLength;
		g_pLTServer->GetClientData(m_ClientData[m_nServerController].m_hClient, pData, nLength);

		if(nLength == sizeof(MPClientData))
		{
			MPClientData *pMPData = (MPClientData*)pData;

			pMPData->m_bServerControl = LTFALSE;
		}
	}

	// now record 
	m_nServerController = nId;

	// mark the persistent info
	{
		void *pData;
		uint32 nLength;
		g_pLTServer->GetClientData(m_ClientData[m_nServerController].m_hClient, pData, nLength);

		if(nLength == sizeof(MPClientData))
		{
			MPClientData *pMPData = (MPClientData*)pData;

			pMPData->m_bServerControl = LTTRUE;
		}
	}

	// inform all the other clients
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
	hMessage->WriteByte(MP_SERVER_CONTROLLER);
	hMessage->WriteDWord(m_ClientData[m_nServerController].m_nClientID);
	hMessage->WriteByte(LTTRUE);
	g_pLTServer->EndMessage(hMessage);

	SetUpdateGameServ( );
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::IncrementConsecutiveFrags()
//
// PURPOSE:		Increment the consecutive frag counter.
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::IncrementConsecutiveFrags(int nIndex)
{
	// sanity check
	if(nIndex < 0 || nIndex >= MAX_CLIENTS)
		return;

	// increment the counter
	m_ClientData[nIndex].m_nConsecutiveFrags++;

	// see if the client is an alien and qualifies for royalty...
	if((m_ClientData[nIndex].m_Data.m_nCharacterClass == Alien) && (m_GameInfo.m_nQueenMolt > 0))
	{
		// see if we need to invalidate the frag
		int	nBursterIndex	= g_pCharacterButeMgr->GetSetFromModelType("Chestburster_MP");
		int nHuggerIndex	= g_pCharacterButeMgr->GetSetFromModelType("Facehugger_MP");
		int nCurIndex		= 0;

		CPlayerObj *pPlayer = FindPlayerObj(m_ClientData[nIndex].m_hClient);
		if(pPlayer)
			nCurIndex = pPlayer->GetCharacter();

		// see if we need to do some special case lifecycle stuff...
		if(m_GameInfo.m_bLifecycle && (nCurIndex == nHuggerIndex || nCurIndex == nBursterIndex))
		{
			// reset the frag count and wait 'till the little guy grows up a bit
			m_ClientData[nIndex].m_nConsecutiveFrags = 0;
		}

		// almost there, see if we have enough frags
		if(m_ClientData[nIndex].m_nConsecutiveFrags == m_GameInfo.m_nQueenMolt)
		{
			// Tell all the clients that someone is mutating
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MPMGR_SERVER_MESSAGE);
			hMessage->WriteByte(MP_QUEEN_MUTATE);
			hMessage->WriteDWord(m_ClientData[nIndex].m_nClientID);
			g_pLTServer->EndMessage(hMessage);

			// Mark this player for molting...  can't change race here...
			m_ClientData[nIndex].m_bSpawnAsQueen	= LTTRUE;
			m_ClientData[nIndex].m_fMiscTimer		= g_pLTServer->GetTime();
		}
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::KickSpeedCheater()
//
// PURPOSE:		Kick the speed cheater...
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::KickSpeedCheater(HCLIENT hClient)
{
	// If our game type is not of a multiplayer variety... then skip this
	if(!GetGameType().IsMultiplayer())
		return;


	// Find the client who cheated...
	int nIndex = FindClient(hClient);

	if(nIndex != -1)
	{
		HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(hClient, MID_SPEED_CHEAT);
		g_pServerDE->EndMessage(hMessage);

		g_pLTServer->KickClient(m_ClientData[nIndex].m_hClient);
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::SetGameInfoChanged()
//
// PURPOSE:		Set the flag and copy the old info...
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::SetGameInfoChanged()
{
	// see if we are alread set up for a change...
	if(m_bGameInfoChanged)
		return;

	// flag the change
	m_bGameInfoChanged = LTTRUE;

	// copy the old data so we have a fresh start point
	m_PendingGameInfo = m_GameInfo;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerMgr::ResetQueen()
//
// PURPOSE:		Reset this character
//
// -------------------------------------------------------------------- //

void MultiplayerMgr::ResetQueen(HCLIENT hClient)
{
	// If our game type is not of a multiplayer variety... then skip this
	if(!GetGameType().IsMultiplayer())
		return;


	// Find the client who cheated...
	int nIndex = FindClient(hClient);

	if(nIndex != -1)
	{
		CPlayerObj *pPlayer = FindPlayerObj(m_ClientData[nIndex].m_hClient);

		if(pPlayer)
		{
			// Now reset this character
			pPlayer->SetCharacter(m_ClientData[nIndex].m_Data.m_nCharacterSet[Alien], LTTRUE);
		}
	}
}


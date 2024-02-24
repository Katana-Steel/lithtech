// -------------------------------------------------------------------- //
//
// MODULE  : MultiplayerMgr.h
//
// PURPOSE : A server-side multiplayer statistics manager
//
// CREATED : 3/27/01
//
// -------------------------------------------------------------------- //

#ifndef __MULTIPLAYER_MGR_H__
#define __MULTIPLAYER_MGR_H__

// -------------------------------------------------------------------- //
#include "MultiplayerMgrDefs.h"
#include "GameSpyMgr.h"

// -------------------------------------------------------------------- //

class CPlayerObj;

// -------------------------------------------------------------------- //

struct MPMgrData
{
	MPMgrData()
	{
		m_hClient = LTNULL;
		m_nClientID = 0;

		m_nUpdateFlags = 0;

		m_nPing = 0;
		m_nFrags = 0;
		m_nConsecutiveFrags = 0;
		m_nScore = 0;

		m_fJoinedTime = 0.0f;
		m_fPickupMsgTime = 0.0f;
		m_bVerified = LTFALSE;

		m_fTauntTimer = 0.0f;

		m_bFirstRespawn = LTTRUE;

		m_fMiscTimer = 0.0f;
		m_nNextClass = -1;

		m_nLives = 1;
		m_bDisplayEvacZone = LTFALSE;

		m_bObserving = LTFALSE;
		m_nObserveMode = 0;

		m_bSpawnAsQueen = LTFALSE;
	}

	MPClientData	m_Data;

	HCLIENT			m_hClient;
	uint32			m_nClientID;

	uint8			m_nUpdateFlags;				// The current info that needs to be updated

	uint16			m_nPing;					// The current ping of this client
	int16			m_nFrags;					// The current number of frags this client has
	int8			m_nConsecutiveFrags;		// The number of consecutive frags this client has
	int16			m_nScore;					// The current score for this client

	LTFLOAT			m_fJoinedTime;				// The time this client joined the game
	LTFLOAT			m_fPickupMsgTime;			// The next time that a pickup message can be sent
	LTBOOL			m_bVerified;				// Has this client given verification?

	LTFLOAT			m_fTauntTimer;				// The last time a player sent a taunt...

	LTBOOL			m_bFirstRespawn;			// Is it the first time this player has spawned in?

	LTBOOL			m_bSpawnAsQueen;			// Should we respawn as a queen?

	// Game type specific variables

	LTFLOAT			m_fMiscTimer;				// A general use timer variable
	uint8			m_nNextClass;				// This is the class that this client will switch to

	int8			m_nLives;					// The number of lives this client has remaining for a round
	LTBOOL			m_bDisplayEvacZone;			// The current evacuation state of this client

	// Observe mode variables

	LTBOOL			m_bObserving;				// Is this client in observe mode?
	uint8			m_nObserveMode;				// The type of observe mode this client is in.
};

// -------------------------------------------------------------------- //

struct MPMgrTeamData
{
	MPMgrTeamData()
	{
		m_nUpdateFlags = 0;

		m_nFrags = 0;
		m_nScore = 0;
	}

	uint8			m_nUpdateFlags;				// The current info that needs to be updated

	int16			m_nFrags;					// The current number of frags this client has
	int16			m_nScore;					// The current score for this client
};

// -------------------------------------------------------------------- //

class MPMgrSender : public CGameSpySender
{
	public:

		void		SendTo(const void *pData, unsigned long len, const char *sAddr, unsigned long port);
};

// -------------------------------------------------------------------- //

class MultiplayerMgr : public CGameSpyMgr
{
	public:

		// -------------------------------------------------------------------- //
		// Construction and destruction

		MultiplayerMgr();
		~MultiplayerMgr();


		// -------------------------------------------------------------------- //
		// Setup and ending of multiplayer GameSpy host

		LTBOOL		Setup();
		LTBOOL		CleanUp();

		void		ProcessPacket(char* sData, uint32 dataLen, uint8 senderAddr[4], uint16 senderPort);

		// -------------------------------------------------------------------- //
		// Client control functions

		LTBOOL		AddClient(HCLIENT hClient, MPClientData *pData);
		LTBOOL		RemoveClient(HCLIENT hClient);

		LTBOOL		CreateUniqueClientName(HCLIENT hClient);

		int			FindClient(HCLIENT hClient);
		int			FindClient(uint32 nClient);
		int			FindClient(char *szName);

		int			FindRandomClient();

		int			FindLowestFragsClient(uint8 nTeam = -1);
		int			FindHighestFragsClient(uint8 nTeam = -1);

		int			FindLowestScoreClient(uint8 nTeam = -1);
		int			FindHighestScoreClient(uint8 nTeam = -1);

		CPlayerObj*	FindPlayerObj(HCLIENT hClient);
		MPMgrData*	GetClientData(HCLIENT hClient);

		uint8		GetClientScore(HCLIENT hClient);
		uint8		GetClientScore(uint32 nClient);
		LTBOOL		ClientsOnSameTeam(HCLIENT hClient1, HCLIENT hClient2);

		LTBOOL		ResetClient(HCLIENT hClient);
		LTBOOL		ResetAllClients();

		void		ClientObserveCommand(HCLIENT hClient, uint8 nCommand);

		void		UpdateClients();
		void		UpdateClientPings();
		void		UpdateTeams();

		int			NumberOfClass(uint8 nClass, LTBOOL bIncludeNextClass, LTBOOL bIgnoreObserving = LTFALSE);

		LTBOOL		ValidateClientData(HCLIENT hClient, MPClientData *pData);
		LTBOOL		ValidateClassAndCharacter(HCLIENT hClient, MPClientData *pData);
		LTBOOL		ValidateRespawn(CPlayerObj *pPlayer, int* pCharId=LTNULL);
		void		VerifyClientData(HCLIENT hClient);


		// -------------------------------------------------------------------- //
		// Server message handling functions

		void		ClientKilledMsg(HCLIENT hVictim, HCLIENT hKiller, uint8 nFlags = 0);
		void		ClientPickupMsg(HCLIENT hClient, uint8 nMsg);

		void		ChangeClientCharacter(HCLIENT hClient, uint8 nCharacterSet, LTBOOL bDisplay = LTTRUE);
		void		ChangeClientClass(HCLIENT hClient, uint8 nCharacterClass, LTBOOL bDisplay = LTTRUE);

		void		ClientCheating(HCLIENT hClient);
		void		KickSpeedCheater(HCLIENT hClient);


		// -------------------------------------------------------------------- //
		// Helper functions

		LTRESULT	ServerAppMessageFn(char *pMsg, int nLen);
		LTBOOL		OnMessage(HCLIENT hSender, uint8 nMessage, HMESSAGEREAD hRead);


		LTBOOL		ProcessServerCommand(char *szMsg, LTBOOL bFromMsg=LTFALSE, int senderID=-1);

		void		ResetQueen(HCLIENT hClient);


		// -------------------------------------------------------------------- //
		// Information retrieval functions

		MPGameInfo*	GetGameInfo()					{ return &m_GameInfo; }
		GameType	GetGameType()					{ return m_GameInfo.m_nGameType; }

		uint8		GetNumClients()					{ return m_nClients; }
		uint8		GetServerState()				{ return m_nServerState; }
		uint8		GetServerSubState()				{ return m_nServerSubState; }

		LTBOOL		AllowFriendlyFire();


		// -------------------------------------------------------------------- //
		// GameSpyMgr function overrides

		void		Update();

		BOOL		OnInfoQuery();
		BOOL		OnRulesQuery();
		BOOL		OnPlayersQuery();


		// -------------------------------------------------------------------- //
		// Single Player helper functions

		void		StartSinglePlayer();


		// -------------------------------------------------------------------- //
		// Serverapp interface.

		void		ServerAppAddClient( HCLIENT hClient );
		void		ServerAppRemoveClient( HCLIENT hClient );
		void		ServerAppShellUpdate( );
		void		ServerAppPreStartWorld( );
		void		ServerAppPostStartWorld( );
		void		ServerAppMessage( const char *szChar, int nClientIndex );

		void		SetUpdateGameServ( )			{ m_bUpdateGameServ = LTTRUE; }

	private:

		// -------------------------------------------------------------------- //
		// Private helper functions

		void		UpdateServerController();
		void		SendGameInfo(HCLIENT hClient);

		LTBOOL		UpdateGameInfo();
		LTBOOL		SetupGameInfo(MPGameInfo *pInfo);

		LTBOOL		CheckEndConditions();

		LTBOOL		CheckEndConditions_DM();
		LTBOOL		CheckEndConditions_TeamDM();
		LTBOOL		CheckEndConditions_Hunt();
		LTBOOL		CheckEndConditions_Survivor();
		LTBOOL		CheckEndConditions_Overrun();
		LTBOOL		CheckEndConditions_Evac();

		LTBOOL		CheckVerification();
		void		ClearVerification();

		LTRESULT	LoadLevel(char *szLevel);
		void		SetPrevLevel();
		void		SetNextLevel();

		void		SetServerController(uint8 nId);

		void		IncrementConsecutiveFrags(int nIndex);

		void		SetGameInfoChanged();


		// -------------------------------------------------------------------- //
		// Game type specific versions of certain functions

		void		ClientKilledMsg_DM(int nVIndex, int nKIndex, uint8 nFlags = 0);
		void		ClientKilledMsg_TeamDM(int nVIndex, int nKIndex, uint8 nFlags = 0);
		void		ClientKilledMsg_Hunt(int nVIndex, int nKIndex, uint8 nFlags = 0);
		void		ClientKilledMsg_Survivor(int nVIndex, int nKIndex, uint8 nFlags = 0);
		void		ClientKilledMsg_Overrun(int nVIndex, int nKIndex, uint8 nFlags = 0);
		void		ClientKilledMsg_Evac(int nVIndex, int nKIndex, uint8 nFlags = 0);

		void		Update_Hunt();
		void		Update_Survivor();
		void		Update_Overrun();
		void		Update_Evac();

	private:

		// The data that we don't want people screwing around with directly

		MPGameInfo		m_PendingGameInfo;					// The information about our pending game
		LTBOOL			m_bGameInfoChanged;					// Is there a change in the game info?

		MPGameInfo		m_GameInfo;							// The information about our current game
		uint8			m_nCurrentLevel;					// The current level being played from GameInfo
		uint8			m_nNextLevel;						// The next level to be played

		uint8			m_nClients;							// The current number of clients
		MPMgrData		m_ClientData[MAX_CLIENTS];			// The data for each client
		MPMgrTeamData	m_TeamData[MAX_TEAMS];				// The data for each team

		MPMgrSender		m_Sender;							// Our sender for GameSpy support
		LTBOOL			m_bUsingGameSpy;					// Are we using GameSpy support?

		LTFLOAT			m_fStartTime;						// The time the game was actually started

		uint16			m_nTimeRemaining;					// This is the number of seconds remaining for this level
		uint8			m_nServerState;						// What is the server currently doing?
		uint8			m_nServerSubState;					// What is the sub state of the server?
		uint8			m_nWaitSeconds;						// Wait time for certain transitions

		uint8			m_nServerController;				// The client who controls the server

		LTFLOAT			m_fVerifyTime;						// The time we started the verify state
		LTFLOAT			m_fMiscTime;						// A general use timer variable

		uint8			m_nCurrentRound;					// The current round of the game (ding ding ding!)

	    LTBOOL			m_bGameServHosted;					// Hosted by serverapp.
		LTBOOL			m_bUpdateGameServ;					// Signals that game server needs updating.

		// Game type specific variables

		uint8			m_nEvacState;						// Evacuation state

};

// -------------------------------------------------------------------- //

#endif


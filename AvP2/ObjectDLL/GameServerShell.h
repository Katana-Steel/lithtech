// ----------------------------------------------------------------------- //
//
// MODULE  : GameServerShell.cpp
//
// PURPOSE : Game's Server Shell - Definition
//
// UPDATED : 3/29/01
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_SERVER_SHELL_H__
#define __GAME_SERVER_SHELL_H__


#include "cpp_servershell_de.h"
#include "ClientServerShared.h"
#include "CharacterMgr.h"
#include "GlobalMgr.h"
#include "CommandMgr.h"
#include "GameType.h"
#include "CVarTrack.h"

#include <list>

//********************************************************************************

class CPlayerObj;
class MPGameInfo;
class MultiplayerMgr;
class GameStartPoint;

//********************************************************************************

class CGameServerShell : public IServerShell
{
	public :

		typedef std::list<int> CharactersToCacheList;

	public :

		// Construction and destruction
		CGameServerShell(ILTServer *pLTServer);
		~CGameServerShell();


		// Saving and loading of games
		void		Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		void		Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);


		// A message from the server app.
		LTRESULT	ServerAppMessageFn(char *pMsg, int nLen);


		// Game object specific functions
		LTBOOL		GetAlarm()					{ return m_bAlarm; }
		void		SetAlarm(LTBOOL bAlarm)		{ m_bAlarm = bAlarm; }


		// Game type and game stat information retrieval
		MPGameInfo*		GetGameInfo() const;
		GameType		GetGameType() const;
		MultiplayerMgr*	GetMultiplayerMgr()		{ return m_pMPMgr; }

		uint32			GetLastSaveLoadVersion() const { return m_nLastSaveLoadVersion; }
		uint32			GetCurrentSaveLoadVersion() const { return m_nCurrentSaveLoadVersion; }

		// Helper functions
		GameStartPoint* FindStartPoint(HSTRING hStartPointName, int nClass = -1, CPlayerObj* pSpawnPlayer = LTNULL, LTBOOL bQueen=LTFALSE);

		void		PauseGame(LTBOOL b = LTTRUE);
		void		ExitLevel();

		void		IncEvacZoneObjects()			{ m_nEvacZoneObjects++; }
		void		DecEvacZoneObjects()			{ m_nEvacZoneObjects--; }
		void		SetEvacZonePos(LTVector vPos)	{ m_vEvacZonePos = vPos; }

		int			GetEvacZoneObjects()		{ return m_nEvacZoneObjects; }
		LTVector	GetEvacZonePos()			{ return m_vEvacZonePos; }

		void*		GetAuthContext();

		void		SetMuteState(LTBOOL b)		{ m_bPlayerMuted = b; }
		LTBOOL		GetMuteState()				{ return m_bPlayerMuted; }

		LTBOOL		IsLoadingKeepAlive() const { return m_bLoadingKeepAlive; }

		void		AddCharacterToCache(int id);

		void		SetCacheFileName(char* pCacheFileName);

		LTBOOL		LowViolence();

		void		ProcessMPStartPointMessages();

		void		SpawnMPExosuits();

		LTBOOL		HasQueenStartPoints() { return m_bHasQueenStartPoint; }

	private :

		// --------------------------------------------------------------------------------- //
		// ***************** LITHTECH!! *******************
		//
		// These are functions called from Lithtech which we handle in the game client shell

		void		Update(LTFLOAT timeElapsed);

		void		OnAddClient(HCLIENT hClient);
		void		OnRemoveClient(HCLIENT hClient);

		void		VerifyClient(HCLIENT hClient, void *pClientData, uint32 &nVerifyCode);
		LPBASECLASS	OnClientEnterWorld(HCLIENT hClient, void *pClientData, uint32 clientDataLen);
		void		OnClientExitWorld(HCLIENT hClient);

		void		OnMessage(HCLIENT hSender, uint8 messageID, HMESSAGEREAD hMessage);
		void		OnCommandOn(HCLIENT hClient, int command);

		void		PreStartWorld(LTBOOL bSwitchingWorlds);
		void		PostStartWorld();

		LTRESULT	ProcessPacket(char* sData, uint32 dataLen, uint8 senderAddr[4], uint16 senderPort);

		void		CacheFiles();

		// --------------------------------------------------------------------------------- //
		// Game message handling functions

		void HandleCheatCode(CPlayerObj* pPlayer, int nCheatCode, uint8 nData, HMESSAGEREAD hMessage);
		void HandleCheatSpectatorMsg(HCLIENT hSender, HMESSAGEREAD hMessage);
		void HandlePlayerMsg(HCLIENT hSender, uint8 messageID, HMESSAGEREAD hMessage);
		void HandleCheatRemoveAI(uint8 nData);
		void HandleTriggerBoxCheat(uint8 nData);

		void HandlePlayerSummary(HCLIENT hSender, HMESSAGEREAD hMessage);
		void HandlePlayerExitLevel(HCLIENT hSender, HMESSAGEREAD hMessage);
		void HandleLoadGameMsg(HCLIENT hSender, HMESSAGEREAD hMessage);
		void HandleSaveGameMsg(HCLIENT hSender, HMESSAGEREAD hMessage);
		void HandleConsoleTriggerMsg(HCLIENT hSender, HMESSAGEREAD hMessage);
		void HandleConsoleCmdMsg(HCLIENT hSender, HMESSAGEREAD hMessage);
		void HandleSFXMsg(HCLIENT hSender, HMESSAGEREAD hMessage);

		//-----------------------------------------------------------------------------------

#ifndef _FINAL
		void		UpdateBoundingBoxes();
		void		RemoveBoundingBoxes();
#endif
		void		ReportError(HCLIENT hClient, uint8 nErrorType);

		CPlayerObj*	CreatePlayer(HCLIENT hClient, void *pClientData, uint32 clientDataLen);

		void		CacheLevelSpecificFiles();

		//-----------------------------------------------------------------------------------

		// Multiplayer and client information
		LTBOOL			ValidateQueenStartPoints();

		MultiplayerMgr	*m_pMPMgr;

		LTBOOL			m_bFirstUpdate;
		LTBOOL			m_bAlarm;			// level-wide alarm state
		uint8			m_nPrevMissionId;

		uint8			m_nLastLGFlags;
		uint32			m_nLastSaveLoadVersion;  // The save/load version of the file being loaded (or last loaded). 
		static uint32	m_nCurrentSaveLoadVersion;  // The save/load version that will be stored with any current save file.

		LTBOOL			m_bGlobalMgrValid;
		CGlobalMgr		m_GlobalMgr;

		CCommandMgr		m_CmdMgr;			// Same as g_pCmdMgr
		CCharacterMgr	m_charMgr;

		int				m_nEvacZoneObjects;
		LTVector		m_vEvacZonePos;
		LTBOOL			m_bDoAutoSave;
		LTBOOL			m_bPlayerMuted;
		LTBOOL			m_bHasQueenStartPoint;

		LTBOOL			m_bLoadingKeepAlive; // Only true while loading the keep-alive file.

		CharactersToCacheList	m_aCharactersToCache;
};

//-----------------------------------------------------------------------------------

extern CGameServerShell* g_pGameServerShell;
extern GameStartPoint* g_pLastPlayerStartPt;

//-----------------------------------------------------------------------------------

#endif  // __GAME_SERVER_SHELL_H__


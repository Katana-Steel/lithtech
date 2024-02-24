// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenJoin.h
//
// PURPOSE : Interface screen to search for and join LAN games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef _SCREEN_JOIN_H_
#define _SCREEN_JOIN_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"
#include "IGameSpy.h"
#include "winsync.h"

struct PlayerEntry
{
	std::string m_sName;
	uint16 m_nPing;
};
typedef std::vector< PlayerEntry > PlayerEntryList;

struct ServerEntry
{
	ServerEntry( )
	{
		m_nNumPlayers = 0;
		m_nMaxPlayers = 0;
		m_nPing = 0;
		m_bUsePassword = false;
		m_nGameType = 0;
		m_nFragLimit = 0;
		m_nTimeLimit = 0;
		m_bDemoServer = false;
		m_bDirectConnect = true;
		m_bConnectViaPublic = true;
		m_pColumnCtrl = NULL;
	}

	std::string m_sPublicAddress;
	std::string m_sName;
	std::string m_sMission;
	std::string m_sVersion;
	std::string m_sModName;
	uint32 m_nNumPlayers;
	uint32 m_nMaxPlayers;
	uint16 m_nPing;
	bool m_bUsePassword;
	uint8 m_nGameType;
	std::string m_sOptions;
	uint16 m_nFragLimit;
	uint16 m_nTimeLimit;
	PlayerEntryList m_lstPlayerEntry;
	bool m_bDemoServer;
	bool m_bDirectConnect;
	bool m_bConnectViaPublic;
	CLTGUIColumnCtrl* m_pColumnCtrl;
};
typedef stdext::hash_map<std::string, ServerEntry, hash_map_stdstring_nocase> TServerEntryMap;


class CScreenJoin : public CBaseScreen
{
public:
	CScreenJoin();
	virtual ~CScreenJoin();

	virtual void	Term();

	// Build the screen
    LTBOOL   Build();

	LTBOOL	Render(HSURFACE hDestSurf);

    void    OnFocus(LTBOOL bFocus);
	LTBOOL	HandleKeyDown(int key, int rep);

	void	CheckPassword();

	virtual void Escape();

	// Deletes serverlist data, which can be very large.
	void	TermServerLists( );

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	uint32	HandleCallback(uint32 dwParam1, uint32 dwParam2);

	void FindServers();

	LTBOOL	IsValidPort(uint16 nPort) { return (nPort > 0); }

	CLTGUITextCtrl*	m_pFindCtrl;
//	CLTGUITextCtrl*	m_pStatusCtrl;
	CLTGUIListCtrl*	m_pServerListCtrl;
	CLTGUIListCtrl*	m_pPlayerListCtrl;
	CLTGUIWindow*	m_pWait;
	CLTGUIWindow*	m_pDetails;
	CLTGUITextCtrl*	m_pJoin;
	CLTGUITextCtrl*	m_pOptions;
	CLTGUITextCtrl* m_pMission;
	CLTGUIColumnCtrl*	m_pPort;

	// Type manages list of servers reported.
	TServerEntryMap m_cServerMap;
	std::string m_sSelectedServerAddress;

	// Dump the server list into the display
	void DisplayCurServerList( uint32 nSortField );

	// Read the peer details
	void DisplayDetails();
	void SetDetailErrorMessage(const char *szError);
	void ReadDMDetails(std::string& sOptions,CLTMsgRef_Read& cRead);
	void ReadCoopDetails(std::string& sOptions,CLTMsgRef_Read& cRead);

	bool IsCurrentGame(const char *pAddr);
	void JoinCurGame();
	
	enum EState {
		eState_Inactive,
		eState_Startup,
		eState_UpdateDir,
		eState_QueryDetails,
		eState_DisplaySysMOTD,
		eState_DisplayGameMOTD,
		eState_Waiting,
		eState_Transition,
		eState_Error
	};
	EState m_eCurState;

	// Called when entering a state
	bool PreState(EState eNewState);
	// Called when leaving a state
	// Returns false if that is an invalid state transition
	bool PostState(EState eNewState);
	// Change states
	bool ChangeState(EState eNewState);
	void Update();

	// Pre-state handling for each state
	bool PreState_Inactive();
	bool PreState_Startup();
	bool PreState_UpdateDir();
	bool PreState_QueryDetails();
	bool PreState_Waiting();

	// Post-state handling for each state
	bool PostState_UpdateDir(EState eNewState);
	bool PostState_QueryDetails(EState eNewState);
	bool PostState_Waiting(EState eNewState);

	// Update handling for each state
	void Update_State_Startup();
	void Update_State_UpdateDir();
	void Update_State_QueryDetails();

	void FilterServers();

	// Callback for IGameSpyBrowser::RequestServerList
	static void ServerInfoCallback( IGameSpyBrowser& gameSpyBrowser, IGameSpyBrowser::HGAMESERVER hServer, void* pUserData );

	uint8	m_nVersionFilter;
	uint8	m_nPlayersFilter;
	uint8	m_nPingFilter;
	uint8	m_nModFilter;
	uint8	m_nTypeFilter;

	uint32	m_nLastSort;

	std::string	m_sPassword;

	char m_szPort[8];

	// UpdateDir states.
	enum EUpdateDirState
	{
		eUpdateDirState_Idle,
		eUpdateDirState_RetailPublic,
		eUpdateDirState_RetailLan,
		eUpdateDirState_DemoPublic,
		eUpdateDirState_DemoLan,
		eUpdateDirState_Finished,

		eUpdateDirState_NumStates,
	};
	EUpdateDirState m_eUpdateDirState;

	CWinSync_CS m_csAddServer;
};

#endif // _SCREEN_MULTI_H_
// ----------------------------------------------------------------------- //
//
// MODULE  : ClientMultiplayerMgr.h
//
// PURPOSE : Clientside multiplayer mgr - Declaration
//
// CREATED : 02/05/02
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENTMULTIPLAYERMGR_H_
#define __CLIENTMULTIPLAYERMGR_H_

#include "VarTrack.h"
#include "NetDefs.h"
#include "ProfileUtils.h"

class IGameSpyBrowser;


enum eDisconnectCodes
{
	eDisconnect_None = 0,
	eDisconnect_NotSameGUID,
	eDisconnect_WrongPassword,
	eDisconnect_InvalidAssets,
	eDisconnect_BadCdKey,
	eDisconnect_Banned,
};


// Controls state machine of connecting with a server
enum EConnectionState
{
	eConnectionState_Disconnected,
	eConnectionState_NatNeg,
	eConnectionState_SettleComm,
	eConnectionState_Connecting,
	eConnectionState_Connected,
	eConnectionState_Failure,
};

class ClientMultiplayerMgr
{
public:
	
	ClientMultiplayerMgr( );
	~ClientMultiplayerMgr( );

	// Update called once per frame.
	void		Update( );

	bool		InitMultiPlayer();
	bool		UpdateMultiPlayer();
	bool		UpdateNetClientData( );
	bool		InitSinglePlayer();

	// Disconnect on the next update
	void		ForceDisconnect() {	m_bForceDisconnect = true;}	

	// Pass the pNatNegBrowser if server requires NAT Negotiations.
	bool		SetupClient(char const* pszHostName, char const* pszPassword,
							bool bDoNatNegotiations, bool bConnectViaPublic, 
							char const* pszPublicAddress, char const* pszPrivateAddress );

	// Setup the server for singleplayer.
	bool		SetupServerSinglePlayer( );

	// Setup the server to host a multiplayer game.
	bool		SetupServerHost( int nPort, bool bLanOnly );

	// Handles engine event.
	void		OnEvent(uint32 dwEventID, uint32 dwParam);

	// Handles message from engine.
	bool		OnMessage(uint8 messageID, ILTMessage_Read *pMsg);


	void		DoTaunt(uint32 nClientID,uint32 nTaunt);


	// Sets the networking service.
	bool		SetService( );

	// Accessors to setup server information.
	StartGameRequest& GetStartGameRequest( ) { return m_StartGameRequest; }
	ServerGameOptions&	GetServerGameOptions( ) { return m_ServerGameOptions; }
	NetClientData&	GetNetClientData( ) { return m_NetClientData; }

	// Start a client of a server based on previously set startgamerequest.
	bool		StartClientServer( );


	// This client is connected to remote server.
	bool		IsConnectedToRemoteServer( ) { return ( m_StartGameRequest.m_Type == STARTGAME_CLIENTTCP ); }

	//returns true if the passed in address matches the current server address
	bool		CheckServerAddress(char const*pszTestAddress, int nPort);
	// Called when the engine wants to tell the game a disconnection code (a.k.a. hack)
	virtual void SetDisconnectCode(uint32 nCode, const char *pMsg);
	// Internal game-side support for the disconnection code
	void		ClearDisconnectCode();
	uint32		GetDisconnectCode() { return m_nDisconnectCode; }
	const char *GetDisconnectMsg() { return m_sDisconnectMsg; }

	const char *GetServerAddress() const {return m_sServerAddress;}
	const char *GetServerName() const {return m_sServerName;}

	// Get/Set the source address of the message which is currently being processed
	void		SetCurMessageSource(const uint8 aAddr[4], uint16 nPort);
	void		GetCurMessageSource(uint8 aAddr[4], uint16 *pPort);

	LTRESULT	GetLastConnectionResult() {return m_nLastConnectionResult;}

	void		SelectTeam(uint8 nTeam, bool bPlayerSelected);
	bool		HasSelectedTeam() { return m_bHasSelectedTeam; }

	void		SetModName( const char *pszModName ) { m_sModName = pszModName; }
	const char*	GetModName( ) const { return m_sModName.c_str(); }

	// Get retail server browser.
	IGameSpyBrowser* GetRetailServerBrowser( ) { return m_pRetailGameSpyBrowser; }
	// Get demo server browser.
	IGameSpyBrowser* GetDemoServerBrowser( ) { return m_pDemoGameSpyBrowser; }
	// Creates the server browsers.
	bool		CreateServerBrowsers( );
	// Deletes the browsers.
	void		TermBrowsers( );

	// Update the state machine.
	void		UpdateConnectionState( );
	EConnectionState GetConnectionState( ) const { return m_eConnectionState; }

protected :

	bool		HandleMsgHandshake( ILTMessage_Read& msg );
	bool		HandleMsgMultiplayerData( ILTMessage_Read& pMsg );
	bool		HandleMsgPlayerMultiplayerInit( ILTMessage_Read& msg );
	bool		HandleMsgPlayerSingleplayerInit (ILTMessage_Read& msg);

	bool		StartServerAsSinglePlayer( );
	bool		StartServerAsHost( );
	bool		StartClient( );

	void		UpdateState_NatNeg( );
	void		UpdateState_SettleComm( );
	void		UpdateState_Connecting( );

private:

	// Must be filled out before StartLocalGame, StartNetworkGameAsXXX calls.
	StartGameRequest	m_StartGameRequest;
	ServerGameOptions	m_ServerGameOptions;
	NetClientData		m_NetClientData;

	// The serverbrowser object containing information on the server we
	// are going to connect to.
	bool		m_bDoNatNegotiation;

	//these refer to the server we are currently connected to
	CString		m_sServerAddress;
	int			m_nServerPort;
	CString		m_sServerName;

	//used for handshaking with server
	uint32		m_nServerKey;
	uint32		m_nClientPass;

	// Disconnection code/msg storage
	uint32		m_nDisconnectCode;
	CString		m_sDisconnectMsg;

	LTRESULT	m_nLastConnectionResult;

	// Connection handling
	bool			m_bForceDisconnect;	// Set this flag to disconnect on the next update

		// Server directory interface
//	IServerDirectory*	m_pServerDir;

	// Current message source
	uint8		m_aCurMessageSourceAddr[4];
	uint16		m_nCurMessageSourcePort;


	uint8	m_nTeam;
	bool	m_bHasSelectedTeam;
	
	// The name of our selected mod...
	std::string		m_sModName;

	// Gamespy browser for retail games.
	IGameSpyBrowser* m_pRetailGameSpyBrowser;

	// Gamespy browser for demo games.
	IGameSpyBrowser* m_pDemoGameSpyBrowser;

	// Should do natnegotions on next client connect.
	bool m_bDoNatNegotiations;

	EConnectionState m_eConnectionState;

	// Used to let comm settle after natneg.
	CTimer m_SettleCommTimer;

	// Try initial connect through public address.
	bool m_bConnectViaPublic;

	// public address to try to connect to.
	std::string m_sConnectPublicAddress;
	// private address to try to connect to.
	std::string m_sConnectPrivateAddress;
};

extern ClientMultiplayerMgr* g_pClientMultiplayerMgr;

#endif // __CLIENTMULTIPLAYERMGR_H_


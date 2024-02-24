//////////////////////////////////////////////////////////////////////////////
// Server directory interface header

#ifndef __IGAMESPY_H__
#define __IGAMESPY_H__

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GAMESPY_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GAMESPY_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef GAMESPY_EXPORTS
#define GAMESPY_API __declspec(dllexport)
#else
#define GAMESPY_API __declspec(dllimport)
#endif

#include <winsock.h>
#include <string>

// Supported Game SKU's for this IServerDir
enum EGameSKU
{
	eGameSKU_Invalid,
	eGameSKU_ContractJack_Retail,
	eGameSKU_ContractJack_MPDemo,
	eGameSKU_ContractJack_SPDemo,
	eGameSKU_ContractJack_PRDemo,
};

class GAMESPY_API IGameSpyServer
{
public:
	IGameSpyServer( ) { };
	virtual ~IGameSpyServer() {}

// Types
public:

	// Contains initialization information about the game.
	struct StartupInfo
	{
		StartupInfo( )
		{
			m_eGameSKU = eGameSKU_Invalid;
			m_nPort = 0;
			m_Socket = NULL;
			m_bPublic = true;
		}

		// Game SKU
		EGameSKU		m_eGameSKU;
		// IP Address to bind to.
		std::string		m_sIpAddress;
		// Port to bind to.
		uint16			m_nPort;
		// Socket to do comm on.  NULL if not to do shared socket.
		SOCKET			m_Socket;
		// Version of the game running.
		std::string		m_sVersion;
		// Region code.
		std::string		m_sRegion;
		// Public game.
		bool			m_bPublic;
	};

	// Status of published stated.
	enum EPublishStatus
	{
		ePublishStatus_Unpublished,
		ePublishStatus_Published,
		ePublishStatus_Error,
	};

	// Status of the cdkey challenge response.
	enum EChallengeResponse
	{
		eChallengeResponse_Invalid,
		eChallengeResponse_Processing,
		eChallengeResponse_Accepted,
		eChallengeResponse_Rejected,
	};

// Functions
public:

	// Create an IGameSpyServer interface.
	static IGameSpyServer* Create( StartupInfo const& StartupInfo );
	// Deletes a IGameSpyServer interface.
	static void Delete( IGameSpyServer* pGameSpyServer );

	//////////////////////////////////////////////////////////////////////////////
	// Commands

	// Publishes the server to the Master Server.
	virtual bool Publish( ) = 0;
	// Removes server from Master Server.
	virtual bool Unpublish( ) = 0;
	// Check if we're published.
	virtual EPublishStatus GetPublishStatus( ) const = 0;

	//////////////////////////////////////////////////////////////////////////////
	// Server information.

	// Registers key with Master Server.  pszKey must NOT include player
	// or team index. (e.g. "player_", "team_t").
	// Keys must be registered in the same order on client and 
	// server.
	virtual bool RegisterKey( char const* pszKey ) = 0;

	// Get server property value.  Key string must include player/team index if applicable
	// (e.g. "player_0", "team_t0" ).
	virtual bool GetServerProperty( char const* pszKey, char* pszValue, uint32 nValueSize ) const = 0;
	// Sets server property value.  Key string must include player/team index if applicable
	// (e.g. "player_0", "team_t0" ).
	virtual bool SetServerProperty( char const* pszKey, char const* pszValue ) = 0;

	// Sets the number of players currently connected.
	virtual void SetNumPlayers( uint32 nNumPlayers ) = 0;
	// Sets the number of teams currently in game.
	virtual void SetNumTeams( uint32 nNumTeams ) = 0;

	//////////////////////////////////////////////////////////////////////////////
	// CDKey Functions

	// Get the cdkeychallenge
	virtual bool GetCDKeyChallenge( char* szChallenge, uint32& nChallengeSize ) = 0;

	// Called when challenge response is received from client.
	virtual bool ProcessCDKeyChallengeResponse( uint32 nClientId, char const* pszIPAddr, 
		char const* pszChallenge, char const* szChallengeResponse ) = 0;

	// Poll to see if client's challenge response was accepted.
	virtual EChallengeResponse GetChallengeResponse( uint32 nClientId ) const = 0;

	// Tells the cdkey server to release any lock on the cdkey.  Called when client disconnects.
	virtual bool ReleaseCDKey( uint32 nClientId ) = 0;

	//////////////////////////////////////////////////////////////////////////////
	// Net message handling

	// Handle an incoming net message
	// Returns false if the message was not recognized
	// Note : Remove all external headers before calling this function
	virtual bool HandleNetMessage( uint8* pData, uint32 nDataLen, uint8 senderAddr[4], uint16 nPort) = 0;

private:

	// Prevent copying.
	IGameSpyServer( IGameSpyServer const& other );
	IGameSpyServer const& operator=( IGameSpyServer const& other );
};


class GAMESPY_API IGameSpyBrowser
{
public:

	IGameSpyBrowser( ) { };
	virtual ~IGameSpyBrowser() {}

// Types
public:

	// Type used to identify a game server in the ServerInfoCallback.  Used to 
	// call back into the gamespybrowser to retrieve information.
	typedef void* HGAMESERVER;

	// Callback to register to receive server information.
	// hServer is temporary and should not be held onto.
	// bDetailsRequest is true if this is in response to a request for server details.
	// pUserData is the user data value passed into StartupInfo.
	typedef void ( *ServerInfoCallback )( IGameSpyBrowser& gameSpyBrowser, HGAMESERVER hServer, void* pUserData );

	// Contains initialization information about the game.
	struct StartupInfo
	{
		StartupInfo( )
		{
			m_eGameSKU = eGameSKU_Invalid;
		}

		// Game SKU
		EGameSKU		m_eGameSKU;
	};

	// Status of browsing state.
	enum EBrowserStatus
	{
		eBrowserStatus_Idle,
		eBrowserStatus_Processing,
		eBrowserStatus_Complete,
		eBrowserStatus_Error,
	};

	enum ENatNegotiationResult
	{
		eNatNegotiationResult_Invalid,
		eNatNegotiationResult_Error,
		eNatNegotiationResult_TimedOut,
		eNatNegotiationResult_Success,
	};

// Functions
public:

	// Create an IGameSpyBrowser interface.
	static IGameSpyBrowser* Create( StartupInfo const& StartupInfo );
	// Deletes a IGameSpyBrowser interface.
	static void Delete( IGameSpyBrowser* pGameSpyBrowser );

	//////////////////////////////////////////////////////////////////////////////
	// Commands

	// Requests list of servers from Master Server. pszFilter uses SQL like 
	// notation (e.g. pszFilter = "fraglimit > 0 && numplayers > 0".
	// Complete when GetBrowsingStatus returns EBrowserStatus_Complete.
	// Callback is called for each server found.
	// pUserData is sent as parameter in the callback call.
	// nPort is the LAN ports to scan.
	virtual bool RequestServerList( bool bPublic, uint16 nPort, char const* pszFilter, ServerInfoCallback cbServerInfo, void* pUserData ) = 0;
	// Requests detail information on specific server.
	// Complete when GetBrowsingStatus returns EBrowserStatus_Complete.
	// Callback is called for each server found.
	// pUserData is sent as parameter in the callback call.
	virtual bool RequestServerDetails( char const* pszAddress, uint16 nPort, ServerInfoCallback cbServerInfo, void* pUserData ) = 0;
	// Requests NAT negotiations with server pszAddress:nPort on socket hSocket.
	virtual bool RequestNatNegotiation( SOCKET hSocket, char const* pszAddress, uint16 nPort ) = 0;
	// Stops all requests.
	virtual bool HaltRequest( ) = 0;
	// Gets the current status of the browser.
	virtual EBrowserStatus GetBrowserStatus( ) const = 0;
	// Gets the results of a NatNeg request.
	virtual ENatNegotiationResult GetNatNegotiationResult( sockaddr* pSockAddr ) const = 0;

	//////////////////////////////////////////////////////////////////////////////
	// Key management

	// Registers key with Master Server.  pszKey must NOT include player
	// or team index. (e.g. "player_", "team_t").
	// Keys must be registered in the same order on client and 
	// server.
	virtual bool RegisterKey( char const* pszKey ) = 0;

	// Adds a Summary key that should be reported by a server when
	// RequestServerList called.  Keep the number of summary
	// keys low for optimal performance.
	virtual bool AddSummaryKey( char const* pszKey ) = 0;

	//////////////////////////////////////////////////////////////////////////////
	// Server information.

	// Gets the number of servers.
	// nPort is the port to scan.
	virtual uint32 GetNumServers( bool bPublic, uint16 nPort ) = 0;

	// Gets the number of players currently on server.
	virtual uint32 GetNumPlayers( HGAMESERVER hGameServer ) const = 0;
	// Gets the number of teams currently on server.
	virtual uint32 GetNumTeams( HGAMESERVER hGameServer ) const = 0;

	// Get server property value.  Key string must include player/team index if applicable
	// (e.g. "player_0", "team_t0" ).
	virtual bool GetServerProperty( HGAMESERVER hGameServer, char const* pszKey, char* pszValue, uint32 nValueSize ) const = 0;

	// Get the ping to a server.
	virtual uint16 GetServerPing( HGAMESERVER hGameServer ) const = 0;

	// Check if server has detail info.
	virtual bool HasServerDetails( HGAMESERVER hGameServer ) const = 0;

	// Gets the address/port to use when connecting.
	virtual bool GetServerConnectInfo( HGAMESERVER hGameServer, 
												char szPrivateAddress[16], uint16& nPrivatePort,
												char szPublicAddress[16], uint16& nPublicPort,
												bool& bConnectViaPublic, bool& bDirectConnect ) const = 0;

	//////////////////////////////////////////////////////////////////////////////
	// CDKey

	// Calculates the challenge response for the client.
	virtual bool CDKeyComputeChallengeResponse( char const* pszCDKey, char const* pszChallenge, 
													  char* pszChallengeResponse, uint32& nChallengeResponseLen ) const = 0;

	// Validates the cdkey to make sure it has the right form.  Does not verify cdkey with server.
	virtual bool IsCDKeyValid( char const* pszCDKey ) const = 0;


	//////////////////////////////////////////////////////////////////////////////
	// Net message handling

	// Handle an incoming net message
	// Returns false if the message was not recognized
	// Note : Remove all external headers before calling this function
	virtual bool HandleNetMessage( uint8* pData, uint32 nDataLen, uint8 senderAddr[4], uint16 nPort) = 0;

private:

	// Prevent copying.
	IGameSpyBrowser( IGameSpyBrowser const& other );
	IGameSpyBrowser const& operator=( IGameSpyBrowser const& other );
};



class GAMESPY_API IGameSpyPatchInfo
{
public:

	IGameSpyPatchInfo( ) { };
	virtual ~IGameSpyPatchInfo() {}

// Types
public:

	// Status of patchinfo state.
	enum EPatchInfoStatus
	{
		eBrowserStatus_Idle,
		eBrowserStatus_Processing,
		eBrowserStatus_Complete,
		eBrowserStatus_Error,
	};

	struct PatchInfoResults
	{
		bool m_bNewVersionAvailable;
		bool m_bMandatoryPatch;
		std::string m_sNewVersion;
		std::string m_sURL;
	};

	// Contains initialization information about the game.
	struct StartupInfo
	{
		StartupInfo( )
		{
			m_eGameSKU = eGameSKU_Invalid;
			m_nPatchInfoDistributionId = 0;
		}

		// Game SKU
		EGameSKU		m_eGameSKU;
		// Version of the game running.
		std::string		m_sVersion;
		// Distribution ID.
		uint32 m_nPatchInfoDistributionId;
	};

// Functions
public:

	// Create an IGameSpyPatchInfo interface.
	static IGameSpyPatchInfo* Create( StartupInfo const& StartupInfo );
	// Deletes a IGameSpyPatchInfo interface.
	static void Delete( IGameSpyPatchInfo* pGameSpyBrowser );

	// Requests patch information from GameSpy.
	virtual bool RequestPatchInfo( ) = 0;
	// Stops request.
	virtual bool HaltRequest( ) = 0;
	// Gets the current status of the patchinfo request.
	virtual EPatchInfoStatus GetPatchInfoStatus( ) const = 0;
	// Gets the results.
	virtual bool GetPatchInfoResults( PatchInfoResults& patchInfoResults ) const = 0;

private:

	// Prevent copying.
	IGameSpyPatchInfo( IGameSpyPatchInfo const& other );
	IGameSpyPatchInfo const& operator=( IGameSpyPatchInfo const& other );
};

#ifdef __cplusplus
extern "C"
{
#endif

// Validates the cdkey to make sure it has the right form.  Does not verify cdkey with server.
GAMESPY_API bool IsCDKeyValid( char const* pszCDKey, EGameSKU eGameSKU );

#ifdef __cplusplus
}
#endif

#endif //__IGAMESPY_H__
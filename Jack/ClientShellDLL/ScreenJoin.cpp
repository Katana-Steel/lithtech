// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenJoin.cpp
//
// PURPOSE : Interface screen to search for and join LAN games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenJoin.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientRes.h"
#include "ClientMultiplayerMgr.h"
#include "msgids.h"
#include "MissionMgr.h"
#include "ResShared.h"
#include "igamespy.h"
#include "GameClientShell.h"
#include <algorithm>

void JoinCallBack(LTBOOL bReturn, void *pData);

namespace
{
	int kColumnWidth_ServerName		= 150;
	int kColumnWidth_PlayerName		= 225;
	int kColumnWidth_Ping			= 60;
	int kColumnWidth_Players		= 80;
	int kColumnWidth_Lock			= 25;
	int kColumnWidth_Type			= 60;
	int kColumnWidth_Version		= 245;

	enum eLocalCommands 
	{
		CMD_SORT_NAME = CMD_CUSTOM+1,
		CMD_SORT_PING,
		CMD_SORT_PLAYER,
		CMD_SORT_LOCK,
		CMD_SORT_VERSION,
		CMD_SORT_TYPE,
		CMD_FILTER_VERSION,
		CMD_FILTER_PLAYERS,
		CMD_FILTER_PING,
		CMD_FILTER_MOD,
		CMD_FILTER_TYPE,
		CMD_DETAILS,

	};


	void JoinConfirmationCallback(LTBOOL bReturn, void *pData)
	{
		CScreenJoin *pThisScreen = (CScreenJoin*)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_JOIN);
		if (bReturn && pThisScreen)
		{
			pThisScreen->CheckPassword();
		}
	}
	void EditPassCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenJoin *pThisScreen = (CScreenJoin *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_JOIN);
		if (bReturn && pThisScreen)
		{
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_PASS);
		}
	};

	void EditPortCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenJoin *pThisScreen = (CScreenJoin *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_JOIN);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_PORT);
	};
}

struct SortedServerEntry
{
	TServerEntryMap::const_iterator m_mapIter;
};
typedef std::vector< SortedServerEntry > SortedServerList;

class CServerSortName
{
public:
	
	bool operator()(const SortedServerEntry & x, const SortedServerEntry & y) const
	{
		ServerEntry const& seX = x.m_mapIter->second;
		ServerEntry const& seY = y.m_mapIter->second;

		return (stricmp( seX.m_sName.c_str(), seY.m_sName.c_str()) < 0 );
	}
};
class CServerSortVersion
{
public:
	
	bool operator()(const SortedServerEntry & x, const SortedServerEntry & y) const
	{
		ServerEntry const& seX = x.m_mapIter->second;
		ServerEntry const& seY = y.m_mapIter->second;

		// If the versions are different, then just compare those.
		int nVersionCmp = stricmp(seX.m_sVersion.c_str(), seY.m_sVersion.c_str());
		if( nVersionCmp != 0 )
			return ( nVersionCmp < 0 );

		// Versions are different, so it's up to the mod name to differentiate them.
		return (stricmp( seX.m_sModName.c_str(), seY.m_sModName.c_str() ) < 0);
	}
};
class CServerSortPing
{
public:
	
	bool operator()(const SortedServerEntry & x, const SortedServerEntry & y) const
	{
		ServerEntry const& seX = x.m_mapIter->second;
		ServerEntry const& seY = y.m_mapIter->second;

		return (seX.m_nPing < seY.m_nPing );
	}
};

class CServerSortPlayers
{
public:
	
	bool operator()(const SortedServerEntry & x, const SortedServerEntry & y) const
	{
		ServerEntry const& seX = x.m_mapIter->second;
		ServerEntry const& seY = y.m_mapIter->second;

		return (seX.m_nNumPlayers < seY.m_nNumPlayers );
	}
};

class CServerSortLock
{
public:
	
	bool operator()(const SortedServerEntry & x, const SortedServerEntry & y) const
	{
		ServerEntry const& seX = x.m_mapIter->second;
		ServerEntry const& seY = y.m_mapIter->second;

		return (seX.m_bUsePassword || !seY.m_bUsePassword );
	}
};

class CServerSortType
{
public:

	bool operator()( const SortedServerEntry &x, const SortedServerEntry &y ) const
	{
		ServerEntry const& seX = x.m_mapIter->second;
		ServerEntry const& seY = y.m_mapIter->second;

		return (seX.m_nGameType < seY.m_nGameType);
	}
};

static void SortServers( TServerEntryMap const& serverEntryMap, uint32 nSortField, SortedServerList& lstSortedServers )
{
	// Add the servers to a sortable vector.
	lstSortedServers.resize( 0 );
	lstSortedServers.reserve( serverEntryMap.size( ));
	SortedServerEntry sortedServerEntry;
	for( TServerEntryMap::const_iterator iter = serverEntryMap.begin( ); iter != serverEntryMap.end( ); iter++ )
	{
		sortedServerEntry.m_mapIter = iter;
		lstSortedServers.push_back( sortedServerEntry );
	}

	switch(nSortField)
	{
	case CMD_SORT_NAME:
		std::sort(lstSortedServers.begin(),lstSortedServers.end(),CServerSortName());
		break;
	case CMD_SORT_PING:
		std::sort(lstSortedServers.begin(),lstSortedServers.end(),CServerSortPing());
		break;
	case CMD_SORT_PLAYER:
		std::sort(lstSortedServers.begin(),lstSortedServers.end(),CServerSortPlayers());
		break;
	case CMD_SORT_LOCK:
		std::sort(lstSortedServers.begin(),lstSortedServers.end(),CServerSortLock());
		break;
	case CMD_SORT_VERSION:
		std::sort(lstSortedServers.begin(),lstSortedServers.end(),CServerSortVersion());
		break;
	case CMD_SORT_TYPE:
		std::sort( lstSortedServers.begin(), lstSortedServers.end(), CServerSortType() );
		break;

	default:
		ASSERT(!"Unknown sort command.");
		return;

	};
}



// ----------------------------------------------------------------------- //
// Function name   : SetSummaryInfo
// Description     : Sets the summary info on the column control.
// Return type     : static bool - true on success.
// Argument        : CLTGUIColumnCtrl& columnCtrl - column control to setup.
// Argument        : char const* pszAddress - address of new server.
// Argument        : ServerEntry const& serverEntry - serverentry to use.
// ----------------------------------------------------------------------- //
static bool SetSummaryInfo( CLTGUIColumnCtrl& columnCtrl, char const* pszAddress, ServerEntry const& serverEntry )
{
	char aTempBuffer[256];
	CLTGUITextCtrl* pColumn = NULL;

	// Make sure we have all the columns.
	if( columnCtrl.GetNumColumns( ) == 0 )
	{
		columnCtrl.AddColumn( "", kColumnWidth_ServerName, LTTRUE );
		columnCtrl.AddColumn( "", kColumnWidth_Type, LTTRUE );
		columnCtrl.AddColumn( "", kColumnWidth_Ping, LTTRUE );
		columnCtrl.AddColumn( "", kColumnWidth_Players, LTTRUE );
		columnCtrl.AddColumn( "", kColumnWidth_Lock, LTTRUE );
		columnCtrl.AddColumn( "", kColumnWidth_Version, LTTRUE );
	}

	// Do the name
	uint8 nColumnIndex = 0;
	pColumn = columnCtrl.GetColumn( nColumnIndex++ );
	pColumn->SetString( serverEntry.m_sName.c_str( ));
	

	// Do the game type...
	switch(serverEntry.m_nGameType) 
	{
	case eGameTypeCooperative:
		sprintf( aTempBuffer, "%s", LoadTempString(IDS_COOPERATIVE) );
		break;
	case eGameTypeDeathmatch:
		sprintf( aTempBuffer, "%s", LoadTempString(IDS_DEATHMATCH) );
		break;
	case eGameTypeTeamDeathmatch:
		sprintf( aTempBuffer, "%s", LoadTempString(IDS_TEAMDEATHMATCH) );
		break;
	case eGameTypeDoomsDay:
		sprintf( aTempBuffer, "%s", LoadTempString(IDS_DOOMSDAY) );
		break;
	case eGameTypeDemolition:
		sprintf( aTempBuffer, "%s", LoadTempString(IDS_DEMOLITION) );
		break;
	default:
		sprintf( aTempBuffer, "%s", LoadTempString( IDS_UNKNOWN ) );
	}
	pColumn = columnCtrl.GetColumn( nColumnIndex++ );
	pColumn->SetString( aTempBuffer);

	// Do the ping
	sprintf(aTempBuffer, "%d", serverEntry.m_nPing);
	pColumn = columnCtrl.GetColumn( nColumnIndex++ );
	pColumn->SetString( aTempBuffer);

	// Do the number of players
	sprintf(aTempBuffer, "%d/%d", serverEntry.m_nNumPlayers, serverEntry.m_nMaxPlayers);
	pColumn = columnCtrl.GetColumn( nColumnIndex++ );
	pColumn->SetString( aTempBuffer);

	if (serverEntry.m_bUsePassword)
	{
		pColumn = columnCtrl.GetColumn( nColumnIndex++ );
		pColumn->SetString( "x" );
	}
	else
	{
		pColumn = columnCtrl.GetColumn( nColumnIndex++ );
		pColumn->SetString( " " );
	}


	// Do the version
	if (stricmp( serverEntry.m_sModName.c_str(), RETAIL_MOD_NAME ) == 0 )
		sprintf( aTempBuffer, "%s", serverEntry.m_sVersion.c_str() );
	else
		sprintf( aTempBuffer, "%s, %s", serverEntry.m_sVersion.c_str(), serverEntry.m_sModName.c_str() );
	pColumn = columnCtrl.GetColumn( nColumnIndex++ );
	pColumn->SetString( aTempBuffer );

	// If their version/mod is different than our version, gray it out.
	bool bEnabled = true;
	if( stricmp(serverEntry.m_sVersion.c_str(), g_pVersionMgr->GetNetVersion()) != 0)
	{
		bEnabled = false;
	}
	if( bEnabled && stricmp( serverEntry.m_sModName.c_str( ), g_pClientMultiplayerMgr->GetModName( )) != 0)
	{
		bEnabled = false;
	}
	columnCtrl.Enable( bEnabled );

	// Remember where we came from
	columnCtrl.SetParam1( reinterpret_cast< uint32 >( pszAddress ));

	return true;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenJoin::CScreenJoin() :
	m_eCurState			( eState_Inactive ),
	m_nLastSort			( CMD_SORT_PING ),
	m_nVersionFilter	( 0 ),
	m_nPlayersFilter	( 0 ),
	m_nPingFilter		( 0 ),
	m_nModFilter		( 0 ),
	m_eUpdateDirState	( eUpdateDirState_Idle ),
	m_pServerListCtrl	( NULL )
{
	
}

CScreenJoin::~CScreenJoin()
{
	if( m_bInit )
		Term( );
}

// Build the screen
LTBOOL CScreenJoin::Build()
{
 	// Make sure to call the base class
	if (!CBaseScreen::Build()) return LTFALSE;

	CreateTitle(IDS_TITLE_JOIN);

	//read layout info -----------------------------------------------
	uint8  headerFontSize = (uint8)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"HeaderFontSize");
	LTRect serverRect = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_JOIN,"ServerRect");

	int totalwidth = serverRect.right - serverRect.left;
	kColumnWidth_Version = totalwidth;

	kColumnWidth_ServerName = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"NameWidth");
	kColumnWidth_Version -= kColumnWidth_ServerName;

	kColumnWidth_Type = g_pLayoutMgr->GetScreenCustomInt( SCREEN_ID_JOIN, "TypeWidth" );
	kColumnWidth_Version -= kColumnWidth_Type;

	kColumnWidth_Ping = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"PingWidth");
	kColumnWidth_Version -= kColumnWidth_Ping;
	
	kColumnWidth_Players = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"PlayerWidth");
	kColumnWidth_Version -= kColumnWidth_Players;

	kColumnWidth_Lock = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"LockWidth");
	kColumnWidth_Version -= kColumnWidth_Lock;

	ASSERT(kColumnWidth_Version > 0);
	if (kColumnWidth_Version <= 0) kColumnWidth_Version = 50;


	//find servers button -----------------------------------------------
	LTIntPt pos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_JOIN,"FindPos");
	m_pFindCtrl = AddTextItem(IDS_FIND_SERVERS,CMD_SEARCH,IDS_HELP_FIND_SERVERS,pos);
	m_pFindCtrl->Enable(LTFALSE);
	m_pFindCtrl->SetFont(NULL,headerFontSize);

	// server sort buttons -----------------------------------------------
	pos.x = serverRect.left;
	pos.y = serverRect.top;

	CLTGUITextCtrl* pCtrl = AddTextItem(IDS_SERVER_NAME,CMD_SORT_NAME,IDS_HELP_SERVER_NAME,pos);
	pCtrl->SetFont(NULL,headerFontSize);
	pos.x += kColumnWidth_ServerName;

	pCtrl = AddTextItem( IDS_GAME_TYPE, CMD_SORT_TYPE, IDS_HELP_SERVER_TYPE, pos );
	pCtrl->SetFont( NULL, headerFontSize );
	pos.x += kColumnWidth_Type;

	pCtrl = AddTextItem(IDS_SERVER_PING,CMD_SORT_PING,IDS_HELP_SERVER_PING,pos);
	pCtrl->SetFont(NULL,headerFontSize);
	pos.x += kColumnWidth_Ping;

	pCtrl = AddTextItem(IDS_SERVER_PLAYERS,CMD_SORT_PLAYER,IDS_HELP_SERVER_PLAYERS,pos);
	pCtrl->SetFont(NULL,headerFontSize);
	pos.x += kColumnWidth_Players;

	pCtrl = AddTextItem(IDS_SERVER_LOCKED,CMD_SORT_LOCK,0,pos,LTTRUE);
	pCtrl->SetFont(NULL,headerFontSize);
	pCtrl->Enable(LTFALSE);
	pos.x += kColumnWidth_Lock;

	pCtrl = AddTextItem(IDS_SERVER_VERSION,CMD_SORT_VERSION,IDS_HELP_SERVER_VERSION,pos);
	pCtrl->SetFont(NULL,headerFontSize);

	// server list -----------------------------------------------
	pos.x = serverRect.left;
	pos.y += headerFontSize + 4;
	uint16 height = serverRect.bottom - pos.y;
	m_pServerListCtrl = AddList(pos,height,LTTRUE,totalwidth);
	m_pServerListCtrl->SetScrollWrap(LTFALSE);
	m_pServerListCtrl->SetFrameWidth(2);
	m_pServerListCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_DisabledColor);
	m_pServerListCtrl->SetIndent(2);

	// filter buttons -----------------------------------------------
	pos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_JOIN,"BaseFilterPos");
	pCtrl = AddTextItem(IDS_SERVER_FILTERS,NULL,0,pos,LTTRUE);
	pCtrl->SetFont(NULL,headerFontSize);
	int kFilterNameWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"FilterNameWidth");

	// Temporarily set the item spacing for the filter controls...

	int nOldSpacing = GetItemSpacing();
	int nFilterSpacing = g_pLayoutMgr->GetScreenCustomInt( SCREEN_ID_JOIN, "FilterItemSpacing" );
	SetItemSpacing( nFilterSpacing );

	m_pPort = AddColumnCtrl(CMD_EDIT_PORT, IDS_HELP_ENTER_PORT);
	int kPortColumnWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"PortColumnWidth");
	m_pPort->AddColumn( LoadTempString( IDS_SCANLANPORTS ), kPortColumnWidth);
	SAFE_STRCPY(m_szPort,"27888");
	m_pPort->AddColumn(m_szPort, kPortColumnWidth, LTTRUE);

	CLTGUICycleCtrl* pCycle = AddCycle( IDS_VERSION_FILTER,IDS_HELP_VERSION_FILTER,kFilterNameWidth, &m_nVersionFilter);
	pCycle->NotifyOnChange(CMD_FILTER_VERSION,this);
	pCycle->AddString(LoadTempString(IDS_ALL_VERSIONS));
	pCycle->AddString(LoadTempString(IDS_CURRENT_VERSION));

	pCycle = AddCycle(IDS_POP_FILTER,IDS_HELP_POP_FILTER,kFilterNameWidth,&m_nPlayersFilter);
	pCycle->NotifyOnChange(CMD_FILTER_PLAYERS,this);
	pCycle->AddString(LoadTempString(IDS_FILTER_ALLPOP));
	pCycle->AddString(LoadTempString(IDS_FILTER_NOTEMPTY));
	pCycle->AddString(LoadTempString(IDS_FILTER_NOTFULL));
	pCycle->AddString(LoadTempString(IDS_FILTER_NOT_BOTH));

	pCycle = AddCycle(IDS_FILTER_PING,IDS_HELP_FILTER_PING,kFilterNameWidth,&m_nPingFilter);
	pCycle->NotifyOnChange(CMD_FILTER_PING,this);
	pCycle->AddString(LoadTempString(IDS_FILTER_ALLPING));
	pCycle->AddString(LoadTempString(IDS_FILTER_LOWPING));
	pCycle->AddString(LoadTempString(IDS_FILTER_MEDPING));
	pCycle->AddString(LoadTempString(IDS_FILTER_HIGHPING));

	// Reset the item spacing before the last filter control is added...

/*
	pCycle = AddCycle( IDS_FILTER_MOD, IDS_HELP_FILTER_MOD, kFilterNameWidth, &m_nModFilter);
	pCycle->NotifyOnChange( CMD_FILTER_MOD, this );
	pCycle->AddString( LoadTempString(IDS_FILTER_ALLMODS) );
	pCycle->AddString( RETAIL_MOD_NAME );
	pCycle->AddString( LoadTempString(IDS_FILTER_CURRENTMOD) );
*/

	SetItemSpacing( nOldSpacing );

	pCycle = AddCycle( IDS_FILTER_TYPE, IDS_HELP_FILTER_TYPE, kFilterNameWidth, &m_nTypeFilter);
	pCycle->NotifyOnChange( CMD_FILTER_TYPE, this );
	pCycle->AddString( LoadTempString(IDS_ALL) );
	pCycle->AddString( LoadTempString(IDS_DEATHMATCH) );
	pCycle->AddString( LoadTempString(IDS_TEAMDEATHMATCH) );
	pCycle->AddString( LoadTempString(IDS_DOOMSDAY) );
	pCycle->AddString( LoadTempString(IDS_DEMOLITION) );
			
	// status text -----------------------------------------------
/*	pos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_JOIN,"StatusPos");
	char szTmp[256] = "";
	LoadString(IDS_WAITING,szTmp,sizeof(szTmp));
	m_pStatusCtrl = AddTextItem(FormatTempString(IDS_STATUS_STRING,szTmp), 0, 0, pos, LTTRUE);
*/
	// "please wait" popup  window -----------------------------------------------
	uint8 nFont = g_pLayoutMgr->GetDialogFontFace();
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
	uint8 nFontSize = g_pLayoutMgr->GetDialogFontSize();

	CLTGUITextCtrl*	pText = debug_new(CLTGUITextCtrl);
    if (!pText->Create(LoadTempString(IDS_QUERYING), NULL, LTNULL, pFont, nFontSize, this))
	{
		debug_delete(pText);
        return LTFALSE;
	}
	pText->SetColors(argbBlack, argbBlack, argbBlack);
	pText->Enable(LTFALSE);

	uint16 w = 16+pText->GetBaseWidth();
	uint16 h = 16+pText->GetBaseHeight();


	m_pWait = debug_new(CLTGUIWindow);
	char szBack[128] = "";
	g_pLayoutMgr->GetDialogFrame(szBack,sizeof(szBack));
	m_pWait->Create(g_pInterfaceResMgr->GetTexture(szBack),w,h);
	uint16 x = (640-w)/2;
	uint16 y = (480-h)/2;
	m_pWait->SetBasePos(LTIntPt(x,y));
	AddControl(m_pWait);
	
	m_pWait->AddControl(pText,LTIntPt(8,8));

	

	// details popup  window -----------------------------------------------
	LTRect detailsRect = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_JOIN,"DetailsRect");
	uint8  optionFontSize = (uint8)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"OptionsFontSize");
	uint16  optionHeight = (uint16)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"OptionsHeight");
	pos.x = detailsRect.left;
	pos.y = detailsRect.top;
	uint16 nDetailsWidth = detailsRect.right-detailsRect.left;
	uint16 nDetailsHeight = detailsRect.bottom-detailsRect.top;

	m_pDetails = debug_new(CLTGUIWindow);
	g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_JOIN,"DetailsFrame",szBack,sizeof(szBack));
	m_pDetails->Create(g_pInterfaceResMgr->GetTexture(szBack),nDetailsWidth,nDetailsHeight);
	m_pDetails->SetBasePos(pos);
	AddControl(m_pDetails);

	pos.x = 8;
	pos.y = 8;

	
	CLTGUITextCtrl* pOption = CreateTextItem(IDS_HOST_OPTIONS,0,0,pos,LTTRUE);
	pOption->SetFont(NULL,headerFontSize);
	m_pDetails->AddControl(pOption,pos);
	pos.y += (headerFontSize + 4);

	uint16 width = nDetailsWidth - 32;
	kColumnWidth_PlayerName = width - kColumnWidth_Ping;

	m_pOptions = CreateTextItem(" ",0,0,pos,LTTRUE);
	m_pOptions->SetFixedWidth(width);
	m_pOptions->SetFont(NULL,optionFontSize);
	m_pDetails->AddControl(m_pOptions,pos);
	pos.y += (optionHeight + 4);

	pOption = CreateTextItem(IDS_SERVER_MAP,0,0,pos,LTTRUE);
	pOption->SetFont(NULL,optionFontSize);
	m_pDetails->AddControl(pOption,pos);

	LTIntPt tmp = pos;
	tmp.x += pOption->GetBaseWidth();
	tmp.x += 16;

	m_pMission = CreateTextItem(IDS_SERVER_MISSION,0,0,pos,LTTRUE);
	m_pMission->SetFont(NULL,optionFontSize);
	m_pDetails->AddControl(m_pMission,tmp);

	pos.y += (optionFontSize + 4);

	CLTGUITextCtrl* pPlayer = CreateTextItem(IDS_SERVER_PLAYERS,0,0,pos,LTTRUE);
	pPlayer->SetFont(NULL,headerFontSize);
	m_pDetails->AddControl(pPlayer,pos);
	pos.x += kColumnWidth_PlayerName;

	CLTGUITextCtrl* pPing = CreateTextItem(IDS_SERVER_PING,0,0,pos,LTTRUE);
	pPing->SetFont(NULL,headerFontSize);
	m_pDetails->AddControl(pPing,pos);

	pos.x = 8;
	pos.y += (headerFontSize + 4);
	height = nDetailsHeight - pos.y - headerFontSize - 2 * m_nItemSpacing;

	m_pPlayerListCtrl = CreateList(pos,height,LTTRUE,width);
	m_pPlayerListCtrl->SetScrollWrap(LTFALSE);
	m_pPlayerListCtrl->SetScrollByPage(LTTRUE);
	m_pPlayerListCtrl->SetFrameWidth(1);
	m_pPlayerListCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
	m_pPlayerListCtrl->SetIndent(2);
	m_pPlayerListCtrl->Enable(LTTRUE);
	m_pDetails->AddControl(m_pPlayerListCtrl,pos);

	CLTGUITextCtrl* pCancel = CreateTextItem(IDS_CANCEL,CMD_CANCEL,0);
	pCancel->SetFont(NULL,headerFontSize);
	pos.y += (height + m_nItemSpacing);
	m_pDetails->AddControl(pCancel,pos);

	m_pJoin = CreateTextItem(IDS_JOIN_GAME,CMD_JOIN,IDS_HELP_JOIN_GAME);
	m_pJoin->SetFont(NULL,headerFontSize);
	pos.x = (nDetailsWidth - 8) - m_pJoin->GetBaseWidth();

	m_pDetails->AddControl(m_pJoin,pos);

	m_pDetails->Show(LTFALSE);

	return LTTRUE;
}

uint32 CScreenJoin::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_SORT_NAME:
	case CMD_SORT_PING:
	case CMD_SORT_PLAYER:
	case CMD_SORT_LOCK:
	case CMD_SORT_VERSION:
	case CMD_SORT_TYPE:
	{
		DisplayCurServerList( dwCommand );
		break;
	}

	case CMD_FILTER_VERSION:
	case CMD_FILTER_PING:
	case CMD_FILTER_PLAYERS:
	case CMD_FILTER_MOD:
	case CMD_FILTER_TYPE:
	{
		UpdateData();
		FilterServers();
		break;
	} 

	case CMD_CANCEL :
	{
		m_pDetails->Show(LTFALSE);
		SetCapture(NULL);
		ChangeState(eState_Waiting);
		break;
	}

	case CMD_DETAILS :
	{
		char const* pszAddress = reinterpret_cast< char const* >( dwParam1 );
		if( !pszAddress || !pszAddress[0] )
			return 0;

		m_sSelectedServerAddress = pszAddress;

		ChangeState(eState_QueryDetails);
		break;
	}

	case CMD_SEARCH :
	{
		FindServers();
		break;
	}

	case CMD_OK:
	{
		HandleCallback(dwParam1,dwParam2);
		break;
	}
	case CMD_JOIN:
	{

		m_pDetails->Show(LTFALSE);
		SetCapture(LTNULL);

		TServerEntryMap::const_iterator iter = m_cServerMap.find(m_sSelectedServerAddress);
		if( iter == m_cServerMap.end( ))
			return 0;
		std::string const& sPriavateAddress = iter->first;
		const ServerEntry &serverEntry = iter->second;

		if (stricmp(serverEntry.m_sVersion.c_str(), g_pVersionMgr->GetNetVersion( )) != 0)
		{
			MBCreate mb;
			g_pInterfaceMgr->ShowMessageBox(IDS_SERVER_WRONGVERSION,&mb);
			return 1;
		}

		if (serverEntry.m_nNumPlayers >= serverEntry.m_nMaxPlayers)
		{
			MBCreate mb;
			g_pInterfaceMgr->ShowMessageBox(IDS_SERVERFULL,&mb);
			return 1;
		}

		if( stricmp( serverEntry.m_sModName.c_str(), g_pClientMultiplayerMgr->GetModName() ) != 0 )
		{
			MBCreate mb;
			g_pInterfaceMgr->ShowMessageBox( IDS_SERVER_WRONGMOD, &mb );
			return 1;
		}

		CheckPassword();

		break;
	}
	case CMD_EDIT_PORT:
	{
		//show edit box here	
		MBCreate mb;
		mb.eType = LTMB_EDIT;
		mb.pFn = EditPortCallBack;
		mb.pString = m_szPort;
		mb.nMaxChars = 5;
		mb.eInput = CLTGUIEditCtrl::kInputNumberOnly;
		g_pInterfaceMgr->ShowMessageBox(IDS_SCANLANPORTS,&mb);
	} break;
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
}

uint32 CScreenJoin::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	switch (dwParam2)
	{
		case CMD_EDIT_PORT:
		{
			char *pszPort = (char *)dwParam1;
			uint16 nPort = (uint16)atoi(pszPort);
			if (IsValidPort(nPort))
			{
				SAFE_STRCPY(m_szPort,pszPort);
				m_pPort->SetString(1,m_szPort);
			}
			break;
		}
		case CMD_EDIT_PASS :
		{
			m_sPassword = (char *)dwParam1;
			JoinCurGame();
			break;
		}
	}
	ChangeState(eState_Waiting);
	return 1;
}


// Change in focus
void    CScreenJoin::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	// Update our state when the focus changes
	if (bFocus)
	{
		m_pWait->Show(LTTRUE);
		SetCapture(m_pWait);

		sprintf(m_szPort,"%d",pProfile->m_nClientPort);
		m_pPort->GetColumn(1)->SetString(m_szPort);

		bool bCreatedBrowsers = false;

		if( !g_pClientMultiplayerMgr->GetRetailServerBrowser( ) ||
			!g_pClientMultiplayerMgr->GetDemoServerBrowser( ))
		{
			g_pClientMultiplayerMgr->CreateServerBrowsers( );
			bCreatedBrowsers = true;
		}

		// Only do startup if the browsers haven't been created yet.
		if( bCreatedBrowsers )
			ChangeState(eState_Startup);
		else
			ChangeState(eState_Waiting);
	}
	else 
	{
		// Don't go!
		ChangeState(eState_Inactive);
	}

	if (bFocus)
	{

		// we may have gotten here after failing a join, so rebuild our history
		if (m_pScreenMgr->GetLastScreenID() == SCREEN_ID_NONE)
		{
			m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MAIN);
			m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MULTI);
		}

		SetSelection(GetIndex(m_pFindCtrl));

        UpdateData(LTFALSE);
	}
	else
	{
		pProfile->m_nClientPort = (uint16)atoi(m_szPort);
		pProfile->Save();

		UpdateData();
	}

	CBaseScreen::OnFocus(bFocus);
}

void CScreenJoin::FindServers()
{
	// Don't allow a new find while the browsers are working.
	if(( g_pClientMultiplayerMgr->GetRetailServerBrowser( )->GetBrowserStatus( ) == IGameSpyBrowser::eBrowserStatus_Processing ) ||
		g_pClientMultiplayerMgr->GetDemoServerBrowser( )->GetBrowserStatus( ) == IGameSpyBrowser::eBrowserStatus_Processing )
		return;

	ChangeState(eState_UpdateDir);
}

void CScreenJoin::DisplayCurServerList( uint32 nSortField )
{
	// Remove all the controls from the list, but don't delete them.
	m_pServerListCtrl->RemoveAll( LTFALSE );

	// Sort the servers.
	SortedServerList lstSortedServers;
	m_nLastSort = nSortField;
	SortServers( m_cServerMap, nSortField, lstSortedServers );

	uint16 nSelected = CLTGUIListCtrl::kNoSelection;

	SortedServerList::const_iterator iCurServer = lstSortedServers.begin();
	for (; iCurServer != lstSortedServers.end(); ++iCurServer)
	{
		std::string const& sPrivateAddress = iCurServer->m_mapIter->first;
		ServerEntry const& serverEntry = iCurServer->m_mapIter->second;

		// Skip it if it the control is invalid.
		if( !serverEntry.m_pColumnCtrl )
		{
			ASSERT( !"CScreenJoin::DisplayCurServerList: Invalid column control." );
			continue;
		}

		// Add the control.
		uint16 nCtrlIndex = m_pServerListCtrl->AddControl( serverEntry.m_pColumnCtrl );

		// If this is the server we had selected before, reselect it.
		if( stricmp( sPrivateAddress.c_str( ), m_sSelectedServerAddress.c_str( )) == 0 )
			nSelected = nCtrlIndex;
	}

//	m_pServerListCtrl->Enable( (m_pServerListCtrl->GetNumControls() > 0));
	m_pServerListCtrl->SetSelection(nSelected);
	FilterServers();
}

void CScreenJoin::DisplayDetails()
{
	char aStringBuffer[256];
	m_pPlayerListCtrl->RemoveAll();

	// Get the serverentry we are showing details for.
	TServerEntryMap::const_iterator iter = m_cServerMap.find(m_sSelectedServerAddress);
	if( iter == m_cServerMap.end( ))
		return;
	std::string const& sPrivateAddress = iter->first;
	const ServerEntry &serverEntry = iter->second;

	int nMission, nLevel;
	std::string sMission = "";
	if(g_pMissionButeMgr->IsMissionLevel( serverEntry.m_sMission.c_str( ),nMission,nLevel))
	{
		MISSION* pMission = g_pMissionButeMgr->GetMission(nMission); 
		if (pMission)
		{
			if (pMission->nNameId > 0)
				sMission = LoadTempString(pMission->nNameId);
			else if (!pMission->sName.empty())
				sMission = pMission->sName;
		}
	}

	if (sMission.empty())
		sMission = serverEntry.m_sMission.c_str( );

	m_pMission->SetString(sMission.c_str());

	//read game options and build the options string
	std::string sOptions = "";
	sOptions.reserve(64);
	char szOptionsString[256] = "";

	LTStrCpy(szOptionsString,serverEntry.m_sOptions.c_str(),sizeof(szOptionsString));

	if (strlen(szOptionsString))
	{

		//find friendly fire value
		char *pTok = strtok(szOptionsString,";");
		sOptions += LoadTempString(IDS_FRIENDLY_FIRE);
		sOptions += ": ";
		pTok = strpbrk( pTok, "0123456789" );
		if (pTok && *pTok >= '1')
		{
			sOptions += LoadTempString(IDS_YES);
		}
		else
		{
			sOptions += LoadTempString(IDS_NO);
		}
		sOptions += ";  ";

		//find weapon stay value
		pTok = strtok(NULL,";");
		sOptions += LoadTempString(IDS_WEAPONS_STAY);
		sOptions += ": ";
		pTok = strpbrk( pTok, "0123456789" );
		if (pTok && *pTok >= '1')
		{
			sOptions += LoadTempString(IDS_YES);
		}
		else
		{
			sOptions += LoadTempString(IDS_NO);
		}
		sOptions += ";  ";

		//num rounds
		pTok = strtok(NULL,";");
		sOptions += LoadTempString(IDS_ROUNDS);
		sOptions += ": ";
		pTok = strpbrk( pTok, "0123456789" );
		sOptions += pTok;
		sOptions += ";  ";

		//runspeed
		pTok = strtok(NULL,";");
		sOptions += LoadTempString(IDS_RUN_SPEED);
		sOptions += ": ";
		pTok = strpbrk( pTok, "0123456789" );
		sOptions += pTok;

	}



	GameType eType = (GameType)serverEntry.m_nGameType;

	if (eType == eGameTypeDeathmatch || eType == eGameTypeTeamDeathmatch)
	{
		sprintf(aStringBuffer,"; %s: %d",LoadTempString(IDS_SCORE_LIMIT), serverEntry.m_nFragLimit);
		sOptions += aStringBuffer;
	}

	if (eType != eGameTypeDemolition)
	{
		sprintf(aStringBuffer,"; %s: %d",LoadTempString(IDS_TIME_LIMIT), serverEntry.m_nTimeLimit);
		sOptions += aStringBuffer;
	}
	
	m_pOptions->SetString(sOptions.c_str());

	//read player list
	char szPing[6];
	for( uint32 i = 0; i < serverEntry.m_lstPlayerEntry.size( ); i++ )
	{
		PlayerEntry const& playerEntry = serverEntry.m_lstPlayerEntry[i];

		// Create a control
		CLTGUIColumnCtrl* pCtrl = CreateColumnCtrl(0,0,kDefaultPos,LTTRUE);
		// Do the name
		pCtrl->AddColumn(playerEntry.m_sName.c_str( ), kColumnWidth_PlayerName,LTTRUE);

		// Do the ping
		sprintf( szPing, "%d", playerEntry.m_nPing );
		pCtrl->AddColumn( szPing, kColumnWidth_Ping);

		m_pPlayerListCtrl->AddControl(pCtrl);
	}

	m_pJoin->Enable(LTTRUE);
	m_pDetails->Show(LTTRUE);
	m_pDetails->SetSelection(m_pDetails->GetIndex(m_pJoin));
	SetCapture(m_pDetails);
}

void CScreenJoin::SetDetailErrorMessage(const char *szError)
{
	m_pPlayerListCtrl->RemoveAll();

	if (m_pWait->IsVisible())
		m_pWait->Show(LTFALSE);
	if (m_pDetails->IsVisible())
		m_pDetails->Show(LTFALSE);
	SetCapture(NULL);

	//show edit box here	
	MBCreate mb;
	g_pInterfaceMgr->ShowMessageBox(szError,&mb);

}


LTBOOL CScreenJoin::Render(HSURFACE hDestSurf)
{
	CWinSync_CSAuto cs( m_csAddServer );

	Update();

	return CBaseScreen::Render(hDestSurf);
}

LTBOOL CScreenJoin::HandleKeyDown(int key, int rep)
{
	if (key == VK_F5)
	{
		FindServers();
        return LTTRUE;
		}
    return CBaseScreen::HandleKeyDown(key,rep);
}

bool CScreenJoin::IsCurrentGame(const char *pAddr)
{
	if (!IsMultiplayerGame( )) return false;

	uint32 aAddr[4];
	uint16 nPort;
	aAddr[0] = 0;
	aAddr[1] = 0;
	aAddr[2] = 0;
	aAddr[3] = 0;
	sscanf(pAddr, "%d.%d.%d.%d:%d", &aAddr[0], &aAddr[1], &aAddr[2], &aAddr[3], &nPort);

	uint8 nServerAddr[4];
	uint16 nServerPort;
	g_pLTClient->GetServerIPAddress(nServerAddr, &nServerPort);

	if ((aAddr[0] == (uint32)nServerAddr[0]) &&
		(aAddr[1] == (uint32)nServerAddr[1]) &&
		(aAddr[2] == (uint32)nServerAddr[2]) &&
		(aAddr[3] == (uint32)nServerAddr[3]) &&
		(nPort == nServerPort))
		return true;
	else
		return false;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenJoin::JoinGame
//
//	PURPOSE:	Joins the given game service
//
// ----------------------------------------------------------------------- //

void CScreenJoin::JoinCurGame()
{
	// Get the currently selected game server...

	TServerEntryMap::const_iterator iter = m_cServerMap.find(m_sSelectedServerAddress);
	if( iter == m_cServerMap.end( ))
		return;
	std::string const& sPrivateAddress = iter->first;
	const ServerEntry &serverEntry = iter->second;

	g_pGameClientShell->SetGameType((GameType)serverEntry.m_nGameType);
	if( !g_pClientMultiplayerMgr->SetupClient( serverEntry.m_sName.c_str(), m_sPassword.c_str(), 
		!serverEntry.m_bDirectConnect, serverEntry.m_bConnectViaPublic, 
		serverEntry.m_sPublicAddress.c_str( ), sPrivateAddress.c_str( )))
	{
		ChangeState(eState_Inactive);
		g_pInterfaceMgr->LoadFailed(SCREEN_ID_JOIN);
		g_pInterfaceMgr->ConnectionFailed(g_pClientMultiplayerMgr->GetLastConnectionResult());
		return;
	}

	if( !g_pMissionMgr->StartGameAsClient( ))
	{
		ChangeState(eState_Inactive);
		g_pInterfaceMgr->LoadFailed(SCREEN_ID_JOIN);
		g_pInterfaceMgr->ConnectionFailed(g_pClientMultiplayerMgr->GetLastConnectionResult());
		return;
	}
}

bool CScreenJoin::PreState(EState eNewState)
{
	switch (eNewState)
	{
		case eState_Inactive :
			return PreState_Inactive();
		case eState_Startup :
			return PreState_Startup();
		case eState_UpdateDir :
			return PreState_UpdateDir();
		case eState_QueryDetails :
			return PreState_QueryDetails();
		case eState_Waiting :
			return PreState_Waiting();
		default :
			return true;
	}
}

bool CScreenJoin::PostState(EState eNewState)
{
	switch (m_eCurState)
	{
		case eState_UpdateDir :
			return PostState_UpdateDir(eNewState);
		case eState_QueryDetails :
			return PostState_QueryDetails(eNewState);
		case eState_Waiting :
			return PostState_Waiting(eNewState);
		default :
			return true;
	}
}

bool CScreenJoin::ChangeState(EState eNewState)
{
	if (!PostState(m_eCurState))
	{
		return false;
	}

	m_eCurState = eState_Transition;

	if (!PreState(eNewState))
	{
		m_pFindCtrl->Enable(LTTRUE);
		if (m_pWait->IsVisible())
			m_pWait->Show(LTFALSE);
		if (GetCapture() == m_pWait)
			SetCapture(NULL);
		m_eCurState = eState_Waiting;
		return false;
	}

	m_eCurState = eNewState;

	return true;
}

void CScreenJoin::Update()
{
	switch (m_eCurState)
	{
		case eState_Startup :
			Update_State_Startup();
			return;
		case eState_UpdateDir :
			Update_State_UpdateDir();
			return;
		case eState_QueryDetails :
			Update_State_QueryDetails();
			return;
		default :
			return;
	}
}

bool CScreenJoin::PreState_Inactive()
{
	// Make sure we're not doing anything...
	if( g_pClientMultiplayerMgr->GetRetailServerBrowser( ))
		g_pClientMultiplayerMgr->GetRetailServerBrowser( )->HaltRequest( );
	if( g_pClientMultiplayerMgr->GetDemoServerBrowser( ))
		g_pClientMultiplayerMgr->GetDemoServerBrowser( )->HaltRequest( );

	return true;
}

bool CScreenJoin::PreState_Startup()
{
	return true;
}

bool CScreenJoin::PreState_UpdateDir()
{
	m_pFindCtrl->Enable(LTFALSE);

	// Stop any requests going.
	if( g_pClientMultiplayerMgr->GetRetailServerBrowser( ))
		g_pClientMultiplayerMgr->GetRetailServerBrowser( )->HaltRequest( );
	if( g_pClientMultiplayerMgr->GetDemoServerBrowser( ))
		g_pClientMultiplayerMgr->GetDemoServerBrowser( )->HaltRequest( );

	// Clear out our current server list.
	m_pServerListCtrl->RemoveAll();

	// Clear out the server entry map.
	m_cServerMap.clear( );

	// Start off our state on the retialpublic.
	m_eUpdateDirState = eUpdateDirState_Idle;

	return true;
}

bool CScreenJoin::PreState_QueryDetails()
{
	TServerEntryMap::const_iterator iter = m_cServerMap.find(m_sSelectedServerAddress);
	if( iter == m_cServerMap.end( ))
		return false;
	std::string const& sPrivateAddress = iter->first;
	const ServerEntry &serverEntry = iter->second;

	if (stricmp(serverEntry.m_sVersion.c_str(), g_pVersionMgr->GetNetVersion()) != 0)
	{
		SetDetailErrorMessage(LoadTempString(IDS_SERVER_DETAIL_VERSION));
		return false;
	}

	// Convert the address string into ip and port.
	char szIP[16];
	uint32 nColon = serverEntry.m_sPublicAddress.rfind( ":", serverEntry.m_sPublicAddress.length( ) - 1);
	if( nColon == std::string::npos || nColon > ARRAY_LEN( szIP ))
		return false;
	strncpy( szIP, serverEntry.m_sPublicAddress.c_str( ), nColon );
	szIP[nColon] = 0;
	if( !serverEntry.m_sPublicAddress.c_str( )[nColon+1] )
		return false;
	uint16 nPort = min( atoi( serverEntry.m_sPublicAddress.c_str( ) + nColon + 1 ), 65535 );

	// Request the details for this server.
	IGameSpyBrowser* pGameSpyBrowser = ( serverEntry.m_bDemoServer ) ? 
		g_pClientMultiplayerMgr->GetDemoServerBrowser( ) : 
		g_pClientMultiplayerMgr->GetRetailServerBrowser( );
	if( !pGameSpyBrowser->RequestServerDetails( szIP, nPort, ServerInfoCallback, this ))
	{
		SetDetailErrorMessage(LoadTempString(IDS_SERVER_NORESPONSE));
		return false;
	}

	return true;
}


bool CScreenJoin::PreState_Waiting()
{
	m_pFindCtrl->Enable(LTTRUE);

	if (m_pWait->IsVisible())
		m_pWait->Show(LTFALSE);
	if (GetCapture() == m_pWait)
		SetCapture(NULL);


	return true;
}

bool CScreenJoin::PostState_UpdateDir(EState eNewState)
{
	// Show it
	DisplayCurServerList( m_nLastSort );
	return true;
}

bool CScreenJoin::PostState_QueryDetails(EState eNewState)
{
	// Read the details
	DisplayDetails();

	m_pFindCtrl->Enable(LTTRUE);
	return true;
}

bool CScreenJoin::PostState_Waiting(EState eNewState)
{
	m_pFindCtrl->Enable(LTFALSE);

	m_pWait->Show(LTTRUE);
	SetCapture(m_pWait);

	return true;
}

void CScreenJoin::Update_State_Startup()
{
	// Update the directory list
	ChangeState(eState_UpdateDir);
}

void CScreenJoin::Update_State_UpdateDir()
{
	IGameSpyBrowser* pResponseGameSpyBrowser;
	EUpdateDirState eRequestUpdateDirState;
	IGameSpyBrowser* pRequestGameSpyBrowser;
	bool bRequestPublic;

	// Setup the response/request data based on the state we're
	// in.  The response variables tell us what gamespybrowser
	// to check for status.  If we get a response, we switch to 
	// the new updatedirstate and request a new dir.
	switch( m_eUpdateDirState )
	{
		case eUpdateDirState_Idle:
		{
			pResponseGameSpyBrowser			= g_pClientMultiplayerMgr->GetRetailServerBrowser( );
			eRequestUpdateDirState			= eUpdateDirState_RetailPublic;
			pRequestGameSpyBrowser			= g_pClientMultiplayerMgr->GetRetailServerBrowser( );
			bRequestPublic					= true;
		}
		break;
		case eUpdateDirState_RetailPublic:
		{
			pResponseGameSpyBrowser			= g_pClientMultiplayerMgr->GetRetailServerBrowser( );
			eRequestUpdateDirState			= eUpdateDirState_RetailLan;
			pRequestGameSpyBrowser			= g_pClientMultiplayerMgr->GetRetailServerBrowser( );
			bRequestPublic					= false;
		}
		break;
		case eUpdateDirState_RetailLan:
		{
			pResponseGameSpyBrowser			= g_pClientMultiplayerMgr->GetRetailServerBrowser( );
			eRequestUpdateDirState			= eUpdateDirState_DemoPublic;
			pRequestGameSpyBrowser			= g_pClientMultiplayerMgr->GetDemoServerBrowser( );
			bRequestPublic					= true;
		}
		break;
		case eUpdateDirState_DemoPublic:
		{
			pResponseGameSpyBrowser			= g_pClientMultiplayerMgr->GetDemoServerBrowser( );
			eRequestUpdateDirState			= eUpdateDirState_DemoLan;
			pRequestGameSpyBrowser			= g_pClientMultiplayerMgr->GetDemoServerBrowser( );
			bRequestPublic					= false;
		}
		break;
		case eUpdateDirState_DemoLan:
		{
			pResponseGameSpyBrowser			= g_pClientMultiplayerMgr->GetDemoServerBrowser( );
			eRequestUpdateDirState			= eUpdateDirState_Finished;
			pRequestGameSpyBrowser			= NULL; // No new request needed when we get here.
			bRequestPublic					= false;
		}
		break;
		case eUpdateDirState_Finished:
		{
			ChangeState(eState_Waiting);
			return;
		}
		break;
		default:
		{
			ASSERT( !"CScreenJoin::Update_State_UpdateDir: Invalid updatedir state." );
			ChangeState(eState_Waiting);
			return;
		}
		break;
	}

	// Are we still waiting?
	switch( pResponseGameSpyBrowser->GetBrowserStatus( ))
	{
		// Still waiting for the results.
		case IGameSpyBrowser::eBrowserStatus_Processing:
			return;
		case IGameSpyBrowser::eBrowserStatus_Idle:
		case IGameSpyBrowser::eBrowserStatus_Complete:
		case IGameSpyBrowser::eBrowserStatus_Error:
		{
			// Stop any previous requests we were doing.
			pResponseGameSpyBrowser->HaltRequest( );

			// Make the new request.  If there isn't anything setup
			// to request from, then we're done.
			if( pRequestGameSpyBrowser )
			{
				uint16 nPort = atoi( m_szPort );

				// Request the next serverlist.
				pRequestGameSpyBrowser->RequestServerList( 
					bRequestPublic, nPort, NULL, ServerInfoCallback, this );
			}

			// Switch to the next state for when we are complete with this request.
			m_eUpdateDirState = eRequestUpdateDirState;

			break;
		}
		default :
		{
			ASSERT(!"Unknown directory status encountered");
			ChangeState(eState_Waiting);
			break;
		}
	}
}

void CScreenJoin::Update_State_QueryDetails()
{
	// Are we still waiting?
	if(( g_pClientMultiplayerMgr->GetRetailServerBrowser( )->GetBrowserStatus( ) == IGameSpyBrowser::eBrowserStatus_Processing ) ||
		g_pClientMultiplayerMgr->GetDemoServerBrowser( )->GetBrowserStatus( ) == IGameSpyBrowser::eBrowserStatus_Processing )
		return;

	//get back to work
	ChangeState(eState_Waiting);
}

void CScreenJoin::FilterServers()
{

	uint16 iFirst = CLTGUIListCtrl::kNoSelection; 
	for (uint16 i = 0 ;i < m_pServerListCtrl->GetNumControls(); ++i)
	{
		char const* pszAddress = reinterpret_cast< char const* >( m_pServerListCtrl->GetControl(i)->GetParam1());
		if( !pszAddress || !pszAddress[0] )
			return;

		TServerEntryMap::const_iterator iter = m_cServerMap.find(pszAddress);
		if( iter == m_cServerMap.end( ))
			return;
		std::string const& sPrivateAddress = iter->first;
		const ServerEntry &serverEntry = iter->second;

		bool bShow = true;

		//Version Filter
		if (m_nVersionFilter)
		{
			bShow &= (stricmp(serverEntry.m_sVersion.c_str(), g_pVersionMgr->GetNetVersion()) == 0);
		}

		//Game type filter
		GameType eType = (GameType)serverEntry.m_nGameType;
		switch(m_nTypeFilter)
		{
		case 1: //deathmatch
			bShow &= (eType == eGameTypeDeathmatch);
			break;
		case 2: //team deathmatch
			bShow &= (eType == eGameTypeTeamDeathmatch);
			break;
		case 3: //doomsday
			bShow &= (eType == eGameTypeDoomsDay);
			break;
		case 4: //demolition
			bShow &= (eType == eGameTypeDemolition);
			break;
		}

		switch (m_nPlayersFilter)
		{
		case 0:
			break;
		case 1:
			bShow &= (serverEntry.m_nNumPlayers > 0);
			break;
		case 2:
			bShow &= (serverEntry.m_nNumPlayers < serverEntry.m_nMaxPlayers);
			break;
		case 3:
			bShow &= (serverEntry.m_nNumPlayers > 0);
			bShow &= (serverEntry.m_nNumPlayers < serverEntry.m_nMaxPlayers);
			break;
		}

		switch (m_nPingFilter)
		{
		case 0:
			break;
		case 1:
			bShow &= (serverEntry.m_nPing < 100);
			break;
		case 2:
			bShow &= (serverEntry.m_nPing < 300);
			break;
		case 3:
			bShow &= (serverEntry.m_nPing < 500);
			break;
		}

		switch( m_nModFilter )
		{
			case 0:		// All Mods (no filtering)
			break; 
		
			case 1:		// Retail games only
			{
				bShow &= (stricmp( serverEntry.m_sModName.c_str(), RETAIL_MOD_NAME ) == 0 );
			}
			break;
		
			case 2:		// Currently selected mod only
			{
				bShow &= (stricmp( serverEntry.m_sModName.c_str(), g_pClientMultiplayerMgr->GetModName() ) == 0 );
			}
			break;

			default:
			break;
		}

		if (bShow && iFirst == CLTGUIListCtrl::kNoSelection)
			iFirst = i;

		m_pServerListCtrl->GetControl(i)->Show(bShow);
	}

	if (iFirst == CLTGUIListCtrl::kNoSelection)
	{
//		m_pServerListCtrl->Enable(false);
	}
	else
	{
//		m_pServerListCtrl->Enable(true);
		m_pServerListCtrl->SetStartIndex(iFirst);
	}

	m_pServerListCtrl->SetSelection(iFirst);

}



void CScreenJoin::CheckPassword()
{
	TServerEntryMap::const_iterator iter = m_cServerMap.find(m_sSelectedServerAddress);
	if( iter == m_cServerMap.end( ))
		return;
	std::string const& sPrivateAddress = iter->first;
	const ServerEntry &serverEntry = iter->second;

	m_sPassword = "";
	if( serverEntry.m_bUsePassword)
	{	
		//show edit box here	
		MBCreate mb;
		mb.eType = LTMB_EDIT;
		mb.pFn = EditPassCallBack;
		mb.pString = "";
		mb.nMaxChars = MAX_PASSWORD-1;
		g_pInterfaceMgr->ShowMessageBox(IDS_PASSWORD,&mb);
	}
	else
	{
		JoinCurGame();
	}
}

void CScreenJoin::Escape()
{
	if (m_pDetails->IsVisible())
	{
		m_pDetails->Show(LTFALSE);
		SetCapture(NULL);
		ChangeState(eState_Waiting);
		return;
	}


	CBaseScreen::Escape();
}

void CScreenJoin::ReadCoopDetails(std::string& sOptions,CLTMsgRef_Read& cRead)
{
	char aStringBuffer[256];

	bool bFriendlyFire = cRead->Readbool();
	sprintf(aStringBuffer,"%s: ",LoadTempString(IDS_FRIENDLY_FIRE));
	sOptions += aStringBuffer;
	if (bFriendlyFire)
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_YES));
	else
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_NO));
	sOptions += aStringBuffer;

	uint8 nDiff = cRead->Readuint8();
	sprintf(aStringBuffer,"%s: ",LoadTempString(IDS_DIFFICULTY));
	sOptions += aStringBuffer;
	switch  (nDiff)
	{
	case 3:
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_NEW_INSANE));
		break;
	case 2:
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_NEW_HARD));
		break;
	case 1:
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_NEW_MEDIUM));
		break;
	case 0:
	default:
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_NEW_EASY));
		break;

	}
	sOptions += aStringBuffer;

	float fPlayerDiff = cRead->Readfloat();
	sprintf(aStringBuffer,"%s: %0.2f; ",LoadTempString(IDS_HOST_PLAYERDIFF),fPlayerDiff);
	sOptions += aStringBuffer;

	//read player list
	bool bPlayerToRead = cRead->Readbool();
	while (bPlayerToRead)
	{
		cRead->ReadString(aStringBuffer, sizeof(aStringBuffer));
		uint16 nPing = cRead->Readuint16();
		bPlayerToRead = cRead->Readbool();

		// Create a control
		CLTGUIColumnCtrl* pCtrl = CreateColumnCtrl(0,0,kDefaultPos,LTTRUE);
		// Do the name
		pCtrl->AddColumn(aStringBuffer, kColumnWidth_PlayerName,LTTRUE);

		// Do the ping
		sprintf(aStringBuffer, "%d", nPing);
		pCtrl->AddColumn(aStringBuffer, kColumnWidth_Ping);

		m_pPlayerListCtrl->AddControl(pCtrl);

	}
}

static bool ReadServerEntry( IGameSpyBrowser& gameSpyBrowser, IGameSpyBrowser::HGAMESERVER hGameServer, std::string& sPrivateAddress, ServerEntry& serverEntry )
{
	char szString[256];

	// Get the connection information.
	char szPublicAddress[256];
	uint16 nPublicPort;
	char szPrivateAddress[256];
	uint16 nPrivatePort;
	if( !gameSpyBrowser.GetServerConnectInfo( hGameServer, szPrivateAddress, nPrivatePort, 
		szPublicAddress, nPublicPort, serverEntry.m_bConnectViaPublic, serverEntry.m_bDirectConnect ))
		return false;

	// Create a combined address string. 
	sPrivateAddress = szPrivateAddress;
	sPrivateAddress += ":";
	sprintf( szString, "%d", nPrivatePort );
	sPrivateAddress += szString;

	// Create a combined address string. 
	serverEntry.m_sPublicAddress = szPublicAddress;
	serverEntry.m_sPublicAddress += ":";
	sprintf( szString, "%d", nPublicPort );
	serverEntry.m_sPublicAddress += szString;

	// Get the server name.
	if( !gameSpyBrowser.GetServerProperty( hGameServer, "hostname", szString, ARRAY_LEN( szString )))
		return false;
	serverEntry.m_sName = szString;

	if( !gameSpyBrowser.GetServerProperty( hGameServer, "gamever", szString, ARRAY_LEN( szString )))
		return false;
	serverEntry.m_sVersion = szString;

	if( !gameSpyBrowser.GetServerProperty( hGameServer, "modname", szString, ARRAY_LEN( szString )))
		return false;
	serverEntry.m_sModName = szString;

	serverEntry.m_nNumPlayers = gameSpyBrowser.GetNumPlayers( hGameServer );

	if( !gameSpyBrowser.GetServerProperty( hGameServer, "maxplayers", szString, ARRAY_LEN( szString )))
		return false;
	serverEntry.m_nMaxPlayers = atoi( szString );

	if( !gameSpyBrowser.GetServerProperty( hGameServer, "password", szString, ARRAY_LEN( szString )))
		return false;
	serverEntry.m_bUsePassword = !!( atoi( szString ));

	if( !gameSpyBrowser.GetServerProperty( hGameServer, "gametype", szString, ARRAY_LEN( szString )))
		return false;
	GameType eGameType = GameStringTypeToGameType( szString );
	serverEntry.m_nGameType = eGameType;
		
	serverEntry.m_nPing = gameSpyBrowser.GetServerPing( hGameServer );

	// Make sure it has the details.
	if( gameSpyBrowser.HasServerDetails( hGameServer ))
	{
		// read the mission name
		if( gameSpyBrowser.GetServerProperty( hGameServer, "mapname", szString, ARRAY_LEN( szString )))
			serverEntry.m_sMission = szString;

		if( gameSpyBrowser.GetServerProperty( hGameServer, "options", szString, ARRAY_LEN( szString )))
			serverEntry.m_sOptions = szString;

		if( gameSpyBrowser.GetServerProperty( hGameServer, "fraglimit", szString, ARRAY_LEN( szString )))
			serverEntry.m_nFragLimit = min( atoi( szString ), 65535 );

		if( gameSpyBrowser.GetServerProperty( hGameServer, "timelimit", szString, ARRAY_LEN( szString )))
			serverEntry.m_nTimeLimit = min( atoi( szString ), 65535 );

		//read player list
		uint32 nNumPlayers = gameSpyBrowser.GetNumPlayers( hGameServer );
		serverEntry.m_lstPlayerEntry.resize( 0 );
		serverEntry.m_lstPlayerEntry.reserve( nNumPlayers );
		char szKey[256];
		PlayerEntry playerEntry;
		for( uint32 i = 0; i < nNumPlayers; i++ )
		{
			sprintf( szKey, "player_%d", i );
			if( !gameSpyBrowser.GetServerProperty( hGameServer, szKey, szString, ARRAY_LEN( szString )))
				break;
			playerEntry.m_sName = szString;

			sprintf( szKey, "ping_%d", i );
			if( gameSpyBrowser.GetServerProperty( hGameServer, szKey, szString, ARRAY_LEN( szString )))
				playerEntry.m_nPing = min( atoi( szString ), 65535 );
			else
				playerEntry.m_nPing = 65535;

			serverEntry.m_lstPlayerEntry.push_back( playerEntry );
		}
	}

	return true;
}

// Callback for IGameSpyBrowser::RequestServerList
void CScreenJoin::ServerInfoCallback( IGameSpyBrowser& gameSpyBrowser, IGameSpyBrowser::HGAMESERVER hGameServer, void* pUserData )
{
	CScreenJoin* pScreenJoin = reinterpret_cast< CScreenJoin* >( pUserData );

	// Read the information on the server into the serverEntry.
	ServerEntry serverEntry;
	std::string sPrivateAddress;
	if( !ReadServerEntry( gameSpyBrowser, hGameServer, sPrivateAddress, serverEntry ))
		return;

	// Set whether this is a demo server or a retail server.
	serverEntry.m_bDemoServer = ( &gameSpyBrowser == g_pClientMultiplayerMgr->GetDemoServerBrowser( ));

	// Assume we don't need to add this to our listctrl.
	bool bAddToList = false;

	// Check if this server isn't in our map yet.  
	TServerEntryMap::iterator iter = pScreenJoin->m_cServerMap.find(sPrivateAddress);
	if( iter == pScreenJoin->m_cServerMap.end( ))
	{
		// We'll need to add this to our list.
		bAddToList = true;

		// We need to make an new column control for it.
		serverEntry.m_pColumnCtrl = pScreenJoin->CreateColumnCtrl(CMD_DETAILS, IDS_HELP_JOIN);

		// Add the server to the map.  We use insert, because we need to get the iterator to the
		// newly added item.
		typedef std::pair< std::string, ServerEntry > TServerMapInsertValuePair;
		typedef std::pair< TServerEntryMap::iterator, bool > TServerMapInsertResultsPair;
		TServerMapInsertResultsPair mapInsert = pScreenJoin->m_cServerMap.insert( TServerMapInsertValuePair( sPrivateAddress, serverEntry ));
		iter = mapInsert.first;
	}
	else
	{
		// Use the previous column control.
		serverEntry.m_pColumnCtrl = iter->second.m_pColumnCtrl;

		// Copy in the serverentry.
		iter->second = serverEntry;
	}


	// Setup the control with the summary information.  We need to send the string
	// key from the map because column control uses its string data and the local
	// sAddress won't stick around.
	std::string const& sKeyAddress = iter->first;
	SetSummaryInfo( *serverEntry.m_pColumnCtrl, sKeyAddress.c_str( ), serverEntry );

	// Add the columnctrl to our list.
	if( bAddToList )
	{
		CWinSync_CSAuto cs( pScreenJoin->m_csAddServer );

		// Add the control to the control list.
		uint16 nCtrlIndex = pScreenJoin->m_pServerListCtrl->AddControl(serverEntry.m_pColumnCtrl);
	}
}

void CScreenJoin::Term( )
{
	TermServerLists( );

	CBaseScreen::Term( );
}

// ----------------------------------------------------------------------- //
// Function name   : CScreenJoin::TermServerLists
// Description     : Deletes the server list objects.  Good idea
//						to delete them when you are done with them,
//						since they can store lots of data in the server lists.
// ----------------------------------------------------------------------- //
void CScreenJoin::TermServerLists( )
{
	// Clear out our current server list.  We need
	// to check m_bInit here because ScreenMgr
	// could have deleted our controls from under us.
	if( m_bInit && m_pServerListCtrl )
		m_pServerListCtrl->RemoveAll();

	// Clear out the server entry map.
	m_cServerMap.clear( );
}
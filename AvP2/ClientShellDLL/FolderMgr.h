// FolderMgr.h: interface for the CFolderMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FOLDERMGR_H__88EE6E20_1515_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_FOLDERMGR_H__88EE6E20_1515_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef NOPS2  //@*
#include "InterfaceResMgr.h"
#endif  // NOPS2  //*@

#define MAX_FOLDER_HISTORY 20

class CGameClientShell;
class CBaseFolder;

enum eFolderID
{
	FOLDER_ID_NONE,
	FOLDER_ID_MAIN,
	FOLDER_ID_SINGLE,
	FOLDER_ID_ESCAPE,
	FOLDER_ID_OPTIONS,
	FOLDER_ID_PROFILE,
	FOLDER_ID_DISPLAY,
	FOLDER_ID_AUDIO,
	FOLDER_ID_GAME,
	FOLDER_ID_CREDITS,
	FOLDER_ID_PERFORMANCE,
	FOLDER_ID_CUST_CONTROLS,
	FOLDER_ID_JOYSTICK,
	FOLDER_ID_MOUSE,
	FOLDER_ID_KEYBOARD,
	FOLDER_ID_CUSTOM_LEVEL,
	FOLDER_ID_MARINE_MISSIONS,
	FOLDER_ID_PREDATOR_MISSIONS,
	FOLDER_ID_ALIEN_MISSIONS,
	FOLDER_ID_LOAD,
	FOLDER_ID_SAVE,
	FOLDER_ID_MULTI,
	FOLDER_ID_PLAYER,
	FOLDER_ID_PLAYER_JOIN,
	FOLDER_ID_JOIN,
	FOLDER_ID_JOIN_INFO,
	FOLDER_ID_HOST,
	FOLDER_ID_HOST_SETUP_DM,
	FOLDER_ID_HOST_SETUP_TEAMDM,
	FOLDER_ID_HOST_SETUP_HUNT,
	FOLDER_ID_HOST_SETUP_SURVIVOR,
	FOLDER_ID_HOST_SETUP_OVERRUN,
	FOLDER_ID_HOST_SETUP_EVAC,
	FOLDER_ID_HOST_OPTS,
	FOLDER_ID_HOST_MAPS,
	FOLDER_ID_HOST_CONFIG,
	FOLDER_ID_LOADSCREEN_ALIEN,
	FOLDER_ID_LOADSCREEN_MARINE,
	FOLDER_ID_LOADSCREEN_PREDATOR,
	FOLDER_ID_LOADSCREEN_MULTI,

	//this must be the last id
	FOLDER_ID_UNASSIGNED,
};



class CFolderMgr
{
public:
	CFolderMgr();
	virtual ~CFolderMgr();
    LTBOOL               Init();
	void				Term();

	void				HandleKeyDown (int vkey, int rep);
	void				HandleKeyUp (int vkey);
	void				HandleChar (char c);

	// Mouse messages
	void				OnLButtonDown(int x, int y);
	void				OnLButtonUp(int x, int y);
	void				OnLButtonDblClick(int x, int y);
	void				OnRButtonDown(int x, int y);
	void				OnRButtonUp(int x, int y);
	void				OnRButtonDblClick(int x, int y);
	void				OnMouseMove(int x, int y);
	void				OnMouseWheel(int x, int y, int delta);

    LTBOOL               ForceFolderUpdate(eFolderID folderID);

	void				OnEnterWorld();
	void				OnExitWorld();

	eFolderID			GetCurrentFolderID()		{return m_eCurrentFolderID;}
	eFolderID			GetLastFolderID()			{return m_eLastFolderID;}
    LTBOOL              SetCurrentFolder(eFolderID folderID);
    LTBOOL              PreviousFolder();
	void				EscapeCurrentFolder();
	void				ExitFolders();

	// Renders the folder to a surface
    LTBOOL               Render(HSURFACE hDestSurf);
	void				UpdateInterfaceSFX();

	CBaseFolder*		GetCurrentFolder()			{return m_pCurrentFolder;}
	CBaseFolder*		GetFolderFromID(eFolderID folderID);

	void				ClearFolderHistory();
	void				AddToFolderHistory(eFolderID nFolder);

	// GameSpy Arcade function shit!
	void				SetGSA(LTBOOL b)			{ m_bGSA = b; }
	void				SetGSA_IP(const char *sz)	{ strncpy(m_szGSA_IP, sz, 64); }
	void				SetGSA_Port(const char *sz)	{ strncpy(m_szGSA_Port, sz, 64); }
	void				SetGSA_PW(const char *sz)	{ strncpy(m_szGSA_PW, sz, 64); }
	void				SetGSA_Name(const char *sz)	{ strncpy(m_szGSA_Name, sz, 64); }

	LTBOOL				GetGSA()					{ return m_bGSA; }
	const char*			GetGSA_IP()					{ return m_szGSA_IP; }
	const char*			GetGSA_Port()				{ return m_szGSA_Port; }
	const char*			GetGSA_PW()					{ return m_szGSA_PW; }
	const char*			GetGSA_Name()				{ return m_szGSA_Name; }


private:

	void				AddFolder(eFolderID folderID);

private:

	int				m_nHistoryLen;
	eFolderID		m_eFolderHistory[MAX_FOLDER_HISTORY];
	eFolderID		m_eCurrentFolderID;
	eFolderID		m_eLastFolderID;
	CBaseFolder*	m_pCurrentFolder;		// The current folder
	void			SwitchToFolder(CBaseFolder *pNewFolder, LTBOOL bBack = LTFALSE);

	//folders
	CMoArray<CBaseFolder *>	m_folderArray;			// Pointer to each folder


	// GameSpy Arcade Shit!
	LTBOOL			m_bGSA;
	char			m_szGSA_IP[64];
	char			m_szGSA_Port[64];
	char			m_szGSA_PW[64];
	char			m_szGSA_Name[64];
};

#endif // !defined(AFX_FOLDERMGR_H__88EE6E20_1515_11D3_B2DB_006097097C7B__INCLUDED_)
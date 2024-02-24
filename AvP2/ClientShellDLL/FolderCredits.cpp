// FolderCredits.cpp: implementation of the CFolderCredits class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderCredits.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
#include "GameSettings.h"
#include "ProfileMgr.h"


namespace
{
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderCredits::CFolderCredits()
{
}

//////////////////////////////////////////////////////////////////////

CFolderCredits::~CFolderCredits()
{

}

//////////////////////////////////////////////////////////////////////
// Build the folder

LTBOOL CFolderCredits::Build()
{
//	CreateTitle(IDS_TITLE_CREDITS);
	UseBack(LTTRUE);

	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////

void CFolderCredits::OnFocus(LTBOOL bFocus)
{
	CBaseFolder::OnFocus(bFocus);
	if (bFocus)
		g_pInterfaceMgr->GetCredits()->Init(CM_CREDITS,LTFALSE,LTTRUE);
	else
		g_pInterfaceMgr->GetCredits()->Term();
}


LTBOOL CFolderCredits::Render(HSURFACE hDestSurf)
{
	g_pInterfaceMgr->GetCredits()->Update();
	return CBaseFolder::Render(hDestSurf);

}
//------------------------------------------------------------------------
//
// MODULE  : MessageMgr.cpp
//
// PURPOSE : Messaging system - ruthlessly stolen from Greg Kettell (B2)
//
// CREATED : 10/22/97
//
// REVISED : 10/27/99 - jrg
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
//------------------------------------------------------------------------

#include "stdafx.h"
#include "MessageMgr.h"
#include "VKDefs.h"
#include "stdio.h"
#include "MsgIDs.h"
#include "SoundTypes.h"
#include "ClientRes.h"
#include "InterfaceMgr.h"
#include "VarTrack.h"
#include "ClientSoundMgr.h"
#include "GameClientShell.h"
#include "MultiplayerClientMgr.h"
#include "ClientButeMgr.h"
#include "CheatMgr.h"

CMessageMgr*	g_pMessageMgr	= LTNULL;

VarTrack		g_vtMsgMgrXEdge;
VarTrack		g_vtMsgMgrYEdge;
VarTrack		g_vtMsgMgrLineSpace;
VarTrack		g_vtMsgMgrMaxHeight;
VarTrack		g_vtMsgMgrMinTime;
VarTrack		g_vtMsgMgrMaxTime;
VarTrack		g_vtMsgMgrTimeScale;
VarTrack		g_vtMsgMgrHistory;

VarTrack		g_vtMsgMgrDisplay;

VarTrack		g_vtInputLineSound;


#define MSGMGR_FONT_SIZE_X		6
#define MSGMGR_FONT_SIZE_Y		12


/*******************************************************************************

	CInputLine

*******************************************************************************/


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::Init()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

LTBOOL CInputLine::Init()
{
	// Create the font
	LITHFONTCREATESTRUCT lfcs;
	lfcs.szFontName = "Arial";
	lfcs.nWidth = MSGMGR_FONT_SIZE_X;
	lfcs.nHeight = MSGMGR_FONT_SIZE_Y;

	m_pFont = CreateLithFont(g_pLTClient, &lfcs, LTTRUE);

	g_vtInputLineSound.Init(g_pLTClient, "InputLineSound", LTNULL, 1.0f);

	// Create a transparent color
	HLTCOLOR hTransColor = SETRGB_T(0, 0, 0);

	// Create the background
	m_hBackground = g_pLTClient->CreateSurfaceFromBitmap("Interface\\MP\\msgbackground.pcx");
	g_pLTClient->OptimizeSurface(m_hBackground, hTransColor);
	g_pLTClient->SetSurfaceAlpha(m_hBackground, 0.75f);

	// Create the strings
	m_szMessage = g_pLTClient->FormatString(IDS_MP_MESSAGE);
	m_szTeamMessage = g_pLTClient->FormatString(IDS_MP_TEAM_MESSAGE);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::Term()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CInputLine::Term()
{
	Done();

	// Free the background surface if needed
	if(m_hBackground)
	{
		g_pLTClient->DeleteSurface(m_hBackground);
		m_hBackground = LTNULL;
	}

	// Free the strings
	if(m_szMessage)
	{
		g_pLTClient->FreeString(m_szMessage);
		m_szMessage = LTNULL;
	}

	if(m_szTeamMessage)
	{
		g_pLTClient->FreeString(m_szTeamMessage);
		m_szTeamMessage = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::PlaySound()
//
//	PURPOSE:	Play an input line sound
//
// ----------------------------------------------------------------------- //

void CInputLine::PlaySound(char *szSound)
{
	if(g_vtInputLineSound.GetFloat())
	{
		g_pClientSoundMgr->PlaySoundLocal(szSound, SOUNDPRIORITY_MISC_MEDIUM);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::Clear()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CInputLine::Clear()
{
	m_zText[0]	= '\0';		// nil the Message
	m_nTextLen	= 0;		// zero the length
	m_bShift	= LTFALSE;	// make sure we start out with this off...
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::Done()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CInputLine::Done()
{
	Clear();

	g_pMessageMgr->SetEditing(LTFALSE);
	g_pLTClient->SetInputState(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::Draw()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

void CInputLine::Draw()
{
	// Make sure the vitals are ok...
	if(!m_pFont) return;

	// Get information about the screen
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	uint32 nWidth, nHeight;

	g_pLTClient->GetSurfaceDims(hScreen, &nWidth, &nHeight);


	// Draw the background
	int x = nWidth / 2;
	int y = nHeight - ((m_pFont->Height() * 3) + ((int)g_vtMsgMgrYEdge.GetFloat() * 2) + (int)g_vtMsgMgrLineSpace.GetFloat());
	g_pLTClient->ScaleSurfaceToSurface(hScreen, m_hBackground, &LTRect(0, y, nWidth, nHeight), LTNULL);


	// Create drawdata structure and fill it in
	LITHFONTDRAWDATA lfDD;
	lfDD.dwFlags		= LTF_DRAW_NORMAL | LTF_DRAW_SOLID;
	lfDD.byJustify		= LTF_JUSTIFY_CENTER;
	lfDD.hColor			= SETRGB(255, 255, 255);
	lfDD.bForceFormat	= LTTRUE; // Always reformat to avoid bugs..


	// Draw the type of message
	if(g_pMessageMgr->GetEditType() == EDIT_TYPE_TEAM)
		m_pFont->Draw(hScreen, m_szTeamMessage, &lfDD, x, y + (int)g_vtMsgMgrYEdge.GetFloat());
	else
		m_pFont->Draw(hScreen, m_szMessage, &lfDD, x, y + (int)g_vtMsgMgrYEdge.GetFloat());


	// Add a flashing cursor to the end of the string
	char zTemp[INPUTLINE_MAX_LINE_SIZE + 2 + 4];
	strcpy(zTemp, m_zText);

	double fTime = g_pLTClient->GetTime();
	double tmp;

	fTime = modf(fTime, &tmp);
	strcat(zTemp, (fTime > 0.5f) ? "_" : " ");


	// Draw the input string
	y += m_pFont->Height() + (int)g_vtMsgMgrYEdge.GetFloat() + (int)g_vtMsgMgrLineSpace.GetFloat();


	// Update drawdata structure
	lfDD.dwFlags		= LTF_DRAW_NORMAL | LTF_DRAW_FORMATTED | LTF_DRAW_SOLID;
	lfDD.dwFormatWidth	= nWidth - ((int)g_vtMsgMgrXEdge.GetFloat() * 2);
	lfDD.bForceFormat	= LTTRUE; // Always reformat to avoid bugs..

	m_pFont->Draw(hScreen, zTemp, &lfDD, x, y);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::CInputLine()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //
			
LTBOOL CInputLine::AddChar(DBYTE ch)
{
	if(!m_nTextLen && (ch == ' ')) return LTFALSE;

	if(m_nTextLen < INPUTLINE_MAX_LINE_SIZE)	// space left in the Message
	{
		m_zText[m_nTextLen] = ch;	// add a character
		m_nTextLen++;				// increment the length
		m_zText[m_nTextLen] = '\0';	// terminate the string


		int nRand = GetRandom(1,3);
		switch(nRand)
		{
			case 1: PlaySound("Sounds\\Interface\\chat_type1.wav"); break;
			case 2: PlaySound("Sounds\\Interface\\chat_type2.wav"); break;
			case 3: PlaySound("Sounds\\Interface\\chat_type3.wav"); break;
		}
		
		return LTTRUE;				// indicate success
	}

	return LTFALSE;	// indicate failure
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::CInputLine()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //
			
void CInputLine::DelChar()
{
	if (m_nTextLen > 0)	// text in the Message?
	{
		m_nTextLen--;				// back up a character
		m_zText[m_nTextLen] = '\0';	// terminate the string

		PlaySound("Sounds\\Interface\\chat_typeback.wav");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::CInputLine()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //
			
void CInputLine::Set(char *pzText)
{
	strncpy(m_zText, pzText, INPUTLINE_MAX_LINE_SIZE);		// copy the text
	m_zText[INPUTLINE_MAX_LINE_SIZE] = '\0';				// enforce null termination
	m_nTextLen = strlen(pzText);							// enforce max length
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::CInputLine()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //
			
void CInputLine::Send()
{
	if(!g_pLTClient) return;

	// If our text is empty... make sure not to send anything...
	if(!m_zText[0])
	{
		g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\error.wav", SOUNDPRIORITY_MISC_MEDIUM);
		Done();
		return;
	}


	// Send the Message to the server
	if(g_pMessageMgr->GetEditType() == EDIT_TYPE_TEAM)
	{
		HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_MPMGR_TEAM_MESSAGE);
		g_pLTClient->WriteToMessageString(hMsg, m_zText);
		g_pLTClient->EndMessage(hMsg);
	}
	else
	{
		// Do a private message if we start with a '<' character and contain a '>' character after it
		if((m_zText[0] == '<') && (strstr(m_zText, ">") > m_zText))
		{
			// Place some markers in the text buffer
			char *szName = &m_zText[1];
			char *szMsg = m_zText;

			// Find the first closing bracket
			while(*szMsg != '>')
				szMsg++;

			*szMsg = '\0';
			szMsg++;

			// Get rid of leading spaces
			while(*szMsg == ' ')
				szMsg++;

			// If our text is empty... make sure not to send anything...
			if(!szMsg[0])
			{
				g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\error.wav", SOUNDPRIORITY_MISC_MEDIUM);
				Done();
				return;
			}


			if(!_stricmp(szName, "cheat"))
			{
				ConParse parse;
				parse.Init(szMsg);

				if(g_pLTClient->Common()->Parse(&parse) == LT_OK)
				{
					g_pCheatMgr->HandleCheat(parse.m_nArgs, parse.m_Args);
				}
			}
			else if(!_stricmp(szName, "name"))
			{
				HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_MPMGR_NAME_CHANGE);
				g_pLTClient->WriteToMessageString(hMsg, szMsg);
				g_pLTClient->EndMessage(hMsg);
			}
			else if((!_stricmp(szName, "serv") || !_stricmp(szName, "server")) && g_pGameClientShell->IsMultiplayerGame())
			{
				HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_MPMGR_SERVER_COMMAND);
				g_pLTClient->WriteToMessageString(hMsg, szMsg);
				g_pLTClient->EndMessage(hMsg);
			}
			else
			{
				uint32 nID = g_pGameClientShell->GetMultiplayerMgr()->FindClientID(szName);

				if(nID != 0)
				{
					HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_MPMGR_PRIVATE_MESSAGE);
					g_pLTClient->WriteToMessageDWord(hMsg, nID);
					g_pLTClient->WriteToMessageString(hMsg, szMsg);
					g_pLTClient->EndMessage(hMsg);
				}
			}
		}
		else
		{
			HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_MPMGR_MESSAGE);
			g_pLTClient->WriteToMessageString(hMsg, m_zText);
			g_pLTClient->EndMessage(hMsg);
		}
	}

	Done();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::CInputLine()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //
			
LTBOOL CInputLine::HandleKeyDown(int key, int rep)
{
	if( key == g_pClientButeMgr->GetEscapeKey() )
	{
		key = VK_ESCAPE;
	}

	switch( key )
	{
		case VK_ESCAPE:
		{
			Done();
		}
		break;

		case VK_HOME:
		{
			Clear();
		}
		break;

		case VK_BACK:
		{
			DelChar();
		}
		break;

		case VK_SHIFT:
		{
			m_bShift = LTTRUE;
		}
		break;

		case VK_UP:
		{
			if (m_nCurrentRecentLine == m_nBaseRecentLine + 1 ||
				(m_nCurrentRecentLine == 0 && m_nBaseRecentLine == INPUTLINE_NUM_RECENT_LINES - 1))
			{
				break;
			}
			
			int nTest = 0;
			if (m_nCurrentRecentLine == -1)
			{
				nTest = m_nBaseRecentLine;
			}
			else
			{
				nTest = m_nCurrentRecentLine - 1;
			}
			if (nTest < 0) nTest = INPUTLINE_NUM_RECENT_LINES - 1;
			
			if (!m_bRecentLineUsed[nTest]) break;
			m_nCurrentRecentLine = nTest;
			SAFE_STRCPY(m_zText, m_pRecentLines[m_nCurrentRecentLine]);
		}
		break;

		case VK_DOWN:
		{
			if (m_nCurrentRecentLine == m_nBaseRecentLine || m_nCurrentRecentLine == -1)
			{
				break;
			}

			m_nCurrentRecentLine++;
			if (m_nCurrentRecentLine == INPUTLINE_NUM_RECENT_LINES) m_nCurrentRecentLine = 0;
			SAFE_STRCPY(m_zText, m_pRecentLines[m_nCurrentRecentLine]);
		}
		break;

		case VK_RETURN:
		{
			AddToRecentList();
			Send();
			g_pClientDE->ClearInput(); // Don't process the key they hit...
		}
		break;

		default:
		{
			char ch;
			if (ch = AsciiXlate(key))
			{
				if (!AddChar(ch))
				{
				}
			}
		}
		break;
	}
	return LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::HandleKeyUp()
//
//	PURPOSE:	Handles a key up Message, used for tracking shift keys
//
// ----------------------------------------------------------------------- //
			
void CInputLine::HandleKeyUp(int key)
{
	switch( key )
	{
		case VK_SHIFT:
			m_bShift = LTFALSE;
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::AddToRecentList()
//
//	PURPOSE:	Adds the current string to the recent-string list
//
// ----------------------------------------------------------------------- //
			
void CInputLine::AddToRecentList()
{
	if(!m_zText[0]) return;

	// add the current line to the array of past input lines

	m_nBaseRecentLine++;
	if (m_nBaseRecentLine >= INPUTLINE_NUM_RECENT_LINES) 
		m_nBaseRecentLine = 0;
	else if(m_nBaseRecentLine < 0)
		m_nBaseRecentLine = 0;
	
	SAFE_STRCPY(m_pRecentLines[m_nBaseRecentLine], m_zText);
	m_bRecentLineUsed[m_nBaseRecentLine] = LTTRUE;
	m_nCurrentRecentLine = -1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::AsciiXlate()
//
//	PURPOSE:	Translates a VK_ code into something viewable.
//
// ----------------------------------------------------------------------- //
			
char CInputLine::AsciiXlate(int key)
{
	char ch = 0;
	char *zNumShift = ")!@#$%^&*(";

	// Check for a letter
	if (key >= VK_A && key <= VK_Z)
	{
		ch = m_bShift ? key : key - 'A' + 'a';
	}
	// Check for a number
	else if (key >= VK_0 && key <= VK_9)
	{
		ch = m_bShift ? zNumShift[key - VK_0] : key;
	}
	// Now check for the remaining usable keys
	else
	{
		switch(key)
		{
			case VK_SPACE: ch = ' '; break;
			case 189: ch = m_bShift ? '_' : '-'; break;
			case 187: ch = m_bShift ? '+' : '='; break;
			case 219: ch = m_bShift ? '{' : '['; break;
			case 221: ch = m_bShift ? '}' : ']'; break;
			case 220: ch = m_bShift ? '|' : '\\'; break;
			case 186: ch = m_bShift ? ':' : ';'; break;
			case 222: ch = m_bShift ? '"' : '\''; break;
			case 188: ch = m_bShift ? '<' : ','; break;
			case 190: ch = m_bShift ? '>' : '.'; break;
			case 191: ch = m_bShift ? '?' : '/'; break;
		}
	}
	return ch;
}



/*******************************************************************************

	CMessageMgr

*******************************************************************************/


LITHFONTDRAWDATA g_lfDD1;
LITHFONTDRAWDATA g_lfDD2;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::CMessageMgr()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //
			
CMessageMgr::CMessageMgr()
{
	g_pMessageMgr = this;

	m_pFont = LTNULL;

	m_hBackground = LTNULL;

	m_bEnabled = LTFALSE;
	m_bEditing = LTFALSE;

	m_fRemoveLastTime = 0.0f;

	m_nFirstMessage = 0;
	m_nMessages = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::Init()
//
//	PURPOSE:	Initializes the Message manager
//
// ----------------------------------------------------------------------- //

LTBOOL CMessageMgr::Init()
{
	if(!m_InputLine.Init()) return LTFALSE;

	// Create the font
	LITHFONTCREATESTRUCT lfcs;
	lfcs.szFontName = "Arial";
	lfcs.nWidth = MSGMGR_FONT_SIZE_X;
	lfcs.nHeight = MSGMGR_FONT_SIZE_Y;

	m_pFont = CreateLithFont(g_pLTClient, &lfcs, LTTRUE);


	g_vtMsgMgrXEdge.Init(g_pLTClient, "MsgMgrXEdge", LTNULL, 75.0f);
	g_vtMsgMgrYEdge.Init(g_pLTClient, "MsgMgrYEdge", LTNULL, 10.0f);
	g_vtMsgMgrLineSpace.Init(g_pLTClient, "MsgMgrLineSpace", LTNULL, 3.0f);
	g_vtMsgMgrMaxHeight.Init(g_pLTClient, "MsgMgrMaxHeight", LTNULL, 0.25f);
	g_vtMsgMgrMinTime.Init(g_pLTClient, "MsgMgrMinTime", LTNULL, 4.0f);
	g_vtMsgMgrMaxTime.Init(g_pLTClient, "MsgMgrMaxTime", LTNULL, 15.0f);
	g_vtMsgMgrTimeScale.Init(g_pLTClient, "MsgMgrTimeScale", LTNULL, 1.0f);
	g_vtMsgMgrHistory.Init(g_pLTClient, "MsgMgrHistory", LTNULL, 10.0f);

	g_vtMsgMgrDisplay.Init(g_pLTClient, "MsgMgrDisplay", LTNULL, 1.0f);

	// Create a transparent color
	HLTCOLOR hTransColor = SETRGB_T(0, 0, 0);

	// Create the background
	m_hBackground = g_pLTClient->CreateSurfaceFromBitmap("Interface\\StatusBar\\Marine\\ObjectiveBackground.pcx");
	g_pLTClient->OptimizeSurface(m_hBackground, hTransColor);
	g_pLTClient->SetSurfaceAlpha(m_hBackground, 0.6f);


	// Create drawdata structure and fill it in
	g_lfDD1.dwFlags			= LTF_DRAW_SOLID;
	g_lfDD1.byJustify		= LTF_JUSTIFY_LEFT;

	g_lfDD2.dwFlags			= LTF_DRAW_SOLID | LTF_DRAW_FORMATTED;
	g_lfDD2.byJustify		= LTF_JUSTIFY_LEFT;


	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::Term()
//
//	PURPOSE:	Free up everything
//
// ----------------------------------------------------------------------- //

void CMessageMgr::Term()
{
	// Clear the messages
	Clear();


	// Free the background surface if needed
	if(m_hBackground)
	{
		g_pLTClient->DeleteSurface(m_hBackground);
		m_hBackground = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::SetEnabled()
//
//	PURPOSE:	Sets the enabled flag
//
// ----------------------------------------------------------------------- //

void CMessageMgr::SetEnabled(LTBOOL bEnabled)
{
	if(m_bEnabled && !bEnabled)
		Clear();

	m_bEnabled = bEnabled;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::SetEditing()
//
//	PURPOSE:	Sets the enabled flag
//
// ----------------------------------------------------------------------- //

LTBOOL CMessageMgr::SetEditing(LTBOOL bEditing, uint8 nType)
{
	// If we tried to chat using the team messages and we're not a team game type... then
	// just play a sound and post a message to let the player know it's not available.
	if(bEditing && nType == EDIT_TYPE_TEAM && !g_pGameClientShell->GetGameType()->IsTeamMode())
	{
		HSTRING hStr = g_pLTClient->FormatString(IDS_MP_NOT_TEAM_GAME);
		AddMessage(g_pLTClient->GetStringData(hStr));
		g_pLTClient->FreeString(hStr);

		g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\error.wav", SOUNDPRIORITY_MISC_MEDIUM);
		return LTFALSE;
	}

	if(!m_bEditing && bEditing)
	{
		g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\chat_open.wav", SOUNDPRIORITY_MISC_MEDIUM);
	}

	m_bEditing = bEditing;
	m_nEditType = nType;
	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::AddMessage()
//
//	PURPOSE:	Adds a new line to the Message buffer
//
// ----------------------------------------------------------------------- //

void CMessageMgr::AddMessage(char *szMsg, HLTCOLOR clrMsg, char *szFrom, HLTCOLOR clrFrom)
{
	// If the message doesn't exist... just boot out
	if(!szMsg) return;


	// If our message list is full... go ahead and remove one
	if(m_nMessages == MSGMGR_MAX_MESSAGES)
		RemoveMessage();


	// Find the index of the last message slot...
	int nIndex = m_nFirstMessage + m_nMessages;

	if(nIndex >= MSGMGR_MAX_MESSAGES)
		nIndex -= MSGMGR_MAX_MESSAGES;


	// Now fill in the message for the last slot
	strcpy(m_Messages[nIndex].szMsg, szMsg);
	m_Messages[nIndex].clrMsg = clrMsg;

	if(szFrom)
		sprintf(m_Messages[nIndex].szFrom, "%s:  ", szFrom);
	else
		m_Messages[nIndex].szFrom[0] = '\0';

	m_Messages[nIndex].clrFrom = clrFrom;


	// Set the time that we recieved this message
	m_Messages[nIndex].fTime = g_pLTClient->GetTime();


	// Increment our message count
	m_nMessages++;


	// Get information about the screen
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	uint32 nWidth, nHeight;

	g_pLTClient->GetSurfaceDims(hScreen, &nWidth, &nHeight);

	// Now get the extents of each of these strings...
	uint32 extX, extY;
	int nFormatWidth = (int)nWidth - ((int)g_vtMsgMgrXEdge.GetFloat() * 2);

	if(m_Messages[nIndex].szFrom[0])
	{
		m_pFont->GetTextExtents(m_Messages[nIndex].szFrom, &g_lfDD1, extX, extY);
		m_Messages[nIndex].extFrom.x = (int)extX;
		m_Messages[nIndex].extFrom.y = (int)extY;
	}
	else
	{
		m_Messages[nIndex].extFrom = LTIntPt(0, 0);
	}

	g_lfDD2.dwFormatWidth	= nFormatWidth - m_Messages[nIndex].extFrom.x;
	m_pFont->GetTextExtents(m_Messages[nIndex].szMsg, &g_lfDD2, extX, extY);
	m_Messages[nIndex].extMsg.x = (int)extX;
	m_Messages[nIndex].extMsg.y = (int)extY;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::RemoveMessage()
//
//	PURPOSE:	Removes the oldest message from the list
//
// ----------------------------------------------------------------------- //

void CMessageMgr::RemoveMessage()
{
	// If there are no lines... just return
	if(m_nMessages <= 0) return;


	// Increment the first message...
	m_nFirstMessage++;

	if(m_nFirstMessage >= MSGMGR_MAX_MESSAGES)
		m_nFirstMessage -= MSGMGR_MAX_MESSAGES;


	// Reduce our message count
	m_nMessages--;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::Clear()
//
//	PURPOSE:	Removes all Messages from the array
//
// ----------------------------------------------------------------------- //

void CMessageMgr::Clear()
{
	memset(&m_Messages, 0, sizeof(Message) * MSGMGR_MAX_MESSAGES);
	m_nMessages = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::Draw()
//
//	PURPOSE:	Displays the Message list
//
// ----------------------------------------------------------------------- //

void CMessageMgr::Draw()
{
	// Make sure the vitals are ok...
	if(!m_pFont) return;

	// Get information about the screen
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	uint32 nWidth, nHeight;

	g_pLTClient->GetSurfaceDims(hScreen, &nWidth, &nHeight);


	int nFirst = m_nFirstMessage;
	int nMessages = m_nMessages;

	if(m_bEditing)
	{
		nFirst = m_nFirstMessage + m_nMessages + (MSGMGR_MAX_MESSAGES - (int)g_vtMsgMgrHistory.GetFloat());

		if(nFirst >= MSGMGR_MAX_MESSAGES)
			nFirst -= MSGMGR_MAX_MESSAGES;

		nMessages = (int)g_vtMsgMgrHistory.GetFloat();
	}


	// ----------------------------------------------------------------------- //
	// See if we are allowed to draw the message list...
	LTBOOL bDrawMessages = LTTRUE;

	CWeaponChooser*	pChooser = g_pGameClientShell->GetInterfaceMgr()->GetWeaponChooser();

	if(pChooser && pChooser->IsOpen())
		bDrawMessages = LTFALSE;


	// ----------------------------------------------------------------------- //
	// Now draw the message list
	if(bDrawMessages)
	{
		if(m_bEditing || (m_bEnabled && (nMessages > 0)))
		{
			int nTotalHeight = 0;


			// Go through each message and calculate the extents
			for(int i = 0; i < nMessages; i++)
			{
				int nIndex = nFirst + i;

				if(nIndex >= MSGMGR_MAX_MESSAGES)
					nIndex -= MSGMGR_MAX_MESSAGES;


				if(m_Messages[nIndex].szMsg[0])
				{
					nTotalHeight += m_Messages[nIndex].extMsg.y;
					nTotalHeight += (int)g_vtMsgMgrLineSpace.GetFloat();
				}
			}


			if(nTotalHeight > 0)
			{
				nTotalHeight += (int)g_vtMsgMgrYEdge.GetFloat() * 2;
				nTotalHeight -= (int)g_vtMsgMgrLineSpace.GetFloat();

				// Draw the background
				g_pLTClient->ScaleSurfaceToSurface(hScreen, m_hBackground, &LTRect(0, 0, nWidth, nTotalHeight), LTNULL);


				// Now draw all the strings on top of the background
				int x, y = (int)g_vtMsgMgrYEdge.GetFloat();

				for(i = 0; i < nMessages; i++)
				{
					int nIndex = nFirst + i;

					if(nIndex >= MSGMGR_MAX_MESSAGES)
						nIndex -= MSGMGR_MAX_MESSAGES;


					if(m_Messages[nIndex].szMsg[0])
					{
						// ***** Keep this calculation outside of the following if statement *****
						x = ((int)nWidth - (m_Messages[nIndex].extFrom.x + m_Messages[nIndex].extMsg.x)) / 2;

						// Draw who sent the message... or nothing
						if(m_Messages[nIndex].szFrom[0])
						{
							g_lfDD1.hColor = m_Messages[nIndex].clrFrom;

							if(g_vtMsgMgrDisplay.GetFloat())
								m_pFont->Draw(hScreen, m_Messages[nIndex].szFrom, &g_lfDD1, x, y, &m_Messages[nIndex].sdFrom);
						}

						g_lfDD2.dwFormatWidth = nWidth - m_Messages[nIndex].extFrom.x - ((int)g_vtMsgMgrXEdge.GetFloat() * 2);
						g_lfDD2.hColor = m_Messages[nIndex].clrMsg;

						// Draw the message itself
						x += m_Messages[nIndex].extFrom.x;// + (m_Messages[nIndex].extMsg.x / 2);

						if(g_vtMsgMgrDisplay.GetFloat())
							m_pFont->Draw(hScreen, m_Messages[nIndex].szMsg, &g_lfDD2, x, y, &m_Messages[nIndex].sdMsg);


						y += m_Messages[nIndex].extMsg.y + (int)g_vtMsgMgrLineSpace.GetFloat();
					}
				}


				if(!m_bEditing)
				{
					// If the total height was larger then what we want to display... cut off a line for the next update
					if((nTotalHeight > (int)(g_vtMsgMgrMaxHeight.GetFloat() * (LTFLOAT)nHeight)) && (nMessages > 1))
						RemoveMessage();


					// Handle the time delay for removing messages
					if(g_pLTClient->GetTime() > m_fRemoveLastTime)
					{
						if(m_fRemoveLastTime)
							RemoveMessage();

						// Calculate a delay
						LTFLOAT fDelay = (LTFLOAT)strlen(m_Messages[m_nFirstMessage].szMsg) / (LTFLOAT)INPUTLINE_MAX_LINE_SIZE * g_vtMsgMgrMaxTime.GetFloat();

						if(fDelay < g_vtMsgMgrMinTime.GetFloat())
							fDelay = g_vtMsgMgrMinTime.GetFloat();

						fDelay *= g_vtMsgMgrTimeScale.GetFloat();

						m_fRemoveLastTime = g_pLTClient->GetTime() + fDelay;
					}
				}
			}
		}
		else
		{
			m_fRemoveLastTime = 0.0f;
		}
	}


	// ----------------------------------------------------------------------- //
	// Draw the input line if we need to
	if(m_bEditing)
	{
		if(g_vtMsgMgrDisplay.GetFloat())
			m_InputLine.Draw();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::HandleKeyDown()
//
//	PURPOSE:	Processes keystrokes
//
// ----------------------------------------------------------------------- //

LTBOOL CMessageMgr::HandleKeyDown(int key, int rep)
{
	if(m_bEditing)
		return m_InputLine.HandleKeyDown(key, rep);
	else
		return LTFALSE;
}



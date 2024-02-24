//-------------------------------------------------------------------------
//
// MODULE  : MessageMgr.h
//
// PURPOSE : Messaging system - ruthlessly stolen from Greg Kettell (B2)
//
// CREATED : 10/22/97
//
// REVISED : 10/27/99 - jrg
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------

#ifndef __MESSAGE_MGR_H__
#define __MESSAGE_MGR_H__

//-------------------------------------------------------------------------

#include "cpp_clientshell_de.h"
#include "client_de.h"
#include "LTGUIMgr.h"

//-------------------------------------------------------------------------

#define	INPUTLINE_NUM_RECENT_LINES			16
#define INPUTLINE_MAX_LINE_SIZE				128

#define MSGMGR_MAX_MESSAGES					16

#define EDIT_TYPE_NORMAL					0
#define EDIT_TYPE_TEAM						1

//-------------------------------------------------------------------------

class CGameClientShell;

//-------------------------------------------------------------------------
// Input line class

class CInputLine
{
	public:

		CInputLine() 
			: m_pFont(LTNULL),
			  m_hBackground(LTNULL),

			  m_bShift(LTFALSE),

			  m_nTextLen(0),
			  // m_zText

			  // m_pRecentLines
			  // m_bRecentLineUsed

			  m_nBaseRecentLine(0),
			  m_nCurrentRecentLine(-1),

			  m_szMessage(LTNULL),
			  m_szTeamMessage(LTNULL)
		{ 
			memset(m_zText,0, INPUTLINE_MAX_LINE_SIZE + 2);
			memset(m_pRecentLines, 0, INPUTLINE_NUM_RECENT_LINES * (INPUTLINE_MAX_LINE_SIZE + 2));
			memset(m_bRecentLineUsed, 0, INPUTLINE_NUM_RECENT_LINES * sizeof (LTBOOL));
		}

		~CInputLine()
		{
			Term();
		}

		LTBOOL		Init();
		void		Term();

		void		PlaySound(char *szSound);

		void		Clear();
		void		Done();

		void		Draw();

		LTBOOL		AddChar(DBYTE ch);
		void		DelChar();

		void		Set(char *pzText);
		void		Send();

		LTBOOL		HandleKeyDown(int key, int rep);
		void		HandleKeyUp(int key);

	private:

		char				AsciiXlate(int key);	// Translates VK_ values to ascii
		void				AddToRecentList();		// Adds current string to recent-string list

	private:

		LithFont	*m_pFont;							// The font to draw with
		HSURFACE	m_hBackground;					// The background surface

		LTBOOL		m_bShift;						// Shift is active

		int			m_nTextLen;						// Current length of input string
		char		m_zText[INPUTLINE_MAX_LINE_SIZE + 2]; // The buffer

		char		m_pRecentLines[INPUTLINE_NUM_RECENT_LINES][INPUTLINE_MAX_LINE_SIZE + 2];	// recent lines (for doskey effect)
		LTBOOL		m_bRecentLineUsed[INPUTLINE_NUM_RECENT_LINES];								// has this recent line been used?

		int			m_nBaseRecentLine;				// Current most-recent-line
		int			m_nCurrentRecentLine;			// Selected recent line

		HSTRING		m_szMessage;
		HSTRING		m_szTeamMessage;
};

//-------------------------------------------------------------------------
// Message structure

struct Message
{
	Message()	{ Clear(); }

	void Clear()
	{
		szFrom[0] = '\0';
		szMsg[0] = '\0';

		clrFrom = SETRGB(255, 255, 255);
		clrMsg = SETRGB(255, 255, 255);

		fTime = 0.0f;
	}

	char			szFrom[INPUTLINE_MAX_LINE_SIZE + 2];
	char			szMsg[INPUTLINE_MAX_LINE_SIZE + 2];

	HLTCOLOR		clrFrom;
	HLTCOLOR		clrMsg;

	LTIntPt			extFrom;
	LTIntPt			extMsg;

	LTFLOAT			fTime;

	LITHFONTSAVEDATA	sdFrom;
	LITHFONTSAVEDATA	sdMsg;
};

//-------------------------------------------------------------------------
// Message manager class

class CMessageMgr
{
	public:

		CMessageMgr();

		LTBOOL		Init();
		void		Term();

		void		SetEnabled(LTBOOL bEnabled);
		LTBOOL		IsEnabled()							{ return m_bEnabled; }

		LTBOOL		SetEditing(LTBOOL bEditing, uint8 nType = EDIT_TYPE_NORMAL);
		LTBOOL		IsEditing()							{ return m_bEditing; }
		uint8		GetEditType()						{ return m_nEditType; }

		void		AddMessage(char *szMsg, HLTCOLOR clrMsg = SETRGB(255,255,0), char *szFrom = LTNULL, HLTCOLOR clrFrom = SETRGB(255,255,255));
		void		RemoveMessage();

		void		Clear();
		void		Draw();

		LTBOOL		HandleKeyDown(int key, int rep);
		void		HandleKeyUp(int key) { m_InputLine.HandleKeyUp(key); }


	private:

		CInputLine	m_InputLine;						// Current input message
		LithFont	*m_pFont;							// The font to draw with

		HSURFACE	m_hBackground;						// The background surface

		LTBOOL		m_bEnabled;							// Can we use the message manager at all?
		LTBOOL		m_bEditing;							// Are we editing a line of text to send?
		uint8		m_nEditType;						// What type of message are we editing

		LTFLOAT		m_fRemoveLastTime;					// Tells when to remove the oldest message

		int			m_nFirstMessage;					// The first message to render
		int			m_nMessages;						// The current number of messages

		Message		m_Messages[MSGMGR_MAX_MESSAGES];	// The list of displayed messages
};

// ----------------------------------------------------------------------- //

extern CMessageMgr *g_pMessageMgr;

//-------------------------------------------------------------------------

#endif	// __MESSAGE_MGR_H__

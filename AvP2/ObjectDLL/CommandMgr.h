// ----------------------------------------------------------------------- //
//
// MODULE  : CommandMgr.h
//
// PURPOSE : CommandMgr definition
//
// CREATED : 06/23/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __COMMAND_MGR_H__
#define __COMMAND_MGR_H__

class ConParse;
class CCommandMgr;
extern CCommandMgr* g_pCmdMgr;

#define CMDMGR_MAX_PENDING_COMMANDS 100
#define CMDMGR_MAX_COMMAND_LENGTH	256
#define CMDMGR_MAX_ID_LENGTH		16
#define CMDMGR_NULL_CHAR			'\0'

typedef DBOOL (*ProcessCmdFn)(CCommandMgr *pCmdMgr, const ConParse & parse, int nCmdIndex);


// This struct holds all the information pending commands...

struct CMD_STRUCT
{
	CMD_STRUCT()
	{
		Clear();
	}

	void Clear()
	{
		fDelay		= fMinDelay = fMaxDelay = 0.0f;
		nNumTimes	= nMinTimes	= nMaxTimes	= -1;
		aCmd[0]		= CMDMGR_NULL_CHAR;
		aId[0]		= CMDMGR_NULL_CHAR;
	}

	void Load(HMESSAGEREAD hRead);
	void Save(HMESSAGEWRITE hWrite);

	float	fDelay;
	float	fMinDelay;
	float	fMaxDelay;
	int		nNumTimes;
	int		nMinTimes;
	int		nMaxTimes;
	char	aCmd[CMDMGR_MAX_COMMAND_LENGTH];	
	char	aId[CMDMGR_MAX_ID_LENGTH];	
};

// This struct is basically exactly the same as the above struct, except that it
// uses pointers instead of arrays.  The idea is to use this struct to pass
// command data between functions without having to copy buffers around (see
// CCommandMgr::AddDelayedCmd() )

struct CMD_STRUCT_PARAM
{
	CMD_STRUCT_PARAM()
	{
		fDelay		= fMinDelay = fMaxDelay = 0.0f;
		nNumTimes	= nMinTimes	= nMaxTimes	= -1;
		pCmd		= 0;
		pId			= 0;
	}

	float	fDelay;
	float	fMinDelay;
	float	fMaxDelay;
	int		nNumTimes;
	int		nMinTimes;
	int		nMaxTimes;
	char*	pCmd;	
	char*	pId;	
};

struct CMD_PROCESS_STRUCT
{
	CMD_PROCESS_STRUCT(char* pCmd="", int nArgs=0, ProcessCmdFn pFn=DNULL, char* pSyn="")
	{
		pCmdName	= pCmd;
		nNumArgs	= nArgs;
		pProcessFn	= pFn;
		pSyntax		= pSyn;
	}

	char*			pCmdName;
	char*			pSyntax;
	int				nNumArgs;
	ProcessCmdFn	pProcessFn;
};

class CCommandMgr
{
	public : 

		CCommandMgr();
		~CCommandMgr();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		void	ListCommands();
		DBOOL	IsValidCmd(const char* pCmd);
		DBOOL	IsValidCmd(const ConParse & parse);
		DBOOL	Process(const char* pCmd) { return ProcessCmd(pCmd); }
		DBOOL	Process(const ConParse & parse) { return ProcessCmd(parse); }
		DBOOL	Update();

		void    ClearCommands();

		// The following methods should only be called via the static cmdmgr_XXX
		// functions...

		DBOOL	ProcessListCommands(const ConParse & parse, int nCmdIndex);
		DBOOL	ProcessDelay(const ConParse & parse, int nCmdIndex);
		DBOOL	ProcessDelayId(const ConParse & parse, int nCmdIndex);
		DBOOL	ProcessMsg(const ConParse & parse, int nCmdIndex);
		DBOOL	ProcessRand(const ConParse & parse, int nCmdIndex);
		DBOOL	ProcessRandArgs(const ConParse & parse, int nCmdIndex, int nNumArgs);
		DBOOL	ProcessRepeat(const ConParse & parse, int nCmdIndex);
		DBOOL	ProcessRepeatId(const ConParse & parse, int nCmdIndex);
		DBOOL	ProcessLoop(const ConParse & parse, int nCmdIndex);
		DBOOL	ProcessLoopId(const ConParse & parse, int nCmdIndex);
		DBOOL	ProcessAbort(const ConParse & parse, int nCmdIndex);
	
	private :

		DBOOL	ProcessCmd(const char* pCmd, int nCmdIndex=-1);
		DBOOL	ProcessCmd(const ConParse & parse, int nCmdIndex=-1);
	
		DBOOL	AddDelayedCmd(CMD_STRUCT_PARAM & cmd, int nCmdIndex);
		DBOOL	CheckArgs(const ConParse & parse, int nNum);
#ifndef _FINAL
		void	DevPrint(char *msg, ...);
#endif

	private :

		CMD_STRUCT	m_PendingCmds[CMDMGR_MAX_PENDING_COMMANDS];
};

#endif // __COMMAND_MGR_H__

// ----------------------------------------------------------------------- //
//
// MODULE  : AIScriptCommands.h
//
// PURPOSE : Implements the Script commands.
//
// CREATED : 8/23/00
//
// ----------------------------------------------------------------------- //

#ifndef AI_SCRIPT_COMMANDS_H
#define AI_SCRIPT_COMMANDS_H

#include "AIStrategyFollowPath.h"
#include "PlugFactoryTemplate.h"

#include "iltmessage.h"

#include <string>

class ScriptGoal;


class CAIScriptCommand
{
public:

	virtual ~CAIScriptCommand() {}

	// Init returns an error string.  If error string is empty, the init was successful.
	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs) { return ""; }

	virtual void Load(HMESSAGEREAD  hRead)  {}
	virtual void Save(HMESSAGEWRITE hWrite) {}

	virtual void Update() {}

	virtual void Start() {}
	virtual void End()   {}

	virtual bool IsDone()    const { return IsSuccess(); }
	virtual bool IsSuccess() const { return true; }


	virtual const char * GetCommandDescription() const { return "unknown"; }
	virtual const std::string & GetKey() const = 0;

};

void SaveScriptCommand(ILTMessage & out, /*const*/ CAIScriptCommand & command);
CAIScriptCommand * LoadScriptCommand(ILTMessage & in, ScriptGoal * pState, CAI * pAI);



typedef PlugFactory<CAIScriptCommand,std::string> CAIScriptCommandFactory;

#endif // AI_SCRIPT_COMMANDS_H

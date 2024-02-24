// ----------------------------------------------------------------------- //
//
// MODULE  : AIScriptCommands.cpp
//
// PURPOSE : Implements the Script commands.
//
// CREATED : 8/23/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIScriptCommands.h"
#include "AINodeMgr.h"
#include "Timer.h"
#include "AI.h"
#include "AIUtils.h"
#include "AIScript.h"
#include "ParseUtils.h"
#include "AIAnimButeMgr.h"
#include "AIActionMisc.h"

const char g_szDialog[] = "RPLY";
const char g_szEnd[] = "END";
const char g_szFace[] = "FC";
const char g_szFire[] = "FR";
const char g_szFollowPath[] = "FP";
const char g_szJumpTo[] = "JP";
const char g_szMoveTo[] = "MT";
const char g_szPlayAnimation[] = "PA";
const char g_szPlayAnimationSet[] = "PAS";
const char g_szPlayInterruptibleAnimation[] = "PIA";
const char g_szPlayInterruptibleAnimationSet[] = "PIAS";
const char g_szPlayLoopingAnimation[] = "PLA";
const char g_szPlayLoopingAnimationSet[] = "PLAS";
const char g_szPreciseMoveTo[] = "PMT";
const char g_szTeleport[] = "TLP";
const char g_szTrigger[] = "MSG";
const char g_szTriggerSelf[] = "MSGSELF";
const char g_szWait[] = "WT";

static char g_szDesc[256];

template< typename ProductT, typename PlugFactoryT>
class FactoryPlugin : public PlugFactoryT
{
public :

	explicit FactoryPlugin( const typename PlugFactoryT::Key & className)
		: PlugFactoryT(className) {}

protected :

	virtual typename PlugFactoryT::ProductFamily * MakeShape() const { return new ProductT(); }

};


template< typename ProductT >
class RegisterCommand : public FactoryPlugin<ProductT,CAIScriptCommandFactory>
{

public :
	explicit RegisterCommand(const std::string & new_key)
		: FactoryPlugin<ProductT,CAIScriptCommandFactory>( new_key ),
		  key(new_key) {}

	const std::string & GetKey() const { return key; }

private :

	const std::string key;

};


void SaveScriptCommand(ILTMessage & out, /*const*/ CAIScriptCommand & command)
{
	out << command.GetKey();

	command.Save(&out);
}

CAIScriptCommand * LoadScriptCommand(ILTMessage & in, ScriptGoal * pState, CAI * pAI)
{
	std::string key;

	in >> key;

	CAIScriptCommandFactory CommandFactory("");
	CAIScriptCommand * pResult = CommandFactory.NewAbstract(key);

	if( pResult ) 
	{
		pResult->Init(pState,pAI,LTNULL,0);
		pResult->Load(&in);
	}

	return pResult;
}


class CAIScriptMoveTo : public CAIScriptCommand
{

private:

	CAI *			  m_pAI;
	ScriptGoal *	  m_pState;
	CAIStrategyFollowPath * m_pFollowPath;
	LTVector              m_vDest;
	std::string			  m_strDestName;
	bool				  no_path;

	static const RegisterCommand<CAIScriptMoveTo> registerCommand;

public:

	CAIScriptMoveTo()
		: m_pAI(LTNULL),
		  m_pState(LTNULL),
		  m_pFollowPath(LTNULL),
		  m_vDest(0,0,0),
		  no_path(false) {}
	
	~CAIScriptMoveTo()
	{
		delete m_pFollowPath;
		m_pFollowPath = LTNULL;
	}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs )
	{
		_ASSERT( pAI );
		m_pAI = pAI;

		_ASSERT( pState );
		if( !pState ) return "Script command not attached to a script state!";
		m_pState = pState;

		delete m_pFollowPath;
		m_pFollowPath = new CAIStrategyFollowPath(pAI);
		m_pFollowPath->ExactMovement(LTFALSE);

		std::string result = CAIScriptCommand::Init(pState,pAI,pszArgs,nArgs);

		if( nArgs == 1 )
		{
			ConParse parse( const_cast<char*>(*pszArgs) );

			if( g_pLTServer->Common()->Parse(&parse) == LT_OK )
			{

				if( parse.m_nArgs == 3 )
				{
					m_vDest.x = (LTFLOAT)atof(parse.m_Args[0]);
					m_vDest.y = (LTFLOAT)atof(parse.m_Args[1]);
					m_vDest.z = (LTFLOAT)atof(parse.m_Args[2]);
					return result;
				}
				else if( parse.m_nArgs == 1 )
				{
					m_strDestName = parse.m_Args[0];
					return result;
				}
			}
		}

		std::string error_report = "Incorrect script arguments: ";
		for( const char * const * pszCurrentArg = pszArgs; 
		     pszCurrentArg < pszArgs + nArgs; ++pszCurrentArg )
		{
			error_report += *pszCurrentArg;
			error_report += " ";
		}
				 
		error_report += "\nUsage: \"mt (# # #)\" or \"mt node_name\"";

		return error_report;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		// m_pState set by Init.
		// m_pFollowPath created by Init.
		ASSERT( m_pFollowPath );
		if( m_pFollowPath )	*hRead >> *m_pFollowPath;
		*hRead >> m_vDest;
		*hRead >> m_strDestName;
		*hRead >> no_path;
	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		// m_pState set by Init.
		if( m_pFollowPath ) *hWrite << *m_pFollowPath;
		*hWrite << m_vDest;
		*hWrite << m_strDestName;
		*hWrite << no_path;

	}

	virtual void Update()
	{
		m_pFollowPath->Update();
	}


	virtual void Start()
	{
		no_path = false;

		if( !m_strDestName.empty() )
		{
			const CAINode * pNode = g_pAINodeMgr->GetNode(m_strDestName.c_str());
			if( pNode )
			{
				if( !m_pFollowPath->Set(pNode, LTTRUE) )
				{
					no_path = true;
					return;
				}
			}
			else
			{
				HOBJECT hObject = LTNULL;
				if( LT_OK == FindNamedObject(m_strDestName.c_str(),hObject) )
				{
					LTVector vPos;
					g_pLTServer->GetObjectPos(hObject,&vPos);
					if( !m_pFollowPath->Set(vPos) )
					{
						CCharacter * pCharTarget = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(hObject) );
						if( pCharTarget && pCharTarget->GetLastVolumePtr() )
						{
							if( !m_pFollowPath->Set( pCharTarget->GetLastVolumePos() ) )
							{
								no_path = true;
								return;
							}
						}
						else
						{
							no_path = true;
							return;
						}
					}
				}
#ifndef _FINAL
				else
				{
					AIErrorPrint( m_pAI, "Object named \"%s\" does not exist!",
						m_strDestName.c_str() );
				}
#endif
			}
		}
		else
		{
			if( !m_pFollowPath->Set(m_vDest) )
			{
				no_path = true;
				return;
			}
		}
	}

	virtual bool IsDone() const
	{
		return no_path || IsSuccess();
	}

	virtual bool IsSuccess() const
	{
		return m_pFollowPath && m_pFollowPath->IsDone();
	}

	virtual const char * GetCommandDescription() const
	{
		if( !m_strDestName.empty() )
		{
			sprintf(g_szDesc,"%s %s", g_szMoveTo, m_strDestName.c_str());
		}
		else
		{
			sprintf(g_szDesc,"%s (%.2f %.2f %.2f)",g_szMoveTo,m_vDest.x,m_vDest.y,m_vDest.z);
		}

		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptMoveTo>  CAIScriptMoveTo::registerCommand(g_szMoveTo);

class CAIScriptPreciseMoveTo : public CAIScriptCommand
{

private:

	CAI *					m_pAI;
	ScriptGoal *			m_pState;
	CAIStrategyFollowPath * m_pFollowPath;
	LTVector              m_vDest;
	std::string			  m_strDestName;
	bool				  no_path;

	static const RegisterCommand<CAIScriptPreciseMoveTo> registerCommand;

public:

	CAIScriptPreciseMoveTo()
		: m_pAI(LTNULL),
		  m_pState(LTNULL),
		  m_pFollowPath(LTNULL),
		  m_vDest(0,0,0),
		  no_path(false) {}

	~CAIScriptPreciseMoveTo()
	{
		delete m_pFollowPath;
		m_pFollowPath = LTNULL;
	}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs )
	{
		_ASSERT( pAI );
		m_pAI = pAI;

		_ASSERT( pState );
		if( !pState ) return "Script command not attached to a script state!";
		m_pState = pState;

		std::string result = CAIScriptCommand::Init(pState,pAI,pszArgs,nArgs);


		delete m_pFollowPath;
		m_pFollowPath = new CAIStrategyFollowPath(pAI);
		m_pFollowPath->ExactMovement(LTTRUE);

		if( nArgs == 1 )
		{
			ConParse parse( const_cast<char*>(*pszArgs) );

			if( g_pLTServer->Common()->Parse(&parse) == LT_OK )
			{

				if( parse.m_nArgs == 3 )
				{
					m_vDest.x = (LTFLOAT)atof(parse.m_Args[0]);
					m_vDest.y = (LTFLOAT)atof(parse.m_Args[1]);
					m_vDest.z = (LTFLOAT)atof(parse.m_Args[2]);
					return result;
				}
				else if( parse.m_nArgs == 1 )
				{
					m_strDestName = parse.m_Args[0];
					return result;
				}
			}
		}

		std::string error_report = "Incorrect script arguments: ";
		for( const char * const * pszCurrentArg = pszArgs; 
		     pszCurrentArg < pszArgs + nArgs; ++pszCurrentArg )
		{
			error_report += *pszCurrentArg;
			error_report += " ";
		}
				 
		error_report += "\nUsage: \"pmt (# # #)\" or \"pmt node_name\"";

		return error_report;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		// m_pState set by Init.
		ASSERT( m_pFollowPath );
		if( m_pFollowPath ) *hRead >> *m_pFollowPath;
		*hRead >> m_vDest;
		*hRead >> m_strDestName;
		*hRead >> no_path;
	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		// m_pState set by Init.
		if( m_pFollowPath ) *hWrite << *m_pFollowPath;
		*hWrite << m_vDest;
		*hWrite << m_strDestName;
		*hWrite << no_path;
	}

	virtual void Update()
	{
		m_pFollowPath->Update();
	}


	virtual void Start()
	{
		no_path = false;

		if( !m_strDestName.empty() )
		{
			const CAINode * pNode = g_pAINodeMgr->GetNode(m_strDestName.c_str());
			if( pNode )
			{
				if( !m_pFollowPath->Set(pNode, LTTRUE) )
				{
					no_path = true;
					return;
				}
			}
			else
			{
				HOBJECT hObject = LTNULL;
				if( LT_OK == FindNamedObject(m_strDestName.c_str(),hObject) )
				{
					LTVector vPos;
					g_pLTServer->GetObjectPos(hObject,&vPos);
					if( !m_pFollowPath->Set(vPos) )
					{
						CCharacter * pCharTarget = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(hObject) );
						if( pCharTarget && pCharTarget->GetLastVolumePtr() )
						{
							if( !m_pFollowPath->Set( pCharTarget->GetLastVolumePos() ) )
							{
								no_path = true;
								return;
							}
						}
						else
						{
							no_path = true;
							return;
						}
					}
				}
#ifndef _FINAL
				else
				{
					AIErrorPrint( m_pAI, "Object named \"%s\" does not exist!",
						m_strDestName.c_str() );
				}
#endif
			}
		}
		else
		{
			if( !m_pFollowPath->Set(m_vDest) )
			{
				no_path = true;
				return;
			}
		}
	}

	virtual bool IsDone() const
	{
		return no_path || IsSuccess();
	}

	virtual bool IsSuccess() const
	{
		return m_pFollowPath && m_pFollowPath->IsDone();
	}

	virtual const char * GetCommandDescription() const
	{
		if( !m_strDestName.empty() )
		{
			sprintf(g_szDesc,"%s %s", g_szPreciseMoveTo, m_strDestName.c_str());
		}
		else
		{
			sprintf(g_szDesc,"%s (%.2f %.2f %.2f)",g_szPreciseMoveTo,m_vDest.x,m_vDest.y,m_vDest.z);
		}

		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptPreciseMoveTo>  CAIScriptPreciseMoveTo::registerCommand(g_szPreciseMoveTo);


class CAIScriptWait : public CAIScriptCommand
{

public:

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs)
	{
		std::string result = CAIScriptCommand::Init(pState,pAI, pszArgs, nArgs);

		if( nArgs == 1 )
		{
			m_fWaitTime = (LTFLOAT)atof(*pszArgs);
			return result;
		}

		std::string error_report = "Incorrect script arguments: ";
		for( const char * const * pszCurrentArg = pszArgs; 
		     pszCurrentArg < pszArgs + nArgs; ++pszCurrentArg )
		{
			error_report += *pszCurrentArg;
			error_report += " ";
		}
				 
		error_report += "\nUsage: wait #";

		return error_report;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		*hRead >> m_tmrWait;
		*hRead >> m_fWaitTime;
	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		*hWrite << m_tmrWait;
		*hWrite << m_fWaitTime;
	}

	virtual void Start()
	{
		m_tmrWait.Start(m_fWaitTime);
	}

	virtual bool IsSuccess() const
	{
		return LTTRUE == m_tmrWait.Stopped();
	}

	virtual const char * GetCommandDescription() const
	{
		sprintf(g_szDesc, "%s %.2f",g_szWait, m_fWaitTime);
		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

private:

	CTimer m_tmrWait;
	LTFLOAT m_fWaitTime;

	static const RegisterCommand<CAIScriptWait> registerCommand;
};

const RegisterCommand<CAIScriptWait> CAIScriptWait::registerCommand(g_szWait);


class CAIScriptEnd : public CAIScriptCommand
{

private:

	static const RegisterCommand<CAIScriptEnd> registerCommand;

	CAI * m_pAI;

public:

	CAIScriptEnd()
		: m_pAI(LTNULL) {}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs)
	{
		std::string result = CAIScriptCommand::Init(pState, pAI, pszArgs, nArgs);

		m_pAI = pAI;

		return (m_pAI != LTNULL) ? result : "Script not attached to an AI!";
	}


	virtual void Update()
	{
		_ASSERT( m_pAI );
		if( !m_pAI ) return;

		// !!!!!! Deletes the script goal !!!!!!
		m_pAI->RemoveGoal( m_pAI->GetCurrentSlotName() );
	}

	virtual bool IsSuccess() const
	{
		return false;
	}

	virtual bool IsDone() const
	{
		return false;
	}

	virtual const char * GetCommandDescription() const
	{
		return g_szEnd;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptEnd> CAIScriptEnd::registerCommand(g_szEnd);


class CAIScriptTriggerSelf : public CAIScriptCommand
{

private:

	CAI * m_pAI;
	std::string m_strMessage;
	bool m_bSentMessage;

	static const RegisterCommand<CAIScriptTriggerSelf> registerCommand;

public:

	CAIScriptTriggerSelf()
		: m_pAI(LTNULL),
		  m_bSentMessage(false) {}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs)
	{
		std::string result =  CAIScriptCommand::Init(pState, pAI, pszArgs, nArgs);

		ASSERT( pAI );
		if( !pAI ) return "Script not attached to an AI!";

		m_pAI = pAI;
		m_strMessage = "";

		for( const char * const * pszCurrentArg = pszArgs; 
		     pszCurrentArg < pszArgs + nArgs; ++pszCurrentArg)
		{
			if( *pszCurrentArg )
			{
				m_strMessage += " (";
				m_strMessage += *pszCurrentArg;
				m_strMessage += ")";
			}
		}

		return result;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		// m_pAI is set by Init.
		*hRead >> m_strMessage;		
		*hRead >> m_bSentMessage;
	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		// m_pAI is set by Init.
		*hWrite << m_strMessage;
		*hWrite << m_bSentMessage;
	}

	virtual void Start()
	{
		SendTriggerMsgToObject(m_pAI, m_pAI->m_hObject, m_strMessage.c_str() );
		m_bSentMessage = true;
	}

	virtual bool IsSuccess() const
	{
		return m_bSentMessage;
	}

	virtual const char * GetCommandDescription() const
	{
		sprintf(g_szDesc, "%s %s",g_szTriggerSelf, m_strMessage.c_str() );
		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptTriggerSelf> CAIScriptTriggerSelf::registerCommand(g_szTriggerSelf);



class CAIScriptTrigger : public CAIScriptCommand
{

private:

	CAI * m_pAI;
	std::string m_strName;
	std::string m_strMessage;
	bool m_bSuccess;
	bool m_bSentMessage;

	static const RegisterCommand<CAIScriptTrigger> registerCommand;


public:

	CAIScriptTrigger()
		: m_pAI(LTNULL),
		  m_bSuccess(false),
		  m_bSentMessage(false) {}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs)
	{
		std::string result = CAIScriptCommand::Init(pState, pAI, pszArgs, nArgs);

		_ASSERT( pAI );
		if( !pAI ) return "Script not attached to an AI!";

		m_pAI = pAI;

		if( nArgs > 0 )
		{
			// The name of the message target.
			m_strName = pszArgs[0];
			m_strMessage = "";

			// If there is another argument, make it the message.
			if( nArgs >= 2 )
			{
				m_strMessage = pszArgs[1];
			}

			// If there is anymore text, add it to the message with space seperators.
			for( const char * const * pszCurrentArg = pszArgs+2;
				 pszCurrentArg < pszArgs + nArgs; ++pszCurrentArg )
			{
				if( *pszCurrentArg )
				{
					m_strMessage += " ";
					m_strMessage += *pszCurrentArg;
				}
			}

			return result;
		}

		std::string error_report = "No script argument given!";
				 
		error_report += "\nUsage: trig target_name trigger_message";

		return error_report;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		// m_pAI is set by Init.
		*hRead >> m_strName;
		*hRead >> m_strMessage;
		*hRead >> m_bSuccess;
		*hRead >> m_bSentMessage;
	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		// m_pAI is set by Init.
		*hWrite << m_strName;
		*hWrite << m_strMessage;
		*hWrite << m_bSuccess;
		*hWrite << m_bSentMessage;
	}

	virtual void Start()
	{
		m_bSuccess = (LTTRUE == SendTriggerMsgToObjects( m_pAI, m_strName.c_str(), m_strMessage.c_str() ));
		m_bSentMessage = true;
	}

	virtual bool IsSuccess() const
	{
		return m_bSuccess;
	}

	virtual bool IsDone() const 
	{
		return m_bSentMessage;
	}

	virtual const char * GetCommandDescription() const
	{
		sprintf(g_szDesc, "%s %s %s",g_szTrigger, m_strName.c_str(), m_strMessage.c_str() );
		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptTrigger> CAIScriptTrigger::registerCommand(g_szTrigger);


class CAIScriptPlayAnimation : public CAIScriptCommand
{

private:

	ScriptGoal * m_pScript;
	std::string m_strAnimName;


	static const RegisterCommand<CAIScriptPlayAnimation> registerCommand;


public:

	CAIScriptPlayAnimation()
		: m_pScript(LTNULL) {}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs)
	{
		std::string result = CAIScriptCommand::Init(pState, pAI, pszArgs, nArgs);

		_ASSERT( pState );
		if( !pState ) return "Script command has now owner!";

		m_pScript = pState;

		if( nArgs == 1 )
		{
			m_strAnimName = *pszArgs;

			return result;
		}

		std::string error_report = "";
		if( nArgs < 1 )
		{
			error_report += "No arguments given";
		}
		else
		{
			error_report += "Too many arguments given :";
			for( const char * const * pszCurrentArg = pszArgs; pszCurrentArg < pszArgs + nArgs; ++pszCurrentArg)
			{
				error_report += " ";
				error_report += *pszCurrentArg;
			}
		}
		
				 
		error_report += "\nUsage: ";
		error_report += g_szPlayAnimation;
		error_report += " animation_name";

		return error_report;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		// m_pScript is set by Init.
		*hRead >> m_strAnimName;
	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		// m_pScript is set by Init.
		*hWrite << m_strAnimName;
	}

	virtual void Start()
	{
		_ASSERT( m_pScript );

		m_pScript->PlayAnimation( m_strAnimName.c_str() );
	}

	virtual void Stop()
	{
		_ASSERT( m_pScript );

		m_pScript->StopPlayingAnimation();
	}

	virtual bool IsSuccess() const
	{
		_ASSERT( m_pScript );
		if( !m_pScript ) return true;

		return m_pScript->FinishedAnimation();
	}

	virtual const char * GetCommandDescription() const
	{
		sprintf(g_szDesc, "%s %s", g_szPlayAnimation, m_strAnimName.c_str() );
		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptPlayAnimation> CAIScriptPlayAnimation::registerCommand(g_szPlayAnimation);

class CAIScriptPlayInterruptibleAnimation : public CAIScriptCommand
{

private:

	ScriptGoal * m_pScript;
	std::string m_strAnimName;


	static const RegisterCommand<CAIScriptPlayInterruptibleAnimation> registerCommand;


public:

	CAIScriptPlayInterruptibleAnimation()
		: m_pScript(LTNULL) {}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs)
	{
		std::string result = CAIScriptCommand::Init(pState, pAI, pszArgs, nArgs);

		_ASSERT( pState );
		if( !pState ) return "Script command has now owner!";

		m_pScript = pState;

		if( nArgs == 1 )
		{
			m_strAnimName = *pszArgs;

			return result;
		}

		std::string error_report = "";
		if( nArgs < 1 )
		{
			m_strAnimName = "none";
		}
		else
		{
			error_report += "Too many arguments given :";
			for( const char * const * pszCurrentArg = pszArgs; pszCurrentArg < pszArgs + nArgs; ++pszCurrentArg)
			{
				error_report += " ";
				error_report += *pszCurrentArg;
			}
		}
		
				 
		error_report += "\nUsage: ";
		error_report += g_szPlayInterruptibleAnimation;
		error_report += " animation_name";

		return error_report;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		// m_pScript is set by Init.
		*hRead >> m_strAnimName;
	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		// m_pScript is set by Init.
		*hWrite << m_strAnimName;
	}

	virtual void Start()
	{
		_ASSERT( m_pScript );

		if( 0 == stricmp(m_strAnimName.c_str(),"none") )
		{
			m_pScript->StopPlayingAnimation();
		}
		else
		{
			m_pScript->PlayInterruptibleAnimation( m_strAnimName.c_str() );
		}
	}

	virtual bool IsSuccess() const
	{
		return true;
	}

	virtual const char * GetCommandDescription() const
	{
		sprintf(g_szDesc, "%s %s", g_szPlayInterruptibleAnimation, m_strAnimName.c_str() );
		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptPlayInterruptibleAnimation> CAIScriptPlayInterruptibleAnimation::registerCommand(g_szPlayInterruptibleAnimation);


class CAIScriptPlayLoopingAnimation : public CAIScriptCommand
{

private:

	ScriptGoal * m_pScript;
	std::string m_strAnimName;


	static const RegisterCommand<CAIScriptPlayLoopingAnimation> registerCommand;


public:

	CAIScriptPlayLoopingAnimation()
		: m_pScript(LTNULL) {}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs)
	{
		std::string result = CAIScriptCommand::Init(pState, pAI, pszArgs, nArgs);

		_ASSERT( pState );
		if( !pState ) return "Script command has now owner!";

		m_pScript = pState;

		if( nArgs == 1 )
		{
			m_strAnimName = *pszArgs;

			return result;
		}

		std::string error_report = "";
		if( nArgs < 1 )
		{
			m_strAnimName = "none";
		}
		else
		{
			error_report += "Too many arguments given :";
			for( const char * const * pszCurrentArg = pszArgs; pszCurrentArg < pszArgs + nArgs; ++pszCurrentArg)
			{
				error_report += " ";
				error_report += *pszCurrentArg;
			}
		}
		
				 
		error_report += "\nUsage: ";
		error_report += g_szPlayLoopingAnimation;
		error_report += " animation_name";

		return error_report;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		// m_pScript is set by Init.
		*hRead >> m_strAnimName;
	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		// m_pScript is set by Init.
		*hWrite << m_strAnimName;
	}

	virtual void Start()
	{
		_ASSERT( m_pScript );

		if( 0 == stricmp(m_strAnimName.c_str(),"none") )
		{
			m_pScript->StopPlayingAnimation();
		}
		else
		{
			m_pScript->PlayLoopingAnimation( m_strAnimName.c_str() );
		}
	}

	virtual bool IsSuccess() const
	{
		return true;
	}

	virtual const char * GetCommandDescription() const
	{
		sprintf(g_szDesc, "%s %s", g_szPlayLoopingAnimation, m_strAnimName.c_str() );
		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptPlayLoopingAnimation> CAIScriptPlayLoopingAnimation::registerCommand(g_szPlayLoopingAnimation);


class CAIScriptDialog : public CAIScriptCommand
{

private:

	CAI * m_pAI;
	std::string m_strDialogName;
	bool m_bPlayedDialog;


	static const RegisterCommand<CAIScriptDialog> registerCommand;


public:

	CAIScriptDialog()
		: m_pAI(LTNULL),
		  m_bPlayedDialog(false) {}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs)
	{
		std::string result = CAIScriptCommand::Init(pState, pAI, pszArgs, nArgs);

		_ASSERT( pAI );
		if( !pAI ) return "Script not attached to an AI!";
		m_pAI = pAI;

		if( nArgs == 1 )
		{
			m_strDialogName = *pszArgs;

			return result;
		}

		std::string error_report = "";
		if( nArgs < 1 )
		{
			error_report += "No arguments given";
		}
		else
		{
			error_report += "Too many arguments given :";
			for( const char * const * pszCurrentArg = pszArgs; pszCurrentArg < pszArgs + nArgs; ++pszCurrentArg)
			{
				error_report += " ";
				error_report += *pszCurrentArg;
			}
		}
		
				 
		error_report += "\nUsage: say dialog_name";

		return error_report;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		// m_pAnim is set by Init.
		*hRead >> m_strDialogName;
		*hRead >> m_bPlayedDialog;
	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		// m_pAnim is set by Init.
		*hWrite << m_strDialogName;
		*hWrite << m_bPlayedDialog;
	}

	virtual void Start()
	{
		_ASSERT( m_pAI );

		m_pAI->PlayDialogueSound( FullSoundName(m_strDialogName).c_str() );
	}

	virtual void Stop()
	{
		_ASSERT( m_pAI );

		if( !m_bPlayedDialog )
			m_pAI->KillVoicedSound();
	}

	virtual void Update()
	{
		_ASSERT( m_pAI );

		if( !m_pAI->IsPlayingDialogue() )
		{
			m_bPlayedDialog = true;
		}
	}

	virtual bool IsSuccess() const
	{
		return m_bPlayedDialog;
	}

	virtual const char * GetCommandDescription() const
	{
		sprintf(g_szDesc, "%s %s",g_szDialog, m_strDialogName.c_str() );
		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptDialog> CAIScriptDialog::registerCommand(g_szDialog);



class CAIScriptFace : public CAIScriptCommand
{

private:

	ScriptGoal * m_pState;
	ParseLocation	 m_PLocation;

	static const RegisterCommand<CAIScriptFace> registerCommand;


public:

	CAIScriptFace() 
		: m_pState(LTNULL) {}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs)
	{
		std::string result = CAIScriptCommand::Init(pState, pAI, pszArgs, nArgs);

		_ASSERT( pState );
		if( !pState ) return "Script command not attached to a script state!";
		
		m_pState = pState;

		if( nArgs == 1 )
		{
			m_PLocation = ParseLocation(pszArgs[0],*pAI);
		} 

		return m_PLocation.error;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		// m_pState is set by Init.
		*hRead >> m_PLocation;
	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		// m_pState is set by Init.
		*hWrite << m_PLocation;
	}

	virtual void Start()
	{
		_ASSERT(m_pState);

		if( m_pState ) 
		{
			if( m_PLocation.bOff )
			{
				m_pState->Face(false);
			}
			else if( m_PLocation.bTarget )
			{
				m_pState->FaceTarget(true);
			}
			else if( m_PLocation.hObject )
			{
				m_pState->Face(true,m_PLocation.hObject);
			}
			else
			{
				m_pState->Face(true,m_PLocation.vPosition);
			}
		}
	}

	virtual bool IsSuccess() const
	{
		return m_PLocation.bOff || (m_pState && m_pState->FinishedFacing());
	}

	virtual const char * GetCommandDescription() const
	{
		if( m_PLocation.bOff )
		{
			sprintf(g_szDesc, "%s off",g_szFace );
		}
		else if( m_PLocation.bTarget )
		{
			sprintf(g_szDesc, "%s target",g_szFace );
		}
		else if( m_PLocation.hObject )
		{
			sprintf(g_szDesc, "%s %s",g_szFace, 
					 g_pLTServer->GetObjectName(m_PLocation.hObject) );
		}
		else
		{
			sprintf(g_szDesc, "%s (%.2f %.2f %.2f)",g_szFace, 
				m_PLocation.vPosition.x, m_PLocation.vPosition.y, m_PLocation.vPosition.z );
		}

		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptFace> CAIScriptFace::registerCommand(g_szFace);

class CAIScriptFire : public CAIScriptCommand
{

private:

	ScriptGoal * m_pState;
	ParseLocation	 m_PLocation;

	static const RegisterCommand<CAIScriptFire> registerCommand;


public:

	CAIScriptFire() 
		: m_pState(LTNULL) {}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs)
	{
		std::string result = CAIScriptCommand::Init(pState, pAI, pszArgs, nArgs);

		_ASSERT( pState );
		if( !pState ) return "Script command not attached to a script state!";
		
		m_pState = pState;

		if( nArgs == 1 )
		{
			m_PLocation = ParseLocation(pszArgs[0],*pAI);
		} 

		return m_PLocation.error;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		// m_pState is set by Init.
		*hRead >> m_PLocation;
	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		// m_pState is set by Init.
		*hWrite << m_PLocation;
	}

	virtual void Start()
	{
		_ASSERT(m_pState);

		if( m_pState ) 
		{
			if( m_PLocation.bOff )
			{
				m_pState->FireAt(false);
			}
			else if( m_PLocation.bTarget )
			{
				m_pState->FireAtTarget(true);
			}
			else if( m_PLocation.hObject )
			{
				m_pState->FireAt(true,m_PLocation.hObject);
			}
			else
			{
				m_pState->FireAt(true,m_PLocation.vPosition);
			}
		}
	}

	virtual bool IsSuccess() const
	{
		return m_PLocation.bOff || (m_pState && m_pState->IsFiring());
	}

	virtual const char * GetCommandDescription() const
	{
		if( m_PLocation.bOff )
		{
			sprintf(g_szDesc, "%s off",g_szFire );
		}
		else if( m_PLocation.bTarget )
		{
			sprintf(g_szDesc, "%s target",g_szFire );
		}
		else if( m_PLocation.hObject )
		{
			sprintf(g_szDesc, "%s %s",g_szFire, 
					 g_pLTServer->GetObjectName(m_PLocation.hObject) );
		}
		else
		{
			sprintf(g_szDesc, "%s (%.2f %.2f %.2f)",g_szFire, 
				m_PLocation.vPosition.x, m_PLocation.vPosition.y, m_PLocation.vPosition.z );
		}

		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptFire> CAIScriptFire::registerCommand(g_szFire);


class CAIScriptFollowPath : public CAIScriptMoveTo
{

private:

	ScriptGoal *	  m_pState;
	CAIStrategyFollowPath * m_pFollowPath;
	uint32				  m_nNodeID;
	std::string base_name;
	uint32		node_number;
	bool		finished_path;
	LTSmartLink m_hAI;

	static const RegisterCommand<CAIScriptFollowPath> registerCommand;


public:

	CAIScriptFollowPath() 
		: m_pState(LTNULL),
		  m_pFollowPath(LTNULL),
		  m_nNodeID(CAINode::kInvalidID),
		  node_number(0),
		  finished_path(false) {}

	~CAIScriptFollowPath()
	{
		delete m_pFollowPath;
		m_pFollowPath = LTNULL;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		// m_pState set by Init.
		if( m_pFollowPath ) *hRead >> *m_pFollowPath;
		*hRead >> m_nNodeID;
		*hRead >> base_name;
		*hRead >> node_number;
		*hRead >> finished_path;
		*hRead >> m_hAI;
	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		// m_pState set by Init.
		if( m_pFollowPath ) *hWrite << *m_pFollowPath;
		*hWrite << m_nNodeID;
		*hWrite << base_name;
		*hWrite << node_number;
		*hWrite << finished_path;
		*hWrite << m_hAI;
	}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs)
	{
		_ASSERT( pState );
		if( !pState ) return "Script command not attached to a script state!";
		m_pState = pState;

		std::string result = CAIScriptCommand::Init(pState, pAI, pszArgs, nArgs);

		m_hAI = pAI->m_hObject;

		delete m_pFollowPath;
		m_pFollowPath = new CAIStrategyFollowPath(pAI);
		m_pFollowPath->ExactMovement(LTFALSE);

		if( nArgs == 1 )
		{
			base_name = *pszArgs;
			return result;
		} 

		std::string error_report = "";
		if( nArgs < 1 )
		{
			error_report += "No arguments given";
		}
		else
		{
			error_report += "Too many arguments given :";
			for( const char * const * pszCurrentArg = pszArgs; pszCurrentArg < pszArgs + nArgs; ++pszCurrentArg)
			{
				error_report += " ";
				error_report += *pszCurrentArg;
			}
		}
		
				 
		error_report += "\nUsage: fp path_base_name";

		return error_report;
	}

	virtual void Start()
	{
		// Initialize our state variables.
		m_pFollowPath->Clear();
		node_number = 0;
		finished_path = false;
	}

	virtual void Update()
	{
		if( IsDone() ) return;

		// See if we need to find our next node.
		if( m_pFollowPath->IsUnset() || m_pFollowPath->IsDone() )
		{
			// Assume we reached the last node. Move on to the next.

			// Get the next node name.
			char node_name[256];
			sprintf(node_name,"%s%d", base_name.c_str(), node_number++);

			// Find the next node.
			const CAINode * pNode = g_pAINodeMgr->GetNode(node_name);
			if( !pNode )
			{
				sprintf(node_name,"%s%02d", base_name.c_str(), node_number);
				pNode = g_pAINodeMgr->GetNode(node_name);
			}

			if( !pNode )
			{
				// We couldn't find the next node, assume we've
				// reached the end of the path.
				finished_path = true;
				return;
			}

			// Record our node id so that we can re-path to it if we get stuck.
			m_nNodeID = pNode->GetID();

			// Check for another node after that.
			sprintf(node_name,"%s%d", base_name.c_str(), node_number+1);
			const CAINode * pNextNode = g_pAINodeMgr->GetNode(node_name);
			if( !pNextNode )
			{
				sprintf(node_name,"%s%02d", base_name.c_str(), node_number+1);
				pNextNode = g_pAINodeMgr->GetNode(node_name);
			}

			if( !pNextNode )
			{
				m_pFollowPath->ExactMovement(LTTRUE);
			}
			else
			{
				m_pFollowPath->ExactMovement(LTFALSE);
			}

			// Okay, we have a node!
			m_pFollowPath->Set(pNode, LTTRUE);

#ifndef _FINAL
			if( ShouldShowScript( m_hAI ) )
			{
				AICPrint(m_hAI, "\"%s\" now going to node \"%s\"",
					GetCommandDescription(), pNode->GetName().c_str() );
			}
#endif
		} //if( m_pFollowPath->IsUnset() || m_pFollowPath->IsDone() )

		// Go to the next node.
		m_pFollowPath->Update();
	}

	virtual bool IsDone() const
	{
		return finished_path;
	}

	virtual bool IsSuccess() const
	{
		return finished_path;
	}

	virtual const char * GetCommandDescription() const
	{
		sprintf(g_szDesc, "%s %s",g_szFollowPath, base_name.c_str());

		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptFollowPath> CAIScriptFollowPath::registerCommand(g_szFollowPath);


class CAIScriptTeleport : public CAIScriptCommand
{

private:

	CAI * m_pAI;
	ParseLocation	 m_PLocation;

	static const RegisterCommand<CAIScriptTeleport> registerCommand;


public:

	CAIScriptTeleport()  {}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs)
	{
		std::string result = CAIScriptCommand::Init(pState, pAI, pszArgs, nArgs);

		if( !result.empty() )
			return result;

		ASSERT( pAI );
		m_pAI = pAI;

		if( nArgs == 1 )
		{
			m_PLocation = ParseLocation(pszArgs[0],*pAI);
		} 

		return m_PLocation.error;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		*hRead >> m_PLocation;
	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		*hWrite << m_PLocation;
	}

	virtual void Start()
	{
		ASSERT( m_pAI );
		if( m_pAI )
			m_pAI->Teleport(m_PLocation);
	}

	virtual bool IsSuccess() const
	{
		return true;
	}

	virtual const char * GetCommandDescription() const
	{
		if( m_PLocation.bOff )
		{
			sprintf(g_szDesc, "%s off",g_szTeleport );
		}
		else if( m_PLocation.bTarget )
		{
			sprintf(g_szDesc, "%s target",g_szTeleport );
		}
		else if( m_PLocation.hObject )
		{
			sprintf(g_szDesc, "%s %s",g_szTeleport, 
					 g_pLTServer->GetObjectName(m_PLocation.hObject) );
		}
		else
		{
			sprintf(g_szDesc, "%s (%.2f %.2f %.2f)",g_szTeleport, 
				m_PLocation.vPosition.x, m_PLocation.vPosition.y, m_PLocation.vPosition.z );
		}

		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptTeleport> CAIScriptTeleport::registerCommand(g_szTeleport);



class CAIScriptPlayAnimationSet : public CAIScriptCommand
{

private:

	ScriptGoal * m_pScript;
	const AIAnimSet * m_pAnimSet;


	static const RegisterCommand<CAIScriptPlayAnimationSet> registerCommand;


public:

	CAIScriptPlayAnimationSet()
		: m_pScript(LTNULL),
		  m_pAnimSet(LTNULL) {}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs)
	{
		std::string result = CAIScriptCommand::Init(pState, pAI, pszArgs, nArgs);

		_ASSERT( pState );
		if( !pState ) return "Script command has now owner!";

		m_pScript = pState;

		if( nArgs == 1 )
		{
			m_pAnimSet = g_pAIAnimButeMgr->GetAnimSetPtr(*pszArgs);
			if( m_pAnimSet )
			{
				return result;
			}

			std::string no_anim_set = g_szPlayAnimationSet;
			no_anim_set += " -- AnimSet \"";
			no_anim_set += *pszArgs;
			no_anim_set += "\" not found.";
			return no_anim_set;
		}

		std::string error_report = "";
		if( nArgs < 1 )
		{
			error_report += "No arguments given";
		}
		else
		{
			error_report += "Too many arguments given :";
			for( const char * const * pszCurrentArg = pszArgs; pszCurrentArg < pszArgs + nArgs; ++pszCurrentArg)
			{
				error_report += " ";
				error_report += *pszCurrentArg;
			}
		}
		
				 
		error_report += "\nUsage: ";
		error_report += g_szPlayAnimationSet;
		error_report += " animation_set_name (in AIAnimations.txt)";

		return error_report;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		// m_pScript is set by Init.
		std::string strAnimSetName;
		*hRead >> strAnimSetName;

		m_pAnimSet = g_pAIAnimButeMgr->GetAnimSetPtr(strAnimSetName.c_str());
	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		// m_pScript is set by Init.
		*hWrite << (m_pAnimSet ? m_pAnimSet->m_strName : std::string());
	}

	virtual void Start()
	{
		_ASSERT( m_pScript );
		_ASSERT( m_pAnimSet );

		if( m_pAnimSet && (m_pAnimSet->m_fPlayChance > GetRandom(0.0f,1.0f))  )
			m_pScript->PlayAnimation( m_pAnimSet->GetRandomAnim().c_str() );
		else
			m_pScript->StopPlayingAnimation();
	}

	virtual void Stop()
	{
		_ASSERT( m_pScript );

		m_pScript->StopPlayingAnimation();
	}

	virtual bool IsSuccess() const
	{
		_ASSERT( m_pScript );
		if( !m_pScript ) return true;

		return m_pScript->FinishedAnimation();
	}

	virtual const char * GetCommandDescription() const
	{
		sprintf(g_szDesc, "%s %s", g_szPlayAnimationSet, m_pAnimSet ? m_pAnimSet->m_strName.c_str() : "No anim. set!" );
		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptPlayAnimationSet> CAIScriptPlayAnimationSet::registerCommand(g_szPlayAnimationSet);

class CAIScriptPlayInterruptibleAnimationSet : public CAIScriptCommand
{

private:

	ScriptGoal * m_pScript;
	const AIAnimSet * m_pAnimSet;


	static const RegisterCommand<CAIScriptPlayInterruptibleAnimationSet> registerCommand;


public:

	CAIScriptPlayInterruptibleAnimationSet()
		: m_pScript(LTNULL),
		  m_pAnimSet(LTNULL) {}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs)
	{
		std::string result = CAIScriptCommand::Init(pState, pAI, pszArgs, nArgs);

		_ASSERT( pState );
		if( !pState ) return "Script command has now owner!";

		m_pScript = pState;

		if( nArgs == 1 )
		{
			if( 0 == stricmp(*pszArgs,"none") )
			{
				m_pAnimSet = LTNULL;
				return result;
			}
			else
			{
				m_pAnimSet = g_pAIAnimButeMgr->GetAnimSetPtr(*pszArgs);
				if( m_pAnimSet )
				{
					return result;
				}

				std::string no_anim_set = g_szPlayInterruptibleAnimationSet;
				no_anim_set += " -- AnimSet \"";
				no_anim_set += *pszArgs;
				no_anim_set += "\" not found.";
				return no_anim_set;
			}
		}

		std::string error_report = "";
		if( nArgs > 1 )
		{
			error_report += "Too many arguments given :";
			for( const char * const * pszCurrentArg = pszArgs; pszCurrentArg < pszArgs + nArgs; ++pszCurrentArg)
			{
				error_report += " ";
				error_report += *pszCurrentArg;
			}
		}
		
				 
		error_report += "\nUsage: ";
		error_report += g_szPlayInterruptibleAnimation;
		error_report += " animation_set_name (in AIAnimations.txt)";

		return error_report;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		// m_pScript is set by Init.
		std::string strAnimSetName;
		*hRead >> strAnimSetName;

		if( !strAnimSetName.empty() )
			m_pAnimSet = g_pAIAnimButeMgr->GetAnimSetPtr(strAnimSetName.c_str());
		else
			m_pAnimSet = LTNULL;

	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		// m_pScript is set by Init.
		*hWrite << (m_pAnimSet ? m_pAnimSet->m_strName : std::string());
	}

	virtual void Start()
	{
		_ASSERT( m_pScript );

		if( m_pAnimSet && (m_pAnimSet->m_fPlayChance > GetRandom(0.0f,1.0f))  )
			m_pScript->PlayInterruptibleAnimation( m_pAnimSet->GetRandomAnim().c_str() );
		else
			m_pScript->StopPlayingAnimation();
	}

	virtual bool IsSuccess() const
	{
		return true;
	}

	virtual const char * GetCommandDescription() const
	{
		sprintf(g_szDesc, "%s %s", g_szPlayInterruptibleAnimationSet, m_pAnimSet ? m_pAnimSet->m_strName.c_str() : "none" );
		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptPlayInterruptibleAnimationSet> CAIScriptPlayInterruptibleAnimationSet::registerCommand(g_szPlayInterruptibleAnimationSet);


class CAIScriptPlayLoopingAnimationSet : public CAIScriptCommand
{

private:

	ScriptGoal * m_pScript;
	const AIAnimSet * m_pAnimSet;


	static const RegisterCommand<CAIScriptPlayLoopingAnimationSet> registerCommand;


public:

	CAIScriptPlayLoopingAnimationSet()
		: m_pScript(LTNULL),
		  m_pAnimSet(LTNULL) {}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs)
	{
		std::string result = CAIScriptCommand::Init(pState, pAI, pszArgs, nArgs);

		_ASSERT( pState );
		if( !pState ) return "Script command has now owner!";

		m_pScript = pState;

		if( nArgs == 1 )
		{
			if( 0 == stricmp(*pszArgs,"none") )
			{
				m_pAnimSet = LTNULL;
				return result;
			}
			else
			{
				m_pAnimSet = g_pAIAnimButeMgr->GetAnimSetPtr(*pszArgs);
				if( m_pAnimSet )
				{
					return result;
				}

				std::string no_anim_set = g_szPlayLoopingAnimationSet;
				no_anim_set += " -- AnimSet \"";
				no_anim_set += *pszArgs;
				no_anim_set += "\" not found.";
				return no_anim_set;
			}
		}

		std::string error_report = "";
		if( nArgs > 1 )
		{
			error_report += "Too many arguments given :";
			for( const char * const * pszCurrentArg = pszArgs; pszCurrentArg < pszArgs + nArgs; ++pszCurrentArg)
			{
				error_report += " ";
				error_report += *pszCurrentArg;
			}
		}
		
				 
		error_report += "\nUsage: ";
		error_report += g_szPlayLoopingAnimationSet;
		error_report += " animation_set_name (in AIAnimations.txt)";

		return error_report;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		// m_pScript is set by Init.
		std::string strAnimSetName;
		*hRead >> strAnimSetName;

		if( !strAnimSetName.empty() )
			m_pAnimSet = g_pAIAnimButeMgr->GetAnimSetPtr(strAnimSetName.c_str());
		else
			m_pAnimSet = LTNULL;
	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		// m_pScript is set by Init.
		*hWrite << (m_pAnimSet ? m_pAnimSet->m_strName : std::string());
	}

	virtual void Start()
	{
		_ASSERT( m_pScript );

		if( m_pAnimSet && (m_pAnimSet->m_fPlayChance > GetRandom(0.0f,1.0f))  )
			m_pScript->PlayLoopingAnimation( m_pAnimSet->GetRandomAnim().c_str() );
		else
			m_pScript->StopPlayingAnimation();
	}

	virtual bool IsSuccess() const
	{
		return true;
	}

	virtual const char * GetCommandDescription() const
	{
		sprintf(g_szDesc, "%s %s", g_szPlayLoopingAnimationSet, m_pAnimSet ? m_pAnimSet->m_strName.c_str() : "none" );
		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptPlayLoopingAnimationSet> CAIScriptPlayLoopingAnimationSet::registerCommand(g_szPlayLoopingAnimationSet);


class CAIScriptJumpTo : public CAIScriptCommand
{

private:

	CAI * m_pAI;
	ParseLocation	 m_PLocation;

	LTBOOL m_bDone;

	static const RegisterCommand<CAIScriptJumpTo> registerCommand;


public:

	CAIScriptJumpTo()  
		: m_pAI(LTNULL),
		  m_bDone(LTFALSE) {}

	virtual std::string Init(ScriptGoal * pState, CAI * pAI, const char * const * pszArgs, int nArgs)
	{
		std::string result = CAIScriptCommand::Init(pState, pAI, pszArgs, nArgs);

		if( !result.empty() )
			return result;

		ASSERT( pAI );
		m_pAI = pAI;

		if( nArgs == 1 )
		{
			m_PLocation = ParseLocation(pszArgs[0],*pAI);
		} 

		return m_PLocation.error;
	}

	virtual void Load(HMESSAGEREAD  hRead)
	{
		ASSERT( hRead );
		if( !hRead ) return;

		CAIScriptCommand::Load(hRead);

		*hRead >> m_PLocation;
		*hRead >> m_bDone;
	}

	virtual void Save(HMESSAGEWRITE hWrite)
	{
		ASSERT( hWrite );
		if( !hWrite ) return;

		CAIScriptCommand::Save(hWrite);

		*hWrite << m_PLocation;
		*hWrite << m_bDone;
	}

	virtual void Start()
	{
		ASSERT( m_pAI );
		if( m_pAI )
		{
			AIActionJumpTo * pAction = dynamic_cast<AIActionJumpTo*>(m_pAI->SetNextAction(CAI::ACTION_JUMPTO));
			if( pAction )
			{
				pAction->Init(m_PLocation.vPosition);
			}
		}
	}

	virtual void Update()
	{
		if( m_pAI->CanDoNewAction() )
		{
			m_bDone = LTTRUE;
		}
	}

	virtual bool IsSuccess() const
	{
		return (m_bDone == LTTRUE);
	}

	virtual const char * GetCommandDescription() const
	{
		if( m_PLocation.bOff )
		{
			sprintf(g_szDesc, "%s off",g_szJumpTo );
		}
		else if( m_PLocation.bTarget )
		{
			sprintf(g_szDesc, "%s target",g_szJumpTo );
		}
		else if( m_PLocation.hObject )
		{
			sprintf(g_szDesc, "%s %s",g_szJumpTo, 
					 g_pLTServer->GetObjectName(m_PLocation.hObject) );
		}
		else
		{
			sprintf(g_szDesc, "%s (%.2f %.2f %.2f)",g_szJumpTo, 
				m_PLocation.vPosition.x, m_PLocation.vPosition.y, m_PLocation.vPosition.z );
		}

		return g_szDesc;
	}

	virtual const std::string & GetKey() const { return registerCommand.GetKey(); }

};

const RegisterCommand<CAIScriptJumpTo> CAIScriptJumpTo::registerCommand(g_szJumpTo);


// ----------------------------------------------------------------------- //
//
// MODULE  : Script.h
//
// PURPOSE : Implements the Script goal and state.
//
// CREATED : 8/23/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AIScript.h"
#include "AIScriptCommands.h"
#include "AI.h"
#include "AITarget.h"
#include "AINodeMgr.h"
#include "AINode.h"
#include "AIButeMgr.h"
#include "CVarTrack.h"
#include "AIUtils.h"
#include "CharacterMgr.h"

#include <string>

#include "AIActionMisc.h"

#ifdef _DEBUG
//#define SCRIPT_DEBUG
#endif

const char g_szIgnoreDamage[] = "IgnoreDamage";
const char g_szShortIgnoreDamage[] = "IGD";

#ifndef _FINAL

bool ShouldShowScript(HOBJECT hObject)
{
	ASSERT( g_pLTServer );

#ifdef SCRIPT_DEBUG
	static CVarTrack vtShowAIScript(g_pLTServer,"ShowAIScript",1.0f);
#else
	static CVarTrack vtShowAIScript(g_pLTServer,"ShowAIScript",0.0f);
#endif

	return vtShowAIScript.GetFloat() > 0.0f && ShouldShow(hObject);
}

#endif

/////////////////////////////////////////////////
//
//     Script Goal
//
/////////////////////////////////////////////////


ScriptGoal::ScriptGoal( CAI * pAI, std::string create_name, LTBOOL bIsIdleScript /* = LTFALSE */ )
	: Goal(pAI, create_name ),
	  m_pCurrentCommand(LTNULL),
	  m_bSetFacing(LTFALSE),
	  m_SetFacing(pAI),
	  m_bFireWeapon(LTFALSE),
	  m_pAnim(pAI ? pAI->GetAnimation() : LTNULL),
	  m_bIsIdleScript(bIsIdleScript),
	  m_bIgnoreDamage(LTFALSE),
	  m_fLastDamageTime(-1.0f)
{
	_ASSERT( pAI );
	if( !pAI ) 
	{
#ifndef _FINAL
		g_pLTServer->CPrint("Script not attached to an AI!");
#endif
		return;
	}

	_ASSERT( m_pAnim );
	if( !m_pAnim ) 
	{
#ifndef _FINAL
		AIErrorPrint(pAI->m_hObject, "AI does not have an animatiom manager!");
#endif
		return;
	}
}


bool ScriptGoal::HandleParameter(const char * const * pszArgs, int nArgs)
{
	if( Goal::HandleParameter(pszArgs,nArgs) )  
		return true;

	if( 0 == stricmp(g_szIgnoreDamage,*pszArgs) )
	{
		m_bIgnoreDamage = LTTRUE;

#ifndef _FINAL
		if( nArgs != 1 )
		{
			AIErrorPrint(GetAIPtr()->m_hObject, "Script goal given too many arguments for parameter %s.  Did you forget a semi-colon?",
				g_szIgnoreDamage);
		}
#endif
		return true;
	}

	// Deal with script commands.
	if( nArgs == 1 )
	{
		ParseScript(pszArgs[0]);
	}
	else
	{
		AddCommand( pszArgs, nArgs );
	}

	return false;
}

bool ScriptGoal::HandleCommand(const char * const * pszArgs, int nArgs)
{
	if( Goal::HandleCommand(pszArgs,nArgs) ) 
		return true;

	if ( !_stricmp(pszArgs[0], g_szAddToScript) )
	{
		_ASSERT( nArgs == 2 );
		if( nArgs != 2 ) 
		{
#ifndef _FINAL
			if( GetAIPtr() )
			{
				std::string message = "Incorrect command arguments for command %s :";
				for(const char * const * pszCurrentArg = pszArgs+1; pszCurrentArg < pszArgs + nArgs; ++pszCurrentArg)
				{
					message += " ";
					message += *pszCurrentArg;
				}
					
				AIErrorPrint(GetAIPtr()->m_hObject, const_cast<char*>(message.c_str()),
					g_szAddToScript);
				AIErrorPrint(GetAIPtr()->m_hObject, "Usage : %s (command1 params; command2 params; ...)",
					g_szAddToScript);
			}		
#endif			
			return false;
		}

		ParseScript( pszArgs[1] );

		return true;
	}
	else if ( !_stricmp(pszArgs[0], g_szInsertIntoScript) )
	{
		_ASSERT( nArgs == 2 );
		if( nArgs != 2 ) 
		{
#ifndef _FINAL
			if( GetAIPtr() )
			{
				std::string message = "Incorrect command arguments for command %s :";
				for(const char * const * pszCurrentArg = pszArgs+1; pszCurrentArg < pszArgs + nArgs; ++pszCurrentArg)
				{
					message += " ";
					message += *pszCurrentArg;
				}
					
				AIErrorPrint(GetAIPtr()->m_hObject, const_cast<char*>(message.c_str()),
					g_szInsertIntoScript );
				AIErrorPrint(GetAIPtr()->m_hObject, "Usage : %s (command1 params; command2 params; ...)",
					g_szInsertIntoScript );
			}		
#endif			
			return false;
		}

		ParseScript( pszArgs[1], true );

		return true;
	}

	return false;
}

void ScriptGoal::Update()
{
	Goal::Update();

	// Set up the current command if we don't have
	// one or if it is done.
	if( !m_pCurrentCommand || m_pCurrentCommand->IsDone() )
	{
		// Clean up the current command.
		if( m_pCurrentCommand )
		{
			m_pCurrentCommand->End();
			delete m_pCurrentCommand;
			m_pCurrentCommand = LTNULL;
		}

		// See if there is another command and start it up if there is.
		if( !commands.empty() )
		{
			m_pCurrentCommand = commands.front();
			commands.pop_front();

			_ASSERT( m_pCurrentCommand );
			if( m_pCurrentCommand ) m_pCurrentCommand->Start();

#ifndef _FINAL
			if( ShouldShowScript(GetAIPtr()->m_hObject) && m_pCurrentCommand )
			{
				AICPrint( GetAIPtr()->m_hObject, "Starting script command: %s",
					m_pCurrentCommand->GetCommandDescription() );
			}
#endif
		}
	}

	// Update the current command and keep us awake.
	if( m_pCurrentCommand )
	{
		m_pCurrentCommand->Update();

		// This will keep us from de-activating when we are outside the PVS.
		g_pLTServer->SetDeactivationTime(GetAIPtr()->m_hObject, c_fAIDeactivationTime);

	}

	// Face target if so requested.
	if( m_bSetFacing && GetAIPtr()->CanDoNewAction() )
	{
		m_SetFacing.Update();
	}

	// Fire weapon, if so requested.
	if( m_bFireWeapon && m_SetFacing.Finished() && GetAIPtr()->CanDoNewAction() )
	{
		const CWeapon * pWeapon = GetAIPtr()->GetCurrentWeaponPtr();
		const AIWBM_AIWeapon * pWeaponButes = GetAIPtr()->GetCurrentWeaponButesPtr();

		if( pWeapon && pWeaponButes )
		{
			if( NeedReload(*pWeapon, *pWeaponButes) )
			{
				GetAIPtr()->SetNextAction(CAI::ACTION_RELOAD);
				return;
			}
			else if( GetAIPtr()->BurstDelayExpired() )
			{
				if (!GetAIPtr()->WillHurtFriend())
				{
					GetAIPtr()->SetNextAction( CAI::ACTION_FIREWEAPON );
					return;
				}
			}
		}
	}
}

void ScriptGoal::Start()
{ 
	Goal::Start();

	GetAIPtr()->Walk();
	GetAIPtr()->SetMovementStyle();
	GetAIPtr()->SetNextAction(CAI::ACTION_IDLE);
	GetAIPtr()->ClearAim();

	m_fLastDamageTime = GetAIPtr()->GetDestructible()->GetLastDamageTime();

	// flashlight is always off
	if (GetAIPtr()->IsFlashlightOn())
		GetAIPtr()->SetFlashlight(LTFALSE);
}

void ScriptGoal::End()
{
	Goal::End();

	GetAIPtr()->Walk();
	StopPlayingAnimation();
	GetAIPtr()->SetMovementStyle();
	GetAIPtr()->ClearAim();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScriptGoal::GetBid
//
//	PURPOSE:	Takes a list of script commands and adds each one.
//
// ----------------------------------------------------------------------- //
bool ScriptGoal::ParseScript(const char * szScript, bool insert /* = false */)
{
	bool bParsedSomething = false;

	ConParse parse( const_cast<char*>(szScript) );

	// Iterate through each script command.
	while(g_pLTServer->Common()->Parse(&parse) == LT_OK)
	{
		bParsedSomething = AddCommand( parse.m_Args, parse.m_nArgs, insert) || bParsedSomething;
	}

	return bParsedSomething;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScriptGoal::HandleParameter
//
//	PURPOSE:	Creates a command out of a pre-parsed set of tokens.
//
// ----------------------------------------------------------------------- //

static std::string MakeUpperString(const char * value)
{
	std::string result = value;

	for( std::string::iterator iter = result.begin();
	     iter != result.end(); ++iter )
	{
		*iter = toupper( *iter );
	}

	return result;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScriptGoal::AddCommand
//
//	PURPOSE:	Adds a command to the script.  
//				Ideally this would return false if the command was not added,
//				but because anything which is not a known script command
//				is interpreted as a "msgself" command, it will always return true.
//
// ----------------------------------------------------------------------- //

bool ScriptGoal::AddCommand(const char * const * pszArgs, int nArgs, bool insert /* = false */)
{
	_ASSERT( nArgs > 0 );

	if( nArgs > 0 )
	{

		if( 0 == stricmp(g_szShortIgnoreDamage, *pszArgs) )
		{
			m_bIgnoreDamage = LTTRUE;

#ifndef _FINAL
			if( nArgs > 1 )
			{
				AIErrorPrint(GetAIPtr(), "Command \"%s\" given arguments!  All arguments will be ignored (it just needs the keyword).",
					g_szShortIgnoreDamage);
			}
#endif
			return true;
		}
		else
		{
			CAIScriptCommandFactory CommandFactory("");

			// Try to construct a command instance.
			CAIScriptCommand * command_ptr = CommandFactory.NewAbstract( MakeUpperString(pszArgs[0]).c_str() );

			if( command_ptr )
			{
				// If we can initialize the command, put it in the queue to be performed.
				std::string error = command_ptr->Init( this, GetAIPtr(), &pszArgs[1], nArgs - 1 );
				if( error.empty() )
				{
					if( insert )
					{
						commands.push_front(command_ptr);
						return true;
					}
					else
					{
						commands.push_back(command_ptr);
						return true;
					}
				}
				else
				{
#ifndef _FINAL
					AIErrorPrint(GetAIPtr(),const_cast<char*>(error.c_str()));
#endif
					return true;
				}
			}
			else
			{
				// The command was not a script command.
				// Assume it was a 
				command_ptr = CommandFactory.NewAbstract( "MSGSELF" );
				_ASSERT( command_ptr );

				if( command_ptr )
				{
					std::string error = command_ptr->Init( this, GetAIPtr(), pszArgs, nArgs );
					if( error.empty() )
					{
						if( insert )
						{
							commands.push_front(command_ptr);
							return true;
						}
						else
						{
							commands.push_back(command_ptr);
							return true;
						}

					}
					else
					{
#ifndef _FINAL
						AIErrorPrint(GetAIPtr(),const_cast<char*>(error.c_str()));
#endif
						return true;
					}
				}
			}
		}

	} //if( nArgs > 0 )

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScriptGoal::Face
//
//	PURPOSE:	Set-up to control our facing.
//
// ----------------------------------------------------------------------- //
void ScriptGoal::Face(LTBOOL val, LTBOOL bUseWeapon /* = LTFALSE */ )
{
	m_bSetFacing = val;

	if( val )
	{
		m_SetFacing.UseWeaponPosition(bUseWeapon);
	}
}

void ScriptGoal::FaceTarget(LTBOOL val, LTBOOL bUseWeapon /* = LTFALSE */ )
{
	m_bSetFacing = val;

	if( val ) 
	{
		m_SetFacing.FaceTarget(); 
		m_SetFacing.UseWeaponPosition(bUseWeapon);
	}
}

void ScriptGoal::Face(LTBOOL val, HOBJECT hObj, LTBOOL bUseWeapon /* = LTFALSE */ )
{ 
	m_bSetFacing = val; 
	if( val ) 
	{
		m_SetFacing.Face(hObj); 
		m_SetFacing.UseWeaponPosition(bUseWeapon);
	}
}

void ScriptGoal::Face(LTBOOL val, const LTVector & vPos, LTBOOL bUseWeapon /* = LTFALSE */ )
{ 
	m_bSetFacing = val; 
	if( val )
	{
		m_SetFacing.Face(vPos); 
		m_SetFacing.UseWeaponPosition(bUseWeapon);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScriptGoal::FireAt
//
//	PURPOSE:	Set-up functions to get us firing at something.
//
// ----------------------------------------------------------------------- //

void ScriptGoal::FireAt(LTBOOL val)
{ 
	m_bFireWeapon = val;  
	Face(val, val); 
	if( !m_bFireWeapon ) 
		GetAIPtr()->ClearAim(); 
}

void ScriptGoal::FireAtTarget(LTBOOL val)
{ 
	m_bFireWeapon = val;  
	FaceTarget(val, val); 
	if( !m_bFireWeapon ) 
		GetAIPtr()->ClearAim(); 
}

void ScriptGoal::FireAt(LTBOOL val, HOBJECT hObj)
{
	m_bFireWeapon = val;
	Face(val, hObj, val);
	if( !m_bFireWeapon )
		GetAIPtr()->ClearAim();
}

void ScriptGoal::FireAt(LTBOOL val, const LTVector & vPos)
{
	m_bFireWeapon = val;
	Face(val,vPos, val);
	if( !m_bFireWeapon )
		GetAIPtr()->ClearAim();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScriptGoal::XXXAnimation
//
//	PURPOSE:	Deal with playing an animation.
//
// ----------------------------------------------------------------------- //
void ScriptGoal::PlayAnimation(const char * anim_name,  bool bLoop /* = false */, bool bInterruptible /* = false */)
{
	_ASSERT(m_pAnim);
	if( !m_pAnim ) return;

	// If an interruptible animation is requested and 
	// we are already playing an animation, stop it
	// in case it is the same animation.  That way
	// the animation will restart.
	if( !bLoop )
	{
	}

	AIActionPlayAnim * pPlayAnim = dynamic_cast<AIActionPlayAnim *>(GetAIPtr()->SetNextAction(CAI::ACTION_PLAYANIM));
	if (pPlayAnim)
	{
		uint32 nFlags = 0;
		if( bLoop )
			nFlags |= AIActionPlayAnim::FLAG_LOOP;
		else
			nFlags |= AIActionPlayAnim::FLAG_RESET;  // Reset non-looping animations if they are already playing.

		if( bInterruptible )
			nFlags |= AIActionPlayAnim::FLAG_INTER;

		pPlayAnim->Init(anim_name, nFlags);
	}
}

void ScriptGoal::StopPlayingAnimation()
{
	GetAIPtr()->SetNextAction(CAI::ACTION_IDLE);
}

bool ScriptGoal::FinishedAnimation() const
{
	_ASSERT(m_pAnim);
	if( !m_pAnim ) return true;

	return !m_pAnim->UsingAltAnim();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScriptGoal::GetBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT ScriptGoal::GetBid()
{
	const LTFLOAT fDefault = g_pAIButeMgr->GetDefaultBid(BID_SCRIPT);
	const LTFLOAT fIdleDefault = g_pAIButeMgr->GetDefaultBid(BID_IDLESCRIPT);

	LTFLOAT fBid = fDefault;  // ***** THIS IS OPPOSITE THE OTHER GOALS *****
	if( m_bIsIdleScript )
		fBid = fIdleDefault;

	// Check for recent damage.
	if( IsActive() && !m_bIgnoreDamage)
	{
		CDestructible * const pDestructible = GetAIPtr()->GetDestructible();
		if( pDestructible && pDestructible->GetLastDamageTime() > m_fLastDamageTime )
		{
			CCharacter * pChar = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(pDestructible->GetLastDamager()) );
			if( pChar && g_pCharacterMgr->AreEnemies(*GetAIPtr(),*pChar) )
			{
				fBid = 0.0f;
			}
		}
	}

	return fBid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScriptGoal::GetBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT ScriptGoal::GetMaximumBid()
{
	if( m_bIsIdleScript )
		return g_pAIButeMgr->GetDefaultBid(BID_IDLESCRIPT);

	return g_pAIButeMgr->GetDefaultBid(BID_SCRIPT);
}
	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScriptGoal::Load/Save
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void ScriptGoal::Load(HMESSAGEREAD hRead)
{
	ASSERT(hRead);
	if (!hRead) return;

	Goal::Load(hRead);

	commands.clear();
	const uint32 num_commands = hRead->ReadWord();
	for(uint32 i = 0; i < num_commands; ++i)
	{
		CAIScriptCommand * pCommand = LoadScriptCommand(*hRead, this, GetAIPtr());
		_ASSERT( pCommand );
		if( pCommand ) 
			commands.push_back(pCommand);
	}

	if( hRead->ReadByte() )
	{
		m_pCurrentCommand = LoadScriptCommand(*hRead, this, GetAIPtr());
	}

	*hRead >> m_bSetFacing;
	*hRead >> m_bFireWeapon;
	*hRead >> m_SetFacing;

	*hRead >> m_bIgnoreDamage;
	m_fLastDamageTime = g_pLTServer->GetTime() - hRead->ReadFloat();

}


void ScriptGoal::Save(HMESSAGEREAD hWrite)
{
	ASSERT(hWrite);
	if (!hWrite) return;

	Goal::Save(hWrite);
	
	hWrite->WriteWord(commands.size());

	for(CommandQueue::iterator iter = commands.begin(); iter != commands.end(); ++iter)
	{
		SaveScriptCommand( *hWrite, *(*iter) );
	}

	hWrite->WriteByte( m_pCurrentCommand != LTNULL );
	if( m_pCurrentCommand != LTNULL )
		SaveScriptCommand( *hWrite, *m_pCurrentCommand );

	*hWrite << m_bSetFacing;
	*hWrite << m_bFireWeapon;
	*hWrite << m_SetFacing;

	// m_pAnim set in init.

	*hWrite << m_bIgnoreDamage;
	hWrite->WriteFloat(g_pLTServer->GetTime() - m_fLastDamageTime);
}


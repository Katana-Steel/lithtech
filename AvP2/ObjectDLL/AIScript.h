// ----------------------------------------------------------------------- //
//
// MODULE  : AIScript.h
//
// PURPOSE : Implements the Script goal and state.
//
// CREATED : 8/23/00
//
// ----------------------------------------------------------------------- //

#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include "Goal.h"
#include "AIStrategySetFacing.h"

#include <deque>

class CAIScriptCommand;
class CharacterAnimation;

class ScriptGoal : public Goal
{
public :

	typedef std::deque<CAIScriptCommand*> CommandQueue;


public :

	ScriptGoal( CAI * pAI, std::string create_name, LTBOOL bIsIdleScript = LTFALSE );

	virtual void Load(HMESSAGEREAD hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	virtual bool HandleParameter(const char * const * pszArgs, int nArgs);
	virtual bool HandleCommand(const char * const * pszArgs, int nArgs);

	virtual void Update();
	virtual void Start();
	virtual void End();

	virtual bool DestroyOnEnd() const { return true; }
#ifndef _FINAL
	virtual const char * GetDescription() const { return m_bIsIdleScript ? "IdleScript" : "Script"; }
#endif
	
	virtual LTFLOAT GetBid();
	virtual LTFLOAT GetMaximumBid();

	virtual CMusicMgr::EMood GetMusicMoodModifier( ) { return m_bIsIdleScript ? CMusicMgr::eMoodAmbient : CMusicMgr::eMoodNone; }

	const char * GetRandomSoundName() const { return m_bIsIdleScript ? "RandomIdle" : LTNULL; } 


	void Face(LTBOOL val, LTBOOL bUseWeapon= LTFALSE);
	void FaceTarget(LTBOOL val, LTBOOL bUseWeapon = LTFALSE);
	void Face(LTBOOL val, HOBJECT hObj, LTBOOL bUseWeapon = LTFALSE);
	void Face(LTBOOL val, const LTVector & vPos, LTBOOL bUseWeapon = LTFALSE);
	
	bool FinishedFacing() const { return (m_bSetFacing == LTFALSE) || m_SetFacing.Finished(); }

	bool IsFacing() const		{ return m_bSetFacing == LTTRUE; }
	bool IsFacingTarget() const { return (m_bSetFacing == LTTRUE) && m_SetFacing.IsFacingTarget(); }
	bool IsFacingObject() const { return (m_bSetFacing == LTTRUE) && m_SetFacing.IsFacingObject(); }
	bool IsFacingPos() const    { return (m_bSetFacing == LTTRUE) && m_SetFacing.IsFacingPos(); }

	const LTSmartLink & GetFacingObject() const { return m_SetFacing.GetFacingObject(); }
	const LTVector &    GetFacingPos()    const { return m_SetFacing.GetFacingPos(); }

	void FireAt(LTBOOL val);
	void FireAtTarget(LTBOOL val);
	void FireAt(LTBOOL val, HOBJECT hObj);
	void FireAt(LTBOOL val, const LTVector & vPos);
	
	bool IsFiring() const { return (m_bFireWeapon == LTTRUE) && FinishedFacing(); }

	void PlayAnimation(const char * anim_name, bool bLoop = false, bool bInterruptible = false );
	void PlayLoopingAnimation(const char * anim_name) { PlayAnimation(anim_name,true); }
	void PlayInterruptibleAnimation(const char * anim_name) { PlayAnimation(anim_name,false, true); }
	void StopPlayingAnimation();
	bool FinishedAnimation() const;


	// These can be used to add commands to the script.
	bool AddToScript(const char * szScript)      { return ParseScript(szScript,false); }
	bool InsertIntoScript(const char * szScript) { return ParseScript(szScript,true); }


	// For debugging.
	const CAIScriptCommand * GetCurrentCommand() const { return m_pCurrentCommand; }

protected :

	bool ParseScript(const char * szScript, bool insert = false);
	bool AddCommand(const char * const * pszArgs, int nArgs, bool insert = false);

private :

	CommandQueue  commands; 
	CAIScriptCommand * m_pCurrentCommand;

	LTBOOL m_bSetFacing;
	LTBOOL m_bFireWeapon;
	CAIStrategySetFacing m_SetFacing;

	CharacterAnimation * m_pAnim;

	LTBOOL         m_bIsIdleScript;
	LTBOOL		   m_bIgnoreDamage;
	LTFLOAT		   m_fLastDamageTime;
};


const char g_szAddToScript[] = "SCR+";
const char g_szInsertIntoScript[] = "SCR*";

#ifndef _FINAL
bool ShouldShowScript(HOBJECT hObject);
#endif

#endif // __SCRIPT_H__

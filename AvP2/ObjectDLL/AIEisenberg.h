// ----------------------------------------------------------------------- //
//
// MODULE  : AIEisenberg.h
//
// PURPOSE : AIEisenberg object
//
// CREATED : 04/09/01
//
// ----------------------------------------------------------------------- //

#ifndef __AIEISENBERG_H__
#define __AIEISENBERG_H__

#include "AI.h"

class AIEisenberg : public CAI
{
	public :

		AIEisenberg();
		virtual ~AIEisenberg();


		virtual Goal * CreateGoal(const char * szGoalName);

		virtual bool CanOpenDoor() const { return false; }

		virtual void	UpdateActionAnimation();

	protected :

		virtual void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		virtual void	InitialUpdate();
		virtual void	PostPropRead(ObjectCreateStruct *pStruct);

		virtual void	ProcessDamageMsg(const DamageStruct & damage_msg);
		virtual LTBOOL	ProcessCommand(const char*const* pTokens, int nArgs);


	private :

};

#endif // __AIEISENBERG_H__

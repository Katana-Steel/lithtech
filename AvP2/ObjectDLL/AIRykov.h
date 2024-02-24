// ----------------------------------------------------------------------- //
//
// MODULE  : AIRykov.h
//
// PURPOSE : AIRykov object
//
// CREATED : 04/09/01
//
// ----------------------------------------------------------------------- //

#ifndef __AIRYKOV_H__
#define __AIRYKOV_H__

#include "AI.h"
#include "CharacterAlignment.h"

class AIRykov : public CAI
{
	public :

		AIRykov();
		virtual ~AIRykov();


		virtual Goal * CreateGoal(const char * szGoalName);

		virtual bool CanOpenDoor() const { return false; }

		virtual void	UpdateActionAnimation();

	protected :

		virtual void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		virtual void	InitialUpdate();
		virtual void	PostPropRead(ObjectCreateStruct *pStruct);

	private :

};

#endif // __AIRYKOV_H__

// ----------------------------------------------------------------------- //
//
// MODULE  : AIPlayer.h
//
// PURPOSE : AIPlayer object
//
// CREATED : 1/1/01
//
// ----------------------------------------------------------------------- //

#ifndef __AIPLAYER_H__
#define __AIPLAYER_H__

#include "AI.h"

class CPlayerObj;

class AIPlayer : public CAI
{
	public : // Public methods

		// Ctors/Dtors/etc

		AIPlayer();
		virtual ~AIPlayer();

		void SetupForSwap(const CPlayerObj & player);

		virtual uint32	GetPredatorEnergy() { return 1000; }

		virtual void	UpdateActionAnimation();

		virtual bool CanOpenDoor() const;

	protected : // Protected methods


		virtual void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		virtual bool Consistent() { return true; }

	private : // Member Variables

};


#endif   //__AIPLAYER_H__

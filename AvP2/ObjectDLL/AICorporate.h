// ----------------------------------------------------------------------- //
//
// MODULE  : AICorporate.h
//
// PURPOSE : AICorporate object
//
// CREATED : 7/6/00
//
// ----------------------------------------------------------------------- //

#ifndef __AICORPORATE_H__
#define __AICORPORATE_H__

#include "AI.h"
#include "CharacterAlignment.h"
#include "AINode.h"

class AICorporate : public CAI
{
	public : // Public methods

		// Ctors/Dtors/etc

		AICorporate();
		virtual ~AICorporate();

	protected : // Protected methods

		virtual void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);


	private : // Member Variables

};


class CAICorporatePlugin : public CAIPlugin
{
public :

	virtual bool IsValidCharacterClass(CharacterClass x) const { return x == CORPORATE || x == SYNTHETIC; }
	virtual bool CanUseWeapon(const AIWBM_AIWeapon & butes) const;
};

#endif // __AICORPORATE_H__

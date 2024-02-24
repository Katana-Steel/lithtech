// ----------------------------------------------------------------------- //
//
// MODULE  : AIMarine.h
//
// PURPOSE : AIMarine object
//
// CREATED : 7/6/00
//
// ----------------------------------------------------------------------- //

#ifndef __AIMARINE_H__
#define __AIMARINE_H__

#include "AI.h"
#include "CharacterAlignment.h"

class AIMarine : public CAI
{
	public : // Public methods

		// Ctors/Dtors/etc

		AIMarine();
		virtual ~AIMarine();

	protected : // Protected methods


		virtual void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);


	private : // Member Variables

};


class CAIMarinePlugin : public CAIPlugin
{
public :

	virtual bool IsValidCharacterClass(CharacterClass x) const { return x == MARINE || x == MARINE_EXOSUIT; }
	virtual bool CanUseWeapon(const AIWBM_AIWeapon & butes) const;
};

#endif // __AIMARINE_H__

// ----------------------------------------------------------------------- //
//
// MODULE  : AIExosuit.h
//
// PURPOSE : AIExosuit object
//
// CREATED : 1/1/01
//
// ----------------------------------------------------------------------- //

#ifndef __AIEXOSUIT_H__
#define __AIEXOSUIT_H__

#include "AI.h"
#include "CharacterAlignment.h"

class AIExosuit : public CAI
{
	public : // Public methods

		// Ctors/Dtors/etc

		AIExosuit();
		virtual ~AIExosuit();

		virtual bool AllowSnapTurn() const { return false; }
		virtual LTVector GetAimFromPosition() const;
	
	protected : // Protected methods

		virtual DDWORD	EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT lData);

		virtual void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		virtual void	InitialUpdate();

		HOBJECT			CreateExosuitHead(uint32 nHeadSet);

		LTBOOL			AddWindshield();	// returns LTTRUE on success
		void			LaunchWindshield();	// blasts Windshield off

	private : // Member Variables

		HOBJECT			m_hWindshield;
};


class CAIExosuitPlugin : public CAIPlugin
{
public :

	virtual bool IsValidCharacterClass(CharacterClass x) const { return x == CORPORATE_EXOSUIT; }
	virtual bool CanUseWeapon(const AIWBM_AIWeapon & butes) const;
};

#endif // __AIEXOSUIT_H__

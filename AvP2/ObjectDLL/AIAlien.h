// ----------------------------------------------------------------------- //
//
// MODULE  : AIAlien.h
//
// PURPOSE : AIAlien object
//
// CREATED : 5/19/00
//
// ----------------------------------------------------------------------- //

#ifndef __AIALIEN_H__
#define __AIALIEN_H__

#include "AI.h"
#include "CharacterAlignment.h"

class AIAlien : public CAI
{
	public : // Public methods

		// Ctors/Dtors/etc

		AIAlien();
		virtual ~AIAlien();


		virtual Goal * CreateGoal(const char * szGoalName);

		virtual bool CanOpenDoor() const { return false; }

		virtual void	UpdateActionAnimation();

	protected : // Protected methods


		virtual void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		virtual void	InitialUpdate();
		virtual LTBOOL	ReadProp(ObjectCreateStruct *pInfo);
		virtual void	PostPropRead(ObjectCreateStruct *pStruct);

		virtual LTBOOL IgnoreDamageMsg(const DamageStruct& damage);

	private : // Member Variables

		LTBOOL		m_bUseDumbAttack;
		std::string m_strDisplayAnim;
};


class CAIAlienPlugin : public CAIPlugin
{
public :

	virtual bool IsValidCharacterClass(CharacterClass x) const { return x == ALIEN; }
	virtual bool CanUseWeapon(const AIWBM_AIWeapon & butes) const;

};

#endif // __AIALIEN_H__

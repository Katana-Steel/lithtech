// ----------------------------------------------------------------------- //
//
// MODULE  : AIPredator.h
//
// PURPOSE : AIPredator object
//
// CREATED : 7/6/00
//
// ----------------------------------------------------------------------- //

#ifndef __AIPREDATOR_H__
#define __AIPREDATOR_H__

#include "AI.h"
#include "CharacterAlignment.h"
#include "AINode.h"

class AIPredator : public CAI
{
	public : // Public methods

		// Ctors/Dtors/etc

		AIPredator();
		virtual ~AIPredator();

	protected : // Protected methods

		virtual LTBOOL	ReadProp(ObjectCreateStruct *pInfo);

		virtual DDWORD	EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT lData);

		virtual void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		virtual uint32	GetPredatorEnergy() { return 1000; }

	private : // Member Variables

		LTBOOL	m_bStartCloaked;
		LTBOOL	m_bStartWithMask;
};


class CAIPredatorPlugin : public CAIPlugin
{
public :

	virtual bool IsValidCharacterClass(CharacterClass x) const { return x == PREDATOR; }
	virtual bool CanUseWeapon(const AIWBM_AIWeapon & butes) const;

};


// Predator with Special Death.  This overrides the default death animation and
// allows the level designer to set a custom one for BodyProp::Setup() to use.
class AIPredatorSD : public AIPredator
{
	public:

		virtual HOBJECT	CreateBody(LTBOOL bCarryOverAttachments = LTTRUE);

	protected:

		virtual LTBOOL	ReadProp(ObjectCreateStruct *pInfo);
		virtual void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

	private:

		std::string		m_sDeathAni;
};

#endif // __AIPREDATOR_H__

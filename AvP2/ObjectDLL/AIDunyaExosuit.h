// ----------------------------------------------------------------------- //
//
// MODULE  : AIDunyaExosuit.h
//
// PURPOSE : AIDunyaExosuit object
//
// CREATED : 04/09/01
//
// ----------------------------------------------------------------------- //

#ifndef __AIDUNYAEXOSUIT_H__
#define __AIDUNYAEXOSUIT_H__

#include "AI.h"
#include "AIExosuit.h"

class AIDunyaExosuit : public AIExosuit
{
	public :

		AIDunyaExosuit();
		virtual ~AIDunyaExosuit();

		virtual Goal * CreateGoal(const char * szGoalName);
		virtual bool CanOpenDoor() const { return false; }
		virtual void	UpdateActionAnimation();

	protected :

		virtual void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		virtual void	InitialUpdate();
		virtual void	PostPropRead(ObjectCreateStruct *pStruct);

		LTFLOAT			GetHealthRatio();

		virtual LTBOOL	IgnoreDamageMsg(const DamageStruct & damage_msg);
		virtual void	ProcessDamageMsg(const DamageStruct & damage_msg);
		virtual LTBOOL	ProcessCommand(const char*const* pTokens, int nArgs);


	private :

		void			PlaySmokeFX(LTBOOL bOn);

		int				m_nNumHits;
		LTBOOL			m_bMaskOpen;
		LTBOOL			m_bHealthMeterOn;
		CTimer			m_tmrInvulnerable;
};

#endif // __AIDUNYAEXOSUIT_H__

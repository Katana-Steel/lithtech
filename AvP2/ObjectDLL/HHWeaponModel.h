// ----------------------------------------------------------------------- //
//
// MODULE  : CHHWeaponModel.h
//
// PURPOSE : CHHWeaponModel definition - Hand Held weapon model support
//
// CREATED : 10/31/97
//
// ----------------------------------------------------------------------- //

#ifndef __HH_WEAPON_MODEL_H__
#define __HH_WEAPON_MODEL_H__

#include "cpp_engineobjects_de.h"

class CWeapon;

class CHHWeaponModel : public BaseClass
{
	public :

		CHHWeaponModel();
		virtual ~CHHWeaponModel();

		void SetParent(CWeapon* pParent);
		CWeapon* GetParent()     const { return m_pParent; }

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

	private :

		CWeapon*	m_pParent;			// Weapon we're associated with

		void	InitialUpdate();
		void	StringKey(ArgList* pArgList);
};

#endif // __HH_WEAPON_MODEL_H__


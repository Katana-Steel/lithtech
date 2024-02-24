// ----------------------------------------------------------------------- //
//
// MODULE  : PredNet.h
//
// PURPOSE : Predator Net class - definition
//
// CREATED : 9/5/2000
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PREDNET_H__
#define __PREDNET_H__

#include "ProjectileTypes.h"

struct PREDNETCLASSDATA;

class CPredNet : public CGrenade
{
	public :

		CPredNet();
		~CPredNet();

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

	protected:
		//overridden virtual functions
		void HandleTouch(HOBJECT hObj);
		void UpdateGrenade();
		void Detonate(HOBJECT hObj, LTBOOL bDidProjectileImpact = LTTRUE);

		void Setup(const ProjectileSetup & setup, const WFireInfo & info);

	private:
		//member functions
		void	CreateNetDebris();
		LTBOOL	m_bRemoved;
};

#endif //  __PREDNET_H__

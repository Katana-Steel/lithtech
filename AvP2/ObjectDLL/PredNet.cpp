// ----------------------------------------------------------------------- //
//
// MODULE  : PredNet.h
//
// PURPOSE : Predator Net Weapon class - implementation
//
// CREATED : 9/5/2000
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PredNet.h"
#include "CharacterHitBox.h"
#include "Character.h"
#include "DebrisMgr.h"
#include "DebrisFuncs.h"

BEGIN_CLASS(CPredNet)
END_CLASS_DEFAULT_FLAGS(CPredNet, CGrenade, LTNULL, LTNULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredNet::CPredNet
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPredNet::CPredNet() : CGrenade()
{
	m_bRemoved = LTFALSE;
}	


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredNet::~CPredNet
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CPredNet::~CPredNet()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredNet::CreateNetDebris
//
//	PURPOSE:	Gib ourself...
//
// ----------------------------------------------------------------------- //

void CPredNet::CreateNetDebris()
{
	DEBRIS* pDebris = g_pDebrisMgr->GetDebris((char*) GetAmmoName() );
	if (pDebris)
	{
		LTVector vVel;
		LTVector vPos;

		g_pLTServer->GetVelocity(m_hObject, &vVel);
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		vVel.Norm();

		vVel = -vVel;
		CreatePropDebris(vPos, vVel, pDebris->nId);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredNet::Detonate
//
//	PURPOSE:	Time to finish up...
//
// ----------------------------------------------------------------------- //

void CPredNet::Detonate(HOBJECT hObj, LTBOOL bDidProjectileImpact)
{
	// Gib and be gone
	CreateNetDebris();

	RemoveObject();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredNet::HandleTouch
//
//	PURPOSE:	Handle hitting stuff
//
// ----------------------------------------------------------------------- //

void CPredNet::HandleTouch(HOBJECT hObj)
{
	//if we are hitting ourself or our hitbox return
	if (hObj == GetFiredFrom() || m_bRemoved) return;

	CCharacterHitBox* pHitBox = LTNULL;
	if (IsCharacterHitBox(hObj))
	{
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hObj);
		if (!pHitBox) return;

		hObj = pHitBox->GetModelObject();
		if (hObj == GetFiredFrom()) return;
	}
	else if (IsCharacter(hObj))
	{
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hObj);
		HOBJECT hHitBox = pChar->GetHitBox();
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hHitBox);
		if (!pHitBox) return;
	}
	else
	{
		// Well we didn't hit a character so just make sure
		// what we hit is solid.
		uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);

		if(!(dwFlags & FLAG_SOLID))
			return;
	}

	if(pHitBox)
	{
		// Hit!  See if this character can be netted
		CCharacter* pCharacter = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hObj));
		if(!pCharacter) return;

		if(g_pCharacterButeMgr->GetCanBeNetted(pCharacter->GetCharacter()))
		{
			// Ok, last check.  See if the victim is already netted...
			if(!pCharacter->IsNetted())
			{
				// Owned!
				LTVector vDir;

				g_pLTServer->GetVelocity(m_hObject, &vDir);
				vDir.Norm();
				pCharacter->HandleNetHit(vDir, GetFiredFrom());

				// Now just go away...
				g_pLTServer->RemoveObject(m_hObject);
				m_bRemoved = LTTRUE;
				return;
			}
		}
	}

	// If we get this far then we hit somthing solid, we should just gib...
	Detonate(hObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredNet::UpdateGrenade()
//
//	PURPOSE:	Update the net...
//
// ----------------------------------------------------------------------- //

void CPredNet::UpdateGrenade()
{
	// Don't do any of the normal grenade update stuff..
	return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredNet::Setup()
//
//	PURPOSE:	Setup the net...
//
// ----------------------------------------------------------------------- //

void CPredNet::Setup(const ProjectileSetup & setup, const WFireInfo & info)
{
	CGrenade::Setup(setup, info);

	//set trans.
	LTFLOAT fR, fG, fB, fA;
	g_pLTServer->GetObjectColor(m_hObject, &fR, &fG, &fB, &fA);
	g_pLTServer->SetObjectColor(m_hObject, fR, fG, fB, 0.99f);

	//turn off the looping
	g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CPredNet::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			// Get the current flags of the projectile
			uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);

			// Make some flag adjustments that we know we'll need
			dwFlags = dwFlags & ~FLAG_SOLID;

			g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
		}
		break;

		default : break;
	}

	return CGrenade::EngineMessageFn(messageID, pData, fData);
}

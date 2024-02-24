// ----------------------------------------------------------------------- //
//
// MODULE  : HUDWeaponHeat.h
//
// PURPOSE : HUDItem to display weapon heat meter
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_WEAPONHEAT_H
#define __HUD_WEAPONHEAT_H

#include "HUDItem.h"
#include "HUDBar.h"

//******************************************************************************************
//** HUD Air display
//******************************************************************************************
class CHUDWeaponHeat : public CHUDItem
{
public:
	CHUDWeaponHeat();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:

    LTIntPt		m_BasePos;

	int			m_nBarHeight;
    float		m_fBarScale;
	CHUDBar		m_Bar;

	bool		m_bDraw;

	LTVector	m_vCool;
	LTVector	m_vWarm;
	LTVector	m_vHot;
	uint32		m_argbOverheat;
};

#endif
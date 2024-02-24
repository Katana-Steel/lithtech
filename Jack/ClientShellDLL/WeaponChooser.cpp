// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponChooser.cpp
//
// PURPOSE : In-game popup for choosing weapons
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponChooser.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "WinUtil.h"
#include "SoundMgr.h"
#include "LayoutMgr.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponMgr.h"

extern CGameClientShell* g_pGameClientShell;


namespace
{
	const int kLastAmmo = 1;
	const int kCurrWeapon = 1;
	const int kCurrAmmo = 0;
	const float kfDelayTime	= 1.5f;
	VarTrack	g_vtChooserAutoSwitchTime;
	VarTrack	g_vtChooserAutoSwitchFreq;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWeaponChooser::CWeaponChooser()
{
	m_nWeapon = -1;
	m_nClass = 0;
    m_bIsOpen = false;
}

CWeaponChooser::~CWeaponChooser()
{
}

void CWeaponChooser::Init()
{
	g_vtChooserAutoSwitchTime.Init(g_pLTClient, "ChooserAutoSwitchTime", NULL, 0.175f);
	g_vtChooserAutoSwitchFreq.Init(g_pLTClient, "ChooserAutoSwitchFreq", NULL, 0.1f);
}

void CWeaponChooser::Term()
{
	if (m_bIsOpen)
		Close();
}

bool CWeaponChooser::Open(uint8 nClass)
{
	if (m_bIsOpen && nClass == m_nClass)
		return true;

	CClientWeaponMgr const *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
	IClientWeaponBase const *pClientWeapon = pClientWeaponMgr->GetCurrentClientWeapon();
	if( !pClientWeapon )
		return false;

	if (!m_bIsOpen)
		m_nWeapon = pClientWeapon->GetWeaponId();

	uint8 nWeapon = m_nWeapon;
	while( true )
	{
		// Get the next weapon.
		nWeapon = pClientWeaponMgr->GetNextWeaponId( nWeapon, nClass );
		WEAPON const* pWeapon = g_pWeaponMgr->GetWeapon( nWeapon );

		// If we couldn't find a weapon, we can't go to a next one.
		if( !pWeapon )
		{
			// If we didn't find a weapon and we're staying within our
			// current weapon's class, then just open the chooser.
			if( g_pWeaponMgr->GetWeaponClass(m_nWeapon) == nClass )
				break;

			if (!m_bIsOpen)
				m_nWeapon = -1;
			g_pHUDMgr->QueueUpdate(kHUDChooser);
			return false;
		}

		if( pWeapon->bShowChooser )
		{
			break;
		}
	}

	m_bIsOpen = true;
	m_nClass = nClass;

	m_AutoCloseTimer.Start(kfDelayTime);
	g_pHUDMgr->QueueUpdate(kHUDChooser);
    return true;
}

void CWeaponChooser::Close()
{
	if (!m_bIsOpen)
		return;

    m_bIsOpen = false;
	m_nClass = 0;
	m_AutoCloseTimer.Stop();
	m_AutoSwitchTimer.Stop();
	m_NextWeaponKeyDownTimer.Stop();
	m_PrevWeaponKeyDownTimer.Stop();

	if (g_pHUDMgr)
		g_pHUDMgr->QueueUpdate(kHUDChooser);

}

void CWeaponChooser::NextWeapon(uint8 nClass)
{
	if (nClass > g_pWeaponMgr->GetNumWeaponClasses())
		nClass = m_nClass;

	uint8 nWeapon = m_nWeapon;
	while( true )
	{
		// get the next avail weapon
		nWeapon = g_pPlayerMgr->GetClientWeaponMgr()->GetNextWeaponId( nWeapon, nClass );
		WEAPON const* pWeapon = g_pWeaponMgr->GetWeapon( nWeapon );

		// If we couldn't find a weapon, or we wrapped around, we can't find a next one.
		if( !pWeapon || nWeapon == m_nWeapon )
			return;

		if( pWeapon->bShowChooser )
		{
			m_nWeapon = nWeapon;
			break;
		}
	}

    g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());

   	m_AutoCloseTimer.Start(kfDelayTime);
	m_NextWeaponKeyDownTimer.Start(g_vtChooserAutoSwitchTime.GetFloat());
	g_pHUDMgr->QueueUpdate(kHUDChooser);

}

void CWeaponChooser::PrevWeapon()
{
	uint8 nWeapon = m_nWeapon;
	while( true )
	{
		// get the next avail weapon
		nWeapon = g_pPlayerMgr->GetClientWeaponMgr()->GetPrevWeaponId( nWeapon, 0 );
		WEAPON const* pWeapon = g_pWeaponMgr->GetWeapon( nWeapon );

		// If we couldn't find a weapon, or we wrapped around, we can't find a next one.
		if( !pWeapon || nWeapon == m_nWeapon )
			return;

		if( pWeapon->bShowChooser )
		{
			m_nWeapon = nWeapon;
			break;
		}
	}

    g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());

   	m_AutoCloseTimer.Start(kfDelayTime);
	m_PrevWeaponKeyDownTimer.Start(g_vtChooserAutoSwitchTime.GetFloat());
	g_pHUDMgr->QueueUpdate(kHUDChooser);
}

void CWeaponChooser::EndAutoSwitch(bool bNextWeaponKey)
{
	if (bNextWeaponKey)
		m_NextWeaponKeyDownTimer.Stop();
	else
		m_PrevWeaponKeyDownTimer.Stop();
	m_AutoSwitchTimer.Stop();
}

void CWeaponChooser::Update()
{
	// If Weapon chooser is being drawn, see if we want to change weapons...
	if (!IsOpen()) return;


	// See if we should close ourselves...
	if (m_AutoCloseTimer.On() && m_AutoCloseTimer.Stopped())
	{
        g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());
		Close();
	}
	else if (m_NextWeaponKeyDownTimer.On() && m_NextWeaponKeyDownTimer.Stopped())
	{
		// See if we should switch to the next weapon...
		if (m_AutoSwitchTimer.On())
		{
			if (m_AutoSwitchTimer.Stopped())
			{
				NextWeapon(-1);
				g_pPlayerMgr->ChangeWeapon( GetCurrentSelection() );
				m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
			}
		}
		else
		{
			m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
		}
	}
	else if (m_PrevWeaponKeyDownTimer.On() && m_PrevWeaponKeyDownTimer.Stopped())
	{
		if (m_PrevWeaponKeyDownTimer.On())
		{
			if (m_AutoSwitchTimer.Stopped())
			{
				PrevWeapon();
				g_pPlayerMgr->ChangeWeapon( GetCurrentSelection());
				m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
			}
		}
		else
		{
			m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
		}
	}

}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAmmoChooser::CAmmoChooser()
{
	m_nAmmo = -1;
    m_bIsOpen = false;
	
}

CAmmoChooser::~CAmmoChooser()
{
}

void CAmmoChooser::Init()
{

}


void CAmmoChooser::Term()
{
	if (m_bIsOpen)
		Close();
}

bool CAmmoChooser::Open()
{
	// Don't allow the chooser to be opened if we're selecting/deselecting a
	// weapon...

	IClientWeaponBase *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();

	WeaponState eState = pClientWeapon->GetState();
	if (W_DESELECT == eState || W_SELECT == eState) return false;


	if (m_bIsOpen)
        return true;


	m_nAmmo = pClientWeapon->GetAmmoId();

	if (m_nAmmo == pClientWeapon->GetNextAvailableAmmo())
	{
		m_nAmmo = -1;
        m_bIsOpen = false;
        return false;
	}
    m_bIsOpen = true;

	m_AutoCloseTimer.Start(kfDelayTime);
	g_pHUDMgr->QueueUpdate(kHUDChooser);
    return true;
}

void CAmmoChooser::Close()
{
	if (!m_bIsOpen)
		return;
    m_bIsOpen = false;
	m_AutoCloseTimer.Stop();
	m_AutoSwitchTimer.Stop();
	m_NextAmmoKeyDownTimer.Stop();
	if (g_pHUDMgr)
		g_pHUDMgr->QueueUpdate(kHUDChooser);

}

void CAmmoChooser::NextAmmo()
{
	IClientWeaponBase *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
	if ( !pClientWeapon )
	{
		return;
	}

	m_nAmmo = pClientWeapon->GetNextAvailableAmmo(m_nAmmo);

    g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());

	m_AutoCloseTimer.Start(kfDelayTime);
	m_NextAmmoKeyDownTimer.Start(g_vtChooserAutoSwitchTime.GetFloat());
	g_pHUDMgr->QueueUpdate(kHUDChooser);
}

void CAmmoChooser::EndAutoSwitch()
{
	m_NextAmmoKeyDownTimer.Stop();
	m_AutoSwitchTimer.Stop();
}

void CAmmoChooser::Update()
{
	// If Weapon chooser is being drawn, see if we want to change weapons...
	if (!IsOpen()) return;


	// See if we should close ourselves...

	// See if we should switch to the next ammo type...
	if (m_AutoCloseTimer.On() && m_AutoCloseTimer.Stopped())
	{
        g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());
		Close();
	}
	else if (m_NextAmmoKeyDownTimer.On() && m_NextAmmoKeyDownTimer.Stopped())
	{
		if (m_AutoSwitchTimer.On())
		{
			if (m_AutoSwitchTimer.Stopped())
			{
				NextAmmo();
				m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
			}
		}
		else
		{
			m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
		}
	}



}

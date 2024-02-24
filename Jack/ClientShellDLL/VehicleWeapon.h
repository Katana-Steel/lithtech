// ----------------------------------------------------------------------- //
//
// MODULE  : VehicleWeapon.h
//
// PURPOSE : The VehicleWeapon object
//
// CREATED : 3/5/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VEHICLE_WEAPON_H__
#define __VEHICLE_WEAPON_H__

//
// Includes..
//

	#include "ClientWeapon.h"

class CVehicleWeapon : public CClientWeapon
{
	public :	// Methods...

		CVehicleWeapon( );
		virtual ~CVehicleWeapon( );

		virtual bool	Activate( );
		virtual void	Deactivate( );

		virtual void	InitAnimations( bool bAllowSelectOverride = false );

		// These should not be implemented for Vehcile Weapons...
		
		virtual void	ResetWeaponFilenames() {};
		virtual void	UpdateBob( LTFLOAT fWidth, LTFLOAT fHeight ) {};
		virtual void	Select() {};
		virtual bool	Deselect( ClientWeaponCallBackFn cbFn, void *pData ) { return false; }
		virtual void	SetVisible( bool bVis = true ) {};
		virtual void	SetDisable( bool bDisable = true ) { m_bDisabled = bDisable; }
		
		virtual void	SetVehicleWeaponModel( HOBJECT hObj );

		virtual void	SetVehicleAtFullSpeed( bool bFullSpeed ) { m_bVehicleAtFullSpeed = bFullSpeed; }
		virtual	void	SetVehicleBrakeOn( bool bBrake ) { m_bVehicleBrakeOn = bBrake; }

	protected : // Methods...

		virtual bool	CreateWeaponModel() { return false; }
		virtual void	RemoveWeaponModel()	{ m_hObject = NULL;	}

		virtual bool	PlaySelectAnimation() { return false; }
		virtual bool	PlayDeselectAnimation() { return false; }
		virtual bool	PlayReloadAnimation() { return false; }
		virtual bool	PlayIdleAnimation() { return false; }
		virtual bool	PlayFireAnimation( bool bResetAni );

		// pre fire animation
		virtual bool	IsPreFireAni( uint32 dwAni ) const;
		virtual uint32	GetPreFireAni() const;

		// fire animation
		virtual bool	IsFireAni( uint32 dwAni , bool bCheckNormalOnly = false) const;
		virtual uint32	GetFireAni( FireType eFireType=FT_NORMAL_FIRE ) const;

		// post fire
		virtual bool	IsPostFireAni( uint32 dwAni ) const;
		virtual uint32	GetPostFireAni() const;

	
	protected :	// Members...

		bool			m_bVehicleAtFullSpeed;
		HMODELANIM		m_nFullSpeedFireAnis[WM_MAX_FIRE_ANIS];	// Fire animations
		HMODELANIM		m_nFullSpeedPreFireAni;					// Optional animation to play before the actual fire ani
		HMODELANIM		m_nFullSpeedPostFireAni;				// Optional animation to play after the actual fire ani
		
		bool			m_bVehicleBrakeOn;
		HMODELANIM		m_nBrakeFireAnis[WM_MAX_FIRE_ANIS];		// Fire animations
		HMODELANIM		m_nBrakePreFireAni;						// Optional animation to play before the actual fire ani
		HMODELANIM		m_nBrakePostFireAni;					// Optional animation to play after the actual fire ani
};

#endif // __VEHICLE_WEAPON_H__
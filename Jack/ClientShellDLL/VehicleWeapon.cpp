// ----------------------------------------------------------------------- //
//
// MODULE  : VehicleWeapon.cpp
//
// PURPOSE : VehicleWeapon implementation
//
// CREATED : 3/5/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "VehicleWeapon.h"
	#include "PlayerViewAttachmentMgr.h"

HMODELANIM const INVALID_ANI = ( static_cast< HMODELANIM >( -1 ) );

static const char *s_szIdleAnimationBasename	= "Idle_";
static const char *s_szPreFireAnimationName		= "IdlePreFire";
static const char *s_szPostFireAnimationName	= "IdlePostFire";
static const char *s_szFireAnimationName		= "IdleFire";
static const char *s_szFireAnimationBasename	= "IdleFire";
static const char *s_szFullSpeedFireAniName		= "FullSpeedFire";
static const char *s_szFullSpeedPostFireAniName = "FullSpeedPostFire";
static const char *s_szFullSpeedPreFireAniName	= "FullSpeedPreFire";
static const char *s_szBrakeFireAniName			= "BrakeFire";
static const char *s_szBrakePostFireAniName		= "BrakePostFire";
static const char *s_szBrakePreFireAniName		= "BrakePreFire";	

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleWeapon::CVehicleWeapon
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CVehicleWeapon::CVehicleWeapon( )
:	CClientWeapon			( ),
	m_bVehicleAtFullSpeed	( false ),
	m_nFullSpeedPreFireAni	( INVALID_ANI ),
	m_nFullSpeedPostFireAni	( INVALID_ANI ),
	m_bVehicleBrakeOn		( false ),
	m_nBrakePreFireAni		( INVALID_ANI ),
	m_nBrakePostFireAni		( INVALID_ANI )
{
	int i = 0;

	for( i = 0; i < WM_MAX_FIRE_ANIS; ++i )
	{
		m_nFullSpeedFireAnis[i] = INVALID_ANI;
	}

	for( i = 0; i < WM_MAX_FIRE_ANIS; ++i )
	{
		m_nBrakeFireAnis[i] = INVALID_ANI;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleWeapon::~CVehicleWeapon
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CVehicleWeapon::~CVehicleWeapon( )
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleWeapon::Activate()
//
//	PURPOSE:	Activate the vehicle weapon...
//
// ----------------------------------------------------------------------- //

bool CVehicleWeapon::Activate()
{
    if( !m_pWeapon || !m_pAmmo ) return false;

	// reset any necessary data
	ResetData();

	// initialize the animations for this model
	InitAnimations();

	// create all client fx
	CreatePVAttachClientFX();
	CreateMuzzleFlash();
	CreateOverheatFx( );

	// Create Player-View attachments..
	
	g_pPVAttachmentMgr->CreatePVAttachments( m_hObject );

	SetState( W_IDLE );

    return true;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CVehicleWeapon::Deactivate()
//
//	PURPOSE:	Put the vehicle weapon into an inactive state...
//
// ----------------------------------------------------------------------- //

void CVehicleWeapon::Deactivate()
{
	// set the state to inactave
	SetState( W_INACTIVE );

	// turn off all keyframed ClientFX
	for ( CLIENTFX_LINK_NODE* pNode = m_KeyframedClientFX.m_pNext; pNode != NULL; pNode = pNode->m_pNext )
	{
		// turn off the effect
		g_pClientFXMgr->ShutdownClientFX( &pNode->m_Link );
	}

	// destroy the list of keyframed ClientFX
	m_KeyframedClientFX.DeleteList();

	// remove player-view attachments
	g_pPVAttachmentMgr->RemovePVAttachments();

	// remove all client fx
	RemoveMuzzleFlash();
	RemoveOverheatFx( );
	RemovePVAttachClientFX();

	KillLoopSound();
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CVehicleWeapon::SetVehicleWeaponModel()
//
//	PURPOSE:	Set the model to use for the vehicle weapon...
//
// ----------------------------------------------------------------------- //

void CVehicleWeapon::SetVehicleWeaponModel( HOBJECT hObj )
{
	m_hObject = hObj;
	
	// Make sure the model is can recieve model keys...

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Client, CF_NOTIFYMODELKEYS, CF_NOTIFYMODELKEYS );
}
 
// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CVehicleWeapon::InitAnimations()
//
//	PURPOSE:	Set the animations...
//
// ----------------------------------------------------------------------- //

void CVehicleWeapon::InitAnimations( bool bAllowSelectOverride /*= false*/ )
{
	ASSERT( 0 != m_hObject );

	m_nSelectAni			= INVALID_ANI;
	m_nDeselectAni			= INVALID_ANI;
	m_nReloadAni			= INVALID_ANI;
	
	m_nAltSelectAni			= INVALID_ANI;
	m_nAltDeselectAni		= INVALID_ANI;
	m_nAltDeselect2Ani		= INVALID_ANI;
	m_nAltReloadAni			= INVALID_ANI;
	
	m_nPreFireAni			= g_pLTClient->GetAnimIndex( m_hObject, s_szPreFireAnimationName );
	m_nPostFireAni			= g_pLTClient->GetAnimIndex( m_hObject, s_szPostFireAnimationName );
	m_nFullSpeedPreFireAni	= g_pLTClient->GetAnimIndex( m_hObject, s_szFullSpeedPreFireAniName );
	m_nFullSpeedPostFireAni	= g_pLTClient->GetAnimIndex( m_hObject, s_szFullSpeedPostFireAniName );
	m_nBrakePreFireAni		= g_pLTClient->GetAnimIndex( m_hObject, s_szBrakePreFireAniName );
	m_nBrakePostFireAni		= g_pLTClient->GetAnimIndex( m_hObject, s_szBrakePostFireAniName );

	char buf[30];
	int i;

	for( i = 0; i < WM_MAX_IDLE_ANIS; ++i )
	{
		sprintf( buf, "%s%d", s_szIdleAnimationBasename, i );
		m_nIdleAnis[ i ] = g_pLTClient->GetAnimIndex( m_hObject, buf );
	}

	for( i = 0; i < WM_MAX_FIRE_ANIS; ++i )
	{
		if( i > 0 )
		{
			sprintf( buf, "%s%d", s_szFireAnimationBasename, i );
		}
		else
		{
			sprintf( buf, s_szFireAnimationName );
		}

		m_nFireAnis[ i ] = g_pLTClient->GetAnimIndex( m_hObject, buf );
	}

	for( i = 0; i < WM_MAX_FIRE_ANIS; ++i )
	{
		if( i > 0 )
		{
			sprintf( buf, "%s%d", s_szFullSpeedFireAniName, i );
		}
		else
		{
			sprintf( buf, s_szFullSpeedFireAniName );
		}

		m_nFullSpeedFireAnis[ i ] = g_pLTClient->GetAnimIndex( m_hObject, buf );
	}

	for( i = 0; i < WM_MAX_FIRE_ANIS; ++i )
	{
		if( i > 0 )
		{
			sprintf( buf, "%s%d", s_szBrakeFireAniName, i );
		}
		else
		{
			sprintf( buf, s_szBrakeFireAniName );
		}

		m_nBrakeFireAnis[ i ] = g_pLTClient->GetAnimIndex( m_hObject, buf );
	}

	for( i = 0; i < WM_MAX_ALTIDLE_ANIS; ++i )
	{
		m_nAltIdleAnis[ i ] = INVALID_ANI;
	}

	for( i = 0; i < WM_MAX_ALTFIRE_ANIS; ++i )
	{
		m_nAltFireAnis[ i ] = INVALID_ANI;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleWeapon::PlayFireAnimation()
//
//	PURPOSE:	Set model to firing animation.  If the model has a PreFire animation
//				we will play that first and then play the Fire animation.  If the model
//				has a PostFire animation we will play that as soon as the Fire ani is done. 
//
// ----------------------------------------------------------------------- //

bool CVehicleWeapon::PlayFireAnimation( bool bResetAni )
{
	return CClientWeapon::PlayFireAnimation( bResetAni );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleWeapon::IsPreFireAni
//
//  PURPOSE:	Is the passed in animation a pre-fire animation
//
// ----------------------------------------------------------------------- //

bool CVehicleWeapon::IsPreFireAni( uint32 dwAni ) const
{
	if( INVALID_ANI == dwAni )
	{
		return false;
	}

	if( (dwAni == m_nPreFireAni) || (dwAni == m_nFullSpeedPreFireAni) || (dwAni == m_nBrakePreFireAni) )
	{
		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleWeapon::GetPreFireAni
//
//  PURPOSE:	Get the pre-fire animation
//
// ----------------------------------------------------------------------- //

uint32 CVehicleWeapon::GetPreFireAni( ) const
{
	if( m_bVehicleBrakeOn )
	{
		return m_nBrakePreFireAni;
	}
	else if( m_bVehicleAtFullSpeed )
	{
		return m_nFullSpeedPreFireAni;
	}
	
	return m_nPreFireAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleWeapon::IsFireAni()
//
//	PURPOSE:	Is the passed in animation any one of the fire animations
//
// ----------------------------------------------------------------------- //

bool CVehicleWeapon::IsFireAni( uint32 dwAni, bool bCheckNormalOnly /*= false*/) const
{
	if( INVALID_ANI == dwAni )
	{
		return false;
	}

	// First check normal fire anis...

	int i;
	for( i = 0; i < WM_MAX_FIRE_ANIS; ++i )
	{
		if ( m_nFireAnis[ i ] == dwAni )
		{
			return true;
		}
	}

	// Then FullSpeed fire anis...

	for( i = 0; i < WM_MAX_FIRE_ANIS; ++i )
	{
		if( m_nFullSpeedFireAnis[ i ] == dwAni )
		{
			return true;
		}
	}

	// Then Brake fire anis...

	for( i = 0; i < WM_MAX_FIRE_ANIS; ++i )
	{
		if( m_nBrakeFireAnis[ i ] == dwAni )
		{
			return true;
		}
	}

	// Finially Alt fire anis...

	for( i = 0; i < WM_MAX_ALTFIRE_ANIS; ++i )
	{
		if ( m_nAltFireAnis[ i ] == dwAni )
		{
			return true;
		}
	}

	// We want to see if the animation is a PreFire ani or PostFire ani because
	// they can be thought of as part of the entire Fire animation sequence.
	if ( !bCheckNormalOnly && (IsPreFireAni( dwAni ) || IsPostFireAni( dwAni )) )
	{
		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleWeapon::GetFireAni()
//
//	PURPOSE:	Get the fire animation based on the fire type
//
// ----------------------------------------------------------------------- //

uint32 CVehicleWeapon::GetFireAni( FireType eFireType ) const
{
	int nNumValid = 0;

	if( ((eFireType == FT_ALT_FIRE) && CanUseAltFireAnis()) ||
	     (m_bUsingAltFireAnis && (eFireType == FT_NORMAL_FIRE)) )
	{
		uint32 dwValidAltFireAnis[ WM_MAX_ALTFIRE_ANIS ];

		for( int i = 0; i < WM_MAX_ALTFIRE_ANIS; ++i )
		{
			if( INVALID_ANI != m_nAltFireAnis[ i ] )
			{
				dwValidAltFireAnis[ nNumValid ] = m_nAltFireAnis[ i ];
				++nNumValid;
			}
		}

		if ( nNumValid > 0 )
		{
			return dwValidAltFireAnis[ GetRandom( 0, (nNumValid - 1) ) ];
		}
	}
	else if( eFireType == FT_NORMAL_FIRE )
	{
		uint32 dwValidFireAnis[ WM_MAX_FIRE_ANIS ];

		for( int i = 0; i < WM_MAX_FIRE_ANIS; ++i )
		{
			// If the vehicle's brake is on grab the brake fire anis otherwise try the 
			// fullspeed anis and the finially the normal idle fire anis...

			if( m_bVehicleBrakeOn )
			{
				if( INVALID_ANI != m_nBrakeFireAnis[ i ] )
				{
					dwValidFireAnis[ nNumValid ] = m_nBrakeFireAnis[ i ];
					++nNumValid;
				}
			}
			else if( m_bVehicleAtFullSpeed )
			{
				if( INVALID_ANI != m_nFullSpeedFireAnis[ i ] )
				{
					dwValidFireAnis[ nNumValid ] = m_nFullSpeedFireAnis[ i ];
					++nNumValid;
				}
			}
			else
			{
				if( INVALID_ANI != m_nFireAnis[ i ] )
				{
					dwValidFireAnis[ nNumValid ] = m_nFireAnis[ i ];
					++nNumValid;
				}
			}
		}

		if ( nNumValid > 0 )
		{
			return dwValidFireAnis[ GetRandom( 0, ( nNumValid - 1 ) ) ];
		}
	}

	return INVALID_ANI;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleWeapon::IsPostFireAni
//
//  PURPOSE:	Is the passed in animation a post-fire animation
//
// ----------------------------------------------------------------------- //

bool CVehicleWeapon::IsPostFireAni( uint32 dwAni ) const
{
	if( INVALID_ANI == dwAni )
	{
		return false;
	}

	if( (dwAni == m_nPostFireAni) || (dwAni == m_nFullSpeedPostFireAni) || (dwAni == m_nBrakePostFireAni))
	{
		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleWeapon::GetPostFireAni
//
//  PURPOSE:	get the post-fire animation
//
// ----------------------------------------------------------------------- //

uint32 CVehicleWeapon::GetPostFireAni() const
{
	if( m_bVehicleBrakeOn )
	{
		return m_nBrakePostFireAni;
	}
	else if( m_bVehicleAtFullSpeed )
	{
		return m_nFullSpeedPostFireAni;
	}

	return m_nPostFireAni;
}

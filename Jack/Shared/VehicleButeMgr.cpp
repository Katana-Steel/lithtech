// ----------------------------------------------------------------------- //
//
// MODULE  : VehicleButeMgr.cpp
//
// PURPOSE : VehicleButeMgr implementation
//
// CREATED : 2/25/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "VehicleButeMgr.h"

//
// Defines...
//

	#define VBMGR_VEHICLE_TAG			"Vehicle"
										
	#define VBMGR_NAME					"Name"
	#define	VBMGR_MODEL					"Model"
	#define VBMGR_SKIN					"Skin"
	#define VBMGR_RENDERSTYLE			"RenderStyle"
	#define VBMGR_TRANSLUCENT			"Translucent"
	#define VBMGR_PVMODEL				"PVModel"
	#define VBMGR_OFFSET				"PVOffset"
	#define VBMGR_PVSKIN				"PVSkin"
	#define VBMGR_PVRENDERSTYLE			"PVRenderStyle"
	#define	VBMGR_STARTUPSND			"StartUpSnd"
	#define VBMGR_IDLESND				"IdleSnd"
	#define VBMGR_ACCELSND				"AccelSnd"
	#define VBMGR_DECELSND				"DecelSnd"
	#define VBMGR_BRAKESND				"BrakeSnd"
	#define	VBMGR_TURNOFFSND			"TurnOffSnd"
	#define VBMGR_SLOWIMPACTSND			"SlowImpactSnd"
	#define VBMGR_MEDIMPACTSND			"MedImpactSnd"
	#define VBMGR_FASTIMPACTSND			"FastImpactSnd"
	#define	VBMGR_MPLOOP				"MPLoop"
	#define VBMGR_VEHICLEWEAPON			"VehicleWeapon"
	#define VBMGR_CANSHOWCROSSHAIR		"CanShowCrosshair"
	#define	VBMGR_CONTOURAFFECTSROLL	"ContourAffectsRoll"
	#define VBMGR_SURFACEVEHICLETYPE	"SurfaceVehicleType"
	#define VBMGR_HASREVERSE			"HasReverse"
	#define VBMGR_MOTORCYCLE			"Motorcycle"

	// The variable below can be controlled in game through console vars...
	
	#define VBMGR_VEL					"Velocity"
	#define VBMGR_ACCEL					"Acceleration"
	#define VBMGR_ACCELTIME				"AccelerationTime"
	#define VBMGR_MAXDECEL				"MaxDeceleration"
	#define VBMGR_DECELTIME				"DecelerationTime"
	#define VBMGR_MAXBRAKE				"MaxBrake"
	#define VBMGR_BRAKETIME				"BrakeTime"
	#define VBMGR_STOPPEDPERCENT		"StoppedPercent"
	#define VBMGR_MAXSPEEDPERCENT		"MaxSpeedPercent"
	#define VBMGR_MINTURNACCEL			"MinTurnAccel"
	#define VBMGR_TURNRATE				"TurnRate"
	#define VBMGR_INITIALTURNRATE		"InitialTurnRate"
	#define	VBMGR_SURFACESPEEDUPPITCH	"SurfaceSpeedUpPitch"
	#define VBMGR_SURFACESLOWDOWNPITCH	"SurfaceSlowDownPitch"
	#define VBMGR_STARTMOTIONPITCH		"StartMotionPitch"
	#define VBMGR_STOPMOTIONPITCH		"StopMotionPitch"
	#define VBMGR_BRAKEPITCH			"BrakePitch"
	#define VBMGR_DECELPITCH			"DecelPitch"
	#define VBMGR_IMPACTANGLESENSITIVITY	"ImpactAngleSensitivity"

	#define VBMGR_FADEFX				"MPFadeFX"
	#define VBMGR_SPAWNFX				"MPSpawnFX"
	#define VBMGR_UNSPAWNFX				"MPUnspawnFX"

//
// Globals...
//

	CVehicleButeMgr *g_pVehicleButeMgr = LTNULL;

	static char s_aTagName[30];
	static char s_aAttName[100];
	static char s_aBuffer[100];


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleButeMgr::CVehicleButeMgr
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CVehicleButeMgr::CVehicleButeMgr()
:	CGameButeMgr()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleButeMgr::Instance()
//
//	PURPOSE:	Instatiator of singleton
//
//  This function is the only way to instatiate this object.  It
//  ensures that there is only one object, the singleton.
//
// ----------------------------------------------------------------------- //

CVehicleButeMgr& CVehicleButeMgr::Instance( )
{
	// Putting the singleton as a static function variable ensures that this
	// object is only created if it is used.
	static CVehicleButeMgr sSingleton;
	return sSingleton;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleButeMgr::~CVehicleButeMgr
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CVehicleButeMgr::~CVehicleButeMgr()
{
	Term( );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleButeMgr::Term
//
//  PURPOSE:	Clean up after ourselfs...
//
// ----------------------------------------------------------------------- //

void CVehicleButeMgr::Term( )
{
	g_pVehicleButeMgr = LTNULL;
	
	VehicleList::iterator iter;
	for( iter = m_lstVehicles.begin(); iter != m_lstVehicles.end(); ++iter )
	{
		debug_delete( *iter );
	}
	
	m_lstVehicles.clear( );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleButeMgr::Init
//
//  PURPOSE:	Init the mgr...
//
// ----------------------------------------------------------------------- //

LTBOOL CVehicleButeMgr::Init( const char *szAttributeFile )
{
	if( g_pVehicleButeMgr || !szAttributeFile ) return LTFALSE;
	if( !Parse( szAttributeFile )) return LTFALSE;

	// Set the singelton ptr

	g_pVehicleButeMgr = this;

	// Go through the list of vehicles and init the ones that are buted...

	int nNumVehicles = 0;
	LTSNPrintF( s_aTagName, ARRAY_LEN(s_aTagName), "%s%d", VBMGR_VEHICLE_TAG, nNumVehicles );

	while( m_buteMgr.Exist( s_aTagName ))
	{
		VEHICLE *pVehicle = debug_new( VEHICLE );
		
		if( pVehicle && pVehicle->Init( m_buteMgr, s_aTagName ))
		{
			// Set the ID and add it to the list...
			
			pVehicle->nId = nNumVehicles;
			m_lstVehicles.push_back( pVehicle );
		}
		else
		{
			debug_delete( pVehicle );
			return LTFALSE;
		}

		++nNumVehicles;
		LTSNPrintF( s_aTagName, ARRAY_LEN(s_aTagName), "%s%d", VBMGR_VEHICLE_TAG, nNumVehicles );
	}
		
	// Free up the bute mgr's memory...

	m_buteMgr.Term();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleButeMgr::GetVehicle
//
//  PURPOSE:	Get the specified Vehicle record...
//
// ----------------------------------------------------------------------- //

VEHICLE* CVehicleButeMgr::GetVehicle( uint8 nId )
{
	if( IsValidVehicleId( nId ))
		return m_lstVehicles[nId];

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleButeMgr::GetVehicle
//
//  PURPOSE:	Get the specified Vehicle record...
//
// ----------------------------------------------------------------------- //

VEHICLE* CVehicleButeMgr::GetVehicle( const char *pName )
{
	if( !pName ) return LTNULL;

	VehicleList::const_iterator iter;
	for( iter = m_lstVehicles.begin(); iter != m_lstVehicles.end(); ++iter )
	{
		if( !_stricmp( (*iter)->sName.c_str(), pName ))
		{
			return *iter;
		}
	}
	
	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleButeMgr::SaveToFile
//
//  PURPOSE:	Write data back out to the bute file...
//
// ----------------------------------------------------------------------- //

bool CVehicleButeMgr::SaveToFile( VEHICLE *pVehicle /*= NULL*/ )
{
	// Re-init our bute mgr...

	m_buteMgr.Init( GBM_DisplayError );
	Parse( VBMGR_DEFAULT_FILE );

	// Just save the one vehicle if thats all we want...

	if( pVehicle )
	{
		pVehicle->Save( m_buteMgr );

	}
	else
	{
		// Save every vehicle record...

		VehicleList::const_iterator iter;
		for( iter = m_lstVehicles.begin(); iter != m_lstVehicles.end(); ++iter )
		{
			(*iter)->Save( m_buteMgr );
		}
	}

	// Save the file and free the memory...

	bool bSaved = m_buteMgr.Save();
	m_buteMgr.Term();

	return bSaved;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	VEHICLE::VEHICLE
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

VEHICLE::VEHICLE()
:	nId						( VBMGR_INVALID_ID ),
	bTranslucent			( false ),
	fVel					( 0.0f ),
	fAccel					( 0.0f ),
	fAccelTime				( 0.0f ),
	fMaxDecel				( 0.0f ),
	fDecelTime				( 0.0f ),
	fMaxBrake				( 0.0f ),
	fBrakeTime				( 0.0f ),
	fStoppedPercent			( 0.0f ),
	fMaxSpeedPercent		( 0.0f ),
	fMinTurnAccel			( 0.0f ),
	fTurnRate				( 0.0f ),
	fInitialTurnRate		( 0.0f ),
	fSurfaceSpeedUpPitch	( 0.0f ),
	fSurfaceSlowDownPitch	( 0.0f ),
	fStartMotionPitch		( 0.0f ),
	fStopMotionPitch		( 0.0f ),
	fBrakePitch				( 0.0f ),
	fDecelPitch				( 0.0f ),
	fImpactAngleSensitivity	( 1.0f ),
	vOffset					( 0.0f, 0.0f, 0.0f ),
	bCanShowCrosshair		( false ),
	bContourAffectsRoll		( true ),
	nSurfaceVehicleType		( -1 )
{
	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	VEHICLE::VEHICLE
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

VEHICLE::~VEHICLE()
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	VEHICLE::Init
//
//  PURPOSE:	Build the Vehicle record...
//
// ----------------------------------------------------------------------- //

bool VEHICLE::Init( CButeMgr &ButeMgr, char *aTagName )
{
	if( !aTagName ) return false;
	
	// The name of the record should be the name of the tag...
	
	sName			= ButeMgr.GetString( aTagName, VBMGR_NAME, "" );
	sModel			= ButeMgr.GetString( aTagName, VBMGR_MODEL, "" );
	sPVModel		= ButeMgr.GetString( aTagName, VBMGR_PVMODEL, "" );
	sStartUpSnd		= ButeMgr.GetString( aTagName, VBMGR_STARTUPSND, "" );
	sIdleSnd		= ButeMgr.GetString( aTagName, VBMGR_IDLESND, "" );
	sAccelSnd		= ButeMgr.GetString( aTagName, VBMGR_ACCELSND, "" );
	sDecelSnd		= ButeMgr.GetString( aTagName, VBMGR_DECELSND, "" );
	sBrakeSnd		= ButeMgr.GetString( aTagName, VBMGR_BRAKESND, "" );
	sTurnOffSnd		= ButeMgr.GetString( aTagName, VBMGR_TURNOFFSND, "" );
	sSlowImpactSnd	= ButeMgr.GetString( aTagName, VBMGR_SLOWIMPACTSND, "" );
	sMedImpactSnd	= ButeMgr.GetString( aTagName, VBMGR_MEDIMPACTSND, "" );
	sFastImpactSnd	= ButeMgr.GetString( aTagName, VBMGR_FASTIMPACTSND, "" );
	sMPLoop			= ButeMgr.GetString( aTagName, VBMGR_MPLOOP, "" );
	sVehicleWeapon	= ButeMgr.GetString( aTagName, VBMGR_VEHICLEWEAPON, "" );
	bHasReverse		= !!ButeMgr.GetInt( aTagName, VBMGR_HASREVERSE, 0 );
	bMotorcycle	= !!ButeMgr.GetInt( aTagName, VBMGR_MOTORCYCLE, 0 );

	blrSkins.Read( &ButeMgr, aTagName, VBMGR_SKIN, VBMGR_MAX_FILE_PATH );
	blrRenderStyles.Read( &ButeMgr, aTagName, VBMGR_RENDERSTYLE, VBMGR_MAX_FILE_PATH );
	blrPVSkins.Read( &ButeMgr, aTagName, VBMGR_PVSKIN, VBMGR_MAX_FILE_PATH );
	blrPVRenderStyles.Read( &ButeMgr, aTagName, VBMGR_PVRENDERSTYLE, VBMGR_MAX_FILE_PATH );

	bTranslucent = ButeMgr.GetBool( aTagName, VBMGR_TRANSLUCENT, false );

	fVel					= (float)ButeMgr.GetDouble( aTagName, VBMGR_VEL, 0.0f );
	fAccel					= (float)ButeMgr.GetDouble( aTagName, VBMGR_ACCEL, 0.0f );
	fAccelTime				= (float)ButeMgr.GetDouble( aTagName, VBMGR_ACCELTIME, 0.0f );
	fMaxDecel				= (float)ButeMgr.GetDouble( aTagName, VBMGR_MAXDECEL, 0.0f );
	fDecelTime				= (float)ButeMgr.GetDouble( aTagName, VBMGR_DECELTIME, 0.0f );
	fMaxBrake				= (float)ButeMgr.GetDouble( aTagName, VBMGR_MAXBRAKE, 0.0f );
	fBrakeTime				= (float)ButeMgr.GetDouble( aTagName, VBMGR_BRAKETIME, 0.0f );
	fStoppedPercent			= (float)ButeMgr.GetDouble( aTagName, VBMGR_STOPPEDPERCENT, 0.0f );
	fMaxSpeedPercent		= (float)ButeMgr.GetDouble( aTagName, VBMGR_MAXSPEEDPERCENT, 0.0f );
	fMinTurnAccel			= (float)ButeMgr.GetDouble( aTagName, VBMGR_MINTURNACCEL, 0.0f );
	fTurnRate				= (float)ButeMgr.GetDouble( aTagName, VBMGR_TURNRATE, 0.0f );
	fInitialTurnRate		= (float)ButeMgr.GetDouble( aTagName, VBMGR_INITIALTURNRATE, 0.0f );
	fSurfaceSpeedUpPitch	= (float)ButeMgr.GetDouble( aTagName, VBMGR_SURFACESPEEDUPPITCH, 0.0f );
	fSurfaceSlowDownPitch	= (float)ButeMgr.GetDouble( aTagName, VBMGR_SURFACESLOWDOWNPITCH, 0.0f );
	fStartMotionPitch		= (float)ButeMgr.GetDouble( aTagName, VBMGR_STARTMOTIONPITCH, 0.0f );
	fStopMotionPitch		= (float)ButeMgr.GetDouble( aTagName, VBMGR_STOPMOTIONPITCH, 0.0f );
	fBrakePitch				= (float)ButeMgr.GetDouble( aTagName, VBMGR_BRAKEPITCH, 0.0f );
	fDecelPitch				= (float)ButeMgr.GetDouble( aTagName, VBMGR_DECELPITCH, 0.0f );
	fImpactAngleSensitivity	= (float)ButeMgr.GetDouble( aTagName, VBMGR_IMPACTANGLESENSITIVITY, 1.0f );

	vOffset				= ButeMgr.GetVector( aTagName, VBMGR_OFFSET, CAVector( 0.0f, 0.0f, 0.0f ));
	bCanShowCrosshair	= !!ButeMgr.GetInt( aTagName, VBMGR_CANSHOWCROSSHAIR, false );
	bContourAffectsRoll	= !!ButeMgr.GetInt( aTagName, VBMGR_CONTOURAFFECTSROLL, false );
	nSurfaceVehicleType	= ButeMgr.GetInt( aTagName, VBMGR_SURFACEVEHICLETYPE, -1 );


	sFadeFX	= ButeMgr.GetString( aTagName, VBMGR_FADEFX, "" );
	sSpawnFX	= ButeMgr.GetString( aTagName, VBMGR_SPAWNFX, "" );
	sUnspawnFX	= ButeMgr.GetString( aTagName, VBMGR_UNSPAWNFX, "" );

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	VEHICLE::Save
//
//  PURPOSE:	Save any necessary data to the bute file...
//
// ----------------------------------------------------------------------- //

void VEHICLE::Save( CButeMgr &ButeMgr ) const
{
	LTSNPrintF( s_aTagName, ARRAY_LEN( s_aTagName ), "%s%d", VBMGR_VEHICLE_TAG, nId );

	ButeMgr.SetDouble( s_aTagName, VBMGR_VEL, fVel );
	ButeMgr.SetDouble( s_aTagName, VBMGR_ACCEL, fAccel );
	ButeMgr.SetDouble( s_aTagName, VBMGR_ACCELTIME, fAccelTime );
	ButeMgr.SetDouble( s_aTagName, VBMGR_MAXDECEL, fMaxDecel );
	ButeMgr.SetDouble( s_aTagName, VBMGR_DECELTIME, fDecelTime );
	ButeMgr.SetDouble( s_aTagName, VBMGR_MAXBRAKE, fMaxBrake );
	ButeMgr.SetDouble( s_aTagName, VBMGR_BRAKETIME, fBrakeTime );
	ButeMgr.SetDouble( s_aTagName, VBMGR_STOPPEDPERCENT, fStoppedPercent );
	ButeMgr.SetDouble( s_aTagName, VBMGR_MAXSPEEDPERCENT, fMaxSpeedPercent );
	ButeMgr.SetDouble( s_aTagName, VBMGR_MINTURNACCEL, fMinTurnAccel );
	ButeMgr.SetDouble( s_aTagName, VBMGR_TURNRATE, fTurnRate );
	ButeMgr.SetDouble( s_aTagName, VBMGR_INITIALTURNRATE, fInitialTurnRate );
	ButeMgr.SetDouble( s_aTagName, VBMGR_SURFACESPEEDUPPITCH, fSurfaceSpeedUpPitch );
	ButeMgr.SetDouble( s_aTagName, VBMGR_SURFACESLOWDOWNPITCH, fSurfaceSlowDownPitch );
	ButeMgr.SetDouble( s_aTagName, VBMGR_STARTMOTIONPITCH, fStartMotionPitch );
	ButeMgr.SetDouble( s_aTagName, VBMGR_STOPMOTIONPITCH, fStopMotionPitch );
	ButeMgr.SetDouble( s_aTagName, VBMGR_BRAKEPITCH, fBrakePitch );
	ButeMgr.SetDouble( s_aTagName, VBMGR_DECELPITCH, fDecelPitch );
	ButeMgr.SetDouble( s_aTagName, VBMGR_IMPACTANGLESENSITIVITY, fImpactAngleSensitivity );

	CAVector vAVec( vOffset.x, vOffset.y, vOffset.z );
	ButeMgr.SetVector( s_aTagName, VBMGR_OFFSET, vAVec );
}
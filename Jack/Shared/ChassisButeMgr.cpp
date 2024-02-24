// ----------------------------------------------------------------------- //
//
// MODULE  : ChassisButeMgr.cpp
//
// PURPOSE : ChassisButeMgr implementation
//
// CREATED : 5/9/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "ChassisButeMgr.h"

//
// Defines...
//


#define CBMGR_CHASSISPIECE_TAG			"ChassisPiece"
									
#define CBMGR_NAME					"Name"
#define CBMGR_PROPTYPE				"PropType"
#define CBMGR_PROPTYPE_TARGET		"PropType_Target"
#define CBMGR_PROPTYPE_REPLACED		"PropType_Replaced"
#define CBMGR_CHASSISSOCKET			"ChassisSocket"
#define CBMGR_TYPE					"Type"
#define CBMGR_HEAVY					"Heavy"
#define CBMGR_RADARTYPE				"RadarType"
#define CBMGR_CARRYINGICON			"CarryingIcon"
#define CBMGR_PICKUPICON			"PickupIcon"
#define CBMGR_FADE					"FadeFX"
#define CBMGR_SPAWN					"SpawnFX"
#define CBMGR_UNSPAWN				"UnspawnFX"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ChassisButeMgr::ChassisButeMgr
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

ChassisButeMgr::ChassisButeMgr()
:	CGameButeMgr()
{
	m_bInitialized = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisButeMgr::Instance()
//
//	PURPOSE:	Instatiator of singleton
//
//  This function is the only way to instatiate this object.  It
//  ensures that there is only one object, the singleton.
//
// ----------------------------------------------------------------------- //

ChassisButeMgr& ChassisButeMgr::Instance( )
{
	// Putting the singleton as a static function variable ensures that this
	// object is only created if it is used.
	static ChassisButeMgr sSingleton;
	return sSingleton;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ChassisButeMgr::~ChassisButeMgr
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

ChassisButeMgr::~ChassisButeMgr()
{
	Term( );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ChassisButeMgr::Term
//
//  PURPOSE:	Clean up after ourselfs...
//
// ----------------------------------------------------------------------- //

void ChassisButeMgr::Term( )
{
	ChassisPieceButeList::iterator iter;
	for( iter = m_lstChassisPieceButes.begin(); iter != m_lstChassisPieceButes.end(); ++iter )
	{
		debug_delete( *iter );
	}
	
	m_lstChassisPieceButes.clear( );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ChassisButeMgr::Init
//
//  PURPOSE:	Init the mgr...
//
// ----------------------------------------------------------------------- //

LTBOOL ChassisButeMgr::Init( const char *szAttributeFile )
{
	if( m_bInitialized ) 
		return LTFALSE;

	if( !szAttributeFile || !szAttributeFile[0] )
		szAttributeFile = CHASSIS_BUTE_FILE;

	if( !Parse( szAttributeFile )) return LTFALSE;

	// Go through the list of chassispieces and init the ones that are buted...

	int nNumChassisPieceButes = 0;
	char szTagName[256];

	while( true )
	{
		LTSNPrintF( szTagName, ARRAY_LEN(szTagName), "%s%d", CBMGR_CHASSISPIECE_TAG, nNumChassisPieceButes );
		if( !m_buteMgr.Exist( szTagName ))
			break;

		ChassisPieceBute *pChassisPieceBute = debug_new( ChassisPieceBute );
		
		if( pChassisPieceBute && pChassisPieceBute->Init( m_buteMgr, szTagName, nNumChassisPieceButes ))
		{
			m_lstChassisPieceButes.push_back( pChassisPieceBute );
		}
		else
		{
			debug_delete( pChassisPieceBute );
			return LTFALSE;
		}

		++nNumChassisPieceButes;
	}
		
	// Free up the bute mgr's memory...

	m_buteMgr.Term();

	m_bInitialized = true;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ChassisButeMgr::GetChassisPieceBute
//
//  PURPOSE:	Get the specified ChassisPieceBute record...
//
// ----------------------------------------------------------------------- //

ChassisPieceBute* ChassisButeMgr::GetChassisPieceBute( const char* pszName )
{
	if( !pszName || !pszName[0] )
		return LTNULL;

	ChassisPieceButeList::const_iterator iter;
	for( iter = m_lstChassisPieceButes.begin(); iter != m_lstChassisPieceButes.end(); ++iter )
	{
		ChassisPieceBute* pChassisPieceBute = (*iter);
		if( !_stricmp( pChassisPieceBute->m_sName.c_str(), pszName ))
		{
			return pChassisPieceBute;
		}
	}
	
	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ChassisButeMgr::GetChassisPieceButeIndex
//
//  PURPOSE:	Get the specified ChassisPieceBute index.  Returns CHASSISPIECEBUTE_INVALID
//				if not found.
//
// ----------------------------------------------------------------------- //

uint8 ChassisButeMgr::GetChassisPieceButeIndex( const char* pszName )
{
	ChassisPieceBute* pChassisPieceBute = GetChassisPieceBute( pszName );
	if( pChassisPieceBute )
		return pChassisPieceBute->m_nIndex;

	return CHASSISPIECEBUTE_INVALID;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ChassisPieceBute::ChassisPieceBute
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

ChassisPieceBute::ChassisPieceBute()
{
	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ChassisPieceBute::ChassisPieceBute
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

ChassisPieceBute::~ChassisPieceBute()
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ChassisPieceBute::Init
//
//  PURPOSE:	Build the Vehicle record...
//
// ----------------------------------------------------------------------- //

bool ChassisPieceBute::Init( CButeMgr &buteMgr, char const* pszTagName, uint8 nIndex )
{
	if( !pszTagName || !pszTagName[0] )
		return false;
	
	m_sName			= buteMgr.GetString( pszTagName, CBMGR_NAME, "" );
	m_sPropType		= buteMgr.GetString( pszTagName, CBMGR_PROPTYPE, "" );
	m_sPropType_Target		= buteMgr.GetString( pszTagName, CBMGR_PROPTYPE_TARGET, "" );
	m_sPropType_Replaced		= buteMgr.GetString( pszTagName, CBMGR_PROPTYPE_REPLACED, "" );
	m_sChassisSocket		= buteMgr.GetString( pszTagName, CBMGR_CHASSISSOCKET, "" );
	m_bHeavy		= !!buteMgr.GetBool( pszTagName, CBMGR_HEAVY, FALSE );
	m_sRadarType = buteMgr.GetString( pszTagName, CBMGR_RADARTYPE );
	m_sCarryingIcon = buteMgr.GetString( pszTagName, CBMGR_CARRYINGICON );
	m_sPickupIcon = buteMgr.GetString( pszTagName, CBMGR_PICKUPICON );
	m_sFadeFX = buteMgr.GetString( pszTagName, CBMGR_FADE );
	m_sSpawnFX = buteMgr.GetString( pszTagName, CBMGR_SPAWN );
	m_sUnspawnFX = buteMgr.GetString( pszTagName, CBMGR_UNSPAWN );
	m_nIndex = nIndex;
	
	return true;
}
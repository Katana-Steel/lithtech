// ----------------------------------------------------------------------- //
//
// MODULE  : ChassisPiece.cpp
//
// PURPOSE : The object used for placing on a Chassis Object.
//
// CREATED : 5/12/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "ParsedMsg.h"
#include "PlayerObj.h"
#include "Chassis.h"
#include "ChassisPiece.h"
#include "ChassisButeMgr.h"

//
// Globals...
//
	
	

LINKFROM_MODULE( ChassisPiece );

BEGIN_CLASS( ChassisPiece )

	ADD_STRINGPROP_FLAG( Type, "", PF_HIDDEN )
	ADD_STRINGPROP_FLAG( ChassisPieceType, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS )
	ADD_REALPROP( RespawnTime, 0.0f )

END_CLASS_DEFAULT_FLAGS_PLUGIN( ChassisPiece, PropType, NULL, NULL, 0, CChassisPiecePlugin )


// Register with the CommandMgr...

CMDMGR_BEGIN_REGISTER_CLASS( ChassisPiece )
CMDMGR_END_REGISTER_CLASS( ChassisPiece, PropType )



LTRESULT CChassisPiecePlugin::PreHook_EditStringList(
	const char *szRezPath,
	const char *szPropName,
	char **aszStrings,
	uint32 *pcStrings,
	const uint32 cMaxStrings,
	const uint32 cMaxStringLen )
{
	// Handle type...
	if (_strcmpi("ChassisPieceType", szPropName) == 0)
	{
		if( !ChassisButeMgr::Instance( ).IsInitialized( ))
		{
			std::string sFile = szRezPath;
			sFile += "\\";
			sFile += CHASSIS_BUTE_FILE;

			ChassisButeMgr::Instance().SetInRezFile( LTFALSE );
			ChassisButeMgr::Instance().Init( sFile.c_str( ));
		}

		ChassisButeMgr::ChassisPieceButeList::const_iterator iter = ChassisButeMgr::Instance( ).GetChassisPieceButeList( ).begin( );
		for( ; iter != ChassisButeMgr::Instance( ).GetChassisPieceButeList( ).end( ); iter++ )
		{
			ASSERT( cMaxStrings > (*pcStrings) + 1 );
			if( cMaxStrings > (*pcStrings) + 1 )
			{
				ChassisPieceBute const* pChassisPieceBute = *iter;
				if( pChassisPieceBute && !pChassisPieceBute->m_sName.empty( ))
				{
					LTStrCpy( aszStrings[(*pcStrings)++], pChassisPieceBute->m_sName.c_str(), cMaxStringLen );
				}
			}
		}

		qsort( aszStrings, *pcStrings, sizeof( char * ), CaseInsensitiveCompare );

		return LT_OK;
	}
	else if( LT_OK == CPropTypePlugin::PreHook_EditStringList( szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLen ))
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT CChassisPiecePlugin::PreHook_Dims(
			const char* szRezPath,
			const char* szPropValue,
			char* szModelFilenameBuf,
			int	  nModelFilenameBufLen,
			LTVector & vDims)
{
	if( !ChassisButeMgr::Instance( ).IsInitialized( ))
	{
		std::string sFile = szRezPath;
		sFile += "\\";
		sFile += CHASSIS_BUTE_FILE;

		ChassisButeMgr::Instance().SetInRezFile( LTFALSE );
		ChassisButeMgr::Instance().Init( sFile.c_str( ));
	}

	ChassisPieceBute const* pChassisPieceBute = ChassisButeMgr::Instance( ).GetChassisPieceBute( szPropValue );
	if( pChassisPieceBute )
	{
		strcpy(szModelFilenameBuf, pChassisPieceBute->m_sPropType.c_str( ));

		// Need to convert the .ltb filename to one that DEdit understands...

		ConvertLTBFilename(szModelFilenameBuf);
		return LT_OK;
	}
	else if( LT_OK == CPropTypePlugin::PreHook_Dims( szRezPath, szPropValue, szModelFilenameBuf, nModelFilenameBufLen, vDims ))
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ChassisPiece::ChassisPiece
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

ChassisPiece::ChassisPiece( )
:	PropType		( ),
	m_hChassis		( LTNULL ),
	m_vOriginalPos	( 0.0f, 0.0f, 0.0f ),
	m_bCanBeCarried	( true ),
	m_fRespawnTime ( 0.0f ),
	m_bBlinking		(false)
{
	m_pChassisPieceBute = NULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ChassisPiece::ChassisPiece
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

ChassisPiece::~ChassisPiece( )
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisPiece::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ChassisPiece::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if( fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP )
			{
				ReadProp( (ObjectCreateStruct*)pData );
			}
		}
		break;
		
		case MID_INITIALUPDATE:
		{
			uint32 nRet = PropType::EngineMessageFn(messageID, pData, fData);

			// Cache the position to be used for respawning the piece when dropped...

			g_pLTServer->GetObjectPos( m_hObject, &m_vOriginalPos );

			CreateSFXMessage(false,INVALID_TEAM);

			return nRet;
		}
		break;
		
		case MID_UPDATE:
		{
			uint32 dwRet = PropType::EngineMessageFn(messageID, pData, fData);
			
			Update( );

			return dwRet;
		}
		break;
		
		case MID_SAVEOBJECT:
		{
			Save( (ILTMessage_Write*)pData, (uint32)fData );
		}
		break;

		case MID_LOADOBJECT:
		{
			Load( (ILTMessage_Read*)pData, (uint32)fData );
		}	
		break;

		default : break;
	}

	return PropType::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisPiece::PropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

bool ChassisPiece::ReadProp( ObjectCreateStruct *pOCS )
{
	if( !pOCS )
		return false;

	GenericProp genProp;

	if( g_pLTServer->GetPropGeneric( "ChassisPieceType", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] )
		{
			m_pChassisPieceBute = ChassisButeMgr::Instance( ).GetChassisPieceBute( genProp.m_String );
			if( m_pChassisPieceBute )
				m_sPropType = m_pChassisPieceBute->m_sPropType.c_str( );
		}
	}

	if( g_pLTServer->GetPropGeneric( "RespawnTime", &genProp ) == LT_OK )
	{
		m_fRespawnTime = genProp.m_Float;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisPiece::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

bool ChassisPiece::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken	s_cTok_Carry( "CARRY" );
	static CParsedMsg::CToken	s_cTok_Drop( "DROP" );

	if( cMsg.GetArg( 0 ) == s_cTok_Carry )
	{
		if( !OnCarry( hSender ))
			return false;

		m_bCanBeCarried = false;
	}
	else if( cMsg.GetArg( 0 ) == s_cTok_Drop )
	{
		if( !OnDrop( hSender ))
			return false;

		m_bCanBeCarried = true;
	}
	
	return PropType::OnTrigger( hSender, cMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisPiece::OnCarry
//
//	PURPOSE:	Handle the Carry message...
//
// ----------------------------------------------------------------------- //

bool ChassisPiece::OnCarry( HOBJECT hSender )
{
	if( !m_bCanBeCarried )
		return false;
	
	CPlayerObj *pPlayer = dynamic_cast< CPlayerObj* >(g_pLTServer->HandleToObject( hSender ));
	if( !pPlayer )
		return false;

	if( !m_DelayTimer.Stopped() )
		return false;

	// Removing the piece from a device...
	Chassis* pChassis = dynamic_cast< Chassis* >( g_pLTServer->HandleToObject( m_hChassis ));
	if( pChassis )
	{
		if( !pChassis->RemoveChassisPiece( *this, pPlayer ) )
			return false;

		m_hChassis = LTNULL;
	}

	// Go non-solid so the character can move while carrying it...

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_SOLID | FLAG_RAYHIT );
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_ATTACH_HIDE1SHOW3, USRFLG_ATTACH_HIDE1SHOW3);

	ShowObjectAttachments( m_hObject, false );
	
	// Set this object as the players carried object...
	pPlayer->SetCarriedObject( m_hObject );
	
	m_bBlinking = false;

	// Stop the respawn counter since the piece is being carried again...
	m_RespawnTimer.Stop();
	m_DelayTimer.Stop();
	SetNextUpdate( UPDATE_NEVER );

	CreateSFXMessage(true,pPlayer->GetTeamID());

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisPiece::OnDrop
//
//	PURPOSE:	Handle the Drop message...
//
// ----------------------------------------------------------------------- //

bool ChassisPiece::OnDrop( HOBJECT hSender )
{
	// Make sure the piece can be picked up again...

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_RAYHIT , FLAG_RAYHIT );
	
	// See if we are being dropped near a Chassis...

	CPlayerObj *pPlayer = dynamic_cast< CPlayerObj* >(g_pLTServer->HandleToObject( hSender ));
	if( !pPlayer )
		return false;

	LTVector vDropPos;
	g_pLTServer->GetObjectPos( hSender, &vDropPos );

	LTVector vChassisPos;

	Chassis::ChassisList::const_iterator iter = Chassis::GetChassisList().begin();
	for( ; iter != Chassis::GetChassisList().end(); iter++ )
	{
		Chassis* pChassis = *iter;
		g_pLTServer->GetObjectPos( pChassis->m_hObject, &vChassisPos );
		float fCalcDist = vDropPos.Dist( vChassisPos );
		if( ( fCalcDist <= pChassis->GetDropZoneRadius() ) &&
			(( pChassis->GetTeamID( ) == INVALID_TEAM ) ||
			( pChassis->GetTeamID() == pPlayer->GetTeamID( ))))
		{
			// Dropping close to a chassis...
			if( !pChassis->AddChassisPiece( *this, pPlayer ))
				continue;

			// Let the piece know it's now part of a device...
			m_hChassis = pChassis->m_hObject;

			CreateSFXMessage(false,pPlayer->GetTeamID());
			
			return true;
		}
	}

	// Begin our respawn timer...
	if( m_fRespawnTime > 0.0f )
		m_RespawnTimer.Start( m_fRespawnTime );
	m_bBlinking = false;

	CreateSFXMessage(false,INVALID_TEAM);

	// Not close enough to a device so just drop where we are...

	g_pLTServer->SetObjectPos( m_hObject, &vDropPos );

	// Go solid again if originally solid...

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, m_dwFlags, m_dwFlags );

	HOBJECT hFilter[] = { m_hObject, hSender, LTNULL };
	MoveObjectToFloor( m_hObject, hFilter );
	
	// Make sure the piece is playing it's base animation...

	HMODELANIM hAnim = 0;
	g_pModelLT->GetAnimIndex( m_hObject, "base", hAnim );
	g_pModelLT->SetCurAnim( m_hObject, MAIN_TRACKER, hAnim );
	g_pModelLT->SetLooping( m_hObject, MAIN_TRACKER, false );
	g_pModelLT->ResetAnim( m_hObject, MAIN_TRACKER );


	m_DelayTimer.Start(0.5f);

	SetNextUpdate( UPDATE_NEXT_FRAME );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisPiece::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ChassisPiece::Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg )
		return;

	m_RespawnTimer.Save( pMsg );
	SAVE_FLOAT( m_fRespawnTime );
	SAVE_CHARSTRING( m_pChassisPieceBute ? m_pChassisPieceBute->m_sName.c_str( ) : "" );
	SAVE_HOBJECT( m_hChassis );
	SAVE_VECTOR( m_vOriginalPos );
	SAVE_bool( m_bCanBeCarried );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisPiece::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ChassisPiece::Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags )
{
	if( !pMsg )
		return;

	m_RespawnTimer.Load( pMsg );
	LOAD_FLOAT( m_fRespawnTime );

	char szName[256];
	LOAD_CHARSTRING( szName, ARRAY_LEN( szName ));
	m_pChassisPieceBute = ChassisButeMgr::Instance( ).GetChassisPieceBute( szName );
	LOAD_HOBJECT( m_hChassis );
	LOAD_VECTOR( m_vOriginalPos );
	LOAD_bool( m_bCanBeCarried );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisPiece::IsHeavy
//
//	PURPOSE:	Checks if piece is heavy type.
//
// ----------------------------------------------------------------------- //

bool ChassisPiece::IsHeavy( ) const
{
	if( m_pChassisPieceBute )
		return m_pChassisPieceBute->m_bHeavy;

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisPiece::Update
//
//	PURPOSE:	Frame Update...
//
// ----------------------------------------------------------------------- //

void ChassisPiece::Update()
{
	if( m_fRespawnTime > 0.0f )
	{
		if( m_RespawnTimer.Stopped() )
		{
			OnRespawn( );
			CreateSFXMessage(false,INVALID_TEAM,false);

			SetNextUpdate( UPDATE_NEVER );
		}
		else if( m_RespawnTimer.On() && m_RespawnTimer.GetCountdownTime() <= 10.0f )
		{
			if (!m_bBlinking)
			{
				m_bBlinking = true;
				CreateSFXMessage(false,INVALID_TEAM);
			}
		
			SetNextUpdate( UPDATE_NEXT_FRAME );
		}
	}
	else
	{
		// We don't respawn, so don't update.
		SetNextUpdate( UPDATE_NEVER );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisPiece::OnRespawn
//
//	PURPOSE:	Place the piece back at it's original position...
//
// ----------------------------------------------------------------------- //

bool ChassisPiece::OnRespawn( )
{

	LTVector vOldPos,vNewPos;

	g_pLTServer->GetObjectPos( m_hObject, &vOldPos );

	// Set the position to the origianl position...

	g_pLTServer->SetObjectPos( m_hObject, &m_vOriginalPos );

	m_bBlinking = false;

	// Move it to the floor...

	HOBJECT hFilter[] = { m_hObject, LTNULL };
	MoveObjectToFloor( m_hObject, hFilter );

	m_RespawnTimer.Stop();

	g_pLTServer->GetObjectPos( m_hObject, &vNewPos );


	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHASSISPIECE_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writebool(true);
	cMsg.WriteLTVector(vOldPos);
	cMsg.WriteLTVector(vNewPos);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	return true;
}


void ChassisPiece::CreateSFXMessage(bool bCarried, uint8 nTeam, bool bUpdate /* = true */)
{
	bool bPlanted = !!(m_hChassis);

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_CHASSISPIECE_ID);
	cMsg.Writeuint8( m_pChassisPieceBute ? m_pChassisPieceBute->m_nIndex : 0 );
	cMsg.Writebool(bCarried);
	cMsg.Writeuint8(nTeam);
	cMsg.Writebool( bPlanted );
	cMsg.Writeuint8( m_ActivateTypeHandler.GetActivateType( ));
	cMsg.Writebool( m_bBlinking );
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());


	if (bUpdate)
	{
		cMsg.Reset();
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHASSISPIECE_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writebool(false);
		cMsg.Writeuint8( m_pChassisPieceBute ? m_pChassisPieceBute->m_nIndex : 0 );
		cMsg.Writebool(bCarried);
		cMsg.Writeuint8(nTeam);
		cMsg.Writebool( bPlanted );
		cMsg.Writeuint8( m_ActivateTypeHandler.GetActivateType( ));
		cMsg.Writebool( m_bBlinking );
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisPiece::SetNextUpdate
//
//	PURPOSE:	SetNextUpdate override to avoid toggling the activation flags
//
// ----------------------------------------------------------------------- //

void ChassisPiece::SetNextUpdate(LTFLOAT fDelta, eUpdateControl eControl)
{
	// Control the Prop's deactivation behavior
	m_bCanDeactivate = (fDelta == UPDATE_NEVER) && (eControl == eControlDeactivation);

	// Pass it up the heirarchy
	PropType::SetNextUpdate(fDelta, eControl);
}

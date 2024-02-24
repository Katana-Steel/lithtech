// ----------------------------------------------------------------------- //
//
// MODULE  : Chassis.cpp
//
// PURPOSE : The object used to represent the chassis object.  An object
//				that allows players to build using chassispiece objects.
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
#include "ParsedMsg.h"
#include "Chassis.h"
#include "TeamMgr.h"
#include "PlayerObj.h"
#include "ChassisPiece.h"
#include "Spawner.h"
#include "ChassisButeMgr.h"

LINKFROM_MODULE( Chassis );

BEGIN_CLASS( Chassis )
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP(1), PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(CanTransition, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG( DropZoneRadius, 64.0f, PF_RADIUS )
	ADD_STRINGPROP_FLAG(AllowedPieces, "", PF_NOTIFYCHANGE )
	ADD_STRINGPROP_FLAG(Team, "NoTeam", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(OnComplete, "", PF_NOTIFYCHANGE )
END_CLASS_DEFAULT_FLAGS_PLUGIN( Chassis, PropType, NULL, NULL, 0, ChassisPlugin )


// Register with the CommandMgr...

CMDMGR_BEGIN_REGISTER_CLASS( Chassis )
	CMDMGR_ADD_MSG( TEAM, 2, NULL, "TEAM <0, 1, -1>" )
CMDMGR_END_REGISTER_CLASS( Chassis, PropType )

Chassis::ChassisList Chassis::m_lstChassis;

LTRESULT ChassisPlugin::PreHook_EditStringList(
	const char *szRezPath,
	const char *szPropName,
	char **aszStrings,
	uint32 *pcStrings,
	const uint32 cMaxStrings,
	const uint32 cMaxStringLen )
{
	// Handle team...

	if( _stricmp( "Team", szPropName ) == 0 )
	{
		char szTeam[32] = {0};

		_ASSERT(cMaxStrings > (*pcStrings) + 1);
		strcpy( aszStrings[(*pcStrings)++], "NoTeam" );
		
		for( int i = 0; i < MAX_TEAMS; ++i )
		{
			_ASSERT(cMaxStrings > (*pcStrings) + 1);

			sprintf( szTeam, "Team%i", i );
			strcpy( aszStrings[(*pcStrings)++], szTeam );
		}

		return LT_OK;
	}

	if( LT_OK == CPropTypePlugin::PreHook_EditStringList( szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLen ))
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT ChassisPlugin::PreHook_PropChanged( const char *szObjName, 
										   const char *szPropName,
										   const int nPropType,
										   const GenericProp &gpPropValue,
										   ILTPreInterface *pInterface,
										   const char *szModifiers )
{
	// Make sure the allowed pieces are part of proptypes.txt.
	if( !_stricmp( szPropName, "AllowedPieces" ) && gpPropValue.m_String[0] )
	{
		ConParse cpAllowedPieces;
		cpAllowedPieces.Init( gpPropValue.m_String );

		while( LT_OK == pInterface->Parse( &cpAllowedPieces ))
		{
			for( int nPiece = 0; nPiece < cpAllowedPieces.m_nArgs; nPiece++ )
			{
				if( !g_pPropTypeMgr->GetPropType( cpAllowedPieces.m_Args[nPiece] ))
					return LT_UNSUPPORTED;
			}
		}

		return LT_OK;
	}
	else if( !_stricmp( szPropName, "OnComplete" ) && gpPropValue.m_String[0] )
	{
		// Just send down to the command mgr plugin...

		if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName,
													szPropName,
													nPropType,
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return CPropTypePlugin::PreHook_PropChanged( szObjName, szPropName, nPropType, gpPropValue,
		pInterface, szModifiers );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Chassis::Chassis
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

Chassis::Chassis( )
:	PropType				( ),
	m_nOwningTeamID			( INVALID_TEAM ),
	m_fDropZoneRadius		( 64.0f )
{
	m_lstChassis.push_back( this );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Chassis::Chassis
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

Chassis::~Chassis( )
{
	ClearChassisPieceSlotList( );

	// Take us out of the master list.
	ChassisList::iterator iter = m_lstChassis.begin();
	for( ; iter != m_lstChassis.end(); iter++ )
	{
		Chassis* pChassis = *iter;
		if( pChassis == this )
		{
			m_lstChassis.erase( iter );
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Chassis::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Chassis::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = PropType::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				if( !ReadProp(( ObjectCreateStruct* )pData ))
					return 0;
			}
			
			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
            uint32 dwRet = PropType::EngineMessageFn(messageID, pData, fData);

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CreateSFXMessage( );

			return dwRet;
		}
		break;
		
		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return PropType::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Chassis::OnTrigger
//
//	PURPOSE:	Handle trigger message.
//
// ----------------------------------------------------------------------- //

bool Chassis::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken	s_cTok_Team( "TEAM" );

	if (cMsg.GetArg(0) == s_cTok_Team)
	{
		if( cMsg.GetArgCount( ) > 1 )
		{
			uint32 nTeamId = atoi( cMsg.GetArg( 1 ));
			if( nTeamId < MAX_TEAMS )
			{
				m_nOwningTeamID = nTeamId;
			}
			else
			{
				m_nOwningTeamID = INVALID_TEAM;
			}

			return true;
		}
	}

	
	return PropType::OnTrigger( hSender, cMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Chassis::ReadProp
//
//	PURPOSE:	Reads properties.
//
// ----------------------------------------------------------------------- //

bool Chassis::ReadProp( ObjectCreateStruct* pStruct )
{
	GenericProp genProp;

	if( IsTeamGameType() )
	{
		if( g_pLTServer->GetPropGeneric( "Team", &genProp ) == LT_OK )
		{
			if( genProp.m_String[0] )
			{
				// The team string should be TeamN, when N is the team id.
				char const szTeam[] = "Team";
				int nLen = strlen( szTeam );
				if( !_strnicmp( genProp.m_String, szTeam, nLen ))
				{
					uint32 nTeamId = atoi( &genProp.m_String[ nLen ] );
					if( nTeamId < MAX_TEAMS )
					{
						m_nOwningTeamID = nTeamId;
					}
				}
			}
		}
	}

	// Get the diminsions considered to be the drop zone...

	if( g_pLTServer->GetPropGeneric( "DropZoneRadius", &genProp ) == LT_OK )
	{
		m_fDropZoneRadius = genProp.m_Float;
	}

	if( g_pLTServer->GetPropGeneric( "AllowedPieces", &genProp ) == LT_OK && genProp.m_String[0] )
	{
		ConParse cpAllowedPieces;
		cpAllowedPieces.Init( genProp.m_String );

		// Parse the allowed pieces string for all the chassispieces.
		while( LT_OK == g_pCommonLT->Parse( &cpAllowedPieces ))
		{
			for( int nPiece = 0; nPiece < cpAllowedPieces.m_nArgs; nPiece++ )
			{
				// If we find a bute for this, then we can define a slot.
				ChassisPieceBute* pChassisPieceBute = ChassisButeMgr::Instance( ).GetChassisPieceBute( cpAllowedPieces.m_Args[nPiece] );
				if( pChassisPieceBute )
				{
					ChassisPieceSlot chassisPieceSlot;
					chassisPieceSlot.m_pChassisPieceBute = pChassisPieceBute;

					m_lstChassisPieceSlots.push_back( chassisPieceSlot );
				}
			}
		}
	}

	if( g_pLTServer->GetPropGeneric( "OnComplete", &genProp ) == LT_OK && genProp.m_String[0] )
	{
		m_sCommandComplete = genProp.m_String;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Chassis::InitialUpdate
//
//	PURPOSE:	MID_INITIALUPDATE handler.
//
// ----------------------------------------------------------------------- //

void Chassis::InitialUpdate( )
{
	// Create the "target" objects...
	
	char szSpawn[256];
		
	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );

	LTRotation rRot;
	g_pLTServer->GetObjectRotation( m_hObject, &rRot );

	ChassisPieceSlotList::iterator iter = m_lstChassisPieceSlots.begin( );
	for( ; iter != m_lstChassisPieceSlots.end( ); iter++ )
	{
		ChassisPieceSlot& slot = *iter;

		// Make the target if it has one.
		if( slot.m_pChassisPieceBute->m_sPropType_Target.length( ))
		{
			// Create a target piece and attach it to us.  The target pieces
			// are like the regular pieces, but are translucent.
			sprintf( szSpawn, "PropType Type %s", slot.m_pChassisPieceBute->m_sPropType_Target.c_str( ));
			PropType* pPropType = ( PropType* )SpawnObject( szSpawn, vPos, rRot );
			if( pPropType && pPropType->m_hObject )
			{
				slot.m_hPlaceHolderObject = pPropType->m_hObject;

				// Make the target object translucent.
				g_pCommonLT->SetObjectFlags( slot.m_hPlaceHolderObject, 
					OFT_Flags2, FLAG2_FORCETRANSLUCENT, FLAG2_FORCETRANSLUCENT );

				// Attach the target to the base...
				HATTACHMENT hAttachment;
				g_pLTServer->CreateAttachment( m_hObject, slot.m_hPlaceHolderObject, 
					slot.m_pChassisPieceBute->m_sChassisSocket.c_str( ), &LTVector(0,0,0), &LTRotation(), &hAttachment );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Chassis::FindChassisPieceSlot
//
//	PURPOSE:	Finds a chassispieceslot based on a chassispiecebute.
//
// ----------------------------------------------------------------------- //
Chassis::ChassisPieceSlot* Chassis::FindChassisPieceSlot( ChassisPieceBute const& chassisPieceBute )
{
	// Check the list of pieces to see if this one has already been added.
	ChassisPieceSlotList::iterator iter = m_lstChassisPieceSlots.begin();
	for( ; iter != m_lstChassisPieceSlots.end(); ++iter )
	{
		ChassisPieceSlot& testSlot = ( *iter );
		ChassisPieceBute* pChassisPieceBute = testSlot.m_pChassisPieceBute;
		if( !pChassisPieceBute )
		{
			ASSERT( !"Chassis::AddChassisPiece:  Invalid chassispieceslot." );
			return NULL;
		}
		
		// Already have this type.
		if( pChassisPieceBute == &chassisPieceBute )
		{
			return &testSlot;
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Chassis::AddChassisPiece
//
//	PURPOSE:	Add a piece to the base and check for completion...
//
// ----------------------------------------------------------------------- //

bool Chassis::AddChassisPiece( ChassisPiece& chassisPiece, CPlayerObj* pPlayer )
{
	// Check if we already have all the pieces.
	if( IsAllPiecesPlaced( ))
		return false;

	// Get the new bute info.
	ChassisPieceBute const* pNewChassisPieceBute = chassisPiece.GetChassisPieceBute( );
	if( !pNewChassisPieceBute )
		return false;

	// This will contain the slot the piece should go into.
	ChassisPieceSlot* pChassisPieceSlot = FindChassisPieceSlot( *pNewChassisPieceBute );
	if( !pChassisPieceSlot )
	{
		return false;
	}

	// Check if slot is already filled.
	if( pChassisPieceSlot->m_hChassisPiece )
	{
		return false;
	}

	// Reset the filenames to the 'replaced' PropType if specified.
	if( pChassisPieceSlot->m_pChassisPieceBute->m_sPropType_Replaced.length( ))
	{
		PROPTYPE *pPropType = g_pPropTypeMgr->GetPropType( pChassisPieceSlot->m_pChassisPieceBute->m_sPropType_Replaced.c_str( ));
		if( !pPropType )
			return false;

		ObjectCreateStruct ocs;

		LTStrCpy( ocs.m_Filename, pPropType->sFilename.c_str(), ARRAY_LEN( ocs.m_Filename ));

		pPropType->blrPropRenderStyleReader.CopyList( 0, ocs.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN + 1 );
		pPropType->blrPropSkinReader.CopyList( 0, ocs.m_SkinNames[0], MAX_CS_FILENAME_LEN + 1 );

		if( g_pCommonLT->SetObjectFilenames( chassisPiece.m_hObject, &ocs ) != LT_OK )
			return false;

		// Attach the piece to the base...

		LTVector vPos;
		g_pLTServer->GetObjectPos( m_hObject, &vPos );
		g_pLTServer->SetObjectPos( chassisPiece.m_hObject, &vPos );

		LTRotation rRot;
		g_pLTServer->GetObjectRotation( m_hObject, &rRot );
		g_pLTServer->SetObjectRotation( chassisPiece.m_hObject, &rRot );

		HATTACHMENT hAttachment;
		g_pLTServer->CreateAttachment( m_hObject, chassisPiece.m_hObject, 
			pChassisPieceSlot->m_pChassisPieceBute->m_sChassisSocket.c_str( ), &LTVector(0,0,0), &LTRotation(), &hAttachment );
	}

	// Hide the target object...
	if( pChassisPieceSlot->m_hPlaceHolderObject )
	{
		g_pCommonLT->SetObjectFlags( pChassisPieceSlot->m_hPlaceHolderObject, OFT_Flags, 0, FLAG_VISIBLE );
	}

	// Add it to the list of pieces...

	pChassisPieceSlot->m_hChassisPiece = chassisPiece.m_hObject;

	// Run the complete command if done.
	if( IsAllPiecesPlaced( ))
	{
		char const* pszCommandComplete = m_sCommandComplete.c_str( );
		if( g_pCmdMgr->IsValidCmd( pszCommandComplete ))
		{
			g_pCmdMgr->Process( pszCommandComplete, m_hObject, NULL );
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Chassis::RemoveChassisPiece
//
//	PURPOSE:	Remove a piece from the base...
//
// ----------------------------------------------------------------------- //

bool Chassis::RemoveChassisPiece( ChassisPiece& chassisPiece, CPlayerObj* pPlayer )
{
	ChassisPieceBute const* pChassisPieceBute = chassisPiece.GetChassisPieceBute( );
	if( !pChassisPieceBute )
		return false;

	ChassisPieceSlot* pChassisPieceSlot = FindChassisPieceSlot( *pChassisPieceBute );
	if( !pChassisPieceSlot )
		return false;

	// Check if there wasn't anything in our slot.
	if( !pChassisPieceSlot->m_hChassisPiece )
		return false;

	// Make sure the slot has the correct piece.
	if( pChassisPieceSlot->m_hChassisPiece != chassisPiece.m_hObject )
	{
		ASSERT( !"Chassis::RemoveChassisPiece:  Invalid chassispiece in slot." );
		return false;
	}

	// Get the proptype used for the chassispiecebute.
	PROPTYPE *pPropType = g_pPropTypeMgr->GetPropType( pChassisPieceSlot->m_pChassisPieceBute->m_sPropType.c_str( ));
	if( !pPropType )
		return false;

	// Remove it from the slot.
	pChassisPieceSlot->m_hChassisPiece = NULL;

	// Detach the piece from the base...
	HATTACHMENT hAttachment;
	if( g_pLTServer->FindAttachment( m_hObject, chassisPiece.m_hObject, &hAttachment ) == LT_OK )
	{
		g_pLTServer->RemoveAttachment( hAttachment ); 	
	}

	// Make sure the chassispiece is set to the correct filenames.  It may have been modified
	// when placed with the replaced proptype filenames.
	ObjectCreateStruct ocs;
	LTStrCpy( ocs.m_Filename, pPropType->sFilename.c_str(), ARRAY_LEN( ocs.m_Filename ));
	pPropType->blrPropRenderStyleReader.CopyList( 0, ocs.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN + 1 );
	pPropType->blrPropSkinReader.CopyList( 0, ocs.m_SkinNames[0], MAX_CS_FILENAME_LEN + 1 );
	g_pCommonLT->SetObjectFilenames( chassisPiece.m_hObject, &ocs );

	// Show the target object...
	if( pChassisPieceSlot->m_hPlaceHolderObject )
	{
		g_pCommonLT->SetObjectFlags( pChassisPieceSlot->m_hPlaceHolderObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Chassis::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Chassis::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if( !pMsg ) 
		return;

	SAVE_BYTE( m_nOwningTeamID );
	SAVE_FLOAT( m_fDropZoneRadius );
	SAVE_CHARSTRING( m_sCommandComplete.c_str( ));

	SAVE_DWORD( m_lstChassisPieceSlots.size( ));

	ChassisPieceSlotList::const_iterator iter = m_lstChassisPieceSlots.begin();
	for( ; iter != m_lstChassisPieceSlots.end(); ++iter )
	{
		ChassisPieceSlot const& slot = *iter;

		SAVE_CHARSTRING( slot.m_pChassisPieceBute->m_sName.c_str( ));
		SAVE_HOBJECT( slot.m_hPlaceHolderObject );
		SAVE_HOBJECT( slot.m_hChassisPiece );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Chassis::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Chassis::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if( !pMsg )
		return;

	char szString[256];

	LOAD_BYTE( m_nOwningTeamID );
	LOAD_FLOAT( m_fDropZoneRadius );
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sCommandComplete = szString;

	uint32 nSlots;
	LOAD_DWORD( nSlots );
	ClearChassisPieceSlotList( );
	for( uint32 nSlot = 0; nSlot < nSlots; nSlot++ )
	{
		ChassisPieceSlot chassisPieceSlot;

		LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
		chassisPieceSlot.m_pChassisPieceBute = ChassisButeMgr::Instance( ).GetChassisPieceBute( szString );
		if( !chassisPieceSlot.m_pChassisPieceBute )
			continue;

		LOAD_HOBJECT( chassisPieceSlot.m_hPlaceHolderObject );
		LOAD_HOBJECT( chassisPieceSlot.m_hChassisPiece );

		m_lstChassisPieceSlots.push_back( chassisPieceSlot );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Chassis::IsAllPiecesPlaced
//
//	PURPOSE:	Checks if chassis has all its pieces.
//
// ----------------------------------------------------------------------- //
bool Chassis::IsAllPiecesPlaced( ) const
{
	// Check all the slots.  If any of them is empty, then all the pieces
	// are not in.
	ChassisPieceSlotList::const_iterator iter = m_lstChassisPieceSlots.begin();
	for( ; iter != m_lstChassisPieceSlots.end( ); ++iter )
	{
		ChassisPieceSlot const& slot = *iter;

		if( !slot.m_hChassisPiece )
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Chassis::IsAllPiecesPlaced
//
//	PURPOSE:	Checks if chassis has all its pieces.
//
// ----------------------------------------------------------------------- //
void Chassis::CreateSFXMessage( )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_CHASSIS_ID);
	cMsg.Writeuint8( m_nOwningTeamID );
	cMsg.Writefloat( m_fDropZoneRadius );
	cMsg.Writeuint8( m_ActivateTypeHandler.GetActivateType( ));
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());


	cMsg.Reset();
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8( m_nOwningTeamID );
	cMsg.Writefloat( m_fDropZoneRadius );
	cMsg.Writeuint8( m_ActivateTypeHandler.GetActivateType( ));
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Chassis::ClearChassisPieceSlotList
//
//	PURPOSE:	Removes all the chassis piece slots from the list.
//
// ----------------------------------------------------------------------- //
void Chassis::ClearChassisPieceSlotList( )
{
	// Remove the target models and the piece models...

	ChassisPieceSlotList::const_iterator iterSlot;
	for( iterSlot = m_lstChassisPieceSlots.begin(); iterSlot != m_lstChassisPieceSlots.end(); ++iterSlot )
	{
		ChassisPieceSlot const& chassisPieceSlot = *iterSlot;

		// Toss the placeholder object.
		HATTACHMENT hAttachment = NULL;
		if( g_pLTServer->FindAttachment( m_hObject, chassisPieceSlot.m_hPlaceHolderObject, &hAttachment ) == LT_OK )
		{
			g_pLTServer->RemoveAttachment( hAttachment ); 	
		}

		if( chassisPieceSlot.m_hPlaceHolderObject )
		{
			g_pLTServer->RemoveObject( chassisPieceSlot.m_hPlaceHolderObject );
		}

		// If this slot has a piece in it, toss it.
		if( chassisPieceSlot.m_hChassisPiece )
		{
			hAttachment = NULL;
			if( g_pLTServer->FindAttachment( m_hObject, chassisPieceSlot.m_hChassisPiece, &hAttachment ) == LT_OK )
			{
				g_pLTServer->RemoveAttachment( hAttachment ); 	
			}

			g_pLTServer->RemoveObject( chassisPieceSlot.m_hChassisPiece );
		}
	}

	m_lstChassisPieceSlots.clear( );
}
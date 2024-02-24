// ----------------------------------------------------------------------- //
//
// MODULE  : AimMagnet.cpp
//
// PURPOSE : Invisible object that be used as an potential target by the auto-aim system
//
// CREATED : 3/11/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AimMagnet.h"
#include "MsgIds.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "ClientServerShared.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"


LINKFROM_MODULE( AimMagnet );

extern CGameServerShell* g_pGameServerShell;

AimMagnet::ObjectList AimMagnet::m_lstObjects;


// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	CLASS:		AimMagnet
//
//	PURPOSE:	Invisible object that be used as an potential target by the auto-aim system
//
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

#pragma force_active on
BEGIN_CLASS(AimMagnet)
	ADD_SOLID_FLAG(0, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG( Target, "", PF_OBJECTLINK )
	//place holder in case it's needed later
	ADD_STRINGPROP_FLAG(Team, "NoTeam", PF_HIDDEN | PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS_PLUGIN(AimMagnet, GameBase, NULL, NULL, 0, CAimMagnetPlugin)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( AimMagnet )

	CMDMGR_ADD_MSG( TEAM, 2, NULL, "TEAM <0, 1, -1>" )
	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )
	CMDMGR_ADD_MSG( TARGET, 2, NULL, "TARGET <objectname>" )

CMDMGR_END_REGISTER_CLASS( AimMagnet, GameBase )



LTRESULT CAimMagnetPlugin::PreHook_EditStringList(const char* szRezPath,
												   const char* szPropName,
												   char** aszStrings,
												   uint32* pcStrings,
												   const uint32 cMaxStrings,
												   const uint32 cMaxStringLength)
{

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


	return LT_UNSUPPORTED;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AimMagnet::AimMagnet()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

AimMagnet::AimMagnet() : GameBase(OT_NORMAL), m_hTarget(NULL)
{
	MakeTransitionable();
	m_hTarget.SetReceiver( *this );
	m_nTeamId = INVALID_TEAM;

	// Add this instance to a list of all AimMagnets.
	m_lstObjects.push_back( this );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AimMagnet::~AimMagnet()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //
AimMagnet::~AimMagnet()
{
	// Erase this instance from the list of all Character's.
	ObjectList::iterator it = m_lstObjects.begin( );
	while( it != m_lstObjects.end( ))
	{
		if( *it == this )
		{
			m_lstObjects.erase( it );
			break;
		}

		it++;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AimMagnet::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 AimMagnet::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			ObjectCreateStruct* pInfo = (ObjectCreateStruct*)pData;
	
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp(pInfo);
			}
			else if (fData == PRECREATE_STRINGPROP)
			{
				ReadProp(pInfo);

				// Show ourself...

				pInfo->m_Flags |= FLAG_VISIBLE;
			}

		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;

		case MID_ALLOBJECTSCREATED:
		{
			AssignTarget();
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
			
			uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);
			
			return dwRet;
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AimMagnet::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool AimMagnet::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Team( "TEAM" );
	static CParsedMsg::CToken s_cTok_Target( "TARGET" );
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");

	if (cMsg.GetArg(0) == s_cTok_Team)
	{
		if( cMsg.GetArgCount( ) > 1 )
		{
			uint32 nTeamId = atoi( cMsg.GetArg( 1 ));
			if( nTeamId < MAX_TEAMS )
			{
				SetTeamId( nTeamId );
			}
			else
			{
				SetTeamId( INVALID_TEAM );
			}

			return true;
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Target)
	{
		if( cMsg.GetArgCount( ) > 1 )
		{
			m_sTargetName = cMsg.GetArg( 1 );
			m_hTarget = NULL;
			AssignTarget();
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_On)
	{
		if (m_hObject)
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Off)
	{
		if (m_hObject)
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE);
		}
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AimMagnet::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL AimMagnet::ReadProp(ObjectCreateStruct *pInfo)
{
	GenericProp genProp;

    if (!pInfo) return LTFALSE;

	if( g_pLTServer->GetPropGeneric( "Target", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] )
		{
			m_sTargetName = genProp.m_String;
		}
	}

	// Get the team this object belongs to.
	if( IsTeamGameType() )
	{
		if( g_pLTServer->GetPropGeneric( "Team", &genProp ) == LT_OK )
		{
			m_nTeamId = TeamStringToTeamId( genProp.m_String );
		}
	}
	else
	{
		m_nTeamId = INVALID_TEAM;
	}

	return LTTRUE;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AimMagnet::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL AimMagnet::InitialUpdate()
{
    SetNextUpdate(UPDATE_NEVER);

//	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_FORCECLIENTUPDATE );
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );

	CreateSpecialFX();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AimMagnet::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

LTBOOL AimMagnet::Update()
{
   SetNextUpdate(UPDATE_NEVER);

   CreateSpecialFX(true);

   return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AimMagnet::CreateSpecialFX
//
//	PURPOSE:	Send the special fx message for this object
//
// ----------------------------------------------------------------------- //

void AimMagnet::CreateSpecialFX( bool bUpdateClients /* = false  */ )
{
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_AIMMAGNET_ID);
		cMsg.Writeuint8(m_nTeamId);
		cMsg.WriteObject(m_hTarget);
		g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	}

	if (bUpdateClients)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_AIMMAGNET_ID );
		cMsg.WriteObject( m_hObject );
		cMsg.Writeuint8( m_nTeamId );
		cMsg.WriteObject(m_hTarget);
		g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AimMagnet::SetTeamId
//
//	PURPOSE:	Set the teamid for this AimMagnet...
//
// ----------------------------------------------------------------------- //

void AimMagnet::SetTeamId( uint8 nTeamId )
{
	m_nTeamId = nTeamId;


	CreateSpecialFX(true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AimMagnet::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void AimMagnet::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_BYTE(m_nTeamId);
	SAVE_CHARSTRING( m_sTargetName.c_str() );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AimMagnet::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void AimMagnet::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_BYTE(m_nTeamId);
	char szTemp[128];
	LOAD_CHARSTRING( szTemp, ARRAY_LEN(szTemp) );
	m_sTargetName = szTemp;

	AssignTarget();
}

	
// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AimMagnet::AssignTarget
//
//  PURPOSE:	Set the target object...
//
// ----------------------------------------------------------------------- //

void AimMagnet::AssignTarget()
{
	if( m_hTarget )
		return;

	// Get the target object if one was specified...
	
	if( !m_sTargetName.empty() )
	{
		HOBJECT hObj = LTNULL;
		if( LT_OK != FindNamedObject( m_sTargetName.c_str(), hObj ))
		{
			m_hTarget = LTNULL;
			g_pLTServer->RemoveObject( m_hObject );
			
			return;
		}

		m_hTarget = hObj;

		// Need to make the magnet transitionable if it's target is...

		GameBase *pTarget = dynamic_cast<GameBase*>(g_pLTServer->HandleToObject( m_hTarget ));
		if( pTarget )
		{
			if( pTarget->CanTransition() )
			{
				// We need to set touch notify so the object will be added to containers, like transition areas...

				g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_TOUCH_NOTIFY, FLAG_TOUCH_NOTIFY );
			}
		}

		SetNextUpdate( UPDATE_NEXT_FRAME );
	}


}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AimMagnet::OnLinkBroken
//
//  PURPOSE:	An object has broken its link with us
//
// ----------------------------------------------------------------------- //

void AimMagnet::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if (pRef == &m_hTarget)
		CreateSpecialFX(true);

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_VISIBLE );
	
	GameBase::OnLinkBroken(pRef, hObj);
}

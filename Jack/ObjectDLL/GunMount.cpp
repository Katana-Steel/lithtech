// ----------------------------------------------------------------------- //
//
// MODULE  : GunMount.cpp
//
// PURPOSE : This object allows a gun to be attached to objects and controlled
//				through triggers.
//
// CREATED : 5/22/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "GunMount.h"
#include "ParsedMsg.h"
#include "WeaponMgr.h"
#include "Weapon.h"
#include "WeaponFireInfo.h"

#define NOWEAPON "<none>"

LINKFROM_MODULE( GunMount );

BEGIN_CLASS( GunMount )
	ADD_STRINGPROP_FLAG(Filename, "", PF_HIDDEN | PF_MODEL)
	ADD_STRINGPROP_FLAG(Weapon,	NOWEAPON, PF_STATICLIST | PF_DIMS | PF_LOCALDIMS )
    ADD_BOOLPROP(Hidden, false)
END_CLASS_DEFAULT_FLAGS_PLUGIN( GunMount, GameBase, NULL, NULL, 0, GunMountPlugin )


// Register with the CommandMgr...

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateFireMsg
//
//  PURPOSE:	Make sure fire message is good
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateFireMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return LTFALSE;

	return LTTRUE;
}


CMDMGR_BEGIN_REGISTER_CLASS( GunMount )
	CMDMGR_ADD_MSG( FIRE, -1, ValidateFireMsg, "FIRE [rounds_to_fire]" )
	CMDMGR_ADD_MSG( BEGINFIRE, 1, NULL, "BEGINFIRE" )
	CMDMGR_ADD_MSG( ENDFIRE, 1, NULL, "ENDFIRE" )
CMDMGR_END_REGISTER_CLASS( GunMount, GameBase )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GunMount::GunMount
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

GunMount::GunMount( )
{
	m_eFiringState = eNotFiring;
	m_nRoundsToFire = 0;
	m_bVisible = true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GunMount::GunMount
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

GunMount::~GunMount( )
{
	// Remove the weaponmodel we made.
	g_pLTServer->RemoveObject( m_Weapons.GetWeaponModel( ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 GunMount::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

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
            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

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

		case MID_UPDATE:
		{
            Update( );
		}
		break;

		default : 
			break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 GunMount::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pRead)
{
	GameBase::ObjectMessageFn( hSender, pRead );

	m_Weapons.ObjectMessageFn( this, hSender, pRead );

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::OnTrigger
//
//	PURPOSE:	Handle trigger message.
//
// ----------------------------------------------------------------------- //

bool GunMount::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken	s_cTok_1("1");
	static CParsedMsg::CToken	s_cTok_True("TRUE");
	static CParsedMsg::CToken	s_cTok_Fire( "FIRE" );
	static CParsedMsg::CToken	s_cTok_BeginFire( "BEGINFIRE" );
	static CParsedMsg::CToken	s_cTok_EndFire( "ENDFIRE" );
	static CParsedMsg::CToken	s_cTok_Visible("VISIBLE");
	static CParsedMsg::CToken	s_cTok_Hidden("HIDDEN");
	static CParsedMsg::CToken	s_cTok_Remove("REMOVE");

	if (cMsg.GetArg(0) == s_cTok_Fire)
	{
		// Check if they specified how many rounds to fire.
		if( cMsg.GetArgCount( ) > 1 )
		{
			m_nRoundsToFire = atoi( cMsg.GetArg( 1 ));
		}
		else
		{
			m_nRoundsToFire = 1;
		}

		StartPreFiring( );
		return true;
	}
	else if (cMsg.GetArg(0) == s_cTok_BeginFire)
	{
		m_nRoundsToFire = -1;
		StartPreFiring( );
		return true;
	}
	else if (cMsg.GetArg(0) == s_cTok_EndFire)
	{
		StartPostFiring( );
		return true;
	}
	else if( cMsg.GetArg(0) == s_cTok_Visible )
	{
		m_bVisible = (( cMsg.GetArg(1) == s_cTok_1) || (cMsg.GetArg(1) == s_cTok_True));
		SetVisible( m_bVisible );

		return true;
	}
	else if( cMsg.GetArg(0) == s_cTok_Hidden )
	{
		m_bVisible = !((cMsg.GetArg(1) == s_cTok_1) || (cMsg.GetArg(1) == s_cTok_True));
		SetVisible( m_bVisible );

		return true;
	}
	
	return GameBase::OnTrigger( hSender, cMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::ReadProp
//
//	PURPOSE:	Reads properties.
//
// ----------------------------------------------------------------------- //

bool GunMount::ReadProp( ObjectCreateStruct* pStruct )
{
	GenericProp genProp;

    if ( g_pLTServer->GetPropGeneric( "Weapon", &genProp ) == LT_OK && genProp.m_String[0] )
	{
		m_sWeapon = genProp.m_String;
	}

    if( g_pLTServer->GetPropGeneric( "Hidden", &genProp ) == LT_OK )
	{
		m_bVisible = !genProp.m_Bool;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::InitialUpdate
//
//	PURPOSE:	MID_INITIALUPDATE handler.
//
// ----------------------------------------------------------------------- //

void GunMount::InitialUpdate( )
{
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES, ( FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES ));

	// Rotate 180 degrees to compensate for backwards HH weapon models.  The HH weapon models are
	// backwards because they need to work with the RightHand sockets of the characters, which 
	// are backwards.  Because we want the model to look right in dedit, the object must be
	// placed backwards in dedit, then we rotate it around here to get the z pointing the right way.
	// We still need to flip the HH weapon model around because it is still pointing backwards.
	LTRotation rot;
	g_pLTServer->GetObjectRotation( m_hObject, &rot );
	rot.Rotate( rot.Up( ), MATH_PI );
	g_pLTServer->SetObjectRotation( m_hObject, &rot );

	CreateWeapon( );

	SetVisible( m_bVisible );

	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::Update
//
//	PURPOSE:	Frame update.
//
// ----------------------------------------------------------------------- //

void GunMount::Update( )
{
	switch( m_eFiringState )
	{
		case eNotFiring:
			break;
		case ePreFiring:
			UpdatePreFiring( );
			break;
		case eFiring:
			UpdateFiring( );
			break;
		case ePostFiring:
			UpdatePostFiring( );
			break;
		default:
			ASSERT( !"GunMount::Update:  Invalid firing state." );
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::AnimationComplete
//
//	PURPOSE:	Starts the prefiring state.
//
// ----------------------------------------------------------------------- //
bool GunMount::IsAnimationComplete( )
{
	CWeapon* pWeapon = m_Weapons.GetCurWeapon( );
	if( !pWeapon )
	{
		ASSERT( !"GunMount::IsAnimationComplete:  Invalid weapon." );
		return true;
	}

	uint32 dwFlags = 0;
	if( g_pModelLT->GetPlaybackState( pWeapon->GetModelObject( ), MAIN_TRACKER, dwFlags ) != LT_OK )
		return true;

	if( dwFlags & MS_PLAYDONE )
		return true;

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::StartNotFiring
//
//	PURPOSE:	Starts the prefiring state.
//
// ----------------------------------------------------------------------- //
bool GunMount::StartNotFiring( )
{
	m_eFiringState = eNotFiring;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::StartPreFiring
//
//	PURPOSE:	Starts the prefiring state.
//
// ----------------------------------------------------------------------- //
bool GunMount::StartPreFiring( )
{
	CWeapon* pWeapon = m_Weapons.GetCurWeapon( );
	if( !pWeapon )
		return false;

	// Get the prefire animation.
	HMODELANIM hAnim = pWeapon->GetPreFireAni( );

	// If they don't have a prefire, just go to firing.
	if( hAnim == INVALID_ANI )
	{
		return StartFiring( );
	}

	// Begin prefire animation.
	pWeapon->PlayAnimation( hAnim, true, false, true );

	SetNextUpdate( UPDATE_NEXT_FRAME );

	m_eFiringState = ePreFiring;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::UpdatePreFiring
//
//	PURPOSE:	Updates the prefiring state.
//
// ----------------------------------------------------------------------- //

void GunMount::UpdatePreFiring( )
{
	// Once the prefire is done, go to firing.
	if( IsAnimationComplete( ))
	{
		StartFiring( );
		return;
	}

	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::StartFiring
//
//	PURPOSE:	Starts the firing state.
//
// ----------------------------------------------------------------------- //
bool GunMount::StartFiring( )
{
	CWeapon* pWeapon = m_Weapons.GetCurWeapon( );
	if( !pWeapon )
		return false;

	// Play the fire animation if they have it.
	HMODELANIM hAnim = pWeapon->GetFireAni( );
	if( hAnim != INVALID_ANI )
	{
		pWeapon->PlayAnimation( hAnim, true, true, false );
	}

	// Fire the weapon.
	if( !FireWeapon( ))
	{
		return StartNotFiring( );
	}

	m_eFiringState = eFiring;

	SetNextUpdate( UPDATE_NEXT_FRAME );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::UpdateFiring
//
//	PURPOSE:	Updates the firing state.
//
// ----------------------------------------------------------------------- //

void GunMount::UpdateFiring( )
{
	// If we're done, then go to post fire.
	if( m_nRoundsToFire == 0 )
	{
		StartPostFiring( );
		return;
	}

	// Continue to fire weapon.
	if( !FireWeapon( ))
	{
		StartPostFiring( );
		return;
	}

	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::StartPostFiring
//
//	PURPOSE:	Starts the postfiring state.
//
// ----------------------------------------------------------------------- //
bool GunMount::StartPostFiring( )
{
	CWeapon* pWeapon = m_Weapons.GetCurWeapon( );
	if( !pWeapon )
		return false;

	// Get the prefire animation.
	HMODELANIM hAnim = pWeapon->GetPostFireAni( );

	// If they don't have a postfire, just go to notfiring.
	if( hAnim == INVALID_ANI )
	{
		return StartNotFiring( );
	}

	// Begin prefire animation.
	pWeapon->PlayAnimation( hAnim, true, false, true );

	m_eFiringState = ePostFiring;

	SetNextUpdate( UPDATE_NEXT_FRAME );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::UpdatePostFiring
//
//	PURPOSE:	Updates the postfiring state.
//
// ----------------------------------------------------------------------- //

void GunMount::UpdatePostFiring( )
{
	// Once the prefire is done, go to notfiring.
	if( IsAnimationComplete( ))
	{
		StartNotFiring( );
		return;
	}

	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::CreateWeapon
//
//	PURPOSE:	Creates the weapon
//
// ----------------------------------------------------------------------- //

bool GunMount::CreateWeapon( )
{
	uint8 nWeaponID = WMGR_INVALID_ID;
	uint8 nAmmoID = WMGR_INVALID_ID;
	g_pWeaponMgr->ReadWeapon( m_sWeapon.c_str( ), nWeaponID, nAmmoID );
	
	// Parse weapon/ammo info
	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponID);
	if( !pWeapon )
		return false;

	// Prepare the object create struct
	ObjectCreateStruct theStruct;
	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD;
	theStruct.m_Flags2 |= FLAG2_DYNAMICDIRLIGHT;
	theStruct.m_NextUpdate = 1.0f;

	pWeapon->blrHHSkins.CopyList(0, theStruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
	pWeapon->blrHHRenderStyles.CopyList(0, theStruct.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);

	// Set up the model/skin filenames

	LTStrCpy(theStruct.m_Filename, pWeapon->szHHModel, ARRAY_LEN( theStruct.m_Filename ));

	// Create the attachment

    HCLASS hClass = g_pLTServer->GetClass("CHHWeaponModel");
    LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
	if( !pModel )
		return false;

	// Attach it

	HATTACHMENT hAttachment;
	// Flip the model around backwards to compensate for backwards orientation due to 
	// socket being backwards.
	LTRotation rot( 0.0f, MATH_PI, 0.0f );
    LTRESULT res = g_pLTServer->CreateAttachment( m_hObject, pModel->m_hObject, NULL,
												 &LTVector(0,0,0), &rot, &hAttachment);
	if( res != LT_OK )
	{
		g_pLTServer->RemoveObject( pModel->m_hObject );
		return false;
	}

	// Add the weapon to our arsenal.
	if( !m_Weapons.Init( m_hObject, pModel->m_hObject ))
	{
		g_pLTServer->RemoveAttachment( hAttachment );
		g_pLTServer->RemoveObject( pModel->m_hObject );
		return false;
	}

	m_Weapons.ObtainWeapon(nWeaponID, nAmmoID, 10000);
	m_Weapons.ChangeWeapon(nWeaponID);
	m_Weapons.GetCurWeapon()->SetAmmoId(nAmmoID);

	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void GunMount::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if( !pMsg ) 
		return;

	SAVE_CHARSTRING( m_sWeapon.c_str( ));
	m_Weapons.Save(pMsg, 0);
	SAVE_DWORD( m_nRoundsToFire );
	SAVE_BYTE( m_eFiringState );
	SAVE_bool( m_bVisible );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void GunMount::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if( !pMsg )
		return;

	char szString[256];

	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sWeapon = szString;

	m_Weapons.Load(pMsg, 0);
	// Check if we had a current weapon.
	int nCurWeaponId = m_Weapons.GetCurWeaponId();
	if( nCurWeaponId >= 0 )
	{
		m_Weapons.DeselectCurWeapon();	// Deselect so we'll change to it
		m_Weapons.ChangeWeapon(nCurWeaponId);
	}

	LOAD_DWORD_CAST( m_nRoundsToFire, int32 );
	LOAD_BYTE_CAST( m_eFiringState, FiringState );
	LOAD_bool( m_bVisible );

	// Restart the firing state.
	switch( m_eFiringState )
	{
		case eNotFiring:
		case ePostFiring:
			StartNotFiring( );
			break;
		case ePreFiring:
		case eFiring:
			StartPreFiring( );
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::Fire
//
//	PURPOSE:	Fires the weapon.
//
// ----------------------------------------------------------------------- //

bool GunMount::FireWeapon( )
{
	CWeapon* pWeapon = m_Weapons.GetCurWeapon( );
	if( !pWeapon )
		return false;

	// Get the direction we're firing.
	LTRotation rot;
	g_pLTServer->GetObjectRotation( m_hObject, &rot );
	LTVector vDir = rot.Forward( );

	LTVector vPos;
	if( !GetFirePos( vPos ))
		return false;

	// Tell the weapon to fire.
	WeaponFireInfo weaponFireInfo;
	weaponFireInfo.hFiredFrom  = m_hObject;
	weaponFireInfo.vPath       = vDir;
	weaponFireInfo.vFirePos    = vPos;
	weaponFireInfo.vFlashPos   = vPos;

	WeaponState eWeaponState = pWeapon->UpdateWeapon( weaponFireInfo, true );
	if( eWeaponState == W_FIRED )
	{
		// Count the round as fired.
		if( m_nRoundsToFire > 0 )
			m_nRoundsToFire--;

	}

	// Always make sure we're ready to fire.
	m_Weapons.AddAmmo( pWeapon->GetAmmoId( ), 10000 );
	pWeapon->ReloadClip( LTFALSE );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::GetFirePos
//
//	PURPOSE:	Get the fire (flash) position of the hand-held weapon
//
// ----------------------------------------------------------------------- //

bool GunMount::GetFirePos( LTVector& vPos )
{
	CWeapon* pWeapon = m_Weapons.GetCurWeapon( );
	if( !pWeapon )
		return false;

	HATTACHMENT hAttachment;
    if( g_pLTServer->FindAttachment(m_hObject, pWeapon->GetModelObject(), &hAttachment) != LT_OK )
	{
		return false;
	}

	HMODELSOCKET hSocket;
    if (g_pModelLT->GetSocket(pWeapon->GetModelObject(), "Flash", hSocket) != LT_OK)
	{
		return false;
	}

	LTransform transform;
	g_pCommonLT->GetAttachedModelSocketTransform(hAttachment, hSocket, transform);
	vPos = transform.m_Pos;
	
	return true;
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::GetFirePos
//
//	PURPOSE:	Changes visibility on gun.
//
// ----------------------------------------------------------------------- //
void GunMount::SetVisible( bool bVisible )
{
	m_bVisible = bVisible;

	CWeapon* pWeapon = m_Weapons.GetCurWeapon( );
	if( pWeapon )
	{
		g_pCommonLT->SetObjectFlags( pWeapon->GetModelObject( ), OFT_Flags, m_bVisible ? ( FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE ) : FLAG_FORCECLIENTUPDATE, ( FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE ));
	}
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	GunMountPlugin
//              
//	PURPOSE:	Dedit Plugin module.
//              
//----------------------------------------------------------------------------


LTRESULT GunMountPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	static bool bInitted = false;
	static CWeaponMgrPlugin weaponMgrPlugin;

	if ( !bInitted )
	{
		bInitted = true;
		weaponMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
	}

	if( stricmp( "Weapon", szPropName ) == 0 )
	{
		strcpy( aszStrings[*pcStrings], NOWEAPON );
		(*pcStrings)++;
		weaponMgrPlugin.PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT GunMountPlugin::PreHook_Dims(const char* szRezPath,
										 const char* szPropValue,
										 char* szModelFilenameBuf,
										 int nModelFilenameBufLen,
										 LTVector & vDims )
{

	if (!szModelFilenameBuf || nModelFilenameBufLen < 1 || !g_pWeaponMgr ) return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';

	// Remove the , that is put into some weapon names.
	char szModifiedPropValue[256];
	SAFE_STRCPY( szModifiedPropValue, szPropValue );
	strtok( szModifiedPropValue, "," );

	WEAPON const* pWeapon = g_pWeaponMgr->GetWeapon(( char* )szModifiedPropValue);
	if( !pWeapon || !pWeapon->szHHModel[0] )
	{
		return LT_UNSUPPORTED;
	}

	strcpy( szModelFilenameBuf, pWeapon->szHHModel );

	// Need to convert the .ltb filename to one that DEdit understands...
	
	ConvertLTBFilename( szModelFilenameBuf );

	return LT_OK;
}
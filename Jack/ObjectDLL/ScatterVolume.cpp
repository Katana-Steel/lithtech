// ----------------------------------------------------------------------- //
//
// MODULE  : ScatterVolume.cpp
//
// PURPOSE : Scattered surface particles volume implementation:
//           - scatter specific parameters
//
// CREATED : 4/3/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MsgIDs.h"
#include "ScatterVolume.h"

extern HSTRING ReadHStringProp(const char* pszPropName, ILTServer* pServer);


LINKFROM_MODULE( ScatterVolume );

BEGIN_CLASS(ScatterVolume)
	ADD_REALPROP_FLAG(BlindDataIndex, -1.0f, PF_HIDDEN)
	ADD_REALPROP(Density, 128.0f)
	ADD_REALPROP(Clumpiness, 0.0f)
	ADD_REALPROP(ClumpSize, 256.0f)
	ADD_REALPROP(ClumpCutoff, 0.5f)
	ADD_VECTORPROP_VAL(ClumpOffset, 0.0f, 0.0f, 0.0f)
	ADD_REALPROP(Width, 64.0f)
	ADD_REALPROP(Height, 64.0f)
	ADD_REALPROP(PlantDepth, 0.0f)
	ADD_REALPROP(Tilt, 30.0f)
	ADD_REALPROP(ScaleRange, 0.2f)
	ADD_REALPROP(WaveRate, 90.0f)
	ADD_REALPROP(WaveDist, 10.0f)
	ADD_REALPROP(WaveSize, 128.0f)
	ADD_REALPROP(WaveDirSize, 128.0f)
	ADD_REALPROP(MaxDrawDist, 1024.0f)
	ADD_STRINGPROP_FLAG(TextureName, "", PF_FILENAME)
	ADD_STRINGPROP_FLAG(PlacementTexture, "", PF_FILENAME)
	ADD_REALPROP(PlacementCutoff, 0.5f)
	ADD_BOOLPROP(UseSaturate, TRUE)
	ADD_COLORPROP(AmbientLight, 0.0f, 0.0f, 0.0f)
	ADD_BOOLPROP(OnlyUseAmbient, FALSE)
END_CLASS_DEFAULT(ScatterVolume, VolumeEffect, NULL, NULL)



ScatterVolume::ScatterVolume()
{
	m_nBlindDataIndex = 0xffffffff;
	m_fHeight = 64.0f;
	m_fWidth = 64.0f;
	m_fMaxScale = 1.0f;
	m_fTilt = 30.0f;
	m_fWaveRate = 90.0f;
	m_fWaveDist = 10.0f;
	m_fMaxDrawDist = 1024.0f;
	m_hstrTextureName = LTNULL;
	m_bUseSaturate = true;
}


ScatterVolume::~ScatterVolume()
{
	if( g_pLTServer )
	{
		if( m_hstrTextureName )
		{
			g_pLTServer->FreeString( m_hstrTextureName );
		}
	}
}


uint32 ScatterVolume::EngineMessageFn( uint32 messageID, void* pData, LTFLOAT fData )
{
	switch( messageID )
	{
	case MID_PRECREATE:
		{
			ObjectCreateStruct* pOCS = (ObjectCreateStruct*)pData;
			if( !pOCS )
				break;

			if( fData == PRECREATE_WORLDFILE )
			{
				ReadProp( pOCS );
				// make sure this object is always in the visible object list
				pOCS->m_Flags |= FLAG_FORCECLIENTUPDATE;
			}
		}
		break;

	case MID_INITIALUPDATE:
		{
			if( fData == INITIALUPDATE_WORLDFILE )			
			{
				InitialUpdate();
			}
		}
		break;

	}

	return VolumeEffect::EngineMessageFn( messageID, pData, fData );
}


bool ScatterVolume::ReadProp( ObjectCreateStruct* pOCS )
{
	if( !pOCS )
		return false;

	float tmp = -2.0f;
	g_pLTServer->GetPropReal( "BlindDataIndex", &tmp );
	if( tmp > 0.0f )
		m_nBlindDataIndex = (uint32)tmp;
	else
		m_nBlindDataIndex = 0xffffffff;

	g_pLTServer->GetPropReal( "Height", &m_fHeight );
	g_pLTServer->GetPropReal( "Width", &m_fWidth );
	g_pLTServer->GetPropReal( "Tilt", &m_fTilt );
	g_pLTServer->GetPropReal( "WaveRate", &m_fWaveRate );
	g_pLTServer->GetPropReal( "WaveDist", &m_fWaveDist );
	g_pLTServer->GetPropReal( "MaxDrawDist", &m_fMaxDrawDist );

	g_pLTServer->GetPropBool( "UseSaturate", &m_bUseSaturate );

	if( g_pLTServer->GetPropReal( "ScaleRange", &tmp ) == LT_OK )
		m_fMaxScale = 1.0f + tmp;
	else
		m_fMaxScale = 1.0f;

	m_hstrTextureName = ReadHStringProp( "TextureName", g_pLTServer );
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SnowVolume::CreateSpecialFX
//
//  PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //

void ScatterVolume::CreateSpecialFX( bool bUpdateClients /* = false  */ )
{
	CAutoMessage cMsg;

	cMsg.Writeuint8( SFX_SCATTER_ID );
	WriteScatterInfo( cMsg );
	g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read() );

	if( bUpdateClients )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_SCATTER_ID );
		cMsg.WriteObject( m_hObject );
		WriteScatterInfo( cMsg );
		g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );
	}
}

void ScatterVolume::InitialUpdate( void )
{
	CreateSpecialFX( );
}


void ScatterVolume::WriteScatterInfo( ILTMessage_Write& cMsg )
{
	cMsg.WriteLTVector( m_vDims );
	cMsg.Writeuint32( m_nBlindDataIndex );
	cMsg.Writefloat( m_fHeight );
	cMsg.Writefloat( m_fWidth );
	cMsg.Writefloat( m_fMaxScale );
	cMsg.Writefloat( m_fTilt );
	cMsg.Writefloat( m_fWaveRate );
	cMsg.Writefloat( m_fWaveDist );
	cMsg.Writefloat( m_fMaxDrawDist );
	cMsg.WriteHString( m_hstrTextureName );
	cMsg.Writeuint32( m_bUseSaturate );
}

void ScatterVolume::Save( ILTMessage_Write *pMsg )
{
	if( !pMsg )
		return;

	VolumeEffect::Save( pMsg );

	SAVE_VECTOR( m_vDims );
	SAVE_DWORD( m_nBlindDataIndex );
	SAVE_FLOAT( m_fHeight );
	SAVE_FLOAT( m_fWidth );
	SAVE_FLOAT( m_fMaxScale );
	SAVE_FLOAT( m_fTilt );
	SAVE_FLOAT( m_fWaveRate );
	SAVE_FLOAT( m_fWaveDist );
	SAVE_FLOAT( m_fMaxDrawDist );
	SAVE_HSTRING( m_hstrTextureName );
	SAVE_bool( m_bUseSaturate );
}

void ScatterVolume::Load( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	VolumeEffect::Load( pMsg );

	LOAD_VECTOR( m_vDims );
	LOAD_DWORD( m_nBlindDataIndex );
	LOAD_FLOAT( m_fHeight );
	LOAD_FLOAT( m_fWidth );
	LOAD_FLOAT( m_fMaxScale );
	LOAD_FLOAT( m_fTilt );
	LOAD_FLOAT( m_fWaveRate );
	LOAD_FLOAT( m_fWaveDist );
	LOAD_FLOAT( m_fMaxDrawDist );
	LOAD_HSTRING( m_hstrTextureName );
	LOAD_bool( m_bUseSaturate );

	CreateSpecialFX( true );
}

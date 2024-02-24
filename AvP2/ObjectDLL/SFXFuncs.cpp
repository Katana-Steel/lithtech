// ----------------------------------------------------------------------- //
//
// MODULE  : SFXFuncs.cpp
//
// PURPOSE : Misc functions used with special fx
//
// CREATED : 6/9/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// Includes...

#include "stdafx.h"
#include "SFXFuncs.h"
#include "SFXMsgIds.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetLensFlareProperties()
//
//	PURPOSE:	Read in the lensflare properties (This should only be
//				called during an object's ReadProp function if the object
//				added the ADD_LENSFLARE_PROPERTIES macro).
//
// ----------------------------------------------------------------------- //

void GetLensFlareProperties(LENSFLARE & lensProps)
{
	if (!g_pServerDE) return;

	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("InSkyBox", &genProp ) == DE_OK)
	{
		lensProps.bInSkyBox = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("MinAngle", &genProp ) == DE_OK)
	{
		lensProps.fMinAngle = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("CreateSprite", &genProp ) == DE_OK)
	{
		lensProps.bCreateSprite = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("SpriteOnly", &genProp ) == DE_OK)
	{
		lensProps.bSpriteOnly = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("UseObjectAngle", &genProp ) == DE_OK)
	{
		lensProps.bUseObjectAngle = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("SpriteOffset", &genProp ) == DE_OK)
	{
		lensProps.fSpriteOffset = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("MinSpriteAlpha", &genProp ) == DE_OK)
	{
		lensProps.fMinSpriteAlpha = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("MaxSpriteAlpha", &genProp ) == DE_OK)
	{
		lensProps.fMaxSpriteAlpha = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("MinSpriteScale", &genProp ) == DE_OK)
	{
		lensProps.fMinSpriteScale = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("MaxSpriteScale", &genProp ) == DE_OK)
	{
		lensProps.fMaxSpriteScale = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("SpriteFile", &genProp ) == DE_OK)
	{
		lensProps.SetSpriteFile(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("BlindingFlare", &genProp ) == DE_OK)
	{
		lensProps.bBlindingFlare = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("BlindObjectAngle", &genProp ) == DE_OK)
	{
		lensProps.fBlindObjectAngle = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("BlindCameraAngle", &genProp ) == DE_OK)
	{
		lensProps.fBlindCameraAngle = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("MinBlindScale", &genProp ) == DE_OK)
	{
		lensProps.fMinBlindScale = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("MaxBlindScale", &genProp ) == DE_OK)
	{
		lensProps.fMaxBlindScale = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("SpriteAdditive", &genProp ) == DE_OK)
	{
		lensProps.bSpriteAdditive = genProp.m_Bool;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildLensFlareSFXMessage()
//
//	PURPOSE:	Build the lens flare special fx message...
//
// ----------------------------------------------------------------------- //

void BuildLensFlareSFXMessage(LENSFLARE & lensProps, LPBASECLASS pClass)
{
	if (!pClass) return;

	HMESSAGEWRITE hMessage = g_pServerDE->StartSpecialEffectMessage(pClass);
	g_pServerDE->WriteToMessageByte(hMessage, SFX_LENSFLARE_ID);
	::AddLensFlareInfoToMessage(lensProps, hMessage);
	g_pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildLensFlareSFXMessage()
//
//	PURPOSE:	Build the lens flare special fx message...
//
// ----------------------------------------------------------------------- //

void AddLensFlareInfoToMessage(LENSFLARE & lensProps, HMESSAGEWRITE hMessage)
{
	if (!hMessage) return;

	g_pServerDE->WriteToMessageByte(hMessage, lensProps.bInSkyBox);
	g_pServerDE->WriteToMessageByte(hMessage, lensProps.bCreateSprite);
	g_pServerDE->WriteToMessageByte(hMessage, lensProps.bSpriteOnly);
	g_pServerDE->WriteToMessageByte(hMessage, lensProps.bUseObjectAngle);
	g_pServerDE->WriteToMessageByte(hMessage, lensProps.bSpriteAdditive);
	g_pServerDE->WriteToMessageFloat(hMessage, lensProps.fSpriteOffset);
	g_pServerDE->WriteToMessageFloat(hMessage, lensProps.fMinAngle);
	g_pServerDE->WriteToMessageFloat(hMessage, lensProps.fMinSpriteAlpha);
	g_pServerDE->WriteToMessageFloat(hMessage, lensProps.fMaxSpriteAlpha);
	g_pServerDE->WriteToMessageFloat(hMessage, lensProps.fMinSpriteScale);
	g_pServerDE->WriteToMessageFloat(hMessage, lensProps.fMaxSpriteScale);
	g_pServerDE->WriteToMessageHString(hMessage, lensProps.hstrSpriteFile);
	g_pServerDE->WriteToMessageByte(hMessage, lensProps.bBlindingFlare);
	g_pServerDE->WriteToMessageFloat(hMessage, lensProps.fBlindObjectAngle);
	g_pServerDE->WriteToMessageFloat(hMessage, lensProps.fBlindCameraAngle);
	g_pServerDE->WriteToMessageFloat(hMessage, lensProps.fMinBlindScale);
	g_pServerDE->WriteToMessageFloat(hMessage, lensProps.fMaxBlindScale);
}

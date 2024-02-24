// ----------------------------------------------------------------------------
//
// MODULE  : LightGroup.cpp
//
// PURPOSE : LightGroup - Implementation
//
// CREATED : 12/21/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------------

#include "stdafx.h"
#include "LightGroup.h"
#include "lmessage.h"
#include "CommonUtilities.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"

#include "SFXMsgIds.h"
#include "iltlightanim.h"

#ifndef __MSG_IDS_H__
#include "MsgIDs.h"
#endif

#ifndef __LIGHT_FX_H__
#include "ClientLightFX.h"
#endif

#pragma warning(disable : 4786)
#include <map>
#include <vector>
#include <string>
#include <algorithm>


#define LIGHTGROUP

LightGroup::LightGroupList LightGroup::m_sLightGroups;

/*
	DFLOAT	m_fIntensityDelta;			// What is the total change so far
	DFLOAT  m_fIntensityMin;			// How Dark light gets
	DFLOAT  m_fIntensityMax;			// How Bright light gets
	DFLOAT  m_fRadiusMin;				// How small light gets
	DFLOAT  m_fRadiusMax;				// How large light gets
*/

//-----------------------------------------------------------------------------
// L O C A L   F U N C T I O N S
//-----------------------------------------------------------------------------
bool GetLightData(PreLightInfo& lightInfo, DBOOL& bUseShadowMaps, 
				  PreLightLT *pInterface, HPREOBJECT hObjectLight);

//-----------------------------------------------------------------------------
// S T A T I C   O B J E C T S
//-----------------------------------------------------------------------------
const std::string LightGroup::LIGHTFX_LIGHTGROUP_EXTENSION = "__LG";
static const float UPDATE_DELTA	= 0.1f;


//-----------------------------------------------------------------------------
// CLASS:		LightGroup
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// This set of macros create an object that DEdit can see and manipulate
//-----------------------------------------------------------------------------
BEGIN_CLASS(LightGroup)
 	ADD_DESTRUCTIBLE_AGGREGATE(PF_GROUP1, 0)
	ADD_BOOLPROP(StartOn, DTRUE)

	ADD_COLORPROP(Color, 255.0f, 255.0f, 255.0f)

	ADD_LONGINTPROP(Waveform, WAVE_NONE)

	ADD_REALPROP(IntensityMin, 0.5f)		 
	ADD_REALPROP(IntensityMax, 1.0f)		 
	ADD_REALPROP(IntensityFreq, 4.0f)
	ADD_REALPROP(IntensityPhase, 0.0f)

	ADD_REALPROP_FLAG(RadiusMin, 200.0f, PF_RADIUS)		   
	ADD_REALPROP_FLAG(RadiusMax, 500.0f, PF_RADIUS)			 
//	ADD_LONGINTPROP(RadiusWaveform, WAVE_NONE)
	ADD_REALPROP(RadiusFreq, 4.0f)
	ADD_REALPROP(RadiusPhase, 0.0f)

	ADD_STRINGPROP_FLAG(RampUpSound,   "", PF_FILENAME)	
	ADD_STRINGPROP_FLAG(RampDownSound, "", PF_FILENAME)

	ADD_REALPROP(LifeTime, 0.0f)		  
	ADD_REALPROP(HitPoints, 1.0f)

	ADD_BOOLPROP(CastShadowsFlag, DFALSE)
	ADD_BOOLPROP(SolidLightFlag, DFALSE)
	ADD_BOOLPROP_FLAG_HELP(OnlyLightWorldFlag, DFALSE, 0, "If this is set to true then the lights in this group will only light the world and not game models like the player view model." )
	ADD_BOOLPROP(DontLightBackfacingFlag, DFALSE)

	ADD_STRINGPROP_FLAG(Object1, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius1, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight1, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object2, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius2, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight2, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object3, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color3, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position3, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius3, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight3, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object4, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color4, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position4, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius4, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight4, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object5, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color5, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position5, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius5, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight5, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object6, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color6, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position6, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius6, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight6, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object7, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color7, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position7, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius7, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight7, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object8, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color8, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position8, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius8, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight8, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object9, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color9, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position9, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius9, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight9, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object10, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color10, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position10, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius10, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight10, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object11, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color11, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position11, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius11, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight11, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object12, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color12, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position12, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius12, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight12, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object13, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color13, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position13, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius13, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight13, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object14, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color14, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position14, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius14, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight14, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object15, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color15, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position15, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius15, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight15, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object16, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color16, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position16, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius16, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight16, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object17, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color17, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position17, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius17, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight17, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object18, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color18, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position18, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius18, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight18, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object19, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color19, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position19, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius19, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight19, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object20, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color20, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position20, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius20, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight20, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object21, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color21, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position21, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius21, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight21, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object22, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color22, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position22, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius22, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight22, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object23, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color23, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position23, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius23, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight23, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object24, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color24, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position24, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius24, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight24, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object25, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color25, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position25, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius25, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight25, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object26, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color26, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position26, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius26, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight26, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object27, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color27, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position27, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius27, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight27, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object28, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color28, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position28, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius28, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight28, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object29, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color29, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position29, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius29, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight29, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object30, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color30, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position30, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius30, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight30, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object31, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color31, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position31, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius31, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight31, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object32, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color32, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position32, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius32, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight32, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object33, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color33, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position33, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius33, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight33, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object34, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color34, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position34, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius34, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight34, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object35, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color35, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position35, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius35, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight35, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object36, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color36, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position36, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius36, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight36, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object37, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color37, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position37, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius37, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight37, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object38, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color38, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position38, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius38, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight38, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object39, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color39, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position39, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius39, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight39, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object40, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color40, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position40, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius40, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight40, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object41, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color41, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position41, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius41, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight41, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object42, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color42, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position42, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius42, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight42, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object43, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color43, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position43, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius43, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight43, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object44, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color44, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position44, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius44, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight44, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object45, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color45, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position45, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius45, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight45, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object46, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color46, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position46, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius46, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight46, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object47, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color47, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position47, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius47, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight47, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object48, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color48, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position48, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius48, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight48, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object49, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color49, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position49, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius49, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight49, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Object50, "", PF_HIDDEN)
	ADD_COLORPROP_FLAG(Color50, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Position50, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Radius50, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(ModelLight50, LTFALSE, PF_HIDDEN)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS_PLUGIN(LightGroup, BaseClass, NULL, NULL, 0, CLightGroupFXPlugin)

// ----------------------------------------------------------------------------
//
//	ROUTINE:	LightGroup::LightGroup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------------
LightGroup::LightGroup() : m_bOn(DTRUE), m_vColor(0.0f, 0.0f, 0.0f),
	m_dwLightFlags(0), m_fIntensityDelta(0.0f),	m_fIntensityMin(0.5f), m_fIntensityMax(1.0f), m_nIntensityWaveform(WAVE_NONE),
	m_fIntensityFreq(4.0f), m_fIntensityPhase(0), m_fRadiusMin(0.0f), m_fRadiusMax(500.0f),
	m_nRadiusWaveform(WAVE_NONE), m_fRadiusFreq(0.0f), m_fRadiusPhase(4.0f), m_hstrRampUpSound(DNULL),
	m_hstrRampDownSound(DNULL), m_bUseLightAnims(DTRUE), m_bDynamic(DTRUE), m_fLifeTime(-1.0f),
	m_fStartTime(0.0f), m_fHitPts(1.0f)
{
	m_sLightGroups.push_back(this);
}

// ----------------------------------------------------------------------------
//
//	ROUTINE:	LightGroup::~LightGroup
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------------
LightGroup::~LightGroup()
{
	for (LightList::iterator iter = m_LightList.begin(); iter != m_LightList.end(); ++iter)
	{
		if (iter->m_hName)
		{
			g_pServerDE->FreeString(iter->m_hName);
		}
	}

	LightGroupList::iterator iter_to_me = std::find(m_sLightGroups.begin(), m_sLightGroups.end(), this);
	if( iter_to_me != m_sLightGroups.end() )
	{
		m_sLightGroups.erase(iter_to_me);
	}
}

// ----------------------------------------------------------------------------
//
//	ROUTINE:	LightObject functions
//
//	PURPOSE:	To allow AI visibility tests with lights.
//
// ----------------------------------------------------------------------------
LTVector LightGroup::GetPos() const
{
	LTVector vPos(0,0,0);
	g_pLTServer->GetObjectPos(m_hObject,&vPos);
	return vPos;
}


LTFLOAT  LightGroup::GetRadiusSqr() const
{
	float radScale = m_fIntensityDelta;
	if (radScale < -1.0f)
	{
		radScale = -1.0f;
	}
	else if( radScale > 0.0f )
	{
		radScale = 0.0f;
	}

	const float radMax = m_fRadiusMax * (1.0f + radScale);
	const float radMin = m_fRadiusMin * (1.0f + radScale);
	const float radAvg = (radMax+radMin)*0.5f;

	return radAvg*radAvg;
}	

LTVector LightGroup::GetColor(const LTVector & vLocation) const
{
	const DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);

	// If light is turned off, return black.
	if( !(dwUsrFlags & USRFLG_VISIBLE) ) 
		return LTVector(0,0,0);

	float intMax = m_fIntensityMax + m_fIntensityDelta;
	float intMin = m_fIntensityMin + m_fIntensityDelta;

	if (intMin < 0)
	{
		intMin = 0;
	}

	if (intMin > 1.0)
	{
		intMin = 1.0;
	}

	if (intMax < 0)
	{
		intMax = 0;
	}

	if (intMax > 1.0)
	{
		intMax = 1.0;
	}

	return m_vColor*((intMax + intMin)*0.5f);
}

// ----------------------------------------------------------------------------
//
//	ROUTINE:	LightGroup::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------------
DWORD LightGroup::EngineMessageFn(DWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			// I don't know if I want this. I'll add it back when I know more
//			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
//			{
				m_bDynamic = DFALSE;
				ReadProp(static_cast<ObjectCreateStruct*>(pData));
				PreCreate(static_cast<ObjectCreateStruct*>(pData));
//			}

			PostPropRead(static_cast<ObjectCreateStruct*>(pData));

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate((DVector *)pData);
				MakeLights();
				SendEffectMessage();
			}
			CacheFiles();
			break;
		}

		case MID_UPDATE:
		{
			if (!Update()) 
			{
				CServerDE* pServerDE = BaseClass::GetServerDE();
				if (pServerDE) pServerDE->RemoveObject(m_hObject);
			}
		}
		break;

		case MID_LOADOBJECT:
		{
			Load(static_cast<HMESSAGEREAD>(pData), static_cast<DWORD>(fData));
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save(static_cast<HMESSAGEWRITE>(pData), static_cast<DWORD>(fData));
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------------
//
//	ROUTINE:	LightGroup::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------------
DWORD LightGroup::ObjectMessageFn(HOBJECT hSender, DWORD messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
 		case MID_TRIGGER:
		{
			HandleTrigger(hSender, hRead);
			break;
		}

		default : break;
	}

	return BaseClass::ObjectMessageFn (hSender, messageID, hRead);
}

// ----------------------------------------------------------------------------
//
//	ROUTINE:	LightGroup::PreCreate
//
//	PURPOSE:	Read through the attached object list and add those lights to
//				the group list.
//
// ----------------------------------------------------------------------------
// !!!!!JKE This should go away, and have the light objects removed from
// the game after the lightmaps have been made
void LightGroup::PreCreate(ObjectCreateStruct *pStruct)
{
	char buf[100];
	GenericProp genProp;

	// Get name of the group object
	g_pServerDE->GetPropGeneric("Name", &genProp);
	std::string groupName = std::string(genProp.m_String);

	// Go through all the data slots
	for (int i=0; i < MAX_LIGHTGROUP_TARGETS; ++i)
	{
		// Make a title based on our number
		sprintf(buf, "Object%d", i+1);
		if (g_pServerDE->GetPropGeneric(buf, &genProp) == LT_OK)
		{
			// If we have a name
			if (genProp.m_String[0])
			{
				// record that this object is part of a group.
				LightManager::GetInstance()->AddName(genProp.m_String, groupName);

				// Add the def to our light group object
				LightDef lightDef;
				lightDef.m_hName = g_pServerDE->CreateString(genProp.m_String);

				// Make a title for color
				sprintf(buf, "Color%d", i+1);
				g_pServerDE->GetPropGeneric(buf, &genProp);
				lightDef.m_Color = genProp.m_Color;

				// Make a title for the Position
				sprintf(buf, "Position%d", i+1);
				g_pServerDE->GetPropGeneric(buf, &genProp);
				lightDef.m_Pos = genProp.m_Vec;

				// Make Radius
				sprintf(buf, "Radius%d", i+1);
				g_pServerDE->GetPropGeneric(buf, &genProp);
				lightDef.m_Radius = genProp.m_Float;

				// Should this light models?
				sprintf(buf, "ModelLight%d", i+1);
				g_pServerDE->GetPropGeneric(buf, &genProp);
				lightDef.m_ModelLight = genProp.m_Bool;

				// Add it to our internal list
				m_LightList.push_back(lightDef);
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
//	ROUTINE:	LightGroup::ReadProp
//
//	PURPOSE:	Read the dynamic light properties that should be sent to the client
//
// ----------------------------------------------------------------------------
// !!!!!JKE This should be an object by itself. There is too much functionality here
DBOOL LightGroup::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	GenericProp genProp;

	if (pServerDE->GetPropGeneric("HitPoints", &genProp) == DE_OK)
		m_fHitPts = genProp.m_Float;

	if (pServerDE->GetPropGeneric("StartOn", &genProp) == DE_OK)
		m_bOn = genProp.m_Bool;

	if (pServerDE->GetPropGeneric("LifeTime", &genProp) == DE_OK)
		m_fLifeTime = genProp.m_Float;

	if (m_fLifeTime < 0.0f) m_fLifeTime = 0.0f;

	if (pServerDE->GetPropGeneric("Color", &genProp) == DE_OK)
		VEC_COPY(m_vColor, genProp.m_Color);

	if (pServerDE->GetPropGeneric("IntensityMin", &genProp) == DE_OK)
		m_fIntensityMin = genProp.m_Float;

	if (m_fIntensityMin < 0.0f) m_fIntensityMin = 0.0f;

	if (pServerDE->GetPropGeneric("IntensityMax", &genProp) == DE_OK)
		m_fIntensityMax = genProp.m_Float;

	if (m_fIntensityMax > 255.0f)   m_fIntensityMax = 255.0f;

	if (pServerDE->GetPropGeneric("Waveform", &genProp) == DE_OK)
		m_nIntensityWaveform = (DBYTE) genProp.m_Long;

	if (pServerDE->GetPropGeneric("IntensityFreq", &genProp) == DE_OK)
		m_fIntensityFreq = genProp.m_Float;

	if (pServerDE->GetPropGeneric("IntensityPhase", &genProp) == DE_OK)
		m_fIntensityPhase = genProp.m_Float;

	if (pServerDE->GetPropGeneric("RadiusMin", &genProp) == DE_OK)
		m_fRadiusMin = genProp.m_Float;

	if (m_fRadiusMin < 0.0f) m_fRadiusMin = 0.0f;

	if (pServerDE->GetPropGeneric("RadiusMax", &genProp) == DE_OK)
		m_fRadiusMax = genProp.m_Float;

	if (pServerDE->GetPropGeneric("Waveform", &genProp) == DE_OK)
		m_nRadiusWaveform = (DBYTE) genProp.m_Long;

	if (pServerDE->GetPropGeneric("RadiusFreq", &genProp) == DE_OK)
		m_fRadiusFreq = genProp.m_Float;

	if (pServerDE->GetPropGeneric("RadiusPhase", &genProp) == DE_OK)
		m_fRadiusPhase = genProp.m_Float;

	if (pServerDE->GetPropGeneric("RampUpSound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrRampUpSound = pServerDE->CreateString(genProp.m_String);
	}

	if (pServerDE->GetPropGeneric("RampDownSound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrRampDownSound = pServerDE->CreateString(genProp.m_String);
	}

	if (pServerDE->GetPropGeneric("CastShadowsFlag", &genProp) == DE_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_CASTSHADOWS : 0;
	}

	if (pServerDE->GetPropGeneric("SolidLightFlag", &genProp) == DE_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_SOLIDLIGHT : 0;
	}

	if (pServerDE->GetPropGeneric("OnlyLightWorldFlag", &genProp) == DE_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_ONLYLIGHTWORLD : 0;
	}

	if (pServerDE->GetPropGeneric("DontLightBackfacingFlag", &genProp) == DE_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_DONTLIGHTBACKFACING : 0;
	}

	m_bUseLightAnims = DTRUE;

	return DTRUE;
}

// ----------------------------------------------------------------------------
//
//	ROUTINE:	LightGroup::PostPropRead()
//
//	PURPOSE:	After everything exists, send us a message so we can turn on
//
// ----------------------------------------------------------------------------
void LightGroup::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// Set the Update!

//	if (m_bOn)
//	{	
		pStruct->m_NextUpdate = 0.01f;
//	}
//	else
//	{
//		pStruct->m_NextUpdate = 0.0f;
//	}

	pStruct->m_Flags = FLAG_VISIBLE;
	pStruct->m_Flags |= FLAG_GOTHRUWORLD;
}


// ----------------------------------------------------------------------------
//
//	ROUTINE:	LightGroup::InitialUpdate()
//
//	PURPOSE:	Set up the damage object, and update ourselves again.
//				Is this second update needed?
//
// ----------------------------------------------------------------------------
DBOOL LightGroup::InitialUpdate(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	//now reset the rayhit flag
//	uint32 dwFlags;
//	g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
//	g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags & ~FLAG_RAYHIT);

	// Set Next update (randomize it if this object was loaded from the
	// level - so we don't have all the lights updating on the same frame)...
	
	DFLOAT fOffset = 0.0f;
	if (!m_bDynamic) fOffset = pServerDE->Random(0.01f, 0.5f);

//	if (m_bOn)
//	{
		pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA + fOffset);
//	}

	Init();


	return DTRUE;
}


// ----------------------------------------------------------------------------
//
//	ROUTINE:	LightGroup::Init()
//
//	PURPOSE:	Convert Phase from degrees to radians, log start time, 
//				make us visible, and give us some nominal dimensions
//
// ----------------------------------------------------------------------------
DBOOL LightGroup::Init()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	VEC_DIVSCALAR(m_vColor, m_vColor, 255.0f);

	m_fStartTime = pServerDE->GetTime();

	m_fIntensityPhase = MATH_DEGREES_TO_RADIANS(m_fIntensityPhase);
	m_fRadiusPhase = MATH_DEGREES_TO_RADIANS(m_fRadiusPhase);

	if (m_bOn)
	{
		DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject);
		dwUsrFlags |= USRFLG_VISIBLE;
		pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);
	}

	// Set the dims to something to avoid situations where the object is considered
	// invisible even though it's visible.
	float fDims = DMAX(m_fRadiusMin, 5.0f);
	DVector vDims(fDims, fDims, fDims);
	pServerDE->SetObjectDims(m_hObject, &vDims);

	return DTRUE;
}

// ----------------------------------------------------------------------------
//
//	ROUTINE:	LightGroup::CacheFiles
//
//	PURPOSE:	Cache sound resources used by the object
//
// ----------------------------------------------------------------------------
void LightGroup::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pFile = DNULL;

	if (m_hstrRampUpSound)
	{
		pFile = g_pServerDE->GetStringData(m_hstrRampUpSound);
		if (pFile)
		{
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
		}
	}

	if (m_hstrRampDownSound)
	{
		pFile = g_pServerDE->GetStringData(m_hstrRampDownSound);
		if (pFile)
		{
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
		}
	}
}

// ----------------------------------------------------------------------------
//
//	ROUTINE:	LightGroup::SendEffectMessage
//
//	PURPOSE:	Sends a message to the client to start a light effect
//
// ----------------------------------------------------------------------------

void LightGroup::SendEffectMessage()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Get our name
	std::string objectName(pServerDE->GetObjectName(m_hObject));

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_LIGHT_ID);

	pServerDE->WriteToMessageVector(hMessage, &m_vColor);
	pServerDE->WriteToMessageDWord(hMessage, m_dwLightFlags);
	pServerDE->WriteToMessageFloat(hMessage, m_fIntensityMin);
	pServerDE->WriteToMessageFloat(hMessage, m_fIntensityMax);
	pServerDE->WriteToMessageByte(hMessage, m_nIntensityWaveform);
	pServerDE->WriteToMessageFloat(hMessage, m_fIntensityFreq);
	pServerDE->WriteToMessageFloat(hMessage, m_fIntensityPhase);
	pServerDE->WriteToMessageFloat(hMessage, m_fRadiusMin);
	pServerDE->WriteToMessageFloat(hMessage, m_fRadiusMax);
	pServerDE->WriteToMessageByte(hMessage, m_nRadiusWaveform);
	pServerDE->WriteToMessageFloat(hMessage, m_fRadiusFreq);
	pServerDE->WriteToMessageFloat(hMessage, m_fRadiusPhase);
	pServerDE->WriteToMessageWord(hMessage, -1);
	pServerDE->WriteToMessageWord(hMessage, -1);
	pServerDE->WriteToMessageByte(hMessage, (DBYTE)m_bUseLightAnims);

	// Write the animation handle.	
	if(m_bUseLightAnims)
	{
		std::string lightAnimName = objectName + LightGroup::LIGHTFX_LIGHTGROUP_EXTENSION;

		HLIGHTANIM hLightAnim = INVALID_LIGHT_ANIM;
		pServerDE->GetLightAnimLT()->FindLightAnim(lightAnimName.c_str(), hLightAnim);

		pServerDE->WriteToMessageDWord(hMessage, (DDWORD)hLightAnim);
	}

	pServerDE->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightGroup::Update
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
DBOOL LightGroup::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);

	DBOOL bRemove = DFALSE;
	if (m_fLifeTime > 0 && (pServerDE->GetTime() - m_fStartTime) >= m_fLifeTime)
	{
		bRemove = DTRUE;
	}

	return (!bRemove);
}

// ----------------------------------------------------------------------------
//
//	ROUTINE:	LightGroup::Load
//
//	PURPOSE:	Handle loading object
//
// ----------------------------------------------------------------------------
void LightGroup::Load(HMESSAGEREAD hRead, DWORD dwSaveFlags)
{
	int i = 0;

	// I am assuming that PreCreate gets called after this. !!!JKE
	if (!hRead) return;

	// Only need to save the data that changes (all the data in the
	// special fx message is saved/loaded for us)...
	m_bOn = hRead->ReadByte();
	m_bDynamic = hRead->ReadByte();
	m_fLifeTime = hRead->ReadFloat();
	m_fStartTime = hRead->ReadFloat() + g_pLTServer->GetTime();

	// We may never need this !!!JKE
	// We should remove the light objects from the world after they have
	// provided the data they have
	int size = hRead->ReadDWord();
	for (i=0; i < size; i++)
	{
		LightDef lightDef;
		lightDef.m_hName = hRead->ReadHString();
		lightDef.m_Color = hRead->ReadVector();
		lightDef.m_Pos = hRead->ReadVector();
		lightDef.m_Radius = hRead->ReadFloat();
		lightDef.m_ModelLight = hRead->ReadByte();
		m_LightList.push_back(lightDef);
	}

	// Reload our list...
	uint32 count;
	*hRead >> count;

	for(i=0 ; i<(int)count; i++)
	{
		HOBJECT hObj;
		*hRead >> hObj;

		m_ObjectList.push_back(hObj);
	}
}

// ----------------------------------------------------------------------------
//
//	ROUTINE:	LightGroup::Save
//
//	PURPOSE:	Handle saving object
//
// ----------------------------------------------------------------------------
void LightGroup::Save(HMESSAGEWRITE hWrite, DWORD dwSaveFlags)
{
	if (!hWrite) return;

	// Only need to save the data that changes (all the data in the
	// special fx message is saved/loaded for us)...
	hWrite->WriteByte(m_bOn);
	hWrite->WriteByte(m_bDynamic);
	hWrite->WriteFloat(m_fLifeTime);
	hWrite->WriteFloat(m_fStartTime - g_pLTServer->GetTime());

	// We may never need this !!!JKE
	int size = m_LightList.size();
	hWrite->WriteDWord(size);
	for (LightList::iterator iter = m_LightList.begin(); iter != m_LightList.end(); ++iter)
	{
		hWrite->WriteHString(iter->m_hName);
		hWrite->WriteVector(iter->m_Color);
		hWrite->WriteVector(iter->m_Pos);
		hWrite->WriteFloat(iter->m_Radius);
		hWrite->WriteByte(iter->m_ModelLight);
	}

	// Gotta save our list of children, duh!
	uint32 count = m_ObjectList.size();

	*hWrite << count;

	for(ClientList::iterator iter2 = m_ObjectList.begin(); iter2 != m_ObjectList.end(); ++iter2)
	{
		*hWrite << *iter2;
	}
}

// ----------------------------------------------------------------------------
//
//	ROUTINE:	LightGroup::HandleTrigger
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------------
void LightGroup::HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead )
{
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	std::string message = g_pServerDE->GetStringData( hMsg );

	std::transform(message.begin(), message.end(), message.begin(), toupper);

	TriggerChildren(hMsg);

	DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
	DDWORD dwFlags	= g_pServerDE->GetObjectFlags(m_hObject);

	if ( message == "TOGGLE" )
	{
		// Toggle the flag
		if (dwUsrFlags & USRFLG_VISIBLE)
		{
			// Main light
			dwUsrFlags &= ~USRFLG_VISIBLE;
			dwFlags	   &= ~FLAG_VISIBLE;
		}
		else
		{
			// Main light
			dwUsrFlags |= USRFLG_VISIBLE;
			dwFlags	   |= FLAG_VISIBLE;
		}
	}
	else if ( message == "ON")
	{
		// Main light
		dwUsrFlags |= USRFLG_VISIBLE;
		dwFlags	   |= FLAG_VISIBLE;
	}
	else if ( message ==  "OFF")
	{
		dwUsrFlags &= ~USRFLG_VISIBLE;
		dwFlags	   &= ~FLAG_VISIBLE;
	}
	else
	{
		std::string::size_type commandStart = message.find("DIM ");
		if (commandStart != std::string::npos)
		{
			// We have a DIM command.
			// Get the tail off

			std::string::size_type valueStart = message.find("(");
			std::string::size_type valueEnd = message.find(")");
			if (std::string::npos == valueStart)
			{
				valueStart = commandStart+4;
			}
			else
			{
				++valueStart;
			}

			const std::string value = message.substr(valueStart, valueEnd - valueStart);
			const float amount = (LTFLOAT)atof(value.c_str());

			m_fIntensityDelta += amount;

			// Cap values here...
			if(m_fIntensityDelta > 1.0f)	m_fIntensityDelta = 1.0f;
			if(m_fIntensityDelta < -1.0f)	m_fIntensityDelta = -1.0f;

			float intMin = m_fIntensityMin + m_fIntensityDelta;
			float intMax = m_fIntensityMax + m_fIntensityDelta;

			if (intMin < 0)
			{
				intMin = 0;
			}

			if (intMin > 1.0)
			{
				intMin = 1.0;
			}

			if (intMax < 0)
			{
				intMax = 0;
			}

			if (intMax > 1.0)
			{
				intMax = 1.0;
			}

			float radMin = m_fRadiusMin * (1.0f + m_fIntensityDelta);
			float radMax = m_fRadiusMax * (1.0f + m_fIntensityDelta);

			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
			g_pLTServer->WriteToMessageByte(hMessage, SFX_LIGHT_ID);
			g_pLTServer->WriteToMessageObject(hMessage, m_hObject);

			g_pLTServer->WriteToMessageFloat(hMessage, intMin);
			g_pLTServer->WriteToMessageFloat(hMessage, intMax);

			g_pLTServer->WriteToMessageFloat(hMessage, radMin);
			g_pLTServer->WriteToMessageFloat(hMessage, radMax);

			g_pLTServer->EndMessage(hMessage);
		}
	}

	g_pServerDE->SetObjectFlags(m_hObject, dwFlags);
	g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);

	g_pServerDE->FreeString( hMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightGroup::MakeLights()
//
//	PURPOSE:	Handle doing explosion
//
// ----------------------------------------------------------------------- //

void LightGroup::MakeLights()
{
	if(!m_hObject) return;

	for (LightList::iterator iter = m_LightList.begin(); iter != m_LightList.end(); ++iter)
	{
		// Only do this if it is a model light
		if (iter->m_ModelLight)
		{
			// Setup an object create structure
			ObjectCreateStruct ocs;
			ocs.Clear();
			ocs.m_ObjectType = OT_NORMAL;
			ocs.m_Pos = iter->m_Pos;
			SAFE_STRCPY(ocs.m_Name, g_pLTServer->GetStringData(iter->m_hName));

			// Create the ClientLightFX Object
			HCLASS hClass = g_pLTServer->GetClass("ClientLightFX");
			ClientLightFX *pClientLight = static_cast<ClientLightFX*>(g_pLTServer->CreateObject(hClass, &ocs));

			// If we have a client light then fill it with data and let it go.
			if(pClientLight)
			{
				m_ObjectList.push_back(pClientLight->m_hObject);

				// Setup an create structure
				ClientLightFXCreateStruct lightCS;

				// Figure out the scale of the individual lights
				float scale = iter->m_Radius/m_fRadiusMax;

				// Member Variables
				lightCS.m_bOn = m_bOn;

				lightCS.m_vColor = iter->m_Color;
				lightCS.m_dwLightFlags = m_dwLightFlags;

				lightCS.m_fIntensityMin = m_fIntensityMin * scale;
				lightCS.m_fIntensityMax = m_fIntensityMax * scale;
				lightCS.m_nIntensityWaveform = m_nIntensityWaveform;
				lightCS.m_fIntensityFreq = m_fIntensityFreq;
				lightCS.m_fIntensityPhase = m_fIntensityPhase;

				lightCS.m_fRadiusMin = m_fRadiusMin * scale;
				lightCS.m_fRadiusMax = m_fRadiusMax * scale;
				lightCS.m_nRadiusWaveform = m_nRadiusWaveform;
				lightCS.m_fRadiusFreq = m_fRadiusFreq;
				lightCS.m_fRadiusPhase = m_fRadiusPhase;

				lightCS.m_nOnSound = -1;
				lightCS.m_nOffSound = -1;

				lightCS.m_bUseLightAnims = LTFALSE;

				lightCS.m_bDynamic = DTRUE;
				lightCS.m_fLifeTime = m_fLifeTime;

				lightCS.m_fStartTime = m_fStartTime;


				lightCS.m_fHitPts = m_fHitPts;

				pClientLight->Setup(lightCS);
			}
		}
	}
}

void LightGroup::TriggerChildren(HSTRING hMsg)
{
	for(ClientList::iterator iter = m_ObjectList.begin(); iter != m_ObjectList.end(); ++iter)
	{
		if(*iter)
			SendTriggerMsgToObject(this, *iter, hMsg);
	}
}

// -----------------------------------------------------------------------
//	CLASS:		CLightGroupFXPlugin
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// ROUTINE:		CLightGroupFXPlugin::PreHook_Light
//
// Preprocessor callback to build a light animation frame.
//
// PreLightLT is a preprocessor lighting interface used to access information
// HPREOBJECT is a handle used to identify the object the data is requested from
// -----------------------------------------------------------------------
DRESULT	CLightGroupFXPlugin::PreHook_Light(
	PreLightLT *pInterface, 
	HPREOBJECT hObject)
{
	// Used localy to get the data slots. preName + (1 - Max) is the slot name
	static const std::string preName("Object");

	// Get name of the group object
	GenericProp genProp;				// Gets data of varying types
	pInterface->GetPropGeneric(hObject, "Name", &genProp);
	std::string groupName = std::string(genProp.m_String);

	// We can only use lightmaps
	DBOOL bUseShadowMaps = FALSE;

	// This is the container of all the lights
//	std::vector<PreLightInfo> LightList;

	// We need this so we can fill it with names
	PreLightAnimFrameInfo frame;

	// The list is one based, not 0 based
	for (int i = 1; i < MAX_LIGHTGROUP_TARGETS + 1; ++i)
	{
		// make a name based on what number it is
		std::string name;
		char buff[5];		// Used for itoa
#ifdef _WIN32
		name = preName + itoa(i, &buff[0], 10);
#else
		sprintf(buff, "%d", i);
		name = preName + buff;
#endif

		// Now, find the light's handle that is in that slot
		pInterface->GetPropGeneric(hObject, name.c_str(), &genProp);
		HPREOBJECT hObjectLight;		// This is a handle for whatever comes out.
		if(LT_OK != pInterface->FindObject(genProp.m_String, hObjectLight))
		{
			continue;			// There was no light here, so go on with life
		}

		// What kind are we?
		if(LT_OK != pInterface->GetPropGeneric(hObjectLight, "Name", &genProp))
		{
			continue;
		}

		std::string lightName = std::string(genProp.m_String);
		frame.m_LightList.push_back(lightName);

		// We don't need to read the info anymore, it is in the processor already
#if 0
		// we have a light, so let's get the data. If the data does not use light anims
		// it will not be added to the lists
		PreLightInfo lightInfo;

		if (GetLightData(lightInfo, bUseShadowMaps, pInterface, hObjectLight))
		{
			LightList.push_back(lightInfo);

			// Now record it so it does not get done again
			LightManager::GetInstance()->AddName(genProp.m_String, groupName);
		}
#endif
	}

#if 0
	// Do we have any lights left?
	if(0 == frame.m_LightList.size())
	{
		return LT_OK;
	}
#endif

	//-------------------------------------------------------------------------
	// Now set up the main frame object
	frame.m_bSunLight = DFALSE;			// No Sunlight

#if 0
	frame.m_Lights = &LightList[0];		// Give frame the address of the light info
	frame.m_nLights = LightList.size();	// How many lights did we give above.
#endif

	frame.m_Lights = NULL;				// Give frame the address of the light info
	frame.m_nLights = 0;				// How many lights did we give above.

	// Get name of the group animation
	std::string animName = groupName + LightGroup::LIGHTFX_LIGHTGROUP_EXTENSION;

	// We have all the information we need, so Create the light maps.
	pInterface->CreateLightAnim(animName.c_str(), &frame, 1, bUseShadowMaps);

	return LT_OK;
}

DRESULT CLightGroupFXPlugin::PreHook_EditStringList
								(const char* szRezPath, const char* szPropName,
								 char* const * aszStrings, DDWORD* pcStrings, const DDWORD 
								 cMaxStrings, const DDWORD cMaxStringLength)
{
	return LT_UNSUPPORTED;
}

//-------------------------------------------------------------------------------------------------
// GetLightData
//	Fill a PreLightInfo object with data based on the handle of the object.
//-------------------------------------------------------------------------------------------------
bool GetLightData(PreLightInfo& lightInfo, DBOOL& bUseShadowMaps, 
				  PreLightLT *pInterface, HPREOBJECT hObjectLight)
{
	GenericProp genProp;

	// Get all the similar stuff
	// Get position
	pInterface->GetPropGeneric(hObjectLight, "Pos", &genProp);
	lightInfo.m_vPos = genProp.m_Vec;

	// Get rotation
	DVector vRight, vUp;
	pInterface->GetPropGeneric(hObjectLight, "Rotation", &genProp);
	pInterface->GetMathLT()->GetRotationVectors(genProp.m_Rotation, vRight, vUp, lightInfo.m_vDirection);

	// Get radius
	pInterface->GetPropGeneric(hObjectLight, "LightRadius", &genProp);
	lightInfo.m_Radius = genProp.m_Float;

	// Set outer color to black
	lightInfo.m_vOuterColor.Init();

	// Get Field Of View
	lightInfo.m_FOV = 0.0f;

	// What kind are we?
	pInterface->GetPropGeneric(hObjectLight, "LightType", &genProp);
	std::string lightType = std::string(genProp.m_String);

	if ("Direct" == lightType)
	{
		// We are directional! rejoice!
		lightInfo.m_bDirectional = LTTRUE;

		// Get Color
		pInterface->GetPropGeneric(hObjectLight, "InnerColor", &genProp);
		lightInfo.m_vInnerColor = genProp.m_Vec;

		pInterface->GetPropGeneric(hObjectLight, "OuterColor", &genProp);
		lightInfo.m_vOuterColor = genProp.m_Vec;

		// Get Field Of View
		pInterface->GetPropGeneric(hObjectLight, "FOV", &genProp);
		lightInfo.m_FOV = MATH_DEGREES_TO_RADIANS(genProp.m_Float);
	}
	else
	{
		// Get Color
		pInterface->GetPropGeneric(hObjectLight, "LightColor", &genProp);
		lightInfo.m_vInnerColor = genProp.m_Vec;
	}

	return true;
}




// -----------------------------------------------------------------------
// 
// Light Manager:
//	Keeps track of the lights that were processed as a part of a group.
//
// Light manager is a singleton that keeps a list of all the 
// lights that were processed as a part of the Group system and so
// should not be processed as a part of the light phase
// -----------------------------------------------------------------------

// For the singleton aspect
LightManager* LightManager::m_pInstance = 0;

// This hides the implimentation from the rest of the program
struct LightMgrData
{
	typedef std::map<std::string, std::string> LightList;
	typedef LightList::iterator Iterator;

	LightList FinishedList;
};

// Constructor is private so the only access is via GetInstance()
LightManager::LightManager()
{
	m_pData = new LightMgrData;
}

// Do nothing since once one is created it sticks around.
LightManager::~LightManager()
{
	if( m_pData )
	{
		delete m_pData;
		m_pData = NULL;
	}

	if( m_pInstance )
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
};

// Reset the lightmanager between levels to clear out the data
bool LightManager::Reset( )
{
	if( m_pData )
	{
		delete m_pData;
		m_pData = NULL;
	}

	m_pData = new LightMgrData;

	return ( m_pData != NULL );
}

// Returns the one and only instance of LightManager
LightManager* LightManager::GetInstance()
{
	// See if one exists right now.
	if(0 == m_pInstance)
	{
		// Make one
		m_pInstance = new LightManager();
	}
	return m_pInstance;
}

// Put a name into the list
void LightManager::AddName(const std::string& name, const std::string& group)
{
	// There should be no name in the list
	assert (0 == GetLightMgrData()->FinishedList.count(name));

	// Now add it.
	GetLightMgrData()->FinishedList[name] = group;
}

// Search to see if this name exists.
// This implimentation assumes that lights are uniquely named.
bool LightManager::InGroup(const std::string& name)
{
	LightMgrData::Iterator found = GetLightMgrData()->FinishedList.find(name);
	return (found != GetLightMgrData()->FinishedList.end());
}

const std::string& LightManager::GetGroup(const std::string& name)
{
	static const std::string failed("FAILED");

	if (!InGroup(name))
	{
		return failed;
	}
	else
	{
		return GetLightMgrData()->FinishedList[name];
	}
}


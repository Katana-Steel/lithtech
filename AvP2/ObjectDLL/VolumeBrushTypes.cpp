// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrushTypes.cpp
//
// PURPOSE : VolumeBrushTypes implementation
//
// CREATED : 2/16/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "VolumeBrushTypes.h"
#include "Character.h"
#include "SFXMsgIds.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LiquidFilterFn()
//
//	PURPOSE:	Filter Liquid volume brushes out of CastRay and/or 
//				IntersectSegment calls (so vectors can go through liquid).
//
//	NOTE:		For now the assumption is if this function is called (via 
//				CastRay or IntersectSegment), FLAG_RAYHIT must be set on the 
//				object (if it is a volume brush).  So, only liquid volume
//				brushes have this flag set on them, so we can conclude that if 
//				this object is of type volume brush that it must be liquid...
//
// ----------------------------------------------------------------------- //

DBOOL LiquidFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj || !g_pLTServer) return LTFALSE;

	return 0 == dynamic_cast<VolumeBrush*>( g_pLTServer->HandleToObject(hObj) );
}


BEGIN_CLASS(Water)

#ifdef _WIN32
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "Spr\\spr0001.spr", PF_GROUP1 | PF_FILENAME)
#else
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "Spr/spr0001.spr", PF_GROUP1 | PF_FILENAME)
#endif

    ADD_COLORPROP_FLAG(SurfaceColor1, 255.0f, 255.0f, 255.0f, PF_GROUP1)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 255.0f, 255.0f, 255.0f, PF_GROUP1)   
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_UNSPECIFIED, PF_HIDDEN)
	ADD_COLORPROP_FLAG(TintColor, 0.0f, 127.0f, 178.5f, 0)   
END_CLASS_DEFAULT(Water, VolumeBrush, NULL, NULL)

BEGIN_CLASS(Ice)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_GROUP1 | PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 255.0f, 255.0f, 255.0f, PF_GROUP1)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 255.0f, 255.0f, 255.0f, PF_GROUP1)   
	ADD_REALPROP_FLAG(Friction, 0.0f, 0)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_UNSPECIFIED, PF_HIDDEN)
END_CLASS_DEFAULT(Ice, VolumeBrush, NULL, NULL)

BEGIN_CLASS(CorrosiveFluid)

#ifdef _WIN32
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "Spr\\spr0001.spr", PF_GROUP1 | PF_FILENAME)
#else
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "Spr/spr0001.spr", PF_GROUP1 | PF_FILENAME)
#endif

    ADD_COLORPROP_FLAG(SurfaceColor1, 255.0f, 255.0f, 255.0f, PF_GROUP1)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 255.0f, 255.0f, 255.0f, PF_GROUP1)   
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 5.0f, 0)
	ADD_LONGINTPROP_FLAG(DamageType, DT_BURN, PF_HIDDEN)
	ADD_COLORPROP_FLAG(TintColor, 76.5f, 102.0f, 0.0f, 0)   
END_CLASS_DEFAULT(CorrosiveFluid, VolumeBrush, NULL, NULL)

BEGIN_CLASS(FreezingWater)

#ifdef _WIN32
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "Spr\\spr0001.spr", PF_GROUP1 | PF_FILENAME)
#else
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "Spr/spr0001.spr", PF_GROUP1 | PF_FILENAME)
#endif

    ADD_COLORPROP_FLAG(SurfaceColor1, 255.0f, 255.0f, 255.0f, PF_GROUP1)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 255.0f, 255.0f, 255.0f, PF_GROUP1)   
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 10.0f, 0)
	ADD_LONGINTPROP_FLAG(DamageType, DT_FREEZE, PF_HIDDEN)
	ADD_COLORPROP_FLAG(TintColor, 0.0f, 127.0f, 178.5f, 0)   
    ADD_COLORPROP_FLAG(LightAdd, 25.5f, 25.5f, 25.5f, 0)   
END_CLASS_DEFAULT(FreezingWater, VolumeBrush, NULL, NULL)

BEGIN_CLASS(PoisonGas)
	ADD_BOOLPROP_FLAG(ShowSurface, LTFALSE, PF_GROUP1 | PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_GROUP1 | PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_GROUP1 | PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_GROUP1 | PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 5.0f, 0)
	ADD_LONGINTPROP_FLAG(DamageType, DT_CHOKE, PF_HIDDEN)
	ADD_COLORPROP_FLAG(TintColor, 255.0f, 255.0f, 76.7f, 0)   
END_CLASS_DEFAULT(PoisonGas, VolumeBrush, NULL, NULL)

BEGIN_CLASS(Electricity)
	ADD_BOOLPROP_FLAG(ShowSurface, LTFALSE, PF_GROUP1 | PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_GROUP1 | PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_GROUP1 | PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_GROUP1 | PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 20.0f, 0)
	ADD_LONGINTPROP_FLAG(DamageType, DT_ELECTRIC, PF_HIDDEN)
    ADD_COLORPROP_FLAG(LightAdd, 25.5f, 25.5f, 25.5f, 0)   
END_CLASS_DEFAULT(Electricity, VolumeBrush, NULL, NULL)

BEGIN_CLASS(EndlessFall)
	ADD_BOOLPROP_FLAG(ShowSurface, LTFALSE, PF_GROUP1 | PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_GROUP1 | PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_GROUP1 | PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 100000000.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_ENDLESS_FALL, PF_HIDDEN)
END_CLASS_DEFAULT(EndlessFall, VolumeBrush, NULL, NULL)

BEGIN_CLASS(Wind)
	ADD_BOOLPROP_FLAG(ShowSurface, LTFALSE, PF_GROUP1 | PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_GROUP1 | PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_GROUP1 | PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_UNSPECIFIED, PF_HIDDEN)
	ADD_REALPROP_FLAG(Friction, 0.0f, 0)
END_CLASS_DEFAULT(Wind, VolumeBrush, NULL, NULL)

BEGIN_CLASS(ColdAir)
	ADD_BOOLPROP_FLAG(ShowSurface, LTFALSE, PF_GROUP1 | PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_GROUP1 | PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_GROUP1 | PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 5.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_FREEZE, PF_HIDDEN)
END_CLASS_DEFAULT(ColdAir, VolumeBrush, NULL, NULL)

BEGIN_CLASS(Ladder)
	ADD_BOOLPROP_FLAG(ShowSurface, LTFALSE, PF_GROUP1 | PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_GROUP1 | PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_GROUP1 | PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_GROUP1 | PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_GROUP1 | PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, 0)
	ADD_REALPROP_FLAG(LiquidGravity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_UNSPECIFIED, PF_HIDDEN)
END_CLASS_DEFAULT(Ladder, VolumeBrush, NULL, NULL)

BEGIN_CLASS(Weather)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Friction, 1.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(LiquidGravity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_CHOKE, PF_HIDDEN)
    ADD_COLORPROP_FLAG(TintColor, 255.0f, 255.0f, 255.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(LightAdd, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
 	ADD_BOOLPROP_FLAG(ShowSurface, LTFALSE, PF_GROUP1 | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_GROUP1 | PF_FILENAME | PF_HIDDEN)   
	ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(XScaleDuration, 60.0f, PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(YScaleDuration, 60.0f, PF_GROUP1 | PF_HIDDEN)   
	ADD_BOOLPROP_FLAG(PanWithCurrent, LTTRUE, PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(XPan, 10.0f, PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(YPan, 10.0f, PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(Frequency, 50.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_COLORPROP_FLAG(SurfaceColor1, 255.0f, 255.0f, 255.0f, PF_GROUP1 | PF_HIDDEN)   
	ADD_COLORPROP_FLAG(SurfaceColor2, 255.0f, 255.0f, 255.0f, PF_GROUP1 | PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_GROUP1 | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Additive, LTFALSE, PF_GROUP1 | PF_HIDDEN)   
	ADD_BOOLPROP_FLAG(Multiply, LTFALSE, PF_GROUP1 | PF_HIDDEN)   
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_GROUP1 | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(FogEnable, LTFALSE, PF_GROUP2)
	ADD_REALPROP_FLAG(MinFogFarZ, 300.0f, PF_GROUP2)
	ADD_REALPROP_FLAG(MinFogNearZ, -100.0f, PF_GROUP2)
	ADD_REALPROP_FLAG(FogFarZ, 300.0f, PF_GROUP2)
	ADD_REALPROP_FLAG(FogNearZ, -100.0f, PF_GROUP2)
	ADD_COLORPROP_FLAG(FogColor, 0.0f, 0.0f, 0.0f, PF_GROUP2)
	PROP_DEFINEGROUP(RainProperties, PF_GROUP3)
		ADD_BOOLPROP_FLAG(LightRain, LTFALSE, PF_GROUP3)
		ADD_BOOLPROP_FLAG(NormalRain, LTTRUE, PF_GROUP3)
		ADD_BOOLPROP_FLAG(HeavyRain, LTFALSE, PF_GROUP3)
	PROP_DEFINEGROUP(SnowProperties, PF_GROUP4)
		ADD_BOOLPROP_FLAG(LightSnow, LTFALSE, PF_GROUP4)
		ADD_BOOLPROP_FLAG(NormalSnow, LTTRUE, PF_GROUP4)
		ADD_BOOLPROP_FLAG(HeavySnow, LTFALSE, PF_GROUP4)
	ADD_REALPROP_FLAG(MaxViewDistance, 1000.0f, 0)
END_CLASS_DEFAULT(Weather, VolumeBrush, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Weather::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void Weather::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	VolumeBrush::ReadProp(pStruct);

	m_dwWeatherFlags = 0;

	DBOOL bVal = LTFALSE;
	g_pServerDE->GetPropBool("LightRain", &bVal);
	if (bVal) m_dwWeatherFlags |= WFLAG_LIGHT_RAIN;

	g_pServerDE->GetPropBool("NormalRain", &bVal);
	if (bVal) m_dwWeatherFlags |= WFLAG_NORMAL_RAIN;

	g_pServerDE->GetPropBool("HeavyRain", &bVal);
	if (bVal) m_dwWeatherFlags |= WFLAG_HEAVY_RAIN;

	g_pServerDE->GetPropBool("LightSnow", &bVal);
	if (bVal) m_dwWeatherFlags |= WFLAG_LIGHT_SNOW;

	g_pServerDE->GetPropBool("NormalSnow", &bVal);
	if (bVal) m_dwWeatherFlags |= WFLAG_NORMAL_SNOW;

	g_pServerDE->GetPropBool("HeavySnow", &bVal);
	if (bVal) m_dwWeatherFlags |= WFLAG_HEAVY_SNOW;

	g_pServerDE->GetPropReal("MaxViewDistance", &m_fViewDist);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Weather::CreateSpecialFXMsg()
//
//	PURPOSE:	Add Weather specific data to the special fx message
//
// ----------------------------------------------------------------------- //

void Weather::WriteSFXMsg(HMESSAGEWRITE hMessage)
{
	m_nSfxMsgId = SFX_WEATHER_ID;

	VolumeBrush::WriteSFXMsg(hMessage);

	// Add weather specific data to the special fx message...

	g_pServerDE->WriteToMessageDWord(hMessage, m_dwWeatherFlags);
	g_pServerDE->WriteToMessageFloat(hMessage, m_fViewDist);
}

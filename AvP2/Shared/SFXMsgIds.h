// ----------------------------------------------------------------------- //
//
// MODULE  : SFXMsgIDs.h
//
// PURPOSE : Special FX message ids
//
// CREATED : 10/28/97
//
// ----------------------------------------------------------------------- //

#ifndef __SFX_MSG_IDS_H__
#define __SFX_MSG_IDS_H__

// SpecialFX types...
//
// The first BYTE in the message associated with the HMESSAGEREAD parameter 
// passed to CSFXMgr::HandleSFXMsg() must be one of the following values:

// NOTE:  It is important that these remain in this order, SFXMgr.cpp assumes
// this order for making optimizations...New FX should be added at the end
// and SFX_TOTAL_NUMBER should be adjusted to account for them (also, the
// s_nDynArrayMaxNums[] should be updated in SFXMgr.cpp).


#define SFX_POLYGRID_ID				1
#define SFX_PARTICLETRAIL_ID		2
#define	SFX_PARTICLESYSTEM_ID		3
#define SFX_PARTICLESHOWER_ID		4
#define SFX_TRACER_ID				5
#define SFX_WEAPON_ID				6
#define SFX_DYNAMICLIGHT_ID			7
#define SFX_PARTICLETRAILSEG_ID		8
#define SFX_SMOKE_ID				9
#define SFX_BULLETTRAIL_ID			10
#define SFX_VOLUMEBRUSH_ID			11
#define SFX_SHELLCASING_ID			12
#define SFX_CAMERA_ID				13
#define SFX_PARTICLEEXPLOSION_ID	14
#define SFX_SCALE_ID				15 // Sprites/Models	
#define SFX_DEBRIS_ID				16
#define SFX_DEATH_ID				17
#define SFX_GIB_ID					18
#define SFX_PROJECTILE_ID			19
#define SFX_MARK_ID					20
#define SFX_LIGHT_ID				21
#define SFX_RANDOMSPARKS_ID			22
#define SFX_PICKUPOBJECT_ID			23
#define SFX_CHARACTER_ID			24
#define SFX_WEAPONSOUND_ID			25
#define SFX_NODELINES_ID			26
#define SFX_WEATHER_ID				27
#define SFX_LIGHTNING_ID			28
#define SFX_SPRINKLES_ID			29
#define	SFX_FIRE_ID					30
#define SFX_LENSFLARE_ID			31
#define SFX_MUZZLEFLASH_ID			32
#define SFX_SEARCHLIGHT_ID			33
#define SFX_POLYDEBRIS_ID			34
#define SFX_STEAM_ID				35
#define SFX_EXPLOSION_ID			36
#define SFX_POLYLINE_ID				37
#define SFX_LASERTRIGGER_ID			38
#define SFX_MINE_ID					39
#define SFX_BEAM_ID					40
#define SFX_SOUND_ID				41

#define SFX_BODYPROP_ID				42
#define SFX_GRAPH_ID				43
#define SFX_STARLIGHT_VISION		44
#define SFX_DEBUGLINE_ID			45
#define SFX_PRED_TARGET_ID			46
#define SFX_SPRITE_CONTROL_ID		47
#define SFX_FOG_VOLUME_ID			48
#define SFX_DRIPPER_ID				49
#define SFX_MULTI_DEBRIS_ID			50

// Important that this is the same as the last FX defined...
#define SFX_TOTAL_NUMBER			SFX_MULTI_DEBRIS_ID


// Shared fx related stuff...


// CharacterFX messages...

#define CFX_NODECONTROL_RECOIL_MSG		1
#define CFX_NODECONTROL_LIP_SYNC		2
#define CFX_NODECONTROL_HEAD_FOLLOW_OBJ	3
#define CFX_NODECONTROL_HEAD_FOLLOW_POS	4
#define CFX_NODECONTROL_SCRIPT			5
#define CFX_NODECONTROL_AIMING_PITCH	6
#define CFX_NODECONTROL_AIMING_YAW		7
#define CFX_NODECONTROL_STRAFE_YAW		8
#define CFX_RESET_MAINTRACKER			9
#define CFX_RESET_ATTRIBUTES			10
#define CFX_FLASHLIGHT					11
#define CFX_TARGETING_SPRITE			12
#define CFX_EYEFLASH					13
#define CFX_BLEEDER						14
#define CFX_INFO_STRING					15
#define CFX_FLAME_FX					16
#define CFX_FLAME_FX_SHOT				17
#define CFX_MODELLIGHTING				18
#define CFX_LAND_SOUND					19
#define CFX_JUMP_SOUND					20
#define CFX_POUNCE_SOUND				21
#define CFX_FOOTSTEP_SOUND				22
#define CFX_BOSS_SMOKE_FX				23		// See AIDunyaExosuit.cpp
#define CFX_CHESTBURST_FX				24
#define CFX_SUBTITLES					25
#define CFX_PLAY_LOOP_FIRE				26
#define CFX_KILL_LOOP_FIRE				27

// BodyPropFX messages...

#define BPFX_CHESTBURST_FX				1
#define BPFX_NODECONTROL_LIP_SYNC		2


// PolyLine values...

enum PolyLineWidthStyle 
{
	PLWS_BIG_TO_SMALL=0,
	PLWS_SMALL_TO_BIG,
	PLWS_SMALL_TO_SMALL,
	PLWS_CONSTANT
};


#endif // __SFX_MSG_IDS_H__

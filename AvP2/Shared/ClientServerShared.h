#ifndef __CLIENTSERVERSHARED_H
#define __CLIENTSERVERSHARED_H

#include "dvector.h"

// Shared client/server enumerations...

// Player states

enum PlayerState { PS_UNKNOWN=0, PS_ALIVE, PS_DEAD, PS_DYING, PS_SWAPPED, PS_OBSERVING };

enum Species
{
	Unknown = -1,
	Alien = 0,
	Marine,
	Predator,
	Corporate
};


enum WeaponModelKeySoundId
{
	WMKS_INVALID=-1,
	WMKS_MISC0=0,
	WMKS_MISC1,
	WMKS_MISC2,
	WMKS_SELECT,
	WMKS_DESELECT,
	WMKS_FIRE,
	WMKS_DRY_FIRE,
	WMKS_ALT_FIRE,
	WMKS_SILENCED_FIRE
};


// Light waveform types

enum WaveTypes 
{
	WAVE_NONE,
	WAVE_SQUARE,
	WAVE_SAW,
	WAVE_RAMPUP,
	WAVE_RAMPDOWN,
	WAVE_SINE,
	WAVE_FLICKER1,
	WAVE_FLICKER2,
	WAVE_FLICKER3,
	WAVE_FLICKER4,
	WAVE_STROBE,
	WAVE_SEARCH
};


enum HitLocation
{
	HL_UNKNOWN	= 0,
	HL_HEAD,
	HL_TORSO,
	HL_ARM,
	HL_LEG,
	HL_TAIL,
	HL_NUM_LOCS,
};

// Strafe flags...

#define SF_LEFT		1
#define SF_RIGHT	2
#define SF_FORWARD	4
#define SF_BACKWARD	8


// Object user flags -
// NOTE: The top byte is reserved for surface flags

#define USRFLG_VISIBLE					(1<<0)
#define USRFLG_KEYFRAMED_MOVEMENT		(1<<1)
#define USRFLG_IGNORE_PROJECTILES		(1<<2)

// Used with player attachments (weapon / flash).  XXX_HIDE1 indicates that this
// object is hidden on the client when in first person mode.  XXX_HIDE1SHOW3
// indicates that this object is hidden on the client in first person mode,
// and shown on the client in 3rd person mode....

#define USRFLG_ATTACH_HIDE1				(1<<3)
#define USRFLG_ATTACH_HIDE1SHOW3		(1<<4)

// Used with Camera objects.  This specifies if a specific camera is "live"

#define USRFLG_CAMERA_LIVE				(1<<5)

// Can this object move...

#define USRFLG_MOVEABLE					(1<<6)

#define USRFLG_MODELADD					(1<<7)
#define USRFLG_GLOW						(1<<8)

// Used with world models only (server-side only).  This determines if 
// the model can crush other objects...

#define USRFLG_CANT_CRUSH				(1<<9)

// Used with the player object only (checked only on the client, and only
// on the player model)...

#define USRFLG_PLAYER_DUCK				(1<<10)

// Used with character objects...

#define USRFLG_CHARACTER				(1<<11)

#define USRFLG_CHAR_UNDERWATER			(1<<12)
#define USRFLG_CHAR_STUNNED				(1<<13)
#define USRFLG_CHAR_EMPEFFECT			(1<<14)
#define USRFLG_CHAR_NAPALM				(1<<15)
#define USRFLG_CHAR_CLOAKED				(1<<16)
#define USRFLG_CHAR_FACEHUG				(1<<17)

const uint32 USERFLG_CHAR_FILTER = (USRFLG_CHAR_UNDERWATER | 
									USRFLG_CHAR_STUNNED | 
									USRFLG_CHAR_EMPEFFECT | 
									USRFLG_CHAR_NAPALM | 
									USRFLG_CHAR_CLOAKED | 
									USRFLG_CHAR_FACEHUG );

// Used with AI objects. This specifies if an AI is using its flashlight.

#define USRFLG_AI_FLASHLIGHT			(1<<18)

// Specifies that this object can be activated...
#define USRFLG_ACTIVATE0			(1<<19)
#define USRFLG_ACTIVATE1			(1<<20)
#define USRFLG_ACTIVATE2			(1<<21)
#define USRFLG_ACTIVATE3			(1<<22)

enum ActivateType
{
	AT_NOT_ACTIVATABLE = 0,
	AT_NORMAL,
	AT_WELD,
	AT_CUT,
	AT_HACK,
	AT_TEAR,
	AT_PRED_PICKUP,
	AT_MARINE_PICKUP,
	AT_EXO_PICKUP,
	AT_BITSHIFT = 19, //this must match the number of bits USRFLG_ACTIVATE0 is shifted
};

#define USRFLG_SPECIAL_ATTACHMENT		(1<<23)

// Set when the object is deactivated on the server.
// This is used to turn off sfx objects when the object has been deactivated.
#define USRFLG_DEACTIVATED				(1<<24)

// DO NOT USE THESE... the top four bits can be used as a surface type
// Only change this is you know it won't affect the surface information.

#define USRFLG_SURFACE_BITOFFSET		28
#define USRFLG_SURFACE1					(1<<28)
#define USRFLG_SURFACE2					(1<<29)
#define USRFLG_SURFACE3					(1<<30)
#define USRFLG_SURFACE4					(1<<31)


// ALL USRFLG's available!!!!


// Camera related flags...

#define CT_FULLSCREEN			0
#define CT_CINEMATIC			1


// Load/Save game defines...

#define LOAD_NEW_GAME			1	// Start from scratch - no saving or restoring
#define LOAD_NEW_LEVEL			2	// Save keep alives and level objects
#define LOAD_RESTORE_GAME		3	// No saving, but restore saved object

#define SERROR_LOADGAME			1
#define SERROR_SAVEGAME			2

#define GAME_NAME				"AVP2"
#define SAVE_DIR				"Save"

#ifdef _WIN32
	#define KEEPALIVE_FILENAME		"Save\\Kpalv.sav"
#else
	#define KEEPALIVE_FILENAME		"Save/Kpalv.sav"
#endif
	
#define RELOADLEVEL_FILENAME	"Reload.sav"
#define QUICKSAVE_FILENAME		"Quick.sav"
#define QUICKSAVE_BACKUP_FILENAME		"OldQuick.sav"
#define SAVEGAMEINI_FILENAME	"Save0000.ini"  // v0.0

#define INFINITE_MASS			100000.0f

#endif

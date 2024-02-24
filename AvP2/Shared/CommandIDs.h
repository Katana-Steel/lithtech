// ----------------------------------------------------------------------- //
//
// MODULE  : CommandIDs.h
//
// PURPOSE : Command ids
//
// CREATED : 9/18/97
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __COMMAND_IDS_H__
#define __COMMAND_IDS_H__


// Player movement commands ids...
#define COMMAND_ID_FORWARD			0
#define COMMAND_ID_BACKWARD			1
#define COMMAND_ID_LEFT				2
#define COMMAND_ID_RIGHT			3
#define COMMAND_ID_STRAFE			4
#define COMMAND_ID_STRAFE_LEFT		5
#define COMMAND_ID_STRAFE_RIGHT		6
#define COMMAND_ID_RUN				7
#define COMMAND_ID_DUCK				8
#define COMMAND_ID_JUMP				9
#define COMMAND_ID_LOOKUP			10
#define COMMAND_ID_LOOKDOWN			11
#define COMMAND_ID_CENTERVIEW		12


// General player commands
#define COMMAND_ID_RUNLOCK			13
#define COMMAND_ID_FIRING			14
#define COMMAND_ID_ALT_FIRING		15
#define COMMAND_ID_RELOAD			16
#define COMMAND_ID_ACTIVATE			17
#define COMMAND_ID_PREV_WEAPON		18
#define COMMAND_ID_NEXT_WEAPON		19
#define COMMAND_ID_ZOOM_IN			20
#define COMMAND_ID_ZOOM_OUT			21
#define COMMAND_ID_LAST_WEAPON		22
#define COMMAND_ID_CROUCH_TOGGLE	23
#define COMMAND_ID_TORCH_SELECT		24
#define COMMAND_ID_WALLWALK			25
#define COMMAND_ID_WALLWALK_TOGGLE	26
#define COMMAND_ID_POUNCE_JUMP		27
#define COMMAND_ID_ENERGY_SIFT		28
#define COMMAND_ID_MEDI_COMP		29


// Weapon quick keys
#define COMMAND_ID_WEAPON_BASE		30
#define COMMAND_ID_WEAPON_0			30
#define COMMAND_ID_WEAPON_1			31
#define COMMAND_ID_WEAPON_2			32
#define COMMAND_ID_WEAPON_3			33
#define COMMAND_ID_WEAPON_4			34
#define COMMAND_ID_WEAPON_5			35
#define COMMAND_ID_WEAPON_6			36
#define COMMAND_ID_WEAPON_7			37
#define COMMAND_ID_WEAPON_8			38
#define COMMAND_ID_WEAPON_9			39
#define COMMAND_ID_WEAPON_MAX		39


// Item quick keys
#define COMMAND_ID_ITEM_BASE		40
#define COMMAND_ID_SHOULDERLAMP		40
#define COMMAND_ID_FLARE			41
#define COMMAND_ID_HACKING_DEVICE	42
#define COMMAND_ID_ITEM_3			43
#define COMMAND_ID_ITEM_4			44
#define COMMAND_ID_ITEM_5			45
#define COMMAND_ID_ITEM_6			46
#define COMMAND_ID_ITEM_7			47
#define COMMAND_ID_CLOAK			48
#define COMMAND_ID_DISC_RETRIEVE	49
#define COMMAND_ID_ITEM_MAX			49


// Extra quick keys (use 40-59)
#define COMMAND_ID_PREV_VISIONMODE	50
#define COMMAND_ID_NEXT_VISIONMODE	51


// Interface control commands
#define COMMAND_ID_PREV_HUD			60
#define COMMAND_ID_NEXT_HUD			61
#define COMMAND_ID_MOUSEAIMTOGGLE	62
#define COMMAND_ID_CROSSHAIRTOGGLE	63
#define COMMAND_ID_CHASEVIEWTOGGLE	64
#define COMMAND_ID_QUICKSAVE		65
#define COMMAND_ID_QUICKLOAD		66


// Multiplayer specific commands
#define COMMAND_ID_MESSAGE			70
#define COMMAND_ID_TEAMMESSAGE		71
#define COMMAND_ID_SCOREDISPLAY		72
#define COMMAND_ID_PLAYERINFO		73
#define COMMAND_ID_TAUNT			74
#define COMMAND_ID_SERVERINFO		75


// Add new commands here 77-98 available...


// Special command ids...
#define COMMAND_ID_UNASSIGNED		99	// always leave this unassigned - used in input code
#define COMMAND_ID_DISPLAYSPECIAL	100 // Used for version checking


#endif // __COMMAND_IDS_H__


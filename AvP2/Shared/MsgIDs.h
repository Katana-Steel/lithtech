// ----------------------------------------------------------------------- //
//
// MODULE  : MsgIDs.h
//
// PURPOSE : Message ids
//
// CREATED : 9/22/97
//
// ----------------------------------------------------------------------- //

#ifndef __MSG_IDS_H__
#define __MSG_IDS_H__

#include "SFXMsgIds.h"				// Special fx message ids

// General Client <-> Server messages

#define MID_PLAYER_UPDATE					100
#define MID_PLAYER_EXITLEVEL				101 // Both ways
#define MID_PLAYER_INFOCHANGE				102
#define MID_PLAYER_AUTOSAVE					103	// Server to client
#define	MID_PLAYER_STATE_CHANGE				104
#define MID_PLAYER_CHEAT					105
#define MID_PLAYER_ORIENTATION				106
#define MID_PLAYER_RESPAWN					107
#define MID_PLAYER_TELETYPE					108
#define MID_MP_RESPAWN						109

#define MID_PLAYER_UNMUTE_SOUND				110
#define MID_PLAYER_MDTRACKER_REMOVE			111
#define MID_PLAYER_LOADCLIENT				112
#define MID_PLAYER_DAMAGE					113
#define MID_PLAYER_ACTIVATE					114
#define MID_PLAYER_CLIENTMSG				115	// Client to server
#define MID_PLAYER_DO_STORYOBJECT			116 // Server to client
#define MID_PLAYER_MDTRACKER				117
#define MID_PLAYER_HEALTH_ADDED				118
#define MID_PLAYER_ARMOR_ADDED				119
#define MID_PLAYER_EXIT_LEVEL				120
#define MID_PLAYER_RELOAD_SKINS				121
#define MID_PLAYER_CONSOLE_STRING			122
#define MID_PLAYER_OBJECTIVE_ADD			123
#define MID_PLAYER_OBJECTIVE_STATECHANGE	124
#define MID_PLAYER_OBJECTIVE_RESET			125
#define MID_PLAYER_DAMAGE_FLASH				126
#define MID_PLAYER_FADEIN					127
#define MID_PLAYER_FADEOUT					128

#define MID_FORCE_POUNCE_JUMP				129
#define MID_FORCE_WEAPON_CHANGE				130	  // Both ways
#define MID_WEAPON_CHANGE					131	  // Both ways
#define MID_WEAPON_SOUND					132
#define MID_WEAPON_LOOPSOUND				133
#define MID_WEAPON_FIRE						134
#define MID_FORCE_POUNCE					135
#define MID_FORCE_FACEHUG					136
#define MID_HOTBOMB							137
#define MID_PREDATORMASK					138
#define MID_WEAPON_HIT						139

#define MID_MUSIC							140

#define MID_NIGHTVISION						141
#define MID_PLAYER_CACHE_STORYOBJECT		142 // Server to client
#define MID_PLAYER_VERIFY_CFX				143 // Client to server
#define MID_PLAYER_MD						144
#define MID_MEDI_FAIL						145
#define MID_ADD_SKULL						146
#define MID_ADD_SKULL2						147
#define MID_PLAYER_OBJECTIVE_OVERVIEW		148
#define MID_MPLAYER_VERIFY_CFX				149

#define MID_LOAD_GAME						150 // Client to server
#define MID_SAVE_GAME						151 // Client to server
#define MID_GAME_PAUSE						152
#define MID_GAME_UNPAUSE					153
#define MID_SERVER_ERROR					154

#define MID_MP_EXO_RESPAWN					155

#define MID_MPMGR_GAME_INFO					160
#define MID_MPMGR_ADD_CLIENT				161
#define MID_MPMGR_REMOVE_CLIENT				162
#define MID_MPMGR_UPDATE_CLIENT				163
#define MID_MPMGR_UPDATE_TEAM				164
#define MID_MPMGR_NEXT_LEVEL				165
#define MID_MPMGR_LOAD_LEVEL				166
#define MID_MPMGR_MESSAGE					167
#define MID_MPMGR_TEAM_MESSAGE				168
#define MID_MPMGR_PRIVATE_MESSAGE			169
#define MID_MPMGR_SERVER_MESSAGE			170
#define MID_MPMGR_SERVER_COMMAND			171
#define MID_MPMGR_TIME						172
#define MID_MPMGR_ROUND						173
#define MID_MPMGR_CHANGE_PLAYER				174
#define MID_MPMGR_OBSERVE_MODE				175
#define MID_MPMGR_RESPAWN					176
#define MID_MPMGR_START_GAME				177
#define MID_MPMGR_TAUNT						178
#define MID_MPMGR_WAIT_SECONDS				179
#define MID_MPMGR_NAME_CHANGE				180

#define	MID_SINGLEPLAYER_START				185

#define MID_PHYSICS_UPDATE					190	// Server to client
#define MID_FRAG_SELF						191 // Client to server
#define MID_SERVERFORCEPOS					192 // Server to client
#define MID_SERVERFORCEVELOCITY				193 // Server to client
#define MID_SHAKE_SCREEN					194 // Client to server
#define MID_CHANGE_FOG						195	// Server to client

#define MID_CLIENTFX_CREATE					200 // Server to client
#define MID_SFX_MESSAGE						201 // Server to client
#define MID_STARLIGHT						202 // Tell Client to make a starlight object

#define	MID_CONSOLE_TRIGGER					210	// Client to server
#define MID_CONSOLE_COMMAND					211	// Client to server

#define MID_DISPLAY_TIMER					220	// Server to client	
#define MID_SPEED_CHEAT						221	// Server to client	

#define MID_CMD_ID_CLOAK					230	// Client to server
#define MID_CMD_ID_DISC_RETRIEVE			231	// Client to server
#define MID_CMD_ID_MEDI_COMP				232	// Client to server
#define MID_CMD_ID_ENERGY_SIFT				233	// Client to server
#define MID_CMD_ID_TORCH_SELECT				234	// Client to server
#define MID_CMD_ID_HACKING_DEVICE			235	// Client to server
#define MID_CMD_ID_VULNERABLE				236	// Client to server

#define MID_PLAYER_MASK_ADDED				240
#define MID_PLAYER_CLOAK_ADDED				241

#define MID_PLAYER_NIGHT_VISION_ADDED		242

#define MID_PLAYER_CREDITS					243

#define MID_PLAYER_RESET_COUNTER			244




// Object <-> Object messages...


// Interface Change ids shared between client and server...These are
// used with above MID_PLAYER_INFOCHANGE

#define IC_AMMO_ID				1
#define IC_HEALTH_ID			2
#define IC_ARMOR_ID				3
#define IC_WEAPON_ID			4
#define IC_WEAPON_PICKUP_ID		5
#define IC_AIRLEVEL_ID			6
#define IC_ROCKETLOCK_ID		7
#define IC_OUTOFAMMO_ID			8
#define IC_OBJECTIVE_ID			9
#define IC_MOD_PICKUP_ID		10
#define IC_EMPTY				11
#define IC_RESET_INVENTORY_ID	12
#define IC_MISSION_TEXT_ID		13
//#define IC_MISSION_FAILED_ID	14 Replaced by SCM_MISSION_FAILED
#define IC_MAX_HEALTH_ID		15
#define IC_MAX_ARMOR_ID			16
#define IC_FILL_ALL_CLIPS_ID	17
#define IC_CANNON_CHARGE_ID		18
#define IC_BATTERY_CHARGE_ID	19
#define IC_FILL_CLIP_ID			20

#define OBJECTIVE_ADD_ID		0
#define OBJECTIVE_REMOVE_ID		1
#define OBJECTIVE_COMPLETE_ID	2


// Client to player messages...These are used with MID_PLAYER_CLIENTMSG

// Motion Status is used to notify the server if the motion of the player
// has changed (did he jump/land/fall)....

#define CP_MOTION_STATUS		1
	#define MS_NONE				0
	#define MS_JUMPED			1
	#define MS_FALLING			2
	#define MS_LANDED			3


// Damage is used to inflict damage by the player to any object (e.g., fall damage)...

#define CP_DAMAGE				2


// Weapon Status is used to tell the server what animation the player-view
// weapon is playing...

#define CP_WEAPON_STATUS		3
	#define WS_NONE				0
	#define WS_RELOADING		1
	#define WS_SELECT			2
	#define WS_DESELECT			3
	#define WS_MELEE			4


// Flashlight is used to simply send new info regarding the location of the 
// flashlight beam impact

#define CP_FLASHLIGHT			4
	#define FL_ON				0
	#define FL_OFF				1
	#define FL_UPDATE			2

// Nightvision is used to simply send new info regarding the status of the
// marine night vision system

#define CP_VISIONMODE			5
	#define VM_NIGHTV_ON		0
	#define VM_NIGHTV_OFF		1
	#define VM_PRED_HEAT		2
	#define VM_PRED_ELEC		3
	#define VM_PRED_NAV			4

// Misc client update messages
#define CP_CANNON_CHARGE			6
#define CP_ENERGY_SIFT				7
#define CP_MEDICOMP					8
#define CP_HOTBOMB_DETONATE			9

// Damage is used to inflict pounce damage by the player to any object

#define CP_POUNCE_DAMAGE	10

// Multiplayer sound messages...
#define CP_POUNCE_SOUND		11
#define CP_JUMP_SOUND		12
#define CP_LAND_SOUND		13
#define CP_FOOTSTEP_SOUND	14


// Music commands
enum MusicCommand { MUSICCMD_LEVEL, MUSICCMD_MOTIF, MUSICCMD_MOTIF_LOOP, MUSICCMD_MOTIF_STOP, MUSICCMD_BREAK };



#endif // __MSG_IDS_H__


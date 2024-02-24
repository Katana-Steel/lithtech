// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectMsgs.h
//
// PURPOSE : Object to object messages (replaces generic_msg_de.h)
//
// CREATED : 4/22/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __OBJECT_MSGS_H__
#define __OBJECT_MSGS_H__

// Defines...

// All of the following messages are sent between objects:
// (i.e., Object <-> Object)

// For MID_TRIGGER, the message should contain 1 HSTRING that represents
// the trigger functionality (e.g. "CLOSE")
//
#define MID_TRIGGER					1000

// For MID_DAMAGE, see DamageStruct in Destructible.h for message info.
//
#define MID_DAMAGE					1001

// For MID_REPAIR, the message should contain 1 DFLOAT that represents the
// repair amount
//
#define MID_REPAIR					1002

// For MID_HEAL, the message should contain 1 DFLOAT that represents the
// heal amount
//
#define MID_HEAL					1003

// For MID_AMMO, the message should contain 
// 1 DBYTE that represents an AmmoPool id, and 
// 1 DFLOAT that represents the amount of ammo of that type

#define MID_AMMO					1004

// For MID_ADDWEAPON, the message should contain (in order), 1 DBYTE that
// represents the weapon id (this id may be game specific), and 1 DFLOAT
// that represents the amount of ammo the weapon contains
//
#define	MID_ADDWEAPON				1005

// For MID_PICKEDUP, the message should contain 1 float that represents
// the time to remain invisible.  Specify -1.0 to use the object's default
// respawn time.
#define MID_PICKUP					1006
#define MID_PICKEDUP				1007

// For MID_NET_SLASH, the message does not need any additional info
#define MID_NET_SLASH				1008

// For the CharacterAnimation class to pass back information to the object
// that it's updating
#define MID_ANIMATION				1009


#endif // __OBJECT_MSGS_H__

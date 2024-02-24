// ----------------------------------------------------------------------- //
//
// MODULE  : SharedMovement.h
//
// PURPOSE : Shared movement definitions.
//
// CREATED : 11/25/98
//
// (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SHAREDMOVEMENT_H__
#define __SHAREDMOVEMENT_H__


// These flags are used in the MID_PHYSICS_UPDATE to tell which things are in the message.

#define PSTATE_ATTRIBUTES			(1<<0)
#define PSTATE_SCALE				(1<<1)
#define PSTATE_GRAVITY				(1<<2)
#define PSTATE_POUNCE				(1<<3)
#define PSTATE_FACEHUG				(1<<4)
#define PSTATE_REMOVE_NET_FX		(1<<5)
#define PSTATE_SET_NET_FX			(1<<6)
#define PSTATE_ALL					(0xFFFF)


// Client update flags used to tell the server about the client's state

#define CLIENTUPDATE_POSITION		(1<<0)
#define CLIENTUPDATE_ROTATION		(1<<1)
#define CLIENTUPDATE_VELOCITY		(1<<2)
#define CLIENTUPDATE_AIMING			(1<<3)
#define CLIENTUPDATE_INPUTALLOWED	(1<<4)
#define CLIENTUPDATE_SCALE			(1<<5)
#define CLIENTUPDATE_DIMS			(1<<6)
#define CLIENTUPDATE_FLAGS			(1<<7)
#define CLIENTUPDATE_FLAGS2			(1<<8)
#define CLIENTUPDATE_MOVEMENTSTATE	(1<<9)
#define CLIENTUPDATE_CTRLFLAGS		(1<<10)
#define CLIENTUPDATE_ALL			(0xFFFF)


#endif  // __SHAREDMOVEMENT_H__



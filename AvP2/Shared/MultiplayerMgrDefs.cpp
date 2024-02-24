// ----------------------------------------------------------------------- //
//
// MODULE  : MultiplayerMgrDefs.cpp
//
// PURPOSE : Common definitions for multiplayer
//
// CREATED : 6/6/01
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MultiplayerMgrDefs.h"


// Guids...

#ifdef _DEMO
DGUID AVP2GUID = {  // {1DFB2BC2-EB40-11d2-B7D2-0060971766C1}
	0x1dfb2bc2, 0xeb40, 0x11d2, 0xb7, 0xd2, 0x0, 0x60, 0x97, 0x17, 0x66, 0xc1
};
#else
DGUID AVP2GUID = { // {1DFB2BC1-EB40-11d2-B7D2-0060971766C1}
	0x1dfb2bc1, 0xeb40, 0x11d2, 0xb7, 0xd2, 0x0, 0x60, 0x97, 0x17, 0x66, 0xc1 
};
#endif


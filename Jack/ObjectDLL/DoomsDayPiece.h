// ----------------------------------------------------------------------- //
//
// MODULE  : DoomsDayPiece.h
//
// PURPOSE : The object used for collecting DoomsDay pieces 
//
// CREATED : 12/13/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DOOMS_DAY_PIECE_H__
#define __DOOMS_DAY_PIECE_H__

//
// Includes...
//

#include "PropType.h"
#include "ChassisPiece.h"
	
LINKTO_MODULE( DoomsDayPiece );

class DoomsDayPiece : public ChassisPiece
{
	public: // Methods...

		DoomsDayPiece( );
		~DoomsDayPiece( );
		
	protected: // Methods... 

        uint32	EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );

		bool	OnRespawn( );

		// Message handlers...
		
		bool	OnCarry( HOBJECT hSender );
		bool	OnDrop( HOBJECT hSender );


	private:	// Members...

		bool	ReadProp( ObjectCreateStruct *pOCS );
};

class CDoomsDayPiecePlugin : public CChassisPiecePlugin
{
	public:

		virtual LTRESULT PreHook_EditStringList( 
			const char *szRezPath,
			const char *szPropName,
			char **aszStrings,
			uint32 *pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLen );
};

#endif // __DOOMS_DAY_PIECE_H__

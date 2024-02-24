// ----------------------------------------------------------------------- //
//
// MODULE  : DemolitionMissionMgr.h
//
// PURPOSE : Definition of class to handle demolition missions
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DEMOLITIONMISSIONMGR_H__
#define __DEMOLITIONMISSIONMGR_H__

//
// Includes...
//

#include "ServerMissionMgr.h"


class CDemolitionMissionMgr : public CServerMissionMgr
{
	public:

		CDemolitionMissionMgr();
		~CDemolitionMissionMgr();

	// Game type overrides...
	public:

		// Initializes the object.  Overrides should call up.
		virtual bool	Init( );

		// Called every frame.  Overrides should call up.
		virtual void	Update();


	// Game type overrides...
	protected:

		// Init game.
		virtual bool	StartGame( ILTMessage_Read& msg );

		// Called when the clients enter the world
		virtual void	LevelStarted();

		//handle updating multiplayer options while in game
		virtual bool	HandleMultiplayerOptions( HCLIENT hSender, ILTMessage_Read& msg );

		// Does loading of next level.
		virtual bool	FinishExitLevel( );

		// End game.
		virtual bool	EndGame( );
};

#endif // __DEMOLITIONMISSIONMGR_H__

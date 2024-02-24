// ----------------------------------------------------------------------- //
//
// MODULE  : GameStartPoint.h
//
// PURPOSE : GameStartPoint - Definition
//
// CREATED : 9/30/97
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_START_POINT_H__
#define __GAME_START_POINT_H__

// ----------------------------------------------------------------------- //

#include "GameBase.h"
#include "CharacterButeMgr.h"

// ----------------------------------------------------------------------- //

#define STARTPOINT_ALLOW_ALIENS			0x0001
#define STARTPOINT_ALLOW_MARINES		0x0002
#define STARTPOINT_ALLOW_PREDATORS		0x0004
#define STARTPOINT_ALLOW_CORPORATES		0x0008
#define STARTPOINT_ALLOW_QUEENS			0x0010
#define STARTPOINT_ALLOW_EXOSUITS		0x0020

#define STARTPOINT_ALLOW_TEAM_1			0x0100
#define STARTPOINT_ALLOW_TEAM_2			0x0200
#define STARTPOINT_ALLOW_TEAM_3			0x0400
#define STARTPOINT_ALLOW_TEAM_4			0x0800

#define STARTPOINT_ALLOW_ALL			0xFFFF

// ----------------------------------------------------------------------- //

class GameStartPoint : public GameBase
{
	public :

		// Construction and destruction
		GameStartPoint();
		~GameStartPoint();

		// Main access to the properties
		int			GetCharacterSet()			const { return m_nCharacterSet; }
		int			GetAllowFlags()				const { return m_nAllowFlags; }
		HSTRING		GetName()					const { return m_hstrName; }
		LTVector	GetPitchYawRoll()			const { return m_vPitchYawRoll; }

		HSTRING		GetTriggerTarget()			const { return m_hstrTriggerTarget; }
		HSTRING		GetTriggerMessage()			const { return m_hstrTriggerMessage; }

		HSTRING		GetMPCommand()				const { return m_hstrMPTriggerMessage; }

		HSTRING		GetMissionTarget()			const { return m_hstrMissionTarget; }
		HSTRING		GetMissionMessage()			const { return m_hstrMissionMessage; }
		LTBOOL		HasOpeningCinematic()		{ return m_bHasOpenCine; }

		void		SetExosuitRespawn();
		void		SpawnExosuit();

	protected :

		// Handles engine messages
		uint32		EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);

	private :

		// Reads the properties and saves them in the member variables
		LTBOOL		ReadProp(ObjectCreateStruct *pStruct);

		void		CacheFiles();

	private:

		// Member variables
		int			m_nCharacterSet;		// The type of character to spawn as (single player only)
		int			m_nAllowFlags;			// The type of team that is allowed to spawn at this point
		HSTRING		m_hstrName;				// Name of start point
		LTVector	m_vPitchYawRoll;		// Pitch, yaw, and roll of start point

		HSTRING		m_hstrTriggerTarget;	// Name of object to trigger at level start
		HSTRING		m_hstrTriggerMessage;	// Message to send to object at level start

		HSTRING		m_hstrMissionTarget;	// Name of object to trigger when starting mission
		HSTRING		m_hstrMissionMessage;	// Message to send to object when starting mission

		HSTRING		m_hstrMPTriggerMessage;	// Message to send to when MP level loads

		static uint32 m_dwCounter;			// Counts gamestartpoints created for the level.
		LTBOOL		m_bHasOpenCine;			// Does this level have an opening cinematic?
		LTFLOAT		m_fRespawnStartTime;	// Are we trying to respawn an exosuit?
};

// ----------------------------------------------------------------------- //

class CGameStartPointPlugin : public IObjectPlugin
{
  public:

	virtual DRESULT	PreHook_EditStringList(
		const char* szRezPath, 
		const char* szPropName, 
		char* const * aszStrings, 
		uint32* pcStrings, 
		const uint32 cMaxStrings, 
		const uint32 cMaxStringLength);

  private:

		CCharacterButeMgrPlugin m_CharacterButeMgrPlugin;
};

// ----------------------------------------------------------------------- //

#endif // __GAME_START_POINT_H__

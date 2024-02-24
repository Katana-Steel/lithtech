// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerMovement.h
//
// PURPOSE : General player movement
//
// CREATED : 2/29/00
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_MOVEMENT_H__
#define __PLAYER_MOVEMENT_H__

// ----------------------------------------------------------------------- //

#include "charactermovement.h"

// ----------------------------------------------------------------------- //

class PlayerMovement : public CharacterMovement
{
	public:
		// Constructor and destructors for our movement
		PlayerMovement();
		~PlayerMovement();

		// Initialization and destruction functions
		virtual LTBOOL	Init(INTERFACE *pInterface, HOBJECT hObj, CharacterAnimation* pAnim=LTNULL);

		void    FollowObject(HOBJECT hObjectToFollow) { m_hFollowObject = hObjectToFollow; }
		HOBJECT GetFollowObject() { return m_hFollowObject; }

		// Client/Server message control: Handles message from the server or client
		virtual LTBOOL	HandleMessageRead(uint8 nType, HMESSAGEREAD hRead, uint32 dwFlags = 0);
		virtual LTBOOL	HandleMessageWrite(uint8 nType, HMESSAGEWRITE hRead, uint32 dwFlags = 0);

		// Update the movement
		virtual void	Update();								// Updates the character movement state machine
		virtual void	UpdateRotation();						// Updates the character's rotation
		virtual void	UpdateAiming();							// Updates the character's aiming direction

		// Temp functions to support old MoveMgr code
		uint8	GetMoveCode()						{ return m_bMoveCode; }

		// Client update flag control
		void	ClearClientUpdateFlags()			{ m_nClientUpdateFlags = 0; }
		void	SetAllClientUpdateFlags()			{ m_nClientUpdateFlags = 0xFFFF; }

		void	AddClientUpdateFlag(uint16 nFlag)	{ m_nClientUpdateFlags |= nFlag; }

		// Client update
		uint16	GetClientUpdateFlags();
		void	UpdateClientToServer();

		// Local flag functions
		uint32	GetLocalControlFlags()				{ return m_nLocalControlFlags; }
		void	SetLocalControlFlags(uint32 nFlag)	{ m_nLocalControlFlags = nFlag; }

//		void	ResetNet();

		void	ReadSaveData(HMESSAGEREAD hData);
		void	WriteSaveData(HMESSAGEWRITE hData);
		void	RestoreSaveData();
		void	SetWeaponSpeedMod(LTFLOAT fMod) { m_Vars.m_fSpeedMod = fMod; }

		LTBOOL	CanPounce();

		void	SetMPButes(uint8 nCharacter);
		void	SetMPTeleport(LTVector vPos, uint8 nMoveCode);
		void	SetMPOrientation(LTVector vVec);


	private:

		// Special update functions
		void	UpdateCameraControl();

		// Message creation functions
		void	WriteMessage_UpdatePosition(HMESSAGEWRITE hWrite);
		void	WriteMessage_UpdateRotation(HMESSAGEWRITE hWrite);
		void	WriteMessage_UpdateVelocity(HMESSAGEWRITE hWrite);
		void	WriteMessage_UpdateAiming(HMESSAGEWRITE hWrite);
		void	WriteMessage_UpdateAllowedInput(HMESSAGEWRITE hWrite);
		void	WriteMessage_UpdateScale(HMESSAGEWRITE hWrite);
		void	WriteMessage_UpdateDims(HMESSAGEWRITE hWrite);
		void	WriteMessage_UpdateFlags(HMESSAGEWRITE hWrite);
		void	WriteMessage_UpdateFlags2(HMESSAGEWRITE hWrite);
		void	WriteMessage_UpdateMovementState(HMESSAGEWRITE hWrite);
		void	WriteMessage_UpdateCtrlFlags(HMESSAGEWRITE hWrite);

		// Special message handling functions for specific types
		LTBOOL	HandleMessage_ServForcePos(HMESSAGEREAD hRead, uint32 dwFlags);
		LTBOOL	HandleMessage_ServForceVelocity(HMESSAGEREAD hRead, uint32 dwFlags);
		LTBOOL	HandleMessage_Physics_Update(HMESSAGEREAD hRead, uint32 dwFlags);
		LTBOOL	HandleMessage_Orientation(HMESSAGEREAD hRead, uint32 dwFlags);

		// Special updates
		void	UpdateSpeedMods();

		// Movement helper functions
		void	CheckCharacterOverlaps();
		void	ResetCharacterOverlaps();


	private:

		// Temp variables to support old MoveMgr code
		uint8	m_bMoveCode;

		// Local movement control flags
		uint32	m_nLocalControlFlags;

		// Object to follow.  Left null if no object to follow.
		HOBJECT m_hFollowObject;

		// Client update flags
		uint16	m_nClientUpdateFlags;
		LTFLOAT	m_fClientUpdateTime;

		// Client update history variables
		LTVector	m_CUPos;
		LTRotation	m_CURot;
		LTVector	m_CUVel;
		int8		m_CUAimingPitch;
		uint8		m_CUAllowedInput;
		LTFLOAT		m_CUScale;
		LTVector	m_CUDims;
		uint32		m_CUFlags;
		uint16		m_CUFlags2;
		uint8		m_CUMovementState;
		uint16		m_CUCtrlFlags;
};

// ----------------------------------------------------------------------- //

#endif

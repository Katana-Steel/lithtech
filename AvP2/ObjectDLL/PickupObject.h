// ----------------------------------------------------------------------- //
//
// MODULE  : PickupObject.h
//
// PURPOSE : An object that can be picked up by characters
//
// CREATED : 8/2/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PICKUP_OBJECT_H__
#define __PICKUP_OBJECT_H__

// ----------------------------------------------------------------------- //

#include "GameBase.h"
#include "PickupButeMgr.h"
#include "ltsmartlink.h"
#include "Character.h"

// ----------------------------------------------------------------------- //

class PickupObject : public GameBase
{
	public:

		PickupObject();
		virtual ~PickupObject();
		LTBOOL		TouchNotify(HOBJECT hSender);

		virtual void PostStartWorld(uint8 nLoadGameFlags);
		HOBJECT	GetStartpointObject()				{ return m_hStartpointSpawner; }
		void	SetStartpointObject(HOBJECT hObj)	{ m_hStartpointSpawner = hObj; }

	protected :

		// Handles engine messages
		virtual uint32		EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual uint32		ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	private:

		// Local functions
		LTBOOL		ReadProp(ObjectCreateStruct *pData);
		LTBOOL		InitialUpdate();
		LTBOOL		Update();
		LTBOOL		Damage(HOBJECT hSender, HMESSAGEREAD hRead);
		LTBOOL		PickedUp(HOBJECT hSender, uint32* nAmt, LTBOOL bPartialPU);
		LTBOOL		TriggerMsg(HOBJECT hSender, const char* szMsg);

		// No load save needed.
		HOBJECT		m_hSender;

		// Save and load functions
		void		Save(HMESSAGEWRITE pData, uint32 fData);
		void		Load(HMESSAGEREAD pData, uint32 fData);

		// Moves the object to the floor with alignment to normal.
		LTBOOL		MoveAndAlignToFloor( );

	protected:

		PickupButes	m_puButes;

		LTFLOAT		m_fCreateTime;

		LTFLOAT		m_fCurrentHitPoints;
		LTFLOAT		m_fRespawnTime;
		LTFLOAT		m_fPickupDelay;
		LTBOOL		m_bInfiniteRespawns;
		LTBOOL		m_bSliding;
		LTBOOL		m_bNeedSebtle;
		LTBOOL		m_bForceNoRespawn;
		LTBOOL		m_bForceNoMoveToGround;
		uint32		m_nUsedAmount[4];
		LTBOOL		m_bAllowMovement;
		LTBOOL		m_bAllowRotation;

		HSTRING		m_hstrPickupMsg;
		HMODELANIM  m_hAnim;

		LTFLOAT		m_fLastTouchTime;

		LTSmartLink	m_hStartpointSpawner;


	public:

		ExosuitData*	m_pUserData;
		void			SetHitpoints(LTFLOAT fHPs)	{ m_fCurrentHitPoints = fHPs; }
		LTFLOAT			GetHitPoints()				{ return m_fCurrentHitPoints; }
		void			SetAllowMovement(LTBOOL b)	{ m_bAllowMovement = b; }
		HOBJECT			GetSender()					{ return m_hSender; }
		void			SetAllowRotation(LTBOOL b)	{ m_bAllowRotation = b; }
};

// ----------------------------------------------------------------------- //
// Plugin class for static string lists

class CPickupObjectPlugin : public IObjectPlugin
{
  public:

	virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath, 
		const char* szPropName, 
		char* const * aszStrings, 
		uint32* pcStrings, 
		const uint32 cMaxStrings, 
		const uint32 cMaxStringLength);

  protected :

		CPickupButeMgrPlugin m_PickupButeMgrPlugin;
};

// ----------------------------------------------------------------------- //

#endif  // __PICKUP_OBJECT_H__


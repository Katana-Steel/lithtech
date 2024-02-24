// ----------------------------------------------------------------------- //
//
// MODULE  : AmmoBox.cpp
//
// PURPOSE : Definition of the Ammo Box object
//
// CREATED : 2/5/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AMMO_BOX_H__
#define __AMMO_BOX_H__

#include "Prop.h"
#include "PickupButeMgr.h"

const uint32 MAX_PICKUPS=10;

class AmmoBox : public Prop
{
	public :

		AmmoBox();
		~AmmoBox();

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

        LTBOOL	ReadProp(ObjectCreateStruct *pData);
        LTBOOL	InitialUpdate();
		LTBOOL	Update();

		void	SetupOpenState(HOBJECT hObj);

	private :

		LTBOOL	m_bOpened;
		LTBOOL	m_bLocked;
		LTBOOL	m_bCanActivate;
		LTBOOL	m_bCanTouch;
		LTBOOL	m_bHumanOnly;

		LTFLOAT	m_fRespawnTime;
		LTFLOAT	m_fRespawnDelay;

		LTString m_strPickupCommand[MAX_PICKUPS];

		LTString m_strOpenCmd;

		//members below here do not need load/save

		void	TriggerMsg(HOBJECT hSender, HSTRING hMsg);

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
};

// ----------------------------------------------------------------------- //
// Plugin class for static string lists

class CAmmoBoxPlugin : public IObjectPlugin
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

#endif // __AMMO_BOX_H__

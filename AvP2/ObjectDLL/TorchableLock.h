// ----------------------------------------------------------------------- //
//
// MODULE  : TorchableLock.cpp
//
// PURPOSE : Definition of the Torchable Lock object
//
// CREATED : 11/16/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TORCHABLE_LOCK_H__
#define __TORCHABLE_LOCK_H__

#include "Prop.h"

enum TorchMode
{
	INVALID = -1,
	BOLT,
	WELD,
};

class TorchableLock : public Prop
{
	public :

		TorchableLock();
		~TorchableLock();

		TorchMode	GetMode(LTFLOAT &fRatio);
		void		DestroyBolt();
		void		SetEnabled(LTBOOL bOn);
		void		Remove() { g_pLTServer->RemoveObject(m_hObject); }

		void		SetCounterObject(HOBJECT hObj)	{ m_hCounterObject = hObj; }

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

        LTBOOL   ReadProp(ObjectCreateStruct *pData);
        LTBOOL   InitialUpdate();

		void	SetupCutState(LTBOOL bForceModeChange=LTFALSE);
		void	SetupWeldedState();

	private :
		
		uint32	m_nCutHitPoints;
		uint32	m_nWeldHitPoints;
		LTFLOAT	m_fMaxCutHitPoints;
		LTFLOAT m_fMaxWeldHitPoints;

		LTString m_hstrDebrisType;

		LTString m_hstrAltModel;
		LTString m_hstrAltSkin;
		LTVector m_vAltModelScale;

		LTSmartLink m_hCounterObject;

		TorchMode m_eMode;
		LTBOOL	m_bAllowModeChange;

		LTBOOL	m_bEnabled;

		LTString	m_strLockCmd;
		LTString	m_strOpenCmd;

		LTBOOL	m_bCanDamage;

		void	HandleDamage(HOBJECT hSender, HMESSAGEREAD hRead);

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

		void	Cache();
};

#endif // __TORCHABLE_LOCK_H__

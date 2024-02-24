// ----------------------------------------------------------------------- //
//
// MODULE  : TorchableLockCounter.h
//
// PURPOSE : TorchableLockCounter - Definition
//
// CREATED : 11.20.2000
//
// ----------------------------------------------------------------------- //

#ifndef __TORCHABLE_LOCK_COUNTER_H__
#define __TORCHABLE_LOCK_COUNTER_H__

// ----------------------------------------------------------------------- //

#include "GameBase.h"
#include "LTString.h"
#include "ltsmartlink.h"
#include "ServerUtilities.h"

// ----------------------------------------------------------------------- //

#define MAX_LOCKS_PER_SIDE			5

// ----------------------------------------------------------------------- //

class TorchableLockCounter : public GameBase
{
	public:

		TorchableLockCounter();
		~TorchableLockCounter();

        uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32	ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	protected:

		LTBOOL	Update();
        LTBOOL	ReadProp(ObjectCreateStruct *pData);
		void	TriggerMsg(HOBJECT hSender, char* pMsg);

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

	private:

		void	SetupObjectHandles();
		HOBJECT	FindSideMatch(HOBJECT hObj);
		void	DestroyAllLocks();
		void	EnableAllLocks();
		void	RemoveAllLocks();
		LTBOOL	StateOfAllLocksEquals(int nState);

	private:

		// Target information
		LTString	m_szMainTarget;
		LTSmartLink	m_hMainTarget;

		LTString	m_strLockCmd;
		LTString	m_strOpenCmd;

		// Lock object information
		LTString	m_szSideA[MAX_LOCKS_PER_SIDE];
		LTSmartLink	m_hSideA[MAX_LOCKS_PER_SIDE];

		LTString	m_szSideB[MAX_LOCKS_PER_SIDE];
		LTSmartLink	m_hSideB[MAX_LOCKS_PER_SIDE];


		// Update information
		LTBOOL		m_bTriggerWhenUnlocked;
		LTBOOL		m_bFirstUpdate;
};

// ----------------------------------------------------------------------- //

#endif // __TORCHABLE_LOCK_COUNTER_H__


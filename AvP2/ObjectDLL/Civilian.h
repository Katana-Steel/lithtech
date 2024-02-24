// ----------------------------------------------------------------------- //
//
// MODULE  : Civilian.h
//
// PURPOSE : A minimal civilian object
//
// CREATED : 12/21/00
//
// ----------------------------------------------------------------------- //

#ifndef __CIVILIAN_H__
#define __CIVILIAN_H__

#include "cpp_engineobjects_de.h"
#include "Prop.h"
#include "fsm.h"
#include "Character.h"
#include "Timer.h"

class Civilian : public Prop
{
	public :

		Civilian();
		~Civilian();

	protected :

		struct Root
		{
			enum State
			{
				eStateIdle,
				eStateCower
			};

			enum Message
			{
				eStimulus,
				eSafe
			};
		};

		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;
		RootFSM	m_RootFSM;	


		virtual DDWORD	EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT lData);
		virtual DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		LTBOOL	InitialUpdate();
		virtual void Update();

		LTBOOL	ReadProp(ObjectCreateStruct *pData);
		LTBOOL	Setup(ObjectCreateStruct *pData );
		void	PostPropRead(ObjectCreateStruct* pData);

		LTBOOL	m_bPlayerUsable;
		HSTRING	m_hstrActivateMessage;
		HSTRING	m_hstrActivateTarget;

	private :

		void		SetupFSM();

		void		StartIdle(RootFSM *);
		void		UpdateIdle(RootFSM *);

		void		StartCower(RootFSM *);
		void		UpdateCower(RootFSM *);

		HOBJECT		CreateBody(LTBOOL bCarryOverAttachments);

		void		CreateSpecialFX();

		void		Save(HMESSAGEWRITE hWrite);
		void		Load(HMESSAGEREAD hRead);
		void		CacheFiles();

		HSTRING				m_hstrCowerSound;
		LTFLOAT				m_fSoundRadius;

		uint32				m_nCharacterSet;

		std::string			m_sIdleAni;
		std::string			m_sCowerAni;
		std::string			m_sDeathAni;

		CTimer				m_tmrCower;
		LTBOOL				m_bLockState;
		LTBOOL				m_bEnableCower;

		LTFLOAT				m_fMinAlienRangeSqr;
		LTFLOAT				m_fMinPredatorRangeSqr;
		LTFLOAT				m_fMinMarineRangeSqr;
		LTFLOAT				m_fMinCorporateRangeSqr;

		LTFLOAT				m_fCowerTime;
		LTFLOAT				m_fTime;
};

class CivilianPlugin : public IObjectPlugin
{
	public:
		virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char* const * aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

	private:
		CCharacterButeMgrPlugin m_CharacterButeMgrPlugin;
};


#endif // __CIVILIAN_H__

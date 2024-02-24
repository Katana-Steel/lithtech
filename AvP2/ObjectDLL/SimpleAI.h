#ifndef __SIMPLE_AI_H__
#define __SIMPLE_AI_H__

#include "Character.h"
#include "MusicMgr.h"

class CPlayerObj;

#include <string>
#include <list>

class CSimpleAI : public CCharacter
{
	public:
		
		CSimpleAI();
		virtual ~CSimpleAI();

		typedef std::list<CSimpleAI*> SimpleAIList;
		typedef SimpleAIList::iterator SimpleAIIterator;

		// Be sure to override this in derived classes
		virtual CharacterClass  GetCharacterClass() const  { return UNKNOWN; }

		// Each simple AI should deal with this!
		virtual void	UpdateAnimation()       { UpdateWeaponAnimation(); UpdateActionAnimation(); }
		virtual	WEAPON*	UpdateWeaponAnimation() { return LTNULL; }
		virtual void	UpdateActionAnimation() { }

		HOBJECT			GetTarget() const { return m_hTarget; }
		void			SetTarget(HOBJECT hTarget);

		const CCharacter *	GetCharTargetPtr() const { return m_pCharTarget; }

		virtual std::string GetCharacterSetName(const std::string & root_name) const { return root_name + std::string("_SAI"); }

	protected :

		virtual uint32	EngineMessageFn(uint32 dwID, void *pData, LTFLOAT fData);

		virtual void	InitialUpdate();
		virtual void	FirstUpdate();
		virtual void	Update();

		virtual LTBOOL	IgnoreDamageMsg(const DamageStruct & damage_msg);
		virtual void	ProcessDamageMsg(const DamageStruct & damage_msg);
		virtual LTBOOL	ProcessCommand(const char* const* pTokens, int nArgs);

		virtual void	Save(HMESSAGEWRITE hWrite);
		virtual void	Load(HMESSAGEREAD hRead);

		virtual void	HandleModelString(ArgList *pArgList);
		
		void SetMusicMood(CMusicMgr::EMood eMood);

		LTBOOL  IsFirstUpdate() const { return m_bFirstUpdate; }

		CCharacter *	IsEnemyInRange(LTFLOAT fRangeSqr);
		CPlayerObj *	IsPlayerInRange(LTFLOAT fRangeSqr);
		CCharacter *	IsCharacterInRange(LTFLOAT fRangeSqr);
		CSimpleAI  *	IsSimpleAIInRange(LTFLOAT fRangeSqr);
		
		LTBOOL			IsTargetVertical();		// is the target aligned with our up vector?
		LTBOOL			WithinLeash(const LTVector & vPos);

		void			AvoidSimpleAI();

		void			Hidden(bool hide, bool save_flags = true );

		LTFLOAT			GetDetectionRadiusSqr() const { return m_fDetectionRadiusSqr; }
		LTFLOAT			GetAlertRadiusSqr() const { return m_fAlertRadiusSqr; }
		LTFLOAT			GetMaxMeleeRange() const { return m_fMaxMeleeRange; }
		LTFLOAT			GetMinMeleeRange() const { return m_fMinMeleeRange; }

		virtual LTBOOL	ReadProp(ObjectCreateStruct *pStruct);
//		virtual void	PostReadProp(ObjectCreateStruct *pStruct);
//		virtual void	HandleModelString(ArgList* pArgList);
//		virtual void	CacheFiles();

	private :

		LTFLOAT			m_fDetectionRadiusSqr;
		LTFLOAT			m_fAlertRadiusSqr;
		LTSmartLink		m_hTarget;
		const CCharacter*		m_pCharTarget;

		LTBOOL			m_bFirstUpdate;
		LTFLOAT			m_fUpdateDelta;
		LTBOOL			m_bIgnoreAI;

		std::string		m_strLeashFromName;
		LTFLOAT			m_fLeashLengthSqr;
		LTVector		m_vLeashFrom;
		
		LTFLOAT			m_fMaxMeleeRange;
		LTFLOAT			m_fMinMeleeRange;
		LTFLOAT			m_fMeleeDamage;

		LTBOOL			m_bHidden;
		uint32			m_dwActiveFlags;
		uint32			m_dwActiveFlags2;
		uint32			m_dwActiveUserFlags;
		LTBOOL			m_bActiveCanDamage;

		LTBOOL			m_bSentInfo;

		// Music mood modifiers
		std::string		m_strMusicAmbientTrackSet;
		std::string		m_strMusicWarningTrackSet;
		std::string		m_strMusicHostileTrackSet;

		static			SimpleAIList sm_SimpleAI;
};


class CSimpleAIPlugin : public IObjectPlugin
{
	public:

		virtual ~CSimpleAIPlugin() {}

		virtual DRESULT	PreHook_EditStringList(const char* szRezPath, const char* szPropName, char* const * aszStrings, DDWORD* pcStrings, const DDWORD cMaxStrings, const DDWORD cMaxStringLength);

	private :
		
};

#endif

// ----------------------------------------------------------------------- //
//
// MODULE  : Prop.h
//
// PURPOSE : Model Prop - Definition
//
// CREATED : 10/9/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROP_H__
#define __PROP_H__

#include "GameBase.h"
#include "DestructibleModel.h"

class CCharacter;
class CPlayerObj;


class Prop : public GameBase
{
	public :

		Prop();
		virtual ~Prop();

		CDestructibleModel* GetDestructible() { return &m_damage; }

		virtual LTBOOL		CanShootThrough() { return m_bCanShootThrough; }
		virtual LTBOOL		CanSeeThrough()   { return m_bCanSeeThrough; }
		virtual void		RemoveOnLinkBroken(LTBOOL b=LTTRUE) { m_bRemoveOnLinkBroken=b; }

	protected :

		uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		virtual void TriggerMsg(HOBJECT hSender, HSTRING hMsg);
		virtual void HandleModelString(ArgList *);

		CCharacter *	IsEnemyInRange(LTFLOAT fRange);
		CPlayerObj *	IsPlayerInRange(LTFLOAT fRange);
		CCharacter *	IsCharacterInRange(LTFLOAT fRange);

		CDestructibleModel			m_damage;

		LTFLOAT			m_fAlpha;
		HSTRING			m_hstrTouchSound;
		LTVector		m_vScale;
		LTVector		m_vObjectColor;
		uint32			m_dwFlags;
		uint32			m_dwFlags2;
		LTBOOL			m_bIgnoreAI;

		LTBOOL			m_bCanShootThrough;
		LTBOOL			m_bCanSeeThrough;

		HSTRING			m_hstrLoopSound;

		//Place members that do not need load/save under here
		HLTSOUND		m_hTouchSnd;
		HLTSOUND		m_hLoopSound;

		std::string		m_strKeyCommand;
		LTBOOL			m_bOneTimeKeyCommand;

	private :
		LTBOOL			m_bRemoveOnLinkBroken;

		void	ReadProp(ObjectCreateStruct *pStruct);
		void	PostPropRead(ObjectCreateStruct *pStruct);
		void	InitialUpdate();

		void    Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		void    Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		void	CacheFiles();

		void	HandleTouch(HOBJECT hObj);
};

class CPropPlugin : public IObjectPlugin
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
	
};

#endif // __PROP_H__

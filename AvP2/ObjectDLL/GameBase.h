// ----------------------------------------------------------------------- //
//
// MODULE  : GameBase.h
//
// PURPOSE : Game base object class definition
//
// CREATED : 10/8/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_BASE_H__
#define __GAME_BASE_H__

// ----------------------------------------------------------------------- //

#include "ltengineobjects.h"
#include "iobjectplugin.h"

// ----------------------------------------------------------------------- //

class GameBase : public BaseClass
{
	public:

		GameBase(uint8 nType = OT_NORMAL);
		virtual ~GameBase();

		virtual void PostStartWorld(uint8 nLoadGameFlags);

#ifndef _FINAL
		virtual void		CreateBoundingBox();
		virtual void		RemoveBoundingBox();
		virtual void		UpdateBoundingBox();
#endif
		virtual LTBOOL		CanShootThrough() { return LTFALSE; }
		virtual LTBOOL		CanSeeThrough()   { return LTFALSE; }

        uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		LTFLOAT GetUpdateDelta() const { return m_fUpdateDelta; }


		// Used to tell the object to move to floor on the next MID_UPDATE
		void				SetMoveToFloor( LTBOOL bMoveToFloor ) { m_bMoveToFloor = bMoveToFloor; }
		void				SetTeleportToFloor( LTBOOL bMoveToFloor ) { m_bTeleportToFloor = bMoveToFloor; }
		LTBOOL				GetMoveToFloor( ) { return m_bMoveToFloor; }

	protected:

		// Handles engine messages
		virtual uint32		EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual LTBOOL		AllowInGameType();

        LTBOOL	ReadProp(ObjectCreateStruct *pData);
#ifndef _FINAL
		virtual	LTVector	GetBoundingBoxColor();
#endif
		virtual void SetNextUpdate(LTFLOAT fDelta, LTFLOAT fDeactivateTime = 0.0f);

	private:

		void				TriggerMsg(HOBJECT hSender, const char* pMsg);
		void				Save(HMESSAGEWRITE hWrite);
		void				Load(HMESSAGEREAD hRead);
		void				Update( );

	protected:

		HSTRING		m_szName;

		uint32		m_dwGameTypes;
		uint32		m_dwDifficulty;
		uint32		m_dwClassWeapons;

		HOBJECT		m_hDimsBox;
		LTVector	m_vColor;
		LTFLOAT		m_fAlpha;

		LTFLOAT		m_fUpdateDelta;
		LTFLOAT		m_fLastUpdateTime;

        uint32      m_dwOriginalFlags;

		LTBOOL		m_bStartHidden;

	private:

		LTBOOL		m_bMoveToFloor;
		LTBOOL		m_bTeleportToFloor;
};

// ----------------------------------------------------------------------- //
// Plugin class for static string lists

class CGameBasePlugin : public IObjectPlugin
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

extern const LTFLOAT c_fMaxUpdateDelta;

// ----------------------------------------------------------------------- //

#endif  // __GAME_BASE_H__


// ----------------------------------------------------------------------- //
//
// MODULE  : DebugLineFX.h
//
// PURPOSE : DebugLine special fx class -- Provides line system for debugging AI's.
//
// CREATED : 3/27/00
//
// ----------------------------------------------------------------------- //

#ifndef __DEBUGLINE_FX_H__
#define __DEBUGLINE_FX_H__

#include "LTBaseTypes.h"
#include "BaseLineSystemFX.h"
#include "DebugLine.h"

#include <deque>

struct DebugLineCreator : public SFXCREATESTRUCT
{
};

class CDebugLineFX : public CBaseLineSystemFX
{

	public :

		typedef std::pair<HLTLINE,DebugLine> LineListElement;
		typedef std::deque< LineListElement > LineList;

	public :

		CDebugLineFX() 
			: m_nMaxLines(0),
			  m_bUpdateLines(false),
			  m_bClearOldLines(false),
			  m_vLastLocation(0,0,0) {}

		virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CreateObject(CClientDE* pClientDE);
		
		virtual void WantRemove(DBOOL bRemove);

		virtual LTBOOL Update();

		virtual	LTBOOL OnServerMessage(HMESSAGEREAD hMessage);

	protected :

		int m_nMaxLines;

		LineList lines;

		bool m_bUpdateLines;
		bool m_bClearOldLines;
		LTVector m_vLastLocation;
};

//-------------------------------------------------------------------------------------------------
// SFX_DebugLineFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_DebugLineFactory : public CSpecialFXFactory
{
	SFX_DebugLineFactory() : CSpecialFXFactory(SFX_DEBUGLINE_ID) {;}
	static const SFX_DebugLineFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __DEBUGLINE_FX_H__
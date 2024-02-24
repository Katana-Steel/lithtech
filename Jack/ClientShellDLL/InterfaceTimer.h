// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceTimer.h
//
// PURPOSE : Definition of InterfaceTimer class
//
// CREATED : 10/18/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __INTERFACE_TIMER_H__
#define __INTERFACE_TIMER_H__

#include "ltbasedefs.h"
#include "iltfontmanager.h"
#include "TeamMgr.h"

extern ILTFontManager*	g_pFontManager;

class CInterfaceTimer
{
	public:

		CInterfaceTimer();
		~CInterfaceTimer();

		bool		Init( uint8 nFont, uint8 nBaseSize, LTIntPt const& basePos, LTVector const& vColor,
						CUI_ALIGNMENTTYPE eAlignment );
		void		Draw();

		void        SetTime(LTFLOAT fTime, bool bPause) { m_fTime = fTime; m_fTimeLeft = m_fTime - g_pLTClient->GetGameTime( ); m_bPause = bPause; }
		float		GetTime() const { return m_fTime; }
		float		GetTimeLeft( ) const { return m_fTimeLeft; }

		void		ScreenDimsChanged();

	private:

		// Server game time when timer runs out.
		float					m_fTime;
		float					m_fTimeLeft;
		bool					m_bPause;

		CUIFormattedPolyString	*m_pTimeStr;

		CUIFont*		m_pFont;
		LTIntPt			m_BasePos;
		uint8			m_nBaseSize;
		uint32			m_nColor;
		CUI_ALIGNMENTTYPE	m_eAlignment;
};

#endif  // __INTERFACE_TIMER_H__
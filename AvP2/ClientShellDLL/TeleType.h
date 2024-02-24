// -------------------------------------------------------------------- //
//
// MODULE  : TeleType.h
//
// PURPOSE : A client-side text print out class
//
// CREATED : 3/27/01
//
// -------------------------------------------------------------------- //

#ifndef __TELETYPE_H__
#define __TELETYPE_H__

// -------------------------------------------------------------------- //

#include "LTGUIMgr.h"

// -------------------------------------------------------------------- //

class TeleType
{
	public:

		// -------------------------------------------------------------------- //
		// Construction and destruction

		TeleType();
		~TeleType();

		LTBOOL		Init();
		void		Term();


		// -------------------------------------------------------------------- //
		// Control functions

		LTBOOL		Start(uint16 nID, uint8 nStyle = 0);
		void		Stop();

		void		Update();

		LTBOOL		OnKeyDown(int key, int rep);

	private:

		// -------------------------------------------------------------------- //
		// Update helper functions

		void		Update_Style0();
		void		Update_Style1();


		// -------------------------------------------------------------------- //
		// Drawing helper functions

		void		Draw_Style0_Background(LTIntPt ptPos, LTIntPt ptSize, int nSteps, HSURFACE hSurf);
		void		Draw_Style1_Background(LTIntPt ptPos, LTIntPt ptSize, int nSteps, HSURFACE hSurf);


	private:

		// -------------------------------------------------------------------- //
		// The data that we don't want people screwing around with directly

		CLTGUIFont			*m_pFont;								// The font to draw with
		LITHFONTDRAWDATA	m_dd;									// The draw data for the text
		LITHFONTSAVEDATA	m_sd;									// The save data for timed text display

		uint32				m_nWidth;								// Formatted text width
		uint32				m_nHeight;								// Formatted text height

		HSTRING				m_hString;								// The string we're drawing
		HSURFACE			m_hBackground;							// The surface for the background
		HSURFACE			m_hEdgeBackground;						// The edge surface for the background
		HLTSOUND			m_hSound;								// The looping sound
		LTFLOAT				m_fTimeStamp;							// A general time stamp
		uint8				m_nState;								// The current state
		uint8				m_nStyle;								// The render style of the teletype
};

// -------------------------------------------------------------------- //

#endif


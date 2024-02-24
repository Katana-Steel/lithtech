// ----------------------------------------------------------------------- //
//
// MODULE  : StoryElement.cpp
//
// PURPOSE : StoryElement implimentation
//
// CREATED : 05.22.2000
//
// ----------------------------------------------------------------------- //

//external dependencies
#include "stdafx.h"
#include "StoryElement.h"
#include "SoundMgr.h"
#include "GameClientShell.h"

//external globals
extern CGameClientShell* g_pGameClientShell;
extern CSoundMgr* g_pSoundMgr;

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	StoryElement::StoryElement
//
//	PURPOSE:	Constructor
//
// --------------------------------------------------------------------------- //

StoryElement::StoryElement()
{
	m_hImage		= LTNULL;
	m_hSoundFile	= LTNULL;
	m_fDuration		= 0.0f;
	m_fStartTime	= 0.0f;
	m_nWidth		= 0;
	m_nHeight		= 0;
	m_fRangeSqr		= 0.0f;
	m_hSurface		= LTNULL;
	m_bInitUpdate	= DTRUE;
	m_nStringID		= -1;
	m_hString		= LTNULL;
	m_nFontId		= 0;	
	m_nYOffset		= -1;
	m_nXOffset		= -1;
	m_nTextWidth	= -1;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	StoryElement::~StoryElement
//
//	PURPOSE:	Destructor
//
// --------------------------------------------------------------------------- //

StoryElement::~StoryElement()
{
//	No need to clean up, res manager handles this
//	m_hSurface

	//play the sound
	if(m_hSoundFile)
	{
		char* pSound = g_pLTClient->GetStringData(m_hSoundFile);

		if (pSound)
			g_pClientSoundMgr->PlaySoundLocal(pSound);
	}

	if(m_hString)
	{
		g_pClientDE->FreeString(m_hString);
		m_hString = LTNULL;
	}

	if(m_hImage)
	{
		g_pClientDE->FreeString(m_hImage);
		m_hImage = LTNULL;
	}

	if(m_hSoundFile)
	{
		g_pClientDE->FreeString(m_hSoundFile);
		m_hSoundFile = LTNULL;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	void StoryElement::Init
//
//	PURPOSE:	Sets up image and starts playing sound
//
// --------------------------------------------------------------------------- //

void StoryElement::Init()
{
	if(m_hImage)
	{
		//create surface
		m_hSurface = g_pInterfaceResMgr->GetSharedSurface(g_pClientDE->GetStringData(m_hImage));

		//set transparent color
		HDECOLOR hTransColor = SETRGB_T(0,0,0);

		//set translucency
		g_pClientDE->SetSurfaceAlpha (m_hSurface, 1.0f);

		//get the dims of our surface
		g_pClientDE->GetSurfaceDims(m_hSurface, &m_nWidth, &m_nHeight);
	}

	if(m_nStringID != -1.0f)
	{
		//clean up the old string
		if(m_hString)
		{
			g_pClientDE->FreeString(m_hString);
			m_hString = LTNULL;
		}

		//load up the new string
		m_hString = g_pLTClient->FormatString(m_nStringID);
	}

	//reset the timer
	m_fStartTime = g_pClientDE->GetTime();

	//play the sound
	if(m_hSoundFile)
	{
		char* pSound = g_pLTClient->GetStringData(m_hSoundFile);

		if (pSound)
			g_pClientSoundMgr->PlaySoundLocal(pSound);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	void StoryElement::Update
//
//	PURPOSE:	Displays the image to the screen and checks time
//
// --------------------------------------------------------------------------- //

void StoryElement::Update()
{
	//sanity check
	if ( !g_pClientDE ) return;

	if(m_hSurface)
	{
		//get handle to screen surface
		HSURFACE hScreen = g_pClientDE->GetScreenSurface();

		//local variables
		DDWORD nScreenWidth, nScreenHeight;

		//get the dims of our surface
		g_pClientDE->GetSurfaceDims(hScreen, &nScreenWidth, &nScreenHeight);

		//set transparent color
		HDECOLOR hTransColor = SETRGB_T(0,0,0);

		//draw our element to the screen
		g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen, 
														m_hSurface, 
														LTNULL,
														(nScreenWidth>>1) - (m_nWidth>>1), 
														(nScreenHeight>>1) - (m_nHeight>>1), 
														hTransColor);
	}

	if(m_hString)
	{
		//display the string
		CLTGUIFont* pFont = GetFont();

		if( pFont )
		{

			LITHFONTDRAWDATA DrawData;

			//set drawdata elements
			DrawData.dwFlags		= LTF_DRAW_FORMATTED; 
			DrawData.byJustify		= m_nFontId==0?LTF_JUSTIFY_CENTER:LTF_JUSTIFY_LEFT;
			DrawData.dwFormatWidth	= m_nTextWidth;

			//get screen handle and dims
			HSURFACE	hScreen = g_pClientDE->GetScreenSurface();
			DDWORD		nHeight, nWidth;
			g_pClientDE->GetSurfaceDims (hScreen, &nWidth, &nHeight);

			// Set horiz pos...
			if(m_nXOffset < 0)
				nWidth >>= 1;
			else
			{
				nWidth >>= 1;
				nWidth -= (m_nWidth>>1);
				nWidth += m_nXOffset;
			}

			// Set vert pos...
			if(m_nYOffset < 0)
			{
				nHeight >>= 1;
				nHeight += 120;
			}
			else
			{
				nHeight >>= 1;
				nHeight -= (m_nHeight>>1);
				nHeight += m_nYOffset;
			}

			pFont->Draw(	g_pLTClient->GetStringData(m_hString), 
							hScreen, 
							&DrawData,
							nWidth, nHeight, 
							LTNULL);
		}
	}

	// Check time
	if(m_fDuration > 0.0f)
		if(g_pClientDE->GetTime()-m_fStartTime >= m_fDuration)
			//time to end
			g_pGameClientShell->EndStoryObject();

	// Check range
	if(CheckRange())
		//out of range
		g_pGameClientShell->EndStoryObject();

	// See if we are dead
	if(g_pGameClientShell->IsPlayerDead())
		// Oops...
		g_pGameClientShell->EndStoryObject();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	void StoryElement::Update
//
//	PURPOSE:	Displays the image to the screen and checks time
//
// --------------------------------------------------------------------------- //

LTBOOL StoryElement::CheckRange()
{
	HLOCALOBJ hPlayerObj = g_pClientDE->GetClientObject();

	//set statics
	if(m_bInitUpdate)
	{
		g_pClientDE->GetObjectPos(hPlayerObj, &m_vInitPos);
		m_bInitUpdate = LTFALSE;
	}

	LTVector vCurPos;

	g_pClientDE->GetObjectPos(hPlayerObj, &vCurPos);

	//test range
	if( (vCurPos-m_vInitPos).MagSqr() > m_fRangeSqr )
	{
		return DTRUE;
	}
	else
		return DFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	void StoryElement::GetFont
//
//	PURPOSE:	Get the display font
//
// --------------------------------------------------------------------------- //

CLTGUIFont* StoryElement::GetFont()
{
	switch((const uint8)m_nFontId)
	{
	case (1):	return g_pInterfaceResMgr->GetPDAFont();
	case (2):	return g_pInterfaceResMgr->GetMEMOFont();
	default:	return g_pInterfaceMgr->GetObjectiveFont();
	}
}

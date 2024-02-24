// ----------------------------------------------------------------------- //
//
// MODULE  : SpriteControlFX.cpp
//
// PURPOSE : Sprite control
//
// CREATED : 7/16/00
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "spritecontrolfx.h"
#include "iltspritecontrol.h"

// ----------------------------------------------------------------------- //

#define SPRITECONTROL_PLAYMODE_STOPPED			0
#define SPRITECONTROL_PLAYMODE_PLAYING			1
#define SPRITECONTROL_PLAYMODE_LOOPING			2

// ----------------------------------------------------------------------- //

LTBOOL CSpriteControlFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	// Make sure everything is okie dokie
	if(!CSpecialFX::Init(hServObj, hRead)) return LTFALSE;
	if(!hRead) return LTFALSE;

	SPRCTRLCREATESTRUCT sprCtrl;

	// Setup the FX create structure
	sprCtrl.hServerObj		= hServObj;
	sprCtrl.nPlayMode		= g_pLTClient->ReadFromMessageByte(hRead);
	sprCtrl.nStartFrame		= g_pLTClient->ReadFromMessageByte(hRead);
	sprCtrl.nEndFrame		= g_pLTClient->ReadFromMessageByte(hRead);
	sprCtrl.nFramerate		= g_pLTClient->ReadFromMessageByte(hRead);

	// Return the value from the structured init
	return Init(&sprCtrl);
}

// ----------------------------------------------------------------------- //

LTBOOL CSpriteControlFX::Init(SFXCREATESTRUCT *psfx)
{
	// Do the base FX initialization (this should fill in the ServerObj variable for us)
	if(!CSpecialFX::Init(psfx)) return LTFALSE;

	// Cast it to a point that we want
	SPRCTRLCREATESTRUCT *pFX = (SPRCTRLCREATESTRUCT*)psfx;

	// Fill in all our derived class member variables
	m_nPlayMode		= pFX->nPlayMode;
	m_nStartFrame	= pFX->nStartFrame;
	m_nEndFrame		= pFX->nEndFrame;
	m_nFramerate	= pFX->nFramerate;

	// Set the initial frame update time
	m_fFrameUpdateTime	= g_pLTClient->GetTime();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

LTBOOL CSpriteControlFX::Update()
{
	// If the server object is invalid, just return
	if(!m_hServerObject) return LTFALSE;

	// Get a handle to the sprite control interface
	ILTSpriteControl *pSprCtrl;
	g_pLTClient->GetSpriteControl(m_hServerObject, pSprCtrl);

	// Get the current game time
	LTFLOAT fTime = g_pLTClient->GetTime();

	// Get the details of the sprite
	uint32 nNumAnims;
	uint32 nNumFrames;

	pSprCtrl->GetNumAnims(nNumAnims);
	pSprCtrl->GetNumFrames(0, nNumFrames);

	// Turn off the sprite animation handling in the engine... cause we're gonna control it ourselves
	pSprCtrl->SetFlags(0);

	// Check to make sure the frame values are within range
	if((m_nStartFrame < 0) || (m_nStartFrame >= nNumFrames))
		m_nStartFrame = 0;

	if(m_nEndFrame < m_nStartFrame)
		m_nEndFrame = m_nStartFrame;
	else if(m_nEndFrame >= nNumFrames)
		m_nEndFrame = (uint8)(nNumFrames - 1);

	// Calculate the total number of animation frames we want to play
	uint8 nNumAnimFrames = (m_nEndFrame - m_nStartFrame) + 1;

	// Calculate the total time it should take to play this sprite
	LTFLOAT fTotalTime = nNumAnimFrames * (1.0f / (LTFLOAT)m_nFramerate);

	// Set the default frame at zero...
	uint8 nFrame = 0;

	// Depending on what state we're on... do something a little different
	switch(m_nPlayMode)
	{
		case SPRITECONTROL_PLAYMODE_PLAYING:
		{
			// Calculate the frame we should be on...
			LTFLOAT fRatio = (fTime - m_fFrameUpdateTime) / fTotalTime;
			if(fRatio > 1.0f)
				nFrame = m_nEndFrame;
			else
			{
				nFrame = (uint8)(nNumAnimFrames * fRatio);
				nFrame = (nFrame >= nNumAnimFrames) ? m_nEndFrame : (m_nStartFrame + nFrame);
			}
			break;
		}

		case SPRITECONTROL_PLAYMODE_LOOPING:
		{
			// Calculate the frame we should be on...
			LTFLOAT fRatio = (fTime - m_fFrameUpdateTime) / fTotalTime;
			nFrame = (uint8)(nNumAnimFrames * fRatio);
			nFrame = m_nStartFrame + (nFrame % nNumAnimFrames);
			break;
		}

		case SPRITECONTROL_PLAYMODE_STOPPED:
		default:
		{
			nFrame = m_nStartFrame;
			break;
		}
	}

	// Set the frame now...
	pSprCtrl->SetCurPos(0, nFrame);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

LTBOOL CSpriteControlFX::OnServerMessage(HMESSAGEREAD hRead)
{
	if(!hRead) return LTFALSE;

	// Read in the new property values
	m_nPlayMode			= g_pLTClient->ReadFromMessageByte(hRead);
	m_nStartFrame		= g_pLTClient->ReadFromMessageByte(hRead);
	m_nEndFrame			= g_pLTClient->ReadFromMessageByte(hRead);
	m_nFramerate		= g_pLTClient->ReadFromMessageByte(hRead);

	// Reset the frame update time
	m_fFrameUpdateTime	= g_pLTClient->GetTime();

	return LTTRUE;
}

//-------------------------------------------------------------------------------------------------
// SFX_SpriteControlFactory
//-------------------------------------------------------------------------------------------------

const SFX_SpriteControlFactory SFX_SpriteControlFactory::registerThis;

CSpecialFX* SFX_SpriteControlFactory::MakeShape() const
{
	return new CSpriteControlFX();
}


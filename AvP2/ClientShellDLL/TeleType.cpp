// -------------------------------------------------------------------- //
//
// MODULE  : TeleType.cpp
//
// PURPOSE : A client-side multiplayer statistics manager
//
// CREATED : 3/27/01
//
// -------------------------------------------------------------------- //


#include "stdafx.h"
#include "TeleType.h"
#include "VarTrack.h"

#include "ClientSoundMgr.h"
#include "InterfaceResMgr.h"
#include "ClientButeMgr.h"

// -------------------------------------------------------------------- //

#define TELETYPE_STATE_OFF				0
#define TELETYPE_STATE_PRE_OPEN			1
#define TELETYPE_STATE_OPENING			2
#define TELETYPE_STATE_POST_OPEN		3
#define TELETYPE_STATE_TYPE				4
#define TELETYPE_STATE_PRE_CLOSE		5
#define TELETYPE_STATE_CLOSING			6
#define TELETYPE_STATE_POST_CLOSE		7

// -------------------------------------------------------------------- //

VarTrack g_vtTeleTypeOpenSpeed;
VarTrack g_vtTeleTypeCloseSpeed;

VarTrack g_vtTeleTypeAlphaSteps;
VarTrack g_vtTeleTypeStartAlpha;
VarTrack g_vtTeleTypeEndAlpha;

VarTrack g_vtTeleTypeEdge;
VarTrack g_vtTeleTypeIndent;
VarTrack g_vtTeleTypeColorEdge;

VarTrack g_vtTeleTypeS0PreTypeDelay;
VarTrack g_vtTeleTypeS0PostTypeDelay;
VarTrack g_vtTeleTypeS0LetterDelay;
VarTrack g_vtTeleTypeS0LineDelay;
VarTrack g_vtTeleTypeS0YOffset;

VarTrack g_vtTeleTypeS1PreTypeDelay;
VarTrack g_vtTeleTypeS1PostTypeDelay;
VarTrack g_vtTeleTypeS1LetterDelay;
VarTrack g_vtTeleTypeS1LineDelay;
VarTrack g_vtTeleTypeS1YOffset;

// -------------------------------------------------------------------- //
//
// FUNCTION:	TeleType::TeleType()
//
// PURPOSE:		Initialize any variables that need it
//
// -------------------------------------------------------------------- //

TeleType::TeleType()
{
	m_pFont = LTNULL;

	m_nWidth = 0;
	m_nHeight = 0;

	m_hString = LTNULL;
	m_hBackground = LTNULL;
	m_hEdgeBackground = LTNULL;
	m_hSound = LTNULL;
	m_fTimeStamp = 0.0f;
	m_nState = TELETYPE_STATE_OFF;
	m_nStyle = 0;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	TeleType::~TeleType()
//
// PURPOSE:		Mass Destruction!! Bwah ha ha!
//
// -------------------------------------------------------------------- //

TeleType::~TeleType()
{
	Term();
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	TeleType::Init()
//
// PURPOSE:		Setup the multiplayer manager
//
// -------------------------------------------------------------------- //

LTBOOL TeleType::Init()
{
	// Create the backgrounds
	m_hBackground = g_pLTClient->CreateSurfaceFromBitmap("Interface\\MP\\background.pcx");
	m_hEdgeBackground = g_pLTClient->CreateSurfaceFromBitmap("Interface\\MP\\background.pcx");

	m_pFont = g_pInterfaceResMgr->GetHelpFont();


	// General style variables
	if(!g_vtTeleTypeOpenSpeed.IsInitted())
		g_vtTeleTypeOpenSpeed.Init(g_pLTClient, "TeleTypeOpenSpeed", LTNULL, 0.33f);
	if(!g_vtTeleTypeCloseSpeed.IsInitted())
		g_vtTeleTypeCloseSpeed.Init(g_pLTClient, "TeleTypeCloseSpeed", LTNULL, 0.33f);

	if(!g_vtTeleTypeAlphaSteps.IsInitted())
		g_vtTeleTypeAlphaSteps.Init(g_pLTClient, "TeleTypeAlphaSteps", LTNULL, 30.0f);
	if(!g_vtTeleTypeStartAlpha.IsInitted())
		g_vtTeleTypeStartAlpha.Init(g_pLTClient, "TeleTypeStartAlpha", LTNULL, 1.0f);
	if(!g_vtTeleTypeEndAlpha.IsInitted())
		g_vtTeleTypeEndAlpha.Init(g_pLTClient, "TeleTypeEndAlpha", LTNULL, 0.01f);

	if(!g_vtTeleTypeEdge.IsInitted())
		g_vtTeleTypeEdge.Init(g_pLTClient, "TeleTypeEdge", LTNULL, 15.0f);
	if(!g_vtTeleTypeIndent.IsInitted())
		g_vtTeleTypeIndent.Init(g_pLTClient, "TeleTypeIndent", LTNULL, 25.0f);
	if(!g_vtTeleTypeColorEdge.IsInitted())
		g_vtTeleTypeColorEdge.Init(g_pLTClient, "TeleTypeColorEdge", LTNULL, 2.0f);


	// Style specific variables
	if(!g_vtTeleTypeS0PreTypeDelay.IsInitted())
		g_vtTeleTypeS0PreTypeDelay.Init(g_pLTClient, "TeleTypeS0PreTypeDelay", LTNULL, 0.25f);
	if(!g_vtTeleTypeS0PostTypeDelay.IsInitted())
		g_vtTeleTypeS0PostTypeDelay.Init(g_pLTClient, "TeleTypeS0PostTypeDelay", LTNULL, 3.0f);
	if(!g_vtTeleTypeS0LetterDelay.IsInitted())
		g_vtTeleTypeS0LetterDelay.Init(g_pLTClient, "TeleTypeS0LetterDelay", LTNULL, 0.025f);
	if(!g_vtTeleTypeS0LineDelay.IsInitted())
		g_vtTeleTypeS0LineDelay.Init(g_pLTClient, "TeleTypeS0LineDelay", LTNULL, 0.5f);
	if(!g_vtTeleTypeS0YOffset.IsInitted())
		g_vtTeleTypeS0YOffset.Init(g_pLTClient, "TeleTypeS0YOffset", LTNULL, 110.0f);

	if(!g_vtTeleTypeS1PreTypeDelay.IsInitted())
		g_vtTeleTypeS1PreTypeDelay.Init(g_pLTClient, "TeleTypeS1PreTypeDelay", LTNULL, 0.25f);
	if(!g_vtTeleTypeS1PostTypeDelay.IsInitted())
		g_vtTeleTypeS1PostTypeDelay.Init(g_pLTClient, "TeleTypeS1PostTypeDelay", LTNULL, 30.0f);
	if(!g_vtTeleTypeS1LetterDelay.IsInitted())
		g_vtTeleTypeS1LetterDelay.Init(g_pLTClient, "TeleTypeS1LetterDelay", LTNULL, 0.0f);
	if(!g_vtTeleTypeS1LineDelay.IsInitted())
		g_vtTeleTypeS1LineDelay.Init(g_pLTClient, "TeleTypeS1LineDelay", LTNULL, 0.0f);
	if(!g_vtTeleTypeS1YOffset.IsInitted())
		g_vtTeleTypeS1YOffset.Init(g_pLTClient, "TeleTypeS1YOffset", LTNULL, 330.0f);


	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	TeleType::Term()
//
// PURPOSE:		Free up all the multiplayer manager resources
//
// -------------------------------------------------------------------- //

void TeleType::Term()
{
	// Free the string
	if(m_hString)
	{
		g_pLTClient->FreeString(m_hString);
		m_hString = LTNULL;
	}

	// Free the surfaces
	if(m_hBackground)
	{
		g_pLTClient->DeleteSurface(m_hBackground);
		m_hBackground = LTNULL;
	}

	if(m_hEdgeBackground)
	{
		g_pLTClient->DeleteSurface(m_hEdgeBackground);
		m_hEdgeBackground = LTNULL;
	}

	// Free the sound
	if(m_hSound)
	{
		g_pLTClient->KillSound(m_hSound);
		m_hSound = LTNULL;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	TeleType::Start()
//
// PURPOSE:		Start the teletype text
//
// -------------------------------------------------------------------- //

LTBOOL TeleType::Start(uint16 nID, uint8 nStyle)
{
	// Free the string
	if(m_hString)
	{
		g_pLTClient->FreeString(m_hString);
		m_hString = LTNULL;
	}


	// Now store the string
	m_hString = g_pLTClient->FormatString(nID);


	// Free the sound
	if(m_hSound)
	{
		g_pLTClient->KillSound(m_hSound);
		m_hSound = LTNULL;
	}


	if(m_hString)
	{
		// Set the state
		m_nState = TELETYPE_STATE_PRE_OPEN;
		m_nStyle = nStyle;

		// Setup the draw data
		m_dd.dwFlags = LTF_DRAW_FORMATTED | LTF_DRAW_TIMED | LTF_TIMED_LETTERS | LTF_TIMED_LINES;
		m_sd.sPrevString.Empty();

		// Setup the color of the background surface
		HLTCOLOR hColor;
		HLTCOLOR hEdgeColor;
		HLTCOLOR hTransColor = SETRGB_T(0, 0, 0);

		switch(m_nStyle)
		{
			case 1:
			{
				m_dd.byJustify		= LTF_JUSTIFY_CENTER;
				m_dd.dwFormatWidth	= 400 - ((int)g_vtTeleTypeIndent.GetFloat() * 3);
				m_dd.fLetterDelay	= g_vtTeleTypeS1LetterDelay.GetFloat();
				m_dd.fLineDelay		= g_vtTeleTypeS1LineDelay.GetFloat();

				hColor = SETRGB(16, 16, 96);
				hEdgeColor = SETRGB(144, 144, 224);
				break;
			}

			case 0:
			default:
			{
				m_dd.byJustify		= LTF_JUSTIFY_LEFT;
				m_dd.dwFormatWidth	= 600 - ((int)g_vtTeleTypeIndent.GetFloat() * 3);
				m_dd.fLetterDelay	= g_vtTeleTypeS0LetterDelay.GetFloat();
				m_dd.fLineDelay		= g_vtTeleTypeS0LineDelay.GetFloat();

				hColor = SETRGB(16, 16, 16);
				hEdgeColor = SETRGB(144, 144, 144);
				break;
			}
		}

		// Fill in the backgrounds
		g_pLTClient->FillRect(m_hBackground, LTNULL, hColor);
		g_pLTClient->OptimizeSurface(m_hBackground, hTransColor);

		g_pLTClient->FillRect(m_hEdgeBackground, LTNULL, hEdgeColor);
		g_pLTClient->OptimizeSurface(m_hEdgeBackground, hTransColor);


		// Get the dimensions of the string...
		LTIntPt pt = m_pFont->GetTextExtentsFormat(m_hString, m_dd.dwFormatWidth);
		m_nWidth = pt.x;
		m_nHeight = pt.y;
	}


	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	TeleType::Stop()
//
// PURPOSE:		Force the teletype to stop
//
// -------------------------------------------------------------------- //

void TeleType::Stop()
{
	m_nState = TELETYPE_STATE_OFF;

	// Free the sound
	if(m_hSound)
	{
		g_pLTClient->KillSound(m_hSound);
		m_hSound = LTNULL;
	}

	// Free the string
	if(m_hString)
	{
		g_pLTClient->FreeString(m_hString);
		m_hString = LTNULL;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	TeleType::Update()
//
// PURPOSE:		Handle the passed in message if necessary
//
// -------------------------------------------------------------------- //

void TeleType::Update()
{
	// Just return if it's off...
	if((m_nState == TELETYPE_STATE_OFF) || !m_hString || !m_pFont)
		return;


	// Update based on style type
	switch(m_nStyle)
	{
		case 1:
		{
			Update_Style1();
			break;
		}

		case 0:
		default:
		{
			Update_Style0();
			break;
		}
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	TeleType::Update_Style0()
//
// PURPOSE:		Handle style 0 updates
//
// -------------------------------------------------------------------- //

void TeleType::Update_Style0()
{
	// Setup some common variables...
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	uint32 nWidth, nHeight;

	g_pLTClient->GetSurfaceDims(hScreen, &nWidth, &nHeight);

	LTFLOAT fTime = g_pLTClient->GetTime();
	LTFLOAT fTimeDif = fTime - m_fTimeStamp;

	LTIntPt ptSize(m_nWidth + (int)(g_vtTeleTypeIndent.GetFloat() * 3), m_nHeight + (int)(g_vtTeleTypeEdge.GetFloat() * 2));
	LTIntPt ptPos(0, nHeight - ptSize.y - (int)(g_vtTeleTypeS0YOffset.GetFloat() * g_pInterfaceResMgr->GetYRatio()) );
	LTIntPt ptTextPos((int)g_vtTeleTypeIndent.GetFloat(), ptPos.y + (int)g_vtTeleTypeEdge.GetFloat());
	


	// Handle things differently based on the state...
	switch(m_nState)
	{
		case TELETYPE_STATE_PRE_OPEN:
		{
			// Play the open sound...
			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\teletype_open.wav", SOUNDPRIORITY_MISC_MEDIUM);

			m_nState = TELETYPE_STATE_OPENING;
			m_fTimeStamp = g_pLTClient->GetTime();
			break;
		}

		case TELETYPE_STATE_OPENING:
		{
			// Go on to the next state...
			if(fTimeDif >= g_vtTeleTypeOpenSpeed.GetFloat())
			{
				m_nState = TELETYPE_STATE_POST_OPEN;
				m_fTimeStamp = g_pLTClient->GetTime();
			}

			int nSteps = (int)(g_vtTeleTypeAlphaSteps.GetFloat() * (fTimeDif / g_vtTeleTypeOpenSpeed.GetFloat()));
			Draw_Style0_Background(ptPos, ptSize, nSteps, hScreen);

			break;
		}

		case TELETYPE_STATE_POST_OPEN:
		{
			// Go on to the next state...
			if(fTimeDif >= g_vtTeleTypeS0PreTypeDelay.GetFloat())
			{
				m_nState = TELETYPE_STATE_TYPE;
				m_fTimeStamp = g_pLTClient->GetTime();

				m_sd.fStartDrawTime	= m_fTimeStamp;
			}

			int nSteps = (int)g_vtTeleTypeAlphaSteps.GetFloat();
			Draw_Style0_Background(ptPos, ptSize, nSteps, hScreen);

			break;
		}

		case TELETYPE_STATE_TYPE:
		{
			LTBOOL bPlaySound = LTFALSE;
			int nSteps = (int)g_vtTeleTypeAlphaSteps.GetFloat();
			Draw_Style0_Background(ptPos, ptSize, nSteps, hScreen);

			m_pFont->Draw(m_hString, hScreen, &m_dd, ptTextPos.x, ptTextPos.y, &m_sd);

			// Go on to the next state...
			if(m_sd.byLastState == LTF_STATE_DRAW_FINISHED)
			{
				m_nState = TELETYPE_STATE_PRE_CLOSE;
				m_fTimeStamp = g_pLTClient->GetTime();
			}
			else if(m_sd.byLastState == LTF_STATE_DRAW_UPDATING)
			{
				bPlaySound = LTTRUE;
			}

			// Take care of playing or freeing the sound
			if(bPlaySound && !m_hSound)
			{
				m_hSound = g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\teletype_loop.wav", SOUNDPRIORITY_MISC_MEDIUM, PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP);
			}
			else if(!bPlaySound && m_hSound)
			{
				g_pLTClient->KillSound(m_hSound);
				m_hSound = LTNULL;
			}

			break;
		}

		case TELETYPE_STATE_PRE_CLOSE:
		{
			// Go on to the next state...
			if(fTimeDif >= g_vtTeleTypeS0PostTypeDelay.GetFloat())
			{
				m_nState = TELETYPE_STATE_CLOSING;
				m_fTimeStamp = g_pLTClient->GetTime();

				// Play the close sound...
				g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\teletype_close.wav", SOUNDPRIORITY_MISC_MEDIUM);
			}

			int nSteps = (int)g_vtTeleTypeAlphaSteps.GetFloat();
			Draw_Style0_Background(ptPos, ptSize, nSteps, hScreen);

			m_pFont->Draw(m_hString, hScreen, &m_dd, ptTextPos.x, ptTextPos.y, &m_sd);

			break;
		}

		case TELETYPE_STATE_CLOSING:
		{
			// Go on to the next state...
			if(fTimeDif >= g_vtTeleTypeCloseSpeed.GetFloat())
			{
				m_nState = TELETYPE_STATE_POST_CLOSE;
				m_fTimeStamp = g_pLTClient->GetTime();
			}

			int nSteps = (int)(g_vtTeleTypeAlphaSteps.GetFloat() * (1.0f - (fTimeDif / g_vtTeleTypeCloseSpeed.GetFloat())));
			Draw_Style0_Background(ptPos, ptSize, nSteps, hScreen);

			break;
		}

		case TELETYPE_STATE_POST_CLOSE:
		{
			m_nState = TELETYPE_STATE_OFF;

			break;
		}
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	TeleType::Update_Style1()
//
// PURPOSE:		Handle style 1 updates
//
// -------------------------------------------------------------------- //

void TeleType::Update_Style1()
{
	// Setup some common variables...
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	uint32 nWidth, nHeight;

	g_pLTClient->GetSurfaceDims(hScreen, &nWidth, &nHeight);

	LTFLOAT fTime = g_pLTClient->GetTime();
	LTFLOAT fTimeDif = fTime - m_fTimeStamp;


	LTIntPt ptSize(m_nWidth + (int)(g_vtTeleTypeIndent.GetFloat() * 5), m_nHeight + (int)(g_vtTeleTypeEdge.GetFloat() * 2));
	LTIntPt ptPos(nWidth / 2, nHeight - ptSize.y - (int)(g_vtTeleTypeS1YOffset.GetFloat() * g_pInterfaceResMgr->GetYRatio()));
	LTIntPt ptTextPos(ptPos.x, ptPos.y + (int)g_vtTeleTypeEdge.GetFloat());


	// Handle things differently based on the state...
	switch(m_nState)
	{
		case TELETYPE_STATE_PRE_OPEN:
		{
			// Play the open sound...
			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\hint_open.wav", SOUNDPRIORITY_MISC_MEDIUM);

			m_nState = TELETYPE_STATE_OPENING;
			m_fTimeStamp = g_pLTClient->GetTime();
			break;
		}

		case TELETYPE_STATE_OPENING:
		{
			// Go on to the next state...
			if(fTimeDif >= g_vtTeleTypeOpenSpeed.GetFloat())
			{
				m_nState = TELETYPE_STATE_POST_OPEN;
				m_fTimeStamp = g_pLTClient->GetTime();
			}

			int nSteps = (int)(g_vtTeleTypeAlphaSteps.GetFloat() * (fTimeDif / g_vtTeleTypeOpenSpeed.GetFloat()));
			Draw_Style1_Background(ptPos, ptSize, nSteps, hScreen);

			break;
		}

		case TELETYPE_STATE_POST_OPEN:
		{
			// Go on to the next state...
			if(fTimeDif >= g_vtTeleTypeS1PreTypeDelay.GetFloat())
			{
				m_nState = TELETYPE_STATE_TYPE;
				m_fTimeStamp = g_pLTClient->GetTime();

				m_sd.fStartDrawTime	= m_fTimeStamp;
			}

			int nSteps = (int)g_vtTeleTypeAlphaSteps.GetFloat();
			Draw_Style1_Background(ptPos, ptSize, nSteps, hScreen);

			break;
		}

		case TELETYPE_STATE_TYPE:
		{
			LTBOOL bPlaySound = LTFALSE;
			int nSteps = (int)g_vtTeleTypeAlphaSteps.GetFloat();
			Draw_Style1_Background(ptPos, ptSize, nSteps, hScreen);

			m_pFont->Draw(m_hString, hScreen, &m_dd, ptTextPos.x, ptTextPos.y, &m_sd);

			// Go on to the next state...
			if(m_sd.byLastState == LTF_STATE_DRAW_FINISHED)
			{
				m_nState = TELETYPE_STATE_PRE_CLOSE;
				m_fTimeStamp = g_pLTClient->GetTime();
			}
			else if(m_sd.byLastState == LTF_STATE_DRAW_UPDATING)
			{
				bPlaySound = LTTRUE;
			}

			// Take care of playing or freeing the sound
			if(bPlaySound && !m_hSound)
			{
				m_hSound = g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\hint_loop.wav", SOUNDPRIORITY_MISC_MEDIUM, PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP);
			}
			else if(!bPlaySound && m_hSound)
			{
				g_pLTClient->KillSound(m_hSound);
				m_hSound = LTNULL;
			}

			break;
		}

		case TELETYPE_STATE_PRE_CLOSE:
		{
			// Go on to the next state...
			if(fTimeDif >= g_vtTeleTypeS1PostTypeDelay.GetFloat())
			{
				m_nState = TELETYPE_STATE_CLOSING;
				m_fTimeStamp = g_pLTClient->GetTime();

				// Play the close sound...
				g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\hint_close.wav", SOUNDPRIORITY_MISC_MEDIUM);
			}

			int nSteps = (int)g_vtTeleTypeAlphaSteps.GetFloat();
			Draw_Style1_Background(ptPos, ptSize, nSteps, hScreen);

			m_pFont->Draw(m_hString, hScreen, &m_dd, ptTextPos.x, ptTextPos.y, &m_sd);

			break;
		}

		case TELETYPE_STATE_CLOSING:
		{
			// Go on to the next state...
			if(fTimeDif >= g_vtTeleTypeCloseSpeed.GetFloat())
			{
				m_nState = TELETYPE_STATE_POST_CLOSE;
				m_fTimeStamp = g_pLTClient->GetTime();
			}

			int nSteps = (int)(g_vtTeleTypeAlphaSteps.GetFloat() * (1.0f - (fTimeDif / g_vtTeleTypeCloseSpeed.GetFloat())));
			Draw_Style1_Background(ptPos, ptSize, nSteps, hScreen);

			break;
		}

		case TELETYPE_STATE_POST_CLOSE:
		{
			m_nState = TELETYPE_STATE_OFF;

			break;
		}
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	TeleType::Draw_Style0_Background()
//
// PURPOSE:		Draws the background
//
// -------------------------------------------------------------------- //

void TeleType::Draw_Style0_Background(LTIntPt ptPos, LTIntPt ptSize, int nSteps, HSURFACE hSurf)
{
	// Calculate the width of each step...
	int nStepWidth = (int)((LTFLOAT)ptSize.x / g_vtTeleTypeAlphaSteps.GetFloat());

	LTFLOAT fAlpha;
	LTFLOAT fAlphaStep = (g_vtTeleTypeEndAlpha.GetFloat() - g_vtTeleTypeStartAlpha.GetFloat()) / g_vtTeleTypeAlphaSteps.GetFloat();

	// Go through each step and blit it to the screen...
	for(int i = 0; i < nSteps; i++)
	{
		fAlpha = g_vtTeleTypeStartAlpha.GetFloat() + ((LTFLOAT)i * fAlphaStep);

		fAlpha = (1.0f - fAlpha);
		fAlpha *= fAlpha;
		fAlpha = (1.0f - fAlpha);

		g_pLTClient->SetSurfaceAlpha(m_hBackground, fAlpha);
		g_pLTClient->ScaleSurfaceToSurface(hSurf, m_hBackground, &LTRect(ptPos.x, ptPos.y, ptPos.x + nStepWidth, ptPos.y + ptSize.y), LTNULL);

		g_pLTClient->SetSurfaceAlpha(m_hEdgeBackground, fAlpha);
		g_pLTClient->ScaleSurfaceToSurface(hSurf, m_hEdgeBackground, &LTRect(ptPos.x, ptPos.y - (int)g_vtTeleTypeColorEdge.GetFloat(), ptPos.x + nStepWidth, ptPos.y), LTNULL);
		g_pLTClient->ScaleSurfaceToSurface(hSurf, m_hEdgeBackground, &LTRect(ptPos.x, ptPos.y + ptSize.y, ptPos.x + nStepWidth, ptPos.y + ptSize.y + (int)g_vtTeleTypeColorEdge.GetFloat()), LTNULL);

		ptPos.x += nStepWidth;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	TeleType::Draw_Style1_Background()
//
// PURPOSE:		Draws the background
//
// -------------------------------------------------------------------- //

void TeleType::Draw_Style1_Background(LTIntPt ptPos, LTIntPt ptSize, int nSteps, HSURFACE hSurf)
{
	// Calculate the width of each step...
	int nStepWidth = (int)((LTFLOAT)(ptSize.x / 2) / g_vtTeleTypeAlphaSteps.GetFloat());

	LTFLOAT fAlpha;
	LTFLOAT fAlphaStep = (g_vtTeleTypeEndAlpha.GetFloat() - g_vtTeleTypeStartAlpha.GetFloat()) / g_vtTeleTypeAlphaSteps.GetFloat();

	int nLeftX = ptPos.x;
	int nRightX = ptPos.x - nStepWidth;

	// Go through each step and blit it to the screen...
	for(int i = 0; i < nSteps; i++)
	{
		fAlpha = g_vtTeleTypeStartAlpha.GetFloat() + ((LTFLOAT)i * fAlphaStep);

		fAlpha = (1.0f - fAlpha);
		fAlpha *= fAlpha;
		fAlpha = (1.0f - fAlpha);

		g_pLTClient->SetSurfaceAlpha(m_hBackground, fAlpha);
		g_pLTClient->ScaleSurfaceToSurface(hSurf, m_hBackground, &LTRect(nRightX, ptPos.y, nRightX + nStepWidth, ptPos.y + ptSize.y), LTNULL);
		g_pLTClient->ScaleSurfaceToSurface(hSurf, m_hBackground, &LTRect(nLeftX, ptPos.y, nLeftX + nStepWidth, ptPos.y + ptSize.y), LTNULL);

		g_pLTClient->SetSurfaceAlpha(m_hEdgeBackground, fAlpha);
		g_pLTClient->ScaleSurfaceToSurface(hSurf, m_hEdgeBackground, &LTRect(nRightX, ptPos.y - (int)g_vtTeleTypeColorEdge.GetFloat(), nRightX + nStepWidth, ptPos.y), LTNULL);
		g_pLTClient->ScaleSurfaceToSurface(hSurf, m_hEdgeBackground, &LTRect(nRightX, ptPos.y + ptSize.y, nRightX + nStepWidth, ptPos.y + ptSize.y + (int)g_vtTeleTypeColorEdge.GetFloat()), LTNULL);
		g_pLTClient->ScaleSurfaceToSurface(hSurf, m_hEdgeBackground, &LTRect(nLeftX, ptPos.y - (int)g_vtTeleTypeColorEdge.GetFloat(), nLeftX + nStepWidth, ptPos.y), LTNULL);
		g_pLTClient->ScaleSurfaceToSurface(hSurf, m_hEdgeBackground, &LTRect(nLeftX, ptPos.y + ptSize.y, nLeftX + nStepWidth, ptPos.y + ptSize.y + (int)g_vtTeleTypeColorEdge.GetFloat()), LTNULL);

		nRightX += nStepWidth;
		nLeftX -= nStepWidth;
	}
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	TeleType::OnKeyDown()
//
// PURPOSE:		Handle key presses, return TRUE if we handled the key
//				presss.
//
// -------------------------------------------------------------------- //

LTBOOL TeleType::OnKeyDown(int key, int rep)
{
	// We only care if we're actually drawing
	if((m_nState == TELETYPE_STATE_OFF) || !m_hString || !m_pFont)
		return LTFALSE;

	// Currently this just checks to see if the Escape or cinematic skip key was
	// pressed when we're showing hints...

	if ( 1 == m_nStyle && 
		 ((g_pClientButeMgr->GetCinematicSkipKey() == key ||
		   g_pClientButeMgr->GetEscapeKey() == key)) )
	{
		// If we haven't played the close sound yet, go ahead and play it...

		if (TELETYPE_STATE_CLOSING != m_nState && TELETYPE_STATE_POST_CLOSE != m_nState)
		{
			// Play the close sound...
			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\hint_close.wav", SOUNDPRIORITY_MISC_MEDIUM);
		}

		Stop();
		return LTTRUE;
	}

	return LTFALSE;
}

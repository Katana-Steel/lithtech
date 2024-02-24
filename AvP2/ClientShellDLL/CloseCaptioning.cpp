#include "stdafx.h"

#include "CloseCaptioning.h"

#include "CloseCaptionBute.h"
#include "VarTrack.h"
#include "ClientSoundMgr.h"
#include "InterfaceResMgr.h"

#pragma warning (disable : 4786)
#include <vector>
#include <utility>


// Forward declare local functions
void		LoadFont(CaptionData *data, std::string font);
void		LoadImages(CaptionData *data, std::string image, LTIntPt point, float alpha);
void		LoadFaces(CaptionData *data, std::string image, LTIntPt point, float alpha);
void		FreeFont(CaptionData *data);

VarTrack	g_vtRadioLipScale;


static const std::string PRENOM("Interface\\StatusBar\\Radio\\av_");
static const std::string FILE_TYPE(".pcx");
static const int FACE_COUNT = 3;

struct CaptionData
{
	CaptionData() : m_pFont(NULL), m_MouthPosition(0), m_bDraw(false) {;}

	LithFont*					m_pFont;			//pointer to small number font

	std::string					m_Caption;			// What do you want to say!
	std::string					m_Speaker;			// Who said that!
	int							m_MouthPosition;	// Which pcx to play

	std::string					m_strStopSound;		// The sound to be played when the caption is cleared.

	CloseCaptioning::ImageList	m_Images;			// Container of image handles
	CloseCaptioning::ImageList	m_Faces;			// List of faces

	LITHFONTSAVEDATA			m_FontData;			// Saved data for the draw routine
	LITHFONTDRAWDATA			m_FontDrawData;		// Draw data

	bool						m_bDraw;
};



CloseCaptioning::CloseCaptioning()
{
	m_pData = new CaptionData();

	LITHFONTDRAWDATA lfDD;
	
	//set drawdata elements
	lfDD.byJustify		= LTF_JUSTIFY_LEFT;
	lfDD.nLetterSpace	= 0;
	lfDD.nLineSpace		= 0;
	lfDD.dwFormatWidth	= 100;
	lfDD.rcClip.left	= 100;
	lfDD.rcClip.top		= 100;
	lfDD.rcClip.right	= 300;
	lfDD.rcClip.bottom	= 200;
	lfDD.dwFlags		= LTF_DRAW_FORMATTED | LTF_USE_ALL | LTF_DRAW_TIMED | LTF_TIMED_ALL;
//	lfDD.bForceFormat	= true;
	lfDD.fLetterDelay	= 0.1f;					// Time it takes to draw one letter (0.1 would draw 10 letters a second)
	lfDD.fLineDelay		= 1.0f;						// Time the drawing will delay inbetween lines
	lfDD.fLineScrollTime = 1.0f;				// Time it takes to scroll to the next line
	lfDD.nNumWaitLines	= 2;					// Number of lines to wait until scrolling the text or locking the last line

	m_pData->m_FontDrawData = lfDD;

}

CloseCaptioning::~CloseCaptioning()
{
	// Free the font
	FreeFont(m_pData);

	RemoveImages();
}

void CloseCaptioning::Draw()
{
	if (!m_pData->m_bDraw)
	{
		return;
	}

	//set transparent color
	static HDECOLOR hTransColor = SETRGB_T(0,0,0);

	//get screen handle and dims
	HSURFACE	hScreen = g_pClientDE->GetScreenSurface();
	DDWORD		screenWidth, screenHeight;
	g_pClientDE->GetSurfaceDims (hScreen, &screenWidth, &screenHeight);

	// Draw sprites first.
	for(ImageList::iterator iter = m_pData->m_Images.begin(); iter != m_pData->m_Images.end(); ++iter)
	{
		// Do something here for screen resolution
		DDWORD x = iter->second.x;
		DDWORD y = iter->second.y;
//		DDWORD x = static_cast<unsigned long>(iter->second.x * screenWidth);
//		DDWORD y = static_cast<unsigned long>(iter->second.y * screenHeight);

		g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen,
														iter->first, 
														DNULL, 
														x, y,
														hTransColor);
	}

	// Draw face if exists
	if (!m_pData->m_Faces.empty())
	{
		DDWORD x = m_pData->m_Faces[m_pData->m_MouthPosition].second.x;
		DDWORD y = m_pData->m_Faces[m_pData->m_MouthPosition].second.y;

		g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen,
														m_pData->m_Faces[m_pData->m_MouthPosition].first, 
														DNULL, 
														x, y,
														hTransColor);
	}

	//set default X and Y drawing offsets
//	DDWORD	nXStart = static_cast<unsigned long>(m_pData->m_Dims.inLeft * screenWidth);
//	DDWORD	nYStart = static_cast<unsigned long>(m_pData->m_Dims.inTop * screenHeight);

	DDWORD	nXStart = m_pData->m_FontDrawData.rcClip.left;
	DDWORD	nYStart = m_pData->m_FontDrawData.rcClip.top;

	if (m_pData->m_pFont)
	{
		m_pData->m_pFont->Draw(	hScreen,
								const_cast<char*>(m_pData->m_Caption.c_str()),
								&m_pData->m_FontDrawData, 
								nXStart, 
								nYStart,
								&m_pData->m_FontData);
	}
	
}

void CloseCaptioning::StopCaptions()
{
	RemoveImages();

	m_pData->m_bDraw = false;

	if( !m_pData->m_strStopSound.empty() )
	{
		g_pClientSoundMgr->PlaySoundLocal( m_pData->m_strStopSound.c_str() );

		m_pData->m_strStopSound.clear();
	}
}

void CloseCaptioning::SetDims (LTRect rect)
{
	m_pData->m_FontDrawData.rcClip = rect;

	ResetDimensions();
}

void CloseCaptioning::SetFont(std::string font)
{
	// Be sure we dont already have a font...
	FreeFont(m_pData);
	LoadFont(m_pData, font);
}

void CloseCaptioning::AddImage(LTIntPt point, std::string image, float alpha)
{
	LoadImages(m_pData, image, point, alpha);
}

void CloseCaptioning::AddFaces(LTIntPt point, std::string name, float alpha)
{
	if (name != "None")
	{
		LoadFaces(m_pData, name, point, alpha);
	}
}

void CloseCaptioning::RemoveImages()
{
	// Free all the surfaces we may have
//	for(ImageList::iterator iter = m_pData->m_Images.begin(); iter < m_pData->m_Images.end(); ++iter)
//	{
//		g_pClientDE->DeleteSurface (iter->first);
//	}

	// empty the container
	if(m_pData->m_Images.size())
	{
		m_pData->m_Images.clear();
	}

	// Free all the surfaces we may have
//	for(iter = m_pData->m_Faces.begin(); iter < m_pData->m_Faces.end(); ++iter)
//	{
//		g_pClientDE->DeleteSurface (iter->first);
//	}

	// empty the container
	if(m_pData->m_Faces.size())
	{
		m_pData->m_Faces.clear();
	}

	m_pData->m_bDraw = false;
}

void CloseCaptioning::SetNewCaption(std::string caption)
{
	if(!g_vtRadioLipScale.IsInitted())
		g_vtRadioLipScale.Init(g_pLTClient, "RadioLipScale", NULL, 6.0f);

	m_pData->m_bDraw = true;
	m_pData->m_Caption = caption;

	//clear the savedata
	memset(&m_pData->m_FontData, 0, sizeof(LITHFONTSAVEDATA));
}

// If the caption does not contain info, the start and end should end up
// being npos. this makes style and name = to "" and the calls to make 
// faces and the bute mgr should simply return nothing. The default is 
// used( actually, the last style of drawing)
void CloseCaptioning::SetNewCaption(int stringNumber)
{
	if(stringNumber < 1000) return;

	if(!g_vtRadioLipScale.IsInitted())
		g_vtRadioLipScale.Init(g_pLTClient, "RadioLipScale", NULL, 5.0f);

	RemoveImages();

	m_pData->m_bDraw = true;
	HSTRING hStr = g_pClientDE->FormatString(stringNumber);
	std::string caption = g_pClientDE->GetStringData(hStr);
	g_pClientDE->FreeString(hStr);

	std::string::size_type start, end;
	start = caption.find("<");
	end = caption.find(" ", start);
	++start;

	std::string style = caption.substr(start, end - start);
	start = end;
	++start;
	end = caption.find(">", start);
	m_pData->m_Speaker = caption.substr(start, end - start);

	m_pData->m_Caption = caption.substr(end + 1);

	const LITHFONTDRAWDATA* pDrawData = CloseCaption::g_pButeMgr->GetDrawData(style);
	if (pDrawData)
	{
		m_pData->m_FontDrawData = *pDrawData;
	}

	//clear the savedata
	memset(&m_pData->m_FontData, 0, sizeof(LITHFONTSAVEDATA));

	const CloseCaption::ImageDataList* imageData = CloseCaption::g_pButeMgr->GetSpriteList(style);

	if(imageData)
	{
		CloseCaption::ImageDataList::const_iterator iter = imageData->begin();

		while ( iter != imageData->end())
		{
			AddImage(iter->m_Location, iter->m_FileName, iter->m_Alpha);
			++iter;
		}
	}

	LTIntPt faceLoc = CloseCaption::g_pButeMgr->GetFaceLocation(style);
	float faceAlpha = CloseCaption::g_pButeMgr->GetFaceAlpha(style);
	if(faceAlpha != 0.0f)
	{
		AddFaces(faceLoc, m_pData->m_Speaker, faceAlpha);
	}

	std::string strStartSound = CloseCaption::g_pButeMgr->GetStartSound(style);
	m_pData->m_strStopSound = CloseCaption::g_pButeMgr->GetStartSound(style);

	if( !strStartSound.empty() )
	{
		g_pClientSoundMgr->PlaySoundLocal( strStartSound.c_str() );
	}
}

void CloseCaptioning::ResetDimensions()
{
	// This may end up doing nothing now
}

void CloseCaptioning::SetLips(float average)
{
	average *= g_vtRadioLipScale.GetFloat();

	m_pData->m_MouthPosition = static_cast<int>(average * FACE_COUNT);
	if(m_pData->m_MouthPosition >= FACE_COUNT)
	{
		m_pData->m_MouthPosition = FACE_COUNT - 1;
	}
}

void FreeFont(CaptionData *data)
{
	if(data->m_pFont)
	{
		FreeLithFont(data->m_pFont);
		data->m_pFont = NULL;
	}
}

void LoadImages(CaptionData *data, std::string image, LTIntPt point, float alpha)
{
	HDECOLOR	hTransColor = SETRGB_T(0,0,0);
	LTFLOAT		translucency(alpha);

	//load up surface
	HSURFACE imageSurface = g_pInterfaceResMgr->GetSharedSurface(const_cast<char*>(image.c_str()));

	//set transparency
//	g_pClientDE->OptimizeSurface (imageSurface, hTransColor);
	//set translucency
	g_pClientDE->SetSurfaceAlpha (imageSurface, translucency);

	data->m_Images.push_back(std::make_pair(imageSurface, point));
}

void LoadFaces(CaptionData *data, std::string name, LTIntPt point, float alpha)
{
	HDECOLOR	hTransColor = SETRGB_T(0,0,0);
	LTFLOAT		translucency(alpha);
	std::string image = PRENOM + name;

	for (int i = 0; i < FACE_COUNT; ++i)
	{
		char buff[10];
		std::string loadImage = image + itoa(i, buff, 10);
		loadImage += FILE_TYPE;

		//load up surface
		HSURFACE imageSurface = g_pInterfaceResMgr->GetSharedSurface(const_cast<char*>(loadImage.c_str()));

		//set transparency
//		g_pClientDE->OptimizeSurface (imageSurface, hTransColor);
		//set translucency
		g_pClientDE->SetSurfaceAlpha (imageSurface, translucency);

		data->m_Faces.push_back(std::make_pair(imageSurface, point));
	}
}

void LoadFont(CaptionData *data, std::string font)
{
	LITHFONTCREATESTRUCT	lfCreateStruct;

	//set create struct fixed elements
	lfCreateStruct.nGroupFlags = LTF_INCLUDE_ALL;
	lfCreateStruct.hTransColor = SETRGB_T(0,0,0);
	lfCreateStruct.bChromaKey	= LTTRUE;

	//set name elelment for font
	lfCreateStruct.szFontBitmap = const_cast<char*>(font.c_str());

	//load up the font
	data->m_pFont = CreateLithFont(g_pClientDE, &lfCreateStruct, DTRUE);

	if(!data->m_pFont)
	{
		g_pClientDE->ShutdownWithMessage("ERROR: Could not initialize font in Close Captioning");
	}
}


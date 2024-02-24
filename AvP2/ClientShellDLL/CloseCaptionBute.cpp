// ----------------------------------------------------------------------- //
//
// MODULE  : VisionModeButeMgr.cpp
//
// PURPOSE : VisionModeButeMgr implementation - Controls attributes of all Vision Modes.
//
// CREATED : 12/02/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "CloseCaptionBute.h"

#include "CloseCaptioning.h"

#include "CommonUtilities.h"

#pragma warning (disable : 4503)
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>

// There are a lot of butes so let's wrap this in a namespace
namespace CloseCaption
{
	// Container of the definitions
	struct ButeMGRData
	{
		typedef std::map<std::string, StyleSet>	Styles;
		Styles									m_StyleList;
	};



	const std::string STYLE("Style");

	const std::string STYLE_NAME("StyleName");
	const std::string SPRITE_FILE_NAME("FileName");
	const std::string SPRITE_LOCATION("SpriteLocation");
	const std::string SPRITE_ALPHA("SpriteAlpha");

	const std::string FLAGS("Flags");
	const std::string JUSTIFY("Justify");
	const std::string OFFSET("Offset");
	const std::string FORCE_FORMAT("ForceFormat");

	const std::string FORMAT_WIDTH("FormatWidth");
	const std::string CLIP_RECT("ClipRect");
	const std::string LETTER_SPACE("LetterSpace");
	const std::string LINE_SPACE("LineSpace");
	const std::string COLOR("Color");

	const std::string ALPHA("Alpha");

	const std::string LETTER_DELAY("LetterDelay");
	const std::string LINE_DELAY("LineDelay");
	const std::string SCROLL_TIME("ScrollTime");
	const std::string LINE_WAIT("LineWait");

	const std::string FACE_LOCATION("FaceLocation");
	const std::string FACE_ALPHA("FaceAlpha");

	const std::string START_SOUND("StopSound");
	const std::string STOP_SOUND("StartSound");
}


CloseCaption::ButeMGR* CloseCaption::g_pButeMgr = DNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CloseCaption::ButeMGR::ButeMGR()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CloseCaption::ButeMGR::ButeMGR()
{
	m_pData = new ButeMGRData();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CloseCaption::ButeMGR::~ButeMGR()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //
CloseCaption::ButeMGR::~ButeMGR()
{
	delete m_pData;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CloseCaption::ButeMGR::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //
DBOOL CloseCaption::ButeMGR::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	if (CloseCaption::g_pButeMgr || !szAttributeFile) return DFALSE;
	if (!Parse(pInterface, szAttributeFile)) return DFALSE;

	// Set up global pointer
	CloseCaption::g_pButeMgr = this;


	// This all needs to be done for each Style
	char stylebuff[10];
	int styleNum = 0;
	std::string styleName = STYLE + itoa(styleNum, stylebuff, 10);


	while (m_buteMgr.Exist(styleName.c_str()))
	{
		StyleSet styleSet;

		std::string style = m_buteMgr.GetString(styleName.c_str(), STYLE_NAME.c_str());

		// Get the Sprite names and coordinates
		char buff[10];
		int sprite = 0;
		std::string tagName = SPRITE_FILE_NAME + itoa(sprite, buff, 10);

		while (m_buteMgr.Exist(styleName.c_str(), tagName.c_str()))
		{
			ImageInfo imageInfo;

			// Read the name
			imageInfo.m_FileName = m_buteMgr.GetString(styleName.c_str(), tagName.c_str());

			// Get the point
			tagName = SPRITE_LOCATION + itoa(sprite, buff, 10);
			CPoint point = m_buteMgr.GetPoint(styleName.c_str(), tagName.c_str());
			imageInfo.m_Location = LTIntPt(point.x, point.y);

			// I should also get alpha here
			tagName = SPRITE_ALPHA + itoa(sprite, buff, 10);
			imageInfo.m_Alpha = static_cast<float>(m_buteMgr.GetDouble(styleName.c_str(), tagName.c_str()));

			styleSet.m_Images.push_back(imageInfo);

			++sprite;
			tagName = SPRITE_FILE_NAME + itoa(sprite, buff, 10);
		}

		// Now Read in the draw structure.
//		styleSet.m_DrawData.dwFlags = static_cast<unsigned long>(m_buteMgr.GetInt(styleName.c_str(), FLAGS.c_str()));
		styleSet.m_DrawData.dwFlags = (LTF_DRAW_FORMATTED | LTF_DRAW_TIMED | LTF_USE_OFFSETS | LTF_USE_CLIPRECT | LTF_TIMED_SCROLL | LTF_TIMED_LETTERS | LTF_EXTRA_LOCKLAST);

		// Justification of the text
		styleSet.m_DrawData.byJustify = static_cast<unsigned char>(m_buteMgr.GetInt(styleName.c_str(), JUSTIFY.c_str()));

		// Drawing offsets from the cursor position (if not from cursor, then top-left of surface)
		CPoint point = m_buteMgr.GetPoint(styleName.c_str(), OFFSET.c_str());
		styleSet.m_DrawData.nXOffset = point.x;
		styleSet.m_DrawData.nYOffset = point.y;

		// Force the string to be reformatted, even if the string remains the same
		styleSet.m_DrawData.bForceFormat = static_cast<int>(m_buteMgr.GetInt(styleName.c_str(), FORCE_FORMAT.c_str()));

		// Formatting data
		styleSet.m_DrawData.dwFormatWidth = static_cast<unsigned long>(m_buteMgr.GetInt(styleName.c_str(), FORMAT_WIDTH.c_str()));

		// Get rectangle
		CRect rect = m_buteMgr.GetRect(styleName.c_str(), CLIP_RECT.c_str());
		styleSet.m_DrawData.rcClip.left = rect.left;
		styleSet.m_DrawData.rcClip.right = rect.right;
		styleSet.m_DrawData.rcClip.top = rect.top;
		styleSet.m_DrawData.rcClip.bottom = rect.bottom;

		// Spacing data
		styleSet.m_DrawData.nLetterSpace = m_buteMgr.GetInt(styleName.c_str(), LETTER_SPACE.c_str());
		styleSet.m_DrawData.nLineSpace = m_buteMgr.GetInt(styleName.c_str(), LINE_SPACE.c_str());

		// Color and translucency information
		styleSet.m_DrawData.hColor = 0;
		styleSet.m_DrawData.fAlpha = 1.0f;

		// Timed drawing variables
		styleSet.m_DrawData.fLetterDelay = static_cast<float>(m_buteMgr.GetDouble(styleName.c_str(), LETTER_DELAY.c_str()));
		styleSet.m_DrawData.fLineDelay = static_cast<float>(m_buteMgr.GetDouble(styleName.c_str(), LINE_DELAY.c_str()));
		styleSet.m_DrawData.fLineScrollTime = static_cast<float>(m_buteMgr.GetDouble(styleName.c_str(), SCROLL_TIME.c_str()));
		styleSet.m_DrawData.nNumWaitLines = m_buteMgr.GetInt(styleName.c_str(), LINE_WAIT.c_str());

		// Face stuff
		point = m_buteMgr.GetPoint(styleName.c_str(), FACE_LOCATION.c_str());
		styleSet.m_FaceLocation.x = point.x;
		styleSet.m_FaceLocation.y = point.y;

		styleSet.m_FaceAlpha = static_cast<float>(m_buteMgr.GetDouble(styleName.c_str(), FACE_ALPHA.c_str()));

		styleSet.m_StartSound = m_buteMgr.GetString(styleName.c_str(), START_SOUND.c_str(), "");
		styleSet.m_StopSound  = m_buteMgr.GetString(styleName.c_str(), STOP_SOUND.c_str(), "");

		m_pData->m_StyleList.insert(std::make_pair(style, styleSet));

		++styleNum;
		styleName = STYLE + itoa(styleNum, buff, 10);
	}
	return DTRUE;
}

const CloseCaption::ImageDataList* CloseCaption::ButeMGR::GetSpriteList(const std::string& styleName)
{
	if(m_pData->m_StyleList.find(styleName) != m_pData->m_StyleList.end())
	{
		return &m_pData->m_StyleList[styleName].m_Images;
	}
	return 0;
}

const LITHFONTDRAWDATA* CloseCaption::ButeMGR::GetDrawData(const std::string& styleName)
{
	if(m_pData->m_StyleList.find(styleName) != m_pData->m_StyleList.end())
	{
		return &m_pData->m_StyleList[styleName].m_DrawData;
	}
	return 0;
}

LTIntPt CloseCaption::ButeMGR::GetFaceLocation(std::string styleName)
{
	if(m_pData->m_StyleList.find(styleName) != m_pData->m_StyleList.end())
	{
		return m_pData->m_StyleList[styleName].m_FaceLocation;
	}
	return LTIntPt(0,0);
}

float CloseCaption::ButeMGR::GetFaceAlpha(std::string styleName)
{
	if(m_pData->m_StyleList.find(styleName) != m_pData->m_StyleList.end())
	{
		return m_pData->m_StyleList[styleName].m_FaceAlpha;
	}
	return 0.0f;
}

std::string CloseCaption::ButeMGR::GetStartSound(const std::string & styleName)
{
	if(m_pData->m_StyleList.find(styleName) != m_pData->m_StyleList.end())
	{
		return m_pData->m_StyleList[styleName].m_StartSound;
	}
	return std::string();
}

std::string CloseCaption::ButeMGR::GetStopSound(const std::string & styleName)
{
	if(m_pData->m_StyleList.find(styleName) != m_pData->m_StyleList.end())
	{
		return m_pData->m_StyleList[styleName].m_StopSound;
	}

	return std::string();
}

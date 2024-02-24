// ----------------------------------------------------------------------- //
//
// MODULE  : CloseCaptionBute.h
//
// PURPOSE : CloseCaptionBute definition
//
// CREATED : 05.22.1999
//
// ----------------------------------------------------------------------- //

#ifndef CLOSE_CAPTION_BUTE_H
#define CLOSE_CAPTION_BUTE_H

// Includes

#include "GameButeMgr.h"
#include "ClientServerShared.h"
#include "CloseCaptioning.h"
#include "LithFontDefs.h"

#pragma warning (disable : 4503)
#include <vector>
#include <string>
#include <map>

// There are a lot of butes so let's wrap this in a namespace
namespace CloseCaption
{
		// Structure for image info
	struct ImageInfo
	{
		std::string								m_FileName;
		LTIntPt									m_Location;
		float									m_Alpha;
	};
	typedef std::vector<ImageInfo>				ImageDataList;

	// Complete style definition
	struct StyleSet
	{
		ImageDataList							m_Images;
		LITHFONTDRAWDATA						m_DrawData;

		LTIntPt									m_FaceLocation;
		float									m_FaceAlpha;

		std::string								m_StartSound;
		std::string								m_StopSound;
	};

	// Pre define the classes we are going to refer to.
	struct	ButeMGRData;
	class	ButeMGR;

	// This should be a singleton, but all the others are done this way
	extern ButeMGR* g_pButeMgr;

	// This is the main class.
	class ButeMGR : public CGameButeMgr
	{
	public:
		ButeMGR();
		virtual ~ButeMGR();

		// Init the class. This might end up being a constructor and remove the default.
		// I have to see where it will be created and inited.
		DBOOL		Init(ILTCSBase *pInterface, const char* szAttributeFile="Attributes\\CloseCaptionButes.txt");

		const ImageDataList* GetSpriteList(const std::string& styleName);
		const LITHFONTDRAWDATA* GetDrawData(const std::string& styleName);

		LTIntPt GetFaceLocation(std::string styleName);
		float GetFaceAlpha(std::string styleName);

		std::string GetStartSound(const std::string& styleName);
		std::string GetStopSound(const std::string& styleName);

	private:
		ButeMGRData*	m_pData;
	};
}

#endif // CLOSE_CAPTION_BUTE_H


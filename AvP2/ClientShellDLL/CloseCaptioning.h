// ----------------------------------------------------------------------- //
//
// MODULE  : MotionDetector.h
//
// PURPOSE : MotionDetector definition
//
// CREATED : 06.06.2000
//
// ----------------------------------------------------------------------- //

#ifndef CLOSE_CAPTIONING_H
#define CLOSE_CAPTIONING_H

//external dependencies
#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef	_LITHTECH_FONT_MGR_H_
#include "LithFontMgr.h"
#endif

#pragma warning (disable : 4786)
#include <string>
#include <vector>
#include <utility>

// Forward declare the data object
struct CaptionData;

class CloseCaptioning
{
public:

	typedef std::vector<std::pair<HSURFACE, LTIntPt> >	ImageList;

	// constructor / destructor
	CloseCaptioning			();
	~CloseCaptioning		();

	// Drawing functions
	void Draw				();

	// Flap yo' gums
	void SetLips			(float average);		// this is between 0 - 1

	// Setup functions
	void SetDims			(LTRect rect);
	void SetFont			(std::string font);
	void AddImage			(LTIntPt point, std::string image, float alpha = 1.0f);
	void AddFaces			(LTIntPt point, std::string name, float alpha = 1.0f);
	void RemoveImages		();
	void SetNewCaption		(std::string caption = std::string(""));
	void SetNewCaption		(int stringNumber);
	void StopCaptions		();
	void ResetDimensions	();											// This is called when screen dims change

private:

	CaptionData*	m_pData;
};

#endif		// CLOSE_CAPTIONING_H

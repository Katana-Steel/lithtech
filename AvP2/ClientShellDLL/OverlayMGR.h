// ----------------------------------------------------------------------- //
//
// MODULE  : OverlayMGR.cpp
//
// PURPOSE : Based on the mode sent down by the viewmodeMGR, change the 
//			 overlay. Also responsible for animating the overlay.
//
// CREATED : 06/08/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef OVERLAY_MGR_H
#define OVERLAY_MGR_H

#pragma warning (disable : 4503)
#include <string>

struct OverlayData;

class OverlayMGR
{
	OverlayData*	m_pData;
	LTBOOL			m_bHidden;

public:
	OverlayMGR();
	~OverlayMGR();

	void	Save(HMESSAGEWRITE hWrite);
	void	Load(HMESSAGEREAD hRead);

	void SetMode(const std::string& mode);
	void Update(LTBOOL bForceUpdate=FALSE);

	void SetNetMode(LTBOOL bOn);
	void SetRailMode(LTBOOL bOn);
	LTBOOL GetRailMode();
	void SetPredZoomMode(LTBOOL bOn);
	void SetOnFireMode(LTBOOL bOn);

	void SetEvacMode(LTBOOL bOn);
	void SetEvacRotation(LTRotation rRot);
	void SetEvacAlpha(LTFLOAT fAlpha);

	void SetOrientMode(LTBOOL bOn);
	void SetOrientAngle(LTFLOAT fAngle);

	void SetNormalOverlayVisible(LTBOOL bVis);
};

#endif		// OVERLAY_MGR_H

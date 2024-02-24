// ----------------------------------------------------------------------- //
//
// MODULE  : CrosshairMgr.h
//
// PURPOSE : Definition of CrosshairMgr class
//
// CREATED : 8/28/00
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
 
#ifndef __CROSSHAIRMGR_H
#define __CROSSHAIRMGR_H

#include "basedefs_de.h"

//activate crosshair modes
enum XHairMode
{
	XHM_TARGETING = 0,
	XHM_DEFAULT_ACTIVATE,
	XHM_MARINE_WELD,
	XHM_MARINE_CUT,
	XHM_MARINE_HACK,
	XHM_PRED_DETONATE,
};

class CCrosshairMgr
{
public:

	CCrosshairMgr();
	~CCrosshairMgr();

	LTBOOL		Init();
	void		Term();

	void		Save(HMESSAGEWRITE hWrite);
	void		Load(HMESSAGEREAD hRead);

	void		ToggleCrosshairOverride(){ m_bCrosshairOverride = !m_bCrosshairOverride; }
	void		ToggleCrosshair(){ m_bCrosshairEnabled = !m_bCrosshairEnabled; }
	void		EnableCrosshair(LTBOOL b=LTTRUE);
	LTBOOL		CrosshairEnabled() const { return m_bCrosshairEnabled; }
	LTBOOL		CrosshairOn() const { return m_bCrosshairEnabled; }

	void		DrawCrosshair();

	void		UpdateCrosshairColors(LTFLOAT fRed = 1.0f, LTFLOAT fGreen = 1.0f, LTFLOAT fBlue = 1.0f, LTFLOAT fAlpha = 1.0f);
	void		SetCrosshairColors(LTFLOAT fRed, LTFLOAT fGreen, LTFLOAT fBlue, LTFLOAT fAlpha);
	void		SetCustomCHPosOn(LTBOOL bOn)	{ m_bCustomCrosshairPos = bOn; }
	void		SetCustomCHPos(LTIntPt pt)		{ m_ptCHPos = pt; }
	void		LoadCrosshair(char* szImage, LTFLOAT fScale, LTFLOAT fAlpha=1.0f);
	void		SetActivateCrosshairOn(LTBOOL bOn) { m_bUseActivate=bOn; }
	void		SetActivateMode(XHairMode eMode);

private:

	// private member functions
	void		SetActivateCrosshair(XHairMode eMode);
	void		DrawTargetingCrosshair();
	void		DrawActivateCrosshair();

	// common members
	LTBOOL		m_bCrosshairEnabled;
	LTBOOL		m_bCrosshairOverride;
	LTBOOL		m_bUseActivate;
	
	// activate crosshair members
	HSURFACE	m_hActivateCrosshair;
	uint32		m_nHalfActXhairWidth;
	uint32		m_nHalfActXhairHeight;
	XHairMode	m_eActMode;
	LTFLOAT		m_fActScale;

	// targeting crosshair members
	HSURFACE	m_hCrosshair;
	LTIntPt		m_ptCHPos;
	LTBOOL		m_bCustomCrosshairPos;
	uint32		m_nHalfXhairWidth;
	uint32		m_nHalfXhairHeight;
	LTFLOAT		m_fScale;
	HLTCOLOR	m_hCursorColor;
	LTBOOL		m_bDrawSolid;
};

#endif

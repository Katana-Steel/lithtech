// ----------------------------------------------------------------------- //
//
// MODULE  : MotionDetector.h
//
// PURPOSE : MotionDetector definition
//
// CREATED : 06.06.2000
//
// ----------------------------------------------------------------------- //

#ifndef __MOTION_DETECTOR_H
#define __MOTION_DETECTOR_H

//external dependencies
#include "LithFontMgr.h"

//prototypes
class CHudElement;
class CHudMgr;
class CPlayerStats;

//constants
const DDWORD MAX_BLIPS = 40;

struct MDBlip
{
	HOBJECT		hObj;
	LTVector	vLoc;
};

struct MDDispBlip
{
	HOBJECT		hObj;
	LTVector	vLoc;
	DIntPt		pt;
};

struct MDDispBlipList
{
	int			nMinRange;
	DDWORD		nNumBlips;
	DIntPt		ptOffset;
	MDDispBlip	BlipDispArray[MAX_BLIPS];
};


class CMotionDetector
{
public:
	//constructor / destructor
	CMotionDetector		();
	~CMotionDetector	() { Term(); }

	//utility functions
	DBOOL Init			();
	void Term			();
	void Draw			(LTBOOL bEMPEffect);

	//misc support functions for drawing and update
	HSURFACE	GetMDAnimFrame		();
	void		DisplayMDRange		(DBOOL bActive, DFLOAT fTotAnimTime, DDWORD nRange, LTBOOL bEMPEffect);
	DRect		GetMDAnimDestRect	(LTBOOL bEMPEffect);
	void		OnObjectMove		(HOBJECT hObj, DBOOL bTeleport, DVector pPos);
	void		OnObjectRemove		(HLOCALOBJ hObj);
	void		SetTrackerObject	(LTVector vPos){ m_vTrackerPos = vPos; m_vTrackerPos.y = 0; m_bTrackerEnabled = LTTRUE; }
	void		RemoveTracker		(){ m_bTrackerEnabled = LTFALSE; }

private:

	CPlayerStats*	m_pPlayerStats;		//pointer to player stats class
	MDBlip			m_pBlipArray[MAX_BLIPS]; //array of motion detector blips
	MDDispBlipList	m_BlipDispList;	//array of motion detector blips for display

	//private utility functions
	LTBOOL		TestObjectForType	(HOBJECT hObj, LTBOOL bKeyFramed);
	LTBOOL		TestObjectForLocation(HOBJECT hObj, DVector vPos);//, DFLOAT *pRval, DDWORD *pRange);
	void		AddObjectToList		(HOBJECT hObj, LTVector vLoc);//DFLOAT fAngle, DDWORD nRange, 
	void		RemoveObjectFromList(HOBJECT hObj);
	void		ResetDisplayList	();
	void		AddToDispList		(MDBlip nBlip);
	LTVector	GetObjectPos		(HOBJECT hObj);
	void		BuildDisplayList	();
	DBOOL		UpdateDispBlipList(int nIndex, LTVector vPlayerPos, LTVector vPlayerForward, LTVector vPlayerRight);
	HSURFACE	UpdateTracker(LTVector vPlayerPos, LTVector vPlayerForward, LTVector vPlayerRight);

	void		DrawMDFrame	(LTBOOL bEMPEffect);
	void		DrawMDBase	();
	void		LoadFonts();
	void		LoadImages();
	void		SetOffsets();

	LithFont*	m_pSmallNums;	//pointer to small number font
	LithFont*	m_pLargeNums;	//pointer to large number font
	HSURFACE*	m_ahBlipSurf;	//array of blip image surfaces
	HSURFACE*	m_ahGridSurf;	//array of MD grid surfaces
	HSURFACE	m_hArcSurf;		//arc image	
	HSURFACE	m_hBaseSurf;	//MD base image 
	HSURFACE	m_hTrackerBlip; //the MD Tracker blip image
	HSURFACE	m_hTrackerArrow;//the MD Tracker arrow
	LTVector	m_vTrackerPos;	//position of the tracker object
	DBOOL		m_bTrackerEnabled; //is the tracker enabled
	DIntPt		m_ptTracker;	//where the tracker blip should be displayed
	DIntPt		m_ptGridDest;	//destination top left corner of MD grid
	DIntPt		m_ptGridOffset;	//where this element is displayed
	DIntPt		m_ptBaseDest;	//destination top left corner of MD grid
	DIntPt		m_ptBaseOffset;	//where this element is displayed
	DFLOAT		m_fGridTrans;	//amount of translucency on the grid
	DFLOAT		m_fBaseTrans;	//amount of translucency on the grid
	DFLOAT		m_fArcTrans;	//amount of translucency on the grid
	DDWORD		m_nGridSurfaces;//number of grid surfaces
	DDWORD		m_nBlipSurfaces;//number of blip surfaces
	DDWORD		m_nBlipIndex;	//index into array of blips
	LTBOOL		m_bFirstUpdate;	//first up date flag
};

#endif

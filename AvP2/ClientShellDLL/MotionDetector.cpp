// ----------------------------------------------------------------------- //
//
// MODULE  : MotionDetector.cpp
//
// PURPOSE : MotionDetector implimentation
//
// CREATED : 06.06.2000
//
// ----------------------------------------------------------------------- //

//external dependencies
#include "stdafx.h"
#include "MotionDetector.h"
#include "LayoutMgr.h"
#include "GameClientShell.h"
#include "SoundMgr.h"
#include "vartrack.h"
#include "PlayerStats.h"

//external globals
extern CLayoutMgr* g_pLayoutMgr;
extern CGameClientShell* g_pGameClientShell;
extern CSoundMgr* g_pSoundMgr;

//console variable trackers
static VarTrack		g_vtMDSound;

//constants
const uint32 MAX_MD_RANGE			= 1920;
const uint32 MAX_MD_RANGE_SQ		= MAX_MD_RANGE * MAX_MD_RANGE;
const uint32 MAX_MD_TRACKER_RANGE	= MAX_MD_RANGE;

// -----------------------------------------------/
//
// void CMotionDetector::CMotionDetector Implimentation
//
// Constructor
//
// -----------------------------------------------/
CMotionDetector::CMotionDetector()
{
	m_pSmallNums		= LTNULL;
	m_pLargeNums		= LTNULL;
	m_pPlayerStats		= LTNULL;
	m_ahBlipSurf		= LTNULL;
	m_ahGridSurf		= LTNULL;
	m_hArcSurf			= LTNULL;
	m_hBaseSurf			= LTNULL;
	m_nBlipIndex		= 0;
	m_hTrackerBlip		= LTNULL;
	m_hTrackerArrow		= LTNULL;
	m_bTrackerEnabled	= LTFALSE;
	m_bFirstUpdate		= LTFALSE;
	
	memset(m_pBlipArray,0, MAX_BLIPS * sizeof(MDBlip));
	memset(&m_BlipDispList,0, sizeof(MDDispBlipList));
}

// -----------------------------------------------/
//
// void CMotionDetector::Init Implimentation
//
// Sets member variables to initial states
//
// -----------------------------------------------/
LTBOOL CMotionDetector::Init()
{
	//sanity check
	if(!g_pGameClientShell || !g_pLayoutMgr) return LTFALSE;
	
	//get pointer to player stats
	m_pPlayerStats = g_pGameClientShell->GetPlayerStats();
	
	//test
	if(!m_pPlayerStats)
		return LTFALSE;

	//load up font resources
	LoadFonts();

	//load up the bitmap images
	LoadImages();

	//setup the console variables
	if(!g_vtMDSound.IsInitted())
		g_vtMDSound.Init(g_pLTClient, "MDSound", LTNULL, 1.0f);

	return LTTRUE;
}

// -----------------------------------------------/
//
// void CMotionDetector::LoadImages Implimentation
//
// Loads the bitmap images
//
// -----------------------------------------------/
void CMotionDetector::LoadImages()
{
	HLTCOLOR	hTransColor = SETRGB_T(0,0,0);
	char		s_aSurfaceName[60];

	m_nBlipSurfaces = g_pLayoutMgr->GetNumMDBlipImages();	
	m_nGridSurfaces = g_pLayoutMgr->GetNumMDGridImages();	

	//set image offset
	m_ptGridOffset	= g_pLayoutMgr->GetMDGridOffset();	
	m_ptBaseOffset	= g_pLayoutMgr->GetMDBaseOffset();	

	//get translucency value
	m_fGridTrans	= g_pLayoutMgr->GetMDGridTranslucency();
	m_fBaseTrans	= g_pLayoutMgr->GetMDBaseTranslucency();
	m_fArcTrans		= g_pLayoutMgr->GetMDArcTranslucency();

	//get surface path and name
	g_pLayoutMgr->GetArcSurfaceName(s_aSurfaceName, 60);
	//load up surface
	m_hArcSurf = g_pInterfaceResMgr->GetSharedSurface(s_aSurfaceName);
	//set translucency
	g_pClientDE->SetSurfaceAlpha (m_hArcSurf, m_fArcTrans);

	//get surface path and name
	g_pLayoutMgr->GetBaseSurfaceName(s_aSurfaceName, 60);
	//load up surface
	m_hBaseSurf = g_pInterfaceResMgr->GetSharedSurface(s_aSurfaceName);
	//set translucency
	g_pClientDE->SetSurfaceAlpha (m_hBaseSurf, m_fBaseTrans);

	//create the array
	m_ahGridSurf = new HSURFACE[m_nGridSurfaces];
	if(!m_ahGridSurf) return;
	//creat my name
	g_pLayoutMgr->GetGridSurfaceName(s_aSurfaceName, 60);

	for(uint32 i=0 ; i<m_nGridSurfaces ; i++)
	{
		char	aTemp[60];
		//parse the name
		sprintf(aTemp,"%s%d%s",s_aSurfaceName,i,".pcx");
		//create surface
		m_ahGridSurf[i] = g_pInterfaceResMgr->GetSharedSurface(aTemp);
		//set translucency
		g_pClientDE->SetSurfaceAlpha (m_ahGridSurf[i], m_fGridTrans);
	}


	//create the array
	m_ahBlipSurf = new HSURFACE[m_nBlipSurfaces];
	if(!m_ahBlipSurf) return;
	//creat my name
	g_pLayoutMgr->GetBlipSurfaceName(s_aSurfaceName, 60);

	for(i=0 ; i<m_nBlipSurfaces ; i++)
	{
		char	aTemp[60];
		//parse the name
		sprintf(aTemp,"%s%d%s",s_aSurfaceName,i,".pcx");
		//create surface
		m_ahBlipSurf[i] = g_pInterfaceResMgr->GetSharedSurface(aTemp);
	}

	//get surface path and name
	g_pLayoutMgr->GetTrackerBlipSurfaceName(s_aSurfaceName, 60);
	//load up surface
	m_hTrackerBlip = g_pInterfaceResMgr->GetSharedSurface(s_aSurfaceName);
	//get surface path and name
	g_pLayoutMgr->GetTrackerArrowSurfaceName(s_aSurfaceName, 60);
	//load up surface
	m_hTrackerArrow = g_pInterfaceResMgr->GetSharedSurface(s_aSurfaceName);

	SetOffsets();
}

// -----------------------------------------------/
//
// void CMotionDetector::SetOffsets Implimentation
//
// Routine that sets various rectangles for drawing
//
// -----------------------------------------------/
void CMotionDetector::SetOffsets()
{
	//local variables
	uint32		nWidth, nHeight, nScreenWidth, nScreenHeight;

	//get the dims of the screen
	g_pClientDE->GetSurfaceDims(g_pClientDE->GetScreenSurface(),
								&nScreenWidth, &nScreenHeight);

	//get the dims of our surface
	g_pClientDE->GetSurfaceDims(m_ahGridSurf[0], &nWidth, &nHeight);

	//set the dest point (neg offsets are from right and bottom)
	if(m_ptGridOffset.x > 0) m_ptGridDest.x	= m_ptGridOffset.x;
	else m_ptGridDest.x	= nScreenWidth + m_ptGridOffset.x;

	if(m_ptGridOffset.y > 0) m_ptGridDest.y	= m_ptGridOffset.y;
	else m_ptGridDest.y	= nScreenHeight + m_ptGridOffset.y;

	//get the dims of our surface
	g_pClientDE->GetSurfaceDims(m_hBaseSurf, &nWidth, &nHeight);

	//set the dest point (neg offsets are from right and bottom)
	if(m_ptBaseOffset.x > 0) m_ptBaseDest.x	= m_ptBaseOffset.x;
	else m_ptBaseDest.x	= nScreenWidth + m_ptBaseOffset.x;

	if(m_ptBaseOffset.y > 0) m_ptBaseDest.y	= m_ptBaseOffset.y;
	else m_ptBaseDest.y	= nScreenHeight + m_ptBaseOffset.y;
}

// -----------------------------------------------/
//
// void CMotionDetector::LoadFonts Implimentation
//
// Loads the numeric display fonts
//
// -----------------------------------------------/
void CMotionDetector::LoadFonts()
{
	LITHFONTCREATESTRUCT	lfCreateStruct;
	char					s_aSurfaceName[60];

	//set create struct fixed elements
	lfCreateStruct.nGroupFlags = LTF_INCLUDE_NUMBERS;
	lfCreateStruct.hTransColor = SETRGB_T(0,0,0);
	lfCreateStruct.bChromaKey	= LTTRUE;

	//get surface path and name
	g_pLayoutMgr->GetMDSmallFontName(s_aSurfaceName, 60);

	//set name elelment for font
	lfCreateStruct.szFontBitmap = s_aSurfaceName;

	//load up the font
	m_pSmallNums = CreateLithFont(g_pClientDE, &lfCreateStruct, LTTRUE);

	//get surface path and name
	g_pLayoutMgr->GetMDLargeFontName(s_aSurfaceName, 60);

	//set name elelment for font
	lfCreateStruct.szFontBitmap = s_aSurfaceName;

	//load up the font
	m_pLargeNums = CreateLithFont(g_pClientDE, &lfCreateStruct, LTTRUE);
}

// -----------------------------------------------/
//
// void CMotionDetector::Term Implimentation
//
// Clen-up routine
//
// -----------------------------------------------/
void CMotionDetector::Term()
{
	//call termination function for all fonts
	if(m_pLargeNums)
		FreeLithFont(m_pLargeNums);

	if(m_pSmallNums)
		FreeLithFont(m_pSmallNums);

	if(m_ahBlipSurf)
	{
//		for(int i=0; i<m_nBlipSurfaces; i++)
//			g_pClientDE->DeleteSurface(m_ahBlipSurf[i]);

		delete [] m_ahBlipSurf;
		m_ahBlipSurf = LTNULL;
	}

	if(m_ahGridSurf)
	{
//		for(int i=0; i<m_nGridSurfaces; i++)
//			g_pClientDE->DeleteSurface(m_ahGridSurf[i]);

		delete [] m_ahGridSurf;
		m_ahGridSurf = LTNULL;
	}

//	if(m_hArcSurf)
//	{
//		g_pClientDE->DeleteSurface(m_hArcSurf);
//		m_hArcSurf = LTNULL;
//	}

//	if(m_hBaseSurf)
//	{
//		g_pClientDE->DeleteSurface(m_hBaseSurf);
//		m_hBaseSurf = LTNULL;
//	}

//	if(m_hTrackerBlip)
//	{
//		g_pClientDE->DeleteSurface(m_hTrackerBlip);
//		m_hTrackerBlip = LTNULL;
//	}

//	if(m_hTrackerArrow)
//	{
//		g_pClientDE->DeleteSurface(m_hTrackerArrow);
//		m_hTrackerArrow = LTNULL;
//	}
}

// -----------------------------------------------/
//
// void CMotionDetector::Term Implimentation
//
// Drawing routine
//
// -----------------------------------------------/
void CMotionDetector::Draw(LTBOOL bEMPEffect)
{
	DrawMDBase	();
	DrawMDFrame	(bEMPEffect);
}

// -----------------------------------------------/
//
// void CMotionDetector::DrawMDFrame Implimentation
//
// Motion Detector Frame drawing routine
//
// -----------------------------------------------/
void CMotionDetector::DrawMDFrame	(LTBOOL bEMPEffect)
{

	//sanity check
	if ( !g_pClientDE  ) return;

	if(m_ahGridSurf)
	{
		DRotation		rRot;
		DFLOAT			fAngle;
		uint32			nIndex;
		DVector			vWForward( 0.0, 0.0, 1.0 ), vWRight( 1.0, 0.0, 0.0 );

		//get handle to screen surface
		HSURFACE hScreen = g_pClientDE->GetScreenSurface();

		//set transparent color
		HLTCOLOR hTransColor = SETRGB_T(0,0,0);

		//get player object
		HLOCALOBJ hPlayerObj = g_pClientDE->GetClientObject();

		//direction vectors
		DVector vRight, vUp, vForward;

		//get my rotation
		g_pClientDE->GetObjectRotation(hPlayerObj, &rRot);

		//calculate my vectors
		g_pMathLT->GetRotationVectors (rRot, vRight, vUp, vForward);

		//get player position
		LTVector vPlayerPos;
		g_pClientDE->GetObjectPos(hPlayerObj, &vPlayerPos);

		if(!bEMPEffect)
		{
			//adjust to 2d
			vPlayerPos.y = 0;

			//get cos of my angle from world forward and convert to rads
			fAngle = vWForward.Dot(vForward);

			// Bounds check
			fAngle = fAngle < -1.0f ? -1.0f : fAngle;
			fAngle = fAngle > 1.0f ? 1.0f : fAngle;

			fAngle = (float) acos(vWForward.Dot(vForward));

			//reverse the angle if we are beyond 180
			fAngle = vWRight.Dot(vForward) > 0 ? MATH_PI-fAngle : fAngle ;

			//if angle is beyond 90, subtract 90
			fAngle -= fAngle > MATH_HALFPI ? MATH_HALFPI : 0;

			//convert angle into our index number
			nIndex = (uint32) MATH_RADIANS_TO_DEGREES(fAngle);

			//only 46 frames of animation for 90 degrees, so divide
			nIndex >>= 1;
		}
		else
		{
			nIndex = GetRandom(0, 45);
		}
		//update our surface
		g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen,
														m_ahGridSurf[nIndex],
														LTNULL,
														m_ptGridDest.x, 
														m_ptGridDest.y, 
														hTransColor);

		//display the blips if any
		if(!bEMPEffect)
		{
			for(uint32 i=0; i<m_BlipDispList.nNumBlips; i++)
			{
				if(UpdateDispBlipList(i, vPlayerPos, vForward, vRight))
					g_pClientDE->DrawSurfaceToSurfaceTransparent
						(	hScreen,
							m_ahBlipSurf[m_nBlipIndex],
							LTNULL,
							m_ptGridDest.x + m_BlipDispList.ptOffset.x + m_BlipDispList.BlipDispArray[i].pt.x, 
							m_ptGridDest.y + m_BlipDispList.ptOffset.y - m_BlipDispList.BlipDispArray[i].pt.y, 
							hTransColor);
			}

			if(m_bFirstUpdate)
			{
				//play blip sound
				m_bFirstUpdate = LTFALSE;

				if(m_BlipDispList.nMinRange != MAX_MD_RANGE)
					g_pClientSoundMgr->PlaySoundLocal(
								"\\Sounds\\Interface\\md_tracking.wav",
								SOUNDPRIORITY_MISC_LOW,
								PLAYSOUND_CTRL_PITCH,
								(uint8)(30.0f + (1.0f - (LTFLOAT)m_BlipDispList.nMinRange / MAX_MD_RANGE) * 70.0f),
								1.0f + (1.0f - (LTFLOAT)m_BlipDispList.nMinRange / MAX_MD_RANGE) * 0.05f);
			}

			//dispaly the tracker blip or arrow
			if(m_bTrackerEnabled)
			{
				HSURFACE hSurf = UpdateTracker(vPlayerPos, vForward, vRight);
					g_pClientDE->DrawSurfaceToSurfaceTransparent
						(	hScreen,
							hSurf,
							LTNULL,
							m_ptGridDest.x + m_BlipDispList.ptOffset.x + m_ptTracker.x, 
							m_ptGridDest.y + m_BlipDispList.ptOffset.y - m_ptTracker.y, 
							hTransColor);
			}
		}

		//draw the animation arc to the screen
		g_pClientDE->ScaleSurfaceToSurfaceTransparent(	hScreen,
														m_hArcSurf,
														&GetMDAnimDestRect(bEMPEffect),
														LTNULL,
														hTransColor);
	}
}

// -----------------------------------------------/
//
// void CMotionDetector::DrawMDBase Implimentation
//
// Motion Detector base drawing routine
//
// -----------------------------------------------/
void CMotionDetector::DrawMDBase	()
{
	//sanity check
	if ( !g_pClientDE  ) return;

	//get handle to screen surface
	HSURFACE hScreen = g_pClientDE->GetScreenSurface();

	//set transparent color
	HLTCOLOR hTransColor = SETRGB_T(0,0,0);

	//draw our element to the screen
	g_pClientDE->DrawSurfaceToSurfaceTransparent(	hScreen, 
													m_hBaseSurf, 
													LTNULL,
													m_ptBaseDest.x, 
													m_ptBaseDest.y, 
													hTransColor);
}

// -----------------------------------------------/
//
// void CMotionDetector::DisplayMDRange Implimentation
//
// Displays the numeric range on the motion detector
//
// -----------------------------------------------/
void CMotionDetector::DisplayMDRange	(LTBOOL bActive, DFLOAT fTotAnimTime, uint32 nRange, LTBOOL bEMPEffect)
{
	static char aBigNums[3];
	static char aLilNums[3];
	static LTBOOL bRunning = LTFALSE;
	static DFLOAT	fStartTime = g_pClientDE->GetTime();
	static DFLOAT	fTemp = 0.5f;
	DFLOAT			fTimeDelta = g_pClientDE->GetTime() - fStartTime;
	uint32			nCount;
	LITHFONTDRAWDATA lfDDLg, lfDDSm;

	//get handle to screen surface
	HSURFACE hScreen = g_pClientDE->GetScreenSurface();

	//set drawdata elements
	lfDDLg.byJustify		= LTF_JUSTIFY_LEFT;
	lfDDLg.nLetterSpace		= 2;
	lfDDLg.bForceFormat		= LTTRUE;
	lfDDSm.byJustify		= LTF_JUSTIFY_LEFT;
	lfDDSm.nLetterSpace		= 1;
	lfDDSm.bForceFormat		= LTTRUE;

	if(bActive)
	{
		if(bRunning)
		{
			if(nRange == MAX_MD_RANGE)
			{
				//continuation of sequence
				nCount = (uint32) (fTimeDelta / fTotAnimTime * 3000);

			}
			else
			{
				nCount = (uint32) ( (DFLOAT)nRange / MAX_MD_RANGE * 3000);
			}

			uint32 nBigNum = nCount / 100;
			uint32 nLilNum = nCount % 100;

			sprintf(aBigNums,"%02d",nBigNum);
			sprintf(aLilNums,"%02d",nLilNum);

		}
		else
		{
			//start of a new sequence
			bRunning = LTTRUE;
			fStartTime = g_pClientDE->GetTime();
			nCount = 0;
		}
	}
	else
	{
		//set flag to false
		bRunning = LTFALSE;

		if(nRange != MAX_MD_RANGE)
		{
			nCount = (uint32) ( (DFLOAT)nRange / MAX_MD_RANGE * 3000);

			uint32 nBigNum = nCount / 100;
			uint32 nLilNum = nCount % 100;

			sprintf(aBigNums,"%02d",nBigNum);
			sprintf(aLilNums,"%02d",nLilNum);
		}
		else
		{
			//set strings
			sprintf(aBigNums,"%s","00");
			sprintf(aLilNums,"%s","00");
		}
	}

	if(bEMPEffect)
	{
		//set strings
		sprintf(aBigNums,"88");
		sprintf(aLilNums,"88");
	}


	//draw large text to the screen
	m_pLargeNums->Draw(	hScreen,
							aBigNums,
							&lfDDLg, 
							m_ptGridDest.x+75, 
							m_ptGridDest.y+90, 
							LTNULL);

	//draw small text to the screen
	m_pSmallNums->Draw(	hScreen,
							aLilNums,
							&lfDDSm, 
							m_ptGridDest.x+106, 
							m_ptGridDest.y+90, 
							LTNULL);
}

// -----------------------------------------------/
//
// void CMotionDetector::GetMDAnimDestRect Implimentation
//
// Calculates destination for MD animation
//
// -----------------------------------------------/
DRect CMotionDetector::GetMDAnimDestRect (LTBOOL bEMPEffect)
{
	static DFLOAT	fStartTime = g_pClientDE->GetTime();
	DFLOAT			fTimeDelta = g_pClientDE->GetTime() - fStartTime;
	DFLOAT			fSeqDelay = 0.5f;
	static LTBOOL	bSeqDelay = LTTRUE;
	static DFLOAT	fTotAnimTime = 0.4f;
	DRect			rcRval;
	static uint32	nWidth		= 195;
	uint32 nHalfWidth	= nWidth>>1;
	static uint32	nHeight	= 98;
	static DFLOAT	fSeqStartTime = g_pClientDE->GetTime();
	static DFLOAT	fTotSeqTime = fTotAnimTime + fSeqDelay;
	DFLOAT			fSeqTimeDelta = g_pClientDE->GetTime() - fSeqStartTime;

	m_nBlipIndex = (uint32)((m_nBlipSurfaces-1)*(1-fSeqTimeDelta/fTotSeqTime));
	m_nBlipIndex = m_nBlipIndex <= m_nBlipSurfaces ? m_nBlipIndex : m_nBlipSurfaces-1;
	m_nBlipIndex = m_nBlipIndex > 0 ? m_nBlipIndex : 0;

	//display numeric range
	DisplayMDRange(!bSeqDelay, fTotAnimTime, m_BlipDispList.nMinRange, bEMPEffect);

	//return null for first sequence delay
	if(bSeqDelay)
	{
		if(fTimeDelta > fSeqDelay)
		{
			//ok waited long enuf, start the animation
			bSeqDelay = LTFALSE;

			//reset the timer
			fStartTime = fSeqStartTime = g_pClientDE->GetTime();

			//play the sound
			if(g_vtMDSound.GetFloat())
				g_pClientSoundMgr->PlaySoundLocal("\\Sounds\\Interface\\md_scan.wav", SOUNDPRIORITY_MISC_LOW, 0, 100, 1.0f);

			BuildDisplayList();
		}
		//set dest rect
		rcRval.left		= m_ptGridDest.x + nHalfWidth;
		rcRval.top		= m_ptGridDest.y + nHeight;
		rcRval.right	= rcRval.left;
		rcRval.bottom	= rcRval.top;

		//return the dest rect
		return rcRval;
	}
	
	//rect ratio
	LTFLOAT	fRatio;

	if(!bEMPEffect)
	{
		if(fTimeDelta < fTotAnimTime)
		{
			fRatio = fTimeDelta/fTotAnimTime;

			//set dest rect
			rcRval.left		= m_ptGridDest.x + (int)(nHalfWidth * (1-fRatio));
			rcRval.top		= m_ptGridDest.y + (int)(nHeight * (1-fRatio));
			rcRval.right	= m_ptGridDest.x + nWidth - (int)(nHalfWidth * (1-fRatio));
			rcRval.bottom	= m_ptGridDest.y + (int)nHeight;
		}
		else
		{
			fStartTime = g_pClientDE->GetTime();

			//time to reset
			bSeqDelay = LTTRUE;

			ResetDisplayList();
		}
	}
	else
	{
		fRatio = GetRandom(0.0f, 1.0f);

		//set dest rect
		rcRval.left		= m_ptGridDest.x + (int)(nHalfWidth * (1-fRatio));
		rcRval.top		= m_ptGridDest.y + (int)(nHeight * (1-fRatio));
		rcRval.right	= m_ptGridDest.x + nWidth - (int)(nHalfWidth * (1-fRatio));
		rcRval.bottom	= m_ptGridDest.y + (int)nHeight;
	}

	return rcRval;
}

// -----------------------------------------------/
//
// void CMotionDetector::OnObjectMove Implimentation
//
// Updates the moving object list for the marine MD
//
// -----------------------------------------------/
void CMotionDetector::OnObjectMove(HOBJECT hObj, LTBOOL bTeleport, DVector vPos)
{
	//verify we are based on the marine and not moving due to teleport
	if(m_pPlayerStats->GetHUDType() != 1)
		return;

	LTBOOL bKeyframed = LTFALSE;

	if(bTeleport)
	{
		// See if this is our special case...
		uint32 dwUsrFlags;
		g_pLTClient->GetObjectUserFlags(hObj, &dwUsrFlags);

		if(!(dwUsrFlags & USRFLG_KEYFRAMED_MOVEMENT))
			return;
		else
			bKeyframed = LTTRUE;
	}

	if(TestObjectForType(hObj, bKeyframed))
	{
		if(TestObjectForLocation(hObj, vPos))
			AddObjectToList(hObj, vPos);
		else
			RemoveObjectFromList(hObj);
	}
}

// -----------------------------------------------/
//
// void CMotionDetector::TestObjectForType Implimentation
//
// Test to see if the object is of the right type to 
// make the list.
//
// -----------------------------------------------/
LTBOOL CMotionDetector::TestObjectForType(HOBJECT hObj, LTBOOL bKeyFramed)
{
	CommonLT *pCommon = g_pInterface->Common();
	
	if(!pCommon)
		return LTFALSE;

	if(hObj == g_pLTClient->GetClientObject())
		return LTFALSE;

	uint32 nType = g_pInterface->GetObjectType(hObj);
	uint32 dwFlags;
	
	pCommon->GetObjectFlags(hObj, OFT_Flags, dwFlags);

	if(nType != OT_WORLDMODEL && nType != OT_MODEL)
		return LTFALSE;
	
	if((dwFlags & FLAG_REALLYCLOSE) || (!(dwFlags & FLAG_SOLID) && !bKeyFramed))
		return LTFALSE;

	return LTTRUE;
}

// -----------------------------------------------/
//
// void CMotionDetector::TestObjectForLocation Implimentation
//
// Test to see if the object is in the right location to 
// make the list. Not to far away and 
//
// -----------------------------------------------/
LTBOOL CMotionDetector::TestObjectForLocation(HOBJECT hObj, DVector vPos)//, DFLOAT *pRval, uint32 *pRange)
{
	//get player object
	HLOCALOBJ hPlayerObj = g_pClientDE->GetClientObject();


	//get player position
	LTVector vPlayerPos;
	g_pClientDE->GetObjectPos(hPlayerObj, &vPlayerPos);

	//adjust to 2d
	vPos.y = 0;
	vPlayerPos.y = 0;

	//get vector from player to object
	LTVector vPlayerToObj = vPos - vPlayerPos;

	//calculate range
	uint32 nRange = (uint32)vPlayerToObj.MagSqr();

	//test range
	if( nRange > MAX_MD_RANGE_SQ )
		return LTFALSE;


	//get my rotation
	DRotation		rPlayerRot;
	g_pClientDE->GetObjectRotation(hPlayerObj, &rPlayerRot);

	//direction vectors
	DVector vRight, vUp, vForward;

	//calculate my vectors
	g_pMathLT->GetRotationVectors (rPlayerRot, vRight, vUp, vForward);

	//norm vector
//	vPlayerToObj.Norm();

	//see if the object is infront of us
	if(vForward.Dot(vPlayerToObj) > 0)
		return LTTRUE;

	return LTFALSE;
}

// -----------------------------------------------/
//
// void CMotionDetector::AddObjectToList Implimentation
//
// Adds object to the display list for the Motion
// Detectot read-out. 
//
// -----------------------------------------------/
void CMotionDetector::AddObjectToList(HOBJECT hObj, LTVector vLoc)//DFLOAT fAngle, uint32 nRange, LTVector vLoc)
{
	MDBlip *pNewBlip = LTNULL;

	for(int i=0; i<MAX_BLIPS; i++)
	{
		if(m_pBlipArray[i].hObj == hObj)
		{
			m_pBlipArray[i].vLoc		= vLoc;
			return;
		}
		if(!pNewBlip && !(m_pBlipArray[i].hObj))
		{
			//found an empty node, save its address
			pNewBlip = &m_pBlipArray[i];
		}
	}

	//if we got this far we have a new object that needs to 
	//be added to the list.
	if(pNewBlip)
	{
		uint32 dwFlags = g_pClientDE->GetObjectClientFlags(hObj);
		dwFlags |= CF_NOTIFYREMOVE;

		g_pClientDE->SetObjectClientFlags(hObj, dwFlags);

			pNewBlip->hObj		= hObj;
			pNewBlip->vLoc		= vLoc;
	}
}

// -----------------------------------------------/
//
// void CMotionDetector::RemoveObjectFromList Implimentation
//
// Removes an object from the MD list.
//
// -----------------------------------------------/
void CMotionDetector::RemoveObjectFromList(HOBJECT hObj)
{
	for(int i=0; i<MAX_BLIPS; i++)
	{
		if(m_pBlipArray[i].hObj == hObj)
		{
			//found it
			memset(&m_pBlipArray[i], 0, sizeof(MDBlip));
			return;
		}
	}
}

// -----------------------------------------------/
//
// LTVector CMotionDetector::GetObjectPos Implimentation
//
// Retrives an objects position from the blip list.
//
// -----------------------------------------------/
LTVector CMotionDetector::GetObjectPos(HOBJECT hObj)
{
	for(int i=0; i<MAX_BLIPS; i++)
	{
		if(m_pBlipArray[i].hObj == hObj)
		{
			//found it
			return m_pBlipArray[i].vLoc;
		}
	}
	return LTVector(0,0,0);
}

// -----------------------------------------------/
//
// void CMotionDetector::ResetDisplayList Implimentation
//
// Resets the dispaly list
//
// -----------------------------------------------/
void CMotionDetector::ResetDisplayList()
{
	//clear the old list
	for(uint32 i=0; i<m_BlipDispList.nNumBlips; i++)
	{
		//check to see that the object is still moving
		LTVector vCurPos, vOldPos;
		g_pClientDE->GetObjectPos(m_BlipDispList.BlipDispArray[i].hObj, &vCurPos);

		vOldPos = GetObjectPos(m_BlipDispList.BlipDispArray[i].hObj);

		if(vCurPos == vOldPos)
			RemoveObjectFromList(m_BlipDispList.BlipDispArray[i].hObj);
		else
			AddObjectToList(m_BlipDispList.BlipDispArray[i].hObj, vCurPos);
	}
}

// -----------------------------------------------/
//
// void CMotionDetector::BuildDisplayList Implimentation
//
// Builds the dispaly list
//
// -----------------------------------------------/
void CMotionDetector::BuildDisplayList()
{
	//clear the display list
	memset(&m_BlipDispList, 0, sizeof(MDDispBlipList));

	//re-set constants
	m_BlipDispList.nMinRange = MAX_MD_RANGE;
	m_BlipDispList.ptOffset.x = 94;
	m_BlipDispList.ptOffset.y = 92;
	m_bFirstUpdate = LTTRUE;

	for(int i=0; i<MAX_BLIPS; i++)
	{
		if(m_pBlipArray[i].hObj)
		{
			//add the node to the list
			AddToDispList(m_pBlipArray[i]);
		}
	}
}

// -----------------------------------------------/
//
// void CMotionDetector::AddToDispList Implimentation
//
// Adds an element to the display list
//
// -----------------------------------------------/
void CMotionDetector::AddToDispList(MDBlip nBlip)
{
	int i = m_BlipDispList.nNumBlips;

	if(i < MAX_BLIPS)
	{
		m_BlipDispList.BlipDispArray[i].hObj = nBlip.hObj;
		m_BlipDispList.BlipDispArray[i].vLoc = nBlip.vLoc;

		//adjust to 2d
		m_BlipDispList.BlipDispArray[i].vLoc.y = 0;

		m_BlipDispList.nNumBlips++;

		return;
	}
}

// -----------------------------------------------/
//
// LTBOOL CMotionDetector::UpdateDispBlipList Implimentation
//
// Updates the display list of blips
//
// -----------------------------------------------/
LTBOOL CMotionDetector::UpdateDispBlipList(int nIndex, LTVector vPlayerPos, LTVector vPlayerForward, LTVector vPlayerRight)
{
	static LTFLOAT fRatio = (LTFLOAT)98 / MAX_MD_RANGE;

	//calculate range and test
	//get vector from player to object
	LTVector vPlayerToObj = m_BlipDispList.BlipDispArray[nIndex].vLoc - vPlayerPos;

	//calculate range
	uint32 nRange = (uint32)vPlayerToObj.Mag();

	//test range
	if( nRange > MAX_MD_RANGE )
		return LTFALSE;

	//test forward dot
	//norm vector
	vPlayerToObj.Norm();

	//see if the object is infront of us
	if(vPlayerForward.Dot(vPlayerToObj) <= 0)
		return LTFALSE;

	//calculate right dot
	LTFLOAT fMag	= ((LTFLOAT)nRange / MAX_MD_RANGE * 88)+10;

	LTFLOAT fAngle	= vPlayerRight.Dot(vPlayerToObj);

	// Bounds check
	fAngle = fAngle < -1.0f ? -1.0f : fAngle;
	fAngle = fAngle > 1.0f ? 1.0f : fAngle;

	fAngle	= (LTFLOAT) acos(fAngle);

	m_BlipDispList.BlipDispArray[nIndex].pt.x = (int)(fMag * cos(fAngle));
	m_BlipDispList.BlipDispArray[nIndex].pt.y = (int)(fMag * sin(fAngle));

	if(nRange < (uint32)m_BlipDispList.nMinRange)
		m_BlipDispList.nMinRange = nRange;

	return LTTRUE;
}
// -----------------------------------------------/
//
// void CMotionDetector::OnObjectRemove Implimentation
//
// Handles removing objects
//
// -----------------------------------------------/
void CMotionDetector::OnObjectRemove(HLOCALOBJ hObj)
{
	RemoveObjectFromList(hObj);
}

// -----------------------------------------------/
//
// void CMotionDetector::UpdateTracker Implimentation
//
// Updates the locatoin of the tracker blip
//
// -----------------------------------------------/
HSURFACE CMotionDetector::UpdateTracker(LTVector vPlayerPos, LTVector vPlayerForward, LTVector vPlayerRight)
{
	static LTFLOAT fRatio = (LTFLOAT)98 / MAX_MD_RANGE;

	//calculate range and test
	//get vector from player to object
	LTVector vPlayerToObj = m_vTrackerPos - vPlayerPos;

	//calculate range
	uint32 nRange = (uint32)vPlayerToObj.Mag();

	//test forward dot
	//norm vector
	vPlayerToObj.Norm();

	//see if the object is infront of us
	if(vPlayerForward.Dot(vPlayerToObj) <= 0)
	{
		//tracker is behiend us
		//test for left or right and set position of arrow
		if(vPlayerRight.Dot(vPlayerToObj) <= 0)
		{
			//behiend and to left
			m_ptTracker.x = -98;
			m_ptTracker.y = 0;
		}
		else
		{
			//behiend and to right
			m_ptTracker.x = 98;
			m_ptTracker.y = 0;
		}
		return m_hTrackerArrow;
	}
	else
	{
		//calculate right dot
		LTFLOAT fMag	= nRange > MAX_MD_TRACKER_RANGE ? 98.0f : ((LTFLOAT)nRange / MAX_MD_RANGE * 88)+10;
		LTFLOAT fAngle	= vPlayerRight.Dot(vPlayerToObj);

		// Bounds check
		fAngle = fAngle < -1.0f ? -1.0f : fAngle;
		fAngle = fAngle > 1.0f ? 1.0f : fAngle;

		fAngle	= (LTFLOAT) acos(fAngle);

		m_ptTracker.x = (int)(fMag * cos(fAngle));
		m_ptTracker.y = (int)(fMag * sin(fAngle));

		return m_hTrackerBlip;
	}
}
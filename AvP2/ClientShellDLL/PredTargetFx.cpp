// ----------------------------------------------------------------------- //
//
// MODULE  : PredTargetFx.cpp
//
// PURPOSE : Predator Target FX - Implementation
//
// CREATED : 6/13/2000
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PredTargetFx.h"
#include "GameClientShell.h"
#include "VarTrack.h"

extern CGameClientShell* g_pGameClientShell;

VarTrack	g_cvarSegmentDelay;
VarTrack	g_cvarRotRate;
VarTrack	g_cvarSegmentTime;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredTargetSFX::Init
//
//	PURPOSE:	Create the target FX
//
// ----------------------------------------------------------------------- //

DBOOL CPredTargetSFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	SFXCREATESTRUCT info;

	info.hServerObj = hServObj;

	return Init(&info);
}

DBOOL CPredTargetSFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct) return DFALSE;

	CSpecialFX::Init(psfxCreateStruct);

	g_cvarSegmentDelay.Init(g_pClientDE, "PredSegmentDelay", NULL, 0.15f);
	g_cvarRotRate.Init(g_pClientDE, "PredRotationRate", NULL, 3.0f);
	g_cvarSegmentTime.Init(g_pClientDE, "PredSegmentTime", NULL, 0.4f);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredTargetSFX::CreateObject
//
//	PURPOSE:	Create object associated with the targeting triangle
//
// ----------------------------------------------------------------------- //

LTBOOL CPredTargetSFX::CreateObject(CClientDE *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE) || !g_pGameClientShell) return DFALSE;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return DFALSE;

	// Setup the target triangle...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_NORMAL;

	HOBJECT hCamera = g_pGameClientShell->GetCameraMgr()->GetCameraObject(g_pGameClientShell->GetPlayerCamera());
	if (!hCamera) return LTFALSE;

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return LTFALSE;

	LTRotation rRot;
	g_pLTClient->GetObjectRotation(hCamera, &rRot);

	LTVector vPos;
	g_pLTClient->GetObjectPos(hPlayerObj, &vPos);

	createStruct.m_Pos = vPos;
	createStruct.m_Rotation = rRot;

	//create our object
	m_hObject = pClientDE->CreateObject(&createStruct);

	HDECOLOR	hTransColor = SETRGB_T(0,0,0);

	//load up the surfaces
	if(!m_hLockedImage)
	{
		m_hLockedImage = g_pInterfaceResMgr->GetSharedSurface("Interface\\StatusBar\\Predator\\locked_triangle.pcx");
		g_pLTClient->SetSurfaceAlpha (m_hLockedImage, 0.5f);
		g_pLTClient->GetSurfaceDims(m_hLockedImage, &m_nHalfImageWidth, &m_nHalfImageHeight);
	}


	if(!m_hTargetingImage[0])
		m_hTargetingImage[0] = g_pInterfaceResMgr->GetSharedSurface("Interface\\StatusBar\\Predator\\targeting_triangle3.pcx");

	if(!m_hTargetingImage[1])
		m_hTargetingImage[1] = g_pInterfaceResMgr->GetSharedSurface("Interface\\StatusBar\\Predator\\targeting_triangle1.pcx");

	if(!m_hTargetingImage[2])
		m_hTargetingImage[2] = g_pInterfaceResMgr->GetSharedSurface("Interface\\StatusBar\\Predator\\targeting_triangle2.pcx");

	for(int i=0 ; i<3 ; i++)
		g_pLTClient->SetSurfaceAlpha (m_hTargetingImage[i], 0.5f);

	//initialize some stuff
	m_fAngle		=	0.0f;

	//get handle to screen surface
	HSURFACE hScreen = g_pClientDE->GetScreenSurface();
	g_pLTClient->GetSurfaceDims(hScreen, &m_nScreenX, &m_nScreenY);

	//set the initial scale
	m_fScale		=	(LTFLOAT)m_nScreenX/m_nHalfImageWidth;

	//now calculate the actual image halves
	m_nHalfImageWidth /= 2;
	m_nHalfImageHeight /= 2;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	pStats->SetAutoTargetOn(LTTRUE);

	//set our start time
	m_fTime = 0.0f;

	//play the triangel sounds
	g_pClientSoundMgr->PlaySoundLocal("pred_lasertrack");

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredTargetSFX::WantRemove
//
//	PURPOSE:	If this gets called, remove the target (this should only get
//				called if we have a server object associated with us, and 
//				that server object gets removed).
//
// ----------------------------------------------------------------------- //

void CPredTargetSFX::WantRemove(DBOOL bRemove)
{
	CSpecialFX::WantRemove(bRemove);

	if (!g_pGameClientShell) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	// Tell the special fx mgr to go ahead and remove us...

	if (m_hObject)
	{
		psfxMgr->RemoveSpecialFX(m_hObject);
	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredTargetSFX::UpdatePhase
//
//	PURPOSE:	Update the targetign phase
//
// ----------------------------------------------------------------------- //

TargetPhase CPredTargetSFX::UpdatePhase()
{
	if(m_fTime > (g_cvarSegmentDelay.GetFloat() *2 + g_cvarSegmentTime.GetFloat()) )
	{
		//play the lock sound
		g_pClientSoundMgr->PlaySoundLocal("pred_lockon");
		m_hLockSound = g_pClientSoundMgr->PlaySoundLocal("pred_lockloop");
		return TP_LOCKED;
	}

	else if(m_fTime < g_cvarSegmentDelay.GetFloat() )
		return TP_LOCKING_0;

	else if(m_fTime < g_cvarSegmentDelay.GetFloat() * 2 )
		return TP_LOCKING_1;

	else return TP_LOCKING_2;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredTargetSFX::Update
//
//	PURPOSE:	Update function
//
// ----------------------------------------------------------------------- //

LTBOOL CPredTargetSFX::Update()
{
	if(g_pGameClientShell->GetPlayerStats()->GetHealthCount() == 0)
		return LTFALSE;

	LTIntPt ptTemp;

	//update the screen position of the target
	GetScreenPos(	g_pGameClientShell->GetPlayerCamera(), 
					g_pInterfaceMgr->GetPlayerStats()->GetAutoTarget(),
					ptTemp, LTTRUE);

	m_fXPos = (float)ptTemp.x;
	m_fYPos = (float)ptTemp.y;

	//update time
	m_fTime += g_pLTClient->GetFrameTime();

	//update the phase
	if(m_ePhase != TP_LOCKED)
		m_ePhase = UpdatePhase();
	else
	{
		//update the angle of the image's rotation
		LTFLOAT fFrameTime = g_pLTClient->GetFrameTime();
		m_fAngle +=  fFrameTime * g_cvarRotRate.GetFloat();
		
		if(m_fAngle > MATH_PI*2)
			m_fAngle = 0.0;
	}

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	return (LTBOOL)pStats->GetAutoTarget();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredTargetSFX::PostRenderDraw
//
//	PURPOSE:	Handles the 2d drawing
//
// ----------------------------------------------------------------------- //

void CPredTargetSFX::PostRenderDraw()
{
	if(g_pGameClientShell->IsFirstPerson())
	{
		//get handle to screen surface
		HSURFACE hScreen = g_pClientDE->GetScreenSurface();

		//set transparent color
		HDECOLOR hTransColor = SETRGB_T(0,0,0);

		
		switch(m_ePhase)
		{
		case(TP_LOCKED):
			{
				g_pLTClient->TransformSurfaceToSurfaceTransparent
					(	hScreen, 
						m_hLockedImage,
						LTNULL, 
						(int)m_fXPos-m_nHalfImageWidth, (int)m_fYPos-m_nHalfImageHeight, 
						m_fAngle,
						1.0f, 
						1.0f, 
						hTransColor);
				break;
			}
		default:
			DrawLockingTris();
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPredTargetSFX::DrawLockingTris
//
//	PURPOSE:	Handles locking segments
//
// ----------------------------------------------------------------------- //

void CPredTargetSFX::DrawLockingTris()
{
	//get handle to screen surface
	HSURFACE hScreen = g_pClientDE->GetScreenSurface();

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	LTRect rcDest;

	if(m_ePhase >= TP_LOCKING_0)
	{
		//draw the base tri
		
		//calc the percentage of travel
		LTFLOAT fRatio = m_fTime / g_cvarSegmentTime.GetFloat();
		if(fRatio > 1.0f) fRatio = 1.0f;
		uint32 destX = (m_nScreenX>>1) - (uint32)(fRatio*((m_nScreenX>>1) - m_fXPos));
		uint32 destY = m_nScreenY - (uint32)(fRatio*(m_nScreenY - m_fYPos));
		LTFLOAT fScale = 1+(1-fRatio)*m_fScale;

		rcDest.top = destY-(int)(m_nHalfImageHeight*fScale);
		rcDest.bottom = rcDest.top + (int)((m_nHalfImageHeight<<1)*fScale);
		rcDest.left = destX-(int)(m_nHalfImageWidth*fScale);
		rcDest.right = rcDest.left + (int)((m_nHalfImageWidth<<1)*fScale);

		g_pLTClient->ScaleSurfaceToSurfaceTransparent
			(	hScreen, 
				m_hTargetingImage[0],
				&rcDest, 
				LTNULL, 
				hTransColor);
	}
	if(m_ePhase >= TP_LOCKING_1)
	{
		//draw the left tri
		
		//calc the percentage of travel
		LTFLOAT fRatio = (m_fTime-g_cvarSegmentDelay.GetFloat()) / g_cvarSegmentTime.GetFloat();
		if(fRatio > 1.0f) fRatio = 1.0f;
		uint32 destX = (uint32)(fRatio*m_fXPos);
		uint32 destY = (m_nScreenY>>1) - (uint32)(fRatio*((m_nScreenY>>1) - m_fYPos));
		LTFLOAT fScale = 1+(1-fRatio)*m_fScale;

		rcDest.top = destY-(int)(m_nHalfImageHeight*fScale);
		rcDest.bottom = rcDest.top + (int)((m_nHalfImageHeight<<1)*fScale);
		rcDest.left = destX-(int)(m_nHalfImageWidth*fScale);
		rcDest.right = rcDest.left + (int)((m_nHalfImageWidth<<1)*fScale);

		g_pLTClient->ScaleSurfaceToSurfaceTransparent
			(	hScreen, 
				m_hTargetingImage[1],
				&rcDest, 
				LTNULL, 
				hTransColor);
	}
	if(m_ePhase >= TP_LOCKING_2)
	{
		//draw the right tri
		
		//calc the percentage of travel
		LTFLOAT fRatio = (m_fTime-(g_cvarSegmentDelay.GetFloat()*2)) / g_cvarSegmentTime.GetFloat();
		if(fRatio > 1.0f) fRatio = 1.0f;
		uint32 destX = m_nScreenX - (uint32)(fRatio*(m_nScreenX - m_fXPos));
		uint32 destY = (m_nScreenY>>1) - (uint32)(fRatio*((m_nScreenY>>1) - m_fYPos));
		LTFLOAT fScale = 1+(1-fRatio)*m_fScale;

		rcDest.top = destY-(int)(m_nHalfImageHeight*fScale);
		rcDest.bottom = rcDest.top + (int)((m_nHalfImageHeight<<1)*fScale);
		rcDest.left = destX-(int)(m_nHalfImageWidth*fScale);
		rcDest.right = rcDest.left + (int)((m_nHalfImageWidth<<1)*fScale);

		g_pLTClient->ScaleSurfaceToSurfaceTransparent
			(	hScreen, 
				m_hTargetingImage[2],
				&rcDest, 
				LTNULL, 
				hTransColor);
	}
}


//-------------------------------------------------------------------------------------------------
// SFX_PredTargetFactory
//-------------------------------------------------------------------------------------------------

const SFX_PredTargetFactory SFX_PredTargetFactory::registerThis;

CSpecialFX* SFX_PredTargetFactory::MakeShape() const
{
	return new CPredTargetSFX();
}


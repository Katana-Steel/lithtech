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

#include "stdafx.h"


#ifndef OVERLAY_MGR_H
#include "OverlayMGR.h"
#endif

#ifndef VISION_MODE_BUTE_MGR_H
#include "VisionModeButeMGR.h"
#endif

#ifndef __GAME_CLIENT_SHELL_H__
#include "GameClientShell.h"
#endif

#pragma warning (disable : 4503)
#include <deque>

//-----------------------------------------------------------------------------
// Constants and defines
//-----------------------------------------------------------------------------
static const float DISTANCE = 33.0f;
static const float DEFAULT_SPRITE_SIZE = 256.0f;

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------
struct Overlay
{
	enum Type
	{
		NORMAL = 0,
		NET,
		RAIL,
		ZOOM,
		FIRE,
		EVAC,
		ORIENT1,
		ORIENT2
	};

	Overlay( )
	{
		eType = NORMAL;
		hObj = NULL;
		fRotSpeed = 0.0f;
		fSize = DEFAULT_SPRITE_SIZE;
	}

	Type		eType;
	HLOCALOBJ	hObj;
	float		fRotSpeed;
	float		fSize;
};

typedef std::deque<Overlay*>			OverlayHandleList;
//typedef OverlayHandleList::iterator		OverlayHandleIterator;

//-----------------------------------------------------------------------------
// Data structure
//-----------------------------------------------------------------------------
struct OverlayData
{
	OverlayHandleList	m_ObjectList;
	float				m_fRoll;
	LTFLOAT				m_FovX, m_FovY;
	LTBOOL				m_bNetMode;
	LTBOOL				m_bRailMode;
	LTBOOL				m_bPredZoomMode;
	LTBOOL				m_bOnFireMode;
	LTBOOL				m_bEvacMode;
	LTBOOL				m_bOrientMode;
	LTRect				m_rcScreen;

	OverlayData();
};

OverlayData::OverlayData() : 
		m_bNetMode(LTFALSE), 
		m_bRailMode(LTFALSE), 
		m_bPredZoomMode(LTFALSE), 
		m_bOnFireMode(LTFALSE),
		m_bEvacMode(LTFALSE),
		m_bOrientMode(LTFALSE)
{
};

//-----------------------------------------------------------------------------
// Local function declarations
//-----------------------------------------------------------------------------
void ResetObjects(OverlayData* data);
void SetObjects(OverlayData* data, const std::string& mode);
void UpdateScale(OverlayData* data);
void UpdateRotation(OverlayData* data);

void PushNetOverlay(OverlayData* data, LTBOOL bHidden);
void PushRailOverlay(OverlayData* data, LTBOOL bHidden);
void PushPredZoomOverlay(OverlayData* data, LTBOOL bHidden);
void PushOnFireOverlay(OverlayData* data, LTBOOL bHidden);
void PushEvacOverlay(OverlayData* data, LTBOOL bHidden);
void PushOrientOverlays(OverlayData* data, LTBOOL bHidden);

void RemoveOverlay(OverlayData* data, Overlay::Type eType);
HOBJECT FindOverlayObject(OverlayData* data, Overlay::Type eType);

LTBOOL HideOverlays(LTBOOL bThirdPerson, OverlayData* data);

//-----------------------------------------------------------------------------
// OVERLAY MGR
//-----------------------------------------------------------------------------
OverlayMGR::OverlayMGR()
{
	m_pData		= new OverlayData();
	m_bHidden	= LTFALSE;
}

OverlayMGR::~OverlayMGR()
{
	// BEP 1/13/00
	// Without calling ResetObjects, it didn't look like the created objects
	// would be guaranteed to be removed.
	if( m_pData )
		ResetObjects( m_pData );

	if(m_pData)
	{
		delete m_pData;
		m_pData = LTNULL;
	}
}

//-----------------------------------------------------------------------------
// This is called when the mode should be set
void OverlayMGR::SetMode(const std::string& mode)
{
	ResetObjects(m_pData);
	SetObjects(m_pData, mode);

	if(m_pData->m_bNetMode)	PushNetOverlay(m_pData, m_bHidden);
	if(m_pData->m_bRailMode) PushRailOverlay(m_pData, m_bHidden);
	if(m_pData->m_bPredZoomMode) PushPredZoomOverlay(m_pData, m_bHidden);
	if(m_pData->m_bOnFireMode) PushOnFireOverlay(m_pData, m_bHidden);
	if(m_pData->m_bOrientMode) PushOrientOverlays(m_pData, m_bHidden);
	if(m_pData->m_bEvacMode) PushEvacOverlay(m_pData, m_bHidden);

	Update(LTTRUE);
}

//-----------------------------------------------------------------------------
// This is called every frame
void OverlayMGR::Update(LTBOOL bForceUpdate)
{
	LTFLOAT fovX, fovY;
	LTRect rc;
	g_pCameraMgr->GetCameraFOV(g_pGameClientShell->GetPlayerCamera(), fovX, fovY, LTTRUE);
	g_pCameraMgr->GetCameraRect(g_pGameClientShell->GetPlayerCamera(), rc, LTTRUE);
	if ((fovX != m_pData->m_FovX) || (fovY != m_pData->m_FovY) || bForceUpdate || rc.top != m_pData->m_rcScreen.top ||
		rc.bottom != m_pData->m_rcScreen.bottom || rc.left != m_pData->m_rcScreen.left || rc.right != m_pData->m_rcScreen.right)
	{
		UpdateScale(m_pData);
		m_pData->m_FovX = fovX;
		m_pData->m_FovY = fovY;
		m_pData->m_rcScreen = rc;
	}

	// Update the overlay rotations
	UpdateRotation(m_pData);

	//see if we should hide the overlays or not
	LTBOOL bThirdPerson = !g_pGameClientShell->IsFirstPerson();

	if(bThirdPerson != m_bHidden)
	{
		m_bHidden = HideOverlays(bThirdPerson, m_pData);
	}
}

//-----------------------------------------------------------------------------
// This is called when we want the net mode changed
void OverlayMGR::SetNetMode(LTBOOL bOn)
{
	//check to see if we are already in the state we need to be in
	if(bOn == m_pData->m_bNetMode)
		return;

	if(bOn)
	{
		//set up new overlay
		PushNetOverlay(m_pData, m_bHidden);
	}
	else
	{
		//remove net overlay
		RemoveOverlay(m_pData, Overlay::Type::NET);
	}

	//now set the data
	m_pData->m_bNetMode = bOn;
}

//-----------------------------------------------------------------------------
// This is called when we want the rail mode changed
void OverlayMGR::SetRailMode(LTBOOL bOn)
{
	//check to see if we are already in the state we need to be in
	if(bOn == m_pData->m_bRailMode)
		return;

	if(bOn)
	{
		//set up new overlay
		PushRailOverlay(m_pData, m_bHidden);
	}
	else
	{
		//remove net overlay
		RemoveOverlay(m_pData, Overlay::Type::RAIL);
	}

	//now set the data
	m_pData->m_bRailMode = bOn;
}

LTBOOL OverlayMGR::GetRailMode()
{
	return m_pData->m_bRailMode;
}

//-----------------------------------------------------------------------------
// This is called when we want the predator zoom mode changed
void OverlayMGR::SetPredZoomMode(LTBOOL bOn)
{
	//check to see if we are already in the state we need to be in
	if(bOn == m_pData->m_bPredZoomMode)
		return;

	if(bOn)
	{
		//set up new overlay
		PushPredZoomOverlay(m_pData, m_bHidden);
	}
	else
	{
		//remove net overlay
		RemoveOverlay(m_pData, Overlay::Type::ZOOM);
	}

	//now set the data
	m_pData->m_bPredZoomMode = bOn;
}

//-----------------------------------------------------------------------------
// This is called when we want the on fire mode changed
void OverlayMGR::SetOnFireMode(LTBOOL bOn)
{
	//check to see if we are already in the state we need to be in
	if(bOn == m_pData->m_bOnFireMode)
		return;

	if(bOn)
	{
		//set up new overlay
		PushOnFireOverlay(m_pData, m_bHidden);
	}
	else
	{
		//remove net overlay
		RemoveOverlay(m_pData, Overlay::Type::FIRE);
	}

	//now set the data
	m_pData->m_bOnFireMode = bOn;
}

//-----------------------------------------------------------------------------
// This is called when we want the evac mode changed
void OverlayMGR::SetEvacMode(LTBOOL bOn)
{
	//check to see if we are already in the state we need to be in
	if(bOn == m_pData->m_bEvacMode)
		return;

	if(bOn)
	{
		//set up new overlay
		PushEvacOverlay(m_pData, m_bHidden);
	}
	else
	{
		//remove evac overlay
		RemoveOverlay(m_pData, Overlay::Type::EVAC);
	}

	//now set the data
	m_pData->m_bEvacMode = bOn;
}

//-----------------------------------------------------------------------------
// This is called when we want to set the rotation of the evac overlay
void OverlayMGR::SetEvacRotation(LTRotation rRot)
{
	//check to see if we are already in the state we need to be in
	if(!m_pData->m_bEvacMode)
		return;

	HOBJECT hObj = FindOverlayObject(m_pData, Overlay::Type::EVAC);

	if(hObj)
	{
		g_pLTClient->SetObjectRotation(hObj, &rRot);
	}
}

//-----------------------------------------------------------------------------
// This is called when we want to set the alpha of the evac overlay
void OverlayMGR::SetEvacAlpha(LTFLOAT fAlpha)
{
	//check to see if we are already in the state we need to be in
	if(!m_pData->m_bEvacMode)
		return;

	HOBJECT hObj = FindOverlayObject(m_pData, Overlay::Type::EVAC);

	if(hObj)
	{
		LTFLOAT r, g, b, a;
		g_pLTClient->GetObjectColor(hObj, &r, &g, &b, &a);
		g_pLTClient->SetObjectColor(hObj, r, g, b, fAlpha);
	}
}

//-----------------------------------------------------------------------------
// This is called when we want the orient mode changed
void OverlayMGR::SetOrientMode(LTBOOL bOn)
{
	//check to see if we are already in the state we need to be in
	if(bOn == m_pData->m_bOrientMode)
		return;

	if(bOn)
	{
		//set up new overlay
		PushOrientOverlays(m_pData, m_bHidden);
	}
	else
	{
		//remove orient overlays
		RemoveOverlay(m_pData, Overlay::Type::ORIENT1);
		RemoveOverlay(m_pData, Overlay::Type::ORIENT2);
	}

	//now set the data
	m_pData->m_bOrientMode = bOn;
}

//-----------------------------------------------------------------------------
// This is called when we want to set the rotation of the evac overlay
void OverlayMGR::SetOrientAngle(LTFLOAT fAngle)
{
	//check to see if we are already in the state we need to be in
	if(!m_pData->m_bOrientMode)
		return;

	HOBJECT hObj1 = FindOverlayObject(m_pData, Overlay::Type::ORIENT1);
	HOBJECT hObj2 = FindOverlayObject(m_pData, Overlay::Type::ORIENT2);

	LTRotation rRot;

	if(hObj1)
	{
		rRot.Init();
		g_pMathLT->EulerRotateZ(rRot, fAngle);
		g_pLTClient->SetObjectRotation(hObj1, &rRot);
	}

	if(hObj2)
	{
		rRot.Init();
		g_pMathLT->EulerRotateZ(rRot, -fAngle);
		g_pLTClient->SetObjectRotation(hObj2, &rRot);
	}
}

//-----------------------------------------------------------------------------
// This is called when we want to set the normal overlay visible or not
void OverlayMGR::SetNormalOverlayVisible(LTBOOL bVis)
{
	// go through all the elements
	for(OverlayHandleList::iterator iter = m_pData->m_ObjectList.begin(); iter != m_pData->m_ObjectList.end(); ++iter)
	{
		if((*iter)->eType == Overlay::Type::NORMAL)
		{
			//found it!
			uint32 dwFlags = g_pLTClient->GetObjectFlags((*iter)->hObj);

			if(bVis)
				dwFlags |= FLAG_VISIBLE;
			else
				dwFlags &= ~FLAG_VISIBLE;

			g_pLTClient->SetObjectFlags((*iter)->hObj, dwFlags);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::Save()
//
//	PURPOSE:	Save the flashlight info...
//
// ----------------------------------------------------------------------- //

void OverlayMGR::Save(HMESSAGEWRITE hWrite)
{
	*hWrite << m_bHidden;
	*hWrite << m_pData->m_bNetMode;
	*hWrite << m_pData->m_bOnFireMode;
	*hWrite << m_pData->m_bOrientMode;
	*hWrite << m_pData->m_bPredZoomMode;
	*hWrite << m_pData->m_bRailMode;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::Load()
//
//	PURPOSE:	Load the flashlight info...
//
// ----------------------------------------------------------------------- //

void OverlayMGR::Load(HMESSAGEREAD hRead)
{
	*hRead >> m_bHidden;
	*hRead >> m_pData->m_bNetMode;
	*hRead >> m_pData->m_bOnFireMode;
	*hRead >> m_pData->m_bOrientMode;
	*hRead >> m_pData->m_bPredZoomMode;
	*hRead >> m_pData->m_bRailMode;
}

//-----------------------------------------------------------------------------
// Local function definition
//-----------------------------------------------------------------------------
void RemoveOverlay(OverlayData* data, Overlay::Type eType)
{
	// go through all the elements
	for(OverlayHandleList::iterator iter = data->m_ObjectList.begin(); iter != data->m_ObjectList.end(); ++iter)
	{
		if((*iter)->eType == eType)
		{
			// free up the resource
			if((*iter)->hObj)
				LTRESULT result = g_pLTClient->DeleteObject((*iter)->hObj);
			delete (*iter);
			data->m_ObjectList.erase(iter);
			return;
		}
	}
}

HOBJECT FindOverlayObject(OverlayData* data, Overlay::Type eType)
{
	// go through all the elements
	for(OverlayHandleList::iterator iter = data->m_ObjectList.begin(); iter != data->m_ObjectList.end(); ++iter)
	{
		if((*iter)->eType == eType)
		{
			// free up the resource
			if((*iter)->hObj)
				return (*iter)->hObj;
		}
	}

	return LTNULL;
}

void PushOnFireOverlay(OverlayData* data, LTBOOL bHidden)
{
	// Our object du'jour
	ObjectCreateStruct	objectStruct;

	// Clear out the object.... very important cause it isn't done in the constructor
	objectStruct.Clear();
	objectStruct.m_ObjectType = OT_SPRITE;
	objectStruct.m_Flags =  FLAG_SPRITE_NOZ | FLAG_REALLYCLOSE | FLAG_NOLIGHT | FLAG_FOGDISABLE;
	if(!bHidden) objectStruct.m_Flags |= FLAG_VISIBLE;
	SAFE_STRCPY(objectStruct.m_Filename, "sprites\\onfireoverlay.spr");

	// This will need to change based on what camera angles we have
	objectStruct.m_Pos.Init();
	objectStruct.m_Pos.z = DISTANCE;
	objectStruct.m_Scale = LTVector(0.10f, 0.10f, 1.00f);

	// Fill in a new structure
	Overlay* newObject = new Overlay;

	newObject->eType = Overlay::Type::FIRE;
	newObject->hObj	= g_pLTClient->CreateObject(&objectStruct);
	newObject->fSize = DEFAULT_SPRITE_SIZE;


	// Now make it!
	data->m_ObjectList.push_back(newObject);

	// See if we can set a better scale here
	UpdateScale(data);
}


void PushNetOverlay(OverlayData* data, LTBOOL bHidden)
{
	// Our object du'jour
	ObjectCreateStruct	objectStruct;

	// Clear out the object.... very important cause it isn't done in the constructor
	objectStruct.Clear();
	objectStruct.m_ObjectType = OT_SPRITE;
	objectStruct.m_Flags =  FLAG_SPRITE_NOZ | FLAG_REALLYCLOSE | FLAG_NOLIGHT | FLAG_FOGDISABLE;
	if(!bHidden) objectStruct.m_Flags |= FLAG_VISIBLE;
	SAFE_STRCPY(objectStruct.m_Filename, "sprites\\netoverlay.spr");

	// This will need to change based on what camera angles we have
	objectStruct.m_Pos.Init();
	objectStruct.m_Pos.z = DISTANCE;
	objectStruct.m_Scale = LTVector(0.10f, 0.10f, 1.00f);

	// Fill in a new structure
	Overlay* newObject = new Overlay;

	newObject->eType = Overlay::Type::NET;
	newObject->hObj	= g_pLTClient->CreateObject(&objectStruct);
	newObject->fSize = DEFAULT_SPRITE_SIZE;

	// Now make it!
	data->m_ObjectList.push_back(newObject);

	// See if we can set a better scale here
	UpdateScale(data);
}

void PushRailOverlay(OverlayData* data, LTBOOL bHidden)
{
	// Our object du'jour
	ObjectCreateStruct	objectStruct;

	// Clear out the object.... very important cause it isn't done in the constructor
	objectStruct.Clear();
	objectStruct.m_ObjectType = OT_SPRITE;
	objectStruct.m_Flags =  FLAG_SPRITE_NOZ | FLAG_REALLYCLOSE | FLAG_NOLIGHT | FLAG_FOGDISABLE;
	if(!bHidden) objectStruct.m_Flags |= FLAG_VISIBLE;
	SAFE_STRCPY(objectStruct.m_Filename, "sprites\\railgunoverlay.spr");

	// This will need to change based on what camera angles we have
	objectStruct.m_Pos.Init();
	objectStruct.m_Pos.z = DISTANCE;
	objectStruct.m_Scale = LTVector(0.10f, 0.10f, 1.00f);

	// Fill in a new structure
	Overlay* newObject = new Overlay;

	newObject->eType = Overlay::Type::RAIL;
	newObject->hObj	= g_pLTClient->CreateObject(&objectStruct);
	newObject->fSize = 510.0f;

	// Now make it!
	data->m_ObjectList.push_front(newObject);

	// See if we can set a better scale here
	UpdateScale(data);
}

void PushPredZoomOverlay(OverlayData* data, LTBOOL bHidden)
{
	// Our object du'jour
	ObjectCreateStruct	objectStruct;

	// Clear out the object.... very important cause it isn't done in the constructor
	objectStruct.Clear();
	objectStruct.m_ObjectType = OT_SPRITE;
	objectStruct.m_Flags =  FLAG_SPRITE_NOZ | FLAG_REALLYCLOSE | FLAG_NOLIGHT | FLAG_FOGDISABLE;
	if(!bHidden) objectStruct.m_Flags |= FLAG_VISIBLE;
	SAFE_STRCPY(objectStruct.m_Filename, "sprites\\predzoom.spr");

	// This will need to change based on what camera angles we have
	objectStruct.m_Pos.Init();
	objectStruct.m_Pos.z = DISTANCE;
	objectStruct.m_Scale = LTVector(0.10f, 0.10f, 1.00f);

	// Fill in a new structure
	Overlay* newObject = new Overlay;

	newObject->eType = Overlay::Type::ZOOM;
	newObject->hObj	= g_pLTClient->CreateObject(&objectStruct);
	newObject->fSize = DEFAULT_SPRITE_SIZE;

	// Now make it!
	data->m_ObjectList.push_front(newObject);

	// See if we can set a better scale here
	UpdateScale(data);
}

void PushEvacOverlay(OverlayData* data, LTBOOL bHidden)
{
	// Our object du'jour
	ObjectCreateStruct	objectStruct;

	// Clear out the object.... very important cause it isn't done in the constructor
	objectStruct.Clear();
	objectStruct.m_ObjectType = OT_SPRITE;
	objectStruct.m_Flags = FLAG_SPRITE_NOZ | FLAG_REALLYCLOSE | FLAG_NOLIGHT| FLAG_FOGDISABLE;
	objectStruct.m_Flags2 = FLAG2_SPRITE_TROTATE;
	if(!bHidden) objectStruct.m_Flags |= FLAG_VISIBLE;
	SAFE_STRCPY(objectStruct.m_Filename, "sprites\\evacoverlay.spr");

	// This will need to change based on what camera angles we have
	objectStruct.m_Pos.Init();
	objectStruct.m_Pos.z = DISTANCE;
	objectStruct.m_Scale = LTVector(0.10f, 0.10f, 1.00f);

	// Fill in a new structure
	Overlay* newObject = new Overlay;

	newObject->eType = Overlay::Type::EVAC;
	newObject->hObj	= g_pLTClient->CreateObject(&objectStruct);
	newObject->fSize = DEFAULT_SPRITE_SIZE;


	// Now make it!
	data->m_ObjectList.push_back(newObject);

	// See if we can set a better scale here
	UpdateScale(data);
}

void PushOrientOverlays(OverlayData* data, LTBOOL bHidden)
{
	// Our object du'jour
	ObjectCreateStruct	objectStruct;

	// Clear out the object.... very important cause it isn't done in the constructor
	objectStruct.Clear();
	objectStruct.m_ObjectType = OT_SPRITE;
	objectStruct.m_Flags = FLAG_SPRITE_NOZ | FLAG_REALLYCLOSE | FLAG_NOLIGHT| FLAG_FOGDISABLE;
	objectStruct.m_Flags2 = FLAG2_SPRITE_TROTATE;
	if(!bHidden) objectStruct.m_Flags |= FLAG_VISIBLE;
	SAFE_STRCPY(objectStruct.m_Filename, "sprites\\orientedge.spr");

	// This will need to change based on what camera angles we have
	objectStruct.m_Pos.Init();
	objectStruct.m_Pos.z = DISTANCE;
	objectStruct.m_Scale = LTVector(0.10f, 0.10f, 1.00f);

	// Fill in a new structure
	Overlay* newObject = new Overlay;

	newObject->eType = Overlay::Type::ORIENT1;
	newObject->hObj	= g_pLTClient->CreateObject(&objectStruct);
	newObject->fSize = DEFAULT_SPRITE_SIZE;

	// Now make it!
	data->m_ObjectList.push_back(newObject);

	// Fill in a new structure
	newObject = new Overlay;

	newObject->eType = Overlay::Type::ORIENT2;
	newObject->hObj	= g_pLTClient->CreateObject(&objectStruct);
	newObject->fSize = DEFAULT_SPRITE_SIZE;

	// Now make it!
	data->m_ObjectList.push_back(newObject);

	// See if we can set a better scale here
	UpdateScale(data);
}


void ResetObjects(OverlayData* data)
{
	// go through all the elements
	for(OverlayHandleList::iterator iter = data->m_ObjectList.begin(); iter != data->m_ObjectList.end(); ++iter)
	{
		// free up the resource
		if((*iter)->hObj)
			LTRESULT result = g_pLTClient->DeleteObject((*iter)->hObj);
		delete (*iter);
	}
	// empty the list
	data->m_ObjectList.clear();
}

//-----------------------------------------------------------------------------
void SetObjects(OverlayData* data, const std::string& mode)
{
	// Get a list of all overlays
	const VisionMode::CamOverlayList* overlayList = VisionMode::g_pButeMgr->GetOverlays(mode);

	if (overlayList)
	{
		// Our object du'jour
		ObjectCreateStruct	objectStruct;

		// Go through them and create them
		for (VisionMode::CamOverlayList::const_iterator iter = overlayList->begin(); iter!= overlayList->end(); ++iter)
		{
			// What is your name!
			std::string sprite = (*iter).m_Name;

			// None is not a good response. It makes us feel unwanted.
			if (sprite != "None")
			{
				// Clear out the object.... very important cause it isn't done in the constructor
				objectStruct.Clear();
				objectStruct.m_ObjectType = OT_SPRITE;
				objectStruct.m_Flags = FLAG_VISIBLE | FLAG_SPRITE_NOZ | FLAG_REALLYCLOSE | FLAG_NOLIGHT| FLAG_FOGDISABLE;
				objectStruct.m_Flags2 = FLAG2_SPRITE_TROTATE;
				SAFE_STRCPY(objectStruct.m_Filename, sprite.c_str());

				// This will need to change based on what camera angles we have
				objectStruct.m_Pos.Init();
				objectStruct.m_Pos.z = DISTANCE;
				objectStruct.m_Scale = LTVector(0.10f, 0.10f, 1.00f);

				// Fill in a new structure
				Overlay* newObject = new Overlay;

				newObject->eType = Overlay::Type::NORMAL;
				newObject->hObj	= g_pLTClient->CreateObject(&objectStruct);
				newObject->fRotSpeed = (*iter).m_fRotSpeed;
				newObject->fSize = DEFAULT_SPRITE_SIZE;

				// Now make it!
				data->m_ObjectList.push_back(newObject);

				// Set fovs so we update them
				data->m_FovX = data->m_FovY = 0;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void UpdateScale(OverlayData* data)
{
	// Get scaling factors
	LTFLOAT cameraX, cameraY, distance = DISTANCE;
	GetCameraScale(g_pGameClientShell->GetPlayerCamera(), distance, cameraX, cameraY);

	LTVector scale;
	scale.z = 1.0f;

	// go through all the elements
	for(OverlayHandleList::iterator iter = data->m_ObjectList.begin(); iter != data->m_ObjectList.end(); ++iter)
	{
		Overlay *pOverlay = *iter;

		scale.x = cameraX / pOverlay->fSize;
		scale.y = cameraY / pOverlay->fSize;

		g_pLTClient->SetObjectScale(pOverlay->hObj, &scale);
	}
}

//-----------------------------------------------------------------------------
void UpdateRotation(OverlayData* data)
{
	for(OverlayHandleList::iterator iter = data->m_ObjectList.begin(); iter != data->m_ObjectList.end(); ++iter)
	{
		Overlay *pOverlay = *iter;

		// Don't update it if it's not rotating
		if (pOverlay->fRotSpeed == 0.0f)
			continue;

		// Calculate the rotation amount, in radians (converted from revolutions per sec)
		float fRotAmount = pOverlay->fRotSpeed * g_pLTClient->GetFrameTime() * 2 * MATH_PI;

		LTRotation rRot;

		g_pLTClient->GetObjectRotation(pOverlay->hObj, &rRot);
		g_pLTClient->GetMathLT()->EulerRotateZ(rRot, fRotAmount);
		g_pLTClient->SetObjectRotation(pOverlay->hObj, &rRot);
	}
}

//-----------------------------------------------------------------------------
LTBOOL HideOverlays(LTBOOL bThirdPerson, OverlayData* data)
{
	// go through all the elements
	for(OverlayHandleList::iterator iter = data->m_ObjectList.begin(); iter != data->m_ObjectList.end(); ++iter)
	{
		uint32 dwFlags = g_pLTClient->GetObjectFlags((*iter)->hObj);

		if(bThirdPerson)
			dwFlags &= ~FLAG_VISIBLE;
		else
			dwFlags |= FLAG_VISIBLE;

		g_pLTClient->SetObjectFlags((*iter)->hObj, dwFlags);
	}

	return bThirdPerson;
}


// ----------------------------------------------------------------------- //
//
// MODULE  : MarkSFX.cpp
//
// PURPOSE : Mark special FX - Implementation
//
// CREATED : 10/13/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MarkSFX.h"
#include "iltclient.h"
#include "ltlink.h"
#include "GameClientShell.h"
#include "SurfaceFunctions.h"
#include "VarTrack.h"

extern CGameClientShell* g_pGameClientShell;

#define REGION_DIAMETER			100.0f  // Squared distance actually
#define MAX_MARKS_IN_REGION		10

VarTrack	g_cvarClipMarks;
VarTrack	g_cvarLightMarks;
VarTrack	g_cvarMarkLifetime;
VarTrack	g_cvarMarkFadetime;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMarkSFX::Init
//
//	PURPOSE:	Create the mark
//
// ----------------------------------------------------------------------- //

LTBOOL CMarkSFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
    if (!hRead) return LTFALSE;

	MARKCREATESTRUCT mark;

	mark.hServerObj = hServObj;
	mark.m_Rotation = hRead->ReadRotation();
	mark.m_vPos = hRead->ReadVector();
	mark.m_fScale		= hRead->ReadFloat();
	mark.nAmmoId		= hRead->ReadByte();
	mark.nSurfaceType	= hRead->ReadByte();

    return Init(&mark);
}

LTBOOL CMarkSFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!psfxCreateStruct) return LTFALSE;

	CSpecialFX::Init(psfxCreateStruct);

	MARKCREATESTRUCT* pMark = (MARKCREATESTRUCT*)psfxCreateStruct;

    m_Rotation = pMark->m_Rotation;
	VEC_COPY(m_vPos, pMark->m_vPos);
	m_fScale		= pMark->m_fScale;
	m_nAmmoId		= pMark->nAmmoId;
	m_nSurfaceType	= pMark->nSurfaceType;
	m_fStartTime	= g_pLTClient->GetTime();

	if (!g_cvarClipMarks.IsInitted())
	{
        g_cvarClipMarks.Init(g_pLTClient, "ClipMarks", NULL, 1.0f);
	}

	if (!g_cvarLightMarks.IsInitted())
	{
        g_cvarLightMarks.Init(g_pLTClient, "LightMarks", NULL, 1.0f);
	}

 	if (!g_cvarMarkLifetime.IsInitted())
	{
        g_cvarMarkLifetime.Init(g_pLTClient, "MarkLife", NULL, 10.0f);
	}

	if (!g_cvarMarkFadetime.IsInitted())
	{
        g_cvarMarkFadetime.Init(g_pLTClient, "MarkFade", NULL, 1.0f);
	}

   return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMarkSFX::CreateObject
//
//	PURPOSE:	Create object associated with the mark
//
// ----------------------------------------------------------------------- //

LTBOOL CMarkSFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !g_pGameClientShell) return LTFALSE;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr) return LTFALSE;


	// Before we create a new buillet hole see if there is already another
	// bullet hole close by that we could use instead...

	CSpecialFXList* pList = psfxMgr->GetFXList(SFX_MARK_ID);
    if (!pList) return LTFALSE;

    HOBJECT hMoveObj         = LTNULL;
    HOBJECT hObj             = LTNULL;
    LTFLOAT  fClosestMarkDist = REGION_DIAMETER;
    uint8   nNumInRegion     = 0;

    LTVector vPos;

	for( CSpecialFXList::Iterator iter = pList->Begin();
		 iter != pList->End(); ++iter)
	{
		if(iter->pSFX)
		{
			hObj = iter->pSFX->GetObject();
			if (hObj)
			{
				pClientDE->GetObjectPos(hObj, &vPos);

                LTFLOAT fDist = VEC_DISTSQR(vPos, m_vPos);
				if (fDist < REGION_DIAMETER)
				{
					if (fDist < fClosestMarkDist)
					{
						fClosestMarkDist = fDist;
						hMoveObj = hObj;
					}

					if (++nNumInRegion > MAX_MARKS_IN_REGION)
					{
						// Just move this bullet-hole to the correct pos, and
						// remove thyself...

						pClientDE->SetObjectPos(hMoveObj, &m_vPos);
                        return LTFALSE;
					}
				}
			}
		}
	}


	// Setup the mark...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

    LTFLOAT fScaleAdjust = 1.0f;
	if (!GetImpactSprite((SurfaceType)m_nSurfaceType, fScaleAdjust, m_nAmmoId,
		createStruct.m_Filename, ARRAY_LEN(createStruct.m_Filename)))
	{
        return LTFALSE;
	}

	createStruct.m_ObjectType = OT_SPRITE;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_ROTATEABLESPRITE;

	// Should probably force this in low detail modes...
	if (g_cvarLightMarks.GetFloat() == 0.0f)
	{
		createStruct.m_Flags |= FLAG_NOLIGHT;
	}

	VEC_COPY(createStruct.m_Pos, m_vPos);
    createStruct.m_Rotation = m_Rotation;

	m_hObject = pClientDE->CreateObject(&createStruct);

	m_fScale *= fScaleAdjust;

    LTVector vScale;
	VEC_SET(vScale, m_fScale, m_fScale, m_fScale);
	m_pClientDE->SetObjectScale(m_hObject, &vScale);

	if (g_cvarClipMarks.GetFloat() > 0)
	{
		// Clip the mark to th poly...

		IntersectQuery qInfo;
		IntersectInfo iInfo;

        LTVector vU, vR, vF;
        g_pLTClient->GetRotationVectors(&m_Rotation, &vU, &vR, &vF);

		qInfo.m_From = m_vPos + (vF * 2.0);
		qInfo.m_To   = m_vPos - (vF * 2.0);

		qInfo.m_Flags = IGNORE_NONSOLID | INTERSECT_HPOLY;

        if (g_pLTClient->IntersectSegment(&qInfo, &iInfo))
		{
            g_pLTClient->ClipSprite(m_hObject, iInfo.m_hPoly);
		}
	}

    LTFLOAT r, g, b, a;
	r = g = b = 1.0f;
	a = 1.0f;
	m_pClientDE->SetObjectColor(m_hObject, r, g, b, a);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMarkSFX::WantRemove
//
//	PURPOSE:	If this gets called, remove the mark (this should only get
//				called if we have a server object associated with us, and
//				that server object gets removed).
//
// ----------------------------------------------------------------------- //

void CMarkSFX::WantRemove(LTBOOL bRemove)
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
//	ROUTINE:	CMarkSFX::Update
//
//	PURPOSE:	Update the mark
//
// ----------------------------------------------------------------------- //

LTBOOL CMarkSFX::Update()
{
	LTFLOAT fTimeDelta = g_pLTClient->GetTime()-m_fStartTime;

	// See if it is time to start the fade
	if(fTimeDelta > g_cvarMarkLifetime.GetFloat())
	{
		// See what our new alpha should be
		fTimeDelta -= g_cvarMarkLifetime.GetFloat();

		// See if we need to just go away
		if(fTimeDelta > g_cvarMarkFadetime.GetFloat())
			return LTFALSE;

		// Ok time to update our alpha
		LTFLOAT r, g, b, a;

		g_pLTClient->GetObjectColor(m_hObject, &r, &g, &b, &a);
		g_pLTClient->SetObjectColor(m_hObject, r, g, b, 1.0f - (fTimeDelta/g_cvarMarkFadetime.GetFloat()));
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMarkSFX::SetMode
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CMarkSFX::SetMode(const std::string& mode)
{
	// Get the current color
	LTFLOAT r, g, b, a;
	g_pLTClient->GetObjectColor(m_hObject, &r, &g, &b, &a);

	// Check to see if we are in the default mode
	if ("None" == mode || "Normal" == mode)
	{
		g_pLTClient->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, a);
	}
	else
	{
		ViewModeMGR* pModeMgr = g_pGameClientShell->GetVisionModeMgr();
		if (!pModeMgr) return;

		LTVector vColor = pModeMgr->GetVertexTint();

		g_pLTClient->SetObjectColor(m_hObject, vColor.x, vColor.y, vColor.z, a);
	}
}



//-------------------------------------------------------------------------------------------------
// SFX_MarkFactory
//-------------------------------------------------------------------------------------------------

const SFX_MarkFactory SFX_MarkFactory::registerThis;

CSpecialFX* SFX_MarkFactory::MakeShape() const
{
	return new CMarkSFX();
}


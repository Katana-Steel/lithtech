// -------------------------------------------------------------------- //
//
// MODULE  : IKChain.cpp
//
// PURPOSE : A contrained IK chain
//
// CREATED : 5/30/01
//
// -------------------------------------------------------------------- //

#include "stdafx.h"
#include "IKChain.h"

// -------------------------------------------------------------------- //
//
// FUNCTION:	IKChain_Constrained::IKChain_Constrained()
//
// PURPOSE:		Init everything
//
// -------------------------------------------------------------------- //

IKChain_Constrained::IKChain_Constrained()
{
	m_pPoints = LTNULL;
	m_nPoints = 0;

	m_fSegmentLength = 0.0f;

	m_fConstraint = 0.0f;
	m_fCorrectionRate = 0.0f;

	m_hLines = LTNULL;
	m_pLines = LTNULL;
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	IKChain_Constrained::~IKChain_Constrained()
//
// PURPOSE:		Make sure everything got terminated
//
// -------------------------------------------------------------------- //

IKChain_Constrained::~IKChain_Constrained()
{
	Term();
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	IKChain_Constrained::Init()
//
// PURPOSE:		Setup the chain with the setting we want
//
// -------------------------------------------------------------------- //

LTBOOL IKChain_Constrained::Init(int nSegments, LTFLOAT fSegmentLength, LTFLOAT fConstraint, LTFLOAT fCorrectionRate)
{
	// Make sure valid info was passed in...
	if((nSegments < 1) || (fConstraint < -1.0f) || (fConstraint > 1.0f) || (fCorrectionRate < 0.0f))
		return LTFALSE;

	// Make sure everything was terminated
	Term();


	// Setup everything
	m_pPoints = new LTVector[nSegments + 1];
	m_nPoints = nSegments + 1;

	m_fSegmentLength = fSegmentLength;

	m_fConstraint = fConstraint;
	m_fCorrectionRate = fCorrectionRate;

#ifdef _CLIENTBUILD

	ObjectCreateStruct ocs;
	ocs.m_ObjectType = OT_LINESYSTEM;
	ocs.m_Flags = FLAG_VISIBLE | FLAG_UPDATEUNSEEN;

	m_hLines = g_pLTClient->CreateObject(&ocs);
	m_pLines = new HLTLINE[nSegments];

	// Add all the lines
	LTLine line;

	line.m_Points[0].m_Pos.Init();
	line.m_Points[0].r = 1.0f;
	line.m_Points[0].g = 1.0f;
	line.m_Points[0].b = 0.0f;
	line.m_Points[0].a = 1.0f;

	line.m_Points[1].m_Pos.Init();
	line.m_Points[1].r = 1.0f;
	line.m_Points[1].g = 1.0f;
	line.m_Points[1].b = 0.0f;
	line.m_Points[1].a = 1.0f;

	for(int i = 0; i < nSegments; i++)
	{
		m_pLines[i] = g_pLTClient->AddLine(m_hLines, &line);
	}

#endif

	return LTTRUE;
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	IKChain_Constrained::Term()
//
// PURPOSE:		Setup the chain with the setting we want
//
// -------------------------------------------------------------------- //

void IKChain_Constrained::Term()
{
	if(m_pPoints)
	{
		delete[] m_pPoints;
		m_pPoints = LTNULL;
	}

#ifdef _CLIENTBUILD

	if(m_hLines)
	{
		g_pLTClient->DeleteObject(m_hLines);
		m_hLines = LTNULL;
	}

#endif
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	IKChain_Constrained::Update()
//
// PURPOSE:		Handle constant updates of the chain
//
// -------------------------------------------------------------------- //

LTBOOL IKChain_Constrained::Update(LTVector vPos, LTVector vDir, LTFLOAT fFrameTime)
{
	// NOTE -- vDir should be normalized when it's passed in... and the vPos is
	// the first point of the chain (origin)


	// If there's no points... just return
	if(m_nPoints < 2)
		return LTFALSE;


	// Setup the first points in the chain
	m_pPoints[0] = vPos;
	Constrain_Point(&m_pPoints[0], &vDir, &m_pPoints[1], m_fConstraint, fFrameTime);


	// Now go through all the points and update them
	for(int i = 2; i < m_nPoints; i++)
	{
		vDir = m_pPoints[i - 1] - m_pPoints[i - 2];
		vDir.Norm();

		Constrain_Point(&m_pPoints[i - 1], &vDir, &m_pPoints[i], m_fConstraint, fFrameTime);
	}


#ifdef _CLIENTBUILD
#ifdef _IK_CHAIN_TEST_

	// Update the line system to this position
	if(m_hLines && m_pLines)
	{
		// Update the system
		g_pLTClient->SetObjectPos(m_hLines, &vPos);


		// Update the lines...
		LTLine line;

		for(int j = 0; j < (m_nPoints - 1); j++)
		{
			g_pLTClient->GetLineInfo(m_pLines[j], &line);

			line.m_Points[0].m_Pos = m_pPoints[j] - m_pPoints[0];
			line.m_Points[1].m_Pos = m_pPoints[j + 1] - m_pPoints[0];

			g_pLTClient->CPrint("Chain points %d: %f %f %f", j, m_pPoints[j].x, m_pPoints[j].y, m_pPoints[j].z);

			g_pLTClient->SetLineInfo(m_pLines[j], &line);
		}
	}
#endif
#endif

	return LTTRUE;
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	IKChain_Constrained::Constrain_Point()
//
// PURPOSE:		Constain a point based off a position and direction
//
// -------------------------------------------------------------------- //

void IKChain_Constrained::Constrain_Point(LTVector *vPos, LTVector *vDir, LTVector *vPt, LTFLOAT fConstraint, LTFLOAT fFrameTime)
{
	// Ok... here's where all the work gets done...


	// Get the angle between the destination facing and the current vector to the point
	LTVector vTemp1 = *vPt - *vPos;

	if(!vTemp1.x && !vTemp1.y && !vTemp1.z)
	{
		*vPt = *vPos + (*vDir * m_fSegmentLength);
		return;
	}

	LTFLOAT fDot = vTemp1.Dot(*vDir);

	// Normalize this vector so we can use it for a distance calculation later
	vTemp1.Norm();


	// -------------------------------------------------------------------- //
	// Here's where the correction code is...

	// If the angle between these is greater than our restraint, then just use that instead...
	LTFLOAT fDestDot = vTemp1.Dot(*vDir);

	if(fDestDot < fConstraint)
		fDestDot = fConstraint;

	fDestDot += (fFrameTime * m_fCorrectionRate);

	// -------------------------------------------------------------------- //
	// Now calculate a vector that goes out from the facing vector to the current point
	LTVector vTemp2 = *vPt - (*vPos + (*vDir * fDot));
	vTemp2.Norm();

	// Now find the distance that the final point needs to be from the facing vector

	// Bounds check
	fDestDot = fDestDot < -1.0f ? -1.0f : fDestDot;
	fDestDot = fDestDot > 1.0f ? 1.0f : fDestDot;

	LTFLOAT fDestAngle = (LTFLOAT)acos(fDestDot);

	LTFLOAT fDist = (LTFLOAT)sin(fDestAngle) * m_fSegmentLength;


	// -------------------------------------------------------------------- //
	// Ok... this should be our final constrained point

	*vPt = *vPos + (*vDir * (m_fSegmentLength * fDestDot)) + (vTemp2 * fDist);
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	IKChain_Constrained::GetPoint()
//
// PURPOSE:		Gets a point and forward vector if asked for...
//
// -------------------------------------------------------------------- //

LTVector IKChain_Constrained::GetPoint(uint8 nPt, LTVector* pvF)
{ 
	LTVector rVal(0,0,0);

	if(nPt >= (uint8)m_nPoints)
	{
		if(pvF)
			*pvF = rVal;
		
		return rVal;
	}

	// See about the forward vector
	if(pvF)
	{
		if(nPt < (uint8)m_nPoints-1)
		{
			LTVector vF =  m_pPoints[nPt+1] - m_pPoints[nPt];
			vF.Norm();
			*pvF = vF;
		}
		else
			*pvF = rVal;
	}

	return m_pPoints[nPt];
}

// ----------------------------------------------------------------------- //
//
// MODULE  : CNodeController.cpp
//
// PURPOSE : CNodeController implementation
//
// CREATED : 05.20.1999
//
// ----------------------------------------------------------------------- //

#include "StdAfx.h"
#include "NodeController.h"
#include "SFXMsgIds.h"
#include "ModelLT.h"
#include "TransformLT.h"
#include "CharacterFx.h"
#include "SoundMgr.h"
#include "VarTrack.h"
#include "ModelButeMgr.h"
#include "InterfaceMgr.h"
#include "BodyPropFX.h"
#include "ClientSoundMgr.h"

// [KLS] Added 9/6/01 for IsCinematic() call
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;
// [KLS] End 9/6/01


//const LTFLOAT s_fMaxHeadYaw = 65.0f * MATH_PI / 180.0f;
//const LTFLOAT s_fMaxHeadPitch = 65.0f * MATH_PI / 180.0f;
const LTFLOAT s_fHardHeadMinYaw = -120.0f * MATH_PI / 180.0f;
const LTFLOAT s_fHardHeadMaxYaw = 120.0f * MATH_PI / 180.0f;
const LTFLOAT s_fMaxAimingYaw = 50.0f * MATH_PI / 180.0f;


static LTFLOAT ClampAngle(LTFLOAT angle)
{
	while( angle >= MATH_PI )
	{
		angle -= MATH_CIRCLE;
	}

	while( angle < -MATH_PI )
	{
		angle += MATH_CIRCLE;
	}

	return angle;
}

static LTFLOAT ClampAngle360(LTFLOAT angle)
{
	while( angle >= MATH_CIRCLE )
	{
		angle -= MATH_CIRCLE;
	}

	while( angle < 0.0f )
	{
		angle += MATH_CIRCLE;
	}

	return angle;
}

static LTFLOAT ClampDegreeAngle(LTFLOAT angle)
{
	while( angle >= 180.0f )
	{
		angle -= 360.0f;
	}

	while( angle < -180.0f )
	{
		angle += 360.0f;
	}

	return angle;
}

static LTFLOAT ClampDegreeAngle360(LTFLOAT angle)
{
	while( angle >= 360.0f )
	{
		angle -= 360.0f;
	}

	while( angle < 0.0f )
	{
		angle += 360.0f;
	}

	return angle;
}

static LTVector ClampAngle(LTVector angles)
{
	return LTVector( ClampAngle(angles.x),
					ClampAngle(angles.y),
					ClampAngle(angles.z) );
}

static LTFLOAT GetObjectYaw(HOBJECT hServerObj)
{
	LTRotation rObjRot;
	g_pLTClient->GetObjectRotation(hServerObj, &rObjRot);

	LTVector vR, vU, vF;
	g_pMathLT->GetRotationVectors(rObjRot, vR, vU, vF);

	vF.y = 0.0f;
	vF.Norm();

	LTFLOAT fAngle = (LTFLOAT)acos(vF.z);
	if(vF.x < 0.0f) fAngle = MATH_CIRCLE - fAngle;

	while(fAngle < 0.0f) fAngle += MATH_CIRCLE;
	while(fAngle > MATH_CIRCLE) fAngle -= MATH_CIRCLE;

	return fAngle;
}


VarTrack	g_vtLipSyncMaxRot;
VarTrack	g_vtLipSyncFreq;

VarTrack	g_vtPitchSmoothRate;
VarTrack	g_vtYawSmoothRate;
VarTrack	g_vtStrafeSmoothRate;

VarTrack	g_vtHeadFollowSpeed;


#ifdef _DEBUG
//#define GRAPH_LIPSYNC_SOUND
#endif




#ifdef GRAPH_LIPSYNC_SOUND
#include "SFXMgr.h"
#include "GameClientShell.h"
#include "GraphFX.h"

class GraphPoints
{
public:

	GraphPoints()
		: data(0),
	      numPoints(0),
		  maxPoints(0)	{}

	void RecordData(int16 * pSixteenData,  uint32 dwNumElements, uint32 dwCurElement)
	{
		RecordData( pSixteenData, NULL, dwNumElements, dwCurElement);
	}

	void RecordData(int8 * pEightData, uint32 dwNumElements, uint32 dwCurElement)
	{
		RecordData( pEightData, dwNumElements, dwCurElement);
	}


	void RecordData(int16 * pSixteenData, int8 * pEightData, uint32 dwNumElements, uint32 dwCurElement)
	{
		const int points_to_display = 500;
		const int interval = 10;

		if( !data )
		{
			delete [] data;
			maxPoints = points_to_display;
			data = new GraphFXPoint[points_to_display];
			ASSERT(data);
			if( !data ) return;
		}

		numPoints = 0;

		if( pEightData )
		{
			for( int8 * iter = pEightData + dwCurElement - points_to_display*interval/2, j = 0; 
			     iter < pEightData + dwCurElement + points_to_display*interval/2 && iter < pEightData + dwNumElements; 
				 iter += interval, ++j)
			{

				data[j] = GraphFXPoint( LTFLOAT(abs(*iter))/LTFLOAT(256/2) );

				++numPoints;
			}	 
		}
		else if( pSixteenData )
		{
			for( int16 * iter = pSixteenData + dwCurElement - points_to_display*interval/2, j = 0; 
			     iter < pSixteenData + dwCurElement + points_to_display*interval/2 && iter < pSixteenData + dwNumElements; 
				 iter += interval, ++j)
			{

				data[j] = GraphFXPoint( LTFLOAT(abs(*iter))/LTFLOAT(65536/2) );

				++numPoints;
			}	 
		}
	}

	LTBOOL GetData(GraphFXPoint * * PtrToData, uint32 * PtrToNumPoints)
	{
		*PtrToData = data;
		*PtrToNumPoints = numPoints;

		return data != NULL;
	}

private:


	GraphFXPoint * data;
	uint32   numPoints;
	uint32   maxPoints;

};


static GraphPoints g_GraphPoints;
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackRotationFn
//
//	PURPOSE:	Hack method of converting time to radians for rotations
//
// ----------------------------------------------------------------------- //

LTFLOAT HackRotationFn(LTFLOAT fDistance, LTFLOAT fTimer, LTFLOAT fDuration)
{
	LTFLOAT fAmount;

	if ( fTimer/fDuration > .25f )
	{
		// Linear decrease after 1/4 of duration

		fAmount = (-4.0f/3.0f)*fTimer/fDuration + 4.0f/3.0f;
	}
	else
	{
		// Linear increase until 1/4 of duration

		fAmount = (float)sqrt(4.0f*fTimer/fDuration);
	}

	return fDistance*fAmount;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::CNodeController
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CNodeController::CNodeController()
{
	m_hServerObj		= LTNULL;
	m_eModelSkeleton	= (ModelSkeleton)0;			
	m_bLocalPlayer		= LTFALSE;						
	m_cNodeControls		= 0;
	m_cNodes			= 0;
	m_cRecoils			= 0;
	m_fCurLipSyncRot	= 0.0f;

	for ( int iRecoilTimer = 0 ; iRecoilTimer < MAX_RECOILS ; iRecoilTimer++ )
	{
		m_fRecoilTimers[iRecoilTimer] = 0.0f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::~CNodeController
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CNodeController::~CNodeController()
{
	if( m_hServerObj )
	{
		g_pLTClient->ModelNodeControl(m_hServerObj, LTNULL, LTNULL);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::Init
//
//	PURPOSE:	Initializes us
//
// ----------------------------------------------------------------------- //

LTBOOL CNodeController::Init(CCharacterFX* pCFX, BodyPropFX* pBPFX, LTBOOL bLocalPlayer)
{
	ASSERT(pCFX || pBPFX);
	if ( !pCFX && !pBPFX) return LTFALSE;

	if(!g_vtLipSyncMaxRot.IsInitted())
		g_vtLipSyncMaxRot.Init(g_pLTClient, "LipSyncMaxRot", NULL, 25.0f);
	if(!g_vtLipSyncFreq.IsInitted())
		g_vtLipSyncFreq.Init(g_pLTClient, "LipSyncFreq", NULL, 20.0f);

	if(!g_vtPitchSmoothRate.IsInitted())
		g_vtPitchSmoothRate.Init(g_pLTClient, "PitchSmoothRate", NULL, MATH_CIRCLE);
	if(!g_vtYawSmoothRate.IsInitted())
		g_vtYawSmoothRate.Init(g_pLTClient, "YawSmoothRate", NULL, MATH_CIRCLE);
	if(!g_vtStrafeSmoothRate.IsInitted())
		g_vtStrafeSmoothRate.Init(g_pLTClient, "StrafeSmoothRate", NULL, MATH_CIRCLE);

	if(!g_vtHeadFollowSpeed.IsInitted())
		g_vtHeadFollowSpeed.Init(g_pLTClient, "HeadFollowSpeed", NULL, MATH_PI);


	// reset all of our old data
	m_hServerObj		= LTNULL;
	m_eModelSkeleton	= (ModelSkeleton)0;			
	m_bLocalPlayer		= LTFALSE;						
	m_cNodeControls		= 0;
	m_cNodes			= 0;
	m_cRecoils			= 0;
	m_fCurLipSyncRot	= 0.0f;
	m_NodeIndex.clear();
	memset(m_aNodeControls, 0, MAX_NODECONTROLS*sizeof(NCSTRUCT));
	memset(m_aNodes, 0, MAX_NODES*sizeof(NSTRUCT));
	memset(m_fRecoilTimers, 0, MAX_RECOILS*sizeof(LTFLOAT));

	// Store our backpointer
	if(pCFX)
	{
		m_hServerObj		= pCFX->GetServerObj();
		m_eModelSkeleton	= pCFX->GetModelSkeleton();
	}
	else
	{
		m_hServerObj		= pBPFX->GetServerObj();
		m_eModelSkeleton	= pBPFX->GetModelSkeleton();
	}


	// Record our player info
	m_bLocalPlayer = bLocalPlayer;


	// Map all the nodes in our skeleton in the bute file to the nodes in the actual .abc model file
	int iNode = 0;
	m_cNodes = 0;

	HMODELNODE hCurNode = INVALID_MODEL_NODE;
	while ( g_pLTClient->GetNextModelNode(m_hServerObj, hCurNode, &hCurNode) == LT_OK)
	{ 
		ASSERT(m_cNodes < MAX_NODES);

		if(m_cNodes >= MAX_NODES)
			break;

		char szName[64] = "";
		g_pLTClient->GetModelNodeName(m_hServerObj, hCurNode, szName, 64);

		ModelNode eModelNode = g_pModelButeMgr->GetSkeletonNode(m_eModelSkeleton, szName);

		if ( eModelNode != eModelNodeInvalid )
		{
			m_aNodes[eModelNode].eModelNode = eModelNode;
			m_aNodes[eModelNode].hModelNode = hCurNode;

			m_NodeIndex[hCurNode] = &m_aNodes[eModelNode];
		}

		m_cNodes++;
	}


	// Reset the controls
	Reset();


	// Find our "rotor" nodes
//	int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(m_eModelSkeleton);

//	for ( iNode = 0 ; iNode < cNodes ; iNode++ )
//	{	
//		if ( NODEFLAG_ROTOR & g_pModelButeMgr->GetSkeletonNodeFlags(m_eModelSkeleton, (ModelNode)iNode) )
//		{
//			AddNodeControlRotationTimed((ModelNode)iNode, LTVector(0,1,0), 40000.0f, 20000.0f);
//		}
//	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::Term
//
//	PURPOSE:	Clears out all affects of node controller
//
// ----------------------------------------------------------------------- //

void CNodeController::Term()
{
	Reset();

	if( m_hServerObj )
	{
		g_pLTClient->ModelNodeControl(m_hServerObj, LTNULL, LTNULL);
	}

	m_hServerObj = LTNULL;
	m_eModelSkeleton	= (ModelSkeleton)0;			
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::Reset
//
//	PURPOSE:	Resets all the node control settings
//
// ----------------------------------------------------------------------- //

void CNodeController::Reset()
{
	for(int iCtrl = 0; iCtrl < MAX_NODECONTROLS; iCtrl++)
	{
		m_aNodeControls[iCtrl].bValid = LTFALSE;
	}

	for(int iNode = 0; iNode < MAX_NODES; iNode++)
	{
		m_aNodes[iNode].cControllers = 0;
	}

	m_cNodeControls = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::ResetControlType
//
//	PURPOSE:	Resets all the node control settings for a particular type
//
// ----------------------------------------------------------------------- //

void CNodeController::ResetControlType(Control eType)
{
	for(int iCtrl = 0; iCtrl < MAX_NODECONTROLS; iCtrl++)
	{
		if((m_aNodeControls[iCtrl].eControl == eType) && m_aNodeControls[iCtrl].bValid)
		{
			m_aNodeControls[iCtrl].bValid = LTFALSE;
			RemoveNodeControl(iCtrl);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::Update
//
//	PURPOSE:	Updates all our Node Controls
//
// ----------------------------------------------------------------------- //

void CNodeController::Update()
{
	if( !m_hServerObj )
		return;

	// Check to see if we are controlling any nodes, and en/dis-able the callback accordingly

	ASSERT(m_cNodeControls >= 0);
	ASSERT(m_cNodeControls < MAX_NODECONTROLS);

	if ( m_cNodeControls > 0 )
	{
		g_pLTClient->ModelNodeControl(m_hServerObj, CNodeController::NodeControlFn, this);
	}
	else
	{
		g_pLTClient->ModelNodeControl(m_hServerObj, LTNULL, LTNULL);
	}


	// Reset all our nodes matrices

	for ( int iNode = 0 ; iNode < MAX_NODES ; iNode++ )
	{
		// Only reset the matrix if our control refcount is greater than zero (ie we are being controlled)
		if ( m_aNodes[iNode].cControllers > 0 )
		{
			m_aNodes[iNode].matTransform.Identity();
		}
	}

	// Update all our active node controls. 

	int cValidNodeControls = 0;
	int cNodeControls = m_cNodeControls;

	for ( int iNodeControl = 0 ; iNodeControl < MAX_NODECONTROLS && cValidNodeControls < cNodeControls ; iNodeControl++ )
	{
		ASSERT( const unsigned int(&m_aNodeControls[iNodeControl]) != 0xFDFDFDFD );

		if ( m_aNodeControls[iNodeControl].bValid )
		{
			// Update the node control

			cValidNodeControls++;

			switch ( m_aNodeControls[iNodeControl].eControl )
			{
				case eControlRotationTimed:
					UpdateRotationTimed(&m_aNodeControls[iNodeControl]);
					break;

				case eControlLipSync:
					UpdateLipSyncControl(&m_aNodeControls[iNodeControl]);
					break;

				case eControlScript:
					UpdateScriptControl(&m_aNodeControls[iNodeControl]);
					break;

				case eControlHeadFollowObj:
					UpdateHeadFollowObjControl(&m_aNodeControls[iNodeControl]);
					break;

				case eControlHeadFollowPos:
					UpdateHeadFollowPosControl(&m_aNodeControls[iNodeControl]);
					break;

				case eControlAimingPitch:
					UpdateAimingPitchControl(&m_aNodeControls[iNodeControl]);
					break;

				case eControlAimingYaw:
					UpdateAimingYawControl(&m_aNodeControls[iNodeControl]);
					break;

				case eControlStrafeYaw:
					UpdateStrafeYawControl(&m_aNodeControls[iNodeControl]);
					break;

				default:
					ASSERT(LTFALSE);
					m_aNodeControls[iNodeControl].bValid = LTFALSE;
					break;
			}

			// Check to see if the node control is done

			if ( !m_aNodeControls[iNodeControl].bValid )
			{
				RemoveNodeControl(iNodeControl);
			}
		}
	}

	// Update all our recoil timers

	for ( int iRecoilTimer = 0 ; iRecoilTimer < m_cRecoils ; iRecoilTimer++ )
	{
		m_fRecoilTimers[iRecoilTimer] -= g_pLTClient->GetFrameTime();

		if ( m_fRecoilTimers[iRecoilTimer] <= 0.0f )
		{
			m_cRecoils--;
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::UpdateRotationTimed
//
//	PURPOSE:	Updates a RotationTimed node control
//
// ----------------------------------------------------------------------- //

void CNodeController::UpdateRotationTimed(NCSTRUCT* pNodeControl)
{
	LTRotation rRot(0.0f, 0.0f, 0.0f, 1.0f);
	LTransform transform;
	LTMatrix matRotate;

//	g_pLTClient->CPrint("Rotating %s around <%f,%f,%f> by %f radians", g_pModelButeMgr->GetSkeletonNodeName(GetCFX()->GetModelSkeleton(), pNodeControl->eModelNode), 
//		pNodeControl->vRotationAxis.x, pNodeControl->vRotationAxis.y, pNodeControl->vRotationAxis.z,
//		pNodeControl->fnRotationFunction(pNodeControl->fRotationDistance, pNodeControl->fRotationTimer, pNodeControl->fRotationDuration));

	// Rotate the node around that axis
	Mat_SetupRot(&matRotate, &pNodeControl->vRotationAxis, 
		pNodeControl->fnRotationFunction(pNodeControl->fRotationDistance, pNodeControl->fRotationTimer, pNodeControl->fRotationDuration));

	// Apply the rotation to the node
	m_aNodes[pNodeControl->eModelNode].matTransform = matRotate * m_aNodes[pNodeControl->eModelNode].matTransform;
	m_bUpdated = true;

	// Update our timing info
	LTFLOAT fRotationTime = g_pLTClient->GetFrameTime();
	pNodeControl->fRotationTimer += fRotationTime;

	if ( pNodeControl->fRotationTimer >= pNodeControl->fRotationDuration )
	{
		// We're done

		pNodeControl->bValid = LTFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::UpdateSoundControl
//
//	PURPOSE:	Updates a sound node control
//
// ----------------------------------------------------------------------- //

void CNodeController::UpdateLipSyncControl(NCSTRUCT *pNodeControl)
{
	// Make sure the sound handle is valid and check to see if the sound is done...
	if(!pNodeControl->hLipSyncSound || g_pLTClient->IsDone(pNodeControl->hLipSyncSound))
	{
		// Turn off captions
		g_pInterfaceMgr->StopCaptions();

		// Since we requested the handle, we have to kill the sound ourself!
		g_pLTClient->KillSound(pNodeControl->hLipSyncSound);

		// Reset the lip-syncing variables so that we'll be ready for the next lip-syncing event.
		pNodeControl->hLipSyncSound = LTNULL;
		pNodeControl->pSixteenBitBuffer = LTNULL;
		pNodeControl->pEightBitBuffer = LTNULL;
		pNodeControl->dwSamplesPerSecond = 0;
		pNodeControl->bValid = LTFALSE;
		return;
	}
		

	//
	// Process the current sound data (average over sound amplitude).
	//

	LTFLOAT fAverage = 0.0f;  // this will hold the average, normalized from 0.0f to 1.0f.


	ASSERT( pNodeControl->pSixteenBitBuffer || pNodeControl->pEightBitBuffer );


	// Average over the data.  We do an average of the data from the current point
	// being played to 1/g_vtLipSyncFreq.GetFloat() seconds ahead of that point.
	uint32 dwOffset = 0;
	uint32 dwSize   = 0;	

	if(	LT_OK == g_pLTClient->GetSoundOffset(pNodeControl->hLipSyncSound, &dwOffset, &dwSize) )
	{
		// Determine the end of the data we wish to average over.
		const uint32 dwDivisor = uint32(g_vtLipSyncFreq.GetFloat());
		uint32 dwOffsetEnd = dwOffset + pNodeControl->dwSamplesPerSecond/dwDivisor;
		if( dwOffsetEnd > dwSize )
			dwOffsetEnd = dwSize;


		// Accumulate the the amplitudes for the average.
		uint32 dwMaxAmplitude = 0;

		uint32 dwNumSamples = 0;
		uint32 dwAccum = 0;

		if( pNodeControl->pSixteenBitBuffer )
		{
			for( int16 * pIterator = pNodeControl->pSixteenBitBuffer + dwOffset; 
			     pIterator < pNodeControl->pSixteenBitBuffer + dwOffsetEnd; 
				 ++pIterator)
			{
				dwAccum += abs(*pIterator);
				++dwNumSamples;
			}	 
			
			dwMaxAmplitude = 65536/2;

			#ifdef GRAPH_LIPSYNC_SOUND		
				g_GraphPoints.RecordData(pNodeControl->pSixteenBitBuffer,dwSize,dwOffset);
			#endif

		}
		else if( pNodeControl->pEightBitBuffer )
		{
			for( int8 * pIterator = pNodeControl->pEightBitBuffer + dwOffset; 
			     pIterator < pNodeControl->pEightBitBuffer + dwOffsetEnd; 
				 ++pIterator)
			{
				dwAccum += abs(*pIterator);
				++dwNumSamples;
			}	 
			
			dwMaxAmplitude = 256/2;

			#ifdef GRAPH_LIPSYNC_SOUND		
				g_GraphPoints.RecordData(pNodeControl->pEightBitBuffer,dwSize,dwOffset);
			#endif
		}

		// And find the average!
		if( dwNumSamples > 0 )
			fAverage = LTFLOAT(dwAccum) / LTFLOAT(dwNumSamples) / LTFLOAT(dwMaxAmplitude);

	} //if(	LT_OK == g_pLTClient->GetSoundOffset(pNodeControl->hLipSyncSound, &dwOffset, &dwSize) )

	//
	// Do the rotation.
	//
	MathLT *pMathLT = g_pLTClient->GetMathLT();

	LTRotation rRot;
	ROT_INIT(rRot);
	LTVector vAxis(0.0f, 0.0f, 1.0f);
	LTFLOAT fMaxRot = MATH_DEGREES_TO_RADIANS(g_vtLipSyncMaxRot.GetFloat());

	// Calculate the rotation.
	m_fCurLipSyncRot =  fAverage*fMaxRot;

//	g_pLTClient->CPrint("LipSyncRot: %f", m_fCurLipSyncRot);

	pMathLT->RotateAroundAxis(rRot, vAxis, -m_fCurLipSyncRot);

	// Create a rotation matrix and apply it to the current offset matrix
	LTMatrix m1;
	pMathLT->SetupRotationMatrix(m1, rRot);
	m_aNodes[pNodeControl->eModelNode].matTransform = m_aNodes[pNodeControl->eModelNode].matTransform * m1;
	m_bUpdated = true;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::UpdateScriptControl
//
//	PURPOSE:	Updates a script node control
//
// ----------------------------------------------------------------------- //

void CNodeController::UpdateScriptControl(NCSTRUCT *pNodeControl)
{
	LTFLOAT fTime = g_pLTClient->GetTime() - pNodeControl->fScriptTime;
	ModelNScript eScript = (ModelNScript)pNodeControl->nScript;
	int nCurPt = pNodeControl->nCurrentScriptPt;
	int nLastPt = pNodeControl->nLastScriptPt;

//	g_pLTClient->CPrint("Running node script!");

	// Check to see if the script is over... and make it invalid if so
	if(fTime > g_pModelButeMgr->GetNScriptPtTime(eScript, nLastPt))
	{
//		g_pLTClient->CPrint("Ending node script!");

		pNodeControl->bValid = LTFALSE;
		return;
	}

	// Get access to the controls...
	MathLT *pMathLT = g_pLTClient->GetMathLT();

	// Get the current script point
	while(fTime > g_pModelButeMgr->GetNScriptPtTime(eScript, nCurPt))
		nCurPt++;

	// Calculate the scale value from the last position to the current
	LTFLOAT fPtTime1 = g_pModelButeMgr->GetNScriptPtTime(eScript, nCurPt - 1);
	LTFLOAT fPtTime2 = g_pModelButeMgr->GetNScriptPtTime(eScript, nCurPt);
	LTFLOAT fScale = (fTime - fPtTime1) / (fPtTime2 - fPtTime1);

	//----------------------------------------------------------------------------
	// Setup the initial position vector
	LTVector vPos;

	// Get a direction vector from the last position to the current
	LTVector vPosDir = g_pModelButeMgr->GetNScriptPtPosOffset(eScript, nCurPt) - g_pModelButeMgr->GetNScriptPtPosOffset(eScript, nCurPt - 1);
	VEC_MULSCALAR(vPosDir, vPosDir, fScale);
	vPos = g_pModelButeMgr->GetNScriptPtPosOffset(eScript, nCurPt - 1) + vPosDir;

	//----------------------------------------------------------------------------
	// Setup the initial rotation vector
	LTVector vRot;

	// Get a direction rotation from the last position to the current
	LTVector vRotDir = g_pModelButeMgr->GetNScriptPtRotOffset(eScript, nCurPt) - g_pModelButeMgr->GetNScriptPtRotOffset(eScript, nCurPt - 1);
	VEC_MULSCALAR(vRotDir, vRotDir, fScale);
	vRot = g_pModelButeMgr->GetNScriptPtRotOffset(eScript, nCurPt - 1) + vRotDir;

	// Calculate the rotation...
	LTRotation rRot;
	LTVector vR, vU, vF;

	ROT_INIT(rRot);
	pMathLT->GetRotationVectors(rRot, vR, vU, vF);

	pMathLT->RotateAroundAxis(rRot, vR, vRot.x);
	pMathLT->RotateAroundAxis(rRot, vU, vRot.y);
	pMathLT->RotateAroundAxis(rRot, vF, vRot.z);

	//----------------------------------------------------------------------------
	// Setup the matrix using the offset position and rotation
	LTMatrix m1;
	pMathLT->SetupTransformationMatrix(m1, vPos, rRot);
	m_aNodes[pNodeControl->eModelNode].matTransform = m_aNodes[pNodeControl->eModelNode].matTransform * m1;
	m_bUpdated = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::UpdateHeadFollowObjControl
//
//	PURPOSE:	Updates a follow object node control
//
// ----------------------------------------------------------------------- //

void CNodeController::UpdateHeadFollowObjControl(NCSTRUCT *pNodeControl)
{
	ModelLT *pModelLT = g_pLTClient->GetModelLT();
	TransformLT *pTransformLT = g_pLTClient->GetTransformLT();

	//----------------------------------------------------------------------
	// Get information about the follow object position...
	LTVector vObjPos(0.0f, 0.0f, 0.0f);


	if(pNodeControl->hFollowObj)
	{
		if(pNodeControl->hFollowObjNode == INVALID_MODEL_NODE)
		{
			// Just get the position of the follow object if the follow node is invalid
			g_pLTClient->GetObjectPos(pNodeControl->hFollowObj, &vObjPos);
		}
		else
		{
			LTransform transform;

			// Get the transform of the node we're following
			LTRESULT rResult = pModelLT->GetNodeTransform(pNodeControl->hFollowObj, pNodeControl->hFollowObjNode, transform, LTTRUE);

			if( rResult == LT_OK )
			{
				// Decompose the transform into the position and rotation
				pTransformLT->GetPos(transform, vObjPos);
			}
			else
			{
				ASSERT( 0 );
			}
		}
	}


	// Turn the follow control off if the expire time has past
	if(pNodeControl->fFollowExpireTime <= 0.0f)
	{
		pNodeControl->fFollowExpireTime = 0.0f;
		pNodeControl->bFollowOn = LTFALSE;
	}
	else
		pNodeControl->fFollowExpireTime -= g_pLTClient->GetFrameTime();


	RotateHeadToPos( pNodeControl, vObjPos );
}

	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::RotateHeadToPos
//
//	PURPOSE:	Actually rotates the head in the direction of a specific point.
//
//				NOTE! -- These calculations only work for head rotation of
//				characters aligned with the world X/Z plane.
//
// ----------------------------------------------------------------------- //

void CNodeController::RotateHeadToPos(NCSTRUCT *pNodeControl, const LTVector & vObjPos)
{
	LTVector vSocketPos;
	LTRotation rNodeRot, rSocketRot, rNewRot;
	LTransform trans;

	LTVector vJunkU, vJunkR, vJunkF;

	LTVector vNR, vNU, vNF;
	LTVector vSR, vSU, vSF;

	LTVector vGlobalAxis, vLocalAxis;
	LTFLOAT fRD, fUD, fFD;

	HMODELSOCKET hSocket;
	LTVector vDir;
	LTFLOAT fAngle, fFullYaw;

	const LTFloatPt & fHeadYawRange   = g_pModelButeMgr->GetSkeletonHeadTurnYawRange( m_eModelSkeleton );
	const LTFloatPt & fHeadPitchRange = g_pModelButeMgr->GetSkeletonHeadTurnPitchRange( m_eModelSkeleton );

	LTFLOAT fRotOffset = pNodeControl->fFollowRate * g_pLTClient->GetFrameTime();


	rNewRot.Init();
	fAngle = 0.0f;
	fFullYaw = 0.0f;


	//----------------------------------------------------------------------
	// Get information about the control node...

	// Get access to the controls...
	ModelLT *pModelLT = g_pLTClient->GetModelLT();
	TransformLT *pTransformLT = g_pLTClient->GetTransformLT();

	// Setting m_bUpdated to true will cause the engine to re-cache its node transforms.  This
	// is needed to get an accurate head socket rotation.
	m_bUpdated = true;

	// Get the reference socket so that we know what directions we need to rotate.
	// We will use the socket's position, and forward orientation.
	// We will use the model's up orientation.  The right vector will be derived from forward and up.
	pModelLT->GetSocket(m_hServerObj, "Head", hSocket);
	pModelLT->GetSocketTransform(m_hServerObj, hSocket, trans, LTTRUE);
	pTransformLT->Get(trans, vSocketPos, rSocketRot);
	g_pMathLT->GetRotationVectors(rSocketRot, vSR, vSU, vSF);

	LTRotation rModelRot;
	LTVector vModelPos;
	LTVector vModelF;
	g_pLTClient->GetObjectPos(m_hServerObj, &vModelPos);
	g_pLTClient->GetObjectRotation(m_hServerObj, &rModelRot);
	g_pMathLT->GetRotationVectors(rModelRot, vJunkR, vSU, vModelF);

	vSF -= vSU*vSU.Dot(vSF);
	vSF.Norm();

	// If that failed, use the model's rotation for our forward as well.
	if( vSF.MagSqr() < 0.5f )
	{
		vSF = vModelF;
	}

	vSR = vSF.Cross(vSU);
	vSR.Norm();
	

	//----------------------------------------------------------------------
	// Calculate the global yaw rotation axis
	g_pMathLT->GetRotationVectorsFromMatrix(m_aNodes[pNodeControl->eModelNode].origMatTransform, vNR, vNU, vNF);

	vGlobalAxis.Init(0.0f, 1.0f, 0.0f);

	fRD = vNR.Dot(vGlobalAxis);
	fUD = vNU.Dot(vGlobalAxis);
	fFD = vNF.Dot(vGlobalAxis);

	vLocalAxis.Init(fRD, fUD, fFD);
	vLocalAxis.Norm();


	//----------------------------------------------------------------------
	// Figure out our destination yaw angle

	if(pNodeControl->bFollowOn)
	{
		vDir = vObjPos - vSocketPos;
		vDir -= vSU*vSU.Dot(vDir);
		vDir.Norm();

		fFD = vSF.Dot(vDir);
		fRD = vSR.Dot(vDir);

		if(fFD < -1.0f) fFD = -1.0f;
		else if(fFD > 1.0f) fFD = 1.0f;

		fAngle = (LTFLOAT)acos(fFD);

		if(fRD < 0.0f)
		{
			fFullYaw = -fAngle;
			fAngle = -fAngle;
		}
		else
		{
			fFullYaw = fAngle;
			fAngle = fAngle;
		}

		// If the angle is beyond the hard maxes, just leave it big so that the
		// head won't try to rotate to that angle.  This prevents the head from
		// oscillating back and forth when the object is directly behind them.
		if( fAngle > s_fHardHeadMinYaw && fAngle < s_fHardHeadMaxYaw )
		{
			fAngle = LTCLAMP(fAngle, fHeadYawRange.x, fHeadYawRange.y);
		}
	}
	else
	{
		fAngle = 0.0f;
	}


	//----------------------------------------------------------------------
	// Smooth the angle transition

	if( fAngle >= fHeadYawRange.x && fAngle <= fHeadYawRange.y )
	{
		if(fAngle < pNodeControl->vFollowAngles.y)
		{
			pNodeControl->vFollowAngles.y -= fRotOffset;
			if(pNodeControl->vFollowAngles.y < fAngle) pNodeControl->vFollowAngles.y = fAngle;
		}
		else if(fAngle > pNodeControl->vFollowAngles.y)
		{
			pNodeControl->vFollowAngles.y += fRotOffset;
			if(pNodeControl->vFollowAngles.y > fAngle) pNodeControl->vFollowAngles.y = fAngle;
		}
	}

	g_pMathLT->RotateAroundAxis(rNewRot, vLocalAxis, pNodeControl->vFollowAngles.y);


	//----------------------------------------------------------------------
	// Calculate a new global pitch axis

	LTRotation rOldSocketRot = rSocketRot;

	g_pMathLT->RotateAroundAxis(rSocketRot, vSU, fAngle);
	g_pMathLT->GetRotationVectors(rSocketRot, vJunkR, vJunkU, vSF);

	vSF -= vSU*vSU.Dot(vSF);
	vSF.Norm();

	// If that failed, use the model's rotation for our forward as well.
	if( vSF.MagSqr() < 0.5f )
	{
		vSF = vModelF;
	}

	vSR = vSF.Cross(vSU);
	vSR.Norm();

	fRD = vNR.Dot(vSR);
	fUD = vNU.Dot(vSR);
	fFD = vNF.Dot(vSR);

	vLocalAxis.Init(fRD, fUD, fFD);
	vLocalAxis.Norm();


	//----------------------------------------------------------------------
	// Figure out our destination pitch angle

	if(pNodeControl->bFollowOn)
	{
		rSocketRot = rOldSocketRot;

		g_pMathLT->RotateAroundAxis(rSocketRot, vSU, fFullYaw);
		g_pMathLT->GetRotationVectors(rSocketRot, vJunkR, vJunkU, vSF);

		vSF -= vSU*vSU.Dot(vSF);
		vSF.Norm();

		// If that failed, use the model's rotation for our forward as well.
		if( vSF.MagSqr() < 0.5f )
		{
			vSF = vModelF;
		}

		// Warning : vSR is invalid!

		vDir = vObjPos - vSocketPos;
		vDir.Norm();

		fFD = vSF.Dot(vDir);
		fUD = vSU.Dot(vDir);

		if(fFD < 0.95f)
		{
			if(fFD < -1.0f) fFD = -1.0f;
			else if(fFD > 1.0f) fFD = 1.0f;

			fAngle = (LTFLOAT)acos(fFD);

			if(fUD > 0.0f)
				fAngle = -fAngle;
			else
				fAngle = fAngle;

			fAngle = LTCLAMP(fAngle, fHeadPitchRange.x, fHeadPitchRange.y);
		}
		else
		{
			fAngle = 0.0f;
		}
	}
	else
	{
		fAngle = 0.0f;
	}


	//----------------------------------------------------------------------
	// Smooth the angle transition

	if(fAngle < pNodeControl->vFollowAngles.x)
	{
		pNodeControl->vFollowAngles.x -= fRotOffset;
		if(pNodeControl->vFollowAngles.x < fAngle) pNodeControl->vFollowAngles.x = fAngle;
	}
	else if(fAngle > pNodeControl->vFollowAngles.x)
	{
		pNodeControl->vFollowAngles.x += fRotOffset;
		if(pNodeControl->vFollowAngles.x > fAngle) pNodeControl->vFollowAngles.x = fAngle;
	}

	g_pMathLT->RotateAroundAxis(rNewRot, vLocalAxis, pNodeControl->vFollowAngles.x);



//	g_pLTClient->CPrint("Head rotation angles!  Object %d  --  Yaw: %f  --  Pitch: %f", m_hServerObj, pNodeControl->vFollowAngles.y, pNodeControl->vFollowAngles.x);


	//----------------------------------------------------------------------
	// Create a rotation matrix and apply it to the current offset matrix

	LTMatrix m1;
	g_pMathLT->SetupRotationMatrix(m1, rNewRot);
	m_aNodes[pNodeControl->eModelNode].matTransform = m_aNodes[pNodeControl->eModelNode].matTransform * m1;
	m_bUpdated = true;


	//----------------------------------------------------------------------
	// Check to see if we should end this control

	if(!pNodeControl->bFollowOn && !pNodeControl->vFollowAngles.x && !pNodeControl->vFollowAngles.y)
	{
		pNodeControl->bValid = LTFALSE;
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::UpdateHeadFollowPosControl
//
//	PURPOSE:	Updates a follow position node control
//
// ----------------------------------------------------------------------- //

void CNodeController::UpdateHeadFollowPosControl(NCSTRUCT *pNodeControl)
{
	RotateHeadToPos( pNodeControl, pNodeControl->vFollowPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::UpdateAimingPitchControl
//
//	PURPOSE:	Updates a the aiming pitch control
//
// ----------------------------------------------------------------------- //

void CNodeController::UpdateAimingPitchControl(NCSTRUCT *pNodeControl)
{
	MathLT *pMathLT = g_pLTClient->GetMathLT();

	// Get information about local node rotation
	LTRotation rRot;
	LTVector vR, vU, vF;
	LTransform transform;

	// Get access to the controls...
	ModelLT *pModelLT = g_pLTClient->GetModelLT();
	TransformLT *pTransformLT = g_pLTClient->GetTransformLT();

	// Get the transform of the node we're controlling
	pModelLT->GetNodeTransform(m_hServerObj, m_aNodes[pNodeControl->eModelNode].hModelNode, transform, LTFALSE);

	// Grab the rotation from the transform
	pTransformLT->GetRot(transform, rRot);
	g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);


	// Find the offsets of the global axis with respect to the local vectors
	LTVector vGlobalAxis(1.0f, 0.0f, 0.0f);
	LTFLOAT fRD, fUD, fFD;

	fRD = vR.Dot(vGlobalAxis);
	fUD = vU.Dot(vGlobalAxis);
	fFD = vF.Dot(vGlobalAxis);

	// Calculate the local axis that represents our global axis
	LTVector vLocalAxis(fRD, fUD, fFD);
	vLocalAxis.Norm();


	// Calculate the rotation.
	LTIntPt ptRange = g_pModelButeMgr->GetSkeletonNodeAimPitchRange(m_eModelSkeleton, pNodeControl->eModelNode, pNodeControl->nPitchSet);
	LTFLOAT fRotAmount = 0.0f;

	// Smooth out the rotation
	if(pNodeControl->fPitch > pNodeControl->fCurPitch)
	{
		pNodeControl->fCurPitch += g_pLTClient->GetFrameTime() * g_vtPitchSmoothRate.GetFloat();

		if(pNodeControl->fCurPitch > pNodeControl->fPitch)
			pNodeControl->fCurPitch = pNodeControl->fPitch;
	}
	else if(pNodeControl->fPitch < pNodeControl->fCurPitch)
	{
		pNodeControl->fCurPitch -= g_pLTClient->GetFrameTime() * g_vtPitchSmoothRate.GetFloat();

		if(pNodeControl->fCurPitch < pNodeControl->fPitch)
			pNodeControl->fCurPitch = pNodeControl->fPitch;
	}

	if(pNodeControl->fCurPitch > 0.0f)
		fRotAmount = MATH_DEGREES_TO_RADIANS(pNodeControl->fCurPitch * ptRange.x);
	else
		fRotAmount = MATH_DEGREES_TO_RADIANS(pNodeControl->fCurPitch * ptRange.y);

	rRot.Init();
	pMathLT->RotateAroundAxis(rRot, vLocalAxis, fRotAmount);


	// Create a rotation matrix and apply it to the current offset matrix
	LTMatrix m1;
	pMathLT->SetupRotationMatrix(m1, rRot);
	m_aNodes[pNodeControl->eModelNode].matTransform = m_aNodes[pNodeControl->eModelNode].matTransform * m1;
	m_bUpdated = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::UpdateAimingYawControl
//
//	PURPOSE:	Updates a the aiming yaw control
//
// ----------------------------------------------------------------------- //

void CNodeController::UpdateAimingYawControl(NCSTRUCT *pNodeControl)
{
	MathLT *pMathLT = g_pLTClient->GetMathLT();

	// Get information about local node rotation
	LTRotation rRot;
	LTVector vR, vU, vF;

	// Get access to the controls...
	ModelLT *pModelLT = g_pLTClient->GetModelLT();
	TransformLT *pTransformLT = g_pLTClient->GetTransformLT();


	//----------------------------------------------------------------------
	// Calculate the rotation axis

	// Get the transform of the node we're controlling
	g_pMathLT->GetRotationVectorsFromMatrix(m_aNodes[pNodeControl->eModelNode].origMatTransform, vR, vU, vF);


	// Find the offsets of the global axis with respect to the local vectors
	LTVector vGlobalAxis(0.0f, 1.0f, 0.0f);

	const LTFLOAT fRD = vR.Dot(vGlobalAxis);
	const LTFLOAT fUD = vU.Dot(vGlobalAxis);
	const LTFLOAT fFD = vF.Dot(vGlobalAxis);

	LTVector vLocalAxis(fRD, fUD, fFD);
	vLocalAxis.Norm();


	//----------------------------------------------------------------------
	// Figure out the rotation angle

	LTFLOAT fObjYaw = GetObjectYaw(m_hServerObj);
	LTFLOAT fRotAmount = fObjYaw - pNodeControl->fCurYaw;

	if(fRotAmount)
	{
		if(fRotAmount <= -MATH_PI)
		{
			fRotAmount += MATH_CIRCLE;
		}
		else if(fRotAmount >= MATH_PI)
		{
			fRotAmount -= MATH_CIRCLE;
		}

		if(fRotAmount > s_fMaxAimingYaw)
		{
			pNodeControl->fCurYaw = fObjYaw - s_fMaxAimingYaw;
		}
		else if(fRotAmount < -s_fMaxAimingYaw)
		{
			pNodeControl->fCurYaw = fObjYaw + s_fMaxAimingYaw;
		}

		ClampAngle360(pNodeControl->fCurYaw);
	}


	//----------------------------------------------------------------------
	// Interpolate the angle

	LTFLOAT fOffset = pNodeControl->fYaw - pNodeControl->fCurYaw;

	if(fOffset)
	{
		if((fOffset <= -MATH_PI) || ((fOffset > 0.0f) && (fOffset <= MATH_PI)))
		{
			pNodeControl->fCurYaw += g_pLTClient->GetFrameTime() * g_vtYawSmoothRate.GetFloat();

			while(pNodeControl->fCurYaw >= MATH_CIRCLE)
				pNodeControl->fCurYaw -= MATH_CIRCLE;

			if(pNodeControl->fCurYaw > pNodeControl->fYaw)
				pNodeControl->fCurYaw = pNodeControl->fYaw;
		}
		else if((fOffset > MATH_PI) || ((fOffset < 0.0f) && (fOffset > -MATH_PI)))
		{
			pNodeControl->fCurYaw -= g_pLTClient->GetFrameTime() * g_vtYawSmoothRate.GetFloat();

			while(pNodeControl->fCurYaw < 0.0f)
				pNodeControl->fCurYaw += MATH_CIRCLE;

			if(pNodeControl->fCurYaw < pNodeControl->fYaw)
				pNodeControl->fCurYaw = pNodeControl->fYaw;
		}
	}


	//----------------------------------------------------------------------
	// Create a rotation matrix and apply it to the current offset matrix

	fRotAmount = pNodeControl->fCurYaw - fObjYaw;

	rRot.Init();
	pMathLT->RotateAroundAxis(rRot, vLocalAxis, fRotAmount);

	LTMatrix m1;
	pMathLT->SetupRotationMatrix(m1, rRot);
	m_aNodes[pNodeControl->eModelNode].matTransform = m_aNodes[pNodeControl->eModelNode].matTransform * m1;
	m_bUpdated = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::UpdateStrafeYawControl
//
//	PURPOSE:	Updates a the strafe yaw control
//
// ----------------------------------------------------------------------- //

void CNodeController::UpdateStrafeYawControl(NCSTRUCT *pNodeControl)
{
	MathLT *pMathLT = g_pLTClient->GetMathLT();

	// Get information about local node rotation
	LTRotation rRot;
	LTVector vR, vU, vF;
	LTransform transform;

	// Get access to the controls...
	ModelLT *pModelLT = g_pLTClient->GetModelLT();
	TransformLT *pTransformLT = g_pLTClient->GetTransformLT();

	// Get the transform of the node we're controlling
	pModelLT->GetNodeTransform(m_hServerObj, m_aNodes[pNodeControl->eModelNode].hModelNode, transform, LTFALSE);

	// Grab the rotation from the transform
	pTransformLT->GetRot(transform, rRot);
	g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);


	// Find the offsets of the global axis with respect to the local vectors
	LTVector vGlobalAxis(0.0f, 1.0f, 0.0f);
	LTFLOAT fRD, fUD, fFD;

	fRD = vR.Dot(vGlobalAxis);
	fUD = vU.Dot(vGlobalAxis);
	fFD = vF.Dot(vGlobalAxis);

	// Calculate the local axis that represents our global axis
	LTVector vLocalAxis(fRD, fUD, fFD);
	vLocalAxis.Norm();


	// Calculate our velocity
	LTVector vObjPos, vVel;

	g_pLTClient->GetObjectPos(m_hServerObj, &vObjPos);
	vVel = vObjPos - pNodeControl->vLastStrafePos;
	pNodeControl->vLastStrafePos = vObjPos;

	LTRotation rObjRot;

	g_pLTClient->GetObjectRotation(m_hServerObj, &rObjRot);
	g_pMathLT->GetRotationVectors(rObjRot, vR, vU, vF);


	// Project the velocity to the character's x/z plane
	LTFLOAT fDist = vU.Dot(vVel);
	vVel -= (vU * fDist);

	if(vVel.MagSqr() <= 1.0f)
		vVel = vF;
	else
		vVel.Norm();


	// Calculate the rotation.
	LTIntPt ptRange = g_pModelButeMgr->GetSkeletonNodeStrafeYawRange(m_eModelSkeleton, pNodeControl->eModelNode, pNodeControl->nStrafeSet);
	LTFLOAT fRotAmount = vR.Dot(vVel);

	if(vF.Dot(vVel) < -0.05f)
		fRotAmount = -fRotAmount;

	// Smooth out the rotation
	if(fRotAmount > pNodeControl->fLastStrafeAngle)
	{
		pNodeControl->fLastStrafeAngle += g_pLTClient->GetFrameTime() * g_vtStrafeSmoothRate.GetFloat();
		
		if(pNodeControl->fLastStrafeAngle > fRotAmount)
			pNodeControl->fLastStrafeAngle = fRotAmount;
	}
	else if(fRotAmount < pNodeControl->fLastStrafeAngle)
	{
		pNodeControl->fLastStrafeAngle -= g_pLTClient->GetFrameTime() * g_vtStrafeSmoothRate.GetFloat();
		
		if(pNodeControl->fLastStrafeAngle < fRotAmount)
			pNodeControl->fLastStrafeAngle = fRotAmount;
	}

	if(pNodeControl->fLastStrafeAngle > 0.0f)
		fRotAmount = MATH_DEGREES_TO_RADIANS(pNodeControl->fLastStrafeAngle * ptRange.y);
	else
		fRotAmount = MATH_DEGREES_TO_RADIANS(pNodeControl->fLastStrafeAngle * ptRange.x);

	rRot.Init();
	pMathLT->RotateAroundAxis(rRot, vLocalAxis, fRotAmount);


	// Create a rotation matrix and apply it to the current offset matrix
	LTMatrix m1;
	pMathLT->SetupRotationMatrix(m1, rRot);
	m_aNodes[pNodeControl->eModelNode].matTransform = m_aNodes[pNodeControl->eModelNode].matTransform * m1;
	m_bUpdated = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControlMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_*_MSG from the server
//
// ----------------------------------------------------------------------- //

LTBOOL CNodeController::HandleNodeControlMessage(uint8 byMsgId, HMESSAGEREAD hMessage)
{
	// Handle it...

	switch ( byMsgId )
	{
		case CFX_NODECONTROL_RECOIL_MSG:
		{
			HandleNodeControlRecoilMessage(hMessage);
			break;
		}

		case CFX_NODECONTROL_LIP_SYNC:
		{
			HandleNodeControlLipSyncMessage(hMessage);
			break;
		}

		case CFX_NODECONTROL_HEAD_FOLLOW_OBJ:
		{
			HandleNodeControlHeadFollowObjMessage(hMessage);
			break;
		}

		case CFX_NODECONTROL_HEAD_FOLLOW_POS:
		{
			HandleNodeControlHeadFollowPosMessage(hMessage);
			break;
		}

		case CFX_NODECONTROL_SCRIPT:
		{
			HandleNodeControlScriptMessage(hMessage);
			break;
		}

		case CFX_NODECONTROL_AIMING_PITCH:
		{
			HandleNodeControlAimingPitchMessage(hMessage);
			break;
		}

		case CFX_NODECONTROL_AIMING_YAW:
		{
			HandleNodeControlAimingYawMessage(hMessage);
			break;
		}

		case CFX_NODECONTROL_STRAFE_YAW:
		{
			HandleNodeControlStrafeYawMessage(hMessage);
			break;
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControlRecoilMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_RECOIL_MSG from the server
//
// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlRecoilMessage(HMESSAGEREAD hMessage)
{
/*	if ( m_cRecoils >= MAX_RECOILS )
		return;

	m_fRecoilTimers[m_cRecoils++] = 0.50f;


	ModelNode eModelNode;
	eModelNode = (ModelNode)g_pLTClient->ReadFromMessageByte(hMessage);

	LTVector vRecoilDir;
	g_pLTClient->ReadFromMessageCompVector(hMessage, &vRecoilDir);

	// Get the magnitude of the recoil vector

	LTFLOAT fRecoilMag = VEC_MAGSQR(vRecoilDir);

	// Get the unit impact/recoil vector

	vRecoilDir /= (float)sqrt(fRecoilMag);

	// Cap it if necessary

	if ( fRecoilMag > 100.0f )
	{
		fRecoilMag = 100.0f;
	}

	// Get the position of the impact

	NSTRUCT* pNode = &m_aNodes[eModelNode];
	ModelLT* pModelLT = g_pLTClient->GetModelLT();
	LTransform transform;
	pModelLT->GetNodeTransform(m_hServerObj, pNode->hModelNode, transform, LTTRUE);

	// Decompose the transform into the position and rotation

	LTVector vPos;
	TransformLT* pTransformLT = g_pLTClient->GetTransformLT();
	pTransformLT->GetPos(transform, vPos);

	LTVector vRecoilPos = vPos;

	// Add angular rotations up the recoil parent chain

	ModelNode eModelNodeCurrent = g_pModelButeMgr->GetSkeletonNodeRecoilParent(GetCFX()->GetModelSkeleton(), eModelNode);

	while ( eModelNodeCurrent != eModelNodeInvalid )
	{
		// Get the rotation of the node 
		
		NSTRUCT* pNode = &m_aNodes[eModelNodeCurrent];

		LTransform transform;
		ModelLT* pModelLT = g_pLTClient->GetModelLT();

		// Get the transform of the node we're controlling

		pModelLT->GetNodeTransform(m_hServerObj, pNode->hModelNode, transform, LTTRUE);

		TransformLT* pTransformLT = g_pLTClient->GetTransformLT();

		// Decompose the transform into the position and rotation

		LTVector vPos;
		LTRotation rRot;
		pTransformLT->Get(transform, vPos, rRot);

		// Get the rotation vectors of the transform

		LTVector vRight, vUp, vForward;
		g_pLTClient->GetRotationVectors(&rRot, &vUp, &vRight, &vForward);

		// Cross the right vector with the impact vector to get swing

		LTVector vRotationAxis = vRight.Cross(vRecoilDir);
		vRotationAxis.Norm();

		// Add the timed rotation control for the swing

		// !!! HACK
		// !!! Do not add swing if this is a leg node

		if ( !strstr(g_pModelButeMgr->GetSkeletonNodeName(GetCFX()->GetModelSkeleton(), eModelNodeCurrent), "leg") )
		AddNodeControlRotationTimed(eModelNodeCurrent, vRotationAxis, MATH_PI/1000.0f*fRecoilMag, 0.50f);

		// Use the right vector to get twist, but make sure the sign is correct based on location
		// of impact and whether we're getting shot at from behind/front etc

		vRotationAxis = vRight;
		vRotationAxis.Norm();

		// Get the twist

		LTVector vSideDir = vRecoilPos-vPos;
		vSideDir.Norm();

		LTFLOAT fSign = vUp.Dot(vRecoilDir);
		fSign *= vForward.Dot(vSideDir);

		if ( fSign > 0.0f )
		{
			vRotationAxis = -vRotationAxis;
		}

		// Add the timed rotation control for the twist

	//	AddNodeControlRotationTimed(eModelNodeCurrent, vRotationAxis, MATH_PI/1000.0f*fRecoilMag, 0.50f);

		// Decrease the magnitude

		fRecoilMag /= 2.0f;
		
		eModelNodeCurrent = g_pModelButeMgr->GetSkeletonNodeRecoilParent(GetCFX()->GetModelSkeleton(), eModelNodeCurrent);
	}
*/
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControlLipSyncMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_LIP_SYNC from the server
//
// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlLipSyncMessage(HMESSAGEREAD hMessage)
{
//	ModelNode eModelNode = (ModelNode)g_pLTClient->ReadFromMessageByte(hMessage);
	HSTRING hSound = g_pLTClient->ReadFromMessageHString(hMessage);
	LTFLOAT fRadius = g_pLTClient->ReadFromMessageFloat(hMessage);
	uint32 dwFlags = g_pLTClient->ReadFromMessageDWord(hMessage);

	ModelNode eModelNode = g_pModelButeMgr->GetSkeletonNode(m_eModelSkeleton, "LowerJaw");

	// See if we are already controlling the jaw node
	int iCtrl = FindNodeControl(eModelNode, eControlLipSync);

	char szSound[128];
	strcpy(szSound, g_pLTClient->GetStringData(hSound));
	g_pLTClient->FreeString(hSound);

	// Check to make sure all the info is ok...
	if((eModelNode == eModelNodeInvalid) || fRadius <= 0.0f)
	{
		if(iCtrl > -1)
		{
			if(m_aNodeControls[iCtrl].hLipSyncSound)
			{
				// Turn off captions
				g_pInterfaceMgr->StopCaptions();

				g_pLTClient->KillSound(m_aNodeControls[iCtrl].hLipSyncSound);
				m_aNodeControls[iCtrl].hLipSyncSound = LTNULL;
				m_aNodeControls[iCtrl].pSixteenBitBuffer = LTNULL;
				m_aNodeControls[iCtrl].pEightBitBuffer = LTNULL;
				m_aNodeControls[iCtrl].dwSamplesPerSecond = 0;
			}

			m_aNodeControls[iCtrl].bValid = LTFALSE;
		}

		return;
	}

	// Add the node control structure...
	int iNodeControl = (iCtrl > -1) ? iCtrl : AddNodeControl();

	ASSERT(iNodeControl >= 0);
	if ( iNodeControl < 0 )
		return;

	if(m_aNodeControls[iNodeControl].hLipSyncSound)
	{
		// Turn off captions
		g_pInterfaceMgr->StopCaptions();

		g_pLTClient->KillSound(m_aNodeControls[iNodeControl].hLipSyncSound);
		m_aNodeControls[iNodeControl].hLipSyncSound = LTNULL;
		m_aNodeControls[iCtrl].pSixteenBitBuffer = LTNULL;
		m_aNodeControls[iCtrl].pEightBitBuffer = LTNULL;
		m_aNodeControls[iCtrl].dwSamplesPerSecond = 0;
	}

	m_aNodeControls[iNodeControl].bValid = LTTRUE;
	m_aNodeControls[iNodeControl].eControl = eControlLipSync;
	m_aNodeControls[iNodeControl].eModelNode = eModelNode;

	// Start the sound playing.
	m_aNodeControls[iNodeControl].hLipSyncSound = PlayLipSyncSound(szSound, fRadius, dwFlags);

	// If the sound didn't exist... turn the control invalid and leave
	if(!m_aNodeControls[iNodeControl].hLipSyncSound)
	{
		m_aNodeControls[iNodeControl].bValid = LTFALSE;
		return;
	}

	// Get the sound buffer.
	uint32 dwChannels = 0;
	if( LT_OK != g_pLTClient->GetSoundData( m_aNodeControls[iNodeControl].hLipSyncSound, 
											m_aNodeControls[iNodeControl].pSixteenBitBuffer,
											m_aNodeControls[iNodeControl].pEightBitBuffer, 
											&m_aNodeControls[iNodeControl].dwSamplesPerSecond, 
											&dwChannels ) )
	{
		g_pLTClient->CPrint("Cannot lip-sync to sound \"%s\".", szSound);

		// Turn off captions
		g_pInterfaceMgr->StopCaptions();

		// Since we requested the handle, we have to kill the sound ourself!
		g_pLTClient->KillSound(m_aNodeControls[iNodeControl].hLipSyncSound);

		// Reset the lip-syncing variables so that we'll be ready for the next lip-syncing event.
		m_aNodeControls[iNodeControl].hLipSyncSound = LTNULL;
		m_aNodeControls[iNodeControl].pSixteenBitBuffer = LTNULL;
		m_aNodeControls[iNodeControl].pEightBitBuffer = LTNULL;
		m_aNodeControls[iNodeControl].dwSamplesPerSecond = 0;
		m_aNodeControls[iNodeControl].bValid = LTFALSE;
		return;
	}

	ASSERT( dwChannels == 1);  
	// If you want to use multi-channel sounds (why would you?), you'll need to
	//  to account for the interleaving of channels in the following code.

	
	// Increment the number of controllers for this node...
	m_aNodes[eModelNode].cControllers++;

	bool bIsDialogue = ( (strstr(szSound,"dialogue\\") != LTNULL) || (strstr(szSound,"Dialogue\\") != LTNULL) );

	if (bIsDialogue)
	{
		LTVector speakerPos;
		LTFLOAT	fDuration;

		g_pLTClient->GetSoundDuration(m_aNodeControls[iNodeControl].hLipSyncSound,&fDuration);
		g_pLTClient->GetObjectPos(m_hServerObj,&speakerPos);
		// Start Captioning
		g_pInterfaceMgr->SetCaption(szSound,speakerPos,fRadius, fDuration );
	}

	
	
	// Show graph over character.
#ifdef GRAPH_LIPSYNC_SOUND
	GCREATESTRUCT graph_init;
	graph_init.hServerObj = m_pCharacterFX->GetServerObj();
	graph_init.m_vOffsetPos = LTVector(0.0f,70.0f,0.0f);
	graph_init.m_fWidth = 60.0f;
	graph_init.m_fHeight = 60.0f;
	graph_init.m_bIgnoreX = LTTRUE;
	graph_init.m_UpdateGraphCallback 
		= make_callback2<LTBOOL,GraphFXPoint * *, uint32 *>(g_GraphPoints,GraphPoints::GetData);

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	psfxMgr->CreateSFX(SFX_GRAPH_ID, &graph_init);
#endif


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControlScriptMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_SCRIPT from the server
//
// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlScriptMessage(HMESSAGEREAD hMessage)
{
	uint8 nNumScripts = g_pLTClient->ReadFromMessageByte(hMessage);

	LTFLOAT fTime = g_pLTClient->GetTime();

	for(uint8 i = 0; i < nNumScripts; i++)
	{
		uint8 nScript = g_pLTClient->ReadFromMessageByte(hMessage);
		ModelNode eModelNode = g_pModelButeMgr->GetSkeletonNode(m_eModelSkeleton, g_pModelButeMgr->GetNScriptNodeName((ModelNScript)nScript));

		// Add the node control structure...
		int iNodeControl = AddNodeControl();

		ASSERT(iNodeControl >= 0);
		if ( iNodeControl < 0 || g_pModelButeMgr->GetNScriptNumPts((ModelNScript)nScript) < 2)
			continue;

		m_aNodeControls[iNodeControl].bValid = LTTRUE;
		m_aNodeControls[iNodeControl].eControl = eControlScript;
		m_aNodeControls[iNodeControl].eModelNode = eModelNode;

		m_aNodeControls[iNodeControl].nScript = nScript;
		m_aNodeControls[iNodeControl].fScriptTime = fTime;
		m_aNodeControls[iNodeControl].nCurrentScriptPt = 0;
		m_aNodeControls[iNodeControl].nLastScriptPt = g_pModelButeMgr->GetNScriptNumPts((ModelNScript)nScript) - 1;

		// Increment the number of controllers for this node...
		m_aNodes[eModelNode].cControllers++;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControlHeadFollowObjMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_HEAD_FOLLOW_OBJ from the server
//
// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlHeadFollowObjMessage(HMESSAGEREAD hMessage)
{
	HOBJECT hObj = g_pLTClient->ReadFromMessageObject(hMessage);
	HSTRING hFollowObjNode = g_pLTClient->ReadFromMessageHString(hMessage);
	LTBOOL bOn = g_pLTClient->ReadFromMessageByte(hMessage);

	ModelNode eModelNode = g_pModelButeMgr->GetSkeletonNode(m_eModelSkeleton, "Head_node");

	// See if we are already controlling the head node
	int iCtrl = FindNodeControl(eModelNode, eControlHeadFollowObj);
	if(iCtrl == -1) 
		iCtrl = FindNodeControl(eModelNode, eControlHeadFollowPos);

	char szFollowObjNode[64];
	strcpy(szFollowObjNode, g_pLTClient->GetStringData(hFollowObjNode));
	g_pLTClient->FreeString(hFollowObjNode);

	// Check to make sure all the info is ok...
	if(eModelNode == eModelNodeInvalid)
	{
		if(iCtrl > -1)
			m_aNodeControls[iCtrl].bValid = LTFALSE;

		return;
	}

	// Add the node control structure...
	int iNodeControl = (iCtrl > -1) ? iCtrl : AddNodeControl();
	ASSERT(iNodeControl >= 0);
	if ( iNodeControl < 0 )
		return;

	m_aNodeControls[iNodeControl].bValid = LTTRUE;
	m_aNodeControls[iNodeControl].eControl = eControlHeadFollowObj;
	m_aNodeControls[iNodeControl].eModelNode = eModelNode;

	m_aNodeControls[iNodeControl].hFollowObj = hObj;
	m_aNodeControls[iNodeControl].hFollowObjNode = INVALID_MODEL_NODE;
	m_aNodeControls[iNodeControl].fFollowRate = g_vtHeadFollowSpeed.GetFloat();
	m_aNodeControls[iNodeControl].fFollowExpireTime = 100000.0f;  // Infinite...
	m_aNodeControls[iNodeControl].bFollowOn = bOn;


	// Grab the current matrix of the node
	LTVector vNodePos;
	LTRotation rNodeRot;
	LTransform trans;

	// Get access to the controls...
	ModelLT *pModelLT = g_pLTClient->GetModelLT();
	TransformLT *pTransformLT = g_pLTClient->GetTransformLT();

	// Get the transform of the node we're controlling
	pModelLT->GetNodeTransform(m_hServerObj, m_aNodes[eModelNode].hModelNode, trans, LTFALSE);
	pTransformLT->Get(trans, vNodePos, rNodeRot);

	g_pMathLT->SetupRotationMatrix(m_aNodes[eModelNode].origMatTransform, rNodeRot);


	// Find the node in the follow object...
	if(szFollowObjNode)
	{
		HMODELNODE hCurNode = INVALID_MODEL_NODE;

		while(g_pLTClient->GetNextModelNode(hObj, hCurNode, &hCurNode) == LT_OK)
		{ 
			char szName[64] = "";
			g_pLTClient->GetModelNodeName(hObj, hCurNode, szName, 64);

			if(!strcmp(szName, szFollowObjNode))
			{
				m_aNodeControls[iNodeControl].hFollowObjNode = hCurNode;
				break;
			}
		}
	}

	// Be sure we are notified if this object is removed!
	if (hObj)
	{
		DDWORD dwCFlags = g_pLTClient->GetObjectClientFlags(hObj);
		g_pLTClient->SetObjectClientFlags(hObj, dwCFlags | CF_NOTIFYREMOVE);
	}

	// Increment the number of controllers for this node...
	m_aNodes[eModelNode].cControllers++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControlHeadFollowPosMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_HEAD_FOLLOW_POS from the server
//
// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlHeadFollowPosMessage(HMESSAGEREAD hMessage)
{
	LTVector vPos;
	g_pLTClient->ReadFromMessageVector(hMessage, &vPos);
	LTBOOL bOn = g_pLTClient->ReadFromMessageByte(hMessage);

	ModelNode eModelNode = g_pModelButeMgr->GetSkeletonNode(m_eModelSkeleton, "Head_node");

	// See if we are already controlling the head node
	int iCtrl = FindNodeControl(eModelNode, eControlHeadFollowObj);
	if(iCtrl == -1) iCtrl = FindNodeControl(eModelNode, eControlHeadFollowPos);

	// Check to make sure all the info is ok...
	if(eModelNode == eModelNodeInvalid)
	{
		if(iCtrl > -1)
			m_aNodeControls[iCtrl].bValid = LTFALSE;

		return;
	}

	// Add the node control structure...
	int iNodeControl = (iCtrl > -1) ? iCtrl : AddNodeControl();
	ASSERT(iNodeControl >= 0);
	if ( iNodeControl < 0 )
		return;

	m_aNodeControls[iNodeControl].bValid = LTTRUE;
	m_aNodeControls[iNodeControl].eControl = eControlHeadFollowPos;
	m_aNodeControls[iNodeControl].eModelNode = eModelNode;

	m_aNodeControls[iNodeControl].vFollowPos = vPos;
	m_aNodeControls[iNodeControl].fFollowRate = g_vtHeadFollowSpeed.GetFloat();
	m_aNodeControls[iNodeControl].fFollowExpireTime = 100000.0f;  // Infinite...
	m_aNodeControls[iNodeControl].bFollowOn = bOn;

	// Increment the number of controllers for this node...
	m_aNodes[eModelNode].cControllers++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControlAimingPitchMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_AIMING_PITCH from the server
//
// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlAimingPitchMessage(HMESSAGEREAD hMessage)
{
	int8 nValue = (int8)g_pLTClient->ReadFromMessageByte(hMessage);
	uint8 nSet = (uint8)g_pLTClient->ReadFromMessageByte(hMessage);

	LTFLOAT fAimPitch = ((LTFLOAT)nValue) / 100.0f;

	int nAimPitchNodes = g_pModelButeMgr->GetNumAimPitchNodes(m_eModelSkeleton, nSet);
	const ModelNode* pAimPitchNodes = g_pModelButeMgr->GetAimPitchNodes(m_eModelSkeleton, nSet);


	// Reset all the control of this type and start over...
	ResetControlType(eControlAimingPitch);


	for(int i = 0; i < nAimPitchNodes; i++)
	{
		ModelNode eModelNode = pAimPitchNodes[i];

		// Add the node control structure...
		int iNodeControl = AddNodeControl();
		ASSERT(iNodeControl >= 0);
		if ( iNodeControl < 0 )
			return;

		m_aNodeControls[iNodeControl].bValid = LTTRUE;
		m_aNodeControls[iNodeControl].eControl = eControlAimingPitch;
		m_aNodeControls[iNodeControl].eModelNode = eModelNode;

		m_aNodeControls[iNodeControl].fPitch = fAimPitch;
		m_aNodeControls[iNodeControl].nPitchSet = nSet;

		// Increment the number of controllers for this node...
		m_aNodes[eModelNode].cControllers++;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControlAimingYawMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_AIMING_YAW from the server
//
// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlAimingYawMessage(HMESSAGEREAD hMessage)
{
	LTFLOAT fGlobalYaw = g_pLTClient->ReadFromMessageFloat(hMessage);

	ModelNode eModelNode = g_pModelButeMgr->GetSkeletonNode(m_eModelSkeleton, "Torso_node");

	// See if we are already controlling the aiming yaw
	int iCtrl = FindNodeControl(eModelNode, eControlAimingYaw);

	// Check to make sure all the info is ok...
	if(eModelNode == eModelNodeInvalid)
	{
		if(iCtrl > -1)
			m_aNodeControls[iCtrl].bValid = LTFALSE;

		return;
	}

	// Add the node control structure...
	int iNodeControl = (iCtrl > -1) ? iCtrl : AddNodeControl();
	ASSERT(iNodeControl >= 0);
	if ( iNodeControl < 0 )
		return;

	m_aNodeControls[iNodeControl].bValid = LTTRUE;
	m_aNodeControls[iNodeControl].eControl = eControlAimingYaw;
	m_aNodeControls[iNodeControl].eModelNode = eModelNode;

	m_aNodeControls[iNodeControl].fYaw = MATH_DEGREES_TO_RADIANS(fGlobalYaw);

	// Increment the number of controllers for this node...
	m_aNodes[eModelNode].cControllers++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControlStrafeYawMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_STRAFE_YAW from the server
//
// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlStrafeYawMessage(HMESSAGEREAD hMessage)
{
	if( !m_hServerObj )
		return;

	uint8 nSet = (uint8)g_pLTClient->ReadFromMessageByte(hMessage);

	int nStrafeYawNodes = g_pModelButeMgr->GetNumStrafeYawNodes(m_eModelSkeleton, nSet);
	const ModelNode* pStrafeYawNodes = g_pModelButeMgr->GetStrafeYawNodes(m_eModelSkeleton, nSet);


	// Reset all the control of this type and start over...
	ResetControlType(eControlStrafeYaw);


	for(int i = 0; i < nStrafeYawNodes; i++)
	{
		ModelNode eModelNode = pStrafeYawNodes[i];

		// Add the node control structure...
		int iNodeControl = AddNodeControl();
		ASSERT(iNodeControl >= 0);
		if ( iNodeControl < 0 )
			return;

		m_aNodeControls[iNodeControl].bValid = LTTRUE;
		m_aNodeControls[iNodeControl].eControl = eControlStrafeYaw;
		m_aNodeControls[iNodeControl].eModelNode = eModelNode;

		g_pLTClient->GetObjectPos(m_hServerObj, &m_aNodeControls[iNodeControl].vLastStrafePos);
		m_aNodeControls[iNodeControl].nStrafeSet = nSet;

		// Increment the number of controllers for this node...
		m_aNodes[eModelNode].cControllers++;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::OnObjectRemove
//
//	PURPOSE:	Turns off any controllers which are referencing this object.
//
// ----------------------------------------------------------------------- //

void CNodeController::OnObjectRemove(HOBJECT hObj)
{
	for(NCSTRUCT * pCtrl = m_aNodeControls; pCtrl < m_aNodeControls + MAX_NODECONTROLS; pCtrl++)
	{
		if( pCtrl->bValid )
		{
			if( pCtrl->hFollowObj == hObj )
			{
				pCtrl->bValid = LTFALSE;
			}
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::FindNode
//
//	PURPOSE:	Finds the node struct given a node handle
//
// ----------------------------------------------------------------------- //

NSTRUCT* CNodeController::FindNode(HMODELNODE hModelNode)
{
	NodeIndexTable::iterator iter = m_NodeIndex.find(hModelNode);

	if( iter != m_NodeIndex.end() )
		return iter->second;

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::FindNodeControl
//
//	PURPOSE:	Finds the node control struct given a node id
//
// ----------------------------------------------------------------------- //

int CNodeController::FindNodeControl(ModelNode eNode, Control eControl)
{
	for(int i = 0; i < MAX_NODECONTROLS; i++)
	{
		if(m_aNodeControls[i].bValid && m_aNodeControls[i].eModelNode == eNode && m_aNodeControls[i].eControl == eControl)
			return i;
	}
	
	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::AddNodeControl
//
//	PURPOSE:	Adds a node control (returns the index of a free one)
//
// ----------------------------------------------------------------------- //

int CNodeController::AddNodeControl()
{
	for ( int iNodeControl = 0 ; iNodeControl < MAX_NODECONTROLS ; iNodeControl++ )
	{
		if ( !m_aNodeControls[iNodeControl].bValid )
		{
			m_cNodeControls++;
			return iNodeControl;
		}
	}

	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::AddNodeControlRotationTimed
//
//	PURPOSE:	Adds a timed rotation acceleration node control
//
// ----------------------------------------------------------------------- //

void CNodeController::AddNodeControlRotationTimed(ModelNode eModelNode, const LTVector& vAxis, LTFLOAT fDistance, LTFLOAT fDuration)
{
	int iNodeControl = AddNodeControl();
	ASSERT(iNodeControl >= 0);
	if ( iNodeControl < 0 ) return;

	m_aNodeControls[iNodeControl].bValid = LTTRUE;
	m_aNodeControls[iNodeControl].eControl = eControlRotationTimed;
	m_aNodeControls[iNodeControl].eModelNode = eModelNode;

	m_aNodeControls[iNodeControl].vRotationAxis = vAxis;
	m_aNodeControls[iNodeControl].fRotationDistance = fDistance;
	m_aNodeControls[iNodeControl].fRotationDuration = fDuration;
	m_aNodeControls[iNodeControl].fRotationTimer = 0.0f;
	m_aNodeControls[iNodeControl].fnRotationFunction = HackRotationFn;

	m_aNodes[eModelNode].cControllers++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::RemoveNodeControl
//
//	PURPOSE:	Removes a node control 
//
// ----------------------------------------------------------------------- //

void CNodeController::RemoveNodeControl(int iNodeControl)
{
	ASSERT(!m_aNodeControls[iNodeControl].bValid);

	m_aNodes[m_aNodeControls[iNodeControl].eModelNode].cControllers--;

	m_cNodeControls--;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::NodeControlFn
//
//	PURPOSE:	Node control callback
//
// ----------------------------------------------------------------------- //

void CNodeController::NodeControlFn(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat, void *pUserData)
{
	ASSERT( hObj );

	CNodeController* pNC = (CNodeController*)pUserData;

	ASSERT(pNC);

	pNC->HandleNodeControl(hObj, hNode, pGlobalMat);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControl
//
//	PURPOSE:	Control the specified node (if applicable)
//
// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControl(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat)
{
	if( hNode != INVALID_MODEL_NODE )
	{
		NSTRUCT* pNode = FindNode(hNode);

		if ( pNode && pNode->cControllers > 0 )
		{
			pNode->origMatTransform = *pGlobalMat;
			*pGlobalMat = *pGlobalMat * const_cast<LTMatrix&>(pNode->matTransform);
		}
	}
	else
	{
		if( m_bUpdated ) 
		{
			pGlobalMat->Scale(5.0f, 5.0f, 5.0f);
			m_bUpdated = false;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::PlayLipSyncSound
//
//	PURPOSE:	Play a lip synced sound.
//
// ----------------------------------------------------------------------- //

HSOUNDDE CNodeController::PlayLipSyncSound(char* szSound, LTFLOAT fRadius, uint32 dwFlags)
{
	if (!szSound || !szSound[0] || fRadius <= 0.0f) return LTNULL;

	if( !m_hServerObj )
		return LTNULL;

	dwFlags |= PLAYSOUND_GETHANDLE;

	SoundPriority ePriority = SOUNDPRIORITY_AI_HIGH;
	if (m_bLocalPlayer)
	{
		ePriority = SOUNDPRIORITY_PLAYER_HIGH;
		dwFlags |= PLAYSOUND_LOCAL;
	}
	else
	{
		// This flag doesn't make sense for AIs or non-local players...
		dwFlags &= ~PLAYSOUND_CLIENTLOCAL;
	}

	// [KLS] Added 9/6/01 - Always playing cinematic dialogue in client's head 
	if (g_pGameClientShell->IsCinematic())
	{
		dwFlags |= PLAYSOUND_LOCAL;
	}
	// [KLS] End 9/6/01 changes

	HSOUNDDE hSound = g_pClientSoundMgr->PlaySoundFromObject(m_hServerObj, 
		szSound, fRadius, ePriority, dwFlags);

	return hSound;
}




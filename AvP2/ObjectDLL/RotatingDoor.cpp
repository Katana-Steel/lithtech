// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingDoor.CPP
//
// PURPOSE : A RotatingDoor object
//
// CREATED : 12/3/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "RotatingDoor.h"


BEGIN_CLASS(RotatingDoor)
	ADD_REALPROP_FLAG(MoveDist, 0.0f, PF_HIDDEN)	//  hide some of our
	ADD_VECTORPROP_FLAG(MoveDir, PF_HIDDEN)			//  parent's properties
	ADD_VECTORPROP_FLAG(RotationPoint, 0)			//  point to rotate around
	ADD_VECTORPROP_FLAG(RotationAngles, 0)			//  where to rotate to
	ADD_REALPROP(Speed, 50.0f)						//  movement speed
	ADD_BOOLPROP(BoxPhysics, DFALSE)

#ifdef _WIN32
	ADD_STRINGPROP_FLAG(OpenSound, "Sounds\\Doors\\Door_01.wav", PF_FILENAME)
	ADD_STRINGPROP_FLAG(CloseSound, "Sounds\\Doors\\Door_01.wav", PF_FILENAME)
	ADD_STRINGPROP_FLAG(LockedSound, "Sounds\\Doors\\metknock.wav", PF_FILENAME)
#else
	ADD_STRINGPROP_FLAG(OpenSound, "Sounds/Doors/Door_01.wav", PF_FILENAME)
	ADD_STRINGPROP_FLAG(CloseSound, "Sounds/Doors/Door_01.wav", PF_FILENAME)
	ADD_STRINGPROP_FLAG(LockedSound, "Sounds/Doors/metknock.wav", PF_FILENAME)
#endif

END_CLASS_DEFAULT(RotatingDoor, Door, NULL, NULL)





// ----------------------------------------------------------------------- //
// Global functions.
// ----------------------------------------------------------------------- //
void Door_CalcPosAndRot(
	MathLT *pMathLT,
	DVector vOriginalPos,	// Original object position.
	DVector vRotationPoint,	// Point to rotate around.
	DVector vAngles,		// Rotation angles.
	DVector &vOutPos,
	DRotation &rOutRot)
{
	DVector vDes, vPoint;
	DVector vOriginTranslation;
	DMatrix mMat;

	pMathLT->SetupEuler(rOutRot, vAngles.x, vAngles.y, vAngles.z);
	vPoint = vOriginalPos - vRotationPoint;

	pMathLT->SetupRotationMatrix(mMat, rOutRot);
	MatVMul_H(&vDes, &mMat, &vPoint);

	vOriginTranslation = vDes - vPoint;
	vOutPos = vOriginalPos + vOriginTranslation;
}

// ----------------------------------------------------------------------- //
// Sets up transforms for the hinged door.
// ----------------------------------------------------------------------- //
void SetupTransform_RotatingDoor(PreLightLT *pInterface, 
	HPREOBJECT hObject, 
	float fPercent,
	DVector &vOutPos, 
	DRotation &rOutRotation)
{
	DVector vStartAngles, vEndAngles, vAngles;
	DVector vRotationPoint, vOriginalPos;
	GenericProp gProp;


	pInterface->GetPropGeneric(hObject, "RotationAngles", &gProp);
	vStartAngles = gProp.m_Vec;

	pInterface->GetPropGeneric(hObject, "RotationPoint", &gProp);
	vRotationPoint = gProp.m_Vec;

	pInterface->GetWorldModelRuntimePos(hObject, vOriginalPos);

	vAngles = vStartAngles + (vEndAngles - vStartAngles) * fPercent;
	vAngles.x = MATH_DEGREES_TO_RADIANS(vAngles.x);
	vAngles.y = MATH_DEGREES_TO_RADIANS(vAngles.y);
	vAngles.z = MATH_DEGREES_TO_RADIANS(vAngles.z);

	Door_CalcPosAndRot(
		pInterface->GetMathLT(),
		vOriginalPos,
		vRotationPoint,
		vAngles,
		vOutPos,
		rOutRotation);		
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::RotatingDoor()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

RotatingDoor::RotatingDoor() : Door()
{
	m_vRotationPoint.Init();
	m_vRotPtOffset.Init();
	m_vRotationAngles.Init();
	m_vOpenAngles.Init();
	m_vClosedAngles.Init();
	m_vOriginalPos.Init();
	m_vOpenDir.Init();

	m_fPitch = 0.0f;
	m_fYaw	 = 0.0f;
	m_fRoll	 = 0.0f;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

DDWORD RotatingDoor::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch (messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

			DDWORD dwRet = Door::EngineMessageFn(messageID, pData, fData);

			if (fData == 1.0f)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			// Convert speeds from degrees to radians
			m_fSpeed		= MATH_DEGREES_TO_RADIANS(m_fSpeed);
			m_fClosingSpeed	= MATH_DEGREES_TO_RADIANS(m_fClosingSpeed);

			return dwRet;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DBYTE)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DBYTE)fData);
		}
		break;

		default : break;
	}

	return Door::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::ReadProp()
//
//	PURPOSE:	Reads RotatingDoor properties
//
// --------------------------------------------------------------------------- //

DBOOL RotatingDoor::ReadProp(ObjectCreateStruct *)
{
	g_pServerDE->GetPropVector("RotationAngles", &m_vRotationAngles);
	g_pServerDE->GetPropVector("RotationPoint", &m_vRotationPoint);

	DVector vTemp;
	g_pServerDE->GetPropRotationEuler("Rotation", &vTemp);

	m_fPitch = vTemp.x;
	m_fYaw	 = vTemp.y;
	m_fRoll	 = vTemp.z;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void RotatingDoor::InitialUpdate()
{
	// Set open/closed angle...

	VEC_SET(m_vClosedAngles, m_fPitch, m_fYaw, m_fRoll);
	m_vOpenAngles.x = MATH_DEGREES_TO_RADIANS(m_vRotationAngles.x) + m_fPitch;
	m_vOpenAngles.y = MATH_DEGREES_TO_RADIANS(m_vRotationAngles.y) + m_fYaw;
	m_vOpenAngles.z = MATH_DEGREES_TO_RADIANS(m_vRotationAngles.z) + m_fRoll;

	
	// Save the object's original position...

	g_pServerDE->GetObjectPos(m_hObject, &m_vOriginalPos);


	// Calculate the rotation point offset (allows for the
	// door to be movied (keyframed) and still rotate correctly...

	m_vRotPtOffset = m_vRotationPoint - m_vOriginalPos;


	// The door must rotate at least 1 degree...
	
	const DFLOAT c_fMinDelta = MATH_DEGREES_TO_RADIANS(1.0f);


	// Determine direction to open door in X...

	DFLOAT fOffset = m_vClosedAngles.x - m_vOpenAngles.x;

	if (fOffset > c_fMinDelta)
	{
		m_vOpenDir.x = -1.0f;
	}
	else if (fOffset < c_fMinDelta)
	{
		m_vOpenDir.x = 1.0f;
	}


	// Determine direction to open door in Y...

	fOffset = m_vClosedAngles.y - m_vOpenAngles.y;

	if (fOffset > c_fMinDelta)
	{
		m_vOpenDir.y = -1.0f;
	}
	else if (fOffset < c_fMinDelta)
	{
		m_vOpenDir.y = 1.0f;
	}


	// Determine direction to open door in Z...

	fOffset = m_vClosedAngles.z - m_vOpenAngles.z;

	if (fOffset > c_fMinDelta)
	{
		m_vOpenDir.z = -1.0f;
	}
	else if (fOffset < c_fMinDelta)
	{
		m_vOpenDir.z = 1.0f;
	}

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::SetOpening()
//
//	PURPOSE:	Sets the door opening state
//
// --------------------------------------------------------------------------- //

void RotatingDoor::SetOpening()
{
	// Recalcualte the original position.  This is incase the door was 
	// moved (e.g., was keyframed on an elevator)...

	if (m_dwDoorState == DOORSTATE_CLOSED)
	{
		g_pServerDE->GetObjectPos(m_hObject, &m_vOriginalPos);
	}

	Door::SetOpening();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::Opening()
//
//	PURPOSE:	Handles the RotatingDoor opening state
//
// --------------------------------------------------------------------------- //

void RotatingDoor::Opening()
{
	float fPercent;

	DBOOL bDoneInX = DFALSE;
	DBOOL bDoneInY = DFALSE;
	DBOOL bDoneInZ = DFALSE;


	DVector vOldAngles(m_fPitch, m_fYaw, m_fRoll);

	// Calculate new pitch, yaw, and roll...

	bDoneInX = CalcAngle(m_fPitch, m_vClosedAngles.x, m_vOpenAngles.x, m_vOpenDir.x, m_fSpeed);
	bDoneInY = CalcAngle(m_fYaw,   m_vClosedAngles.y, m_vOpenAngles.y, m_vOpenDir.y, m_fSpeed);
	bDoneInZ = CalcAngle(m_fRoll,  m_vClosedAngles.z, m_vOpenAngles.z, m_vOpenDir.z, m_fSpeed);

	// Update the light animation.
	fPercent = GetRotatingLightAnimPercent();
	ReallySetLightAnimPos(fPercent);

	
	// Rotate the object...

	DVector vPos;
	DRotation rRot;

	CalcPosAndRot(vPos, rRot);

	// Set the object's new rotation and position...

	g_pServerDE->RotateObject(m_hObject, &rRot);	
	g_pServerDE->MoveObject(m_hObject, &vPos);

	if (bDoneInX && bDoneInY && bDoneInZ)
	{
		SetOpen();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::Closing()
//
//	PURPOSE:	Handles the RotatingDoor closing state
//
// --------------------------------------------------------------------------- //

void RotatingDoor::Closing()
{
	float fPercent;

	DBOOL bDoneInX = DFALSE;
	DBOOL bDoneInY = DFALSE;
	DBOOL bDoneInZ = DFALSE;

	DVector vOldAngles(m_fPitch, m_fYaw, m_fRoll);

	// Calculate new pitch, yaw, and roll...

	bDoneInX = CalcAngle(m_fPitch, m_vOpenAngles.x, m_vClosedAngles.x, -m_vOpenDir.x, m_fClosingSpeed);
	bDoneInY = CalcAngle(m_fYaw,   m_vOpenAngles.y, m_vClosedAngles.y, -m_vOpenDir.y, m_fClosingSpeed);
	bDoneInZ = CalcAngle(m_fRoll,  m_vOpenAngles.z, m_vClosedAngles.z, -m_vOpenDir.z, m_fClosingSpeed);


	// Update the light animation.
	fPercent = GetRotatingLightAnimPercent();
	ReallySetLightAnimPos(fPercent);
	
	// Rotate the object...

	DVector vPos;
	DRotation rRot;

	CalcPosAndRot(vPos, rRot);

	// Set the object's new rotation and position...

	g_pServerDE->RotateObject(m_hObject, &rRot);	
	g_pServerDE->MoveObject(m_hObject, &vPos);

	if (bDoneInX && bDoneInY && bDoneInZ)
	{
		SetClosed();
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::FirstUpdate()
//
//	PURPOSE:	Do first update (after all objects have been loaded)...
//
// --------------------------------------------------------------------------- //

void RotatingDoor::FirstUpdate()
{
	// Check to see what state we want to start in...

	Door::FirstUpdate();

	if(m_dwStateFlags & DF_STARTOPEN)
	{
		m_fPitch	= m_vOpenAngles.x;
		m_fYaw		= m_vOpenAngles.y;
		m_fRoll		= m_vOpenAngles.z;

		DVector vPos;
		DRotation rRot;
		CalcPosAndRot(vPos, rRot);

		// Set the object's new rotation and position...

		g_pServerDE->RotateObject(m_hObject, &rRot);	
		g_pServerDE->TeleportObject(m_hObject, &vPos);

		SetOpen(LTTRUE);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::CalcPosAndRot()
//
//	PURPOSE:	Do the rotation calculation based on the current values of
//				m_fPitch, m_fYaw, and m_fRoll
//
// --------------------------------------------------------------------------- //

void RotatingDoor::CalcPosAndRot(DVector & vPos, DRotation & rRot)
{
	Door_CalcPosAndRot(
		g_pServerDE->GetMathLT(),
		m_vOriginalPos,
		m_vOriginalPos + m_vRotPtOffset /*m_vRotationPoint*/,
		DVector(m_fPitch, m_fYaw, m_fRoll),
		vPos, rRot);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::CalcAngle()
//
//	PURPOSE:	Calculate the new value of fAngle
//
// --------------------------------------------------------------------------- //

DBOOL RotatingDoor::CalcAngle(DFLOAT & fAngle, DFLOAT fInitial, DFLOAT fTarget, DFLOAT fDir, DFLOAT fSpeed)
{
	DBOOL bRet = DFALSE; // Are we at the target angle?

	DFLOAT fPercent = 1.0f - (fTarget - fAngle) / (fTarget - fInitial);
	DFLOAT fAmount  = GetDoorWaveValue(fSpeed, fPercent, m_dwWaveform) * g_pLTServer->GetFrameTime();

	// Calculate percentage moved so far...

	if (fDir != 0.0f)
	{
		if (fDir > 0.0f)
		{
			if (fAngle < fTarget)
			{
				fAngle += fAmount;
			}
			else
			{
				fAngle = fTarget;
				bRet   = DTRUE;
			}
		}
		else
		{
			if (fAngle > fTarget)
			{
				fAngle -= fAmount;
			}
			else
			{
				fAngle = fTarget;
				bRet   = DTRUE;
			}
		}
	}

	if (fDir != 0.0f)
	{
		if (fDir > 0.0f)
		{
			if (fAngle < fTarget)
			{
				fAngle += fAmount;
			}
			else
			{
				fAngle = fTarget;
				bRet   = DTRUE;
			}
		}
		else
		{
			if (fAngle > fTarget)
			{
				fAngle -= fAmount;
			}
			else
			{
				fAngle = fTarget;
				bRet   = DTRUE;
			}
		}
	}
	else
	{
		bRet = DTRUE;
	}

	return bRet;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::GetMoveTestPos()
//
//	PURPOSE:	Get the test position of the door
//
// --------------------------------------------------------------------------- //

DBOOL RotatingDoor::GetMoveTestPos(DVector & vTestPos)
{
	if (!m_hActivateObj) return DFALSE;

	DVector vFinalPos(0.0f, 0.0f, 0.0f);
	DFLOAT fSpeed = 0.0f;

	DVector vOldAngles(m_fPitch, m_fYaw, m_fRoll);

	switch (m_dwDoorState)
	{
		case DOORSTATE_CLOSING:
		{
			// Calculate new pitch, yaw, and roll...

			CalcAngle(m_fPitch, m_fPitch, m_vOpenAngles.x, m_vOpenDir.x, m_fSpeed);
			CalcAngle(m_fYaw,   m_fYaw,   m_vOpenAngles.y, m_vOpenDir.y, m_fSpeed);
			CalcAngle(m_fRoll,  m_fRoll,  m_vOpenAngles.z, m_vOpenDir.z, m_fSpeed);
		}
		break;

		case DOORSTATE_OPENING:
		{
			// Calculate new pitch, yaw, and roll...

			CalcAngle(m_fPitch, m_fPitch, m_vClosedAngles.x, -m_vOpenDir.x, m_fClosingSpeed);
			CalcAngle(m_fYaw,   m_fYaw,   m_vClosedAngles.y, -m_vOpenDir.y, m_fClosingSpeed);
			CalcAngle(m_fRoll,  m_fRoll,  m_vClosedAngles.z, -m_vOpenDir.z, m_fClosingSpeed);
		}
		break;

		default:
			return DFALSE;
		break;
	}

	// Rotate the object...

	DVector vPos;
	DRotation rRot;

	CalcPosAndRot(vTestPos, rRot);

	// Restore real angles...

	m_fPitch = vOldAngles.x;
	m_fYaw	 = vOldAngles.y;
	m_fRoll	 = vOldAngles.z;

	return DTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::ActivateObjectCollision()
//
//	PURPOSE:	Determine if the activate object would collide with the door if
//				the door were oriented in the test position
//
// --------------------------------------------------------------------------- //

DBOOL RotatingDoor::ActivateObjectCollision(DVector vTestPos)
{
	if (!m_hActivateObj) return DFALSE;

	DVector vObjPos, vDoorCurPos;
	g_pServerDE->GetObjectPos(m_hActivateObj, &vObjPos);
	g_pServerDE->GetObjectPos(m_hObject, &vDoorCurPos);


	// If the door's new position is farther away from the touch object 
	// then its current position, we'll assume there can't be a collision.

	DFLOAT fCurDist  = VEC_DISTSQR(vDoorCurPos, vObjPos);
	DFLOAT fTestDist = VEC_DISTSQR(vTestPos, vObjPos);

	if (fCurDist <= fTestDist)
	{ 
		return DFALSE;
	}
	
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void RotatingDoor::Save(HMESSAGEWRITE hWrite, DBYTE nType)
{
	if (!hWrite) return;

	g_pServerDE->WriteToMessageVector(hWrite, &m_vRotationAngles);	
	g_pServerDE->WriteToMessageVector(hWrite, &m_vRotationPoint);
	g_pServerDE->WriteToMessageVector(hWrite, &m_vRotPtOffset);
	g_pServerDE->WriteToMessageVector(hWrite, &m_vOpenAngles);	
	g_pServerDE->WriteToMessageVector(hWrite, &m_vClosedAngles);	
	g_pServerDE->WriteToMessageVector(hWrite, &m_vOriginalPos);	
	g_pServerDE->WriteToMessageVector(hWrite, &m_vOpenDir);	

	g_pServerDE->WriteToMessageFloat(hWrite, m_fPitch);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fYaw);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fRoll);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void RotatingDoor::Load(HMESSAGEREAD hRead, DBYTE nType)
{
	if (!hRead) return;

	g_pServerDE->ReadFromMessageVector(hRead, &m_vRotationAngles);	
	g_pServerDE->ReadFromMessageVector(hRead, &m_vRotationPoint);
	g_pServerDE->ReadFromMessageVector(hRead, &m_vRotPtOffset);
	g_pServerDE->ReadFromMessageVector(hRead, &m_vOpenAngles);	
	g_pServerDE->ReadFromMessageVector(hRead, &m_vClosedAngles);	
	g_pServerDE->ReadFromMessageVector(hRead, &m_vOriginalPos);	
	g_pServerDE->ReadFromMessageVector(hRead, &m_vOpenDir);	

	m_fPitch	= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw		= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fRoll		= g_pServerDE->ReadFromMessageFloat(hRead);
}


void RotatingDoor::SetLightAnimOpen()
{
}


void RotatingDoor::SetLightAnimClosed()
{
	ReallySetLightAnimPos(0.5f);
}


float RotatingDoor::GetRotatingLightAnimPercent()
{
	float fOpenPercent, fClosedPercent, fPercentOpen, fMaxDiff, fTestDiff;
	DDWORD iDim, iBestDim;

	// How far along are we?
	
	// Pick a dimension to use (we need a dimension it rotates in).
	iBestDim = 0;
	fMaxDiff = 0.0f;
	for(iDim=0; iDim < 3; iDim++)
	{
		fTestDiff = (float)fabs(m_vOpenAngles[iDim] - m_vClosedAngles[iDim]);
		if(fTestDiff > fMaxDiff)
		{
			iBestDim = iDim;
			fMaxDiff = fTestDiff;
		}
	}
	
	if(fMaxDiff < 0.001f)
	{
		return 0.0f;
	}

	// How close are we to being open?
	fPercentOpen = (GetCurAnglesDim(iBestDim) - m_vClosedAngles[iBestDim]) / 
		(m_vOpenAngles[iBestDim] - m_vClosedAngles[iBestDim]);

	fClosedPercent	= 0.5f;
	fOpenPercent	= 0.0f;   // m_bOpeningNormal ? 0.0f : 1.0f;

	return fClosedPercent + (fOpenPercent - fClosedPercent) * fPercentOpen;
}


float RotatingDoor::GetCurAnglesDim(DDWORD iDim)
{
	if(iDim == 0)
		return m_fPitch;
	else if(iDim == 1)
		return m_fYaw;
	else
		return m_fRoll;
}


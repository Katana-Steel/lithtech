// ----------------------------------------------------------------------- //
//
// MODULE  : HingedDoor.CPP
//
// PURPOSE : A HingedDoor object
//
// CREATED : 12/3/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HingedDoor.h"


BEGIN_CLASS(HingedDoor)
	ADD_BOOLPROP_FLAG(OpenAway, 1, 0)
	ADD_VECTORPROP_VAL_FLAG(RotationAngles, 0.0f, 90.0f, 0.0f, 0)//  Where to rotate to
	PROP_DEFINEGROUP(StateFlags, PF_GROUP4)
		ADD_BOOLPROP_FLAG(ActivateTrigger, DTRUE, PF_GROUP4)
		ADD_BOOLPROP_FLAG(StartOpen, DFALSE, PF_GROUP4)
		ADD_BOOLPROP_FLAG(TriggerClose, DTRUE, PF_GROUP4)
		ADD_BOOLPROP_FLAG(RemainsOpen, DTRUE, PF_GROUP4)
		ADD_BOOLPROP_FLAG(ForceMove, DFALSE, PF_GROUP4)

#ifdef _WIN32
	ADD_STRINGPROP_FLAG(OpenSound, "Sounds\\Doors\\Door_01.wav", PF_FILENAME)
	ADD_STRINGPROP_FLAG(CloseSound, "Sounds\\Doors\\Door_01.wav", PF_FILENAME)
	ADD_STRINGPROP_FLAG(LockedSound, "Sounds\\Doors\\metknock.wav", PF_FILENAME)
#else
	ADD_STRINGPROP_FLAG(OpenSound, "Sounds/Doors/Door_01.wav", PF_FILENAME)
	ADD_STRINGPROP_FLAG(CloseSound, "Sounds/Doors/Door_01.wav", PF_FILENAME)
	ADD_STRINGPROP_FLAG(LockedSound, "Sounds/Doors/metknock.wav", PF_FILENAME)
#endif

END_CLASS_DEFAULT(HingedDoor, RotatingDoor, NULL, NULL)




// ----------------------------------------------------------------------- //
// Sets up transforms for the hinged door.
// ----------------------------------------------------------------------- //
void SetupTransform_HingedDoor(PreLightLT *pInterface, 
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
	vEndAngles = -vStartAngles;

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
//	ROUTINE:	HingedDoor::HingedDoor()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

HingedDoor::HingedDoor() : RotatingDoor()
{
	m_bOpeningNormal = DTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	HingedDoor::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

DDWORD HingedDoor::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch (messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call ReadProp()

			DDWORD dwRet = RotatingDoor::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				DDWORD dwRet = RotatingDoor::EngineMessageFn(messageID, pData, fData);
				InitialUpdate();
				return dwRet;
			}
		}
		break;

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

	return RotatingDoor::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	HingedDoor::ReadProp()
//
//	PURPOSE:	Reads HingedDoor properties
//
// --------------------------------------------------------------------------- //

DBOOL HingedDoor::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->GetPropBool("OpenAway", &m_bOpenAway);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HingedDoor::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void HingedDoor::InitialUpdate()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Save original angles...

	m_vOriginalOpenAngles   = m_vOpenAngles;
	m_vOriginalOpenDir		= m_vOpenDir;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	HingedDoor::SetOpen()
//
//	PURPOSE:	Set the door to the closed state
//
// --------------------------------------------------------------------------- //

void HingedDoor::SetClosed(DBOOL bInitialize)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (!bInitialize)
	{
		// Reset the open angles / direction...
	
		m_vOpenAngles = m_vOriginalOpenAngles;
		m_vOpenDir	  = m_vOriginalOpenDir;
	}

	RotatingDoor::SetClosed(bInitialize);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	HingedDoor::SetOpening()
//
//	PURPOSE:	Set the door to the opening state
//
// --------------------------------------------------------------------------- //

void HingedDoor::SetOpening()
{
	// Recalcualte the original position.  This is incase the door was 
	// moved (e.g., was keyframed on an elevator)...

 	if (m_dwDoorState == DOORSTATE_CLOSED)
	{
		g_pServerDE->GetObjectPos(m_hObject, &m_vOriginalPos);
	}


	// If this is an open away from door, Determine what open angles and
	// direction to use...

	if (m_bOpenAway)
	{
		// See if opening the default direction will hit the object opening
		// the door...
					
		DVector vOldAngles(m_fPitch, m_fYaw, m_fRoll);

		CalcAngle(m_fPitch, m_fPitch, m_vOpenAngles.x, m_vOpenDir.x, m_fSpeed);
		CalcAngle(m_fYaw,   m_fYaw,   m_vOpenAngles.y, m_vOpenDir.y, m_fSpeed);
		CalcAngle(m_fRoll,  m_fRoll,  m_vOpenAngles.z, m_vOpenDir.z, m_fSpeed);

		// Test rotate the object...

		DVector vTestPos;
		DRotation rRot;

		CalcPosAndRot(vTestPos, rRot);

		// Restore real angles...

		m_fPitch = vOldAngles.x;
		m_fYaw	 = vOldAngles.y;
		m_fRoll	 = vOldAngles.z;

		m_bOpeningNormal = DTRUE;

		if (ActivateObjectCollision(vTestPos))
		{
			m_bOpeningNormal = DFALSE;
			
			// Set the open angles so the door opens the opposite way...
	
			m_vOpenAngles.y = m_vOriginalOpenAngles.y - MATH_PI;
			m_vOpenDir.y = -m_vOriginalOpenDir.y;
		}
	}

	RotatingDoor::SetOpening();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HingedDoor::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void HingedDoor::Save(HMESSAGEWRITE hWrite, DBYTE nType)
{
	if (!hWrite) return;

	*hWrite << m_bOpeningNormal;
								
	*hWrite << m_bOpenAway;
	*hWrite << m_vOriginalOpenDir;
	*hWrite << m_vOriginalOpenAngles;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HingedDoor::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void HingedDoor::Load(HMESSAGEREAD hRead, DBYTE nType)
{
	if (!hRead) return;

	*hRead >> m_bOpeningNormal;

	*hRead >> m_bOpenAway;
	*hRead >> m_vOriginalOpenDir;
	*hRead >> m_vOriginalOpenAngles;
}


void HingedDoor::SetLightAnimOpen()
{
	if(m_bOpeningNormal)
		ReallySetLightAnimPos(0.0f);
	else
		ReallySetLightAnimPos(1.0f);
}


float HingedDoor::GetRotatingLightAnimPercent()
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

	fClosedPercent = 0.5f;
	fOpenPercent = m_bOpeningNormal ? 0.0f : 1.0f;

	return fClosedPercent + (fOpenPercent - fClosedPercent) * fPercentOpen;
}


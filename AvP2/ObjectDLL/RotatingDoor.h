// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingDoor.h
//
// PURPOSE : A RotatingDoor object
//
// CREATED : 12/3/97
//
// ----------------------------------------------------------------------- //

#ifndef __ROTATING_DOOR_H__
#define __ROTATING_DOOR_H__

#include "Door.h"

class RotatingDoor : public Door
{
	public:

		RotatingDoor();

	protected:

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, float fData);

		virtual void	SetOpening();
		virtual void	Opening();
		virtual void	Closing();
		virtual void	FirstUpdate();

		virtual void	CalcPosAndRot(DVector & vPos, DRotation & rRot);
		virtual DBOOL	CalcAngle(DFLOAT & fAngle, DFLOAT fInitial, 
								  DFLOAT fTarget, DFLOAT fDir, DFLOAT fAmount);

		virtual DBOOL	GetMoveTestPos(DVector & vTestPos);
		virtual DBOOL	ActivateObjectCollision(DVector vTestPos);

		virtual void	SetLightAnimOpen();
		virtual void	SetLightAnimClosed();

		virtual DBOOL	TreatLikeWorld() { return DFALSE; }

		// Returns the position it should put the light animation at.
		virtual float	GetRotatingLightAnimPercent();
		
		// This is here so we can treat pitch, yaw, and roll like a vector..
		float			GetCurAnglesDim(DDWORD iDim);

		DVector m_vRotationAngles;		// Direction to open
		DVector m_vRotPtOffset;			// Offset from door's postion to rotation point
		DVector m_vRotationPoint;		// Point to rotate around
		DVector m_vOpenAngles;			// Angles when object is open
		DVector m_vClosedAngles;		// Angles when object is closed
		DVector	m_vOriginalPos;			// Door's original position
		DVector m_vOpenDir;				// Direction door opens

		DFLOAT	m_fPitch;				// Pitch of door	
		DFLOAT	m_fYaw;					// Yaw of door
		DFLOAT	m_fRoll;				// Roll of door

	private :
	
		DBOOL	ReadProp(ObjectCreateStruct *pData);
		void	InitialUpdate();
		void	Save(HMESSAGEWRITE hWrite, DBYTE nType);
		void	Load(HMESSAGEREAD hRead, DBYTE nType);
};


// Used by the preprocessor functions to determine where the door will be rotating.
void Door_CalcPosAndRot(
	MathLT *pMathLT,
	DVector vOriginalPos,	// Original object position.
	DVector vRotationPoint,	// Point to rotate around.
	DVector vAngles,		// Rotation angles.
	DVector &vOutPos,
	DRotation &rOutRot);

// Used by preprocessor lighting stuff.
void SetupTransform_RotatingDoor(PreLightLT *pInterface, 
	HPREOBJECT hObject, 
	float fPercent,
	DVector &vOutPos, 
	DRotation &rOutRotation);



#endif // __ROTATING_DOOR_H__


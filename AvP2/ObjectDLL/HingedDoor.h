// ----------------------------------------------------------------------- //
//
// MODULE  : HingedDoor.h
//
// PURPOSE : A HingedDoor object
//
// CREATED : 12/3/97
//
// ----------------------------------------------------------------------- //

#ifndef __HINGED_DOOR_H__
#define __HINGED_DOOR_H__

#include "RotatingDoor.h"

class HingedDoor : public RotatingDoor
{
	public:

		HingedDoor();

	protected:

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, float fData);

		virtual void SetOpening();
		virtual void SetClosed(DBOOL bInitialize=DFALSE);
		virtual void SetLightAnimOpen();
		virtual float GetRotatingLightAnimPercent();

	private :
	
		DBOOL	ReadProp(ObjectCreateStruct *pData);
		void	InitialUpdate();
		void	Save(HMESSAGEWRITE hWrite, DBYTE nType);
		void	Load(HMESSAGEREAD hRead, DBYTE nType);

		DBOOL	m_bOpeningNormal;	// Tells if it's opening in its normal direction or
									// the opposite.
		DBOOL	m_bOpenAway;
		DVector m_vOriginalOpenDir;
		DVector m_vOriginalOpenAngles;
};


// Used by preprocessor lighting stuff.
void SetupTransform_HingedDoor(PreLightLT *pInterface, 
	HPREOBJECT hObject, 
	float fPercent,
	DVector &vOutPos, 
	DRotation &rOutRotation);


#endif // __HINGED_DOOR_H__


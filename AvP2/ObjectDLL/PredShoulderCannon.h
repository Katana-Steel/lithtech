// ----------------------------------------------------------------------- //
//
// MODULE  : PredShoulderCannon.h
//
// PURPOSE : Predator Shoulder Cannon object class definition
//
// CREATED : 6/21/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PRED_SHOULDERCANNON_H__
#define __PRED_SHOULDERCANNON_H__

// ----------------------------------------------------------------------- //

#include "ltengineobjects.h"
#include "GameBase.h"
#include "SFXFuncs.h"
// ----------------------------------------------------------------------- //

class ShoulderCannon : public GameBase
{
public:
	
	ShoulderCannon();
	virtual ~ShoulderCannon();
	
protected :
	
	// Handles engine messages
	uint32		EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
	
private:
	DBOOL	ReadProp(ObjectCreateStruct *pInfo);
	void	Save(HMESSAGEWRITE hWrite);
	void	Load(HMESSAGEREAD hRead);
	void	CreateLensFlareEffect();
	LENSFLARE	m_lensProps;
};

// ----------------------------------------------------------------------- //

#endif  // __PRED_SHOULDERCANNON_H__


// ----------------------------------------------------------------------- //
//
// MODULE  : SearchLight.h
//
// PURPOSE : An object which scans for the player and then sends a message
//
// CREATED : 3/29/99
//
// ----------------------------------------------------------------------- //

#ifndef __SEARCH_LIGHT_H__
#define __SEARCH_LIGHT_H__

#include "Scanner.h"
#include "SFXFuncs.h"

class SearchLight : public CScanner
{
	public :

		SearchLight();
		~SearchLight();

	protected :

		enum State
		{
			eStateTurningTo1,
			eStateTurningTo2,
			eStatePausingAt1,
			eStatePausingAt2,
			eStateDetected,
			eStateDestroyed
		};

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		virtual DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	protected :

		DBOOL	ReadProp(ObjectCreateStruct *pData);
		void	PostPropRead(ObjectCreateStruct* pData);
		DBOOL	InitialUpdate();

		DBOOL	Update();
		void	UpdateRotation();

		virtual DBOOL UpdateDetect();

		void	SetState(State eNewState);

	protected :

		State	m_eState;
		State	m_ePreviousState;

		DFLOAT	m_fYaw;
		DFLOAT	m_fYaw1;
		DFLOAT	m_fYaw2;
		DFLOAT	m_fYawSpeed;
		DFLOAT	m_fYaw1PauseTime;
		DFLOAT	m_fYaw2PauseTime;
		DFLOAT	m_fYawPauseTimer;
		
		DBOOL	m_bFirstUpdate;
		DBOOL	m_bOn;

		HSTRING	m_hstrTargetName;
		HOBJECT	m_hTargetObject;

		//** EVERYTHING BELOW HERE DOES NOT NEED SAVING

		// These are all saved by the engine in the special fx message...

		DFLOAT	m_fBeamLength;
		DFLOAT	m_fBeamRadius;
		DFLOAT	m_fBeamAlpha;
		DFLOAT	m_fBeamRotTime;
		DFLOAT	m_fLightRadius;
		DBOOL	m_bBeamAdditive;
		DVector	m_vLightColor;

		LENSFLARE	m_LensInfo;		// Lens flare info

	private :

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

		void	FirstUpdate();
};

#endif // __SEARCH_LIGHT_H__

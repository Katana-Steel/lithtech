// ----------------------------------------------------------------------- //
//
// MODULE  : SecurityCamera.h
//
// PURPOSE : An object which scans for the player and then sends a message
//
// CREATED : 3/29/99
//
// ----------------------------------------------------------------------- //

#ifndef __SECURITYCAMERA_H__
#define __SECURITYCAMERA_H__

#include "cpp_engineobjects_de.h"
#include "CharacterAlignment.h"
#include "Scanner.h"

class SecurityCamera : public CScanner
{
	public :

		SecurityCamera();
		~SecurityCamera();

	protected :

		enum State
		{
			eStateTurningTo1,
			eStateTurningTo2,
			eStatePausingAt1,
			eStatePausingAt2,
			eStateDetected,
			eStateOff,
			eStateDisabled,
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
		void	UpdateSounds(State eStatePrevious);

		virtual DBOOL	  UpdateDetect();
		virtual DRotation GetScanRotation();

		HSOUNDDE	PlaySound(const char* szSound, DBOOL bLoop = DFALSE, DBOOL bLouder = DFALSE);
		void		PlayStartSound();
		void		PlayStopSound();
		void		PlayLoopSound();
		void		PlayDetectedSound();
		void		KillAllSounds();
		
		void		SetState(State eNewState);
		void		SetupDisabledState();
		void		HandleGadgetMsg(ConParse & parse);

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
		
		HSTRING	m_hstrStartSound;
		HSTRING	m_hstrLoopSound;
		HSTRING	m_hstrStopSound;
		HSTRING	m_hstrDetectSound;

		DFLOAT	m_fInnerRadius;
		DFLOAT	m_fOuterRadius;
		DFLOAT	m_fDetectRadiusScale;

		HSOUNDDE m_hSound;
		HSOUNDDE m_hLoopSound;

		HOBJECT	m_hDisablerModel;

		DBOOL	m_bDisabled;

		//** EVERYTHING BELOW HERE DOES NOT NEED SAVING

	private :

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
		void	CacheFiles();
};

#endif // __SECURITYCAMERA_H__

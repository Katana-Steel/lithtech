// ----------------------------------------------------------------------- //
//
// MODULE  : SecurityCamera.cpp
//
// PURPOSE : Implementation of Security Camera
//
// CREATED : 3/27/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SecurityCamera.h"
#include "cpp_server_de.h"
#include "ServerUtilities.h"
#include "SoundMgr.h"
#include "ObjectMsgs.h"
#include "WeaponMgr.h"

// Statics

static char s_szOff[]		= "OFF";
static char s_szGadget[]	= "GADGET";

// Defines

#ifdef _WIN32
	#define DEFAULT_FILENAME	"Props\\Models\\SecurityCam.abc"
	#define DEFAULT_SKIN		"Props\\Skins\\SecurityCam.dtx"

	#define DEFAULT_BROKE_FILENAME	"Props\\Models\\SecurityCamBroke.abc"
	#define DEFAULT_BROKE_SKIN		"Props\\Skins\\SecurityCamBroke.dtx"
#else
	#define DEFAULT_FILENAME	"Props/Models/SecurityCam.abc"
	#define DEFAULT_SKIN		"Props/Skins/SecurityCam.dtx"

	#define DEFAULT_BROKE_FILENAME	"Props/Models/SecurityCamBroke.abc"
	#define DEFAULT_BROKE_SKIN		"Props/Skins/SecurityCamBroke.dtx"
#endif
// ----------------------------------------------------------------------- //
//
//	CLASS:		SecurityCamera
//
//	PURPOSE:	Scans for players
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(SecurityCamera)

	ADD_STRINGPROP_FLAG(Filename, DEFAULT_FILENAME, PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, DEFAULT_SKIN, PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedFilename, DEFAULT_BROKE_FILENAME, PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedSkin, DEFAULT_BROKE_SKIN, PF_FILENAME)

	ADD_REALPROP(Yaw1, 0.0)
	ADD_REALPROP(Yaw2, 180.0)
	ADD_REALPROP(YawTime, 5.0)

	ADD_REALPROP(Yaw1PauseTime, 2.0)
	ADD_REALPROP(Yaw2PauseTime, 2.0)

#ifdef _WIN32
	ADD_STRINGPROP_FLAG(StartSound, "Snd\\Props\\SecurityCamera\\Start.wav", PF_FILENAME)
	ADD_STRINGPROP_FLAG(LoopSound, "Snd\\Props\\SecurityCamera\\Loop.wav", PF_FILENAME)
	ADD_STRINGPROP_FLAG(StopSound, "Snd\\Props\\SecurityCamera\\Stop.wav", PF_FILENAME)
	ADD_STRINGPROP_FLAG(DetectSound, "Snd\\Props\\SecurityCamera\\Detect.wav", PF_FILENAME)
#else
	ADD_STRINGPROP_FLAG(StartSound, "Snd/Props/SecurityCamera/Start.wav", PF_FILENAME)
	ADD_STRINGPROP_FLAG(LoopSound, "Snd/Props/SecurityCamera/Loop.wav", PF_FILENAME)
	ADD_STRINGPROP_FLAG(StopSound, "Snd/Props/SecurityCamera/Stop.wav", PF_FILENAME)
	ADD_STRINGPROP_FLAG(DetectSound, "Snd/Props/SecurityCamera/Detect.wav", PF_FILENAME)
#endif

	ADD_REALPROP_FLAG(OuterRadius, 500.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(InnerRadius, 100.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(DetectRadiusScale, 2.0f, PF_RADIUS)

END_CLASS_DEFAULT_FLAGS(SecurityCamera, CScanner, NULL, NULL, PF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::SecurityCamera()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

SecurityCamera::SecurityCamera() : CScanner()
{
	m_ePreviousState	= eStatePausingAt1;
	m_eState			= eStatePausingAt1;

	m_fYaw				= 0.0f;
	m_fYaw1				= 0.0f;
	m_fYaw2				= 0.0f;
	m_fYawSpeed			= 0.0f;
	m_fYaw1PauseTime	= 0.0f;
	m_fYaw2PauseTime	= 0.0f;
	m_fYawPauseTimer	= 0.0f;
	
	m_fOuterRadius		 = 100.0f;
	m_fInnerRadius		 = 10.0f;
	m_fDetectRadiusScale = 2.0f;

	m_hstrStartSound	= DNULL;
	m_hstrLoopSound		= DNULL;
	m_hstrStopSound		= DNULL;
	m_hstrDetectSound	= DNULL;
	m_hSound			= DNULL;
	m_hLoopSound		= DNULL;
	m_hDisablerModel	= DNULL;

	m_bDisabled			= DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::~SecurityCamera()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

SecurityCamera::~SecurityCamera()
{
	FREE_HSTRING(m_hstrStartSound);
	FREE_HSTRING(m_hstrLoopSound);
	FREE_HSTRING(m_hstrStopSound);
	FREE_HSTRING(m_hstrDetectSound);

	KillAllSounds();

	if (m_hDisablerModel)
	{
		// Remove the model...

		HATTACHMENT hAttachment;
		if (g_pServerDE->FindAttachment(m_hObject, m_hDisablerModel, &hAttachment) == DE_OK)
		{
			g_pServerDE->RemoveAttachment(hAttachment);
		}

		g_pServerDE->RemoveObject(m_hDisablerModel);
		m_hDisablerModel = DNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD SecurityCamera::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			DDWORD dwRet = CScanner::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);
			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CacheFiles();
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
		}
		break;

		default : break;
	}

	return CScanner::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD SecurityCamera::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	if (!g_pServerDE) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			CommonLT* pCommon = g_pServerDE->Common();
			if (!pCommon) return 0;

			HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			char* szMsg = hMsg ? g_pServerDE->GetStringData(hMsg) : DNULL;
			if (!szMsg) return 0;

			ConParse parse;
			parse.Init(szMsg);

			while (pCommon->Parse(&parse) == LT_OK)
			{
				if (parse.m_nArgs > 0 && parse.m_Args[0])
				{
					if (_stricmp(parse.m_Args[0], s_szOff) == 0)
					{
						SetState(eStateOff);
					}
					else if (_stricmp(parse.m_Args[0], s_szGadget) == 0)
					{
						HandleGadgetMsg(parse);
					}
				}
			}

			g_pServerDE->FreeString(hMsg);
		}
		break;

		case MID_DAMAGE:
		{
			// Let our damage aggregate process the message first...

			DDWORD dwRet = CScanner::ObjectMessageFn(hSender, messageID, hRead);

			// Check to see if we have been destroyed

			if (m_damage.IsDead())
			{
				SetState(eStateDestroyed);
			}
			return dwRet;
		}
		break;

		default : break;
	}

	return CScanner::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL SecurityCamera::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) return DFALSE;

	GenericProp genProp;

	if ( g_pServerDE->GetPropGeneric("Yaw1", &genProp ) == DE_OK )
		m_fYaw1 = genProp.m_Float;
	
	if ( g_pServerDE->GetPropGeneric("Yaw2", &genProp ) == DE_OK )
		m_fYaw2 = genProp.m_Float;
	
	if ( g_pServerDE->GetPropGeneric("YawTime", &genProp ) == DE_OK )
		m_fYawSpeed = genProp.m_Float;
	
	if ( g_pServerDE->GetPropGeneric("Yaw1PauseTime", &genProp ) == DE_OK )
		m_fYaw1PauseTime = genProp.m_Float;
	
	if ( g_pServerDE->GetPropGeneric("Yaw2PauseTime", &genProp ) == DE_OK )
		m_fYaw2PauseTime = genProp.m_Float;

	if ( g_pServerDE->GetPropGeneric( "StartSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrStartSound = g_pServerDE->CreateString( genProp.m_String );

	if ( g_pServerDE->GetPropGeneric( "LoopSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrLoopSound = g_pServerDE->CreateString( genProp.m_String );

	if ( g_pServerDE->GetPropGeneric( "StopSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrStopSound = g_pServerDE->CreateString( genProp.m_String );

	if ( g_pServerDE->GetPropGeneric( "DetectSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrDetectSound = g_pServerDE->CreateString( genProp.m_String );

	g_pServerDE->GetPropReal( "OuterRadius", &m_fOuterRadius );
	g_pServerDE->GetPropReal( "InnerRadius", &m_fInnerRadius );
	g_pServerDE->GetPropReal( "DetectRadiusScale", &m_fDetectRadiusScale );

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void SecurityCamera::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// Convert all our stuff into radians

	m_fYaw1 = MATH_DEGREES_TO_RADIANS(m_fYaw1);
	m_fYaw2 = MATH_DEGREES_TO_RADIANS(m_fYaw2);

	// YawSpeed was actually specified as time, so make it a rate

	m_fYawSpeed = (m_fYaw2 - m_fYaw1)/m_fYawSpeed;

	// Adjust yaws based on initial pitch yaw roll

	m_fYaw1 += GetInitialPitchYawRoll().y;
	m_fYaw2 += GetInitialPitchYawRoll().y;

	m_fYaw = GetInitialPitchYawRoll().y;

	if (m_fYaw > m_fYaw1)
	{
		SetState(eStateTurningTo2);
	}
	else
	{
		SetState(eStatePausingAt1);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

DBOOL SecurityCamera::InitialUpdate()
{
	g_pServerDE->SetNextUpdate(m_hObject, 0.001f);

	// Setup the object so we can be disabled by the camera disabler...

	const DDWORD dwUsrFlgs = g_pServerDE->GetObjectUserFlags(m_hObject);
	g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlgs | USRFLG_GADGET_CAMERA_DISABLER);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

DBOOL SecurityCamera::Update()
{
	State eStatePrevious = m_eState;

	if (m_eState == eStateDestroyed || m_eState == eStateOff) 
	{
		g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
		return DTRUE;
	}

	UpdateRotation();
	
	if (!m_bDisabled)
	{
		UpdateDetect();
	}
	
	UpdateSounds(eStatePrevious);

	g_pServerDE->SetNextUpdate(m_hObject, 0.001f);

	return DTRUE;
}

void SecurityCamera::UpdateSounds(State eStatePrevious)
{
	if ( (eStatePrevious == eStateTurningTo1 && m_eState == eStatePausingAt1) || 
		 (eStatePrevious == eStateTurningTo2 && m_eState == eStatePausingAt2) )
	{
		PlayStopSound();
	}
	else if ( (eStatePrevious == eStatePausingAt1 && m_eState == eStateTurningTo2) || 
		      (eStatePrevious == eStatePausingAt2 && m_eState == eStateTurningTo1) )
	{
		PlayStartSound();
	}
	else if ( m_eState == eStateTurningTo1 || m_eState == eStateTurningTo2 )
	{
		PlayLoopSound();
	}
	else if ( m_eState == eStateDetected && eStatePrevious != eStateDetected )
	{
		PlayDetectedSound();
	}
}

HSOUNDDE SecurityCamera::PlaySound(const char* szSound, DBOOL bLoop /* = DFALSE */, DBOOL bLouder /* = DFALSE */)
{
	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT(playSoundInfo);

	strcpy(playSoundInfo.m_szSoundName, szSound);

	playSoundInfo.m_dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_REVERB | PLAYSOUND_3D;
	if ( bLoop )
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
	}
	else
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_TIME;
	}
	playSoundInfo.m_fOuterRadius = m_fOuterRadius;
	playSoundInfo.m_fInnerRadius = m_fInnerRadius;
	if ( bLouder )
	{
		playSoundInfo.m_fOuterRadius *= m_fDetectRadiusScale;
		playSoundInfo.m_fInnerRadius *= m_fDetectRadiusScale;
		playSoundInfo.m_nPriority    = SOUNDPRIORITY_MISC_HIGH;
	}
	else
	{
		playSoundInfo.m_nPriority    = SOUNDPRIORITY_MISC_LOW;
	}
	playSoundInfo.m_nVolume = 100;
	g_pServerDE->GetObjectPos(m_hObject, &playSoundInfo.m_vPosition);

	g_pServerDE->PlaySound(&playSoundInfo);

	return playSoundInfo.m_hSound;
}

void SecurityCamera::PlayStartSound()
{
	KillAllSounds();

	char* szStartSound = g_pServerDE->GetStringData(m_hstrStartSound);

	if ( szStartSound && *szStartSound )
	{
		m_hSound = PlaySound(szStartSound);
	}
}

void SecurityCamera::PlayStopSound()
{
	KillAllSounds();

	char* szStopSound = g_pServerDE->GetStringData(m_hstrStopSound);

	if ( szStopSound && *szStopSound )
	{
		m_hSound = PlaySound(szStopSound);
	}
}

void SecurityCamera::PlayLoopSound()
{
	if ( m_hSound )
	{
		DBOOL bDone = DTRUE;

		if ( DE_OK != g_pServerDE->IsSoundDone(m_hSound, &bDone) )
		{
			return;
		}
		
		if ( !bDone )
		{
			return;
		}

		g_pServerDE->KillSound(m_hSound);
		m_hSound = DNULL;
	}

	if ( !m_hLoopSound )
	{
		char* szLoopSound = g_pServerDE->GetStringData(m_hstrLoopSound);

		if ( szLoopSound && *szLoopSound )
		{
			m_hLoopSound = PlaySound(szLoopSound, DTRUE);
		}
	}
}


void SecurityCamera::PlayDetectedSound()
{
	KillAllSounds();

	if ( !m_hSound )
	{
		char* szDetectSound = g_pServerDE->GetStringData(m_hstrDetectSound);

		if ( szDetectSound && *szDetectSound )
		{
			m_hSound = PlaySound(szDetectSound);
		}
	}
}


void SecurityCamera::KillAllSounds()
{
	if ( m_hSound )
	{
		g_pServerDE->KillSound(m_hSound);
		m_hSound = DNULL;
	}

	if ( m_hLoopSound )
	{
		g_pServerDE->KillSound(m_hLoopSound);
		m_hLoopSound = DNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::UpdateDetect()
//
//	PURPOSE:	Checks to see if we can see anything
//
// ----------------------------------------------------------------------- //

DBOOL SecurityCamera::UpdateDetect()
{
	if (CScanner::UpdateDetect())
	{
		SetState(eStateDetected);
		return DTRUE;
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::GetScanRotation()
//
//	PURPOSE:	Get the scan rotation (just use our yaw value)
//
// ----------------------------------------------------------------------- //

DRotation SecurityCamera::GetScanRotation()
{
	DRotation rRot;
	g_pServerDE->SetupEuler(&rRot, 0.0f, m_fYaw, 0.0f);

	return rRot;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::UpdateRotation()
//
//	PURPOSE:	Handles updating the camera's rotation
//
// ----------------------------------------------------------------------- //

void SecurityCamera::UpdateRotation()
{
	DFLOAT fTimeDelta = g_pServerDE->GetFrameTime();

	if ( m_eState == eStateDetected || m_eState == eStateDestroyed )
	{
		return;
	}

	if ( m_eState == eStateTurningTo1 || m_eState == eStateTurningTo2 )
	{
		DFLOAT fYaw = g_pServerDE->GetFrameTime()*m_fYawSpeed;

		if ( m_eState == eStateTurningTo1 )
		{
			fYaw = -fYaw;
			m_fYaw += fYaw;

			if ( m_fYaw < m_fYaw1 )
			{
				if ( m_fYaw1PauseTime )
				{
					SetState(eStatePausingAt1);
				}
				else
				{
					SetState(eStateTurningTo2);
				}
			}
		}
		else 
		{
			m_fYaw += fYaw;

			if ( m_fYaw > m_fYaw2 )
			{
				if ( m_fYaw2PauseTime )
				{
					SetState(eStatePausingAt2);
				}
				else
				{
					SetState(eStateTurningTo1);
				}
			}
		}

		static DVector vUp(0.0f,1.0f,0.0f);
		DRotation rRot;
		g_pServerDE->GetObjectRotation(m_hObject, &rRot);
		g_pServerDE->RotateAroundAxis(&rRot, &vUp, fYaw);
		g_pServerDE->SetObjectRotation(m_hObject, &rRot);
	}

	if ( m_eState == eStatePausingAt1 )
	{
		m_fYawPauseTimer += fTimeDelta;

		if ( m_fYawPauseTimer > m_fYaw1PauseTime )
		{
			m_fYawPauseTimer = 0.0f;
			SetState(eStateTurningTo2);
		}
	}

	if ( m_eState == eStatePausingAt2 )
	{
		m_fYawPauseTimer += fTimeDelta;

		if ( m_fYawPauseTimer > m_fYaw2PauseTime )
		{
			m_fYawPauseTimer = 0.0f;
			SetState(eStateTurningTo1);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::CacheFiles
//
//	PURPOSE:	Cache whatever resources this object uses
//
// ----------------------------------------------------------------------- //

void SecurityCamera::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrDetectSound)
	{
		char* pFile = pServerDE->GetStringData(m_hstrDetectSound);
		if (pFile && pFile[0])
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrStartSound)
	{
		char* pFile = pServerDE->GetStringData(m_hstrStartSound);
		if (pFile && pFile[0])
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
	
	if (m_hstrLoopSound)
	{
		char* pFile = pServerDE->GetStringData(m_hstrLoopSound);
		if (pFile && pFile[0])
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrStopSound)
	{
		char* pFile = pServerDE->GetStringData(m_hstrStopSound);
		if (pFile && pFile[0])
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::SetState
//
//	PURPOSE:	Change our current state
//
// ----------------------------------------------------------------------- //

void SecurityCamera::SetState(State eNewState)
{
	if (m_eState == eNewState) return;

	// Handle switching to the new state...
	
	switch (eNewState)
	{
		case eStateDestroyed:
		{
			SetDestroyedModel();

			// Camera can't be disabled now...

			DDWORD dwUserFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
			g_pServerDE->SetObjectUserFlags(m_hObject, dwUserFlags & ~USRFLG_GADGET_CAMERA_DISABLER);

			KillAllSounds();
		}
		break;

		case eStateOff:
		{
			KillAllSounds();
		}
		break;

		case eStateDisabled:
		{
			SetupDisabledState();
			return;  // Don't change states...
		}
		break;

		default : break;
	}

	m_ePreviousState = m_eState;
	m_eState = eNewState;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::SetupDisabledState
//
//	PURPOSE:	Setup the disabled state
//
// ----------------------------------------------------------------------- //

void SecurityCamera::SetupDisabledState()
{
	if (m_eState == eStateDisabled || m_hDisablerModel) return;

	DDWORD dwUserFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
	g_pServerDE->SetObjectUserFlags(m_hObject, dwUserFlags & ~USRFLG_GADGET_CAMERA_DISABLER);

	// Create the camera disabler model, and attach it to the camera...

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	DVector vPos;
	g_pServerDE->GetObjectPos(m_hObject, &vPos);
	VEC_COPY(theStruct.m_Pos, vPos);

#ifdef _WIN32
	SAFE_STRCPY(theStruct.m_Filename, "Guns\\Models_HH\\Cameradis.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "Guns\\Skins_HH\\Cameradis.dtx");
#else
	SAFE_STRCPY(theStruct.m_Filename, "Guns/Models_HH/Cameradis.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "Guns/Skins_HH/Cameradis.dtx");
#endif
	
	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD;
	theStruct.m_ObjectType  = OT_MODEL;

	HCLASS hClass = g_pServerDE->GetClass("BaseClass");
	LPBASECLASS pModel = g_pServerDE->CreateObject(hClass, &theStruct);
	if (!pModel) return;

	m_hDisablerModel = pModel->m_hObject;

	// Attach the model to the the camera...

	DVector vOffset;
	VEC_INIT(vOffset);

	DRotation rOffset;
	ROT_INIT(rOffset);

	HATTACHMENT hAttachment;
	DRESULT dRes = g_pServerDE->CreateAttachment(m_hObject, m_hDisablerModel, "Disabler", 
											     &vOffset, &rOffset, &hAttachment);
	if (dRes != DE_OK)
	{
		g_pServerDE->RemoveObject(m_hDisablerModel);
		m_hDisablerModel = DNULL;
		return;
	}

	// Set the Disabler's animation...

	HMODELANIM hAni = g_pServerDE->GetAnimIndex(m_hDisablerModel, "DownUp");
	if (hAni)
	{
		g_pServerDE->SetModelLooping(m_hDisablerModel, DFALSE);
		g_pServerDE->SetModelAnimation(m_hDisablerModel, hAni);
	}


	// Play the activate sound...

#ifdef _WIN32
	char* pSound = "Guns\\Snd\\Cam_dis\\activate.wav";
#else
	char* pSound = "Guns/Snd/Cam_dis/activate.wav";
#endif

	g_pSoundMgr->PlaySoundFromPos(vPos, pSound, 500.0f, SOUNDPRIORITY_MISC_LOW);


	// Camera is now disabled...

	m_bDisabled = DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::HandleGadgetMsg
//
//	PURPOSE:	Handle the camer disabler gadget
//
// ----------------------------------------------------------------------- //

void SecurityCamera::HandleGadgetMsg(ConParse & parse)
{
	if (parse.m_nArgs < 2 || !parse.m_Args[1]) return;

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(atol(parse.m_Args[1]));
	if (!pAmmo) return;

//	if (pAmmo->eInstDamageType == DT_GADGET_CAMERA_DISABLER)
//	{
//		SetState(eStateDisabled);
//	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SecurityCamera::Save(HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	g_pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hDisablerModel);

	g_pServerDE->WriteToMessageDWord(hWrite, m_eState);
	g_pServerDE->WriteToMessageDWord(hWrite, m_ePreviousState);

	g_pServerDE->WriteToMessageFloat(hWrite, m_fYaw);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fYaw1);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fYaw2);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fYawSpeed);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fYaw1PauseTime);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fYaw2PauseTime);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fYawPauseTimer);

	g_pServerDE->WriteToMessageHString(hWrite, m_hstrStartSound);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrLoopSound);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrStopSound);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrDetectSound);

	g_pServerDE->WriteToMessageFloat(hWrite, m_fInnerRadius);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fOuterRadius);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fDetectRadiusScale);

	g_pServerDE->WriteToMessageByte(hWrite, m_bDisabled);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SecurityCamera::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	g_pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hDisablerModel);

	m_eState			= (State)g_pServerDE->ReadFromMessageDWord(hRead);
	m_ePreviousState	= (State)g_pServerDE->ReadFromMessageDWord(hRead);

	m_fYaw				= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw1				= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw2				= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fYawSpeed			= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw1PauseTime	= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw2PauseTime	= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fYawPauseTimer	= g_pServerDE->ReadFromMessageFloat(hRead);
	
	m_hstrStartSound	= g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrLoopSound		= g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrStopSound		= g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrDetectSound	= g_pServerDE->ReadFromMessageHString(hRead);

	m_fInnerRadius		 = g_pServerDE->ReadFromMessageFloat(hRead);
	m_fOuterRadius		 = g_pServerDE->ReadFromMessageFloat(hRead);
	m_fDetectRadiusScale = g_pServerDE->ReadFromMessageFloat(hRead);

	m_bDisabled			 = (DBOOL)g_pServerDE->ReadFromMessageByte(hRead);

	// Restore our sounds

	if (m_eState == eStateTurningTo1 || m_eState == eStateTurningTo2)
	{
		PlayLoopSound();
	}
}



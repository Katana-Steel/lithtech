// ----------------------------------------------------------------------- //
//
// MODULE  : Turret.cpp
//
// PURPOSE : Implementation of Turret object.
//
// CREATED : 2/14/00
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Turret.h"
#include "cpp_server_de.h"
#include "iobjectplugin.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "SFXMsgIds.h"
#include "DestructibleModel.h"
#include "Character.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"
#include "WeaponMgr.h"
#include "ServerSoundMgr.h"
#include "AIButeMgr.h" // for g_szAIButeMgrFile
#include "AIUtils.h"  // for operator<<(LTStream&,std::string)
#include "ltrotation.h"
#include "GameServerShell.h"
#include "AISenseMgr.h"

#include <algorithm>

#ifdef _DEBUG
//#include "DebugLineSystem.h"
//#define TURRET_DEBUG
#endif

ILTMessage & operator<<(ILTMessage & out, Turret::ObjectiveType & x)
{
	out.WriteDWord( uint32(x) );
	return out;
}

ILTMessage & operator>>(ILTMessage & in, Turret::ObjectiveType & x)
{
	x = Turret::ObjectiveType(in.ReadDWord());
	return in;
}

namespace /*unnamed*/
{
	class ClosestTo
	{
		const LTVector m_vPos;

	public :

		ClosestTo( const LTVector & vPos)
			: m_vPos(vPos) {}

		bool operator()(const CCharacter * pCharA, const CCharacter * pCharB) const
		{
			return (pCharA->GetPosition() - m_vPos).MagSqr() < (pCharB->GetPosition() - m_vPos).MagSqr();
		}
	};

	const char * GetWeaponSound(const BARREL & barrel, DBYTE nType)
	{
		switch(nType)
		{
			case WMKS_FIRE:				return barrel.szFireSound;			break;
			case WMKS_ALT_FIRE:			return barrel.szAltFireSound;			break;
			case WMKS_DRY_FIRE:			return barrel.szDryFireSound;			break;

			case WMKS_MISC0:
			case WMKS_MISC1:
			case WMKS_MISC2:			return barrel.szMiscSounds[nType];	break;

			case WMKS_SELECT:
			case WMKS_DESELECT:
			case WMKS_INVALID:
			default:					return LTNULL;
		}
	}


	static LTFLOAT ClampAngle(LTFLOAT angle)
	{
		int max_adjust = 20;

		while( angle > MATH_PI && --max_adjust > 0 )
		{
			angle -= MATH_CIRCLE;
		}

		while( angle < -MATH_PI && --max_adjust > 0 )
		{
			angle += MATH_CIRCLE;
		}

		_ASSERT( max_adjust > 0 );

		return angle;
	}


	LTBOOL SoundIsDone(HLTSOUND sound)
	{
		LTBOOL result = LTTRUE;
		g_pLTServer->IsSoundDone(sound,&result);
		return result;
	}


	double GetDoubleProp(GenericProp & genProp, const char * prop_name, double def_value = 0.0f)
	{
		if (g_pLTServer->GetPropGeneric(const_cast<char*>(prop_name), &genProp) == LT_OK)
		{
			return genProp.m_Float;
		}

		return def_value;
	}

	LTBOOL GetBoolProp(GenericProp & genProp, const char * prop_name, LTBOOL def_value = LTFALSE)
	{
		if (g_pLTServer->GetPropGeneric(const_cast<char*>(prop_name), &genProp) == LT_OK)
		{
			return genProp.m_Bool;
		}

		return def_value;
	}

	std::string GetStringProp(GenericProp & genProp, const char * prop_name, std::string def_value = std::string() )
	{
		if (g_pLTServer->GetPropGeneric(const_cast<char*>(prop_name), &genProp) == LT_OK)
		{
			if( genProp.m_String[0] )
			{
				return genProp.m_String;
			}
		}

		return def_value;
	}

}; //namespace /*unnamed*/


// Defines

#ifdef _WIN32
	#define DEFAULT_FILENAME		"Models\\Props\\Tripod.abc"
	#define DEFAULT_SKIN			"Skins\\Props\\Tripod.dtx"
#else
	#define DEFAULT_FILENAME		"Models/Props/Tripod.abc"
	#define DEFAULT_SKIN			"Skins/Props/Tripod.dtx"
#endif

#define DEFAULT_GUN_HIT_POINTS  100.0f

#define UPDATE_DELTA			0.1f

const char * g_szObjectiveTypes[] = 
{
	"Ignore",
	"Interest",
	"Target",
	"Preferred",
	0
};


const char * g_szActions[] = 
{
	"Sleep",
	"Sweep",
	"Track",
	"Fire",
	0
	// If you change these please update Turret::GetRootState().
};

const char * g_szIdleActions[] = 
{
	"Sleep",
	"Sweep",
	0
	// If you change these please update Turret::GetIdleRootState().
};

class TurretPlugin : public IObjectPlugin
{

public :

	virtual LTRESULT	PreHook_EditStringList(const char* szRezPath, const char* szPropName, char* const * aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
	{
		if( !sm_bInitted )
		{
			sm_WeaponMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
			sm_bInitted = LTTRUE;
		}

		if( stricmp(szPropName, "WeaponType" ) == 0 )
		{
			sm_WeaponMgrPlugin.PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
			return LT_OK;
		}
		else if( stricmp(szPropName, "Marine") == 0 
			|| stricmp(szPropName, "Corporate") == 0 
			|| stricmp(szPropName, "Alien") == 0 
			|| stricmp(szPropName, "Predator") == 0 )
		{
			*pcStrings = 0;

			for( const char ** iter = g_szObjectiveTypes; *iter && (*pcStrings) < cMaxStrings; ++iter, ++aszStrings )
			{
				ASSERT( strlen(*iter) < cMaxStringLength );
				strcpy(*aszStrings, *iter);
				++(*pcStrings);
			}

			return LT_OK;
		}
		else if(   stricmp(szPropName, "DetectInterest") == 0 
				|| stricmp(szPropName, "DetectTarget") == 0 )
		{
			*pcStrings = 0;

			
			for( const char ** iter = g_szActions; *iter && (*pcStrings) < cMaxStrings; ++iter, ++aszStrings )
			{
				ASSERT( strlen(*iter) < cMaxStringLength );
				strcpy(*aszStrings, *iter);
				++(*pcStrings);
			}

			return LT_OK;
		}
		else if(   stricmp(szPropName, "IdleTimeOut") == 0
			    || stricmp(szPropName, "Idle") == 0 )
		{
			*pcStrings = 0;

			
			for( const char ** iter = g_szIdleActions; *iter && (*pcStrings) < cMaxStrings; ++iter, ++aszStrings )
			{
				ASSERT( strlen(*iter) < cMaxStringLength );
				strcpy(*aszStrings, *iter);
				++(*pcStrings);
			}

			return LT_OK;
		}

		return LT_UNSUPPORTED;
	}

	private :

		static LTBOOL				sm_bInitted;
		static CWeaponMgrPlugin		sm_WeaponMgrPlugin;
};

LTBOOL TurretPlugin::sm_bInitted = LTFALSE;
CWeaponMgrPlugin TurretPlugin::sm_WeaponMgrPlugin;




BEGIN_CLASS(Turret)
	ADD_STRINGPROP_FLAG(Filename, DEFAULT_FILENAME, PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, DEFAULT_SKIN, PF_FILENAME)

	ADD_STRINGPROP_FLAG(AttributeName, "Tripod", 0)

	ADD_STRINGPROP_FLAG( Corporate, const_cast<char*>(g_szObjectiveTypes[0]), PF_STATICLIST )
	ADD_STRINGPROP_FLAG( Marine,    const_cast<char*>(g_szObjectiveTypes[2]), PF_STATICLIST )
	ADD_STRINGPROP_FLAG( Alien,     const_cast<char*>(g_szObjectiveTypes[2]), PF_STATICLIST )
	ADD_STRINGPROP_FLAG( Predator,  const_cast<char*>(g_szObjectiveTypes[2]), PF_STATICLIST )

	ADD_STRINGPROP_FLAG( IdleTimeOut,    const_cast<char*>(g_szIdleActions[0]), PF_STATICLIST )
	ADD_REALPROP_FLAG( MaxIdleTime,   300.0f, PF_STATICLIST )
	ADD_STRINGPROP_FLAG( Idle,           const_cast<char*>(g_szIdleActions[1]), PF_STATICLIST )
	ADD_STRINGPROP_FLAG( DetectInterest, const_cast<char*>(g_szActions[2]), PF_STATICLIST )
	ADD_STRINGPROP_FLAG( DetectTarget,   const_cast<char*>(g_szActions[3]), PF_STATICLIST )

	ADD_REALPROP_FLAG( TargetDelay, 1.0f, 0)
	ADD_REALPROP_FLAG( TargetTimeOut, 4.0f, 0)

	ADD_BOOLPROP_FLAG( UseAwakeSound, LTTRUE, 0 )
	ADD_BOOLPROP_FLAG( UseDetectSound, LTTRUE, 0 )

	ADD_BOOLPROP_FLAG_HELP( AttachToBase, LTFALSE, 0, "If the base will be moving, this must be true.  The turret rotations look better with this false." )

	ADD_STRINGPROP_FLAG(DebrisType, "Metal03", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SurfaceOverride, "Metal", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(ImpactFXName, "SpiderGrenadeImpact", PF_STATICLIST)

	ADD_REALPROP_FLAG(HitPoints, 50.0f, PF_GROUP1)
	ADD_REALPROP_FLAG(MaxHitPoints, 50.0f, PF_GROUP1)
	ADD_REALPROP_FLAG(Armor, 100.0f, PF_GROUP1)
	ADD_REALPROP_FLAG(MaxArmor, 100.0f, PF_GROUP1)

	PROP_DEFINEGROUP( Motion, PF_GROUP4 )

		ADD_REALPROP_FLAG(MinYaw, -45.0, PF_GROUP4)
		ADD_REALPROP_FLAG(MaxYaw, 45.0, PF_GROUP4)

		ADD_REALPROP_FLAG(MinPitch, -20.0, PF_GROUP4)
		ADD_REALPROP_FLAG(MaxPitch, 20.0, PF_GROUP4)

		ADD_REALPROP_FLAG(SweepPauseTime, 0.0, PF_GROUP4)

	ADD_REALPROP( VisualRange, 1000.0f )

	ADD_BOOLPROP_FLAG_HELP( StartOff, LTFALSE, 0, "If true, you will need to trigger the turret with a \"on\" message.")

	ADD_BOOLPROP_FLAG_HELP( CanSeeThrough, LTFALSE, 0, "Can Turret see through things just like AI's?" )
	ADD_BOOLPROP_FLAG_HELP( CanShootThrough, LTFALSE, 0, "Can Turret shoot through things just like AI's?" )

	// Defaults at Nathan Hendrickson's request.  3-13-01
	ADD_SOLID_FLAG(1, 0)
    ADD_BOOLPROP_FLAG(CanHeal, LTFALSE, PF_GROUP1)
    ADD_BOOLPROP_FLAG(CanRepair, LTFALSE, PF_GROUP1)
    ADD_BOOLPROP_FLAG(CanDamage, LTTRUE, PF_GROUP1)
    ADD_BOOLPROP_FLAG(NeverDestroy, LTFALSE, PF_GROUP1)
	ADD_BOOLPROP_FLAG_HELP(CreateExplosion, LTFALSE, (PF_GROUP1 << 1), "Should an explosion be created when this object dies?") \
	ADD_REALPROP_FLAG_HELP(MaxDamage, 25.0f, (PF_GROUP1 << 1), "The maximum amount of damage at the center point of the explosion. The amount of damage falls off towards the edge of the damage radius.")\

END_CLASS_DEFAULT_FLAGS_PLUGIN(Turret, Prop, NULL, NULL, 0, TurretPlugin)


// Local functions
struct PositionAndRotation
{
	LTVector vPos;
	LTRotation rRot;

	PositionAndRotation()
		: vPos(0,0,0),
		  rRot(0,0,0,1) {}
};

static PositionAndRotation GetPositionAndRotateOffset( HOBJECT hObject, 
													   const LTVector & vNewTranslation,    const LTRotation & rNewRotation,  
													   const LTVector & vOffsetTranslation, const LTRotation & rOffsetRotation )
{
	PositionAndRotation result;

	if( !hObject ) 
		return result;

	// Get the transformation matrix for a rotation about the offset.
	LTMatrix mMat;
	LTRotation rRot = rOffsetRotation*rNewRotation;
	LTVector vTempOffset = -vOffsetTranslation;

	g_pMathLT->SetupRotationAroundPoint(mMat, rRot, vTempOffset);
//	g_pMathLT->SetupRotationAroundPoint(mMat, const_cast<LTRotation&>(rOffsetRotation*rNewRotation), const_cast<LTVector&>(-vOffsetTranslation) );

	// Get the position, sum of (non-offset) position and translations due to offset and rotation.
	g_pMathLT->SetupTranslationFromMatrix(result.vPos, mMat);
	result.vPos += vNewTranslation + vOffsetTranslation;

	// Get the actual rotation.
	g_pMathLT->SetupRotationFromMatrix(result.rRot, mMat);

	return result;
}

static LTRESULT GetSocketPosAndRot(HOBJECT hObject, HATTACHMENT hAttachment, const char * szSocketName, LTVector * pvPos, LTRotation * prRot, LTBOOL bUseWorldCoord = LTTRUE)
{

	if( !hObject && !hAttachment) return LT_ERROR;

	if( !szSocketName || !szSocketName[0] )
	{
		if( !bUseWorldCoord )
		{
			if( pvPos )
				pvPos->Init();
			if( prRot )
				prRot->Init();

			return LT_OK;
		}

		if( pvPos )
			g_pLTServer->GetObjectPos(hObject,pvPos);
		if( prRot )
			g_pLTServer->GetObjectRotation(hObject,prRot);

		return LT_OK;
	}

	// Get socket.
	HMODELSOCKET hSocket = NULL;
	LTRESULT result = g_pModelLT->GetSocket(hObject,const_cast<char*>(szSocketName),hSocket);
	if( LT_OK !=  result ) return result;

	// Get socket's transform.
	LTransform transform;
	if( hAttachment )
	{
		LTransform attachment_transform, socket_transform;

		// Get attachment transform.
		LTRESULT result = g_pLTServer->Common()->GetAttachmentTransform(hAttachment, attachment_transform, bUseWorldCoord);
		if(result != LT_OK)
			return result;

		// Get socket transform.
		result = g_pModelLT->GetSocketTransform(hObject, hSocket, socket_transform, LTFALSE);

		if(result != LT_OK)
			return result;

		// Now multiply them..
		g_pTransLT->Multiply(transform, attachment_transform, socket_transform);

	}
	else
	{
		LTRESULT result = g_pModelLT->GetSocketTransform(hObject, hSocket, transform, bUseWorldCoord);
		if( LT_OK != result ) 
			return result;
	}

	// Load up results.
	if( pvPos )
		g_pTransLT->GetPos(transform,*pvPos);
	if( prRot )
		g_pTransLT->GetRot(transform,*prRot);

	return LT_OK;
}

static LTRESULT GetSocketPos(HOBJECT hObject, HATTACHMENT hAttachment, const char * szSocketName, LTVector * pvPos, LTBOOL bUseWorldCoord = LTTRUE)
{
	return GetSocketPosAndRot(hObject,hAttachment,szSocketName,pvPos,LTNULL,bUseWorldCoord);
}

static LTRESULT GetSocketRot(HOBJECT hObject, HATTACHMENT hAttachment, const char * szSocketName, LTRotation * prRot, LTBOOL bUseWorldCoord = LTTRUE)
{
	return GetSocketPosAndRot(hObject,hAttachment,szSocketName,LTNULL,prRot,bUseWorldCoord);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::Turret()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Turret::Turret() 
	: m_fGunMaxHitPoints(0.0f),
	  m_fWarningSoundDistance(0.0f),
	  m_fMovingSoundDistance(0.0f),


	  m_nWeaponId(0),
	  m_fBurstDelay(0),
	  m_fDamageMultiplier(1.0f),

	  m_nStartFireSound(WMKS_INVALID),
	  m_nLoopFireSound(WMKS_INVALID),
	  m_nStopFireSound(WMKS_INVALID),

	  m_fYawSpeed(0.0f),
	  m_fPitchSpeed(0.0f),

	  m_fSweepYawSpeed(0.0f),

	  m_fVisualRange(0.0f),

	  m_bStartOff(LTFALSE),

	  m_bHasGunModel(LTFALSE),
	  m_bAttachGun(LTFALSE),
	  m_hGunHandle(LTNULL),
	  m_vGunOffset(0.0f,0.0f,0.0f),
	  m_hGunAttachment(LTNULL),
	  m_vAimingOffset(0,0,0),

	  m_bUseAwakeSound(LTFALSE),
	  m_bUseDetectSound(LTFALSE),

	  m_fMaxYaw(0.0f),
	  m_fMinYaw(0.0f),

	  m_fMinPitch(0.0f),
	  m_fMaxPitch(0.0f),
		
	  m_fSweepPauseTime(0.0f),

	  m_vInitialPosition(0.0f,0.0f,0.0f),
	  m_rInitialRotation(0.0f,0.0f,0.0f,1.0f),
	  m_vInitialAngles(0.0f,0.0f,0.0f),
	  m_vInitialUp(0.0f,0.0f,0.0f),
	  m_vInitialRight(0,0,0),

	  m_fMaxIdleTime(0.0f),

	  m_hOpeningAni(INVALID_MODEL_ANIM),
	  m_hOpenedAni(INVALID_MODEL_ANIM),
	  m_hClosingAni(INVALID_MODEL_ANIM),
	  m_hClosedAni(INVALID_MODEL_ANIM),

	  m_hObjective(LTNULL),
	  m_pCharObjective(LTNULL),
	  m_vObjectiveOffset(0.0f,0.0f,0.0f),
	  m_ObjectiveType(Ignore),

	  m_hWarningSound(LTNULL),
	  m_hMovingSound(LTNULL),
	  m_hFiringSound(LTNULL),

	  m_bFirstUpdate(LTTRUE),
	  m_bOn(LTTRUE),
		
	  m_fYaw(0.0f),
	  m_fPitch(0.0f),

	  m_vRight(1,0,0),
	  m_vUp(0,1,0),
	  m_vForward(0,0,1),

	  m_bFiring(LTFALSE),
	  m_bFiredWeapon(LTFALSE),

	  m_bMoving(LTFALSE),

	  // m_tmrIdle
	  // m_tmrPause
	  // m_tmrBurstDelay

	  m_bCanDamage(LTTRUE)
{
	AddAggregate(&m_TurretVision);
	AddAggregate(&m_GunMgr);

	SetupFSM();

	m_TurretVision.SetFullCallback( m_TurretVision.MakeCallback(*this,&Turret::SeeCharacter) );
}



Turret::~Turret()
{
	if( m_hMovingSound ) g_pLTServer->KillSound(m_hMovingSound);
	m_hMovingSound = LTNULL;

	if( m_hWarningSound ) g_pLTServer->KillSound(m_hWarningSound);
	m_hWarningSound = LTNULL;

	if( m_hFiringSound ) g_pLTServer->KillSound(m_hFiringSound);
	m_hFiringSound = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Turret::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			if( m_bHasGunModel && m_hGunHandle )
			{
				LPBASECLASS pGunModel = g_pLTServer->HandleToObject(m_hGunHandle);
				if( pGunModel )
					pGunModel->EngineMessageFn(messageID,pData,fData);
			}

			uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				PostPropRead((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			// Our initial update needs to be called after Prop's becuase
			// prop call SetNextUpdate(m_hObject,0.0f).
			if( m_bHasGunModel && m_hGunHandle )
			{
				LPBASECLASS pGunModel = g_pLTServer->HandleToObject(m_hGunHandle);
				if( pGunModel )
					pGunModel->EngineMessageFn(messageID,pData,fData);
			}

			uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
			
			return dwRet;
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

	if( m_bHasGunModel && m_hGunHandle )
	{
		LPBASECLASS pGunModel = g_pLTServer->HandleToObject(m_hGunHandle);
		if( pGunModel )
			pGunModel->EngineMessageFn(messageID,pData,fData);
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 Turret::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	if (!g_pLTServer) return 0;

	if( m_bHasGunModel && m_hGunHandle )
	{
		LPBASECLASS pGunModel = g_pLTServer->HandleToObject(m_hGunHandle);
		if( pGunModel )
		{
			pGunModel->ObjectMessageFn(hSender, messageID,hRead);
			hRead->ResetPos();
		}

	}

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			CommonLT* pCommon = g_pLTServer->Common();
			if (!pCommon) return 0;

			LTString hMsg = g_pLTServer->ReadFromMessageHString(hRead);
			const char* szMsg = hMsg.CStr();
			if (!szMsg) return 0;

			ConParse parse;
			parse.Init(const_cast<char*>(szMsg) );

			while (pCommon->Parse(&parse) == LT_OK)
			{
				if (parse.m_nArgs > 0 && parse.m_Args[0])
				{
					if( stricmp(parse.m_Args[0], "OFF") == 0 )
					{
						if( m_bOn )
						{
							m_RootFSM.AddMessage(Root::eTurnOff);
							m_bOn = LTFALSE;
						}
						
						return LTTRUE;
					}
					else if( stricmp(parse.m_Args[0], "ON") == 0 )
					{
						if( !m_bOn )
						{
							m_RootFSM.AddMessage(Root::eTurnOn);
							m_bOn = LTTRUE;
						}
						SetNextUpdate(UPDATE_DELTA);

						return LTTRUE;
					}
					else if( stricmp(parse.m_Args[0], "SLEEP") == 0 )
					{
						if( m_bOn ) // if off, it is already "sleeping"
						{
							m_RootFSM.AddMessage(Root::eSleep);
							m_bOn = LTTRUE;
						}
						SetNextUpdate(UPDATE_DELTA);

						return LTTRUE;
					}
					else if( stricmp(parse.m_Args[0], "SEETHROUGH") == 0 )
					{
						if( parse.m_nArgs > 1 && parse.m_Args[1] )
						{
							m_TurretVision.SetCanSeeThrough(IsTrueChar(*parse.m_Args[1]));

							return LTTRUE;
						}
#ifndef _FINAL
						else
						{
							AIErrorPrint(m_hObject, "SEETHROUGH missing argument");
						}
#endif
					}
					else if( stricmp(parse.m_Args[0], "SHOOTTHROUGH") == 0 )
					{
						if( parse.m_nArgs > 1 && parse.m_Args[1] )
						{
							m_TurretVision.SetCanShootThrough(IsTrueChar(*parse.m_Args[1]));

							return LTTRUE;
						}
#ifndef _FINAL
						else
						{
							AIErrorPrint(m_hObject, "SHOOTTHROUGH missing argument");
						}
#endif
					}
					else if( stricmp(parse.m_Args[0], "TRG") == 0 )
					{
						if( parse.m_nArgs > 1 && parse.m_Args[1] )
						{
							const char * szObjectName = parse.m_Args[1];

							if( stricmp(szObjectName, "OFF") == 0 )
							{
								if( m_hObjective && m_ObjectiveType == Override )
								{
									m_hObjective = LTNULL;
									m_pCharObjective = LTNULL;
									m_ObjectiveType = Ignore;
									m_vObjectiveOffset = LTVector(0,0,0);

									return LTTRUE;
								}
							}
							else if( stricmp( szObjectName, "Player" ) == 0)
							{
								// Find the closest player.
								CCharacterMgr::PlayerIterator closest_player = std::min_element( g_pCharacterMgr->BeginPlayers(),
									                                                             g_pCharacterMgr->EndPlayers(), 
																								 ClosestTo( m_vInitialPosition ) );
								// Target that player.
								ASSERT( closest_player != g_pCharacterMgr->EndPlayers() );
								if( closest_player != g_pCharacterMgr->EndPlayers() )
								{
									m_hObjective = (*closest_player)->m_hObject;
									m_pCharObjective = (*closest_player);
									m_ObjectiveType = Override;
									m_vObjectiveOffset = (*closest_player)->GetChestOffset();
								}

							}
							else 
							{
								// Find the named object.
								HOBJECT hNamedObject = LTNULL;
								if( LT_OK != FindNamedObject(szObjectName, hNamedObject) )
								{
#ifndef _FINAL
									AIErrorPrint( m_hObject, "Object \"%s\" does not exist or is not a unique name!", szObjectName);
#endif
									return LTTRUE;
								}
								
								ASSERT( hNamedObject );

								// And target that object.
								m_hObjective = hNamedObject;
								m_pCharObjective = LTNULL;
								m_ObjectiveType = Override;
								m_vObjectiveOffset = LTVector(0,0,0);

								if( CCharacter * pCharacter = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(m_hObjective) ) )
								{
									m_vObjectiveOffset = pCharacter->GetChestOffset();
									m_pCharObjective = pCharacter;
								}
							}

							if( m_bOn ) m_RootFSM.AddMessage(Root::eNewTargetDetected);
							return LTTRUE;
						}
#ifndef _FINAL
						else
						{
							AIErrorPrint(m_hObject, "Command \"TRG\" missing argument.  Format is \"TRG object_name\".");
						}
#endif
					}

				} //if (parse.m_nArgs > 0 && parse.m_Args[0])

			} //while (pCommon->Parse(&parse) == LT_OK)
		} 
		break;

		case MID_DAMAGE:
		{
			// Let our damage aggregate process the message first...

			uint32 dwRet = Prop::ObjectMessageFn(hSender, messageID, hRead);

			// Check to see if we have been destroyed

			if (m_damage.IsDead())
			{
				if( m_bHasGunModel && m_hGunHandle)
				{
					if( m_hGunAttachment )
					{
						g_pLTServer->RemoveAttachment(m_hGunAttachment);
						m_hGunAttachment = LTNULL;
					}

					g_pLTServer->RemoveObject(m_hGunHandle);
				}					
					
				m_RootFSM.AddMessage(Root::eDestroyed);
			}
			return dwRet;
		}
		break;

		default : break;
	}

	return Prop::ObjectMessageFn(hSender, messageID, hRead);
}


void   Turret::Destroyed(LTBOOL val /* = LTTRUE */)
{ 
	m_RootFSM.AddMessage(Root::eDestroyed); 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::ReadButes
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //




void Turret::ReadButes(const char * tag_name)
{
	CHierarchicalButeMgr bute_mgr;
	bute_mgr.Init(g_pLTServer, g_szAIButeMgrFile);

	m_strGunFilename = bute_mgr.GetString(tag_name,"GunModel");
	m_strGunSkin     = bute_mgr.GetString(tag_name,"GunSkin");
	m_strDeadGunFilename = bute_mgr.GetString(tag_name,"DeadGunModel");
	m_strDeadGunSkin = bute_mgr.GetString(tag_name,"DeadGunSkin");
	m_fGunMaxHitPoints = (LTFLOAT)bute_mgr.GetDouble(tag_name,"HitPoints", DEFAULT_GUN_HIT_POINTS );

	m_strGunBaseSocket = bute_mgr.GetString(tag_name,"GunBaseSocket");
	m_strTurretBaseSocket    = bute_mgr.GetString(tag_name,"TripodBaseSocket");

	m_strAwakeSound = bute_mgr.GetString(tag_name,"AwakeSound","");
	m_strDetectSound = bute_mgr.GetString(tag_name,"DetectSound","");
	m_strMoveSound   = bute_mgr.GetString(tag_name,"MoveSound","");


	m_fWarningSoundDistance = (LTFLOAT)bute_mgr.GetDouble(tag_name,"WarningSoundDistance", 500.0f);
	m_fMovingSoundDistance  = (LTFLOAT)bute_mgr.GetDouble(tag_name,"MovingSoundDistance", 200.0f);

	WEAPON * pWeapon = g_pWeaponMgr->GetWeapon(bute_mgr.GetString(tag_name,"WeaponType","Pulse Rifle"));
	if( pWeapon )
		m_nWeaponId = pWeapon->nId;

	m_fDamageMultiplier = (LTFLOAT)bute_mgr.GetDouble(tag_name,"DamageMultiplier", 1.0f);
	
	m_fPitchSpeed = (LTFLOAT)bute_mgr.GetDouble(tag_name,"PitchSpeed", 0.0f);
	m_fYawSpeed   = (LTFLOAT)bute_mgr.GetDouble(tag_name,"YawSpeed", 0.0f);

	m_fSweepYawSpeed = (LTFLOAT)bute_mgr.GetDouble(tag_name,"SweepYawSpeed", 0.0f);

	m_OpeningAnimName = bute_mgr.GetString(tag_name, "OpeningAnim" );
	m_OpenedAnimName = bute_mgr.GetString(tag_name, "OpenedAnim" );
	m_ClosingAnimName = bute_mgr.GetString(tag_name, "ClosingAnim" );
	m_ClosedAnimName = bute_mgr.GetString(tag_name, "ClosedAnim" );

	m_fBurstDelay = (LTFLOAT)bute_mgr.GetDouble(tag_name, "BurstSpeed",0.0f);
	if( m_fBurstDelay > 0.0f )
		m_fBurstDelay = 1.0f / m_fBurstDelay;
	else if ( m_fBurstDelay < 0.0f )
		m_fBurstDelay = 0.0f;

	m_nStartFireSound = bute_mgr.GetInt(tag_name, "StartFireSound", WMKS_INVALID);
	m_nLoopFireSound = bute_mgr.GetInt(tag_name, "LoopFireSound", WMKS_INVALID);
	m_nStopFireSound = bute_mgr.GetInt(tag_name, "StopFireSound", WMKS_INVALID);
}

	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

static Turret::ObjectiveType GetObjectiveType(const char * szType )
{
	if( stricmp(szType,g_szObjectiveTypes[0]) == 0 )
	{
		return Turret::Ignore;
	}
	else if( stricmp(szType,g_szObjectiveTypes[1]) == 0 )
	{
		return Turret::Interest;
	}
	else if( stricmp(szType,g_szObjectiveTypes[2]) == 0 )
	{
		return Turret::Target;
	}
	else if( stricmp(szType,g_szObjectiveTypes[3]) == 0 )
	{
		return Turret::Prefer;
	}

	_ASSERT(0);
	return Turret::Ignore;
}


LTBOOL Turret::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) return LTFALSE;

	GenericProp genProp;

	ReadButes( GetStringProp( genProp, "AttributeName" ).c_str() );

	m_fMaxYaw = (LTFLOAT)GetDoubleProp(genProp,"MaxYaw");
	m_fMinYaw = (LTFLOAT)GetDoubleProp(genProp,"MinYaw");

	m_fMaxPitch = (LTFLOAT)GetDoubleProp(genProp,"MaxPitch");
	m_fMinPitch = (LTFLOAT)GetDoubleProp(genProp,"MinPitch");

	m_fSweepPauseTime = (LTFLOAT)GetDoubleProp(genProp,"SweepPauseTime");

	m_fVisualRange = (LTFLOAT)GetDoubleProp(genProp,"VisualRange");
	
	m_bStartOff = GetBoolProp(genProp,"StartOff");

	m_TurretVision.SetCanSeeThrough( GetBoolProp(genProp,"CanSeeThrough", LTFALSE) );
	m_TurretVision.SetCanShootThrough( GetBoolProp(genProp,"CanShootThrough", LTFALSE) );

	m_fMaxIdleTime = (LTFLOAT)GetDoubleProp(genProp,"MaxIdleTime");

	m_bUseAwakeSound  = GetBoolProp(genProp, "UseAwakeSound");
	m_bUseDetectSound = GetBoolProp(genProp, "UseDetectSound");
	m_bAttachGun      = GetBoolProp(genProp, "AttachToBase" );


	LTFLOAT fTempFloat = (LTFLOAT)GetDoubleProp(genProp, "TargetDelay");
	if( fTempFloat > 0.0f )	
		m_TurretVision.SetReactionRate( 1.0f / fTempFloat );
	else
		m_TurretVision.SetReactionRate( 1e10f );

	fTempFloat = (LTFLOAT)GetDoubleProp(genProp, "TargetTimeOut");
	if( fTempFloat > 0.0f )	
		m_TurretVision.SetDeactionRate( 1.0f / fTempFloat );
	else
		m_TurretVision.SetDeactionRate( 0.0f );

	m_aClassTypes[UNKNOWN] = Ignore;
	m_aClassTypes[MARINE] = GetObjectiveType( GetStringProp(genProp, "Marine").c_str() );
	m_aClassTypes[CORPORATE] = GetObjectiveType( GetStringProp(genProp, "Corporate").c_str() );
	m_aClassTypes[PREDATOR] = GetObjectiveType( GetStringProp(genProp, "Predator").c_str() );
	m_aClassTypes[ALIEN] = GetObjectiveType( GetStringProp(genProp, "Alien").c_str() );

	m_TurretVision.SetRanks(m_aClassTypes[MARINE],m_aClassTypes[CORPORATE],m_aClassTypes[PREDATOR],m_aClassTypes[ALIEN]);

	m_RootFSM.DefineTransition(Root::eIdleTimedOut, GetIdleRootState(GetStringProp( genProp, "IdleTimeOut" ).c_str()) );
	m_RootFSM.DefineTransition(Root::eStateIdle, GetIdleRootState(GetStringProp( genProp, "Idle" ).c_str()) );

	Root::State eDetectInterestState = GetRootState(GetStringProp( genProp, "DetectInterest" ).c_str());
	Root::State eDetectTargetState = GetRootState(GetStringProp( genProp, "DetectTarget" ).c_str());

	m_RootFSM.DefineTransition(Root::eNewInterestDetected,  eDetectInterestState );
	m_RootFSM.DefineTransition(Root::eNewTargetDetected, eDetectTargetState );

	
	m_TurretVision.SetCanBeShot( ( eDetectInterestState == Root::eStateFire && m_aClassTypes[MARINE] == Interest )
		                         || ( eDetectTargetState == Root::eStateFire && m_aClassTypes[MARINE] > Interest ),

									( eDetectInterestState == Root::eStateFire && m_aClassTypes[CORPORATE] == Interest )
		                         || ( eDetectTargetState == Root::eStateFire && m_aClassTypes[CORPORATE] > Interest ),

								 ( eDetectInterestState == Root::eStateFire && m_aClassTypes[PREDATOR] == Interest )
		                         || ( eDetectTargetState == Root::eStateFire && m_aClassTypes[PREDATOR] > Interest ),
								 
								 ( eDetectInterestState == Root::eStateFire && m_aClassTypes[ALIEN] == Interest )
		                         || ( eDetectTargetState == Root::eStateFire && m_aClassTypes[ALIEN] > Interest ) );

	return LTTRUE;
}


Turret::Root::State Turret::GetRootState(const char * state_name)
{
	if( stricmp(state_name,g_szActions[0]) == 0 )
	{
		return Root::eStateBeginClosing;
	}
	else if( stricmp( state_name, g_szActions[1] ) == 0 )
	{
		return Root::eStateSweep;
	}
	else if( stricmp( state_name, g_szActions[2] ) == 0 )
	{
		return Root::eStateTrack;
	}
	else if( stricmp( state_name, g_szActions[3] ) == 0 )
	{
		return Root::eStateFire;
	}

	_ASSERT(0);
	return Root::eStateBeginClosing;
}

Turret::Root::State Turret::GetIdleRootState(const char * state_name)
{
	if( stricmp(state_name,g_szIdleActions[0]) == 0 )
	{
		return Root::eStateBeginClosing;
	}
	else if( stricmp( state_name, g_szIdleActions[1] ) == 0 )
	{
		return Root::eStateSweep;
	}

	_ASSERT(0);
	return Root::eStateBeginClosing;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void Turret::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// Convert all our stuff into radians
	m_fMaxYaw = MATH_DEGREES_TO_RADIANS(m_fMaxYaw);
	m_fMinYaw = MATH_DEGREES_TO_RADIANS(m_fMinYaw);
	m_fYawSpeed = MATH_DEGREES_TO_RADIANS(m_fYawSpeed);
	m_fSweepYawSpeed = MATH_DEGREES_TO_RADIANS(m_fSweepYawSpeed);
	_ASSERT( m_fYawSpeed >= 0.0f );

	m_fMaxPitch = MATH_DEGREES_TO_RADIANS(m_fMaxPitch);
	m_fMinPitch = MATH_DEGREES_TO_RADIANS(m_fMinPitch);
	m_fPitchSpeed = MATH_DEGREES_TO_RADIANS(m_fPitchSpeed);
	_ASSERT( m_fPitchSpeed >= 0.0f );

	// Be sure mins are less than maxes.
	if( m_fMinYaw > m_fMaxYaw )
	{
		LTFLOAT temp = m_fMaxYaw;
		m_fMaxYaw = m_fMinYaw;
		m_fMinYaw = temp;
	}

	if( m_fMinPitch > m_fMaxPitch )
	{
		LTFLOAT temp = m_fMaxPitch;
		m_fMaxPitch = m_fMinPitch;
		m_fMinPitch = temp;
	}
}


	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL Turret::InitialUpdate()
{
	uint32 dwInitUserFlags = USRFLG_MOVEABLE;

	uint32 dwSTFlag = SurfaceToUserFlag((SurfaceType)ST_UNKNOWN);
	uint32 dwUserFlags = UserFlagSurfaceMask(dwInitUserFlags);

	g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags | dwSTFlag);

	// Make sure the base cannot be moved.
	m_damage.SetMass(INFINITE_MASS);

	// Record our can damage property.
	m_bCanDamage = m_damage.GetCanDamage();

	// Get our initial rotation.
	if( m_bHasGunModel )
	{
		GetSocketPos(m_hObject, LTNULL, m_strTurretBaseSocket.c_str(),&m_vInitialPosition);
	}
	else
	{
		g_pLTServer->GetObjectPos(m_hObject, &m_vInitialPosition);
	}

	g_pLTServer->GetObjectRotation(m_hObject, &m_rInitialRotation);


	LTVector vF; // not used
	g_pMathLT->GetRotationVectors(m_rInitialRotation,m_vInitialRight,m_vInitialUp,vF);
	g_pMathLT->GetEulerAngles(m_rInitialRotation,m_vInitialAngles);
	m_vInitialAngles.x = -m_vInitialAngles.x;

	m_vRight = m_vInitialRight;
	m_vUp = m_vInitialUp;
	m_vForward = vF;

	// All angles will be relative to our initial angle, so our current angle is 0.0f.
	m_fYaw = 0.0f;
	m_fPitch = 0.0f;

	//
	// Create the gun
	//

	if( !m_strGunFilename.empty() && !m_strGunSkin.empty() )
	{
		m_bHasGunModel = LTTRUE;

		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);

#ifdef _DEBUG
		sprintf(theStruct.m_Name, "%s-GunModel", g_pLTServer->GetObjectName(m_hObject) );
#endif

		theStruct.m_Pos = m_vInitialPosition;
		theStruct.m_Rotation = m_rInitialRotation;

		SAFE_STRCPY(theStruct.m_Filename, m_strGunFilename.c_str());
		SAFE_STRCPY(theStruct.m_SkinName, m_strGunSkin.c_str());

		theStruct.m_NextUpdate = 1.0f;

		const uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);

		HCLASS hClass = g_pLTServer->GetClass("TurretWeapon");
		char szProps[1024];
		sprintf(szProps,"HitPoints %f ; DestroyedModel \"%s\" ; DestroyedSkin \"%s\" ; Visible %d ; Solid 0 ; RayHit %d ",
				m_fGunMaxHitPoints, m_strDeadGunFilename.c_str(), m_strDeadGunSkin.c_str(),
				(dwFlags & FLAG_VISIBLE) ? 1 : 0,
				(dwFlags & FLAG_RAYHIT) ? 1 : 0  );

		TurretWeapon * pGunModel = dynamic_cast<TurretWeapon*>(
			g_pLTServer->CreateObjectProps(hClass, &theStruct, szProps));

		ASSERT( pGunModel );

		if( pGunModel )
		{

			pGunModel->Init(this);

			pGunModel->GetDestructible()->SetCanDamage( m_bCanDamage );
			pGunModel->GetDestructible()->SetDebrisId( GetDestructible()->GetDebrisId() );

			// Store gun handle.
			m_hGunHandle = pGunModel->m_hObject;

			// Determine offset.
			if( !m_strGunBaseSocket.empty() )
			{
				GetSocketPos(m_hGunHandle, m_hGunAttachment, m_strGunBaseSocket.c_str(),&m_vGunOffset,LTFALSE);
				m_vGunOffset = -m_vGunOffset;
			}
			else
			{
				m_vGunOffset = LTVector(0,0,0);
			}
			
			// Place gun.
			PositionAndRotation offsets = GetPositionAndRotateOffset(m_hGunHandle, m_vInitialPosition, LTRotation(0.0f,0.0f,0.0f,1.0f),m_vGunOffset,m_rInitialRotation);
			if( m_bAttachGun )
			{
				LTVector vPos = offsets.vPos - m_vInitialPosition;
				g_pServerDE->CreateAttachment( m_hObject, m_hGunHandle, const_cast<char*>(m_strTurretBaseSocket.c_str()), 
											   &vPos, &offsets.rRot, &m_hGunAttachment);
			}
			else
			{
				g_pLTServer->TeleportObject(m_hGunHandle,&offsets.vPos );
				g_pLTServer->RotateObject(m_hGunHandle, &offsets.rRot);
			}
		}
	} 
	else  //!( !m_strGunFilename.c_str() && !m_strGunSkin.c_str() )
	{
		m_bHasGunModel = LTFALSE;
		m_hGunHandle = m_hObject;
		m_bAttachGun = LTFALSE;
	}

	//
	// Set up the Weapon Manager
	//
	m_GunMgr.Init(m_hObject);
	m_GunMgr.ObtainWeapon(m_nWeaponId);
	m_GunMgr.ChangeWeapon(m_nWeaponId);

	// Do set-up stuff for the weapon.
	CWeapon * pWeapon = m_GunMgr.GetCurWeapon();
	if( pWeapon )
	{
//		LTVector vGunPos;
//		g_pLTServer->GetObjectPos(m_hGunHandle, &vGunPos);
		
		// Cache the weapon stuff.
		pWeapon->CacheFiles();

		// Set up the aiming offset.  It is the average of all the flash socket offsets.
		LTVector vCurrentFlashPos(0,0,0);
		LTVector vSumFlashOffsets(0,0,0);
		LTFLOAT fNumFlashes = 0;
		const uint32 nNumSockets = pWeapon->GetNumFlashSockets();
		for( uint32 i = 0; i < nNumSockets; ++i )
		{
			const char * const szFlashSocketName = pWeapon->GetFlashSocketName(i);
			if( szFlashSocketName && szFlashSocketName[0] )
			{
				if( LT_OK == GetSocketPos(m_hGunHandle, m_hGunAttachment, szFlashSocketName,&vCurrentFlashPos, LTFALSE) )
				{
					vSumFlashOffsets += vCurrentFlashPos; //(vCurrentFlashPos - vGunPos);
					fNumFlashes += 1.0f;
				}
			}
		}

		if( fNumFlashes > 0 )
		{
			m_vAimingOffset = vSumFlashOffsets / fNumFlashes;
		}
	}

	// Get the opening and closing ani's
	if( !m_OpeningAnimName.empty() )
		m_hOpeningAni = g_pLTServer->GetAnimIndex(m_hObject, const_cast<char*>(m_OpeningAnimName.c_str()) );

	if( !m_OpenedAnimName.empty() )
		m_hOpenedAni = g_pLTServer->GetAnimIndex(m_hObject, const_cast<char*>(m_OpenedAnimName.c_str()) );

	if( !m_ClosingAnimName.empty() )
		m_hClosingAni = g_pLTServer->GetAnimIndex(m_hObject, const_cast<char*>(m_ClosingAnimName.c_str()) );

	if( !m_ClosedAnimName.empty() )
		m_hClosedAni = g_pLTServer->GetAnimIndex(m_hObject, const_cast<char*>(m_ClosedAnimName.c_str()) );

	g_pLTServer->SetModelLooping(m_hObject,LTFALSE);

	// Start the state machines rolling.
	
	m_RootFSM.SetState( Root::eStateIdle );
	m_RootFSM.AddMessage(Root::eIdleTimedOut);

	if (m_fYaw > 0.0f)
	{
		m_SweepFSM.SetState(Sweep::eStateTurningToMax);
	}
	else
	{
		m_SweepFSM.SetState(Sweep::eStateTurningToMin);
	}

	m_tmrIdle.Stop(); // Set to off.

	// Get the updates going.
	SetNextUpdate(m_bOn ? UPDATE_DELTA : 0.0f, UPDATE_DELTA + 1.0f);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

LTBOOL Turret::Update()
{
/*	uint32 nBaseFlags;
	uint32 nBaseFlags2;

	g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags, nBaseFlags);
	g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags2, nBaseFlags2);

	uint32 nWeaponFlags = 0;
	uint32 nWeaponFlags2 = 0;

	if( m_hGunHandle )
	{
		g_pLTServer->Common()->GetObjectFlags(m_hGunHandle, OFT_Flags, nWeaponFlags);
		g_pLTServer->Common()->GetObjectFlags(m_hGunHandle, OFT_Flags2, nWeaponFlags2);
	}

	AICPrint(m_hObject, "base flags %x %x, Rayhit %s Touch %s.  Model Flags %x %x, Rayhit %s Touch %s.",
		nBaseFlags, nBaseFlags2, (nBaseFlags & FLAG_RAYHIT) ? "1" : "0", (nBaseFlags & FLAG_TOUCH_NOTIFY) ? "1" : "0", 
		nWeaponFlags, nWeaponFlags2, (nWeaponFlags & FLAG_RAYHIT) ? "1" : "0", (nWeaponFlags & FLAG_TOUCH_NOTIFY) ? "1" : "0" );
*/

	if (m_bFirstUpdate)
	{
		FirstUpdate();
		m_bFirstUpdate = LTFALSE;
	}

	SetNextUpdate(UPDATE_DELTA);

	// Update our position and rotation relative to the base (in case the base has moved).
	if( m_bHasGunModel )
	{
		LTVector vNewPos;
		LTRotation rNewRot;
		GetSocketPosAndRot(m_hObject,LTNULL,m_strTurretBaseSocket.c_str(),&vNewPos,&rNewRot);

		if( vNewPos != m_vInitialPosition || !(rNewRot == m_rInitialRotation) )
		{
			m_vInitialPosition = vNewPos;
			m_rInitialRotation = rNewRot;

			TurnTo(m_fPitch,m_fYaw,m_fPitchSpeed,m_fYawSpeed,LTTRUE);
		}
	}
	else
	{
		// Be sure to update our position to account for any physics.
		g_pLTServer->GetObjectPos(m_hObject,&m_vInitialPosition);
		g_pLTServer->GetObjectRotation(m_hObject,&m_rInitialRotation);

		// Remove the rotation due to targeting from our rotation.
		g_pMathLT->EulerRotateY(m_rInitialRotation, -m_fYaw );
		g_pMathLT->EulerRotateX(m_rInitialRotation, m_fPitch );
	}

	
	// Record our rotation.
	LTRotation rCurrentRotation = m_rInitialRotation;
	g_pMathLT->EulerRotateY(rCurrentRotation, m_fYaw );
	g_pMathLT->EulerRotateX(rCurrentRotation, -m_fPitch );
	
	g_pMathLT->GetRotationVectors(rCurrentRotation, m_vRight, m_vUp, m_vForward);


	// Clean-up our warning sound.
	if( m_hWarningSound && SoundIsDone(m_hWarningSound) )
	{
		g_pLTServer->KillSound( m_hWarningSound );
		m_hWarningSound = LTNULL;
	}


	// Detect if we fired our weapon this update or not.
	const LTBOOL bOldFiredWeapon = m_bFiredWeapon;
	m_bFiredWeapon = LTFALSE;

	// Detect if we are moving this update or not.
	m_bMoving = LTFALSE;

	// Detect an idle time-out.
	if( !m_hObjective && m_tmrIdle.On() && m_tmrIdle.Stopped() )
	{
		m_tmrIdle.Stop(); // Set to off.
		m_RootFSM.AddMessage(Root::eIdleTimedOut);
	}


	m_RootFSM.Update();

	// See if we should play our movement sound.
	if( !m_strMoveSound.empty() && m_bMoving )
	{
		if( !m_hMovingSound )
		{
			m_hMovingSound = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, const_cast<char*>(m_strMoveSound.c_str()), 
				m_fMovingSoundDistance,SOUNDPRIORITY_MISC_HIGH, PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP );
		}
	}
	else if( m_hMovingSound )
	{
		g_pLTServer->KillSound( m_hMovingSound );
		m_hMovingSound = LTNULL;
	}


	// See if our weapon firing status has changed.
	if( m_bFiredWeapon != bOldFiredWeapon )
	{
 		const BARREL * pBarrel = LTNULL;
		
		if( const CWeapon * pWeapon = m_GunMgr.GetCurWeapon() )
		{
			pBarrel = pWeapon->GetBarrelData();
		}


		// Did we start firing our weapon?
		if( m_bFiredWeapon )
		{
			// Play the start fire sound (if there is one).
			if( m_nStartFireSound > WMKS_INVALID && pBarrel)
			{
				if( const char * szSoundName = GetWeaponSound(*pBarrel, m_nStartFireSound) )
				{
					g_pServerSoundMgr->PlaySoundFromObject(m_hObject, 
										const_cast<char*>(szSoundName), pBarrel->fFireSoundRadius, 
										SOUNDPRIORITY_AI_HIGH);
				}
			}

			// Kill any left over looping sound.
			if( m_hFiringSound )
			{
				g_pLTServer->KillSound(m_hFiringSound);
				m_hFiringSound = LTNULL;
			}

			// Start the looping sound.
			if( m_nLoopFireSound > WMKS_INVALID && pBarrel)
			{

				if( const char * szSoundName = GetWeaponSound(*pBarrel, m_nLoopFireSound) )
				{
					m_hFiringSound = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, 
										const_cast<char*>(szSoundName), pBarrel->fFireSoundRadius, 
										SOUNDPRIORITY_AI_MEDIUM, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);
				}
			}
		}
		// Otherwise, we stopped firing our weapon.
		else
		{
			// Kill the looping sound.
			if( m_hFiringSound )
			{
				g_pLTServer->KillSound(m_hFiringSound);
				m_hFiringSound = LTNULL;
			}

			// Play the end fire sound.
			if( m_nStopFireSound > WMKS_INVALID && pBarrel )
			{
				if( const char * szSoundName = GetWeaponSound(*pBarrel, m_nStopFireSound) )
				{
					g_pServerSoundMgr->PlaySoundFromObject(m_hObject, 
										const_cast<char*>(szSoundName), pBarrel->fFireSoundRadius, 
										SOUNDPRIORITY_AI_HIGH);
				}
			}

		} //if( m_bFiredWeapon )

	} //if( m_bFiredWeapon != bOldFiredWeapon )


	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::FirstUpdate()
//
//	PURPOSE:	First Update
//
// ----------------------------------------------------------------------- //

void Turret::FirstUpdate()
{
	if( m_bStartOff )
	{
		m_RootFSM.AddMessage(Root::eTurnOff);
		m_bOn = LTFALSE;
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::SetupFSM()
//
//	PURPOSE:	Define the transitions and such for the FSMs.
//
// ----------------------------------------------------------------------- //

void Turret::SetupFSM()
{
	//
	// Set up the root FSM.
	//
	// The idle state will fall into begin closing or sweep, depending on how it is set in ReadProp.
	m_RootFSM.DefineState( Root::eStateIdle,  m_RootFSM.MakeCallback(),
											  m_RootFSM.MakeCallback(*this,&Turret::StartIdleTimer) );

	m_RootFSM.DefineState( Root::eStateSweep, m_RootFSM.MakeCallback(*this,&Turret::DoSweep) );

	m_RootFSM.DefineState( Root::eStateTrack, m_RootFSM.MakeCallback(*this,&Turret::DoTrack),
		                                      m_RootFSM.MakeCallback(*this,&Turret::StartTrack),
											  m_RootFSM.MakeCallback(*this,&Turret::EndTrack) );


	m_RootFSM.DefineState( Root::eStateFire, m_RootFSM.MakeCallback(),
		                                     m_RootFSM.MakeCallback(*this,&Turret::StartFire) );

	m_RootFSM.DefineState( Root::eStateDestroyed, m_RootFSM.MakeCallback(),
		                                          m_RootFSM.MakeCallback(*this,&Turret::DoDestroyed) );

	m_RootFSM.DefineState( Root::eStateOpening, m_RootFSM.MakeCallback(*this,&Turret::DoOpening),
												m_RootFSM.MakeCallback(*this,&Turret::StartOpening) );

	m_RootFSM.DefineState( Root::eStateBeginClosing, m_RootFSM.MakeCallback(*this,&Turret::DoBeginClosing) );

	m_RootFSM.DefineState( Root::eStateClosing, m_RootFSM.MakeCallback(*this,&Turret::DoClosing),
												m_RootFSM.MakeCallback(*this,&Turret::StartClosing) );

	m_RootFSM.DefineState( Root::eStateClosed, m_RootFSM.MakeCallback(),
											   m_RootFSM.MakeCallback(*this,&Turret::StartClosed),
											   m_RootFSM.MakeCallback(*this,&Turret::EndClosed) );


	m_RootFSM.DefineTransition( Root::eObjectiveGone, Root::eStateIdle );
	m_RootFSM.DefineTransition( Root::eSleep, Root::eStateBeginClosing );
	m_RootFSM.DefineTransition( Root::eStateFire, Root::eStateTrack );
	m_RootFSM.DefineTransition( Root::eTurnOn, Root::eStateOpening );
	m_RootFSM.DefineTransition( Root::eTurnOff, Root::eStateBeginClosing );
	m_RootFSM.DefineTransition(Root::eDestroyed, Root::eStateDestroyed);

	m_RootFSM.DefineTransition( Root::eStateOpening, Root::eCompleted, Root::eStateIdle );
	m_RootFSM.DefineTransition( Root::eStateBeginClosing, Root::eCompleted, Root::eStateClosing );
	m_RootFSM.DefineTransition( Root::eStateClosing, Root::eCompleted, Root::eStateClosed );

	// The begin closing states should ignore objective gone.
	m_RootFSM.DefineTransition( Root::eStateBeginClosing, Root::eObjectiveGone, Root::eStateBeginClosing );

	// The closing states should ignore any change in objective and just keep closing.
	m_RootFSM.DefineTransition( Root::eStateClosing, Root::eNewInterestDetected, Root::eStateOpening );
	m_RootFSM.DefineTransition( Root::eStateClosing, Root::eNewTargetDetected, Root::eStateOpening );
	m_RootFSM.DefineTransition( Root::eStateClosing, Root::eObjectiveGone, Root::eStateClosing );

	// The closing state should start opening if there is any change in the objective.
	m_RootFSM.DefineTransition( Root::eStateClosed, Root::eNewInterestDetected, Root::eStateOpening );
	m_RootFSM.DefineTransition( Root::eStateClosed, Root::eNewTargetDetected, Root::eStateOpening );
	m_RootFSM.DefineTransition( Root::eStateClosed, Root::eObjectiveGone, Root::eStateClosed );
	
	// Make sure destroyed state ignores all messages.
	m_RootFSM.DefineTransition(Root::eStateDestroyed, Root::eCompleted , Root::eStateDestroyed);
	m_RootFSM.DefineTransition(Root::eStateDestroyed, Root::eIdleTimedOut , Root::eStateDestroyed);
	m_RootFSM.DefineTransition(Root::eStateDestroyed, Root::eNewInterestDetected , Root::eStateDestroyed);
	m_RootFSM.DefineTransition(Root::eStateDestroyed, Root::eNewTargetDetected , Root::eStateDestroyed);
	m_RootFSM.DefineTransition(Root::eStateDestroyed, Root::eObjectiveGone , Root::eStateDestroyed);
	m_RootFSM.DefineTransition(Root::eStateDestroyed, Root::eTurnOff, Root::eStateDestroyed);
	m_RootFSM.DefineTransition(Root::eStateDestroyed, Root::eTurnOn, Root::eStateDestroyed);
	m_RootFSM.DefineTransition(Root::eStateDestroyed, Root::eDestroyed , Root::eStateDestroyed);
	

	// Further transitions are defined in ReadProp.

	m_RootFSM.SetInitialState( Root::eStateIdle );

	//
	// Set up the sweep(idle movement) FSM.
	//
	m_SweepFSM.DefineState( Sweep::eStateTurningToMin, m_SweepFSM.MakeCallback(*this,&Turret::DoTurningToMin) );

	m_SweepFSM.DefineState( Sweep::eStateTurningToMax, m_SweepFSM.MakeCallback(*this,&Turret::DoTurningToMax) );

	m_SweepFSM.DefineState( Sweep::eStatePausingAtMin, m_SweepFSM.MakeCallback(*this,&Turret::DoSweepPausing), 
											   	       m_SweepFSM.MakeCallback(*this,&Turret::StartSweepPausing) );

	m_SweepFSM.DefineState( Sweep::eStatePausingAtMax, m_SweepFSM.MakeCallback(*this,&Turret::DoSweepPausing),
												       m_SweepFSM.MakeCallback(*this,&Turret::StartSweepPausing) );


	m_SweepFSM.DefineTransition(Sweep::eStateTurningToMin,Sweep::eComplete,Sweep::eStatePausingAtMin);
	m_SweepFSM.DefineTransition(Sweep::eStatePausingAtMin,Sweep::eComplete,Sweep::eStateTurningToMax);
	m_SweepFSM.DefineTransition(Sweep::eStateTurningToMax,Sweep::eComplete,Sweep::eStatePausingAtMax);
	m_SweepFSM.DefineTransition(Sweep::eStatePausingAtMax,Sweep::eComplete,Sweep::eStateTurningToMin);

	m_SweepFSM.SetInitialState( Sweep::eStateTurningToMin );

#ifdef TURRET_DEBUG
	m_RootFSM.SetChangingStateCallback(m_RootFSM.MakeChangingStateCallback(*this,&Turret::PrintRootFSMState));
	m_RootFSM.SetProcessingMessageCallback(m_RootFSM.MakeProcessingMessageCallback(*this,&Turret::PrintRootFSMMessage));
#endif
}


// ----------------------------------------------------------------------- //
//
//	ROUTINES:	State callbacks used by RootFSM
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void Turret::StartIdleTimer(RootFSM * fsm)
{
	m_tmrIdle.Start( m_fMaxIdleTime );
}



void Turret::DoSweep(RootFSM * fsm)
{
	// Only sweep if we do not have 360 degrees of freedom.
	if( ClampAngle(m_fMaxYaw - m_fMinYaw) > 0.0f )
		m_SweepFSM.Update();
	else
		TurnTo(0.0f, 0.0f, m_fPitchSpeed, m_fSweepYawSpeed);
}

void Turret::DoDestroyed(RootFSM * fsm)
{
	if( m_hMovingSound ) g_pLTServer->KillSound(m_hMovingSound);
	m_hMovingSound = LTNULL;

	if( m_hWarningSound ) g_pLTServer->KillSound(m_hWarningSound);
	m_hWarningSound = LTNULL;

	if( m_hFiringSound ) g_pLTServer->KillSound(m_hFiringSound);
	m_hFiringSound = LTNULL;

	m_bMoving = LTFALSE;

	if( !m_damage.IsDead() )
	{
		DamageStruct damage;

		damage.fDamage	= damage.kInfiniteDamage;
		damage.hDamager = m_hObject;

		damage.DoDamage(this, m_hObject);
	}

	SetNextUpdate(0.0f);
}

void Turret::StartFire(RootFSM * fsm)
{
	m_bFiring = LTTRUE;
}

void Turret::StartTrack(RootFSM * fsm)
{
	if( m_bUseDetectSound && !m_strDetectSound.empty() )
	{
		if( !m_hWarningSound )
			m_hWarningSound = g_pServerSoundMgr->PlaySoundFromObject(m_hGunHandle, const_cast<char*>(m_strDetectSound.c_str()), 
															   m_fWarningSoundDistance,SOUNDPRIORITY_MISC_HIGH, PLAYSOUND_GETHANDLE);
	}
}

void Turret::DoTrack(RootFSM * fsm)
{
	if( !m_hObjective )
	{
		m_ObjectiveType = Ignore;
		m_vObjectiveOffset = LTVector(0,0,0);
		m_RootFSM.AddMessage(Root::eObjectiveGone);
		return;
	}

	if( !m_hGunHandle )
	{
		m_RootFSM.AddMessage(Root::eDestroyed);
	}

	LTVector vObjectivePos;
	g_pLTServer->GetObjectPos(m_hObjective, &vObjectivePos);
	vObjectivePos += m_vObjectiveOffset;

	LTVector vAimPos;
	if( m_hGunAttachment )
	{
		LTransform transform;
		g_pLTServer->Common()->GetAttachmentTransform(m_hGunAttachment,transform,LTTRUE);
		g_pTransLT->GetPos(transform, vAimPos);
	}
	else
	{
		g_pLTServer->GetObjectPos(m_hGunHandle, &vAimPos);
	}
	vAimPos += m_vRight*m_vAimingOffset.x;
	vAimPos += m_vUp*m_vAimingOffset.y;


#ifdef TURRET_DEBUG
	LineSystem::GetSystem(this,"AimingVector")
		<< LineSystem::Clear()
		<< LineSystem::Arrow( vAimPos, vObjectivePos )
		<< LineSystem::Arrow( vAimPos, vAimPos + m_vRight*20.0f, Color::Blue )
		<< LineSystem::Arrow( vAimPos, vAimPos + m_vUp*20.0f, Color::Blue );
#endif

	LTVector vDir = vObjectivePos - vAimPos;

	// follow Objective
	LTRotation rRot;
	LTVector vAngle;
	g_pMathLT->AlignRotation(rRot, vDir, m_vInitialUp);
	rRot = (~m_rInitialRotation)*rRot;
	g_pMathLT->GetEulerAngles(rRot,vAngle);
	vAngle.x = -vAngle.x;

	LTBOOL bCompletedTurn = TurnTo( vAngle.x, vAngle.y, m_fPitchSpeed, m_fYawSpeed);

	if( m_bFiring )
	{
		LTBOOL bFire = bCompletedTurn;

		if(    !bFire 
			&& (m_fPitch == m_fMaxPitch || m_fPitch == m_fMinPitch) 
			&& (m_fYaw > m_fMinYaw && m_fYaw < m_fMaxYaw) )
		{
			LTVector vObjectiveDims;
			g_pLTServer->GetObjectDims(m_hObjective, &vObjectiveDims);

			const LTFLOAT fObjectiveHeight = (LTFLOAT)fabs(vObjectiveDims.Dot(m_vUp));
			const LTFLOAT fObjectiveVertDisp = (LTFLOAT)fabs((vAimPos - vObjectivePos).Dot(m_vUp));
			if( fObjectiveHeight > fObjectiveVertDisp )
			{
				bFire = LTTRUE;
			}
		}

		if( bFire )
		{
			if( m_tmrBurstDelay.Stopped() )
			{
				if(    m_TurretVision.FocusIsVisible() 
					|| m_TurretVision.FocusCanBeShotFrom(vAimPos, GetVisualRange()*GetVisualRange()) ) 
				
				{
					FireWeapon();

					// Notify all the AI's.
					if(    g_pGameServerShell->GetGameType().IsSinglePlayer() 
						&& m_hObjective 
						&& m_pCharObjective )
					{

						CharFireInfo char_info;

						// Set the position so that the AI will look at what the turret is firing at.
						char_info.vFiredPos = (vObjectivePos + vAimPos)*0.5f;  
						char_info.vFiredDir = vDir;

						// Make sure we have a weapon (we better!).
						CWeapon * pWeapon = m_GunMgr.GetCurWeapon();
						_ASSERT( pWeapon );
						if( pWeapon )
						{
							// Fill in the weapon info.
							char_info.nWeaponId = pWeapon->GetId();
							char_info.nBarrelId = pWeapon->GetBarrelId();
							char_info.nAmmoId = pWeapon->GetAmmoId();
							char_info.fTime		= g_pLTServer->GetTime();

							// Finally,  we can really tell the other AI's!
							for( CCharacterMgr::AIIterator iter = g_pCharacterMgr->BeginAIs();
								 iter != g_pCharacterMgr->EndAIs(); ++iter )
							{
								(*iter)->GetSenseMgr().TurretWeaponFire(char_info,m_pCharObjective);
							}
						}
					}

				}
			}
			else 
			{
				// Go ahead and keep playing the fired weapon sound.
				m_bFiredWeapon = LTTRUE;
			}
		}
	}
}

void Turret::EndTrack(RootFSM * fsm)
{
	m_bFiring = LTFALSE;
}

void Turret::StartOpening(RootFSM * fsm)
{
	if( m_hOpeningAni != INVALID_MODEL_ANIM )
	{
		g_pLTServer->SetModelAnimation(m_hObject,m_hOpeningAni);
	}
}

void Turret::DoOpening(RootFSM * fsm)
{
	if( m_hOpeningAni == INVALID_MODEL_ANIM
		|| ( g_pLTServer->GetModelAnimation(m_hObject) == m_hOpeningAni && 
		     g_pLTServer->GetModelPlaybackState(m_hObject) == MS_PLAYDONE ) )
	{
		m_RootFSM.AddMessage(Root::eCompleted);
		
		if( m_hObjective )
		{
			if(	   m_ObjectiveType >= Target )
			{
				m_tmrIdle.Stop();
				m_RootFSM.AddMessage(Root::eNewTargetDetected);
			}
			else if( m_ObjectiveType == Interest )
			{
				m_tmrIdle.Stop();
				m_RootFSM.AddMessage(Root::eNewInterestDetected);
			}
		}

		if( m_hOpenedAni != INVALID_MODEL_ANIM )
		{
			g_pLTServer->SetModelAnimation(m_hObject,m_hOpenedAni);
		}
	}
}

void Turret::StartClosing(RootFSM * fsm)
{
	if( m_hClosingAni != INVALID_MODEL_ANIM )
	{
		g_pLTServer->SetModelAnimation(m_hObject,m_hClosingAni);
	}
}

void Turret::DoClosing(RootFSM * fsm)
{
	if( m_hClosingAni == INVALID_MODEL_ANIM
		|| ( g_pLTServer->GetModelAnimation(m_hObject) == m_hClosingAni && 
		     g_pLTServer->GetModelPlaybackState(m_hObject) == MS_PLAYDONE ) )
	{
		m_RootFSM.AddMessage(Root::eCompleted);
	}
}

void Turret::DoBeginClosing(RootFSM * fsm)
{
	if( TurnTo(0,0, m_fPitchSpeed, m_fYawSpeed) )
	{
		m_RootFSM.AddMessage(Root::eCompleted);
	}
}


void Turret::StartClosed(RootFSM * fsm)
{
	// make gun non-visible
	if( m_bHasGunModel && m_hClosingAni != INVALID_MODEL_ANIM )
	{
		g_pLTServer->SetObjectFlags(m_hGunHandle, g_pLTServer->GetObjectFlags(m_hGunHandle) & ~FLAG_VISIBLE );

		// Make a closed turret indestructible.
		m_bCanDamage = m_damage.GetCanDamage();
		m_damage.SetCanDamage(LTFALSE);
		
		TurretWeapon * pTurretWeapon = dynamic_cast<TurretWeapon*>( g_pLTServer->HandleToObject(m_hGunHandle) );
		_ASSERT( pTurretWeapon );
		if( pTurretWeapon )
		{
			pTurretWeapon->GetDestructible()->SetCanDamage(LTFALSE);
		}

	}

	if( m_hClosedAni != INVALID_MODEL_ANIM )
	{
		g_pLTServer->SetModelAnimation(m_hObject,m_hClosedAni);
	}


}

void Turret::EndClosed(RootFSM * fsm)
{
	// PLH TODO
	if( m_bHasGunModel && m_hClosingAni != INVALID_MODEL_ANIM )
	{
		g_pLTServer->SetObjectFlags(m_hGunHandle, g_pLTServer->GetObjectFlags(m_hGunHandle)  | FLAG_VISIBLE );
	
		m_damage.SetCanDamage(m_bCanDamage);

		TurretWeapon * pTurretWeapon = dynamic_cast<TurretWeapon*>( g_pLTServer->HandleToObject(m_hGunHandle) );
		_ASSERT( pTurretWeapon );
		if( pTurretWeapon )
		{
			pTurretWeapon->GetDestructible()->SetCanDamage(m_bCanDamage);
		}
	}

	if( m_bUseAwakeSound && m_strAwakeSound.empty() )
	{
		if( !m_hWarningSound )
			m_hWarningSound = g_pServerSoundMgr->PlaySoundFromObject(m_hGunHandle, const_cast<char*>(m_strAwakeSound.c_str()), 
															   m_fWarningSoundDistance, SOUNDPRIORITY_MISC_HIGH, PLAYSOUND_GETHANDLE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINES:	State callbacks used by SweepFSM
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void Turret::DoTurningToMin(SweepFSM * fsm)
{
	if( TurnTo(0.0f,m_fMinYaw, m_fPitchSpeed, m_fSweepYawSpeed) ) 
	{
		fsm->AddMessage(Sweep::eComplete);
	}
}

void Turret::DoTurningToMax(SweepFSM * fsm)
{
	if( TurnTo(0.0f,m_fMaxYaw, m_fPitchSpeed, m_fSweepYawSpeed) ) 
	{
		fsm->AddMessage(Sweep::eComplete);
	}
}

void Turret::StartSweepPausing(SweepFSM * fsm)
{
	m_tmrPause.Start(m_fSweepPauseTime);
}

void Turret::DoSweepPausing(SweepFSM * fsm)
{
	if( m_tmrPause.Stopped() )
	{
		fsm->AddMessage(Sweep::eComplete);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SeeCharacter
//
//	PURPOSE:	Used when a TurretVisionInstance reaches full stimulation.
//
// ----------------------------------------------------------------------- //
void Turret::SeeCharacter(const CTurretVision & sense, const CCharacter * potential_target)
{
	// Don't see a thing if turned off.

	if( m_hObjective && potential_target == LTNULL )
	{
		// We can no longer see a target, clear out our objective
		// unless it is an override objective.
		if( m_ObjectiveType != Override )
		{
			m_hObjective = LTNULL;
			m_pCharObjective = LTNULL;
			m_ObjectiveType = Ignore;
			m_vObjectiveOffset = LTVector(0,0,0);

			if( m_bOn )	m_RootFSM.AddMessage( Root::eObjectiveGone );

#ifdef TURRET_DEBUG
			AICPrint(m_hObject,"Lost objective.");
#endif
		}
	}
	else if( potential_target && potential_target->m_hObject )
	{
		// We have a new potential target!

		// We can only tareget characters.
		CCharacter * pChar = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(potential_target->m_hObject) );
		if( pChar )
		{
			// Decide what to do base on the target's type.
			switch( m_aClassTypes[pChar->GetCharacterClass()] )
			{
				case Ignore :
				{
					// do nothing.
#ifdef TURRET_DEBUG
					AICPrint(m_hObject,"Ignoring potential target, \"%s\".",
						g_pLTServer->GetObjectName(pChar->m_hObject) );
#endif
				}
				break;

				case Interest :
				{
					// Follow an interest if we do not have something more important.
					if( m_ObjectiveType <= Interest )
					{
						m_hObjective = potential_target->m_hObject;
						m_pCharObjective = potential_target;
						m_ObjectiveType = Interest;
						m_vObjectiveOffset = potential_target->GetChestOffset();

						m_tmrIdle.Stop(); // Be sure we don't go into idle!

						if( m_bOn ) m_RootFSM.AddMessage(Root::eNewInterestDetected);
#ifdef TURRET_DEBUG
						AICPrint(m_hObject,"Interested in potential target \"%s\".",
							g_pLTServer->GetObjectName(pChar->m_hObject) );
#endif
					}
				}
				break;

				case Target :
				{
					// Follow a target if we do not have something more important (like a Prefered or an Override).
					if( m_ObjectiveType <= Target )
					{
						m_hObjective = potential_target->m_hObject;
						m_pCharObjective = potential_target;
						m_ObjectiveType = Target;
						m_vObjectiveOffset = potential_target->GetChestOffset();

						m_tmrIdle.Stop(); // Be sure we don't go into idle!

						if( m_bOn ) m_RootFSM.AddMessage(Root::eNewTargetDetected);
#ifdef TURRET_DEBUG
						AICPrint(m_hObject,"Targeting potential target \"%s\".",
							g_pLTServer->GetObjectName(pChar->m_hObject) ); 
#endif
					}
				}
				break;

				case Prefer :
				{
					// Follow a target if we do not have something more important (like an Override).
					if( m_ObjectiveType <= Prefer )
					{
						m_hObjective = potential_target->m_hObject;
						m_pCharObjective = potential_target;
						m_ObjectiveType = Prefer;
						m_vObjectiveOffset = potential_target->GetChestOffset();

						m_tmrIdle.Stop(); // Be sure we don't go into idle!

						if( m_bOn ) m_RootFSM.AddMessage(Root::eNewTargetDetected);
#ifdef TURRET_DEBUG
						AICPrint(m_hObject,"Prefering potential target \"%s\".",
							g_pLTServer->GetObjectName(pChar->m_hObject) );
#endif
					}
				}
			} //switch( m_aClassTypes[pChar->GetCharacterClass()] )

#ifdef TURRET_DEBUG
		AICPrint(m_hObject, "%s : SeeTarget found %s.",
			g_pLTServer->GetObjectName(m_hObject),
			g_pLTServer->GetObjectName(potential_target->m_hObject) );
#endif

		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINES:	TurnTo
//
//	PURPOSE:	Turns turret to desired rotation (in turret's coordinates).
//				Manages turning rate and not going over max/min angles.
//
// ----------------------------------------------------------------------- //
LTBOOL Turret::TurnTo(LTFLOAT fDesiredPitch, LTFLOAT fDesiredYaw, LTFLOAT fPitchSpeed, LTFLOAT fYawSpeed, LTBOOL bForced /* = LTFALSE */)
{
	// If we're already there, we can just leave now.
	if( fDesiredYaw == m_fYaw && fDesiredPitch == m_fPitch && !bForced) 
		return LTTRUE;

	if( m_bAttachGun && m_hGunAttachment )
	{
		g_pLTServer->RemoveAttachment( m_hGunAttachment );
		m_hGunAttachment = LTNULL;
	}

	// Set up the desired angle changes
	LTFLOAT fDesiredYawDiff   = ClampAngle(fDesiredYaw - m_fYaw);
	LTFLOAT fDesiredPitchDiff = ClampAngle(fDesiredPitch - m_fPitch);

	// Set angle change to maximum possible.
	LTFLOAT fYaw = GetUpdateDelta()*fYawSpeed;
	LTFLOAT fPitch = GetUpdateDelta()*fPitchSpeed;
	
	// Adjust yaw change to smaller value, if that is all that is needed.
	// Otherwise get it going in the right direction.
	LTBOOL bReachedYawGoal = LTFALSE;
	if(    fDesiredYawDiff <= fYaw 
		&& fDesiredYawDiff >= -fYaw ) 
	{
		fYaw = fDesiredYawDiff;
		bReachedYawGoal = LTTRUE;
	}
	else if( fDesiredYawDiff < 0.0f ) 
	{
		fYaw = -fYaw;
	}

	// Adjust Pitch change to smaller value, if that is all that is needed.
	// Otherwise get it going in the right direction.
	LTBOOL bReachedPitchGoal = LTFALSE;
	if(    fDesiredPitchDiff <= fPitch 
		&& fDesiredPitchDiff >= -fPitch )
	{
		fPitch = fDesiredPitchDiff;
		bReachedPitchGoal = LTTRUE;
	}
	else if( fDesiredPitchDiff < 0.0f ) 
	{
		fPitch = -fPitch;
	}

	// Record the change.
	m_fYaw   = ClampAngle(m_fYaw + fYaw);
	m_fPitch = ClampAngle(m_fPitch + fPitch);

	if( m_fYaw > m_fMaxYaw )
	{
		bReachedYawGoal = LTFALSE;

		m_fYaw = m_fMaxYaw;
	}
	if( m_fYaw < m_fMinYaw )
	{
		bReachedYawGoal = LTFALSE;

		m_fYaw = m_fMinYaw;
	}

	if( m_fPitch > m_fMaxPitch )
	{
		bReachedPitchGoal = LTFALSE;

		m_fPitch = m_fMaxPitch;
	}
	if( m_fPitch < m_fMinPitch )
	{
		bReachedPitchGoal = LTFALSE;
		m_fPitch = m_fMinPitch;
	}

	// Make sure the movement sound is playing
	m_bMoving = (!bReachedYawGoal || !bReachedPitchGoal);

	//
	// Do the rotation.
	//

	// Set up the desired rotation.
	LTRotation rGunRot(0.0f,0.0f,0.0f,1.0f);
	g_pMathLT->EulerRotateY(rGunRot, m_fYaw );
	g_pMathLT->EulerRotateX(rGunRot, -m_fPitch );
	
	// And rotate to that position.
	if( m_bAttachGun && m_hGunHandle != m_hObject )
	{
		PositionAndRotation offsets = GetPositionAndRotateOffset( m_hGunHandle, LTVector(0,0,0), rGunRot, m_vGunOffset, LTRotation(0,0,0,1) );

		g_pServerDE->CreateAttachment( m_hObject, m_hGunHandle, const_cast<char*>(m_strTurretBaseSocket.c_str()), 
									   &offsets.vPos, &offsets.rRot, &m_hGunAttachment);
	}
	else
	{
		PositionAndRotation offsets = GetPositionAndRotateOffset( m_hGunHandle, m_vInitialPosition, rGunRot, m_vGunOffset, m_rInitialRotation );
		g_pLTServer->TeleportObject(m_hGunHandle,&offsets.vPos );
		g_pLTServer->RotateObject(m_hGunHandle, &offsets.rRot);
	}

	// That's all.
	return bReachedYawGoal && bReachedPitchGoal;

} //LTBOOL Turret::TurnTo(LTFLOAT fDesiredPitch, LTFLOAT fDesiredYaw)




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FireWeapon
//
//	PURPOSE:	Fires a round from the weapon.
//
// ----------------------------------------------------------------------- //
void Turret::FireWeapon()
{

	LTVector vR,vU,vForward;
	LTRotation rRot;
	if( m_hGunAttachment )
	{
		LTransform transform;
		g_pLTServer->Common()->GetAttachmentTransform(m_hGunAttachment,transform, LTTRUE);
		g_pTransLT->GetRot(transform,rRot);
	}
	else
	{
		g_pLTServer->GetObjectRotation(m_hGunHandle,&rRot);
	}
	g_pMathLT->GetRotationVectors(rRot,vR,vU,vForward);
	
	CWeapon * pWeapon = m_GunMgr.GetCurWeapon();
	if( pWeapon )
	{
		LTVector vFlashPos;
		if( LT_OK == GetSocketPos(m_hGunHandle, m_hGunAttachment, pWeapon->GetFlashSocketName(),&vFlashPos) )
		{
			if( !m_TurretVision.FocusCanBeShotFrom(vFlashPos, LTFLOAT(pWeapon->GetBarrelData()->nRangeSqr) ) )
			{
				return;
			}
			
			LTVector vObjectivePos;
			g_pLTServer->GetObjectPos(m_hObjective, &vObjectivePos);
			vObjectivePos += m_vObjectiveOffset;

			WFireInfo fireInfo;
			fireInfo.hFiredFrom = m_hGunHandle;
			fireInfo.vPath		= vObjectivePos - vFlashPos;
			fireInfo.vFirePos	= vFlashPos;
			fireInfo.vFlashPos	= vFlashPos;
			fireInfo.nFlashSocket = pWeapon->GetCurrentFlashSocket();
			fireInfo.nSeed		= GetRandom(2,255);
			fireInfo.bAltFire	= FALSE;
			fireInfo.fPerturbR	= 0.0f;
			fireInfo.fPerturbU	= 0.0f;

			fireInfo.fDamageMultiplier = m_fDamageMultiplier*GetDifficultyFactor();

			pWeapon->Fire(fireInfo);

#ifdef TURRET_DEBUG
			LineSystem::GetSystem(this, "ShowFiring")
				<< LineSystem::Clear()
				<< LineSystem::Arrow(vFlashPos, vFlashPos + vForward*LTFLOAT(pWeapon->GetBarrelData()->nRange), Color::Red);
#endif
		}
	}

	m_bFiredWeapon = LTTRUE;
	m_tmrBurstDelay.Start( m_fBurstDelay );
}







// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Turret::Save(HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	// m_TurretVision saves itself.
	// m_GunMgr saves itself.

	// m_RootFSM set-up stuff.
	hWrite->WriteDWord( m_RootFSM.GetDefault(Root::eIdleTimedOut) );
	hWrite->WriteDWord( m_RootFSM.GetAutomatic(Root::eStateIdle) );
	hWrite->WriteDWord( m_RootFSM.GetDefault(Root::eNewInterestDetected) );
	hWrite->WriteDWord( m_RootFSM.GetDefault(Root::eNewTargetDetected) );

	*hWrite << m_RootFSM;	
	*hWrite << m_SweepFSM;

	*hWrite << m_strGunFilename;
	*hWrite << m_strGunSkin;
	*hWrite << m_strDeadGunFilename;
	*hWrite << m_strDeadGunSkin;

	*hWrite << m_fGunMaxHitPoints;

	*hWrite << m_strGunBaseSocket;
	*hWrite << m_strTurretBaseSocket;

	*hWrite << m_strAwakeSound;
	*hWrite << m_strDetectSound;
	*hWrite << m_strMoveSound;
	*hWrite << m_fWarningSoundDistance;
	*hWrite << m_fMovingSoundDistance;

	*hWrite << m_nWeaponId;
	*hWrite << m_fBurstDelay;
	*hWrite << m_fDamageMultiplier;

	*hWrite << m_nStartFireSound;
	*hWrite << m_nLoopFireSound;
	*hWrite << m_nStopFireSound;

	*hWrite << m_fYawSpeed;
	*hWrite << m_fPitchSpeed;
	*hWrite << m_fSweepYawSpeed;

	*hWrite << m_fVisualRange;

	*hWrite << m_bStartOff;

	*hWrite << m_bHasGunModel;
	*hWrite << m_bAttachGun;
	*hWrite << m_hGunHandle;
	*hWrite << m_vGunOffset;
	// m_hGunAttachment handled in load only.
	*hWrite << m_vAimingOffset;

	*hWrite << m_bUseAwakeSound;
	*hWrite << m_bUseDetectSound;

	*hWrite << m_fMinYaw;
	*hWrite << m_fMaxYaw;

	*hWrite << m_fMinPitch;
	*hWrite << m_fMaxPitch;
	
	*hWrite << m_fSweepPauseTime;

	*hWrite << m_vInitialPosition;
	*hWrite << m_rInitialRotation;
	*hWrite << m_vInitialAngles;
	*hWrite << m_vInitialUp;
	*hWrite << m_vInitialRight;

	*hWrite << m_fMaxIdleTime;

	for( int i = 0; i < 5; ++i )
	{
		*hWrite << m_aClassTypes[i];
	}

	*hWrite << m_OpeningAnimName;
	*hWrite << m_ClosingAnimName;
	*hWrite << m_OpenedAnimName;
	*hWrite << m_ClosedAnimName;

	// m_hOpeningAni set at load.
	// m_hClosingAni set at load.
	// m_hOpenedAni set at load.
	// m_hClosedAni set at load.

	*hWrite << m_hObjective;
	// m_pCharObjective will be reset by Load
	*hWrite << m_vObjectiveOffset;
	*hWrite << m_ObjectiveType;
	
	// m_hWarningSound isn't saved.
	// m_hMovingSound isn't saved.
	// m_hFiringSound isn't saved.

	*hWrite << m_bFirstUpdate;
	*hWrite << m_bOn;
	
	*hWrite << m_fYaw;
	*hWrite << m_fPitch;

	*hWrite << m_vRight;
	*hWrite << m_vUp;
	*hWrite << m_vForward;

	*hWrite << m_bFiring;
	*hWrite << m_bFiredWeapon;

	*hWrite << m_bMoving;

	*hWrite << m_tmrIdle;
	*hWrite << m_tmrPause;
	*hWrite << m_tmrBurstDelay;

	*hWrite << m_bCanDamage;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Turret::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	// m_TurretVision saves itself.
	// m_GunMgr loads itself.

	// m_RootFSM setup stuff.
	m_RootFSM.DefineTransition(Root::eIdleTimedOut, Root::State(hRead->ReadDWord()) );
	m_RootFSM.DefineTransition(Root::eStateIdle, Root::State(hRead->ReadDWord()) );
	m_RootFSM.DefineTransition(Root::eNewInterestDetected, Root::State(hRead->ReadDWord()) );
	m_RootFSM.DefineTransition(Root::eNewTargetDetected, Root::State(hRead->ReadDWord()) );

	*hRead >> m_RootFSM;	
	*hRead >> m_SweepFSM;

	*hRead >> m_strGunFilename;
	*hRead >> m_strGunSkin;
	*hRead >> m_strDeadGunFilename;
	*hRead >> m_strDeadGunSkin;

	*hRead >> m_fGunMaxHitPoints;

	*hRead >> m_strGunBaseSocket;
	*hRead >> m_strTurretBaseSocket;

	*hRead >> m_strAwakeSound;
	*hRead >> m_strDetectSound;
	*hRead >> m_strMoveSound;
	*hRead >> m_fWarningSoundDistance;
	*hRead >> m_fMovingSoundDistance;

	*hRead >> m_nWeaponId;
	*hRead >> m_fBurstDelay;
	*hRead >> m_fDamageMultiplier;

	*hRead >> m_nStartFireSound;
	*hRead >> m_nLoopFireSound;
	*hRead >> m_nStopFireSound;

	*hRead >> m_fYawSpeed;
	*hRead >> m_fPitchSpeed;
	*hRead >> m_fSweepYawSpeed;

	*hRead >> m_fVisualRange;

	*hRead >> m_bStartOff;

	*hRead >> m_bHasGunModel;
	*hRead >> m_bAttachGun;
	*hRead >> m_hGunHandle;
	*hRead >> m_vGunOffset;
	if( m_bAttachGun && m_hGunHandle != m_hObject)
	{
		g_pLTServer->FindAttachment(m_hObject, m_hGunHandle, &m_hGunAttachment);
	}
	*hRead >> m_vAimingOffset;

	*hRead >> m_bUseAwakeSound;
	*hRead >> m_bUseDetectSound;

	*hRead >> m_fMinYaw;
	*hRead >> m_fMaxYaw;

	*hRead >> m_fMinPitch;
	*hRead >> m_fMaxPitch;
	
	*hRead >> m_fSweepPauseTime;

	*hRead >> m_vInitialPosition;
	*hRead >> m_rInitialRotation;
	*hRead >> m_vInitialAngles;
	*hRead >> m_vInitialUp;
	*hRead >> m_vInitialRight;

	*hRead >> m_fMaxIdleTime;

	for( int i = 0; i < 5; ++i )
	{
		*hRead >> m_aClassTypes[i];
	}

	*hRead >> m_OpeningAnimName;
	*hRead >> m_ClosingAnimName;
	*hRead >> m_OpenedAnimName;
	*hRead >> m_ClosedAnimName;

	if( !m_OpeningAnimName.empty() )
		m_hOpeningAni = g_pLTServer->GetAnimIndex(m_hObject, const_cast<char*>(m_OpeningAnimName.c_str()) );

	if( !m_OpenedAnimName.empty() )
		m_hOpenedAni = g_pLTServer->GetAnimIndex(m_hObject, const_cast<char*>(m_OpenedAnimName.c_str()) );

	if( !m_ClosingAnimName.empty() )
		m_hClosingAni = g_pLTServer->GetAnimIndex(m_hObject, const_cast<char*>(m_ClosingAnimName.c_str()) );

	if( !m_ClosedAnimName.empty() )
		m_hClosedAni = g_pLTServer->GetAnimIndex(m_hObject, const_cast<char*>(m_ClosedAnimName.c_str()) );

	*hRead >> m_hObjective;
	m_pCharObjective = dynamic_cast<const CCharacter *>( g_pLTServer->HandleToObject( m_hObjective ) );
	*hRead >> m_vObjectiveOffset;
	*hRead >> m_ObjectiveType;

	// Warning and moving sounds aren't saved,
	//   just be sure any old ones are turned off.
	if( m_hWarningSound ) 
		g_pLTServer->KillSound(m_hWarningSound);
	m_hWarningSound = LTNULL;

	if( m_hMovingSound ) 
		g_pLTServer->KillSound(m_hMovingSound);
	m_hMovingSound = LTNULL;
	
	if( m_hFiringSound ) 
		g_pLTServer->KillSound(m_hFiringSound);
	m_hFiringSound = LTNULL;

	*hRead >> m_bFirstUpdate;
	*hRead >> m_bOn;
	
	*hRead >> m_fYaw;
	*hRead >> m_fPitch;

	*hRead >> m_vRight;
	*hRead >> m_vUp;
	*hRead >> m_vForward;

	*hRead >> m_bFiring;
	*hRead >> m_bFiredWeapon;

	*hRead >> m_bMoving;

	*hRead >> m_tmrIdle;
	*hRead >> m_tmrPause;
	*hRead >> m_tmrBurstDelay;

	*hRead >> m_bCanDamage;
}


/////////////////////////////////////////////////////////////////////////////
//  Class TurretWeapon
/////////////////////////////////////////////////////////////////////////////

BEGIN_CLASS(TurretWeapon)
END_CLASS_DEFAULT_FLAGS(TurretWeapon, Prop, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TurretWeapon::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 TurretWeapon::ObjectMessageFn( HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead )
{
	if (!g_pLTServer) return 0;

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			// Let our damage aggregate process the message first...

			uint32 dwRet = Prop::ObjectMessageFn(hSender, messageID, hRead);

			// Check to see if we have been destroyed

			if (m_pOwner && m_damage.IsDead())
			{
				m_pOwner->Destroyed();
			}
			return dwRet;
		}
		break;

		default : break;
	}

	return Prop::ObjectMessageFn(hSender, messageID, hRead);
}


uint32 TurretWeapon::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct * const pStruct = reinterpret_cast<ObjectCreateStruct *>(pData);

			const uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			// Need to undo prop's forcing of 
			// FLAG_VISIBLE if fData == PRECREATE_STRINGPROP.
			if( fData == PRECREATE_STRINGPROP )
			{
				if( !(pStruct->m_Flags & FLAG_VISIBLE ) )
					m_dwFlags = m_dwFlags & ~FLAG_VISIBLE;
			}

			return dwRet;
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
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TurretWeapon::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void TurretWeapon::Save(HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	if( m_pOwner )
		hWrite->WriteObject( m_pOwner->m_hObject );
	else
		hWrite->WriteObject( LTNULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TurretWeapon::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void TurretWeapon::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	HOBJECT hOwner = hRead->ReadObject();

	if( hOwner )
		m_pOwner = dynamic_cast<Turret*>( g_pLTServer->HandleToObject(hOwner) );
	else
		m_pOwner = LTNULL;
}



#ifdef TURRET_DEBUG
void Turret::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{
	const char * szMessage = 0;
	switch( message )
	{
		case Root::eCompleted : szMessage = "eCompleted"; break;
		case Root::eIdleTimedOut : szMessage = "eIdleTimedOut"; break;
		case Root::eNewInterestDetected : szMessage = "eNewInterestDetected"; break;
		case Root::eNewTargetDetected : szMessage = "eNewTargetDetected"; break;
		case Root::eObjectiveGone : szMessage = "eObjectiveGone"; break;
		case Root::eTurnOff : szMessage = "eTurnOff"; break;
		case Root::eTurnOn : szMessage = "eTurnOn"; break;
		case Root::eSleep : szMessage = "eSleep"; break;
		case Root::eDestroyed : szMessage = "eDestroyed"; break;
		default : szMessage = "Unknown"; break;
	}

	g_pLTServer->CPrint("%f %s : RootFSM is processing %s.", 
		g_pLTServer->GetTime(),
		g_pLTServer->GetObjectName(m_hObject),
		szMessage );
}

void Turret::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
	const char * szMessage = 0;
	switch( state )
	{
		case Root::eStateIdle  : szMessage = "eStateIdle"; break;
		case Root::eStateSweep : szMessage = "eStateSweep"; break;
		case Root::eStateTrack : szMessage = "eStateTrack"; break;
		case Root::eStateFire : szMessage = "eStateFire"; break;
		case Root::eStateBeginClosing : szMessage = "eStateBeginClosing"; break;
		case Root::eStateClosing : szMessage = "eStateClosing"; break;
		case Root::eStateClosed : szMessage = "eStateClosed"; break;
		case Root::eStateOpening : szMessage = "eStateOpening"; break;
		case Root::eStateDestroyed : szMessage = "eStateDestroyed"; break;
		default : szMessage = "Unknown"; break;
	}

	g_pLTServer->CPrint("%f %s : RootFSM is entering state %s.", 
		g_pLTServer->GetTime(),
		g_pLTServer->GetObjectName(m_hObject),
		szMessage );
}
#endif

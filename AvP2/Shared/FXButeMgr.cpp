// ----------------------------------------------------------------------- //
//
// MODULE  : FXButeMgr.cpp
//
// PURPOSE : FXButeMgr implementation - Controls attributes of special fx
//
// CREATED : 12/08/98
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "FXButeMgr.h"
#include "WeaponFXTypes.h"
#include "CommonUtilities.h"
#include "SurfaceFunctions.h"
#include "SoundButeMgr.h"

#ifdef _CLIENTBUILD
#include "..\ClientShellDLL\ParticleShowerFX.h"
#include "..\ClientShellDLL\PolyDebrisFX.h"
#include "..\ClientShellDLL\ParticleExplosionFX.h"
#include "..\ClientShellDLL\GameClientShell.h"
#include "..\ClientShellDLL\InterfaceMgr.h"
#include "..\ClientShellDLL\BeamFX.h"

extern CGameClientShell* g_pGameClientShell;
#endif

#define FXBMGR_IMPACTFX_TAG				"ImpactFX"
#define FXBMGR_IMPACTFX_NAME			"Name"
#define FXBMGR_IMPACTFX_3RDPERSONONLY	"3rdPersonOnly"
#define FXBMGR_IMPACTFX_SOUND			"Sound"
#define FXBMGR_IMPACTFX_SOUND_DIR		"ImpactSoundDir"
#define FXBMGR_IMPACTFX_SOUNDRADIUS		"SoundRadius"
#define FXBMGR_IMPACTFX_SOUNDBUTE		"SoundBute"
#define FXBMGR_IMPACTFX_AISOUNDRADIUS	"AISoundRadius"
#define FXBMGR_IMPACTFX_CREATEMARK		"CreateMark"
#define FXBMGR_IMPACTFX_CREATESMOKE		"CreateSmoke"
#define FXBMGR_IMPACTFX_IMPACTONSKY		"ImpactOnSky"
#define FXBMGR_IMPACTFX_IMPACTONFLESH	"ImpactOnFlesh"
#define FXBMGR_IMPACTFX_DOSURFACEFX		"DoSurfaceFX"
#define FXBMGR_IMPACTFX_SCREENTINT		"ScreenTint"
#define FXBMGR_IMPACTFX_PUSHERNAME		"PusherName"
#define FXBMGR_IMPACTFX_MARK			"Mark"
#define FXBMGR_IMPACTFX_MARKSCALE		"MarkScale"
#define FXBMGR_IMPACTFX_TINTCOLOR		"TintColor"
#define FXBMGR_IMPACTFX_TINTRAMPUP		"TintRampUp"
#define FXBMGR_IMPACTFX_TINTRAMPDOWN	"TintRampDown"
#define FXBMGR_IMPACTFX_TINTMAXTIME		"TintMaxTime"
#define FXBMGR_IMPACTFX_TINTRADIUS		"TintRadius"
#define FXBMGR_IMPACTFX_DEBRISNAME		"DebrisName"
#define FXBMGR_IMPACTFX_SCALENAME		"ScaleName"
#define FXBMGR_IMPACTFX_PEXPLNAME		"PExplName"
#define FXBMGR_IMPACTFX_DLIGHTNAME		"DLightName"
#define FXBMGR_IMPACTFX_PDEBRISNAME		"PolyDebrisName"
#define FXBMGR_IMPACTFX_PSHOWERNAME		"PShowerName"

#define FXBMGR_FIREFX_TAG				"FireFX"
#define FXBMGR_FIREFX_NAME				"Name"
#define FXBMGR_FIREFX_MUZZLESMOKE		"MuzzleSmoke"
#define FXBMGR_FIREFX_EJECTSHELLS		"EjectShells"
#define FXBMGR_FIREFX_MUZZLELIGHT		"MuzzleLight"
#define FXBMGR_FIREFX_FIRESOUND			"FireSound"
#define FXBMGR_FIREFX_EXITMARK			"ExitMark"
#define FXBMGR_FIREFX_EXITDEBRIS		"ExitDebris"
#define FXBMGR_FIREFX_SHELLMODEL		"ShellModel"
#define FXBMGR_FIREFX_SHELLSKIN			"ShellSkin"
#define FXBMGR_FIREFX_SHELLSCALE		"ShellScale"
#define FXBMGR_FIREFX_BEAMFXNAME		"BeamName"

#define FXBMGR_PARTICLETRAILFX_TAG			"ParticleTrailFX"
#define FXBMGR_PARTICLETRAILFX_NAME			"Name"
#define FXBMGR_PARTICLETRAILFX_TEXTURE		"Texture"
#define FXBMGR_PARTICLETRAILFX_MINCOLOR		"MinColor"
#define FXBMGR_PARTICLETRAILFX_MAXCOLOR		"MaxColor"
#define FXBMGR_PARTICLETRAILFX_MINDRIFT		"MinDrift"
#define FXBMGR_PARTICLETRAILFX_MAXDRIFT		"MaxDrift"
#define FXBMGR_PARTICLETRAILFX_LIFETIME		"Lifetime"
#define FXBMGR_PARTICLETRAILFX_FADETIME		"Fadetime"
#define FXBMGR_PARTICLETRAILFX_MINEMITRANGE	"MinEmitRange"
#define FXBMGR_PARTICLETRAILFX_MAXEMITRANGE	"MaxEmitRange"
#define FXBMGR_PARTICLETRAILFX_EMITDISTANCE	"EmitDistance"
#define FXBMGR_PARTICLETRAILFX_EMITAMOUNT	"EmitAmount"
#define FXBMGR_PARTICLETRAILFX_STARTSCALE	"StartScale"
#define FXBMGR_PARTICLETRAILFX_ENDSCALE		"EndScale"
#define FXBMGR_PARTICLETRAILFX_STARTALPHA	"StartAlpha"
#define FXBMGR_PARTICLETRAILFX_ENDALPHA		"EndAlpha"
#define FXBMGR_PARTICLETRAILFX_RADIUS		"Radius"
#define FXBMGR_PARTICLETRAILFX_GRAVITY		"Gravity"
#define FXBMGR_PARTICLETRAILFX_ADDITIVE		"Additive"
#define FXBMGR_PARTICLETRAILFX_MULTIPLY		"Multiply"
#define FXBMGR_PARTICLETRAILFX_IGNORWIND	"IgnorWind"

#define FXBMGR_PROJECTILEFX_TAG				"ProjectileFX"
#define FXBMGR_PROJECTILEFX_NAME			"Name"
#define FXBMGR_PROJECTILEFX_PARTICLETRAIL	"ParticleTrail"
#define FXBMGR_PROJECTILEFX_UWPARTICLETRAIL	"UWParticleTrail"
#define FXBMGR_PROJECTILEFX_FLARE			"Flare"
#define FXBMGR_PROJECTILEFX_LIGHT			"Light"
#define FXBMGR_PROJECTILEFX_FLYSOUND		"FlySound"
#define FXBMGR_PROJECTILEFX_CLASS			"Class"
#define FXBMGR_PROJECTILEFX_CLASSDATA		"ClassData"
#define FXBMGR_PROJECTILEFX_MODEL			"Model"
#define FXBMGR_PROJECTILEFX_MODELSCALE		"ModelScale"
#define FXBMGR_PROJECTILEFX_SKIN			"Skin"
#define FXBMGR_PROJECTILEFX_SCALEFX			"ToggleScaleFX"
#define FXBMGR_PROJECTILEFX_SOCKET			"Socket"
#define FXBMGR_PROJECTILEFX_SOUND			"Sound"
#define FXBMGR_PROJECTILEFX_SOUNDRADIUS		"SoundRadius"
#define FXBMGR_PROJECTILEFX_FLARESPRITE		"FlareSprite"
#define FXBMGR_PROJECTILEFX_FLARESCALE		"FlareScale"
#define FXBMGR_PROJECTILEFX_LIGHTCOLOR		"LightColor"
#define FXBMGR_PROJECTILEFX_LIGHTRADIUS		"LightRadius"
#define FXBMGR_PROJECTILEFX_GRAVITY			"Gravity"
#define FXBMGR_PROJECTILEFX_LIFETIME		"Lifetime"
#define FXBMGR_PROJECTILEFX_VELOCITY		"Velocity"
#define FXBMGR_PROJECTILEFX_ALTVELOCITY		"AltVelocity"
#define FXBMGR_PROJECTILEFX_NOLIGHTING		"NoLighting"
#define FXBMGR_PROJECTILEFX_ALPHA			"ModelAlpha"

#define FXBMGR_PROXCLASS_TAG			"ProxClassData"
#define FXBMGR_PROXCLASS_ACTRADIUS		"ActivateRadius"
#define FXBMGR_PROXCLASS_ARMDELAY		"ArmDelay"
#define FXBMGR_PROXCLASS_ARMSND			"ArmSound"
#define FXBMGR_PROXCLASS_ARMSNDRADIUS	"ArmSndRadius"
#define FXBMGR_PROXCLASS_ACTDELAY		"ActivateDelay"
#define FXBMGR_PROXCLASS_ACTSND			"ActivateSound"
#define FXBMGR_PROXCLASS_ACTSNDRADIUS	"ActivateSndRadius"

#define FXBMGR_SPIDERCLASS_TAG			"SpiderClassData"
#define FXBMGR_SPIDERCLASS_ACTRADIUS	"ActivateRadius"
#define FXBMGR_SPIDERCLASS_DETRADIUS	"DetonateRadius"
#define FXBMGR_SPIDERCLASS_ARMDELAY		"ArmDelay"
#define FXBMGR_SPIDERCLASS_ARMSND		"ArmSound"
#define FXBMGR_SPIDERCLASS_ARMSNDRADIUS	"ArmSndRadius"
#define FXBMGR_SPIDERCLASS_ACTSND		"ActivateSound"
#define FXBMGR_SPIDERCLASS_ACTSNDRADIUS	"ActivateSndRadius"

#define FXBMGR_TRACKINGSADARCLASS_TAG		"TrackingSADARClassData"
#define FXBMGR_TRACKINGSADARCLASS_RADIUS	"TurnRadius"

#define FXBMGR_PREDDISKCLASS_TAG		"PredatorDiskClassData"
#define FXBMGR_PREDDISKCLASS_RET_COST	"NormalRetrieveEnergyCost"
#define FXBMGR_PREDDISKCLASS_TELE_COST	"TeleportRetrieveEnergyCost"

#define FXBMGR_SCALEFX_TAG				"ScaleFX"
#define FXBMGR_PSHOWERFX_TAG			"PShowerFX"
#define FXBMGR_POLYDEBRISFX_TAG			"PolyDebrisFX"

#define FXBMGR_PEXPLFX_TAG				"PExplFX"
#define	FXBMGR_PEXPLFX_NAME				"Name"
#define	FXBMGR_PEXPLFX_FILE				"File"
#define	FXBMGR_PEXPLFX_POSOFFSET		"PosOffset"
#define	FXBMGR_PEXPLFX_NUMPERPUFF		"NumPerPuff"
#define	FXBMGR_PEXPLFX_NUMEMITTERS		"NumEmitters"
#define	FXBMGR_PEXPLFX_NUMSTEPS			"NumSteps"
#define	FXBMGR_PEXPLFX_CREATEDEBRIS		"CreateDebris"
#define	FXBMGR_PEXPLFX_ROTATEDEBRIS		"RotateDebris"
#define	FXBMGR_PEXPLFX_IGNOREWIND		"IgnoreWind"
#define	FXBMGR_PEXPLFX_DOBUBBLES		"DoBubbles"
#define	FXBMGR_PEXPLFX_COLOR1			"Color1"
#define	FXBMGR_PEXPLFX_COLOR2			"Color2"
#define	FXBMGR_PEXPLFX_MINVEL			"MinVel"
#define	FXBMGR_PEXPLFX_MAXVEL			"MaxVel"
#define	FXBMGR_PEXPLFX_MINDRIFTVEL		"MinDriftVel"
#define	FXBMGR_PEXPLFX_MAXDRIFTVEL		"MaxDriftVel"
#define	FXBMGR_PEXPLFX_LIFETIME			"LifeTime"
#define	FXBMGR_PEXPLFX_FADETIME			"FadeTime"
#define	FXBMGR_PEXPLFX_OFFSETTIME		"OffsetTime"
#define	FXBMGR_PEXPLFX_RADIUS			"Radius"
#define	FXBMGR_PEXPLFX_GRAVITY			"Gravity"
#define	FXBMGR_PEXPLFX_ADDITIVE			"Additive"
#define	FXBMGR_PEXPLFX_MULTIPLY			"Multiply"

#define FXBMGR_DLIGHTFX_TAG				"DLightFX"
#define	FXBMGR_DLIGHTFX_NAME			"Name"
#define	FXBMGR_DLIGHTFX_COLOR			"Color"
#define	FXBMGR_DLIGHTFX_MINRADIUS		"MinRadius"
#define	FXBMGR_DLIGHTFX_MAXRADIUS		"MaxRadius"
#define	FXBMGR_DLIGHTFX_MINTIME			"MinTime"
#define	FXBMGR_DLIGHTFX_MAXTIME			"MaxTime"
#define	FXBMGR_DLIGHTFX_RAMPUPTIME		"RampUpTime"
#define	FXBMGR_DLIGHTFX_RAMPDOWNTIME	"RampDownTime"
#define	FXBMGR_DLIGHTFX_WASHOUT			"ScreenWashout"

#define FXBMGR_SOUNDFX_TAG				"SoundFX"
#define	FXBMGR_SOUNDFX_NAME				"Name"
#define	FXBMGR_SOUNDFX_FILE				"File"
#define	FXBMGR_SOUNDFX_LOOP				"Loop"
#define	FXBMGR_SOUNDFX_RADIUS			"Radius"
#define	FXBMGR_SOUNDFX_PITCHSHIFT		"PitchShift"

#define FXBMGR_PUSHERFX_TAG				"PusherFX"
#define	FXBMGR_PUSHERFX_NAME			"Name"
#define	FXBMGR_PUSHERFX_RADIUS			"Radius"
#define	FXBMGR_PUSHERFX_STARTDELAY		"StartDelay"
#define	FXBMGR_PUSHERFX_DURATION		"Duration"
#define	FXBMGR_PUSHERFX_STRENGTH		"Strength"

#define FXBMGR_PVFX_TAG					"PVFX"
#define FXBMGR_PVFX_NAME				"Name"
#define FXBMGR_PVFX_SOCKET				"Socket"
#define FXBMGR_PVFX_SCALENAME			"ScaleName"
#define FXBMGR_PVFX_DLIGHTNAME			"DLightName"
#define FXBMGR_PVFX_SOUNDNAME			"SoundName"

#define FXBMGR_PARTMUZZLEFX_TAG			"ParticleMuzzleFX"
#define FXBMGR_PARTMUZZLEFX_NAME		"Name"
#define FXBMGR_PARTMUZZLEFX_FILE		"File"
#define FXBMGR_PARTMUZZLEFX_STARTSCALE	"StartScale"
#define FXBMGR_PARTMUZZLEFX_ENDSCALE	"EndScale"
#define FXBMGR_PARTMUZZLEFX_RADIUS		"BaseRadius"
#define FXBMGR_PARTMUZZLEFX_LENGTH		"Length"
#define FXBMGR_PARTMUZZLEFX_DURATION	"Duration"
#define FXBMGR_PARTMUZZLEFX_NUMBER		"NumParticles"
#define FXBMGR_PARTMUZZLEFX_STARTCOLOR	"StartColor"
#define FXBMGR_PARTMUZZLEFX_ENDCOLOR	"EndColor"
#define FXBMGR_PARTMUZZLEFX_STARTALPHA	"StartAlpha"
#define FXBMGR_PARTMUZZLEFX_ENDALPHA	"EndAlpha"
#define	FXBMGR_PARTMUZZLEFX_ADDITIVE	"Additive"
#define	FXBMGR_PARTMUZZLEFX_MULTIPLY	"Multiply"
#define	FXBMGR_PARTMUZZLEFX_MIN_Y_VEL	"MinVertVelocity"
#define	FXBMGR_PARTMUZZLEFX_MAX_Y_VEL	"MaxVertVelocity"
#define	FXBMGR_PARTMUZZLEFX_MIN_X_VEL	"MinHorizVelocity"
#define	FXBMGR_PARTMUZZLEFX_MAX_X_VEL	"MaxHorizVelocity"

#define FXBMGR_MUZZLEFX_TAG				"MuzzleFX"
#define FXBMGR_MUZZLEFX_DURATION		"Duration"
#define FXBMGR_MUZZLEFX_NAME			"Name"
#define FXBMGR_MUZZLEFX_PMUZZLEFXNAME	"PMuzzleFXName"
#define FXBMGR_MUZZLEFX_SCALEFXNAME		"ScaleFXName"
#define FXBMGR_MUZZLEFX_DLIGHTFXNAME	"DLightFXName"

#define FXBMGR_TRACERFX_TAG				"TracerFX"
#define FXBMGR_TRACERFX_NAME			"Name"
#define FXBMGR_TRACERFX_TEXTURE			"Texture"
#define FXBMGR_TRACERFX_FREQUENCY		"Frequency"
#define FXBMGR_TRACERFX_VELOCITY		"Velocity"
#define FXBMGR_TRACERFX_WIDTH			"Width"
#define FXBMGR_TRACERFX_INITIALALPHA	"InitialAlpha"
#define FXBMGR_TRACERFX_FINALALPHA		"FinalAlpha"
#define FXBMGR_TRACERFX_COLOR			"Color"

#define FXBMGR_BEAMFX_TAG				"BeamFX"
#define FXBMGR_BEAMFX_NAME				"Name"
#define FXBMGR_BEAMFX_TEXTURE			"Texture"
#define FXBMGR_BEAMFX_DURATION			"Duration"
#define FXBMGR_BEAMFX_WIDTH				"Width"
#define FXBMGR_BEAMFX_INITIALALPHA		"InitialAlpha"
#define FXBMGR_BEAMFX_FINALALPHA		"FinalAlpha"
#define FXBMGR_BEAMFX_COLOR				"Color"
#define FXBMGR_BEAMFX_ALIGNUP			"AlignUp"
#define FXBMGR_BEAMFX_ALIGNFLAT			"AlignFlat"

#define FXBMGR_SMOKEFX_TAG				"SmokeFX"
#define FXBMGR_SMOKEFX_NAME				"Name"
#define FXBMGR_SMOKEFX_TEXTURE			"Texture"
#define FXBMGR_SMOKEFX_COLOR1			"Color1"
#define FXBMGR_SMOKEFX_COLOR2			"Color2"
#define FXBMGR_SMOKEFX_MINDRIFT			"MinDriftVel"
#define FXBMGR_SMOKEFX_MAXDRIFT			"MaxDriftVel"
#define FXBMGR_SMOKEFX_LIFETIME			"Lifetime"
#define FXBMGR_SMOKEFX_EMITRADIUS		"EmitRadius"
#define FXBMGR_SMOKEFX_RADIUS			"Radius"
#define FXBMGR_SMOKEFX_CREATEDELTA		"CreateDelta"
#define FXBMGR_SMOKEFX_ADJUSTALPHA		"AdjustAlpha"
#define FXBMGR_SMOKEFX_STARTALPHA		"StartAlpha"
#define FXBMGR_SMOKEFX_ENDALPHA			"EndAlpha"
#define FXBMGR_SMOKEFX_ADJUSTSCALE		"AdjustScale"
#define FXBMGR_SMOKEFX_STARTSCALE		"StartScale"
#define FXBMGR_SMOKEFX_ENDSCALE			"EndScale"
#define FXBMGR_SMOKEFX_MINLIFE			"MinParticleLife"
#define FXBMGR_SMOKEFX_MAXLIFE			"MaxParticleLife"
#define FXBMGR_SMOKEFX_NUMPARTICLES		"NumParticles"
#define FXBMGR_SMOKEFX_ADDITIVE			"Additive"
#define FXBMGR_SMOKEFX_MULTIPLY			"Multiply"
#define FXBMGR_SMOKEFX_IGNORWIND		"IgnorWind"
#define FXBMGR_SMOKEFX_UPDATEPOS		"MaintainParticlePos"

#define FXBMGR_STEAMFX_TAG				"SteamObjFX"
#define FXBMGR_STEAMFX_NAME				"Name"
#define FXBMGR_STEAMFX_SMOKEFX			"SmokeFX"
#define FXBMGR_STEAMFX_SOUNDFX			"SoundFX"
#define FXBMGR_STEAMFX_DAMAGEDIST		"DamageDist"
#define FXBMGR_STEAMFX_DAMAGE			"Damage"

#define FXBMGR_FIREOBJFX_TAG			"FireObjFX"
#define FXBMGR_FIREOBJFX_NAME			"Name"
#define FXBMGR_FIREOBJFX_INNERFX		"InnerFireFX"
#define FXBMGR_FIREOBJFX_OUTERFX		"OuterFireFX"
#define FXBMGR_FIREOBJFX_SMOKEFX		"SmokeFX"
#define FXBMGR_FIREOBJFX_SOUNDFX		"SoundFX"
#define FXBMGR_FIREOBJFX_SCALERADIUS	"ScaleRadius"
#define FXBMGR_FIREOBJFX_CREATELIGHT	"CreateLight"
#define FXBMGR_FIREOBJFX_LIGHTCOLOR		"LightColor"
#define FXBMGR_FIREOBJFX_LIGHTRADIUS	"LightRadius"
#define FXBMGR_FIREOBJFX_LIGHTPHASE		"LightPhase"
#define FXBMGR_FIREOBJFX_LIGHTFREQ		"LightFreq"
#define FXBMGR_FIREOBJFX_LIGHTOFFSET	"LightOffset"
#define FXBMGR_FIREOBJFX_CREATESPARKS	"CreateSparks"

#define FXBMGR_DRIPPEROBJFX_TAG			"DripperObjFX"
#define FXBMGR_DRIPPEROBJFX_NAME		"Name"
#define FXBMGR_DRIPPEROBJFX_PARTICLE	"Particle"
#define FXBMGR_DRIPPEROBJFX_MINCOLOR	"MinColor"
#define FXBMGR_DRIPPEROBJFX_MAXCOLOR	"MaxColor"
#define FXBMGR_DRIPPEROBJFX_ALPHA		"Alpha"
#define FXBMGR_DRIPPEROBJFX_ADDITIVE	"Additive"
#define FXBMGR_DRIPPEROBJFX_MULTIPLY	"Multiply"
#define FXBMGR_DRIPPEROBJFX_MINSIZE		"MinSize"
#define FXBMGR_DRIPPEROBJFX_MAXSIZE		"MaxSize"
#define FXBMGR_DRIPPEROBJFX_MINAMOUNT	"MinDripAmount"
#define FXBMGR_DRIPPEROBJFX_MAXAMOUNT	"MaxDripAmount"
#define FXBMGR_DRIPPEROBJFX_MINFREQUENCY "MinDripFrequency"
#define FXBMGR_DRIPPEROBJFX_MAXFREQUENCY "MaxDripFrequency"
#define FXBMGR_DRIPPEROBJFX_MINTIME		"MinDripTime"
#define FXBMGR_DRIPPEROBJFX_MAXTIME		"MaxDripTime"
#define FXBMGR_DRIPPEROBJFX_DISTANCE	"DripDistance"
#define FXBMGR_DRIPPEROBJFX_SPEED		"DripSpeed"
#define FXBMGR_DRIPPEROBJFX_ACCELTIME	"AcclerateTime"
#define FXBMGR_DRIPPEROBJFX_TLENGTH		"TrailLength"
#define FXBMGR_DRIPPEROBJFX_TDENSITY	"TrailDensity"
#define FXBMGR_DRIPPEROBJFX_WAMOUNT		"WiggleAmount"
#define FXBMGR_DRIPPEROBJFX_WFREQUENCY	"WiggleFrequency"
#define FXBMGR_DRIPPEROBJFX_CYLINDER	"CylinderShaped"
#define FXBMGR_DRIPPEROBJFX_PERCENT		"ImpactFXPercent"
#define FXBMGR_DRIPPEROBJFX_IMPACTFX	"ImpactFX"


static char s_aTagName[30];
static char s_aAttName[100];
static char s_FileBuffer[MAX_CS_FILENAME_LEN];

CFXButeMgr* g_pFXButeMgr = LTNULL;


#ifndef _CLIENTBUILD

#include "DebrisMgr.h"
#include "SoundButeMgr.h"


// Plugin statics

static CDebrisMgrPlugin		m_DebrisMgrPlugin;
static CSoundButeMgrPlugin	m_SoundButeMgrPlugin;
static CFXButeMgr			sm_FXButeMgr;

#endif // _CLIENTBUILD

static CAVector ToCAVector( const LTVector & vec)
{
	return CAVector(vec.x, vec.y, vec.z);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CFXButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CFXButeMgr::CFXButeMgr()
{
	m_ParticleTrailFXList.Init(LTTRUE);
    m_ProjectileFXList.Init(LTTRUE);
    m_ProjClassDataList.Init(LTTRUE);
    m_ImpactFXList.Init(LTTRUE);
    m_FireFXList.Init(LTTRUE);
//  m_ScaleFXList.Init(LTTRUE);
    m_PShowerFXList.Init(LTTRUE);
    m_PolyDebrisFXList.Init(LTTRUE);
    m_PExplFXList.Init(LTTRUE);
    m_DLightFXList.Init(LTTRUE);
    m_SoundFXList.Init(LTTRUE);
    m_PusherFXList.Init(LTTRUE);
    m_PVFXList.Init(LTTRUE);
    m_PartMuzzleFXList.Init(LTTRUE);
    m_MuzzleFXList.Init(LTTRUE);
    m_TracerFXList.Init(LTTRUE);
    m_BeamFXList.Init(LTTRUE);
	m_SmokeFXList.Init(LTTRUE);
	m_SteamFXList.Init(LTTRUE);
	m_FireObjFXList.Init(LTTRUE);
	m_DripperObjFXList.Init(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::~CFXButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CFXButeMgr::~CFXButeMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (g_pFXButeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;


	// Set up global pointer...

	g_pFXButeMgr = this;


	// Build our lists...

	BuildScaleFXList(m_ScaleFXList, m_buteMgr, FXBMGR_SCALEFX_TAG, m_ScaleFXTable);
	BuildPShowerFXList(m_PShowerFXList, m_buteMgr, FXBMGR_PSHOWERFX_TAG);
	BuildPolyDebrisFXList(m_PolyDebrisFXList, m_buteMgr, FXBMGR_POLYDEBRISFX_TAG);


	// Read in the properties for each projectile class data record...
	// NOTE: This must be done before the ProjectileFX records are
	// read in...

	// Read in the properties for the ProxClassData record...

	if (m_buteMgr.Exist(FXBMGR_PROXCLASS_TAG))
	{
		PROJECTILECLASSDATA* pPCD = new PROXCLASSDATA;

		if (pPCD && pPCD->Init(m_buteMgr, FXBMGR_PROXCLASS_TAG))
		{
			m_ProjClassDataList.AddTail(pPCD);
		}
	}

	if (m_buteMgr.Exist(FXBMGR_SPIDERCLASS_TAG))
	{
		PROJECTILECLASSDATA* pPCD = new SPIDERCLASSDATA;

		if (pPCD && pPCD->Init(m_buteMgr, FXBMGR_SPIDERCLASS_TAG))
		{
			m_ProjClassDataList.AddTail(pPCD);
		}
	}

	if (m_buteMgr.Exist(FXBMGR_TRACKINGSADARCLASS_TAG))
	{
		PROJECTILECLASSDATA* pPCD = new TRACKINGSADARCLASSDATA;

		if (pPCD && pPCD->Init(m_buteMgr, FXBMGR_TRACKINGSADARCLASS_TAG))
		{
			m_ProjClassDataList.AddTail(pPCD);
		}
	}

	if (m_buteMgr.Exist(FXBMGR_PREDDISKCLASS_TAG))
	{
		PROJECTILECLASSDATA* pPCD = new PREDDISKCLASSDATA;

		if (pPCD && pPCD->Init(m_buteMgr, FXBMGR_PREDDISKCLASS_TAG))
		{
			m_ProjClassDataList.AddTail(pPCD);
		}
	}


	// Read in the properties for each particle trail fx type...

	int nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_PARTICLETRAILFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		PARTICLETRAILFX* pPFX = new PARTICLETRAILFX;

		if (pPFX && pPFX->Init(m_buteMgr, s_aTagName))
		{
			pPFX->nId = nNum;
			m_ParticleTrailFXList.AddTail(pPFX);
		}
		else
		{
			delete pPFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_PARTICLETRAILFX_TAG, nNum);
	}


	// Read in the properties for each projectile fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_PROJECTILEFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		PROJECTILEFX* pPFX = new PROJECTILEFX;

		if (pPFX && pPFX->Init(m_buteMgr, s_aTagName))
		{
			pPFX->nId = nNum;
			m_ProjectileFXList.AddTail(pPFX);
		}
		else
		{
			delete pPFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_PROJECTILEFX_TAG, nNum);
	}


	// Read in the properties for each beam fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_BEAMFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		BEAMFX* pFX = new BEAMFX;

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_BeamFXList.AddTail(pFX);
		}
		else
		{
			delete pFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_BEAMFX_TAG, nNum);
	}


	// Read in the properties for each fire fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_FIREFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		FIREFX* pFFX = new FIREFX;

		if (pFFX && pFFX->Init(m_buteMgr, s_aTagName))
		{
			pFFX->nId = nNum;
			m_FireFXList.AddTail(pFFX);
		}
		else
		{
			delete pFFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_FIREFX_TAG, nNum);
	}


	// Read in the properties for each particle explosion fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_PEXPLFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		PEXPLFX* pPEFX = new PEXPLFX;

		if (pPEFX && pPEFX->Init(m_buteMgr, s_aTagName))
		{
			pPEFX->nId = nNum;
			m_PExplFXList.AddTail(pPEFX);
		}
		else
		{
			delete pPEFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_PEXPLFX_TAG, nNum);
	}


	// Read in the properties for each dynamic light fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_DLIGHTFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		DLIGHTFX* pDLightFX = new DLIGHTFX;

		if (pDLightFX && pDLightFX->Init(m_buteMgr, s_aTagName))
		{
			pDLightFX->nId = nNum;
			m_DLightFXList.AddTail(pDLightFX);
		}
		else
		{
			delete pDLightFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_DLIGHTFX_TAG, nNum);
	}


	// Read in the properties for each sound light fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_SOUNDFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		SOUNDFX* pFX = new SOUNDFX;

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_SoundFXList.AddTail(pFX);
		}
		else
		{
			delete pFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_SOUNDFX_TAG, nNum);
	}


	// Read in the properties for each pusher fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_PUSHERFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		PUSHERFX* pFX = new PUSHERFX;

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_PusherFXList.AddTail(pFX);
		}
		else
		{
			delete pFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_PUSHERFX_TAG, nNum);
	}


	// Read in the properties for each impact fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_IMPACTFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		IMPACTFX* pIFX = new IMPACTFX;

		if (pIFX && pIFX->Init(m_buteMgr, s_aTagName))
		{
			pIFX->nId = nNum;
			m_ImpactFXList.AddTail(pIFX);
		}
		else
		{
			delete pIFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_IMPACTFX_TAG, nNum);
	}


	// Read in the properties for each pv fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_PVFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		PVFX* pFX = new PVFX;

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_PVFXList.AddTail(pFX);
		}
		else
		{
			delete pFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_PVFX_TAG, nNum);
	}


	// Read in the properties for each particle muzzle fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_PARTMUZZLEFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		CParticleMuzzleFX* pFX = new CParticleMuzzleFX;

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_PartMuzzleFXList.AddTail(pFX);
		}
		else
		{
			delete pFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_PARTMUZZLEFX_TAG, nNum);
	}


	// Read in the properties for each muzzle fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_MUZZLEFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		CMuzzleFX* pFX = new CMuzzleFX;

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_MuzzleFXList.AddTail(pFX);
		}
		else
		{
			delete pFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_MUZZLEFX_TAG, nNum);
	}


	// Read in the properties for each tracer fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_TRACERFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		TRACERFX* pFX = new TRACERFX;

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_TracerFXList.AddTail(pFX);
		}
		else
		{
			delete pFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_TRACERFX_TAG, nNum);
	}


	// Read in the properties for each smoke fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_SMOKEFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		SMOKEFX* pFX = new SMOKEFX;

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_SmokeFXList.AddTail(pFX);
		}
		else
		{
			delete pFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_SMOKEFX_TAG, nNum);
	}


	// Read in the properties for each steam fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_STEAMFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		STEAMFX* pFX = new STEAMFX;

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_SteamFXList.AddTail(pFX);
		}
		else
		{
			delete pFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_STEAMFX_TAG, nNum);
	}


	// Read in the properties for each fire obj fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_FIREOBJFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		FIREOBJFX* pFX = new FIREOBJFX;

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_FireObjFXList.AddTail(pFX);
		}
		else
		{
			delete pFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_FIREOBJFX_TAG, nNum);
	}


	// Read in the properties for each fire obj fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_DRIPPEROBJFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		DRIPPEROBJFX* pFX = new DRIPPEROBJFX;

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_DripperObjFXList.AddTail(pFX);
		}
		else
		{
			delete pFX;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_DRIPPEROBJFX_TAG, nNum);
	}


	// Free up the bute mgr's memory...

	m_buteMgr.Term();


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CFXButeMgr::Term()
{
    g_pFXButeMgr = LTNULL;

	m_ParticleTrailFXList.Clear();
    m_ProjectileFXList.Clear();
    m_ProjClassDataList.Clear();
    m_ImpactFXList.Clear();
    m_FireFXList.Clear();
	m_ScaleFXList.clear();
    m_PShowerFXList.Clear();
    m_PolyDebrisFXList.Clear();
    m_PExplFXList.Clear();
    m_DLightFXList.Clear();
    m_SoundFXList.Clear();
    m_PusherFXList.Clear();
    m_PVFXList.Clear();
    m_PartMuzzleFXList.Clear();
    m_MuzzleFXList.Clear();
    m_TracerFXList.Clear();
    m_BeamFXList.Clear();
	m_SmokeFXList.Clear();
	m_SteamFXList.Clear();
	m_FireObjFXList.Clear();
	m_DripperObjFXList.Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::Reload()
//
//	PURPOSE:	Reload data from the bute file
//
// ----------------------------------------------------------------------- //

void CFXButeMgr::Reload(ILTCSBase *pInterface)
{
	Term();
    Init(pInterface);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetScaleFX
//
//	PURPOSE:	Get the specified scale fx struct
//
// ----------------------------------------------------------------------- //

CScaleFX* CFXButeMgr::GetScaleFX(int nScaleFXId)
{
    CScaleFX** pCur  = LTNULL;

	if(nScaleFXId < 0) return LTNULL;

	if((int)m_ScaleFXList.size() > nScaleFXId)
		return m_ScaleFXList[nScaleFXId];
	else
		return LTNULL;

//	pCur = m_ScaleFXList.GetItem(TLIT_FIRST);
//
//	while (pCur)
//	{
//		if (*pCur && (*pCur)->nId == nScaleFXId)
//		{
//			return *pCur;
//		}
//
//		pCur = m_ScaleFXList.GetItem(TLIT_NEXT);
//	}
//
//	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetScaleFX
//
//	PURPOSE:	Get the specified scale fx struct
//
// ----------------------------------------------------------------------- //

CScaleFX* CFXButeMgr::GetScaleFX(char* pName)
{
    if (!pName) return LTNULL;

	// Look the name up in our index table.
	IndexTable::const_iterator iter = m_ScaleFXTable.find(pName);

	// If we found it, return the index.
	if( iter != m_ScaleFXTable.end() )
	{
		return GetScaleFX(iter->second);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetScaleFX
//
//	PURPOSE:	Get the specified scale fx struct
//
// ----------------------------------------------------------------------- //
/*
CScaleFX* CFXButeMgr::GetScaleFX(char* pName)
{
    if (!pName) return LTNULL;

    CScaleFX** pCur  = LTNULL;

	pCur = m_ScaleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_ScaleFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}
*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPShowerFX
//
//	PURPOSE:	Get the specified PShower fx struct
//
// ----------------------------------------------------------------------- //

CPShowerFX* CFXButeMgr::GetPShowerFX(int nPShowerFXId)
{
    if (nPShowerFXId < 0 || nPShowerFXId > m_PShowerFXList.GetLength()) return LTNULL;

	CPShowerFX** pCur = m_PShowerFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nPShowerFXId)
		{
			return *pCur;
		}

		pCur = m_PShowerFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPShowerFX
//
//	PURPOSE:	Get the specified PShower fx struct
//
// ----------------------------------------------------------------------- //

CPShowerFX* CFXButeMgr::GetPShowerFX(char* pName)
{
    if (!pName) return LTNULL;

	CPShowerFX** pCur = m_PShowerFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PShowerFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPShowerFX
//
//	PURPOSE:	Get the specified PolyDebris fx struct
//
// ----------------------------------------------------------------------- //

CPolyDebrisFX* CFXButeMgr::GetPolyDebrisFX(int nPolyDebrisFXId)
{
    if (nPolyDebrisFXId < 0 || nPolyDebrisFXId > m_PolyDebrisFXList.GetLength()) return LTNULL;

	CPolyDebrisFX** pCur = m_PolyDebrisFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nPolyDebrisFXId)
		{
			return *pCur;
		}

		pCur = m_PolyDebrisFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPShowerFX
//
//	PURPOSE:	Get the specified PolyDebris fx struct
//
// ----------------------------------------------------------------------- //

CPolyDebrisFX* CFXButeMgr::GetPolyDebrisFX(char* pName)
{
    if (!pName) return LTNULL;

	CPolyDebrisFX** pCur = m_PolyDebrisFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PolyDebrisFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetParticleTrailFX
//
//	PURPOSE:	Get the specified ParticleTrail fx struct
//
// ----------------------------------------------------------------------- //

PARTICLETRAILFX* CFXButeMgr::GetParticleTrailFX(int nParticleTrailFXId)
{
    PARTICLETRAILFX** pCur  = LTNULL;

	pCur = m_ParticleTrailFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nParticleTrailFXId)
		{
			return *pCur;
		}

		pCur = m_ParticleTrailFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetParticleTrailFX
//
//	PURPOSE:	Get the specified ParticleTrail fx struct
//
// ----------------------------------------------------------------------- //

PARTICLETRAILFX* CFXButeMgr::GetParticleTrailFX(char* pName)
{
    if (!pName) return LTNULL;

    PARTICLETRAILFX** pCur  = LTNULL;

	pCur = m_ParticleTrailFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_ParticleTrailFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetProjectileFX
//
//	PURPOSE:	Get the specified projectile fx struct
//
// ----------------------------------------------------------------------- //

PROJECTILEFX* CFXButeMgr::GetProjectileFX(int nProjectileFXId)
{
    PROJECTILEFX** pCur  = LTNULL;

	pCur = m_ProjectileFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nProjectileFXId)
		{
			return *pCur;
		}

		pCur = m_ProjectileFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetProjectileFX
//
//	PURPOSE:	Get the specified projectile fx struct
//
// ----------------------------------------------------------------------- //

PROJECTILEFX* CFXButeMgr::GetProjectileFX(char* pName)
{
    if (!pName) return LTNULL;

    PROJECTILEFX** pCur  = LTNULL;

	pCur = m_ProjectileFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_ProjectileFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetProjectileClassData
//
//	PURPOSE:	Get the specified projectile class data struct
//
// ----------------------------------------------------------------------- //

PROJECTILECLASSDATA* CFXButeMgr::GetProjectileClassData(char* pName)
{
    if (!pName) return LTNULL;

    PROJECTILECLASSDATA** pCur  = LTNULL;

	pCur = m_ProjClassDataList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_ProjClassDataList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetImpactFX
//
//	PURPOSE:	Get the specified impact fx struct
//
// ----------------------------------------------------------------------- //

IMPACTFX* CFXButeMgr::GetImpactFX(int nImpactFXId)
{
    IMPACTFX** pCur  = LTNULL;

	pCur = m_ImpactFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nImpactFXId)
		{
			return *pCur;
		}

		pCur = m_ImpactFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetImpactFX
//
//	PURPOSE:	Get the specified impact fx struct
//
// ----------------------------------------------------------------------- //

IMPACTFX* CFXButeMgr::GetImpactFX(char* pName)
{
    if (!pName) return LTNULL;

    IMPACTFX** pCur  = LTNULL;

	pCur = m_ImpactFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_ImpactFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetFireFX
//
//	PURPOSE:	Get the specified fire fx struct
//
// ----------------------------------------------------------------------- //

FIREFX* CFXButeMgr::GetFireFX(int nFireFXId)
{
    FIREFX** pCur  = LTNULL;

	pCur = m_FireFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nFireFXId)
		{
			return *pCur;
		}

		pCur = m_FireFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetFireFX
//
//	PURPOSE:	Get the specified fire fx struct
//
// ----------------------------------------------------------------------- //

FIREFX* CFXButeMgr::GetFireFX(char* pName)
{
    if (!pName) return LTNULL;

    FIREFX** pCur  = LTNULL;

	pCur = m_FireFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_FireFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPExplFX
//
//	PURPOSE:	Get the specified PExpl fx struct
//
// ----------------------------------------------------------------------- //

PEXPLFX* CFXButeMgr::GetPExplFX(int nPExpFXId)
{
    PEXPLFX** pCur  = LTNULL;

	pCur = m_PExplFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nPExpFXId)
		{
			return *pCur;
		}

		pCur = m_PExplFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPExplFX
//
//	PURPOSE:	Get the specified PExpl fx struct
//
// ----------------------------------------------------------------------- //

PEXPLFX* CFXButeMgr::GetPExplFX(char* pName)
{
    if (!pName) return LTNULL;

    PEXPLFX** pCur  = LTNULL;

	pCur = m_PExplFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PExplFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetDLightFX
//
//	PURPOSE:	Get the specified DLIGHT fx struct
//
// ----------------------------------------------------------------------- //

DLIGHTFX* CFXButeMgr::GetDLightFX(int nDLightFXId)
{
    DLIGHTFX** pCur  = LTNULL;

	pCur = m_DLightFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nDLightFXId)
		{
			return *pCur;
		}

		pCur = m_DLightFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetDLightFX
//
//	PURPOSE:	Get the specified DLIGHT fx struct
//
// ----------------------------------------------------------------------- //

DLIGHTFX* CFXButeMgr::GetDLightFX(char* pName)
{
    if (!pName) return LTNULL;

    DLIGHTFX** pCur  = LTNULL;

	pCur = m_DLightFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_DLightFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetSoundFX
//
//	PURPOSE:	Get the specified SOUNDFX struct
//
// ----------------------------------------------------------------------- //

SOUNDFX* CFXButeMgr::GetSoundFX(int nSoundFXId)
{
    SOUNDFX** pCur  = LTNULL;

	pCur = m_SoundFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nSoundFXId)
		{
			return *pCur;
		}

		pCur = m_SoundFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetSoundFX
//
//	PURPOSE:	Get the specified SOUNDFX struct
//
// ----------------------------------------------------------------------- //

SOUNDFX* CFXButeMgr::GetSoundFX(char* pName)
{
    if (!pName) return LTNULL;

    SOUNDFX** pCur  = LTNULL;

	pCur = m_SoundFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_SoundFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPusherFX
//
//	PURPOSE:	Get the specified PUSHERFX struct
//
// ----------------------------------------------------------------------- //

PUSHERFX* CFXButeMgr::GetPusherFX(int nSoundFXId)
{
    PUSHERFX** pCur  = LTNULL;

	pCur = m_PusherFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nSoundFXId)
		{
			return *pCur;
		}

		pCur = m_PusherFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPusherFX
//
//	PURPOSE:	Get the specified PUSHERFX struct
//
// ----------------------------------------------------------------------- //

PUSHERFX* CFXButeMgr::GetPusherFX(char* pName)
{
    if (!pName) return LTNULL;

    PUSHERFX** pCur  = LTNULL;

	pCur = m_PusherFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PusherFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPVFX
//
//	PURPOSE:	Get the specified pv fx struct
//
// ----------------------------------------------------------------------- //

PVFX* CFXButeMgr::GetPVFX(int nPVFXId)
{
    PVFX** pCur = LTNULL;

	pCur = m_PVFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nPVFXId)
		{
			return *pCur;
		}

		pCur = m_PVFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPVFX
//
//	PURPOSE:	Get the specified pv fx struct
//
// ----------------------------------------------------------------------- //

PVFX* CFXButeMgr::GetPVFX(char* pName)
{
    if (!pName) return LTNULL;

    PVFX** pCur  = LTNULL;

	pCur = m_PVFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PVFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetParticleMuzzleFX
//
//	PURPOSE:	Get the specified particle muzzle fx struct
//
// ----------------------------------------------------------------------- //

CParticleMuzzleFX* CFXButeMgr::GetParticleMuzzleFX(int nPMFXId)
{
    CParticleMuzzleFX** pCur = LTNULL;

	pCur = m_PartMuzzleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nPMFXId)
		{
			return *pCur;
		}

		pCur = m_PartMuzzleFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetParticleMuzzleFX
//
//	PURPOSE:	Get the specified particle muzzle fx struct
//
// ----------------------------------------------------------------------- //

CParticleMuzzleFX* CFXButeMgr::GetParticleMuzzleFX(char* pName)
{
    if (!pName) return LTNULL;

    CParticleMuzzleFX** pCur  = LTNULL;

	pCur = m_PartMuzzleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PartMuzzleFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetMuzzleFX
//
//	PURPOSE:	Get the specified muzzle fx struct
//
// ----------------------------------------------------------------------- //

CMuzzleFX* CFXButeMgr::GetMuzzleFX(int nMuzzleFXId)
{
    CMuzzleFX** pCur = LTNULL;

	pCur = m_MuzzleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nMuzzleFXId)
		{
			return *pCur;
		}

		pCur = m_MuzzleFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetMuzzleFX
//
//	PURPOSE:	Get the specified muzzle fx struct
//
// ----------------------------------------------------------------------- //

CMuzzleFX* CFXButeMgr::GetMuzzleFX(char* pName)
{
    if (!pName) return LTNULL;

    CMuzzleFX** pCur  = LTNULL;

	pCur = m_MuzzleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_MuzzleFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetTracerFX
//
//	PURPOSE:	Get the specified tracer fx struct
//
// ----------------------------------------------------------------------- //

TRACERFX* CFXButeMgr::GetTracerFX(int nTracerFXId)
{
    TRACERFX** pCur = LTNULL;

	pCur = m_TracerFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nTracerFXId)
		{
			return *pCur;
		}

		pCur = m_TracerFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetTracerFX
//
//	PURPOSE:	Get the specified tracer fx struct
//
// ----------------------------------------------------------------------- //

TRACERFX* CFXButeMgr::GetTracerFX(char* pName)
{
    if (!pName) return LTNULL;

    TRACERFX** pCur  = LTNULL;

	pCur = m_TracerFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_TracerFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetBeamFX
//
//	PURPOSE:	Get the specified beam fx struct
//
// ----------------------------------------------------------------------- //

BEAMFX* CFXButeMgr::GetBeamFX(int nBeamFXId)
{
    BEAMFX** pCur = LTNULL;

	pCur = m_BeamFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nBeamFXId)
		{
			return *pCur;
		}

		pCur = m_BeamFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetBeamFX
//
//	PURPOSE:	Get the specified beam fx struct
//
// ----------------------------------------------------------------------- //

BEAMFX* CFXButeMgr::GetBeamFX(char* pName)
{
    if (!pName) return LTNULL;

    BEAMFX** pCur  = LTNULL;

	pCur = m_BeamFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_BeamFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetSmokeFX
//
//	PURPOSE:	Get the specified smoke fx struct
//
// ----------------------------------------------------------------------- //

SMOKEFX* CFXButeMgr::GetSmokeFX(int nSmokeFXId)
{
    SMOKEFX** pCur = LTNULL;

	pCur = m_SmokeFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nSmokeFXId)
		{
			return *pCur;
		}

		pCur = m_SmokeFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetSmokeFX
//
//	PURPOSE:	Get the specified smoke fx struct
//
// ----------------------------------------------------------------------- //

SMOKEFX* CFXButeMgr::GetSmokeFX(char* pName)
{
    if (!pName) return LTNULL;

    SMOKEFX** pCur  = LTNULL;

	pCur = m_SmokeFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_SmokeFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetSteamFX
//
//	PURPOSE:	Get the specified steam fx struct
//
// ----------------------------------------------------------------------- //

STEAMFX* CFXButeMgr::GetSteamFX(int nSteamFXId)
{
    STEAMFX** pCur = LTNULL;

	pCur = m_SteamFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nSteamFXId)
		{
			return *pCur;
		}

		pCur = m_SteamFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetSteamFX
//
//	PURPOSE:	Get the specified steam fx struct
//
// ----------------------------------------------------------------------- //

STEAMFX* CFXButeMgr::GetSteamFX(char* pName)
{
    if (!pName) return LTNULL;

    STEAMFX** pCur  = LTNULL;

	pCur = m_SteamFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_SteamFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetFireObjFX
//
//	PURPOSE:	Get the specified fire obj fx struct
//
// ----------------------------------------------------------------------- //

FIREOBJFX* CFXButeMgr::GetFireObjFX(int nFireObjFXId)
{
    FIREOBJFX** pCur = LTNULL;

	pCur = m_FireObjFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nFireObjFXId)
		{
			return *pCur;
		}

		pCur = m_FireObjFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetFireObjFX
//
//	PURPOSE:	Get the specified steam fx struct
//
// ----------------------------------------------------------------------- //

FIREOBJFX* CFXButeMgr::GetFireObjFX(char* pName)
{
    if (!pName) return LTNULL;

    FIREOBJFX** pCur  = LTNULL;

	pCur = m_FireObjFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_FireObjFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetDripperObjFX
//
//	PURPOSE:	Get the specified fire obj fx struct
//
// ----------------------------------------------------------------------- //

DRIPPEROBJFX* CFXButeMgr::GetDripperObjFX(int nDripperObjFXId)
{
    DRIPPEROBJFX** pCur = LTNULL;

	pCur = m_DripperObjFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nDripperObjFXId)
		{
			return *pCur;
		}

		pCur = m_DripperObjFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetDripperObjFX
//
//	PURPOSE:	Get the specified steam fx struct
//
// ----------------------------------------------------------------------- //

DRIPPEROBJFX* CFXButeMgr::GetDripperObjFX(char* pName)
{
    if (!pName) return LTNULL;

    DRIPPEROBJFX** pCur  = LTNULL;

	pCur = m_DripperObjFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_DripperObjFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}







/////////////////////////////////////////////////////////////////////////////
//
//	P E X P L  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PEXPLFX::PEXPLFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PEXPLFX::PEXPLFX()
{
	nId				= FXBMGR_INVALID_ID;

	szName[0]		= '\0';
	szFile[0]		= '\0';

	nNumPerPuff		= 0;
	nNumEmitters	= 0;
	nNumSteps		= 0;
    bCreateDebris   = LTFALSE;
    bRotateDebris   = LTFALSE;
    bIgnoreWind     = LTFALSE;
    bDoBubbles      = LTFALSE;
    bAdditive       = LTFALSE;
    bMultiply       = LTFALSE;
	fLifeTime		= 0.0f;
	fFadeTime		= 0.0f;
	fOffsetTime		= 0.0f;
	fRadius			= 0.0f;
	fGravity		= 0.0f;

	vPosOffset.Init();
	vColor1.Init();
	vColor2.Init();
	vMinVel.Init();
	vMaxVel.Init();
	vMinDriftVel.Init();
	vMaxDriftVel.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PEXPLFX::Init
//
//	PURPOSE:	Build the particle explosion struct
//
// ----------------------------------------------------------------------- //

LTBOOL PEXPLFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	nNumPerPuff		= buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_NUMPERPUFF,nNumPerPuff);
	nNumEmitters	= buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_NUMEMITTERS,nNumEmitters);
	nNumSteps		= buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_NUMSTEPS,nNumSteps);

    bCreateDebris   = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_CREATEDEBRIS,bCreateDebris);
    bRotateDebris   = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_ROTATEDEBRIS,bRotateDebris);
    bIgnoreWind     = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_IGNOREWIND,bIgnoreWind);
    bDoBubbles      = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_DOBUBBLES,bDoBubbles);
    bAdditive       = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_ADDITIVE,bAdditive);
    bMultiply       = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_MULTIPLY,bMultiply);

    fLifeTime       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PEXPLFX_LIFETIME,fLifeTime);
    fFadeTime       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PEXPLFX_FADETIME,fFadeTime);
    fOffsetTime     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PEXPLFX_OFFSETTIME,fOffsetTime);
    fRadius         = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PEXPLFX_RADIUS,fRadius);
    fGravity        = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PEXPLFX_GRAVITY,fGravity);

	CAVector vTemp;

	vTemp = ToCAVector(vPosOffset);
	vPosOffset		= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_POSOFFSET,vTemp);

	vTemp = ToCAVector(vColor1);
	vColor1			= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_COLOR1, vTemp);

	vTemp = ToCAVector(vColor2);
	vColor2			= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_COLOR2, vTemp);

	vTemp = ToCAVector(vMinVel);
	vMinVel			= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_MINVEL, vTemp);

	vTemp = ToCAVector(vMaxVel);
	vMaxVel			= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_MAXVEL, vTemp);

	vTemp = ToCAVector(vMinDriftVel);
	vMinDriftVel	= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_MINDRIFTVEL, vTemp);

	vTemp = ToCAVector(vMaxDriftVel);
	vMaxDriftVel	= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_MAXDRIFTVEL, vTemp);

	CString str = buteMgr.GetString(aTagName, FXBMGR_PEXPLFX_FILE, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szFile, (char*)(LPCSTR)str, ARRAY_LEN(szFile));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PEXPLFX_NAME, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PEXPLFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the particle
//				explosion struct
//
// ----------------------------------------------------------------------- //

void PEXPLFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (!pFXButeMgr) return;

	if (szFile[0])
	{
		if (strstr(szFile, ".spr"))
		{
            g_pLTServer->CacheFile(FT_SPRITE, szFile);
		}
		else
		{
            g_pLTServer->CacheFile(FT_TEXTURE, szFile);
		}
	}
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//	D L I G H T  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DLIGHTFX::DLIGHTFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DLIGHTFX::DLIGHTFX()
{
	nId		= FXBMGR_INVALID_ID;

	szName[0]		= '\0';

	vColor.Init();

	fMinRadius		= 0.0f;
	fMaxRadius		= 0.0f;
	fMinTime		= 0.0f;
	fMaxTime		= 0.0f;
	fRampUpTime		= 0.0f;
	fRampDownTime	= 0.0f;
	fWashoutAmount	= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DLIGHTFX::Init
//
//	PURPOSE:	Build the dynamic light struct
//
// ----------------------------------------------------------------------- //

LTBOOL DLIGHTFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

    fMinRadius      = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_MINRADIUS, fMinRadius);
    fMaxRadius      = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_MAXRADIUS, fMaxRadius);
    fMinTime        = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_MINTIME, fMinTime);
    fMaxTime        = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_MAXTIME, fMaxTime);
    fRampUpTime     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_RAMPUPTIME, fRampUpTime);
    fRampDownTime   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_RAMPDOWNTIME,fRampDownTime );
    fWashoutAmount  = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_WASHOUT,fWashoutAmount );

	CAVector vTemp;
	
	vTemp = ToCAVector(vColor);
	vColor			= buteMgr.GetVector(aTagName, FXBMGR_DLIGHTFX_COLOR, vTemp);
	vColor /= 255.0f;

	CString str = buteMgr.GetString(aTagName, FXBMGR_DLIGHTFX_NAME, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DLIGHTFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the dynamic
//				light struct
//
// ----------------------------------------------------------------------- //

void DLIGHTFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
#endif
}



/////////////////////////////////////////////////////////////////////////////
//
//	I M P A C T  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IMPACTFX::IMPACTFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

IMPACTFX::IMPACTFX()
{
	int i = 0;

	nId		= FXBMGR_INVALID_ID;

	szName[0]	= '\0';
	szSound[0]	= '\0';
	szMark[0]	= '\0';
	szSoundDir[0]	= '\0';

	nSoundRadius	= 0;
	nAISoundRadius	= 0;
	nSoundBute		= -1;
	nFlags			= 0;
	fMarkScale		= 0.0f;
	fTintRampUp		= 0.0f;
	fTintRampDown	= 0.0f;
	fTintMaxTime	= 0.0f;
	fTintRadius		= 0.0f;
    bDoSurfaceFX    = LTFALSE;

	vTintColor.Init();

	pPusherFX		= LTNULL;

	nNumDebrisFXTypes = 0;
	for (i=0; i < IMPACT_MAX_DEBRISFX_TYPES; i++)
	{
		aDebrisFXTypes[i] = FXBMGR_INVALID_ID;
	}

	nNumScaleFXTypes = 0;
	for (i=0; i < IMPACT_MAX_SCALEFX_TYPES; i++)
	{
		aScaleFXTypes[i] = FXBMGR_INVALID_ID;
	}

	nNumPExplFX = 0;
	for (i=0; i < IMPACT_MAX_PEXPLFX_TYPES; i++)
	{
		aPExplFXTypes[i] = FXBMGR_INVALID_ID;
	}

	nNumDLightFX = 0;
	for (i=0; i < IMPACT_MAX_DLIGHTFX_TYPES; i++)
	{
		aDLightFXTypes[i] = FXBMGR_INVALID_ID;
	}

	nNumPolyDebrisFX = 0;
	for (i=0; i < IMPACT_MAX_PDEBRISFX_TYPES; i++)
	{
		aPolyDebrisFXTypes[i] = FXBMGR_INVALID_ID;
	}

	nNumPShowerFX = 0;
	for (i=0; i < IMPACT_MAX_PSHOWERFX_TYPES; i++)
	{
		aPShowerFXTypes[i] = FXBMGR_INVALID_ID;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IMPACTFX::Init
//
//	PURPOSE:	Build the impact fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL IMPACTFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_IMPACTFX_SOUND, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szSound, (char*)(LPCSTR)str, ARRAY_LEN(szSound));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_IMPACTFX_SOUND_DIR, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szSoundDir, (char*)(LPCSTR)str, ARRAY_LEN(szSoundDir));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_IMPACTFX_MARK, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szMark, (char*)(LPCSTR)str, ARRAY_LEN(szMark));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_IMPACTFX_NAME, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_IMPACTFX_PUSHERNAME, CString("") );
	if (!str.IsEmpty())
	{
		pPusherFX = g_pFXButeMgr->GetPusherFX((char*)(LPCSTR)str);
	}

	nSoundRadius	= buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_SOUNDRADIUS, nSoundRadius);
	nAISoundRadius	= buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_AISOUNDRADIUS, nAISoundRadius);

	str = buteMgr.GetString(aTagName, FXBMGR_IMPACTFX_SOUNDBUTE, CString("") );
	if (!str.IsEmpty() && g_pSoundButeMgr)
	{
		nSoundBute = g_pSoundButeMgr->GetSoundSetFromName((char*)(LPCSTR)str);
	}

    fMarkScale      = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_MARKSCALE, fMarkScale);
    fTintRampUp     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_TINTRAMPUP, fTintRampUp);
    fTintRampDown   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_TINTRAMPDOWN, fTintRampDown);
    fTintMaxTime    = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_TINTMAXTIME, fTintMaxTime);
	fTintRadius		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_TINTRADIUS, fTintRadius);

	CAVector vTemp;
	
	vTemp = ToCAVector(vTintColor);
	vTintColor		= buteMgr.GetVector(aTagName, FXBMGR_IMPACTFX_TINTCOLOR, vTemp);
	vTintColor /= 255.0f;

    bDoSurfaceFX    = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_DOSURFACEFX, bDoSurfaceFX);

	// Build our debris fx types id array...

	nNumDebrisFXTypes = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_DEBRISNAME, nNumDebrisFXTypes);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumDebrisFXTypes < IMPACT_MAX_DEBRISFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			DEBRIS* pDebrisFX = g_pDebrisMgr->GetDebris((char*)(LPCSTR)str);
			if (pDebrisFX)
			{
				aDebrisFXTypes[nNumDebrisFXTypes] = pDebrisFX->nId;
			}
		}
		else
			break;

		nNumDebrisFXTypes++;
		sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_DEBRISNAME, nNumDebrisFXTypes);
	}

	// Build our scale fx types id array...

	nNumScaleFXTypes = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_SCALENAME, nNumScaleFXTypes);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumScaleFXTypes < IMPACT_MAX_SCALEFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX((char*)(LPCSTR)str);
			if (pScaleFX)
			{
				aScaleFXTypes[nNumScaleFXTypes] = pScaleFX->nId;
			}
		}
		else
			break;

		nNumScaleFXTypes++;
		sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_SCALENAME, nNumScaleFXTypes);
	}

	// Build our particle explosion fx types id array...

	nNumPExplFX = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_PEXPLNAME, nNumPExplFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumPExplFX < IMPACT_MAX_PEXPLFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			PEXPLFX* pPExplFX = g_pFXButeMgr->GetPExplFX((char*)(LPCSTR)str);
			if (pPExplFX)
			{
				aPExplFXTypes[nNumPExplFX] = pPExplFX->nId;
			}
		}
		else
			break;

		nNumPExplFX++;
		sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_PEXPLNAME, nNumPExplFX);
	}

	// Build our dynamic light fx types id array...

	nNumDLightFX = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_DLIGHTNAME, nNumDLightFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumDLightFX < IMPACT_MAX_DLIGHTFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			DLIGHTFX* pDLightFX = g_pFXButeMgr->GetDLightFX((char*)(LPCSTR)str);
			if (pDLightFX)
			{
				aDLightFXTypes[nNumDLightFX] = pDLightFX->nId;
			}
		}
		else
			break;

		nNumDLightFX++;
		sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_DLIGHTNAME, nNumDLightFX);
	}


	// Build our poly debris fx types id array...

	nNumPolyDebrisFX = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_PDEBRISNAME, nNumPolyDebrisFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumPolyDebrisFX < IMPACT_MAX_PDEBRISFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			CPolyDebrisFX* pPDebrisFX = g_pFXButeMgr->GetPolyDebrisFX((char*)(LPCSTR)str);
			if (pPDebrisFX)
			{
				aPolyDebrisFXTypes[nNumPolyDebrisFX] = pPDebrisFX->nId;
			}
		}
		else
			break;

		nNumPolyDebrisFX++;
		sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_PDEBRISNAME, nNumPolyDebrisFX);
	}


	// Build our pshower fx types id array...

	nNumPShowerFX = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_PSHOWERNAME, nNumPShowerFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumPShowerFX < IMPACT_MAX_PSHOWERFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX((char*)(LPCSTR)str);
			if (pPShowerFX)
			{
				aPShowerFXTypes[nNumPShowerFX] = pPShowerFX->nId;
			}
		}
		else
			break;

		nNumPShowerFX++;
		sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_PSHOWERNAME, nNumPShowerFX);
	}


	nFlags = 0;

	if (buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_CREATEMARK, 0))
	{
		nFlags |= WFX_MARK;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_SCREENTINT, 0))
	{
		nFlags |= WFX_TINTSCREEN;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_IMPACTONSKY, 0))
	{
		nFlags |= WFX_IMPACTONSKY;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_IMPACTONFLESH, 0))
	{
		nFlags |= WFX_IMPACTONFLESH;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_3RDPERSONONLY, 0))
	{
		nFlags |= WFX_3RDPERSONONLY;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IMPACTFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the impact
//				fx struct
//
// ----------------------------------------------------------------------- //

void IMPACTFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	int i = 0;

	if (pPusherFX)
	{
		pPusherFX->Cache(pFXButeMgr);
	}

	if (szSound[0] && strstr(szSound, ".wav"))
	{
        g_pLTServer->CacheFile(FT_SOUND, szSound);
	}

	if (szMark[0] && strstr(szMark, ".spr"))
	{
        g_pLTServer->CacheFile(FT_SPRITE, szMark);
	}

	for (i=0; i < nNumDebrisFXTypes; i++)
	{
		DEBRIS* pDebrisFX = g_pDebrisMgr->GetDebris(aDebrisFXTypes[i]);
		if (pDebrisFX)
		{
			pDebrisFX->Cache(g_pDebrisMgr);
		}
	}


	for (i=0; i < nNumScaleFXTypes; i++)
	{
		CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX(aScaleFXTypes[i]);
		if (pScaleFX)
		{
			pScaleFX->Cache();
		}
	}

	for (i=0; i < nNumPExplFX; i++)
	{
		PEXPLFX* pPExplFX = g_pFXButeMgr->GetPExplFX(aPExplFXTypes[i]);
		if (pPExplFX)
		{
			pPExplFX->Cache(pFXButeMgr);
		}
	}

	for (i=0; i < nNumDLightFX; i++)
	{
		DLIGHTFX* pDLightFX = g_pFXButeMgr->GetDLightFX(aDLightFXTypes[i]);
		if (pDLightFX)
		{
			pDLightFX->Cache(pFXButeMgr);
		}
	}

	for (i=0; i < nNumPolyDebrisFX; i++)
	{
		CPolyDebrisFX* pPolyDebrisFX = g_pFXButeMgr->GetPolyDebrisFX(aPolyDebrisFXTypes[i]);
		if (pPolyDebrisFX)
		{
			pPolyDebrisFX->Cache();
		}
	}

	for (i=0; i < nNumPShowerFX; i++)
	{
		CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX(aPShowerFXTypes[i]);
		if (pPShowerFX)
		{
			pPShowerFX->Cache();
		}
	}

#endif
}




/////////////////////////////////////////////////////////////////////////////
//
//	F I R E  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FIREFX::FIREFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FIREFX::FIREFX()
{
	nId		= FXBMGR_INVALID_ID;

	szName[0]		= '\0';
	szShellModel[0] = '\0';
	szShellSkin[0]	= '\0';

	nFlags = 0;
	vShellScale.Init(1, 1, 1);

	nNumBeamFX = 0;

	for (int i=0; i < FIRE_MAX_BEAMFX; i++)
	{
		pBeamFX[i] = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FIREFX::Init
//
//	PURPOSE:	Build the fire fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL FIREFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_FIREFX_SHELLMODEL, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szShellModel, (char*)(LPCSTR)str, ARRAY_LEN(szShellModel));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_FIREFX_SHELLSKIN, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szShellSkin, (char*)(LPCSTR)str, ARRAY_LEN(szShellSkin));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_FIREFX_NAME, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	CAVector vTemp;
	
	vTemp = ToCAVector(vShellScale);
	vShellScale = buteMgr.GetVector(aTagName, FXBMGR_FIREFX_SHELLSCALE, vTemp);

	nFlags = 0;

	if (buteMgr.GetInt(aTagName, FXBMGR_FIREFX_MUZZLESMOKE, 0))
	{
		nFlags |= WFX_MUZZLE;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_FIREFX_EJECTSHELLS, 0))
	{
		nFlags |= WFX_SHELL;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_FIREFX_MUZZLELIGHT, 0))
	{
		nFlags |= WFX_LIGHT;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_FIREFX_FIRESOUND, 0))
	{
		nFlags |= WFX_FIRESOUND;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_FIREFX_EXITMARK, 0))
	{
		nFlags |= WFX_EXITMARK;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_FIREFX_EXITDEBRIS, 0))
	{
		nFlags |= WFX_EXITDEBRIS;
	}

	// Build our beam fx array...

	nNumBeamFX = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_FIREFX_BEAMFXNAME, nNumBeamFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumBeamFX < IMPACT_MAX_PDEBRISFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName, CString("") );
		if (!str.IsEmpty())
		{
			BEAMFX* pFX = g_pFXButeMgr->GetBeamFX((char*)(LPCSTR)str);
			if (pFX)
			{
				pBeamFX[nNumBeamFX] = pFX;
			}
		}
		else
			break;

		nNumBeamFX++;
		sprintf(s_aAttName, "%s%d", FXBMGR_FIREFX_BEAMFXNAME, nNumBeamFX);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FIREFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the fire
//				fx struct
//
// ----------------------------------------------------------------------- //

void FIREFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	if (szShellModel[0])
	{
        g_pLTServer->CacheFile(FT_MODEL, szShellModel);
	}

	if (szShellSkin[0])
	{
        g_pLTServer->CacheFile(FT_TEXTURE, szShellSkin);
	}

	for (int i=0; i < nNumBeamFX; i++)
	{
		if (pBeamFX[i])
		{
			pBeamFX[i]->Cache(g_pFXButeMgr);
		}
	}

#endif
}





/////////////////////////////////////////////////////////////////////////////
//
//	P A R T I C L E T R A I L  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PARTICLETRAILFX::PARTICLETRAILFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PARTICLETRAILFX::PARTICLETRAILFX()
{
	nId		= FXBMGR_INVALID_ID;

	szName[0]			= '\0';
	szTexture[0]		= '\0';

	vMinColor.Init();
	vMaxColor.Init();
	vMinDrift.Init();
	vMaxDrift.Init();
	fLifetime			= 0.0f;
	fFadetime			= 0.0f;
	vMinEmitRange.Init();
	vMaxEmitRange.Init();
	fEmitDistance		= 0.0f;
	nEmitAmount			= 0;
	fStartScale			= 0.0f;
	fEndScale			= 0.0f;
	fStartAlpha			= 0.0f;
	fEndAlpha			= 0.0f;
	fRadius				= 0.0f;
	fGravity			= 0.0f;
	bAdditive			= LTFALSE;
	bMultiply			= LTFALSE;
	bIgnorWind			= LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PARTICLETRAILFX::Init
//
//	PURPOSE:	Build the projectile fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL PARTICLETRAILFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_PARTICLETRAILFX_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PARTICLETRAILFX_TEXTURE);
	if (!str.IsEmpty())
	{
		strncpy(szTexture, (char*)(LPCSTR)str, ARRAY_LEN(szTexture));
	}

	CAVector vTemp;

	vTemp = ToCAVector(vMinColor);
	vMinColor		= buteMgr.GetVector(aTagName, FXBMGR_PARTICLETRAILFX_MINCOLOR, vTemp);

	vTemp = ToCAVector(vMaxColor);
	vMaxColor		= buteMgr.GetVector(aTagName, FXBMGR_PARTICLETRAILFX_MAXCOLOR, vTemp);

	vTemp = ToCAVector(vMinDrift);
	vMinDrift		= buteMgr.GetVector(aTagName, FXBMGR_PARTICLETRAILFX_MINDRIFT, vTemp);

	vTemp = ToCAVector(vMaxDrift);
	vMaxDrift		= buteMgr.GetVector(aTagName, FXBMGR_PARTICLETRAILFX_MAXDRIFT, vTemp);

	fLifetime		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTICLETRAILFX_LIFETIME, fLifetime );
	fFadetime		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTICLETRAILFX_FADETIME, fFadetime );
	
	vTemp = ToCAVector(vMinEmitRange);
	vMinEmitRange	= buteMgr.GetVector(aTagName, FXBMGR_PARTICLETRAILFX_MINEMITRANGE, vTemp);

	vTemp = ToCAVector(vMaxEmitRange);
	vMaxEmitRange	= buteMgr.GetVector(aTagName, FXBMGR_PARTICLETRAILFX_MAXEMITRANGE, vTemp);

	fEmitDistance	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTICLETRAILFX_EMITDISTANCE, fEmitDistance );
	nEmitAmount		= buteMgr.GetInt(aTagName, FXBMGR_PARTICLETRAILFX_EMITAMOUNT, nEmitAmount );
	fStartScale		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTICLETRAILFX_STARTSCALE, fStartScale );
	fEndScale		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTICLETRAILFX_ENDSCALE, fEndScale );
	fStartAlpha		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTICLETRAILFX_STARTALPHA, fStartAlpha );
	fEndAlpha		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTICLETRAILFX_ENDALPHA, fEndAlpha );
	fRadius			= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTICLETRAILFX_RADIUS, fRadius );
	fGravity		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTICLETRAILFX_GRAVITY, fGravity );
	bAdditive		= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PARTICLETRAILFX_ADDITIVE, bAdditive );
	bMultiply		= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PARTICLETRAILFX_MULTIPLY, bMultiply );
	bIgnorWind		= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PARTICLETRAILFX_IGNORWIND, bIgnorWind );

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PARTICLETRAILFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the particle trail
//				fx struct
//
// ----------------------------------------------------------------------- //

void PARTICLETRAILFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	if (szTexture[0])
	{
        g_pLTServer->CacheFile(FT_TEXTURE, szTexture);
	}

#endif
}




/////////////////////////////////////////////////////////////////////////////
//
//	P R O J E C T I L E  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROJECTILEFX::PROJECTILEFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PROJECTILEFX::PROJECTILEFX()
{
	nId		= FXBMGR_INVALID_ID;

	szName[0]			= '\0';
	szFlareSprite[0]	= '\0';
	szSound[0]			= '\0';
	szClass[0]			= '\0';
	szModel[0]			= '\0';
	szSkin[0]			= '\0';
	szToggleScaleFX[0]	= '\0';
	szSocket[0]			= '\0';

	bNoLighting		= LTFALSE;
	fModelAlpha		= 1.0f;

	nAltVelocity	= 0;
	nVelocity		= 0;
	nFlags			= 0;
	nLightRadius	= 0;
	nSoundRadius	= 0;
	fLifeTime		= 0.0f;
	fFlareScale		= 0.0f;
	dwObjectFlags	= 0;
	nParticleTrail	= -1;
	nUWParticleTrail= -1;

    pClassData      = LTNULL;

	vLightColor.Init();
	vModelScale.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROJECTILEFX::Init
//
//	PURPOSE:	Build the projectile fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL PROJECTILEFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	nFlags = 0;

	CString str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_FLARESPRITE, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szFlareSprite, (char*)(LPCSTR)str, ARRAY_LEN(szFlareSprite));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_SOUND, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szSound, (char*)(LPCSTR)str, ARRAY_LEN(szSound));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_CLASS, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szClass, (char*)(LPCSTR)str, ARRAY_LEN(szClass));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_CLASSDATA, CString("") );
	if (!str.IsEmpty())
	{
		pClassData = g_pFXButeMgr->GetProjectileClassData((char*)(LPCSTR)str);
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_MODEL, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szModel, (char*)(LPCSTR)str, ARRAY_LEN(szModel));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_SKIN, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szSkin, (char*)(LPCSTR)str, ARRAY_LEN(szSkin));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_SCALEFX, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szToggleScaleFX, (char*)(LPCSTR)str, ARRAY_LEN(szToggleScaleFX));
		nFlags |= PFX_SCALEFX;
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_SOCKET, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szSocket, (char*)(LPCSTR)str, ARRAY_LEN(szSocket));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_NAME, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	bNoLighting		= (LTBOOL)buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_NOLIGHTING, bNoLighting);
    fModelAlpha		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PROJECTILEFX_ALPHA, fModelAlpha);

	nAltVelocity	= buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_ALTVELOCITY, nAltVelocity);
	nVelocity		= buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_VELOCITY, nVelocity);
	nLightRadius	= buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_LIGHTRADIUS, nLightRadius);
	nSoundRadius	= buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_SOUNDRADIUS, nSoundRadius);
    fLifeTime       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PROJECTILEFX_LIFETIME, fLifeTime);
    fFlareScale     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PROJECTILEFX_FLARESCALE, fFlareScale);

	CAVector vTemp;

	vTemp = ToCAVector(vLightColor);
	vLightColor	= buteMgr.GetVector(aTagName, FXBMGR_PROJECTILEFX_LIGHTCOLOR, vTemp);
	vLightColor /= 255.0f;

	vTemp = ToCAVector(vModelScale);
	vModelScale	= buteMgr.GetVector(aTagName, FXBMGR_PROJECTILEFX_MODELSCALE, vTemp);

	dwObjectFlags = 0;

	if (buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_GRAVITY,0))
	{
		dwObjectFlags |= FLAG_GRAVITY;
	}


	str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_PARTICLETRAIL, CString("") );
	if (!str.IsEmpty())
	{
		PARTICLETRAILFX *ptfx = g_pFXButeMgr->GetParticleTrailFX(str.GetBuffer(0));

		if(ptfx)
		{
			nParticleTrail = ptfx->nId;
			nFlags |= PFX_PARTICLETRAIL;
		}
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_UWPARTICLETRAIL, CString("") );
	if (!str.IsEmpty())
	{
		PARTICLETRAILFX *ptfx = g_pFXButeMgr->GetParticleTrailFX(str.GetBuffer(0));

		if(ptfx)
		{
			nUWParticleTrail = ptfx->nId;
			nFlags |= PFX_PARTICLETRAIL;
		}
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_FLARE,0))
	{
		nFlags |= PFX_FLARE;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_LIGHT,0))
	{
		nFlags |= PFX_LIGHT;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_FLYSOUND,0))
	{
		nFlags |= PFX_FLYSOUND;
	}


    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROJECTILEFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the projectile
//				fx struct
//
// ----------------------------------------------------------------------- //

void PROJECTILEFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	if (szFlareSprite[0])
	{
        g_pLTServer->CacheFile(FT_SPRITE, szFlareSprite);
	}

	if (szSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szSound);
	}

	if (szModel[0])
	{
        g_pLTServer->CacheFile(FT_MODEL, szModel);
	}

	if (szSkin[0])
	{
        g_pLTServer->CacheFile(FT_TEXTURE, szSkin);
	}

	if (pClassData)
	{
		pClassData->Cache(g_pFXButeMgr);
	}

	if (szToggleScaleFX[0])
	{
        g_pLTServer->CacheFile(FT_SPRITE, szToggleScaleFX);
	}

#endif
}




/////////////////////////////////////////////////////////////////////////////
//
//	P R O J E C T I L E  C L A S S  D A T A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROJECTILECLASSDATA::Init
//
//	PURPOSE:	Build the projectile class data struct
//
// ----------------------------------------------------------------------- //

LTBOOL PROJECTILECLASSDATA::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	strncpy(szName, aTagName, ARRAY_LEN(szName));

    return LTTRUE;
}

/////////////////////////////////////////////////////////////////////////////
//
//	P R O X  C L A S S  D A T A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROXCLASSDATA::PROXCLASSDATA
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PROXCLASSDATA::PROXCLASSDATA()
{
	nActivateRadius		= 0;
	nArmSndRadius		= 0;
	nActivateSndRadius	= 0;

	fArmDelay		= 0.0f;
	fActivateDelay	= 0.0f;

	szArmSound[0]		= '\0';
	szActivateSound[0]	= '\0';
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROXCLASSDATA::Init
//
//	PURPOSE:	Build the prox class data class data struct
//
// ----------------------------------------------------------------------- //

LTBOOL PROXCLASSDATA::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;
    if (!PROJECTILECLASSDATA::Init(buteMgr, aTagName)) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_PROXCLASS_ARMSND, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szArmSound, (char*)(LPCSTR)str, ARRAY_LEN(szArmSound));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROXCLASS_ACTSND, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szActivateSound, (char*)(LPCSTR)str, ARRAY_LEN(szActivateSound));
	}

	nActivateRadius		= buteMgr.GetInt(aTagName, FXBMGR_PROXCLASS_ACTRADIUS, nActivateRadius);
	nArmSndRadius		= buteMgr.GetInt(aTagName, FXBMGR_PROXCLASS_ARMSNDRADIUS, nArmSndRadius);
	nActivateSndRadius	= buteMgr.GetInt(aTagName, FXBMGR_PROXCLASS_ACTSNDRADIUS, nActivateSndRadius);

    fArmDelay       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PROXCLASS_ARMDELAY, fArmDelay);
    fActivateDelay  = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PROXCLASS_ACTDELAY, fActivateDelay);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROXCLASSDATA::Cache
//
//	PURPOSE:	Cache all the resources associated with the prox class
//				data struct
//
// ----------------------------------------------------------------------- //

void PROXCLASSDATA::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	PROJECTILECLASSDATA::Cache(pFXButeMgr);

	if (szArmSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szArmSound);
	}

	if (szActivateSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szActivateSound);
	}

#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//	S P I D E R  C L A S S  D A T A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SPIDERCLASSDATA::SPIDERCLASSDATA
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SPIDERCLASSDATA::SPIDERCLASSDATA()
{
	nActivateRadius		= 0;
	nDetonateRadius		= 0;
	nArmSndRadius		= 0;
	nActivateSndRadius	= 0;

	fArmDelay		= 0.0f;

	szArmSound[0]		= '\0';
	szActivateSound[0]	= '\0';
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SPIDERCLASSDATA::Init
//
//	PURPOSE:	Build the spider class data class data struct
//
// ----------------------------------------------------------------------- //

LTBOOL SPIDERCLASSDATA::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;
    if (!PROJECTILECLASSDATA::Init(buteMgr, aTagName)) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_SPIDERCLASS_ARMSND, CString(""));
	if (!str.IsEmpty())
	{
		strncpy(szArmSound, (char*)(LPCSTR)str, ARRAY_LEN(szArmSound));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_SPIDERCLASS_ACTSND, CString(""));
	if (!str.IsEmpty())
	{
		strncpy(szActivateSound, (char*)(LPCSTR)str, ARRAY_LEN(szActivateSound));
	}

	nActivateRadius		= buteMgr.GetInt(aTagName, FXBMGR_SPIDERCLASS_ACTRADIUS, nActivateRadius);
	nDetonateRadius		= buteMgr.GetInt(aTagName, FXBMGR_SPIDERCLASS_DETRADIUS, nDetonateRadius);
	nArmSndRadius		= buteMgr.GetInt(aTagName, FXBMGR_SPIDERCLASS_ARMSNDRADIUS, nArmSndRadius);
	nActivateSndRadius	= buteMgr.GetInt(aTagName, FXBMGR_SPIDERCLASS_ACTSNDRADIUS, nActivateSndRadius);

    fArmDelay       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SPIDERCLASS_ARMDELAY, fArmDelay);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SPIDERCLASSDATA::Cache
//
//	PURPOSE:	Cache all the resources associated with the prox class
//				data struct
//
// ----------------------------------------------------------------------- //

void SPIDERCLASSDATA::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	PROJECTILECLASSDATA::Cache(pFXButeMgr);

	if (szArmSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szArmSound);
	}

	if (szActivateSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szActivateSound);
	}

#endif
}

/////////////////////////////////////////////////////////////////////////////
//
//	T R A C K I N G S A D A R  C L A S S  D A T A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TRACKINGSADARCLASSDATA::TRACKINGSADARCLASSDATA
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

TRACKINGSADARCLASSDATA::TRACKINGSADARCLASSDATA()
{
	fTurnRadius		= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TRACKINGSADARCLASSDATA::Init
//
//	PURPOSE:	Build the Tracking SADAR class data class data struct
//
// ----------------------------------------------------------------------- //

LTBOOL TRACKINGSADARCLASSDATA::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;
    if (!PROJECTILECLASSDATA::Init(buteMgr, aTagName)) return LTFALSE;

    fTurnRadius = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_TRACKINGSADARCLASS_RADIUS, fTurnRadius);

    return LTTRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
//	P R E D A T O R D I S K  C L A S S  D A T A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PREDDISKCLASSDATA::PREDDISKCLASSDATA
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PREDDISKCLASSDATA::PREDDISKCLASSDATA()
{
    nNormRetrieveCost = 0;
    nTeleRetrieveCost = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PREDDISKCLASSDATA::Init
//
//	PURPOSE:	Build the Predator disk class data class data struct
//
// ----------------------------------------------------------------------- //

LTBOOL PREDDISKCLASSDATA::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;
    if (!PROJECTILECLASSDATA::Init(buteMgr, aTagName)) return LTFALSE;

    nNormRetrieveCost = buteMgr.GetInt(aTagName, FXBMGR_PREDDISKCLASS_RET_COST, nNormRetrieveCost);
    nTeleRetrieveCost = buteMgr.GetInt(aTagName, FXBMGR_PREDDISKCLASS_TELE_COST, nTeleRetrieveCost);

    return LTTRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
//	P V  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PVFX::PVFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PVFX::PVFX()
{
	int i = 0;

	nId		= FXBMGR_INVALID_ID;

	szName[0]	= '\0';
	szSocket[0] = '\0';

	nNumScaleFXTypes = 0;
	for (i=0; i < PV_MAX_SCALEFX_TYPES; i++)
	{
		aScaleFXTypes[i] = FXBMGR_INVALID_ID;
	}

	nNumDLightFX = 0;
	for (i=0; i < PV_MAX_DLIGHTFX_TYPES; i++)
	{
		aDLightFXTypes[i] = FXBMGR_INVALID_ID;
	}

	nNumSoundFX = 0;
	for (i=0; i < PV_MAX_DLIGHTFX_TYPES; i++)
	{
		aSoundFXTypes[i] = FXBMGR_INVALID_ID;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PVFX::Init
//
//	PURPOSE:	Build the pv fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL PVFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_PVFX_NAME, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PVFX_SOCKET, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szSocket, (char*)(LPCSTR)str, ARRAY_LEN(szSocket));
	}


	// Build our scale fx types id array...

	nNumScaleFXTypes = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_PVFX_SCALENAME, nNumScaleFXTypes);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumScaleFXTypes < PV_MAX_SCALEFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName, CString("") );
		if (!str.IsEmpty())
		{
			CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX((char*)(LPCSTR)str);
			if (pScaleFX)
			{
				aScaleFXTypes[nNumScaleFXTypes] = pScaleFX->nId;
			}
		}
		else
			break;

		nNumScaleFXTypes++;
		sprintf(s_aAttName, "%s%d", FXBMGR_PVFX_SCALENAME, nNumScaleFXTypes);
	}


	// Build our dynamic light fx types id array...

	nNumDLightFX = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_PVFX_DLIGHTNAME, nNumDLightFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumDLightFX < PV_MAX_DLIGHTFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName, CString("") );
		if (!str.IsEmpty())
		{
			DLIGHTFX* pDLightFX = g_pFXButeMgr->GetDLightFX((char*)(LPCSTR)str);
			if (pDLightFX)
			{
				aDLightFXTypes[nNumDLightFX] = pDLightFX->nId;
			}
		}
		else
			break;

		nNumDLightFX++;
		sprintf(s_aAttName, "%s%d", FXBMGR_PVFX_DLIGHTNAME, nNumDLightFX);
	}

	// Build our sound fx types id array...

	nNumSoundFX = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_PVFX_SOUNDNAME, nNumSoundFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumSoundFX < PV_MAX_SOUNDFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName, CString("") );
		if (!str.IsEmpty())
		{
			SOUNDFX* pFX = g_pFXButeMgr->GetSoundFX((char*)(LPCSTR)str);
			if (pFX)
			{
				aSoundFXTypes[nNumSoundFX] = pFX->nId;
			}
		}
		else
			break;

		nNumSoundFX++;
		sprintf(s_aAttName, "%s%d", FXBMGR_PVFX_SOUNDNAME, nNumSoundFX);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PVFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the pv
//				fx struct
//
// ----------------------------------------------------------------------- //

void PVFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	int i = 0;

	if (!pFXButeMgr) return;

	for (i=0; i < nNumScaleFXTypes; i++)
	{
		CScaleFX* pScaleFX = pFXButeMgr->GetScaleFX(aScaleFXTypes[i]);
		if (pScaleFX)
		{
			pScaleFX->Cache();
		}
	}

	for (i=0; i < nNumDLightFX; i++)
	{
		DLIGHTFX* pDLightFX = pFXButeMgr->GetDLightFX(aDLightFXTypes[i]);
		if (pDLightFX)
		{
			pDLightFX->Cache(pFXButeMgr);
		}
	}

	for (i=0; i < nNumSoundFX; i++)
	{
		SOUNDFX* pFX = pFXButeMgr->GetSoundFX(aSoundFXTypes[i]);
		if (pFX)
		{
			pFX->Cache(pFXButeMgr);
		}
	}
#endif
}



/////////////////////////////////////////////////////////////////////////////
//
//	P A R T I C L E  M U Z Z L E  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleMuzzleFX::CParticleMuzzleFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CParticleMuzzleFX::CParticleMuzzleFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';
	szFile[0]		= '\0';
	fStartScale		= 0.0f;
	fEndScale		= 0.0f;
    fRadius 		= 0.0f;
	fLength			= 0.0f;
    fDuration		= 0.0f;
	nNumParticles	= 0;
	vStartColor.Init();
	vEndColor.Init();
    fStartAlpha		= 0.0f;
    fEndAlpha		= 0.0f;
    bAdditive       = LTFALSE;
    bMultiply       = LTFALSE;
	fMinVertVel		= 0.0f;
	fMaxVertVel		= 0.0f;
	fMinHorizVel	= 0.0f;
	fMaxHorizVel	= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleMuzzleFX::Init
//
//	PURPOSE:	Build the particle muzzle fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleMuzzleFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_PARTMUZZLEFX_NAME, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PARTMUZZLEFX_FILE, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szFile, (char*)(LPCSTR)str, ARRAY_LEN(szFile));
	}

	fStartScale     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_STARTSCALE, fStartScale);
	fEndScale       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_ENDSCALE, fEndScale);
    fRadius         = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_RADIUS, fRadius);
	fLength         = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_LENGTH, fLength);
    fDuration       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_DURATION, fDuration);
	nNumParticles	= buteMgr.GetInt(aTagName, FXBMGR_PARTMUZZLEFX_NUMBER, nNumParticles);

	CAVector vTemp;

	vTemp = ToCAVector(vStartColor);
	vStartColor		= buteMgr.GetVector(aTagName, FXBMGR_PARTMUZZLEFX_STARTCOLOR, vTemp);

	vTemp = ToCAVector(vEndColor);
	vEndColor		= buteMgr.GetVector(aTagName, FXBMGR_PARTMUZZLEFX_ENDCOLOR, vTemp);

    fStartAlpha     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_STARTALPHA, fStartAlpha);
    fEndAlpha       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_ENDALPHA, fEndAlpha);
    bAdditive       = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PARTMUZZLEFX_ADDITIVE, bAdditive);
    bMultiply       = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PARTMUZZLEFX_MULTIPLY, bMultiply);
	fMinVertVel		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_MIN_Y_VEL, fMinVertVel);
	fMaxVertVel		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_MAX_Y_VEL, fMaxVertVel);
	fMinHorizVel	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_MIN_X_VEL, fMinHorizVel);
	fMaxHorizVel	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_MAX_X_VEL, fMaxHorizVel);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleMuzzleFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the particle
//				muzzle fx struct
//
// ----------------------------------------------------------------------- //

void CParticleMuzzleFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (!pFXButeMgr) return;

	if (szFile[0])
	{
		if (strstr(szFile, ".spr"))
		{
            g_pLTServer->CacheFile(FT_SPRITE, szFile);
		}
		else
		{
            g_pLTServer->CacheFile(FT_TEXTURE, szFile);
		}
	}

#endif
}




/////////////////////////////////////////////////////////////////////////////
//
//	M U Z Z L E  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFX::CMuzzleFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMuzzleFX::CMuzzleFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';
	fDuration		= 0.0f;

    pPMuzzleFX      = LTNULL;
    pScaleFX        = LTNULL;
    pDLightFX       = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFX::Init
//
//	PURPOSE:	Build the muzzle fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL CMuzzleFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_MUZZLEFX_NAME, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_MUZZLEFX_PMUZZLEFXNAME, CString("") );
	if (!str.IsEmpty())
	{
		pPMuzzleFX = g_pFXButeMgr->GetParticleMuzzleFX((char*)(LPCSTR)str);
	}

	str = buteMgr.GetString(aTagName, FXBMGR_MUZZLEFX_SCALEFXNAME, CString("") );
	if (!str.IsEmpty())
	{
		pScaleFX = g_pFXButeMgr->GetScaleFX((char*)(LPCSTR)str);
	}

	str = buteMgr.GetString(aTagName, FXBMGR_MUZZLEFX_DLIGHTFXNAME, CString("") );
	if (!str.IsEmpty())
	{
		pDLightFX = g_pFXButeMgr->GetDLightFX((char*)(LPCSTR)str);
	}

    fDuration = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_MUZZLEFX_DURATION, fDuration);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the muzzle fx struct
//
// ----------------------------------------------------------------------- //

void CMuzzleFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (!pFXButeMgr) return;

	if (pPMuzzleFX)
	{
		pPMuzzleFX->Cache(pFXButeMgr);
	}

	if (pScaleFX)
	{
		pScaleFX->Cache();
	}

	if (pDLightFX)
	{
		pDLightFX->Cache(pFXButeMgr);
	}

#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//	T R A C E R  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TRACERFX::TRACERFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

TRACERFX::TRACERFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';
	szTexture[0]	= '\0';

	nFrequency		= 1;
	fVelocity		= 0.0f;
	fWidth			= 0.0f;
	fInitialAlpha	= 0.0f;
	fFinalAlpha		= 0.0f;
	vColor.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TRACERFX::Init
//
//	PURPOSE:	Build the tracer fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL TRACERFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_TRACERFX_NAME, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_TRACERFX_TEXTURE, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szTexture, (char*)(LPCSTR)str, ARRAY_LEN(szTexture));
	}

	nFrequency		= buteMgr.GetInt(aTagName, FXBMGR_TRACERFX_FREQUENCY, nFrequency);
    fVelocity       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_TRACERFX_VELOCITY, fVelocity);
    fWidth          = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_TRACERFX_WIDTH, fWidth);
    fInitialAlpha   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_TRACERFX_INITIALALPHA, fInitialAlpha);
    fFinalAlpha     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_TRACERFX_FINALALPHA, fFinalAlpha);

	CAVector vTemp;
	
	vTemp = ToCAVector(vColor);
	vColor			= buteMgr.GetVector(aTagName, FXBMGR_TRACERFX_COLOR, vTemp);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TRACERFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the tracer fx struct
//
// ----------------------------------------------------------------------- //

void TRACERFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (szTexture[0])
	{
		g_pLTServer->CacheFile(FT_TEXTURE, szTexture);
	}
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//  B E A M  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BEAMFX::BEAMFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

BEAMFX::BEAMFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';
	szTexture[0]	= '\0';

	fDuration		= 0.0f;
	fWidth			= 0.0f;
	fInitialAlpha	= 0.0f;
	fFinalAlpha		= 0.0f;
	vColor.Init();
	bAlignUp		= LTFALSE;
	bAlignFlat		= LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BEAMFX::Init
//
//	PURPOSE:	Build the beam fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL BEAMFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_BEAMFX_NAME, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_BEAMFX_TEXTURE, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szTexture, (char*)(LPCSTR)str, ARRAY_LEN(szTexture));
	}

    fDuration       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_BEAMFX_DURATION, fDuration);
    fWidth          = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_BEAMFX_WIDTH, fWidth);
    fInitialAlpha   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_BEAMFX_INITIALALPHA, fInitialAlpha);
    fFinalAlpha     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_BEAMFX_FINALALPHA, fFinalAlpha);

	CAVector vTemp;

	vTemp = ToCAVector(vColor);
	vColor			= buteMgr.GetVector(aTagName, FXBMGR_BEAMFX_COLOR, vTemp);

    bAlignUp        = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_BEAMFX_ALIGNUP, bAlignUp);
    bAlignFlat      = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_BEAMFX_ALIGNFLAT, bAlignFlat);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BEAMFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the beam fx struct
//
// ----------------------------------------------------------------------- //

void BEAMFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (szTexture[0])
	{
		g_pLTServer->CacheFile(FT_TEXTURE, szTexture);
	}
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//  S O U N D  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SOUNDFX::SOUNDFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SOUNDFX::SOUNDFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';
	szFile[0]		= '\0';

	fRadius			= 0.0f;
	fPitchShift		= 1.0f;
	bLoop			= LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SOUNDFX::Init
//
//	PURPOSE:	Build the sound fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL SOUNDFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_SOUNDFX_NAME, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_SOUNDFX_FILE, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szFile, (char*)(LPCSTR)str, ARRAY_LEN(szFile));
	}

    fRadius     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SOUNDFX_RADIUS, fRadius);
    fPitchShift	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SOUNDFX_PITCHSHIFT, fPitchShift);
    bLoop		= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_SOUNDFX_LOOP, bLoop);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SOUNDFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the sound fx struct
//
// ----------------------------------------------------------------------- //

void SOUNDFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	if (szFile[0] && strstr(szFile, ".wav"))
	{
        g_pLTServer->CacheFile(FT_SOUND, szFile);
	}

#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//  P U S H E R  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PUSHERFX::PUSHERFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PUSHERFX::PUSHERFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';

	fRadius			= 0.0f;
	fStartDelay		= 0.0f;
	fDuration		= 0.0f;
	fStrength		= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PUSHERFX::Init
//
//	PURPOSE:	Build the pusher fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL PUSHERFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_PUSHERFX_NAME, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

    fRadius		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PUSHERFX_RADIUS, fRadius);
    fStartDelay = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PUSHERFX_STARTDELAY, fStartDelay);
    fDuration	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PUSHERFX_DURATION, fDuration);
    fStrength	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PUSHERFX_STRENGTH, fStrength);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PUSHERFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the pusher fx struct
//
// ----------------------------------------------------------------------- //

void PUSHERFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
#endif
}



/////////////////////////////////////////////////////////////////////////////
//
//  S M O K E  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SMOKEFX::SMOKEFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SMOKEFX::SMOKEFX()
{
	nId					= FXBMGR_INVALID_ID;
	szName[0]			= '\0';
	szFile[0]			= '\0';

	vColor1.Init(0, 0, 0);
	vColor2.Init(0, 0, 0);
	vMinDrift.Init(0, 0, 0);
	vMaxDrift.Init(0, 0, 0);
	fLifetime			= 0.0f;
	fEmitRadius			= 0.0f;
	fRadius				= 0.0f;
	fCreateDelta		= 0.0f;
	bAdjustAlpha		= 0;
	fStartAlpha			= 0.0f;
	fEndAlpha			= 0.0f;
	bAdjustScale		= 0;
	fStartScale			= 0.0f;
	fEndScale			= 0.0f;
	fMinParticleLife	= 0.0f;
	fMaxParticleLife	= 0.0f;
	nNumParticles		= 0;
	bAdditive			= 0;
	bMultiply			= 0;
	bIgnorWind			= 0;
	bUpdatePos			= 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SMOKEFX::Init
//
//	PURPOSE:	Build the smoke fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL SMOKEFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_SMOKEFX_NAME, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_SMOKEFX_TEXTURE, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szFile, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	CAVector vTemp;

	vTemp = ToCAVector(vColor1);
	vColor1				= buteMgr.GetVector(aTagName, FXBMGR_SMOKEFX_COLOR1, vTemp);

	vTemp = ToCAVector(vColor2);
	vColor2				= buteMgr.GetVector(aTagName, FXBMGR_SMOKEFX_COLOR2, vTemp);

	vTemp = ToCAVector(vMinDrift);
	vMinDrift			= buteMgr.GetVector(aTagName, FXBMGR_SMOKEFX_MINDRIFT, vTemp);

	vTemp = ToCAVector(vMaxDrift);
	vMaxDrift			= buteMgr.GetVector(aTagName, FXBMGR_SMOKEFX_MAXDRIFT, vTemp);

	fLifetime			= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SMOKEFX_LIFETIME, fLifetime);
	fEmitRadius			= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SMOKEFX_EMITRADIUS, fEmitRadius);
	fRadius				= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SMOKEFX_RADIUS, fRadius);
	fCreateDelta		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SMOKEFX_CREATEDELTA, fCreateDelta);
	bAdjustAlpha		= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_SMOKEFX_ADJUSTALPHA, bAdjustAlpha);
	fStartAlpha			= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SMOKEFX_STARTALPHA, fStartAlpha);
	fEndAlpha			= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SMOKEFX_ENDALPHA, fEndAlpha);
	bAdjustScale		= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_SMOKEFX_ADJUSTSCALE, bAdjustScale);
	fStartScale			= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SMOKEFX_STARTSCALE, fStartScale);
	fEndScale			= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SMOKEFX_ENDSCALE, fEndScale);
	fMinParticleLife	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SMOKEFX_MINLIFE, fMinParticleLife);
	fMaxParticleLife	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SMOKEFX_MAXLIFE, fMaxParticleLife);
	nNumParticles		= buteMgr.GetInt(aTagName, FXBMGR_SMOKEFX_NUMPARTICLES, nNumParticles);
	bAdditive			= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_SMOKEFX_ADDITIVE, bAdditive);
	bMultiply			= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_SMOKEFX_MULTIPLY, bMultiply);
	bIgnorWind			= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_SMOKEFX_IGNORWIND, bIgnorWind);
	bUpdatePos			= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_SMOKEFX_UPDATEPOS, bUpdatePos);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SMOKEFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the smoke fx struct
//
// ----------------------------------------------------------------------- //

void SMOKEFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	if (szFile[0])
	{
		if (strstr(szFile, ".spr"))
		{
            g_pLTServer->CacheFile(FT_SPRITE, szFile);
		}
		else
		{
            g_pLTServer->CacheFile(FT_TEXTURE, szFile);
		}
	}

#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//  S T E A M  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	STEAMFX::STEAMFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

STEAMFX::STEAMFX()
{
	nId					= FXBMGR_INVALID_ID;
	szName[0]			= '\0';

	nSmokeFX			= FXBMGR_INVALID_ID;
	nSoundFX			= FXBMGR_INVALID_ID;

	fDamageDist			= 0.0f;
	fDamage				= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	STEAMFX::Init
//
//	PURPOSE:	Build the smoke fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL STEAMFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_STEAMFX_NAME, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_STEAMFX_SMOKEFX, CString("") );
	if (!str.IsEmpty())
	{
		SMOKEFX *pSmokeFX = g_pFXButeMgr->GetSmokeFX(str.GetBuffer(0));

		if(pSmokeFX)
			nSmokeFX = pSmokeFX->nId;
	}

	str = buteMgr.GetString(aTagName, FXBMGR_STEAMFX_SOUNDFX, CString("") );
	if (!str.IsEmpty())
	{
		SOUNDFX *pSoundFX = g_pFXButeMgr->GetSoundFX(str.GetBuffer(0));

		if(pSoundFX)
			nSoundFX = pSoundFX->nId;
	}

	fDamageDist			= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_STEAMFX_DAMAGEDIST, fDamageDist);
	fDamage				= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_STEAMFX_DAMAGE, fDamage);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	STEAMFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the smoke fx struct
//
// ----------------------------------------------------------------------- //

void STEAMFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//  F I R E O B J  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FIREOBJFX::FIREOBJFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FIREOBJFX::FIREOBJFX()
{
	nId					= FXBMGR_INVALID_ID;
	szName[0]			= '\0';

	nInnerFireFX		= FXBMGR_INVALID_ID;
	nOuterFireFX		= FXBMGR_INVALID_ID;
	nSmokeFX			= FXBMGR_INVALID_ID;
	nSoundFX			= FXBMGR_INVALID_ID;

	fSizeRadiusScale	= 0.0f;

	bCreateLight		= LTFALSE;
	vLightColor.Init(0.0f, 0.0f, 0.0f);
	fLightRadius		= 0.0f;
	fLightPhase			= 0.0f;
	fLightFreq			= 0.0f;
	vLightOffset.Init(0.0f, 0.0f, 0.0f);

	bCreateSparks		= LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FIREOBJFX::Init
//
//	PURPOSE:	Build the smoke fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL FIREOBJFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_FIREOBJFX_NAME, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_FIREOBJFX_INNERFX, CString("") );
	if (!str.IsEmpty())
	{
		SMOKEFX *pSmokeFX = g_pFXButeMgr->GetSmokeFX(str.GetBuffer(0));

		if(pSmokeFX)
			nInnerFireFX = pSmokeFX->nId;
	}

	str = buteMgr.GetString(aTagName, FXBMGR_FIREOBJFX_OUTERFX, CString("") );
	if (!str.IsEmpty())
	{
		SMOKEFX *pSmokeFX = g_pFXButeMgr->GetSmokeFX(str.GetBuffer(0));

		if(pSmokeFX)
			nOuterFireFX = pSmokeFX->nId;
	}

	str = buteMgr.GetString(aTagName, FXBMGR_FIREOBJFX_SMOKEFX, CString("") );
	if (!str.IsEmpty())
	{
		SMOKEFX *pSmokeFX = g_pFXButeMgr->GetSmokeFX(str.GetBuffer(0));

		if(pSmokeFX)
			nSmokeFX = pSmokeFX->nId;
	}

	str = buteMgr.GetString(aTagName, FXBMGR_FIREOBJFX_SOUNDFX, CString("") );
	if (!str.IsEmpty())
	{
		SOUNDFX *pSoundFX = g_pFXButeMgr->GetSoundFX(str.GetBuffer(0));

		if(pSoundFX)
			nSoundFX = pSoundFX->nId;
	}

	fSizeRadiusScale	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_FIREOBJFX_SCALERADIUS, fSizeRadiusScale);

	bCreateLight		= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_FIREOBJFX_CREATELIGHT, bCreateLight);

	CAVector vTemp;

	vTemp = ToCAVector(vLightColor);
	vLightColor			= buteMgr.GetVector(aTagName, FXBMGR_FIREOBJFX_LIGHTCOLOR, vTemp);

	fLightRadius		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_FIREOBJFX_LIGHTRADIUS, fLightRadius);
	fLightPhase			= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_FIREOBJFX_LIGHTPHASE, fLightPhase);
	fLightFreq			= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_FIREOBJFX_LIGHTFREQ, fLightFreq);

	vTemp = ToCAVector(vLightOffset);
	vLightOffset		= buteMgr.GetVector(aTagName, FXBMGR_FIREOBJFX_LIGHTOFFSET, vTemp);

	bCreateSparks		= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_FIREOBJFX_CREATESPARKS, bCreateSparks);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FIREOBJFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the smoke fx struct
//
// ----------------------------------------------------------------------- //

void FIREOBJFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

#endif
}



/////////////////////////////////////////////////////////////////////////////
//
//  D R I P P E R O B J  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DRIPPEROBJFX::DRIPPEROBJFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DRIPPEROBJFX::DRIPPEROBJFX()
{
	nId					= FXBMGR_INVALID_ID;
	szName[0]			= '\0';

	vMinColor.Init();
	vMaxColor.Init();
	fAlpha = 1.0f;
	bAdditive = LTFALSE;
	bMultiply = LTFALSE;
	fMinSize = 0.0f;
	fMaxSize = 0.0f;

	nMinDripAmount = 0;
	nMaxDripAmount = 0;
	fMinDripFrequency = 0.0f;
	fMaxDripFrequency = 0.0f;
	fMinDripTime = 0.0f;
	fMaxDripTime = 0.0f;
	fDripDistance = 0.0f;
	fDripSpeed = 0.0f;
	fAccelerateTime = 0.0f;

	fDripTrailLength = 0.0f;
	fDripTrailDensity = 0.0f;

	fWiggleAmount = 0.0f;
	fWiggleFrequency = 0.0f;

	bCylinderShape = LTFALSE;

	fImpactFXPercent = 0.0f;
	nImpactFX = 0;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DRIPPEROBJFX::Init
//
//	PURPOSE:	Build the smoke fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL DRIPPEROBJFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_DRIPPEROBJFX_NAME, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_DRIPPEROBJFX_PARTICLE, CString("") );
	if (!str.IsEmpty())
	{
		strncpy(szFile, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	CAVector vTemp;

	vTemp = ToCAVector(vMinColor);
	vMinColor			= buteMgr.GetVector(aTagName, FXBMGR_DRIPPEROBJFX_MINCOLOR, vTemp);

	vTemp = ToCAVector(vMaxColor);
	vMaxColor			= buteMgr.GetVector(aTagName, FXBMGR_DRIPPEROBJFX_MAXCOLOR, vTemp);

	fAlpha				= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DRIPPEROBJFX_ALPHA, fAlpha);
	bAdditive			= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_DRIPPEROBJFX_ADDITIVE, bAdditive);
	bMultiply			= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_DRIPPEROBJFX_MULTIPLY, bMultiply);
	fMinSize			= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DRIPPEROBJFX_MINSIZE, fMinSize);
	fMaxSize			= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DRIPPEROBJFX_MAXSIZE, fMaxSize);

	nMinDripAmount		= buteMgr.GetInt(aTagName, FXBMGR_DRIPPEROBJFX_MINAMOUNT, nMinDripAmount);
	nMaxDripAmount		= buteMgr.GetInt(aTagName, FXBMGR_DRIPPEROBJFX_MAXAMOUNT, nMaxDripAmount);
	fMinDripFrequency	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DRIPPEROBJFX_MINFREQUENCY, fMinDripFrequency);
	fMaxDripFrequency	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DRIPPEROBJFX_MAXFREQUENCY, fMaxDripFrequency);
	fMinDripTime		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DRIPPEROBJFX_MINTIME, fMinDripTime);
	fMaxDripTime		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DRIPPEROBJFX_MAXTIME, fMaxDripTime);
	fDripDistance		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DRIPPEROBJFX_DISTANCE, fDripDistance);
	fDripSpeed			= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DRIPPEROBJFX_SPEED, fDripSpeed);
	fAccelerateTime		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DRIPPEROBJFX_ACCELTIME, fAccelerateTime);

	fDripTrailLength	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DRIPPEROBJFX_TLENGTH, fDripTrailLength);
	fDripTrailDensity	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DRIPPEROBJFX_TDENSITY, fDripTrailDensity);

	fWiggleAmount		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DRIPPEROBJFX_WAMOUNT, fWiggleAmount);
	fWiggleFrequency	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DRIPPEROBJFX_WFREQUENCY, fWiggleFrequency);

	bCylinderShape		= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_DRIPPEROBJFX_CYLINDER, bCylinderShape);

	fImpactFXPercent	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DRIPPEROBJFX_PERCENT, fImpactFXPercent);

	str = buteMgr.GetString(aTagName, FXBMGR_DRIPPEROBJFX_IMPACTFX, CString("") );
	if (!str.IsEmpty())
	{
		IMPACTFX *pImpactFX = g_pFXButeMgr->GetImpactFX(str.GetBuffer(0));

		if(pImpactFX)
			nImpactFX = pImpactFX->nId;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DRIPPEROBJFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the smoke fx struct
//
// ----------------------------------------------------------------------- //

void DRIPPEROBJFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

#endif
}
















/////////////////////////////////////////////////////////////////////////////
//
//	C L I E N T - S I D E  U T I L I T Y  F U N C T I O N S
//
/////////////////////////////////////////////////////////////////////////////

#ifdef _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreateScaleFX()
//
//	PURPOSE:	Create a scale fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreateScaleFX(CScaleFX* pScaleFX, LTVector vPos,
                                     LTVector vDir, LTVector* pvSurfaceNormal,
                                     LTRotation* prRot, CBaseScaleFX* pFX)
{
    if (!pScaleFX || !pScaleFX->szFile[0]) return LTNULL;

	// Create scale fx...

	BSCREATESTRUCT scale;
	scale.dwFlags = FLAG_VISIBLE | FLAG_NOLIGHT; // | FLAG_NOGLOBALLIGHTSCALE;

	if (prRot)
	{
		scale.rRot = *prRot;
	}

	// Set up fx flags...

	if (pScaleFX->eType == SCALEFX_SPRITE)
	{
		if (pScaleFX->bAlignToSurface && pvSurfaceNormal)
		{
			scale.dwFlags |= FLAG_ROTATEABLESPRITE;
            g_pLTClient->AlignRotation(&(scale.rRot), pvSurfaceNormal, LTNULL);
		}
		else if (!pScaleFX->bReallyClose)
		{
			// We want FLAG_REALLYCLOSE to override the default
			// FLAG_SPRITEBIAS...

			scale.dwFlags |= FLAG_SPRITEBIAS;
		}

		if (pScaleFX->bNoZ)
		{
			scale.dwFlags |= FLAG_SPRITE_NOZ;
		}

		if (pScaleFX->bRotate)
		{
			scale.dwFlags |= FLAG_ROTATEABLESPRITE;
		}
	}

	if (pScaleFX->bReallyClose)
	{
		scale.dwFlags |= FLAG_REALLYCLOSE;

		// Get the position into camera-relative space...

		if (g_pInterfaceMgr->UseInterfaceCamera())
		{
			HOBJECT hCamera = g_pGameClientShell->GetInterfaceCamera();
			LTVector vOffset;
			hCamera = g_pGameClientShell->GetInterfaceCamera();
			g_pLTClient->GetObjectPos(hCamera, &vOffset);

			vPos -= vOffset;
		}
		else
		{
			LTVector vOffset;
			HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
			g_pGameClientShell->GetCameraMgr()->GetCameraPos(hCamera, vOffset);

			vPos -= vOffset;
		}
	}

	// Adjust the position based on the offsets...

	scale.vPos = vPos + (vDir * pScaleFX->fDirOffset);

	if (pScaleFX->fDirROffset || pScaleFX->fDirUOffset)
	{
        LTRotation rTempRot;
		rTempRot.Init();
        LTVector vUp(0, 1, 0), vR, vF, vU;
        g_pLTClient->AlignRotation(&rTempRot, &vDir, &vUp);
        g_pLTClient->GetRotationVectors(&rTempRot, &vU, &vR, &vF);

		scale.vPos += (vR * pScaleFX->fDirROffset);
		scale.vPos += (vU * pScaleFX->fDirUOffset);
	}

	scale.Filename			= pScaleFX->szFile;
	scale.Skins[0]			= pScaleFX->szSkin;
	scale.vVel				= pScaleFX->vVel;
	scale.vInitialScale		= pScaleFX->vInitialScale;
	scale.vFinalScale		= pScaleFX->vFinalScale;
	scale.vInitialColor		= pScaleFX->vInitialColor;
	scale.vFinalColor		= pScaleFX->vFinalColor;
	scale.bUseUserColors	= pScaleFX->bUseColors;
	scale.fLifeTime			= pScaleFX->fLifeTime;
	scale.fInitialAlpha		= pScaleFX->fInitialAlpha;
	scale.fFinalAlpha		= pScaleFX->fFinalAlpha;
	scale.bLoop				= pScaleFX->bLoop;
	scale.fDelayTime		= pScaleFX->fDelayTime;
	scale.bAdditive			= pScaleFX->bAdditive;
	scale.bMultiply			= pScaleFX->bMultiply;
	scale.nType				= (pScaleFX->eType == SCALEFX_MODEL) ? OT_MODEL : OT_SPRITE;
    scale.bUseUserColors    = LTTRUE;
	scale.bFaceCamera		= pScaleFX->bFaceCamera;
	scale.nRotationAxis		= pScaleFX->nRotationAxis;
	scale.bRotate			= pScaleFX->bRotate;
	scale.fMinRotateVel		= pScaleFX->fMinRotVel;
	scale.fMaxRotateVel		= pScaleFX->fMaxRotVel;

	if (!pFX)
	{
		CSpecialFX* pNewFX = g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_SCALE_ID, &scale);
		if (pNewFX)
		{
			pNewFX->Update();
		}

		return pNewFX;
	}
	else
	{
		pFX->Init(&scale);
        pFX->CreateObject(g_pLTClient);
	}

	return pFX;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreatePShowerFX()
//
//	PURPOSE:	Create a particle shower fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreatePShowerFX(CPShowerFX* pPShowerFX, LTVector vPos,
                                        LTVector vDir, LTVector vSurfaceNormal)
{
    if (!pPShowerFX || !pPShowerFX->szTexture[0]) return LTNULL;

	// Create particle shower fx...

	PARTICLESHOWERCREATESTRUCT ps;

	ps.vPos				= vPos + (vDir * pPShowerFX->fDirOffset);
	ps.vDir				= (vSurfaceNormal * GetRandom(pPShowerFX->fMinVel, pPShowerFX->fMaxVel));
	ps.vColor1			= pPShowerFX->vColor1;
	ps.vColor2			= pPShowerFX->vColor2;
	ps.pTexture			= pPShowerFX->szTexture;
	ps.nParticles		= GetRandom(pPShowerFX->nMinParticles, pPShowerFX->nMaxParticles);
	ps.fMinSize			= pPShowerFX->fMinSize;
	ps.fMaxSize			= pPShowerFX->fMaxSize;
	ps.fDuration		= GetRandom(pPShowerFX->fMinDuration, pPShowerFX->fMaxDuration);
	ps.fEmissionRadius	= pPShowerFX->fEmissionRadius;
	ps.fRadius			= pPShowerFX->fRadius;
	ps.fGravity			= pPShowerFX->fGravity;
	ps.bAdditive		= pPShowerFX->bAdditive;
	ps.bMultiply		= pPShowerFX->bMultiply;
	ps.fFadeTime		= pPShowerFX->fFadeTime;
	ps.bRemoveIfNotLiquid = pPShowerFX->bRemoveIfNotLiquid;

	return g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_PARTICLESHOWER_ID, &ps);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreatePolyDebrisFX()
//
//	PURPOSE:	Create a poly debris fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreatePolyDebrisFX(CPolyDebrisFX* pPolyDebrisFX, LTVector vPos,
                                          LTVector vDir, LTVector vSurfaceNormal)
{
    if (!pPolyDebrisFX) return LTNULL;

	POLYDEBRISCREATESTRUCT pdebris;

	pdebris.vNormal			= vSurfaceNormal;
	pdebris.vPos			= vPos;
	pdebris.vDir			= vDir;
	pdebris.PolyDebrisFX	= *pPolyDebrisFX;

	return g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_POLYDEBRIS_ID, &pdebris);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreateBeamFX()
//
//	PURPOSE:	Create a beam fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreateBeamFX(BEAMFX* pBeamFX, LTVector vStartPos,
                                     LTVector vEndPos)
{
    if (!pBeamFX) return LTNULL;

	BEAMCREATESTRUCT beam;

	beam.vStartPos		= vStartPos;
	beam.vEndPos		= vEndPos;
	beam.pBeamFX		= pBeamFX;

	return g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_BEAM_ID, &beam);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreateSoundFX()
//
//	PURPOSE:	Create a sound fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreateSoundFX(SOUNDFX* pSoundFX, LTVector vPos,
                                     CSoundFX* pFX)
{
    if (!pSoundFX) return LTNULL;

	SNDCREATESTRUCT snd;

	snd.bLocal		= vPos.Equals(LTVector(0,0,0)) ? LTTRUE : LTFALSE;
	snd.bLoop		= pSoundFX->bLoop;
	snd.fPitchShift	= pSoundFX->fPitchShift;
	snd.fRadius		= pSoundFX->fRadius;
	snd.pSndName	= pSoundFX->szFile;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr) return LTNULL;

	if (pFX)
	{
		pFX->Init(&snd);
        pFX->CreateObject(g_pLTClient);

		return pFX;
	}

	return psfxMgr->CreateSFX(SFX_SOUND_ID, &snd);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreatePusherFX()
//
//	PURPOSE:	Create a pusher fx
//
// ----------------------------------------------------------------------- //

void CFXButeMgr::CreatePusherFX(PUSHERFX* pPusherFX, LTVector vPos)
{
    if (!pPusherFX) return;

//	g_pGameClientShell->GetMoveMgr()->AddPusher(vPos, pPusherFX->fRadius,
//		pPusherFX->fStartDelay, pPusherFX->fDuration, pPusherFX->fStrength);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreatePExplFX()
//
//	PURPOSE:	Create a paritlce explosion specific fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreatePExplFX(PEXPLFX* pPExplFX, LTRotation rSurfaceRot,
                                     SurfaceType eSurfaceType, LTVector vPos,
									 ContainerCode eCode)
{
    if (!pPExplFX) return LTNULL;

	// Create a particle explosion...

	PESCREATESTRUCT pe;
    pe.rSurfaceRot = rSurfaceRot;

	pe.nSurfaceType		= eSurfaceType;
	pe.vPos				= vPos + pPExplFX->vPosOffset;
	pe.bCreateDebris	= pPExplFX->bCreateDebris;
	pe.bRotateDebris	= pPExplFX->bRotateDebris;
	pe.bIgnoreWind		= pPExplFX->bIgnoreWind;
	pe.vColor1			= pPExplFX->vColor1;
	pe.vColor2			= pPExplFX->vColor2;
	pe.vMinVel			= pPExplFX->vMinVel;
	pe.vMaxVel			= pPExplFX->vMaxVel;
	pe.vMinDriftVel		= pPExplFX->vMinDriftVel;
	pe.vMaxDriftVel		= pPExplFX->vMaxDriftVel;
	pe.fLifeTime		= pPExplFX->fLifeTime;
	pe.fFadeTime		= pPExplFX->fFadeTime;
	pe.fOffsetTime		= pPExplFX->fOffsetTime;
	pe.fRadius			= pPExplFX->fRadius;
	pe.fGravity			= pPExplFX->fGravity;
	pe.nNumPerPuff		= pPExplFX->nNumPerPuff;
	pe.nNumEmitters		= pPExplFX->nNumEmitters;
	pe.nNumSteps		= pPExplFX->nNumSteps;
	pe.pFilename		= pPExplFX->szFile;
	pe.bAdditive		= pPExplFX->bAdditive;
	pe.bMultiply		= pPExplFX->bMultiply;

	if (IsLiquid(eCode) && pPExplFX->bDoBubbles)
	{
		GetLiquidColorRange(eCode, &pe.vColor1, &pe.vColor2);
		pe.pFilename = DEFAULT_BUBBLE_TEXTURE;
	}

	CSpecialFX* pFX = g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_PARTICLEEXPLOSION_ID, &pe);
	if (pFX) pFX->Update();

	return pFX;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreateDLightFX()
//
//	PURPOSE:	Create a dynamic light specific fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreateDLightFX(DLIGHTFX* pDLightFX, LTVector vPos,
									   CDynamicLightFX* pFX)
{
    if (!pDLightFX) return LTNULL;

	DLCREATESTRUCT dl;
	dl.vPos			 = vPos;
	dl.vColor		 = pDLightFX->vColor;
	dl.fMinRadius    = pDLightFX->fMinRadius;
	dl.fMaxRadius	 = pDLightFX->fMaxRadius;
	dl.fRampUpTime	 = pDLightFX->fRampUpTime;
	dl.fMaxTime		 = pDLightFX->fMaxTime;
	dl.fMinTime		 = pDLightFX->fMinTime;
	dl.fRampDownTime = pDLightFX->fRampDownTime;
	dl.dwFlags		 = FLAG_VISIBLE | FLAG_DONTLIGHTBACKFACING;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr) return LTNULL;

	if (pFX)
	{
		pFX->Init(&dl);
        pFX->CreateObject(g_pLTClient);

		return pFX;
	}

	return psfxMgr->CreateSFX(SFX_DYNAMICLIGHT_ID, &dl);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreateImpactFX()
//
//	PURPOSE:	Create the specified impact fx
//
// ----------------------------------------------------------------------- //

void CFXButeMgr::CreateImpactFX(IMPACTFX* pImpactFX, IFXCS & cs)
{
	// Sanity checks...

	if (!pImpactFX) return;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	if (!pSettings) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;


	// Get the impact FX detail setting
	int nDiv = 3 - GetConsoleInt("ImpactFXLevel", 2);
	int nNum = 1;


	// Create any debris fx...

	nNum = pImpactFX->nNumDebrisFXTypes / nDiv;
	if(pImpactFX->nNumDebrisFXTypes && nNum == 0) nNum = 1;

	for (int i=0; i < nNum; i++)
	{
		DEBRISCREATESTRUCT debris;
		debris.rRot			= cs.rSurfRot;
		debris.vPos			= cs.vPos;
		debris.nDebrisId	= pImpactFX->aDebrisFXTypes[i];

		psfxMgr->CreateSFX(SFX_DEBRIS_ID, &debris);
	}

	// Create any scale fx...

	nNum = pImpactFX->nNumScaleFXTypes / nDiv;
	if(pImpactFX->nNumScaleFXTypes && nNum == 0) nNum = 1;

	for (i=0; i < nNum; i++)
	{
		CScaleFX* pScaleFX = GetScaleFX(pImpactFX->aScaleFXTypes[i]);
		if (pScaleFX)
		{
			CreateScaleFX(pScaleFX, cs.vPos, cs.vDir, &(cs.vSurfNormal), &(cs.rSurfRot));
		}
	}

	// Create the particle explosion fx...

	nNum = pImpactFX->nNumPExplFX / nDiv;
	if(pImpactFX->nNumPExplFX && nNum == 0 && nDiv < 3) nNum = 1;

	for (i=0; i < nNum; i++)
	{
		PEXPLFX* pPExplFX = GetPExplFX(pImpactFX->aPExplFXTypes[i]);
		if (pPExplFX)
		{
			CreatePExplFX(pPExplFX, cs.rSurfRot, cs.eSurfType, cs.vPos, cs.eCode);
		}
	}


	// Create the poly debris fx...

	nNum = pImpactFX->nNumPolyDebrisFX / nDiv;
	if(pImpactFX->nNumPolyDebrisFX && nNum == 0 && nDiv < 3) nNum = 1;

	for (i=0; i < nNum; i++)
	{
		CPolyDebrisFX* pPolyDebrisFX = GetPolyDebrisFX(pImpactFX->aPolyDebrisFXTypes[i]);
		if (pPolyDebrisFX)
		{
			CreatePolyDebrisFX(pPolyDebrisFX, cs.vPos, cs.vDir, cs.vSurfNormal);
		}
	}


	// Create the pshower fx...

	nNum = pImpactFX->nNumPShowerFX / nDiv;
	if(pImpactFX->nNumPShowerFX && nNum == 0 && nDiv < 3) nNum = 1;

	for (i=0; i < nNum; i++)
	{
		CPShowerFX* pPShowerFX = GetPShowerFX(pImpactFX->aPShowerFXTypes[i]);
		if (pPShowerFX)
		{
			CreatePShowerFX(pPShowerFX, cs.vPos, cs.vDir, cs.vSurfNormal);
		}
	}


	// Create the dynamic light fx...

	nNum = pImpactFX->nNumDLightFX / nDiv;
	if(pImpactFX->nNumDLightFX && nNum == 0 && nDiv < 3) nNum = 1;

	for (i=0; i < nNum; i++)
	{
		DLIGHTFX* pDLightFX = GetDLightFX(pImpactFX->aDLightFXTypes[i]);
		if (pDLightFX)
		{
			CreateDLightFX(pDLightFX, cs.vPos);
		}
	}


	// Play the impact sound if appropriate...

	if (cs.bPlaySound && (pImpactFX->szSound[0] || (pImpactFX->nSoundBute != -1)))
	{
		if(pImpactFX->nSoundBute != -1)
		{
			g_pClientSoundMgr->PlaySoundFromPos(cs.vPos, pImpactFX->nSoundBute);
		}
		else
		{
			LTFLOAT fSndRadius = (LTFLOAT) pImpactFX->nSoundRadius;

			static char szImpactSnd[255];

			szImpactSnd[0] = 0;

			if (_stricmp(pImpactFX->szSound, "SURFACE") == 0)
			{
				SURFACE* pSurf = g_pSurfaceMgr->GetSurface(cs.eSurfType);
				_ASSERT(pSurf);
				if (pSurf) 
				{
					sprintf(szImpactSnd, pSurf->szBulletImpactSnds[GetRandom(0, SRF_MAX_IMPACT_SNDS-1)], pImpactFX->szSoundDir);
				}
			}
			else
			{
				SAFE_STRCPY(szImpactSnd, pImpactFX->szSound);
			}


			if( szImpactSnd[0] )
			{
				g_pClientSoundMgr->PlaySoundFromPos(cs.vPos, szImpactSnd, fSndRadius,
						SOUNDPRIORITY_MISC_MEDIUM);
			}
		}
	}


	// Tint screen if appropriate...

	if ((pImpactFX->nFlags & WFX_TINTSCREEN))
	{
        LTVector vTintColor  = pImpactFX->vTintColor;
        LTFLOAT fRampUp      = pImpactFX->fTintRampUp;
        LTFLOAT fRampDown    = pImpactFX->fTintRampDown;
        LTFLOAT fTintTime    = pImpactFX->fTintMaxTime;

		LTFLOAT fTRadius = (pImpactFX->fTintRadius == 0.0f) ? cs.fTintRange : pImpactFX->fTintRadius;

		g_pGameClientShell->FlashScreen(vTintColor, cs.vPos, fTintTime, fRampUp, fRampDown, fTRadius);

		// If close enough, shake the screen...

		HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (hPlayerObj)
		{
            LTVector vPlayerPos, vDir;
            g_pLTClient->GetObjectPos(hPlayerObj, &vPlayerPos);

			vDir = vPlayerPos - cs.vPos;
            LTFLOAT fDist = vDir.Mag();

            LTFLOAT fRadius = fTRadius / 2;

			if (fDist < fRadius * 2.0f)
			{
                LTFLOAT fVal = fDist < 1.0f ? 3.0f : fRadius / fDist;
				fVal = fVal > 3.0f ? 3.0f : fVal;

                LTVector vShake(fVal, fVal, fVal);
				g_pGameClientShell->ShakeScreen(vShake);
			}
		}
	}

	// Add a pusher if necessary...

	CreatePusherFX(pImpactFX->pPusherFX, cs.vPos);
}

#else // _CLIENTBUILD

/////////////////////////////////////////////////////////////////////////////
//
//	S E R V E R - S I D E  U T I L I T Y  F U N C T I O N S
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::ReadImpactFXProp
//
//	PURPOSE:	Read in the impact fx properties
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgr::ReadImpactFXProp(char* pPropName, uint8 & nImpactFXId)
{
    if (!pPropName || !pPropName[0]) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric(pPropName, &genProp) == LT_OK)
	{
		// Get the impact fx

		IMPACTFX* pImpactFX = GetImpactFX(genProp.m_String);
		if (pImpactFX)
		{
			nImpactFXId = pImpactFX->nId;
		}

        return LTTRUE;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::ReadSoundFXProp
//
//	PURPOSE:	Read in the smoke fx properties
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgr::ReadSoundFXProp(char* pPropName, uint8 & nSoundFXId)
{
    if (!pPropName || !pPropName[0]) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric(pPropName, &genProp) == LT_OK)
	{
		// Get the impact fx

		SOUNDFX* pSoundFX = GetSoundFX(genProp.m_String);
		if (pSoundFX)
		{
			nSoundFXId = pSoundFX->nId;
		}

        return LTTRUE;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::ReadSmokeFXProp
//
//	PURPOSE:	Read in the smoke fx properties
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgr::ReadSmokeFXProp(char* pPropName, uint8 & nSmokeFXId)
{
    if (!pPropName || !pPropName[0]) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric(pPropName, &genProp) == LT_OK)
	{
		// Get the impact fx

		SMOKEFX* pSmokeFX = GetSmokeFX(genProp.m_String);
		if (pSmokeFX)
		{
			nSmokeFXId = pSmokeFX->nId;
		}

        return LTTRUE;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::ReadSteamFXProp
//
//	PURPOSE:	Read in the steam fx properties
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgr::ReadSteamFXProp(char* pPropName, uint8 & nSteamFXId)
{
    if (!pPropName || !pPropName[0]) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric(pPropName, &genProp) == LT_OK)
	{
		// Get the impact fx

		STEAMFX* pSteamFX = GetSteamFX(genProp.m_String);
		if (pSteamFX)
		{
			nSteamFXId = pSteamFX->nId;
		}

        return LTTRUE;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::ReadFireObjFXProp
//
//	PURPOSE:	Read in the fire obj fx properties
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgr::ReadFireObjFXProp(char* pPropName, uint8 & nFireObjFXId)
{
    if (!pPropName || !pPropName[0]) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric(pPropName, &genProp) == LT_OK)
	{
		// Get the impact fx

		FIREOBJFX* pFireObjFX = GetFireObjFX(genProp.m_String);
		if (pFireObjFX)
		{
			nFireObjFXId = pFireObjFX->nId;
		}

        return LTTRUE;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::ReadDripperObjFXProp
//
//	PURPOSE:	Read in the dripper obj fx properties
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgr::ReadDripperObjFXProp(char* pPropName, uint8 & nDripperObjFXId)
{
    if (!pPropName || !pPropName[0]) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric(pPropName, &genProp) == LT_OK)
	{
		// Get the impact fx

		DRIPPEROBJFX* pDripperObjFX = GetDripperObjFX(genProp.m_String);
		if (pDripperObjFX)
		{
			nDripperObjFXId = pDripperObjFX->nId;
		}

        return LTTRUE;
	}

    return LTFALSE;
}

////////////////////////////////////////////////////////////////////////////
//
// CFXButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use FXButeMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CFXButeMgrPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char* const * aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (!g_pFXButeMgr)
	{
		// Make sure debris mgr is inited...

		m_DebrisMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings,	pcStrings, cMaxStrings, cMaxStringLength);

		// Make sure sound mgr is inited...

		m_SoundButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings,	pcStrings, cMaxStrings, cMaxStringLength);

		// This will set the g_pFXButeMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];
#ifdef _WIN32
		sprintf(szFile, "%s\\%s", szRezPath, FXBMGR_DEFAULT_FILE);
#else
		sprintf(szFile, "%s/%s", szRezPath, FXBMGR_DEFAULT_FILE);
#endif
        sm_FXButeMgr.SetInRezFile(LTFALSE);
        sm_FXButeMgr.Init(g_pLTServer, szFile);
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgrPlugin::PopulateImpactFXStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgrPlugin::PopulateImpactFXStringList(char* const * aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
    if (!aszStrings || !pcStrings) return LTFALSE;
	_ASSERT(aszStrings && pcStrings);

	// Add an entry for each impact fx

	int nNumImpactFX = g_pFXButeMgr->GetNumImpactFX();
	_ASSERT(nNumImpactFX > 0);

    IMPACTFX* pImpactFX = LTNULL;

	for (int i=0; i < nNumImpactFX; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pImpactFX = g_pFXButeMgr->GetImpactFX(i);
        uint32 dwImpactFXNameLen = strlen(pImpactFX->szName);

		if (pImpactFX && pImpactFX->szName[0] &&
			dwImpactFXNameLen < cMaxStringLength &&
			((*pcStrings) + 1) < cMaxStrings)
		{
			strcpy(aszStrings[(*pcStrings)++], pImpactFX->szName);
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgrPlugin::PopulateSmokeFXStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgrPlugin::PopulateSmokeFXStringList(char* const * aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
    if (!aszStrings || !pcStrings) return LTFALSE;
	_ASSERT(aszStrings && pcStrings);

	// Add an entry for each smoke fx

	int nNumSmokeFX = g_pFXButeMgr->GetNumSmokeFX();
	_ASSERT(nNumSmokeFX > 0);

    SMOKEFX* pSmokeFX = LTNULL;

	for (int i=0; i < nNumSmokeFX; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pSmokeFX = g_pFXButeMgr->GetSmokeFX(i);
        uint32 dwSmokeFXNameLen = strlen(pSmokeFX->szName);

		if (pSmokeFX && pSmokeFX->szName[0] &&
			dwSmokeFXNameLen < cMaxStringLength &&
			((*pcStrings) + 1) < cMaxStrings)
		{
			strcpy(aszStrings[(*pcStrings)++], pSmokeFX->szName);
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgrPlugin::PopulateSteamFXStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgrPlugin::PopulateSteamFXStringList(char* const * aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
    if (!aszStrings || !pcStrings) return LTFALSE;
	_ASSERT(aszStrings && pcStrings);

	// Add an entry for each smoke fx

	int nNumSteamFX = g_pFXButeMgr->GetNumSteamFX();
	_ASSERT(nNumSteamFX > 0);

    STEAMFX* pSteamFX = LTNULL;

	for (int i=0; i < nNumSteamFX; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pSteamFX = g_pFXButeMgr->GetSteamFX(i);
        uint32 dwSteamFXNameLen = strlen(pSteamFX->szName);

		if (pSteamFX && pSteamFX->szName[0] &&
			dwSteamFXNameLen < cMaxStringLength &&
			((*pcStrings) + 1) < cMaxStrings)
		{
			strcpy(aszStrings[(*pcStrings)++], pSteamFX->szName);
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgrPlugin::PopulateFireObjFXStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgrPlugin::PopulateFireFXStringList(char* const * aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
    if (!aszStrings || !pcStrings) return LTFALSE;
	_ASSERT(aszStrings && pcStrings);

	// Add an entry for each smoke fx

	int nNumFireObjFX = g_pFXButeMgr->GetNumFireObjFX();
	_ASSERT(nNumFireObjFX > 0);

    FIREOBJFX* pFireObjFX = LTNULL;

	for (int i=0; i < nNumFireObjFX; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pFireObjFX = g_pFXButeMgr->GetFireObjFX(i);
        uint32 dwFireObjFXNameLen = strlen(pFireObjFX->szName);

		if (pFireObjFX && pFireObjFX->szName[0] &&
			dwFireObjFXNameLen < cMaxStringLength &&
			((*pcStrings) + 1) < cMaxStrings)
		{
			strcpy(aszStrings[(*pcStrings)++], pFireObjFX->szName);
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgrPlugin::PopulateDripperFXStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgrPlugin::PopulateDripperFXStringList(char* const * aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
    if (!aszStrings || !pcStrings) return LTFALSE;
	_ASSERT(aszStrings && pcStrings);

	// Add an entry for each smoke fx

	int nNumDripperObjFX = g_pFXButeMgr->GetNumDripperObjFX();
	_ASSERT(nNumDripperObjFX > 0);

    DRIPPEROBJFX* pDripperObjFX = LTNULL;

	for (int i=0; i < nNumDripperObjFX; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pDripperObjFX = g_pFXButeMgr->GetDripperObjFX(i);
        uint32 dwDripperObjFXNameLen = strlen(pDripperObjFX->szName);

		if (pDripperObjFX && pDripperObjFX->szName[0] &&
			dwDripperObjFXNameLen < cMaxStringLength &&
			((*pcStrings) + 1) < cMaxStrings)
		{
			strcpy(aszStrings[(*pcStrings)++], pDripperObjFX->szName);
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgrPlugin::PopulateSoundFXStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgrPlugin::PopulateSoundFXStringList(char* const * aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
    if (!aszStrings || !pcStrings) return LTFALSE;
	_ASSERT(aszStrings && pcStrings);

	// Add an entry for each smoke fx

	int nNumSoundFX = g_pFXButeMgr->GetNumSoundFX();
	_ASSERT(nNumSoundFX > 0);

    SOUNDFX* pSoundFX = LTNULL;

	for (int i=0; i < nNumSoundFX; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pSoundFX = g_pFXButeMgr->GetSoundFX(i);
        uint32 dwSoundFXNameLen = strlen(pSoundFX->szName);

		if (pSoundFX && pSoundFX->szName[0] &&
			dwSoundFXNameLen < cMaxStringLength &&
			((*pcStrings) + 1) < cMaxStrings)
		{
			strcpy(aszStrings[(*pcStrings)++], pSoundFX->szName);
		}
	}

    return LTTRUE;
}

#endif // _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
// MODULE  : Striker.cpp
//
// PURPOSE : Implementation of the Striker
//
// CREATED : 11.13.2000
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Striker.h"
#include "cpp_server_de.h"
#include "ServerUtilities.h"
#include "ServerSoundMgr.h"
#include "SoundButeMgr.h"
#include "CharacterMgr.h"
#include "ObjectMsgs.h"
#include "PlayerObj.h"
#include "Character.h"
#include "CharacterHitBox.h"
#include "ServerUtilities.h"
#include "AIUtils.h"

#include <algorithm>

// Statics

// These two are copied from CharacterFX.cpp
#define FOOTSTEP_SOUND_RADIUS	1000.0f
#define KEY_FOOTSTEP_SOUND		"FOOTSTEP_KEY"

static char *s_szActivate		= "ACTIVATE";
const LTFLOAT c_fStrikerUpdateDelta = 0.1f;

static ILTMessage & operator<<(ILTMessage & out, Striker::IdleAction & action )
{
	ASSERT( uint32(action) < 256 );
	out.WriteByte(action);
	return out;
}

static ILTMessage & operator>>(ILTMessage & in, Striker::IdleAction & action )
{
	action = Striker::IdleAction(in.ReadByte());
	return in;
}

// Idle stuff.
const int c_nNumIdleAnims = 2;
const char * aszIdleAnims[c_nNumIdleAnims] = { "Idle_0", "Idle_1" };

const LTFLOAT c_fIdleRotationRate = MATH_DEGREES_TO_RADIANS(45.0f); // in radians/sec.


// Chance that each action will be performed next.
// Be sure the chances add up to 1.0!!
const LTFLOAT s_afIdleActionChance[Striker::NumIdleActions] =
{ 0.33f, // Anim
  0.33f, // Turn
  0.33f  // Move
};

// The min time that that action will go on.
const LTFLOAT s_afIdleActionMinTime[Striker::NumIdleActions] =
{ 3.0f,  // Anim
  0.5f, // Turn
  1.0f   // Move
};

// The max time that that action will go on.
const LTFLOAT s_afIdleActionMaxTime[Striker::NumIdleActions] =
{ 7.0f,  // Anim
  1.5f, // Turn
  3.0f   // Move
};


static bool BoxIntersection( const LTVector & pos1, const LTVector & dims1, 
							 const LTVector & pos2, const LTVector & dims2  )
{
	_ASSERT( dims1.x >= 0.0f && dims1.y >= 0.0f && dims1.z >= 0.0f );
	_ASSERT( dims2.x >= 0.0f && dims2.y >= 0.0f && dims2.z >= 0.0f );

	return (   (float)fabs(pos1.x - pos2.x) < dims1.x + dims2.x 
			&& (float)fabs(pos1.y - pos2.y) < dims1.y + dims2.y  			   
			&& (float)fabs(pos1.z - pos2.z) < dims1.z + dims2.z  );
}


// Copied from CharacterFx.cpp
static void GetFootStepSound(char* szSound, int nCharacterSet, SurfaceType eSurfType, LTBOOL bLeftFoot)
{
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
	_ASSERT(pSurf);
	if (!pSurf) return;

	int nIndex = GetRandom(0, 1);

	sprintf(szSound, bLeftFoot ? pSurf->szLtFootStepSnds[nIndex]:pSurf->szRtFootStepSnds[nIndex],
				g_pCharacterButeMgr->GetFootStepSoundDir(nCharacterSet) );
				
}

// ----------------------------------------------------------------------- //
//
//	CLASS:		Striker
//
//	PURPOSE:	Blindly pursues the first player to step in range
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(Striker)

#ifdef _WIN32
	ADD_STRINGPROP_FLAG(Filename, "Models\\Props\\creature_striker.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Skins\\Props\\creature_striker.dtx", PF_FILENAME)
#else
	ADD_STRINGPROP_FLAG(Filename, "Models/Props/creature_striker.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Skins/Props/creature_striker.dtx", PF_FILENAME)
#endif

	ADD_STRINGPROP_FLAG(SurfaceOverride, "Flesh_Alien", PF_STATICLIST)

	ADD_REALPROP_FLAG_HELP( LeashLength, 1500.0f, 0, "Striker will not move farther than this length away from the leash point.  Set to zero to keep striker unleashed.")
	ADD_STRINGPROP_FLAG_HELP( LeashPoint, "", PF_OBJECTLINK, "Striker will not move farther than LeashLength away from this object. Leave blank to use striker's initial position.")

	ADD_STRINGPROP_FLAG(DebrisType, "", PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SHADOW_FLAG(0, 0)
	ADD_BOOLPROP_FLAG(MoveToFloor, LTTRUE, 0)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)

	ADD_BOOLPROP_FLAG_HELP(IgnoreAI, LTTRUE, 0,			"TRUE is activated by players ONLY")
	ADD_REALPROP_FLAG_HELP(DetectionRadius, 250.0f, 0,	"Striker attacks first thing to enter this radius")

	ADD_BOOLPROP_FLAG(PlayerUsable, LTFALSE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(ActivateTarget, LTFALSE, PF_OBJECTLINK)
	ADD_STRINGPROP(ActivateMessage, LTFALSE)

    ADD_BOOLPROP_FLAG(DetailTexture, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Chrome, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)
    ADD_CHROMAKEY_FLAG(LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Additive, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Multiply, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(RayHit, LTTRUE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(TouchSound, "", PF_FILENAME | PF_HIDDEN)
	ADD_REALPROP_FLAG(TouchSoundRadius, 500.0, PF_RADIUS | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(DetailLevel, "Low", PF_STATICLIST)

	ADD_BOOLPROP_FLAG(NeverDestroy, LTFALSE, PF_GROUP1)

END_CLASS_DEFAULT(Striker, Prop, NULL, NULL)


Striker::StrikerList Striker::sm_Strikers;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Striker::Striker()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Striker::Striker()
: m_bPlayerUsable(LTFALSE),
  m_hstrActivateMessage(LTNULL),
  m_hstrActivateTarget(LTNULL),

  m_bFirstUpdate(LTTRUE),
  
  m_fLeashLengthSqr(0),
  m_vLeashFrom(0,0,0),
  // m_strLeashFromName

  m_eIdleAction(eIdleAnim),
  // m_tmrIdleAction
  m_bIdleTurnRight(LTFALSE),


  m_fDetectionRadiusSqr(1000.0f*1000.0f),
  m_fChaseSpeed(25.0f),
  m_fDamage(10.0f),
  m_fClawRange(200.0f),
  // m_hTarget
  // cm_Movement
  m_bLeftFoot(LTTRUE),
  m_nRandomSoundBute(-1)
  // m_tmrRandomSound
{
	SetupFSM();

	m_RootFSM.SetInitialState(Root::eStateIdle);

	m_damage.SetRemoveOnDeath(LTFALSE);

	// Register ourself.
	sm_Strikers.push_back(this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Striker::~Striker()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Striker::~Striker()
{
	if(!g_pInterface)
		return;

	FREE_HSTRING(m_hstrActivateMessage);
	FREE_HSTRING(m_hstrActivateTarget);

	// Remove ourself from the registry.
	sm_Strikers.remove(this);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Striker::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Striker::EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate();

			uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);
			g_pInterface->SetNextUpdate(m_hObject, c_fStrikerUpdateDelta);

			CacheFiles();
			return dwRet;
		}

		case MID_UPDATE:
		{
			// DON'T call Prop::EngineMessageFn, object might get removed...
			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			Update();
			g_pInterface->SetNextUpdate(m_hObject, c_fStrikerUpdateDelta);
	
			return dwRet;

			break;
		}

		case MID_MODELSTRINGKEY:
		{
			HandleModelString((ArgList*)pData);
			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData);
			break;
		}

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
			break;
		}

		default: 
			break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Striker::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Striker::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) 
		return LTFALSE;

	GenericProp genProp;

	// Striker stuff
	if (g_pServerDE->GetPropGeneric("IgnoreAI", &genProp) == DE_OK)
		m_bIgnoreAI = genProp.m_Bool;

	if (g_pServerDE->GetPropGeneric("DetectionRadius", &genProp) == DE_OK)
		m_fDetectionRadiusSqr = genProp.m_Float * genProp.m_Float;


	// Prop stuff
	if ( g_pServerDE->GetPropGeneric( "ActivateMessage", &genProp ) == DE_OK )
	if ( genProp.m_String[0] )
		m_hstrActivateMessage = g_pServerDE->CreateString( genProp.m_String );

	if ( g_pServerDE->GetPropGeneric( "ActivateTarget", &genProp ) == DE_OK )
	if ( genProp.m_String[0] )
		m_hstrActivateTarget = g_pServerDE->CreateString( genProp.m_String );
	
	if ( g_pServerDE->GetPropGeneric("PlayerUsable", &genProp ) == DE_OK )
		m_bPlayerUsable = genProp.m_Bool;

	if ( g_pServerDE->GetPropGeneric("PlayerUsable", &genProp ) == DE_OK )
		m_bPlayerUsable = genProp.m_Bool;

	
	if (g_pLTServer->GetPropGeneric("LeashLength", &genProp) == DE_OK)
	{
		m_fLeashLengthSqr = genProp.m_Float*genProp.m_Float;
	}

	if (g_pLTServer->GetPropGeneric("LeashPoint", &genProp) == DE_OK)
	{
		if ( genProp.m_String[0] )
			m_strLeashFromName = genProp.m_String;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Striker::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void Striker::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) 
		return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Striker::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD Striker::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	if (!g_pServerDE) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			char* szMsg = hMsg ? g_pServerDE->GetStringData(hMsg) : LTNULL;

			if ( szMsg && !_stricmp(szMsg, s_szActivate) )
			{
				if ( m_bPlayerUsable || g_pServerDE->GetObjectClass(hSender) != g_pServerDE->GetClass("CPlayerObj") )
				{
					// We were triggered, now send our message

					if ( m_hstrActivateTarget && m_hstrActivateMessage )
					{
						SendTriggerMsgToObjects(this, m_hstrActivateTarget, m_hstrActivateMessage);
						FREE_HSTRING(m_hstrActivateMessage);
						FREE_HSTRING(m_hstrActivateTarget);
					}
				}
			}

			g_pServerDE->FreeString(hMsg);

			break;
		}

		case MID_DAMAGE:
		{
			DDWORD dwRet = Prop::ObjectMessageFn(hSender, messageID, hRead);

			if (m_damage.IsDead())
			{
				m_RootFSM.AddMessage(Root::eDead);
			}
			else
			{
				// If our damager is not another striker, kill the bastard!
				if( !dynamic_cast<Striker*>( g_pInterface->HandleToObject(m_damage.GetLastDamager()) ) )
				{
					g_pServerSoundMgr->PlayCharSndFromObject(cm_Movement.GetCharacterButes()->m_nId, m_hObject, "Pain");

					m_hTarget = m_damage.GetLastDamager();
					m_RootFSM.AddMessage(Root::eEnemyDetected);
				}
			}

			return dwRet;
			break;
		}

		default:
		{
			break;
		}
	}

	return Prop::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Striker::InitialUpdate()
//
//	PURPOSE:	Initial update
//
// ----------------------------------------------------------------------- //

LTBOOL Striker::InitialUpdate()
{
	cm_Movement.Init(g_pInterface, m_hObject);
	cm_Movement.SetCharacterButes("Striker");

	cm_Movement.SetObjectFlags( OFT_Flags,
								FLAG_STAIRSTEP | FLAG_TOUCH_NOTIFY | FLAG_SOLID |
								FLAG_GRAVITY | FLAG_MODELKEYS | FLAG_RAYHIT | FLAG_VISIBLE, LTTRUE );
	cm_Movement.SetObjectFlags( OFT_Flags2,
								FLAG2_SEMISOLID, LTTRUE );

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Striker::HandleFootstepKey
//
//	PURPOSE:	Plays the footstep sound.
//
// ----------------------------------------------------------------------- //

void Striker::HandleFootstepKey()
{
	static char szSound[255];
	szSound[0] = '\0';

	// Get the surface we're standing on.
	SurfaceType eStandingOnSurface = ST_UNKNOWN;
	if (  cm_Movement.GetCharacterVars()->m_bStandingOn )
	{
		eStandingOnSurface = GetSurfaceType(cm_Movement.GetCharacterVars()->m_ciStandingOn);
	}


	m_bLeftFoot = !m_bLeftFoot;

	GetFootStepSound(szSound, cm_Movement.GetCharacterButes()->m_nId, eStandingOnSurface, m_bLeftFoot);

	if (szSound[0] != '\0') 
	{
		uint32 dwFlags = 0;
		SoundPriority ePriority = SOUNDPRIORITY_MISC_LOW;
		int nVolume = 100;

		g_pServerSoundMgr->PlaySoundFromPos(const_cast<LTVector&>(cm_Movement.GetPos()), szSound, FOOTSTEP_SOUND_RADIUS, SOUNDPRIORITY_MISC_LOW, dwFlags, nVolume);
	}

	// now play the random movement sound
	g_pServerSoundMgr->PlayCharSndFromObject(cm_Movement.GetCharacterButes()->m_nId, m_hObject, "Move");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Striker::FirstUpdate()
//
//	PURPOSE:	First update.
//
// ----------------------------------------------------------------------- //
void Striker::FirstUpdate()
{
	if( !m_strLeashFromName.empty() )
	{
		HOBJECT hFoundObj = LTNULL;

		if( LT_OK == FindNamedObject(m_strLeashFromName.c_str(),hFoundObj) )
		{
			g_pLTServer->GetObjectPos(hFoundObj,&m_vLeashFrom);
		}
		else
		{
#ifndef _FINAL
			g_pLTServer->CPrint("Striker \"%s\" given leash from unknown object named \"%s\".",
				g_pLTServer->GetObjectName(m_hObject), m_strLeashFromName.c_str() );
			g_pLTServer->CPrint("   Using initial position as leash point.");
#endif
			g_pLTServer->GetObjectPos(m_hObject, &m_vLeashFrom);
		}
	}
	else
	{
		g_pLTServer->GetObjectPos(m_hObject, &m_vLeashFrom);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Striker::Update()
//
//	PURPOSE:	Update state machine
//
// ----------------------------------------------------------------------- //

void Striker::Update()
{
	if( m_bFirstUpdate )
	{
		FirstUpdate();
		m_bFirstUpdate = LTFALSE;
	}

	// Update our movement.  Should be done before 
	// anything else to be sure position and rotation vectors
	// are up to date.
	cm_Movement.Update(GetUpdateDelta());

	// Check for clipping.
	int nNumChecked = 0;
	int nWeightedSum = 0;
	for( StrikerList::iterator iter = sm_Strikers.begin();
	     iter != sm_Strikers.end(); ++iter )
	{
		++nNumChecked;
		if(  *iter != this 
		   && BoxIntersection( cm_Movement.GetPos(), cm_Movement.GetObjectDims(),
			                   (*iter)->cm_Movement.GetPos(), (*iter)->cm_Movement.GetObjectDims() ) )
		{
			if( (cm_Movement.GetPos() - (*iter)->cm_Movement.GetPos()).Dot(cm_Movement.GetRight()) > 0.0f )
			{
				nWeightedSum += nNumChecked;
			}
			else
			{
				nWeightedSum -= nNumChecked;
			}

		}
	}

	if( nWeightedSum == 0 )
	{
		cm_Movement.SetControlFlags( cm_Movement.GetControlFlags() & ~(CM_FLAG_STRAFERIGHT | CM_FLAG_STRAFELEFT) );
	}
	else if( nWeightedSum > 0 )
	{
		 cm_Movement.SetControlFlags( cm_Movement.GetControlFlags() | CM_FLAG_STRAFERIGHT);
	}
	else
	{
		cm_Movement.SetControlFlags( cm_Movement.GetControlFlags() | CM_FLAG_STRAFELEFT);
	}

	// Finally, do our behaviours.
	m_RootFSM.Update();

	if( m_nRandomSoundBute > 0 && m_tmrRandomSound.Stopped() )
	{
		g_pServerSoundMgr->PlayCharSndFromObject( cm_Movement.GetCharacterButes()->m_nId, 
		                                          m_hObject, 
												  g_pSoundButeMgr->GetSoundButeName(m_nRandomSoundBute) );

		const SoundButes & butes = g_pSoundButeMgr->GetSoundButes(m_nRandomSoundBute);
		m_tmrRandomSound.Start( GetRandom(butes.m_fMinDelay, butes.m_fMaxDelay) );
	}
}

void Striker::SetRandomSound(const char * szSoundName)
{
	const int nCharacterSet = cm_Movement.GetCharacterButes()->m_nId;
	CString strSoundButes = g_pServerSoundMgr->GetSoundSetName( g_pCharacterButeMgr->GetGeneralSoundDir(nCharacterSet), szSoundName );

	if( !strSoundButes.IsEmpty() )
	{
		m_nRandomSoundBute = g_pSoundButeMgr->GetSoundSetFromName(strSoundButes);

		if( m_nRandomSoundBute > 0 )
		{
			const SoundButes & butes = g_pSoundButeMgr->GetSoundButes(m_nRandomSoundBute);
			m_tmrRandomSound.Start( butes.m_fMinDelay );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Striker::Update()
//
//	PURPOSE:	Update state machine
//
// ----------------------------------------------------------------------- //

LTBOOL Striker::WithinLeash(const LTVector & vPos)
{
	if( m_fLeashLengthSqr > 0.0f && m_fLeashLengthSqr < (vPos - m_vLeashFrom).MagSqr() )
		return LTFALSE;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//   State Machine & related methods
// ----------------------------------------------------------------------- //
void Striker::SetupFSM()
{
	m_RootFSM.DefineState(Root::eStateIdle, 
						  m_RootFSM.MakeCallback(*this, &Striker::UpdateIdle),
						  m_RootFSM.MakeCallback(*this, &Striker::StartIdle));

	m_RootFSM.DefineState(Root::eStateChase,
						  m_RootFSM.MakeCallback(*this, &Striker::UpdateChase),
						  m_RootFSM.MakeCallback(*this, &Striker::StartChase));

	m_RootFSM.DefineState(Root::eStateAttack,
						  m_RootFSM.MakeCallback(*this, &Striker::UpdateAttack),
						  m_RootFSM.MakeCallback(*this, &Striker::StartAttack));

	m_RootFSM.DefineState(Root::eStateDead,
						  m_RootFSM.MakeCallback(*this, &Striker::UpdateDead),
						  m_RootFSM.MakeCallback(*this, &Striker::StartDead));

	// Default Transition
	m_RootFSM.DefineTransition(Root::eNoTarget, Root::eStateIdle);
	m_RootFSM.DefineTransition(Root::eDead, Root::eStateDead);

	// Standard Transitions
	m_RootFSM.DefineTransition(Root::eStateIdle, Root::eEnemyDetected, Root::eStateChase);
	m_RootFSM.DefineTransition(Root::eStateChase, Root::eInAttackRange, Root::eStateAttack);
	m_RootFSM.DefineTransition(Root::eStateAttack, Root::eOutOfAttackRange, Root::eStateChase);
}


void Striker::StartIdle(RootFSM * fsm)
{
	if (!m_hObject)
		return;

	SetRandomSound("RandomIdle");

	cm_Movement.SetControlFlags(CM_FLAG_NONE);
	
	m_tmrIdleAction.Stop();

	HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, const_cast<char*>(aszIdleAnims[0]) );

	g_pInterface->SetModelAnimation(m_hObject, hAnim);
	g_pInterface->SetModelLooping(m_hObject, LTTRUE);
}

void Striker::UpdateIdle(RootFSM * fsm)
{
	CCharacter *pChar;

	pChar = IsEnemyInRange(m_fDetectionRadiusSqr);
	if (pChar && WithinLeash(pChar->GetPosition()) )
	{
		m_hTarget = pChar->m_hObject;
		fsm->AddMessage(Root::eEnemyDetected);
		return;
	}

	if( !m_tmrIdleAction.Stopped() )
	{
			
		switch( m_eIdleAction )
		{
			case eIdleTurn :
			{
				LTRotation rRot = cm_Movement.GetRot();
				LTVector vUp = cm_Movement.GetUp();

				LTFLOAT fRotationAmount = c_fIdleRotationRate*c_fStrikerUpdateDelta;
				if( !m_bIdleTurnRight )
					fRotationAmount = -fRotationAmount;

				g_pMathLT->RotateAroundAxis(rRot, vUp, fRotationAmount);
				cm_Movement.SetRot(rRot);
			}
			break;

			case eIdleMove :
			{
				if( !WithinLeash(cm_Movement.GetPos()) )
				{
					m_tmrIdleAction.Stop();
				}
			}
			break;

			
			case eIdleAnim :
			default :
			{
				// do nothing.
			}
			break;

		} //switch( eIdleAction )

	}
	else // !( !m_tmrIdleAction.Stopped() )
	{

		LTFLOAT fChance = GetRandom(0.0f, 1.0f );

		int i = 0;
		for(; fChance > s_afIdleActionChance[i] && i < NumIdleActions; ++i)
		{
			fChance -= s_afIdleActionChance[i];
		}

		m_tmrIdleAction.Start( GetRandom( s_afIdleActionMinTime[i], s_afIdleActionMaxTime[i] ) );

		m_eIdleAction = IdleAction(i);
		switch( m_eIdleAction )
		{
			case eIdleMove :
			{
				cm_Movement.SetControlFlags(CM_FLAG_FORWARD);
				cm_Movement.AllowRotation(LTTRUE);

				HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, "walk_F");

				g_pInterface->SetModelAnimation(m_hObject, hAnim);
				g_pInterface->SetModelLooping(m_hObject, LTTRUE);
			}
			break;

			case eIdleTurn :
			{
				cm_Movement.SetControlFlags(CM_FLAG_NONE);

				const LTVector & vMyPos = cm_Movement.GetPos();

				if( (m_vLeashFrom - vMyPos).MagSqr() > m_fLeashLengthSqr*0.5f )
				{
					const LTVector & vMyRight = cm_Movement.GetRight();
					m_bIdleTurnRight = vMyRight.Dot( m_vLeashFrom - vMyPos ) > 0.0f;
				}
				else
				{
					m_bIdleTurnRight = GetRandom(0,10) > 5;
				}

				HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, "walk_B");

				g_pInterface->SetModelAnimation(m_hObject, hAnim);
				g_pInterface->SetModelLooping(m_hObject, LTTRUE);
			}
			break;

			default :
			case eIdleAnim :
			{
				cm_Movement.SetControlFlags(CM_FLAG_NONE);

				HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, 
					const_cast<char*>( aszIdleAnims[ GetRandom(0, c_nNumIdleAnims - 1) ] ) );

				g_pInterface->SetModelAnimation(m_hObject, hAnim);
				g_pInterface->SetModelLooping(m_hObject, LTTRUE);
			}
			break;

		} //switch( s_aeIdleAction[i] )

	} //if( !m_tmrIdleAction.Stopped() )
				
}

void Striker::StartChase(RootFSM * fsm)
{
	g_pServerSoundMgr->PlayCharSndFromObject(cm_Movement.GetCharacterButes()->m_nId, m_hObject, "Attack");
	SetRandomSound("RandomAttack");

	if (!m_hTarget)
	{
		fsm->AddMessage(Root::eNoTarget);
		return;
	}
	
	cm_Movement.SetControlFlags(CM_FLAG_FORWARD);
	cm_Movement.AllowRotation(LTTRUE);
	HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, "walk_F");

	g_pInterface->SetModelAnimation(m_hObject, hAnim);
	g_pInterface->SetModelLooping(m_hObject, LTTRUE);
}

void Striker::UpdateChase(RootFSM * fsm)
{
	if (!m_hTarget)
	{
		fsm->AddMessage(Root::eNoTarget);
		return;
	}

	// Check for target in range.
	LTVector vTargetPos, vPos;

	g_pInterface->GetObjectPos(m_hTarget, &vTargetPos);
	g_pInterface->GetObjectPos(m_hObject, &vPos);

	if( !WithinLeash(vTargetPos) )
	{
		m_hTarget = LTNULL;
		fsm->AddMessage(Root::eNoTarget);
		return;
	}


	LTFLOAT fDistSqr = (vTargetPos - vPos).MagSqr();
	if (fDistSqr < 100.0f * 100.0f)
	{
		fsm->AddMessage(Root::eInAttackRange);
		return;
	}
	

	// Update direction to face
	LTVector vUp;
	LTRotation rRot;

	rRot = cm_Movement.GetRot();
	vUp = cm_Movement.GetUp();
	LTVector vTemp = vTargetPos - vPos;
	vTemp -= vUp* vUp.Dot(vTemp);  // remove up component

	g_pMathLT->AlignRotation(rRot, vTemp, const_cast<LTVector&>(vUp));
	cm_Movement.SetRot(rRot);

}

void Striker::StartAttack(RootFSM * fsm)
{
	if (!m_hTarget)
	{
		fsm->AddMessage(Root::eNoTarget);
		return;
	}
	
	SetRandomSound("RandomAttack");

	// Stop moving
	cm_Movement.SetVelocity(LTVector(0,0,0));
	cm_Movement.SetControlFlags(CM_FLAG_NONE);

	HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, "Attack");

	g_pInterface->SetModelAnimation(m_hObject, hAnim);
	g_pInterface->SetModelLooping(m_hObject, LTTRUE);
}

void Striker::UpdateAttack(RootFSM * fsm)
{
	if (!m_hTarget)
	{
		fsm->AddMessage(Root::eNoTarget);
		return;
	}

	// Be sure target is still in range.
	LTVector vTargetPos, vMyPos;
	LTFLOAT fDistSqr;

	g_pInterface->GetObjectPos(m_hTarget, &vTargetPos);
	g_pInterface->GetObjectPos(m_hObject, &vMyPos);

	fDistSqr = (vTargetPos - vMyPos).MagSqr();

	if (fDistSqr >= 100.0f * 100.0f)
	{
		fsm->AddMessage(Root::eOutOfAttackRange);
		return;
	}

	// Keep facing the target.
	LTVector vUp;
	LTRotation rRot;

	rRot = cm_Movement.GetRot();
	vUp = cm_Movement.GetUp();
	LTVector vTemp = vTargetPos - vMyPos;
	vTemp -= vUp* vUp.Dot(vTemp);  // remove up component

	// Update direction to face
	g_pMathLT->AlignRotation(rRot, vTemp, const_cast<LTVector&>(vUp));
	cm_Movement.SetRot(rRot);
}

void Striker::StartDead(RootFSM * fsm)
{
	// Play our death sound.
	DamageType death_type = m_damage.GetDeathType();

	const char * szDeathSound = "Death";

	switch( death_type )
	{
		case DT_TORCH_CUT:
		case DT_TORCH_WELD:
		case DT_BURN:
		case DT_NAPALM:
		case DT_ACID:
			szDeathSound = "Death_Burn";
	}

	g_pServerSoundMgr->PlayCharSndFromObject(cm_Movement.GetCharacterButes()->m_nId, m_hObject, szDeathSound);
	

	// Play our death animation.
	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, "Death");
				
	g_pLTServer->SetModelAnimation(m_hObject, hAnim);
	g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
}

void Striker::UpdateDead(RootFSM * fsm)
{
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);
	if (dwState & MS_PLAYDONE)
	{
		g_pInterface->RemoveObject(m_hObject);
		return;
	}
}


void Striker::HandleModelString(ArgList* pArgList)
{
	if (!g_pLTServer || !pArgList || !pArgList->argv || pArgList->argc == 0) 
		return;

	char* pKey = pArgList->argv[0];
	if (!pKey) 
		return;

	if (stricmp(pKey, "Fire") == 0)
	{
		LTVector vDir, vPos;
		g_pInterface->GetObjectPos(m_hObject, &vPos);
		vDir = cm_Movement.GetForward();
		vDir.Norm();
		vDir *= m_fClawRange;

		IntersectQuery IQuery;
		IntersectInfo IInfo;
		IInfo.m_hObject = LTNULL;
		
		IQuery.m_From	= vPos;
		IQuery.m_To		= vPos + vDir;
		IQuery.m_FilterFn = LTNULL;
		IQuery.m_pUserData = LTNULL;

		IQuery.m_Flags = INTERSECT_HPOLY | IGNORE_NONSOLID | INTERSECT_OBJECTS;
		
		++g_cIntersectSegmentCalls;
		g_pServerDE->IntersectSegment(&IQuery, &IInfo);

		if (IInfo.m_hObject)
		{
			HOBJECT hObj;
			CCharacterHitBox *pHitBox;
			
			pHitBox = dynamic_cast<CCharacterHitBox *>(g_pInterface->HandleToObject(IInfo.m_hObject));
			if (pHitBox)
				hObj = pHitBox->GetModelObject();
			else
				hObj = IInfo.m_hObject;

			// Don't damage other strikers!
			if( !dynamic_cast<Striker*>( g_pInterface->HandleToObject(IInfo.m_hObject) ) )
			{
				// Send damage message...
				DamageStruct damage;

				damage.eType	= DT_SLICE;
				damage.fDamage	= m_fDamage*GetDifficultyFactor();
				damage.hDamager = m_hObject;

				damage.DoDamage(this, hObj);
			}
		}
/*
		// Play the noise

		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurface);
		_ASSERT(pSurf);
		if (pSurf && pSurf->szBodyFallSnd[0])
		{
			g_pServerSoundMgr->PlaySoundFromPos(vPos, pSurf->szBodyFallSnd, pSurf->fBodyFallSndRadius, SOUNDPRIORITY_MISC_LOW);
		}
*/
	}
	else if (stricmp(pKey, KEY_FOOTSTEP_SOUND) == 0 )
	{
		HandleFootstepKey();
	}

	Prop::HandleModelString(pArgList);
}

LTBOOL StrikerFilterFn(HOBJECT hObject, void *pUserData)
{
	if (!hObject || !g_pInterface)
		return LTFALSE;

	// ignore self
	if (hObject == (HOBJECT)pUserData)
		return LTFALSE;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Striker::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Striker::Save(HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) 
		return;

	g_pServerDE->WriteToMessageDWord(hWrite, m_bPlayerUsable);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrActivateMessage);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrActivateTarget);

	*hWrite << m_RootFSM;

	*hWrite << m_bFirstUpdate;
	*hWrite << m_fLeashLengthSqr;
	*hWrite << m_vLeashFrom;

	*hWrite << m_eIdleAction;
	*hWrite << m_tmrIdleAction;
	*hWrite << m_bIdleTurnRight;

	*hWrite << m_fDetectionRadiusSqr;
	*hWrite << m_fChaseSpeed;
	*hWrite << m_fDamage;
	*hWrite << m_fClawRange;
	*hWrite << m_hTarget;

	*hWrite << m_bLeftFoot;

	*hWrite << m_nRandomSoundBute;
	*hWrite << m_tmrRandomSound;

	cm_Movement.Save(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Striker::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Striker::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) 
		return;

	m_bPlayerUsable = (LTBOOL)g_pServerDE->ReadFromMessageDWord(hRead);
	m_hstrActivateMessage = g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrActivateTarget = g_pServerDE->ReadFromMessageHString(hRead);

	*hRead >> m_RootFSM;

	*hRead >> m_bFirstUpdate;
	*hRead >> m_fLeashLengthSqr;
	*hRead >> m_vLeashFrom;

	*hRead >> m_eIdleAction;
	*hRead >> m_tmrIdleAction;
	*hRead >> m_bIdleTurnRight;

	*hRead >> m_fDetectionRadiusSqr;
	*hRead >> m_fChaseSpeed;
	*hRead >> m_fDamage;
	*hRead >> m_fClawRange;
	*hRead >> m_hTarget;

	*hRead >> m_bLeftFoot;

	*hRead >> m_nRandomSoundBute;
	*hRead >> m_tmrRandomSound;

	cm_Movement.Load(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Striker::CacheFiles
//
//	PURPOSE:	Cache whatever resources this object uses
//
// ----------------------------------------------------------------------- //

void Striker::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) 
		return;
}

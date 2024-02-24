// ----------------------------------------------------------------------- //
//
// MODULE  : Beetle.cpp
//
// PURPOSE : Implementation of the Beetle
//
// CREATED : 11.13.2000
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Beetle.h"
#include "cpp_server_de.h"
#include "ServerUtilities.h"
#include "SoundMgr.h"
#include "CharacterMgr.h"
#include "ObjectMsgs.h"
#include "ServerSoundMgr.h"

// Statics

static char *s_szActivate		= "ACTIVATE";

// ----------------------------------------------------------------------- //
//
//	CLASS:		Beetle
//
//	PURPOSE:	Blindly pursues the first player to step in range
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(Beetle)

#ifdef _WIN32
	ADD_STRINGPROP_FLAG(Filename, "Models\\Props\\creature_beetle.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Skins\\Props\\creature_beetle.dtx", PF_FILENAME)
#else
	ADD_STRINGPROP_FLAG(Filename, "Models/Props/creature_beetle.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Skins/Props/creature_beetle.dtx", PF_FILENAME)
#endif

	ADD_STRINGPROP_FLAG(SurfaceOverride, "Flesh_Alien", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(DebrisType, "", PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SHADOW_FLAG(0, 0)
	ADD_BOOLPROP_FLAG(MoveToFloor, LTTRUE, 0)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)

	ADD_BOOLPROP_FLAG_HELP(IgnoreAI, LTTRUE, 0,			"TRUE is activated by players ONLY.")
	ADD_REALPROP_FLAG_HELP(DetectionRadius, 500.0f, 0,	"Beetle will jump if anything enters this radius.")

	ADD_REALPROP_FLAG_HELP(MinJumpDelay, 2.0f, 0, "Minimum time the beetle will wait between jumps." )
	ADD_REALPROP_FLAG_HELP(MaxJumpDelay, 5.0f, 0, "Maximum time the beetle will wait between jumps." )

	ADD_REALPROP_FLAG_HELP(MinJump, 500.0f, 0, "The minimum jumping velocity.")
	ADD_REALPROP_FLAG_HELP(MaxJump, 750.0f, 0, "The maximum jumping velocity.")


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

END_CLASS_DEFAULT(Beetle, Prop, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Beetle::Beetle()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Beetle::Beetle() : Prop ()
{
	m_bPlayerUsable = LTFALSE;
	m_hstrActivateMessage = LTNULL;
	m_hstrActivateTarget = LTNULL;

	m_fDetectionRadiusSqr = 1000.0f * 1000.0f;
	m_bIgnoreAI = LTTRUE;

	m_fMinJumpDelay = 2.0f;
	m_fMaxJumpDelay = 5.0f;

	m_fMinJump = 500.0f;
	m_fMaxJump = 750.0f;

	m_damage.SetRemoveOnDeath(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Beetle::~Beetle()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Beetle::~Beetle()
{
	if(!g_pInterface)
		return;

	FREE_HSTRING(m_hstrActivateMessage);
	FREE_HSTRING(m_hstrActivateTarget);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Beetle::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Beetle::EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT fData)
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
			g_pInterface->SetNextUpdate(m_hObject, 0.1f);

			CacheFiles();
			return dwRet;
		}

		case MID_UPDATE:
		{
			// DON'T call Prop::EngineMessageFn, object might get removed...
			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			Update();
			g_pInterface->SetNextUpdate(m_hObject, 0.1f);
	
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
//	ROUTINE:	Beetle::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Beetle::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) 
		return LTFALSE;

	GenericProp genProp;

	// Beetle stuff
	if (g_pServerDE->GetPropGeneric("IgnoreAI", &genProp) == DE_OK)
		m_bIgnoreAI = genProp.m_Bool;

	if (g_pServerDE->GetPropGeneric("DetectionRadius", &genProp) == DE_OK)
		m_fDetectionRadiusSqr = genProp.m_Float * genProp.m_Float;

	if (g_pServerDE->GetPropGeneric("MinJumpDelay", &genProp) == DE_OK)
		m_fMinJumpDelay = genProp.m_Float;

	if (g_pServerDE->GetPropGeneric("MaxJumpDelay", &genProp) == DE_OK)
		m_fMaxJumpDelay = genProp.m_Float;

	if (g_pServerDE->GetPropGeneric("MinJump", &genProp) == DE_OK)
		m_fMinJump = genProp.m_Float;

	if (g_pServerDE->GetPropGeneric("MaxJump", &genProp) == DE_OK)
		m_fMaxJump = genProp.m_Float;

	// Prop stuff
	if ( g_pServerDE->GetPropGeneric( "ActivateMessage", &genProp ) == DE_OK )
	if ( genProp.m_String[0] )
		m_hstrActivateMessage = g_pServerDE->CreateString( genProp.m_String );

	if ( g_pServerDE->GetPropGeneric( "ActivateTarget", &genProp ) == DE_OK )
	if ( genProp.m_String[0] )
		m_hstrActivateTarget = g_pServerDE->CreateString( genProp.m_String );
	
	if ( g_pServerDE->GetPropGeneric("PlayerUsable", &genProp ) == DE_OK )
		m_bPlayerUsable = genProp.m_Bool;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Beetle::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void Beetle::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) 
		return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Beetle::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD Beetle::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
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

			if(szMsg)
			{
				ILTCommon* pCommon = g_pLTServer->Common();
				if (pCommon)
				{
					// ConParse does not destroy szMsg, so this is safe
					ConParse parse;
					parse.Init((char*)szMsg);
					uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);

					while (pCommon->Parse(&parse) == LT_OK)
					{
						if (_stricmp(parse.m_Args[0], "HIDDEN") == 0)
						{
							if (parse.m_nArgs == 2 && parse.m_Args[1])
							{
								if ((_stricmp(parse.m_Args[1], "1") == 0) ||
									(_stricmp(parse.m_Args[1], "TRUE") == 0))
								{
									dwFlags &= ~FLAG_VISIBLE;
									dwFlags &= ~FLAG_SOLID;
									dwFlags &= ~FLAG_RAYHIT;
								}
								else if ((_stricmp(parse.m_Args[1], "0") == 0) ||
										(_stricmp(parse.m_Args[1], "FALSE") == 0))
								{
									dwFlags |= FLAG_VISIBLE;
									dwFlags |= FLAG_SOLID;
									dwFlags |= FLAG_RAYHIT;
								}

								g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
							}

							// Ok make sure nobody else does anything with this...
							g_pServerDE->FreeString(hMsg);
							return LTTRUE;
						}
					}
				}
			}

			g_pServerDE->FreeString(hMsg);

			break;
		}

		case MID_DAMAGE:
		{
			// Let our damage aggregate process the message first...

			DDWORD dwRet = Prop::ObjectMessageFn(hSender, messageID, hRead);

			// Check to see if we have been destroyed

			if ( m_damage.IsDead() )
			{
				// handle death

				//play a death sound
				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "Beetle_Death");

				MULTI_DEBRIS *debris = g_pDebrisMgr->GetMultiDebris("Beetle");

				if(debris)
				{
					LTVector vPos;
					g_pLTServer->GetObjectPos(m_hObject, &vPos);

					CLIENTDEBRIS fxStruct;

					fxStruct.rRot.Init();
					fxStruct.vPos = vPos;
					fxStruct.nDebrisId = debris->nId;

					CreateMultiDebris(fxStruct);
				}
			}
			else if( m_tmrJumpDelay.Stopped() )
			{
				m_RootFSM.AddMessage(Root::eInRange);
			}

			// TODO: Check to see if we have been disabled

			return dwRet;
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
//	ROUTINE:	Beetle::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL Beetle::InitialUpdate()
{
	SetupFSM();
	m_RootFSM.SetState(Root::eStateIdle);

	cm_Movement.Init(g_pInterface, m_hObject);
	cm_Movement.SetCharacterButes("Beetle");

	cm_Movement.SetObjectFlags( OFT_Flags,
								FLAG_STAIRSTEP | FLAG_TOUCH_NOTIFY | FLAG_SOLID |
								FLAG_GRAVITY | FLAG_MODELKEYS | FLAG_RAYHIT | FLAG_VISIBLE, LTTRUE );

	uint32 dwInitUserFlags = USRFLG_MOVEABLE | USRFLG_CHARACTER;

	uint32 dwSTFlag = SurfaceToUserFlag((SurfaceType)cm_Movement.GetCharacterButes()->m_nSurfaceType);
	uint32 dwUserFlags = UserFlagSurfaceMask(dwInitUserFlags);

	g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags | dwSTFlag);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Beetle::Update()
//
//	PURPOSE:	Update state machine
//
// ----------------------------------------------------------------------- //

void Beetle::Update()
{
	// Don't update if we have been hidden.
	if( (FLAG_VISIBLE & g_pLTServer->GetObjectFlags(m_hObject)) )
		m_RootFSM.Update();
}


// ----------------------------------------------------------------------- //
//   State Machine & related methods
// ----------------------------------------------------------------------- //
void Beetle::SetupFSM()
{
	m_RootFSM.DefineState(Root::eStateIdle, 
						  m_RootFSM.MakeCallback(*this, &Beetle::UpdateIdle),
						  m_RootFSM.MakeCallback(*this, &Beetle::StartIdle));

	m_RootFSM.DefineState(Root::eStateFlap,
						  m_RootFSM.MakeCallback(*this, &Beetle::UpdateFlap),
						  m_RootFSM.MakeCallback(*this, &Beetle::StartFlap));

	m_RootFSM.DefineState(Root::eStateCrawl,
						  m_RootFSM.MakeCallback(*this, &Beetle::UpdateCrawl),
						  m_RootFSM.MakeCallback(*this, &Beetle::StartCrawl));

	// Standard Transitions
	m_RootFSM.DefineTransition(Root::eStateIdle, Root::eInRange, Root::eStateFlap);
	m_RootFSM.DefineTransition(Root::eStateFlap, Root::eDoneMoving, Root::eStateIdle);
}


void Beetle::StartIdle(RootFSM * fsm)
{
	if (!m_hObject)
		return;

	cm_Movement.AllowRotation(LTFALSE);
	cm_Movement.SetVelocity(LTVector(0,0,0));
	cm_Movement.SetControlFlags(CM_FLAG_NONE);

	HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, "Idle_0");

	g_pInterface->SetModelAnimation(m_hObject, hAnim);
	g_pInterface->SetModelLooping(m_hObject, LTTRUE);
}

void Beetle::UpdateIdle(RootFSM * fsm)
{
	if( m_tmrJumpDelay.Stopped() )
	{
		if (IsEnemyInRange(m_fDetectionRadiusSqr))
		{
			fsm->AddMessage(Root::eInRange);
			return;
		}
	}
}

void Beetle::StartFlap(RootFSM * fsm)
{
//	cm_Movement.SetControlFlags(CM_FLAG_FORWARD);
	cm_Movement.AllowRotation(LTTRUE);

	LTVector vUp, vVel;
	vUp = cm_Movement.GetUp();

	LTRotation rRot = cm_Movement.GetRot();

	// face a random direction and fly there
	LTFLOAT fAmount = GetRandom(0.0f, 2 * MATH_PI);
	g_pMathLT->RotateAroundAxis(rRot, vUp, fAmount);
	cm_Movement.SetRot(rRot);

	vVel = cm_Movement.GetForward();
	vVel += vUp;
	vVel *= GetRandom(m_fMinJump, m_fMaxJump);

	cm_Movement.SetVelocity(vVel);	// one shot "jump jet"

	HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, "Flap");

	g_pInterface->SetModelAnimation(m_hObject, hAnim);
	g_pInterface->SetModelLooping(m_hObject, LTTRUE);

	//play the jump sound
	g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "Beetle_Jump");
}

void Beetle::UpdateFlap(RootFSM * fsm)
{
	if (cm_Movement.GetVelocity().MagSqr() < 10.0f)
	{
		m_tmrJumpDelay.Start( GetRandom(m_fMinJumpDelay, m_fMaxJumpDelay) );

		fsm->AddMessage(Root::eDoneMoving);
		return;
	}

	cm_Movement.Update(GetUpdateDelta());
}

void Beetle::StartCrawl(RootFSM * fsm)
{
/*
	cm_Movement.SetVelocity(LTVector(0,0,0));
	cm_Movement.SetControlFlags(CM_FLAG_NONE);

	HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, "Crawl");

	g_pInterface->SetModelAnimation(m_hObject, hAnim);
	g_pInterface->SetModelLooping(m_hObject, LTTRUE);
*/
}

void Beetle::UpdateCrawl(RootFSM * fsm)
{
	return;		// not implemented yet
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Beetle::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Beetle::Save(HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) 
		return;

	g_pServerDE->WriteToMessageDWord(hWrite, m_bPlayerUsable);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrActivateMessage);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrActivateTarget);

	*hWrite << m_RootFSM;
	*hWrite << m_fDetectionRadiusSqr;
	*hWrite << m_fMinJumpDelay;
	*hWrite << m_fMaxJumpDelay;
	*hWrite << m_fMinJump;
	*hWrite << m_fMaxJump;


	*hWrite << m_tmrJumpDelay;
	cm_Movement.Save(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Beetle::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Beetle::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) 
		return;

	m_bPlayerUsable = (LTBOOL)g_pServerDE->ReadFromMessageDWord(hRead);
	m_hstrActivateMessage = g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrActivateTarget = g_pServerDE->ReadFromMessageHString(hRead);

	*hRead >> m_RootFSM;
	*hRead >> m_fDetectionRadiusSqr;
	*hRead >> m_fMinJumpDelay;
	*hRead >> m_fMaxJumpDelay;
	*hRead >> m_fMinJump;
	*hRead >> m_fMaxJump;
	
	*hRead >> m_tmrJumpDelay;
	cm_Movement.Load(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Beetle::CacheFiles
//
//	PURPOSE:	Cache whatever resources this object uses
//
// ----------------------------------------------------------------------- //

void Beetle::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) 
		return;
}

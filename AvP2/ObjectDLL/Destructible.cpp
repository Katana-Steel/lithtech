// ----------------------------------------------------------------------- //
//
// MODULE  : Destructible.h
//
// PURPOSE : Destructible class
//
// CREATED : 9/23/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "Destructible.h"
#include "iltserver.h"
#include "MsgIDs.h"
#include "Projectile.h"
#include "ServerUtilities.h"
#include "Character.h"
#include "PlayerObj.h"
#include "BodyProp.h"
#include "GameServerShell.h"
#include "ObjectMsgs.h"
#include "iltphysics.h"
#include "AIUtils.h"
#include "MultiplayerMgr.h"

#define TRIGGER_MSG_DESTROY	"DESTROY"
#define TRIGGER_MSG_DAMAGE	"DAMAGE"
#define TRIGGER_MSG_RESET	"RESET"
#define TRIGGER_MSG_CANDAMAGE "CANDAMAGE"
#define TRIGGER_MSG_NEVERDESTROY "NEVERDESTROY"

#define MAXIMUM_BLOCK_PRIORITY	255.0f
#define MINIMUM_FRICTION		5.0f
#define MAXIMUM_FRICTION		15.0f
#define MINIMUM_FORCE			0.0f

#define CRITICAL_HIT_CHANCE			0.05f	// Percentage of time critical hits happen
#define CRITICAL_HIT_RATIO			4.0f	// 4 times as much damage
#define CRITICAL_HIT_HEALTH_BONUS	.25f	// Percentage of health bonus

extern CGameServerShell* g_pGameServerShell;

#ifndef _FINAL
extern CVarTrack g_DamageScale;
extern CVarTrack g_HealScale;
#endif

const LTFLOAT DamageStruct::kInfiniteDamage = 100000.0f; // Infinite damage

#ifndef _FINAL
static bool ShouldShowDamage(HOBJECT hObject)
{
	ASSERT( g_pLTServer );

	static CVarTrack vtShowDamage(g_pLTServer,"ShowDamage",0.0f);

	return vtShowDamage.GetFloat() > 0.0f && ShouldShow(hObject);
}
#endif

ILTMessage & operator<<(ILTMessage & out, DamageHistory & x)
{
	out << x.fDamage;
	out.WriteFloat(x.fTime - g_pLTServer->GetTime());
	out << x.eType;
	out << x.nModelNode;

	return out;
}

ILTMessage & operator>>(ILTMessage & in, DamageHistory & x)
{
	in >> x.fDamage;
	x.fTime = in.ReadFloat() + g_pLTServer->GetTime();
	in >> x.eType;
	in >> x.nModelNode;

	return in;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageStruct::InitFromMessage
//
//	PURPOSE:	Initialize from a MID_DAMAGE message
//
// ----------------------------------------------------------------------- //

LTBOOL DamageStruct::InitFromMessage(HMESSAGEREAD hRead)
{
    if (!hRead) return LTFALSE;

    g_pLTServer->ReadFromMessageVector(hRead, &vDir);
    fDuration  = g_pLTServer->ReadFromMessageFloat(hRead);
    fDamage    = g_pLTServer->ReadFromMessageFloat(hRead);
    eType      = (DamageType)g_pLTServer->ReadFromMessageByte(hRead);
    hDamager   = g_pLTServer->ReadFromMessageObject(hRead);
    hContainer = g_pLTServer->ReadFromMessageObject(hRead);
    bIgnoreFF = g_pLTServer->ReadFromMessageByte(hRead);

#ifndef _FINAL
	fDamage *= g_DamageScale.GetFloat(1.0f);
#endif

    g_pLTServer->ResetRead(hRead);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageStruct::DoDamage
//
//	PURPOSE:	Send the MID_DAMAGE message to do the damage
//
// ----------------------------------------------------------------------- //

LTBOOL DamageStruct::DoDamage(LPBASECLASS pDamager, HOBJECT hVictim, HOBJECT hContainer)
{
    if (!pDamager || !hVictim) return LTFALSE;

    HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(pDamager, hVictim, MID_DAMAGE);
    g_pLTServer->WriteToMessageVector(hWrite, &vDir);
    g_pLTServer->WriteToMessageFloat(hWrite, fDuration);
    g_pLTServer->WriteToMessageFloat(hWrite, fDamage);
    g_pLTServer->WriteToMessageByte(hWrite, eType);
    g_pLTServer->WriteToMessageObject(hWrite, hDamager);
    g_pLTServer->WriteToMessageObject(hWrite, hContainer);
    g_pLTServer->WriteToMessageByte(hWrite, bIgnoreFF);
   g_pLTServer->EndMessage(hWrite);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::CDestructible
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CDestructible::CDestructible() : IAggregate()
{
    m_hObject                   = LTNULL;
    m_hLastDamager              = LTNULL;

    m_bDead                     = LTFALSE;
    m_bIsPlayer                 = LTFALSE;
    m_bApplyDamagePhysics       = LTFALSE;
	m_fMass						= 1.0f;
	m_fHitPoints				= 1.0f;
	m_fMaxHitPoints				= 1.0f;
	m_fArmorPoints				= 0.0f;
	m_fMaxArmorPoints			= 1.0f;
	m_fDeathDamage				= 0.0f;
	m_dwCantDamageTypes			= 0;//DamageTypeToFlag(DT_GADGET_DECAYPOWDER);
	m_eLastDamageType			= DT_UNSPECIFIED;
	m_fLastDamage				= 0.0f;
	m_eDeathType				= DT_UNSPECIFIED;

	VEC_INIT(m_vDeathDir);
	VEC_INIT(m_vLastDamageDir);
	m_fLastDamageTime = -1.0f;

    m_hstrDamageTriggerTarget       = LTNULL;
    m_hstrDamageTriggerMessage      = LTNULL;
    m_hstrDamagerMessage            = LTNULL;
    m_hstrDeathTriggerTarget        = LTNULL;
    m_hstrDeathTriggerMessage       = LTNULL;
    m_hstrPlayerDeathTriggerTarget  = LTNULL;
    m_hstrPlayerDeathTriggerMessage = LTNULL;
    m_hstrKillerMessage             = LTNULL;
	m_nDamageTriggerCounter			= 0;
	m_nDamageTriggerNumSends		= 1;

    m_bCanHeal                  = LTTRUE;
    m_bCanRepair                = LTTRUE;
    m_bCanDamage                = LTTRUE;
    m_bNeverDestroy             = LTFALSE;
    m_bCanCrush					= LTFALSE;

	m_fMinHealth				= -1.0f;
	m_bCanSetRayHit				= LTTRUE;
	m_bMassBasedBlockingPriority = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::~CDestructible
//
//	PURPOSE:	Destructible
//
// ----------------------------------------------------------------------- //

CDestructible::~CDestructible()
{
	if (m_hstrDamageTriggerTarget)
	{
        g_pLTServer->FreeString(m_hstrDamageTriggerTarget);
	}

	if (m_hstrDamageTriggerMessage)
	{
        g_pLTServer->FreeString(m_hstrDamageTriggerMessage);
	}

	if (m_hstrDeathTriggerTarget)
	{
        g_pLTServer->FreeString(m_hstrDeathTriggerTarget);
	}

	if (m_hstrDeathTriggerMessage)
	{
        g_pLTServer->FreeString(m_hstrDeathTriggerMessage);
	}

	if (m_hstrPlayerDeathTriggerTarget)
	{
        g_pLTServer->FreeString(m_hstrPlayerDeathTriggerTarget);
	}

	if (m_hstrPlayerDeathTriggerMessage)
	{
        g_pLTServer->FreeString(m_hstrPlayerDeathTriggerMessage);
	}

	if (m_hstrDamagerMessage)
	{
        g_pLTServer->FreeString(m_hstrDamagerMessage);
	}

	if (m_hstrKillerMessage)
	{
        g_pLTServer->FreeString(m_hstrKillerMessage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //

uint32 CDestructible::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_TOUCHNOTIFY:
		{
			HandleTouch(pObject, (HOBJECT)pData, fData);
		}
		break;

		case MID_CRUSH:
		{
			HandleCrush(pObject, (HOBJECT)pData);
		}
		break;

		case MID_PRECREATE:
		{
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp(pObject, (ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate(pObject);
			}
		}
		break;

		case MID_UPDATE:
		{
			ASSERT( dynamic_cast<GameBase*>( pObject ) );
			
			Update(static_cast<GameBase*>( pObject )->GetUpdateDelta());
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint8)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint8)fData);
		}
		break;
	}

    return IAggregate::EngineMessageFn(pObject, messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CDestructible::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HandleTrigger(pObject, hSender, hRead);
		}
		break;

		case MID_DAMAGE:
		{
			HandleDamage(pObject, hSender, hRead);
		}
		break;

		case MID_REPAIR:
		{
			if (m_bCanRepair)
			{
				HandleRepair(pObject, hSender, hRead);
			}
		}
		break;

		case MID_HEAL:
		{
			if (m_bCanHeal)
			{
				HandleHeal(pObject, hSender, hRead);
			}
		}
		break;

		default : break;
	}

    return IAggregate::ObjectMessageFn(pObject, hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::UpdateHistory
//
//	PURPOSE:	Update the history
//
// ----------------------------------------------------------------------- //

void CDestructible::UpdateHistory(DamageHistory &pNewHistory)
{
	// Move everything down the list
	for(int i = (MAX_DAMAGE_HISTORY - 1); i > 0; i--)
	{
		m_pHistory[i] = m_pHistory[i - 1];
	}

	m_pHistory[0] = pNewHistory;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::UpdateHistory
//
//	PURPOSE:	Update the history
//
// ----------------------------------------------------------------------- //

LTFLOAT CDestructible::GetHistoryDamageAmount(LTFLOAT fTime, uint32 nModelNode)
{
	LTFLOAT fCurTime = g_pLTServer->GetTime();
	LTFLOAT fAmount = 0.0f;

	for(int i = 0; i < MAX_DAMAGE_HISTORY; i++)
	{
		if(m_pHistory[i].fTime >= (fCurTime - fTime))
		{
			if((nModelNode == -1) || (nModelNode == m_pHistory[i].nModelNode))
				fAmount += m_pHistory[i].fDamage;
		}
		else
			break;   // They're in order... so if we get here, then there's nothing left to add
	}

	return fAmount;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::ReadProp(BaseClass *pObject, ObjectCreateStruct *pStruct)
{
    if (!pStruct) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("Mass", &genProp) == LT_OK)
	{
		SetMass(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("HitPoints", &genProp) == LT_OK)
	{
		SetHitPoints(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("MaxHitPoints", &genProp) == LT_OK)
	{
		SetMaxHitPoints(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("Armor", &genProp) == LT_OK)
	{
		SetArmorPoints(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("MaxArmor", &genProp) == LT_OK)
	{
		SetMaxArmorPoints(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("DamageTriggerTarget", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrDamageTriggerTarget = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("DamageTriggerMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrDamageTriggerMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("DamageTriggerCounter", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_nDamageTriggerCounter = genProp.m_Long;
		}
	}

    if (g_pLTServer->GetPropGeneric("DamageTriggerNumSends", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_nDamageTriggerNumSends = genProp.m_Long;
		}
	}

    if (g_pLTServer->GetPropGeneric("DamagerMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrDamagerMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("DeathTriggerTarget", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrDeathTriggerTarget = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("DeathTriggerMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrDeathTriggerMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("PlayerDeathTriggerTarget", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrPlayerDeathTriggerTarget = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("PlayerDeathTriggerMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrPlayerDeathTriggerMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("KillerMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrKillerMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("CanHeal", &genProp) == LT_OK)
	{
		m_bCanHeal = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("CanRepair", &genProp) == LT_OK)
	{
		m_bCanRepair = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("CanDamage", &genProp) == LT_OK)
	{
		m_bCanDamage = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("NeverDestroy", &genProp) == LT_OK)
	{
		m_bNeverDestroy = genProp.m_Bool;
	}


    return LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::InitialUpdate
//
//	PURPOSE:	Handle object initial update
//
// ----------------------------------------------------------------------- //

void CDestructible::InitialUpdate(LPBASECLASS pObject)
{
	if (!pObject || !pObject->m_hObject) return;
	if (!m_hObject) m_hObject = pObject->m_hObject;

	SetBlockingPriority();

	if (m_fMass > 1.0f)
	{
        g_pLTServer->SetObjectMass(m_hObject, m_fMass);
	}

	// Determine if we are a player object or not

	if (IsPlayer(m_hObject))
	{
        m_bIsPlayer = LTTRUE;
	}

	if(m_bCanSetRayHit)
	{
		// Make sure the ray hit flag is set on this object... otherwise, it will
		// only be damaged through splash damage
		uint32 dwFlags;
		g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
		g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags | FLAG_RAYHIT);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Update
//
//	PURPOSE:	Handle object updates
//
// ----------------------------------------------------------------------- //

void CDestructible::Update( LTFLOAT fFrameTime )
{
	// Update progressive damage...

	UpdateProgressiveDamage(fFrameTime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::UpdateProgressiveDamage
//
//	PURPOSE:	Update any progressive damage being done to the object.
//
// ----------------------------------------------------------------------- //

void CDestructible::UpdateProgressiveDamage(LTFLOAT fFrameTime)
{
	DamageStruct damage;

    LTBOOL bStunned		= LTFALSE;
    LTBOOL bEMPEffected	= LTFALSE; 
	LTBOOL bNapalmed	= LTFALSE;

	CCharacter* pCharacter = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(m_hObject));
	BodyProp* pBodyProp = dynamic_cast<BodyProp*>(g_pLTServer->HandleToObject(m_hObject));

	int nCharacter = -1;

	if(pCharacter)
		nCharacter = pCharacter->GetCharacter();
	else if(pBodyProp)
		nCharacter = pBodyProp->GetCharacter();

	LTBOOL bCanBeOnFire = g_pCharacterButeMgr->GetCanBeOnFire(nCharacter);


	// Loop over all the progressive damage done to us...

	for (int i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		if (m_ProgressiveDamage[i].fDuration > 0.0f)
		{
			m_ProgressiveDamage[i].fDuration -= fFrameTime;

			// Process the damage for this frame...

			damage = m_ProgressiveDamage[i];
			damage.fDamage = damage.fDamage * fFrameTime;
			DoDamage(damage, LTNULL);

			switch(damage.eType)
			{
				case DT_STUN:		bStunned		= LTTRUE;	break;
				case DT_EMP:		bEMPEffected	= LTTRUE;	break;
				case DT_NAPALM:
				{
					if(!bCanBeOnFire)
					{
						m_ProgressiveDamage[i].fDuration = 0.0f;
						bNapalmed = LTFALSE;
					}
					else
					{
						bNapalmed = LTTRUE;
					}

					break;
				}
			}
		}
	}

	// Set our user flags if we're bleeding/poisoned/stunned...

	if (pCharacter || pBodyProp)
	{
		uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);

		dwUserFlags = bStunned ? (dwUserFlags | USRFLG_CHAR_STUNNED) : (dwUserFlags & ~USRFLG_CHAR_STUNNED);
		dwUserFlags = bNapalmed ? (dwUserFlags | USRFLG_CHAR_NAPALM | USRFLG_VISIBLE) : (dwUserFlags & ~USRFLG_CHAR_NAPALM);
		dwUserFlags = bEMPEffected ? (dwUserFlags | USRFLG_CHAR_EMPEFFECT) : (dwUserFlags & ~USRFLG_CHAR_EMPEFFECT);


		if(pCharacter)
		{
			if(bStunned)
				pCharacter->HandleStunEffect();

			if(bEMPEffected)
				pCharacter->HandleEMPEffect();
		}

		g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::StunTest
//
//	PURPOSE:	Test to see if the stun is successfull
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::StunTest(DamageStruct & damage)
{
	//only effect characters
	if(!IsCharacter(m_hObject))
		return LTFALSE;

	//get pointer to character data
	CCharacter* pChar = (CCharacter*)g_pServerDE->HandleToObject(m_hObject);

	LTFLOAT fStunChance = g_pCharacterButeMgr->GetBaseStunChance(pChar->GetCharacter());

	//modify stun chance based on hitpoints
	if( fStunChance > 0.0f )
		fStunChance += (100-fStunChance)*(1-m_fHitPoints/m_fMaxHitPoints);

	if(GetRandom(0.0f,100.0f) > fStunChance)
		//character saved
		return LTFALSE;

	//character is stuned, set duration
	damage.fDuration = GetRandom(damage.fDuration/2, damage.fDuration);

	//return
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::AddProgressiveDamage
//
//	PURPOSE:	Add progressive damage to the object
//
// ----------------------------------------------------------------------- //

void CDestructible::AddProgressiveDamage(DamageStruct & damage)
{
    LTFLOAT fLeastDuration = 100000.0f;
	int nIndex = -1;

	// God Mode
	if (!m_bCanDamage)
		return;

	//test for stun
	if(damage.eType == DT_STUN)
	{
		if(!StunTest(damage))
			//failed to stun the target
			return;
	}
	
	// If the new damage is from a container, see if this container
	// is already damaging us...

    int i;
	if (damage.hContainer)
	{
        for (i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
		{
			if (m_ProgressiveDamage[i].hContainer == damage.hContainer)
			{
				m_ProgressiveDamage[i] = damage;
				return;
			}
		}
	}

	//see if we are already being damaged by this type
    for (i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		if (m_ProgressiveDamage[i].eType == damage.eType)
		{
			//yep just replace it with the new damage
			m_ProgressiveDamage[i] = damage;
			return;
		}
	}
	
    for (i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		if (m_ProgressiveDamage[i].fDuration <= 0.0f)
		{
			m_ProgressiveDamage[i] = damage;
			return;
		}
		else
		{
			if (m_ProgressiveDamage[i].fDuration < fLeastDuration)
			{
				fLeastDuration = m_ProgressiveDamage[i].fDuration;
				nIndex = i;
			}
		}
	}

	// If we got to here there were no empty slots...Replace the slot
	// with the least amount of time if the new progressive damage item
	// would do more damage...

	if (nIndex != -1)
	{
        LTFLOAT fDamageLeft = m_ProgressiveDamage[nIndex].fDuration * m_ProgressiveDamage[nIndex].fDamage;
        LTFLOAT fNewDamage = damage.fDuration * damage.fDamage;

		// This assumes that all damage is equal.  In the future we may
		// want to check for powerups or other items that could make this
		// assumption false...

		if (fNewDamage > fDamageLeft)
		{
			m_ProgressiveDamage[nIndex] = damage;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::SetMass
//
//	PURPOSE:	Set the blocking priority for this object
//
// ----------------------------------------------------------------------- //

void CDestructible::SetMass(LTFLOAT fMass)
{
	m_fMass = fMass;

	SetBlockingPriority();

	// Set the friction based on the mass of the object...

	if (!m_hObject) return;

    g_pLTServer->SetObjectMass(m_hObject, m_fMass);

    LTFLOAT fFricCoeff = MINIMUM_FRICTION + (m_fMass * MAXIMUM_FRICTION / INFINITE_MASS);
    g_pLTServer->SetFrictionCoefficient(m_hObject, fFricCoeff);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::SetBlockingPriority
//
//	PURPOSE:	Set the blocking priority for this object
//
// ----------------------------------------------------------------------- //

void CDestructible::SetBlockingPriority()
{
	if (!m_hObject) return;

	// Make sure we should set the blocking priority based on the mass.
	if( !m_bMassBasedBlockingPriority )
		return;

    uint8 nPriority = (uint8)(m_fMass * MAXIMUM_BLOCK_PRIORITY / INFINITE_MASS);
	if (nPriority <= 0) nPriority = 1;

	SetBlockingPriority( nPriority );
}

void CDestructible::SetBlockingPriority( uint8 nBlockingPriority )
{
	if (!m_hObject) return;

    g_pLTServer->SetBlockingPriority(m_hObject, nBlockingPriority);
    g_pLTServer->SetForceIgnoreLimit(m_hObject, MINIMUM_FORCE);

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	dwFlags |= FLAG_TOUCH_NOTIFY;
    g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::GetMaxHitPoints
//
//	PURPOSE:	Returns the maximum hit points for this object
//
// ----------------------------------------------------------------------- //

LTFLOAT CDestructible::GetMaxHitPoints() const
{
    LTFLOAT fAdjustedMax = m_fMaxHitPoints;

	if(m_bIsPlayer)
	{
        CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hObject));

		if(pPlayer)
		{
			// See if we should increase/decrease the max...
		}
	}

	return fAdjustedMax;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::GetMaxArmorPoints
//
//	PURPOSE:	Returns the maximum armor points for this object
//
// ----------------------------------------------------------------------- //

LTFLOAT CDestructible::GetMaxArmorPoints() const
{
    LTFLOAT fAdjustedMax = m_fMaxArmorPoints;

	if(m_bIsPlayer)
	{
        CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hObject));

		if(pPlayer)
		{
			// See if we should increase/decrease the max...
		}
	}

	return fAdjustedMax;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleDamage
//
//	PURPOSE:	Handle damage message
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleDamage(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
	DamageStruct damage;
	damage.InitFromMessage(hRead);

	// Check for progressive damage...

	if (damage.fDuration > 0.0f)
	{
		AddProgressiveDamage(damage);
	}
	else
	{
		DoDamage(damage, hSender);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::DoDamage
//
//	PURPOSE:	Do the damage
//
// ----------------------------------------------------------------------- //

void CDestructible::DoDamage(DamageStruct & damage, HOBJECT hSender)
{
    LTVector vDir		= damage.vDir;
    LTFLOAT  fDamage	= damage.fDamage;
	DamageType eType	= damage.eType;
	HOBJECT hDamager	= damage.hDamager;

    LPBASECLASS pObject = g_pLTServer->HandleToObject(m_hObject);

	// Get a character pointer to see if this is a character...
	CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(m_hObject));
	BodyProp *pBody = dynamic_cast<BodyProp*>(g_pLTServer->HandleToObject(m_hObject));


	// ----------------------------------------------------------------------- //
	// Do some special multiplayer damage checks
	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hObject));
		CPlayerObj* pDamagerPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(hDamager));

		// Scale the damage
		fDamage *= g_pGameServerShell->GetGameInfo()->m_fDamageScale;

		// Ignore friendly fire damage... but still damage yourself if you mess up you idiot!
		if(!damage.bIgnoreFF && !g_pGameServerShell->GetMultiplayerMgr()->AllowFriendlyFire())
		{
			if(pPlayer && pDamagerPlayer && (m_hObject != hDamager))
			{
				HCLIENT hClient1 = pPlayer->GetClient();
				HCLIENT hClient2 = pDamagerPlayer->GetClient();

				if(g_pGameServerShell->GetMultiplayerMgr()->ClientsOnSameTeam(hClient1, hClient2))
				{
					damage.fDamage = fDamage = 0.0f;
				}
			}
		}
	}


	// See if we should actually process damage...

	if (m_bDead)
	{
		// Take a snapshot of this damage and get outta here!

		// ----------------------------------------------------------------------- //
		// Update the damage history 
		DamageHistory history;
		history.fDamage = fDamage;
		history.fTime = g_pLTServer->GetTime();
		history.eType = eType;
		history.nModelNode = (uint32)(pChar ? pChar->GetModelNodeLastHit() : (pBody ? pBody->GetModelNodeLastHit() : -1));
		UpdateHistory(history);

		return;
	}


	// See if this type of damage applies to us...

	if ( m_dwCantDamageTypes )
	{
		if (m_dwCantDamageTypes & DamageTypeToFlag(eType)) return;
	}

	// Do rag-doll, and adjust damage if it was greater than 0...(if the damage
	// was zero, we still want to process it)...

	LTFLOAT fOrigDamage = fDamage;
	LTFLOAT fAPPercent = 0.0f;
	LTFLOAT fHPPercent = 0.0f;
    LTFLOAT fAbsorb = 0.0f;

	if (fDamage > 0.0f)
	{
		// If the damage is not greater than the resistance, just return
		if(DamageTypeResistance(eType))
		{
			LTFLOAT fDamageResist = 1.0f;

			if(pChar)
				fDamageResist = pChar->GetButes()->m_fDamageResistance;

			fDamage *= fDamageResist;
		}


		// Rag-doll time...

		if (CanRagDoll(vDir, eType))
		{
            LTVector vImpactVel, vTemp, vVel;

            g_pLTServer->GetVelocity(m_hObject, &vVel);

			VEC_COPY(vTemp, vDir);
			vTemp.Norm();

			if (m_fMass <= 0.0f)
			{
				m_fMass = 100.0f;
			}

            LTFLOAT fMultiplier = fDamage * (INFINITE_MASS/(m_fMass*5.0f));

			// If not on ground, scale down velocity...

			CollisionInfo standingInfo;
            g_pLTServer->GetStandingOn(m_hObject, &standingInfo);

			if (!standingInfo.m_hObject)
			{
				fMultiplier /= 10.0f;
			}

			VEC_MULSCALAR(vTemp, vTemp, fMultiplier);
			VEC_ADD(vImpactVel, vTemp, vVel);

            g_pLTServer->SetVelocity(m_hObject, &vImpactVel);
		}


		// Armor absorbs as much damage as it can based off the ArmorPoints
		// percent that the damage type has
		fAPPercent = DamageTypeAPPercent(eType) * fDamage;
		fHPPercent = DamageTypeHPPercent(eType) * fDamage;
		fDamage = fAPPercent + fHPPercent;

		if((m_fArmorPoints > 0.0) && (DamageTypeAPPercent(eType) > 0.0f))
		{
			fAbsorb = fAPPercent > m_fArmorPoints ? m_fArmorPoints : fAPPercent;
			fDamage	-= fAbsorb;

			if (fDamage < 0.0)
			{
				fDamage = 0.0f;
			}
		}
	}


	m_fLastDamage		= fOrigDamage;
	m_eLastDamageType	= eType;
	m_vLastDamageDir	= vDir;
	m_fLastDamageTime   = g_pLTServer->GetTime();
	SetLastDamager(hDamager);


	// Actually apply the damage
	uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);

	if(!m_bCanDamage || DebugDamageOn() || (dwUserFlags & USRFLG_CHAR_FACEHUG))
	{
		if((m_fMinHealth > 0.0f) && (m_fHitPoints > m_fMinHealth))
		{
			LTFLOAT fTempHit = m_fHitPoints - fDamage;

			if(fTempHit < m_fMinHealth)
			{
				fDamage = m_fHitPoints - m_fMinHealth;
			}
		}
		else
		{
			// ----------------------------------------------------------------------- //
			// Update the damage history 
			DamageHistory history;
			history.fDamage = fDamage;
			history.fTime = g_pLTServer->GetTime();
			history.eType = eType;
			history.nModelNode = (uint32)(pChar ? pChar->GetModelNodeLastHit() : (pBody ? pBody->GetModelNodeLastHit() : -1));
			UpdateHistory(history);
			return;
		}
	}

#ifndef _FINAL
	if( ShouldShowDamage(m_hObject) )
	{
		AICPrint(m_hObject, "Damage type %d, %f damage (%f secs.) to node %d, from %s.",
			damage.eType,
			damage.fDamage,
			damage.fDuration,
			pChar ? pChar->GetModelNodeLastHit() : -1,
			damage.hDamager ? g_pLTServer->GetObjectName(damage.hDamager) : "<null>" );
	}
#endif

	m_fArmorPoints -= fAbsorb;

	if (m_fArmorPoints < 0.0f)
	{
		m_fArmorPoints = 0.0f;
	}

	m_fHitPoints -= fDamage;

	if ( m_nDamageTriggerCounter > 0 )
	{
		m_nDamageTriggerCounter--;
	}


	// If this is supposed to send a damage trigger, send it now...

	if (m_nDamageTriggerCounter <= 0)
	{
		if (hSender != m_hObject)
		{
			if (m_hstrDamageTriggerTarget && m_hstrDamageTriggerMessage && m_nDamageTriggerNumSends != 0)
			{
				SendTriggerMsgToObjects(pObject, m_hstrDamageTriggerTarget, m_hstrDamageTriggerMessage);
				m_nDamageTriggerNumSends--;
			}
		}
	}

	// Send message to object that damaged us...

	if (hDamager && m_hstrDamagerMessage)
	{
		if (hSender != m_hObject)
		{
			SendTriggerMsgToObject(pObject, hDamager, m_hstrDamagerMessage);
		}
	}


	// Force dead
	LTBOOL bForceDead = LTFALSE;

	if(IsAI(m_hObject) && (eType == DT_NAPALM) && pChar)
	{
		if(g_pCharacterButeMgr->GetCanBeOnFire(pChar->GetCharacter()))
			bForceDead = LTTRUE;
	}


	// See if we're dead...

	if (((m_fHitPoints <= 1.0f) || bForceDead) && !m_bNeverDestroy)
	{
		m_fHitPoints	= 0.0f;
		m_fDeathDamage	= fDamage;
		m_eDeathType	= eType;
		m_vDeathDir		= vDir;

		HandleDestruction(hDamager);

		if (m_bIsPlayer)
		{
			ProcessPlayerDeath(hDamager);
		}
	}

	// ----------------------------------------------------------------------- //
	// Update the damage history 
	DamageHistory history;
	history.fDamage = fDamage;
	history.fTime = g_pLTServer->GetTime();
	history.eType = eType;
	history.nModelNode = (uint32)(pChar ? pChar->GetModelNodeLastHit() : (pBody ? pBody->GetModelNodeLastHit() : -1));
	UpdateHistory(history);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::SetLastDamager
//
//	PURPOSE:	Save our last damager
//
// ----------------------------------------------------------------------- //

void CDestructible::SetLastDamager(HOBJECT hDamager)
{
	// Save info on the guy who hit us...

	if (m_hLastDamager != hDamager)
	{
		m_hLastDamager = hDamager;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ProcessPlayerDeath
//
//	PURPOSE:	Process the death of a player
//
// ----------------------------------------------------------------------- //

void CDestructible::ProcessPlayerDeath(HOBJECT hDamager)
{
	// Stop player movement if dead...

    LTVector vZero;
	VEC_INIT(vZero);
    g_pLTServer->SetVelocity(m_hObject, &vZero);
    g_pLTServer->SetAcceleration(m_hObject, &vZero);

	// If this was a player-player frag, send a message to all clients

    CPlayerObj* pVictim = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hObject));
    CPlayerObj* pKiller = LTNULL;
	
	if(hDamager)
		pKiller = IsPlayer(hDamager) ? dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(hDamager)) : pVictim;

	if(pVictim && pKiller)
	{
		HCLIENT hClientVictim = pVictim->GetClient();
		HCLIENT hClientKiller = pKiller->GetClient();

		if(hClientVictim && hClientKiller && g_pGameServerShell->GetGameType().IsMultiplayer())
		{
			g_pGameServerShell->GetMultiplayerMgr()->ClientKilledMsg(hClientVictim, hClientKiller);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleDestruction
//
//	PURPOSE:	Handle destruction
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleDestruction(HOBJECT hDamager)
{
	m_bDead      = LTTRUE;
	m_fHitPoints = 0.0;

	LPBASECLASS pD = g_pLTServer->HandleToObject(m_hObject);

	if (m_hstrDeathTriggerTarget && m_hstrDeathTriggerMessage)
	{
		SendTriggerMsgToObjects(pD, m_hstrDeathTriggerTarget, m_hstrDeathTriggerMessage);
	}

	if (m_hstrPlayerDeathTriggerTarget && m_hstrPlayerDeathTriggerMessage && hDamager && IsPlayer(hDamager))
	{
		SendTriggerMsgToObjects(pD, m_hstrPlayerDeathTriggerTarget, m_hstrPlayerDeathTriggerMessage);
	}

	// Send message to object that killed us...

	if(hDamager && m_hstrKillerMessage)
	{
		SendTriggerMsgToObject(pD, hDamager, m_hstrKillerMessage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::SetCanDamage
//
//	PURPOSE:	Sets whether this object can be damaged or not
//
// ----------------------------------------------------------------------- //

void CDestructible::SetCanDamage(LTBOOL bCanDamage, LTFLOAT fMin)
{
	m_bCanDamage = bCanDamage;
	m_fMinHealth = fMin;


	// Multiplayer specific stuff
	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hObject));

		if(pPlayer)
		{
			// Set a flag for them to glow
			uint32 dwUsrFlg = g_pLTServer->GetObjectUserFlags(m_hObject);

			uint32 dwUserFlags	= g_pLTServer->GetObjectUserFlags(m_hObject);

			if(m_bCanDamage || dwUserFlags & USRFLG_CHAR_FACEHUG)
				dwUsrFlg &= ~USRFLG_GLOW;
			else
				dwUsrFlg |= USRFLG_GLOW;

			g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlg);


			// Set their alpha to half
			if(!(dwUserFlags & USRFLG_CHAR_FACEHUG))
			{
				LTFLOAT r, g, b, a;
				g_pLTServer->GetObjectColor(m_hObject, &r, &g, &b, &a);
				g_pLTServer->SetObjectColor(m_hObject, r, g, b, m_bCanDamage ? 1.0f : 0.1f);
			}


			// Make them solid or non-solid
			uint32 dwFlags;
			g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);

			if(m_bCanDamage)
				g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags | FLAG_SOLID);
			else
				g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags & ~FLAG_SOLID);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ClearProgressiveDamages
//
//	PURPOSE:	Handle destruction
//
// ----------------------------------------------------------------------- //

void CDestructible::ClearProgressiveDamages()
{
	// Kill all the progressive damage types
	for (int i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
		m_ProgressiveDamage[i].Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ClearProgressiveDamages
//
//	PURPOSE:	Handle destruction
//
// ----------------------------------------------------------------------- //

void CDestructible::ClearProgressiveDamages(DamageType dt)
{
	// Kill all the progressive damage types
	for (int i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		if(m_ProgressiveDamage[i].eType == dt)
			m_ProgressiveDamage[i].Clear();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleHeal
//
//	PURPOSE:	Handle heal message.
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleHeal(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
/*	if (m_bDead) return;

    LTFLOAT fAmount = g_pLTServer->ReadFromMessageFloat(hRead) * g_HealScale.GetFloat(1.0f);

	// See if we can heal

	if (Heal(fAmount))
	{
        HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(pObject, hSender, MID_PICKEDUP);
        g_pLTServer->WriteToMessageFloat(hWrite, -1.0f);
        g_pLTServer->EndMessage(hWrite);
	}
*/
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Heal
//
//	PURPOSE:	Add some value to hit points
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::Heal(LTFLOAT fAmount)
{
	if (m_bDead) return LTFALSE;

    LTFLOAT fMax = GetMaxHitPoints();
    if (m_fHitPoints >= fMax) return LTFALSE;

	// only heal what we need to (cap hit points at maximum)

	if (m_fHitPoints + fAmount > fMax)
	{
		fAmount = fMax - m_fHitPoints;
	}

	// now actually heal the object

	DoActualHealing (fAmount);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::DoActualHealing()
//
//	PURPOSE:	Simply adds a value to the hit points variable
//
// ----------------------------------------------------------------------- //

void CDestructible::DoActualHealing(LTFLOAT fAmount)
{
	// NOTE: This function should only be called directly from the ultra heal
	//		 powerup code, as it does not do any bounds checking on the hit points.
	//       All other healing should be done through the HandleHeal() function above.

	if(!m_bCanHeal) return;

    m_bDead = LTFALSE;

	m_fHitPoints += fAmount;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleRepair()
//
//	PURPOSE:	Handle Repair message
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleRepair(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
/*    LTFLOAT fAmount = g_pLTServer->ReadFromMessageFloat(hRead);

	if (Repair(fAmount))
	{
        HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(pObject, hSender, MID_PICKEDUP);
        g_pLTServer->WriteToMessageFloat(hWrite, -1.0f);
        g_pLTServer->EndMessage(hWrite);
	}
*/
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Repair()
//
//	PURPOSE:	Add some value to armor
//
// ----------------------------------------------------------------------- //
LTBOOL CDestructible::Repair(LTFLOAT fAmount)
{
    if (!m_bCanRepair) return LTFALSE;

    LTFLOAT fMax = GetMaxArmorPoints();
    if (m_fArmorPoints >= fMax) return LTFALSE;

	m_fArmorPoints += fAmount;

	if (m_fArmorPoints > fMax)
	{
		m_fArmorPoints = fMax;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Reset
//
//	PURPOSE:	Reset
//
// ----------------------------------------------------------------------- //

void CDestructible::Reset(LTFLOAT fHitPts, LTFLOAT fArmorPts)
{
    LTFLOAT fMaxHitPoints = GetMaxHitPoints();
    LTFLOAT fMaxArmorPoints = GetMaxArmorPoints();

	m_fHitPoints	= fHitPts <= fMaxHitPoints ? fHitPts : fMaxHitPoints;
	m_fArmorPoints  = fArmorPts <= fMaxArmorPoints ? fArmorPts : fMaxArmorPoints;
	m_eDeathType	= DT_UNSPECIFIED;
	m_fDeathDamage	= 0.0f;
    m_bDead         = LTFALSE;

	m_eLastDamageType = DT_UNSPECIFIED;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleTrigger
//
//	PURPOSE:	Handle trigger message.
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleTrigger(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
	if (m_bDead) return;

    HSTRING hMsg = g_pLTServer->ReadFromMessageHString(hRead);
	if (!hMsg) return;

    char* pStr = g_pLTServer->GetStringData(hMsg);
	if(!pStr) return;


	ConParse parse;
	parse.Init(pStr);

	while (g_pLTServer->Common()->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			// See if we should destroy ourself...
			if (_stricmp(TRIGGER_MSG_DESTROY, parse.m_Args[0]) == 0)
			{
				DamageStruct damage;

				damage.fDamage	= damage.kInfiniteDamage;
				damage.hDamager = hSender;

				damage.DoDamage(pObject, pObject->m_hObject);
			}
			// See if we should damage ourself...
			else if ((parse.m_nArgs >= 2) && (_stricmp(TRIGGER_MSG_DAMAGE, parse.m_Args[0]) == 0))
			{
				DamageStruct damage;

				damage.fDamage	= (LTFLOAT)atof(parse.m_Args[1]);
				damage.hDamager = hSender;

				damage.DoDamage(pObject, pObject->m_hObject);
			}
			else if ((parse.m_nArgs >= 2) && (_stricmp(TRIGGER_MSG_CANDAMAGE, parse.m_Args[0]) == 0))
			{
				SetCanDamage( IsTrueChar(parse.m_Args[1][0]) );
			}
			else if ((parse.m_nArgs >= 2) && (_stricmp(TRIGGER_MSG_NEVERDESTROY, parse.m_Args[0]) == 0))
			{
				SetNeverDestroy( IsTrueChar(parse.m_Args[1][0]) );
			}
			// See if we should reset ourself...
			else if (_stricmp(TRIGGER_MSG_RESET, parse.m_Args[0]) == 0)
			{
				Reset(GetMaxHitPoints(), GetMaxArmorPoints());
			}
			else if ((parse.m_nArgs >= 2) && (_stricmp("SET_HEALTH", parse.m_Args[0]) == 0))
			{
				LTFLOAT fAmt = (LTFLOAT)atof(parse.m_Args[1]);

				if(fAmt > this->m_fMaxHitPoints)
					fAmt = m_fMaxHitPoints;
				else if(fAmt < 1.0f)
					fAmt = 1.0f;

				SetHitPoints(fAmt);
			}
			else if ((parse.m_nArgs >= 2) && (_stricmp("SET_ARMOR", parse.m_Args[0]) == 0))
			{
				LTFLOAT fAmt = (LTFLOAT)atof(parse.m_Args[1]);

				if(fAmt > this->m_fMaxHitPoints)
					fAmt = m_fMaxHitPoints;
				else if(fAmt < 0)
					fAmt = 0;

				SetArmorPoints(fAmt);
			}
		}
	}

    g_pLTServer->FreeString(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleCrush
//
//	PURPOSE:	Handle crush message
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleCrush(LPBASECLASS pObject, HOBJECT hSender)
{
	if (!pObject || !hSender || m_bDead) return;

	// Check if we can't be crushed.
	if( !m_bCanCrush )
		return;

    LTFLOAT fMyMass, fHisMass;
    fMyMass  = g_pLTServer->GetObjectMass(m_hObject);
    fHisMass = g_pLTServer->GetObjectMass(hSender);

    uint32 dwUsrFlg = g_pLTServer->GetObjectUserFlags(hSender);
	if (dwUsrFlg & USRFLG_CANT_CRUSH) return;

	// If the other guy is not more than 15% bigger, don't crush.
	float fMassRatio = fHisMass / ( fMyMass + 0.0001f );
	if( fMassRatio < 1.15f )
		return;

	// Calculate the damage based on the mass ratio.
    LTFLOAT fDamage = 20.0f * fMassRatio;

	DamageStruct damage;

	damage.eType	= DT_CRUSH;
	damage.fDamage	= fDamage;
	damage.hDamager = hSender;

	damage.DoDamage(pObject, pObject->m_hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleTouch
//
//	PURPOSE:	Handle object contact
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleTouch(LPBASECLASS pObject, HOBJECT hSender, LTFLOAT fForce)
{
	if (!pObject || !hSender || m_bDead || fForce < 0.0f) return;

	// Don't do touch damage...

	return;


    LTFLOAT fMyMass, fHisMass;
    fMyMass  = g_pLTServer->GetObjectMass(pObject->m_hObject);
    fHisMass = g_pLTServer->GetObjectMass(hSender);

    LTBOOL bIsWorld = (LT_YES == g_pLTServer->Physics()->IsWorldObject(hSender));

	if (bIsWorld)
	{
		fHisMass = INFINITE_MASS;

		// Don't allow the world to damage the player...

		if (IsPlayer(pObject->m_hObject)) return;
	}

    uint32 dwUsrFlg = g_pLTServer->GetObjectUserFlags(hSender);
	if (dwUsrFlg & USRFLG_CANT_CRUSH) return;

	if (fHisMass <= 0.1f || fMyMass <= 0.1f) return;

	// If we're within 15% of each other, don't do anything rash...

	if (fHisMass <= fMyMass*1.15 || fMyMass > fHisMass) return;


	// Not enough force to do anything...

    LTFLOAT fVal = bIsWorld ? fMyMass * 15.0f : fMyMass * 10.0f;
	if (fForce < fVal) return;


	// Okay, one last check.  Make sure that he is moving, if I run into him
	// that shouldn't cause anything to happen.  It is only when he runs into
	// me that I can take damage...(well, unless it is the world...

	if (!bIsWorld)
	{
        LTVector vVel;
        g_pLTServer->GetVelocity(hSender, &vVel);
		if (VEC_MAG(vVel) < 1.0f) return;
	}


	// Calculate damage...

    LTFLOAT fDamage = (fForce / fMyMass);
	fDamage *= bIsWorld ? 5.0f : 10.0f;

    //g_pLTServer->BPrint("%.2f, %.2f, %.2f, %.2f",
	//				  fHisMass, fMyMass, fForce, fDamage);

    LTVector vDir;
	VEC_INIT(vDir);

	if (bIsWorld)
	{
        g_pLTServer->GetVelocity(pObject->m_hObject, &vDir);
		VEC_NEGATE(vDir, vDir);
	}
	else
	{
        LTVector vMyPos, vHisPos;
        g_pLTServer->GetObjectPos(pObject->m_hObject, &vMyPos);
        g_pLTServer->GetObjectPos(hSender, &vHisPos);

		VEC_SUB(vDir, vMyPos, vHisPos);
	}

	vDir.Norm();

	DamageStruct damage;

	damage.eType	= DT_CRUSH;
	damage.fDamage	= fDamage;
	damage.hDamager = hSender;
	damage.vDir		= vDir;

	damage.DoDamage(pObject, pObject->m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::DebugDamageOn
//
//	PURPOSE:	See if the object can be damaged
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::DebugDamageOn()
{
    if (!m_hObject) return LTFALSE;

    LTBOOL bRet = LTFALSE;

    HCONVAR hVar  = g_pLTServer->GetGameConVar("SetDamage");
    char* pVal = g_pLTServer->GetVarValueString(hVar);

    if (!pVal) return LTFALSE;

    return ((LTBOOL) !atoi(pVal));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CDestructible::Save(HMESSAGEWRITE hWrite, uint8 nType)
{
	if (!hWrite) return;

    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hObject);
    *hWrite << m_hLastDamager;

	for( DamageHistory * pHistory = m_pHistory; pHistory < m_pHistory + MAX_DAMAGE_HISTORY; ++pHistory )
	{
		*hWrite << *pHistory;
	}

    g_pLTServer->WriteToMessageFloat(hWrite, m_fHitPoints);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fMaxHitPoints);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fArmorPoints);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fMaxArmorPoints);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fMass);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fLastDamage);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fDeathDamage);
    g_pLTServer->WriteToMessageByte(hWrite, m_bDead);
    g_pLTServer->WriteToMessageByte(hWrite, m_bApplyDamagePhysics);
    g_pLTServer->WriteToMessageByte(hWrite, m_bIsPlayer);
    g_pLTServer->WriteToMessageByte(hWrite, m_bCanHeal);
    g_pLTServer->WriteToMessageByte(hWrite, m_bCanCrush);
    g_pLTServer->WriteToMessageByte(hWrite, m_bCanRepair);
    g_pLTServer->WriteToMessageByte(hWrite, m_bCanDamage);
    g_pLTServer->WriteToMessageByte(hWrite, m_bNeverDestroy);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fMinHealth);
    g_pLTServer->WriteToMessageByte(hWrite, m_eDeathType);
    g_pLTServer->WriteToMessageByte(hWrite, m_eLastDamageType);
    g_pLTServer->WriteToMessageDWord(hWrite, m_nDamageTriggerNumSends);
    g_pLTServer->WriteToMessageDWord(hWrite, m_dwCantDamageTypes);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrDamageTriggerTarget);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrDamageTriggerMessage);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrDamagerMessage);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrDeathTriggerTarget);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrDeathTriggerMessage);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrPlayerDeathTriggerTarget);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrPlayerDeathTriggerMessage);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrKillerMessage);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vDeathDir);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vLastDamageDir);
	hWrite->WriteFloat(g_pLTServer->GetTime() - m_fLastDamageTime);
	*hWrite << m_bCanSetRayHit;
	*hWrite << m_bMassBasedBlockingPriority;

	for (int i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		m_ProgressiveDamage[i].Save(hWrite);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CDestructible::Load(HMESSAGEREAD hRead, uint8 nType)
{
	if (!hRead) return;

    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hObject);
	*hRead >> m_hLastDamager;

	for( DamageHistory * pHistory = m_pHistory; pHistory < m_pHistory + MAX_DAMAGE_HISTORY; ++pHistory )
	{
		*hRead >> *pHistory;
	}

    m_fHitPoints                = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fMaxHitPoints             = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fArmorPoints              = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fMaxArmorPoints           = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fMass                     = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fLastDamage               = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fDeathDamage              = g_pLTServer->ReadFromMessageFloat(hRead);
    m_bDead                     = g_pLTServer->ReadFromMessageByte(hRead);
    m_bApplyDamagePhysics       = g_pLTServer->ReadFromMessageByte(hRead);
    m_bIsPlayer                 = g_pLTServer->ReadFromMessageByte(hRead);
    m_bCanHeal                  = g_pLTServer->ReadFromMessageByte(hRead);
    m_bCanCrush                 = g_pLTServer->ReadFromMessageByte(hRead);
    m_bCanRepair                = g_pLTServer->ReadFromMessageByte(hRead);
    m_bCanDamage                = g_pLTServer->ReadFromMessageByte(hRead);
    m_bNeverDestroy             = g_pLTServer->ReadFromMessageByte(hRead);
	m_fMinHealth				= g_pLTServer->ReadFromMessageFloat(hRead);
    m_eDeathType                = (DamageType) g_pLTServer->ReadFromMessageByte(hRead);
    m_eLastDamageType           = (DamageType) g_pLTServer->ReadFromMessageByte(hRead);
    m_nDamageTriggerNumSends    = g_pLTServer->ReadFromMessageDWord(hRead);
    m_dwCantDamageTypes         = g_pLTServer->ReadFromMessageDWord(hRead);
    m_hstrDamageTriggerTarget   = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrDamageTriggerMessage  = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrDamagerMessage        = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrDeathTriggerTarget    = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrDeathTriggerMessage   = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrPlayerDeathTriggerTarget  = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrPlayerDeathTriggerMessage = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrKillerMessage         = g_pLTServer->ReadFromMessageHString(hRead);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vDeathDir);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vLastDamageDir);
	m_fLastDamageTime = g_pLTServer->GetTime() - hRead->ReadFloat();
	*hRead >> m_bCanSetRayHit;
	*hRead >> m_bMassBasedBlockingPriority;

	for (int i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		m_ProgressiveDamage[i].Load(hRead);
	}
}

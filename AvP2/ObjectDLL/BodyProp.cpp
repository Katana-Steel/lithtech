// ----------------------------------------------------------------------- //
//
// MODULE  : BodyProp.cpp
//
// PURPOSE : Model BodyProp - Definition
//
// CREATED : 6/2/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>

#include "BodyProp.h"
#include "cpp_server_de.h"
#include "WeaponFXTypes.h"
#include "ClientServerShared.h"
#include "SFXMsgIds.h"
#include "ClientDeathSFX.h"
#include "GameServerShell.h"
#include "transformlt.h"
#include "modellt.h"
#include "SurfaceFunctions.h"
#include "ObjectMsgs.h"
#include "ServerSoundMgr.h"
#include "CharacterHitBox.h"
#include "SharedFXStructs.h"
#include "Attachments.h"
#include "CharacterMgr.h"
#include "DebrisFuncs.h"
#include "CharacterFuncs.h"
#include "AI.h"
#include "AIVolumeMgr.h"
#include "MsgIDs.h"
#include "PlayerObj.h"
#include "Explosion.h"
#include "MultiplayerMgr.h"
#include "ServerSoundMgr.h"
#include "SoundButeMgr.h"
#include "AISenseMgr.h"
#include "PickupObject.h"

// ----------------------------------------------------------------------- //
// Externs

extern CGameServerShell* g_pGameServerShell;


// ----------------------------------------------------------------------- //
// LT Class Defs

BEGIN_CLASS(BodyProp)
	ADD_LONGINTPROP_FLAG(DeathType, 0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG_HELP(CharacterType, "Harris", PF_STATICLIST, "Type of character." )
	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS | PF_LOCALDIMS | PF_FILENAME | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME | PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS_PLUGIN(BodyProp, Prop, LTNULL, LTNULL, 0, BodyPropPlugin)


// ----------------------------------------------------------------------- //
// Static

static CVarTrack g_BodyLifetimeTrack;
static CVarTrack g_BodyFadeOffEffectTime;
static CVarTrack g_BodyLimbTrajectory;
static CVarTrack g_BodyLimbDirScale;
static CVarTrack g_BodyDamageHistoryTime;

static CVarTrack g_FaceHugImplantTime;
static CVarTrack g_FaceHugGestationTime;
static CVarTrack g_ChestBurstVelocity;

static CVarTrack g_cvarAcidDamageDelay;
static CVarTrack g_cvarMinAcidPoolDamage;
static CVarTrack g_cvarMaxAcidPoolDamage;
static CVarTrack g_cvarAcidPoolRad;

extern CVarTrack g_cvarAcidHitRad;

static LTFLOAT s_fUpdateDelta			= 0.1f;
static LTFLOAT s_fFreezeTime			= 5.0f;
static LTFLOAT s_fVaporizeTime			= 2.0f;
static LTFLOAT s_fHitPtsAdjustFactor	= 2.0f;

static char s_szKeyNoise[]				= "NOISE";
static char s_szKeyChestburstFX[]		= "CHESTBURSTFX";
static char s_szKeyChestburst[]			= "CHESTBURST";
static uint32  s_nMaxClawImpacts		= 14;

const static char s_szMessageFall[]		= "FALL ";
const static int s_nMessageFallLen		= strlen(s_szMessageFall);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::BodyProp()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

BodyProp::BodyProp() : Prop()
{
	m_nCharacterSet			= 0;
	m_eModelSkeleton		= eModelSkeletonInvalid;
	m_pAttachments			= LTNULL;

	m_fHeadBiteHealPoints	= 0.0f;
	m_fClawHealPoints		= 0.0f;

	m_hLastDamager			= LTNULL;
	m_eNodeLastHit			= eModelNodeInvalid;
	m_eDamageType			= DT_UNSPECIFIED;
	m_eDeathType			= CD_NORMAL;
	m_vDeathDir				= LTVector(0.0f, -1.0f, 0.0f);

	m_hDeathAnim			= INVALID_ANI;
	m_hStaticDeathAnim		= INVALID_ANI;

	m_hFaceHugger			= LTNULL;
	m_hFaceHuggerAttachment	= LTNULL;

	m_bCanGIB				= LTTRUE;
	m_bLowViolenceGIB		= LTFALSE;

	m_hstrInitialMessage	= LTNULL;

	m_hHitBox				= LTNULL;
	m_vColor				= LTVector(0.0f, 0.0f, 0.0f);

	m_fStartTime			= 0.0f;
	m_fFaceHugTime			= 0.0f;
	m_fAdjustFactor			= s_fHitPtsAdjustFactor;
	m_bFirstUpdate			= LTTRUE;
	m_bLimbProp				= LTFALSE;
	m_bSliding				= LTFALSE;
	m_bFadeAway				= LTTRUE;
	m_pCurrentVolume		= LTNULL;
	m_bDimsSet				= LTFALSE;
	m_bAcidPool				= LTFALSE;
	m_fLastAcidSpew			= 0.0f;
	m_fLastDamage			= 0.0f;
	m_bFlying				= LTFALSE;
	m_bFadeIn				= LTFALSE;
	m_fDecloakStartTime		= 0.0f;
	m_cSpears				= 0;
	m_bUpdateGravity		= LTTRUE;

	m_bHeadAvailable		= LTTRUE;
	m_nClawHits				= 0;

	m_vOrigDims.Init();

	// Move to the floor on the first update.
    SetMoveToFloor( LTFALSE );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::~BodyProp()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

BodyProp::~BodyProp()
{

	if (m_pAttachments) 
	{
		m_pAttachments->HandleDeath();

		delete m_pAttachments; 
		m_pAttachments = LTNULL;
	}

	HOBJECT hAttachList[30];
	uint32 dwListSize, dwNumAttachments;

	// At this point there should be no attachments so go ahead and nuke em...
	if (g_pLTServer->Common()->GetAttachments(m_hObject, hAttachList, 
		ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
	{
		if(dwNumAttachments != 0)
		{
#ifndef _FINAL
			g_pLTServer->CPrint("WARNING: BodyProp still has attachments (%d) upon death.", dwNumAttachments);
#endif
			for (uint32 i = 0; i < dwNumAttachments; i++)
			{
				HATTACHMENT hAttachment;
				if (g_pLTServer->FindAttachment(m_hObject, hAttachList[i], &hAttachment) == LT_OK)
				{
					g_pLTServer->RemoveAttachment(hAttachment);

				}
				g_pLTServer->RemoveObject(hAttachList[i]);
			}
		}
	}


	FREE_HSTRING(m_hstrInitialMessage);

	if (m_hHitBox)
	{
		g_pLTServer->RemoveObject(m_hHitBox);
	}

	// remove it from the list
	g_pCharacterMgr->RemoveDeadBody(this);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::Setup()
//
//	PURPOSE:	Setup the object with all the information needed from
//				the character model
//
// ----------------------------------------------------------------------- //

void BodyProp::Setup(CharInfo *pCharInfo, LTBOOL bCarryOverAttachments /* = LTTRUE */, std::string strDefaultDeathAnim /* = std::string() */ )
{
	if (!pCharInfo || !pCharInfo->pDest || !pCharInfo->hObj)
		return;

	CDestructible *pDest	= pCharInfo->pDest;
	m_nCharacterSet			= pCharInfo->nCharacterSet;
	
	const CharacterButes & butes	= g_pCharacterButeMgr->GetCharacterButes(m_nCharacterSet);
	m_eModelSkeleton		= (ModelSkeleton)butes.m_nSkeleton;
	m_fHeadBiteHealPoints	= butes.m_fHeadBiteHealPoints;
	m_fClawHealPoints		= butes.m_fClawHealPoints;

	m_eNodeLastHit			= eModelNodeInvalid;

	m_eDamageType			= pDest->GetDeathType();

	// Change the head bite to a slice if this character isn't "bite-able".
	if( !IsHuman(pCharInfo->nCharacterSet) && !IsPredator(pCharInfo->nCharacterSet) )
	{
		if( m_eDamageType == DT_HEADBITE )
		{
			m_eDamageType = DT_ALIEN_CLAW;
		}
	}

	m_eDeathType			= GetDeathType(m_eDamageType);

	// Special case for exosuits...
	if(IsExosuit(pCharInfo->nCharacterSet))
	{
		m_eDeathType = CD_GIB;
		m_bLowViolenceGIB = LTTRUE;
	}

	m_damage.SetApplyDamagePhysics(LTFALSE);

	if( strDefaultDeathAnim.empty() )
	{
		strDefaultDeathAnim = g_pModelButeMgr->GetSkeletonDefaultDeathAni(m_eModelSkeleton);
		if(m_eNodeLastHit != eModelNodeInvalid)
			strDefaultDeathAnim = g_pModelButeMgr->GetSkeletonNodeDeathAniFrag(m_eModelSkeleton, m_eNodeLastHit);
	}

	Setup(pCharInfo->hObj, pDest, bCarryOverAttachments, strDefaultDeathAnim, LTFALSE);

	return;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void BodyProp::Setup(CCharacter *pChar, LTBOOL bCarryOverAttachments, std::string strDefaultDeathAnim, LTBOOL bForceDefaultDeathAnim )
{
	if (!pChar || !pChar->m_hObject)
		return;

	CDestructible *pDest = pChar->GetDestructible();
	if (!pDest)
		return;

	m_hLastDamager			= pDest->GetLastDamager();
	m_nCharacterSet			= pChar->GetCharacter();
	m_eModelSkeleton		= pChar->GetModelSkeleton();

	m_fHeadBiteHealPoints	= pChar->GetButes()->m_fHeadBiteHealPoints;
	m_fClawHealPoints		= pChar->GetButes()->m_fClawHealPoints;

	m_eNodeLastHit			= pChar->GetModelNodeLastHit();
	m_eDamageType			= pDest->GetDeathType();
	m_eDeathType			= GetDeathType(m_eDamageType);
	m_pCurrentVolume		= pChar->GetCurrentVolumePtr();

	// Completely gib body if it was wall walking.
	if(    ( pChar->GetMovement()->GetMovementState() == CMS_WALLWALKING && pChar->GetUp().y < c_fCos20 )
		|| IsExosuit(m_nCharacterSet) )
	{
		m_eDeathType = CD_GIB;
		m_bLowViolenceGIB = LTTRUE;

		if(m_eModelSkeleton != eModelSkeletonInvalid)
		{
			m_eNodeLastHit = g_pModelButeMgr->GetSkeletonNode(m_eModelSkeleton, "Torso_u_node");
		}
	}

	m_damage.SetApplyDamagePhysics(LTTRUE);

	Setup(pChar->m_hObject, pDest, bCarryOverAttachments, strDefaultDeathAnim, bForceDefaultDeathAnim );
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// Private

void BodyProp::Setup(HOBJECT hObj, CDestructible *pDest, LTBOOL bCarryOverAttachments, std::string strDefaultDeathAnim, LTBOOL bForceDefaultDeathAnim )
{
	if(!pDest || !hObj) 
		return;

	const CharacterButes & butes	= g_pCharacterButeMgr->GetCharacterButes(m_nCharacterSet);

	// ----------------------------------------------------------------------- //
	// Be sure predator has mask hidden
	if(IsPredator(butes))
	{
		HMODELPIECE hPiece;
		g_pModelLT->GetPiece(m_hObject, "Head_Mask", hPiece);
		g_pModelLT->SetPieceHideStatus(m_hObject, hPiece, LTTRUE);
	}

	if(IsAlien(butes))
	{
		//match all the hide status'
		for(int i=0; i<32 ; i++)
		{
			LTBOOL bHidden;
			g_pModelLT->GetPieceHideStatus(hObj, (HMODELPIECE)i, bHidden);
			g_pModelLT->SetPieceHideStatus(m_hObject, (HMODELPIECE)i, bHidden);
		}
	}

	// ----------------------------------------------------------------------- //
	// Setup the damage properties
	LTFLOAT fHitPts = pDest->GetMaxHitPoints() * m_fAdjustFactor;
	m_damage.Reset(fHitPts, 0.0f);
	m_damage.SetHitPoints(fHitPts);
	m_damage.SetMaxHitPoints(fHitPts);
	m_damage.SetArmorPoints(0.0f);
	m_damage.SetMaxArmorPoints(0.0f);
	m_damage.SetCanDamage(LTFALSE);//pChar->CanDamageBody());
	m_damage.SetCanCrush( LTTRUE );
//	m_damage.SetApplyDamagePhysics(pChar->CanDamageBody()); moved to caller
	m_damage.SetMass(butes.m_fMass);
	m_damage.SetDebrisId(DEBRISMGR_INVALID_ID);		// We'll handle creating the necessary debris...
	m_fLastDamage = pDest->GetLastDamage();

	for(int i=0; i<MAX_PROGRESSIVE_DAMAGE ; i++)
	{
		m_damage.SetProgressiveDamage(i, pDest->GetProgressiveDamage(i));
	}

	//clear up the old destructible
	pDest->ClearProgressiveDamages();


	// Record the death direction...
	m_vDeathDir = pDest->GetDeathDir();

	// Set the gib override...
	m_bCanGIB = g_pCharacterButeMgr->GetCanLoseLimb(m_nCharacterSet);

	// Special case for stair volumes
	if (m_pCurrentVolume)
	{
		if (m_pCurrentVolume->IsStair())
			m_vDeathDir = m_pCurrentVolume->GetForward();
	}

	m_vDeathDir.Norm();


	// ----------------------------------------------------------------------- //
	// Make sure the color is the same as the character
	LTFLOAT r, g, b, a;
	g_pLTServer->GetObjectColor(hObj, &r, &g, &b, &a);
	g_pLTServer->SetObjectColor(m_hObject, r, g, b, a);
	if(a != 1.0f)
	{
		m_bFadeIn			= LTTRUE;
		m_fDecloakStartTime	= g_pLTServer->GetTime();
	}


	// ----------------------------------------------------------------------- //
	// Make sure the scale is the same as the character
	LTVector vScale;
	g_pLTServer->GetObjectScale(hObj, &vScale);
	g_pLTServer->ScaleObject(m_hObject, &vScale);


	// ----------------------------------------------------------------------- //
	// Get the current flags of the character
	uint32 dwFlags, dwFlags2;
	uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(hObj);

	g_pLTServer->Common()->GetObjectFlags(hObj, OFT_Flags, dwFlags);
	g_pLTServer->Common()->GetObjectFlags(hObj, OFT_Flags2, dwFlags2);

	// Make some flag adjustments that we know we'll need
	dwFlags = dwFlags & ~FLAG_SOLID & ~FLAG_GOTHRUWORLD & ~FLAG_SHADOW & ~FLAG_STAIRSTEP;
	dwFlags |= FLAG_GRAVITY | FLAG_VISIBLE | FLAG_REMOVEIFOUTSIDE | FLAG_NOSLIDING;

	dwFlags2 = 0;

	// Need to do this here and after the call to SetupByDeathType... this is
	// so that the flags are set properly for limbs that might need to be created
	// during that setup process.
	g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags2, dwFlags2);
	g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);

	SetupByDeathType(dwFlags, dwUserFlags);

	g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);

	// Now make sure we are not activateable...  Bodys are never activateable...
	SetActivateType(m_hObject, AT_NOT_ACTIVATABLE);

	// ----------------------------------------------------------------------- //
	// See if we get a special limb explosion due to lots of damage
	if(	m_eDeathType == CD_NORMAL || 
		m_eDeathType == CD_SHOTGUN || 
		(m_eDeathType == CD_BULLET_NOGIB && IsAlien(m_nCharacterSet)) )
	{
		if(m_eNodeLastHit != eModelNodeInvalid)
		{
			LTFLOAT fHistDamage = pDest->GetHistoryDamageAmount(g_BodyDamageHistoryTime.GetFloat());
			LTFLOAT fFactor = g_pModelButeMgr->GetSkeletonNodeDamageFactor(m_eModelSkeleton, m_eNodeLastHit);

			fHistDamage /= fFactor;

			if(g_pModelButeMgr->ExplodeNode(m_eModelSkeleton, m_eNodeLastHit, fHistDamage))
			{
				LTVector vVelOffset;
				g_pLTServer->GetVelocity(hObj, &vVelOffset);
				ExplodeLimb(m_eNodeLastHit, LTFALSE, vVelOffset);

				DoSpecialDamageSound(m_hLastDamager);
			}
		}
	}


	// ----------------------------------------------------------------------- //
	// Set our friction values
	LTFLOAT fFriction = 0.0f;
	g_pLTServer->Physics()->GetFrictionCoefficient(hObj, fFriction);
	g_pLTServer->Physics()->SetFrictionCoefficient(m_hObject, fFriction * 2.0f);


	// ----------------------------------------------------------------------- //
	// Set our dimensions
	LTVector vDims;
	g_pLTServer->GetObjectDims(hObj, &vDims);

	// If we can't set the dims that big, set them as big as possible...
	m_vOrigDims = vDims;
	vDims.x = vDims.z = 5.0f;
	g_pLTServer->Physics()->SetObjectDims(m_hObject, &vDims, 0);


	// HACK
	CCharacter *pChar = dynamic_cast<CCharacter *>(g_pInterface->HandleToObject(hObj));

	// ----------------------------------------------------------------------- //
	// Transfer any attachments from the character...
	if(!m_pAttachments && bCarryOverAttachments)
	{
		if (pChar)
		{
			pChar->TransferSpears(this);
			m_pAttachments = pChar->TransferAttachments();
		}

		delete m_pAttachments;
		m_pAttachments = LTNULL;
	}


	// ----------------------------------------------------------------------- //
	// Setup the death and static death animations
	char szBuffer[64];
	const char* pDeathAnim = LTNULL;

	if( bForceDefaultDeathAnim )
	{
		pDeathAnim = strDefaultDeathAnim.c_str();
	}

	if( !pDeathAnim )
	{
		pDeathAnim = GetSpecialDeathTypeAnim(m_eDeathType);

		if(!pDeathAnim)
		{
			pDeathAnim = strDefaultDeathAnim.c_str();
		}
	}

	if(pDeathAnim)
	{
		if(stricmp("Flying", pDeathAnim)==0)
		{
			m_hDeathAnim = g_pLTServer->GetAnimIndex(m_hObject, "Death_f_Thrw_Start0");
			m_hStaticDeathAnim = g_pLTServer->GetAnimIndex(m_hObject, "Death_f_Thrw_end0");
		}
		else
		{
			m_hDeathAnim = g_pLTServer->GetAnimIndex(m_hObject, const_cast<char*>(pDeathAnim));
			sprintf(szBuffer, "%s_S", pDeathAnim);
			m_hStaticDeathAnim = g_pLTServer->GetAnimIndex(m_hObject, szBuffer);
		}

		// If we didn't find this animation... try again with the default
		if(m_hDeathAnim == INVALID_ANI)
		{
			pDeathAnim = g_pModelButeMgr->GetSkeletonDefaultDeathAni(m_eModelSkeleton);
			m_hDeathAnim = g_pLTServer->GetAnimIndex(m_hObject, const_cast<char*>(pDeathAnim));
			sprintf(szBuffer, "%s_S", pDeathAnim);
			m_hStaticDeathAnim = g_pLTServer->GetAnimIndex(m_hObject, szBuffer);
		}

		// If we're ok... then go ahead and play the animation
		if(m_hDeathAnim != INVALID_ANI)
		{
			g_pLTServer->SetModelAnimation(m_hObject, m_hDeathAnim);
			g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
		}
	}

	// Play the death sound
	if(pChar && !HeadHidden() )
	{
		g_pServerSoundMgr->PlayCharSndFromObject(pChar->GetButes()->m_nId, m_hObject, GetSpecialDeathTypeSound(m_eDeathType));

		// Notify the AI's about death sound
		if( g_pGameServerShell->GetGameType().IsSinglePlayer() )
		{
			const CString strSoundSetName = g_pServerSoundMgr->GetSoundSetName( g_pCharacterButeMgr->GetGeneralSoundDir(pChar->GetButes()->m_nId), GetSpecialDeathTypeSound(m_eDeathType) );

			const int nDeathSoundBute = g_pSoundButeMgr->GetSoundSetFromName( strSoundSetName );
			if(nDeathSoundBute >= 0) 
			{
				const SoundButes & death_bute = g_pSoundButeMgr->GetSoundButes(nDeathSoundBute);

				for( CCharacterMgr::AIIterator iter = g_pCharacterMgr->BeginAIs(); iter != g_pCharacterMgr->EndAIs(); ++iter )
				{
					(*iter)->GetSenseMgr().Death(death_bute,this);
				}
			}
		}
	}

	//now send the FX message to the players
	BODYPROPCREATESTRUCT cs;

	cs.nCharacterSet	= (uint8)m_nCharacterSet;
	cs.bBloodPool		= CanDoBloodPool();
	cs.bBloodTrail		= LTFALSE;

	HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_BODYPROP_ID);
	cs.Write(g_pLTServer, hMessage);
	g_pLTServer->EndMessage(hMessage);

	// See about doing the acid damage...
	m_bAcidPool = (cs.bBloodPool && IsAlien(m_nCharacterSet));

	// Make sure we stop if we are not flying
	if(!m_bFlying)
	{
		MoveObjectToFloor(m_hObject, LTFALSE, LTFALSE);
		dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
		dwFlags &= ~FLAG_GRAVITY;
		g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
		g_pLTServer->SetVelocity(m_hObject, &LTVector(0,0,0));
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::SetupLimb()
//
//	PURPOSE:	Setup the object with all the information needed from
//				the base BodyProp
//
// ----------------------------------------------------------------------- //

void BodyProp::SetupLimb(BodyProp *pProp, ModelNode eParent, LTBOOL bFirey)
{
	if(!pProp || !pProp->m_hObject) return;


	CDestructible* pDest = pProp->GetDestructible();

	// ----------------------------------------------------------------------- //
	// Get information that we need from the character class
	m_hLastDamager			= pDest->GetLastDamager();
	m_nCharacterSet			= pProp->GetCharacter();
	m_eModelSkeleton		= pProp->GetModelSkeleton();

	m_fHeadBiteHealPoints	= pProp->GetHeadBiteHealPoints() * 0.33f;
	m_fClawHealPoints		= pProp->GetClawHealPoints() * 0.33f;

	m_eNodeLastHit			= eParent;
	m_eDamageType			= DT_UNSPECIFIED;
	m_eDeathType			= CD_NORMAL;

	m_bLimbProp				= LTTRUE;


	// ----------------------------------------------------------------------- //
	// Setup the damage properties
	m_damage.Reset(pProp->GetMaxHitPoints() * 0.5f, 0.0f);
	m_damage.SetHitPoints(pProp->GetMaxHitPoints() * 0.5f);
	m_damage.SetMaxHitPoints(pProp->GetMaxHitPoints() * 0.5f);
	m_damage.SetArmorPoints(0.0f);
	m_damage.SetMaxArmorPoints(0.0f);
	m_damage.SetCanDamage(LTFALSE);//LTTRUE);
	m_damage.SetCanCrush( LTTRUE );
	m_damage.SetApplyDamagePhysics(LTTRUE);
	m_damage.SetMass(pProp->GetMass() * 0.25f);
	m_damage.SetDebrisId(DEBRISMGR_INVALID_ID);		// We'll handle creating the necessary debris...

	for(int i=0; i<MAX_PROGRESSIVE_DAMAGE ; i++)
	{
		m_damage.SetProgressiveDamage(i, pDest->GetProgressiveDamage(i));
	}


	// ----------------------------------------------------------------------- //
	// Make sure the color is the same as the character
	LTFLOAT r, g, b, a;
	g_pLTServer->GetObjectColor(pProp->m_hObject, &r, &g, &b, &a);
	g_pLTServer->SetObjectColor(m_hObject, r, g, b, a);


	// ----------------------------------------------------------------------- //
	// Make sure the scale is the same as the character
	LTVector vScale;
	g_pLTServer->GetObjectScale(pProp->m_hObject, &vScale);
	g_pLTServer->ScaleObject(m_hObject, &vScale);


	// ----------------------------------------------------------------------- //
	// Get the current flags of the character
	uint32 dwFlags = g_pLTServer->GetObjectFlags(pProp->m_hObject);
	uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(pProp->m_hObject);

	dwFlags |= FLAG_GRAVITY | FLAG_VISIBLE | FLAG_REMOVEIFOUTSIDE;
	dwFlags &= ~FLAG_SHADOW;

	g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
	g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);


	// ----------------------------------------------------------------------- //
	// Set up a' flame if we're supposed to
#ifndef _DEMO
	if(bFirey && !GetRandom(3))
	{
		DamageStruct damage;
		damage.eType = DT_NAPALM;
		damage.fDamage = 1.0f;
		damage.fDuration = 10.0f;
		damage.DoDamage(this, m_hObject);
	}
#endif


	// ----------------------------------------------------------------------- //
	// Set our friction values
	LTFLOAT fFriction = 0.0f;
	g_pLTServer->Physics()->GetFrictionCoefficient(pProp->m_hObject, fFriction);
	g_pLTServer->Physics()->SetFrictionCoefficient(m_hObject, fFriction * 0.5f);


	// ----------------------------------------------------------------------- //
	// Setup the animation for this limb
	const char* pLimbAnim = g_pModelButeMgr->GetSkeletonNodeLimbAni(m_eModelSkeleton, m_eNodeLastHit);
	m_hDeathAnim = g_pLTServer->GetAnimIndex(m_hObject, const_cast<char*>(pLimbAnim));

	// If we're ok... then go ahead and set the animation
	if(m_hDeathAnim != INVALID_ANI)
	{
		g_pLTServer->SetModelAnimation(m_hObject, m_hDeathAnim);
		g_pLTServer->SetModelLooping(m_hObject, LTFALSE);

		// ----------------------------------------------------------------------- //
		// Set our dimensions
		LTVector vDims;
		g_pLTServer->Common()->GetModelAnimUserDims(m_hObject, &vDims, m_hDeathAnim);

		m_vOrigDims = vDims;
		vDims.x = vDims.z = 5.0f;
		g_pLTServer->Physics()->SetObjectDims(m_hObject, &vDims, 0);
	}

	//be sure to tell the players
	BODYPROPCREATESTRUCT cs;

	cs.nCharacterSet = (uint8)GetCharacter();
	cs.bLimbProp = m_bLimbProp;

	HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_BODYPROP_ID);
	cs.Write(g_pLTServer, hMessage);
	g_pLTServer->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::ComputeDamageModifier
//
//	PURPOSE:	Adjust the amount of damage based on the node hit...
//
// ----------------------------------------------------------------------- //

LTFLOAT BodyProp::ComputeDamageModifier(ModelNode eModelNode) const
{
	return g_pModelButeMgr->GetSkeletonNodeDamageFactor(GetModelSkeleton(), eModelNode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::AddSpear
//
//	PURPOSE:	Stick a spear into us
//
// ----------------------------------------------------------------------- //

void BodyProp::AddSpear(HOBJECT hSpear, const LTRotation& rRot, ModelNode eModelNode)
{
	if(eModelNode == eModelNodeInvalid)
	{
		g_pLTServer->RemoveObject(hSpear);
		return;
	}

	if(m_eModelSkeleton == eModelSkeletonInvalid)
	{
		g_pLTServer->RemoveObject(hSpear);
		return;			
	}

	char* szNode = (char *)g_pModelButeMgr->GetSkeletonNodeName(m_eModelSkeleton, eModelNode);


	// Get the node transform because we need to make rotation relative

	LTVector vPos;
	LTRotation rRotNode;
	if(GetNodePosition(eModelNode, vPos, &rRotNode))
	{
		LTRotation rAttachment = ~rRotNode*rRot;
		LTVector vAttachment, vNull;
		g_pMathLT->GetRotationVectors(rAttachment, vNull, vNull, vAttachment);
		vAttachment *= -10.0f;

		HATTACHMENT hAttachment;

		if ( LT_OK == g_pLTServer->CreateAttachment(m_hObject, hSpear,
			szNode, &vAttachment, &rAttachment, &hAttachment) )
		{
			m_aSpears[m_cSpears].hObject = hSpear;
			m_aSpears[m_cSpears].eModelNode = eModelNode;
			m_aSpears[m_cSpears].rRot = rRot;
			m_cSpears++;

			return;
		}
	}

	// Unless we actually stuck the spear into us, we'll fall through into here.

	g_pLTServer->RemoveObject(hSpear);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::AddNet
//
//	PURPOSE:	Add a net around the body
//
// ----------------------------------------------------------------------- //

void BodyProp::AddNet(HOBJECT hNet, const LTRotation& rRot, ModelNode eModelNode)
{
	if(eModelNode == eModelNodeInvalid)
	{
		g_pLTServer->RemoveObject(hNet);
		return;
	}

	if(m_eModelSkeleton == eModelSkeletonInvalid)
	{
		g_pLTServer->RemoveObject(hNet);
		return;			
	}

	char* szNode = (char *)g_pModelButeMgr->GetSkeletonNodeName(m_eModelSkeleton, eModelNode);


	// Get the node transform because we need to make rotation relative

	HMODELNODE hNode;
	if (szNode && LT_OK == g_pModelLT->GetNode(m_hObject, szNode, hNode) )
	{
		LTransform transform;
		if ( LT_OK == g_pModelLT->GetNodeTransform(m_hObject, hNode, transform, LTTRUE) )
		{
			LTRotation rRotNode;
			if ( LT_OK == g_pTransLT->GetRot(transform, rRotNode) )
			{
				LTRotation rAttachment = ~rRotNode*rRot;
				HATTACHMENT hAttachment;
				g_pLTServer->CreateAttachment(m_hObject, hNet, szNode, &LTVector(0,0,0), &rAttachment, &hAttachment);
				return;
			}
		}
	}

	//if we made it to here somthing is wrong
	g_pLTServer->RemoveObject(hNet);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::HideAllPieces
//
//	PURPOSE:	Goes through all model pieces and hides them
//
// ----------------------------------------------------------------------- //

void BodyProp::HideAllPieces()
{
	// Lithtech assumes that a model has no more than 32 pieces
	for(int i = 0; i < 32; i++)
	{
		g_pModelLT->SetPieceHideStatus(m_hObject, (HMODELPIECE)i, LTTRUE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::AllPiecesHidden
//
//	PURPOSE:	Goes through all model pieces and sees if they're all hidden
//
// ----------------------------------------------------------------------- //

LTBOOL BodyProp::AllPiecesHidden()
{
	LTBOOL bHidden = LTTRUE;

	// Lithtech assumes that a model has no more than 32 pieces
	for(int i = 0; i < 32; i++)
	{
		g_pModelLT->GetPieceHideStatus(m_hObject, (HMODELPIECE)i, bHidden);

		if(!bHidden)
			return LTFALSE;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::HeadHidden
//
//	PURPOSE:	Determines if the head is hidden or not.
//
// ----------------------------------------------------------------------- //

LTBOOL BodyProp::HeadHidden()
{
	LTBOOL bHidden = LTFALSE;

	HMODELPIECE hPiece;
	if( LT_OK == g_pModelLT->GetPiece(m_hObject, "Head", hPiece) )
	{
		if( LT_OK == g_pModelLT->GetPieceHideStatus(m_hObject, hPiece, bHidden) )
			return bHidden;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 BodyProp::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_LINKBROKEN:
		{
			HOBJECT hLink = (HOBJECT)pData;

			// See if this was a stuck spear...
			HATTACHMENT hAttachment;
			if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hLink, &hAttachment) )
			{
				if(g_pLTServer->RemoveAttachment(hAttachment)!=LT_OK)
					ASSERT(0);
				break;
			}

			// See if this was a skewered head...
			PickupObject* pPU = dynamic_cast<PickupObject*>(g_pLTServer->HandleToObject(hLink));

			if(pPU)
			{
				HOBJECT hPlayer = pPU->GetSender();

				if(hPlayer && IsPredator(hPlayer))
				{
					//  Increment our skull count if applicable.
					CPlayerObj* pPlayer = (CPlayerObj*)g_pServerDE->HandleToObject(hPlayer);

					if(pPlayer)
					{
						HMESSAGEWRITE hMsg = g_pInterface->StartMessage(pPlayer->GetClient(), MID_ADD_SKULL2);
						g_pInterface->EndMessage(hMsg);
					}

					// Now send a trigger message to any event counter
					ObjArray<HOBJECT,1> objArray;
					if( LT_OK == g_pServerDE->FindNamedObjects( "TrophyCounter", objArray ) )
					{
						if( objArray.NumObjects() > 0 )
						{
							SendTriggerMsgToObject(this, 	objArray.GetObject(0), "INC");
						}
					}
				}

				// Now go away...
				g_pLTServer->RemoveObject(m_hObject);
			}
		}
		break;

		case MID_UPDATE:
		{
			// DON'T call Prop::EngineMessageFn, object might get removed...
			uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			Update();

			return dwRet;
		}
		break;

		// [KLS] 10/2/01 - Since HandleModelString() is a virtual function, Prop's
		// EngineMessageFn() will call it, calling it here causes it to be called
		// twice!
		//case MID_MODELSTRINGKEY:
		//{
		//	HandleModelString((ArgList*)pData);
		//}
		//break;

		case MID_PRECREATE:
		{
			if (fData == 1.0f || fData == 2.0f)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			// Handle the message in the base class and make sure we
			// get a next update
			uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);
			SetNextUpdate(s_fUpdateDelta, s_fUpdateDelta + 1.0f);


			// Make sure the our console variables are initted
			if(!g_BodyLifetimeTrack.IsInitted())
				g_BodyLifetimeTrack.Init(g_pLTServer, "BodyLifetime", NULL, 30.0f);

			if(!g_BodyFadeOffEffectTime.IsInitted())
				g_BodyFadeOffEffectTime.Init(g_pLTServer, "BodyFadeOffTime", NULL, 3.0f);

			if(!g_BodyLimbTrajectory.IsInitted())
				g_BodyLimbTrajectory.Init(g_pLTServer, "BodyLimbTrajectory", NULL, 275.0f);

			if(!g_BodyLimbDirScale.IsInitted())
				g_BodyLimbDirScale.Init(g_pLTServer, "BodyLimbDirScale", NULL, 1.25f);

			if(!g_BodyDamageHistoryTime.IsInitted())
				g_BodyDamageHistoryTime.Init(g_pLTServer, "BodyDamageHistoryTime", NULL, 0.2f);

			if(!g_FaceHugImplantTime.IsInitted())
				g_FaceHugImplantTime.Init(g_pLTServer, "FaceHugImplantTime", NULL, 10.0);

			if(!g_FaceHugGestationTime.IsInitted())
				g_FaceHugGestationTime.Init(g_pLTServer, "FaceHugGestationTime", NULL, 5.0);

			if(!g_ChestBurstVelocity.IsInitted())
				g_ChestBurstVelocity.Init(g_pLTServer, "ChestBurstVelocity", NULL, 350.0);

			if(!g_cvarAcidDamageDelay.IsInitted())
				g_cvarAcidDamageDelay.Init(g_pLTServer, "AcidDamageDelay", NULL, 0.5f);

			if(!g_cvarMaxAcidPoolDamage.IsInitted())
				g_cvarMaxAcidPoolDamage.Init(g_pLTServer, "AcidPoolDamageMax", NULL, 8.0f);

			if(!g_cvarMinAcidPoolDamage.IsInitted())
				g_cvarMinAcidPoolDamage.Init(g_pLTServer, "AcidPoolDamageMin", NULL, 5.0f);

			if(!g_cvarAcidPoolRad.IsInitted())
				g_cvarAcidPoolRad.Init(g_pLTServer, "AcidPoolRadius", NULL, 50.0f);

			// If we're not fading... that means we were placed from DEdit, so we can just
			// send the SFX here instead of in the setup or setuplimb functions
			if(!m_bFadeAway)
			{
				// Set our file names...
				ObjectCreateStruct createStruct;
				createStruct.Clear();
				g_pCharacterButeMgr->GetDefaultFilenames(m_nCharacterSet, createStruct);
				g_pLTServer->Common()->SetObjectFilenames(m_hObject, &createStruct);

				// Set our animations...
				char szBuffer[64];
				const char* pDeathAnim = LTNULL;
				pDeathAnim = g_pModelButeMgr->GetSkeletonDefaultDeathAni(m_eModelSkeleton);
				m_hDeathAnim = g_pLTServer->GetAnimIndex(m_hObject, const_cast<char*>(pDeathAnim));
				sprintf(szBuffer, "%s_S", pDeathAnim);
				m_hStaticDeathAnim = g_pLTServer->GetAnimIndex(m_hObject, szBuffer);

				// Go ahead and set our animation...
				g_pLTServer->SetModelAnimation(m_hObject, m_hStaticDeathAnim);
				g_pLTServer->SetModelLooping(m_hObject, LTFALSE);

				// Tell the client about us...
				BODYPROPCREATESTRUCT cs;
				cs.nCharacterSet = (uint8)m_nCharacterSet;

				HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
				g_pLTServer->WriteToMessageByte(hMessage, SFX_BODYPROP_ID);
				cs.Write(g_pLTServer, hMessage);
				g_pLTServer->EndMessage(hMessage);
			}

			// Add this body to the dead body list
			g_pCharacterMgr->AddDeadBody(this);

			return dwRet;
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			if(m_bFlying)
			{
				HOBJECT hObj = (HOBJECT)pData;
				uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);

				if (dwFlags & FLAG_SOLID)
				{
					// Ok finish up flying
					m_bFlying = LTFALSE;

					g_pLTServer->SetModelAnimation(m_hObject, m_hStaticDeathAnim);
					g_pLTServer->SetModelLooping(m_hObject, LTFALSE);

					uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
					g_pLTServer->SetObjectFlags(m_hObject, dwFlags & ~(FLAG_SOLID | FLAG_GRAVITY) );

					// Move to floor on the next MID_UPDATE.
					SetTeleportToFloor( LTTRUE );

					// Now swap out the static death animation so we can twitch properly
					m_hStaticDeathAnim = g_pLTServer->GetAnimIndex(m_hObject, "Death_f_Thrw_s0");

					CollisionInfo info;
					g_pLTServer->GetLastCollision(&info);
					LTVector vVel(0,0,0);
					vVel -= info.m_vStopVel;
					g_pLTServer->SetVelocity(m_hObject, &vVel);
				}
			}

			HOBJECT hObj = (HOBJECT)pData;
			if(IsPlayer(hObj) && IsPredator(hObj))
			{
				for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
				{
					HATTACHMENT hAttachment;
					HOBJECT hSpear = m_aSpears[iSpear].hObject;
					if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hSpear, &hAttachment) )
					{
						if ( LT_OK == g_pLTServer->RemoveAttachment(hAttachment) )
						{
							// Tell the spear to give up it's pickup...
							PickupObject* pPU = dynamic_cast<PickupObject*>(g_pLTServer->HandleToObject(hSpear));

							if(pPU)
								pPU->TouchNotify(hObj);
						}
						g_pLTServer->RemoveObject(hSpear);
					}
				}
				m_cSpears = 0;
			}

			break;
		}

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 BodyProp::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_DAMAGE:
		{
			// DON'T call Prop::EngineMessageFn, object might get removed...
			uint32 ret = BaseClass::ObjectMessageFn(hSender, messageID, hRead);

			DamageStruct damage;
			damage.InitFromMessage(hRead);

			// record who hit us...
			m_hLastDamager = damage.hDamager;

			Damage(damage);

			return ret;
		}
		break;

		default : break;
	}

	return Prop::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::PropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void BodyProp::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	if ( g_pLTServer->GetPropGeneric( "DeathType", &genProp ) == LT_OK )
	{
		m_eDeathType = (DeathType)genProp.m_Long;
	}

	if ( g_pLTServer->GetPropGeneric( "CharacterType", &genProp ) == LT_OK )
	{
		if(g_pCharacterButeMgr)
		{
			m_nCharacterSet = g_pCharacterButeMgr->GetSetFromModelType(genProp.m_String);

			if( m_nCharacterSet > (uint32)g_pCharacterButeMgr->GetNumSets() )
			{
				m_nCharacterSet = 0;
#ifndef _FINAL
				g_pLTServer->CPrint("Body prop could not find character set \"%s\".", genProp.m_String);
#endif
			}

			const CharacterButes & butes = g_pCharacterButeMgr->GetCharacterButes(m_nCharacterSet);
			m_eModelSkeleton = (ModelSkeleton)butes.m_nSkeleton;

			m_fHeadBiteHealPoints = butes.m_fHeadBiteHealPoints;
			m_fClawHealPoints = butes.m_fClawHealPoints;

			pStruct->m_UserFlags = SurfaceToUserFlag((SurfaceType)butes.m_nSurfaceType);
		}
	}

	m_bFadeAway = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::HandleModelString()
//
//	PURPOSE:	Handles model keyframe strings
//
// ----------------------------------------------------------------------- //

void BodyProp::HandleModelString(ArgList* pArgList)
{
	if(!pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if(!pKey) return;

	// This handles the 'Noise' string key to play a body thump sound
	if(!_stricmp(pKey, s_szKeyNoise))
	{
		// Find the surface type that we're on
		CollisionInfo Info;
		g_pLTServer->GetStandingOn(m_hObject, &Info);

		SurfaceType eSurface = GetSurfaceType(Info);

		// Play a noise sound
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurface);
		if(!pSurf) return;

		if(pSurf->szBodyFallSnd[0])
			g_pServerSoundMgr->PlaySoundFromPos(vPos, pSurf->szBodyFallSnd, pSurf->fBodyFallSndRadius, SOUNDPRIORITY_MISC_LOW);
	}
	else if(!_stricmp(pKey, s_szKeyChestburstFX))
	{
		// Send the FX message down to the client to create the chest burst blood
		// and model FX

		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_BODYPROP_ID);
		g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
		g_pLTServer->WriteToMessageByte(hMessage, BPFX_CHESTBURST_FX);
		g_pLTServer->EndMessage(hMessage);
	}
	else if(!_stricmp(pKey, s_szKeyChestburst))
	{
		// Change the character that's attached to a chest burster and send him
		// flying from the stomach of this body
		if(m_hFaceHugger && !IsAI(m_hFaceHugger))
		{
			// Get a handle to the character object
			CCharacter *pObj = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(m_hFaceHugger));
			CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hFaceHugger));

			int nSet = -1;

			if(g_pGameServerShell->GetGameType().IsMultiplayer())
				nSet = g_pCharacterButeMgr->GetSetFromModelType("Chestburster_MP");
			else
				nSet = g_pCharacterButeMgr->GetSetFromModelType("Chestburster");

			if( nSet < 0 )
			{
				nSet = 0;
			}

			if(pObj)
			{
				pObj->SetChestburstedFrom(m_nCharacterSet);

				// [KLS] We want to make sure the player gets reset correctly in 
				// the call to SetCharacter() below, so set this here...
			
				if (pPlayer)
				{
					pPlayer->ResetHealthAndArmor(LTTRUE);
				}
				
				pObj->SetCharacter(nSet);

				pObj->GetMovement()->SetScale(1.0f);
			}


			// Set the user flags of this guy
			uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(pObj->m_hObject);
			g_pLTServer->SetObjectUserFlags(pObj->m_hObject, dwUserFlags & ~USRFLG_CHAR_FACEHUG);


			// Calculate the position and direction we want to shoot out from
			LTVector vPos, vDir(0.0f, 1.0f, 0.0f);

			HMODELSOCKET hSocket;
			g_pLTServer->GetModelLT()->GetSocket(m_hObject, "Stomach", hSocket);

			if(hSocket != INVALID_MODEL_SOCKET && !g_pGameServerShell->GetGameType().IsMultiplayer())
			{
				LTVector vUnused;
				LTRotation rRot;
				LTransform trans;

				g_pLTServer->GetModelLT()->GetSocketTransform(m_hObject, hSocket, trans, LTTRUE);
				g_pLTServer->GetTransformLT()->Get(trans, vPos, rRot);
				g_pLTServer->GetMathLT()->GetRotationVectors(rRot, vUnused, vUnused, vDir);
			}
			else
			{
				g_pLTServer->GetObjectPos(m_hObject, &vPos);
			}

			vDir *= g_ChestBurstVelocity.GetFloat();

			if (pPlayer)
			{
				pPlayer->SendScaleInformation();
				pPlayer->AquireDefaultWeapon();

				HCLIENT hClient = pPlayer->GetClient();

				// Update the multiplayer mgr to send the new character information to the clients
				g_pGameServerShell->GetMultiplayerMgr()->ChangeClientCharacter(hClient, nSet, LTFALSE);

				g_pLTServer->SetObjectPos(pPlayer->m_hObject, &vPos);
				pPlayer->TeleportClientToServerPos();

				LTRotation rRot;
				LTVector vDirAlign = vDir;
				LTVector vUp(0.0f, 1.0f, 0.0f);

				vDirAlign.y = 0.0f;
				vDirAlign.Norm();
				g_pMathLT->AlignRotation(rRot, vDirAlign, vUp);

				g_pLTServer->SetObjectRotation(pPlayer->m_hObject, &rRot);

				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_PLAYER_ORIENTATION);
				g_pLTServer->WriteToMessageByte(hMessage, LTTRUE);
				g_pLTServer->WriteToMessageRotation(hMessage, &rRot);
				g_pLTServer->EndMessage(hMessage);

				hMessage = g_pLTServer->StartMessage(hClient, MID_SERVERFORCEVELOCITY);
				g_pLTServer->WriteToMessageVector(hMessage, &vDir);
				g_pLTServer->EndMessage(hMessage);
			}
		}

		m_bFadeAway = LTTRUE;
		m_bCanGIB = LTTRUE;
		m_eDeathType = CD_NORMAL;
	}

	Prop::HandleModelString(pArgList);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::TriggerMsg()
//
//	PURPOSE:	Handler for BodyProp trigger messages.
//
// --------------------------------------------------------------------------- //

void BodyProp::TriggerMsg(HOBJECT hSender, HSTRING hMsg)
{
	if (!g_pLTServer || !m_hObject) return;

	Prop::TriggerMsg(hSender, hMsg);

	char* pMsg = g_pLTServer->GetStringData(hMsg);
	if (!pMsg) return;

	// Handles a 'Fall X, Y, Z' trigger message
	if(!_strnicmp(pMsg, s_szMessageFall, s_nMessageFallLen))
	{
		// A move message
		LTVector vMoveDir;
		sscanf(&pMsg[5], "%f,%f,%f", &vMoveDir.x, &vMoveDir.y, &vMoveDir.z);

		LTFLOAT fMass = m_damage.GetMass();
		m_damage.SetMass(fMass * 2.0f);

		uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
		g_pLTServer->SetObjectFlags(m_hObject, dwFlags | FLAG_SOLID);

		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		g_pLTServer->MoveObject(m_hObject, &(vPos + vMoveDir));
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::Damage()
//
//	PURPOSE:	Remove the body if appropriate
//
// ----------------------------------------------------------------------- //

void BodyProp::Damage(DamageStruct damage)
{
	DeathType dt = GetDeathType(m_damage.GetLastDamageType());

	// Special case for these death types
	if(m_eDeathType == CD_FACEHUG || m_eDeathType == CD_CHESTBURST)
	{
//		m_fClawHealPoints = 0.0f;
	}
	else
	{
		// If this bodyprop died due to an explosion... create some gibs
		if((dt == CD_GIB) || (dt == CD_EXPLODE))
		{
			// Get around the leg test that prevents gibbage.
			m_eNodeLastHit = g_pModelButeMgr->GetSkeletonNode(m_eModelSkeleton, "Torso_u_node");

			m_vDeathDir = m_damage.GetDeathDir();
			m_vDeathDir.Norm();
			m_vDeathDir *= 1.0f + (m_damage.GetDeathDamage() * m_fAdjustFactor / m_damage.GetMaxHitPoints());

			ExplodeLimb(m_eNodeLastHit, (LTBOOL)(m_eDeathType == CD_EXPLODE));
		}
		else if(dt == CD_SLICE)
		{
			LTBOOL bIsPredator = LTFALSE;
			if(damage.hDamager && IsPredator(damage.hDamager))
				bIsPredator = LTTRUE;

			//only cut off the head
			const char* pExplPiece = g_pModelButeMgr->GetSkeletonNodePiece(m_eModelSkeleton, m_eNodeLastHit);
			if(pExplPiece && pExplPiece[0] != '\0')
			{
				if (_stricmp("Head", pExplPiece) == 0 && !m_bLimbProp && !bIsPredator)
					DetachLimb(m_eNodeLastHit);
				else
				{
					//Now check for gibs
					if(CheckForGibs())
					{
						// Play the evil sound!
						DoSpecialDamageSound(damage.hDamager, LTTRUE);
					}
				}
			}
		}
		else if(dt == CD_HEADBITE)
		{
			// Gib the head here...
			ModelNode eHeadNode = g_pModelButeMgr->GetSkeletonNode(m_eModelSkeleton, "Head_node");
			ExplodeLimb(eHeadNode);

			// Make sure the head is not available
			m_bHeadAvailable = LTFALSE;
		}
		else if(dt == CD_NORMAL || dt == CD_SHOTGUN || dt == CD_BULLET_NOGIB || dt == CD_SPEAR) 
		{
			// See if somthing needs to gib
			if(dt != CD_BULLET_NOGIB || IsAlien(m_nCharacterSet))
			{
				if(CheckForGibs())
				{
					// Play the evil sound!
					DoSpecialDamageSound(damage.hDamager, LTTRUE);	
				}
			}

			// Now play the twitch
			if(m_hStaticDeathAnim != INVALID_ANI && !m_bFlying)
			{
				//get the current animation and playing state
				uint32 dwAni	= g_pLTServer->GetModelAnimation(m_hObject);
				uint32 dwState	= g_pLTServer->GetModelPlaybackState(m_hObject);

				//test to see that we are not already playing an animation
				if((dwAni == m_hDeathAnim) && (dwState & MS_PLAYDONE))
				{
					g_pLTServer->SetModelAnimation(m_hObject, m_hStaticDeathAnim);
					g_pLTServer->ResetModelAnimation(m_hObject);
					g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
				}
				else if(dwAni == m_hStaticDeathAnim)
				{
					g_pLTServer->ResetModelAnimation(m_hObject);
					g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

void BodyProp::Update()
{
	if (!m_hObject) return;

	// ----------------------------------------------------------------------- //
	// Create the box used for weapon impact detection if one doesn't already exist
	CreateHitBox();

	if(m_hHitBox)
	{
		CCharacterHitBox* pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(m_hHitBox);

		if(pHitBox)
		{
			pHitBox->SetUseOverrideDims(LTTRUE);
			pHitBox->SetOverrideDims(m_vOrigDims);
		}
	}


	// If all are pieces are hidden... go ahead and remove us
	if(AllPiecesHidden())
	{
		RemoveObject();
	}


	// Get time information and set our next update
	LTFLOAT fTime = g_pLTServer->GetTime();
	SetNextUpdate(s_fUpdateDelta);

	// If we're not supposed to fade away... keep the start time reset
	if(!m_bFadeAway) m_fStartTime = fTime;


	// If we're not just a limb... then just to see when we should
	// change to the static death animation
	if(!m_bLimbProp)
	{
		// Set the animation to our static one

		if(!m_bDimsSet)
		{
			if(m_hStaticDeathAnim != INVALID_ANI)
			{
				LTVector vDims;
				g_pLTServer->Common()->GetModelAnimUserDims(m_hObject, &vDims, m_hStaticDeathAnim);

				m_vOrigDims = vDims;
				vDims.x = vDims.z = 5.0f;
				g_pLTServer->Physics()->SetObjectDims(m_hObject, &vDims, 0);

				// turn on our flag
				m_bDimsSet = LTTRUE;
			}
		}


		// If we died from a facehug attack... check to see if we need to play
		// the chest burst animation yet.

		LTAnimTracker* pTracker;
		g_pLTServer->GetModelLT()->GetMainTracker(m_hObject, pTracker);

		uint32 dwState; 
		g_pLTServer->GetModelLT()->GetPlaybackState(pTracker, dwState);

		if((m_eDeathType == CD_FACEHUG) && (dwState & MS_PLAYDONE))
		{
			LTFLOAT fFHTime = g_FaceHugImplantTime.GetFloat() + g_FaceHugGestationTime.GetFloat();

			// Update the position of the facehugger
			HMODELSOCKET hSocket;
			LTransform trans;
			LTVector vPos;
			LTBOOL bPosOk = LTFALSE;

			if(m_hFaceHugger)
			{
				g_pLTServer->GetModelLT()->GetSocket(m_hObject, "FaceHug", hSocket);

				if(hSocket != INVALID_MODEL_SOCKET)
				{
					g_pLTServer->GetModelLT()->GetSocketTransform(m_hObject, hSocket, trans, LTTRUE);
					g_pLTServer->GetTransformLT()->GetPos(trans, vPos);

					g_pLTServer->SetObjectPos(m_hFaceHugger, &vPos);
					bPosOk = LTTRUE;
				}
			}


			// See if it's time to start chestbursting
			if((fTime - m_fFaceHugTime) > fFHTime)
			{
				// m_hFaceHugger will be null after the object is removed.
				if(m_hFaceHugger && !IsAI(m_hFaceHugger))
				{
					if(g_pGameServerShell->GetGameType().IsMultiplayer())
					{
						m_hDeathAnim = g_pLTServer->GetAnimIndex(m_hObject, "Death_CBurstr");
						m_hStaticDeathAnim = g_pLTServer->GetAnimIndex(m_hObject, "Death_CBurstr_s");

						g_pLTServer->GetModelLT()->SetCurAnim(pTracker, m_hDeathAnim);
						g_pLTServer->GetModelLT()->SetLooping(pTracker, LTFALSE);
						g_pLTServer->GetModelLT()->SetPlaying(pTracker, LTTRUE);

						LTVector vDims;
						g_pLTServer->Common()->GetModelAnimUserDims(m_hObject, &vDims, m_hDeathAnim);

						m_vOrigDims = vDims;
						vDims.x = vDims.z = 5.0f;
						g_pLTServer->Physics()->SetObjectDims(m_hObject, &vDims, 0);

						m_eDeathType = CD_CHESTBURST;
					}
				}
				else
				{
					if(m_hFaceHugger)
						g_pLTServer->RemoveObject(m_hFaceHugger);

					m_bFadeAway = LTTRUE;
					m_bCanGIB = LTTRUE;
				}
			}
			else if((fTime - m_fFaceHugTime) > g_FaceHugImplantTime.GetFloat())
			{
				// Create the facehugger body prop...
				if(m_hFaceHugger)
				{
					// Tell the character to create a body prop
					CCharacter *pObj = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(m_hFaceHugger));

					if(pObj)
					{
						LTFLOAT fScale = pObj->GetMovement()->GetScale();

						if(fScale > 0.1f)
						{
							// Make sure we don't create an acid pool
							if(g_pGameServerShell->GetGameType().IsMultiplayer() && pObj->GetDestructible())
								pObj->GetDestructible()->SetDeathType(DT_ACID);

							// Create the facehugger body prop
							HOBJECT hBody = pObj->CreateBody(LTFALSE);
							if(bPosOk) g_pLTServer->SetObjectPos(hBody, &vPos);

							uint32 nFlags = 0;
							g_pLTServer->Common()->GetObjectFlags(hBody, OFT_Flags, nFlags);
							g_pLTServer->Common()->SetObjectFlags(hBody, OFT_Flags, nFlags | FLAG_GRAVITY | FLAG_VISIBLE);

							LTVector vScale(1.0f, 1.0f, 1.0f);
							g_pLTServer->ScaleObject(hBody, &vScale);

							// Give the body some velocity
							LTVector vVel(GetRandom(-50.0f, 50.0f), 350.0f, GetRandom(-50.0f, 50.0f));
							g_pLTServer->Physics()->SetVelocity(hBody, &vVel);

							pObj->GetMovement()->SetScale(0.01f);
						}
					}
				}

				// Get rid of the attachment
				g_pLTServer->RemoveAttachment(m_hFaceHuggerAttachment);
				m_hFaceHuggerAttachment = LTNULL;
			}
		}
	}
	else
	{
		// Do some special limb stuff... like rotation
		LTVector vVel;
		g_pLTServer->Physics()->GetVelocity(m_hObject, &vVel);

		LTVector vVelTemp = vVel;
		vVelTemp.y = 0.0f;
		LTFLOAT fVelMag = vVelTemp.Mag();
		LTFLOAT fRotAmount;

		if(fVelMag > 5.0f)
		{
			if(fVelMag > 500.0f)
				fRotAmount = MATH_HALFPI;
			else
				fRotAmount = (fVelMag / 500.0f) * MATH_HALFPI;

			if(m_bSliding)
			{
				if(fVelMag > 0.1f)
				{
					LTRotation rRot;
					LTVector vR, vU, vF;

					g_pLTServer->GetObjectRotation(m_hObject, &rRot);
					g_pLTServer->GetMathLT()->GetRotationVectors(rRot, vR, vU, vF);
					g_pLTServer->GetMathLT()->RotateAroundAxis(rRot, vU, fRotAmount);
					g_pLTServer->RotateObject(m_hObject, &rRot);
				}
			}
			else
			{
				CollisionInfo pInfo;
				g_pLTServer->Physics()->GetStandingOn(m_hObject, &pInfo);

				if(pInfo.m_hObject)
				{
					LTRotation rRot;
					LTVector vR, vU, vF;

					vU = pInfo.m_Plane.m_Normal;
					vF = LTVector(0.0f, 0.0f, 1.0f);
					vR = vU.Cross(vF);
					vR.Norm();

					g_pLTServer->GetMathLT()->AlignRotation(rRot, vR, vU);
					g_pLTServer->GetMathLT()->RotateAroundAxis(rRot, vU, GetRandom(-MATH_PI, MATH_PI));
					g_pLTServer->RotateObject(m_hObject, &rRot);

					m_bSliding = LTTRUE;
				}
				else
				{
					LTRotation rRot;
					LTVector vR, vU, vF;

					g_pLTServer->GetMathLT()->AlignRotation(rRot, vVel, vVel);
					g_pLTServer->GetMathLT()->GetRotationVectors(rRot, vR, vU, vF);
					g_pLTServer->GetObjectRotation(m_hObject, &rRot);
					g_pLTServer->GetMathLT()->RotateAroundAxis(rRot, vR, fRotAmount);
					g_pLTServer->RotateObject(m_hObject, &rRot);
				}
			}
		}
	}


	// Make sure our hit box is in the correct position...
	UpdateHitBox();


	if(m_bFirstUpdate)
	{
		m_fStartTime   = fTime;
		m_bFirstUpdate = LTFALSE;

		if(m_hstrInitialMessage)
		{
			SendTriggerMsgToObject(this, m_hObject, m_hstrInitialMessage);
			FREE_HSTRING(m_hstrInitialMessage);
		}
	}


	// Update type specific death stuff
	switch(m_eDeathType)
	{
		case CD_FREEZE:		UpdateFreezeDeath(fTime);		break;
		case CD_NORMAL:		UpdateNormalDeath(fTime); 		break;
		default:			UpdateNormalDeath(fTime); 		break;
	}

	// Update the acid effect
	if(m_bAcidPool && !m_bFlying)
		UpdateAcidPool();

	// See about fading in
	if(m_bFadeIn)
	{
		// Get the time difference
		LTFLOAT fTimeDelta = g_pLTServer->GetTime() - m_fDecloakStartTime;

		// Get the color of our character 
		LTFLOAT r, g, b, a;
		g_pLTServer->GetObjectColor(m_hObject, &r, &g, &b, &a);

		// Change states if needed
		if(fTimeDelta > 1.0f)
		{
			m_bFadeIn = LTFALSE;
			fTimeDelta = 1.0f;
		}

		// Fade him out
		g_pLTServer->SetObjectColor(m_hObject, r, g, b, fTimeDelta);
	}


	// Setup some temporary container storage
	HOBJECT hContainers[CM_MAX_CONTAINERS];
	uint32 nNumContainers = 0;
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	// Retrieve the position containers
	nNumContainers = g_pInterface->GetPointContainers(&vPos, hContainers, CM_MAX_CONTAINERS);

	// Check our volume stuff..
	// See if the upper section of our character is contained in water
	LTBOOL bInLiquid = LTFALSE;
	uint16 nCode;

	for(uint32 i = 0; i < nNumContainers; i++)
	{
		g_pLTServer->GetContainerCode(hContainers[i], &nCode);

		if(IsLiquid((ContainerCode)nCode))
		{
			bInLiquid = LTTRUE;
			break;
		}
	}

	if (bInLiquid)
	{
		// Make sure we are not on fire since fire is put out by liquid...
		m_damage.ClearProgressiveDamages(DT_BURN);
		m_damage.ClearProgressiveDamages(DT_NAPALM);
	}

	// see if we don't have standing on info then set gravity.
	uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	if(!(dwFlags & FLAG_GRAVITY) && m_bUpdateGravity )
	{
		CollisionInfo standingInfo;
		g_pLTServer->GetStandingOn(m_hObject, &standingInfo);

		if (!standingInfo.m_hObject)
			g_pLTServer->SetObjectFlags(m_hObject, dwFlags | FLAG_GRAVITY);
		else
			g_pLTServer->SetObjectFlags(m_hObject, dwFlags & ~FLAG_GRAVITY);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::UpdateNormalDeath()
//
//	PURPOSE:	Update normal death
//
// ----------------------------------------------------------------------- //

LTBOOL BodyProp::UpdateNormalDeath(LTFLOAT fTime)
{
	if(!m_hObject) return LTFALSE;

	// Calculate some final times to be doing stuff
	LTFLOAT fRemoveTime = m_fStartTime + g_BodyLifetimeTrack.GetFloat();
	LTFLOAT fStartEffectTime = m_fStartTime + g_BodyLifetimeTrack.GetFloat() - g_BodyFadeOffEffectTime.GetFloat();

	// Make sure we don't try to start the effect before the start of the death
	if(fStartEffectTime < m_fStartTime)
		fStartEffectTime = m_fStartTime;

	// Check to see if it's time to do a little effect on the prop
	if(fTime >= fStartEffectTime)
	{
		// Make sure we can't GIB this body
		m_bCanGIB = LTFALSE;

		// Calculate our scale for the effect (going from 1.0f to 0.0f)
		LTFLOAT fScale = 1.0f - ((fTime - fStartEffectTime) / (fRemoveTime - fStartEffectTime));

		// Cap it...
		fScale = fScale < 0.0f?0.0f:fScale;

		// Get and set the color of the model
		LTFLOAT r, g, b, a;
		g_pLTServer->GetObjectColor(m_hObject, &r, &g, &b, &a);
		g_pLTServer->SetObjectColor(m_hObject, r, g, b, fScale);

		// Now do the same for the attachments...
		HOBJECT hAttachList[30];
		uint32 dwListSize, dwNumAttachments;

		if (g_pLTServer->Common()->GetAttachments(m_hObject, hAttachList, 
			ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
		{
			for (uint32 i = 0; i < dwNumAttachments; i++)
			{
				if (hAttachList[i])
				{
					g_pLTServer->GetObjectColor(hAttachList[i], &r, &g, &b, &a);
					g_pLTServer->SetObjectColor(hAttachList[i], r, g, b, fScale);
				}
			}
		}
	}

	if(fTime >= fRemoveTime)
	{
		RemoveObject();
		return LTFALSE;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::UpdateFreezeDeath()
//
//	PURPOSE:	Update freeze death
//
// ----------------------------------------------------------------------- //

LTBOOL BodyProp::UpdateFreezeDeath(LTFLOAT fTime)
{
	if(!m_hObject) return LTFALSE;
	
	LTBOOL bRet = LTTRUE;

	if(fTime >= m_fStartTime + s_fFreezeTime)
	{
		m_vColor.z = 255.0f;
		m_eDeathType = CD_NORMAL;
		bRet = LTFALSE;
	}
	else
	{
		m_vColor.z = 255.0f * (fTime - m_fStartTime) / s_fFreezeTime;
		m_vColor.z = m_vColor.z > 255.0f ? 255.0f : m_vColor.z;
	}

	// Pack our color into the upper 3 bytes of the user data...
	uint8 r = (uint8)m_vColor.x;
	uint8 g = (uint8)m_vColor.y;
	uint8 b = (uint8)m_vColor.z;

	uint32 dwData = g_pLTServer->GetObjectUserFlags(m_hObject);
	dwData = ((r<<24) | (g<<16) | (b<<8) | (dwData & 0x000F));
	g_pLTServer->SetObjectUserFlags(m_hObject, dwData);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::CreateHitBox()
//
//	PURPOSE:	Create our hit box
//
// ----------------------------------------------------------------------- //

void BodyProp::CreateHitBox()
{
	if (m_hHitBox) return;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	HCLASS hClass = g_pLTServer->GetClass("CCharacterHitBox");
	if (!hClass) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

#ifdef _DEBUG
	sprintf(theStruct.m_Name, "%s-HitBox", g_pLTServer->GetObjectName(m_hObject) );
#endif

	theStruct.m_Pos = vPos;
	g_pLTServer->GetObjectRotation(m_hObject, &theStruct.m_Rotation);

	// Allocate an object...

	CCharacterHitBox* pHitBox = (CCharacterHitBox *)g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pHitBox) return;

	m_hHitBox = pHitBox->m_hObject;

	pHitBox->Init(m_hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::UpdateHitBox()
//
//	PURPOSE:	Update our hit box position
//
// ----------------------------------------------------------------------- //

void BodyProp::UpdateHitBox()
{
	if (!m_hHitBox) return;

	CCharacterHitBox* pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(m_hHitBox));
	if (pHitBox)
	{
		pHitBox->Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::GetDeathType()
//
//	PURPOSE:	Sets the death type based on the damage type
//
// ----------------------------------------------------------------------- //

DeathType BodyProp::GetDeathType(DamageType eDamageType)
{
	switch(eDamageType)
	{
		case DT_TORCH_CUT:
		case DT_TORCH_WELD:
		case DT_BURN:
		case DT_NAPALM:
		case DT_ACID:			return CD_BURN;

		case DT_CRUSH:			return CD_GIB;

		case DT_ALIEN_CLAW:
		case DT_SHOTGUN:		return CD_SHOTGUN;

		case DT_EXPLODE:		return CD_EXPLODE;

		case DT_FREEZE:			return CD_FREEZE;
		case DT_HEADBITE:		return CD_HEADBITE;
		case DT_SPEAR:			return CD_SPEAR;

		case DT_MARINE_HACKER:
		case DT_UNSPECIFIED:
		case DT_BULLET:
		case DT_HIGHCALIBER:
		case DT_CHOKE:
		case DT_ELECTRIC:
		case DT_POISON:
		case DT_BLEEDING:
		case DT_STUN:
		case DT_BITE:
		case DT_ENDLESS_FALL:	return CD_NORMAL;

		case DT_BULLET_NOGIB:	return CD_BULLET_NOGIB;

		case DT_SLICE:			return CD_SLICE;

		case DT_FACEHUG:		return CD_FACEHUG;

		default:				return CD_NORMAL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::SetupByDeathType()
//
//	PURPOSE:	Do special setup stuff with respect to the death type
//
// ----------------------------------------------------------------------- //

void BodyProp::SetupByDeathType(uint32 &dwFlags, uint32 &dwUserFlags)
{
	switch(m_eDeathType)
	{
//		case CD_FREEZE:
//		{
//			dwUserFlags |= USRFLG_MODELADD;
//			dwFlags |= FLAG_SOLID;
//			return;
//		}

		case CD_GIB:
		{
			ExplodeLimb(m_eNodeLastHit, (LTBOOL)(m_eDeathType == CD_EXPLODE));
			return;
		}

		case CD_EXPLODE:
		{
			m_eNodeLastHit = g_pModelButeMgr->GetSkeletonNode(m_eModelSkeleton, "Torso_u_node");
			CheckForGibs( LTTRUE, m_vDeathDir*500.0f, LTFALSE);

			// Now set us flying
			const CharacterButes & butes	= g_pCharacterButeMgr->GetCharacterButes(m_nCharacterSet);
			if(!IsAlien(butes))
			{
				// We need to know when we hit somthing
				uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
				g_pLTServer->SetObjectFlags(m_hObject, dwFlags | ( FLAG_TOUCH_NOTIFY));

				g_pLTServer->SetVelocity(m_hObject, &((m_vDeathDir+LTVector(0.0f,0.75f, 0.0f))*400.0f));
			}

			return;
		}

		case CD_SHOTGUN:
		{
			// Now set us flying
			const CharacterButes & butes	= g_pCharacterButeMgr->GetCharacterButes(m_nCharacterSet);
			if(!IsAlien(butes))
			{
				// We need to know when we hit somthing
				uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
				g_pLTServer->SetObjectFlags(m_hObject, dwFlags | ( FLAG_TOUCH_NOTIFY));

				g_pLTServer->SetVelocity(m_hObject, &((m_vDeathDir+LTVector(0.0f,1.0f, 0.0f))*400.0f));
			}

			// Only gib if it's not the torso, this way we always send the body flying
			// and never blow it up.
			const char* pExplPiece = g_pModelButeMgr->GetSkeletonNodePiece(m_eModelSkeleton, m_eNodeLastHit);

			if(pExplPiece)
			{
				if(pExplPiece[0] != '\0')
				{
					if (_stricmp("Torso", pExplPiece) == 0)
						m_eNodeLastHit = eModelNodeInvalid;
				}
			}

			return;
		}

		case CD_HEADBITE:
		{
			// Gib the head here...
			ModelNode eHeadNode = g_pModelButeMgr->GetSkeletonNode(m_eModelSkeleton, "Head_node");
			ExplodeLimb(eHeadNode);

			// Make sure we don't give any extra credit
			m_bHeadAvailable = LTFALSE;

//			// now credit the heal...
//			CCharacter*	pAttacker = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(m_hLastDamager));
//
//			//aply the heal
//			if(pAttacker)
//			{
//				CDestructible* pDest = pAttacker->GetDestructible();
//
//				if(pDest)
//					pDest->Heal(m_fHeadBiteHealPoints);
//			}
			return;
		}

		case CD_SPEAR:
		{
			if(!g_pGameServerShell->LowViolence())
			{
				// If the head was hit... it shoulda been taken off, so hide it on the body.
				ModelNode eHeadNode = g_pModelButeMgr->GetSkeletonNode(m_eModelSkeleton, "Head_node");

				if(m_eNodeLastHit == eHeadNode && (IsHuman(m_nCharacterSet) || IsSynthetic(m_nCharacterSet)))
				{
					HMODELPIECE hPiece;
					g_pLTServer->GetModelLT()->GetPiece(m_hObject, "Head", hPiece);
					g_pLTServer->GetModelLT()->SetPieceHideStatus(m_hObject, hPiece, LTTRUE);

					// Now try the sub pieces
					char pTempPiece[255];

					for(int i=1; hPiece != INVALID_MODEL_NODE; i++)
					{
						sprintf(pTempPiece, "%s%s%d", "Head", "_OBJ", i);

						if(g_pModelLT->GetPiece(m_hObject, pTempPiece, hPiece) == LT_OK)
							g_pModelLT->SetPieceHideStatus(m_hObject, hPiece, LTTRUE);	
					}
				}
			}

			return;
		}

		case CD_SLICE:
		{
			LTBOOL bIsPredator = LTFALSE;
			if(m_hLastDamager && IsPredator(m_hLastDamager))
				bIsPredator = LTTRUE;

			//only cut off the head
			const char* pExplPiece = g_pModelButeMgr->GetSkeletonNodePiece(m_eModelSkeleton, m_eNodeLastHit);

			if(pExplPiece)
			{
				if(pExplPiece[0] != '\0')
				{
					if (_stricmp("Head", pExplPiece) == 0 && !m_bLimbProp && !bIsPredator)
						DetachLimb(m_eNodeLastHit);
					else
					{
						//Now check for gibs
						if(CheckForGibs())
						{
							// Play the evil sound!
							DoSpecialDamageSound(m_hLastDamager);
						}
					}
				}
			}
			return;
		}

		case CD_FACEHUG:
		{
			// Make sure the character types are ok...
			if( g_pCharacterButeMgr->GetCanBeFacehugged(m_nCharacterSet) )
			{
				LTBOOL bOk = LTFALSE;
				CCharacter *pObj = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(m_hLastDamager));

				if(pObj)
				{
					int nCharacter = pObj->GetCharacter();
					const CharacterButes & butes = g_pCharacterButeMgr->GetCharacterButes(nCharacter);

					if(butes.m_szName[0])
					{
						if(	!_stricmp(butes.m_szName, "Facehugger") ||
							!_stricmp(butes.m_szName, "Facehugger_AI") ||
							!_stricmp(butes.m_szName, "Facehugger_SAI") ||
							!_stricmp(butes.m_szName, "Facehugger_MP"))
						{
							bOk = LTTRUE;
						}
					}
				}

				// We're set... go ahead and setup the attachments and junk
				if(bOk)
				{
					// Attach this character to the body prop
					LTVector vOffset;
					LTRotation rOffset;

					vOffset.Init();
					g_pMathLT->SetupEuler(rOffset, 0.0f, MATH_PI, 0.0f);

					if(LT_OK == g_pLTServer->CreateAttachment(m_hObject, pObj->m_hObject, "FaceHug", &vOffset, &rOffset, &m_hFaceHuggerAttachment))
					{
						// Set the facehugger object handle
						m_hFaceHugger = pObj->m_hObject;

						// Set the user flags of this guy
						uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(pObj->m_hObject) | USRFLG_CHAR_FACEHUG;
						g_pLTServer->SetObjectUserFlags(pObj->m_hObject, dwUserFlags);

						// Make sure we can't screw up this body... that just causes many issues
						m_fFaceHugTime = g_pLTServer->GetTime();
						m_bFadeAway = LTFALSE;
						m_bCanGIB = LTFALSE;

						// Send some special message if the attacker is a player
						CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hLastDamager));

						if(pPlayer)
						{
							LTVector vPos;
							LTRotation rRot;

							g_pLTServer->GetObjectPos(m_hObject, &vPos);
							g_pLTServer->GetObjectRotation(m_hObject, &rRot);

							HCLIENT hClient = pPlayer->GetClient();

							g_pLTServer->SetObjectPos(pPlayer->m_hObject, &vPos);
							pPlayer->TeleportClientToServerPos();

							HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_PLAYER_ORIENTATION);
							g_pLTServer->WriteToMessageByte(hMessage, LTTRUE);
							g_pLTServer->WriteToMessageRotation(hMessage, &rRot);
							g_pLTServer->EndMessage(hMessage);
						}
					}
				}
			}

			return;
		}

		default:
			return;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::ExplodeLimb()
//
//	PURPOSE:	Explode one limb and create more if needed
//
// ----------------------------------------------------------------------- //

void BodyProp::ExplodeLimb(ModelNode eParentNode, LTBOOL bFirey, LTVector vVelOffset, LTBOOL bGib)
{
	int i = 0;

	if(g_pGameServerShell->LowViolence())
	{
		// No GIBs on low violence unless it's a special case
		if(m_eDeathType != CD_GIB)
			return;

		if(!m_bLowViolenceGIB)
			return;
	}


	if(!m_bCanGIB || eParentNode == eModelNodeInvalid || (!m_damage.GetCanDamage() && !m_bFadeAway)) return;

	// Get the name of the piece so we can make sure to not allow it on
	// the other limbs
	const char* pExplPiece = g_pModelButeMgr->GetSkeletonNodePiece(m_eModelSkeleton, eParentNode);
	if(!pExplPiece || pExplPiece[0] == '\0') return;

	HMODELPIECE hPiece = INVALID_MODEL_PIECE;


	// Get the hit location of this node...
	HitLocation hitLoc = g_pModelButeMgr->GetSkeletonNodeHitLocation(m_eModelSkeleton, eParentNode);

	// don't gib heads that were collected by predators...  sick bastards!
	if(m_hLastDamager && IsPredator(m_hLastDamager))
		if(hitLoc == HL_HEAD) bGib = LTFALSE;


	// If this is a leg node and we're not in our static death animation, then
	// don't allow it to explode yet.
	uint32 dwAni = g_pLTServer->GetModelAnimation(m_hObject);
	uint32 dwState = g_pLTServer->GetModelPlaybackState(m_hObject);

	if((hitLoc == HL_LEG) && (dwAni == m_hDeathAnim) && !(dwState & MS_PLAYDONE))
		return;


	// Make the piece invisible
	if(g_pModelLT->GetPiece(m_hObject, const_cast<char*>(pExplPiece), hPiece) == LT_OK)
	{
		// First get the hide status... if it's already invisible, just get out of here.
		LTBOOL bHidden;
		g_pModelLT->GetPieceHideStatus(m_hObject, hPiece, bHidden);
		if(bHidden) return;
	}
	else
		// If you can't find the piece then we must stop here
		// else we can make mass gibs and never hide the piece
		// and hiding is our signal.
		return;


	// Start a stack to go through multiple limb branches
	ModelNode pNodeStack[32];
	int nStackAmount = -1;

	ModelNode *pChildren = g_pModelButeMgr->GetSkeletonNodeChildren(m_eModelSkeleton, eParentNode);
	int nChildren = g_pModelButeMgr->GetSkeletonNodeNumChildren(m_eModelSkeleton, eParentNode);

	for(i = 0; i < nChildren; i++)
	{
		++nStackAmount;
		_ASSERT(nStackAmount < 32);

		pNodeStack[nStackAmount] = pChildren[i];
	}


	// Find more children if we can, and create limbs for any appropriate branches
	ModelNode mnTemp = eModelNodeInvalid;
	const char* pPiece = LTNULL;

	while(nStackAmount > -1)
	{
		// Get the current model node to check
		mnTemp = pNodeStack[nStackAmount];
		--nStackAmount;

		// Get the name of the piece associated with this node
		pPiece = g_pModelButeMgr->GetSkeletonNodePiece(m_eModelSkeleton, mnTemp);
		if(pPiece[0] == '\0') continue;

		// If this is the same as our explosion piece... check for more children
		if(!_stricmp(pPiece, pExplPiece))
		{
			pChildren = g_pModelButeMgr->GetSkeletonNodeChildren(m_eModelSkeleton, mnTemp);
			nChildren = g_pModelButeMgr->GetSkeletonNodeNumChildren(m_eModelSkeleton, mnTemp);

			for(i = 0; i < nChildren; i++)
			{
				++nStackAmount;
				_ASSERT(nStackAmount < 32);

				pNodeStack[nStackAmount] = pChildren[i];
			}
		}
		else
		{
			if(!bFirey)
				// Recurse here
				ExplodeLimb(mnTemp, bFirey, vVelOffset, LTFALSE);
			else
				DetachLimb(mnTemp, bFirey);
		}
	}


	// Set the explode piece invisible now that we created all the limbs
	if(hPiece != INVALID_MODEL_NODE)
		g_pModelLT->SetPieceHideStatus(m_hObject, hPiece, LTTRUE);

	// Now try the sub pieces
	char pTempPiece[255];

	for(i=1; hPiece != INVALID_MODEL_NODE; i++)
	{
		sprintf(pTempPiece, "%s%s%d", pExplPiece, "_OBJ", i);

		if(g_pModelLT->GetPiece(m_hObject, pTempPiece, hPiece) == LT_OK)
			g_pModelLT->SetPieceHideStatus(m_hObject, hPiece, LTTRUE);	
	}

	// Now create some appropriate GIBs at the location of our parent node
	if(bGib)
		CreateGibs(eParentNode, vVelOffset);


	// If it's part of the torso or unknown... then go ahead and remove this object
	if(hitLoc == HL_TORSO || hitLoc == HL_UNKNOWN)
	{
		RemoveObject();
	}

	// Make sure this object fades out if it's going to hang around
//	m_bFadeAway = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::DetachLimb()
//
//	PURPOSE:	Create a smaller body prop starting at a parent node
//
// ----------------------------------------------------------------------- //

void BodyProp::DetachLimb(ModelNode eParentNode, LTBOOL bFirey)
{
	if(g_pGameServerShell->LowViolence())
	{
		// No GIBs on low violence unless it's a special case
		if(m_eDeathType != CD_GIB)
			return;

		if(!m_bLowViolenceGIB)
			return;
	}


	if(!m_bCanGIB || eParentNode == eModelNodeInvalid || (!m_damage.GetCanDamage() && !m_bFadeAway)) return;

	// Get the hit location of this node...
	HitLocation hitLoc = g_pModelButeMgr->GetSkeletonNodeHitLocation(m_eModelSkeleton, eParentNode);

	// If it's part of the torso... then don't do any of this
	if(hitLoc == HL_TORSO || hitLoc == HL_UNKNOWN)
		return;


	// If this is a leg node and we're not in our static death animation, then
	// don't allow it to explode yet.
	uint32 dwAni = g_pLTServer->GetModelAnimation(m_hObject);
	uint32 dwState = g_pLTServer->GetModelPlaybackState(m_hObject);

	if((hitLoc == HL_LEG) && (dwAni == m_hDeathAnim) && !(dwState & MS_PLAYDONE))
		return;


	// ----------------------------------------------------------------------- //
	// Test to see if the parent is invisible... cause if it is, there's nothing to
	// detach from.  That mean we should scat!
	HMODELPIECE hPiece = INVALID_MODEL_PIECE;
	const char* pPiece = LTNULL;
	const char* pParentPiece = LTNULL;

	ModelNode eSPNode = g_pModelButeMgr->GetSkeletonNodeParent(m_eModelSkeleton, eParentNode);
	if(eSPNode == eModelNodeInvalid) return;

	pPiece = g_pModelButeMgr->GetSkeletonNodePiece(m_eModelSkeleton, eParentNode);
	pParentPiece = g_pModelButeMgr->GetSkeletonNodePiece(m_eModelSkeleton, eSPNode);

	if(!pPiece[0] || !pParentPiece[0]) return;

	while(!_stricmp(pPiece, pParentPiece))
	{
		eSPNode = g_pModelButeMgr->GetSkeletonNodeParent(m_eModelSkeleton, eSPNode);
		if(eSPNode == eModelNodeInvalid) return;

		pParentPiece = g_pModelButeMgr->GetSkeletonNodePiece(m_eModelSkeleton, eSPNode);
	}

	if(g_pModelLT->GetPiece(m_hObject, const_cast<char*>(pParentPiece), hPiece) != LT_OK)
		return;

	// ----------------------------------------------------------------------- //
	// See if we are already hidden.
	LTBOOL bHidden;
	g_pModelLT->GetPieceHideStatus(m_hObject, hPiece, bHidden);
	if(bHidden) return;

	// ----------------------------------------------------------------------- //
	// Get the node position
	LTVector vNodePos;
	GetNodePosition(eParentNode, vNodePos);


	// ----------------------------------------------------------------------- //
	// Create a duplicate object of this bodyprop
	HCLASS hClass = g_pLTServer->GetClass("BodyProp");
	if(!hClass) return;

	ObjectCreateStruct theStruct;
	theStruct.Clear();

#ifdef _DEBUG
	sprintf(theStruct.m_Name, "%s-%s", g_pLTServer->GetObjectName(m_hObject), pPiece );
#endif

	g_pCharacterButeMgr->GetDefaultFilenames(m_nCharacterSet, theStruct);

	theStruct.m_Pos = vNodePos;
	g_pLTServer->GetObjectRotation(m_hObject, &theStruct.m_Rotation);

	// Offset the rotation to match a standing position
	LTRotation rOffsetRot;
	g_pMathLT->SetupEuler(rOffsetRot, MATH_HALFPI, 0.0f, 0.0f);
	theStruct.m_Rotation = theStruct.m_Rotation * rOffsetRot;

	// Allocate an object...
	BodyProp* pProp = (BodyProp *)g_pLTServer->CreateObject(hClass, &theStruct);
	if(!pProp) return;


	// Setup the limb
	pProp->SetupLimb(this, eParentNode, bFirey);


	// ----------------------------------------------------------------------- //
	// Go through the nodes and make sure only the nodes we want are visible
	pProp->HideAllPieces();

	ModelNode pNodeStack[32];
	int nStackAmount = 0;

	hPiece = INVALID_MODEL_PIECE;
	ModelNode mnTemp = eModelNodeInvalid;
	int nChildren = 0;
	int nTotalChildren = 1;

	// Fill in the first stack position
	pNodeStack[0] = eParentNode;

	while(nStackAmount > -1)
	{
		// Get the current model node to check
		mnTemp = pNodeStack[nStackAmount];
		--nStackAmount;

		// Get the name of the piece associated with this node
		pPiece = g_pModelButeMgr->GetSkeletonNodePiece(m_eModelSkeleton, mnTemp);
		if(pPiece[0] == '\0') continue;

		// Make the piece visible on the new limb and invisible on the current bodyprop
		if(g_pModelLT->GetPiece(pProp->m_hObject, const_cast<char*>(pPiece), hPiece) == LT_OK)
		{
			g_pModelLT->GetPieceHideStatus(m_hObject, hPiece, bHidden);

			if(!bHidden)
			{
				g_pModelLT->SetPieceHideStatus(pProp->m_hObject, hPiece, LTFALSE);
				g_pModelLT->SetPieceHideStatus(m_hObject, hPiece, LTTRUE);

				// Now try the sub pieces
				char pTempPiece[255];

				for(int i=1; hPiece != INVALID_MODEL_NODE; i++)
				{
					sprintf(pTempPiece, "%s%s%d", pPiece, "_OBJ", i);

					if(g_pModelLT->GetPiece(pProp->m_hObject, pTempPiece, hPiece) == LT_OK)
					{
						g_pModelLT->SetPieceHideStatus(pProp->m_hObject, hPiece, LTFALSE);	
						g_pModelLT->SetPieceHideStatus(m_hObject, hPiece, LTTRUE);	
					}
				}
			}
		}


		// Get the number of children this node has
		nChildren = g_pModelButeMgr->GetSkeletonNodeNumChildren(m_eModelSkeleton, mnTemp);

		if(nChildren > 0)
		{
			ModelNode *pTemp = g_pModelButeMgr->GetSkeletonNodeChildren(m_eModelSkeleton, mnTemp);

			// Add the children to the stack
			for(int i = 0; i < nChildren; i++)
			{
				++nStackAmount;
				_ASSERT(nStackAmount < 32);

				pNodeStack[nStackAmount] = pTemp[i];
			}

			// Increment the total children count
			nTotalChildren += nChildren;
		}
	}


	// ----------------------------------------------------------------------- //
	// Give the limb some velocity depending on the damage, position, and weight
	LTVector vObjPos, vDiff, vTemp;
	LTVector vRand(GetRandom(-0.0f, 0.0f), GetRandom(-0.0f, 0.0f), GetRandom(-0.0f, 0.0f));
	g_pLTServer->GetObjectPos(m_hObject, &vObjPos);

	vTemp = m_vDeathDir;
	vTemp.Norm();

	vDiff = vNodePos - vObjPos;
	vDiff.Norm(g_BodyLimbDirScale.GetFloat());

	vDiff += vDiff + vTemp + vRand;
	vDiff *= g_BodyLimbTrajectory.GetFloat() / (LTFLOAT)nTotalChildren;

	if(m_eDeathType == CD_GIB)
		vDiff *= 2.0f;

	g_pLTServer->Physics()->SetVelocity(pProp->m_hObject, &vDiff);


	// Make sure this object fades out if it's going to hang around
	m_bFadeAway = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::CreateGibs()
//
//	PURPOSE:	Create the gibs props
//
// ----------------------------------------------------------------------- //

void BodyProp::CreateGibs(ModelNode eNode, LTVector vVelOffset)
{
	LTVector vPos;
	GetNodePos(eNode, vPos, LTTRUE);

	const char* pDebrisType = g_pModelButeMgr->GetSkeletonNodeGIBType(m_eModelSkeleton, eNode);

	if(pDebrisType)
	{
		MULTI_DEBRIS *debris = g_pDebrisMgr->GetMultiDebris(const_cast<char*>(pDebrisType));

		if(debris)
		{
			CLIENTDEBRIS fxStruct;

			fxStruct.rRot.Init();
			fxStruct.vPos = vPos;
			fxStruct.nDebrisId = debris->nId;
			fxStruct.vVelOffset = vVelOffset;

			CreateMultiDebris(fxStruct);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::GetNodePosition()
//
//	PURPOSE:	Get the node position... or it's parent if this node
//				is represented by a socket instead
//
// ----------------------------------------------------------------------- //

LTBOOL BodyProp::GetNodePosition(ModelNode eNode, LTVector &vPos, LTRotation* pRot)
{
	HMODELNODE hNode = INVALID_MODEL_NODE;
	ModelNode pPosNode = eNode;
	const char* pNode = LTNULL;
	LTBOOL bFoundNodePos = LTFALSE;
	const char * szNodeName = g_pModelButeMgr->GetSkeletonNodeName(m_eModelSkeleton, eNode);
	HMODELSOCKET hSocket;
	char pTemp[255];
	sprintf(pTemp,szNodeName);

	while(pPosNode != eModelNodeInvalid)
	{
		// Get the name of the node
		pNode = g_pModelButeMgr->GetSkeletonNodeName(m_eModelSkeleton, pPosNode);

		// Make the piece visible on the new limb and invisible on the current bodyprop
		if(g_pModelLT->GetNode(m_hObject, const_cast<char*>(pNode), hNode) == LT_OK)
		{
			LTransform trans;
			g_pModelLT->GetNodeTransform(m_hObject, hNode, trans, LTTRUE);
			g_pLTServer->GetTransformLT()->GetPos(trans, vPos);

			if(pRot)
				g_pLTServer->GetTransformLT()->GetRot(trans, *pRot);

			bFoundNodePos = LTTRUE;
			break;
		}
		else if (g_pModelLT->GetSocket(m_hObject, pTemp, hSocket) == LT_OK)
		{
			LTransform	tTransform;

			if (g_pModelLT->GetSocketTransform(m_hObject, hSocket, tTransform, LTTRUE) == LT_OK)
			{
				TransformLT* pTransLT = g_pLTServer->GetTransformLT();
				pTransLT->GetPos(tTransform, vPos);

				if(pRot)
					g_pLTServer->GetTransformLT()->GetRot(tTransform, *pRot);

				bFoundNodePos = LTTRUE;
				break;
			}
		}
		else
		{
			// Go back a node in the tree
			pPosNode = g_pModelButeMgr->GetSkeletonNodeParent(m_eModelSkeleton, pPosNode);
		}

		// Get the hit location of this node... if it's all the way back to the torso
		// or something unknown... then just exit, we'll use the object position instead
		HitLocation hlTemp = g_pModelButeMgr->GetSkeletonNodeHitLocation(m_eModelSkeleton, pPosNode);
		if(hlTemp == HL_TORSO || hlTemp == HL_UNKNOWN)
		{
			g_pLTServer->GetObjectPos(m_hObject, &vPos);
			break;
		}
	}

	return bFoundNodePos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::GetNodePos
//
//	PURPOSE:	Get the position of the node
//
// ----------------------------------------------------------------------- //

LTBOOL BodyProp::GetNodePos(ModelNode eNode, LTVector &vPos, LTBOOL bWorldPos, LTRotation* pRot)
{
	const char * szNodeName = g_pModelButeMgr->GetSkeletonNodeName(m_eModelSkeleton, eNode);

	//see about head bite
	ModelLT* pModelLT   = g_pLTServer->GetModelLT();

	char pTemp[255];
	sprintf(pTemp,szNodeName);

	HMODELNODE hNode;

	if (pModelLT->GetNode(m_hObject, pTemp, hNode) == LT_OK)
	{
		LTransform	tTransform;

		if (pModelLT->GetNodeTransform(m_hObject, hNode, tTransform, bWorldPos) == LT_OK)
		{
			TransformLT* pTransLT = g_pLTServer->GetTransformLT();
			pTransLT->GetPos(tTransform, vPos);
			if(pRot)
				pTransLT->GetRot(tTransform, *pRot);

			return LTTRUE;
		}
	}
	else
	{
		HMODELSOCKET hSocket;

		if (pModelLT->GetSocket(m_hObject, pTemp, hSocket) == LT_OK)
		{
			LTransform	tTransform;

			if (pModelLT->GetSocketTransform(m_hObject, hSocket, tTransform, bWorldPos) == LT_OK)
			{
				TransformLT* pTransLT = g_pLTServer->GetTransformLT();
				pTransLT->GetPos(tTransform, vPos);
				if(pRot)
					pTransLT->GetRot(tTransform, *pRot);

				return LTTRUE;
			}
		}
	}

	// hmmm... can't find it!
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::RemoveObject()
//
//	PURPOSE:	Remove the object
//
// ----------------------------------------------------------------------- //

void BodyProp::RemoveObject()
{
	// Now do the same for the attachments...
	HOBJECT hAttachList[30];
	uint32 dwListSize, dwNumAttachments;

	if (g_pLTServer->Common()->GetAttachments(m_hObject, hAttachList, 
		ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
	{
		for (uint32 i = 0; i < dwNumAttachments; i++)
		{
			HATTACHMENT hAttachment;
			if (g_pLTServer->FindAttachment(m_hObject, hAttachList[i], &hAttachment) == LT_OK)
			{
				g_pLTServer->RemoveAttachment(hAttachment);

			}

			g_pLTServer->RemoveObject(hAttachList[i]);
		}
	}

	if(m_hObject)
	{
		g_pLTServer->RemoveObject(m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::GetSpecialDeathTypeAnim()
//
//	PURPOSE:	Gets a special animation specific to a death type
//
// ----------------------------------------------------------------------- //

const char* BodyProp::GetSpecialDeathTypeAnim(DeathType eType)
{
	switch(m_eDeathType)
	{
		case CD_GIB:			return LTNULL;
		case CD_FREEZE:			return LTNULL;
		case CD_BURN:
		{
			if(!GetRandom(0, 1))
				return "Death_Burn0";

			return "Death_Burn1";
		}

		case CD_CHESTBURST:		return "Death_CBurstr";
		case CD_HEADBITE:		return LTNULL;
		case CD_SPEAR:			return LTNULL;
		case CD_SLICE:			return LTNULL;
		case CD_FACEHUG:		return "Death_Fhgger";
		case CD_EXPLODE:
		case CD_SHOTGUN:
			{
				if(HasFlyingDeathAnims() && GetRandom(0.0f, 1.0f) < 0.75f)
				{
					m_bFlying = LTTRUE;
					return "Flying";
				}
			}
		default:				return LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::GetSpecialDeathTypeSound()
//
//	PURPOSE:	Gets a special animation specific to a death type
//
// ----------------------------------------------------------------------- //

const char* BodyProp::GetSpecialDeathTypeSound(DeathType eType)
{
	switch(m_eDeathType)
	{
		case CD_BURN:			return "Death_Burn";
		case CD_NORMAL:			return "Death";
		default:				return "Death";
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::UpdateAcidPool()
//
//	PURPOSE:	Update the blood pool damage effect
//
// ----------------------------------------------------------------------- //

void BodyProp::UpdateAcidPool()
{
	// Make sure we are not still animating
	LTAnimTracker* pTracker;
	g_pLTServer->GetModelLT()->GetMainTracker(m_hObject, pTracker);

	uint32 dwState; 
	g_pLTServer->GetModelLT()->GetPlaybackState(pTracker, dwState);

	if(dwState & MS_PLAYDONE)
	{
		// See if it is time to do another expolsion
		LTFLOAT fTime = g_pLTServer->GetTime();

		if(fTime - m_fLastAcidSpew > g_cvarAcidDamageDelay.GetFloat())
		{
			// Record the new time
			m_fLastAcidSpew = fTime;

			// Create the expolsion
			HCLASS hClass = g_pLTServer->GetClass("Explosion");
			if (!hClass) return;

			ObjectCreateStruct theStruct;
			INIT_OBJECTCREATESTRUCT(theStruct);

#ifdef _DEBUG
			sprintf(theStruct.m_Name, "%s-Explosion", g_pLTServer->GetObjectName(m_hObject) );
#endif
			Explosion* pExplosion = (Explosion*)g_pLTServer->CreateObject(hClass, &theStruct);

			if (pExplosion)
			{
				EXPLOSION_CREATESTRUCT expCS;

				expCS.bHideFromShooter	= LTFALSE;
				expCS.bRemoveWhenDone	= LTTRUE;
				expCS.eDamageType		= DT_ACID;
				expCS.fDamageRadius		= g_cvarAcidPoolRad.GetFloat();
				expCS.fMaxDamage		= GetRandom(g_cvarMinAcidPoolDamage.GetFloat(), g_cvarMaxAcidPoolDamage.GetFloat());
				expCS.hFiredFrom		= m_hObject;
				expCS.nImpactFXId		= FXBMGR_INVALID_ID;

				// Get torso position
				ModelLT* pModelLT   = g_pLTServer->GetModelLT();

				HMODELNODE hNode;
				LTVector vPos;

				if (pModelLT->GetNode(m_hObject, "Torso_u_node", hNode) == LT_OK)
				{
					LTransform	tTransform;

					if (pModelLT->GetNodeTransform(m_hObject, hNode, tTransform, LTTRUE) == LT_OK)
					{
						TransformLT* pTransLT = g_pLTServer->GetTransformLT();
						pTransLT->GetPos(tTransform, expCS.vPos);
						pExplosion->Setup(expCS);
						return;
					}
				}

				//hmm... if we get to here then something is wrong!
				g_pLTServer->RemoveObject(pExplosion->m_hObject);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::CanDoBloodPool()
//
//	PURPOSE:	Should this death type make a blood pool?
//
// ----------------------------------------------------------------------- //

LTBOOL BodyProp::CanDoBloodPool()
{
	switch(m_eDeathType)
	{
		case CD_FREEZE:		
		case CD_BURN:		
		case CD_FACEHUG:		
		case CD_CHESTBURST:		return LTFALSE;

		
		case CD_HEADBITE:	
		case CD_GIB:			
		case CD_SPEAR:			
		case CD_SLICE:			
		default:				return LTTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::CheckForGibs()
//
//	PURPOSE:	Check to see about gibs
//
// ----------------------------------------------------------------------- //

LTBOOL BodyProp::CheckForGibs( LTBOOL bFirey, LTVector vVelOffset, LTBOOL bUseHistory)
{
	if(m_eNodeLastHit != eModelNodeInvalid)
	{
		LTFLOAT fHistDamage;

		if(bUseHistory)
			fHistDamage = m_damage.GetHistoryDamageAmount(g_BodyDamageHistoryTime.GetFloat(), (uint32)m_eNodeLastHit);
		else
			fHistDamage = m_fLastDamage;

		LTFLOAT fFactor = g_pModelButeMgr->GetSkeletonNodeDamageFactor(m_eModelSkeleton, m_eNodeLastHit);

		fHistDamage /= fFactor;

		if(g_pModelButeMgr->ExplodeNode(m_eModelSkeleton, m_eNodeLastHit, fHistDamage))
		{
			ExplodeLimb(m_eNodeLastHit, bFirey, vVelOffset);
			return LTTRUE;
		}
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::HasFlyingDeathAnims()
//
//	PURPOSE:	Get a random flying animation...
//
// ----------------------------------------------------------------------- //

LTBOOL BodyProp::HasFlyingDeathAnims()
{
	HMODELANIM hAnim = INVALID_ANI;
	LTBOOL bRval = LTTRUE;

	hAnim =  g_pLTServer->GetAnimIndex(m_hObject, "Death_f_Thrw_Start0");
	if(hAnim == INVALID_ANI) bRval = LTFALSE;
	hAnim =  g_pLTServer->GetAnimIndex(m_hObject, "Death_f_Thrw0");
	if(hAnim == INVALID_ANI) bRval = LTFALSE;
	hAnim =  g_pLTServer->GetAnimIndex(m_hObject, "Death_f_Thrw_end0");
	if(hAnim == INVALID_ANI) bRval = LTFALSE;
	hAnim =  g_pLTServer->GetAnimIndex(m_hObject, "Death_f_Thrw_s0");
	if(hAnim == INVALID_ANI) bRval = LTFALSE;

	return bRval;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::DoSpecialDamageSound()
//
//	PURPOSE:	Do the special damage sound...
//
// ----------------------------------------------------------------------- //

void BodyProp::DoSpecialDamageSound(HOBJECT hDamager, LTBOOL bHeadOnly)
{
	if(!hDamager) return;

	// Play the evil sound!
	if( IsPlayer(hDamager) && IsPredator(hDamager))
	{
		HitLocation eLocation =	g_pModelButeMgr->GetSkeletonNodeHitLocation(m_eModelSkeleton, m_eNodeLastHit);

		switch (eLocation)
		{
			case HL_HEAD :
				//if the head is not a synth then give credit where credit is due...
				if(!IsSynthetic(GetCharacter()) && m_bHeadAvailable)
				{
					g_pServerSoundMgr->PlaySoundFromObject(hDamager, "pred_headgib");

					//  Increment our skull count if applicable.
					{
					CPlayerObj* pPlayer = (CPlayerObj*)g_pServerDE->HandleToObject(hDamager);

					if(pPlayer)
					{
						HMESSAGEWRITE hMsg = g_pInterface->StartMessage(pPlayer->GetClient(), MID_ADD_SKULL);
						g_pInterface->EndMessage(hMsg);
					}
					}

					// Now send a trigger message to any event counter
					{
					ObjArray<HOBJECT,1> objArray;
					if( LT_OK == g_pServerDE->FindNamedObjects( "TrophyCounter", objArray ) )
					{
						if( objArray.NumObjects() > 0 )
						{
							SendTriggerMsgToObject(this, 	objArray.GetObject(0), "INC");
						}
					}
					}

					// ok, one head is enough...
					m_bHeadAvailable = LTFALSE;
				}

			break;

			case HL_TORSO :
				if(!bHeadOnly)
					g_pServerSoundMgr->PlaySoundFromObject(hDamager, "pred_bodygib");
			break;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::IncrementClawHits()
//
//	PURPOSE:	Increment the number of claw hits taken...
//
// ----------------------------------------------------------------------- //

void BodyProp::IncrementClawHits()
{
	// Incriment our counter
	m_nClawHits++;

	if(m_nClawHits == s_nMaxClawImpacts && g_pGameServerShell->LowViolence())
	{
		// make the body start to fade now!
		if(m_bFadeAway)
		{
			// get the current time
			LTFLOAT fTime = g_pLTServer->GetTime();

			// see if we are not already faiding
			LTFLOAT fStartEffectTime = m_fStartTime + g_BodyLifetimeTrack.GetFloat() - g_BodyFadeOffEffectTime.GetFloat();

			if(fTime >= fStartEffectTime)
				return;

			// ok time to set us faiding
			m_fStartTime = fTime - g_BodyLifetimeTrack.GetFloat() + g_BodyFadeOffEffectTime.GetFloat();

			// don't give any more healing
			m_fClawHealPoints = 0.0f;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void BodyProp::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	int i = 0;

	if(!hWrite) return;

	*hWrite << m_nCharacterSet;
	*hWrite << m_fHeadBiteHealPoints;
	*hWrite << m_fClawHealPoints;
	*hWrite << m_hLastDamager;

	uint32 nTemp;

	nTemp = m_eModelSkeleton;	*hWrite << nTemp;
	nTemp = m_eNodeLastHit;		*hWrite << nTemp;
	nTemp = m_eDamageType;		*hWrite << nTemp;
	nTemp = m_eDeathType;		*hWrite << nTemp;

	*hWrite << m_vDeathDir;
	*hWrite << m_hDeathAnim;
	*hWrite << m_hStaticDeathAnim;
	*hWrite << m_hFaceHugger;
	*hWrite << m_bCanGIB;
	*hWrite << m_hstrInitialMessage;
	*hWrite << m_hHitBox;
	*hWrite << m_vColor;
	hWrite->WriteFloat(m_fStartTime - g_pLTServer->GetTime());
	hWrite->WriteFloat(m_fFaceHugTime - g_pLTServer->GetTime());
	*hWrite << m_fAdjustFactor;
	*hWrite << m_bFirstUpdate;
	*hWrite << m_bLimbProp;		
	*hWrite << m_bSliding;			
	*hWrite << m_bFadeAway;	
	*hWrite << m_bDimsSet;	
	*hWrite << m_bAcidPool;		
	hWrite->WriteFloat(m_fLastAcidSpew - g_pLTServer->GetTime());
	*hWrite << m_fLastDamage;	
	*hWrite << m_bFadeIn;	
	hWrite->WriteFloat(m_fDecloakStartTime - g_pLTServer->GetTime());
	*hWrite << m_bUpdateGravity;
	
	*hWrite << m_vOrigDims;

	hWrite->WriteDWord( m_pCurrentVolume ? m_pCurrentVolume->GetID() : CAIVolume::kInvalidID );

	//now build the hide status flag
	uint32 nHideFlags = 0;

	//get the info and save it
	for(i=0; i<32 ; i++)
	{
		LTBOOL bHidden;
		g_pModelLT->GetPieceHideStatus(m_hObject, (HMODELPIECE)i, bHidden);
		if(bHidden)
			nHideFlags |= (1<<i);
	}

	*hWrite << nHideFlags;	

	*hWrite << m_cSpears;				
	for ( i = 0 ; i < kMaxSpears ; i++ )
		m_aSpears[i].Save(hWrite);

	*hWrite << m_bHeadAvailable;				
	*hWrite << m_nClawHits;		
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void BodyProp::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	int i = 0;

	if(!hRead) return;

	if( m_hFaceHuggerAttachment )
	{
		g_pLTServer->RemoveAttachment(m_hFaceHuggerAttachment);
		m_hFaceHuggerAttachment = LTNULL;
	}

	*hRead >> m_nCharacterSet;
	

	*hRead >> m_fHeadBiteHealPoints;
	*hRead >> m_fClawHealPoints;
	*hRead >> m_hLastDamager;

	uint32 nTemp; 

	*hRead >> nTemp;	m_eModelSkeleton	= (ModelSkeleton)nTemp;
	*hRead >> nTemp;	m_eNodeLastHit		= (ModelNode)nTemp;
	*hRead >> nTemp;	m_eDamageType		= (DamageType)nTemp;
	*hRead >> nTemp;	m_eDeathType		= (DeathType)nTemp;

	*hRead >> m_vDeathDir;
	*hRead >> m_hDeathAnim;
	*hRead >> m_hStaticDeathAnim;
	*hRead >> m_hFaceHugger;
	if( m_hFaceHugger )
	{
		g_pLTServer->FindAttachment(m_hObject, m_hFaceHugger, &m_hFaceHuggerAttachment);
	}
	*hRead >> m_bCanGIB;
	*hRead >> m_hstrInitialMessage;
	*hRead >> m_hHitBox;
	*hRead >> m_vColor;
	m_fStartTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	m_fFaceHugTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	*hRead >> m_fAdjustFactor;
	*hRead >> m_bFirstUpdate;
	*hRead >> m_bLimbProp;		
	*hRead >> m_bSliding;			
	*hRead >> m_bFadeAway;	
	*hRead >> m_bDimsSet;	
	*hRead >> m_bAcidPool;		
	m_fLastAcidSpew = hRead->ReadFloat() + g_pLTServer->GetTime();
	*hRead >> m_fLastDamage;	
	*hRead >> m_bFadeIn;	
	m_fDecloakStartTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	*hRead >> m_bUpdateGravity;

	*hRead >> m_vOrigDims;


	const uint32 current_vol_id = hRead->ReadDWord();
	m_pCurrentVolume = g_pAIVolumeMgr->GetVolumePtr(current_vol_id);

	//now re-build the hide status flag
	uint32 nHideFlags;
	*hRead >> nHideFlags;	

	//make the piece invisible or not
	for(i=0; i<32 ; i++)
	{
		LTBOOL bHidden = (nHideFlags & (1<<i));
		g_pModelLT->SetPieceHideStatus(m_hObject, (HMODELPIECE)i, bHidden);
	}

	// We need to reset our hit box, as it doesn't save the node info.
	if( m_hHitBox )
	{
		CCharacterHitBox * pHitBox = dynamic_cast<CCharacterHitBox*>( g_pLTServer->HandleToObject(m_hHitBox) );
		if( pHitBox )
			pHitBox->ResetSkeleton();
	}

	*hRead >> m_cSpears;				
	for ( i = 0 ; i < kMaxSpears ; i++ )
		m_aSpears[i].Load(hRead);

	// Must be wrapped by GetLastSaveLoadVersion()
	if( g_pGameServerShell->GetLastSaveLoadVersion() >= 200111210)
	{
		*hRead >> m_bHeadAvailable;				
		*hRead >> m_nClawHits;			
	}
}


// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

LTRESULT	BodyPropPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	if (!aszStrings || !pcStrings) return LT_UNSUPPORTED;

	if (_stricmp("CharacterType", szPropName) == 0)
	{
		m_CharacterButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}


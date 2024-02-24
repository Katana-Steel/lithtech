// ----------------------------------------------------------------------- //
//
// MODULE  : AIPlayer.cpp
//
// PURPOSE : AIPlayer object - Implementation
//
// CREATED : 1/1/01
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "AIPlayer.h"
#include "PlayerObj.h"
#include "CharacterFuncs.h"

BEGIN_CLASS(AIPlayer)
	ADD_STRINGPROP_FLAG_HELP(Weapon, const_cast<char*>(g_szNone), 0, "AI's primary weapon.")
	ADD_STRINGPROP_FLAG_HELP(SecondaryWeapon, const_cast<char*>(g_szNone), 0, "If AI cannot use primary weapon, use this weapon.")
	ADD_STRINGPROP_FLAG_HELP(TertiaryWeapon, const_cast<char*>(g_szNone), 0, "If AI cannot use primary or secondary weapon, use this weapon.")
END_CLASS_DEFAULT_FLAGS(AIPlayer, CAI, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CAI()
//
//	PURPOSE:	Constructor - Initialize
//
// ----------------------------------------------------------------------- //

AIPlayer::AIPlayer()
{
	SetCharacter(g_pCharacterButeMgr->GetSetFromModelType("Harris_AI"));
}

AIPlayer::~AIPlayer()
{
}

void AIPlayer::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!g_pLTServer || !hWrite) return;
	
	CAI::Save(hWrite,dwSaveFlags);
}


void AIPlayer::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!g_pLTServer || !hRead) return;

	CAI::Load(hRead,dwLoadFlags);

}

void AIPlayer::SetupForSwap( const CPlayerObj & player)
{
	const CharacterMovement & player_movement = *player.GetMovement();

	GetMovement()->SetScale( player_movement.GetScale() );
	GetMovement()->SetPos( player_movement.GetPos(), MOVEOBJECT_TELEPORT );
	GetMovement()->SetRot( player_movement.GetRot() );

	SetHasMask( player.HasMask() );
	SetHasNightVision( player.HasNightVision() );
	SetHasCloakDevice( player.HasCloakDevice() );
	SetCloakState( player.GetCloakState() );

	GetMovement()->SetObjectUserFlags( player_movement.GetObjectUserFlags());

	GetMovement()->SetMovementState(CMS_IDLE);

	GetMovement()->GetCharacterVars()->m_fStandingOnCheckTime = 0.0f;
	GetMovement()->GetCharacterVars()->m_bStandingOn = LTTRUE;

}

bool AIPlayer::CanOpenDoor() const 
{ 
	return !IsAlien(GetButes()); 
}

void AIPlayer::UpdateActionAnimation()
{
	if( !IsAlien(GetButes()) )
	{
		CAI::UpdateActionAnimation();
	}
	else
	{
		if(    GetMovement()->GetMovementState() == CMS_POUNCING 
			|| GetMovement()->GetMovementState() == CMS_FACEHUG  )
		{
			GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Pounce" );
			return;
		}

		if(    GetMovement()->GetMovementState() == CMS_FACEHUG_ATTACH )
		{
			GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Facehug" );
			return;
		}

		if( !GetCurrentAction() )
		{
			if( GetMovement()->GetMovementState() == CMS_WALLWALKING )
				GetAnimation()->SetLayerReference(ANIMSTYLELAYER_ACTION, "Crouch");
			else
				CAI::UpdateActionAnimation();
		}
	}
}

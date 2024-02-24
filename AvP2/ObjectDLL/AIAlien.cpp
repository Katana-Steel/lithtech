// ----------------------------------------------------------------------- //
//
// MODULE  : AIAlien.cpp
//
// PURPOSE : AIAlien object - Implementation
//
// CREATED : 5/19/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "AIAlien.h"
#include "AIAlienAttack.h"
#include "AIButeMgr.h"
#include "ObjectMsgs.h"
#include "SimpleAIAlien.h"
#include "AIScript.h"

#include "AIUtils.h" // for operator<<(ILTMessage &, std::string &)
BEGIN_CLASS(AIAlien)

	ADD_STRINGPROP_FLAG(AttributeTemplate, "BasicAlien", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(CharacterType, "Drone", PF_STATICLIST)
	ADD_BOOLPROP_FLAG_HELP(UseDisplay, LTFALSE, 0, "If true, the AI will put on a display before attacking.  AI will not attack until target is seen.")
	ADD_COLORPROP_FLAG_HELP(DEditColor, 255.0f, 255.0f, 0.0f, 0, "Color used for object in DEdit.") 

#ifdef _WIN32	
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models\\Characters\\drone.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#else
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models/Characters/drone.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#endif

	// These need to be here in order to get CAIAlienPlugin to fill the static list.
	ADD_STRINGPROP_FLAG_HELP(Weapon, "", PF_STATICLIST, "AI's primary weapon.")
	ADD_STRINGPROP_FLAG_HELP(SecondaryWeapon, "", PF_STATICLIST, "If AI cannot use primary weapon, use this weapon.")
	ADD_STRINGPROP_FLAG_HELP(TertiaryWeapon, "", PF_STATICLIST, "If AI cannot use primary or secondary weapon, use this weapon.")

	ADD_BOOLPROP_FLAG_HELP(UseCover,	LTTRUE,	PF_GROUP4, "AI will use cover nodes if available." )
	ADD_BOOLPROP_FLAG_HELP(NeverLoseTarget,	LTTRUE,	PF_HIDDEN, "This should always be true for aliens!" )

END_CLASS_DEFAULT_FLAGS_PLUGIN(AIAlien, CAI, NULL, NULL, 0, CAIAlienPlugin)


#include "CharacterMgr.h"
#include "AITarget.h"
#include "PlayerObj.h"



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CAI()
//
//	PURPOSE:	Constructor - Initialize
//
// ----------------------------------------------------------------------- //

AIAlien::AIAlien()
	: m_bUseDumbAttack(LTFALSE)
{
	// Default the character to the Alien
	SetCharacter(g_pCharacterButeMgr->GetSetFromModelType("Runner_AI"));
}

AIAlien::~AIAlien()
{
}


LTBOOL AIAlien::IgnoreDamageMsg(const DamageStruct& damage)
{
	if (!g_pLTServer) 
		return LTTRUE;

	if( damage.eType == DT_ACID )
	{
		return LTTRUE;
	}

	return CAI::IgnoreDamageMsg(damage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL AIAlien::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	if (!g_pLTServer || !pData) return LTFALSE;

	// Override the CharacterType from CCharacter::ReadProp.
	if ( g_pLTServer->GetPropGeneric( "UseDisplay", &genProp ) == LT_OK )
	{
		if ( genProp.m_Bool )
		{
			m_strDisplayAnim = "Idl2";
		}
		else
		{
			m_strDisplayAnim.clear();
		}
	} 

	return CAI::ReadProp(pData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PostPropRead()
//
//	PURPOSE:	Initialize model
//
// ----------------------------------------------------------------------- //

void AIAlien::PostPropRead(ObjectCreateStruct *pStruct)
{
	CAI::PostPropRead(pStruct);

	if (!g_pLTServer || !pStruct) return;

	// Read our attributes
	if( GetTemplateId() >= 0 )
	{
		const AIBM_Template & butes = g_pAIButeMgr->GetTemplate(GetTemplateId());

		m_bUseDumbAttack = butes.bUseDumbAttack;
	}
}

void AIAlien::InitialUpdate()
{
	if( !m_strDisplayAnim.empty() )
	{
		const HMODELANIM hDisplayAnim = g_pLTServer->GetAnimIndex(m_hObject, const_cast<char*>(m_strDisplayAnim.c_str()) );

		if( hDisplayAnim == INVALID_MODEL_ANIM)
		{
#ifndef _FINAL
			AIErrorPrint(m_hObject,"Model \"%s\" does not have an \"%s\" animation.  UseDisplay will be ignored.",
				GetButes()->m_szModel, m_strDisplayAnim.c_str() );
#endif
			m_strDisplayAnim.clear();
		}
	}

	CAI::InitialUpdate();

	// Really make sure this is set for the aliens.
	// Sorry for the redundant functionality.
	SetNeverLoseTarget(LTTRUE);
}

Goal * AIAlien::CreateGoal(const char * szGoalName)
{
	ASSERT( szGoalName );
	

	if( stricmp(szGoalName, "ATTACK") == 0 )
	{
		Goal * pGoal = LTNULL;

		if( !m_bUseDumbAttack )
			pGoal = new AlienAttackGoal(this, szGoalName);
		else
			pGoal = new DumbAlienAttackGoal(this,szGoalName);

	
		if( !m_strDisplayAnim.empty() )
		{
			const int nParameterArgs = 2;
			const char * aszParameters[nParameterArgs] = 
			{
				"DisplayAnim",
				m_strDisplayAnim.c_str() 
			};

			pGoal->HandleParameter(aszParameters,nParameterArgs);
		}

		return pGoal;
	}
	else if( stricmp(szGoalName, "Idle") == 0 )
	{
		if( !m_strDisplayAnim.empty() )
		{
			Goal * pGoal = new ScriptGoal(this, szGoalName, LTTRUE);

			const int nParameterArgs = 1;
			const char * aszParameters[nParameterArgs] = 
			{
				"pla Idl2"
			};

			pGoal->HandleParameter(aszParameters,nParameterArgs);

			return pGoal;
		}

	}
	else if( stricmp(szGoalName, "INVESTIGATE") == 0 )
	{
		if( !m_strDisplayAnim.empty() )
		{
			// AI's with a pre-attack display should not investigate.
			return LTNULL;
		}
	}

	return CAI::CreateGoal(szGoalName);
}



void AIAlien::UpdateActionAnimation()
{
//	CCharacter::UpdateActionAnimation();

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

void AIAlien::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!g_pServerDE || !hWrite) return;
	
	*hWrite << m_bUseDumbAttack;
	*hWrite << m_strDisplayAnim;

	// This must be below m_bUseDumbAttack and
	// m_strDisplayAnim, see load for why.
	CAI::Save(hWrite,dwSaveFlags);
}


void AIAlien::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!g_pServerDE || !hRead) return;

	*hRead >> m_bUseDumbAttack;
	*hRead >> m_strDisplayAnim;

	// This must be after m_strDisplayAnim and m_bUseDumbAttack,
	// as those affect how CreateGoal works and CreateGoal
	// is used to restore the AI's goals on load.
	CAI::Load(hRead,dwLoadFlags);
}

bool CAIAlienPlugin::CanUseWeapon(const AIWBM_AIWeapon & butes) const
{
	return butes.eRace == AIWBM_AIWeapon::Alien;
}

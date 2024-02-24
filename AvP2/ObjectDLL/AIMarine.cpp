// ----------------------------------------------------------------------- //
//
// MODULE  : AIMarine.cpp
//
// PURPOSE : AIMarine object - Implementation
//
// CREATED : 7/6/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "AIMarine.h"
#include "AIButeMgr.h"
#include "ObjectMsgs.h"


BEGIN_CLASS(AIMarine)
	ADD_STRINGPROP_FLAG(AttributeTemplate, "BasicMarine", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(CharacterType, "Harris", PF_STATICLIST)
	ADD_COLORPROP_FLAG_HELP(DEditColor, 0.0f, 255.0f, 255.0f, 0, "Color used for object in DEdit.") 

#ifdef _WIN32
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models\\Characters\\harris.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#else
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models/Characters/harris.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#endif

	ADD_BOOLPROP_FLAG_HELP(UseCover, LTTRUE, PF_GROUP4, "AI will use cover nodes if available." )
	ADD_BOOLPROP_FLAG_HELP(UseShoulderLamp, LTTRUE, PF_GROUP4, "Setting this to TRUE adds a ShoulderLamp attachment to this AI.")

	// These need to be here in order to get CAIMarinePlugin to fill the static list.
	ADD_STRINGPROP_FLAG_HELP(Weapon, "", PF_STATICLIST, "AI's primary weapon.")
	ADD_STRINGPROP_FLAG_HELP(SecondaryWeapon, "", PF_STATICLIST, "If AI cannot use primary weapon, use this weapon.")
	ADD_STRINGPROP_FLAG_HELP(TertiaryWeapon, "", PF_STATICLIST, "If AI cannot use primary or secondary weapon, use this weapon.")

END_CLASS_DEFAULT_FLAGS_PLUGIN(AIMarine, CAI, NULL, NULL, 0, CAIMarinePlugin)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CAI()
//
//	PURPOSE:	Constructor - Initialize
//
// ----------------------------------------------------------------------- //

AIMarine::AIMarine()
{
	// Default the character to the Marine
	SetCharacter(g_pCharacterButeMgr->GetSetFromModelType("Harris_AI"));
}

AIMarine::~AIMarine()
{
}


void AIMarine::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!g_pServerDE || !hWrite) return;
	
	CAI::Save(hWrite,dwSaveFlags);
}


void AIMarine::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!g_pServerDE || !hRead) return;

	CAI::Load(hRead,dwLoadFlags);

}

bool CAIMarinePlugin::CanUseWeapon(const AIWBM_AIWeapon & butes) const
{
	return butes.eRace == AIWBM_AIWeapon::Human;
}

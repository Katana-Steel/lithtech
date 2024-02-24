// ----------------------------------------------------------------------- //
//
// MODULE  : AIPredator.cpp
//
// PURPOSE : AIPredator object - Implementation
//
// CREATED : 7/6/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "AIPredator.h"
#include "AIButeMgr.h"
#include "ObjectMsgs.h"
#include "BodyProp.h"
#include "AIUtils.h"


BEGIN_CLASS(AIPredator)
	ADD_STRINGPROP_FLAG(AttributeTemplate, "BasicPredator", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(CharacterType, "Predator", PF_STATICLIST)
	ADD_COLORPROP_FLAG_HELP(DEditColor, 0.0f, 255.0f, 0.0f, 0, "Color used for object in DEdit.") 

#ifdef _WIN32
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models\\Characters\\predator.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#else
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models/Characters/predator.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#endif

	// These need to be here in order to get CAIMarinePlugin to fill the static list.
	ADD_STRINGPROP_FLAG_HELP(Weapon, "", PF_STATICLIST, "AI's primary weapon.")
	ADD_STRINGPROP_FLAG_HELP(SecondaryWeapon, "", PF_STATICLIST, "If AI cannot use primary weapon, use this weapon.")
	ADD_STRINGPROP_FLAG_HELP(TertiaryWeapon, "", PF_STATICLIST, "If AI cannot use primary or secondary weapon, use this weapon.")

	ADD_BOOLPROP_FLAG_HELP(HasMask, LTTRUE, PF_GROUP4, "Should this predator wear a face mask?")
	ADD_BOOLPROP_FLAG_HELP(StartCloaked, LTFALSE, PF_GROUP4, "If TRUE, this Predator starts out with cloaking turned on.")

END_CLASS_DEFAULT_FLAGS_PLUGIN(AIPredator, CAI, NULL, NULL, 0, CAIPredatorPlugin)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CAI()
//
//	PURPOSE:	Constructor - Initialize
//
// ----------------------------------------------------------------------- //

AIPredator::AIPredator()
	: m_bStartCloaked(LTFALSE),
	  m_bStartWithMask(LTTRUE)
{
	// Default the character to the Predator
	SetCharacter(g_pCharacterButeMgr->GetSetFromModelType("Predator_AI"));
}

AIPredator::~AIPredator()
{
}

LTBOOL AIPredator::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	if (!g_pLTServer || !pData)
		return LTFALSE;

	CAI::ReadProp(pData);

	if ( g_pServerDE->GetPropGeneric("StartCloaked", &genProp ) == DE_OK )
		m_bStartCloaked = genProp.m_Bool;

	if ( g_pServerDE->GetPropGeneric("HasMask", &genProp ) == DE_OK )
		m_bStartWithMask = genProp.m_Bool;

	return LTTRUE;
}


DDWORD AIPredator::EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			DDWORD dwRet = CAI::EngineMessageFn(messageID, pData, fData);

			SetHasMask(m_bStartWithMask);

			SetHasCloakDevice(LTTRUE);
			
			if (m_bStartCloaked)
				SetCloakState(CLOAK_STATE_ACTIVE);

			return dwRet;
			break;
		}
		default:
		{
			break;
		}
	}

	return CAI::EngineMessageFn(messageID, pData, fData);
}

void AIPredator::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!g_pServerDE || !hWrite) return;
	
	CAI::Save(hWrite,dwSaveFlags);

	*hWrite << m_bStartCloaked;
	*hWrite << m_bStartWithMask;
}


void AIPredator::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!g_pServerDE || !hRead) return;

	CAI::Load(hRead,dwLoadFlags);

	*hRead >> m_bStartCloaked;
	*hRead >> m_bStartWithMask;
}

bool CAIPredatorPlugin::CanUseWeapon(const AIWBM_AIWeapon & butes) const
{
	return butes.eRace == AIWBM_AIWeapon::Predator;
}



BEGIN_CLASS(AIPredatorSD)
ADD_STRINGPROP_FLAG_HELP(DeathAni, "", 0, "Name of the animation this AI should use when he dies or is destroyed")
END_CLASS_DEFAULT_FLAGS(AIPredatorSD, AIPredator, NULL, NULL, 0)

LTBOOL AIPredatorSD::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	if (!g_pLTServer || !pData)
		return LTFALSE;

	AIPredator::ReadProp(pData);

	if (g_pLTServer->GetPropGeneric("DeathAni", &genProp) == LT_OK)
		m_sDeathAni = genProp.m_String;

	return LTTRUE;
}

HOBJECT AIPredatorSD::CreateBody(LTBOOL bCarryOverAttachments)
{
	HCLASS hClass = g_pLTServer->GetClass("BodyProp");
	if (!hClass) return LTNULL;

	std::string name = g_pLTServer->GetObjectName(m_hObject);
	name += "_dead";

	ObjectCreateStruct theStruct;
	theStruct.Clear();

	// Name our body-prop.
	strncpy( theStruct.m_Name, name.c_str(), MAX_CS_FILENAME_LEN );
	theStruct.m_Name[MAX_CS_FILENAME_LEN] = '\0';  // the char array is actually (MAX_CS_FILENAME_LEN + 1) long.

	// Setup the file names.
	g_pCharacterButeMgr->GetDefaultFilenames(m_nCharacterSet, theStruct);

	// Setup the head skin...
	g_pLTServer->GetObjectPos(m_hObject, &theStruct.m_Pos);
	theStruct.m_Pos += LTVector(0.0f, 0.1f, 0.0f);
	g_pLTServer->GetObjectRotation(m_hObject, &theStruct.m_Rotation);

	// Allocate an object...
	BodyProp* pProp = (BodyProp *)g_pLTServer->CreateObject(hClass, &theStruct);

	_ASSERT(pProp);
	if (!pProp) return LTNULL;

	pProp->Setup(this, bCarryOverAttachments, m_sDeathAni, LTTRUE);
	return pProp->m_hObject;
}

void AIPredatorSD::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!g_pServerDE || !hWrite) return;
	
	AIPredator::Save(hWrite,dwSaveFlags);

	*hWrite << m_sDeathAni;
}


void AIPredatorSD::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!g_pServerDE || !hRead) return;

	AIPredator::Load(hRead,dwLoadFlags);

	*hRead >> m_sDeathAni;
}


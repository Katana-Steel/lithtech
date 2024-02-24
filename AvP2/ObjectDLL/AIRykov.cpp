// ----------------------------------------------------------------------- //
//
// MODULE  : AIRykov.cpp
//
// PURPOSE : AIRykov object - Implementation
//
// CREATED : 04/09/01
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "AIRykov.h"
#include "AIButeMgr.h"
#include "ObjectMsgs.h"
#include "AISnipe.h"
#include "CharacterMgr.h"
#include "AITarget.h"
#include "PlayerObj.h"


#define ADD_GOAL_DEFAULTS(number, group, type, priority) \
		ADD_STRINGPROP_FLAG_HELP(GoalType_##number##, type, (group) | PF_DYNAMICLIST, "Type of goal.") \
		ADD_OBJECTPROP_FLAG_HELP(Link_##number##, "", group, "Link to an object used by the goal (can be null).") \
		ADD_STRINGPROP_FLAG_HELP(Parameters_##number##, "", group, "Other parameters for goal (ie. TARGET=Player).")

BEGIN_CLASS(AIRykov)

	ADD_STRINGPROP_FLAG(AttributeTemplate, "Rykov", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(CharacterType, "Rykov", PF_HIDDEN)
	ADD_COLORPROP_FLAG_HELP(DEditColor, 255.0f, 255.0f, 0.0f, 0, "Color used for object in DEdit.") 

	ADD_STRINGPROP_FLAG_HELP(Weapon, "<default>", PF_HIDDEN, "AI's primary weapon.")
	ADD_STRINGPROP_FLAG_HELP(SecondaryWeapon, "<default>", PF_HIDDEN, "If AI cannot use primary weapon, use this weapon.")
	ADD_STRINGPROP_FLAG_HELP(TertiaryWeapon, "<default>", PF_HIDDEN, "If AI cannot use primary or secondary weapon, use this weapon.")

	ADD_BOOLPROP_FLAG_HELP(UseCover, LTTRUE, PF_HIDDEN, "AI will use cover nodes if available." )
	ADD_BOOLPROP_FLAG_HELP(NeverLoseTarget,	LTTRUE,	PF_HIDDEN, "This should always be true for aliens!" )

	PROP_DEFINEGROUP(Goals, PF_GROUP5 | PF_HIDDEN)
		ADD_GOAL_DEFAULTS(1, PF_GROUP5, "Snipe", 1000.0f )
		ADD_GOAL_DEFAULTS(2, PF_GROUP5, "<none>", 150.0f )

	
END_CLASS_DEFAULT_FLAGS(AIRykov, CAI, NULL, NULL, 0)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIRykov::AIRykov()
//
//	PURPOSE:	Constructor - Initialize
//
// ----------------------------------------------------------------------- //

AIRykov::AIRykov()
{
}

AIRykov::~AIRykov()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIRykov::PostPropRead()
//
//	PURPOSE:	Initialize model
//
// ----------------------------------------------------------------------- //

void AIRykov::PostPropRead(ObjectCreateStruct *pStruct)
{
	CAI::PostPropRead(pStruct);

	if (!g_pLTServer || !pStruct) return;

	// Read our attributes
	if( GetTemplateId() >= 0 )
	{
		const AIBM_Template & butes = g_pAIButeMgr->GetTemplate(GetTemplateId());
	}
}

void AIRykov::InitialUpdate()
{
	CAI::InitialUpdate();

//	SetNeverLoseTarget(LTTRUE);
}

Goal * AIRykov::CreateGoal(const char * szGoalName)
{
	ASSERT( szGoalName );

	Goal * pGoal = LTNULL;

	if( stricmp(szGoalName, "SNIPE") == 0 )
	{
		pGoal = new SnipeGoal(this, szGoalName);
	}
	else
	{
		pGoal = CAI::CreateGoal(szGoalName);
	}

	return pGoal;
}



void AIRykov::UpdateActionAnimation()
{
	CAI::UpdateActionAnimation();
}

void AIRykov::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!g_pServerDE || !hWrite) return;
	
	CAI::Save(hWrite,dwSaveFlags);
}


void AIRykov::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!g_pServerDE || !hRead) return;

	CAI::Load(hRead,dwLoadFlags);
}

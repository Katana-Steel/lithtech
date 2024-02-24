// ----------------------------------------------------------------------- //
//
// MODULE  : GameBase.cpp
//
// PURPOSE : Game base object class implementation
//
// CREATED : 10/8/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GameBase.h"
#include "GameType.h"
#include "CVarTrack.h"
#include "GameServerShell.h"
#include "ObjectMsgs.h"
#include "AIUtils.h" // for AIErrorPrint
#include "TeleportPoint.h"
#include "MultiplayerMgrDefs.h"

// ----------------------------------------------------------------------- //

#define SHOW_DIMS_BOX_MODEL				0x01
#define SHOW_DIMS_BOX_WORLDMODEL		0x02
#define SHOW_DIMS_BOX_CONTAINER			0x04
#define SHOW_DIMS_BOX_NORMAL			0x08
#define SHOW_DIMS_BOX_OTHER				0x10

const LTFLOAT c_fMaxUpdateDelta = 0.5f;

#ifdef _DEBUG
//#define UPDATE_DELTA_DEBUG
#endif

// ----------------------------------------------------------------------- //

#ifndef _FINAL
extern CVarTrack g_ShowDimsTrack;
CVarTrack g_vtDimsAlpha;
#endif

// ----------------------------------------------------------------------- //

BEGIN_CLASS(GameBase)
	PROP_DEFINEGROUP(AllowInGameType, PF_GROUP6)
		ADD_BOOLPROP_FLAG_HELP(SP_Story, LTTRUE, PF_GROUP6, "Single Player - Story Mode")
		ADD_BOOLPROP_FLAG_HELP(MP_DM, LTTRUE, PF_GROUP6, "Multiplayer - Deathmatch Mode")
		ADD_BOOLPROP_FLAG_HELP(MP_TeamDM, LTTRUE, PF_GROUP6, "Multiplayer - Team Deathmatch Mode")
		ADD_BOOLPROP_FLAG_HELP(MP_Hunt, LTTRUE, PF_GROUP6, "Multiplayer - Hunt Mode")
		ADD_BOOLPROP_FLAG_HELP(MP_Survivor, LTTRUE, PF_GROUP6, "Multiplayer - Survivor Mode")
		ADD_BOOLPROP_FLAG_HELP(MP_Overrun, LTTRUE, PF_GROUP6, "Multiplayer - Overrun Mode")
		ADD_BOOLPROP_FLAG_HELP(MP_Evac, LTTRUE, PF_GROUP6, "Multiplayer - Evac Mode")
		ADD_BOOLPROP_FLAG_HELP(PC_Easy, LTTRUE, PF_GROUP6, "PC - Easy Difficulty")
		ADD_BOOLPROP_FLAG_HELP(PC_Medium, LTTRUE, PF_GROUP6, "PC - Medium Difficulty")
		ADD_BOOLPROP_FLAG_HELP(PC_Hard, LTTRUE, PF_GROUP6, "PC - Hard Difficulty")
		ADD_BOOLPROP_FLAG_HELP(PC_Insane, LTTRUE, PF_GROUP6, "PC - Insane Difficulty")
		ADD_BOOLPROP_FLAG_HELP(MP_ClassWeapons, LTTRUE, PF_GROUP6, "This object will be available when the Weapon Class multiplayer setting is used.")
		ADD_BOOLPROP_FLAG_HELP(MP_NoClassWeapons, LTTRUE, PF_GROUP6, "This object will be available when the Weapon Class multiplayer setting is NOT used.")

	ADD_BOOLPROP_FLAG_HELP(StartHidden, LTFALSE, 0, "This will start the object as 'hidden 1'.")
	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS_PLUGIN(GameBase, BaseClass, NULL, NULL, CF_HIDDEN, CGameBasePlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::GameBase()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

GameBase::GameBase(uint8 nType) : BaseClass(nType) 
{ 
	m_dwGameTypes = 0xFFFF;
	m_dwDifficulty = 0xFFFF;
	m_dwClassWeapons = 0xFFFF;
	m_szName = LTNULL;
	m_hDimsBox = LTNULL;
	m_dwOriginalFlags = 0;
	m_bStartHidden = LTFALSE;
	m_bMoveToFloor = LTFALSE;
	m_bTeleportToFloor = LTFALSE;

	m_fUpdateDelta = 0.0f;
	m_fLastUpdateTime = g_pLTServer ? g_pLTServer->GetTime() : 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::~GameBase()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

GameBase::~GameBase() 
{
	if(m_szName)
	{
		g_pLTServer->FreeString(m_szName);
		m_szName = LTNULL;
	}

#ifndef _FINAL
	RemoveBoundingBox();
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::PostStartWorld()
//
//	PURPOSE:	Called after all the objects and geometry has been loaded.
//
// ----------------------------------------------------------------------- //
void GameBase::PostStartWorld(uint8 nLoadGameFlags)
{
	if( nLoadGameFlags != LOAD_RESTORE_GAME )
	{
		// Check if the object was commanded to move to the floor.
		if( m_bMoveToFloor )
		{
			// Consider it done.
			m_bMoveToFloor = FALSE;

			// Move the object to the floor.  Since this is a level placed object
			// we can do a setpos rather than a full moveobject.  Level designers
			// are assumed to place the object correctly.
			MoveObjectToFloor( m_hObject, LTFALSE, LTTRUE );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 GameBase::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    if (!g_pLTServer) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hStr = g_pLTServer->ReadFromMessageHString(hRead);
			TriggerMsg(hSender, g_pLTServer->GetStringData(hStr));
			g_pLTServer->FreeString(hStr);

			// Make sure other people can read it...

            g_pLTServer->ResetRead(hRead);
		}
		break;

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::TriggerMsg()
//
//	PURPOSE:	Process trigger messages
//
// --------------------------------------------------------------------------- //

void GameBase::TriggerMsg(HOBJECT hSender, const char* szMsg)
{
	if (!szMsg) return;

    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return;

	// ConParse does not destroy szMsg, so this is safe
	ConParse parse;
	parse.Init((char*)szMsg);

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
            uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
			if (!m_dwOriginalFlags)
			{
				m_dwOriginalFlags = dwFlags;
			}

			if (_stricmp(parse.m_Args[0], "VISIBLE") == 0)
			{
				if (parse.m_nArgs == 2 && parse.m_Args[1])
				{
					if ((_stricmp(parse.m_Args[1], "1") == 0) ||
						(_stricmp(parse.m_Args[1], "TRUE") == 0))
					{
						dwFlags |= FLAG_VISIBLE;
					}
					else if ((_stricmp(parse.m_Args[1], "0") == 0) ||
						(_stricmp(parse.m_Args[1], "FALSE") == 0))
					{
						dwFlags &= ~FLAG_VISIBLE;
					}

				    g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
				}
#ifndef _FINAL
				else
				{
					if( parse.m_nArgs > 2 )
					{
						AIErrorPrint(m_hObject, "Command \"%s\" has too many arguments! Expecting \"visible 0\" or \"visible 1\".", 
							parse.m_Args[0] );
					}
					else
					{
						AIErrorPrint(m_hObject, "Command \"%s\" does too many arguments! Expecting \"visible 0\" or \"visible 1\".",
							parse.m_Args[0] );
					}

					AIErrorPrint(m_hObject, "Ignoring \"%s\".", szMsg );
				}
#endif
			}
			else if (_stricmp(parse.m_Args[0], "SOLID") == 0)
			{
				if (parse.m_nArgs == 2 && parse.m_Args[1])
				{
					if ((_stricmp(parse.m_Args[1], "1") == 0) ||
						(_stricmp(parse.m_Args[1], "TRUE") == 0))
					{
						dwFlags |= FLAG_SOLID;

						if (m_dwOriginalFlags & FLAG_RAYHIT)
						{
							dwFlags |= FLAG_RAYHIT;
						}
					}
					else if ((_stricmp(parse.m_Args[1], "0") == 0) ||
							(_stricmp(parse.m_Args[1], "FALSE") == 0))
					{
						dwFlags &= ~FLAG_SOLID;
						dwFlags &= ~FLAG_RAYHIT;
					}

				    g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
				}
#ifndef _FINAL
				else
				{
					if( parse.m_nArgs > 2 )
					{
						AIErrorPrint(m_hObject, "Command \"%s\" has too many arguments! Expecting \"solid 0\" or \"solid 1\".", 
							parse.m_Args[0] );
					}
					else
					{
						AIErrorPrint(m_hObject, "Command \"%s\" does too many arguments! Expecting \"solid 0\" or \"solid 1\".",
							parse.m_Args[0] );
					}

					AIErrorPrint(m_hObject, "Ignoring \"%s\".", szMsg );
				}
#endif
			}
			else if (_stricmp(parse.m_Args[0], "HIDDEN") == 0)
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

						if (m_dwOriginalFlags & FLAG_RAYHIT)
						{
							dwFlags |= FLAG_RAYHIT;
						}
					}

					g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
				}
#ifndef _FINAL
				else
				{
					if( parse.m_nArgs > 2 )
					{
						AIErrorPrint(m_hObject, "Command \"%s\" has too many arguments! Expecting \"hidden 0\" or \"hidden 1\".", 
							parse.m_Args[0] );
					}
					else
					{
						AIErrorPrint(m_hObject, "Command \"%s\" does too many arguments! Expecting \"hidden 0\" or \"hidden 1\".",
							parse.m_Args[0] );
					}

					AIErrorPrint(m_hObject, "Ignoring \"%s\".", szMsg );
				}
#endif
			}
			else if (_stricmp(parse.m_Args[0], "REMOVE") == 0)
			{
				g_pLTServer->RemoveObject(m_hObject);
			}
			else if (_stricmp(parse.m_Args[0], "TELEPORT") == 0)
			{
				if (parse.m_nArgs == 4)
				{
					LTVector vPos((float)atof(parse.m_Args[1]), (float)atof(parse.m_Args[2]), (float)atof(parse.m_Args[3]));
					g_pLTServer->TeleportObject(m_hObject, &vPos);
				}
				else if (parse.m_nArgs == 2)
				{
					TeleportPoint* pTeleportPt = DNULL;

					ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
					g_pLTServer->FindNamedObjects(const_cast<char*>(parse.m_Args[1]),objArray);

					if(objArray.NumObjects())
						pTeleportPt = dynamic_cast<TeleportPoint*>(g_pLTServer->HandleToObject(objArray.GetObject(0)));
					else
						return;

					if(pTeleportPt)
					{
						// Get the position to teleport to...
						LTVector vPos;
						g_pLTServer->GetObjectPos(pTeleportPt->m_hObject, &vPos);

						// Get the rotation to teleport to...
						LTRotation rRot;
						g_pLTServer->GetObjectRotation(pTeleportPt->m_hObject, &rRot);


						// Now set this object to these values
						g_pLTServer->SetObjectPos(m_hObject, &vPos);
						g_pLTServer->SetObjectRotation(m_hObject, &rRot);
					}
#ifndef _FINAL
					else
					{
						AIErrorPrint(m_hObject, "Command \"%s\" could not find TeleportPoint object named \"%s\"", 
							parse.m_Args[0], parse.m_Args[1] );
					}
#endif
				}
#ifndef _FINAL
				else
				{
					AIErrorPrint(m_hObject, "Command \"%s\" does not have the right arguments! Expecting \"teleport # # #\".", 
						parse.m_Args[0] );

					AIErrorPrint(m_hObject, "Ignoring \"%s\".", szMsg );
				}
#endif
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::SetNextUpdate()
//
//	PURPOSE:	Allows objects to set their next update time and at
//				time same time update autodeactivation
//
// ----------------------------------------------------------------------- //

void GameBase::SetNextUpdate(LTFLOAT fDelta, LTFLOAT fDeactivateTime)
{
	if (!m_hObject) return;

	fDelta = fDelta <= 0.0f ? 0.0f : fDelta;
    g_pLTServer->SetNextUpdate(m_hObject, fDelta);

	if (fDelta == 0.0f && fDeactivateTime == 0.0f)
	{
		if (!g_pGameServerShell->GetGameType().IsMultiplayer())
		{
			g_pLTServer->SetDeactivationTime(m_hObject, 0.001f);
		}
		g_pLTServer->SetObjectState(m_hObject, OBJSTATE_AUTODEACTIVATE_NOW);
	}
	else if (fDeactivateTime)
	{
		g_pLTServer->SetDeactivationTime(m_hObject, fDeactivateTime + 1.0f);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::CreateBoundingBox()
//
//	PURPOSE:	Create a bounding box
//
// ----------------------------------------------------------------------- //
#ifndef _FINAL
	
void GameBase::CreateBoundingBox()
{
	// If we already created a dims box, get out of this function...
	if(m_hDimsBox)
		return;

	// If our console variable for controlling the alpha value hasn't been
	// initialized yet, then set it to the default value
	if(!g_vtDimsAlpha.IsInitted())
		g_vtDimsAlpha.Init(g_pLTServer, "ShowDimsAlpha", DNULL, 0.9f);

	// Setup the object creation structure
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

#ifdef _DEBUG
	sprintf(theStruct.m_Name, "%s-BoundingBox", g_pLTServer->GetObjectName(m_hObject) );
#endif

	g_pLTServer->GetObjectPos(m_hObject, &(theStruct.m_Pos));
	theStruct.m_Flags = FLAG_VISIBLE;
	theStruct.m_ObjectType = OT_MODEL;

#ifdef _WIN32
	SAFE_STRCPY(theStruct.m_Filename, "Models\\1x1_square.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "Skins\\1x1_square.dtx");
#else
	SAFE_STRCPY(theStruct.m_Filename, "Models/1x1_square.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "Skins/1x1_square.dtx");
#endif

	// Create the model
	HCLASS hClass = g_pLTServer->GetClass("BaseClass");
	LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);

	// If the model creation was successful, set the object handle
	if(pModel)
		m_hDimsBox = pModel->m_hObject;

	if(m_hDimsBox)
	{
		// Initialize position and rotation offsets
		LTVector vOffset;
		vOffset.Init();

		DRotation rOffset;
		ROT_INIT(rOffset);

		// Now set the color
		m_vColor = GetBoundingBoxColor();
		m_fAlpha = g_vtDimsAlpha.GetFloat();
		g_pLTServer->SetObjectColor(m_hDimsBox, m_vColor.x, m_vColor.y, m_vColor.z, m_fAlpha);
	}
}

#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::RemoveBoundingBox()
//
//	PURPOSE:	Remove the bounding box
//
// ----------------------------------------------------------------------- //
#ifndef _FINAL
	
void GameBase::RemoveBoundingBox()
{
	if(m_hDimsBox)
	{
		// Delete the dims box object
		g_pLTServer->RemoveObject(m_hDimsBox);
		m_hDimsBox = DNULL;
	}
}

#endif
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::GetBoundingBoxColor()
//
//	PURPOSE:	Get the color of the bounding box
//
// ----------------------------------------------------------------------- //
#ifndef _FINAL
	
LTVector GameBase::GetBoundingBoxColor()
{
	// Setup a default color
	LTVector vColor(1, 1, 1);

	// Switch the color according to the type of model
	switch(GetType())
	{
		case OT_MODEL:			vColor.Init(1, 1, 1);		break;
		case OT_WORLDMODEL:		vColor.Init(0, 1, 0);		break;
		case OT_CONTAINER:		vColor.Init(0, 0, 1);		break;
		case OT_NORMAL:			vColor.Init(1, 1, 0);		break;
		default:				vColor.Init(1, 1, 1);		break;
	}

	return vColor;
}
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::UpdateBoundingBox()
//
//	PURPOSE:	Update bounding box
//
// ----------------------------------------------------------------------- //
#ifndef _FINAL

void GameBase::UpdateBoundingBox()
{
	// Get the flags for which bounding boxes to display
	int nVal = (int)g_ShowDimsTrack.GetFloat();
	int nType = (int)GetType();

	// Check each type to see if we should display them
	switch(nType)
	{
		case OT_MODEL:		if(!(nVal & SHOW_DIMS_BOX_MODEL))		{ RemoveBoundingBox(); return; }	break;
		case OT_WORLDMODEL:	if(!(nVal & SHOW_DIMS_BOX_WORLDMODEL))	{ RemoveBoundingBox(); return; }	break;
		case OT_CONTAINER:	if(!(nVal & SHOW_DIMS_BOX_CONTAINER))	{ RemoveBoundingBox(); return; }	break;
		case OT_NORMAL:		if(!(nVal & SHOW_DIMS_BOX_NORMAL))		{ RemoveBoundingBox(); return; }	break;
		default:			if(!(nVal & SHOW_DIMS_BOX_OTHER))		{ RemoveBoundingBox(); return; }	break;
	}

	// Create the actual dims box object
	CreateBoundingBox();

	// Update the scale of the dims box object according to the dims of our base object
	if(m_hDimsBox)
	{
		LTVector vDims,vScale;
		g_pLTServer->GetObjectDims(m_hObject, &vDims);

		vScale = (vDims * 2.0);
		g_pLTServer->ScaleObject(m_hDimsBox, &vScale);

		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		g_pLTServer->SetObjectPos(m_hDimsBox, &vPos);

		if(m_fAlpha != g_vtDimsAlpha.GetFloat())
		{
			m_fAlpha = g_vtDimsAlpha.GetFloat();
			g_pLTServer->SetObjectColor(m_hDimsBox, m_vColor.x, m_vColor.y, m_vColor.z, m_fAlpha);
		}


	}
}
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 GameBase::EngineMessageFn(uint32 messageID, void *pData, DFLOAT fData)
{
	uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if(fData == INITIALUPDATE_WORLDFILE)
			{
				if(!AllowInGameType())
				{
					g_pLTServer->RemoveObject(m_hObject);
					return LTFALSE;
				}

				if(m_bStartHidden)
				{
					TriggerMsg(LTNULL, "hidden 1");
				}
			}
		}
		break;

		case MID_UPDATE:
		{
			Update( );
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
		}
		break;

		case MID_ACTIVATING:
		{
			// Flag the object as activated.
			uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags( m_hObject );
			g_pLTServer->SetObjectUserFlags( m_hObject, dwUsrFlags & ~USRFLG_DEACTIVATED );
		}
		break;

		case MID_DEACTIVATING:
		{
			uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags( m_hObject );

			if (IsPlayer(m_hObject) && g_pGameServerShell->GetGameType().IsMultiplayer())
			{
				// Force the flag to active in multiplayer to ensure that we don't send
				// down an object update everytime players go in/out of the vis...
				g_pLTServer->SetObjectUserFlags( m_hObject, dwUsrFlags & ~USRFLG_DEACTIVATED );
			}
			else
			{
				// Flag the object as deactivated.
				g_pLTServer->SetObjectUserFlags( m_hObject, dwUsrFlags | USRFLG_DEACTIVATED );
			}
		}
		break;

		default : break;
	}

	return dwRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL GameBase::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;

	GenericProp genProp;

	if(g_pLTServer->GetPropGeneric("Name", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			if(m_szName)
			{
				g_pLTServer->FreeString(m_szName);
				m_szName = LTNULL;
			}

			m_szName = g_pLTServer->CreateString(genProp.m_String);
		}
	}


	m_dwGameTypes = 0xFFFFFFFF;

    if(g_pLTServer->GetPropGeneric("SP_Story", &genProp) == LT_OK)
	{
		if(!genProp.m_Bool)
			m_dwGameTypes &= ~(1 << SP_STORY);
	}

	if(g_pLTServer->GetPropGeneric("MP_DM", &genProp) == LT_OK)
	{
		if(!genProp.m_Bool)
			m_dwGameTypes &= ~(1 << MP_DM);
	}

	if(g_pLTServer->GetPropGeneric("MP_TeamDM", &genProp) == LT_OK)
	{
		if(!genProp.m_Bool)
			m_dwGameTypes &= ~(1 << MP_TEAMDM);
	}

	if(g_pLTServer->GetPropGeneric("MP_Hunt", &genProp) == LT_OK)
	{
		if(!genProp.m_Bool)
			m_dwGameTypes &= ~(1 << MP_HUNT);
	}

	if(g_pLTServer->GetPropGeneric("MP_Survivor", &genProp) == LT_OK)
	{
		if(!genProp.m_Bool)
			m_dwGameTypes &= ~(1 << MP_SURVIVOR);
	}

	if(g_pLTServer->GetPropGeneric("MP_Overrun", &genProp) == LT_OK)
	{
		if(!genProp.m_Bool)
			m_dwGameTypes &= ~(1 << MP_OVERRUN);
	}

	if(g_pLTServer->GetPropGeneric("MP_Evac", &genProp) == LT_OK)
	{
		if(!genProp.m_Bool)
			m_dwGameTypes &= ~(1 << MP_EVAC);
	}


	m_dwDifficulty = 0xFFFFFFFF;

	if(g_pLTServer->GetPropGeneric("PC_Easy", &genProp) == LT_OK)
	{
		if(!genProp.m_Bool)
			m_dwDifficulty &= ~(1 << DIFFICULTY_PC_EASY);
	}

    if(g_pLTServer->GetPropGeneric("PC_Medium", &genProp) == LT_OK)
	{
		if(!genProp.m_Bool)
			m_dwDifficulty &= ~(1 << DIFFICULTY_PC_MEDIUM);
	}

	if(g_pLTServer->GetPropGeneric("PC_Hard", &genProp) == LT_OK)
	{
		if(!genProp.m_Bool)
			m_dwDifficulty &= ~(1 << DIFFICULTY_PC_HARD);
	}

	if(g_pLTServer->GetPropGeneric("PC_Insane", &genProp) == LT_OK)
	{
		if(!genProp.m_Bool)
			m_dwDifficulty &= ~(1 << DIFFICULTY_PC_INSANE);
	}


	m_dwClassWeapons = 0xFFFFFFFF;

	if(g_pLTServer->GetPropGeneric("MP_ClassWeapons", &genProp) == LT_OK)
	{
		if(!genProp.m_Bool)
			m_dwClassWeapons &= ~(1 << 0);
	}

	if(g_pLTServer->GetPropGeneric("MP_NoClassWeapons", &genProp) == LT_OK)
	{
		if(!genProp.m_Bool)
			m_dwClassWeapons &= ~(1 << 1);
	}


	if(g_pLTServer->GetPropGeneric("StartHidden", &genProp) == LT_OK)
	{
		m_bStartHidden = genProp.m_Bool;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::Update
//
//	PURPOSE:	MID_UPDATE handler.
//
// ----------------------------------------------------------------------- //
void GameBase::Update( )
{
	// Check if the object was commanded to move to the floor.
	if( m_bMoveToFloor )
	{
		// Consider it done.
		m_bMoveToFloor = FALSE;

		// Move the object to the floor.  Since this can happen anytime, 
		// we need to do a full moveobject rather than a setpos.
		MoveObjectToFloor( m_hObject, LTFALSE, LTFALSE );
	}

	if( m_bTeleportToFloor )
	{
		// Consider it done.
		m_bTeleportToFloor = FALSE;

		// Move the object to the floor.  Since this can happen anytime, 
		// we need to do a full moveobject rather than a setpos.
		MoveObjectToFloor( m_hObject, LTFALSE, LTTRUE );
	}

	const LTFLOAT fGameTime = g_pLTServer->GetTime();
	m_fUpdateDelta = fGameTime - m_fLastUpdateTime;
	m_fLastUpdateTime = fGameTime;

#ifdef UPDATE_DELTA_DEBUG
	char szClassName[256];
	g_pLTServer->GetClassName( g_pLTServer->GetObjectClass(m_hObject), szClassName, 256);
	g_pLTServer->CPrint("%f %s (%s) %f %f",
		fGameTime, g_pLTServer->GetObjectName(m_hObject), szClassName, m_fUpdateDelta, m_fLastUpdateTime );
#endif

	// Cap our update time.
	if( m_fUpdateDelta > c_fMaxUpdateDelta )
		m_fUpdateDelta = g_pLTServer->GetFrameTime();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::AllowInGameType()
//
//	PURPOSE:	See if we should create this object for this game type
//
// ----------------------------------------------------------------------- //

LTBOOL GameBase::AllowInGameType()
{
	GameType nGameType = g_pGameServerShell->GetGameType();
	uint32 nFlagType = (1 << nGameType.Get());
	uint32 nFlagDifficulty = (1 << nGameType.GetDifficulty());

	if((m_dwGameTypes & nFlagType) && (m_dwDifficulty & nFlagDifficulty))
	{
		if(nGameType.IsMultiplayer())
		{
			LTBOOL bClassWeapons = g_pGameServerShell->GetGameInfo()->m_bClassWeapons;

			if(bClassWeapons && !(m_dwClassWeapons & 0x00000001))
				return LTFALSE;

			if(!bClassWeapons && !(m_dwClassWeapons & 0x00000002))
				return LTFALSE;
		}

		return LTTRUE;
	}

#ifdef _DEBUG
	char szGameName[64] = "\0";
	nGameType.GetDescription(szGameName, 64);

	g_pLTServer->CPrint("GameBase: Object named \"%s\" was not allowed in game type \"%s\"", g_pLTServer->GetStringData(m_szName), szGameName);
#endif

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void GameBase::Save(HMESSAGEWRITE hWrite)
{
    g_pLTServer->WriteToMessageDWord(hWrite, m_dwOriginalFlags);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fLastUpdateTime - g_pLTServer->GetTime());
    g_pLTServer->WriteToMessageByte(hWrite, !!m_bMoveToFloor);
    g_pLTServer->WriteToMessageByte(hWrite, !!m_bTeleportToFloor);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void GameBase::Load(HMESSAGEREAD hRead)
{
    m_dwOriginalFlags = g_pLTServer->ReadFromMessageDWord(hRead);
	m_fLastUpdateTime = g_pLTServer->ReadFromMessageFloat(hRead) + g_pLTServer->GetTime();
    m_bMoveToFloor = g_pLTServer->ReadFromMessageByte(hRead);
    m_bTeleportToFloor = g_pLTServer->ReadFromMessageByte(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameBasePlugin::PreHook_EditStringList()
//
//	PURPOSE:	Update the static list properties
//
// ----------------------------------------------------------------------- //

LTRESULT CGameBasePlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	// See if we can handle the property...
/*	if(_strcmpi("AllowInGameType", szPropName) == 0)
	{
		for(int i = 0; i < g_nPropGameTypes; i++)
		{
			_ASSERT(cMaxStrings > (*pcStrings) + 1);

			if(cMaxStrings > (*pcStrings) + 1)
			{
				strcpy(aszStrings[(*pcStrings)++], g_pPropGameTypes[i]);
			}
		}

		return LT_OK;
	}
*/
	return LT_UNSUPPORTED;
}


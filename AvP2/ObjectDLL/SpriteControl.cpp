// ----------------------------------------------------------------------- //
//
// MODULE  : SpriteControl.cpp
//
// PURPOSE : SpriteControl implementation
//
// CREATED : 7/12/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SpriteControl.h"
#include "cpp_server_de.h"
#include "ObjectMsgs.h"
#include "MsgIDs.h"

// ----------------------------------------------------------------------- //

BEGIN_CLASS(SpriteControlObj)
	ADD_STRINGPROP_FLAG_HELP(Sprite, "", PF_FILENAME,					"The filename of the sprite this object will use.")

	PROP_DEFINEGROUP(ObjectFlags, PF_GROUP1)
		ADD_BOOLPROP_FLAG_HELP(Visible, LTTRUE, PF_GROUP1,				"Tells if this sprite start as a visible object.")
		ADD_BOOLPROP_FLAG_HELP(SpriteBias, LTFALSE, PF_GROUP1,			"Biases the Z towards the view so a sprite doesn't clip as much.")
		ADD_BOOLPROP_FLAG_HELP(RotateableSprite, LTFALSE, PF_GROUP1,	"This makes the sprite always face the rotation specified in the properties, otherwise it will always face the camera.")
		ADD_BOOLPROP_FLAG_HELP(NoZ, LTFALSE, PF_GROUP1,					"Disable Z read/write on sprite (good for lens flares).")
		ADD_BOOLPROP_FLAG_HELP(GlowSprite, LTFALSE, PF_GROUP1,			"Shrinks the sprite as the viewer gets nearer.")
		ADD_BOOLPROP_FLAG_HELP(FogDisable, LTFALSE, PF_GROUP1,			"Disables fog effects on the sprite.")
		ADD_BOOLPROP_FLAG_HELP(NoLight, LTFALSE, PF_GROUP1,				"Just use the object's color and global light scale. (Don't affect by area or by dynamic lights).")
		ADD_BOOLPROP_FLAG_HELP(Additive, LTFALSE, PF_GROUP1,			"Makes the sprite use additive blending.")
		ADD_BOOLPROP_FLAG_HELP(Multiply, LTFALSE, PF_GROUP1,			"Makes the sprite multiply it's color blending.")
		ADD_BOOLPROP_FLAG_HELP(SkyObject, LTFALSE, PF_GROUP1,			"Don't render this object thru the normal stuff, only render it when processing sky objects.")

	ADD_COLORPROP_FLAG_HELP(Color, 255.0f, 255.0f, 255.0f, 0,			"You can set the color to tint this object.")
	ADD_REALPROP_FLAG_HELP(Alpha, 1.0f, 0,								"The default alpha value for this object.")
	ADD_REALPROP_FLAG_HELP(Scale, 1.0f, 0,								"The scale of the sprite.")

	ADD_STRINGPROP_FLAG_HELP(PlayMode, "Stopped", PF_STATICLIST,		"This tells how the sprite will be updated initially.")
	ADD_LONGINTPROP_FLAG_HELP(StartFrame, 0, 0,							"This is the starting frame of the sprite. If this index is out of range, it will set the value to ZERO.\n\nZero is considered the first frame of the sprite.")
	ADD_LONGINTPROP_FLAG_HELP(EndFrame, -1, 0,							"This is the ending frame of the sprite, If the index is out of range, it will set the value to -1 (last frame).\n\nZero is considered the first frame of the sprite.")
	ADD_LONGINTPROP_FLAG_HELP(Framerate, 10, 0,							"This is the framerate the sprite will play or loop at.")

	ADD_LONGINTPROP_FLAG_HELP(Instructions, 0, 0,						"*** This property does nothing to the object! It is only here in order to list instructions on how to use this object.\n\nIn order to change the functionality of this sprite control object, trigger messages must be sent to it. Here is a list of the available commands:\n\nSTOP, PLAY, or LOOP\nSTARTFRAME <INTEGER>\nENDFRAME <INTEGER>\nFRAMERATE <INTEGER>\nSPRITE <STRING>\nSCALE <FLOAT>\nALPHA <FLOAT>\n<FLAG PROP> <0, 1>\n\nNOTE: Keep in mind that if you change the VISIBLE property, you'll have to reset your basic control properties (framerate, startframe, etc.)\n\nExample 1: If you wanted to make the sprite loop from frames 2 to 6 at 12 frames per second, your trigger message would be:\n   LOOP; STARTFRAME 2; ENDFRAME 6; FRAMERATE 12\n\nExample 2: If you wanted to switch the sprite to a new file and make it stopped at frame 5 when it displays, your trigger message would be:\n   STOP; STARTFRAME 5; SPRITE Sprites\newsprite.spr")

END_CLASS_DEFAULT_FLAGS_PLUGIN(SpriteControlObj, GameBase, LTNULL, LTNULL, 0, CSpriteControlPlugin)


// ----------------------------------------------------------------------- //

#define SPRITECONTROL_PLAYMODE_STOPPED			0
#define SPRITECONTROL_PLAYMODE_PLAYING			1
#define SPRITECONTROL_PLAYMODE_LOOPING			2

// ----------------------------------------------------------------------- //

const int g_nPlayModes = 3;
static char *g_szPlayModes[g_nPlayModes] =
{
	"Stopped",
	"Playing",
	"Looping",
};

const int g_nFlagValues = 7;
static char *g_szFlagValues[g_nFlagValues] =
{
	"Visible",
	"SpriteBias",
	"RotateableSprite",
	"NoZ",
	"GlowSprite",
	"FogDisable",
	"NoLight",
};

static uint32 g_pFlagValues[g_nFlagValues] =
{
	FLAG_VISIBLE,
	FLAG_SPRITEBIAS,
	FLAG_ROTATEABLESPRITE,
	FLAG_SPRITE_NOZ,
	FLAG_GLOWSPRITE,
	FLAG_FOGDISABLE,
	FLAG_NOLIGHT,
};

const int g_nFlag2Values = 3;
static char *g_szFlag2Values[g_nFlag2Values] =
{
	"Additive",
	"Multiply",
	"SkyObject",
};

static uint32 g_pFlag2Values[g_nFlag2Values] =
{
	FLAG2_ADDITIVE,
	FLAG2_MULTIPLY,
	FLAG2_SKYOBJECT,
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpriteControlObj::SpriteControlObj()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

SpriteControlObj::SpriteControlObj() : GameBase(OT_SPRITE)
{
	m_vColor		= LTVector(1.0f, 1.0f, 1.0f);
	m_fAlpha		= 1.0f;
	m_vScale		= LTVector(1.0f, 1.0f, 1.0f);

	m_nPlayMode		= 0;
	m_nStartFrame	= 0;
	m_nEndFrame		= 0;
	m_nFramerate	= 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpriteControlObj::~SpriteControlObj()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

SpriteControlObj::~SpriteControlObj()
{

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpriteControlObj::EngineMessageFn
//
//	PURPOSE:	Handle engine EngineMessageFn messages
//
// ----------------------------------------------------------------------- //

uint32 SpriteControlObj::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData)
{
	uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, lData);

	switch (messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

			if(pStruct)
			{
				// Read the properties
				ReadProp(pStruct);
			}

			return dwRet;
		}

		case MID_INITIALUPDATE:
		{
			if (lData != INITIALUPDATE_SAVEGAME)
			{
				// Setup the sprite color
				g_pLTServer->SetObjectColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z, m_fAlpha);

				// Setup the sprite scale
				g_pLTServer->ScaleObject(m_hObject, &m_vScale);

				// Send the inital SFX message
				HMESSAGEWRITE hWrite = g_pLTServer->StartSpecialEffectMessage(this);
				g_pLTServer->WriteToMessageByte(hWrite, SFX_SPRITE_CONTROL_ID);
				g_pLTServer->WriteToMessageByte(hWrite, m_nPlayMode);
				g_pLTServer->WriteToMessageByte(hWrite, m_nStartFrame);
				g_pLTServer->WriteToMessageByte(hWrite, m_nEndFrame);
				g_pLTServer->WriteToMessageByte(hWrite, m_nFramerate);
				g_pLTServer->EndMessage(hWrite);
			}
			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (uint32)lData);
			break;
		}

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (uint32)lData);
			break;
		}

		default: break;
	}

	return dwRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpriteControlObj::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 SpriteControlObj::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			if(HandleTriggerMsg(hSender, hRead))
				UpdateClients();

			break;
		}
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpriteControlObj::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

LTBOOL SpriteControlObj::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;

	// ----------------------------------------------------------------------- //
	// Fill in the sprite filename to create
	if(g_pLTServer->GetPropGeneric("Sprite", &genProp) == DE_OK)
	{
		if(genProp.m_String[0])
		{
			SAFE_STRCPY(pData->m_Filename, genProp.m_String);
		}
	}

	// ----------------------------------------------------------------------- //
	// Fill in the object flags

	for(int i = 0; i < g_nFlagValues; i++)
	{
		if(g_pLTServer->GetPropGeneric(g_szFlagValues[i], &genProp) == DE_OK)
		{
			if(genProp.m_Bool)
				pData->m_Flags |= g_pFlagValues[i];
		}
	}

	for(int j = 0; j < g_nFlag2Values; j++)
	{
		if(g_pLTServer->GetPropGeneric(g_szFlag2Values[j], &genProp) == DE_OK)
		{
			if(genProp.m_Bool)
				pData->m_Flags2 |= g_pFlag2Values[j];
		}
	}

	// ----------------------------------------------------------------------- //
	// Fill in the object color variables
	if(g_pLTServer->GetPropGeneric("Color", &genProp) == DE_OK)
	{
		m_vColor = genProp.m_Color;
		m_vColor /= 255.0f;
	}

	if(g_pLTServer->GetPropGeneric("Alpha", &genProp) == DE_OK)
	{
		m_fAlpha = genProp.m_Float;
	}

	// ----------------------------------------------------------------------- //
	// Fill in the object scale variables
	if(g_pLTServer->GetPropGeneric("Scale", &genProp) == DE_OK)
	{
		m_vScale = LTVector(genProp.m_Float, genProp.m_Float, genProp.m_Float);
	}

	// ----------------------------------------------------------------------- //
	// Fill in the object playing properties
	if(g_pLTServer->GetPropGeneric("PlayMode", &genProp) == DE_OK)
	{
		m_nPlayMode = 0;

		if(genProp.m_String)
		{
			for(int i = 0; i < g_nPlayModes; i++)
			{
				if(!strcmp(genProp.m_String, g_szPlayModes[i]))
				{
					m_nPlayMode = i;
					break;
				}
			}
		}
	}

	if(g_pLTServer->GetPropGeneric("StartFrame", &genProp) == DE_OK)
	{
		m_nStartFrame = (uint8)genProp.m_Long;
	}

	if(g_pLTServer->GetPropGeneric("EndFrame", &genProp) == DE_OK)
	{
		m_nEndFrame = (uint8)genProp.m_Long;
	}

	if(g_pLTServer->GetPropGeneric("Framerate", &genProp) == DE_OK)
	{
		m_nFramerate = (uint8)genProp.m_Long;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpriteControlObj::HandleTriggerMsg
//
//	PURPOSE:	Handle the message send to this object
//
// ----------------------------------------------------------------------- //

LTBOOL SpriteControlObj::HandleTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead)
{
    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon || !m_hObject) return LTFALSE;

	HSTRING msg = g_pLTServer->ReadFromMessageHString(hRead);
	LTBOOL bRet = LTFALSE;

	ConParse parse;
	parse.Init(g_pLTServer->GetStringData(msg));

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			switch(parse.m_nArgs)
			{
				// ----------------------------------------------------------------------- //
				// Check all the single parameter tokens
				case 1:
				{
					if(_stricmp(parse.m_Args[0], "STOP") == 0)
					{
						m_nPlayMode = SPRITECONTROL_PLAYMODE_STOPPED;
						bRet = LTTRUE;
					}
					else if(_stricmp(parse.m_Args[0], "PLAY") == 0)
					{
						m_nPlayMode = SPRITECONTROL_PLAYMODE_PLAYING;
						bRet = LTTRUE;
					}
					else if(_stricmp(parse.m_Args[0], "LOOP") == 0)
					{
						m_nPlayMode = SPRITECONTROL_PLAYMODE_LOOPING;
						bRet = LTTRUE;
					}
				}
				break;

				// ----------------------------------------------------------------------- //
				// Check all the tokens with 2 parameters
				case 2:
				{
					if(_stricmp(parse.m_Args[0], "STARTFRAME") == 0)
					{
						m_nStartFrame = (uint8)atoi(parse.m_Args[1]);
						bRet = LTTRUE;
					}
					else if(_stricmp(parse.m_Args[0], "ENDFRAME") == 0)
					{
						m_nEndFrame = (uint8)atoi(parse.m_Args[1]);
						bRet = LTTRUE;
					}
					else if(_stricmp(parse.m_Args[0], "FRAMERATE") == 0)
					{
						m_nFramerate = (uint8)atoi(parse.m_Args[1]);
						bRet = LTTRUE;
					}
					else if(_stricmp(parse.m_Args[0], "SPRITE") == 0)
					{
						ObjectCreateStruct ocs;
						ocs.Clear();

						SAFE_STRCPY(ocs.m_Filename, parse.m_Args[1]);
						g_pLTServer->Common()->SetObjectFilenames(m_hObject, &ocs);
					}
					else if(_stricmp(parse.m_Args[0], "SCALE") == 0)
					{
						LTFLOAT fScale = (LTFLOAT)atof(parse.m_Args[1]);
						m_vScale = LTVector(fScale, fScale, fScale);
						g_pLTServer->ScaleObject(m_hObject, &m_vScale);
					}
					else if(_stricmp(parse.m_Args[0], "ALPHA") == 0)
					{
						m_fAlpha = (LTFLOAT)atof(parse.m_Args[1]);
						g_pLTServer->SetObjectColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z, m_fAlpha);
					}
					else
					{
						LTBOOL bFound = LTFALSE;

						if(!bFound)
						{
							// Go through all the FLAG properties and check for changes
							for(int i = 0; i < g_nFlagValues; i++)
							{
								if(_stricmp(parse.m_Args[0], g_szFlagValues[i]) == 0)
								{
									uint32 nFlags;
									g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags, nFlags);

									nFlags = (LTBOOL)atoi(parse.m_Args[1]) ? (nFlags | g_pFlagValues[i]) : (nFlags & ~g_pFlagValues[i]);
									g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, nFlags);

									bFound = LTTRUE;
									break;
								}
							}
						}

						if(!bFound)
						{
							// Go through all the FLAG2 properties and check for changes
							for(int i = 0; i < g_nFlag2Values; i++)
							{
								if(_stricmp(parse.m_Args[0], g_szFlag2Values[i]) == 0)
								{
									uint32 nFlags2;
									g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags2, nFlags2);

									nFlags2 = (LTBOOL)atoi(parse.m_Args[1]) ? (nFlags2 | g_pFlag2Values[i]) : (nFlags2 & ~g_pFlag2Values[i]);
									g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags2, nFlags2);

									bFound = LTTRUE;
									break;
								}
							}
						}
					}
				}
				break;

				default:
				break;
			}
		}
	}

	// We're done... so return the bool value that says whether we need
	// to send a message to the clients
	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpriteControlObj::UpdateClients
//
//	PURPOSE:	Tell the clients that there's been a change to this object
//
// ----------------------------------------------------------------------- //

void SpriteControlObj::UpdateClients()
{
	// Send the immediate changes down to the client
	HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hWrite, SFX_SPRITE_CONTROL_ID);
	g_pLTServer->WriteToMessageObject(hWrite, m_hObject);

	g_pLTServer->WriteToMessageByte(hWrite, m_nPlayMode);
	g_pLTServer->WriteToMessageByte(hWrite, m_nStartFrame);
	g_pLTServer->WriteToMessageByte(hWrite, m_nEndFrame);
	g_pLTServer->WriteToMessageByte(hWrite, m_nFramerate);

	g_pLTServer->EndMessage(hWrite);


	// Reset up the special FX message so that if people come back into the
	// vis-list, the changes will still be correct
	hWrite = g_pLTServer->StartSpecialEffectMessage(this);
	g_pLTServer->WriteToMessageByte(hWrite, SFX_SPRITE_CONTROL_ID);
	g_pLTServer->WriteToMessageByte(hWrite, m_nPlayMode);
	g_pLTServer->WriteToMessageByte(hWrite, m_nStartFrame);
	g_pLTServer->WriteToMessageByte(hWrite, m_nEndFrame);
	g_pLTServer->WriteToMessageByte(hWrite, m_nFramerate);
	g_pLTServer->EndMessage(hWrite);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpriteControlObj::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SpriteControlObj::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	g_pLTServer->WriteToMessageVector(hWrite, &m_vColor);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fAlpha);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vScale);

	g_pLTServer->WriteToMessageByte(hWrite, m_nPlayMode);
	g_pLTServer->WriteToMessageByte(hWrite, m_nStartFrame);
	g_pLTServer->WriteToMessageByte(hWrite, m_nEndFrame);
	g_pLTServer->WriteToMessageByte(hWrite, m_nFramerate);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpriteControlObj::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SpriteControlObj::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	g_pLTServer->ReadFromMessageVector(hRead, &m_vColor);
	m_fAlpha		= g_pLTServer->ReadFromMessageFloat(hRead);
	g_pLTServer->ReadFromMessageVector(hRead, &m_vScale);

	m_nPlayMode		= g_pLTServer->ReadFromMessageByte(hRead);
	m_nStartFrame	= g_pLTServer->ReadFromMessageByte(hRead);
	m_nEndFrame		= g_pLTServer->ReadFromMessageByte(hRead);
	m_nFramerate	= g_pLTServer->ReadFromMessageByte(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpriteControlPlugin::PreHook_EditStringList()
//
//	PURPOSE:	Update the static list properties
//
// ----------------------------------------------------------------------- //

LTRESULT CSpriteControlPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	// See if we can handle the property...
	if(_stricmp("PlayMode", szPropName) == 0)
	{
		for(int i = 0; i < g_nPlayModes; i++)
		{
			if(cMaxStrings > (*pcStrings) + 1)
			{
				strcpy(aszStrings[(*pcStrings)++], g_szPlayModes[i]);
			}
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

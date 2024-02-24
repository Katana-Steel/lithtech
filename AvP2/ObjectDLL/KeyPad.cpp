// ----------------------------------------------------------------------- //
//
// MODULE  : KeyPad.cpp
//
// PURPOSE : Implementation of the Key pad model
//
// CREATED : 4/30/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "KeyPad.h"
#include "SoundMgr.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "WeaponMgr.h"

// Statics

static char s_szGadget[]	= "GADGET";


BEGIN_CLASS(KeyPad)

#ifdef _WIN32
	ADD_STRINGPROP_FLAG(Filename, "Props\\Models\\Keypad.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Props\\Skins\\Keypad.dtx", PF_FILENAME)
#else
	ADD_STRINGPROP_FLAG(Filename, "Props/Models/Keypad.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Props/Skins/Keypad.dtx", PF_FILENAME)
#endif

	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(MoveToFloor, DFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(CanDamage, DFALSE, PF_GROUP1)
	ADD_BOOLPROP_FLAG(NeverDestroy, DTRUE, PF_GROUP1)
	ADD_OBJECTPROP(DisabledTarget, "")
	ADD_STRINGPROP(DisabledMessage, "")
END_CLASS_DEFAULT_FLAGS(KeyPad, Prop, NULL, NULL, PF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::KeyPad()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

KeyPad::KeyPad() : Prop()
{
	m_hstrDisabledMessage	= DNULL;
	m_hstrDisabledTarget	= DNULL;
	m_hDeciphererModel		= DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::~KeyPad()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

KeyPad::~KeyPad()
{
	FREE_HSTRING(m_hstrDisabledTarget);
	FREE_HSTRING(m_hstrDisabledMessage);

	if (m_hDeciphererModel)
	{
		// Remove the model...

		HATTACHMENT hAttachment;
		if (g_pServerDE->FindAttachment(m_hObject, m_hDeciphererModel, &hAttachment) == DE_OK)
		{
			g_pServerDE->RemoveAttachment(hAttachment);
		}
		g_pServerDE->RemoveObject(m_hDeciphererModel);
		m_hDeciphererModel = DNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD KeyPad::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
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

		case MID_INITIALUPDATE :
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
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

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD KeyPad::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	if (!g_pServerDE) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			CommonLT* pCommon = g_pServerDE->Common();
			if (!pCommon) return 0;

			HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			char* szMsg = hMsg ? g_pServerDE->GetStringData(hMsg) : DNULL;
			if (!szMsg) return 0;

			ConParse parse;
			parse.Init(szMsg);

			while (pCommon->Parse(&parse) == LT_OK)
			{
				if (parse.m_nArgs > 0 && parse.m_Args[0])
				{
					if (_stricmp(parse.m_Args[0], s_szGadget) == 0)
					{
						HandleGadgetMsg(parse);
					}
				}
			}

			g_pServerDE->FreeString(hMsg);

			break;
		}

		default : break;
	}

	return Prop::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::SetupDisabledState
//
//	PURPOSE:	Handle gadget messages
//
// ----------------------------------------------------------------------- //

void KeyPad::HandleGadgetMsg(ConParse & parse)
{
	if (parse.m_nArgs < 2 || !parse.m_Args[1]) return;

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(atol(parse.m_Args[1]));
	if (!pAmmo) return;

//	if (pAmmo->eInstDamageType == DT_GADGET_CODE_DECIPHERER)
//	{
//		SetupDisabledState();
//	}
}
  
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL KeyPad::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) return DFALSE;

	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("DisabledMessage", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrDisabledMessage = g_pServerDE->CreateString(genProp.m_String);
		}
	}

	if (g_pServerDE->GetPropGeneric("DisabledTarget", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrDisabledTarget = g_pServerDE->CreateString(genProp.m_String);
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

DBOOL KeyPad::InitialUpdate()
{
	g_pServerDE->SetNextUpdate(m_hObject, 0.0);

	// Setup the object so we can be disabled by the code decipherer...

	const DDWORD dwUsrFlgs = g_pServerDE->GetObjectUserFlags(m_hObject);
	g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlgs | USRFLG_GADGET_CODE_DECIPHERER);


	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::SetupDisabledState
//
//	PURPOSE:	Setup the disabled state
//
// ----------------------------------------------------------------------- //

void KeyPad::SetupDisabledState()
{
	if (m_hDeciphererModel) return;

	DDWORD dwUserFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
	g_pServerDE->SetObjectUserFlags(m_hObject, dwUserFlags & ~USRFLG_GADGET_CODE_DECIPHERER);

	// Create the code decipherer, and attach it to the keypad...

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	DVector vPos;
	g_pServerDE->GetObjectPos(m_hObject, &vPos);
	VEC_COPY(theStruct.m_Pos, vPos);

#ifdef _WIN32
	SAFE_STRCPY(theStruct.m_Filename, "Guns\\Models_HH\\Codedec.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "Guns\\Skins_HH\\Codedec.dtx");
#else
	SAFE_STRCPY(theStruct.m_Filename, "Guns/Models_HH/Codedec.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "Guns/Skins_HH/Codedec.dtx");
#endif
	
	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD;
	theStruct.m_ObjectType  = OT_MODEL;

	HCLASS hClass = g_pServerDE->GetClass("BaseClass");
	LPBASECLASS pModel = g_pServerDE->CreateObject(hClass, &theStruct);
	if (!pModel) return;

	m_hDeciphererModel = pModel->m_hObject;

	// Attach the model to the the camera...

	DVector vOffset;
	VEC_INIT(vOffset);

	DRotation rOffset;
	ROT_INIT(rOffset);

	HATTACHMENT hAttachment;
	DRESULT dRes = g_pServerDE->CreateAttachment(m_hObject, m_hDeciphererModel, "Decipher", 
											     &vOffset, &rOffset, &hAttachment);
	if (dRes != DE_OK)
	{
		g_pServerDE->RemoveObject(m_hDeciphererModel);
		m_hDeciphererModel = DNULL;
		return;
	}

	// Set the Decipherer's animation...

	//HMODELANIM hAni = g_pServerDE->GetAnimIndex(m_hDeciphererModel, "DownUp");
	//if (hAni)
	//{
	//	g_pServerDE->SetModelLooping(m_hDisablerModel, DFALSE);
	//	g_pServerDE->SetModelAnimation(m_hDisablerModel, hAni);
	//}


	// Play the activate sound...

#ifdef _WIN32
	char* pSound = "Guns\\Snd\\Code_dec\\activate.wav";
#else
	char* pSound = "Guns/Snd/Code_dec/activate.wav";
#endif

	g_pSoundMgr->PlaySoundFromPos(vPos, pSound, 500.0f, SOUNDPRIORITY_MISC_LOW);


	if (m_hstrDisabledTarget && m_hstrDisabledMessage)
	{
		SendTriggerMsgToObjects(this, m_hstrDisabledTarget, m_hstrDisabledMessage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void KeyPad::Save(HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	g_pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hDeciphererModel);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrDisabledMessage);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrDisabledTarget);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void KeyPad::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	g_pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hDeciphererModel);
	m_hstrDisabledMessage = g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrDisabledTarget  = g_pServerDE->ReadFromMessageHString(hRead);
}


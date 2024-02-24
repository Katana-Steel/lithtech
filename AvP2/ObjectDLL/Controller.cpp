// ----------------------------------------------------------------------- //
//
// MODULE  : Controller.cpp
//
// PURPOSE : Controller - Implementation
//
// CREATED : 4/17/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "Controller.h"
#include "lmessage.h"
#include "CommonUtilities.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"


BEGIN_CLASS(Controller)
	ADD_STRINGPROP_FLAG(Target0, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target1, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target2, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target3, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target4, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target5, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target6, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target7, "", PF_OBJECTLINK)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(Controller, BaseClass, NULL, NULL)




// ----------------------------------------------------------------------------------------------- //
// ParamValue functions.
// ----------------------------------------------------------------------------------------------- //

void ParamValue::Load(HMESSAGEREAD hRead)
{
	m_Color = hRead->ReadVector();
}

void ParamValue::Save(HMESSAGEWRITE hWrite)
{
	hWrite->WriteVector(m_Color);
}


// ----------------------------------------------------------------------------------------------- //
// Controller functions.
// ----------------------------------------------------------------------------------------------- //

Controller::Controller()
{
	m_fStartTime = 0.0f;
	m_fDuration = 0.0f;
	m_State = CState_Off;
	m_bFirstUpdate = DTRUE;
}


DWORD Controller::EngineMessageFn(DWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			PreCreate((ObjectCreateStruct*)pData);
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate();
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DWORD)fData);
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DWORD)fData);
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


DWORD Controller::ObjectMessageFn(HOBJECT hSender, DWORD messageID, HMESSAGEREAD hRead)
{
	char msgBuf[256];

	switch (messageID)
	{
		case MID_TRIGGER:
		{
			hRead->ReadHStringAsStringFL(msgBuf, sizeof(msgBuf));
			HandleTrigger(hSender, msgBuf);
		}
		break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


void Controller::PreCreate(ObjectCreateStruct *pStruct)
{
	char buf[256];
	DWORD i;
	GenericProp gProp;


	// Get target object names.
	for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		sprintf(buf, "Target%d", i);
		if(g_pServerDE->GetPropGeneric(buf, &gProp) == LT_OK)
		{
			SAFE_STRCPY(m_Fades[i].m_ObjectName, gProp.m_String);
		}
	}
}


void Controller::InitialUpdate()
{
	g_pServerDE->SetNextUpdate(m_hObject, 0.001f);
}


void Controller::FirstUpdate()
{
	DWORD i;
//	ObjectList *pList;

	
	// Find target objects and make interlinks.
	for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		// Kai Martin - 10/26/99
		// replaced linked list of objects in favor of new
		// static array of objects.  
		ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;

		g_pServerDE->FindNamedObjects(m_Fades[i].m_ObjectName,objArray);

		if(objArray.NumObjects())
		{
			HOBJECT hObject = objArray.GetObject(0);

			m_Fades[i].m_hTarget = hObject;
			g_pServerDE->CreateInterObjectLink(m_hObject, m_Fades[i].m_hTarget);
		}
	}
}


void Controller::Update()
{
	DWORD i;
	FadeState *pState;
	float curTime, t;

	if(m_bFirstUpdate)
	{
		FirstUpdate();
		m_bFirstUpdate = DFALSE;
	}

	if(m_State == CState_Fade)
	{
		// Find out if we're even interpolating.
		curTime = g_pServerDE->GetTime();

		if(curTime >= (m_fStartTime + m_fDuration))
		{
			t = 1.0f;
		}
		else
		{
			t = (curTime - m_fStartTime) / m_fDuration;
			t = GetWaveFn(m_WaveType)(t); // Apply wave function.
		}

		for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
		{
			pState = &m_Fades[i];

			if(!pState->m_hTarget)
				continue;

			InterpolateValue(pState, t);
		}

		g_pServerDE->SetNextUpdate(m_hObject, 0.001f);
	}
	else if(m_State == CState_Flicker)
	{
		if(g_pServerDE->GetTime() > m_fNextFlickerTime)
		{
			// Send the message.
			for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
			{
				pState = &m_Fades[i];

				if(!pState->m_hTarget)
					continue;

				SendTriggerMsgToObject(this, pState->m_hTarget, m_FlickerMsg);
			}

			// Go again?
			if(m_FlickerCounter != FLICKER_FOREVER)
				--m_FlickerCounter;

			if(m_FlickerCounter == 0)
				m_State = CState_Off;

			m_fNextFlickerTime = g_pServerDE->GetTime() + GetRandom(m_fIntervalMin, m_fIntervalMax);
		}

		g_pServerDE->SetNextUpdate(m_hObject, 0.001f);
	}
}


void Controller::OnLinkBroken(HOBJECT hObj)
{
	DWORD i;

	// Disable the fade with this object.
	for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		if(m_Fades[i].m_hTarget == hObj)
		{
			m_Fades[i].m_hTarget = DNULL;
		}
	}
}


void Controller::Load(HMESSAGEREAD hRead, DWORD dwSaveFlags)
{
	FadeState *pState;
	DWORD i;

	m_State = (CState)hRead->ReadDWord();

	// Read FLICKER vars.
	m_fNextFlickerTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	m_fIntervalMin = hRead->ReadFloat();
	m_fIntervalMax = hRead->ReadFloat();
	m_FlickerCounter = hRead->ReadDWord();
	m_FlickerMsg[0] = 0;
	hRead->ReadStringFL(m_FlickerMsg, sizeof(m_FlickerMsg));

	// Read FADE vars.
	m_fStartTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	m_fDuration = hRead->ReadFloat();
	m_WaveType = (WaveType)hRead->ReadDWord();
	m_ParamType = (ParamType)hRead->ReadDWord();
	m_DestValue.Load(hRead);
	
	for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		pState = &m_Fades[i];
		
		pState->m_StartVal.Load(hRead);
		pState->m_hTarget = hRead->ReadObject();
	}
}


void Controller::Save(HMESSAGEWRITE hWrite, DWORD dwSaveFlags)
{
	FadeState *pState;
	DWORD i;

	hWrite->WriteDWord((DWORD)m_State);

	// Write FLICKER vars.
	hWrite->WriteFloat(m_fNextFlickerTime - g_pLTServer->GetTime());
	hWrite->WriteFloat(m_fIntervalMin);
	hWrite->WriteFloat(m_fIntervalMax);
	hWrite->WriteDWord(m_FlickerCounter);
	hWrite->WriteString(m_FlickerMsg);
										
	// Write FADE vars.
	hWrite->WriteFloat(m_fStartTime - g_pLTServer->GetTime());
	hWrite->WriteFloat(m_fDuration);
	hWrite->WriteDWord((DWORD)m_WaveType);
	hWrite->WriteDWord((DWORD)m_ParamType);
	m_DestValue.Save(hWrite);
	
	for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		pState = &m_Fades[i];
		
		pState->m_StartVal.Save(hWrite);
		hWrite->WriteObject(pState->m_hTarget);
	}
}


void Controller::HandleTrigger(HOBJECT hSender, char *pMsg)
{
	ConParse parse;
	char *pCmd;


	parse.Init(pMsg);
	if(g_pServerDE->Common()->Parse(&parse) == LT_OK)
	{
		if(parse.m_nArgs == 0)
		{
#ifndef _FINAL
			ShowTriggerError(pMsg);
#endif
			return;
		}

		pCmd = parse.m_Args[0];
		if(stricmp(pCmd, "FADE") == 0)
		{
			HandleFadeCommand(pMsg, &parse);
		}
		else if(stricmp(pCmd, "FLICKER") == 0)
		{
			HandleFlickerCommand(pMsg, &parse);
		}
		else if(stricmp(pCmd, "OFF") == 0)
		{
			HandleOffCommand(pMsg, &parse);
		}
		else
		{
#ifndef _FINAL
			ShowTriggerError(pMsg);
#endif
		}
	}
}


void Controller::HandleFadeCommand(char *pMsg, ConParse *pParse)
{
	char *pParamType, *pValue, *pDuration, *pWaveType;
	ParamType paramType;
	ParamValue paramValue;
	WaveType waveType;
	float duration;
	DWORD i;


	if(pParse->m_nArgs < 4)
	{
#ifndef _FINAL
		ShowTriggerError(pMsg);
#endif
		return;
	}

	pParamType = pParse->m_Args[1];
	pValue = pParse->m_Args[2];
	pDuration = pParse->m_Args[3];

	// Parse everything.. it doesn't do anything if there's an error.
	if(stricmp(pParamType, "ALPHA") == 0)
	{
		paramType = Param_Alpha;
	}
	else if(stricmp(pParamType, "COLOR") == 0)
	{
		paramType = Param_Color;
	}
	else
	{
#ifndef _FINAL
		ShowTriggerError(pMsg);
#endif
		return;
	}

	paramValue = ParseValue(paramType, pValue);
	duration = (float)atof(pDuration);
	duration = DCLAMP(duration, 0.0f, 100000.0f);
	
	waveType = Wave_Sine;
	if(pParse->m_nArgs >= 5)
	{
		pWaveType = pParse->m_Args[4];
		waveType = ParseWaveType(pWaveType);
	}

	// Ok, configure...
	m_fStartTime = g_pServerDE->GetTime();
	m_fDuration = duration;
	m_ParamType = paramType;
	m_WaveType = waveType;
	m_DestValue = paramValue;
	
	for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		SetupCurrentValue(&m_Fades[i]);
	}

	m_State = CState_Fade;
	g_pServerDE->SetNextUpdate(m_hObject, 0.001f);
}


void Controller::HandleFlickerCommand(char *pMsg, ConParse *pParse)
{
	char *pMin, *pMax, *pMessage;

	if(pParse->m_nArgs < 4)
	{
#ifndef _FINAL
		ShowTriggerError(pMsg);
#endif
		return;
	}

	pMin = pParse->m_Args[1];
	pMax = pParse->m_Args[2];
	pMessage = pParse->m_Args[3];
#ifndef _FINAL
	if(strlen(pMsg) > MAX_FLICKERMSG_LEN)
	{
		g_pServerDE->CPrint("Controller: Warning, msg '%s' greater than %d", pMsg, MAX_FLICKERMSG_LEN);
	}
#endif

	m_fIntervalMin = (float)atof(pMin);
	m_fIntervalMax = (float)atof(pMax);
	SAFE_STRCPY(m_FlickerMsg, pMessage);
	m_FlickerCounter = FLICKER_FOREVER;

	if(pParse->m_nArgs >= 5)
	{
		m_FlickerCounter = (DWORD)atof(pParse->m_Args[4]);
	}

	m_fNextFlickerTime = g_pServerDE->GetTime() + GetRandom(m_fIntervalMin, m_fIntervalMax);
	m_State = CState_Flicker;
	g_pServerDE->SetNextUpdate(m_hObject, 0.001f);
}


void Controller::HandleOffCommand(char *pMsg, ConParse *pParse)
{
	m_State = CState_Off;
}


#ifndef _FINAL
void Controller::ShowTriggerError(char *pMsg)
{
	g_pServerDE->CPrint("Controller: Invalid msg: %s", pMsg);
}
#endif

ParamValue Controller::ParseValue(ParamType paramType, char *pValue)
{
	ParamValue ret;
	DVector color;
	char colorStr[3][256];
	DWORD i;

	if(paramType == Param_Alpha)
	{
		ret.SetAlpha((float)atof(pValue));
	}
	else
	{
		sscanf(pValue, "%s %s %s", colorStr[0], colorStr[1], colorStr[2]);

		// X's mean to not interpolate that value.	
		for(i=0; i < 3; i++)
		{
			color[i] = (float)atof(colorStr[i]);
			if(stricmp(colorStr[i], "X") == 0)
				color[i] = -1.0f;
		}

		ret.SetColor(color);
	}

	return ret;
}


void Controller::SetupCurrentValue(FadeState *pState)
{
	float r, g, b, a;

	if(!pState->m_hTarget)
		return;

	if(m_ParamType == Param_Alpha)
	{
		g_pServerDE->GetObjectColor(pState->m_hTarget, &r, &g, &b, &a);
		pState->m_StartVal.SetAlpha(a);
	}
	else
	{
		g_pServerDE->GetObjectColor(pState->m_hTarget, &r, &g, &b, &a);
		pState->m_StartVal.SetColor(DVector(r*255.0f, g*255.0f, b*255.0f));
	}
}


void Controller::InterpolateValue(FadeState *pState, float t)
{
	float newAlpha, r, g, b, a;
	DVector newColor, destColor;
	DWORD i;

	if(m_ParamType == Param_Alpha)
	{
		// Alpha.
		newAlpha = DLERP(pState->m_StartVal.GetAlpha(), m_DestValue.GetAlpha(), t);
		g_pServerDE->GetObjectColor(pState->m_hTarget, &r, &g, &b, &a);
		g_pServerDE->SetObjectColor(pState->m_hTarget, r, g, b, newAlpha);
	}
	else
	{
		// Color.
		destColor = m_DestValue.GetColor();
		for(i=0; i < 3; i++)
		{
			if(destColor[i] == -1.0f)
				newColor[i] = pState->m_StartVal.m_Color[i];
			else
				newColor[i] = DLERP(pState->m_StartVal.m_Color[i], m_DestValue.m_Color[i], t);
		}

		g_pServerDE->GetObjectColor(pState->m_hTarget, &r, &g, &b, &a);
		g_pServerDE->SetObjectColor(pState->m_hTarget, 
			newColor.x / 255.0f, 
			newColor.y / 255.0f, 
			newColor.z / 255.0f, 
			a);
	}
}


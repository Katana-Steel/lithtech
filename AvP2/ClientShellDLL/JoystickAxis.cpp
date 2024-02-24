#include "stdafx.h"
#include "JoystickAxis.h"
#include "ProfileMgr.h"
#include "clientres.h"


namespace
{
	int kGap = 200;
	int kWidth = 200;
}



void CAxisBindingData::Init()
{
	m_nAxis = JOYSTICK_AXIS_NONE;
    m_bInvertAxis = LTFALSE;
	m_nDeadZone = 2;
    m_bAnalog = LTFALSE;
	m_nSensitivity = 10;
    m_bCenterOffset = LTFALSE;
}

/**************************************************************************/
// Joystick Axis base class
/**************************************************************************/

// Constructor
CJoystickAxisBase::CJoystickAxisBase()
{
	// names of possible actions
	m_sActionDigitalLow[0] = '\0';
	m_sActionDigitalHigh[0] = '\0';
	m_sActionAnalog[0] = '\0';
	m_sDeadZoneConsoleVar[0] = '\0';

	// numbers used to make the scale from the slider
	m_fScaleCenter = 1.0f;
	m_fScaleRangeDiv2 = 0.5f;

	// set default values for rangebind and rangescale data
	m_sDeviceName[0] = '\0';
	m_sTriggerName[0] = '\0';
	m_fScale = 1.0f;
	m_fRangeScaleMin = -1.0f;
	m_fRangeScaleMax = 1.0f;
	m_fRangeScalePreCenterOffset;
	m_sActionName[0] = '\0';
	m_fRangeLow = 0.0f;
	m_fRangeHigh = 0.0f;
	m_sActionName2[0] = '\0';
	m_fRangeLow2 = 0.0f;
	m_fRangeHigh2 = 0.0f;
	m_fDeadZone = 0.10f;

	// set to LTTRUE if bindings were read in successfully
	m_bBindingsRead = LTFALSE;

    m_bAddDigitalBindingsToAnalog = LTFALSE;

}

// Send out a rangebind to the console
void CJoystickAxisBase::DoRangeBind( char *lpszDevice, char *lpszControlName, char *lpszActionName, float nRangeLow, float nRangeHigh)
{
	assert(lpszControlName);
	assert(lpszActionName);
	assert(lpszDevice);

	if (lpszControlName == NULL) return;
	if (lpszActionName == NULL) return;
	if (lpszDevice == NULL) return;

	char tempStr[512];

	// Set the binding
	sprintf(tempStr, "rangebind \"%s\" \"%s\" %f %f \"%s\"", lpszDevice, lpszControlName, nRangeLow, nRangeHigh, lpszActionName);
	g_pLTClient->RunConsoleString(tempStr);
}

// Clear a rangebind to the console
void CJoystickAxisBase::ClearRangeBind( char *lpszDevice, char *lpszControlName)
{
	assert(lpszControlName);
	assert(lpszDevice);

	if (lpszControlName == NULL) return;
	if (lpszDevice == NULL) return;

	char tempStr[512];

	// Set the binding
	sprintf(tempStr, "unbind \"%s\" \"%s\"", lpszDevice, lpszControlName);
	g_pLTClient->RunConsoleString(tempStr);
}

// Send out a rangescale to the console
void CJoystickAxisBase::DoRangeScale( char *lpszDevice, char *lpszControlName, float nScale, float nRangeScaleMin, float nRangeScaleMax, float nRangeScalePreCenterOffset)
{
	assert(lpszControlName);
	assert(lpszDevice);

	if (lpszControlName == NULL) return;
	if (lpszDevice == NULL) return;

	char tempStr[512];

	// Set the rangescale
	sprintf(tempStr, "rangescale \"%s\" \"%s\" %f %f %f %f", lpszDevice, lpszControlName, nScale, nRangeScaleMin, nRangeScaleMax, nRangeScalePreCenterOffset);
	g_pLTClient->RunConsoleString(tempStr);
}

// find the index into the axis array for the given axis name (return 0 if not found)
int CJoystickAxisBase::GetAxisIndex(char* sAxisName)
{
	// loop through all axis trying to find the matching axis (skip 0 becaus it is the none axis)
	for (int i = 1; i < g_pProfileMgr->GetNumAxis(); i++)
	{
		CDeviceAxisData *pAxis = g_pProfileMgr->GetAxisData(i);
		if (pAxis && stricmp(sAxisName, pAxis->m_sName) == 0) return i;
	}
	return 0;
}

// scales a value from one range to another in a linear fashion
float CJoystickAxisBase::ScaleToRange(float fFromVal, float fFromMin, float fFromMax, float fToMin, float fToMax)
{
	float fMultiplier = 1.0f;
	float fOffset;

	float fFromMinMinusMax = fFromMin - fFromMax;
	if (fFromMinMinusMax != 0.0f) fMultiplier = (fToMin - fToMax) / fFromMinMinusMax;
	fOffset = fToMin - (fMultiplier * fFromMin);

	float fRetVal = (fFromVal * fMultiplier) + fOffset;
	if (fRetVal < fToMin) fRetVal = fToMin;
	if (fRetVal > fToMax) fRetVal = fToMax;

	return fRetVal;
}

//set up default values
void CJoystickAxisBase::Init()
{
	// set up the default variables for this axis
	g_pLTClient->GetDeviceName (DEVICETYPE_JOYSTICK,m_sDeviceName, sizeof(m_sDeviceName));
	strncpy(m_sTriggerName, "none", INPUTNAME_LEN);
	m_sTriggerName[INPUTNAME_LEN-1] = '\0';
	m_fRangeScaleMin = -1.0f;
	m_fRangeScaleMax = 1.0f;

	m_Data.Init();

    m_bBindingsRead = LTFALSE;

}

// Load variables from the console
void CJoystickAxisBase::LoadFromConsole()
{

	if (!m_bBindingsRead)
	{
		Init();
		return;
	}

	// read in the dead zone console variable
	LTFLOAT fDeadZoneVar = GetConsoleFloat(m_sDeadZoneConsoleVar,0.1f);
	if (m_Data.m_bAnalog)
	{
		m_fDeadZone = fDeadZoneVar / m_fScale;
		//g_pLTClient->CPrint("Read Dead Zone Analog : m_fDeadZone = %f m_fScale = %f ", m_fDeadZone, m_fScale ); // BLB TEMP
	}
	else
	{
		m_fDeadZone = fDeadZoneVar;
		//g_pLTClient->CPrint("Read Dead Zone Digital : m_fDeadZone = %f", m_fDeadZone ); // BLB TEMP
	}

	// convert all of the variables read in from the bindings to our internal controls variables
	// figure out the index of the axis that we found
	m_Data.m_nAxis = GetAxisIndex(m_sTriggerName);

	// scale the sensitivity slider bar from the rangescale values that we read in
	if (m_Data.m_bAnalog) 
		m_Data.m_nSensitivity = (int)ScaleToRange(m_fScale, m_fScaleCenter-m_fScaleRangeDiv2, m_fScaleCenter+m_fScaleRangeDiv2, (float)kSensitivityLow, (float)kSensitivityHigh);

	// scale the dead zone that we read in to the dead zone slider bar
	m_Data.m_nDeadZone = (int)ScaleToRange(m_fDeadZone, 0.0f, 0.90f, (float)kDeadZoneLow, (float)kDeadZoneHigh);

	// figure out if the bindings that were read in had inverted axis
	if (m_Data.m_bAnalog)
	{
        if (m_fRangeScaleMin > m_fRangeScaleMax) 
			m_Data.m_bInvertAxis = LTTRUE;
        else 
			m_Data.m_bInvertAxis = LTFALSE;
	}
	else
	{
		if (stricmp(m_sActionName, m_sActionDigitalLow) == 0)
		{
            if (m_fRangeLow < m_fRangeLow2) 
				m_Data.m_bInvertAxis = LTFALSE;
            else 
				m_Data.m_bInvertAxis = LTTRUE;
		}
		else
		{
            if (m_fRangeLow <= m_fRangeLow2) 
				m_Data.m_bInvertAxis = LTTRUE;
            else 
				m_Data.m_bInvertAxis = LTFALSE;
		}
	}

	// figure out the correct value of the center offset flag from the data read in
    if ((long)m_fRangeScalePreCenterOffset != 100) 
		m_Data.m_bCenterOffset = LTFALSE;
    else 
		m_Data.m_bCenterOffset = LTTRUE;


}


// Save an analog binding to the consle
void CJoystickAxisBase::SaveToConsoleAnalog()
{
	// set the new scale if it has been adjusted or was never read in
	m_fScale = ScaleToRange((float)m_Data.m_nSensitivity, (float)kSensitivityLow, (float)kSensitivityHigh, m_fScaleCenter-m_fScaleRangeDiv2, m_fScaleCenter+m_fScaleRangeDiv2);
	if (m_fScale < 0.001) m_fScale = 0.001f; // don't let the scale get too small

	// adjust values for the invert axis flag if it has been adjusted or was never read in
	if (m_Data.m_bInvertAxis)
	{
		if (m_fRangeScaleMin < m_fRangeScaleMax)
		{
			float fTemp;
			fTemp = m_fRangeScaleMin;
			m_fRangeScaleMin = m_fRangeScaleMax;
			m_fRangeScaleMax = fTemp;
		}
	}
	else
	{
		if (m_fRangeScaleMin > m_fRangeScaleMax)
		{
			float fTemp;
			fTemp = m_fRangeScaleMin;
			m_fRangeScaleMin = m_fRangeScaleMax;
			m_fRangeScaleMax = fTemp;
		}
	}

	// set the center offsets if it has changed or was never read in
	if (m_Data.m_bCenterOffset) 
		m_fRangeScalePreCenterOffset = 100.0f;
	else 
		m_fRangeScalePreCenterOffset = 0.0f;

	// write out the rangebind and rangescale
	char tempStr[1024];

	if (m_bAddDigitalBindingsToAnalog)
	{
		if (m_Data.m_bInvertAxis)
		{
			sprintf(tempStr, "rangebind \"%s\" \"%s\" %f %f \"%s\" %f %f \"%s\" %f %f \"%s\"", m_sDeviceName, m_sTriggerName, 0.0f, 0.0f, m_sActionAnalog,
				(m_fDeadZone*m_fScale)-0.001f, (1.0f*m_fScale)+0.001f, m_sActionDigitalLow, -((m_fDeadZone*m_fScale)-0.001f), -((1.0f*m_fScale)+0.001f), m_sActionDigitalHigh);
		}
		else
		{
			sprintf(tempStr, "rangebind \"%s\" \"%s\" %f %f \"%s\" %f %f \"%s\" %f %f \"%s\"", m_sDeviceName, m_sTriggerName, 0.0f, 0.0f, m_sActionAnalog,
				(m_fDeadZone*m_fScale)-0.001f, (1.0f*m_fScale)+0.001f, m_sActionDigitalHigh, -((m_fDeadZone*m_fScale)-0.001f), -((1.0f*m_fScale)+0.001f), m_sActionDigitalLow);
		}
	}
	else
	{
		sprintf(tempStr, "rangebind \"%s\" \"%s\" %f %f \"%s\"", m_sDeviceName, m_sTriggerName, 0.0f, 0.0f, m_sActionAnalog);
	}

	g_pLTClient->RunConsoleString(tempStr);

	DoRangeScale(m_sDeviceName, m_sTriggerName, m_fScale, m_fRangeScaleMin, m_fRangeScaleMax, m_fRangeScalePreCenterOffset);
	//g_pLTClient->CPrint("Joystick Analog Binding : %s m_fScale=%f m_fRangeScaleMin=%f m_fRangeScaleMax=%f m_fRangeScalePreCenterOffset=%f ", tempStr, m_fScale, m_fRangeScaleMin, m_fRangeScaleMax, m_fRangeScalePreCenterOffset ); // BLB TEMP
}


// Save a digital binding to the consle
void CJoystickAxisBase::SaveToConsoleDigital()
{
	CDeviceAxisData *pAxis = g_pProfileMgr->GetAxisData(m_Data.m_nAxis);
	if (!pAxis) return;


	// figure out the intermediate range information
	float fRange = pAxis->m_fRangeHigh - pAxis->m_fRangeLow;
	float fHalfRange = fRange / 2.0f;
	float fActiveRange = fHalfRange - (m_fDeadZone * fRange);

	// figure out the numbers to write out
	float fLeftLow = pAxis->m_fRangeLow;
	float fLeftHigh = pAxis->m_fRangeLow + fActiveRange;
	float fRightLow = pAxis->m_fRangeHigh - fActiveRange;
	float fRightHigh = pAxis->m_fRangeHigh;

	// clear the bindings
	char str[512];
	sprintf (str, "unbind \"%s\" \"%s\"", m_sDeviceName, m_sTriggerName);
	g_pLTClient->RunConsoleString (str);

	// write out the new bindings
	if (m_Data.m_bInvertAxis)
	{
		sprintf (str, "rangebind \"%s\" \"%s\" \"%f\" \"%f\" \"%s\" \"%f\" \"%f\" \"%s\"", m_sDeviceName, m_sTriggerName,
				 fRightLow, fRightHigh, m_sActionDigitalLow, fLeftLow, fLeftHigh, m_sActionDigitalHigh);
	}
	else
	{
		sprintf (str, "rangebind \"%s\" \"%s\" \"%f\" \"%f\" \"%s\" \"%f\" \"%f\" \"%s\"", m_sDeviceName, m_sTriggerName,
				 fLeftLow, fLeftHigh, m_sActionDigitalLow, fRightLow, fRightHigh, m_sActionDigitalHigh);
	}

	g_pLTClient->RunConsoleString (str);
	//g_pLTClient->CPrint("Joystick Digital Binding : %s", str ); // BLB TEMP
}


// Save variables to the console
void CJoystickAxisBase::SaveToConsole()
{
	CDeviceAxisData *pAxis = g_pProfileMgr->GetAxisData(m_Data.m_nAxis);
	if (!pAxis || m_Data.m_nAxis == JOYSTICK_AXIS_NONE) return;

	// figure out the new trigger name
	strncpy(m_sTriggerName, pAxis->m_sName, INPUTNAME_LEN);
	m_sTriggerName[INPUTNAME_LEN-1] = '\0';

	// set the new dead zone if it has been adjusted or was never read in
	m_fDeadZone = ScaleToRange((float)m_Data.m_nDeadZone, (float)kDeadZoneLow, (float)kDeadZoneHigh, 0.0f, 0.90f);
	if (m_Data.m_nDeadZone == kDeadZoneLow) m_fDeadZone = 0.0f;

	// do the analog or digital binding
	if (m_Data.m_nAxis > JOYSTICK_AXIS_NONE)
	{
		// clear any previous bindings to this axis
		ClearRangeBind(m_sDeviceName, m_sTriggerName);

		// save out analog options
		if (m_Data.m_bAnalog) 
			SaveToConsoleAnalog();
		// save out digital options
		else 
			SaveToConsoleDigital();
	}

	// write out the dead zone console variable
	if ((m_sDeviceName[0] != '\0') && (m_sTriggerName[0] != '\0') && (m_sActionName[0] != '\0'))
	{
		float fDeadZone = m_fDeadZone;
		if (m_Data.m_bAnalog) 
			fDeadZone = m_fDeadZone * m_fScale;
		if (fDeadZone < 0.0f) 
			fDeadZone = 0.0f;
		WriteConsoleFloat(m_sDeadZoneConsoleVar, fDeadZone);
		//g_pLTClient->CPrint("Saving Dead Zone : fDeadZone = %f m_fDeadZone = %f", fDeadZone, m_fDeadZone ); // BLB TEMP
	}
}


void CJoystickAxisBase::ReadFromProfile(CButeMgr* pButeMgr)
{
	CAxisBindingData data;
	char s_aTagName[30];

	strcpy(s_aTagName,m_sActionAnalog);

	CString str = pButeMgr->GetString(s_aTagName,"Axis");

	data.m_nAxis = GetAxisIndex(str.GetBuffer());
    data.m_bInvertAxis = (LTBOOL)pButeMgr->GetInt(s_aTagName,"InvertAxis",LTFALSE);
	data.m_nDeadZone = pButeMgr->GetInt(s_aTagName,"DeadZone",2);
    data.m_bAnalog = (LTBOOL)pButeMgr->GetInt(s_aTagName,"Analog",LTFALSE);
	data.m_nSensitivity = pButeMgr->GetInt(s_aTagName,"Sensitivity",10);
    data.m_bCenterOffset = (LTBOOL)pButeMgr->GetInt(s_aTagName,"CenterOffset",LTFALSE);

	SetBindingData(&data);

}

void CJoystickAxisBase::WriteToProfile(CButeMgr* pButeMgr)
{
	char s_aTagName[30];

	strcpy(s_aTagName,m_sActionAnalog);

	CDeviceAxisData *pAxis = g_pProfileMgr->GetAxisData(m_Data.m_nAxis);
	if (pAxis)
	{
		pButeMgr->SetString(s_aTagName,"Axis",pAxis->m_sName);
	}
	else
	{
		pButeMgr->SetString(s_aTagName,"Axis","none");
	}

	pButeMgr->SetInt(s_aTagName,"InvertAxis",m_Data.m_bInvertAxis);
	pButeMgr->SetInt(s_aTagName,"DeadZone",m_Data.m_nDeadZone);
	pButeMgr->SetInt(s_aTagName,"Analog",m_Data.m_bAnalog);
	pButeMgr->SetInt(s_aTagName,"Sensitivity",m_Data.m_nSensitivity);
	pButeMgr->SetInt(s_aTagName,"CenterOffset",m_Data.m_bCenterOffset);

}


// Check to see if a binding relates to us
LTBOOL CJoystickAxisBase::CheckBinding(DeviceBinding *pBinding)
{
//	g_pLTClient->CPrint("Checking device = %s trigger = %s", pCurrentBinding->strDeviceName, pCurrentBinding->strTriggerName ); // BLB TEMP

    LTBOOL       bFound = LTFALSE;
	GameAction* pDigitalLowAction = NULL;
	GameAction* pDigitalHighAction = NULL;
	GameAction* pAnalogAction = NULL;
	GameAction* pAction = pBinding->pActionHead;

	// go through all actions looking for our actions
	while (pAction != NULL)
	{
		char *sActionName=pAction->strActionName;
		if (sActionName != NULL)
		{
//			g_pLTClient->CPrint("Checking action = %s", sActionName ); // BLB TEMP

			if (stricmp(sActionName, m_sActionAnalog) == 0) pAnalogAction = pAction;
			if (stricmp(sActionName, m_sActionDigitalHigh) == 0) pDigitalHighAction = pAction;
			if (stricmp(sActionName, m_sActionDigitalLow) == 0) pDigitalLowAction = pAction;
		}
		pAction = pAction->pNext;
		if (pAction == pBinding->pActionHead) break;
	}

	// if we found an analog action set up the variables for it
	if (pAnalogAction != NULL)
	{
		strncpy(m_sActionName, pAnalogAction->strActionName, MAX_ACTIONNAME_LEN);
		m_sActionName[MAX_ACTIONNAME_LEN-1] = '\0';
		m_fRangeLow = pAnalogAction->nRangeLow;
		m_fRangeHigh = pAnalogAction->nRangeHigh;

		m_Data.m_bAnalog = LTTRUE;
        bFound = LTTRUE;

//		g_pLTClient->CPrint("Digital binding read Device = %s Trigger = %s Action1 = %s", m_sDeviceName, pCurrentBinding->strTriggerName, m_sActionName ); // BLB TEMP
	}

	// otherwise if we found the two digital actions with no analog we are in digital mode
	else if ((pDigitalLowAction != NULL) && (pDigitalHighAction != NULL))
	{
		strncpy(m_sActionName, pDigitalLowAction->strActionName, MAX_ACTIONNAME_LEN);
		m_sActionName[MAX_ACTIONNAME_LEN-1] = '\0';
		m_fRangeLow = pDigitalLowAction->nRangeLow;
		m_fRangeHigh = pDigitalLowAction->nRangeHigh;

		strncpy(m_sActionName2, pDigitalHighAction->strActionName, MAX_ACTIONNAME_LEN);
		m_sActionName2[MAX_ACTIONNAME_LEN-1] = '\0';
		m_fRangeLow2 = pDigitalHighAction->nRangeLow;
		m_fRangeHigh2 = pDigitalHighAction->nRangeHigh;

		m_Data.m_bAnalog = LTFALSE;
        bFound = LTTRUE;

//		g_pLTClient->CPrint("Digital binding read Device = %s Trigger = %s Action1 = %s Action2 = %s", m_sDeviceName, pCurrentBinding->strTriggerName, m_sActionName, m_sActionName2 ); // BLB TEMP
	}

	else
	{
//		g_pLTClient->CPrint("No binding found!"); // BLB TEMP
	}

	// if we did find some bindings transfer the information
	if (bFound)
	{
		strncpy(m_sTriggerName, pBinding->strTriggerName, INPUTNAME_LEN);
		m_sTriggerName[INPUTNAME_LEN-1] = '\0';
		m_fScale = pBinding->nScale;
		m_fRangeScaleMin = pBinding->nRangeScaleMin;
		m_fRangeScaleMax = pBinding->nRangeScaleMax;
		m_fRangeScalePreCenterOffset = pBinding->nRangeScalePreCenterOffset;
        m_bBindingsRead = LTTRUE;
	}
	return bFound;
}


void CJoystickAxisBase::SetBindingData(CAxisBindingData* pData)
{
	CDeviceAxisData *pAxis = g_pProfileMgr->GetAxisData(pData->m_nAxis);
	if (!pAxis || pData->m_nAxis == JOYSTICK_AXIS_NONE)
	{
		Init();
		return;
	}
	strncpy(m_sTriggerName, pAxis->m_sName, INPUTNAME_LEN);
	m_sTriggerName[INPUTNAME_LEN-1] = '\0';

	m_Data = *pData;

	// set the new scale if it has been adjusted or was never read in
	m_fScale = ScaleToRange((float)m_Data.m_nSensitivity, (float)kSensitivityLow, (float)kSensitivityHigh, m_fScaleCenter-m_fScaleRangeDiv2, m_fScaleCenter+m_fScaleRangeDiv2);
	if (m_fScale < 0.001) m_fScale = 0.001f; // don't let the scale get too small

	m_fRangeScaleMin = -1.0f;
	m_fRangeScaleMax = 1.0f;

	if (m_Data.m_bCenterOffset) 
		m_fRangeScalePreCenterOffset = 100.0f;
	else 
		m_fRangeScalePreCenterOffset = 0.0f;

}


/**************************************************************************/
// Joystick Axis turn class
/**************************************************************************/
CJoystickAxisTurn::CJoystickAxisTurn()
{
    m_bAddDigitalBindingsToAnalog = LTFALSE;

	// set up range of sensitivity scaling
	m_fScaleCenter = 0.51f;
	m_fScaleRangeDiv2 = 0.5f;

	// set the string names for this axis
	strncpy(m_sActionDigitalLow, "Left", MAX_ACTIONNAME_LEN);
	m_sActionDigitalLow[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionDigitalHigh, "Right", MAX_ACTIONNAME_LEN);
	m_sActionDigitalHigh[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionAnalog, "AxisYaw", MAX_ACTIONNAME_LEN);
	m_sActionAnalog[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sDeadZoneConsoleVar, "AxisYawDeadZone", MAX_ACTIONNAME_LEN+16);
	m_sDeadZoneConsoleVar[MAX_ACTIONNAME_LEN+16-1] = '\0';


}


/**************************************************************************/
// Joystick Axis look class
/**************************************************************************/

// Constructor
CJoystickAxisLook::CJoystickAxisLook()
{
    m_bAddDigitalBindingsToAnalog = LTFALSE;

    m_bFixedPosition=LTTRUE;
	m_fScaleCenter = 1.0f;
	m_fScaleRangeDiv2 = 0.5f;

	// set the string names for this axis
	strncpy(m_sActionDigitalLow, "LookUp", MAX_ACTIONNAME_LEN);
	m_sActionDigitalLow[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionDigitalHigh, "LookDown", MAX_ACTIONNAME_LEN);
	m_sActionDigitalHigh[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionAnalog, "AxisPitch", MAX_ACTIONNAME_LEN);
	m_sActionAnalog[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sDeadZoneConsoleVar, "AxisPitchDeadZone", MAX_ACTIONNAME_LEN+16);
	m_sDeadZoneConsoleVar[MAX_ACTIONNAME_LEN+16-1] = '\0';

}

void CJoystickAxisLook::Init()
{
	CJoystickAxisBase::Init();

	// Members
    m_bFixedPosition=LTTRUE;
	m_fScaleCenter = 1.0f;
	m_fScaleRangeDiv2 = 0.5f;

}

// Load from the console
void CJoystickAxisLook::LoadFromConsole()
{
	// call the base code to load everything in and set up the variables
	CJoystickAxisBase::LoadFromConsole();

	// read in and set up the fixed position variable
	m_bFixedPosition = (LTBOOL)GetConsoleInt("FixedAxisPitch",0);

	// set up range of sensitivity scaling
	if (m_bFixedPosition) 
		m_fScaleCenter = 1.0f;
	else 
		m_fScaleCenter = 0.51f;

	m_fScaleRangeDiv2 = 0.5f;

}

// Save to the console
void CJoystickAxisLook::SaveToConsole()
{
	// figure out correct rangescale values based on fixedposition
	// write out the fixed axis pitch console variable
	if ((m_bFixedPosition) && (m_Data.m_bAnalog) && (m_Data.m_nAxis > JOYSTICK_AXIS_NONE)) 
	{
		WriteConsoleInt( "FixedAxisPitch", 1);
		m_fRangeScaleMin = -(3.14159265f/2.0f);
		m_fRangeScaleMax = (3.14159265f/2.0f);
		m_fScaleCenter = 1.0f;
	}
	else
	{
		WriteConsoleInt( "FixedAxisPitch", 0);
		m_fRangeScaleMin = -1.0;
		m_fRangeScaleMax = 1.0;
		m_fScaleCenter = 0.51f;
	}

	// call base class to save bindings
	CJoystickAxisBase::SaveToConsole();

}

void CJoystickAxisLook::SetBindingData(CAxisBindingData* pData)
{
	CJoystickAxisBase::SetBindingData(pData);

	// figure out correct rangescale values based on fixedposition
	if (m_bFixedPosition)
	{
		m_fRangeScaleMin = -(3.14159265f/2.0f);
		m_fRangeScaleMax = (3.14159265f/2.0f);
		m_fScaleCenter = 1.0f;
	}
	else
	{
		m_fRangeScaleMin = -1.0;
		m_fRangeScaleMax = 1.0;
		m_fScaleCenter = 0.51f;
	}

}

void CJoystickAxisLook::ReadFromProfile(CButeMgr* pButeMgr)
{
	char s_aTagName[30];

	strcpy(s_aTagName,m_sActionAnalog);

    m_bFixedPosition = (LTBOOL)pButeMgr->GetInt(s_aTagName,"FixedPosition",LTFALSE);

	CJoystickAxisBase::ReadFromProfile(pButeMgr);

}

void CJoystickAxisLook::WriteToProfile(CButeMgr* pButeMgr)
{
	char s_aTagName[30];

	strcpy(s_aTagName,m_sActionAnalog);

	pButeMgr->SetInt(s_aTagName,"FixedPosition",m_bFixedPosition);
	CJoystickAxisBase::WriteToProfile(pButeMgr);

}

/**************************************************************************/
// Joystick Axis move class
/**************************************************************************/

// Contructor
CJoystickAxisMove::CJoystickAxisMove()
{
    m_bAddDigitalBindingsToAnalog = LTTRUE;

	// set up range of sensitivity scaling
	m_fScaleCenter = 1.0f;
	m_fScaleRangeDiv2 = 0.5f;

	// set the string names for this axis
	strncpy(m_sActionDigitalLow, "Forward", MAX_ACTIONNAME_LEN);
	m_sActionDigitalLow[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionDigitalHigh, "Backward", MAX_ACTIONNAME_LEN);
	m_sActionDigitalHigh[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionAnalog, "AxisForwardBackward", MAX_ACTIONNAME_LEN);
	m_sActionAnalog[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sDeadZoneConsoleVar, "AxisForwardBackwardDeadZone", MAX_ACTIONNAME_LEN+16);
	m_sDeadZoneConsoleVar[MAX_ACTIONNAME_LEN+16-1] = '\0';

}

/**************************************************************************/
// Joystick Axis stafe class
/**************************************************************************/
CJoystickAxisStrafe::CJoystickAxisStrafe()
{
    m_bAddDigitalBindingsToAnalog = LTTRUE;
	m_fScaleCenter = 1.0f;
	m_fScaleRangeDiv2 = 0.5f;

	// set the string names for this axis
	strncpy(m_sActionDigitalLow, "StrafeLeft", MAX_ACTIONNAME_LEN);
	m_sActionDigitalLow[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionDigitalHigh, "StrafeRight", MAX_ACTIONNAME_LEN);
	m_sActionDigitalHigh[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionAnalog, "AxisLeftRight", MAX_ACTIONNAME_LEN);
	m_sActionAnalog[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sDeadZoneConsoleVar, "AxisLeftRightDeadZone", MAX_ACTIONNAME_LEN+16);
	m_sDeadZoneConsoleVar[MAX_ACTIONNAME_LEN+16-1] = '\0';

}


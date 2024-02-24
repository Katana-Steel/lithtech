// JoystickAxis.h: interface for the  joystick axis classes
//
//////////////////////////////////////////////////////////////////////

#ifndef USE_JOYSTICKAXIS
#define USE_JOYSTICKAXIS


#define JOYSTICK_AXIS_NONE	0

// Slider ranges
const int kDeadZoneLow = 0;
const int kDeadZoneHigh = 18;
const int kSensitivityLow = 0;
const int kSensitivityHigh = 20;

class CDeviceAxisData;
class CButeMgr;


class CAxisBindingData
{
public:
	CAxisBindingData() { Init(); }
	void Init();

	int					m_nAxis;
    LTBOOL              m_bInvertAxis;
	int					m_nDeadZone;
    LTBOOL              m_bAnalog;
	int					m_nSensitivity;
    LTBOOL              m_bCenterOffset;
};

/**************************************************************************/
// Joystick Axis base class
/**************************************************************************/

class CJoystickAxisBase
{
public:
	// Constructor
	CJoystickAxisBase();

	//set up default values
    virtual void        Init();

	// Load/Save to and from the console
    virtual void        LoadFromConsole();
    virtual void        SaveToConsole();

	// Load/Save to and from profiles
    virtual void        ReadFromProfile(CButeMgr* pButeMgr);
    virtual void        WriteToProfile(CButeMgr* pButeMgr);
	

	// Check to see if a binding relates to us
	LTBOOL CheckBinding(DeviceBinding *pBinding);
	LTBOOL IsBound() {return m_bBindingsRead;}

	const CAxisBindingData* GetBindingData() {return &m_Data;}
	virtual void SetBindingData(CAxisBindingData* pData);


protected:

	// Send out a rangebind to the console
    void    DoRangeBind(char *lpszDevice, char *lpszControlName, char *lpszActionName, float nRangeLow, float nRangeHigh);

	// Clear a rangebind to the console
    void    ClearRangeBind(char *lpszDevice, char *lpszControlName);

	// Send out a rangescale to the console
    void    DoRangeScale(char *lpszDevice, char *lpszControlName, float nScale, float nRangeScaleMin, float nRangeScaleMax, float nRangeScalePreCenterOffset);

	// find the index into the axis array for the given axis name (return 0 if not found)
    int     GetAxisIndex(char* sAxisName);

	// scales a value from one range to another in a linear fashion
	float	ScaleToRange(float fFromVal, float fFromMin, float fFromMax, float fToMin, float fToMax);

	// Save an analog binding to the consle
    void SaveToConsoleAnalog();

	// Save a digital binding to the consle
    void SaveToConsoleDigital();

protected:
	// names of possible actions
	char	m_sActionDigitalLow[MAX_ACTIONNAME_LEN];
	char	m_sActionDigitalHigh[MAX_ACTIONNAME_LEN];
	char	m_sActionAnalog[MAX_ACTIONNAME_LEN];
	char	m_sDeadZoneConsoleVar[MAX_ACTIONNAME_LEN+16];

	// numbers used to make the scale from the slider
	float	m_fScaleCenter;
	float	m_fScaleRangeDiv2;

	// rangebind and rangescale data
	char	m_sDeviceName[INPUTNAME_LEN];
	char	m_sTriggerName[INPUTNAME_LEN];
	float	m_fScale;
	float	m_fRangeScaleMin;
	float	m_fRangeScaleMax;
	float	m_fRangeScalePreCenterOffset;
	char	m_sActionName[MAX_ACTIONNAME_LEN];
	float	m_fRangeLow;
	float	m_fRangeHigh;
	char	m_sActionName2[MAX_ACTIONNAME_LEN];
	float	m_fRangeLow2;
	float	m_fRangeHigh2;
	float	m_fDeadZone;

	// set to TRUE if bindings were read in successfully
    LTBOOL               m_bBindingsRead;

    LTBOOL               m_bAddDigitalBindingsToAnalog;

	// Data members
	CAxisBindingData	m_Data;


};

/**************************************************************************/
// Joystick Axis look class
/**************************************************************************/

class CJoystickAxisLook : public CJoystickAxisBase
{
public:
	// Constructor
	CJoystickAxisLook();

	//set up default values
    virtual void        Init();

	// Load/Save to and from the console
    virtual void        LoadFromConsole();
    virtual void        SaveToConsole();

	// Load/Save to and from profiles
    virtual void        ReadFromProfile(CButeMgr* pButeMgr);
    virtual void        WriteToProfile(CButeMgr* pButeMgr);

	LTBOOL GetFixedPosition() { return m_bFixedPosition; }
	void   SetFixedPosition(LTBOOL bFixed) { m_bFixedPosition = bFixed; }

	//call after SetFixedPosition();
	virtual void SetBindingData(CAxisBindingData* pData);


protected:
    LTBOOL m_bFixedPosition;

};

/**************************************************************************/
// Joystick Axis turn class
/**************************************************************************/

class CJoystickAxisTurn : public CJoystickAxisBase
{
public:
	// Constructor
	CJoystickAxisTurn();

};

/**************************************************************************/
// Joystick Axis move class
/**************************************************************************/

class CJoystickAxisMove : public CJoystickAxisBase
{
public:
	// Constructor
	CJoystickAxisMove();

};

/**************************************************************************/
// Joystick Axis stafe class
/**************************************************************************/

class CJoystickAxisStrafe : public CJoystickAxisMove
{
public:
	// Constructor
	CJoystickAxisStrafe();

};

#endif
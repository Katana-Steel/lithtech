// FolderJoystick.h: interface for the CFolderJoystick class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_JOYSTICK_H_
#define _FOLDER_JOYSTICK_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"
#include "JoystickAxis.h"

class CFolderJoystick : public CBaseFolder
{
public:
	CFolderJoystick();
	virtual ~CFolderJoystick();

	// Build the folder
    LTBOOL   Build();

	// Change in focus
    void    OnFocus(LTBOOL bFocus);

	void	Escape();
	LTBOOL	OnLeft();
	LTBOOL	OnRight();
	LTBOOL	OnLButtonUp(int x, int y);
	LTBOOL	OnRButtonUp(int x, int y);

	virtual char*	GetSelectSound();

protected:
	void	SetCurrentAxis(uint32 dwAxisCmd);

    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

protected:
    LTBOOL	m_bUseJoystick;     // TRUE if the joystick should be used
	uint32	m_dwCurrentAxis;

	CAxisBindingData	m_AxisData[4];

	CCycleCtrl*			m_pAxisCycle;
	CToggleCtrl*		m_pInvertToggle;
	CSliderCtrl*		m_pDeadZoneSlider;
	CToggleCtrl*		m_pAnalogToggle;
	CSliderCtrl*		m_pSensitivitySlider;
	CToggleCtrl*		m_pCenterToggle;
	CToggleCtrl*		m_pFixedToggle;
	
	LTBOOL				m_bFixedPositionLook;

	char			m_szTabSound[64];

};

#endif // _FOLDER_JOYSTICK_H_
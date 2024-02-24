// FolderPerformance.h: interface for the CFolderPerformance class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_PERFORM_H_
#define _FOLDER_PERFORM_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderPerformance : public CBaseFolder
{
public:
	CFolderPerformance();
	virtual ~CFolderPerformance();

	// Build the folder
    LTBOOL   Build();
	LTBOOL	OnLeft();
	LTBOOL	OnRight();
	LTBOOL	OnLButtonUp(int x, int y);
	LTBOOL	OnRButtonUp(int x, int y);

	void	OnFocus(LTBOOL bFocus);

protected:

	void	BuildControlList();
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void	UpdateSliders();

	void	GetPerformanceValues();
	void	SetPerformanceValues();


	LTBOOL	m_bLightMaps;
	LTBOOL	m_b32BitLightMaps;
	LTBOOL	m_b32BitTextures;
	LTBOOL	m_bMirrors;
	LTBOOL	m_bShadows;
	LTBOOL	m_bDetTextures;
	LTBOOL  m_bEnvironment;
	LTBOOL  m_bChrome;
	LTBOOL  m_bTrilinear;
	LTBOOL  m_bTriple;
	LTBOOL	m_bDrawSky;
	LTBOOL  m_bCasings;
	LTBOOL	m_bViewModels;
	LTBOOL  m_bMuzzle;
	int		m_nImpact;
	int		m_nDebris;

	int		m_nTextures[10];
	int		m_nDetail;

    LTBOOL	m_bSoundQuality;
	int		m_nSoundQuantity;

	CLTGUITextItemCtrl *m_pGeneral;
	CLTGUITextItemCtrl *m_pSFX;
	CLTGUITextItemCtrl *m_pTex;
	CLTGUITextItemCtrl *m_pSound;

	CCycleCtrl *m_pOverall;

	CCycleCtrl*						m_pPerformance;
	int								m_nPerform;

	int		m_nFogDistance;
};

#endif // _FOLDER_PERFORM_H_
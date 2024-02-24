// FolderAudio.cpp: implementation of the CFolderAudio class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderAudio.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
#include "GameSettings.h"

#define VOLUME_MIN					0
#define VOLUME_MAX					100
#define VOLUME_SLIDER_INC			10
#define	SOUND_VOLUME_DEFAULT		90
#define	MUSIC_VOLUME_DEFAULT		75

namespace
{
	int kGap = 0;
	int kWidth = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderAudio::CFolderAudio()
{
    m_bSoundOn = LTFALSE;
	m_bOldSoundOn = LTFALSE;

    m_bMusicOn = LTFALSE;
	m_bOldMusicOn = LTFALSE;

    m_bSoundQuality = LTFALSE;
    m_bOldSoundQuality = LTFALSE;

	m_nSoundQuantity = 32;
	m_nOldSoundQuantity = 32;

	m_nSoundVolume = SOUND_VOLUME_DEFAULT;
	m_nMusicVolume = MUSIC_VOLUME_DEFAULT;

    m_pSoundVolumeCtrl = LTNULL;
    m_pMusicVolumeCtrl = LTNULL;
    m_pSoundQualityCtrl = LTNULL;
	m_pSoundQuantityCtrl = LTNULL;
}

CFolderAudio::~CFolderAudio()
{

}

// Build the folder
LTBOOL CFolderAudio::Build()
{

	CreateTitle(IDS_TITLE_AUDIO);

	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_AUDIO,"ColumnWidth"))
		kGap = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_AUDIO,"ColumnWidth");
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_AUDIO,"SliderWidth"))
		kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_AUDIO,"SliderWidth");


	m_pSoundVolumeCtrl=AddSlider(IDS_SOUND_FXVOL, IDS_HELP_SOUNDVOL, kGap, kWidth, &m_nSoundVolume);

	m_pSoundQuantityCtrl=AddSlider(IDS_SOUND_QUANTITY, IDS_HELP_SOUND_QUANTITY, kGap, kWidth, &m_nSoundQuantity);
	m_pSoundQuantityCtrl->SetNumericDisplay(LTTRUE, LTTRUE);
	m_pSoundQuantityCtrl->SetSliderRange(16, 64);
	m_pSoundQuantityCtrl->SetSliderIncrement(16);

	m_pSoundQualityCtrl=AddToggle(IDS_SOUND_QUALITY, IDS_HELP_SOUNDQUAL, kGap, &m_bSoundQuality);
	m_pSoundQualityCtrl->SetOnString(IDS_SOUND_HIGH);
	m_pSoundQualityCtrl->SetOffString(IDS_SOUND_LOW);


	m_pMusicVolumeCtrl = AddSlider(IDS_SOUND_MUSICVOL, IDS_HELP_MUSICVOL, kGap, kWidth, &m_nMusicVolume);

	if ( m_pSoundVolumeCtrl )
	{
		m_pSoundVolumeCtrl->SetSliderRange(VOLUME_MIN, VOLUME_MAX);
		m_pSoundVolumeCtrl->SetSliderIncrement(VOLUME_SLIDER_INC);
	}
	if ( m_pMusicVolumeCtrl )
	{
		m_pMusicVolumeCtrl->SetSliderRange(VOLUME_MIN, VOLUME_MAX);
		m_pMusicVolumeCtrl->SetSliderIncrement(VOLUME_SLIDER_INC);
	}

	// Load the sound settings
	LoadSoundSettings();

	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	return LTTRUE;
}

uint32 CFolderAudio::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
};

// Enable/Disable the volume controls based on m_bSoundOn and m_bMusicOn
void CFolderAudio::EnableDisableControls()
{
	if ( !m_bSoundOn )
	{
		if ( m_pSoundVolumeCtrl )
		{
            m_pSoundVolumeCtrl->Enable(LTFALSE);
            m_pSoundQualityCtrl->Enable(LTFALSE);
			m_pSoundQuantityCtrl->Enable(LTFALSE);
		}
	}
	else
	{
		if ( m_pSoundVolumeCtrl )
		{
            m_pSoundVolumeCtrl->Enable(LTTRUE);
            m_pSoundQualityCtrl->Enable(LTTRUE);
			m_pSoundQuantityCtrl->Enable(LTTRUE);
		}
	}

	if ( !m_bMusicOn )
	{
		if ( m_pMusicVolumeCtrl )
		{
            m_pMusicVolumeCtrl->Enable(LTFALSE);
		}
	}
	else
	{
		if ( m_pMusicVolumeCtrl )
		{
            m_pMusicVolumeCtrl->Enable(LTTRUE);
		}
	}
}

void CFolderAudio::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		LoadSoundSettings();
	}
	else
	{
		SaveSoundSettings();
	}

	CBaseFolder::OnFocus(bFocus);
}


// Load the sound settings
void CFolderAudio::LoadSoundSettings()
{
	CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();
	m_nSoundVolume = (int)pSettings->SFXVolume();
	m_nMusicVolume = (int)pSettings->MusicVolume();

	m_bOldSoundOn = m_bSoundOn = (LTBOOL)pSettings->GetFloatVar("soundenable");
	m_bOldMusicOn = m_bMusicOn = (m_nMusicVolume != VOLUME_MIN);
	m_bOldSoundQuality = m_bSoundQuality = pSettings->GetBoolVar("sound16bit");
	m_nOldSoundQuantity = m_nSoundQuantity = (int)pSettings->GetFloatVar("maxswvoices");

    UpdateData(LTFALSE);
}

// Save the sound settings
void CFolderAudio::SaveSoundSettings()
{
    UpdateData(LTTRUE);
	CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();

	// m_bSoundOn = (m_nSoundVolume != SOUND_MIN_VOL);
	// pSettings->SetBoolVar("soundenable", m_bSoundOn);
	// Turning off the sound is more efficient however, lip-syncing
	// and subtitles won't work...so, we'll leave it on and just
	// live with it...
	m_bSoundOn = (LTBOOL)pSettings->GetFloatVar("soundenable");
	
	pSettings->SetFloatVar("sfxvolume",(float)m_nSoundVolume);
	pSettings->ImplementSoundVolume();

	if (m_bOldSoundOn != m_bSoundOn)
	{
		m_bOldSoundOn = m_bSoundOn;
	}

	m_bMusicOn = (m_nMusicVolume != VOLUME_MIN);

	pSettings->SetBoolVar("MusicEnable",m_bMusicOn);
	if (m_bOldMusicOn != m_bMusicOn)
	{
		pSettings->ImplementMusicSource();
		m_bOldMusicOn = m_bMusicOn;
	}

	pSettings->SetFloatVar("musicvolume",(float)m_nMusicVolume);
	pSettings->ImplementMusicVolume();


	pSettings->SetBoolVar("sound16bit",m_bSoundQuality);
	pSettings->SetFloatVar("maxswvoices",(float)m_nSoundQuantity);

	if(	(m_bSoundQuality != m_bOldSoundQuality) ||
		(m_nSoundQuantity != m_nOldSoundQuantity))
	{
		g_pGameClientShell->InitSound();

		m_bOldSoundQuality = m_bSoundQuality;
		m_nOldSoundQuantity = m_nSoundQuantity;
	}


	// Enable/Disable the volume controls based on m_bSoundOn and m_bMusicOn
	//EnableDisableControls();
	g_pGameClientShell->SetSFXandMusicVolume((LTFLOAT)m_nSoundVolume, (LTFLOAT)m_nMusicVolume);
}



// Override the left and right controls so that the volumes can be changed
LTBOOL CFolderAudio::OnLeft()
{
    LTBOOL handled = CBaseFolder::OnLeft();

	if (handled)
		SaveSoundSettings();
	return handled;
}

LTBOOL CFolderAudio::OnRight()
{
    LTBOOL handled = CBaseFolder::OnRight();

	if (handled)
		SaveSoundSettings();
	return handled;
}


LTBOOL CFolderAudio::OnEnter()
{
    LTBOOL handled = CBaseFolder::OnEnter();

	if (handled)
		SaveSoundSettings();
	return handled;
}

LTBOOL CFolderAudio::OnLButtonUp(int x, int y)
{
    LTBOOL handled = CBaseFolder::OnLButtonUp(x, y);

	if (handled)
		SaveSoundSettings();
	return handled;
}

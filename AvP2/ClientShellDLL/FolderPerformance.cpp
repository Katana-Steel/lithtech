// FolderPerformance.cpp: implementation of the CFolderPerformance class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderPerformance.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
#include "GameSettings.h"
#include "PerformanceMgr.h"


namespace
{
	LTIntPt ptAdditionalPerformance;
	int kAdditionalWidth = 0;

	int kGeneralWidth = 0;
	int kTextureWidth = 0;
	int kGap = 0;
	int kSlider = 0;

	enum eLocalCommands
	{
		CMD_GENERAL = FOLDER_CMD_CUSTOM,
		CMD_SFX,
		CMD_TEX,
		CMD_SOUND,
	};

	int kNumCfg;

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderPerformance::CFolderPerformance()
{
	m_pGeneral = LTNULL;
	m_pSFX = LTNULL;
	m_pTex = LTNULL;
	m_pSound = LTNULL;
	m_pOverall = LTNULL;
}

CFolderPerformance::~CFolderPerformance()
{

}

// Build the folder
LTBOOL CFolderPerformance::Build()
{
	kGeneralWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_PERFORMANCE,"GeneralWidth");
	kTextureWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_PERFORMANCE,"TextureWidth");
	kGap = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_PERFORMANCE,"ColumnWidth");
	kSlider = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_PERFORMANCE,"SliderWidth");

	ptAdditionalPerformance = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_PERFORMANCE,"AdditionalPos");
	kAdditionalWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_PERFORMANCE,"AdditionalWidth");


	CreateTitle(IDS_TITLE_PERFORM);

	m_pGeneral = AddTab(IDS_PERFORM_GENERAL, CMD_GENERAL, IDS_HELP_PERFORM_GENERAL);
	m_pSFX = AddTab(IDS_PERFORM_SFX, CMD_SFX, IDS_HELP_PERFORM_SFX);
	m_pTex = AddTab(IDS_PERFORM_TEX, CMD_TEX, IDS_HELP_PERFORM_TEX);
//	m_pSound = AddTab(IDS_PERFORM_SOUND, CMD_SOUND, IDS_HELP_PERFORM_SOUND);

	m_pPerformance = CreateCycleItem(IDS_PERFORMANCE,IDS_HELP_PERFORMANCE,kGap-25,25,&m_nPerform, LTFALSE);
	kNumCfg = g_pPerformanceMgr->m_ConfigList.size();

	for (int i = 0; i < kNumCfg; i++)
	{
		HSTRING hTemp = g_pLTClient->CreateString(g_pPerformanceMgr->m_ConfigList[i]->szNiceName);
		m_pPerformance->AddString(hTemp);
		g_pLTClient->FreeString(hTemp);
	}

	m_pPerformance->AddString(IDS_PERFORMANCE_CUSTOM);

	LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_PERFORMANCE,"PerformPos");
	AddFixedControl(m_pPerformance,kCustomGroup0,pos);


	CStaticTextCtrl *pStatic = CreateStaticTextItem(IDS_ADDITIONAL_PERFORMANCE, LTNULL, LTNULL, kAdditionalWidth, 100, LTTRUE, GetHelpFont());
	AddFixedControl(pStatic, kNoGroup, ptAdditionalPerformance, LTFALSE);



	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	return LTTRUE;
}

uint32 CFolderPerformance::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_GENERAL:
		{
			SetTab(0);
			m_PageRect = g_pLayoutMgr->GetFolderCustomRect(FOLDER_ID_PERFORMANCE,"GeneralRect");
			m_pGeneral->Enable(LTFALSE);
			m_pSFX->Enable(LTTRUE);
			m_pTex->Enable(LTTRUE);
//			m_pSound->Enable(LTTRUE);
			UpdateData(LTTRUE);
			BuildControlList();
			UpdateData(LTFALSE);
			break;
		}
	case CMD_SFX:
		{
			SetTab(1);
			m_PageRect = g_pLayoutMgr->GetFolderCustomRect(FOLDER_ID_PERFORMANCE,"GeneralRect");
			m_pGeneral->Enable(LTTRUE);
			m_pSFX->Enable(LTFALSE);
			m_pTex->Enable(LTTRUE);
//			m_pSound->Enable(LTTRUE);
			UpdateData(LTTRUE);
			BuildControlList();
			UpdateData(LTFALSE);
			break;
		}
	case CMD_TEX:
		{
			SetTab(2);
			m_PageRect = g_pLayoutMgr->GetFolderCustomRect(FOLDER_ID_PERFORMANCE,"TextureRect");
			m_pGeneral->Enable(LTTRUE);
			m_pSFX->Enable(LTTRUE);
			m_pTex->Enable(LTFALSE);
//			m_pSound->Enable(LTTRUE);
			UpdateData(LTTRUE);
			BuildControlList();
			UpdateData(LTFALSE);
			break;
		}
	case CMD_SOUND:
		{
			SetTab(3);
			m_PageRect = g_pLayoutMgr->GetFolderCustomRect(FOLDER_ID_PERFORMANCE,"TextureRect");
			m_pGeneral->Enable(LTTRUE);
			m_pSFX->Enable(LTTRUE);
			m_pTex->Enable(LTTRUE);
//			m_pSound->Enable(LTFALSE);
			UpdateData(LTTRUE);
			BuildControlList();
			UpdateData(LTFALSE);
			break;
		}
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


void CFolderPerformance::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		SetTab(0);
		m_PageRect = g_pLayoutMgr->GetFolderCustomRect(FOLDER_ID_PERFORMANCE,"GeneralRect");
		m_pGeneral->Enable(LTFALSE);
		m_pSFX->Enable(LTTRUE);
		m_pTex->Enable(LTTRUE);
		BuildControlList();

		m_nPerform = g_pPerformanceMgr->GetPerformanceCfg();
		if (m_nPerform < 0)
			m_nPerform = kNumCfg;

		GetPerformanceValues();

	}
	else
	{
		SetPerformanceValues();
		RemoveFree();
	}

	CBaseFolder::OnFocus(bFocus);
}

void CFolderPerformance::BuildControlList()
{
	RemoveFree();
	m_pOverall = LTNULL;

	switch (m_nCurTab)
	{
		case 0:
		{
/*			CSliderCtrl *pSlider = AddSlider(IDS_FOG_DISTANCE, IDS_HELP_FOG_DISTANCE, kTextureWidth, kSlider, &m_nFogDistance);
			pSlider->SetSliderRange(0, 100);
			pSlider->SetSliderIncrement(10);
			pSlider->SetParam1(50);
*/
			CToggleCtrl *pToggle = AddToggle(IDS_DISPLAY_TEXTURE,IDS_HELP_TEXTUREDEPTH,kGeneralWidth,&m_b32BitTextures);
			pToggle->SetOnString(IDS_DISPLAY_32BIT);
			pToggle->SetOffString(IDS_DISPLAY_16BIT);

			pToggle=AddToggle(IDS_LIGHTMAP_RES, IDS_HELP_LIGHTMAP_RES, kGeneralWidth, &m_b32BitLightMaps);
			pToggle->SetOnString(IDS_DISPLAY_32BIT);
			pToggle->SetOffString(IDS_DISPLAY_16BIT);

			pToggle=AddToggle(IDS_SHADOWS, IDS_HELP_SHADOWS, kGeneralWidth, &m_bShadows);
			pToggle->SetOnString(IDS_ON);
			pToggle->SetOffString(IDS_OFF);

			pToggle=AddToggle(IDS_DET_TEXTURES, IDS_HELP_DET_TEXTURES, kGeneralWidth, &m_bDetTextures);
			pToggle->SetOnString(IDS_ON);
			pToggle->SetOffString(IDS_OFF);

			pToggle=AddToggle(IDS_ENVIRONMENT, IDS_HELP_ENVIRONMENT, kGeneralWidth, &m_bEnvironment);
			pToggle->SetOnString(IDS_ON);
			pToggle->SetOffString(IDS_OFF);

			pToggle=AddToggle(IDS_CHROME, IDS_HELP_CHROME, kGeneralWidth, &m_bChrome);
			pToggle->SetOnString(IDS_ON);
			pToggle->SetOffString(IDS_OFF);

			pToggle=AddToggle(IDS_TRILINEAR, IDS_HELP_TRILINEAR, kGeneralWidth, &m_bTrilinear);
			pToggle->SetOnString(IDS_ON);
			pToggle->SetOffString(IDS_OFF);

			pToggle=AddToggle(IDS_TRIPLE, IDS_HELP_TRIPLE, kGeneralWidth, &m_bTriple);
			pToggle->SetOnString(IDS_ON);
			pToggle->SetOffString(IDS_OFF);

			uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();
			pToggle->Enable((dwAdvancedOptions & AO_TRIPLEBUFFER));

			pToggle=AddToggle(IDS_DRAWSKY, IDS_HELP_DRAWSKY, kGeneralWidth, &m_bDrawSky);
			pToggle->SetOnString(IDS_ON);
			pToggle->SetOffString(IDS_OFF);

			break;
		} 

		case 1:
		{
			CToggleCtrl *pToggle = AddToggle(IDS_VIEW_MODELS, IDS_HELP_VIEW_MODELS, kGeneralWidth, &m_bViewModels);
			pToggle->SetOnString(IDS_ON);
			pToggle->SetOffString(IDS_OFF);

			pToggle=AddToggle(IDS_MUZZLE_FX, IDS_HELP_MUZZLE_FX, kGeneralWidth, &m_bMuzzle);
			pToggle->SetOnString(IDS_ON);
			pToggle->SetOffString(IDS_OFF);

			pToggle=AddToggle(IDS_CASINGS, IDS_HELP_CASINGS, kGeneralWidth, &m_bCasings);
			pToggle->SetOnString(IDS_ON);
			pToggle->SetOffString(IDS_OFF);

			CCycleCtrl *pCycle = AddCycleItem(IDS_IMPACT_FX, IDS_HELP_IMPACT_FX, kGeneralWidth-25, 25, &m_nImpact);
			pCycle->AddString(IDS_LOW);
			pCycle->AddString(IDS_MEDIUM);
			pCycle->AddString(IDS_HIGH);

			pCycle=AddCycleItem(IDS_DEBRIS_FX, IDS_HELP_DEBRIS_FX, kGeneralWidth-25, 25, &m_nDebris);
			pCycle->AddString(IDS_LOW);
			pCycle->AddString(IDS_MEDIUM);
			pCycle->AddString(IDS_HIGH);

			break;
		}

		case 2:
		{
			m_pOverall=AddCycleItem(IDS_TEX_OVERALL, IDS_HELP_TEX_OVERALL, kTextureWidth-25, 25, &m_nDetail);
			m_pOverall->AddString(IDS_EXTRA_LOW);
			m_pOverall->AddString(IDS_LOW);
			m_pOverall->AddString(IDS_MEDIUM);
			m_pOverall->AddString(IDS_HIGH);
			m_pOverall->AddString(IDS_EXTRA_HIGH);
			m_pOverall->AddString(IDS_DETAIL_ADVANCED);

			CSliderCtrl *pSlider = AddSlider(IDS_TEXTURE_WORLD, IDS_HELP_TEXTURE_WORLD, kTextureWidth, kSlider, &m_nTextures[0]);
			pSlider->SetSliderRange(0, 3);
			pSlider->SetSliderIncrement(1);
			pSlider->SetParam1(1);

			pSlider = AddSlider(IDS_TEXTURE_SKY, IDS_HELP_TEXTURE_SKY, kTextureWidth, kSlider, &m_nTextures[1]);
			pSlider->SetSliderRange(0, 3);
			pSlider->SetSliderIncrement(1);
			pSlider->SetParam1(1);

			pSlider = AddSlider(IDS_TEXTURE_CHARS, IDS_HELP_TEXTURE_CHARS, kTextureWidth, kSlider, &m_nTextures[4]);
			pSlider->SetSliderRange(0, 3);
			pSlider->SetSliderIncrement(1);
			pSlider->SetParam1(1);

			pSlider = AddSlider(IDS_TEXTURE_WEAPON, IDS_HELP_TEXTURE_WEAPONS, kTextureWidth, kSlider, &m_nTextures[3]);
			pSlider->SetSliderRange(0, 3);
			pSlider->SetSliderIncrement(1);
			pSlider->SetParam1(1);

			pSlider = AddSlider(IDS_TEXTURE_PROPS, IDS_HELP_TEXTURE_PROPS, kTextureWidth, kSlider, &m_nTextures[2]);
			pSlider->SetSliderRange(0, 3);
			pSlider->SetSliderIncrement(1);
			pSlider->SetParam1(1);

			pSlider = AddSlider(IDS_TEXTURE_SFX, IDS_HELP_TEXTURE_SFX, kTextureWidth, kSlider, &m_nTextures[5]);
			pSlider->SetSliderRange(0, 3);
			pSlider->SetSliderIncrement(1);
			pSlider->SetParam1(1);

			break;
		}

		case 3:
		{
/*			CSliderCtrl *pSlider = AddSlider(IDS_SOUND_QUANTITY, IDS_HELP_SOUND_QUANTITY, kTextureWidth, kSlider, &m_nSoundQuantity);
			pSlider->SetNumericDisplay(LTTRUE, LTTRUE);
			pSlider->SetSliderRange(16, 64);
			pSlider->SetSliderIncrement(16);

			CToggleCtrl *pToggle = AddToggle(IDS_SOUND_QUALITY, IDS_HELP_SOUNDQUAL, kTextureWidth, &m_bSoundQuality);
			pToggle->SetOnString(IDS_SOUND_HIGH);
			pToggle->SetOffString(IDS_SOUND_LOW);
*/
			break;
		}
	}

}




LTBOOL CFolderPerformance::OnLeft()
{
	if (GetSelectedControl() == m_pPerformance)
	{
		int detailLevel = m_pPerformance->GetSelIndex();
		--detailLevel;

		if (detailLevel < 0)
			detailLevel = kNumCfg-1;

		m_nPerform = detailLevel;
		g_pPerformanceMgr->SetPerformanceCfg(m_nPerform);
		GetPerformanceValues();
        return LTTRUE;
	}

	if (m_nCurTab == 2)
	{
		CLTGUICtrl* pCtrl = GetSelectedControl();

		if (pCtrl == m_pOverall)
		{
			int detailLevel = m_pOverall->GetSelIndex();
			--detailLevel;

			if (detailLevel < 0)
				detailLevel = 4;

			m_pOverall->SetSelIndex(detailLevel);
			m_nDetail = detailLevel;

			SetPerformanceValues();
			UpdateSliders();
			m_nPerform = g_pPerformanceMgr->GetPerformanceCfg();

			if (m_nPerform < 0)
				m_nPerform = kNumCfg;

			UpdateData(LTFALSE);
			return LTTRUE;
		}
		else if (pCtrl && pCtrl->GetParam1())
		{
			pCtrl->OnLeft();
			pCtrl->UpdateData(LTTRUE);

			int  nActualTex[kNumGroups];
			for (int i = 0; i < kNumGroups; i++)
			{
				nActualTex[i] =  2 - m_nTextures[i];
			}

			m_nDetail = g_pPerformanceMgr->GetDetailLevel(nActualTex, kNumGroups);
			m_pOverall->UpdateData(LTFALSE);

			SetPerformanceValues();
			m_nPerform = g_pPerformanceMgr->GetPerformanceCfg();

			if (m_nPerform < 0)
				m_nPerform = kNumCfg;

			UpdateData(LTFALSE);
			return LTTRUE;
	
		}
	}

	if (!CBaseFolder::OnLeft()) return LTFALSE;

	SetPerformanceValues();
	m_nPerform = g_pPerformanceMgr->GetPerformanceCfg();

	if (m_nPerform < 0)
		m_nPerform = kNumCfg;

	UpdateData(LTFALSE);
	return LTTRUE;
}

LTBOOL CFolderPerformance::OnRight()
{
	if (GetSelectedControl() == m_pPerformance)
	{
		int detailLevel = m_pPerformance->GetSelIndex();
		++detailLevel;

		if (detailLevel >= kNumCfg)
			detailLevel = 0;

		m_nPerform = detailLevel;
		m_pPerformance->SetSelIndex(m_nPerform);
		g_pPerformanceMgr->SetPerformanceCfg(m_nPerform);
		GetPerformanceValues();
        return LTTRUE;
	}
	if (m_nCurTab == 2)
	{
		CLTGUICtrl* pCtrl = GetSelectedControl();

		if (pCtrl == m_pOverall)
		{
			int detailLevel = m_pOverall->GetSelIndex();
			++detailLevel;

			if (detailLevel > 4)
				detailLevel = 0;

			m_pOverall->SetSelIndex(detailLevel);
			m_nDetail = detailLevel;

			SetPerformanceValues();
			UpdateSliders();
			m_nPerform = g_pPerformanceMgr->GetPerformanceCfg();

			if (m_nPerform < 0)
				m_nPerform = kNumCfg;

			UpdateData(LTFALSE);
			return LTTRUE;
		}
		else if (pCtrl && pCtrl->GetParam1())
		{
			pCtrl->OnRight();
			pCtrl->UpdateData(LTTRUE);

			int  nActualTex[kNumGroups];
			for (int i = 0; i < kNumGroups; i++)
			{
				nActualTex[i] =  2 - m_nTextures[i];
			}

			m_nDetail = g_pPerformanceMgr->GetDetailLevel(nActualTex, kNumGroups);
			m_pOverall->UpdateData(LTFALSE);

			SetPerformanceValues();
			m_nPerform = g_pPerformanceMgr->GetPerformanceCfg();

			if (m_nPerform < 0)
				m_nPerform = kNumCfg;

			UpdateData(LTFALSE);
			return LTTRUE;
		}
	}

	if (!CBaseFolder::OnRight()) return LTFALSE;

	SetPerformanceValues();
	m_nPerform = g_pPerformanceMgr->GetPerformanceCfg();

	if (m_nPerform < 0)
		m_nPerform = kNumCfg;

	UpdateData(LTFALSE);
	return LTTRUE;
}


LTBOOL CFolderPerformance::OnLButtonUp(int x, int y)
{
	if (GetSelectedControl() == m_pPerformance)
	{
		int detailLevel = m_pPerformance->GetSelIndex();
		++detailLevel;

		if (detailLevel >= kNumCfg)
			detailLevel = 0;

		m_nPerform = detailLevel;
		m_pPerformance->SetSelIndex(m_nPerform);
		g_pPerformanceMgr->SetPerformanceCfg(m_nPerform);
		GetPerformanceValues();
        return LTTRUE;
	}

	if (m_nCurTab == 2)
	{
		int nControlIndex=0;
		if (GetControlUnderPoint(x, y, &nControlIndex))
		{
			CLTGUICtrl* pCtrl = GetControl(nControlIndex);
			if (pCtrl == m_pOverall) 
			{
				return OnRight();
			}
			else if (pCtrl && pCtrl->GetParam1())
			{
				LTBOOL bHandled = CBaseFolder::OnLButtonUp(x, y);
				if (bHandled)
				{
					pCtrl->UpdateData(LTTRUE);

					int  nActualTex[kNumGroups];
					for (int i = 0; i < kNumGroups; i++)
					{
						nActualTex[i] =  2 - m_nTextures[i];
					}

					m_nDetail = g_pPerformanceMgr->GetDetailLevel(nActualTex, kNumGroups);
					m_pOverall->UpdateData(LTFALSE);

					SetPerformanceValues();
					m_nPerform = g_pPerformanceMgr->GetPerformanceCfg();

					if (m_nPerform < 0)
						m_nPerform = kNumCfg;

					UpdateData(LTFALSE);
				}

				return bHandled;
			}
		}
	}

	if (!CBaseFolder::OnLButtonUp(x, y)) return LTFALSE;

	SetPerformanceValues();
	m_nPerform = g_pPerformanceMgr->GetPerformanceCfg();

	if (m_nPerform < 0)
		m_nPerform = kNumCfg;

	UpdateData(LTFALSE);
	return LTTRUE;
}


LTBOOL CFolderPerformance::OnRButtonUp(int x, int y)
{
	return OnLButtonUp(x, y);
}


void CFolderPerformance::UpdateSliders()
{
	int nActualTex[kNumGroups];

	g_pPerformanceMgr->SetDetailLevels(m_nDetail, nActualTex);

	for (int i = 0; i < kNumGroups; i++)
	{
		m_nTextures[i] = 2 - nActualTex[i];
	}

	UpdateData(LTFALSE);
}


void CFolderPerformance::GetPerformanceValues()
{
	m_nFogDistance = GetConsoleInt("FogPerformanceScale", 100);
	m_b32BitTextures = (LTBOOL)GetConsoleInt("32BitTextures",1);
	m_b32BitLightMaps = (LTBOOL)GetConsoleInt("32BitLightMaps",1);
	m_bShadows = (LTBOOL)GetConsoleInt("MaxModelShadows",0);
	m_bDetTextures = (LTBOOL)GetConsoleInt("DetailTextures",1);
	m_bEnvironment = (LTBOOL)GetConsoleInt("EnvMapWorld",1);
	m_bChrome = (LTBOOL)GetConsoleInt("EnvMapEnable",1);
	m_bTrilinear = (LTBOOL)GetConsoleInt("Trilinear",0);
	m_bTriple = (LTBOOL)GetConsoleInt("TripleBuffer",0);
	m_bDrawSky = (LTBOOL)GetConsoleInt("EnableSky",1);
	m_bCasings = (LTBOOL)GetConsoleInt("ShellCasings",1);
	m_bViewModels = (LTBOOL)GetConsoleInt("DrawViewModel",1);
	m_bMuzzle = (LTBOOL)GetConsoleInt("MuzzleLight",1);
	m_nImpact = GetConsoleInt("ImpactFXLevel",1);
	m_nDebris = GetConsoleInt("DebrisFXLevel",1);

	char szVar[32] = "";
	int  nActualTex[kNumGroups];
	for (int i = 0; i < kNumGroups; i++)
	{
		sprintf(szVar,"GroupOffset%d",i);
		nActualTex[i] =  GetConsoleInt(szVar,0);
		m_nTextures[i] = 2 - nActualTex[i];
	}

	m_nDetail = g_pPerformanceMgr->GetDetailLevel(nActualTex, kNumGroups);

	UpdateData(LTFALSE);
}


void CFolderPerformance::SetPerformanceValues()
{
	UpdateData(LTTRUE);

	WriteConsoleInt("FogPerformanceScale", m_nFogDistance);
	WriteConsoleInt("32BitTextures",m_b32BitTextures);
	WriteConsoleInt("32BitLightMaps",m_b32BitLightMaps);
	WriteConsoleInt("MaxModelShadows",m_bShadows);
	WriteConsoleInt("DetailTextures",m_bDetTextures);
	WriteConsoleInt("EnvMapWorld",m_bEnvironment);
	WriteConsoleInt("EnvMapEnable",m_bChrome);
	WriteConsoleInt("Trilinear",m_bTrilinear);
	WriteConsoleInt("TripleBuffer",m_bTriple);
	WriteConsoleInt("EnableSky",m_bDrawSky);
	WriteConsoleInt("ShellCasings",m_bCasings);
	WriteConsoleInt("DrawViewModel",m_bViewModels);
	WriteConsoleInt("MuzzleLight",m_bMuzzle);
	WriteConsoleInt("ImpactFXLevel",m_nImpact);
	WriteConsoleInt("DebrisFXLevel",m_nDebris);
	WriteConsoleInt("DetailLevel",m_nDetail);

	char szVar[32] = "";
	for (int i = 0; i < kNumGroups; i++)
	{
		sprintf(szVar,"GroupOffset%d",i);
		int nActualTex = 2-m_nTextures[i];
		WriteConsoleInt(szVar,nActualTex);
	}

    g_pLTClient->WriteConfigFile("autoexec.cfg");

	if(g_pGameClientShell->GetVisionModeMgr())
		g_pGameClientShell->GetVisionModeMgr()->ResetEnvMap();
}


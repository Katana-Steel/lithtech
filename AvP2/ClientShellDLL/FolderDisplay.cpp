// FolderDisplay.cpp: implementation of the CFolderDisplay class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderDisplay.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
#include "GameSettings.h"
#include "PerformanceMgr.h"
extern CGameClientShell* g_pGameClientShell;

namespace
{
	LTIntPt ptAdditionalPerformance;
	int kAdditionalWidth = 0;

	int kGap = 0;
	int kWidth = 0;
	const int CMD_PERFORM = FOLDER_CMD_CUSTOM+1;

	int nInitVal[kNumGroups];
	int nInitTB;
	LTBOOL bInitTex;
	LTBOOL bInitLM;
	int kNumCfg;

}

//helper function used for sorting
int FolderDisplayCompare( const void *arg1, const void *arg2 )
{
	FolderDisplayResolution *pRes1=(FolderDisplayResolution *)arg1;
	FolderDisplayResolution *pRes2=(FolderDisplayResolution *)arg2;

	if (pRes1->m_dwWidth < pRes2->m_dwWidth)
	{
		return -1;
	}
	else if (pRes1->m_dwWidth > pRes2->m_dwWidth)
	{
		return 1;
	}
	else
	{
		if ( pRes1->m_dwHeight < pRes2->m_dwHeight )
		{
			return -1;
		}
		else if ( pRes1->m_dwHeight > pRes2->m_dwHeight )
		{
			return 1;
		}
		else
		{
			if ( pRes1->m_dwBitDepth < pRes2->m_dwBitDepth )
			{
				return -1;
			}
			else if ( pRes1->m_dwBitDepth > pRes2->m_dwBitDepth )
			{
				return 1;
			}

		}
	}

	return 0;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderDisplay::CFolderDisplay()
{

	m_bEscape		=	LTFALSE;

    m_pResolutionLabel  = LTNULL;
    m_pResolutionCtrl   = LTNULL;
}

CFolderDisplay::~CFolderDisplay()
{

}

// Build the folder
LTBOOL CFolderDisplay::Build()
{
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_DISPLAY,"ColumnWidth"))
		kGap = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_DISPLAY,"ColumnWidth");
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_DISPLAY,"SliderWidth"))
		kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_DISPLAY,"SliderWidth");

	ptAdditionalPerformance = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_DISPLAY,"AdditionalPos");
	kAdditionalWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_PERFORMANCE,"AdditionalWidth");


	CreateTitle(IDS_TITLE_DISPLAY);

	// Build the array of renderers
	GetRendererData();

	// Add the "resolution" control
    m_pResolutionCtrl = AddCycleItem(IDS_DISPLAY_RESOLUTION,IDS_HELP_RESOLUTION,kGap-25,25,LTNULL, LTFALSE);

	// Setup the resolution control based on the current renderer
	SetupResolutionCtrl();


	CStaticTextCtrl *pStatic = CreateStaticTextItem(IDS_ADDITIONAL_PERFORMANCE, LTNULL, LTNULL, kAdditionalWidth, 100, LTTRUE, GetHelpFont());
	AddFixedControl(pStatic, kNoGroup, ptAdditionalPerformance, LTFALSE);


	m_pPerformance = AddCycleItem(IDS_PERFORMANCE,IDS_HELP_PERFORMANCE,kGap-25,25,&m_nOverall, LTFALSE);
	kNumCfg = g_pPerformanceMgr->m_ConfigList.size();

	for (int i = 0; i < kNumCfg; i++)
	{
		HSTRING hTemp = g_pLTClient->CreateString(g_pPerformanceMgr->m_ConfigList[i]->szNiceName);
		m_pPerformance->AddString(hTemp);
		g_pLTClient->FreeString(hTemp);
	}

	m_pPerformance->AddString(IDS_PERFORMANCE_CUSTOM);

	LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_DISPLAY,"CustomPos");
	CLTGUITextItemCtrl* pCtrl = CreateTextItem(IDS_CUSTOM_PERFORM,CMD_PERFORM,IDS_HELP_CUSTOM_PERFORM, LTFALSE, GetLargeFont());
	AddFixedControl(pCtrl,kCustomGroup0,pos);	


 	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;

	return LTTRUE;
}

void CFolderDisplay::Escape()
{
	m_bEscape = LTTRUE;
	CBaseFolder::Escape();
}

uint32 CFolderDisplay::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_PERFORM:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_PERFORMANCE);
			break;
		}
	case FOLDER_CMD_OPTIONS:
		{
			m_bEscape = LTTRUE;
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_OPTIONS);
			break;
		}
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}

	return 1;

};

// Setup the resolution control based on the currently selected resolution
void CFolderDisplay::SetupResolutionCtrl()
{
	if (!m_pResolutionCtrl)
	{
		return;
	}


	// Get the selected renderer
	int nResolutionIndex=m_pResolutionCtrl->GetSelIndex();

	// Clear the current resolutions
	m_pResolutionCtrl->RemoveAll();
    uint32 dwOldWidth=0;
    uint32 dwOldHeight=0;
    uint32 dwOldBitDepth=0;
    uint32 dwCurWidth = 0;
    uint32 dwCurHeight = 0;
    uint32 dwCurBitDepth = 0;
	int	 nNewRes = 0;

	unsigned int nResolutions=m_rendererData.m_resolutionArray.GetSize();

	if ((unsigned int)nResolutionIndex < nResolutions)
	{
		dwOldWidth=m_rendererData.m_resolutionArray[nResolutionIndex].m_dwWidth;
		dwOldHeight=m_rendererData.m_resolutionArray[nResolutionIndex].m_dwHeight;
		dwOldBitDepth=m_rendererData.m_resolutionArray[nResolutionIndex].m_dwBitDepth;
	}


	// Add each resolution
	unsigned int i;
	for (i=0; i < nResolutions; i++)
	{
        uint32 dwWidth=m_rendererData.m_resolutionArray[i].m_dwWidth;
        uint32 dwHeight=m_rendererData.m_resolutionArray[i].m_dwHeight;
        uint32 dwBitDepth=m_rendererData.m_resolutionArray[i].m_dwBitDepth;

        if (    LTDIFF(dwWidth,dwOldWidth) <= LTDIFF(dwCurWidth,dwOldWidth) &&
                LTDIFF(dwHeight,dwOldHeight) <= LTDIFF(dwCurHeight,dwOldHeight) &&
                LTDIFF(dwBitDepth,dwOldBitDepth) < LTDIFF(dwCurBitDepth,dwOldBitDepth)
				)
		{
			nNewRes = i;
			dwCurWidth = dwWidth;
			dwCurHeight = dwHeight;
			dwCurBitDepth = dwBitDepth;
		}
		// Load the resolution format string.  This is "Resolution: [%dx%dx%d]" in English
        HSTRING hResolutionFormat=g_pLTClient->FormatString(IDS_DMODE_RESOLUTION, dwWidth, dwHeight, dwBitDepth);
		m_pResolutionCtrl->AddString(hResolutionFormat);
        g_pLTClient->FreeString(hResolutionFormat);
	}

	m_pResolutionCtrl->SetSelIndex(nNewRes);

}

// Build the array of renderers
void CFolderDisplay::GetRendererData()
{
	m_rendererData.m_renderDll[0] = '\0';
	m_rendererData.m_description[0] = '\0';
	m_rendererData.m_internalName[0] = '\0';

	// Get the name of our selected renderer
	char szRend[128];
	GetConsoleString("carddesc",szRend,"none");

	// see if we have a prefered card..
	LTBOOL bSpecifyCard = LTTRUE;
	if(_stricmp(szRend,"none")==0)
		bSpecifyCard = LTFALSE;


	// Build the list of render modes
    RMode *pRenderModes=g_pLTClient->GetRenderModes();

	// Iterate through the list of render modes adding each one to the array
	RMode *pCurrentMode=pRenderModes;
    while (pCurrentMode != LTNULL)
	{
		if (pCurrentMode->m_Width >= 640 && pCurrentMode->m_Height >= 480)
		{
			// disallow non-standard aspect ratios
			uint32 testWidth = (pCurrentMode->m_Height * 4 / 3);
			if (pCurrentMode->m_Width != testWidth)
			{
			
				// Go to the next render mode
				pCurrentMode=pCurrentMode->m_pNext;
				continue;
			}

			// see if we need to specify the card..
			if(bSpecifyCard)
			{
				if(_stricmp(pCurrentMode->m_InternalName, szRend) != 0)
				{
					// Go to the next render mode
					pCurrentMode=pCurrentMode->m_pNext;
					continue;
				}
			}

			// Check to see if we need to add this renderer
			if (!m_rendererData.m_renderDll[0])
			{
				m_rendererData.m_bHardware=pCurrentMode->m_bHardware;
				_mbscpy((unsigned char*)m_rendererData.m_renderDll, (const unsigned char*)pCurrentMode->m_RenderDLL);
				_mbscpy((unsigned char*)m_rendererData.m_description, (const unsigned char*)pCurrentMode->m_Description);
				_mbscpy((unsigned char*)m_rendererData.m_internalName, (const unsigned char*)pCurrentMode->m_InternalName);

			}

			//HACK ALERT: we only support one renderer, so if there are multiple options, only use resolutions
			//	for the first one we find
			if ( stricmp(m_rendererData.m_renderDll, pCurrentMode->m_RenderDLL) == 0 && 
				 stricmp(m_rendererData.m_description, pCurrentMode->m_Description) == 0 && 
				 stricmp(m_rendererData.m_internalName, pCurrentMode->m_InternalName) == 0
				)
			{
				// Add the display resolutions for this renderer
				FolderDisplayResolution resolution;
				resolution.m_dwWidth=pCurrentMode->m_Width;
				resolution.m_dwHeight=pCurrentMode->m_Height;
				resolution.m_dwBitDepth=pCurrentMode->m_BitDepth;

				m_rendererData.m_resolutionArray.Add(resolution);
			}
		}

		// Go to the next render mode
		pCurrentMode=pCurrentMode->m_pNext;
	}

	// Free the linked list of render modes
    g_pLTClient->RelinquishRenderModes(pRenderModes);

	// Sort the render resolution based on screen width and height
	SortRenderModes();
}

// Sort the render resolution based on screen width and height
void CFolderDisplay::SortRenderModes()
{
	// Build a temporary array of render modes
	int nResolutions=m_rendererData.m_resolutionArray.GetSize();
	if ( nResolutions < 1 )
	{
		return;
	}

	FolderDisplayResolution *pResolutions = new FolderDisplayResolution[nResolutions];

	int i;
	for (i=0; i < nResolutions; i++)
	{
		pResolutions[i]=m_rendererData.m_resolutionArray[i];
	}

	// Sort the array
	qsort(pResolutions, nResolutions, sizeof(FolderDisplayResolution), FolderDisplayCompare);

	// Clear the current renderer resolutions array
	m_rendererData.m_resolutionArray.SetSize(0);

	// Copy the sorted array back to the resolutions array
	for (i=0; i < nResolutions; i++)
	{
		m_rendererData.m_resolutionArray.Add(pResolutions[i]);
	}

	delete []pResolutions;
}

// Sets the renderer
LTBOOL CFolderDisplay::SetRenderer(int nResolutionIndex)
{
	// Get the new renderer structure
	RMode newMode=GetRendererModeStruct(nResolutionIndex);

	// If the current renderer is the same as the one we are changing to, then don't set the renderer!
	RMode currentMode;
	if (g_pLTClient->GetRenderMode(&currentMode) == LT_OK)
	{
		if (IsRendererEqual(&newMode, &currentMode))
		{
			return LTFALSE;
		}
	}

	// Set the renderer mode
	g_pInterfaceMgr->SetSwitchingRenderModes(LTTRUE);
	g_pInterfaceMgr->SetSwitchingRenderModes(LTTRUE);
	g_pLTClient->Start3D();
	g_pLTClient->StartOptimized2D();

	g_pInterfaceResMgr->DrawMessage(GetSmallFont(),IDS_REINITIALIZING_RENDERER);

	g_pLTClient->EndOptimized2D();
	g_pLTClient->End3D();
	g_pLTClient->FlipScreen(0);
	g_pLTClient->SetRenderMode(&newMode);
	g_pInterfaceMgr->SetSwitchingRenderModes(LTFALSE);



	// Write the renderer information to the autoexec.cfg file
	char szTemp[256];
	sprintf(szTemp, "+RenderDll %s\0", newMode.m_RenderDLL);
	g_pLTClient->RunConsoleString(szTemp);

	sprintf(szTemp, "+CardDesc %s\0", newMode.m_InternalName);
	g_pLTClient->RunConsoleString(szTemp);

	sprintf(szTemp, "+GameScreenWidth %d\0", newMode.m_Width);
	g_pLTClient->RunConsoleString(szTemp);

	sprintf(szTemp, "+GameScreenHeight %d\0", newMode.m_Height);
	g_pLTClient->RunConsoleString(szTemp);

	sprintf(szTemp, "+GameBitDepth %d\0", newMode.m_BitDepth);
	g_pLTClient->RunConsoleString(szTemp);


	g_pInterfaceMgr->ScreenDimsChanged();
	// Add this when I know what it does
//	g_pInterfaceMgr->InitCursor();


    return LTTRUE;
}

// Gets a RMode structure based on a renderer index and a resolution index
RMode CFolderDisplay::GetRendererModeStruct(int nResolutionIndex)
{
	// Copy the renderer information from the renderer structure to the temporary RMode structure
	RMode mode;
	mode.m_bHardware=m_rendererData.m_bHardware;

	_mbscpy((unsigned char*)mode.m_RenderDLL, (const unsigned char*)m_rendererData.m_renderDll);
	_mbscpy((unsigned char*)mode.m_InternalName, (const unsigned char*)m_rendererData.m_internalName);
	_mbscpy((unsigned char*)mode.m_Description, (const unsigned char*)m_rendererData.m_description);

	FolderDisplayResolution resolution=m_rendererData.m_resolutionArray[nResolutionIndex];
	mode.m_Width=resolution.m_dwWidth;
	mode.m_Height=resolution.m_dwHeight;
	mode.m_BitDepth=resolution.m_dwBitDepth;

    mode.m_pNext=LTNULL;

	return mode;
}

// Returns TRUE if two renderers are the same
LTBOOL CFolderDisplay::IsRendererEqual(RMode *pRenderer1, RMode *pRenderer2)
{
	_ASSERT(pRenderer1);
	_ASSERT(pRenderer2);

	if (_mbsicmp((const unsigned char*)pRenderer1->m_RenderDLL, (const unsigned char*)pRenderer2->m_RenderDLL) != 0)
	{
        return LTFALSE;
	}

	if (_mbsicmp((const unsigned char*)pRenderer1->m_InternalName, (const unsigned char*)pRenderer2->m_InternalName) != 0)
	{
        return LTFALSE;
	}

	if (_mbsicmp((const unsigned char*)pRenderer1->m_Description, (const unsigned char*)pRenderer2->m_Description) != 0)
	{
        return LTFALSE;
	}

	if (pRenderer1->m_Width != pRenderer2->m_Width)
	{
        return LTFALSE;
	}

	if (pRenderer1->m_Height != pRenderer2->m_Height)
	{
        return LTFALSE;
	}

	if (pRenderer1->m_BitDepth != pRenderer2->m_BitDepth)
	{
        return LTFALSE;
	}

    return LTTRUE;
}

// Called when the folder gains or loses focus
void CFolderDisplay::OnFocus(LTBOOL bFocus)
{
	CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();

	if (bFocus)
	{

		m_bEscape = LTFALSE;

		if (m_pFolderMgr->GetLastFolderID() != FOLDER_ID_PERFORMANCE)
		{
			// The current render mode
			RMode currentMode;
			g_pLTClient->GetRenderMode(&currentMode);

			// Read in any changes
			HCONSOLEVAR	hVar;
			hVar					= g_pLTClient->GetConsoleVar("GameScreenWidth");
			currentMode.m_Width		= (DDWORD) g_pLTClient->GetVarValueFloat(hVar);
			hVar					= g_pLTClient->GetConsoleVar("GameScreenHeight");
			currentMode.m_Height	= (DDWORD) g_pLTClient->GetVarValueFloat(hVar);
			hVar					= g_pLTClient->GetConsoleVar("GameBitDepth");
			currentMode.m_BitDepth	= (DDWORD) g_pLTClient->GetVarValueFloat(hVar);

			// Set the renderer controls so that they match the currently selected renderer
			unsigned int i;
			for (i=0; i < m_rendererData.m_resolutionArray.GetSize(); i++)
			{
				RMode mode=GetRendererModeStruct(i);

				if (IsRendererEqual(&currentMode, &mode))
				{
					// Setup the resolution control
					SetupResolutionCtrl();

					// Set the resolution index
					m_pResolutionCtrl->SetSelIndex(i);
				}
			}

			for (i = 0; i < kNumGroups; i++)
			{
				char szKey[16];
				sprintf(szKey,"GroupOffset%d",i);
				nInitVal[i] = GetConsoleInt(szKey,0);
			}
			nInitTB = GetConsoleInt("TripleBuffer",0);
			bInitTex = (LTBOOL)GetConsoleInt("32BitTextures",0);
			bInitLM	= (LTBOOL)GetConsoleInt("32BitLightmaps",0);
		}

		m_nOverall = g_pPerformanceMgr->GetPerformanceCfg();
		if (m_nOverall < 0)
			m_nOverall = kNumCfg;

        UpdateData(LTFALSE);

	}
	else
	{
		UpdateData();

		g_pPerformanceMgr->SetPerformanceCfg(m_nOverall);

		if (m_bEscape)
		{


			LTBOOL bRebind = (bInitTex != (LTBOOL)GetConsoleInt("32BitTextures",0));
			LTBOOL bRebindLM = (bInitLM	!= (LTBOOL)GetConsoleInt("32BitLightmaps",0));
			LTBOOL bSetRend = LTFALSE;

			// Set the render mode if we are losing focus
			if (m_pResolutionCtrl)
			{

				int oldMO = (int)pSettings->GetFloatVar("MipmapOffset");

				bSetRend = SetRenderer(m_pResolutionCtrl->GetSelIndex());

				// If we didn't switch resolutions and the mipmap offset changed, rebind textures.
				if(!bSetRend)
				{
					int curMO = (int)pSettings->GetFloatVar("MipmapOffset");
					if(curMO != oldMO)
					{
						bRebind = LTTRUE;
					}
				}
			}
			if (!bSetRend)
			{
				for (int i = 0; i < kNumGroups; i++)
				{
					char szKey[16];
					sprintf(szKey,"GroupOffset%d",i);
					if (nInitVal[i] != GetConsoleInt(szKey,0))
					{
						bRebind = LTTRUE;
					}
				}
				if (nInitTB != GetConsoleInt("TripleBuffer",0))
				{
					// Set the renderer mode
					g_pInterfaceMgr->SetSwitchingRenderModes(LTTRUE);
					g_pLTClient->Start3D();
					g_pLTClient->StartOptimized2D();

					g_pInterfaceResMgr->DrawMessage(GetSmallFont(),IDS_REINITIALIZING_RENDERER);

					g_pLTClient->EndOptimized2D();
					g_pLTClient->End3D();
					g_pLTClient->FlipScreen(0);
					g_pLTClient->RunConsoleString("RestartRender");
					g_pInterfaceMgr->SetSwitchingRenderModes(LTFALSE);

					//no need to rebind if we restarted renderer
					bRebind = LTFALSE;
					bRebindLM = LTFALSE;
				}
			}

			if (bRebind)
			{

                g_pLTClient->Start3D();
                g_pLTClient->StartOptimized2D();

				g_pInterfaceResMgr->DrawMessage(GetSmallFont(),IDS_REBINDING_TEXTURES);

                g_pLTClient->EndOptimized2D();
                g_pLTClient->End3D();
                g_pLTClient->FlipScreen(0);
				g_pLTClient->RunConsoleString("RebindTextures");

			}
			if (bRebindLM)
			{

                g_pLTClient->Start3D();
                g_pLTClient->StartOptimized2D();

				g_pInterfaceResMgr->DrawMessage(GetSmallFont(),IDS_REBINDING_LIGHTMAPS);

                g_pLTClient->EndOptimized2D();
                g_pLTClient->End3D();
                g_pLTClient->FlipScreen(0);
				g_pLTClient->RunConsoleString("RebindLightmaps");

			}
		
			m_bEscape = LTFALSE;
			
		}

        g_pLTClient->WriteConfigFile("autoexec.cfg");


	}
	CBaseFolder::OnFocus(bFocus);
}

// Returns the currently selected resolution
FolderDisplayResolution CFolderDisplay::GetCurrentSelectedResolution()
{
	int nResolutionIndex=m_pResolutionCtrl->GetSelIndex();

	_ASSERT(nResolutionIndex >= 0);
	if (nResolutionIndex >= 0)
		return m_rendererData.m_resolutionArray[nResolutionIndex];
	else
		return m_rendererData.m_resolutionArray[0];
}

// Set the resolution for the resolution control.  If it cannot be found the
// next highest resolution is selected.
void CFolderDisplay::SetCurrentCtrlResolution(FolderDisplayResolution resolution)
{

	// Go through the resolutions searching for a match
	unsigned int i;
	for (i=0; i < m_rendererData.m_resolutionArray.GetSize(); i++)
	{
		FolderDisplayResolution searchRes=m_rendererData.m_resolutionArray[i];

		if (resolution.m_dwWidth == searchRes.m_dwWidth &&
			resolution.m_dwHeight == searchRes.m_dwHeight &&
			resolution.m_dwBitDepth == searchRes.m_dwBitDepth)
		{
			m_pResolutionCtrl->SetSelIndex(i);
			return;
		}
	}

	// Since an exact match wasn't found, set it to the next highest resolution
	for (i=0; i < m_rendererData.m_resolutionArray.GetSize(); i++)
	{
		FolderDisplayResolution searchRes=m_rendererData.m_resolutionArray[i];

		if (resolution.m_dwWidth > searchRes.m_dwWidth ||
			resolution.m_dwHeight > searchRes.m_dwHeight &&
			resolution.m_dwBitDepth == searchRes.m_dwBitDepth)
		{
			if (i > 0)
			{
				m_pResolutionCtrl->SetSelIndex(i-1);
			}
			else
			{
				m_pResolutionCtrl->SetSelIndex(0);
			}
			return;
		}
	}
}


LTBOOL CFolderDisplay::OnLeft()
{
	if (GetSelectedControl() == m_pPerformance)
	{
		int detailLevel = m_pPerformance->GetSelIndex();
		--detailLevel;
		if (detailLevel < 0)
			detailLevel = kNumCfg-1;
		m_pPerformance->SetSelIndex(detailLevel);
		m_nOverall = detailLevel;
        return LTTRUE;
	}
	LTBOOL bHandled = CBaseFolder::OnLeft();
	if (bHandled)
	{
	}
	return bHandled;
}

LTBOOL CFolderDisplay::OnRight()
{
	if (GetSelectedControl() == m_pPerformance)
	{
		int detailLevel = m_pPerformance->GetSelIndex();
		++detailLevel;
		if (detailLevel >= kNumCfg)
			detailLevel = 0;
		m_pPerformance->SetSelIndex(detailLevel);
		m_nOverall = detailLevel;

		return LTTRUE;
	}

	LTBOOL bHandled = CBaseFolder::OnRight();
	if (bHandled)
	{
	}
	return bHandled;
	
}


LTBOOL CFolderDisplay::OnLButtonUp(int x, int y)
{
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pPerformance)
		{
			return OnRight();
		}
	}
	return CBaseFolder::OnLButtonUp(x, y);
}

LTBOOL CFolderDisplay::OnRButtonUp(int x, int y)
{
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pPerformance)
		{
			return OnLeft();
		}
	}
	return CBaseFolder::OnRButtonUp(x, y);
}

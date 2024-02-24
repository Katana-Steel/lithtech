/****************************************************************************
;
;	 MODULE:		DISPLAYMGR (.CPP)
;
;	PURPOSE:		Display manager class (for movies, demos, etc.)
;
;	HISTORY:		06/08/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


// Includes...

#include "StdAfx.h"
#include "DisplayMgr.h"
#include "io.h"

#include "d3d8.h"

// Dummy declaration for proper linking
IDirect3D8 * WINAPI Direct3DCreate8(UINT SDKVersion) { return 0; }
typedef IDirect3D8 *(WINAPI *TD3DCreateFn)(UINT SDKVersion);

#define DX8_RENDERER_NAME "DX8 Renderer"

// Functions...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CResolution::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CResolution::Init(int nWidth, int nHeight, int nDepth)
{
	// Sanity checks...

	ASSERT(!IsValid());


	// Set simple members...

	m_nWidth  = nWidth;
	m_nHeight = nHeight;
	m_nDepth  = nDepth;


	// Construct the name...

	m_sName.Format("%i x %i x %i", m_nWidth, m_nHeight, m_nDepth);


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CResolution::GetListText
//
//	PURPOSE:	Formats a text string to be used for display purposes
//
// ----------------------------------------------------------------------- //

CString CResolution::GetListText()
{
	// All done...

	return(GetName());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplay::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CDisplay::Init(const char* sName, const char* sDesc, BOOL bHardware)
{
	// Sanity checks...

	ASSERT(!IsValid());


	// Set simple members...

	m_sName     = sName;
	m_sDesc     = sDesc;
	m_bHardware = bHardware;


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplay::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CDisplay::Term()
{
	// Terminate all resolution objects...

	POSITION pos = m_collResolutions.GetHeadPosition();

	while (pos)
	{
		CResolution* pResolution = (CResolution*)m_collResolutions.GetNext(pos);
		ASSERT(pResolution);
		if (pResolution) delete pResolution;
	}

	m_collResolutions.RemoveAll();


	// Clear all members...

	Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplay::GetListText
//
//	PURPOSE:	Formats a text string to be used for display purposes
//
// ----------------------------------------------------------------------- //

CString CDisplay::GetListText()
{
	// Build the text string...

	CString sDisplay;

	sDisplay.Format("%s (%s)", GetDescription(), GetName());


	// All done...

	return(sDisplay);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplay::GetSelectedListResolution
//
//	PURPOSE:	Gets the currently selected resolution from the given list
//				box.
//
// ----------------------------------------------------------------------- //

CResolution* CDisplay::GetSelectedListResolution(HWND hList)
{
	// Sanity checks...

	ASSERT(IsValid());
	if (!hList) return(NULL);


	// Get the current selection index...

	int index = SendMessage(hList, LB_GETCURSEL, 0, 0);
	if (index == LB_ERR) return(NULL);


	// Get the selected resolution from the index...

	LRESULT lResult = SendMessage(hList, LB_GETITEMDATA, index, 0);
	if (lResult == LB_ERR) return(NULL);

	CResolution* pResolution = (CResolution*)lResult;
	if (!pResolution) return(NULL);


	// All done...

	return(pResolution);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplay::AddResolution
//
//	PURPOSE:	Adds a new resolution object
//
// ----------------------------------------------------------------------- //

CResolution* CDisplay::AddResolution(int nWidth, int nHeight, int nDepth)
{
	// Create and init a new display object...

	CResolution* pResolution = new CResolution();

	if (!pResolution->Init(nWidth, nHeight, nDepth))
	{
		delete pResolution;
		return(NULL);
	}


	// Add the new object to our list...

	m_collResolutions.AddTail(pResolution);


	// All done...

	return(pResolution);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplay::FindResolution
//
//	PURPOSE:	Searches for a resolution with the given dimensions
//
// ----------------------------------------------------------------------- //

CResolution* CDisplay::FindResolution(int nWidth, int nHeight, int nDepth)
{
	// Sanity checks...

	ASSERT(IsValid());


	// Search for a resolution object with the given dimensions...

	POSITION pos = m_collResolutions.GetHeadPosition();

	while (pos)
	{
		CResolution* pResolution = (CResolution*)m_collResolutions.GetNext(pos);
		ASSERT(pResolution);

		if (pResolution->GetWidth() == nWidth && pResolution->GetHeight() == nHeight && pResolution->GetDepth() == nDepth)
		{
			return(pResolution);
		}
	}


	// If we get here, we didn't find a match...

	return(NULL);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplay::SetCurrentResolution
//
//	PURPOSE:	Sets the current resolution for the display
//
// ----------------------------------------------------------------------- //

BOOL CDisplay::SetCurrentResolution(int nWidth, int nHeight, int nDepth)
{
	// Sanity checks...

	ASSERT(IsValid());


	// Find the requested resolution...

	CResolution* pResolution = FindResolution(nWidth, nHeight, nDepth);
	if (!pResolution) return(FALSE);


	// Set the current resolution...

	m_pCurResolution = pResolution;


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplay::SelectCurrentListResolution
//
//	PURPOSE:	Select the current resolution in the given list box
//
// ----------------------------------------------------------------------- //

BOOL CDisplay::SelectCurrentListResolution(HWND hList)
{
	// Sanity checks...

	ASSERT(IsValid());
	if (!hList) return(FALSE);
	if (!m_pCurResolution) return(FALSE);


	// Search for this item in the list box...

	int nCount = SendMessage(hList, LB_GETCOUNT, 0, 0);
	if (nCount <= 0) return(FALSE);

	for (int i = 0; i < nCount; i++)
	{
		LRESULT lRet = SendMessage(hList, LB_GETITEMDATA, i, 0);
		if (lRet != LB_ERR)
		{
			CResolution* pResolution = (CResolution*)lRet;

			if (pResolution == m_pCurResolution)
			{
				SendMessage(hList, LB_SETCURSEL, i, 0);
				return(TRUE);
			}
		}
	}


	// If we get here, we didn't fine this resolution in the list box...

	return(FALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplay::FillListBox
//
//	PURPOSE:	Fills the given list box with the current resolutions
//
// ----------------------------------------------------------------------- //

BOOL CDisplay::FillListBox(HWND hList)
{
	// Sanity checks...

	if (!hList) return(FALSE);


	// Clear the contents of the list box...

	SendMessage(hList, LB_RESETCONTENT, 0, 0);


	// Insert each resolution...

	POSITION pos = m_collResolutions.GetHeadPosition();

	while (pos)
	{
		CResolution* pResolution = (CResolution*)m_collResolutions.GetNext(pos);
		ASSERT(pResolution);

		int iItem = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)pResolution->GetListText());

		if (iItem != LB_ERR)
		{
			SendMessage(hList, LB_SETITEMDATA, iItem, (LPARAM)pResolution);
		}
	}


	// Set the current selection if requested...

	if (!SelectCurrentListResolution(hList))
	{
		SendMessage(hList, LB_SETCURSEL, 0, 0);
	}


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderer::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CRenderer::Init(const char* sFile)
{
	// Load d3d8.dll
	HMODULE hD3DDll = LoadLibrary("d3d8.dll");
	if (!hD3DDll)
		return FALSE;

	TD3DCreateFn exp_Direct3DCreate8 = (TD3DCreateFn)GetProcAddress(hD3DDll, "Direct3DCreate8");
	if (!exp_Direct3DCreate8)
		return FALSE;

	// Gimmie D3D, baby!
	IDirect3D8 *pD3D = exp_Direct3DCreate8(D3D_SDK_VERSION);
	if (!pD3D)
		return FALSE;

	// Go through the adapters on the system
	uint32 nAdapterCount = pD3D->GetAdapterCount();
    for (uint32 iAdapter = 0; iAdapter < nAdapterCount; ++iAdapter) 
	{
        D3DADAPTER_IDENTIFIER8 AdapterInfo;		// Fill in adapter info
		pD3D->GetAdapterIdentifier(iAdapter,0,&AdapterInfo);

		// Enumerate all display modes on this adapter...
        uint32 iNumAdapterModes = pD3D->GetAdapterModeCount(iAdapter);

        for (uint32 iMode = 0; iMode < iNumAdapterModes; iMode++) 
		{
			D3DDISPLAYMODE d3dDisplayMode; // Get the display mode attributes
			pD3D->EnumAdapterModes(iAdapter,iMode, &d3dDisplayMode);

			// Filter out low-resolution modes
			if (d3dDisplayMode.Width < 640 || d3dDisplayMode.Height < 480) 
				continue;

			// Only allow 4x3 aspect ratio video modes
			uint32 nTestWidth = (d3dDisplayMode.Height * 4 / 3);
			if (nTestWidth != d3dDisplayMode.Width)
				continue;

			// 32-bit display modes only
			if (d3dDisplayMode.Format != D3DFMT_X8R8G8B8)
				continue;

			RMode cMode;
			
			cMode.m_Width = d3dDisplayMode.Width;
			cMode.m_Height = d3dDisplayMode.Height;
			strcpy(cMode.m_InternalName, AdapterInfo.Driver);
			strcpy(cMode.m_Description, AdapterInfo.Description);
			// We're always hardware now..  This should really be removed...
			cMode.m_bHWTnL = true;

			// Find the display adaptor
			CDisplay *pDisplay = FindDisplay(&cMode);
			if (!pDisplay)
				pDisplay = AddDisplay(&cMode);

			// Add the resolution
			if (pDisplay)
			{
				if (!pDisplay->FindResolution(cMode.m_Width, cMode.m_Height, 32))
					pDisplay->AddResolution(cMode.m_Width, cMode.m_Height, 32);
			}
		}
	}

	// All done...

	pD3D->Release();

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderer::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CRenderer::Term()
{
	// Terminate all resolution objects...

	POSITION pos = m_collDisplays.GetHeadPosition();

	while (pos)
	{
		CDisplay* pDisplay = (CDisplay*)m_collDisplays.GetNext(pos);
		ASSERT(pDisplay);
		if (pDisplay) delete pDisplay;
	}

	m_collDisplays.RemoveAll();


	// Clear all members...

	Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderer::GetListText
//
//	PURPOSE:	Formats a text string to be used for display purposes
//
// ----------------------------------------------------------------------- //

CString CRenderer::GetListText()
{
	return DX8_RENDERER_NAME;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderer::GetSelectedListDisplay
//
//	PURPOSE:	Gets the currently selected display from the given list
//				box.
//
// ----------------------------------------------------------------------- //

CDisplay* CRenderer::GetSelectedListDisplay(HWND hList)
{
	// Sanity checks...

	ASSERT(IsValid());
	if (!hList) return(NULL);


	// Get the current selection index...

	int index = SendMessage(hList, LB_GETCURSEL, 0, 0);
	if (index == LB_ERR) return(NULL);


	// Get the selected resolution from the index...

	LRESULT lResult = SendMessage(hList, LB_GETITEMDATA, index, 0);
	if (lResult == LB_ERR) return(NULL);

	CDisplay* pDisplay = (CDisplay*)lResult;
	if (!pDisplay) return(NULL);


	// All done...

	return(pDisplay);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderer::AddDisplay
//
//	PURPOSE:	Adds a new display object
//
// ----------------------------------------------------------------------- //

CDisplay* CRenderer::AddDisplay(const char* sName, const char* sDesc, BOOL bHardware)
{
	// Sanity checks...

	if (!sName || !sName[0]) return(NULL);
	if (!sDesc || !sDesc[0]) return(NULL);


	// Create and init a new display object...

	CDisplay* pDisplay = new CDisplay();

	if (!pDisplay->Init(sName, sDesc, bHardware))
	{
		delete pDisplay;
		return(NULL);
	}


	// Add the new object to our list...

	m_collDisplays.AddTail(pDisplay);


	// All done...

	return(pDisplay);
}

CDisplay* CRenderer::AddDisplay(RMode* pMode)
{
	// Sanity checks...

	if (!pMode) return(NULL);


	// Let the main function do the real work...

	return(AddDisplay(pMode->m_InternalName, pMode->m_Description, pMode->m_bHWTnL));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderer::FindDisplay
//
//	PURPOSE:	Searches for a display with the given dimensions
//
// ----------------------------------------------------------------------- //

CDisplay* CRenderer::FindDisplay(const char* sName)
{
	// Sanity checks...

	ASSERT(IsValid());


	// Search for a display object with the given dimensions...

	POSITION pos = m_collDisplays.GetHeadPosition();

	while (pos)
	{
		CDisplay* pDisplay = (CDisplay*)m_collDisplays.GetNext(pos);
		ASSERT(pDisplay);

		if (strcmp(pDisplay->GetName(), sName) == 0)
		{
			return(pDisplay);
		}
	}


	// If we get here, we didn't find a match...

	return(NULL);
}

CDisplay* CRenderer::FindDisplay(RMode* pMode)
{
	// Sanity checks...

	if (!pMode) return(NULL);


	// Let the main function do the real work...

	return(FindDisplay(pMode->m_InternalName));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderer::SetCurrentDisplay
//
//	PURPOSE:	Sets the current display for the display
//
// ----------------------------------------------------------------------- //

BOOL CRenderer::SetCurrentDisplay(const char* sName)
{
	// Sanity checks...

	ASSERT(IsValid());


	// Find the requested display...

	CDisplay* pDisplay = FindDisplay(sName);
	if (!pDisplay) return(FALSE);


	// Set the current display...

	m_pCurDisplay = pDisplay;


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderer::SelectCurrentListDisplay
//
//	PURPOSE:	Select the current display in the given list box
//
// ----------------------------------------------------------------------- //

BOOL CRenderer::SelectCurrentListDisplay(HWND hList)
{
	// Sanity checks...

	ASSERT(IsValid());
	if (!hList) return(FALSE);
	if (!m_pCurDisplay) return(FALSE);


	// Search for this item in the list box...

	int nCount = SendMessage(hList, LB_GETCOUNT, 0, 0);
	if (nCount <= 0) return(FALSE);

	for (int i = 0; i < nCount; i++)
	{
		LRESULT lRet = SendMessage(hList, LB_GETITEMDATA, i, 0);
		if (lRet != LB_ERR)
		{
			CDisplay* pDisplay = (CDisplay*)lRet;

			if (pDisplay == m_pCurDisplay)
			{
				SendMessage(hList, LB_SETCURSEL, i, 0);
				return(TRUE);
			}
		}
	}


	// If we get here, we didn't fine this display in the list box...

	return(FALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderer::FillListBox
//
//	PURPOSE:	Fills the given list box with the current displays
//
// ----------------------------------------------------------------------- //

BOOL CRenderer::FillListBox(HWND hList)
{
	// Sanity checks...

	if (!hList) return(FALSE);


	// Clear the contents of the list box...

	SendMessage(hList, LB_RESETCONTENT, 0, 0);


	// Insert each display...

	POSITION pos = m_collDisplays.GetHeadPosition();

	while (pos)
	{
		CDisplay* pDisplay = (CDisplay*)m_collDisplays.GetNext(pos);
		ASSERT(pDisplay);

		int iItem = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)pDisplay->GetListText());

		if (iItem != LB_ERR)
		{
			SendMessage(hList, LB_SETITEMDATA, iItem, (LPARAM)pDisplay);
		}
	}


	// Set the current selection if requested...

	if (!SelectCurrentListDisplay(hList))
	{
		int nCount = SendMessage(hList, LB_GETCOUNT, 0, 0);

		if (nCount > 0)
		{
			int nRet = SendMessage(hList, LB_SETCURSEL, nCount-1, 0);

			if (nRet == LB_ERR)
			{
				SendMessage(hList, LB_SETCURSEL, 0, 0);
			}
		}
	}


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplayMgr::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CDisplayMgr::Init(const char* sDir, const char* sConfigFile)
{
	// Set simple members...

	m_sDir = sDir;


	// Enumerate the display renderers...

	CWaitCursor wc;

	int count = EnumerateRenderers(sDir);
	if (count < 0) return(FALSE);


	// Set default parameters...

	SetDefaultParameters();


	// Read the config file...

	ReadConfigFile(sConfigFile);


	// All done...

	return(TRUE);
}
		

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplayMgr::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CDisplayMgr::Term()
{
	// Terminate all display objects...

	POSITION pos = m_collRenderers.GetHeadPosition();

	while (pos)
	{
		CRenderer* pRenderer = (CRenderer*)m_collRenderers.GetNext(pos);
		ASSERT(pRenderer);
		if (pRenderer) delete pRenderer;
	}

	m_collRenderers.RemoveAll();


	// Clear all members...

	Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplayMgr::AddRenderer
//
//	PURPOSE:	Adds a new Renderer object
//
// ----------------------------------------------------------------------- //

CRenderer* CDisplayMgr::AddRenderer(const char* sFile)
{
	// Sanity checks...

	ASSERT(IsValid());
	if (!sFile) return(NULL);


	// Create and init a new Renderer object...

	CRenderer* pRenderer = new CRenderer();

	if (!pRenderer->Init(sFile))
	{
		delete pRenderer;
		return(NULL);
	}


	// Add the new object to our list...

	m_collRenderers.AddTail(pRenderer);


	// All done...

	return(pRenderer);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplayMgr::FindRenderer
//
//	PURPOSE:	Finds a renderer object with the given filename
//
// ----------------------------------------------------------------------- //

CRenderer* CDisplayMgr::FindRenderer(const char* sFile)
{
	// Sanity checks...

	ASSERT(IsValid());
	if (!sFile) return(NULL);


	// Search all display objects...

	POSITION pos = m_collRenderers.GetHeadPosition();

	while (pos)
	{
		CRenderer* pRenderer = (CRenderer*)m_collRenderers.GetNext(pos);
		ASSERT(pRenderer);
		if (pRenderer)
		{
			return(pRenderer);
		}
	}


	// If we get here, we didn't find a match...

	return(NULL);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplayMgr::FillListBox
//
//	PURPOSE:	Fills the given list box with the current displays
//
// ----------------------------------------------------------------------- //

BOOL CDisplayMgr::FillListBox(HWND hList)
{
	// Sanity checks...

	if (!hList) return(FALSE);


	// Clear the contents of the list box...

	SendMessage(hList, LB_RESETCONTENT, 0, 0);


	// Insert each display...

	POSITION pos = m_collRenderers.GetHeadPosition();

	while (pos)
	{
		CRenderer* pRenderer = (CRenderer*)m_collRenderers.GetNext(pos);
		ASSERT(pRenderer);

		int iItem = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)pRenderer->GetListText());

		if (iItem != LB_ERR)
		{
			SendMessage(hList, LB_SETITEMDATA, iItem, (LPARAM)pRenderer);
		}
	}


	// Set the selection if requested...

	if (!SelectCurrentListRenderer(hList))
	{
		SendMessage(hList, LB_SETCURSEL, 0, 0);
	}


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplayMgr::SelectCurrentListRenderer
//
//	PURPOSE:	Select the current Renderer in the given list box
//
// ----------------------------------------------------------------------- //

BOOL CDisplayMgr::SelectCurrentListRenderer(HWND hList)
{
	// Sanity checks...

	ASSERT(IsValid());
	if (!hList) return(FALSE);
	if (!m_pCurRenderer) return(FALSE);


	// Search for this item in the list box...

	int nCount = SendMessage(hList, LB_GETCOUNT, 0, 0);
	if (nCount <= 0) return(FALSE);

	for (int i = 0; i < nCount; i++)
	{
		LRESULT lRet = SendMessage(hList, LB_GETITEMDATA, i, 0);
		if (lRet != LB_ERR)
		{
			CRenderer* pRenderer = (CRenderer*)lRet;

			if (pRenderer == m_pCurRenderer)
			{
				SendMessage(hList, LB_SETCURSEL, i, 0);
				return(TRUE);
			}
		}
	}


	// If we get here, we didn't fine this Renderer in the list box...

	return(FALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplayMgr::GetSelectedListRenderer
//
//	PURPOSE:	Gets the currently selected renderer from the given list
//				box.
//
// ----------------------------------------------------------------------- //

CRenderer* CDisplayMgr::GetSelectedListRenderer(HWND hList)
{
	// Sanity checks...

	ASSERT(IsValid());
	if (!hList) return(NULL);


	// Get the current selection index...

	int index = SendMessage(hList, LB_GETCURSEL, 0, 0);
	if (index == LB_ERR) return(NULL);


	// Get the selected Renderer from the index...

	LRESULT lResult = SendMessage(hList, LB_GETITEMDATA, index, 0);
	if (lResult == LB_ERR) return(NULL);

	CRenderer* pRenderer = (CRenderer*)lResult;
	if (!pRenderer) return(NULL);


	// All done...

	return(pRenderer);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplayMgr::EnumerateRenderers
//
//	PURPOSE:	Enumerates all renderers in the given directory
//
// ----------------------------------------------------------------------- //

int CDisplayMgr::EnumerateRenderers(const char* sDir)
{
	// Sanity checks...

	ASSERT(IsValid());


	// Enumerate each .REN file in the given directory...

	char		sPath[256];
	_finddata_t fd;

	if (sDir && strlen(sDir) > 0)
	{
		int len = strlen(sDir);
		if (sDir[len-1] != '\\')
		{
			sprintf(sPath, "%s\\lithtech.exe", sDir);
		}
		else
		{
			sprintf(sPath, "%slithtech.exe", sDir);
		}
	}
	else
	{
		strcpy(sPath, "lithtech.exe");
	}

	int  count     = 0;
	int  handle    = _findfirst(sPath, &fd);
	BOOL bContinue = (handle != -1);

	while (bContinue)
	{
		CRenderer* pRenderer = AddRenderer(fd.name);

		if (pRenderer)
		{
			count++;
		}

		if (_findnext(handle, &fd) == -1) bContinue = FALSE;
	}


	// All done...

	return(count);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplayMgr::EnumerateRenderers
//
//	PURPOSE:	Enumerates all renderers in the given directory
//
// ----------------------------------------------------------------------- //

BOOL CDisplayMgr::ReadConfigFile(const char* sFile)
{
	// Sanity checks...

	ASSERT(IsValid());


	// Verify that the file exists...

	if (!sFile) return(FALSE);

	OFSTRUCT ofs;
	HFILE hFile = OpenFile (sFile, &ofs, OF_EXIST);
	if (hFile == HFILE_ERROR) return(FALSE);


	// Open the file...

	char strBuff[512];
	FILE* pFile = fopen(sFile, "r");
	if (!pFile) return(FALSE);


	// Declare some local variables we'll need...

	BOOL bFoundCardDesc = FALSE;
	BOOL bFoundWidth    = FALSE;
	BOOL bFoundHeight   = FALSE;
	BOOL bFoundDepth    = FALSE;

	char sCardDesc[128];
	char sWidth[64];
	char sHeight[64];
	char sDepth[64];

	strcpy(sWidth, "640");
	strcpy(sHeight, "480");
	strcpy(sDepth, "16");


	// Scan for the parameters we are looking for...

	while (fgets(strBuff, 512, pFile))
	{
		char* ptr = strBuff;
		if (*ptr == '"') ptr++;

		if (strnicmp(ptr, "CardDesc", 8) == 0)
		{
			ptr += 8;
			while (*ptr == '"' || *ptr == ' ') ptr++;
			strcpy(sCardDesc, ptr);

			ptr = &sCardDesc[strlen(sCardDesc) - 1];
			while ((*ptr == '"' || *ptr == '\n') && ptr > sCardDesc) ptr--;
			*(ptr + 1) = '\0';

			bFoundCardDesc = TRUE;
		}
		else if (strnicmp(ptr, "ScreenWidth", 11) == 0)
		{
			ptr += 11;
			while (*ptr == '"' || *ptr == ' ') ptr++;
			strcpy(sWidth, ptr);

			ptr = &sWidth[strlen(sWidth) - 1];
			while ((*ptr == '"' || *ptr == '\n') && ptr > sWidth) ptr--;
			*(ptr + 1) = '\0';

			bFoundWidth = TRUE;
		}
		else if (strnicmp(ptr, "ScreenHeight", 12) == 0)
		{
			ptr += 12;
			while (*ptr == '"' || *ptr == ' ') ptr++;
			strcpy (sHeight, ptr);

			ptr = &sHeight[strlen(sHeight) - 1];
			while ((*ptr == '"' || *ptr == '\n') && ptr > sHeight) ptr--;
			*(ptr + 1) = '\0';

			bFoundHeight = TRUE;
		}
		else if (strnicmp(ptr, "BitDepth", 8) == 0)
		{
			ptr += 8;
			while (*ptr == '"' || *ptr == ' ') ptr++;
			strcpy (sDepth, ptr);

			ptr = &sDepth[strlen(sDepth) - 1];
			while ((*ptr == '"' || *ptr == '\n') && ptr > sDepth) ptr--;
			*(ptr + 1) = '\0';

			bFoundDepth = TRUE;
		}

		if (bFoundCardDesc && bFoundWidth && bFoundHeight && bFoundDepth) break;
	}


	// Set the parameters that we found...

	m_pCurRenderer = FindRenderer(DX8_RENDERER_NAME);

	if (m_pCurRenderer)
	{
		CDisplay* pDisplay = LTNULL;

		if (bFoundCardDesc)
		{
			m_pCurRenderer->SetCurrentDisplay(sCardDesc);
			pDisplay = m_pCurRenderer->GetCurrentDisplay();
		}

		if (!pDisplay)
			pDisplay = m_pCurRenderer->GetFirstDisplay();

		if (pDisplay)
		{
			pDisplay->SetCurrentResolution(atoi(sWidth), atoi(sHeight), atoi(sDepth));
		}
	}


	// Close the file...

	fclose(pFile);


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplayMgr::UpdateListSelections
//
//	PURPOSE:	Updates all list selections
//
// ----------------------------------------------------------------------- //

BOOL CDisplayMgr::UpdateListSelections(HWND hRenList, HWND hDisList, HWND hResList)
{
	// Sanity checks...

	ASSERT(IsValid());
	if (!hRenList) return(FALSE);
	if (!hDisList) return(FALSE);
	if (!hResList) return(FALSE);


	// Update the selected renderer...

	BOOL bForce = FALSE;

	CRenderer* pOrgRen  = GetCurrentRenderer();
	CRenderer* pListRen = GetSelectedListRenderer(hRenList);

	if (pOrgRen != pListRen)
	{
		SetCurrentRenderer(pListRen);
		if (!SelectCurrentListRenderer(hRenList)) return(FALSE);
		if (pListRen) pListRen->FillListBox(hDisList);
		bForce = TRUE;
	}

	CRenderer* pCurRen = pListRen;
	if (!pCurRen) return(FALSE);


	// Update the selected display...

	CDisplay* pOrgDis  = pCurRen->GetCurrentDisplay();
	CDisplay* pListDis = pCurRen->GetSelectedListDisplay(hDisList);

	if (pOrgDis != pListDis || bForce)
	{
		pCurRen->SetCurrentDisplay(pListDis);
		if (!pCurRen->SelectCurrentListDisplay(hDisList)) return(FALSE);
		if (pListDis) pListDis->FillListBox(hResList);
		bForce = TRUE;
	}

	CDisplay* pCurDis = pListDis;
	if (!pCurDis) return(FALSE);


	// Update the selected resolution...

	CResolution* pOrgRes  = pCurDis->GetCurrentResolution();
	CResolution* pListRes = pCurDis->GetSelectedListResolution(hResList);

	if (pOrgRes != pListRes || bForce)
	{
		pCurDis->SetCurrentResolution(pListRes);
	}


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplayMgr::FillListBoxes
//
//	PURPOSE:	Fills all list boxes with the renderer, display, and
//				resolutions.
//
// ----------------------------------------------------------------------- //

BOOL CDisplayMgr::FillListBoxes(HWND hRenList, HWND hDisList, HWND hResList)
{
	// Sanity checks...

	ASSERT(IsValid());
	if (!hRenList) return(FALSE);
	if (!hDisList) return(FALSE);
	if (!hResList) return(FALSE);


	// Fill the renderer list...

	FillListBox(hRenList);

	CRenderer* pRenderer = GetSelectedListRenderer(hRenList);
	if (!pRenderer) return(FALSE);


	// Fill the display list...

	pRenderer->FillListBox(hDisList);

	CDisplay* pDisplay = pRenderer->GetSelectedListDisplay(hDisList);
	if (!pDisplay) return(FALSE);


	// Fill the resolution list...

	pDisplay->FillListBox(hResList);


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplayMgr::GetDisplaySettingsString
//
//	PURPOSE:	Builds a display settings string for use on the command
//				line.
//
// ----------------------------------------------------------------------- //

CString CDisplayMgr::GetDisplaySettingsString()
{
	// Sanity checks...

	if (!m_pCurRenderer) return("");


	// Get the display name...

	CDisplay* pDisplay = m_pCurRenderer->GetCurrentDisplay();
	if (!pDisplay) return("");

	CString sRenderer = pDisplay->GetDescription();
	if (sRenderer.IsEmpty()) return("");

	CString sCardDesc = pDisplay->GetName();


	// Build the command-line info without the resolution...

	CString sCmdLine;

	sCmdLine += " ++CardDesc ";
	sCmdLine += sCardDesc;

	sCmdLine += " ++Renderer ";
	sCmdLine += "\"";
	sCmdLine += sRenderer;
	sCmdLine += "\"";


	// Add the resolution settings if available..

	CResolution* pRes = pDisplay->GetCurrentResolution();

	if (pRes)
	{
		CString sRes;
		sRes.Format(" +ScreenWidth %i +ScreenHeight %i +BitDepth %i", pRes->GetWidth(), pRes->GetHeight(), pRes->GetDepth());

		sCmdLine += sRes;
	}


	// All done...

	return(sCmdLine);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisplayMgr::SetDefaultParameters
//
//	PURPOSE:	Sets default parameters for everything
//
// ----------------------------------------------------------------------- //

void CDisplayMgr::SetDefaultParameters()
{
	// Loop over each renderer...

	CRenderer* pRen = GetFirstRenderer();

	while (pRen)
	{
		// Loop over each display...

		CDisplay* pDis = pRen->GetFirstDisplay();

		while (pDis)
		{
			// If the display does not have a current resolution, set one...

			if (!pDis->GetCurrentResolution())
			{
				// Set the default resolution based on hardware or software...

				CResolution* pRes = NULL;

				if (pDis->IsSoftware())
				{
					pRes = pDis->FindResolution(320, 200, 16);
					if (!pRes) pRes = pDis->FindResolution(320, 240, 16);
					if (!pRes) pRes = pDis->FindResolution(512, 384, 16);
					if (!pRes) pRes = pDis->FindResolution(640, 400, 16);
					if (!pRes) pRes = pDis->FindResolution(640, 480, 16);
				}
				else
				{
					pRes = pDis->FindResolution(640, 480, 16);
					if (!pRes) pRes = pDis->FindResolution(512, 384, 16);
					if (!pRes) pRes = pDis->FindResolution(800, 600, 16);
				}

				pDis->SetCurrentResolution(pRes);
			}


			// Get the next display...

			pDis = pRen->GetNextDisplay();
		}


		// Get the next renderer...

		pRen = GetNextRenderer();
	}
}



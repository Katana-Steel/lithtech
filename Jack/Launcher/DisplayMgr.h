/****************************************************************************
;
;	 MODULE:		DISPLAYMGR (.H)
;
;	PURPOSE:		Display manager class (for renderes)
;
;	HISTORY:		06/15/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _DISPLAYMGR_H_
#define _DISPLAYMGR_H_


// Includes...

#include "LTBaseDefs.h"

// Typedefs...

typedef RMode*	(*LTF_GETSUPPORTEDMODES)();
typedef void	(*LTF_FREEMODELIST)(RMode *pHead);


// Classes...

class CResolution
{
	// Member functions...

public:
	CResolution() { Clear(); }
	~CResolution() { Term(); }

	BOOL					Init(int nWidth, int nHeight, int nDepth);
	void					Term();
	void					Clear();

	BOOL					IsValid() { return(m_nWidth > 0 && m_nHeight > 0); }

	CString					GetName() { return(m_sName); }
	int						GetWidth() { return(m_nWidth); }
	int						GetHeight() { return(m_nHeight); }
	int						GetDepth() { return(m_nDepth); }
	CString					GetListText();


	// Member variables...

private:
	int						m_nWidth;
	int						m_nHeight;
	int						m_nDepth;
	CString					m_sName;
};

class CDisplay
{
	// Member functions...

public:
	CDisplay() { Clear(); }
	~CDisplay() { Term(); }

	BOOL					Init(const char* sName, const char* sDesc, BOOL bHardware);
	void					Term();
	void					Clear();

	BOOL					IsValid() { return(!m_sName.IsEmpty()); }

	CString					GetName() { return(m_sName); }
	CString					GetDescription() { return(m_sDesc); }
	CString					GetListText();
	CResolution*			GetSelectedListResolution(HWND hList);
	CResolution*			GetCurrentResolution() { return(m_pCurResolution); }
	CResolution*			GetFirstResolution();
	CResolution*			GetNextResolution();

	BOOL					SetCurrentResolution(int nWidth, int nHeight, int nDepth);
	void					SetCurrentResolution(CResolution* pRes) { m_pCurResolution = pRes; }

	CResolution*			AddResolution(int nWidth, int nHeight, int nDepth);
	CResolution*			FindResolution(int nWidth, int nHeight, int nDepth);

	BOOL					FillListBox(HWND hList);
	BOOL					SelectCurrentListResolution(HWND hList);

	BOOL					IsSoftware() { return(!IsHardware()); }
	BOOL					IsHardware() { return(m_bHardware); }


	// Member variables...

private:
	CString					m_sName;
	CString					m_sDesc;
	BOOL					m_bHardware;
	CPtrList				m_collResolutions;
	CResolution*			m_pCurResolution;
	POSITION				m_pos;
};

class CRenderer
{
	// Member functions...

public:
	CRenderer() { Clear(); }
	~CRenderer() { Term(); }

	BOOL					Init(const char* sFile);
	void					Term();
	void					Clear();

	BOOL					IsValid() { return true; }

	CString					GetListText();
	CDisplay*				GetCurrentDisplay() { return(m_pCurDisplay); }
	CDisplay*				GetSelectedListDisplay(HWND hList);
	CDisplay*				GetFirstDisplay();
	CDisplay*				GetNextDisplay();

	BOOL					SetCurrentDisplay(const char* sName);
	void					SetCurrentDisplay(CDisplay* pDisplay) { m_pCurDisplay = pDisplay; }

	CDisplay*				AddDisplay(const char* sName, const char* sDesc, BOOL bHardware);
	CDisplay*				AddDisplay(RMode* pMode);
	CDisplay*				FindDisplay(const char* sName);
	CDisplay*				FindDisplay(RMode* pMode);

	BOOL					FillListBox(HWND hList);
	BOOL					SelectCurrentListDisplay(HWND hList);


	// Member variables...

private:
	CPtrList				m_collDisplays;
	CDisplay*				m_pCurDisplay;
	POSITION				m_pos;
};
	
class CDisplayMgr
{
	// Member functions...

public:
	CDisplayMgr() { Clear(); }
	~CDisplayMgr() { Term(); }

	BOOL					Init(const char* sDir, const char* sConfigFile);
	void					Term();
	void					Clear();

	BOOL					IsValid() { return(TRUE); }

	CRenderer*				GetSelectedListRenderer(HWND hList);
	CRenderer*				GetCurrentRenderer() { return(m_pCurRenderer); }
	CRenderer*				GetFirstRenderer();
	CRenderer*				GetNextRenderer();
	CString					GetDisplaySettingsString();
	int						GetNumRenderers() { return(m_collRenderers.GetCount()); }

	BOOL					SetCurrentRenderer(const char* sFile);
	void					SetCurrentRenderer(CRenderer* pRen) { m_pCurRenderer = pRen; }
	void					SetDefaultParameters();

	CRenderer*				AddRenderer(const char* sFile);
	CRenderer*				FindRenderer(const char* sFile);

	BOOL					FillListBox(HWND hList);
	BOOL					FillListBoxes(HWND hRenList, HWND hDisList, HWND hResList);
	BOOL					SelectCurrentListRenderer(HWND hList);
	BOOL					UpdateListSelections(HWND hRenList, HWND hDisList, HWND hResList);

private:
	int						EnumerateRenderers(const char* sDir);
	BOOL					ReadConfigFile(const char* sFile);


	// Member variables...

private:
	CString					m_sDir;
	CPtrList				m_collRenderers;
	CRenderer*				m_pCurRenderer;
	POSITION				m_pos;
};


// Inlines...

inline void CResolution::Clear()
{
	m_nWidth  = 0;
	m_nHeight = 0;
}

inline void CResolution::Term()
{
	Clear();
}

inline void CRenderer::Clear()
{
	m_pCurDisplay = NULL;
}

inline void CDisplay::Clear()
{
	m_sName.Empty();
	m_bHardware = FALSE;
	m_pCurResolution = NULL;
}

inline void CDisplayMgr::Clear()
{
	m_pCurRenderer = NULL;
}

inline CRenderer* CDisplayMgr::GetFirstRenderer()
{
	m_pos = m_collRenderers.GetHeadPosition();
	if (!m_pos) return(NULL);
	return((CRenderer*)m_collRenderers.GetNext(m_pos));
}

inline CRenderer* CDisplayMgr::GetNextRenderer()
{
	if (!m_pos) return(NULL);
	return((CRenderer*)m_collRenderers.GetNext(m_pos));
}

inline CDisplay* CRenderer::GetFirstDisplay()
{
	m_pos = m_collDisplays.GetHeadPosition();
	if (!m_pos) return(NULL);
	return((CDisplay*)m_collDisplays.GetNext(m_pos));
}

inline CDisplay* CRenderer::GetNextDisplay()
{
	if (!m_pos) return(NULL);
	return((CDisplay*)m_collDisplays.GetNext(m_pos));
}

inline CResolution* CDisplay::GetFirstResolution()
{
	m_pos = m_collResolutions.GetHeadPosition();
	if (!m_pos) return(NULL);
	return((CResolution*)m_collResolutions.GetNext(m_pos));
}

inline CResolution* CDisplay::GetNextResolution()
{
	if (!m_pos) return(NULL);
	return((CResolution*)m_collResolutions.GetNext(m_pos));
}


// EOF...

#endif


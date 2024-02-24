/****************************************************************************
;
;	 MODULE:		CLTAlphaWnd (.h)
;
;	PURPOSE:		Class for a window with translucency
;
;	HISTORY:		12/10/98 [kml] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/
#ifndef _LTALPHAWND_H_
#define _LTALPHAWND_H_

// Includes
#include "LTWnd.h"

#define NO_POLY_DRAW
#ifdef NO_POLY_DRAW

class CLTAlphaWnd : public CLTWnd
{
public:
	virtual void Term() {}

	virtual BOOL Init(int nControlID, char* szWndName, CLTWnd* pParentWnd, int xPos, int yPos, 
						int nWidth, int nHeight, int nAlpha = 255, int rTop = 0, int gTop = 0, 
						int bTop = 0, int rBottom = 0, int gBottom = 0, int bBottom = 0, 
						DWORD dwFlags = LTWF_NORMAL, DWORD dwState = LTWS_NORMAL) { return FALSE; }

	void	SetColor(int rTop, int gTop, int bTop, int rBottom, int gBottom, int bBottom) {}
	virtual BOOL DrawToSurface(HSURFACE hSurfDest) { return FALSE; }
};

#else

// Classes
class CLTAlphaWnd : public CLTWnd
{
public:

	CLTAlphaWnd();
	virtual ~CLTAlphaWnd() { Term(); }
	virtual void Term();

	virtual BOOL Init(int nControlID, char* szWndName, CLTWnd* pParentWnd, int xPos, int yPos, 
						int nWidth, int nHeight, int nAlpha = 255, int rTop = 0, int gTop = 0, 
						int bTop = 0, int rBottom = 0, int gBottom = 0, int bBottom = 0, 
						DWORD dwFlags = LTWF_NORMAL, DWORD dwState = LTWS_NORMAL);
	void	SetColor(int rTop, int gTop, int bTop, int rBottom, int gBottom, int bBottom);
	virtual BOOL DrawToSurface(HSURFACE hSurfDest);

protected:
	DVector m_vVerts[4];
	DETri m_Tris[2];
};

// Inlines
inline CLTAlphaWnd::CLTAlphaWnd()
{
	memset(m_vVerts,0,4*sizeof(DVector));
	memset(m_Tris,0,2*sizeof(DETri));
}

inline void CLTAlphaWnd::SetColor(int rTop, int gTop, int bTop, int rBottom, int gBottom, int bBottom)
{
	m_Tris[0].m_Indices[0].r = rTop;
	m_Tris[0].m_Indices[0].g = gTop;
	m_Tris[0].m_Indices[0].b = bTop;
	m_Tris[0].m_Indices[1].r = rTop;
	m_Tris[0].m_Indices[1].g = gTop;
	m_Tris[0].m_Indices[1].b = bTop;
	m_Tris[0].m_Indices[2].r = rBottom;
	m_Tris[0].m_Indices[2].g = gBottom;
	m_Tris[0].m_Indices[2].b = bBottom;
	m_Tris[1].m_Indices[0].r = rTop;
	m_Tris[1].m_Indices[0].g = gTop;
	m_Tris[1].m_Indices[0].b = bTop;
	m_Tris[1].m_Indices[1].r = rBottom;
	m_Tris[1].m_Indices[1].g = gBottom;
	m_Tris[1].m_Indices[1].b = bBottom;
	m_Tris[1].m_Indices[2].r = rBottom;
	m_Tris[1].m_Indices[2].g = gBottom;
	m_Tris[1].m_Indices[2].b = bBottom;
}

#endif  // NO_POLY_DRAW

#endif
// InterfaceSurfMgr.cpp: implementation of the CInterfaceSurfMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "basedefs_de.h"
#include "cpp_clientshell_de.h"
#include "InterfaceSurfMgr.h"
#include <mbstring.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInterfaceSurfMgr::CInterfaceSurfMgr()
{
	m_pClientDE=DNULL;
}

CInterfaceSurfMgr::~CInterfaceSurfMgr()
{
	Term();
}

// Intialization
DBOOL CInterfaceSurfMgr::Init(CClientDE *pClientDE)
{
	m_pClientDE=pClientDE;

	if (m_pClientDE == DNULL)
	{
		return DFALSE;
	}

	return DTRUE;
}

// Termination
void CInterfaceSurfMgr::Term()
{
	if (!m_pClientDE)
	{
		return;
	}
	FreeAllSurfaces();
	return;
}


void CInterfaceSurfMgr::FreeAllSurfaces()
{
	unsigned int i;
	for (i=0; i < m_sharedSurfaceArray.GetSize(); i++)
	{
		m_pClientDE->DeleteSurface(m_sharedSurfaceArray[i]->m_hSurface);
		delete m_sharedSurfaceArray[i];
	}
	m_sharedSurfaceArray.SetSize(0);
}

// Returns a handle to a surface from its bitmap filename. 
HSURFACE CInterfaceSurfMgr::GetSurface(char *lpszSurface)
{
	if (!lpszSurface)
	{
		return DNULL;
	}

	// Search for the surface to see if it has been loaded yet
	int nIndex=FindSurfaceIndex(lpszSurface);

	// Return the surface if it is already loaded
	if (nIndex != -1)
	{
		return m_sharedSurfaceArray[nIndex]->m_hSurface;
	}

	// Load the surface
	HSURFACE hSurface=m_pClientDE->CreateSurfaceFromBitmap(lpszSurface);
	if (hSurface == DNULL)
	{
		return DNULL;
	}

	//optimize surface
	g_pClientDE->OptimizeSurface (hSurface, SETRGB_T(0,0,0));

	// Create the new resource class
	CSharedSurface *pSharedSurface=new CSharedSurface;

	// Copy the filename
	int nBufferSize=_mbstrlen(lpszSurface)+1;
	pSharedSurface->m_lpszPathName=new char[nBufferSize];
	_mbsncpy((unsigned char*)pSharedSurface->m_lpszPathName, (const unsigned char*)lpszSurface, nBufferSize);
	
	pSharedSurface->m_hSurface=hSurface;		// Copy the surface handle
	m_sharedSurfaceArray.Add(pSharedSurface);	// Add the class to the array

	// Return the surface handle
	return pSharedSurface->m_hSurface;
}

// Frees a surface.  It is only removed when its reference count reaches zero
void CInterfaceSurfMgr::FreeSurface(char *lpszSurface)
{
	int nIndex=FindSurfaceIndex(lpszSurface);

	if (nIndex == -1)
	{
		return;
	}

	FreeSurface(nIndex);
}

// Frees a surface.  It is only removed when its reference count reaches zero
void CInterfaceSurfMgr::FreeSurface(HSURFACE hSurface)
{
	int nIndex=FindSurfaceIndex(hSurface);

	if (nIndex == -1)
	{
		return;
	}

	FreeSurface(nIndex);
}

// Frees a surface based on its index into the surface array
void CInterfaceSurfMgr::FreeSurface(int nIndex)
{
	if (!m_pClientDE)
	{
		return;
	}

	m_pClientDE->DeleteSurface(m_sharedSurfaceArray[nIndex]->m_hSurface);			

	delete m_sharedSurfaceArray[nIndex];
	m_sharedSurfaceArray.Remove(nIndex);
}

// Finds a surface index into the surface array
int CInterfaceSurfMgr::FindSurfaceIndex(char *lpszSurface)
{
	unsigned int i;
	for (i=0; i < m_sharedSurfaceArray.GetSize(); i++)
	{
		if (_mbsicmp((const unsigned char*)m_sharedSurfaceArray[i]->m_lpszPathName, (const unsigned char*)lpszSurface) == 0)
		{
			return i;
		}
	}

	// Surface was not found
	return -1;
}

// Finds a surface index into the surface array
int CInterfaceSurfMgr::FindSurfaceIndex(HSURFACE hSurface)
{
	unsigned int i;
	for (i=0; i < m_sharedSurfaceArray.GetSize(); i++)
	{
		if (m_sharedSurfaceArray[i]->m_hSurface == hSurface)
		{
			return i;
		}
	}

	// Surface was not found
	return -1;
}

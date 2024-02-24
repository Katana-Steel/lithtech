// PlaySound.h : header file
//
/*******************************************************************************
;
;	MODULE:			PlaySound (.H)
;
;	PURPOSE:		Utility function to play a sound
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#ifndef _PLAYSOUND_H_
#define _PLAYSOUND_H_

#include "mmsystem.h"
#pragma comment(lib, "winmm.lib")

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlaySound
//
//	PURPOSE:	Helper function to play a sound
//
// ----------------------------------------------------------------------- //
static void PlaySound(LPCTSTR lpszSound)
{
	HRSRC hRes; // resource handle to wave file
	HGLOBAL hData;
	BOOL bOk = FALSE;
	if ((hRes = ::FindResource(AfxGetResourceHandle(), lpszSound,
	  _T("WAVE"))) != NULL &&
	  (hData = ::LoadResource(AfxGetResourceHandle(), hRes)) != NULL)
	{
		// found the resource, play it
		//bOk = sndPlaySound((LPCTSTR)::LockResource(hData),
		//	SND_MEMORY|SND_ASYNC|SND_NODEFAULT);

		bOk = PlaySound((LPCTSTR)::LockResource(hData), NULL, SND_MEMORY | SND_NODEFAULT | SND_ASYNC | SND_NOSTOP  ); 
/*		if( !bOk )
		{
			MessageBox(NULL, "PlaySound Failed!", "FAILED", MB_OK);
		}
*/
		FreeResource(hData);
	}
}

inline static void PlaySound(UINT nIDS)
	{ PlaySound(MAKEINTRESOURCE(nIDS)); }

#endif

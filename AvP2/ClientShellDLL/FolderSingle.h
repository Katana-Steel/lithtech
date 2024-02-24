// FolderSingle.h: interface for the CFolderSingle class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_SINGLE_H_
#define _FOLDER_SINGLE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderSingle : public CBaseFolder
{
public:
	CFolderSingle();
	virtual ~CFolderSingle();

	// Build the folder
    LTBOOL   Build();

    void    OnFocus(LTBOOL bFocus);
	LTBOOL	Render(HSURFACE hDestSurf);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	CLTGUITextItemCtrl* m_pLoad;

	int			m_LastCharacter;

	enum ePrivateConstants
	{
		kMaxModels = 2,
		kMaxModelNames = 6,
	};

	CBaseScaleFX*	m_pModels[kMaxModels];
	char		m_FXNames[kMaxModelNames][128];
};

#endif // _FOLDER_SINGLE_H_
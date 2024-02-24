// FolderCredits.h: interface for the CFolderCredits class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_CREDITS_H_
#define _FOLDER_CREDITS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderCredits : public CBaseFolder
{
public:

	CFolderCredits();
	virtual ~CFolderCredits();

	// Build the folder
	LTBOOL	Build();
	void	OnFocus(LTBOOL bFocus);

			// Renders the folder to a surface
    virtual LTBOOL   Render(HSURFACE hDestSurf);


};

#endif // _FOLDER_CREDITS_H_

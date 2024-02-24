// FolderEscape.h: interface for the CFolderEscape class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_ESC_H_
#define _FOLDER_ESC_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"
#include "MessageBox.h"
#include "BaseScaleFX.h"

class CFolderEscape : public CBaseFolder
{
public:
	CFolderEscape();
	virtual ~CFolderEscape();

	// Build the folder
    LTBOOL   Build();

	// This is called when the folder gets or loses focus
    virtual void    OnFocus(LTBOOL bFocus);

	void	Escape();

	LTBOOL IsAborting() const { return m_bAborting; }

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	CLTGUITextItemCtrl* m_pResume;
	CLTGUITextItemCtrl* m_pLoad;
	CLTGUITextItemCtrl* m_pSave;
	CLTGUITextItemCtrl* m_pOptions;
	CLTGUITextItemCtrl* m_pAbort;

private :

	LTBOOL m_bAborting;
};

#endif // _FOLDER_ESC_H_
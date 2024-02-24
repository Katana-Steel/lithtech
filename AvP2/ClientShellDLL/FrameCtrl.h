// FrameCtrl.h: interface for the CBitmapCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FRAMECTRL_H_
#define _FRAMECTRL_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LTGUIMgr.h"

class CFrameCtrl : public CLTGUICtrl
{
public:
	CFrameCtrl();
	virtual ~CFrameCtrl();
    LTBOOL   Create(ILTClient *pClientDE, HLTCOLOR hColor, LTFLOAT fAlpha, int nWidth, int nHeight, LTBOOL bSolid = LTTRUE);

	virtual void	SetSize(int nWidth, int nHeight) { m_nWidth=nWidth; m_nHeight=nHeight; }
	virtual void    Enable ( LTBOOL bEnabled )       { m_bEnabled=LTFALSE; }
	virtual void	SetSolid(LTBOOL bSolid)			 {m_bSolid = bSolid;}

	// Destruction
	virtual void	Destroy();

	// Render the control
	virtual void	Render ( HSURFACE hDestSurf );

	// Width/Height calculations
	virtual int		GetWidth ( ) {return m_nWidth;}
	virtual int		GetHeight ( ) {return m_nHeight;}


protected:

    HSURFACE        m_hSurf;
	int				m_nWidth;
	int				m_nHeight;
	LTBOOL			m_bSolid;


};

#endif
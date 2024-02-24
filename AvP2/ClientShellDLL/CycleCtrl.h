// CycleCtrl.h: interface for the CCycleCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWCYCLECTRL_H__E06741C2_A969_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_NEWCYCLECTRL_H__E06741C2_A969_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LTGUIMgr.h"
#include "stdlith.h"

class CCycleCtrl : public CLTGUICycleCtrl  
{
public:
	CCycleCtrl();
	virtual ~CCycleCtrl();

	virtual DBOOL OnEnter() {return DFALSE;}
	virtual DBOOL OnLButtonUp(int x, int y) {return OnRight();}

};

class CToggleCtrl : public CLTGUIOnOffCtrl  
{
public:
	CToggleCtrl();
	virtual ~CToggleCtrl();

	virtual DBOOL OnEnter() {return DFALSE;}
	virtual DBOOL OnLButtonUp(int x, int y) {return OnRight();}
};

#endif // !defined(AFX_NEWCYCLECTRL_H__E06741C2_A969_11D3_B2DB_006097097C7B__INCLUDED_)

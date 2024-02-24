// ----------------------------------------------------------------------- //
//
// MODULE  : HUDBombTarget.h
//
// PURPOSE : HUDItem to display status of doomsday pieces
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDBOMBTARGET_H__
#define __HUDBOMBTARGET_H__

#include "HUDItem.h"
#include <vector>
#include "InterfaceTimer.h"

//******************************************************************************************
//** HUD Bombtarget
//******************************************************************************************
class CHUDBombTarget : public CHUDItem
{
public:
	CHUDBombTarget();

    LTBOOL      Init();
	void		Term();
    void        Render();
	void        Update();
    void        UpdateLayout();

	// Gets/Sets time on timer.
	bool		SetTimer( uint8 nTimer, float fTime, bool bPaused );
	float		GetTimer( uint8 nTimer ) const;

private:

    LTIntPt		m_BasePos;
	uint16		m_nSize;

	struct IconData
	{
		IconData( )
		{
			m_hIcon = NULL;
		}

		CInterfaceTimer m_Timer;
		LTPoly_GT4	m_Poly;
		HTEXTURE	m_hIcon;
	};
	typedef std::vector< IconData > TIconDataList;
	TIconDataList	m_lstIconData;

	float m_fScale;
};

#endif // __HUDBOMBTARGET_H__
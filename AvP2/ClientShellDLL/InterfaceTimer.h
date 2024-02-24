// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceTimer.h
//
// PURPOSE : Definition of InterfaceTimer class
//
// CREATED : 10/18/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __INTERFACE_TIMER_H__
#define __INTERFACE_TIMER_H__

#include "basedefs_de.h"


class CInterfaceTimer
{
  public:

	CInterfaceTimer()
	{
		m_fTime		= 0.0f;
	}

	void		Draw(HSURFACE hScreen);
	void		SetTime(DFLOAT fTime) { m_fTime = fTime; }
	DFLOAT		GetTime() const { return m_fTime; }

  private:

	DFLOAT		m_fTime;
};

#endif  // __INTERFACE_TIMER_H__

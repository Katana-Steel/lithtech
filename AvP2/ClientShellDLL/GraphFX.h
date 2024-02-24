// ----------------------------------------------------------------------- //
//
// MODULE  : LineSystemFX.h
//
// PURPOSE : LineSystem special fx class - Definition
//
// CREATED : 2/8/00
//
// ----------------------------------------------------------------------- //

#ifndef __GRAPH_FX_H__
#define __GRAPH_FX_H__

#include "BaseLineSystemFX.h"
#include "Callback.h"


struct GraphFXPoint
{
	DFLOAT y;  // y value
	DFLOAT x;  // x value (ignored if in IgnoreX mode)

	DFLOAT r;  //  red value of point
	DFLOAT g;  //  blue value of point
	DFLOAT b;  //  green value of point
	DFLOAT a;  //  alpha value of point

	GraphFXPoint(DFLOAT y_ = 0.0f,DFLOAT x_ = 0.0f, 
		         DFLOAT r_ = 1.0f, DFLOAT g_ = 1.0f, DFLOAT b_ = 1.0f, DFLOAT a_ = 1.0f)
				 : y(y_), x(x_), r(r_), g(g_), b(b_), a(a_) {}
};


class CGraphFX : public CBaseLineSystemFX
{
	public:

		typedef Callback2<DBOOL,GraphFXPoint * *, DDWORD *> UpdateCallbackFn;

	public :

		CGraphFX();
		~CGraphFX();

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();
		virtual DBOOL CreateObject(CClientDE* pClientDE);

	protected :

		void RemoveLines();
		void UpdateLines();

	private :

		DBOOL  m_bFirstUpdate;				// Is this the first update
		DFLOAT m_fNextUpdate;				// Time between updates
		DFLOAT m_fLastTime;					// When was the last update time
		
		DVector m_vOffset;					// Offset Position
		DFLOAT  m_fWidth;
		DFLOAT  m_fHeight;
		DBOOL	m_bIgnoreX;

		UpdateCallbackFn m_UpdateGraphCallback;  // Function which updates m_pLines. 
												 // Is called as m_UpdateGraphCallback(&m_pLines,&m_nNumLines).
												 // Should return TRUE if graph is be updated, otherwise old graph is left displayed.
		GraphFXPoint * m_pPoints;           // pointer to data
		DDWORD         m_nNumPoints;        // number of data items
};


struct GCREATESTRUCT : public SFXCREATESTRUCT
{
	GCREATESTRUCT()
		: m_vOffsetPos(0.0f,0.0f,0.0f),
		  m_fWidth(0.0f),
		  m_fHeight(0.0f),
		  m_bIgnoreX(DFALSE) {} 
	
	DVector   m_vOffsetPos;
	DFLOAT    m_fWidth;
	DFLOAT    m_fHeight;
	DBOOL	  m_bIgnoreX;

	CGraphFX::UpdateCallbackFn m_UpdateGraphCallback;

};


//-------------------------------------------------------------------------------------------------
// SFX_GraphFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_GraphFactory : public CSpecialFXFactory
{
	SFX_GraphFactory() : CSpecialFXFactory(SFX_GRAPH_ID) {;}
	static const SFX_GraphFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};


#endif // __GRAPH_FX_H__

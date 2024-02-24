// ----------------------------------------------------------------------- //
//
// MODULE  : PolyGrid.h
//
// PURPOSE : PolyGrid - Definition
//
// CREATED : 10/20/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __POLY_GRID_H__
#define __POLY_GRID_H__

#include "ClientSFX.h"

namespace ObjectDLL
{

class PolyGrid : public CClientSFX
{
	public :

		PolyGrid();
		~PolyGrid();
		
		void Setup(DVector* pvDims, DVector* pvColor1, DVector* pvColor2,
				   HSTRING hstrSurfaceSprite, DFLOAT fXScaleMin, DFLOAT fXScaleMax,
				   DFLOAT fYScaleMin, DFLOAT fYScaleMax, DFLOAT fXScaleDuration,
				   DFLOAT fYScaleDuration, DFLOAT fXPan, DFLOAT fYPan, DFLOAT fFrequency,
				   DFLOAT fAlpha, DDWORD dwNumPolies, DBOOL bAdditive, DBOOL bMultiply);

		DVector m_vDims;
		DVector m_vColor1;
		DVector m_vColor2;
		DFLOAT	m_fXScaleMin;
		DFLOAT	m_fXScaleMax;
		DFLOAT	m_fYScaleMin;
		DFLOAT	m_fYScaleMax;
		DFLOAT	m_fXScaleDuration;
		DFLOAT	m_fYScaleDuration;
		DFLOAT	m_fXPan;
		DFLOAT	m_fYPan;
		DFLOAT	m_fFrequency;
		DFLOAT	m_fAlpha;
		DBYTE	m_nPlasmaType;
		DBYTE	m_nRingRate[4];
		DDWORD	m_dwNumPolies;
		DBOOL	m_bAdditive;
		DBOOL	m_bMultiply;

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
	
	private :

		HSTRING m_hstrSurfaceSprite;
		DBOOL	m_bSetup;

		void ReadProp(ObjectCreateStruct *pStruct);
		void Update();

		void CacheFiles();
};

} // end namespace ObjectDLL

using namespace ObjectDLL;

#endif // __POLY_GRID_H__

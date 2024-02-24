// ----------------------------------------------------------------------- //
//
// MODULE  : LightFX.h
//
// PURPOSE : LightFX Inventory Item
//
// CREATED : 02/03/98
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHT_FX_H__
#define __LIGHT_FX_H__

#include "SpecialFX.h"
#include "ClientServerShared.h"
#include "light_anim_lt.h"


struct LIGHTCREATESTRUCT : public SFXCREATESTRUCT
{
	LIGHTCREATESTRUCT::LIGHTCREATESTRUCT();

	DVector		vColor;
	DVector		vOffset;
	DDWORD		dwLightFlags;
	DFLOAT		fIntensityMin;
	DFLOAT		fIntensityMax;
	DBYTE		nIntensityWaveform;
	DFLOAT		fIntensityFreq;
	DFLOAT		fIntensityPhase;
	DFLOAT		fRadiusMin;
	DFLOAT		fRadiusMax;
	DBYTE		nRadiusWaveform;
	DFLOAT		fRadiusFreq;
	DFLOAT		fRadiusPhase;
	uint16		nOnSound;
	uint16		nOffSound;
	HLIGHTANIM	m_hLightAnim;	// INVALID_LIGHT_ANIM if none..
};

inline LIGHTCREATESTRUCT::LIGHTCREATESTRUCT()
{
	vColor.Init();
	vOffset.Init();
	dwLightFlags		= 0;
	fIntensityMin		= 0.0f;
	fIntensityMax		= 0.0f;
	nIntensityWaveform	= 0;
	fIntensityFreq		= 0.0f;
	fIntensityPhase		= 0.0f;
	fRadiusMin			= 0.0f;
	fRadiusMax			= 0.0f;
	nRadiusWaveform		= 0;
	fRadiusFreq			= 0.0f;
	fRadiusPhase		= 0.0f;
	nOnSound			= -1;
	nOffSound			= -1;
	m_hLightAnim		= DNULL;
}


class CLightFX : public CSpecialFX
{
	public :

		CLightFX();
		~CLightFX();

		virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Update();
		virtual LTBOOL CreateObject(CClientDE* pClientDE);
		virtual LTBOOL OnServerMessage(HMESSAGEREAD hRead);

	protected:

        void SetRadius(DFLOAT fRadius, LAInfo &info);
        void SetColor(DFLOAT fRedValue, DFLOAT fGreenValue, DFLOAT fBlueValue, LAInfo &info);
        
        virtual void UpdateLightRadius(LAInfo &info);
        virtual void UpdateLightIntensity(LAInfo &info);
        virtual void PlaySound(uint16 nBute);

	private :
    
		// Member Variables

		DVector m_vColor;				    // First color to use
		DVector	m_vOffset;					// Offset relative to server obj

		DDWORD	m_dwLightFlags;

		DFLOAT  m_fIntensityMin;			// How Dark light gets
		DFLOAT  m_fIntensityMax;			// How Bright light gets
		DBYTE	m_nIntensityWaveform;
		DFLOAT	m_fIntensityFreq;
		DFLOAT	m_fIntensityPhase;

		DFLOAT	m_fLastIntensity;

		DDWORD  m_nNumRadiusCycles;		    // Number of times to cycle through
		DFLOAT  m_fRadiusMin;				// How small light gets
		DFLOAT  m_fRadiusMax;				// How large light gets
		DBYTE	m_nRadiusWaveform;
		DFLOAT	m_fRadiusFreq;
		DFLOAT	m_fRadiusPhase;

		DFLOAT	m_fLastRadius;

		DFLOAT  m_fLifeTime;				// How long should this light stay around

		DVector m_vCurrentColor;			// Color currently using
		DFLOAT	m_fCurrentRadius;		    // Radius currently using

		DFLOAT	m_fIntensityTime;		    // Intensity timer
		DFLOAT	m_fRadiusTime;			    // Radius timer
		DFLOAT	m_fColorTime;			    // Color timer

		int	    m_nCurIntensityState;	    // What intensity state are we in
		int	    m_nCurRadiusState;	        // What radius state are we in
		bool	m_bUpdate;					// Do we need to update radius and intensity
		bool	m_bLastOnState;				// Were we on or off last time.

		uint16	m_nOnSound;
		uint16	m_nOffSound;

		DBOOL	m_bUseServerPos;			// Should we use the server pos?
		DFLOAT	m_fStartTime;			    // When did this light get created
        DFLOAT  m_fRadius;

		HLIGHTANIM	m_hLightAnim;			// INVALID_LIGHT_ANIM if we're not using light animations.

		HLTSOUND	m_hSound;				// The on / off sound
};

//-------------------------------------------------------------------------------------------------
// SFX_LightFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_LightFactory : public CSpecialFXFactory
{
	SFX_LightFactory() : CSpecialFXFactory(SFX_LIGHT_ID) {;}
	static const SFX_LightFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __LIGHT_FX_H__



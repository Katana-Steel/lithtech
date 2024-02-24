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

//#include "LightObject.h"
#include "cpp_engineobjects_de.h"
#include "Destructible.h"
#include "SoundButeMgr.h"
#include "iobjectplugin.h"

struct ClientLightFXCreateStruct
{
	ClientLightFXCreateStruct();

	// Member Variables
	DBOOL   m_bOn;				        // Are we on?

	DVector m_vColor;				    // First color to use
	DDWORD	m_dwLightFlags;

	DFLOAT  m_fIntensityMin;			// How Dark light gets
	DFLOAT  m_fIntensityMax;			// How Bright light gets
	DBYTE	m_nIntensityWaveform;
	DFLOAT	m_fIntensityFreq;
	DFLOAT	m_fIntensityPhase;

	DFLOAT  m_fRadiusMin;				// How small light gets
	DFLOAT  m_fRadiusMax;				// How large light gets
	DBYTE	m_nRadiusWaveform;
	DFLOAT	m_fRadiusFreq;
	DFLOAT	m_fRadiusPhase;

	uint16	m_nOnSound;
	uint16	m_nOffSound;

	DBOOL	m_bUseLightAnims;			// Are we using light animations (precalculated shadows)?

	DBOOL	m_bDynamic;					// Was this object dynamically create
	DFLOAT  m_fLifeTime;				// How long should this light stay around
   
	DFLOAT	m_fStartTime;			    // When did this light get created


	DFLOAT	m_fHitPts;

};

class ClientLightFX : public BaseClass// : public LightObject
{
	public :

		typedef std::list<ClientLightFX*> ClientLightFXList;
		typedef ClientLightFXList::const_iterator ClientLightFXIterator;

		static ClientLightFXIterator BeginClientLightFXs() { return m_sClientLightFXs.begin(); }
		static ClientLightFXIterator EndClientLightFXs()   { return m_sClientLightFXs.end(); }

	public :

 		ClientLightFX();
		virtual ~ClientLightFX();

		void Setup(ClientLightFXCreateStruct createLight);

		// For LightObject
		virtual LTVector GetPos() const;
		virtual LTFLOAT  GetRadiusSqr() const;
		virtual LTVector GetColor(const LTVector & /*unused*/) const;

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
        DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private : 
		void    HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead );
        DBOOL   ReadProp(ObjectCreateStruct *pData);
		void    PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);
		DBOOL   Update();

		void	SendEffectMessage(LTBOOL bForceMessage=LTFALSE);

		void TurnOn();
		void TurnOff();
        void ToggleLight()      { if (m_LightData.m_bOn) TurnOff();  else TurnOn(); }

		DBOOL Init();

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void CacheFiles();

		ClientLightFXCreateStruct m_LightData;

		DFLOAT	m_fIntensityDelta;			// What is the total change so far

		static ClientLightFXList m_sClientLightFXs;
};

#ifdef __LINUX

inline void ClientLightFX::TurnOn()
{
}

inline void ClientLightFX::TurnOff()
{
}

#endif

// Additional light wave classes
class SquareWaveLightFX : public ClientLightFX
{
};


class SawWaveLightFX : public ClientLightFX
{
};


class RampUpWaveLightFX : public ClientLightFX
{
};


class RampDownWaveLightFX : public ClientLightFX
{
};


class SineWaveLightFX : public ClientLightFX
{
};


class Flicker1WaveLightFX : public ClientLightFX
{
};


class Flicker2WaveLightFX : public ClientLightFX
{
};


class Flicker3WaveLightFX : public ClientLightFX
{
};


class Flicker4WaveLightFX : public ClientLightFX
{
};


class StrobeWaveLightFX : public ClientLightFX
{
};


class SearchWaveLightFX : public ClientLightFX
{
};


class FlickerLight : public ClientLightFX
{
};


class GlowingLight : public ClientLightFX
{
};


// The LightFX plugin class to create light animations.
class CLightFXPlugin : public IObjectPlugin
{
	public:

		virtual DRESULT	PreHook_Light(
			PreLightLT *pInterface, 
			HPREOBJECT hObject);

		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength);

	private:

		CSoundButeMgrPlugin	m_SoundButeMgrPlugin;
};

void CLightFXPreprocessorCB(HPREOBJECT hObject, PreLightLT *pInterface);


#endif // __LIGHT_FX_H__



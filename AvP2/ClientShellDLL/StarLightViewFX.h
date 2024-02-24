// ----------------------------------------------------------------------- //
//
// MODULE  : StarLightViewFX.h
//
// PURPOSE : StarLightViewFX Inventory Item
//
// CREATED : 02/03/98
//
// ----------------------------------------------------------------------- //

#ifndef __STARLIGHT_VIEW_FX_H__
#define __STARLIGHT_VIEW_FX_H__

#include "SpecialFX.h"
#include "ClientServerShared.h"
#include "light_anim_lt.h"


struct STARLIGHTCREATESTRUCT : public SFXCREATESTRUCT
{
	STARLIGHTCREATESTRUCT::STARLIGHTCREATESTRUCT();

	std::string	m_ActiveMode;				// What mode are we active on?
	DVector		vColor;
	DVector		m_vFogColor;				// Color fog is using
	DDWORD		m_MinFogNearZ, m_MinFogFarZ;// Minimum Fog near and far
	DDWORD		m_FogNearZ, m_FogFarZ;		// Fog near and far

	DDWORD		dwLightFlags;
	HLIGHTANIM	m_hLightAnim;				// INVALID_LIGHT_ANIM if none..

	DVector		m_vVertexTintColor;			// Old style light scale
	DVector		m_vModelAddColor;			// Model add color

	DVector		m_vAmbientColor;
};

inline STARLIGHTCREATESTRUCT::STARLIGHTCREATESTRUCT() : vColor(0,0,0),
	m_vFogColor(0,0,0), m_MinFogNearZ(0), m_MinFogFarZ(5000), m_FogNearZ(0), m_FogFarZ(5000), 
	dwLightFlags(0), m_hLightAnim(INVALID_LIGHT_ANIM), m_vVertexTintColor(0,0,0),
	m_vModelAddColor(0,0,0)
{
}


class StarLightViewFX : public CSpecialFX
{
	public :

		StarLightViewFX();
		~StarLightViewFX();

		virtual DBOOL	Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual DBOOL	Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL	Update();
		virtual DBOOL	CreateObject(CClientDE* pClientDE);
		virtual void	SetMode(const std::string& mode);
		LTVector		GetVertexTint() { return m_vVertexTintColor; }
		const std::string GetActiveMode() { return m_ActiveMode; } 
		virtual void	ResetEnvMap();

	protected:

        void SetColor(DFLOAT fRedValue, DFLOAT fGreenValue, DFLOAT fBlueValue, LAInfo &info);
        
	private :
    
		// Member Variables

		DVector m_vColor;				    // First color to use
		DVector	m_vOffset;					// Offset relative to server obj

		DDWORD	m_dwLightFlags;

		DFLOAT  m_fLifeTime;				// How long should this light stay around

		DVector m_vCurrentColor;			// Color currently using

		DFLOAT	m_fColorTime;			    // Color timer

		DBOOL	m_bUseServerPos;			// Should we use the server pos?
		DFLOAT	m_fStartTime;			    // When did this light get created

		HLIGHTANIM	m_hLightAnim;			// INVALID_LIGHT_ANIM if we're not using light animations.

		DVector m_vFogColor;				// Color fog is using
		DDWORD	m_MinFogNearZ, m_MinFogFarZ;// Minimum Fog near and far
		DDWORD	m_FogNearZ, m_FogFarZ;		// Fog near and far

		DVector m_vVertexTintColor;
		DVector m_vModelAddColor;
		DVector m_vAmbientColor;

		std::string			m_ActiveMode;	// What mode do we turn on in.

		LTVector			m_OldLight;		// The old value of the global light.

		LTVector			m_OldVertexColor;	// Old tint color for vertices

		static bool			m_GlobalLight;	// Are we all in global light mode?
		static std::string	m_Mode;			// What mode are we in?
		bool				m_ModeOn;		// Is our mode on?

		LTBOOL				m_bEnvWorld;	// Is the world noramlly env mapped?
		LTBOOL				m_bEnvObj;		// Are models noramlly env mapped?
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

class SFX_StarLightViewFactory : public CSpecialFXFactory
{
	SFX_StarLightViewFactory() : CSpecialFXFactory(SFX_STARLIGHT_VISION) {;}
	static const SFX_StarLightViewFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __STARLIGHT_VIEW_FX_H__



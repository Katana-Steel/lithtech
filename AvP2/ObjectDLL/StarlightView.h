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

#include "cpp_engineobjects_de.h"
#include "Destructible.h"
#include "iobjectplugin.h"

#include <string>

class StarLightView : public BaseClass
{
	public :

 		StarLightView();
		~StarLightView();

		void	SendEffectMessage(HCLIENT hClient);

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
        DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private : 
		void    HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead );
        DBOOL   ReadProp(ObjectCreateStruct *pData);
		void    PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);
		DBOOL   Update();


#ifndef __LINUX
		void TurnOn();
		void TurnOff();
		void ToggleLight()      { if (m_bOn) TurnOff();  else TurnOn(); }
#endif

		DBOOL Init();

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void CacheFiles();

		// Member Variables
		DBOOL   m_bOn;				        // Are we on?
		DBOOL	m_bDynamic;					// Was this object dynamically create
		DFLOAT  m_fLifeTime;				// How long should this light stay around
		DFLOAT	m_fStartTime;			    // When did this light get created
		std::string		m_ActiveMode;		// What mode turns us on....oooh baby...
		std::string		m_ObjectName;		// What are we called...
		DVector m_vColor;				    // Ambient color to use
		DDWORD	m_dwLightFlags;
		DVector m_vFogColor;			    // Ambient color to use
		DDWORD	m_MinFogNearZ, m_MinFogFarZ;// Minimum Fog near and far planes
		DDWORD	m_FogNearZ, m_FogFarZ;		// Fog near and far planes
		DVector	m_vVertexTintColor;			// Color to set the Vertices to
		DVector	m_vModelAddColor;			// Color to add to models
		DVector	m_vAmbientColor;			// Color to add to world ambient

		// MEMBERS LISTED BELOW HERE DO NOT NEED LOAD/SAVE

		bool	m_bFirstUpdate;
};


// The LightFX plugin class to create light animations.
class StarLightViewPlugin : public IObjectPlugin
{
public:
	virtual DRESULT	PreHook_Light(
		PreLightLT *pInterface, 
		HPREOBJECT hObject);

};
void StarLightViewPreprocessorCB(HPREOBJECT hObject, PreLightLT *pInterface);



#endif // __LIGHT_FX_H__



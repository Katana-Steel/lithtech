// ----------------------------------------------------------------------- //
//
// MODULE  : LightGroup.h
//
// PURPOSE : LightGroup - Definition
//
// CREATED : 02/22/00
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTGROUP_H__
#define __LIGHTGROUP_H__

//#include "cpp_engineobjects_de.h"
//#include "commonutilities.h"

#ifndef __DESTRUCTIBLE_H__
#include "Destructible.h"
#endif

#ifndef __I_OBJECT_PLUGIN_H__
#include "iobjectplugin.h"
#endif

//#ifndef __LIGHT_OBJECT_H__
//#include "LightObject.h"
//#endif

#pragma warning(disable : 4786)
#include <string>
#include <vector>


#define MAX_LIGHTGROUP_TARGETS	50

class ClientLightDef
{
	public:

		ClientLightDef() : m_InnerColor(0.0f, 0.0f, 0.0f), m_OuterColor(0.0f, 0.0f, 0.0f),
			m_Pos(0.0f, 0.0f, 0.0f), m_MaxDist(0.0f), m_MaxDistSqr(0.0f), m_bClip(TRUE),
			m_FOV(MATH_HALFPI), m_BrightScale(1.0f)
		{
		}

		DVector			m_Direction;
		DVector			m_Pos;

		DVector			m_InnerColor; // Colors.
		DVector			m_OuterColor;
		
		DFLOAT			m_FOV; // FOV in radians.
		DFLOAT			m_BrightScale;
		
		DFLOAT			m_MaxDist;
		DFLOAT			m_MaxDistSqr;
		BOOL			m_bClip;	// Should this light clip against walls?
		BOOL			m_bDirectional;
};


class LightGroup : public BaseClass// : public LightObject
{
public:
	static const std::string LIGHTFX_LIGHTGROUP_EXTENSION;

	typedef std::list<LightGroup*> LightGroupList;
	typedef LightGroupList::const_iterator LightGroupIterator;

	static LightGroupIterator BeginLightGroups() { return m_sLightGroups.begin(); }
	static LightGroupIterator EndLightGroups()   { return m_sLightGroups.end(); }

public :
	LightGroup();
	virtual ~LightGroup();

	// For LightObject
	virtual LTVector GetPos() const;
	virtual LTFLOAT  GetRadiusSqr() const;
	virtual LTVector GetColor(const LTVector & vLocation) const;

protected :

	DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
	DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

private:

	void	MakeLights();

	DBOOL	ReadProp(ObjectCreateStruct *);
	void	PostPropRead(ObjectCreateStruct *pStruct);
	DBOOL	InitialUpdate(DVector *pMovement);
	void	CacheFiles();
	void	SendEffectMessage();
	DBOOL	Update();
	DBOOL	Init();

	void	PreCreate(ObjectCreateStruct *pStruct);
	void	Load(HMESSAGEREAD hRead, DDWORD dwSaveFlags);
	void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
	void	HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead);
	void	TriggerChildren(HSTRING hMsg);

	//-------------------------------------------------------------------------
	struct LightDef
	{
		HSTRING		m_hName;
		LTVector	m_Pos;
		LTVector	m_Color;
		LTFLOAT		m_Radius;
		LTBOOL		m_ModelLight;
	};
	typedef std::vector<LightDef>	LightList;
	LightList						m_LightList;

	typedef std::vector<LTSmartLink>ClientList;
	ClientList						m_ObjectList;

	//-------------------------------------------------------------------------
	// This is the info that gets sent to the Client
	DVector m_vColor;				    // First color to use
	DDWORD	m_dwLightFlags;

	DFLOAT	m_fIntensityDelta;			// What is the total change so far
	DFLOAT  m_fIntensityMin;			// How Dark light gets
	DFLOAT  m_fIntensityMax;			// How Bright light gets
	DBYTE	m_nIntensityWaveform;		// What is the wave form for the intensity
	DFLOAT	m_fIntensityFreq;			// And frequency
	DFLOAT	m_fIntensityPhase;			// What is the phase

	DFLOAT  m_fRadiusMin;				// How small light gets
	DFLOAT  m_fRadiusMax;				// How large light gets
	DBYTE	m_nRadiusWaveform;			// Wave form for radius
	DFLOAT	m_fRadiusFreq;				// Frequency for radius
	DFLOAT	m_fRadiusPhase;				// Phase for radius

    HSTRING m_hstrRampUpSound;           // Sounds for RampUp and RampDown
    HSTRING m_hstrRampDownSound;         

	DBOOL	m_bUseLightAnims;			// Are we using light animations (precalculated shadows)?
	//-------------------------------------------------------------------------

	DBOOL   m_bOn;				        // Are we on?
	
	DFLOAT  m_fLifeTime;				// How long should this light stay around

	DBOOL	m_bDynamic;					// Was this object dynamically create

	DFLOAT	m_fStartTime;			    // When did this light get created


	DFLOAT	m_fHitPts;

	static LightGroupList m_sLightGroups;
};

// The LightFX plugin class to create light animations.
class CLightGroupFXPlugin : public IObjectPlugin
{
public:
	virtual DRESULT	PreHook_Light(
		PreLightLT *pInterface, 
		HPREOBJECT hObject);

	virtual DRESULT PreHook_EditStringList
					(const char* szRezPath, const char* szPropName,
					 char* const * aszStrings, DDWORD* pcStrings, const DDWORD 
					 cMaxStrings, const DDWORD cMaxStringLength);
};

//void CLightGroupFXPreprocessorCB(HPREOBJECT hObject, PreLightLT *pInterface);

//-----------------------------------------------------------------------------
// Light manager is a singleton that keeps a list of all the 
// lights that were processed as a part of the Group system and so
// should not be processed as a part of the light phase
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//	F O R W A R D   D E C L A R A T I O N S
//-----------------------------------------------------------------------------
struct LightMgrData;

//-----------------------------------------------------------------------------
// C L A S S   L I G H T   M A N A G E R
//-----------------------------------------------------------------------------
class LightManager
{
	// No one should be able to make one
	LightManager();
	// No one can copy one
	LightManager(const LightManager& lhs);
	// No one can equal one
	LightManager& operator=(const LightManager& lhs);

	// For the singleton aspect
	static LightManager* m_pInstance;

	// Data for the class
	struct LightMgrData *m_pData;

public:
	// They can be destructed, though it does nothing. This aids in cases
	// Where they are made non singleton objects.
	~LightManager();
	// This is used for private functions to get access to the data
	LightMgrData* GetLightMgrData() {return m_pData;}

	// This gives global access to the class without it being global
	static LightManager* GetInstance();

	// Reset the lightmanager between levels.
	bool Reset( );

	// Register a light name
	void AddName(const std::string& name, const std::string& group);

	// Is name in one of the groups?
	bool InGroup(const std::string& name);

	// What is the name of the group this belongs to?
	const std::string& GetGroup(const std::string& name);
};


//-----------------------------------------------------------------------------
#endif  // __LIGHTGROUP_H__



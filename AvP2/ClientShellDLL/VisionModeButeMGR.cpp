// ----------------------------------------------------------------------- //
//
// MODULE  : VisionModeButeMgr.cpp
//
// PURPOSE : VisionModeButeMgr implementation - Controls attributes of all Vision Modes.
//
// CREATED : 12/02/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#ifndef VISION_MODE_BUTE_MGR_H
#include "VisionModeButeMGR.h"
#endif

#ifndef VIEW_ORDER_ODDS_H
#include "ViewOrderOdds.h"
#endif

#include "CommonUtilities.h"

#pragma warning (disable : 4503)
#include <map>
#include <string>
#include <vector>
#include <algorithm>

// There are a lot of butes so let's wrap this in a namespace
namespace VisionMode
{
	// This is the container of vision sets. A vision set is a list of the
	// name of all the modes that make up a set.
	typedef std::map<std::string, std::vector<std::string> >			VisionSetContainer;

	// A vision mode is the basic ingredient list of a vision mode. It 
	// defines the parts needed to create this mode.
	struct MasterVisionMode
	{
		MasterVisionMode() 
		{
			m_TargetHumans		= 0;
			m_TargetAliens		= 0;
			m_TargetPredators	= 0;
			m_AllowHud			= 0;
			m_Invert			= 0;
			m_FullBright		= 0;
			m_DrawSky			= 1;
		}

		std::string					m_DefaultTexState;
		std::string					m_LightState;
		LTBOOL						m_TargetHumans;
		LTBOOL						m_TargetAliens;
		LTBOOL						m_TargetPredators;
		LTBOOL						m_AllowHud;
		LTBOOL						m_Invert;
		LTBOOL						m_FullBright;
		LTBOOL						m_DrawSky;
		CamOverlayList				m_CameraOverlays;
	};

	// This is used to store and reference the various Vision Modes
	typedef std::map<std::string, MasterVisionMode>		VisionModeMap;

	// The name of a set of state change commands
	typedef std::map<std::string, VisionStateList >		VisionStateContainer;

	struct ButeMGRData
	{
		// Set of vision states
		VisionSetContainer		m_VisionSets;

		// Overall definition of modes
		VisionModeMap			m_VisionModes;

		// State changes for texture and light rendering.
		VisionStateContainer	m_TextureStates;
		VisionStateContainer	m_LightStates;
		DefaultStateContainer	m_DefaultTextureStates;
	};

		static const std::string VISION_SET("VisionSet");
		static const std::string VISION_SET_NAME("VisionSetName");
		static const std::string VISION_MODE("VisionMode");

		static const std::string VISION_MODE_NAME("VisionModeName");
		static const std::string DEFAULT_TEX_STATE("DefaultTexState");
		static const std::string LIGHT_STATE("LightState");
		static const std::string CAMERA_OVERLAY("CameraOverlay");
		static const std::string CAMERA_OVERLAY_ROT("CameraOverlayRot");

		static const std::string TEXTURE_STATE("TextureState");
		static const std::string TEXTURE_STATE_NAME("TextureStateName");
		static const std::string STATE_TYPE("StateType");
		static const std::string AFFECTED_STATE("AffectedState");
		static const std::string STATE_VALUE("StateValue");

		static const std::string LIGHT_STATE_NAME("LightStateName");

		static const std::string TARGET_HUMAN("TargetHuman");
		static const std::string TARGET_ALIEN("TargetAlien");
		static const std::string TARGET_PREDATOR("TargetPredator");
		static const std::string ALLOW_HUD("AllowHud");
		static const std::string INVERT("Invert");
		static const std::string FULLBRIGHT("FullBright");
		static const std::string DRAWSKY("DrawSky");
}


VisionMode::ButeMGR* VisionMode::g_pButeMgr = DNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VisionMode::ButeMGR::ButeMGR()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
VisionMode::ButeMGR::ButeMGR()
{
	m_pData = new ButeMGRData();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VisionMode::ButeMGR::~ButeMGR()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //
VisionMode::ButeMGR::~ButeMGR()
{
	delete m_pData;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VisionMode::ButeMGR::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //
DBOOL VisionMode::ButeMGR::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	if (VisionMode::g_pButeMgr || !szAttributeFile) return DFALSE;
	if (!Parse(pInterface, szAttributeFile)) return DFALSE;

	// Set up global pointer
	VisionMode::g_pButeMgr = this;


	// Get the vision set
	char buff[10];
	int set = 0;
	std::string tagName = VISION_SET + itoa(set, buff, 10);

	while (m_buteMgr.Exist(tagName.c_str()))
	{
		// Read the name
		std::string name = m_buteMgr.GetString(tagName.c_str(), VISION_SET_NAME.c_str());

		// Set up for next while loop
		int view = 0;
		std::string viewName = VISION_MODE + itoa(view, buff, 10);

		while (m_buteMgr.Exist(tagName.c_str(), viewName.c_str()))
		{
			// Read the modes into the vision lists
			// What I'll code just to not use CString!
			std::string visionName = (m_buteMgr.GetString(tagName.c_str(), viewName.c_str())).GetBuffer();
			m_pData->m_VisionSets[name].push_back(visionName);

			++view;
			viewName = VISION_MODE + itoa(view, buff, 10);
		}
	
		++set;
		tagName = VISION_SET + itoa(set, buff, 10);
	}


	// Get MasterVisionMode data
	set = 0;
	tagName = VISION_MODE + itoa(set, buff, 10);

	while (m_buteMgr.Exist(tagName.c_str()))
	{
		// Read the name
		std::string name = m_buteMgr.GetString(tagName.c_str(), VISION_MODE_NAME.c_str());
		// Make a struct for the data
		MasterVisionMode masterVision;

		// Fill 'er up
		masterVision.m_DefaultTexState = m_buteMgr.GetString(tagName.c_str(), DEFAULT_TEX_STATE.c_str());
		masterVision.m_LightState = m_buteMgr.GetString(tagName.c_str(), LIGHT_STATE.c_str());

		masterVision.m_TargetHumans		= (LTBOOL)m_buteMgr.GetInt(tagName.c_str(), TARGET_HUMAN.c_str());
		masterVision.m_TargetAliens		= (LTBOOL)m_buteMgr.GetInt(tagName.c_str(), TARGET_ALIEN.c_str());
		masterVision.m_TargetPredators	= (LTBOOL)m_buteMgr.GetInt(tagName.c_str(), TARGET_PREDATOR.c_str());
		masterVision.m_AllowHud			= (LTBOOL)m_buteMgr.GetInt(tagName.c_str(), ALLOW_HUD.c_str());
		masterVision.m_Invert			= (LTBOOL)m_buteMgr.GetInt(tagName.c_str(), INVERT.c_str(), 0);
		masterVision.m_FullBright		= (LTBOOL)m_buteMgr.GetInt(tagName.c_str(), FULLBRIGHT.c_str(), 0);
		masterVision.m_DrawSky			= (LTBOOL)m_buteMgr.GetInt(tagName.c_str(), DRAWSKY.c_str(), 1);

		// Make the default state list
		m_pData->m_DefaultTextureStates[name] = masterVision.m_DefaultTexState;


		// Set up for next while loop
		int overlay = 0;
		std::string viewName = CAMERA_OVERLAY + itoa(overlay, buff, 10);
		std::string rotName = CAMERA_OVERLAY_ROT + itoa(overlay, buff, 10);

		while (m_buteMgr.Exist(tagName.c_str(), viewName.c_str()))
		{
			// Read the overlays into the vector
			// What I'll code just to not use CString!
			masterVision.m_CameraOverlays.push_back(
				CamOverlay(
					(m_buteMgr.GetString(tagName.c_str(), viewName.c_str())).GetBuffer(),
					m_buteMgr.GetFloat(tagName.c_str(), rotName.c_str(), 0.0f)
				)
			);

			++overlay;
			viewName = CAMERA_OVERLAY + itoa(overlay, buff, 10);
			rotName = CAMERA_OVERLAY_ROT + itoa(overlay, buff, 10);
		}
		m_pData->m_VisionModes[name] = masterVision;

		++set;
		tagName = VISION_MODE + itoa(set, buff, 10);
	}

	
	// Get texture states
	set = 0;
	tagName = TEXTURE_STATE + itoa(set, buff, 10);

	while (m_buteMgr.Exist(tagName.c_str()))
	{
		// Read the name
		std::string name = m_buteMgr.GetString(tagName.c_str(), TEXTURE_STATE_NAME.c_str());

		// Set up for next while loop
		int state = 0;
		std::string stateName = STATE_TYPE + itoa(state, buff, 10);

		while (m_buteMgr.Exist(tagName.c_str(), stateName.c_str()))
		{
			VisionStateDefinition stateDefinition;

			// Read in the type
			int type = m_buteMgr.GetInt(tagName.c_str(), stateName.c_str());
			if (type)
			{
				stateDefinition.m_StateType = TEXTURE;
			}
			else
			{
				stateDefinition.m_StateType = RENDER;
			}

			// Read in the state
			std::string whichState = AFFECTED_STATE + itoa(state, buff, 10);
			stateDefinition.m_AffectedState = m_buteMgr.GetInt(tagName.c_str(), whichState.c_str());

			// Read in the value
			std::string whichValue = STATE_VALUE + itoa(state, buff, 10);
			stateDefinition.m_StateValue = m_buteMgr.GetInt(tagName.c_str(), whichValue.c_str());

			// Add this to our master list
			m_pData->m_TextureStates[name].push_back(stateDefinition);

			++state;
			stateName = STATE_TYPE + itoa(state, buff, 10);
		}
	
		++set;
		tagName = TEXTURE_STATE + itoa(set, buff, 10);
	}


	// Get light states
	set = 0;
	tagName = LIGHT_STATE + itoa(set, buff, 10);

	while (m_buteMgr.Exist(tagName.c_str()))
	{
		// Read the name
		std::string name = m_buteMgr.GetString(tagName.c_str(), LIGHT_STATE_NAME.c_str());

		// Set up for next while loop
		int state = 0;
		std::string stateName = STATE_TYPE + itoa(state, buff, 10);

		while (m_buteMgr.Exist(tagName.c_str(), stateName.c_str()))
		{
			VisionStateDefinition stateDefinition;

			// Read in the type
			int type = m_buteMgr.GetInt(tagName.c_str(), stateName.c_str());
			if (type)
			{
				stateDefinition.m_StateType = TEXTURE;
			}
			else
			{
				stateDefinition.m_StateType = RENDER;
			}

			// Read in the state
			std::string whichState = AFFECTED_STATE + itoa(state, buff, 10);
			stateDefinition.m_AffectedState = m_buteMgr.GetInt(tagName.c_str(), whichState.c_str());

			// Read in the value
			std::string whichValue = STATE_VALUE + itoa(state, buff, 10);
			stateDefinition.m_StateValue = m_buteMgr.GetInt(tagName.c_str(), whichValue.c_str());

			// Add this to our master list
			m_pData->m_LightStates[name].push_back(stateDefinition);

			++state;
			stateName = STATE_TYPE + itoa(state, buff, 10);
		}
	
		++set;
		tagName = LIGHT_STATE + itoa(set, buff, 10);
	}
	return DTRUE;
}

const VisionMode::CamOverlayList* VisionMode::ButeMGR::GetOverlays(const std::string& modeName)
{
	VisionMode::VisionModeMap::iterator iter = m_pData->m_VisionModes.find(modeName);
	if ( iter != m_pData->m_VisionModes.end())
	{
		return &(iter->second.m_CameraOverlays);
	}
	else
	{
		return 0;
	}
}

const VisionMode::DefaultStateContainer& VisionMode::ButeMGR::GetDefaultTextureStates()
{
	return m_pData->m_DefaultTextureStates;
}

StringList VisionMode::ButeMGR::GetStateNames()
{
	StringList stateNames;
	stateNames.resize(m_pData->m_TextureStates.size(),"");

	VisionStateContainer::const_iterator iter = m_pData->m_TextureStates.begin();
	while (iter != m_pData->m_TextureStates.end())
	{
		std::string name = iter->first;
		stateNames.push_back(name);
		++iter;
	}

	return stateNames;
}

const VisionMode::VisionStateList* VisionMode::ButeMGR::GetD3DState(const std::string& stateName)
{
	if (m_pData->m_TextureStates.find(stateName) != m_pData->m_TextureStates.end())
	{
		return &(m_pData->m_TextureStates[stateName]);
	}
	else
	{
		return 0;
	}
}

const std::string& VisionMode::ButeMGR::GetNextMode(const std::string& setName, const std::string& oldMode)
{
	// When all else fails return the old mode.
	const std::string* newMode = &oldMode;

	// Exit if there are no sets
	if (m_pData->m_VisionSets.empty())
	{
		newMode = &Views::START_MODE;
	}

	// Exit if there are no modes
	else if (m_pData->m_VisionSets[setName].empty())
	{
		newMode = &Views::START_MODE;
	}

	else
	{
		// Get the two ends of the list
		std::vector<std::string>::iterator iterBegin = m_pData->m_VisionSets[setName].begin();
		std::vector<std::string>::iterator iterEnd = m_pData->m_VisionSets[setName].end();

		// find the mode
		std::vector<std::string>::iterator find = std::find(iterBegin, iterEnd, oldMode);
		// if the mode exists
		if (find != iterEnd)
		{
			++find;

			// Now if we are at the end set it to the beginning
			if (find == iterEnd)
			{
				find = iterBegin;
			}

			// Set the new mode
			newMode = &(*find);
		}
	}
	return *newMode;
}

const std::string& VisionMode::ButeMGR::GetPrevMode(const std::string& setName, const std::string& oldMode)
{
	// When all else fails return the old mode.
	const std::string* newMode = &oldMode;

	// Exit if there are no sets
	if (m_pData->m_VisionSets.empty())
	{
		newMode = &Views::START_MODE;
	}

	// Exit if there are no modes
	else if (m_pData->m_VisionSets[setName].empty())
	{
		newMode = &Views::START_MODE;
	}

	else
	{
		// Get the two ends of the list
		std::vector<std::string>::iterator iterBegin = m_pData->m_VisionSets[setName].begin();
		std::vector<std::string>::iterator iterEnd = m_pData->m_VisionSets[setName].end();

		// find the mode
		std::vector<std::string>::iterator find = std::find(iterBegin, iterEnd, oldMode);
		// if the mode exists
		if (find != iterEnd)
		{
			// Is this the first mode?
			if (find == iterBegin)
			{
				find = iterEnd;
			}

			// Decrement the mode
			--find;

			// Set the new mode
			newMode = &(*find);
		}
	}
	return *newMode;
}

LTBOOL VisionMode::ButeMGR::AllowHud(const std::string& modeName)
{
	VisionMode::VisionModeMap::iterator iter = m_pData->m_VisionModes.find(modeName);
	if ( iter != m_pData->m_VisionModes.end())
	{
		return iter->second.m_AllowHud;
	}
	else
	{
		return LTFALSE;
	}
}

LTBOOL VisionMode::ButeMGR::IsInverted(const std::string& modeName)
{
	VisionMode::VisionModeMap::iterator iter = m_pData->m_VisionModes.find(modeName);
	if ( iter != m_pData->m_VisionModes.end())
	{
		return iter->second.m_Invert;
	}
	else
	{
		return LTFALSE;
	}
}

LTBOOL VisionMode::ButeMGR::ShouldDrawSky(const std::string& modeName)
{
	VisionMode::VisionModeMap::iterator iter = m_pData->m_VisionModes.find(modeName);
	if ( iter != m_pData->m_VisionModes.end())
	{
		return iter->second.m_DrawSky;
	}
	else
	{
		return LTFALSE;
	}
}

LTBOOL VisionMode::ButeMGR::IsFullBright(const std::string& modeName)
{
	VisionMode::VisionModeMap::iterator iter = m_pData->m_VisionModes.find(modeName);
	if ( iter != m_pData->m_VisionModes.end())
	{
		return iter->second.m_FullBright;
	}
	else
	{
		return LTFALSE;
	}
}

LTBOOL VisionMode::ButeMGR::IsHumanTargeting(const std::string& modeName)
{
	VisionMode::VisionModeMap::iterator iter = m_pData->m_VisionModes.find(modeName);
	if ( iter != m_pData->m_VisionModes.end())
	{
		return iter->second.m_TargetHumans;
	}
	else
	{
		return LTFALSE;
	}
}

LTBOOL VisionMode::ButeMGR::IsAlienTargeting(const std::string& modeName)
{
	VisionMode::VisionModeMap::iterator iter = m_pData->m_VisionModes.find(modeName);
	if ( iter != m_pData->m_VisionModes.end())
	{
		return iter->second.m_TargetAliens;
	}
	else
	{
		return LTFALSE;
	}
}

LTBOOL VisionMode::ButeMGR::IsPredatorTargeting(const std::string& modeName)
{
	VisionMode::VisionModeMap::iterator iter = m_pData->m_VisionModes.find(modeName);
	if ( iter != m_pData->m_VisionModes.end())
	{
		return iter->second.m_TargetPredators;
	}
	else
	{
		return LTFALSE;
	}
}

LTBOOL VisionMode::ButeMGR::IsPredatorHeatVision(const std::string& modeName)
{
	VisionMode::VisionModeMap::iterator iter = m_pData->m_VisionModes.find(modeName);

	return iter == m_pData->m_VisionModes.find("HeatVision");
}

LTBOOL VisionMode::ButeMGR::IsPredatorElecVision(const std::string& modeName)
{
	VisionMode::VisionModeMap::iterator iter = m_pData->m_VisionModes.find(modeName);

	return iter == m_pData->m_VisionModes.find("ElectroVision");
}

LTBOOL VisionMode::ButeMGR::IsPredatorNavVision(const std::string& modeName)
{
	VisionMode::VisionModeMap::iterator iter = m_pData->m_VisionModes.find(modeName);

	return iter == m_pData->m_VisionModes.find("NavVision");
}

LTBOOL VisionMode::ButeMGR::IsMarineNightVision(const std::string& modeName)
{
	VisionMode::VisionModeMap::iterator iter = m_pData->m_VisionModes.find(modeName);

	return iter == m_pData->m_VisionModes.find("NightVision");
}

LTBOOL VisionMode::ButeMGR::IsAlienHuntingVision(const std::string& modeName)
{
	VisionMode::VisionModeMap::iterator iter = m_pData->m_VisionModes.find(modeName);

	return iter == m_pData->m_VisionModes.find("Hunting");
}

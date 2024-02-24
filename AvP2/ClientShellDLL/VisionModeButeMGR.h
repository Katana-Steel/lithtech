// ----------------------------------------------------------------------- //
//
// MODULE  : VisionModeButeMGR.h
//
// PURPOSE : VisionModeButeMGR definition
//
// CREATED : 05.22.1999
//
// ----------------------------------------------------------------------- //

#ifndef VISION_MODE_BUTE_MGR_H
#define VISION_MODE_BUTE_MGR_H

// Includes

#include "GameButeMgr.h"
#include "ClientServerShared.h"

#pragma warning (disable : 4503)
#include <vector>
#include <string>
#include <map>

// There are a lot of butes so let's wrap this in a namespace
namespace VisionMode
{
	// Pre define the classes we are going to refer to.
	struct	ButeMGRData;
	class	ButeMGR;

	// This should be a singleton, but all the others are done this way
	extern ButeMGR* g_pButeMgr;

	// A camera overlay
	struct CamOverlay
	{
		CamOverlay() {}
		CamOverlay(const CamOverlay &cOther) : 
			m_Name(cOther.m_Name), 
			m_fRotSpeed(cOther.m_fRotSpeed) {
		}
		CamOverlay(const std::string &sName, float fRotSpeed) :
			m_Name(sName), 
			m_fRotSpeed(fRotSpeed) {
		}
		std::string m_Name;
		float		m_fRotSpeed;
	};

	// Lists of camera overlays
	typedef std::vector<CamOverlay>	CamOverlayList;

	// This is a list of the vision mode name and the default value of 
	// the texture.
	typedef std::map<const std::string, 
						std::string>			DefaultStateContainer;

	// What kind of states should be effected
	enum StateType
	{
		RENDER		= 0,
		TEXTURE		= 1
	};

	// A struct to hold the info for the state change
	struct VisionStateDefinition
	{
		StateType					m_StateType;		// Either Render or Texture
		int							m_AffectedState;	// One of the valid states to set
		int							m_StateValue;		// A valid value to set this state to.
	};

	// There are an indeterminant number of these, so this vector holds them
	typedef std::vector<VisionStateDefinition>		VisionStateList;

	// This is the main class.
	class ButeMGR : public CGameButeMgr
	{
	public:
		ButeMGR();
		virtual ~ButeMGR();

		// Init the class. This might end up being a constructor and remove the default.
		// I have to see where it will be created and inited.
		DBOOL		Init(ILTCSBase *pInterface, const char* szAttributeFile="Attributes\\VisionModeButes.txt");

		const CamOverlayList* GetOverlays(const std::string& modeName);
		const DefaultStateContainer& GetDefaultTextureStates();
		const VisionStateList* GetD3DState(const std::string& stateName);
		StringList GetStateNames();
		const std::string& GetNextMode(const std::string& setName, const std::string& oldMode);
		const std::string& GetPrevMode(const std::string& setName, const std::string& oldMode);

		LTBOOL	IsHumanTargeting(const std::string& modeName);
		LTBOOL	IsAlienTargeting(const std::string& modeName);
		LTBOOL	IsPredatorTargeting(const std::string& modeName);
		LTBOOL	IsPredatorHeatVision(const std::string& modeName);
		LTBOOL	IsPredatorElecVision(const std::string& modeName);
		LTBOOL	IsPredatorNavVision(const std::string& modeName);
		LTBOOL	IsMarineNightVision(const std::string& modeName);
		LTBOOL	AllowHud(const std::string& modeName);
		LTBOOL	IsAlienHuntingVision(const std::string& modeName);
		LTBOOL	IsInverted(const std::string& modeName);
		LTBOOL	IsFullBright(const std::string& modeName);
		LTBOOL	ShouldDrawSky(const std::string& modeName);

	private:
		ButeMGRData*	m_pData;
	};
}

#endif // VISION_MODE_BUTE_MGR_H


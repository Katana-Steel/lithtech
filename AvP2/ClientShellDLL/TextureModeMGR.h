// ----------------------------------------------------------------------- //
//
// MODULE  : TextureModeMGR.h
//
// PURPOSE : Manages the special renderstate modes for textures based on
//			 vision modes and flags set on the texture.
//
// CREATED : 05/16/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef TEXTURE_MODE_MGR_H
#define TEXTURE_MODE_MGR_H

#ifndef VIEW_ORDER_ODDS_H
#include "ViewOrderOdds.h"
#endif

#ifndef VISION_MODE_BUTE_MGR_H
#include "VisionModeButeMGR.h"
#endif

#include <vector>
#include <utility>
#include <map>
#include <string>

struct StateChange;

class TextureModeMGR
{
	// This container holds the statechange objects for the textures
	// currently in the game. The first is the list of textures, and
	// the second is all the states that texture has.
	typedef std::vector<StateChange*>								StateContainer;
	typedef StateContainer::iterator								StateIterator;

	typedef std::vector<std::pair<StateChange**, StateContainer*> >	VisionContainer;
	typedef VisionContainer::iterator								VisionIterator;
	VisionContainer													m_VisionVector;

	// This is used so StateChange objects can be refered to by name.
	typedef std::map<const std::string, StateChange*, CaselessLesser>	MasterStateContainer;
	typedef MasterStateContainer::iterator								MasterStateIterator;
	MasterStateContainer												m_MasterStateMap;

	VisionMode::DefaultStateContainer								m_Defaults;

	// This gives us the order that modes should be set to.
	// The mode MGR needs to give this to us so we can coordinate
	// requests.
	typedef std::map<std::string, int, CaselessLesser>				ModeListContainer;
	ModeListContainer												m_ModeList;

	// Used to set new textures to the current mode
	std::string m_CurrentMode;
	bool m_bInited;

	void Init();

	void SetMode(int mode);
public:
	TextureModeMGR(std::string mode = "None");
	~TextureModeMGR();
	void RegisterTexture(StateChange** stateChange, const char* commandLine);
	void SetMode(std::string mode);
	void Flush();
};


#endif		// TEXTURE_MODE_MGR_H

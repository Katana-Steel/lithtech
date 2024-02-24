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

#include "stdafx.h"


#ifndef TEXTURE_MODE_MGR_H
#include "TextureModeMGR.h"
#endif

#ifndef D3D_STATE_CHANGE
#include "D3DStateChange.h"
#endif

#ifndef VISION_MODE_BUTE_MGR_H
#include "VisionModeButeMGR.h"
#endif

#ifndef VIEW_ORDER_ODDS_H
#include "ViewOrderOdds.h"
#endif

#include <strstrea.h>

TextureModeMGR::TextureModeMGR(std::string mode) : m_CurrentMode(mode), m_bInited(false)
{
}

TextureModeMGR::~TextureModeMGR()
{
	for(MasterStateIterator iter1 = m_MasterStateMap.begin()
		;iter1 != m_MasterStateMap.end(); ++iter1)
	{
		delete iter1->second;
	}

	for(VisionIterator iter2 = m_VisionVector.begin()
		;iter2 != m_VisionVector.end(); ++iter2)
	{
		delete iter2->second;
	}

}

void TextureModeMGR::RegisterTexture(StateChange** stateChange, const char* commandLine)
{
	// Check here to see if we are intitialized. If not do it now. This ensures
	// that Bute MGR has been intited.
	if (!m_bInited)
	{
		Init();
	}

	// for each view texture, make a vector of the stateChange objects based on the default
	// states.
	StateContainer* p_visionVector = new StateContainer(m_Defaults.size());

	// Make a default copy of the modes
	StateIterator vecIter = p_visionVector->begin();
	for (VisionMode::DefaultStateContainer::const_iterator mapIter = m_Defaults.begin()
		; mapIter != m_Defaults.end(); ++mapIter, ++vecIter)
	{
		// Make sure this mode exists
		MasterStateContainer::const_iterator masterStateIter = m_MasterStateMap.find(mapIter->second);
		if ( masterStateIter != m_MasterStateMap.end())
		{
			*vecIter = masterStateIter->second;
		}
		else
		{
			*vecIter = 0;
		}
	}


	// Use the commandLine to modify the default textures
	// This is coming soon
	if (commandLine)
	{
		char szBuffer[256];
		char *szToks[64];
		int nToks = 0;
		strcpy(szBuffer, commandLine);

		szToks[nToks] = strtok(szBuffer, ";");

		while(szToks[nToks])
		{
			++nToks;
			szToks[nToks] = strtok(NULL, ";");
		}


		// We're looking for a command string parameter that starts with 'ViewModes'
		for(int i = 0; i < nToks; i++)
		{
			// If this is the part we want
			if(!strnicmp(szToks[i], Views::VIEW_MODES.c_str(), strlen(Views::VIEW_MODES.c_str()) - 1))
			{
				char *szSubToks[64];
				int nSubToks = 0;
				memset(szSubToks, 0, sizeof(char*) * 64);
				strcpy(szBuffer, szToks[i]);

				szSubToks[nSubToks] = strtok(szBuffer, " ");

				while(szSubToks[nSubToks])
				{
					++nSubToks;
					szSubToks[nSubToks] = strtok(NULL, " ");
				}


				// Go through the pairs of vision mode commands and set them up
				for(int j = 1; j < nSubToks; j += 2)
				{
					// Get the slot
					if(szSubToks[j])
					{
						ModeListContainer::iterator modeIter = m_ModeList.find(szSubToks[j]);
						if (modeIter != m_ModeList.end())
						{
							int modeSlot = modeIter->second;

							ASSERT((uint32)modeSlot < p_visionVector->size());

							// Make sure this mode exists
							if(szSubToks[j + 1])
							{
								MasterStateContainer::const_iterator masterIter = m_MasterStateMap.find(szSubToks[j + 1]);
								if ( masterIter != m_MasterStateMap.end())
								{
									(*p_visionVector)[modeSlot] = masterIter->second;
								}
								else
								{
									// If not just be 0
									(*p_visionVector)[modeSlot] = 0;
								}
							}
						}
					}
				}
			}
		}
	}

	// Register the testure's pointer and the view mode list to keep them in sync
	m_VisionVector.push_back(std::make_pair(stateChange, p_visionVector));

	// Set the texture to the current texture.
	// Get the slot
	ModeListContainer::iterator modeIter = m_ModeList.find(m_CurrentMode);
	if (modeIter != m_ModeList.end())
	{
		*stateChange = p_visionVector->at(modeIter->second);
	}
	else
	{
		*stateChange = 0;
	}
}

void TextureModeMGR::Flush()
{
	m_VisionVector.clear();
}

void TextureModeMGR::SetMode(std::string mode)
{
	// Update the current texture
	m_CurrentMode = mode;

	ModeListContainer::iterator modeIter = m_ModeList.find(m_CurrentMode);
	if (modeIter != m_ModeList.end())
	{
		SetMode(modeIter->second);
	}
}

void TextureModeMGR::SetMode(int mode)
{
	// Loop through all the textures and set the shared textures state object 
	// to the correct object based on the mode
	for(VisionIterator iter = m_VisionVector.begin(); iter != m_VisionVector.end(); ++iter)
	{
		*(iter->first) = iter->second->at(mode);
	}
}

void TextureModeMGR::Init()
{
	m_Defaults = VisionMode::g_pButeMgr->GetDefaultTextureStates();

	// This sets up the mode order list. This provides faster dereferencing
	int i = 0;
	for (VisionMode::DefaultStateContainer::const_iterator defIter = m_Defaults.begin();
			defIter != m_Defaults.end(); ++defIter)
	{
		m_ModeList[defIter->first] = i;
		++i;
	}


	// Read each of the state names
	// for each state read and create a statechange object
	// (Make some as examples)
	const StringList stateNames = VisionMode::g_pButeMgr->GetStateNames();

	for (StringList::const_iterator stateIter = stateNames.begin(); stateIter != stateNames.end(); ++stateIter)
	{
		// Make each view
		const VisionMode::VisionStateList* visionList = VisionMode::g_pButeMgr->GetD3DState(*stateIter);
		// Make sure this list exists before refering to it
		if (visionList)
		{
			// Make a new object. The master map takes posession of this pointer so it is not deleted
			StateChange* StateOne = new StateChange();
			// Get each stage and add it to the StateChange object
			for (VisionMode::VisionStateList::const_iterator iter = visionList->begin();
					iter != visionList->end(); ++iter)
			{
				if (VisionMode::RENDER == iter->m_StateType)
				{
					StateOne->Add(RenderState(static_cast<D3DRENDERSTATETYPE>(iter->m_AffectedState),
												iter->m_StateValue));
				}
				else
				{
					StateOne->Add(TextureState(0, static_cast<D3DTEXTURESTAGESTATETYPE>(iter->m_AffectedState),
												iter->m_StateValue));
				}
			}
			// Add this to the map
			m_MasterStateMap[*stateIter] = StateOne;
		}
	}
	m_bInited = true;
}
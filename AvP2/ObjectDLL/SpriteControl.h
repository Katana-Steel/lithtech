// ----------------------------------------------------------------------- //
//
// MODULE  : SpriteControlObj.h
//
// PURPOSE : An sprite object which can manipulate it through messages
//
// CREATED : 7/12/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SPRITE_CONTROL_OBJ_H__
#define __SPRITE_CONTROL_OBJ_H__

// ----------------------------------------------------------------------- //

#include "GameBase.h"

// ----------------------------------------------------------------------- //

class SpriteControlObj : public GameBase
{
	public:

		SpriteControlObj();
		virtual ~SpriteControlObj();

	protected :

		// Handles engine messages
		virtual uint32		EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual uint32		ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	private:

		// Local functions
		LTBOOL		ReadProp(ObjectCreateStruct *pData);
		LTBOOL		HandleTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead);
		void		UpdateClients();

		// Save and load functions
		void		Save(HMESSAGEWRITE pData, uint32 fData);
		void		Load(HMESSAGEREAD pData, uint32 fData);

	protected:

		// Property members that need to be set on the first update
		LTVector	m_vColor;
		LTFLOAT		m_fAlpha;
		LTVector	m_vScale;

		// Members that get communicated to the clients
		uint8		m_nPlayMode;			// The current play mode
		uint8		m_nStartFrame;			// The current start frame
		uint8		m_nEndFrame;			// The current ending frame
		uint8		m_nFramerate;			// The current framerate to play at
};

// ----------------------------------------------------------------------- //
// Plugin class for static string lists

class CSpriteControlPlugin : public IObjectPlugin
{
  public:

	virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath, 
		const char* szPropName, 
		char* const * aszStrings, 
		uint32* pcStrings, 
		const uint32 cMaxStrings, 
		const uint32 cMaxStringLength);

  protected :

};

// ----------------------------------------------------------------------- //

#endif  // __SPRITE_CONTROL_OBJ_H__


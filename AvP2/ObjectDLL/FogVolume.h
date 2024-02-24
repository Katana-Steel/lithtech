// ----------------------------------------------------------------------- //
//
// MODULE  : FogVolume.h
//
// PURPOSE : A container that represents a volumetric fog area
//
// CREATED : 8/27/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __FOG_VOLUME_H__
#define __FOG_VOLUME_H__

// ----------------------------------------------------------------------- //

#include "GameBase.h"

// ----------------------------------------------------------------------- //

class FogVolume : public GameBase
{
	public:

		FogVolume();
		virtual ~FogVolume();

	protected :

		// Handles engine messages
		virtual uint32		EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual uint32		ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	private:

		// Local functions
		LTBOOL		ReadProp(ObjectCreateStruct *pData);
		LTBOOL		HandleTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead);

		// Special FX create and update functions
		void		CreateSpecialFXMessage();
		void		UpdateClients();

		// Save and load functions
		void		Save(HMESSAGEWRITE pData, uint32 fData);
		void		Load(HMESSAGEREAD pData, uint32 fData);

	protected:

		// Property members that need to be set on the first update
		LTVector	m_vDims;
		LTVector	m_vColor;

		LTFLOAT		m_fDensity;
		LTFLOAT		m_fMinOpacity;
		LTFLOAT		m_fMaxOpacity;

		uint8		m_nRes;
		uint8		m_nShape;
		uint8		m_nGradient;
		uint8		m_nDensityFn;

		LTBOOL		m_bFogWorld;
		LTBOOL		m_bFogModels;
		LTBOOL		m_bVisible;
};

// ----------------------------------------------------------------------- //
// Plugin class for static string lists

class CFogVolumePlugin : public IObjectPlugin
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

#endif  // __FOG_VOLUME_H__


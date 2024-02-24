// ----------------------------------------------------------------------- //
//
// MODULE  : WorldProperties.h
//
// PURPOSE : WorldProperties object - Definition
//
// CREATED : 9/25/98
//
// (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _WORLD_PROPERTIES_H_
#define _WORLD_PROPERTIES_H_

#include "cpp_engineobjects_de.h"
#include "iobjectplugin.h"

class WorldProperties;
extern WorldProperties* g_pWorldProperties;

class CWorldPropertiesPlugin : public IObjectPlugin
{
	public:

		virtual DRESULT	PreHook_EditStringList(const char* szRezPath, const char* szPropName, char* const * aszStrings, DDWORD* pcStrings, const DDWORD cMaxStrings, const DDWORD cMaxStringLength);

	private :
		
};


class WorldProperties : public BaseClass
{
	public:

		WorldProperties();
		~WorldProperties();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private :

		void	ReadProps();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		void	HandleMsg(HOBJECT hSender, HSTRING hMsg);
		void	SendClientsFogValues();

		DFLOAT	m_fWorldTimeSpeed;
};

#endif // _WORLD_PROPERTIES_H_


//*********************************************************************************
//*********************************************************************************
// Project:		Aliens vs. Predator 2
// Purpose:		Retrieves attributes from the SpawnButes.txt
//*********************************************************************************
// File:		SpawnButeMgr.h
// Created:		Aug. 28, 2000
// Updated:		Aug. 28, 2000
// Author:		Andy Mattingly
//*********************************************************************************
//*********************************************************************************

#ifndef		__SPAWN_BUTE_MGR_H__
#define		__SPAWN_BUTE_MGR_H__

//*********************************************************************************

#include "HierarchicalButeMgr.h"

//*********************************************************************************

#ifdef _WIN32
	#define SPAWN_BUTES_DEFAULT_FILE		"Attributes\\SpawnButes.txt"
#else
	#define SPAWN_BUTES_DEFAULT_FILE		"Attributes/SpawnButes.txt"
#endif

#define SPAWN_BUTES_STRING_SIZE			64

//*********************************************************************************

class CSpawnButeMgr;
extern CSpawnButeMgr* g_pSpawnButeMgr;

//*********************************************************************************

typedef struct t_SpawnButes
{
	t_SpawnButes::t_SpawnButes();

	// Model and skin information
	char	m_szName[SPAWN_BUTES_STRING_SIZE];		// The name of this pickup

	CString	m_szClass;								// The name of the class of the object
	CString	m_szProps;								// The list of props for the object
	CString m_szCharacterClass;						// Used for cacheing character skins.
	CString m_szCacheWeapons;						// Used for cacheing weapons.

}	SpawnButes;

inline t_SpawnButes::t_SpawnButes()
{
	m_szClass = "\0";
	m_szProps = "\0";
}

//*********************************************************************************

class CSpawnButeMgr : public CHierarchicalButeMgr
{
	public:

		CSpawnButeMgr();
		virtual ~CSpawnButeMgr();

		LTBOOL		Init(ILTCSBase *pInterface, const char* szAttributeFile = SPAWN_BUTES_DEFAULT_FILE);
		void		Term();

		LTBOOL		WriteFile()							{ return m_buteMgr.Save(); }
		void		Reload()							{ m_buteMgr.Parse(m_strAttributeFile); }

		int			GetNumSpawnButes()					const { return m_nNumSpawnButes; }
		int			GetSpawnButesFromName(char *szName)	const;

		//-------------------------------------------------------------------------------//

		void		CheckSpawn(int nSet)				const { ASSERT(nSet < m_nNumSpawnButes); ASSERT(m_pSpawnButes); }

		//-------------------------------------------------------------------------------//
		// Special function to fill in a SpawnButes structure

		const SpawnButes & GetSpawnButes(int nSet)		const { CheckSpawn(nSet); return m_pSpawnButes[nSet];  }
		const SpawnButes & GetSpawnButes(char *szName)	const;

	private:

		// Loading functions (called from Init)
		void LoadSpawnButes(int nSet, SpawnButes &butes);

		// Helper functions to get at the values easier
		CString		GetStringAttrib(int nSet, const char *szAttrib);
		CString		GetStringAttrib(int nSet, const char *szAttrib, const char * szDefault);

	private:

		// Spawn attribute set variables
		int				m_nNumSpawnButes;
		SpawnButes		*m_pSpawnButes;
};

////////////////////////////////////////////////////////////////////////////
//
// CCharacterButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use WeaponMgr
//
////////////////////////////////////////////////////////////////////////////

#ifndef _CLIENTBUILD

#include "iobjectplugin.h"

class CSpawnButeMgrPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength);

		void PopulateStringList(char* const * aszStrings, uint32* pcStrings, 
			const uint32 cMaxStrings, const uint32 cMaxStringLength);

	protected :

		static LTBOOL				sm_bInitted;
		static CSpawnButeMgr		sm_ButeMgr;
};

#endif // _CLIENTBUILD


#endif // _SPAWN_BUTE_MGR_H_

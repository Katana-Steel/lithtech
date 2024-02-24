// ----------------------------------------------------------------------- //
//
// MODULE  : VersionMgr.h
//
// PURPOSE : Definition of versioning manager
//
// CREATED : 11/16/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VERSION_MGR_H__
#define __VERSION_MGR_H__

#include "ltbasetypes.h"
#include "regmgr.h"
#include <string>

extern LTGUID GAMEGUID;

extern int const GAME_HANDSHAKE_VER_MAJOR;
extern int const GAME_HANDSHAKE_VER_MINOR;
extern int const GAME_HANDSHAKE_VER;

// Defines the retail mod name.  Used to
// compare mp games from any locale.  Does not get localized.
#define RETAIL_MOD_NAME "Retail"

class CVersionMgr
{
	public :

		CVersionMgr();

		void GetDisplayVersion( char* pszDisplayVersion, uint32 nDisplayVersionLen ) const;
		const char* GetBuild() const { return m_sBuild.c_str( ); }
		const uint32 GetSaveVersion();
		const char* GetNetVersion() const;
		const char* GetPatchVersion() const { return m_sPatchVersion.c_str( ); }
		const char* GetNetRegion() const { return m_sNetRegion.c_str( ); }

		void Update();

		//low violence needs to be implemented on clientside only
#ifdef _CLIENTBUILD
		inline bool	IsLowViolence() const { return m_bLowViolence; }
#endif

		void GetCDKey( char* pszCDKey, uint32 nCDKeySize );
		void SetCDKey( char const* pszCDKey );

		CRegMgr*	GetRegMgr()		{ return &m_regMgr; }

		typedef uint32 TSaveVersion;

		// The SaveVersions should be set to the build number of the released version.  The CurrentBuild
		// should always be set to the curent development build number.  The latest save version should also 
		// equal the CurrentBuild.

		static const TSaveVersion	kSaveVersion__1_0;

	private:

		static const TSaveVersion	kSaveVersion__CurrentBuild;

	public:

		void			SetCurrentSaveVersion( TSaveVersion nCurrentSaveVersion ) { m_nCurrentSaveVersion = nCurrentSaveVersion; }
		TSaveVersion	GetCurrentSaveVersion( ) const;

		void			SetLGFlags( uint8 nLGFlags ) { m_nLastLGFlags = nLGFlags; }

	protected :

		CRegMgr	m_regMgr;
		std::string m_sNetRegion;
		std::string	m_sBuild;
		std::string m_sPatchVersion;
		bool	m_bLowViolence;

		// This is the save version of the currently loading save game.
		// This number is only valid during a ILTServer::RestoreObjects call.
		TSaveVersion	m_nCurrentSaveVersion;

		uint8			m_nLastLGFlags;
};

extern CVersionMgr* g_pVersionMgr;

#endif __VERSION_MGR_H__
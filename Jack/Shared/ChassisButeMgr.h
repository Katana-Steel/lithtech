// ----------------------------------------------------------------------- //
//
// MODULE  : ChassisButeMgr.h
//
// PURPOSE : The ChassisButeMgr object
//
// CREATED : 5/9/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHASSIS_BUTE_MGR_H__
#define __CHASSIS_BUTE_MGR_H__

//
// Includes...
//

	#include "GameButeMgr.h"
	#include "TemplateList.h"
	#include "SharedMovement.h"
	
//
//	Forwards...
//

	class ChassisButeMgr;

//
// Defines...
//
#define CHASSIS_BUTE_FILE "attributes\\chassis.txt"

#define CHASSISPIECEBUTE_INVALID	255

struct ChassisPieceBute
{
	ChassisPieceBute( );
	~ChassisPieceBute( );

	bool				Init( CButeMgr &ButeMgr, char const* pszTagName, uint8 nIndex );

	std::string			m_sName;
	std::string			m_sPropType;
	std::string			m_sPropType_Target;
	std::string			m_sPropType_Replaced;
	std::string			m_sChassisSocket;
	uint8				m_nIndex;  // Used for network messages only.
	bool				m_bHeavy;
	std::string			m_sRadarType;
	std::string			m_sCarryingIcon;
	std::string			m_sPickupIcon;
	std::string			m_sFadeFX;
	std::string			m_sSpawnFX;
	std::string			m_sUnspawnFX;
};

class ChassisButeMgr : public CGameButeMgr
{
	public :	// Methods...

		ChassisButeMgr( );
		~ChassisButeMgr( );

		// Call this to get the singleton instance of the vehicle bute mgr.
		static ChassisButeMgr& Instance( );

		void			Reload() { Term(); m_buteMgr.Term(); Init(); }

        virtual LTBOOL	Init(const char* szAttributeFile="");
		void			Term( );
		bool			IsInitialized( ) const { return m_bInitialized; }

		typedef std::vector< ChassisPieceBute* > ChassisPieceButeList;
		ChassisPieceButeList const& GetChassisPieceButeList( ) const { return m_lstChassisPieceButes; }

		ChassisPieceBute* GetChassisPieceBute( const char* pszName );

		uint8			GetChassisPieceButeIndex( const char* pszName );

	private :	// Members...

		bool m_bInitialized;

		ChassisPieceButeList	m_lstChassisPieceButes;
};

#endif // __CHASSIS_BUTE_MGR_H__

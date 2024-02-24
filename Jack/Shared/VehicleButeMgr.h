// ----------------------------------------------------------------------- //
//
// MODULE  : VehicleButeMgr.h
//
// PURPOSE : The VehicleButeMgr object
//
// CREATED : 2/25/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VEHICLE_BUTE_MGR_H__
#define __VEHICLE_BUTE_MGR_H__

//
// Includes...
//

	#include "GameButeMgr.h"
	#include "TemplateList.h"
	#include "SharedMovement.h"
	
//
//	Forwards...
//

	class CVehicleButeMgr;
	extern CVehicleButeMgr *g_pVehicleButeMgr;

//
// Defines...
//

	#define VBMGR_DEFAULT_FILE	"Attributes\\VehicleButes.txt"

	#define VBMGR_MAX_FILE_PATH	64
	#define VBMGR_INVALID_ID	(uint8)-1


struct VEHICLE
{
	VEHICLE( );
	~VEHICLE( );

	bool				Init( CButeMgr &ButeMgr, char *aTagName );
	void				Save( CButeMgr &ButeMgr ) const;

	uint8				nId;

	std::string			sName;
	
	// Vehicle Model...

	std::string			sModel;
	CButeListReader		blrSkins;
	CButeListReader		blrRenderStyles;
	bool				bTranslucent;

	// Player View Model...

	std::string			sPVModel;
	CButeListReader		blrPVSkins;
	CButeListReader		blrPVRenderStyles;

	// Sounds...

	std::string			sStartUpSnd;
	std::string			sIdleSnd;
	std::string			sAccelSnd;
	std::string			sDecelSnd;
	std::string			sBrakeSnd;
	std::string			sTurnOffSnd;
	std::string			sSlowImpactSnd;
	std::string			sMedImpactSnd;
	std::string			sFastImpactSnd;
	std::string			sMPLoop;

	// Console var physics values...

	float				fVel;
	float				fAccel;
	float				fAccelTime;
	float				fMaxDecel;
	float				fDecelTime;
	float				fMaxBrake;
	float				fBrakeTime;
	float				fStoppedPercent;
	float				fMaxSpeedPercent;
	float				fMinTurnAccel;
	float				fTurnRate;
	float				fInitialTurnRate;
	float				fSurfaceSpeedUpPitch;
	float				fSurfaceSlowDownPitch;
	float				fStartMotionPitch;
	float				fStopMotionPitch;
	float				fBrakePitch;
	float				fDecelPitch;
	float				fImpactAngleSensitivity;

	LTVector			vOffset;
	std::string			sVehicleWeapon;
	bool				bCanShowCrosshair;
	bool				bContourAffectsRoll;
	uint8				nSurfaceVehicleType;
	bool				bHasReverse;

	bool				bMotorcycle;

	//used for MP respawning
	std::string			sFadeFX;
	std::string			sSpawnFX;
	std::string			sUnspawnFX;

};

class CVehicleButeMgr : public CGameButeMgr
{
	public :	// Methods...

		CVehicleButeMgr( );
		~CVehicleButeMgr( );

		// Call this to get the singleton instance of the vehicle bute mgr.
		static CVehicleButeMgr& Instance( );

		void			Reload() { Term(); m_buteMgr.Term(); Init(); }

		LTBOOL			Init( const char *szAttributeFile = VBMGR_DEFAULT_FILE );
		void			Term( );

		uint8			GetNumVehicles( ) const;

		VEHICLE*		GetVehicle( uint8 nId );
		VEHICLE*		GetVehicle( const char *pName );

		bool			SaveToFile( VEHICLE *pVehicle = NULL );

		bool			IsValidVehicleId( uint8 nId ) const;

	private :	// Members...

		typedef std::vector< VEHICLE* > VehicleList;

		VehicleList		m_lstVehicles;

};

inline bool CVehicleButeMgr::IsValidVehicleId( uint8 nId ) const
{
	return (nId < m_lstVehicles.size());
}

inline uint8 CVehicleButeMgr::GetNumVehicles( ) const
{
	return m_lstVehicles.size();
}


#endif // __VEHICLE_BUTE_MGR_H__

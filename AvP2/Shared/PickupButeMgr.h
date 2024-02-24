//*********************************************************************************
//*********************************************************************************
// Project:		Aliens vs. Predator 2
// Purpose:		Retrieves attributes from the PickupButes.txt
//*********************************************************************************
// File:		PickupButeMgr.h
// Created:		Jan. 24, 2000
// Updated:		Jan. 24, 2000
// Author:		Andy Mattingly
//*********************************************************************************
//*********************************************************************************

#ifndef		_PICKUP_BUTE_MGR_H_
#define		_PICKUP_BUTE_MGR_H_

//*********************************************************************************

#include "HierarchicalButeMgr.h"

//*********************************************************************************

#ifdef _WIN32
	#define PICKUP_BUTES_DEFAULT_FILE		"Attributes\\PickupButes.txt"
#else
	#define PICKUP_BUTES_DEFAULT_FILE		"Attributes/PickupButes.txt"
#endif

#define PICKUP_BUTES_STRING_SIZE		256

//*********************************************************************************

class CPickupButeMgr;
extern CPickupButeMgr* g_pPickupButeMgr;

//*********************************************************************************

typedef struct t_PickupButes
{
	t_PickupButes::t_PickupButes();

	// Loading and saving the pickup bute information
	void	Write(HMESSAGEWRITE hWrite);
	void	Read(HMESSAGEREAD hRead);

	// Parent information
	char	m_szParent[PICKUP_BUTES_STRING_SIZE];		// The name of the parent of this character

	// Model and skin information
	char	m_szName[PICKUP_BUTES_STRING_SIZE];							// The name of this pickup
	char	m_szModel[PICKUP_BUTES_STRING_SIZE];						// Model filename
	char	m_szSkins[MAX_MODEL_TEXTURES][PICKUP_BUTES_STRING_SIZE];	// Skin filenames

	LTVector	m_vDims;								// The default dimensions for this model
	LTFLOAT		m_fScale;								// The scale of the model
	LTFLOAT		m_fHitPoints;							// The number of hit points the pickup starts with when spawned
	char		m_szMessage[PICKUP_BUTES_STRING_SIZE];	// The message that gets sent to the character who touches the pickup
	LTFLOAT		m_fRespawnTime;							// The amount of time it takes to respawn after the pickup is gone
	LTFLOAT		m_fPickupDelay;							// A delay after the pickup is spawned or reset when it cannot be picked up
	uint32		m_nNumRespawns;							// The number of respawns allowed for this pickup
	LTBOOL		m_bCanTouch;							// Can this pickup be touched to get it?
	LTBOOL		m_bCanActivate;							// Can this pickup be activated to get it?
	LTBOOL		m_bCanDamage;							// Can this pickup be damaged?
	LTBOOL		m_bPlayerOnly;							// Can only players get this pickup?

	uint32		m_nFXType;									// The type of FX applied to this pickup
	char		m_szPickupFX[PICKUP_BUTES_STRING_SIZE];		// The type of FX applied to this pickup when picked up
	char		m_szRespawnFX[PICKUP_BUTES_STRING_SIZE];	// The type of FX applied to this pickup when respawned
	char		m_szDestroyFX[PICKUP_BUTES_STRING_SIZE];	// The type of FX applied to this pickup when destroyed

	uint32		m_nMPFXType;								// The type of FX applied to this pickup in Multiplayer
	char		m_szMPPickupFX[PICKUP_BUTES_STRING_SIZE];	// The type of FX applied to this pickup when picked up in Multiplayer
	char		m_szMPRespawnFX[PICKUP_BUTES_STRING_SIZE];	// The type of FX applied to this pickup when respawned in Multiplayer
	char		m_szMPDestroyFX[PICKUP_BUTES_STRING_SIZE];	// The type of FX applied to this pickup when destroyed in Multiplayer

	char		m_szPickupSound[PICKUP_BUTES_STRING_SIZE];	// The sound played when the pickup is retrieved
	char		m_szRespawnSound[PICKUP_BUTES_STRING_SIZE];	// The sound played when the pickup respawns

	LTBOOL		m_bMoveToGround;							// Should the object be moved to the gound
	LTBOOL		m_bSolid;									// Is the object solid
	uint32		m_nType;								// The type of pickup 
}	PickupButes;

inline t_PickupButes::t_PickupButes()
{
	memset(this, 0, sizeof(t_PickupButes));
}

//*********************************************************************************

typedef struct t_PickupFXButes
{
	t_PickupFXButes::t_PickupFXButes();

	// Loading and saving the pickup bute information
	void	Write(HMESSAGEWRITE hWrite);
	void	Read(HMESSAGEREAD hRead);

	// Parent information
	char	m_szParent[PICKUP_BUTES_STRING_SIZE];		// The name of the parent of this character

	// Model and skin information
	char	m_szName[PICKUP_BUTES_STRING_SIZE];			// The name of this FX
	char	m_szParticle[PICKUP_BUTES_STRING_SIZE];		// The name of DTX to use in the particle system

	uint32		m_nNumParticles;						// The number of particles created per second
	uint32		m_nMaxParticles;						// The maxium number of particles that can exist at one time
	LTFLOAT		m_fParticleLifetime;					// The lifetime of each particle
	LTFLOAT		m_fParticleSize;						// The size of the particle system
	uint32		m_nEmitType;							// The way the particles will emit from the pickup
	LTBOOL		m_bLight;								// Will there be a dynamic light with this FX?
	uint32		m_nLightRadius;							// The radius of the dynamic light
	LTBOOL		m_bGlow;								// Will the pickup model glow?
	LTBOOL		m_bSpin;								// Will the pickup model spin?
	LTBOOL		m_bBounce;								// Will the pickup model bounce?

}	PickupFXButes;

inline t_PickupFXButes::t_PickupFXButes()
{
	memset(this, 0, sizeof(t_PickupFXButes));
}

//*********************************************************************************

class CPickupButeMgr : public CHierarchicalButeMgr
{
	public:

		CPickupButeMgr();
		virtual ~CPickupButeMgr();

		LTBOOL		Init(ILTCSBase *pInterface, const char* szAttributeFile = PICKUP_BUTES_DEFAULT_FILE);
		void		Term();

		LTBOOL		WriteFile()						{ return m_buteMgr.Save(); }
		void		Reload()						{ m_buteMgr.Parse(m_strAttributeFile); }

		int			GetNumPickupButes()				const { return m_nNumPickupButes; }
		int			GetNumPickupFXButes()			const { return m_nNumPickupFXButes; }

		int			GetPickupSetFromName(char *szName)		const;
		int			GetPickupFXSetFromName(char *szName)	const;

		//-------------------------------------------------------------------------------//

		void		CheckPickup(int nSet)			const { ASSERT(nSet < m_nNumPickupButes); ASSERT(m_pPickupButes); }
		void		CheckPickupFX(int nSet)			const { ASSERT(nSet < m_nNumPickupFXButes); ASSERT(m_pPickupFXButes); }

		//-------------------------------------------------------------------------------//
		// Special function to fill in a PickupButes structure

		const PickupButes & GetPickupButes(int nSet)		const { CheckPickup(nSet); return m_pPickupButes[nSet];  }
		const PickupButes & GetPickupButes(char *szName)	const;

		const PickupFXButes & GetPickupFXButes(int nSet)		const { CheckPickupFX(nSet); return m_pPickupFXButes[nSet];  }
		const PickupFXButes & GetPickupFXButes(char *szName)	const;

	private:

		// Loading functions (called from Init)
		void LoadPickupButes(int nSet, PickupButes &butes);
		void LoadPickupFXButes(int nSet, PickupFXButes &butes);

		// Helper functions to get at the values easier
		CString		GetStringAttrib(int nSet, char *szAttrib);
		LTVector	GetVectorAttrib(int nSet, char *szAttrib);
		CPoint		GetPointAttrib(int nSet, char *szAttrib);
		int			GetIntAttrib(int nSet, char *szAttrib);
		double		GetDoubleAttrib(int nSet, char *szAttrib);

		// Helper functions to get at the values easier
		CString		GetFXStringAttrib(int nSet, char *szAttrib);
		LTVector	GetFXVectorAttrib(int nSet, char *szAttrib);
		CPoint		GetFXPointAttrib(int nSet, char *szAttrib);
		int			GetFXIntAttrib(int nSet, char *szAttrib);
		double		GetFXDoubleAttrib(int nSet, char *szAttrib);

	private:

		// Pickup attribute set variables
		int				m_nNumPickupButes;
		PickupButes		*m_pPickupButes;

		// Pickup FX attribute set variables
		int				m_nNumPickupFXButes;
		PickupFXButes	*m_pPickupFXButes;
};

////////////////////////////////////////////////////////////////////////////
//
// CCharacterButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use WeaponMgr
//
////////////////////////////////////////////////////////////////////////////

#ifndef _CLIENTBUILD

#include "iobjectplugin.h"

class CPickupButeMgrPlugin : public IObjectPlugin
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
		static CPickupButeMgr		sm_ButeMgr;
};

#endif // _CLIENTBUILD


#endif // _CHARACTER_BUTE_MGR_H_

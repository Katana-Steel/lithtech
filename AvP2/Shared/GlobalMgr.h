// ----------------------------------------------------------------------- //
//
// MODULE  : GlobalMgr.h
//
// PURPOSE : Definition of global definitions
//
// CREATED : 7/07/99
//
// ----------------------------------------------------------------------- //

#ifndef __GLOBAL_MGR_H__
#define __GLOBAL_MGR_H__

class CFXButeMgr;
class CWeaponMgr;
class CModelButeMgr;
class CSurfaceMgr;
class CMissionMgr;
class CCharacterButeMgr;
class CDebrisMgr;
class CPickupButeMgr;
class CAnimationButeMgr;
class CSoundButeMgr;
class CObjectiveButeMgr;
class CAIAnimButeMgr;


#ifdef _CLIENTBUILD

	class CClientButeMgr;
	class CClientSoundMgr;

	namespace VisionMode
	{
		class ButeMGR;
	}

#else

	class CAIButeMgr;
	class CAttachButeMgr;

	class CServerSoundMgr;
	class CPropTypeMgr;
	class CSpawnButeMgr;

#endif

// ----------------------------------------------------------------------- //

class CGlobalMgr
{
	public :

		CGlobalMgr()
		:	m_pFXButeMgr(LTNULL),
			m_pWeaponMgr(LTNULL),
			m_pModelButeMgr(LTNULL),
			m_pSurfaceMgr(LTNULL),
			m_pMissionMgr(LTNULL),
			m_pCharacterButeMgr(LTNULL),
			m_pDebrisMgr(LTNULL),
			m_pPickupButeMgr(LTNULL),
			m_pAnimationButeMgr(LTNULL),
			m_pSoundButeMgr(LTNULL),
			m_pObjectiveButeMgr(LTNULL),
#ifdef _CLIENTBUILD
			m_pClientButeMgr(LTNULL),
			m_pClientSoundMgr(LTNULL),
			m_pVisionButeMgr(LTNULL)
#else
			m_pAIButeMgr(LTNULL),
			m_pAttachButeMgr(LTNULL),
			m_pServerSoundMgr(LTNULL),
			m_pPropTypeMgr(LTNULL),
			m_pSpawnButeMgr(LTNULL),
			m_pAIAnimButeMgr(LTNULL)
#endif
		{}
		
		virtual ~CGlobalMgr();

		LTBOOL Init(ILTCSBase *pInterface);
		void   Term();

	private :

		CFXButeMgr*				m_pFXButeMgr;			// Same as g_pFXButeMgr
		CWeaponMgr*				m_pWeaponMgr;			// Same as g_pWeaponMgr
		CModelButeMgr*			m_pModelButeMgr;		// Same as g_pModelButeMgr
		CSurfaceMgr*			m_pSurfaceMgr;			// Same as g_pSurfaceMgr
		CMissionMgr*			m_pMissionMgr;			// Same as g_pMissionMgr
		CCharacterButeMgr*		m_pCharacterButeMgr;	// Same as g_pCharacterButeMgr
		CDebrisMgr*				m_pDebrisMgr;			// Same as g_pDebrisMgr
		CPickupButeMgr*			m_pPickupButeMgr;		// Same as g_pPickupButeMgr
		CAnimationButeMgr*		m_pAnimationButeMgr;	// Same as g_pAnimationButeMgr
		CSoundButeMgr*			m_pSoundButeMgr;		// Same as g_pSoundButeMgr
		CObjectiveButeMgr*		m_pObjectiveButeMgr;	// Same as g_pObjectiveButeMgr

#ifdef _CLIENTBUILD

		// Client only global managers
		CClientButeMgr*			m_pClientButeMgr;	// Same as g_pClientButeMgr
		CClientSoundMgr*		m_pClientSoundMgr;	// Same as g_pClientSoundMgr
		VisionMode::ButeMGR*	m_pVisionButeMgr;	// Manage the vision mode data

#else

		// Server only global managers
		CAIButeMgr*				m_pAIButeMgr;		// Same as g_pAIButeMgr
		CAttachButeMgr*			m_pAttachButeMgr;	// Same as g_pAttachButeMgr
		CServerSoundMgr*		m_pServerSoundMgr;	// Same as g_pServerSoundMgr
		CPropTypeMgr*			m_pPropTypeMgr;		// Same as g_pPropTypeMgr
		CSpawnButeMgr*			m_pSpawnButeMgr;	// Same as g_pSpawnButeMgr
		CAIAnimButeMgr*			m_pAIAnimButeMgr;	// Same as g_pAIAnimButeMgr

#endif
};

#endif // __GLOBAL_MGR_H__

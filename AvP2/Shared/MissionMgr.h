// ----------------------------------------------------------------------- //
//
// MODULE  : MissionMgr.h
//
// PURPOSE : MissionMgr definition - Controls attributes of all Missions
//
// CREATED : 11/9/2000
//
// ----------------------------------------------------------------------- //

#ifndef __MISSION_MGR_H__
#define __MISSION_MGR_H__

#include "GameButeMgr.h"
#include "basetypes_de.h"
#include "TemplateList.h"
#include "HierarchicalButeMgr.h"
#include "ClientServerShared.h"


#ifdef _WIN32
	#define MISSION_DEFAULT_FILE		"Attributes\\Missions.txt"
#else
	#define MISSION_DEFAULT_FILE		"Attributes/Missions.txt"
#endif

#define MMGR_INVALID_ID			255	// Invalid id
#define MISSION_DEFAULT_ID		0	// First Level in Mission attribute file

class CMissionMgr;
extern CMissionMgr* g_pMissionMgr;

#define MISSION_BUTES_STRING_SIZE			64

typedef struct MISSION
{
	MISSION();

	uint32		m_nId;
	char		m_szName[MISSION_BUTES_STRING_SIZE];
	int			m_nNextDesc;
	char		m_szNextLevel[MISSION_BUTES_STRING_SIZE];
	LTBOOL		m_bResetHealthAndArmor;
	LTFLOAT		m_fScreenFadeTime;

}	MissionButes;

inline MISSION::MISSION()
{
	memset(this, 0, sizeof(MISSION));
}

typedef struct MISSIONSTART
{
	MISSIONSTART();
	void	Init(CButeMgr & buteMgr, char* aTagName, int nNum);

	int		m_nNameId;
	int		m_nDescId;
	char	m_szWorld[MISSION_BUTES_STRING_SIZE];

}	MissionStartButes;

inline MISSIONSTART::MISSIONSTART()
{
	memset(this, 0, sizeof(MISSIONSTART));
}


#define MAX_CAMPAIGN_MISSIONS	7

typedef struct CAMPAIGN
{
	CAMPAIGN();
	void	Init(CButeMgr & buteMgr, char* aTagName);

	int				  m_nNumMissions;
	MissionStartButes m_Missions[MAX_CAMPAIGN_MISSIONS];
}	CampaignButes;

inline CAMPAIGN::CAMPAIGN()
{
	memset(this, 0, sizeof(CAMPAIGN));
}



class CMissionMgr : public CGameButeMgr
{
	public :

		CMissionMgr();
		~CMissionMgr();

		//-------------------------------------------------------------------------------//
		// Utility functions
		DBOOL			Init(ILTCSBase *pInterface, const char* szAttributeFile=MISSION_DEFAULT_FILE);
		void			Term();
		void			Reload(ILTCSBase *pInterface) { Term(); Init(pInterface); }
		int				GetNumSets()					const { return m_nNumSets; }

		//-------------------------------------------------------------------------------//
		// Accessor functions
		uint32		GetMissionId(int nSet)				const { Check(nSet); return m_pButeSets[nSet].m_nId; }

		CString		GetMissionName(int nSet)			const { Check(nSet); return m_pButeSets[nSet].m_szName; }
		int			GetMissionDescription(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_nNextDesc; }
		CString		GetNextLevelName(int nSet)			const { Check(nSet); return m_pButeSets[nSet].m_szNextLevel; }

		LTBOOL		GetResetHealthAndArmor(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_bResetHealthAndArmor; }
		
		LTFLOAT		GetScreenFadeTime(int nSet)			const { Check(nSet); return m_pButeSets[nSet].m_fScreenFadeTime; }

		//-------------------------------------------------------------------------------//
		// Special functions
		void		Check(int nSet)					const { ASSERT(nSet < m_nNumSets); ASSERT(m_pButeSets); }
		int			GetSetFromName(char *szName)	const;
		int			GetSetFromNextWorld(char *szWorld)	const;

		//-------------------------------------------------------------------------------//
		// Special function to fill in a MissionButes structure
		const MissionButes & GetMissionButes(int nSet)		const { Check(nSet); return m_pButeSets[nSet];  }
		const MissionButes & GetMissionButes(char *szName)	const;

		const CampaignButes & GetCampaignButes(Species species)		const { if( species < 0 || species > 2) species = Marine; return m_Campaigns[species];  }

	protected :
		// Loading functions (called from Init)
		void LoadMissionButes(int nSet, MissionButes &butes);
		void LoadCampaignButes(int nCampaign, CampaignButes &butes);

		// Helper functions to get at the values easier
		CString		GetStringAttrib(int nSet, char *szAttrib);
		LTVector	GetVectorAttrib(int nSet, char *szAttrib);
		CPoint		GetPointAttrib(int nSet, char *szAttrib);
		int			GetIntAttrib(int nSet, char *szAttrib);
		double		GetDoubleAttrib(int nSet, char *szAttrib);

		int				m_nNumSets;
		MissionButes*	m_pButeSets;

		CampaignButes	m_Campaigns[3];

};


// Gets the campaign mission offset based on the world name.
// Returns -1 if no campaign has that mission.
inline int GetCampaignMission(const CampaignButes & campaign, const char * szWorldName)
{
	for( const MissionStartButes * pButes = campaign.m_Missions; pButes < campaign.m_Missions + campaign.m_nNumMissions; ++pButes )
	{
		if( 0 == stricmp(pButes->m_szWorld, szWorldName) )
		{
			return (pButes - campaign.m_Missions);
		}
	}

	return -1;
}

////////////////////////////////////////////////////////////////////////////
//
// CMissionButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use MissionMgr
//
////////////////////////////////////////////////////////////////////////////

#ifndef _CLIENTBUILD

#include "iobjectplugin.h"

class CMissionButeMgrPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT	PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength);

	protected :

		static LTBOOL		sm_bInitted;
		static CMissionMgr	sm_ButeMgr;
};

#endif // _CLIENTBUILD


#endif // __MISSION_MGR_H__

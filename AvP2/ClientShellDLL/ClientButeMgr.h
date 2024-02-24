// ----------------------------------------------------------------------- //
//
// MODULE  : ClientButeMgr.h
//
// PURPOSE : ClientButeMgr definition - Client-side attributes
//
// CREATED : 2/02/99
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_BUTE_MGR_H__
#define __CLIENT_BUTE_MGR_H__

#include "GameButeMgr.h"
#include "basetypes_de.h"


#define CBMGR_DEFAULT_FILE "Attributes\\ClientButes.txt"

class CClientButeMgr;
extern CClientButeMgr* g_pClientButeMgr;

class CClientButeMgr : public CGameButeMgr
{
	public :

		typedef std::vector<int> DebugLevels;

	public :
	
		CClientButeMgr();
		~CClientButeMgr();

		DBOOL		Init(ILTCSBase *pInterface, const char* szAttributeFile=CBMGR_DEFAULT_FILE);
		void		Term();

		DBOOL		WriteFile() { return m_buteMgr.Save(); }
		void		Reload()    { m_buteMgr.Parse(m_strAttributeFile); }

		int			GetNumCheatAttributes() const { return m_nNumCheatAttributes; }
		CString		GetCheat(DBYTE nCheatNum);

		float		GetReverbAttributeFloat(char* pAttribute);

		int			GetCameraAttributeInt(char* pAttribute);
		float		GetCameraAttributeFloat(char* pAttribute);
		CString		GetCameraAttributeString(char* pAttribute);

		int			GetGameAttributeInt(char* pAttribute);
		float		GetGameAttributeFloat(char* pAttribute);
		CString		GetGameAttributeString(char* pAttribute);

		float		GetWeatherAttributeFloat(char* pAttribute);
		CString		GetWeatherAttributeString(char* pAttribute);

		int			GetNumLevels() const { return m_nNumLevels; }
		int			GetLevelAttributeInt(DBYTE nLevelNum, char* pAttribute);
		float		GetLevelAttributeFloat(DBYTE nLevelNum, char* pAttribute);
		CString		GetLevelAttributeString(DBYTE nLevelNum, char* pAttribute);

		CString		GetInterfaceAttributeString(char* pAttribute);

		int			GetNumDebugKeys() const { return m_nNumDebugKeys; }
		int			GetNumDebugLevels(DBYTE nDebugNum) const;
		int			GetDebugKey(DBYTE nDebugNum);
		CString		GetDebugName(DBYTE nDebugNum);
		CString		GetDebugString(DBYTE nDebugNum, DBYTE nLevel);
		CString		GetDebugTitle(DBYTE nDebugNum, DBYTE nLevel);

		const LTVector & GetPredAuraColor() const { return m_vPredAuraColor; }
		const LTVector & GetAlienAuraColor() const { return m_vAlienAuraColor; }
		const LTVector & GetHumanAuraColor() const { return m_vHumanAuraColor; }
		const LTVector & GetSelHumanAuraColor() const { return m_vSelHumanAuraColor; }

		const LTVector & GetDeadPredAuraColor() const { return m_vDeadPredAuraColor; }
		const LTVector & GetDeadAlienAuraColor() const { return m_vDeadAlienAuraColor; }
		const LTVector & GetDeadHumanAuraColor() const { return m_vDeadHumanAuraColor; }
		const LTVector & GetDeadSelHumanAuraColor() const { return m_vDeadSelHumanAuraColor; }


		int GetQuickSaveKey() const { return m_nQuickSaveKey; }
		int GetQuickLoadKey() const { return m_nQuickLoadKey; }
		int GetCinematicSkipKey() const { return m_nCinematicSkipKey; }
		int GetEscapeKey() const { return m_nEscapeKey; }
		int GetPauseKey() const { return m_nPauseKey; }

	protected :

	private :

		DBYTE	m_nNumCheatAttributes;
		DBYTE	m_nNumLevels;
		DBYTE	m_nNumDebugKeys;

		DebugLevels m_aNumDebugLevels;

		LTVector m_vPredAuraColor;
		LTVector m_vAlienAuraColor;
		LTVector m_vHumanAuraColor;
		LTVector m_vSelHumanAuraColor;

		LTVector m_vDeadPredAuraColor;
		LTVector m_vDeadAlienAuraColor;
		LTVector m_vDeadHumanAuraColor;
		LTVector m_vDeadSelHumanAuraColor;

		int m_nQuickSaveKey;
		int m_nQuickLoadKey;
		int m_nCinematicSkipKey;
		int m_nEscapeKey;
		int m_nPauseKey;
};



#endif // __CLIENT_BUTE_MGR_H__


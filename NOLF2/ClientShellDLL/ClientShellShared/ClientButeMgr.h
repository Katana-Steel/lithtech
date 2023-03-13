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
#include "ltbasetypes.h"


#define CBMGR_DEFAULT_FILE "Attributes\\ClientButes.txt"

class CClientButeMgr;
extern CClientButeMgr* g_pClientButeMgr;

class CClientButeMgr : public CGameButeMgr
{
	public :

		CClientButeMgr();
		CClientButeMgr(const CClientButeMgr&)=delete;
		CClientButeMgr(CClientButeMgr&&)=delete;
		~CClientButeMgr();
		// assignment operators
		CClientButeMgr& operator=(const CClientButeMgr&) = delete;
		CClientButeMgr& operator=(CClientButeMgr&&) = delete;
        LTBOOL       Init(const char* szAttributeFile=CBMGR_DEFAULT_FILE);
		void		Term();

        LTBOOL       WriteFile() { return m_buteMgr.Save(); }
		void		Reload()    { m_buteMgr.Parse(m_strAttributeFile); }

		int			GetNumCheatAttributes() const { return m_nNumCheatAttributes; }
        void		GetCheat(uint8 nCheatNum, char *pBuf, uint16 nBufLen);

		float		GetReverbAttributeFloat(const char* pAttribute);

		int			GetCameraAttributeInt(const char* pAttribute);
		float		GetCameraAttributeFloat(const char* pAttribute);
		void		GetCameraAttributeString(const char* pAttribute, char *pBuf, uint16 nBufLen);

		int			GetGameAttributeInt(const char* pAttribute);
		float		GetGameAttributeFloat(const char* pAttribute);
		void		GetGameAttributeString(const char* pAttribute, char *pBuf, uint16 nBufLen);

		float		GetWeatherAttributeFloat(const char* pAttribute);
		void		GetWeatherAttributeString(const char* pAttribute, char *pBuf, uint16 nBufLen);

		float		GetSpecialFXAttributeFloat(const char* pAttribute);
		void		GetSpecialFXAttributeString(const char* pAttribute, char *pBuf, uint16 nBufLen);

		float		GetBreathFXAttributeFloat(const char* pAttribute);
		int			GetBreathFXAttributeInt(const char* pAttribute);
		void		GetBreathFXAttributeString(const char* pAttribute, char *pBuf, uint16 nBufLen);
        LTVector     GetBreathFXAttributeVector(const char* pAttribute);

		void		GetInterfaceAttributeString(const char* pAttribute, char *pBuf, uint16 nBufLen);
		float		GetInterfaceAttributeFloat(const char* pAttribute, float fDef);

		int			GetNumSingleWorldPaths()	const { return m_nNumSingleWorldPaths; }
		int			GetNumMultiWorldPaths()	const { return m_nNumMultiWorldPaths; }
        void        GetWorldPath(uint8 nPath, char* pBuf, int nBufLen, LTBOOL bSingle = LTTRUE);

		int			GetNumDebugKeys() const { return m_nNumDebugKeys; }
		int			GetNumDebugLevels(uint8 nDebugKey) const;
		int			GetDebugKeyId(uint8 nDebugKey);
		int			GetDebugModifierId(uint8 nDebugKey);
		void		GetDebugName(uint8 nDebugKey, char * pBuf, uint16 nBufLen);
		void		GetDebugString(uint8 nDebugKey, uint8 nLevel, char * pBuf, uint16 nBufLen);
		void		GetDebugTitle(uint8 nDebugKey, uint8 nLevel, char * pBuf, uint16 nBufLen);

		uint32		GetNumGlowMappings() const	{ return m_nNumGlowMappings; }
		void		GetDefaultGlowRS(char* pBuf, uint16 nBufLen);
		void		GetNoGlowRS(char* pBuf, uint16 nBufLen);
		void		GetGlowMappingRS(uint32 nMapping, char* pMapBuf, uint16 nMapLen, char* pMapToBuf, uint16 nMapToLen);

	protected :

	private :

		uint32	m_nNumGlowMappings;

        uint8   m_nNumCheatAttributes;
		int		m_nNumSingleWorldPaths;
		int		m_nNumMultiWorldPaths;

		int		m_nNumDebugKeys;
		int*	m_aNumDebugLevels;

};



#endif // __CLIENT_BUTE_MGR_H__
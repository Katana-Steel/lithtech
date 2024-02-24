// ----------------------------------------------------------------------- //
//
// MODULE  : GameButeMgr.h
//
// PURPOSE : GameButeMgr definition - Base class for all bute mgrs
//
// CREATED : 3/30/99
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_BUTE_MGR_H__
#define __GAME_BUTE_MGR_H__

#include "butemgr.h"

void GBM_DisplayError(const char* szMsg);

// ----------------------------------------------------------------------- //
//
// Character pointer hash map definition...
//
// ----------------------------------------------------------------------- //

struct eqstr_nocase
{
  bool operator()(const char* s1, const char* s2) const
  {
	return stricmp(s1, s2) == 0;
  }
};

struct GBM_hash_str_nocase
{
	// Copied for stl-port's std::hash<const char*>.
	// Added tolower function on the string.
	unsigned long operator()(const char* str) const 
	{
	  unsigned long hash = 0; 
	  for ( ; *str; ++str)
		  hash = 5*hash + tolower(*str);
  
	  return hash;
	}
};

typedef std::hash_map<const char *,int, GBM_hash_str_nocase, eqstr_nocase> IndexTable;

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

class CGameButeMgr
{
	public :

        virtual LTBOOL Init(ILTCSBase *pInterface, const char* szAttributeFile="") = 0;

		CGameButeMgr()
		{
			m_buteMgr.Init(GBM_DisplayError);
            m_pCryptKey = LTNULL;
            m_bInRezFile = LTTRUE;
		}

		virtual ~CGameButeMgr() { m_buteMgr.Term(); }

		CString GetErrorString() { return m_buteMgr.GetErrorString(); }
        inline void SetInRezFile(LTBOOL bInRezFile) { m_bInRezFile = bInRezFile; }

	protected :

		CString		m_strAttributeFile;
		CButeMgr	m_buteMgr;

		char*		m_pCryptKey;
        LTBOOL       m_bInRezFile;

        LTBOOL       Parse(ILTCSBase *pInterface, const char* sButeFile);
};

#endif // __GAME_BUTE_MGR_H__



// Console variable tracker.. makes it easy to get and set the value of
// console variables.

#ifndef __CVARTRACK_H__
#define __CVARTRACK_H__


	#include "server_de.h"


	class CVarTrack
	{
	public:

		CVarTrack()
			: m_hVar(LTNULL),
			  m_pServerDE(LTNULL),
			  m_pVarName(LTNULL)	{}

		CVarTrack(ServerDE *pServerDE, const char *pVarName, float fStartVal)
			: m_hVar(LTNULL),
			  m_pServerDE(LTNULL),
			  m_pVarName(LTNULL)	{ Init(pServerDE,pVarName,fStartVal); }

		CVarTrack(ServerDE *pServerDE, const char *pVarName, const char * pStartVal)
			: m_hVar(LTNULL),
			  m_pServerDE(LTNULL),
			  m_pVarName(LTNULL)	{ Init(pServerDE,pVarName,pStartVal); }
					
		LTBOOL	Init(ServerDE *pServerDE, const char *pVarName, float fStartVal)
		{
			return Init(pServerDE, pVarName, LTNULL, fStartVal);
		}

		LTBOOL  Init(ServerDE *pServerDE, const char *pVarName, const char *pStartVal)
		{
			return Init(pServerDE, pVarName, pStartVal, 0.0f);
		}

		LTBOOL  Init(ServerDE *pServerDE, const char *pVarName)
		{
			return Init(pServerDE, pVarName, LTNULL, 0.0f);
		}

		LTBOOL  Init(ServerDE *pServerDE, const char *pVarName, const char *pStartVal, float fStartVal)
		{
			char tempStr[128];

			m_pVarName = pVarName;
			if(!pStartVal)
			{
				sprintf(tempStr, "%5f", fStartVal);
				pStartVal = tempStr;
			}

			m_hVar = pServerDE->GetGameConVar( const_cast<char*>(pVarName) );
			if(!m_hVar)
			{
				pServerDE->SetGameConVar( const_cast<char*>(pVarName), const_cast<char*>(pStartVal) );
				m_hVar = pServerDE->GetGameConVar( const_cast<char*>(pVarName) );
				if(!m_hVar)
				{
					return LTFALSE;
				}
			}
					
			m_pServerDE = pServerDE;
			return LTTRUE;
		}

		LTBOOL IsInitted() const
		{
			return !!m_pServerDE;
		}

		float GetFloat(float defVal=0.0f)
		{
			if(m_pServerDE && m_hVar)
				return m_pServerDE->GetVarValueFloat(m_hVar);
			else
				return defVal;
		}

		const char*	GetStr(const char *pDefault="")
		{
			const char *pRet;

			if(m_pServerDE && m_hVar)
			{
				if(pRet = m_pServerDE->GetVarValueString(m_hVar))
					return pRet;
			}
			return pDefault;
		}

		void SetFloat(float val)
		{
			char str[128];

			if(!m_pServerDE || !m_pVarName)
				return;

			sprintf(str, "%f", val);
			m_pServerDE->SetGameConVar(const_cast<char*>(m_pVarName), const_cast<char*>(str) );
		}

		void SetStr(const char *pStr)
		{
			if(!m_pServerDE || !m_pVarName)
				return;

			m_pServerDE->SetGameConVar(const_cast<char*>(m_pVarName), const_cast<char*>(pStr));
		}

	protected:
		
		HCONVAR		m_hVar;
		ServerDE	*m_pServerDE;
		const char	*m_pVarName;
	};


#endif  // __CVARTRACK_H__





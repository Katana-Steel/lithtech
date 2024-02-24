	// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef AIANIMATIONMGR_H
#define AIANIMATIONMGR_H

#pragma warning(disable : 4786)

#include "HierarchicalButeMgr.h"

#include <deque>
#include <string>
#include <hash_map>
#include <vector>

extern class CAIAnimButeMgr* g_pAIAnimButeMgr;
extern const char * g_szAIAnimButeMgrFile;

struct AIAnimSet
{
	struct Element
	{
		std::string m_strAnimName;
		LTFLOAT     m_fAnimWeight;

		Element()
			: m_fAnimWeight(0.0f) {}
	};

	typedef std::deque<Element> ElementList;

	std::string			m_strName;
	LTFLOAT				m_fPlayChance;
	ElementList			m_Elements;

	AIAnimSet()
		: m_fPlayChance(0.0f) {}

	const std::string & GetRandomAnim() const;
};

class CAIAnimButeMgr : public CHierarchicalButeMgr
{

	public :

		// MSVC6.x spews many warnings if you use this definition.  
		// The decorated names get too long.
		// typedef std::hash_map<std::string,int> AnimSetIndex;

		struct AnimSetIndex // hack to get around MSVC
		{
			typedef std::hash_map<std::string,int> Table;
			Table m_Table;
		};

		
		typedef std::vector<AIAnimSet> AnimSetList;		
		typedef std::hash_map<std::string, int> AnimSetNames;
		typedef std::hash_map<std::string, AnimSetIndex > AnimSetTable;

	public :

		CAIAnimButeMgr()
		{
			g_pAIAnimButeMgr = this;
			m_bInitted = FALSE;
		}

		~CAIAnimButeMgr()
		{
			g_pAIAnimButeMgr = NULL;
		}

        LTBOOL	Init(ILTCSBase *pInterface, const char* szAttributeFile = g_szAIAnimButeMgrFile);


		const AIAnimSet * GetAnimSetPtr(std::string strAIType, std::string strSetType) const;
		const AIAnimSet * GetAnimSetPtr(std::string strAnimSet) const;


	private:

		// Private functions to facilate loading the butes.
		static bool CountAnimSetButes(const char *szTagName, void *pData);
		static bool LoadAnimSetButes(const char *szTagName, void *pData);
		void LoadAnimSet(const char *szTagName, AIAnimSet * pAnimSet);
	
		static bool LoadAnimTableKeys(const char * szKeyName, CButeMgr::CSymTabItem* pItem, void* pData );
		static bool LoadAnimTableButes(const char *szTagName, void *pData);

		BOOL m_bInitted;

		AnimSetTable m_AnimSetTable;
		AnimSetNames m_AnimSetNames;
		AnimSetList  m_AnimSets;

		int m_nNumAnimSets; // This is really just a temporary for the static call back functions.
};


#endif // AIANIMATIONMGR_H

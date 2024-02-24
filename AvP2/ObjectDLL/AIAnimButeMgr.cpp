// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIAnimButeMgr.h"

class CAIAnimButeMgr* g_pAIAnimButeMgr = LTNULL;

// Our text file.
#ifdef _WIN32
	const char * g_szAIAnimButeMgrFile = "Attributes\\AIAnimations.txt";
#else
	const char * g_szAIAnimButeMgrFile = "Attributes/AIAnimations.txt";
#endif

// Type names.
const char * g_szType = "Type";
const char * g_szAnimSetType = "AnimSet";
const char * g_szAnimTableType = "AnimTable";


// Attribute names.
const char * g_szAnimNameAttrib = "Anim";
const char * g_szAnimWeightAttrib = "Weight";

const char * g_szAnimPlayChanceAttrib = "PlayChance";

static void MakeLower(std::string * pStr)
{
	if( !pStr )
		return;

	std::transform(pStr->begin(), pStr->end(), pStr->begin(), tolower);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINES:	CAIButeMgr::GetRandomAnim()
//
//	PURPOSE:	Pick a random weighted animation.
//
// ----------------------------------------------------------------------- //

const std::string & AIAnimSet::GetRandomAnim() const
{
	static std::string strNull;

	if( m_Elements.empty() )
		return strNull;

	if(m_Elements.size() == 1)
	{
		return m_Elements.front().m_strAnimName;
	}

	// Get a random value to test with
	const LTFLOAT fRand = GetRandom(0.0f, 1.0f);

	// And find the animation that matches that.
	for( ElementList::const_iterator iter = m_Elements.begin(); iter != m_Elements.end(); ++iter )
	{
		if( iter->m_fAnimWeight >= fRand )
			return iter->m_strAnimName;
	}

	// Otherwise, if we got here... just return the last sound
	return m_Elements.back().m_strAnimName;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINES:	CAIButeMgr::LoadAnimTable()
//
//	PURPOSE:	Sets up the m_AnimSets
//
// ----------------------------------------------------------------------- //


bool CAIAnimButeMgr::CountAnimSetButes(const char *szTagName, void *pData)
{
	if(!szTagName || !pData) return true;

	CAIAnimButeMgr *pButeMgr = (CAIAnimButeMgr*)pData;

	CString cstrTypeName = pButeMgr->GetString(szTagName, g_szType);

	if( 0 == cstrTypeName.CompareNoCase(g_szAnimSetType) )
	{
		++pButeMgr->m_nNumAnimSets;
	}

	// Keep iterating.
	return true;
}

bool CAIAnimButeMgr::LoadAnimSetButes(const char *szTagName, void *pData)
{
	if(!szTagName || !pData) return true;

	CAIAnimButeMgr *pButeMgr = (CAIAnimButeMgr*)pData;

	CString cstrTypeName = pButeMgr->GetString(szTagName, g_szType);

	if( 0 == cstrTypeName.CompareNoCase(g_szAnimSetType) )
	{
		pButeMgr->LoadAnimSet( szTagName, &pButeMgr->m_AnimSets[pButeMgr->m_nNumAnimSets] );

		const std::string & strSetName = pButeMgr->m_AnimSets[pButeMgr->m_nNumAnimSets].m_strName;
		pButeMgr->m_AnimSetNames[strSetName] = pButeMgr->m_nNumAnimSets;

		++pButeMgr->m_nNumAnimSets;
	}

	// Keep iterating.
	return true;
}

void CAIAnimButeMgr::LoadAnimSet(const char *szTagName, AIAnimSet * pAnimSet)
{
	// Local variables.
	char aAttribName[64];
	LTFLOAT fWeightAccum = 0;

	_ASSERT( szTagName && pAnimSet );
	if( !szTagName || !pAnimSet )
		return;

	// Remember our name.
	pAnimSet->m_strName = szTagName;
	MakeLower(&pAnimSet->m_strName);

	// Get the animation names and weights
	int nNumAnims = 0;
	sprintf(aAttribName, "%s%d", g_szAnimNameAttrib, nNumAnims);

	while( Exist(szTagName, aAttribName) )
	{
		CString cstrAnimName = GetString(szTagName, aAttribName, CString(""));

		if( !cstrAnimName.IsEmpty() )
		{
			// Store the anim element.
			pAnimSet->m_Elements.push_back(AIAnimSet::Element());

			pAnimSet->m_Elements.back().m_strAnimName = cstrAnimName;

			// Read the weight value
			sprintf(aAttribName, "%s%d", g_szAnimWeightAttrib, nNumAnims);
			const LTFLOAT fWeight = (float)GetDouble(szTagName, aAttribName, 1.0f);

			pAnimSet->m_Elements.back().m_fAnimWeight = fWeight;

			fWeightAccum += fWeight;
		}

		++nNumAnims;
		sprintf(aAttribName, "%s%d", g_szAnimNameAttrib, nNumAnims);
	}

	// The weight values are evened out into a range that's easy to work with (to speed up calculations later)
	LTFLOAT fCurrentWeightAccum = 0.0f;
	for( AIAnimSet::ElementList::iterator iter = pAnimSet->m_Elements.begin(); 
	     iter != pAnimSet->m_Elements.end(); ++iter )
	{
		fCurrentWeightAccum += iter->m_fAnimWeight / fWeightAccum;

		iter->m_fAnimWeight = fCurrentWeightAccum;
	}

	pAnimSet->m_fPlayChance = (LTFLOAT)GetDouble(szTagName, g_szAnimPlayChanceAttrib, 1.0f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINES:	CAIButeMgr::LoadAnimTable()
//
//	PURPOSE:	Sets up the m_AnimSetTable
//
// ----------------------------------------------------------------------- //

struct AnimTableKeysContext
{
	CAIAnimButeMgr::AnimSetIndex   * pIndex;
	CAIAnimButeMgr * pButeMgr;
	
	AnimTableKeysContext(CAIAnimButeMgr::AnimSetIndex * index_ptr, CAIAnimButeMgr * butemgr_ptr )
		: pIndex(index_ptr),
		  pButeMgr(butemgr_ptr) {}
};


bool CAIAnimButeMgr::LoadAnimTableKeys(const char * szKeyName, CButeMgr::CSymTabItem* pItem, void* pData )
{
	AnimTableKeysContext *pContext = (AnimTableKeysContext*)pData;

	if( pItem->SymType == CButeMgr::StringType )
	{
		const CString & cstrSetName = *pItem->data.s;
		
		std::string strSetName( cstrSetName );
		MakeLower(&strSetName);

		AnimSetNames::const_iterator iter = pContext->pButeMgr->m_AnimSetNames.find( strSetName );

		if( iter != pContext->pButeMgr->m_AnimSetNames.end() )
		{
			const int offset = iter->second;
			std::string strKeyName(szKeyName);
			MakeLower(&strKeyName);

			(*pContext->pIndex).m_Table[strKeyName] = offset;
		}
	}

	// Keep iterating.
	return true;
}


bool CAIAnimButeMgr::LoadAnimTableButes(const char *szTagName, void *pData)
{
	if(!szTagName || !pData) return true;

	CAIAnimButeMgr *pButeMgr = (CAIAnimButeMgr*)pData;

	CString cstrTypeName = pButeMgr->GetString(szTagName, g_szType);

	if( 0 == cstrTypeName.CompareNoCase(g_szAnimTableType) )
	{
		std::string strTagName(szTagName);
		MakeLower(&strTagName);

		AnimTableKeysContext context(&(pButeMgr->m_AnimSetTable[strTagName]),pButeMgr);

		pButeMgr->m_buteMgr.GetKeys( szTagName, LoadAnimTableKeys, &context );
	}

	// Keep iterating.
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::Init()
//
//	PURPOSE:	Initializes the butes.
//
// ----------------------------------------------------------------------- //

LTBOOL	CAIAnimButeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile /* = g_szAIAnimButeMgrFile */)
{

	if( !Parse(pInterface, szAttributeFile) )
	{
		return LTFALSE;
	}

	// Load the animation sets.
	m_nNumAnimSets = 0;
	m_buteMgr.GetTags(CountAnimSetButes,this);

	AnimSetList old_sets(m_nNumAnimSets);
	m_AnimSets.swap( old_sets );
	
	m_nNumAnimSets = 0;
	m_buteMgr.GetTags(LoadAnimSetButes,this);

	// Load the animation reference tables (must be done after 
	// the sets have been loaded).
	m_buteMgr.GetTags(LoadAnimTableButes,this);

	// Clean up our memory.
	m_buteMgr.Term();

	return LTTRUE;

}

const AIAnimSet * CAIAnimButeMgr::GetAnimSetPtr(std::string strAIType, std::string strSetType) const
{
	MakeLower(&strAIType);
	MakeLower(&strSetType);

	// See if we have an entry for this AI type.
	AnimSetTable::const_iterator table_iter = m_AnimSetTable.find(strAIType);

	if( table_iter != m_AnimSetTable.end() )
	{
		const AnimSetIndex::Table & set_index = table_iter->second.m_Table;

		// See if this AI has an entry for this AnimSet type.
		AnimSetIndex::Table::const_iterator index_iter = set_index.find(strSetType);
		
		if( index_iter != set_index.end() )
		{
			// Alright, we have an entry!  Lets try 
			// to get the AnimSet.
			const int offset = index_iter->second;

			if( offset >= 0 && uint32(offset) < m_AnimSets.size() )
			{
				return &m_AnimSets[offset];
			}
		}
	}

	// We didn't find it, so return null.
	return LTNULL;
}

const AIAnimSet * CAIAnimButeMgr::GetAnimSetPtr(std::string strAnimSet) const
{
	MakeLower(&strAnimSet);

	// See if we have an entry for this AI type.
	AnimSetNames::const_iterator iter = m_AnimSetNames.find(strAnimSet);

	if( iter != m_AnimSetNames.end() )
	{
		// Alright, we have an entry!  Lets try 
		// to get the AnimSet.
		const int offset = iter->second;

		if( offset >= 0 && uint32(offset) < m_AnimSets.size() )
		{
			return &m_AnimSets[offset];
		}
	}

	// We didn't find it, so return null.
	return LTNULL;
}

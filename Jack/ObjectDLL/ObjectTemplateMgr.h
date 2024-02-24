//////////////////////////////////////////////////////////////////////////////
// Object Template handling code

#ifndef __OBJECTTEMPLATEMGR_H__
#define __OBJECTTEMPLATEMGR_H__

#include <hash_map>

class CObjectTemplateMgr
{
public:
	CObjectTemplateMgr();
	~CObjectTemplateMgr();

	void AddTemplate(const ObjectCreateStruct *pOCS);
	const ObjectCreateStruct *FindTemplate(const char *pName) const;
	void Clear();
	
	void Save( ILTMessage_Write *pMsg );
	void Load( ILTMessage_Read *pMsg );

protected:
	// The template dictionary  (Note the startling similarity to GenericPropList..)

	// Template name hash functor
	struct SHash_TemplateName
	{
		enum
		{
			bucket_size = 4,
			min_buckets = 8,
		};
		bool operator ()(const std::string &cLHS, const std::string &cRHS) const {
			return stricmp(cLHS.c_str(), cRHS.c_str()) < 0;
		}
		size_t operator()(const std::string &sName) const { 
			uint32 nHash = 0;
			std::string::const_iterator iName = sName.begin();
			for (; iName != sName.end(); ++iName)
				nHash = 13 * nHash + (toupper(*iName) - '@');
			return nHash;
		}
	};

	// The actual template dictionary type
	typedef stdext::hash_map<std::string, ObjectCreateStruct, SHash_TemplateName> TTemplateMap;
	TTemplateMap m_cTemplates;
};

#endif //__OBJECTTEMPLATEMGR_H__
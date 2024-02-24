// ----------------------------------------------------------------------- //
//
// MODULE  : CEditable.h
//
// PURPOSE : CEditable class definition
//
// CREATED : 3/10/99
//
// ----------------------------------------------------------------------- //

#ifndef __EDITABLE_H__
#define __EDITABLE_H__

#include "cpp_aggregate_de.h"
#include "cpp_engineobjects_de.h"
#include "TemplateList.h"

class CPropDef
{
	public :

		CPropDef();
		~CPropDef();

		enum	PropType { PT_UNKNOWN_TYPE, PT_FLOAT_TYPE, PT_DWORD_TYPE, PT_BYTE_TYPE, PT_BOOL_TYPE, PT_VECTOR_TYPE };

		DBOOL	Init(char* pName, PropType eType, void* pAddress);
		char*	GetPropName();
		DBOOL	GetStringValue(CString & str);

		DBOOL	SetValue(char* pPropName, char* pValue);

	private :

		HSTRING		m_strPropName;
		PropType	m_eType;
		void*		m_pAddress;

		DBOOL	GetFloatValue(DFLOAT & fVal);
		DBOOL	GetDWordValue(DDWORD & dwVal);
		DBOOL	GetByteValue(DBYTE & nVal);
		DBOOL	GetBoolValue(DBOOL & bVal);
		DBOOL	GetVectorValue(DVector & vVal);
};


class CEditable : public Aggregate
{
	public :

		CEditable();
		virtual ~CEditable();
	
		void AddFloatProp(char* pPropName, DFLOAT* pPropAddress);
		void AddDWordProp(char* pPropName, DDWORD* pPropAddress);
		void AddByteProp(char* pPropName, DBYTE* pPropAddress);
		void AddBoolProp(char* pPropName, DBOOL* pPropAddress);
		void AddVectorProp(char* pPropName, DVector* pPropAddress);

	protected :

		DDWORD ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		void	TriggerMsg(LPBASECLASS pObject, HOBJECT hSender, HSTRING hMsg);
		void	EditProperty(LPBASECLASS pObject, char* pPropName, char* pPropValue);
		void	ListProperties(LPBASECLASS pObject);

	private :

		CTList<CPropDef*>		m_propList;
};

#endif // __EDITABLE_H__


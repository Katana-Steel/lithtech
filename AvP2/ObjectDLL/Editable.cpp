//----------------------------------------------------------
//
// MODULE  : Editable.cpp
//
// PURPOSE : Editable aggreate
//
// CREATED : 3/10/99
//
//----------------------------------------------------------

#include "stdafx.h"
#include "Editable.h"
#include "cpp_server_de.h"
#include "ObjectMsgs.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::CEditable()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CEditable::CEditable() : Aggregate()
{
	m_propList.Init(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::~CEditable()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CEditable::~CEditable()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CEditable::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			TriggerMsg(pObject, hSender, hMsg);
			g_pServerDE->FreeString(hMsg);
		}
		break;

		default : break;
	}

	return Aggregate::ObjectMessageFn(pObject, hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::AddFloatProp
//
//	PURPOSE:	Add a float prop to our list
//
// ----------------------------------------------------------------------- //

void CEditable::AddFloatProp(char* pPropName, DFLOAT* pPropAddress)
{
	if (!pPropName || !pPropAddress) return;

	CPropDef* pProp = new CPropDef;
	if (!pProp) return;

	pProp->Init(pPropName, CPropDef::PT_FLOAT_TYPE, (void*)pPropAddress);

	m_propList.Add(pProp);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::AddDWordProp
//
//	PURPOSE:	Add a dword prop to our list
//
// ----------------------------------------------------------------------- //

void CEditable::AddDWordProp(char* pPropName, DDWORD* pPropAddress)
{
	if (!pPropName || !pPropAddress) return;

	CPropDef* pProp = new CPropDef;
	if (!pProp) return;

	pProp->Init(pPropName, CPropDef::PT_DWORD_TYPE, (void*)pPropAddress);

	m_propList.Add(pProp);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::AddByteProp
//
//	PURPOSE:	Add a byte prop to our list
//
// ----------------------------------------------------------------------- //

void CEditable::AddByteProp(char* pPropName, DBYTE* pPropAddress)
{
	if (!pPropName || !pPropAddress) return;

	CPropDef* pProp = new CPropDef;
	if (!pProp) return;

	pProp->Init(pPropName, CPropDef::PT_BYTE_TYPE, (void*)pPropAddress);

	m_propList.Add(pProp);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::AddBoolProp
//
//	PURPOSE:	Add a bool prop to our list
//
// ----------------------------------------------------------------------- //

void CEditable::AddBoolProp(char* pPropName, DBOOL* pPropAddress)
{
	if (!pPropName || !pPropAddress) return;

	CPropDef* pProp = new CPropDef;
	if (!pProp) return;

	pProp->Init(pPropName, CPropDef::PT_BOOL_TYPE, (void*)pPropAddress);

	m_propList.Add(pProp);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::AddVectorProp
//
//	PURPOSE:	Add a vector prop to our list
//
// ----------------------------------------------------------------------- //

void CEditable::AddVectorProp(char* pPropName, DVector* pPropAddress)
{
	if (!pPropName || !pPropAddress) return;

	CPropDef* pProp = new CPropDef;
	if (!pProp) return;

	pProp->Init(pPropName, CPropDef::PT_VECTOR_TYPE, (void*)pPropAddress);

	m_propList.Add(pProp);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::TriggerMsg()
//
//	PURPOSE:	Process trigger messages
//
// --------------------------------------------------------------------------- //

void CEditable::TriggerMsg(LPBASECLASS pObject, HOBJECT hSender, HSTRING hMsg)
{
	if (!hMsg) return;

	char* pMsg = g_pServerDE->GetStringData(hMsg);
	if (!pMsg) return;

	CommonLT* pCommon = g_pServerDE->Common();
	if (!pCommon) return;

	ConParse parse;
	parse.Init(pMsg);

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			if (_stricmp(parse.m_Args[0], "DISPLAYPROPERTIES") == 0)
			{
				ListProperties(pObject);
			}
			else if (_stricmp(parse.m_Args[0], "EDIT") == 0)
			{
				if (parse.m_nArgs > 2)
				{
					EditProperty(pObject, parse.m_Args[1], parse.m_Args[2]);
				}
			}
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::EditProperty()
//
//	PURPOSE:	Edit the specified property
//
// --------------------------------------------------------------------------- //

void CEditable::EditProperty(LPBASECLASS pObject, char* pPropName, char* pPropValue)
{
	if (!pObject || !pPropName || !pPropValue) return;

	// Edit the appropriate property...

	CPropDef** pCur = m_propList.GetItem(TLIT_FIRST);
	CPropDef*  pPropDef = DNULL;

	while (pCur)
	{
		pPropDef = *pCur;
		if (pPropDef)
		{
			char* pName = pPropDef->GetPropName();
			if (pName && _strnicmp(pName, pPropName, strlen(pName)) == 0)
			{
				if (pPropDef->SetValue(pPropName, pPropValue))
				{
					ListProperties(pObject);
				}
				else
				{
					g_pServerDE->CPrint("Couldn't set '%s' to '%s'!", pName, pPropValue);
				}
				return;
			}
		}

		pCur = m_propList.GetItem(TLIT_NEXT);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::ListProperties()
//
//	PURPOSE:	List our properties/values
//
// --------------------------------------------------------------------------- //

void CEditable::ListProperties(LPBASECLASS pObject)
{
	if (!pObject) return;

	g_pServerDE->CPrint("Object Properties------------------------");
	g_pServerDE->CPrint("'Name' = '%s'", g_pServerDE->GetObjectName(pObject->m_hObject));

	CPropDef** pCur = m_propList.GetItem(TLIT_FIRST);
	CPropDef*  pPropDef = DNULL;

	while (pCur)
	{
		pPropDef = *pCur;
		if (pPropDef)
		{
			char* pPropName = pPropDef->GetPropName();
			CString str;
			pPropDef->GetStringValue(str);

			g_pServerDE->CPrint("'%s' = %s", pPropName ? pPropName : "(Invalid name)",
				str.GetLength() > 1 ? str.GetBuffer(1) : "(Invalid value)");
		}
		pCur = m_propList.GetItem(TLIT_NEXT);
	}

	g_pServerDE->CPrint("-----------------------------------------");
}


// --------------------------------------------------------------------------- //
// --------------------------------------------------------------------------- //
//
//	CPropDef class methods
//
// --------------------------------------------------------------------------- //
// --------------------------------------------------------------------------- //

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::CPropDef()
//
//	PURPOSE:	Constructor
//
// --------------------------------------------------------------------------- //

CPropDef::CPropDef()
{
	m_strPropName	= DNULL;
	m_eType			= PT_UNKNOWN_TYPE;
	m_pAddress		= DNULL;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::~CPropDef()
//
//	PURPOSE:	Destructor
//
// --------------------------------------------------------------------------- //

CPropDef::~CPropDef()
{
	if (m_strPropName)
	{
		g_pServerDE->FreeString(m_strPropName);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::Init()
//
//	PURPOSE:	Set up our data members
//
// --------------------------------------------------------------------------- //

DBOOL CPropDef::Init(char* pName, PropType eType, void* pAddress)
{
	if (m_strPropName || !pName) return DFALSE;

	m_strPropName = g_pServerDE->CreateString(pName);
	m_eType = eType;
	m_pAddress = pAddress;
	
	return DTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::GetFloatValue()
//
//	PURPOSE:	Get the value of the property as a float
//
// --------------------------------------------------------------------------- //

DBOOL CPropDef::GetFloatValue(DFLOAT & fRet)
{
	if (m_eType == PT_FLOAT_TYPE && m_pAddress)
	{
		fRet = *((DFLOAT*)m_pAddress);
		return DTRUE;
	}

	return DFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::GetDWordValue()
//
//	PURPOSE:	Get the value of the property as a dword
//
// --------------------------------------------------------------------------- //

DBOOL CPropDef::GetDWordValue(DDWORD & dwRet)
{
	if (m_eType == PT_DWORD_TYPE && m_pAddress)
	{
		dwRet = *((DDWORD*)m_pAddress);
		return DTRUE;
	}

	return DFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::GetByteValue()
//
//	PURPOSE:	Get the value of the property as a byte
//
// --------------------------------------------------------------------------- //

DBOOL CPropDef::GetByteValue(DBYTE & nRet)
{
	if (m_eType == PT_BYTE_TYPE && m_pAddress)
	{
		nRet = *((DBYTE*)m_pAddress);
		return DTRUE;
	}

	return DFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::GetBoolValue()
//
//	PURPOSE:	Get the value of the property as a bool
//
// --------------------------------------------------------------------------- //

DBOOL CPropDef::GetBoolValue(DBOOL & bRet)
{
	if (m_eType == PT_BOOL_TYPE && m_pAddress)
	{
		bRet = *((DBOOL*)m_pAddress);
		return DTRUE;
	}

	return DFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::GetVectorValue()
//
//	PURPOSE:	Get the value of the property as a vector
//
// --------------------------------------------------------------------------- //

DBOOL CPropDef::GetVectorValue(DVector & vRet)
{
	if (m_eType == PT_VECTOR_TYPE && m_pAddress)
	{
		vRet = *((DVector*)m_pAddress);
		return DTRUE;
	}

	return DFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::GetPropName()
//
//	PURPOSE:	Get the name of the property
//
// --------------------------------------------------------------------------- //

char* CPropDef::GetPropName()
{
	if (!m_strPropName) return DNULL;

	return g_pServerDE->GetStringData(m_strPropName);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::GetStringValue()
//
//	PURPOSE:	Get the value of the property as a string
//
// --------------------------------------------------------------------------- //

DBOOL CPropDef::GetStringValue(CString & str)
{
	switch (m_eType)
	{
		case PT_BYTE_TYPE:
		{
			DBYTE nVal;
			if (GetByteValue(nVal))
			{
				str.Format("%d", nVal);
				return DTRUE;
			}
		}
		break;

		case PT_BOOL_TYPE:
		{
			DBOOL bVal;
			if (GetBoolValue(bVal))
			{
				str.Format("%s", bVal ? "True" : "False");
				return DTRUE;
			}
		}
		break;

		case PT_FLOAT_TYPE:
		{
			DFLOAT fVal;
			if (GetFloatValue(fVal))
			{
				str.Format("%.2f", fVal);
				return DTRUE;
			}
		}
		break;

		case PT_VECTOR_TYPE:
		{
			DVector vVal;
			if (GetVectorValue(vVal))
			{
				str.Format("(%.2f, %.2f, %.2f)", vVal.x, vVal.y, vVal.z);
				return DTRUE;
			}
		}
		break;

		case PT_DWORD_TYPE:
		{
			DDWORD dwVal;
			if (GetDWordValue(dwVal))
			{
				str.Format("%d", dwVal);
				return DTRUE;
			}
		}
		break;

		default : break;
	}

	return DFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::SetValue()
//
//	PURPOSE:	Set this property to the value specified...
//
// --------------------------------------------------------------------------- //

DBOOL CPropDef::SetValue(char* pPropName, char* pValue)
{
	if (!pPropName || !pValue) return DFALSE;

	switch (m_eType)
	{
		case PT_BYTE_TYPE:
		{
			DBYTE nVal = (DBYTE) atol(pValue);
			*((DBYTE*)m_pAddress) = nVal;
		}
		break;

		case PT_BOOL_TYPE:
		{
			DBOOL bVal = (DBOOL) atol(pValue);
			*((DBOOL*)m_pAddress) = bVal;
		}
		break;

		case PT_FLOAT_TYPE:
		{
			DFLOAT fVal = (DFLOAT) atof(pValue);
			*((DFLOAT*)m_pAddress) = fVal;
		}
		break;

		case PT_VECTOR_TYPE:
		{
			DFLOAT fVal = (DFLOAT) atof(pValue);

			if (strstr(pPropName, ".x") || strstr(pPropName, ".r"))
			{
				((DVector*)m_pAddress)->x = fVal;
			}
			else if (strstr(pPropName, ".y") || strstr(pPropName, ".g"))
			{
				((DVector*)m_pAddress)->y = fVal;
			}
			else if (strstr(pPropName, ".z") || strstr(pPropName, ".b"))
			{
				((DVector*)m_pAddress)->z = fVal;
			}
		}
		break;

		case PT_DWORD_TYPE:
		{
			DDWORD dwVal = (DDWORD) atol(pValue);
			*((DDWORD*)m_pAddress) = dwVal;
		}
		break;

		default : break;
	}

	return DTRUE;
}

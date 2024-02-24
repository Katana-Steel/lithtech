// ----------------------------------------------------------------------- //
//
// MODULE  : ServerUtilities.h
//
// PURPOSE : Server-side Utility functions
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#ifndef __SERVER_UTILITIES_H__
#define __SERVER_UTILITIES_H__

#include "ltengineobjects.h"
#include "CommonUtilities.h"

// Save/Load Macros

#define SAVE_BYTE(variable) g_pLTServer->WriteToMessageByte(hWrite, variable);
#define LOAD_BYTE(variable) variable = g_pLTServer->ReadFromMessageByte(hRead);
#define LOAD_BYTE_CAST(variable, clazz) variable = (clazz)g_pLTServer->ReadFromMessageByte(hRead);
#define SAVE_WORD(variable) g_pLTServer->WriteToMessageWord(hWrite, variable);
#define LOAD_WORD(variable) variable = g_pLTServer->ReadFromMessageWord(hRead);
#define LOAD_WORD_CAST(variable, clazz) variable = (clazz)g_pLTServer->ReadFromMessageWord(hRead);
#define SAVE_INT(variable) g_pLTServer->WriteToMessageFloat(hWrite, (float)variable);
#define LOAD_INT(variable) variable = (int)g_pLTServer->ReadFromMessageFloat(hRead);
#define LOAD_INT_CAST(variable, clazz) variable = (clazz)g_pLTServer->ReadFromMessageFloat(hRead);
#define SAVE_BOOL(variable) g_pLTServer->WriteToMessageByte(hWrite, variable);
#define LOAD_BOOL(variable) variable = (LTBOOL)g_pLTServer->ReadFromMessageByte(hRead);
#define SAVE_DWORD(variable) g_pLTServer->WriteToMessageDWord(hWrite, variable);
#define LOAD_DWORD(variable) variable = g_pLTServer->ReadFromMessageDWord(hRead);
#define LOAD_DWORD_CAST(variable, clazz) variable = (clazz)g_pLTServer->ReadFromMessageDWord(hRead);
#define SAVE_FLOAT(variable) g_pLTServer->WriteToMessageFloat(hWrite, variable);
#define LOAD_FLOAT(variable) variable = g_pLTServer->ReadFromMessageFloat(hRead);
#define SAVE_HOBJECT(variable) g_pLTServer->WriteToLoadSaveMessageObject(hWrite, variable);
#define LOAD_HOBJECT(variable) g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &variable);
#define SAVE_VECTOR(variable)   g_pLTServer->WriteToMessageVector(hWrite, &variable);
#define LOAD_VECTOR(variable)   g_pLTServer->ReadFromMessageVector(hRead, &variable);
#define SAVE_ROTATION(variable) g_pLTServer->WriteToMessageRotation(hWrite, &variable);
#define LOAD_ROTATION(variable) g_pLTServer->ReadFromMessageRotation(hRead, &variable);
#define SAVE_HSTRING(variable)  g_pLTServer->WriteToMessageHString(hWrite, variable);
#define LOAD_HSTRING(variable)  variable = g_pLTServer->ReadFromMessageHString(hRead);
#define SAVE_RANGE(variable) g_pLTServer->WriteToMessageFloat(hWrite, (float)variable.GetMin()); g_pLTServer->WriteToMessageFloat(hWrite, (float)variable.GetMax());
#define LOAD_RANGE(variable) variable.Set(g_pLTServer->ReadFromMessageFloat(hRead), g_pLTServer->ReadFromMessageFloat(hRead));
#define LOAD_RANGE_CAST(variable, type) variable.Set((type)g_pLTServer->ReadFromMessageFloat(hRead), (type)g_pLTServer->ReadFromMessageFloat(hRead));


// No range checking is done on the array!
// operator>> may write past end of array if szCharArray is shorter than
// the saved string.
ILTMessage & operator<<(ILTMessage & out, char * szCharArray);
ILTMessage & operator>>(ILTMessage & in, char * szCharArray);

ILTMessage & operator<<(ILTMessage & out, CString & cstring);
ILTMessage & operator>>(ILTMessage & in, CString & cstring);

ILTMessage & operator<<(ILTMessage & out, int & x);
ILTMessage & operator>>(ILTMessage & in, int & x);

// FindNamedObject will return LT_OK if exactly one object of that name is found,
// LT_NOTFOUND if no objects of that name are found, or LT_ERROR if more than one
// of the named object is found (unless bMultipleOkay is LTTRUE, in which it will
// return LT_OK and just use the first object in the array)

inline LTRESULT FindNamedObject(const char* szName, HOBJECT& hObject, LTBOOL bMultipleOkay = LTFALSE)
{
	if ( !szName || !*szName ) return LT_NOTFOUND;

	ObjArray<HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects((char*)szName,objArray);

	switch ( objArray.NumObjects() )
	{
		case 1:
		{
			hObject = objArray.GetObject(0);
			return LT_OK;
		}
		case 0:
		{
			return LT_NOTFOUND;
		}
		default:
		{
			if ( bMultipleOkay )
			{
				hObject = objArray.GetObject(0);
				return LT_OK;
			}
			else
			{
#ifndef _FINAL
                g_pLTServer->CPrint("Error, %d objects named \"%s\" present in level", objArray.NumObjects(), szName);
#endif
				return LT_ERROR;
			}
		}
	}
}

inline LTRESULT FindNamedObject(HSTRING hstrName, HOBJECT& hObject, LTBOOL bMultipleOkay = LTFALSE)
{
    return FindNamedObject(g_pLTServer->GetStringData(hstrName), hObject, bMultipleOkay);
}

#define DEG2RAD(x)		(((x)*MATH_PI)/180.0f)

#define FREE_HSTRING(x) if ( x ) { g_pLTServer->FreeString(x); x = LTNULL; }

#ifndef _FINAL
void StartTimingCounter();
void EndTimingCounter(char *pMsg, ...);
#endif

LTBOOL SendTriggerMsgToObjects(LPBASECLASS pSender, HSTRING hName, HSTRING hMsg);
LTBOOL SendTriggerMsgToObjects(LPBASECLASS pSender, const char* pName, const char* pMsg);
void SendTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, HSTRING hMsg);
void SendTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, const char* pStr);

LTBOOL IsPlayer( HOBJECT hObject );
LTBOOL IsAI( HOBJECT hObject );
LTBOOL IsCharacter( HOBJECT hObject );
LTBOOL IsCharacterHitBox( HOBJECT hObject );
LTBOOL IsBodyProp( HOBJECT hObject );
LTBOOL IsGameBase( HOBJECT hObject );
LTBOOL IsKindOf( HOBJECT hObject, const char* szClass );
LTBOOL IsKindOf( HOBJECT hObject, HOBJECT hObject2 );
LTBOOL IsBody( HOBJECT hObject );

uint16 Color255VectorToWord( LTVector *pVal );

LTBOOL MoveObjectToFloor(HOBJECT hObj, LTBOOL bIgnoreObjects = LTFALSE, LTBOOL bUseSetPos=LTTRUE);
LTBOOL MoveObjectToFloor(HOBJECT hObj, IntersectInfo& IInfo, LTBOOL bIgnoreObjects = LTFALSE, LTBOOL bUseSetPos=LTTRUE);

inline LTBOOL Equal(const LTVector & v1, const LTVector & v2)
{
    LTBOOL bRet = LTTRUE;

    const LTFLOAT c_fError = 0.001f;

    LTVector vTemp;
	VEC_SUB(vTemp, v1, v2);

	if (vTemp.x < -c_fError || vTemp.x > c_fError)
	{
        bRet = LTFALSE;
	}
	else if (vTemp.y < -c_fError || vTemp.y > c_fError)
	{
        bRet = LTFALSE;
	}
	else if (vTemp.z < -c_fError || vTemp.z > c_fError)
	{
        bRet = LTFALSE;
	}

	return bRet;
}

LTBOOL DoesSegmentIntersectAABB(const LTVector& Point1, const LTVector& Point2, const LTVector& MinBox, const LTVector& MaxBox, LTVector *pIntersectPt = NULL, LTPlane *pIntersectPlane = NULL);

LTBOOL TestLOSIgnoreCharacters(HOBJECT hTarget, HOBJECT hFrom, const LTVector& vFrom = LTVector(0, 0, 0), LTBOOL bIncludeBodies=LTFALSE);
LTBOOL TestLOSFilterFn(HOBJECT hObject, void *pUserData);

struct TestLOSData
{
	HOBJECT hFrom;
	HOBJECT hTarget;

	TestLOSData()
		: hFrom(NULL),
		  hTarget(NULL) {}
};

#endif // __SERVER_UTILITIES_H__

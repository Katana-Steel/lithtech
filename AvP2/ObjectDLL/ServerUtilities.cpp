// ----------------------------------------------------------------------- //
//
// MODULE  : ServerUtilities.cpp
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdlib.h>
#include "ServerUtilities.h"
#include "iltserver.h"
#include "MsgIDs.h"
#include "CVarTrack.h"
#include "ObjectMsgs.h"
#include "CommandMgr.h"
#include "ltserverobj.h"
#include "Character.h"
#include "CharacterHitBox.h"

ILTMessage & operator<<(ILTMessage & out, char * szCharArray)
{
	out.WriteDWord( strlen(szCharArray) + 1 );
	out.WriteString( szCharArray );

	return out;
}

ILTMessage & operator>>(ILTMessage & in, char * szCharArray)
{
	const uint32 nLength = in.ReadDWord();
	in.ReadStringFL( szCharArray, nLength);

	return in;
}

ILTMessage & operator<<(ILTMessage & out, CString & cstring)
{
	out.WriteDWord( cstring.GetLength() + 1 );
	out.WriteString( cstring.GetBuffer() );

	return out;
}

ILTMessage & operator>>(ILTMessage & in, CString & cstring)
{
	const uint32 nLength = in.ReadDWord();
	in.ReadStringFL( cstring.GetBuffer(nLength), nLength);

	return in;
}

	
ILTMessage & operator<<(ILTMessage & out, int & x)
{
	out.WriteDWord( uint32(x) );
	return out;
}

ILTMessage & operator>>(ILTMessage & in, int & x)
{
	x = int(in.ReadDWord());
	return in;
}


// The following two functions should be used to determine how long a block
// of code takes to execute.  For example:
//
// StartTimingCounter();
// float p1 = 30.0f, p2 = 50.0f;
// Function(p1, p2);
// EndTimingCounter("Function(%.2f, %.2f)", p1, p2);
//
// If "Function" took 1000 ticks to execute, the above code would print in
// the console:
//		Function(30.00, 50.00) : 1000 ticks
//
// NOTE:  The timing information is only printed to the console if the server
// console variable "ShowTiming" is set to 1. (i.e., serv showtiming 1)


#ifndef _FINAL
extern CVarTrack	g_ShowTimingTrack;
extern CVarTrack	g_ShowTriggersTrack;
extern CVarTrack	g_ShowTriggersFilter;

static LTCounter     s_counter;

void StartTimingCounter()
{
    if (!g_pLTServer || g_ShowTimingTrack.GetFloat() < 1.0f) return;

    g_pLTServer->StartCounter(&s_counter);
}

void EndTimingCounter(char *msg, ...)
{
    if (!g_pLTServer || g_ShowTimingTrack.GetFloat() < 1.0f) return;

    uint32 dwTicks = g_pLTServer->EndCounter(&s_counter);

	// parse the message

	char pMsg[256];
	va_list marker;
	va_start(marker, msg);
	int nSuccess = vsprintf(pMsg, msg, marker);
	va_end(marker);

	if (nSuccess < 0) return;

    g_pLTServer->CPrint("%s : %d ticks", pMsg, dwTicks);
}

#endif

// Send hMsg string to all objects named hName...

LTBOOL SendTriggerMsgToObjects(LPBASECLASS pSender, HSTRING hName, HSTRING hMsg)
{
    if (!hMsg) return LTFALSE;

    const char* pMsg = g_pLTServer->GetStringData(hMsg);

	// Process the message as a command if it is a valid command...

	if (g_pCmdMgr->IsValidCmd(pMsg))
	{
		return g_pCmdMgr->Process(pMsg);
	}

    if (!hName) return LTFALSE;

    const char* pName = g_pLTServer->GetStringData(hName);

	return SendTriggerMsgToObjects(pSender, pName, pMsg);
}

// This version is used to support shared code between projects...

LTBOOL SendTriggerMsgToObjects(LPBASECLASS pSender, const char* pName, const char* pMsg)
{
    if (!pMsg) return LTFALSE;

	// Process the message as a command if it is a valid command...

	if (g_pCmdMgr->IsValidCmd(pMsg))
	{
		return g_pCmdMgr->Process(pMsg);
	}

    if (!pName || pName[0] == '\0') return LTFALSE;

	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects(const_cast<char*>(pName), objArray);

	int numObjects = objArray.NumObjects();
    if (!numObjects) return LTFALSE;

	for (int i = 0; i < numObjects; i++)
	{
		SendTriggerMsgToObject(pSender, objArray.GetObject(i), pMsg);
	}

    return LTTRUE;
}

void SendTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, HSTRING hMsg)
{
	HMESSAGEWRITE hMessage;

    char* pMsg = g_pLTServer->GetStringData(hMsg);


	// Process the message as a command if it is a valid command...

	if (g_pCmdMgr->IsValidCmd(pMsg))
	{
		g_pCmdMgr->Process(pMsg);
		return;
	}


    hMessage = g_pLTServer->StartMessageToObject(pSender, hObj, MID_TRIGGER);
	if (hMessage)
	{
#ifndef _FINAL
		if (g_ShowTriggersTrack.GetFloat() != 0.0f)
		{
			char *pSendName = LTNULL;
            if (pSender) pSendName = g_pLTServer->GetObjectName(pSender->m_hObject);
			else pSendName = "Command Manager";

            const char *pRecvName   = g_pLTServer->GetObjectName(hObj);
			const char * pFilter	= const_cast<char*>( g_ShowTriggersFilter.GetStr() );

			// Filter out displaying any unwanted messages...

            LTBOOL bPrintMsg = (!pFilter || !pFilter[0]);
			if (!bPrintMsg)
			{
                bPrintMsg = (pMsg ? !strstr(pFilter, pMsg) : LTTRUE);
			}

			if (bPrintMsg)
			{
                g_pLTServer->CPrint("Message: %s", pMsg ? pMsg : "NULL");
				g_pLTServer->CPrint("  Sent from '%s', to '%s'", pSendName, pRecvName);
			}
		}
#endif
        g_pLTServer->WriteToMessageHString(hMessage, hMsg);
        g_pLTServer->EndMessage(hMessage);
	}
}

// This version is used to support shared code between projects...

void SendTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, const char* pStr)
{
	if (!pStr) return;

    HSTRING hstr = g_pLTServer->CreateString( const_cast<char*>(pStr) );
	SendTriggerMsgToObject(pSender, hObj, hstr);
    g_pLTServer->FreeString(hstr);
}

//-------------------------------------------------------------------------------------------
// IsPlayer
//
// Checks if handle is a handle to a CPlayerObj
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsPlayer( HOBJECT hObject )
{
	if (!hObject)
		return LTFALSE;

    HCLASS hPlayerTest = g_pLTServer->GetClass( "CPlayerObj" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hPlayerTest ));
}

//-------------------------------------------------------------------------------------------
// IsAI
//
// Checks if handle is a handle to a CAI object
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsAI( HOBJECT hObject )
{
	if (!hObject)
		return LTFALSE;

	// An AI is any character which is not a player.
	// This catches CAI and CSimpleAI.
	return IsCharacter(hObject) && !IsPlayer(hObject);
}

//-------------------------------------------------------------------------------------------
// IsCharacter
//
// Checks if handle is a handle to a CCharacter
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsCharacter( HOBJECT hObject )
{
	if (!hObject)
		return LTFALSE;

    HCLASS hCharacterTest = g_pLTServer->GetClass( "CCharacter" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hCharacterTest ));
}

//-------------------------------------------------------------------------------------------
// IsCharacterHitBox
//
// Checks if handle is a handle to a CCharacterHitBox
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsCharacterHitBox( HOBJECT hObject )
{
	if (!hObject)
		return LTFALSE;

    HCLASS hTest  = g_pLTServer->GetClass( "CCharacterHitBox" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

//-------------------------------------------------------------------------------------------
// IsCharacterHitBox
//
// Checks if handle is a handle to a Body
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsBodyProp( HOBJECT hObject )
{
	if (!hObject)
		return LTFALSE;

    HCLASS hTest  = g_pLTServer->GetClass( "BodyProp" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

//-------------------------------------------------------------------------------------------
// IsBody
//
// Checks if handle is a handle to a Body
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsBody( HOBJECT hObject )
{
	if (!hObject)
		return LTFALSE;

    static HCLASS hTest  = g_pLTServer->GetClass( "BodyProp" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

//-------------------------------------------------------------------------------------------
// IsGameBase
//
// Checks if handle is a handle to a GameBase
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsGameBase( HOBJECT hObject )
{
	if (!hObject)
		return LTFALSE;

    HCLASS hTest = g_pLTServer->GetClass( "GameBase" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

//-------------------------------------------------------------------------------------------
// IsKindOf
//
// Checks if hObject is of a particular class
// Arguments:
//		hObject - handle to object to test
//		szClass - the class
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsKindOf( HOBJECT hObject, const char* szClass )
{
	if (!hObject)
		return LTFALSE;

    HCLASS hClassTest = g_pLTServer->GetClass( (char*)szClass );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hClassTest ));
}

//-------------------------------------------------------------------------------------------
// IsKindOf
//
// Checks if hObject is of a particular class
// Arguments:
//		hObject  - handle to object to test
//		hObject2 - handle to test agains
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsKindOf( HOBJECT hObject, HOBJECT hObject2 )
{
	if (!hObject)
		return LTFALSE;

    HCLASS hClassTest = g_pLTServer->GetObjectClass( hObject2 );
    HCLASS hClass     = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hClassTest ));
}

//-------------------------------------------------------------------------------------------
// MoveObjectToFloor
//
// Move the object down to the floor (or down to rest on an object)
// Arguments:
//		hObject - handle to object to move
//		IntersectInfo& IInfo - intersect segment information.
//		LTBOOL bIgnoreObjects - Ignore objects when checking for height.
//		LTBOOL bUseSetPos - Uses SetObjectPos instead of MoveObject
// Return:
//		True if object was moved
//-------------------------------------------------------------------------------------------

LTBOOL MoveObjectToFloor(HOBJECT hObj, IntersectInfo& IInfo, LTBOOL bIgnoreObjects, 
						 LTBOOL bUseSetPos)
{
    if (!hObj) return LTFALSE;

	// Intersect with the world to find the poly we're standing on...

    LTVector vPos, vDims, vDir;
    g_pLTServer->GetObjectPos(hObj, &vPos);
    g_pLTServer->GetObjectDims(hObj, &vDims);

	VEC_SET(vDir, 0.0f, -1.0f, 0.0f);

	IntersectQuery IQuery;

	VEC_COPY(IQuery.m_From, vPos);
	VEC_COPY(IQuery.m_Direction, vDir);

	IQuery.m_Flags	   = IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;

	if(bIgnoreObjects)
		IQuery.m_Flags &= ~INTERSECT_OBJECTS;

	IQuery.m_FilterFn  = NULL;
	IQuery.m_pUserData = NULL;

    if (g_pLTServer->CastRay(&IQuery, &IInfo))
	{
        LTFLOAT fDist = vPos.y - IInfo.m_Point.y;
		// If the distance to move is greater than the dims, then move it.
		// If the distance is less than the dims, then assume the clipping is intentional
		// and leave the object.
		if( fDist - vDims.y > -0.0001f )
		{
			vPos.y -= (fDist - (vDims.y + 0.1f));

			if (bUseSetPos || bIgnoreObjects)
			{
				g_pLTServer->SetObjectPos(hObj, &vPos);
			}
			else
			{
				g_pLTServer->MoveObject(hObj, &vPos);
			}
            return LTTRUE;
		}
	}

    return LTFALSE;
}

LTBOOL MoveObjectToFloor(HOBJECT hObj, LTBOOL bIgnoreObjects, LTBOOL bUseSetPos)
{
	IntersectInfo IInfo;
	return MoveObjectToFloor(hObj, IInfo, bIgnoreObjects, bUseSetPos);
}


// Get the intersection point and see if it's inside the other dimensions.
#define DO_PLANE_TEST(MinBox, MaxBox, planeCoord, coord0, coord1, coord2, normalDirection) \
	t = (planeCoord - Point1.coord0) / (Point2.coord0 - Point1.coord0);\
	testCoords[0] = Point1.coord1 + ((Point2.coord1 - Point1.coord1) * t);\
	if(testCoords[0] > MinBox.coord1-.05f && testCoords[0] < MaxBox.coord1+.05f)\
	{\
		testCoords[1] = Point1.coord2 + ((Point2.coord2 - Point1.coord2) * t);\
		if(testCoords[1] > MinBox.coord2-.05f && testCoords[1] < MaxBox.coord2+.05f)\
		{\
			if ( pIntersectPt ) \
			{ \
				pIntersectPt->coord0 = planeCoord;\
				pIntersectPt->coord1 = testCoords[0];\
				pIntersectPt->coord2 = testCoords[1];\
			} \
			if ( pIntersectPlane ) \
			{ \
				pIntersectPlane->m_Normal.x = normalDirection;\
				pIntersectPlane->m_Normal.y = 0.0f;\
				pIntersectPlane->m_Normal.z = 0.0f;\
				pIntersectPlane->m_Dist = MinBox.x * normalDirection;\
			} \
            return LTTRUE;\
		}\
	}

LTBOOL DoesSegmentIntersectAABB(const LTVector& Point1, const LTVector& Point2, const LTVector& MinBox, const LTVector& MaxBox, LTVector *pIntersectPt /*= NULL*/, LTPlane *pIntersectPlane /*= NULL*/)
{
	float t;
	float testCoords[2];

	// Left/Right.
	if(Point1.x < MinBox.x)
	{
		if(Point2.x < MinBox.x)
            return LTFALSE;

		DO_PLANE_TEST(MinBox, MaxBox, MinBox.x, x, y, z, -1.0f);
	}
	else if(Point1.x > MaxBox.x)
	{
		if(Point2.x > MaxBox.x)
            return LTFALSE;

		DO_PLANE_TEST(MinBox, MaxBox, MaxBox.x, x, y, z, 1.0f);
	}

	// Top/Bottom.
	if(Point1.y < MinBox.y)
	{
		if(Point2.y < MinBox.y)
            return LTFALSE;

		DO_PLANE_TEST(MinBox, MaxBox, MinBox.y, y, x, z, -1.0f);
	}
	else if(Point1.y > MaxBox.y)
	{
		if(Point2.y > MaxBox.y)
            return LTFALSE;

		DO_PLANE_TEST(MinBox, MaxBox, MaxBox.y, y, x, z, 1.0f);
	}

	// Front/Back.
	if(Point1.z < MinBox.z)
	{
		if(Point2.z < MinBox.z)
            return LTFALSE;

		DO_PLANE_TEST(MinBox, MaxBox, MinBox.z, z, x, y, -1.0f);
	}
	else if(Point1.z > MaxBox.z)
	{
		if(Point2.z > MaxBox.z)
            return LTFALSE;

		DO_PLANE_TEST(MinBox, MaxBox, MaxBox.z, z, x, y, 1.0f);
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TestLOSIgnoreCharacters
//
//	PURPOSE:	This is a recursive function that tests for Line of Sight
//				from one object (doesn't need to be a character), hFrom, 
//				to a character, hTarget.  This function 
//				will ignore characters that block LOS and will return FLASE only if
//				a non-character object or poly blocks LOS.
//
// ----------------------------------------------------------------------- //
LTBOOL TestLOSIgnoreCharacters(HOBJECT hTarget, HOBJECT hFrom, const LTVector& vFrom, LTBOOL bIncludeBodies)
{
	LTVector vMyPos, vTargetPos;

	// no target means no line of sight
	if (!hTarget)
		return LTFALSE;

	//get positions
	if (hFrom)
	{
		g_pLTServer->GetObjectPos(hFrom, &vMyPos);
	}
	else
		vMyPos = vFrom;

	g_pLTServer->GetObjectPos(hTarget, &vTargetPos);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From	= vMyPos;
	IQuery.m_To		= vTargetPos;
	IQuery.m_FilterFn = TestLOSFilterFn;
	IQuery.m_pUserData = &bIncludeBodies;

	IQuery.m_Flags = INTERSECT_HPOLY | IGNORE_NONSOLID | INTERSECT_OBJECTS;

	// See if we hit anything in our path
	return !(g_pServerDE->IntersectSegment(&IQuery, &IInfo));
}

LTBOOL TestLOSFilterFn(HOBJECT hObject, void *pUserData)
{
	if (!hObject || !g_pInterface) return LTFALSE;

	LTBOOL bIncludeBodies = *((LTBOOL*)pUserData);

	// Classes we want to ignor
	HCLASS hClassCharacter = g_pInterface->GetClass("CCharacter");
	HCLASS hClassHitBox = g_pInterface->GetClass("CCharacterHitBox");
	HCLASS hClassBody = g_pInterface->GetClass("BodyProp");

	HCLASS hClass = g_pInterface->GetObjectClass(hObject);

	if(g_pInterface->IsKindOf(hClass, hClassHitBox))
		return LTFALSE;

	if(g_pInterface->IsKindOf(hClass, hClassCharacter))
		return LTFALSE;

	if(bIncludeBodies && g_pInterface->IsKindOf(hClass, hClassBody))
		return LTFALSE;

	return LTTRUE;
}



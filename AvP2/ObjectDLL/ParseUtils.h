// ----------------------------------------------------------------------- //
//
// MODULE  : ParseUtils.h
//
// PURPOSE : Parsing utilities.
//
// CREATED : 9/8/00
//
// ----------------------------------------------------------------------- //

#ifndef __PARSE_UTILS_H__
#define __PARSE_UTILS_H__

#include "ltbasetypes.h"
#include "float.h"
#include "ltsmartlink.h"

#include <string>

class CAI;
class CAINode;

struct ParseLocation
{
	std::string error;
	bool bOff;
	bool bTarget;
	CAINode * pNode;
	LTSmartLink hObject;
	LTVector vPosition;

	ParseLocation()
		: bOff(false),
		  bTarget(false),
		  pNode(LTNULL),
		  hObject(LTNULL),
		  vPosition(FLT_MAX,FLT_MAX,FLT_MAX) {}

	ParseLocation( const char * location_str, const CAI & ai );
};

ILTMessage & operator<<(ILTMessage & out, const ParseLocation & x);
ILTMessage & operator>>(ILTMessage & in, ParseLocation & x);


#endif //__PARSE_UTILS_H__

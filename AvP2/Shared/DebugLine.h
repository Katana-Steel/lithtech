// ----------------------------------------------------------------------- //
//
// MODULE  : DebugLine.h
//
// PURPOSE : 
//
// CREATED : 3/29/2000
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DEBUG_LINE_H__
#define __DEBUG_LINE_H__


class ILTMessage;

struct DebugLine
{
	LTVector vSource;
	LTVector vDest;
	
	LTVector vColor;
	float fAlpha;
	
	explicit DebugLine( const LTVector & source = LTVector(0.0f,0.0f,0.0f),
						const LTVector & dest = LTVector(0.0f,0.0f,0.0f),
						const LTVector & color = LTVector(1.0f,1.0f,1.0f),
						float  alpha = 0.0f)
		: vSource(source),
		  vDest(dest),
		  vColor(color),
		  fAlpha(alpha) {}
};


ILTMessage & operator<<(ILTMessage & out, const DebugLine & line_to_save);
ILTMessage & operator>>(ILTMessage & out, DebugLine & line_to_load);

#endif //__DEBUG_LINE_H__

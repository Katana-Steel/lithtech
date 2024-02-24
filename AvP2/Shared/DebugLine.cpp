// ----------------------------------------------------------------------- //
//
// MODULE  : DebugLine.cpp
//
// PURPOSE : 
//
// CREATED : 3/29/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DebugLine.h"
#include "iltmessage.h"


ILTMessage & operator<<(ILTMessage & out, const DebugLine & line)
{
	out.WriteVector( const_cast<LTVector&>(line.vSource) );
	out.WriteVector( const_cast<LTVector&>(line.vDest) );
	out.WriteVector( const_cast<LTVector&>(line.vColor) );
	out.WriteFloat( line.fAlpha );

	return out;
}

ILTMessage & operator>>(ILTMessage & in, DebugLine & line)
{
	line.vSource = in.ReadVector();
	line.vDest   = in.ReadVector();
	line.vColor  = in.ReadVector();
	line.fAlpha  = in.ReadFloat();

	return in;
}

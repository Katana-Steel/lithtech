// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterAlignment.h
//
// PURPOSE : Character alignment
//
// CREATED : 12/23/97
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_ALIGNEMENT_H__
#define __CHARACTER_ALIGNEMENT_H__

#include "iltmessage.h"

// This is being used to determine character to character alignments, it may be
// changed.  So rely on CCharacterMgr::GetRelationship to determine attitude toward between characters.
enum CharacterClass   	
{ 
	UNKNOWN=0, 
	MARINE, 
	CORPORATE, 
	PREDATOR, 
	ALIEN, 
	MARINE_EXOSUIT, 
	CORPORATE_EXOSUIT, 
	SYNTHETIC 
};


inline CharacterClass GetCharacterClass(const char * szClassName)
{
	if ( 0 == stricmp(szClassName,"MARINE") )		
	{
		return MARINE;
	}
	else if ( 0 == stricmp(szClassName,"CORPORATE") )		
	{
		return CORPORATE;
	}
	else if ( 0 == stricmp(szClassName,"PREDATOR") )
	{
		return PREDATOR;
	}
	else if ( 0 == stricmp(szClassName,"ALIEN") )
	{
		return ALIEN;
	}
	else if ( 0 == stricmp(szClassName,"SYNTHETIC") )
	{
		return SYNTHETIC;
	}
	else if ( 0 == stricmp(szClassName,"MARINE_EXOSUIT") )
	{
		return MARINE_EXOSUIT;
	}
	else if ( 0 == stricmp(szClassName,"CORPORATE_EXOSUIT") )
	{
		return CORPORATE_EXOSUIT;
	}

	return UNKNOWN;
}


inline ILTMessage & operator>>(ILTMessage & in, CharacterClass & cc)
{
	cc = CharacterClass(in.ReadByte());
	return in;
}

inline ILTMessage & operator<<(ILTMessage & out, CharacterClass cc)
{
	out.WriteByte(cc);
	return out;
}


#endif // __CHARACTER_ALIGNEMENT_H__

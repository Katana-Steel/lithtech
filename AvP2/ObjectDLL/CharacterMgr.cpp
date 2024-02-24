// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterMgr.cpp
//
// PURPOSE : CharacterMgr implementation
//
// CREATED : 7/9/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "CharacterMgr.h"
#include "AI.h"
#include "AIPathMgr.h"
#include "iltmessage.h"
#include "PlayerObj.h"
#include "AIMiser.h"
#include "GameServerShell.h"
#include "SimpleNodeMgr.h"

#include <algorithm>

// Statics

static CAIPathMgr s_AIPathMgr;
static CSimpleNodeMgr s_SimpleNodeMgr;

//static CAIMiser s_AIMiser;

// Globals

CCharacterMgr* g_pCharacterMgr = DNULL;

const int num_classes = 8;
const CCharacterMgr::Relationship default_relationship[num_classes][num_classes] =
{ 	// UNKNOWN : 
	{ CCharacterMgr::Neutral,  // UNKNOWN
	  CCharacterMgr::Neutral,    // MARINE
	  CCharacterMgr::Neutral,    // CORPORATE
	  CCharacterMgr::Neutral,    // PREDATOR
	  CCharacterMgr::Neutral,    // ALIEN
	  CCharacterMgr::Neutral,    // MARINE_EXOSUIT
	  CCharacterMgr::Neutral,    // CORPORATE_EXOSUIT
	  CCharacterMgr::Neutral },  // SYNTHETIC
	// MARINE : 
	{ CCharacterMgr::Neutral,  // UNKNOWN
	  CCharacterMgr::Friend,    // MARINE
	  CCharacterMgr::Foe,    // CORPORATE
	  CCharacterMgr::Foe,    // PREDATOR
	  CCharacterMgr::Foe,    // ALIEN
	  CCharacterMgr::Friend,    // MARINE_EXOSUIT
	  CCharacterMgr::Foe,    // CORPORATE_EXOSUIT
	  CCharacterMgr::Foe },  // SYNTHETIC
	// CORPORATE : 
	{ CCharacterMgr::Neutral,  // UNKNOWN
	  CCharacterMgr::Foe,    // MARINE
	  CCharacterMgr::Friend,    // CORPORATE
	  CCharacterMgr::Foe,    // PREDATOR
	  CCharacterMgr::Foe,    // ALIEN
	  CCharacterMgr::Foe,    // MARINE_EXOSUIT
	  CCharacterMgr::Friend,    // CORPORATE_EXOSUIT
	  CCharacterMgr::Friend },  // SYNTHETIC
	// PREDATOR : 
	{ CCharacterMgr::Neutral,  // UNKNOWN
	  CCharacterMgr::Foe,    // MARINE
	  CCharacterMgr::Foe,    // CORPORATE
	  CCharacterMgr::Friend,    // PREDATOR
	  CCharacterMgr::Foe,    // ALIEN
	  CCharacterMgr::Foe,    // MARINE_EXOSUIT
	  CCharacterMgr::Foe,    // CORPORATE_EXOSUIT
	  CCharacterMgr::Foe },  // SYNTHETIC
	// ALIEN : 
	{ CCharacterMgr::Neutral,  // UNKNOWN
	  CCharacterMgr::Foe,    // MARINE
	  CCharacterMgr::Foe,    // CORPORATE
	  CCharacterMgr::Foe,    // PREDATOR
	  CCharacterMgr::Friend,    // ALIEN
	  CCharacterMgr::Foe,    // MARINE_EXOSUIT
	  CCharacterMgr::Foe,    // CORPORATE_EXOSUIT
	  CCharacterMgr::Foe },  // SYNTHETIC
	// MARINE_EXOSUIT : 
	{ CCharacterMgr::Neutral,  // UNKNOWN
	  CCharacterMgr::Friend,    // MARINE
	  CCharacterMgr::Foe,    // CORPORATE
	  CCharacterMgr::Foe,    // PREDATOR
	  CCharacterMgr::Foe,    // ALIEN
	  CCharacterMgr::Friend,    // MARINE_EXOSUIT
	  CCharacterMgr::Foe,    // CORPORATE_EXOSUIT
	  CCharacterMgr::Foe },  // SYNTHETIC
	// CORPORATE_EXOSUIT : 
	{ CCharacterMgr::Neutral,  // UNKNOWN
	  CCharacterMgr::Foe,    // MARINE
	  CCharacterMgr::Friend,    // CORPORATE
	  CCharacterMgr::Foe,    // PREDATOR
	  CCharacterMgr::Foe,    // ALIEN
	  CCharacterMgr::Foe,    // MARINE_EXOSUIT
	  CCharacterMgr::Friend,    // CORPORATE_EXOSUIT
	  CCharacterMgr::Friend },  // SYNTHETIC
	// SYNTHETIC : 
	{ CCharacterMgr::Neutral,  // UNKNOWN
	  CCharacterMgr::Foe,    // MARINE
	  CCharacterMgr::Friend,    // CORPORATE
	  CCharacterMgr::Foe,    // PREDATOR
	  CCharacterMgr::Foe,    // ALIEN
	  CCharacterMgr::Foe,    // MARINE_EXOSUIT
	  CCharacterMgr::Friend,    // CORPORATE_EXOSUIT
	  CCharacterMgr::Friend }  // SYNTHETIC

}; //const CCharacterMgr::Relationship default_relationship

// Functions

static LTVector FindMovingBoxMax( const LTVector & start_pos,
						   const LTVector & end_pos,
						   const LTVector & dims )
{
	return LTVector( std::max(start_pos.x + dims.x, end_pos.x + dims.x),
					 std::max(start_pos.y + dims.y, end_pos.y + dims.y),
					 std::max(start_pos.z + dims.z, end_pos.z + dims.z) );
}

static bool BoxIntersection( const LTVector & pos1, const LTVector & dims1, 
		               const LTVector & pos2, const LTVector & dims2  )
{
	_ASSERT( dims1.x >= 0.0f && dims1.y >= 0.0f && dims1.z >= 0.0f );
	_ASSERT( dims2.x >= 0.0f && dims2.y >= 0.0f && dims2.z >= 0.0f );

	return (   (float)fabs(pos1.x - pos2.x) < dims1.x + dims2.x 
			&& (float)fabs(pos1.y - pos2.y) < dims1.y + dims2.y  			   
			&& (float)fabs(pos1.z - pos2.z) < dims1.z + dims2.z  );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::CCharacterMgr()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CCharacterMgr::CCharacterMgr()
{
	g_pCharacterMgr = this;

	// All initialization takes place in CCharacterMgr::PreStartWorld.
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::~CCharacterMgr()
//
//	PURPOSE:	Remove the object
//
// ----------------------------------------------------------------------- //

CCharacterMgr::~CCharacterMgr()
{
	g_pCharacterMgr = DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::AddToGlobalList()
//
//	PURPOSE:	Add a character
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::AddToGlobalList(CCharacter* pChar)
{
	if (!pChar || !pChar->m_hObject) return;

	// Remove an old reference to this character.
	if( std::find(m_globalCharacterList.begin(),m_globalCharacterList.end(),pChar) != m_globalCharacterList.end() )
	{
		Remove(pChar);
	}

	// Add the character to our list.
	m_globalCharacterList.push_back( pChar );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::RemoveFromGlobalList()
//
//	PURPOSE:	Remove a character
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::RemoveFromGlobalList(CCharacter* pChar)
{
	if (!pChar || !pChar->m_hObject) return;
	
	GlobalCharacterList::iterator found_character = std::find(	m_globalCharacterList.begin(),
																m_globalCharacterList.end(),
																pChar);

	if( found_character != m_globalCharacterList.end() )
	{
		m_globalCharacterList.erase( found_character );
	}
}


CCharacterMgr::Relationship CCharacterMgr::FindRelationship(const CCharacter &jack, const CCharacter &jill)
{
	if (jack.GetCharacterClass() < num_classes
		&& jill.GetCharacterClass() < num_classes)
		return default_relationship[jack.GetCharacterClass()][jill.GetCharacterClass()];

	return Neutral;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::Add()
//
//	PURPOSE:	Add a character
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::Add(CCharacter* pChar)
{
	if (!pChar || !pChar->m_hObject) return;

	// Remove an old reference to this character.
	if( std::find(m_characterList.begin(),m_characterList.end(),pChar) != m_characterList.end() )
	{
		Remove(pChar);
	}

	// Add the character to our list.
	m_characterList.push_back( pChar );

	// Add it to any specialised lists.
	if( CPlayerObj * pPlayer = dynamic_cast<CPlayerObj*>(pChar) )
	{
		m_playerList.push_back( pPlayer );
	}
	else if( CAI * pAI = dynamic_cast<CAI*>(pChar) )
	{
		m_aiList.push_back( pAI );
	}

	// Fill in the relationship table.

	for( CharacterIterator iter = BeginCharacters();
		  iter != EndCharacters(); ++iter )
	{
		SetRelationship( *pChar, **iter, FindRelationship(*pChar,**iter) );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::Remove()
//
//	PURPOSE:	Remove a character
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::Remove(CCharacter* pChar)
{
	if (!pChar || !pChar->m_hObject) 
		return;
	
	CharacterList::iterator found_character = std::find(m_characterList.begin(),
														m_characterList.end(),
														pChar);

	if( found_character != m_characterList.end() )
	{
		m_characterList.erase( found_character );

		{
		m_relationshipTable.erase(pChar);
		for( RelationshipTable::iterator iter = m_relationshipTable.begin();
		     iter != m_relationshipTable.end(); ++iter )
		{
			iter->second.erase(pChar);
		}
		}

	}

	if( CPlayerObj * pPlayer = dynamic_cast<CPlayerObj*>(pChar) )
	{
		PlayerList::iterator found_player = std::find( m_playerList.begin(),
													   m_playerList.end(),
													   pPlayer );

		if( found_player != m_playerList.end() )  // avoid bug in MSVC's STL.
			m_playerList.erase( found_player );
	}
	else if( CAI * pAI = dynamic_cast<CAI*>(pChar) )
	{
		AIList::iterator found_ai = std::find( m_aiList.begin(),
											   m_aiList.end(),
											   pAI );

		if( found_ai != m_aiList.end() )  // avoid bug in MSVC's STL.
			m_aiList.erase( found_ai );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::PostStartWorld()
//
//	PURPOSE:	Post start world
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::PostStartWorld(DBYTE nLoadGameFlags)
{
	// Check if we're starting the world from a saved game.
	if (nLoadGameFlags != LOAD_RESTORE_GAME)
	{
		// If we are loading a new game, or switching levels, build the
		// ai path list.  If we are restoring a game our list has already
		// been build, to do nothing...

		s_AIPathMgr.Init();
		//s_AIMiser.Init();
		s_SimpleNodeMgr.Init();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::PreStartWorld()
//
//	PURPOSE:	Pre start world
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::PreStartWorld()
{
	// Clear all our lists...

	m_characterList.clear();
	m_playerList.clear();
	m_aiList.clear();

	m_relationshipTable.clear();

	s_AIPathMgr.Term();
	//s_AIMiser.Term();
	s_SimpleNodeMgr.Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::Load
//
//	PURPOSE:	Load our data
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);
	if (!hRead) return;

	s_AIPathMgr.Load(hRead);
	//s_AIMiser.Load(hRead);
	s_SimpleNodeMgr.Load(hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::Save
//
//	PURPOSE:	Save our data
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::Save(HMESSAGEWRITE hWrite)
{
	_ASSERT(hWrite);
	if (!hWrite) return;

	s_AIPathMgr.Save(hWrite);
	//s_AIMiser.Save(hWrite);
	s_SimpleNodeMgr.Save(hWrite);

}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::GetRelationship
//
//	PURPOSE:	Gets the relationship between two characters.
//
// ----------------------------------------------------------------------- //
CCharacterMgr::Relationship 
CCharacterMgr::GetRelationship(const CCharacter & jack, const CCharacter & jill) const
{
	// Characters are always Friend with themselves.
	// (We have only happy, well adjusted characters in our game.)
	if( &jack == &jill ) return Friend;

	// We have to use find, rather than operator[], because this
	// is a const function.

	// The row should be the lesser of the two CCharacter
	// pointers.
	const CCharacter * row_key = &jack;
	const CCharacter * col_key = &jill;

	if( &jill < &jack )
	{
		row_key = &jill;
		col_key = &jack;
	}


	// Find the appropriate row, if the row-entry
	// does not exist, return an error.
	RelationshipTable::const_iterator row = m_relationshipTable.find(row_key);
	if( row == m_relationshipTable.end() )
	{
		return Unknown;
	}

	// Find the entry location given the row and the
	// column key.

	RelationshipTable::mapped_type::const_iterator location = row->second.find(col_key);
	if( location == row->second.end() )
	{
		return Unknown;
	}

	// We found the relationship!
	return location->second;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::SetRelationship
//
//	PURPOSE:	Changes (or sets, if not previously set) the relationship
//				between two characters.
//
// ----------------------------------------------------------------------- //
void CCharacterMgr::SetRelationship(const CCharacter & jack, const CCharacter & jill,
									CCharacterMgr::Relationship relationship )
{
	// Nothing to be done if they are the same character.
	// (System will always return Friend for same character).
	if( &jack == &jill ) 
		return;

	if( &jack < &jill )
		m_relationshipTable[&jack][&jill] = relationship;
	else
		m_relationshipTable[&jill][&jack] = relationship;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetClosestPlayerPtr
//
//	PURPOSE:	Returns the closest player
//
// ----------------------------------------------------------------------- //
CPlayerObj * GetClosestPlayerPtr(const CCharacter & jack)
{
	_ASSERT(g_pCharacterMgr);
	if( !g_pCharacterMgr ) return LTNULL;

	LTFLOAT fClosestDistSqr = FLT_MAX;
	CPlayerObj * pClosestPlayer = LTNULL;


	for(CCharacterMgr::PlayerIterator iter = g_pCharacterMgr->BeginPlayers();
	    iter != g_pCharacterMgr->EndPlayers(); ++iter )
	{
		if( *iter && *iter != &jack )
		{
			const LTFLOAT fDistSqr = ((*iter)->GetPosition() - jack.GetPosition()).MagSqr();
			if( fDistSqr < fClosestDistSqr )
			{
				fClosestDistSqr = fDistSqr;
				pClosestPlayer = *iter;
			}
		}
	}

	return pClosestPlayer;
}

CCharacter * GetClosestAllyPtr(const CCharacter & src)
{
	_ASSERT(g_pCharacterMgr);
	if (!g_pCharacterMgr)
		return LTNULL;

	LTFLOAT fClosestDistSqr = FLT_MAX;
	CCharacter * pChar = LTNULL;

	for (CCharacterMgr::CharacterIterator iter = g_pCharacterMgr->BeginCharacters();
	iter != g_pCharacterMgr->EndCharacters(); ++iter)
	{
		if (*iter && *iter != &src)
		{
			const LTFLOAT fDistSqr = ((*iter)->GetPosition() - src.GetPosition()).MagSqr();
			if (fDistSqr < fClosestDistSqr)
			{
				if (g_pCharacterMgr->AreAllies(**iter, src))
				{
					fClosestDistSqr = fDistSqr;
					pChar = *iter;
				}
			}
		}
	}

	return pChar;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WillClip
//
//	PURPOSE:	Returns a pointer the character that will be clipped into. Returns first one in character list if more than one.
//
// ----------------------------------------------------------------------- //
static CCharacter * ClipBoxIntoCharacterList(const LTVector & vBoxPos, const LTVector & vBoxDims, const CCharacter * pIgnoreCharacter)
{
	if( !g_pCharacterMgr )
		return LTNULL;

	_ASSERT( vBoxDims.x >= 0.0f );
	_ASSERT( vBoxDims.y >= 0.0f );
	_ASSERT( vBoxDims.z >= 0.0f );


	for( CCharacterMgr::CharacterIterator iter = g_pCharacterMgr->BeginCharacters();
	     iter != g_pCharacterMgr->EndCharacters(); ++iter )
	{
		ASSERT( *iter );

		if( *iter != pIgnoreCharacter )
		{
			if( BoxIntersection( vBoxPos, vBoxDims, (*iter)->GetPosition(), (*iter)->GetDims() ) )
				return *iter;
		}
	}

	return LTNULL;
}

CCharacter* WillClip( const CCharacter & character, const LTVector & vPos)
{
	return ClipBoxIntoCharacterList(character.GetPosition(), character.GetDims(), &character);
}

CCharacter* WillClip( const CCharacter & character, const LTVector & vStartPos, const LTVector & vDestPos)
{
	// The movement_box is the larger box which circumscribes the box at its start
	// and end position.
	const LTVector movement_box_max = FindMovingBoxMax(vStartPos,vDestPos,character.GetDims());
	const LTVector movement_box_pos  = vStartPos + (vDestPos - vStartPos) * 0.5f;
	const LTVector movement_box_dims = movement_box_max - movement_box_pos;

	_ASSERT( movement_box_dims.x >= 0.0f );
	_ASSERT( movement_box_dims.y >= 0.0f );
	_ASSERT( movement_box_dims.z >= 0.0f );


	return ClipBoxIntoCharacterList(movement_box_pos, movement_box_dims, &character);
}

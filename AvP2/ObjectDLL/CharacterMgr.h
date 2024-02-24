// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterMgr.h
//
// PURPOSE : CharacterMgr class definition
//
// CREATED : 7/9/98
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_MGR_H__
#define __CHARACTER_MGR_H__


#include <list>
#include <map>
#include <algorithm>

#include <float.h>

class CAI;
class CCharacter;
class CCharacterMgr;
class CPlayerObj;
class CAISense;
class BodyProp;
class TrailNode;

extern CCharacterMgr *g_pCharacterMgr;


class CCharacterMgr
{

	public :

		enum Relationship
		{
			Unknown = -1,
			Friend = 0,
			Neutral,
			Foe,
			EvilIncarnate
		};

		typedef std::list<CCharacter*>  GlobalCharacterList;
		typedef std::list<CCharacter*>  CharacterList;
		typedef std::list<CPlayerObj*>  PlayerList;
		typedef std::list<CAI*>			AIList;
		typedef std::list<BodyProp*>	DeadBodyList;
		typedef std::list<TrailNode*>	TrailNodeList;

		typedef GlobalCharacterList::iterator	GlobalCharacterIterator;
		typedef CharacterList::iterator			CharacterIterator;
		typedef PlayerList::iterator			PlayerIterator;
		typedef AIList::iterator				AIIterator;
		typedef DeadBodyList::iterator			DeadBodyIterator;
		typedef TrailNodeList::iterator			TrailNodeIterator;

		typedef std::map< const CCharacter*, std::map< const CCharacter *, Relationship    > > RelationshipTable;

	public : // Public methods

		CCharacterMgr();
		~CCharacterMgr();

		// Methods

		void Add(CCharacter* pChar);
		void Remove(CCharacter* pChar);

		void AddToGlobalList(CCharacter* pChar);
		void RemoveFromGlobalList(CCharacter* pChar);

		// Engine functions

		void PostStartWorld(DBYTE nLoadGameFlags);
		void PreStartWorld();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);



		// Helper functions for character to character information.

		Relationship GetRelationship(const CCharacter & jack, const CCharacter & jill) const;
		void SetRelationship(const CCharacter & jack, const CCharacter & jill,
			                          CCharacterMgr::Relationship relationship );

		Relationship FindRelationship(const CCharacter & jack, const CCharacter & jill);

		bool AreEnemies(const CCharacter & jack, const CCharacter & jill) const
		{
			return GetRelationship(jack,jill) > Neutral;
		}

		bool AreAllies(const CCharacter & jack, const CCharacter & jill) const
		{
			return GetRelationship(jack,jill) < Neutral;
		}
		

		// List iteration.
		GlobalCharacterIterator BeginGlobalCharacters()  { return m_globalCharacterList.begin(); }
		GlobalCharacterIterator EndGlobalCharacters()    { return m_globalCharacterList.end(); }

		CharacterIterator BeginCharacters()  { return m_characterList.begin(); }
		CharacterIterator EndCharacters()    { return m_characterList.end(); }

		PlayerIterator BeginPlayers() { return m_playerList.begin(); }
		PlayerIterator EndPlayers()   { return m_playerList.end(); }

		AIIterator BeginAIs()	{ return m_aiList.begin(); }
		AIIterator EndAIs()		{ return m_aiList.end(); }

		DeadBodyIterator BeginDeadBodies() { return m_DeadBodyList.begin(); }
		DeadBodyIterator EndDeadBodies()   { return m_DeadBodyList.end(); }
		void			 AddDeadBody(BodyProp * pBodyToAdd)		 { if( m_DeadBodyList.end() == std::find(m_DeadBodyList.begin(), m_DeadBodyList.end(), pBodyToAdd) ) m_DeadBodyList.push_back(pBodyToAdd); }
		void			 RemoveDeadBody(BodyProp* pBodyToRemove) { m_DeadBodyList.remove(pBodyToRemove); }

		TrailNodeIterator BeginTrailNodes() { return m_TrailNodeList.begin(); }
		TrailNodeIterator EndTrailNodes() { return m_TrailNodeList.end(); }
		void			  AddTrailNode(TrailNode * pNodeToAdd)		 { if( m_TrailNodeList.end() == std::find(m_TrailNodeList.begin(), m_TrailNodeList.end(), pNodeToAdd) ) m_TrailNodeList.push_back(pNodeToAdd); }
		void			  RemoveTrailNode(TrailNode* pNodeToRemove) { m_TrailNodeList.remove(pNodeToRemove); }

		// AI Senses

	private : // Private member variables

		RelationshipTable	m_relationshipTable;

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		GlobalCharacterList	m_globalCharacterList;
		PlayerList			m_playerList;		
		CharacterList		m_characterList;
		AIList				m_aiList;
		DeadBodyList		m_DeadBodyList;
		TrailNodeList		m_TrailNodeList;
};


CPlayerObj * GetClosestPlayerPtr(const CCharacter & character);
CCharacter * GetClosestAllyPtr(const CCharacter & src);

CCharacter * WillClip( const CCharacter & character, const LTVector & vPos);
CCharacter * WillClip( const CCharacter & character, const LTVector & vStartPos, const LTVector & vDestPos);


#endif // __CHARACTER_MGR_H__


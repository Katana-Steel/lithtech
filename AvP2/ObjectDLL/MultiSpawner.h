// ----------------------------------------------------------------------- //
//
// MODULE  : MultiSpawner.h
//
// PURPOSE : MultiSpawner class - implementation
//
// CREATED : 8/27/00
//
// ----------------------------------------------------------------------- //

#ifndef __MULTI_SPAWNER_H__
#define __MULTI_SPAWNER_H__

// ----------------------------------------------------------------------- //

#include "Spawner.h"
#include "SpawnButeMgr.h"

// ----------------------------------------------------------------------- //

#define MAX_MULTISPAWNER_ALIVE			16

// ----------------------------------------------------------------------- //

class MultiSpawner : public Spawner
{
	public :

		typedef std::list<MultiSpawner*> MultiSpawnerList;
		typedef MultiSpawnerList::const_iterator MultiSpawnerIterator;

		static MultiSpawnerIterator BeginMultiSpawners() { return m_sMultiSpawners.begin(); }
		static MultiSpawnerIterator EndMultiSpawners()   { return m_sMultiSpawners.end(); }

	public :

		MultiSpawner();
		virtual ~MultiSpawner();
		int	GetObjectPropsId() { return m_nObjectProps; }

	protected :

		virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	protected :

		LTBOOL	ReadProp(ObjectCreateStruct *pData);
		LTBOOL	InitialUpdate();
		LTBOOL	Update();

		void	Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, uint32 dwSaveFlags);
		void	CacheFiles();

	private :

		void		HandleSpawnInterval();
		BaseClass*	HandleSpawn(char *szClass, char *szProps, LTVector vPos, LTRotation rRot);
		char*		GetCloneProps(HOBJECT hObject);
		uint8		PlayersInRadius(LTFLOAT fRadius);

	private :

		// Object Property variables
		int			m_nObjectProps;			// The index to the object props from the attribute file
		HSTRING		m_hstrObjectClone;		// The name of the object to clone

		LTBOOL		m_bSpawnToFloor;		// If false, "MoveToFloor 0" will be added to the spawn props.

		LTBOOL		m_bActive;				// Is this spawner active?
		LTBOOL		m_bActivateDelay;		// Should this spawn have a delay when activated?
		LTFLOAT		m_fActiveRadius;		// The radius that a player has to be within it order to be active
		LTFLOAT		m_fInactiveRadius;		// The radius that a player has to be away for the spawner to remain active

		LTBOOL		m_bContinuous;			// Should this spawner continuously spawn object?
		uint32		m_nMinIntervals;		// The minimum number of spawn intervals
		uint32		m_nMaxIntervals;		// The maximum number of spawn intervals

		LTFLOAT		m_fRandomChance;		// The random chance that anything will be spawned at an interval
		LTFLOAT		m_fMinDelay;			// The minimum delay between intervals
		LTFLOAT		m_fMaxDelay;			// The maximum delay between intervals
		uint32		m_nMinAmount;			// The minimum amount of objects to spawn at an interval
		uint32		m_nMaxAmount;			// The maximum amount of objects to spawn at an interval
		LTVector	m_vMinVelocity;			// The minimum velocity to initially apply to a spawned object
		LTVector	m_vMaxVelocity;			// The maximum velocity to initially apply to a spawned object

		uint32		m_nMaxAlive;			// The maximum number of objects that can be alive from this spawner

		// Extra update variables
		LTFLOAT		m_fNextSpawnTime;		// This is the next time we should attempt to spawn new objects
		uint32		m_nRemainingIntervals;	// The number of remaining spawn intervals
		uint32		m_nCurrentAlive;		// The current number of alive spawned objects

		HOBJECT		m_pSpawnedList[MAX_MULTISPAWNER_ALIVE];		// The list of spawned items

		static MultiSpawnerList m_sMultiSpawners;
};

// ----------------------------------------------------------------------- //
// Plugin class for static string lists

class CMultiSpawnerPlugin : public IObjectPlugin
{
  public:

	virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath, 
		const char* szPropName, 
		char* const * aszStrings, 
		uint32* pcStrings, 
		const uint32 cMaxStrings, 
		const uint32 cMaxStringLength);

  protected :

		CSpawnButeMgrPlugin m_SpawnButeMgrPlugin;
};

// ----------------------------------------------------------------------- //

#endif // __AI_SPAWNER_H__


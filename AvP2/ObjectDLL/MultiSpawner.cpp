// ----------------------------------------------------------------------- //
//
// MODULE  : MultiSpawner.cpp
//
// PURPOSE : MultiSpawner class - implementation
//
// CREATED : 8/27/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MultiSpawner.h"
#include "cpp_server_de.h"
#include "ObjectMsgs.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "ServerSoundMgr.h"
#include "DebrisMgr.h"
#include "GameServerShell.h"

MultiSpawner::MultiSpawnerList MultiSpawner::m_sMultiSpawners;

BEGIN_CLASS(MultiSpawner)

	ADD_STRINGPROP_FLAG(DefaultSpawn, "", PF_HIDDEN)

	ADD_STRINGPROP_FLAG_HELP(ObjectProps, "", PF_STATICLIST,	"The kind of object to spawn at this object.")
/*	ADD_OBJECTPROP_FLAG_HELP(ObjectClone, "", 0,				"This will override the ObjectProps by instead creating a clone of the object pointed to.")*/

	ADD_BOOLPROP_FLAG_HELP(Active, LTFALSE, 0,					"Tells the spawner to be active right when it's created.")
	ADD_BOOLPROP_FLAG_HELP(ActivateDelay, LTFALSE, 0,			"Tells the wait the delay amount after activation before spawning an object. Otherwise, it will spawn an object right when activated.")
	ADD_REALPROP_FLAG_HELP(ActiveRadius, -1.0f, PF_RADIUS,		"This spawner is only active when a Player is within this radius of it. If this value is -1.0, this property is ignored.")
	ADD_REALPROP_FLAG_HELP(InactiveRadius, 500.0f, PF_RADIUS,	"If a Player is within this radius, the spawner will pause and not function until he's stepped out of it.")

	ADD_BOOLPROP_FLAG_HELP(SpawnToFloor, LTTRUE, 0,				"If false, the spawner will add \"MoveToFloor 0\" to the object properties.")

	ADD_BOOLPROP_FLAG_HELP(Continuous, LTFALSE, 0,				"Tells this spawner to continuously spawn objects's until it's removed or turned inactive?")
	ADD_LONGINTPROP_FLAG_HELP(MinIntervals, 1, 0,				"The minimum number of spawn intervals to try and create objects.")
	ADD_LONGINTPROP_FLAG_HELP(MaxIntervals, 1, 0,				"The maximum number of spawn intervals to try and create objects.")

	ADD_REALPROP_FLAG_HELP(RandomChance, 1.0f, 0,				"This is the random chance that any object will get spawned on the next interval. (from 0 to 1)")
	ADD_REALPROP_FLAG_HELP(MinDelay, 5.0f, 0,					"The minimum time to wait (in seconds) for the next spawn interval.")
	ADD_REALPROP_FLAG_HELP(MaxDelay, 10.0f, 0,					"The maximum time to wait (in seconds) for the next spawn interval.")
//	ADD_LONGINTPROP_FLAG_HELP(MinAmount, 1, 0,					"The minimum amount of objects to try to spawn at a spawn interval.")
//	ADD_LONGINTPROP_FLAG_HELP(MaxAmount, 1, 0,					"The maximum amount of objects to try to spawn at a spawn interval.")
	ADD_VECTORPROP_VAL_FLAG_HELP(MinVelocity, 0.0f, 0.0f, 0.0f, 0,	"The minimum velocity to apply to an object when spawned. These values are based of the spawner's rotation. This will be random for each spawned object during each interval.")
	ADD_VECTORPROP_VAL_FLAG_HELP(MaxVelocity, 0.0f, 0.0f, 0.0f, 0,	"The maximum velocity to apply to an object when spawned. These values are based of the spawner's rotation. This will be random for each spawned object during each interval.")

	ADD_LONGINTPROP_FLAG_HELP(MaxAlive, 2, 0,					"This is the maximum number of objects created by one spawner that can be alive. The maximum is 16.")

END_CLASS_DEFAULT_FLAGS_PLUGIN(MultiSpawner, Spawner, LTNULL, LTNULL, 0, CMultiSpawnerPlugin)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MultiSpawner::MultiSpawner
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

MultiSpawner::MultiSpawner() : Spawner()
{
	// Object Property variables
	m_nObjectProps			= 0;
	m_hstrObjectClone		= LTNULL;

	m_bSpawnToFloor			= LTTRUE;

	m_bActive				= LTFALSE;
	m_bActivateDelay		= LTFALSE;
	m_fActiveRadius			= -1.0f;
	m_fInactiveRadius		= 500.0f;

	m_bContinuous			= LTFALSE;
	m_nMinIntervals			= 1;
	m_nMaxIntervals			= 1;

	m_fRandomChance			= 1.0f;
	m_fMinDelay				= 5.0f;
	m_fMaxDelay				= 10.0f;
	m_nMinAmount			= 1;
	m_nMaxAmount			= 1;
	m_vMinVelocity			= LTVector(0.0f, 0.0f, 0.0f);
	m_vMaxVelocity			= LTVector(0.0f, 0.0f, 0.0f);

	m_nMaxAlive				= 3;

	// Extra update variables
	m_fNextSpawnTime		= 0.0f;
	m_nRemainingIntervals	= 0;
	m_nCurrentAlive			= 0;

	for(int i = 0; i < MAX_MULTISPAWNER_ALIVE; i++)
		m_pSpawnedList[i] = LTNULL;

	m_sMultiSpawners.push_back(this);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MultiSpawner::~MultiSpawner
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

MultiSpawner::~MultiSpawner()
{
	if(m_hstrObjectClone)
	{
		g_pLTServer->FreeString(m_hstrObjectClone);
		m_hstrObjectClone = LTNULL;
	}

	MultiSpawnerList::iterator iter_to_me = std::find(m_sMultiSpawners.begin(), m_sMultiSpawners.end(), this);
	if( iter_to_me != m_sMultiSpawners.end() )
	{
		m_sMultiSpawners.erase(iter_to_me);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MultiSpawner::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 MultiSpawner::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	LTBOOL bRet = Spawner::EngineMessageFn(messageID, pData, fData);

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			break;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate();
			CacheFiles();
			break;
		}

		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_LINKBROKEN:
		{
			HOBJECT hLink = (HOBJECT)pData;

			// Make sure to free up a spot for a new object
			for(int i = 0; i < MAX_MULTISPAWNER_ALIVE; i++)
			{
				if(m_pSpawnedList[i] == hLink)
				{
					m_pSpawnedList[i] = LTNULL;
					m_nCurrentAlive--;
					break;
				}
			}

			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (uint32)fData);
			break;
		}

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (uint32)fData);
			break;
		}
	
		default : break;
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MultiSpawner::InitialUpdate
//
//	PURPOSE:	Do initial updating
//
// ----------------------------------------------------------------------- //

LTBOOL MultiSpawner::InitialUpdate()
{
	// Make sure we get the next update ASAP
	g_pLTServer->SetNextUpdate(m_hObject, (LTFLOAT)0.001f);

	// If we started as active... set our next spawn time
	if(m_bActive)
	{
		if(m_bActivateDelay)
			m_fNextSpawnTime = g_pLTServer->GetTime() + GetRandom(m_fMinDelay, m_fMaxDelay);
		else
			m_fNextSpawnTime = g_pLTServer->GetTime();
	}

	// Choose a random number of intervals
	m_nRemainingIntervals = GetRandom((int)m_nMinIntervals, (int)m_nMaxIntervals);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MultiSpawner::Update
//
//	PURPOSE:	Do initial updating
//
// ----------------------------------------------------------------------- //

LTBOOL MultiSpawner::Update()
{
	// Set our next update to a fair amount... 0.05 will mean that you'll
	// have a max of 20 possible spawn intervals per second
	g_pLTServer->SetNextUpdate(m_hObject, (LTFLOAT)0.05f);

	// First, check to see if we're active
	if(m_bActive)
	{
		LTBOOL bPause = LTFALSE;
		LTFLOAT fTime = g_pLTServer->GetTime();

		// See if we need to pause due to no players within the active radius
		if((m_fActiveRadius != -1.0f) && !PlayersInRadius(m_fActiveRadius))
			bPause = LTTRUE;

		// See if we need to pause due to a player being inside the inactive radius
		if(PlayersInRadius(m_fInactiveRadius))
			bPause = LTTRUE;

		// If our current number of alive objects is maxed out... pause also
		if(m_nCurrentAlive >= m_nMaxAlive)
			bPause = LTTRUE;


//		g_pLTServer->CPrint("Multispawner: NextSpawn = %f, Time = %f", m_fNextSpawnTime, fTime);


		// Make sure there are intervals remaining
		if(!bPause && (m_bContinuous || (m_nRemainingIntervals > 0)))
		{
			// Now see if it's time to spawn something yet
			if(fTime >= m_fNextSpawnTime)
			{
//				g_pLTServer->CPrint("Multispawner: New spawn Interval!");

				// Decrement our remaining intervals
				if(!m_bContinuous) m_nRemainingIntervals--;

				// Set our next spawn interval
				m_fNextSpawnTime = fTime + GetRandom(m_fMinDelay, m_fMaxDelay);

				// See if our random chance succeeds
				if(GetRandom(0.0f, 1.0f) <= m_fRandomChance)
				{
					// Spawn the object(s)
					HandleSpawnInterval();
				}
			}
		}
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MultiSpawner::ReadProp
//
//	PURPOSE:	Reads properties from level info
//
// ----------------------------------------------------------------------- //

LTBOOL MultiSpawner::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	// Read in the base class props
	Spawner::ReadProp(pStruct);


	// ----------------------------------------------------------------------- //
	// Read in the props specific to this class

	if(g_pLTServer->GetPropGeneric( "ObjectProps", &genProp ) == LT_OK)
	{
		if(genProp.m_String[0])
		{
			m_nObjectProps = g_pSpawnButeMgr->GetSpawnButesFromName(genProp.m_String);
		}
	}

	if(g_pLTServer->GetPropGeneric( "ObjectClone", &genProp ) == LT_OK)
	{
		if(genProp.m_String[0])
		{
			m_hstrObjectClone = g_pLTServer->CreateString( genProp.m_String);
		}
	}

	// ----------------------------------------------------------------------- //

	if(g_pLTServer->GetPropGeneric( "Active", &genProp ) == LT_OK)
		m_bActive = genProp.m_Bool;

	if(g_pLTServer->GetPropGeneric( "ActivateDelay", &genProp ) == LT_OK)
		m_bActivateDelay = genProp.m_Bool;

	if(g_pLTServer->GetPropGeneric( "ActiveRadius", &genProp ) == LT_OK)
		m_fActiveRadius = genProp.m_Float;

	if(g_pLTServer->GetPropGeneric( "InactiveRadius", &genProp ) == LT_OK)
		m_fInactiveRadius = genProp.m_Float;

	if(g_pLTServer->GetPropGeneric( "SpawnToFloor", &genProp ) == LT_OK)
		m_bSpawnToFloor = genProp.m_Bool;

	// ----------------------------------------------------------------------- //

	if(g_pLTServer->GetPropGeneric( "Continuous", &genProp ) == LT_OK)
		m_bContinuous = genProp.m_Bool;

	if(g_pLTServer->GetPropGeneric( "MinIntervals", &genProp ) == LT_OK)
		m_nMinIntervals = genProp.m_Long;

	if(g_pLTServer->GetPropGeneric( "MaxIntervals", &genProp ) == LT_OK)
		m_nMaxIntervals = genProp.m_Long;

	// ----------------------------------------------------------------------- //

	if(g_pLTServer->GetPropGeneric( "RandomChance", &genProp ) == LT_OK)
		m_fRandomChance = genProp.m_Float;

	if(g_pLTServer->GetPropGeneric( "MinDelay", &genProp ) == LT_OK)
		m_fMinDelay = genProp.m_Float;

	if(g_pLTServer->GetPropGeneric( "MaxDelay", &genProp ) == LT_OK)
		m_fMaxDelay = genProp.m_Float;

	if(g_pLTServer->GetPropGeneric( "MinAmount", &genProp ) == LT_OK)
		m_nMinAmount = genProp.m_Long;

	if(g_pLTServer->GetPropGeneric( "MaxAmount", &genProp ) == LT_OK)
		m_nMaxAmount = genProp.m_Long;

	if(g_pLTServer->GetPropGeneric( "MinVelocity", &genProp ) == LT_OK)
		m_vMinVelocity = genProp.m_Vec;

	if(g_pLTServer->GetPropGeneric( "MaxVelocity", &genProp ) == LT_OK)
		m_vMaxVelocity = genProp.m_Vec;

	// ----------------------------------------------------------------------- //

	if(g_pLTServer->GetPropGeneric( "MaxAlive", &genProp ) == LT_OK)
		m_nMaxAlive = genProp.m_Long;

	if(m_nMaxAlive > MAX_MULTISPAWNER_ALIVE)
		m_nMaxAlive = MAX_MULTISPAWNER_ALIVE;

	// ----------------------------------------------------------------------- //

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MultiSpawner::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 MultiSpawner::ObjectMessageFn( HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead )
{
	switch(messageID)
	{
		case MID_TRIGGER:
		{
            HSTRING hMsg = g_pLTServer->ReadFromMessageHString(hRead);
			char* pMsg = g_pLTServer->GetStringData(hMsg);
			if(!pMsg) break;

			if(_stricmp(pMsg, "ON") == 0)
			{
				m_bActive = LTTRUE;
				
				// If we have the activate delay turned on... then choose a delay time
				if(m_bActivateDelay)
					m_fNextSpawnTime = g_pLTServer->GetTime() + GetRandom(m_fMinDelay, m_fMaxDelay);
				else
					m_fNextSpawnTime = g_pLTServer->GetTime();
			}
			else if(_stricmp(pMsg, "OFF") == 0)
			{
				m_bActive = LTFALSE;
			}
			else if(_stricmp(pMsg, "RESET") == 0)
			{
				// If we have the activate delay turned on... then choose a delay time
				if(m_bActivateDelay)
					m_fNextSpawnTime = g_pLTServer->GetTime() + GetRandom(m_fMinDelay, m_fMaxDelay);
				else
					m_fNextSpawnTime = g_pLTServer->GetTime();

				// Choose a random number of intervals
				m_nRemainingIntervals = GetRandom((int)m_nMinIntervals, (int)m_nMaxIntervals);
			}
			else if(_stricmp(pMsg, "REMOVE") == 0)
			{
				g_pLTServer->RemoveObject(m_hObject);
			}

            g_pLTServer->FreeString(hMsg);
			break;
		}

		default: break;
	}

	// Skip over the Spawner ObjectMessageFn and go straight through the GameBase
	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MultiSpawner::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void MultiSpawner::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if(!hWrite) return;

	Spawner::Save(hWrite, dwSaveFlags);

	// Write to object properties
	g_pLTServer->WriteToMessageDWord(hWrite, m_nObjectProps);
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrObjectClone);

	g_pLTServer->WriteToMessageByte(hWrite, m_bSpawnToFloor);

	g_pLTServer->WriteToMessageByte(hWrite, m_bActive);
	g_pLTServer->WriteToMessageByte(hWrite, m_bActivateDelay);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fActiveRadius);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fInactiveRadius);

	g_pLTServer->WriteToMessageByte(hWrite, m_bContinuous);
	g_pLTServer->WriteToMessageDWord(hWrite, m_nMinIntervals);
	g_pLTServer->WriteToMessageDWord(hWrite, m_nMaxIntervals);

	g_pLTServer->WriteToMessageFloat(hWrite, m_fRandomChance);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fMinDelay);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fMaxDelay);
	g_pLTServer->WriteToMessageDWord(hWrite, m_nMinAmount);
	g_pLTServer->WriteToMessageDWord(hWrite, m_nMaxAmount);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vMinVelocity);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vMaxVelocity);

	g_pLTServer->WriteToMessageDWord(hWrite, m_nMaxAlive);

	// Write the extra update data
	g_pLTServer->WriteToMessageFloat(hWrite, m_fNextSpawnTime - g_pLTServer->GetTime());
	g_pLTServer->WriteToMessageDWord(hWrite, m_nRemainingIntervals);
	g_pLTServer->WriteToMessageDWord(hWrite, m_nCurrentAlive);

	for(int i = 0; i < MAX_MULTISPAWNER_ALIVE; i++)
		g_pLTServer->WriteToMessageObject(hWrite, m_pSpawnedList[i]);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MultiSpawner::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void MultiSpawner::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if(!hRead) return;

	Spawner::Load(hRead, dwLoadFlags);

	// Read to object properties
	m_nObjectProps		= g_pLTServer->ReadFromMessageDWord(hRead);
	m_hstrObjectClone	= g_pLTServer->ReadFromMessageHString(hRead);

	m_bSpawnToFloor		= (LTBOOL)g_pLTServer->ReadFromMessageByte(hRead);

	m_bActive			= (LTBOOL)g_pLTServer->ReadFromMessageByte(hRead);
	m_bActivateDelay	= (LTBOOL)g_pLTServer->ReadFromMessageByte(hRead);
	m_fActiveRadius		= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fInactiveRadius	= g_pLTServer->ReadFromMessageFloat(hRead);

	m_bContinuous		= (LTBOOL)g_pLTServer->ReadFromMessageByte(hRead);
	m_nMinIntervals		= g_pLTServer->ReadFromMessageDWord(hRead);
	m_nMaxIntervals		= g_pLTServer->ReadFromMessageDWord(hRead);

	m_fRandomChance		= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fMinDelay			= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fMaxDelay			= g_pLTServer->ReadFromMessageFloat(hRead);
	m_nMinAmount		= g_pLTServer->ReadFromMessageDWord(hRead);
	m_nMaxAmount		= g_pLTServer->ReadFromMessageDWord(hRead);
	g_pLTServer->ReadFromMessageVector(hRead, &m_vMinVelocity);
	g_pLTServer->ReadFromMessageVector(hRead, &m_vMaxVelocity);

	m_nMaxAlive			= g_pLTServer->ReadFromMessageDWord(hRead);

	// Read the extra update data
	m_fNextSpawnTime		= g_pLTServer->ReadFromMessageFloat(hRead) + g_pLTServer->GetTime();
	m_nRemainingIntervals	= g_pLTServer->ReadFromMessageDWord(hRead);
	m_nCurrentAlive			= g_pLTServer->ReadFromMessageDWord(hRead);

	for(int i = 0; i < MAX_MULTISPAWNER_ALIVE; i++)
		m_pSpawnedList[i] = g_pLTServer->ReadFromMessageObject(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MultiSpawner::CacheFiles
//
//	PURPOSE:	Cache resources associated with this object
//
// ----------------------------------------------------------------------- //

void MultiSpawner::CacheFiles()
{
	Spawner::CacheFiles();

	// Figure out what we are going to spawn and cache our 
	// gib debris stuff...
	if (g_pServerDE->GetServerFlags() & SS_CACHING)
	{
		// Found a spawner!
		const SpawnButes & butes = g_pSpawnButeMgr->GetSpawnButes(m_nObjectProps);

		// See if it is going to pump out a character
		if( !butes.m_szCharacterClass.IsEmpty() )
		{
			int nIndex = g_pCharacterButeMgr->GetSetFromModelType(butes.m_szCharacterClass);
		
			// Cache all the character sounds...
			g_pGameServerShell->AddCharacterToCache(nIndex);
		}

		if( !butes.m_szCacheWeapons.IsEmpty() )
		{
			ConParse parse( const_cast<char*>(butes.m_szCacheWeapons.GetBuffer()) );

			while( LT_OK == g_pLTServer->Common()->Parse(&parse) )
			{
				if( parse.m_Args[0] && parse.m_Args[0][0] )
				{
					WEAPON * pWeaponData = g_pWeaponMgr->GetWeapon(parse.m_Args[0]);

					if( pWeaponData )
					{
						pWeaponData->Cache(g_pWeaponMgr);
					}
				}

			}


		}

	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MultiSpawner::HandleSpawnInterval()
//
//	PURPOSE:	Handle the spawn interval
//
// ----------------------------------------------------------------------- //

void MultiSpawner::HandleSpawnInterval()
{
	// Get the number of objects to create
	int nObjects = GetRandom((int)m_nMinAmount, (int)m_nMaxAmount);
	if(nObjects < 1) nObjects = 1;

	// Cap this value if need to due to the max alive value
	if((nObjects + m_nCurrentAlive) > m_nMaxAlive)
		nObjects = m_nMaxAlive - m_nCurrentAlive;

	// Make sure our amount is still ok...
	if(nObjects <= 0) return;


	// Get the position and rotation of the spawner
	LTVector vPos;
	LTRotation rRot;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);


	// FAIL FAIL FAIL!!
	// Couldn't get cloning working with the current Lithtech functionality.
	// If I want to do this, I'll have to add some engine features probably

	// See if we can find an object with the clone name
/*	char *szClone = g_pLTServer->GetStringData(m_hstrObjectClone);
	char *szCloneProps = LTNULL;

	if(szClone && (szClone[0] != '\0'))
	{
		ObjArray<HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
		g_pLTServer->FindNamedObjects(szClone, objArray);

		if(objArray.NumObjects())
		{
			// Get the first object we find with this name
			HOBJECT hClone = objArray.GetObject(0);
			szCloneProps = GetCloneProps(hClone);
		}
	}


	// If we didn't fill in the clone props... get the attribute based ones instead
	if(!szCloneProps)
	{

	}
*/
	if(m_nObjectProps == -1)
	{
#ifndef _FINAL
		char szName[32];
		g_pLTServer->GetObjectName(m_hObject, szName, 32);
		g_pLTServer->CPrint("Multispawner %s: Can't spawn due to invalid props from attribute file!", szName);
#endif
		return;
	}

	const SpawnButes & butes = g_pSpawnButeMgr->GetSpawnButes(m_nObjectProps);


	// Go through and create each object one by one
	for(int i = 0; i < nObjects; i++)
	{
		CString cstrSpawnProps = butes.m_szProps;
		if( !m_bSpawnToFloor )
		{
			if( !cstrSpawnProps.IsEmpty() )
				cstrSpawnProps += "; MoveToFloor 0";
			else
				cstrSpawnProps = "MoveToFloor 0";
		}

		BaseClass *pBase = HandleSpawn(const_cast<char*>(butes.m_szClass.GetBuffer()), cstrSpawnProps.GetBuffer(), vPos, rRot);

		if(pBase)
		{
			// Add this to our alive list
			for(int j = 0; j < MAX_MULTISPAWNER_ALIVE; j++)
			{
				if(!m_pSpawnedList[j])
				{
					g_pLTServer->CreateInterObjectLink(m_hObject, pBase->m_hObject);
					m_pSpawnedList[j] = pBase->m_hObject;
					m_nCurrentAlive++;
					break;
				}
			}

			// Get the rotation of the spawner
			LTVector vR, vU, vF;
			g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);

			LTVector vMinVel = (vR * m_vMinVelocity.x) + (vU * m_vMinVelocity.y) + (vF * m_vMinVelocity.z);
			LTVector vMaxVel = (vR * m_vMaxVelocity.x) + (vU * m_vMaxVelocity.y) + (vF * m_vMaxVelocity.z);

			// Add a random velocity to the object
			LTVector vVel;
			vVel.x = GetRandom(vMinVel.x, vMaxVel.x);
			vVel.y = GetRandom(vMinVel.y, vMaxVel.y);
			vVel.z = GetRandom(vMinVel.z, vMaxVel.z);

			g_pPhysicsLT->SetVelocity(pBase->m_hObject, &vVel);


			// Play spawn sound...
			if(m_hstrSpawnSound)
			{
				char* pSound = g_pLTServer->GetStringData(m_hstrSpawnSound);
				g_pServerSoundMgr->PlaySoundFromPos(vPos, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW);
			}

			// Send the initial message
			if( m_hstrInitialMsg )
			{
				SendTriggerMsgToObject(this,pBase->m_hObject, g_pLTServer->GetStringData(m_hstrInitialMsg));
			}
			
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MultiSpawner::HandleSpawn()
//
//	PURPOSE:	Create the object
//
// ----------------------------------------------------------------------- //

BaseClass* MultiSpawner::HandleSpawn(char *szClass, char *szProps, LTVector vPos, LTRotation rRot)
{
	if(!szClass) return LTNULL;

	HCLASS hClass = g_pLTServer->GetClass(szClass);

	if(hClass)
	{
		// Setup the basic object create structure...
		ObjectCreateStruct ocs;
		ocs.Clear();
		ocs.m_Pos = vPos;
		ocs.m_Rotation = rRot;

		return (BaseClass*)g_pLTServer->CreateObjectProps(hClass, &ocs, szProps);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MultiSpawner::GetCloneProps()
//
//	PURPOSE:	Get the props of the clone object
//
// ----------------------------------------------------------------------- //

char* MultiSpawner::GetCloneProps(HOBJECT hObject)
{
	// FAIL FAIL FAIL!!!  This function doesn't work to what I wanted it
	// to do.  Unfortunately, it doesn't look like you can do what I wanted
	// with Lithtech... so I'm just gonna leave this function here as reference
	// even though it sucks and won't be used.


	// Get the class of this object
	HCLASS hClass = g_pLTServer->GetObjectClass(hObject);

	if(hClass)
	{
		// Get the name of the class first
		char szClassProps[512];
		g_pLTServer->GetClassName(hClass, szClassProps, 512);

		uint32 nProps;
		g_pLTServer->GetNumClassProps(hClass, nProps);

		ClassPropInfo info;
		GenericProp prop;

		// Go through all the props for this class and create a string of them
		for(uint32 i = 0; i < nProps; i++)
		{
			g_pLTServer->GetClassProp(hClass, i, info);
			g_pLTServer->GetPropGeneric(info.m_PropName, &prop);
		}
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MultiSpawner::PlayersInRadius()
//
//	PURPOSE:	Returns the number of PlayerObjs within the given radius
//
// ----------------------------------------------------------------------- //

uint8 MultiSpawner::PlayersInRadius(LTFLOAT fRadius)
{
	CCharacterMgr::PlayerIterator iter;
	LTVector vPos, vObjPos, vDir;
	LTFLOAT fRadSqr = fRadius * fRadius;
	uint8 nPlayersInRadius = 0;

	// Get the position of our spawner object
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	// Go through the list of players
	for(iter = g_pCharacterMgr->BeginPlayers(); iter != g_pCharacterMgr->EndPlayers(); ++iter)
	{
		const CPlayerObj* pPlayer = *iter;

		// Get the position of this player and create a direction vector
		g_pLTServer->GetObjectPos(pPlayer->m_hObject, &vObjPos);
		vDir = vObjPos - vPos;

		// If this player is within the radius... increment our counter
		if(vDir.MagSqr() <= fRadSqr)
			nPlayersInRadius++;
	}

	return nPlayersInRadius;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiSpawnerPlugin::PreHook_EditStringList()
//
//	PURPOSE:	Update the static list properties
//
// ----------------------------------------------------------------------- //

LTRESULT CMultiSpawnerPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	// See if we can handle the property...
	if(_stricmp("ObjectProps", szPropName) == 0)
	{
		m_SpawnButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
					aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		m_SpawnButeMgrPlugin.PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

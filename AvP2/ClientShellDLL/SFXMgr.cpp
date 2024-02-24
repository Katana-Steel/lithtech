// ----------------------------------------------------------------------- //
//
// MODULE  : CSFXMgr.cpp
//
// PURPOSE : Special FX Mgr	- Implementation
//
// CREATED : 10/24/97
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SFXMgr.h"
#include "CommonUtilities.h"
#include "CameraFX.h"
#include "GameClientShell.h"
#include "StarLightViewFX.h"
#include "CharacterFX.h"
#include "MultiplayerClientMgr.h"

#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif
// Make the static container for the Plug Factory
//CSpecialFXFactory::PlugFactoryContainer CSpecialFXFactory::m_Registry;

// NOTE:  These indexes should map EXACTLY to the SFX ids defined
// in SFXMsgIds.h...

static unsigned int s_nDynArrayMaxNums[DYN_ARRAY_SIZE] =
{
	100,	// General fx
	20,		// Polygrid
	100,	// Particle trails
	60,		// Particle systems
	60,		// Particle shower
	10,		// Tracers
	20,		// Weapons
	20,		// Dynamic lights
	250,	// Particle trail segments
	20,		// Smoke
	20,		// Bullet trail
	50,		// Volume brush
	100,	// Shell casings
	20,		// Camera
	10,		// Particle explosions
	200,	// Sprites/Models (base scale)
	100,	// Debris
	20,		// Death
	50,		// Gibs
	200,	// Projectile
	100,	// Marks - bullet holes
	200,	// Light
	20,		// Random sparks
	100,	// Pickup item
	75,		// Character
	15,		// Weapon sounds
	10,		// Node Lines (used for AI nodes)
	50,		// Weather
	75,		// Lightning
	2,		// Sprinkles
	75,		// Fire
	150,	// Lens Flares
	10,		// Muzzle flash
	50,		// Search lights
	100,	// Polygon debris
	50,		// Steam,
	30,		// Explosion
	100,	// Poly line
	30,		// Lasers
	20,		// Mines
	30,		// Beams
	50,		// SoundFX

	50,		// Body Prop
	5,		// Graph
	5,		// Starlight Vision
	100,	// AILine FX
	1,		// Predator Targeting Triangle
	25,		// Sprite control objects
	1,		// Fog Volumes
	20,		// Drippers
	1,		// Multi-Debris
};


extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SFXObjectFilterFn()
//
//	PURPOSE:	This function will loop through all the objects in one of 
//				the SFX lists and return FALSE if it finds hObj in the list
//				specified by the integer value in pUserData.
//
//	NOTE:		This function is used to ignore types of objects, i.e. if
//				you wanted to ignore projectiles you would asign this function
//				to the intersect segment info and set pUserData to point to 
//				a uint32 == SFX_PROJECTILE_ID
//
// ----------------------------------------------------------------------- //

LTBOOL SFXObjectFilterFn(HOBJECT hObj, void *pUserData)
{
	//sanity check
	if (!hObj || !pUserData) return LTTRUE;

	//get SFX Mgr
	CSFXMgr *pSFXMgr = g_pGameClientShell->GetSFXMgr();
	
	//test
	if(!pSFXMgr) return LTTRUE;

	SfxFilterStruct* pData = (SfxFilterStruct*)pUserData;

	//loop
	for(int i=0 ; i < pData->nNumTypes ; ++i)
	{
		if(pSFXMgr->TestForObjectInList(hObj, pData->pTypes[i]))
			return LTFALSE;
	}

	if(pData->bFilterFriends)
	{
		// See if what we are testing is even a character...
		CSpecialFX* pFX = g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_CHARACTER_ID, hObj);

		if(pFX)
		{
			CCharacterFX* pCharFX = dynamic_cast<CCharacterFX*>(pFX);

			if(pCharFX)
			{	
				// Get the target's character class...
				CharacterClass TargetCC = g_pCharacterButeMgr->GetClass(pCharFX->m_cs.nCharacterSet);

				// Now lets see about the client
				pFX = g_pGameClientShell->GetSFXMgr()->GetClientCharacterFX();

				if(pFX)
				{
					CCharacterFX* pPlayerFX = dynamic_cast<CCharacterFX*>(pFX);

					if(pPlayerFX)
					{
						// Get the client's character class...
						CharacterClass PlayerCC = g_pCharacterButeMgr->GetClass(pPlayerFX->m_cs.nCharacterSet);

						// see if this is multiplayer
						if(g_pGameClientShell->IsMultiplayerGame() && g_pGameClientShell->GetMultiplayerMgr())
						{
							PlayerCC = (CharacterClass)(g_pGameClientShell->GetMultiplayerMgr()->GetCharacterClass(pPlayerFX->m_cs.nClientID));
							TargetCC = (CharacterClass)(g_pGameClientShell->GetMultiplayerMgr()->GetCharacterClass(pCharFX->m_cs.nClientID));
						}

						// Return the results.
						return (PlayerCC != TargetCC);
					}
				}
			}
		}
	}

	return LTTRUE;
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::TestForObjectInList()
//
//	PURPOSE:	Init the CSFXMgr
//
// ----------------------------------------------------------------------- //

LTBOOL CSFXMgr::TestForObjectInList(HOBJECT hObj, uint32 nSFXType)
{
	//sanity check
	if(!hObj || nSFXType > SFX_TOTAL_NUMBER) 
		return LTFALSE;

	for( CSpecialFXList::const_Iterator iter = m_dynSFXLists[nSFXType].Begin();
	     iter != m_dynSFXLists[nSFXType].End(); ++iter )
	{
		if(iter->pSFX)
		{
			HOBJECT hSFXObj = iter->pSFX->GetServerObj();

			if(hSFXObj)
			{
				//we have an object, test it
				if(hSFXObj == hObj)
					return LTTRUE;
			}
		}
	}

	//couldn't find it
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::Init()
//
//	PURPOSE:	Init the CSFXMgr
//
// ----------------------------------------------------------------------- //

DBOOL CSFXMgr::Init(CClientDE *pClientDE)
{
	if (!pClientDE) return DFALSE;

	m_pClientDE = pClientDE;

	for (int i=0; i < DYN_ARRAY_SIZE; i++)
	{
		m_dynSFXLists[i].SetMaxSize(GetDynArrayMaxNum(i));
	}

	m_cameraSFXList.SetMaxSize(CAMERA_LIST_SIZE);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::HandleSFXMsg()
//
//	PURPOSE:	Handle a special fx message
//
// ----------------------------------------------------------------------- //

void CSFXMgr::HandleSFXMsg(HLOCALOBJ hObj, HMESSAGEREAD hMessage)
{
	if (!m_pClientDE) return;

	DBYTE nId = m_pClientDE->ReadFromMessageByte(hMessage);

	CreateSFX(nId, DNULL, hMessage, hObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::CreateSFX()
//
//	PURPOSE:	Create the special fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CSFXMgr::CreateSFX(DBYTE nId, SFXCREATESTRUCT *psfxCreateStruct,
							   HMESSAGEREAD hMessage, HOBJECT hServerObj)
{
	// Make sure we have a ClientDE
	if (!m_pClientDE)
	{
		return DNULL;
	}

	// Only create the sfx if we are connected to the server...
	if (!m_pClientDE->IsConnected())
	{
		return DNULL;
	}

	CSpecialFX* pSFX = DNULL;


	// See if there's enough room for this effect type in it's list
	int nIndex = GetDynArrayIndex(nId);

	if (0 <= nIndex && nIndex < DYN_ARRAY_SIZE)
	{
		// Create a copy of the factory.
		// This is all static variables and functions so this is free
		CSpecialFXFactory FXFactory(0);

		// First initialize with the create struct...
		if (psfxCreateStruct)
		{
			// Get an SFX based on the ID and Create Structure
			pSFX = FXFactory.NewSFX(nId, psfxCreateStruct);
		}
		else if (hMessage)  // Initialize using the hMessage
		{
			// Get an SFX based on the ID a message and an HObject
			pSFX = FXFactory.NewSFX(nId, hMessage, hServerObj);
		}
		else
		{
			return DNULL;
		}

		// Now finish the job
		// Check for pSFX == 0 first
		if (!pSFX)
		{
			return DNULL;
		}

		if (!pSFX->CreateObject(m_pClientDE))
		{
			delete pSFX;
			return DNULL;
		}

		// And add it.
		AddDynamicSpecialFX(pSFX, nId);

		// Be sure the skin is correct
		pSFX->SetMode(m_szMode);
	}

	return pSFX;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::UpdateSpecialFX()
//
//	PURPOSE:	Update any the special FX
//
// ----------------------------------------------------------------------- //

void CSFXMgr::UpdateSpecialFX()
{
	if (!m_pClientDE) return;

	// Check if sfx are turned off.
	if( !GetConsoleInt( "ENABLESFX", 1 ))
		return;

	DFLOAT fTime = m_pClientDE->GetTime();

	// Update dynamic sfx...

	UpdateDynamicSpecialFX();


	// Update camera sfx...
/*
	int nNumSFX = m_cameraSFXList.GetSize();
	
	for (int i=0; i < nNumSFX; i++)
	{
		if (m_cameraSFXList[i]) 
		{
			if (fTime >= m_cameraSFXList.GetNextUpdateTime(i))
			{
				if (!m_cameraSFXList[i]->Update())
				{
					m_cameraSFXList.Remove(m_cameraSFXList[i]);
				}
				else
				{
					m_cameraSFXList.SetNextUpdateTime(i,fTime + m_cameraSFXList[i]->GetUpdateDelta());
				}
			}
		}
	}
*/
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::RemoveSpecialFX()
//
//	PURPOSE:	Remove the specified special fx
//
// ----------------------------------------------------------------------- //

void CSFXMgr::RemoveSpecialFX(HLOCALOBJ hObj)
{
	if (!m_pClientDE) return;

	// Remove the dynamic special fx associated with this object..

	RemoveDynamicSpecialFX(hObj);


	// See if this was a camera...

/*	int nNumSFX = m_cameraSFXList.GetSize();

	for (int i=0; i < nNumSFX; i++)
	{
		if (m_cameraSFXList[i] && m_cameraSFXList[i]->GetServerObj() == hObj)
		{
			m_cameraSFXList[i]->WantRemove();
		}
	}
*/
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::RemoveAll()
//
//	PURPOSE:	Remove all the special fx
//
// ----------------------------------------------------------------------- //

void CSFXMgr::RemoveAll()
{
	RemoveAllDynamicSpecialFX();

/*	int nNumSFX = m_cameraSFXList.GetSize();

	for (int i=0; i < nNumSFX; i++)
	{
		m_cameraSFXList.Remove(m_cameraSFXList[i]);
	}
*/
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::AddDynamicSpecialFX()
//
//	PURPOSE:	Add a dyanamic special fx to our lists
//
// ----------------------------------------------------------------------- //

void CSFXMgr::AddDynamicSpecialFX(CSpecialFX* pSFX, DBYTE nId)
{
	int nIndex = GetDynArrayIndex(nId);

	if (0 <= nIndex && nIndex < DYN_ARRAY_SIZE)
	{
		m_dynSFXLists[nIndex].Add(pSFX);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::UpdateDynamicSpecialFX()
//
//	PURPOSE:	Update the dyanamic special fxs
//
// ----------------------------------------------------------------------- //

void CSFXMgr::UpdateDynamicSpecialFX()
{
	if (!m_pClientDE) return;

	DFLOAT fTime = m_pClientDE->GetTime();

	// Check if we're supposed to watch the deactive flag on the
	// server object before we update the sfx.
	BOOL bCheckDeactiveFlag = GetConsoleInt( "SFXDEACTIVEFLAG", 1 );

	for (int j=0; j < DYN_ARRAY_SIZE; j++)
	{
		for( CSpecialFXList::Iterator iter = m_dynSFXLists[j].Begin();
			 iter != m_dynSFXLists[j].End(); )
		{
			if (iter->pSFX) 
			{
				// Check if the server object has been deactivated.  Don't update the
				// sfx if it's deactive.  If the server object doesn't exist then just
				// assume we have to update.  This is done because the server object 
				// will be removed, then we set the WantRemove flag, then the update 
				// does the actual removal.
				uint32 dwUsrFlags = 0;
				HLOCALOBJ hServerObj = iter->pSFX->GetServerObj( );
				if( hServerObj )
					m_pClientDE->GetObjectUserFlags( hServerObj, &dwUsrFlags );

				if( !bCheckDeactiveFlag || !( dwUsrFlags & USRFLG_DEACTIVATED ))
				{
					if (fTime >= iter->fNextUpdateTime )
					{
						if (!iter->pSFX->Update())
						{
							CSpecialFXList::Iterator last_iter = iter++;
							m_dynSFXLists[j].Remove(last_iter);
							continue;
						}

						iter->fNextUpdateTime =  fTime + iter->pSFX->GetUpdateDelta();
					}
				}
			}

			++iter;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::PostRenderDraw()
//
//	PURPOSE:	Calls any post-render drawing routines
//
// ----------------------------------------------------------------------- //


void CSFXMgr::PostRenderDraw()
{
	if (!m_pClientDE) return;

	// Add any special fx which need a post render draw to this loop.

	{for( CSpecialFXList::Iterator iter = m_dynSFXLists[SFX_PRED_TARGET_ID].Begin();
		 iter != m_dynSFXLists[SFX_PRED_TARGET_ID].End(); ++iter)
	{
		if (iter->pSFX) 
		{
			iter->pSFX->PostRenderDraw();
		}
	}}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::RemoveDynamicSpecialFX()
//
//	PURPOSE:	Remove the specified special fx
//
// ----------------------------------------------------------------------- //

void CSFXMgr::RemoveDynamicSpecialFX(HOBJECT hObj)
{
	for (int j=0; j < DYN_ARRAY_SIZE; j++)
	{
		for( CSpecialFXList::Iterator iter = m_dynSFXLists[j].Begin();
			 iter != m_dynSFXLists[j].End(); ++iter)
		{
			// More than one sfx may have the same server handle, so let them
			// all have an opportunity to remove themselves...

			if (iter->pSFX )
			{
				iter->pSFX->OnObjectRemove(hObj);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::RemoveAllDynamicSpecialFX()
//
//	PURPOSE:	Remove all dynamic special fx
//
// ----------------------------------------------------------------------- //

void CSFXMgr::RemoveAllDynamicSpecialFX()
{
	for (int j=0; j < DYN_ARRAY_SIZE; j++)
	{
		m_dynSFXLists[j].RemoveAll();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::FindSpecialFX()
//
//	PURPOSE:	Find the specified special fx type associated with the 
//				object (see SFXMsgIds.h for valid types)
//
// ----------------------------------------------------------------------- //

CSpecialFX* CSFXMgr::FindSpecialFX(DBYTE nType, HLOCALOBJ hObj)
{
	if (0 <= nType && nType < DYN_ARRAY_SIZE)
	{
		for( CSpecialFXList::Iterator iter = m_dynSFXLists[nType].Begin();
			 iter != m_dynSFXLists[nType].End(); ++iter)
		{
			if (iter->pSFX && iter->pSFX->GetServerObj() == hObj)
			{
				return iter->pSFX;
			}
		}
	}

	return DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::GetDynArrayIndex()
//
//	PURPOSE:	Get the array index associated with a particular type of
//				dynamic special fx
//
// ----------------------------------------------------------------------- //

int	CSFXMgr::GetDynArrayIndex(DBYTE nFXId)
{
	// All valid fxids should map directly to the array index...If this is
	// an invalid id, use the general fx index (i.e., 0)...

	if (nFXId < 0 || nFXId >= DYN_ARRAY_SIZE)
	{
		return 0;
	}

	return nFXId;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::GetDynArrayMaxNum()
//
//	PURPOSE:	Find a dyanamic special fx associated with an object
//
// ----------------------------------------------------------------------- //

unsigned int CSFXMgr::GetDynArrayMaxNum(DBYTE nIndex)
{
	if (0 <= nIndex && nIndex < DYN_ARRAY_SIZE)
	{
		// Use detail setting for bullet holes...

		if (nIndex == SFX_MARK_ID)
		{
			CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
			if (pSettings)
			{
				return int(pSettings->NumBulletHoles() + 1);
			}
		}

		return s_nDynArrayMaxNums[nIndex];
	}

	return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::OnTouchNotify()
//
//	PURPOSE:	Handle client-side touch notify
//
// ----------------------------------------------------------------------- //

void CSFXMgr::OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, float forceMag)
{
	if (!hMain) return;

	CSpecialFX* pFX = FindSpecialFX(SFX_PROJECTILE_ID, hMain);

	if (pFX)
	{
		pFX->HandleTouch(pInfo, forceMag);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::OnSFXMessage()
//
//	PURPOSE:	Handle server-to-sfx messages
//
// ----------------------------------------------------------------------- //

void CSFXMgr::OnSFXMessage(HMESSAGEREAD hMessage)
{
	if (!hMessage) return;

	DBYTE nFXType	= m_pClientDE->ReadFromMessageByte(hMessage);
	HOBJECT hObj	= m_pClientDE->ReadFromMessageObject(hMessage);

	if (0 <= nFXType && nFXType < DYN_ARRAY_SIZE && hObj)
	{
		CSpecialFX* pFX = FindSpecialFX(nFXType, hObj);

		if (pFX)
		{
			pFX->OnServerMessage(hMessage);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::OnModelKey()
//
//	PURPOSE:	Handle client-side model key
//
// ----------------------------------------------------------------------- //

void CSFXMgr::OnModelKey(HLOCALOBJ hObj, ArgList *pArgs)
{
	if (!hObj || !pArgs) return;

	// Only pass these on to the player (and AI)...

	for( CSpecialFXList::Iterator iter = m_dynSFXLists[SFX_CHARACTER_ID].Begin();
		 iter != m_dynSFXLists[SFX_CHARACTER_ID].End(); ++iter)
	{
		if (iter->pSFX && iter->pSFX->GetServerObj() == hObj)
		{
			iter->pSFX->OnModelKey(hObj, pArgs);
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::SetMode(int)
//
//	PURPOSE:	Set a new vision mode for all FX
//
// ----------------------------------------------------------------------- //

void CSFXMgr::SetMode(const std::string& mode)
{
	if (!m_pClientDE) return;

	// Record the mode
	m_szMode = mode;

	// Update camera sfx...

	for( CSpecialFXList::Iterator iter = m_cameraSFXList.Begin();
		 iter != m_cameraSFXList.End(); ++iter )
	{
		if (iter->pSFX) 
		{
			iter->pSFX->SetMode(mode);
		}
	}

	for (int j=0; j < DYN_ARRAY_SIZE; j++)
	{
		for( CSpecialFXList::Iterator iter = m_dynSFXLists[j].Begin();
			 iter != m_dynSFXLists[j].End(); ++iter)
		{
			if (iter->pSFX) 
			{
				iter->pSFX->SetMode(mode);
			}
		}
	}
}



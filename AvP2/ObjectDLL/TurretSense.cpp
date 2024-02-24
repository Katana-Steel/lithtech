// ----------------------------------------------------------------------- //
//
// MODULE  : TurretSense.cpp
//
// PURPOSE : Sense information aggregrate for Turrets.
//
// CREATED : 2.29.00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Turret.h"
#include "Character.h"
#include "CharacterMgr.h"
#include "BodyProp.h"
#include "VolumeBrushTypes.h"
#include "CharacterHitBox.h"
#include "AIUtils.h"

#include <algorithm>

#ifdef _DEBUG
//#include "DebugLineSystem.h"
//#define TURRET_SENSE_DEBUG
#endif


static LTBOOL TurretShootingFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj || !g_pServerDE) return LTFALSE;

	LPBASECLASS pObj = g_pServerDE->HandleToObject(hObj);
	if( dynamic_cast<BodyProp*>(pObj) )
	{
		// Ignore dead bodies.
		return LTFALSE;
	}
	
	if( dynamic_cast<CCharacterHitBox*>(pObj) )
	{
		return LTFALSE;
	}

	return LiquidFilterFn(hObj, pUserData);
}

static LTBOOL TurretShootThroughFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj || !g_pServerDE) return LTFALSE;

	GameBase * pObject = dynamic_cast<GameBase *>( g_pLTServer->HandleToObject( hObj ) );
	if( pObject && pObject->CanShootThrough() )
	{
		return LTFALSE;
	}

	return TurretShootingFilterFn(hObj, pUserData);
}

static LTBOOL TurretShootThroughPolyFilterFn(HPOLY hPoly, void *pUserData)
{
/*	if ( INVALID_HPOLY == hPoly ) return LTFALSE;

	// Check to see if this is a poly from the main world.
	// If it is, don't ignore it!
	HOBJECT hPolyObject = LTNULL;
	g_pLTServer->GetHPolyObject(hPoly,hPolyObject);
	
	if( g_pLTServer->Physics()->IsWorldObject(hPolyObject) )
		return LTTRUE;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hPoly));
	if ( pSurf && pSurf->bCanShootThrough ) 
	{
		return LTFALSE;
	}
*/
	return LTTRUE;
}

static LTBOOL TurretSeeFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj || !g_pServerDE) return LTFALSE;

	LPBASECLASS pObj = g_pServerDE->HandleToObject(hObj);
	if( dynamic_cast<BodyProp*>(pObj) )
	{
		// Ignore dead bodies.
		return LTFALSE;
	}

	if( dynamic_cast<CCharacterHitBox*>(pObj) )
	{
		// We only want to see the character.
		return LTFALSE;
	}

	if( hObj != (HOBJECT)pUserData
		&& dynamic_cast<CCharacter*>(pObj) )
	{
		// Don't let another character block our line 
		// of sight.
		return LTFALSE;
	}

	return LiquidFilterFn(hObj, pUserData);
}

static LTBOOL TurretSeeThroughFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj || !g_pLTServer) return LTFALSE;

	// Be sure we can see our target, even if they are marked see through!
	if( pUserData == hObj )
	{
//		return LTTRUE;
	}

	GameBase * pObject = dynamic_cast<GameBase *>( g_pLTServer->HandleToObject( hObj ) );
	if( pObject && pObject->CanSeeThrough() )
	{
		return LTFALSE;
	}

	return TurretSeeFilterFn(hObj,pUserData);
}


static LTBOOL TurretSeeThroughPolyFilterFn(HPOLY hPoly, void *pUserData)
{
/*	if ( INVALID_HPOLY == hPoly ) return LTFALSE;

	// Check to see if this is a poly from the main world.
	// If it is, don't ignore it!
	HOBJECT hPolyObject = LTNULL;
	g_pLTServer->GetHPolyObject(hPoly,hPolyObject);
	
	if( g_pLTServer->Physics()->IsWorldObject(hPolyObject) )
		return LTTRUE;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hPoly));
	if(pSurf && pSurf->bCanSeeThrough)
		return LTFALSE;
*/
	return LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVisionInstance::Check
//
//	PURPOSE:	Check for visibility of target.  
//				Returns true if the instance could be targetted instead of current focus.
//
// ----------------------------------------------------------------------- //

LTBOOL CTurretVisionInstance::Check()
{
	if( m_bIsVisible ) 
		Stimulate(1.0f);
	else
		Stimulate(0.0f);

	if( !m_owner.GetFocus() )
	{
		return LTTRUE;
	}

	if( m_owner.GetFocus() == m_target.m_hObject )
	{
		return LTTRUE;
	}

	// check if rank is greater than focus

	const int focus_rank = m_owner.GetFocusRank();
	const int target_rank = m_owner.GetRank(m_target.GetCharacterClass());
	if(  focus_rank >  target_rank )
	{
		return LTFALSE;
	}
	else if( focus_rank < target_rank )
	{
		return LTTRUE;
	}


	// For same rank targets, Switch to the closest one.
	DVector vOwnerPos;
	g_pServerDE->GetObjectPos(m_owner.GetOwner(),&vOwnerPos);

	DVector vTargetDisplacement;
	g_pServerDE->GetObjectPos(m_target.m_hObject, &vTargetDisplacement);
	vTargetDisplacement -= const_cast<DVector&>(vOwnerPos);
	const LTFLOAT fTargetDistSqr = vTargetDisplacement.MagSqr();

	DVector vFocusDisplacement;
	g_pServerDE->GetObjectPos(m_owner.GetFocus(), &vFocusDisplacement);
	vFocusDisplacement -= const_cast<DVector&>(vOwnerPos);
	const LTFLOAT fFocusDistSqr = vFocusDisplacement.MagSqr();

	if( fTargetDistSqr < fFocusDistSqr )
	{
		return LTTRUE;
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVisionInstance::Save
//
//	PURPOSE:	Save sense instance.
//
// ----------------------------------------------------------------------- //

void CTurretVisionInstance::Save(LMessage * pMsg)
{
	if( !pMsg ) return;

	CSenseInstance::Save(pMsg);

	// m_owner handled by owner.
	// m_target handled by owner
	*pMsg << m_bIsVisible;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVisionInstance::Load
//
//	PURPOSE:	Load sense instance.
//
// ----------------------------------------------------------------------- //

void CTurretVisionInstance::Load(LMessage * pMsg)
{
	if( !pMsg ) return;

	CSenseInstance::Load(pMsg);

	// m_owner handled by owner.
	// m_target handled by owner
	*pMsg >> m_bIsVisible;
}


static void ReadDeadVisionInstanceData(LMessage * pMsg)
{
	LTBOOL bBool;

	if( !pMsg ) return;

	ReadDummySenseInstance(pMsg);

	*pMsg >> bBool;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVision::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //
		
uint32 CTurretVision::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE :
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			uint32 dwRet = IAggregate::EngineMessageFn(pObject, messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate(dynamic_cast<Turret*>(pObject));
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((LMessage*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((LMessage*)pData);
		}
		break;
	}

	return IAggregate::EngineMessageFn(pObject, messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVision::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void CTurretVision::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) return;

	GenericProp genProp;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVision::InitialUpdate
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CTurretVision::InitialUpdate(const Turret * pTurret)
{
	ASSERT( pTurret );
	if( !pTurret ) return;

	// Record our owner.
	m_hOwner = pTurret->m_hObject;

	m_vOwnerUp  = pTurret->GetUpVector();

	m_fMaxRangeSqr = pTurret->GetVisualRange()*pTurret->GetVisualRange();

	// This is a hack to get around the fact that the gun may be rotated (like the hanging guns).
	m_fMinPitch = pTurret->GetMinPitch();
	m_fMaxPitch = pTurret->GetMaxPitch();
	m_fMinYaw   = pTurret->GetMinYaw();
	m_fMaxYaw   = pTurret->GetMaxYaw();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVision::SetReactionRate
//
//	PURPOSE:	Determines if the focus can be shot at without hitting something else.
//
// ----------------------------------------------------------------------- //
bool CTurretVision::FocusCanBeShotFrom(const LTVector & vShootFromPos, LTFLOAT fWeaponRangeSqr) const
{
	if( !m_hFocus )
		return LTFALSE;

	LTVector vFocusPos;
	g_pLTServer->GetObjectPos(m_hFocus, &vFocusPos);

	if( (vShootFromPos - vFocusPos).MagSqr() > fWeaponRangeSqr )
		return LTFALSE;

	// Do an interesect check to be sure nothing is blocking view.
	IntersectQuery iQuery;
	IntersectInfo iInfo;

	iQuery.m_From = vShootFromPos;
	iQuery.m_To = vFocusPos;
	iQuery.m_Flags	  = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iQuery.m_FilterFn = m_bCanShootThrough ? TurretShootThroughFilterFn : TurretShootingFilterFn;
	iQuery.m_PolyFilterFn = LTNULL;
	iQuery.m_pUserData = m_hFocus;


	LTBOOL bDone = LTFALSE;
	int nIterations = 0;
	const int c_nMaxIterations = 10;
	while( !bDone && nIterations < c_nMaxIterations)
	{
		g_cIntersectSegmentCalls++;
		++nIterations;
		bDone = true;

		if( g_pLTServer->IntersectSegment(&iQuery, &iInfo) )
		{
			// Is it the object that we hit?
			if (iInfo.m_hObject == m_hFocus)
			{
				return true;
			}

			// Return true if the blocking object is something we would also like to kill.
			CCharacter * pChar = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(iInfo.m_hObject) );
			if( pChar && GetCanBeShot( pChar->GetCharacterClass() )  )
				return true;

			// Maybe we want to continue.
			if( m_bCanShootThrough )
			{
				const SurfaceType eSurfType = GetSurfaceType(iInfo); 

				const bool bHitWorld = (LT_YES == g_pLTServer->Physics()->IsWorldObject(iInfo.m_hObject));

				SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
				
				if (!bHitWorld && pSurf )
				{
					if( pSurf->bCanShootThrough )
					{
						iQuery.m_From = iInfo.m_Point;

						// Keep doing intersects if the impact point is not too near the impact point.
						bDone = iQuery.m_From.Equals(iQuery.m_To,1.0f);
					}
				}
			} //if( bShootThrough || bSeeThrough)

		} // if( g_pLTServer->IntersectSegment(&iQuery, &iInfo) )
	} // while( !bDone && nIterations < c_nMaxIterations)

	_ASSERT( nIterations < c_nMaxIterations );

	// Return true if we didn't hit anything at all,
	// otherwise return false.
	return nIterations == 1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVision::GetRank
//
//	PURPOSE:	Determines rank of character class (like PREDATOR, MARINE, ALIEN).
//
// ----------------------------------------------------------------------- //
static int TranslateCharacterClass(int character_class)
{
	// Translate a few of the more bizarre character class's.
	if( character_class == MARINE_EXOSUIT)
		character_class = MARINE;
	else if( CORPORATE_EXOSUIT == character_class )
		character_class = CORPORATE;
	else if( SYNTHETIC == character_class )
		character_class = CORPORATE;

	return character_class;
}

int CTurretVision::GetRank(int character_class) const
{
	character_class = TranslateCharacterClass(character_class);

	// Be sure we are within range of the array.
	if( character_class < 0 || character_class > 4 )
	{
		_ASSERT( 0 );
		return -1;
	}

	// Return the appropriate rank!
	return m_nCharacterRanks[ character_class ];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVision::GetCanBeShot
//
//	PURPOSE:	Determines if character class (like PREDATOR, MARINE, ALIEN) can be shot.
//
// ----------------------------------------------------------------------- //
LTBOOL CTurretVision::GetCanBeShot(int character_class) const
{
	character_class = TranslateCharacterClass(character_class);

	// Be sure we are within range of the array.
	if( character_class < 0 || character_class > 4 )
	{
		_ASSERT( 0 );
		return LTFALSE;
	}

	// Return the appropriate can be shot!
	return m_bCanBeShot[ character_class ];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVision::SetReactionRate
//
//	PURPOSE:	Sets the default accumulation rate and changes all active senses.
//
// ----------------------------------------------------------------------- //

void CTurretVision::SetReactionRate(LTFLOAT fRate)
{
	m_fReactionRate = fRate;

	for( InstanceList::iterator iter =  m_instances.begin();
	     iter != m_instances.end(); ++iter )
	{
		iter->second->SetAccumRate(fRate);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVision::SetDeactionRate
//
//	PURPOSE:	Sets the default decay rate and changes all active senses.
//
// ----------------------------------------------------------------------- //

void CTurretVision::SetDeactionRate(LTFLOAT fRate)
{
	m_fDeactionRate = fRate;

	for( InstanceList::iterator iter =  m_instances.begin();
	     iter != m_instances.end(); ++iter )
	{
		iter->second->SetDecayRate(fRate);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVision::PruneDeadInstances
//
//	PURPOSE:	This removes any instances which refer to a dead or non-existant
//				character.  This should be called before any use of the first
//				element in the InstanceList elements.
//
// ----------------------------------------------------------------------- //

void CTurretVision::PruneDeadInstances()
{
	InstanceList::iterator iter = m_instances.begin();
	while(iter != m_instances.end())
	{
		const CCharacter * pTarget = iter->first;
		if( std::find( g_pCharacterMgr->BeginCharacters(), g_pCharacterMgr->EndCharacters(), pTarget ) == g_pCharacterMgr->EndCharacters() )
		{
			if( pTarget->m_hObject == m_hFocus )
			{
				m_hFocus = LTNULL;
				m_nFocusRank = -1;
				m_bFocusIsVisible = false;
				m_cbFullStimulation(*this, LTNULL);
			}

			m_instances.erase( iter++ );
		}
		else if (pTarget->IsDead() || (g_pLTServer->GetObjectState(pTarget->m_hObject) & OBJSTATE_INACTIVE))
		{
			if( pTarget->m_hObject == m_hFocus )
			{
				m_hFocus = LTNULL;
				m_nFocusRank = -1;
				m_bFocusIsVisible = false;
				m_cbFullStimulation(*this, LTNULL);
			}

			m_instances.erase(iter++);
		}
		else
		{
			++iter;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVision::Update
//
//	PURPOSE:	Perform the vision test.
//
// ----------------------------------------------------------------------- //

void CTurretVision::Update()
{
	// Remove any instances refering to dead characters.
	PruneDeadInstances();

#ifdef TURRET_SENSE_DEBUG
	LineSystem::GetSystem(this,"ShowSee")
		<< LineSystem::Clear();
#endif

	// Check for characters within our vision range, adding or removing instances as needed.
	DVector vFrom;
	g_pServerDE->GetObjectPos( m_hOwner, &vFrom);

	{ for(CCharacterMgr::CharacterIterator iter = g_pCharacterMgr->BeginCharacters();
	      iter != g_pCharacterMgr->EndCharacters(); ++iter )
	{
		const CCharacter * const pChar = *iter;

		DVector vSeperation;
		g_pServerDE->GetObjectPos( pChar->m_hObject, &vSeperation);
		vSeperation -= vFrom;

		if( IsVisible(pChar) && !pChar->IsDead() && !(g_pLTServer->GetObjectState(pChar->m_hObject) & OBJSTATE_INACTIVE))
		{
			// Add new sense instance if one doesn't exist.
			if( m_instances.find( pChar ) == m_instances.end() )
			{
				m_instances[ pChar ] 
					= new CTurretVisionInstance(*this,*pChar, m_fReactionRate, m_fDeactionRate);
			}

			m_instances[ pChar ]->SetVisible(LTTRUE);
		}
		else
		{
			// Remove sense instance if target is out of range and stimulation level has dropped to zero.
			if( m_instances.find( pChar ) != m_instances.end() )
			{
				m_instances[ pChar ]->SetVisible(LTFALSE);

				if( m_instances[pChar]->GetStimulationLevel() == 0.0f )
				{
					if( pChar->m_hObject == m_hFocus )
					{
						m_hFocus = LTNULL;
						m_nFocusRank = -1;
						m_bFocusIsVisible = false;
						m_cbFullStimulation(*this, LTNULL);
					}

					delete m_instances[pChar];
					m_instances.erase( pChar );
				}
			}
		}
	}}

	if( m_instances.empty() ) return;

	// Update the sense instances.
	{for( InstanceList::const_iterator iter = m_instances.begin();
	  	  iter != m_instances.end(); ++iter )
	{
		CTurretVisionInstance & instance = *(iter->second);
		if( instance.Check() )
		{
			if( instance.GetTarget().m_hObject == m_hFocus )
			{
				m_bFocusIsVisible = instance.IsVisible();
			}
			else if( instance.IsFull() )
			{
				FullStimulation(instance.GetTarget());
				m_bFocusIsVisible = instance.IsVisible();
			}
		}


#ifdef TURRET_SENSE_DEBUG
//		g_pServerDE->CPrint( "%f %s : Instance for %s, at level %f.",
//			g_pServerDE->GetTime(),
//			g_pServerDE->GetObjectName( (*iter).second->GetOwner().GetOwner() ),
//			g_pServerDE->GetObjectName( (*iter).second->GetTarget().m_hObject ),
//			(*iter).second->GetStimulationLevel() );
#endif
	}}		 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVision::IsVisible
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
bool CTurretVision::IsVisible(const CCharacter * pChar)
{
	DVector vOwnerPos;
	g_pServerDE->GetObjectPos(m_hOwner, &vOwnerPos);

	DVector vTargetPos;
	g_pServerDE->GetObjectPos(pChar->m_hObject, &vTargetPos);
	DVector vTargetDisplacement = vTargetPos - vOwnerPos;

	// Be sure target is within range.
	const LTFLOAT fTargetDistSqr = vTargetDisplacement.MagSqr();

	if(  fTargetDistSqr > m_fMaxRangeSqr ) return false;

	// Be sure target is within arc of fire.
	DRotation rOwnerRot;
	g_pServerDE->GetObjectRotation(m_hOwner, &rOwnerRot);

	DRotation rRot;
	DVector vAngle;
	g_pMathLT->AlignRotation(rRot, vTargetDisplacement, m_vOwnerUp);
	rRot = (~rOwnerRot)*rRot;
	g_pMathLT->GetEulerAngles(rRot,vAngle);

	if(    vAngle.y < m_fMinYaw   || vAngle.y > m_fMaxYaw )
		return false;

	if( vAngle.x < m_fMinPitch || vAngle.x > m_fMaxPitch ) 
	{
		LTVector vTargetDims;
		g_pLTServer->GetObjectDims(pChar->m_hObject, &vTargetDims);

		// If we are at our min/max pitch, only back out if the target's 
		// dims are smaller than the vertical displacement.  Otherwise,
		// we can still hit the target, so keep seeing it!
		const LTFLOAT fTargetDistVertDist = (LTFLOAT)fabs(vTargetDisplacement.Dot(m_vOwnerUp));
		const LTFLOAT fTargetVertDims = (LTFLOAT)fabs(vTargetDims.Dot(m_vOwnerUp));
		
		if( fTargetVertDims < fTargetDistVertDist )
			return false;
	}

	// Do an interesect check to be sure nothing is blocking view.
	IntersectQuery iQuery;
	IntersectInfo iInfo;

	VEC_COPY(iQuery.m_From, vOwnerPos);
	VEC_COPY(iQuery.m_To, vTargetPos);
	iQuery.m_Flags	  = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iQuery.m_FilterFn = m_bCanSeeThrough ? TurretSeeThroughFilterFn : TurretSeeFilterFn;
	iQuery.m_PolyFilterFn = LTNULL;
	iQuery.m_pUserData = pChar->m_hObject;

#ifdef TURRET_SENSE_DEBUG
//	AICPrint(m_hOwner, "Looking from (%.2f,%.2f,%.2f).",
//		iQuery.m_From.x,iQuery.m_From.y,iQuery.m_From.z);

	LineSystem::GetSystem(this,"ShowSee")
		<< LineSystem::Line(iQuery.m_From, iQuery.m_To, Color::Purple);
#endif

	LTBOOL bDone = LTFALSE;
	int nIterations = 0;
	const int c_nMaxIterations = 10;
	while( !bDone && nIterations < c_nMaxIterations)
	{
		g_cIntersectSegmentCalls++;
		++nIterations;
		bDone = true;

		if( g_pLTServer->IntersectSegment(&iQuery, &iInfo) )
		{
			// Is it the object that we hit?
			if (iInfo.m_hObject == pChar->m_hObject)
			{
				return true;
			}

			// Maybe we want to continue.
			if( m_bCanSeeThrough )
			{
				const SurfaceType eSurfType = GetSurfaceType(iInfo); 

				const bool bHitWorld = (LT_YES == g_pLTServer->Physics()->IsWorldObject(iInfo.m_hObject));

				SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
				
				if (!bHitWorld && pSurf )
				{
					if( pSurf->bCanSeeThrough )
					{
						iQuery.m_From = iInfo.m_Point;

						// Keep doing intersects if the impact point is not too near the impact point.
						bDone = iQuery.m_From.Equals(iQuery.m_To,1.0f);
					}
				}
			} //if( bShootThrough || bSeeThrough)

		} // if( g_pLTServer->IntersectSegment(&iQuery, &iInfo) )

	} //while( !bDone && nIterations < c_nMaxIterations)

	_ASSERT( nIterations < c_nMaxIterations );

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVision::FullStimulation
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CTurretVision::FullStimulation(const CCharacter & character)
{
	m_hFocus = character.m_hObject;
	m_nFocusRank = GetRank( character.GetCharacterClass() );
	m_cbFullStimulation(*this, &character);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVision::Save
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CTurretVision::Save(LMessage * pMsg)
{
	if (!pMsg) return;

	PruneDeadInstances();

	*pMsg << m_hOwner;
	*pMsg << m_hFocus;

	pMsg->WriteDWord(m_nFocusRank);
	*pMsg << m_bFocusIsVisible;

	pMsg->WriteDWord(m_instances.size());
	for( InstanceList::iterator iter = m_instances.begin();
	     iter != m_instances.end(); ++iter )
	{
		const CCharacter * const pTarget = iter->first;
		CTurretVisionInstance * const pInstance = iter->second;

		ASSERT( pTarget && pTarget->m_hObject && pInstance );

		*pMsg << const_cast<HOBJECT>(pTarget->m_hObject);
		if( pTarget->m_hObject )
		{
			*pMsg << *pInstance;
		}
	}

	*pMsg << m_vOwnerUp;
	*pMsg << m_fMaxRangeSqr;
	*pMsg << m_fMaxPitch;
	*pMsg << m_fMinPitch;
	*pMsg << m_fMaxYaw;
	*pMsg << m_fMinYaw;

	*pMsg << m_fReactionRate;
	*pMsg << m_fDeactionRate;

	*pMsg << m_bCanSeeThrough;
	*pMsg << m_bCanShootThrough;

	// m_cbFullStimulation must be handled by owner.

	*pMsg << m_nCharacterRanks[0];
	*pMsg << m_nCharacterRanks[1];
	*pMsg << m_nCharacterRanks[2];
	*pMsg << m_nCharacterRanks[3];
	*pMsg << m_nCharacterRanks[4];

	*pMsg << m_bCanBeShot[0];
	*pMsg << m_bCanBeShot[1];
	*pMsg << m_bCanBeShot[2];
	*pMsg << m_bCanBeShot[3];
	*pMsg << m_bCanBeShot[4];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTurretVision::Load
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CTurretVision::Load(LMessage * pMsg)
{
	if (!pMsg) return;

	*pMsg >> m_hOwner;
	*pMsg >> m_hFocus;

	m_nFocusRank = int(pMsg->ReadDWord());
	*pMsg >> m_bFocusIsVisible;

	const int size = pMsg->ReadDWord();
	for( int i = 0; i < size; ++i)
	{
		HOBJECT hTarget;
		*pMsg >> hTarget;

		if( hTarget )
		{
			CCharacter * pTarget = dynamic_cast<CCharacter*>( g_pServerDE->HandleToObject(hTarget) );
			if( pTarget )
			{
				CTurretVisionInstance * pInstance = new CTurretVisionInstance(*this,*pTarget);
				*pMsg >> *pInstance;

				m_instances[ pTarget ] = pInstance;
			}
			else
			{
				ReadDeadVisionInstanceData(pMsg);
			}

		}
		else
		{
			ReadDeadVisionInstanceData(pMsg);
		}
	}

	*pMsg >> m_vOwnerUp;
	*pMsg >> m_fMaxRangeSqr;
	*pMsg >> m_fMaxPitch;
	*pMsg >> m_fMinPitch;
	*pMsg >> m_fMaxYaw;
	*pMsg >> m_fMinYaw;

	*pMsg >> m_fReactionRate;
	*pMsg >> m_fDeactionRate;

	*pMsg >> m_bCanSeeThrough;
	*pMsg >> m_bCanShootThrough;

	// m_cbFullStimulation must be handled by owner.

	*pMsg >> m_nCharacterRanks[0];
	*pMsg >> m_nCharacterRanks[1];
	*pMsg >> m_nCharacterRanks[2];
	*pMsg >> m_nCharacterRanks[3];
	*pMsg >> m_nCharacterRanks[4];

	*pMsg >> m_bCanBeShot[0];
	*pMsg >> m_bCanBeShot[1];
	*pMsg >> m_bCanBeShot[2];
	*pMsg >> m_bCanBeShot[3];
	*pMsg >> m_bCanBeShot[4];

}

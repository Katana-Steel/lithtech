// ----------------------------------------------------------------------- //
//
// MODULE  : CAINodeMgr.cpp
//
// PURPOSE : CAINodeMgr implementation
//
// CREATED : 1/6/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AINodeMgr.h"
#include "ServerUtilities.h"
#include "VolumeBrushTypes.h"
#include "SurfaceFunctions.h"
#include "ctype.h"
#include "SFXMsgIds.h"
#include "NodeLine.h"
#include "AIUtils.h"
#include "AIVolume.h"
#include "ObjectMsgs.h"
#include "AINodeGroup.h"
#include "AI.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "CharacterHitBox.h"
#include "ParseUtils.h"

#include "AINode.h"
#include "AINodeCover.h"
#include "AINodeSnipe.h"
#include "AINodeAlarm.h"
#include "AINodeVolume.h"
#include "AINodeWallWalk.h"

#include <algorithm>

// Globals

CAINodeMgr* g_pAINodeMgr = LTNULL;

// Externs

extern int g_cIntersectSegmentCalls;

BEGIN_CLASS(AINodeMgr)
END_CLASS_DEFAULT(AINodeMgr, GameBase, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD AINodeMgr::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	if (!g_pServerDE) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			CommonLT* pCommon = g_pServerDE->Common();
			if (!pCommon) 
			{
				_ASSERT(0);
				return LT_ERROR;
			}

			LTString hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			const char* szMsg = hMsg ? g_pServerDE->GetStringData(hMsg) : DNULL;
			if (!szMsg) 
			{
				_ASSERT(0);
				return LT_ERROR;
			}

			if( !g_pAINodeMgr )
			{
#ifndef _FINAL
				g_pLTServer->CPrint("%s : Could not find node manager!!!!!!!",
					g_pLTServer->GetObjectName(m_hObject) );
				g_pLTServer->CPrint("%s : Trigger \"%s\" ignored.", 
					g_pLTServer->GetObjectName(m_hObject), szMsg );
#endif
				return LT_ERROR;
			}

			ConParse parse;
			parse.Init(const_cast<char*>(szMsg));

			while (pCommon->Parse(&parse) == LT_OK)
			{
				if( parse.m_nArgs == 2 )
				{
					bool bOn = true;
					
					if(0 == stricmp("OFF",parse.m_Args[1]))
					{
						bOn = false;
					}
					else if( 0 != stricmp("ON",parse.m_Args[1]) )
					{
#ifndef _FINAL
						g_pLTServer->CPrint("%s : Command \"%s\" not understood, assuming you meant \"ON\".",
							g_pLTServer->GetObjectName(m_hObject), parse.m_Args[0]);
#endif
					}

					for( CAINodeMgr::NodeList::iterator iter = g_pAINodeMgr->BeginNodes();
					     iter != g_pAINodeMgr->EndNodes(); ++iter )
					{
						if( 0 == stricmp( parse.m_Args[0], (*iter)->GetName().c_str()) )
						{
							(*iter)->SetActive(bOn);
						}
					}
				}
				else
				{
#ifndef _FINAL
					g_pLTServer->CPrint("%s : Incorrect format found in \"%s\".",
						g_pLTServer->GetObjectName(m_hObject), szMsg);
					g_pLTServer->CPrint("%s : Format should be \"(node_name {on,off}; node_name {on,off};...\".",
						g_pLTServer->GetObjectName(m_hObject));
#endif
				}

			}
		}
		break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::CAINodeMgr
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CAINodeMgr::CAINodeMgr()
{
	g_pAINodeMgr = this;

	m_bInitialized = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::Term
//
//	PURPOSE:	Terminates the AINodeMgr
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::Term()
{
	m_bInitialized = LTFALSE;

	while( !m_apNodes.empty() )
	{
		delete m_apNodes.back();
		m_apNodes.pop_back();
	}

	m_apCoverNodes.clear();
	m_apSnipeNodes.clear();
	m_apAlarmNodes.clear();
	m_apWWAdvanceNodes.clear();

#ifndef __LINUX
	m_AITeleportList.clear();
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::Init
//
//	PURPOSE:	Create a list of all the Nodes in the level.
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::Init()
{
	// Find all the AINodes and make corresponding CAINodes.
	{for( NodeObjectList::iterator iter = m_apNodeObjects.begin(); iter != m_apNodeObjects.end(); ++iter )
	{
		AINode * pAINode = *iter;
		if( pAINode )
		{
			CAINode * pCAINode = pAINode->MakeCAINode(m_apNodes.size());
			if( pCAINode )
				m_apNodes.push_back(pCAINode);
		}
	}}

	// And remove the AINodes.
	while( !m_apNodeObjects.empty() )
	{
		AINode * pAINode = m_apNodeObjects.back();
		if( pAINode )
		{
			g_pLTServer->RemoveObject(pAINode->m_hObject);
		}

		m_apNodeObjects.pop_back();
	}

	// Store cover nodes seperately
	FillSpecialNodeLists();

	m_bInitialized = LTTRUE;

	
	// Teleport the AI's waiting to be teleported.
	{ for( AITeleportList::iterator iter = m_AITeleportList.begin(); iter != m_AITeleportList.end(); ++iter )
	{
		CAI * const pAI = iter->first;
		const std::string & strNodeName = iter->second;

		const ParseLocation result(strNodeName.c_str(), *pAI); 
		if( result.error.empty() )
		{
			g_pLTServer->SetObjectPos(pAI->m_hObject, const_cast<LTVector*>(&result.vPosition) );
		}
	}}

	m_AITeleportList.clear();
}

void CAINodeMgr::AddTeleportee(CAI * pAI, const std::string & strNodeName)
{
	ASSERT( pAI );
	ASSERT( !strNodeName.empty() );

	if( !m_bInitialized )
	{
		m_AITeleportList.push_back( std::make_pair(pAI, strNodeName) );
	}
	else
	{
		const ParseLocation result(strNodeName.c_str(), *pAI);  
		if( result.error.empty() )
		{
			g_pLTServer->SetObjectPos(pAI->m_hObject, const_cast<LTVector*>(&result.vPosition) );
		}
	}
}

void CAINodeMgr::FillSpecialNodeLists()
{
	if (!m_apCoverNodes.empty()) 
		m_apCoverNodes.clear();

	if (!m_apSnipeNodes.empty())
		m_apSnipeNodes.clear();

	if (!m_apAlarmNodes.empty())
		m_apAlarmNodes.clear();

	if (!m_apWWAdvanceNodes.empty())
		m_apWWAdvanceNodes.clear();

	{for (NodeList::iterator iter = m_apNodes.begin();
	     iter != m_apNodes.end(); ++iter) 
	{
		if (dynamic_cast<CAINodeCover *>(*iter))
		{
			m_apCoverNodes.push_back( (CAINodeCover *)*iter );
		}
		else if (dynamic_cast<CAINodeSnipe *>(*iter))
		{
			m_apSnipeNodes.push_back((CAINodeSnipe *)*iter);
		}
		else if (dynamic_cast<CAINodeAlarm *>(*iter))
		{
			m_apAlarmNodes.push_back((CAINodeAlarm *)*iter);
		}
		else if (CAINodeWallWalk * pWWNode = dynamic_cast<CAINodeWallWalk *>(*iter))
		{
			if( pWWNode->IsAdvanceNode() )
				m_apWWAdvanceNodes.push_back(pWWNode);
		}
	}}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::Load
//
//	PURPOSE:	Restores the state of the AINodeMgr
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::Load(HMESSAGEREAD hRead)
{
	uint32 dwNodes, iNode;

	ASSERT( hRead );
	if( !hRead ) return;

	Term();

	*hRead >> m_bInitialized;

	dwNodes = hRead->ReadDWord();
	for ( iNode = 0 ; iNode < dwNodes; iNode++ )
	{
		m_apNodes.push_back(CAINode::LoadCAINode(*hRead));
	}
	
	FillSpecialNodeLists();
	// m_apCoverNodes set by FillSpecialNodes
	// m_apSnipeNodes set by FillSpecialNodes
	// m_apAlarmNodes set by FillSpecialNodes
	// m_apWWAdvanceNodes set by FillSpecialNodes
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::Save
//
//	PURPOSE:	Saves the state of the AINodeMgr
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	*hWrite << m_bInitialized;

	hWrite->WriteDWord(m_apNodes.size());
	{for ( NodeList::iterator iter = m_apNodes.begin();
	      iter != m_apNodes.end(); ++iter )
	{
		// We should never have a null pointer in this list
		_ASSERT(*iter);

		CAINode::SaveCAINode(*hWrite, *(*iter) );
	}}

	
	// m_apCoverNodes set by FillSpecialNodes in CAINodeMgr::Load
	// m_apSnipeNodes set by FillSpecialNodes in CAINodeMgr::Load
	// m_apAlarmNodes set by FillSpecialNodes in CAINodeMgr::Load

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::AddNodeDebug
//
//	PURPOSE:	Add the node models
//
// ----------------------------------------------------------------------- //

class SortNodesAround
{
	const LTVector & vAroundPos;
public :

	SortNodesAround( const LTVector & vPos )
		: vAroundPos(vPos) {}

	bool operator()( const CAINode * pNodeA, const CAINode * pNodeB)
	{
		return (pNodeA->GetPos() - vAroundPos).MagSqr() < (pNodeB->GetPos() - vAroundPos).MagSqr();
	}
};

#ifndef _FINAL

void CAINodeMgr::AddNodeDebug(int level)
{
	std::vector<CAINode*> sorted_nodes;


	{for(NodeList::iterator iter = m_apNodes.begin();
	    iter != m_apNodes.end(); ++iter)
	{
		sorted_nodes.push_back( (*iter) );
	}}


	CCharacterMgr::PlayerIterator player_iter = g_pCharacterMgr->BeginPlayers();
	if( player_iter != g_pCharacterMgr->EndPlayers() )
	{
		CPlayerObj * pPlayerObj = *player_iter;
		const LTVector & vPlayerPos = pPlayerObj->GetPosition();

		std::sort( sorted_nodes.begin(), sorted_nodes.end(), SortNodesAround( vPlayerPos ) );
	}

	{for ( NodeList::iterator iter = m_apNodes.begin();
	      iter != m_apNodes.end(); ++iter )
	{
		if( (*iter)->IsActive() )
			(*iter)->DrawDebug(level);
	}}
}

#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::RemoveNodeDebug
//
//	PURPOSE:	Removes the node models
//
// ----------------------------------------------------------------------- //

#ifndef _FINAL

void CAINodeMgr::RemoveNodeDebug()
{
	{for ( NodeList::iterator iter = m_apNodes.begin();
		  iter != m_apNodes.end(); ++iter )
	{
		(*iter)->ClearDebug();
	}}
}

#endif

//-------------------------------------------------------------------------



// SortNodesByDist
//
// Performs a quick sort on the node pointer list,
// based on the distance of each node to the position given
void CAINodeMgr::SortNodesByDist(CoverNodeList &aNodes, uint32 lo, uint32 hi, const LTVector& vPos)
{
	uint32 dwHigh, dwLow;
	LTFLOAT fPivot, fDistSqr;
	CAINodeCover * pTemp;
	
	if (lo < hi)
	{
		dwLow = lo;					// index
		dwHigh = hi;				// index
		fPivot = VEC_DISTSQR(vPos, aNodes[(hi - lo / 2)]->GetPos());
		
		do
		{
			fDistSqr = VEC_DISTSQR(vPos, aNodes[dwLow]->GetPos());
			while ((dwLow < dwHigh) && (fDistSqr <= fPivot))
			{
				dwLow = dwLow + 1;
				fDistSqr = VEC_DISTSQR(vPos, aNodes[dwLow]->GetPos());
			}

			fDistSqr = VEC_DISTSQR(vPos, aNodes[dwHigh]->GetPos());
			while ((dwHigh > dwLow) && (fDistSqr >= fPivot))
			{
				dwHigh = dwHigh-1;
				fDistSqr = VEC_DISTSQR(vPos, aNodes[dwHigh]->GetPos());
			}

			// Swap nodes
			if (dwLow < dwHigh)
			{
				pTemp = aNodes[dwLow];
				aNodes[dwLow] = aNodes[dwHigh];
				aNodes[dwHigh] = pTemp;
			}
		}
		while (dwLow < dwHigh);
		
		pTemp = aNodes[dwLow];
		aNodes[dwLow] = aNodes[hi];
		aNodes[hi] = pTemp;
		
		if (dwLow > 0)
			SortNodesByDist(aNodes, lo, dwLow - 1, vPos);
	
		if (dwLow < hi)
			SortNodesByDist(aNodes, dwLow + 1, hi, vPos);
	}
}


namespace 
{
	class CmpDist
	{
		LTVector vPos;

	public:

		CmpDist(LTVector new_vPos)
			: vPos(new_vPos) {}

		bool operator()(const CAINode * pX, const CAINode * pY) const
		{
			// This gets optimized incorrectly if you don't write
			// this so explicitly.
			LTVector xPos = pX->GetPos();
			xPos -= vPos;
			LTFLOAT xDistSqr = xPos.MagSqr();
			
			LTVector yPos = pY->GetPos();
			yPos -= vPos;
			LTFLOAT yDistSqr = yPos.MagSqr();

			return  xDistSqr < yDistSqr;
		}
	};
}

// use like this : 	std::sort( m_aCoverNode.begin(), m_aCoverNode.begin()+10, CmpDist(vMyPos) );

//--------------------------------------------------------------------
// FindAdvanceNode
//
// This finds the nearest cover node with some extra rules to
// facilitate advancing on a target:
// - must not be locked
// - must be closer to the target than the AI's current position
// - must not be visible to the target
//--------------------------------------------------------------------
CAINodeCover* CAINodeMgr::FindAdvanceNode(HOBJECT hAI, HOBJECT hTarget)
{
	LTVector vAI(0,0,0);
	LTVector vTarget(0,0,0);

	if (!g_pAINodeMgr->IsInitialized())
		return LTNULL;

	CAI *pAI = dynamic_cast<CAI *>(g_pInterface->HandleToObject(hAI));
	if (!pAI)
		return LTNULL;

	g_pInterface->GetObjectPos(hAI, &vAI);
	g_pInterface->GetObjectPos(hTarget, &vTarget);

	const LTFLOAT fAIToTargetDistSqr = (vAI - vTarget).MagSqr();

	std::sort(m_apCoverNodes.begin(), m_apCoverNodes.end(), CmpDist(vAI));

	static CoverNodeList tempNodes;
	tempNodes.clear();

	{for (uint32 i = 0; i < m_apCoverNodes.size(); i++)
	{
		if (!m_apCoverNodes[i]->IsLocked())
		{
			const LTFLOAT fNodeToTargetDistSqr = (m_apCoverNodes[i]->GetPos() - vTarget).MagSqr();

			// Node must be closer to the target in order to be an "advance" node
			if (fNodeToTargetDistSqr < fAIToTargetDistSqr)
			{
				// check LOS
//				if (!TestLOSIgnoreCharacters(hTarget, NULL, aNodes[i]->GetPos()))
				if (m_apCoverNodes[i]->IsCoverFromThreat(*pAI ,hTarget))
				{
					// copy valid nodes for a final check (see below)
					tempNodes.push_back(m_apCoverNodes[i]);
				}
			}
		}
	}}


	// This portion of the code attempts to select a node in such a way
	// that the AIs will follow a criss-cross pattern as they approach.
	LTVector vDirToTarget = vTarget - vAI;
	vDirToTarget.Norm();

	LTFLOAT fResult = 1.0f, fTemp = 1.0f;
	CAINodeCover *pTempNode = LTNULL;
	{for (uint32 i = 0; i < tempNodes.size(); i++)
	{
		LTVector vDirToNode = tempNodes[i]->GetPos() - vAI;
		vDirToNode.Norm();

		fTemp = vDirToTarget.Dot(vDirToNode);

		// Dot products closer to zero cause lateral movement
		if (fTemp < fResult)
		{
			fResult = fTemp;
			pTempNode = tempNodes[i];
		}
	}}
	
	return pTempNode;

//	return LTNULL;
}

// Returns the nearest unlocked Cover node that does not have LOS to the AI's target.
// The AI must be within the node's radius.
CAINodeCover* CAINodeMgr::FindDodgeNode(HOBJECT hAI, HOBJECT hAttacker)
{
	LTVector vAI;
	CAI *pAI;
	LTFLOAT fDistSqr;

	if (!g_pAINodeMgr->IsInitialized())
		return LTNULL;

	pAI = dynamic_cast<CAI *>(g_pInterface->HandleToObject(hAI));
	if (!pAI)
		return LTNULL;

	g_pInterface->GetObjectPos(hAI, &vAI);

	std::sort(m_apCoverNodes.begin(), m_apCoverNodes.end(), CmpDist(vAI));

	{for (uint32 i = 0; i < m_apCoverNodes.size(); i++)
	{
		if (!m_apCoverNodes[i]->IsLocked() || m_apCoverNodes[i]->GetLockOwner() == hAI)
		{
			fDistSqr = VEC_DISTSQR(vAI, m_apCoverNodes[i]->GetPos());
			if (fDistSqr < m_apCoverNodes[i]->GetCoverRadiusSqr())
			{
//				if (!TestLOSIgnoreCharacters(hAttacker, NULL, aNodes[i]->GetPos()))
				if (m_apCoverNodes[i]->IsCoverFromThreat(*pAI, hAttacker))
				{
					return m_apCoverNodes[i];
				}
			}
		}
	}}

	return LTNULL;
}

// Returns the nearest unlocked wall walk node (set to AttackPoint TRUE) .
CAINodeWallWalk * CAINodeMgr::FindWWAdvanceNode(HOBJECT hAI, HOBJECT hTarget)
{
	if (!g_pAINodeMgr->IsInitialized())
		return LTNULL;

	LTVector vAI;
	g_pInterface->GetObjectPos(hAI, &vAI);

	LTVector vTarget;
	g_pInterface->GetObjectPos(hTarget, &vTarget);

	const LTFLOAT fAIToTargetDistSqr = (vAI - vTarget).MagSqr();

	std::sort(m_apWWAdvanceNodes.begin(), m_apWWAdvanceNodes.end(), CmpDist(vTarget));

	{for( WWAdvanceNodeList::iterator iter = m_apWWAdvanceNodes.begin(); 
	     iter != m_apWWAdvanceNodes.end(); ++iter )
	{
		CAINodeWallWalk * pNode = *iter;
		if( !pNode->IsLocked() )
		{
			const LTFLOAT fNodeToTargetDistSqr = (pNode->GetPos() - vTarget).MagSqr();

			// Node must be closer to the target in order to be an "advance" node
			if( fNodeToTargetDistSqr < fAIToTargetDistSqr )
			{
				// check LOS
				if( TestLOSIgnoreCharacters(hTarget, NULL, pNode->GetPos()) )
				{
					return pNode;
				}
			}
		}
	}}


	return LTNULL;
}

// If there is a valid target, returns the nearest snipe node to that target;
// otherwise returns the nearest snipe node to the AI.  Only returns an unlocked node.
CAINodeSnipe* CAINodeMgr::FindSnipeNode(HOBJECT hAI, HOBJECT hTarget, HOBJECT hGroup)
{
	SnipeNodeList aNodes;
	LTVector vObj;
	HOBJECT hObj;

	if (hTarget)
		hObj = hTarget;
	else
		hObj = hAI;

	if (!g_pAINodeMgr->IsInitialized())
		return LTNULL;

	g_pInterface->GetObjectPos(hObj, &vObj);

	{for (uint32 i = 0; i < m_apSnipeNodes.size(); i++)
	{
		// Only add nodes whose AINodeGroup matches the provided one
		if (!hGroup || (hGroup == m_apSnipeNodes[i]->GetAINodeGroup()))
			aNodes.push_back(m_apSnipeNodes[i]);
	}}

	std::sort(aNodes.begin(), aNodes.end(), CmpDist(vObj));

	{for (uint32 i = 0; i < aNodes.size(); i++)
	{
		if (!aNodes[i]->IsLocked())
			return aNodes[i];
	}}
	
	return LTNULL;
}

// Returns the nearest unlocked Alarm node to the AI
CAINodeAlarm* CAINodeMgr::FindAlarmNode(HOBJECT hAI, HOBJECT hGroup)
{
	AlarmNodeList aNodes;
	LTVector vAI;

	if (!g_pAINodeMgr->IsInitialized())
		return LTNULL;

	g_pInterface->GetObjectPos(hAI, &vAI);

	{for (uint32 i = 0; i < m_apAlarmNodes.size(); i++)
	{
		// Only add nodes whose AINodeGroup matches the provided one
		if (!hGroup || (hGroup == m_apAlarmNodes[i]->GetAINodeGroup()))
			aNodes.push_back(m_apAlarmNodes[i]);
	}}

	std::sort(aNodes.begin(), aNodes.end(), CmpDist(vAI));

	{for (uint32 i = 0; i < aNodes.size(); i++)
	{
		if (!aNodes[i]->IsLocked())
		{
			return aNodes[i];
		}
	}}
	
	return LTNULL;
}

// Returns the nearest unlocked Cover node that does not have LOS to the AI's target.
// The AI must be within the node's radius.
CAINodeCover* CAINodeMgr::FindCowerNode(HOBJECT hAI, HOBJECT hTarget, HOBJECT hGroup)
{
	CoverNodeList aNodes;
	LTVector vAI, vTarget;
	CAI *pAI;

	if (!g_pAINodeMgr->IsInitialized())
		return LTNULL;

	pAI = dynamic_cast<CAI *>(g_pInterface->HandleToObject(hAI));
	if (!pAI)
		return LTNULL;

	g_pInterface->GetObjectPos(hAI, &vAI);
	g_pInterface->GetObjectPos(hTarget, &vTarget);
	
	vAI.y = 0.0f;
	vTarget.y = 0.0f;

	const LTFLOAT fAIToTargetDistSqr = (vTarget - vAI).MagSqr();

	{for (uint32 i = 0; i < m_apCoverNodes.size(); i++)
	{
		// Only add Cower nodes whose AINodeGroup matches the provided one
		if (m_apCoverNodes[i]->GetCower())
		{
			if (!hGroup || (hGroup == m_apCoverNodes[i]->GetAINodeGroup()))
				aNodes.push_back(m_apCoverNodes[i]);
		}
	}}
	
	std::sort(aNodes.begin(), aNodes.end(), CmpDist(vAI));

	{for (uint32 i = 0; i < aNodes.size(); i++)
	{
		if (!aNodes[i]->IsLocked())
		{
			LTVector vNodePos = aNodes[i]->GetPos();
			vNodePos.y = 0.0f;
			const LTFLOAT fNodeToTargetDistSqr = (vNodePos - vTarget).MagSqr();

			// Make sure we run to a node away from the target
			if (fNodeToTargetDistSqr > fAIToTargetDistSqr)
			{
				return aNodes[i];
			}
		}
	}}

	return LTNULL;
}


static LTBOOL NoHitBox(HOBJECT hObj, void *pUserData)
{
	if ( !hObj ) return LTFALSE;

	if( dynamic_cast<CCharacterHitBox*>( g_pLTServer->HandleToObject(hObj) ) )
	{
		return LTFALSE;
	}

	return LTTRUE;
}

CAINodeSnipe* CAINodeMgr::FindVisibleSnipeNode(HOBJECT hAI, HOBJECT hTarget, HOBJECT hGroup, LTFLOAT fVerticalOffset)
{
	SnipeNodeList aNodes;
	LTVector vAI;

	if (!g_pAINodeMgr->IsInitialized())
		return LTNULL;

	g_pInterface->GetObjectPos(hAI, &vAI);

	
	IntersectQuery	IQuery;
	IntersectInfo	IInfo;
	g_pInterface->GetObjectPos(hTarget, &IQuery.m_To);
	IQuery.m_Flags	= IGNORE_NONSOLID;
	IQuery.m_FilterFn = NoHitBox;

	{for (uint32 i = 0; i < m_apSnipeNodes.size(); i++)
	{
		IQuery.m_From = m_apSnipeNodes[i]->GetPos();
		IQuery.m_From.y += fVerticalOffset;

		if (!hGroup || (hGroup == m_apSnipeNodes[i]->GetAINodeGroup()))
		{
			++g_cIntersectSegmentCalls;
			if( g_pInterface->IntersectSegment(&IQuery, &IInfo) )
			{
				aNodes.push_back(m_apSnipeNodes[i]);
			}
		}
	}}

	std::sort(aNodes.begin(), aNodes.end(), CmpDist(vAI));

	{for (uint32 i = 0; i < aNodes.size(); i++)
	{
		if (!aNodes[i]->IsLocked())
			return aNodes[i];
	}}
	
	return LTNULL;
}

uint32 CAINodeMgr::AddVolumeNodes(CAIVolume * pVolume)
{
	_ASSERT( pVolume );
	if( !pVolume ) return CAINode::kInvalidID;

	// Add the volume's center point.
	CAIVolumeNode * pVolumeNode = new CAIVolumeNode(*pVolume,m_apNodes.size());
	m_apNodes.push_back( pVolumeNode );

	pVolume->SetNodeID(pVolumeNode->GetID());

	pVolumeNode->SetActive( pVolume->StartActive() );

	// Make the neighbor connections.
	{for( CAIVolume::NeighborNodeList::const_iterator iter = pVolume->BeginNeighborNodes();
	     iter != pVolume->EndNeighborNodes(); ++iter )
	{
		// Volume center to neighbor node.
		pVolumeNode->AddNeighbor(*iter);
		m_apNodes[*iter]->AddNeighbor(pVolumeNode->GetID());

		m_apNodes[*iter]->SetActive( pVolume->StartActive() && m_apNodes[*iter]->IsActive() );

		// Connect to all the other neighbors.
		for( CAIVolume::NeighborNodeList::const_iterator iNeighbor = iter + 1;
			 iNeighbor != pVolume->EndNeighborNodes(); ++iNeighbor )
		{
			m_apNodes[*iter]->AddNeighbor(*iNeighbor);
			m_apNodes[*iNeighbor]->AddNeighbor(*iter);
		}
	}}

	return pVolumeNode->GetID();
}


uint32 CAINodeMgr::AddVolumeNeighborNode(const CAINeighborInfo & neighbor_info, const CAIVolume & volume)
{
	CAIVolumeNeighborNode * pVolumeNeighborNode = new CAIVolumeNeighborNode(neighbor_info,volume, m_apNodes.size());
	m_apNodes.push_back( pVolumeNeighborNode );

	return pVolumeNeighborNode->GetID();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::GetNode
//
//	PURPOSE:	Finds a node based on its name
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::GetNode(const char *szName)
{
	if ( !g_pLTServer ) return LTNULL;

	for ( NodeList::iterator iter = m_apNodes.begin();
	      iter != m_apNodes.end(); ++iter )
	{
		if( !(*iter)->GetName().empty() )
		{
			if ( !_stricmp((*iter)->GetName().c_str(), szName) )
			{
				return *iter;
			}
		}
	}

	return LTNULL;
}

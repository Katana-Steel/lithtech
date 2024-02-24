// ----------------------------------------------------------------------- //
//
// MODULE  : CAIVolumeMgr.cpp
//
// PURPOSE : CAIVolumeMgr implementation
//
// CREATED : 1/6/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIVolumeMgr.h"
#include "AIPath.h"
#include "DebugLineSystem.h"
#include "AINodeMgr.h"
#include "AINodeVolume.h"
#include "ObjectMsgs.h"
#include "AINodeWallWalk.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "AStar.h"


#ifdef USE_AIPOLY
#define AIPOLY_LOAD_TIMING
#define AIPOLY_FIND_NEIGHBORS
#endif

#ifdef _DEBUG
#define AIVOLUME_LOAD_TIMING
//#define AIPOLY_DEBUG
//#define AIVOLUMEMGR_DEBUG
#endif

// Globals
const uint32 g_nMaxVerticesInPoly = 32;

CAIVolumeMgr* g_pAIVolumeMgr = LTNULL;

// Helper classes

namespace /*unnamed*/
{
	const LTFLOAT g_fUnpassableCost = 1e25f;
	const LTFLOAT g_fMaxSearchCost = 1e5f;


	class VolumeHeuristic
	{
		const LTVector goal;

	public:

		VolumeHeuristic(const LTVector & new_goal)
			: goal(new_goal) {}

		LTFLOAT operator()(const CAIVolume & x) const
		{
			const CAINode * pNode = g_pAINodeMgr->GetNode( x.GetNodeID() );

			if( !pNode || !pNode->IsActive() )
				return g_fUnpassableCost;

			return (x.GetCenter() - goal).Mag();
		}
	};


	class VolumeCost
	{

	public:

		LTFLOAT operator()(const CAIVolume & src_node, const CAIVolume & dest_node) const
		{
			return (src_node.GetCenter() - dest_node.GetCenter()).Mag();
		}
	};

}; //namespace unnamed

#ifdef USE_AIPOLY
void AIPoly::finish( uint32 num_vertices )
{
	vertexList.resize(num_vertices); // NOTE: this does not release memory!

	// Find the polie's center.
	{for( VertexList::iterator iter = vertexList.begin(); 
	      iter != vertexList.end(); ++iter )
	{
		center += *iter;
	}}
	if( vertexList.size() > 0 )
		center /= LTFLOAT(vertexList.size());


	LTFLOAT max_dist_sqr = 0;
	{for( VertexList::iterator iter = vertexList.begin(); 
	      iter != vertexList.end(); ++iter )
	{
		const LTFLOAT & cur_dist_sqr = (center - *iter).MagSqr();
		if(  cur_dist_sqr > max_dist_sqr )
		{
			max_dist_sqr = cur_dist_sqr;
		}
	}}
	radius = LTFLOAT(sqrt(max_dist_sqr));
}

bool IsNeighborPoly(const AIPoly & poly1, const AIPoly & poly2)
{
	if( (poly1.center - poly2.center).MagSqr() 
		  < (poly1.radius + poly2.radius)*(poly1.radius + poly2.radius) + 0.1f )
	{
		bool sharing_one_vertex = false;

		for( AIPoly::VertexList::const_iterator iter1 = poly1.vertexList.begin(); iter1 != poly1.vertexList.end(); ++iter1 )
		{
			for( AIPoly::VertexList::const_iterator iter2 = poly2.vertexList.begin(); iter2 != poly2.vertexList.end(); ++iter2 )
			{
				if( iter2->Equals(*iter1,0.1f) )
				{
					if( !sharing_one_vertex )
					{
						sharing_one_vertex = true;
					}
					else
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}
#endif

BEGIN_CLASS(AIVolumeMgr)
END_CLASS_DEFAULT(AIVolumeMgr, GameBase, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD AIVolumeMgr::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
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
			const char* szMsg = hMsg ? g_pServerDE->GetStringData(hMsg) : LTNULL;
			if (!szMsg) 
			{
				_ASSERT(0);
				return LT_ERROR;
			}

			if( !g_pAIVolumeMgr )
			{
#ifndef _FINAL
				g_pLTServer->CPrint("%s : Could not find volume manager!!!!!!!",
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
#ifndef _FINAL
					else if( 0 != stricmp("ON",parse.m_Args[1]) )
					{
						g_pLTServer->CPrint("%s : Command \"%s\" not understood, assuming you meant \"ON\".",
							g_pLTServer->GetObjectName(m_hObject), parse.m_Args[0]);
					}
#endif

					for( CAIVolumeMgr::VolumeList::iterator iter = g_pAIVolumeMgr->BeginVolumes();
					     iter != g_pAIVolumeMgr->EndVolumes(); ++iter )
					{
						if( 0 == stricmp( parse.m_Args[0], (*iter).GetName().c_str() ) )
						{
							// Set all the nodes
							_ASSERT( g_pAINodeMgr );
							if( g_pAINodeMgr ) 
							{
								CAINode * pVolumeNode = g_pAINodeMgr->GetNode( (*iter).GetNodeID() );

								_ASSERT( pVolumeNode );
								if( pVolumeNode ) 
								{
									pVolumeNode->SetActive(bOn);

									for( CAINode::NeighborList::iterator node_iter = pVolumeNode->BeginNeighbors();
										 node_iter != pVolumeNode->EndNeighbors(); ++node_iter )
									{
										if( (*iter).Contains( (*node_iter)->GetPos() ) )
										{
											(*node_iter)->SetActive(bOn);
										}
									}
								}
							}
						}
					}
				}
#ifndef _FINAL
				else
				{
					g_pLTServer->CPrint("%s : Incorrect format found in \"%s\".",
						g_pLTServer->GetObjectName(m_hObject), szMsg);
					g_pLTServer->CPrint("%s : Format should be \"(volume_name {on,off}; volume_name {on,off};...\".",
						g_pLTServer->GetObjectName(m_hObject));
				}
#endif
			}
		}
		break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}


// Methods

CAIVolumeMgr::CAIVolumeMgr()
: m_bInitialized(false),
  m_bShowingVolumes(false)
{
	g_pAIVolumeMgr = this;
}

CAIVolumeMgr::~CAIVolumeMgr()
{
	g_pAIVolumeMgr = LTNULL;
	Term();
}

void CAIVolumeMgr::Term()
{
	m_bInitialized = LTFALSE;
	if( !m_aVolumes.empty() )   m_aVolumes.clear();
	if( !m_aVolumeIDs.empty() ) m_aVolumeIDs.clear();
#ifdef USE_AIPOLY
	if( !m_aPolies.empty() ) m_aPolies.clear();
#endif

#ifndef _FINAL
	ClearTheVolumes();
#endif
	m_bShowingVolumes = false;
}


void CAIVolumeMgr::Init()
{
#ifdef AIVOLUME_LOAD_TIMING
	LTCounter volume_load_counter;
	g_pServerDE->StartCounter(&volume_load_counter);
#endif

	Term();

	_ASSERT( m_aVolumes.empty() );
	_ASSERT( m_aVolumeIDs.empty() );

	if( !m_aVolumes.empty() )   m_aVolumes.clear();
	if( !m_aVolumeIDs.empty() ) m_aVolumeIDs.clear();

#ifdef USE_AIPOLY
	if( !m_aPolies.empty() ) m_aPolies.clear();
#endif

	// Now we put the Volumes into our array and remove the objects.
	while( !m_aVolumeObjects.empty() )
	{
		AIVolume * pCurVolume = m_aVolumeObjects.back();

		if( pCurVolume )
		{
			// Setup the volume (excluding wall walk and ladder volumes).
			if( !pCurVolume->IsWallWalkOnly() && !pCurVolume->IsLadder() )
			{
				m_aVolumes.push_back( CAIVolume(m_aVolumeIDs.size(),*pCurVolume) );
				m_aVolumeIDs.push_back( &m_aVolumes.back() );
			}
#ifndef _FINAL
			else
			{
				if( pCurVolume->IsWallWalkOnly() )
				{
					g_pLTServer->CPrint("AIVolume \"%s\" is a wall walk only volume.  It will be ignored.",
						g_pLTServer->GetObjectName(pCurVolume->m_hObject) );
				}
				else if( pCurVolume->IsLadder() )
				{
					g_pLTServer->CPrint("AIVolume \"%s\" is a ladder volume.  It will be ignored.",
						g_pLTServer->GetObjectName(pCurVolume->m_hObject) );
				}
				else
				{
					g_pLTServer->CPrint("AIVolume \"%s\" is bad for some reason.  It will be ignored.",
						g_pLTServer->GetObjectName(pCurVolume->m_hObject) );
				}
			}
#endif
			g_pLTServer->RemoveObject(pCurVolume->m_hObject);
		}

		m_aVolumeObjects.pop_back();
	}

	// Build the neighboring connections

#ifdef AIVOLUME_LOAD_TIMING
	LTCounter neighbor_finding_counter;
	g_pServerDE->StartCounter(&neighbor_finding_counter);
#endif

	uint32 cTotalNeighbors = 0;
	{for ( VolumeList::iterator iVolume = m_aVolumes.begin(); 
	       iVolume != m_aVolumes.end(); ++iVolume )
	{
		cTotalNeighbors += iVolume->InitNeighbors();
	}}


	// Put the nodes into the volumes.
	{for ( CAINodeMgr::NodeList::iterator iter = g_pAINodeMgr->BeginNodes();
	       iter != g_pAINodeMgr->EndNodes(); ++iter )
	{
		CAINode * const pNode = *iter;
		_ASSERT( pNode );
		if( pNode )
		{
			// Connect to a node if has connect-to-volume true
			// and if it or any of its corners are inside the volume.
			if(    pNode->ConnectToVolume() )
			{
				const LTVector & vDims = pNode->GetDims();

				// Try to find a containgin volume.
				CAIVolume * pContainingVolume = FindContainingVolume(pNode->GetPos());

				// If we didn't find one at the node's center, keep trying at each corner.
				if( !pContainingVolume )
				{
					pContainingVolume = FindContainingVolume( pNode->GetPos() + LTVector(vDims.x,vDims.y,vDims.z) );
				}

				if( !pContainingVolume )
				{
					pContainingVolume = FindContainingVolume( pNode->GetPos() + LTVector(vDims.x,vDims.y,-vDims.z) );
				}

				if( !pContainingVolume )
				{
					pContainingVolume = FindContainingVolume( pNode->GetPos() + LTVector(vDims.x,-vDims.y,vDims.z) );
				}

				if( !pContainingVolume )
				{
					pContainingVolume = FindContainingVolume( pNode->GetPos() + LTVector(vDims.x,-vDims.y,-vDims.z) );
				}

				if( !pContainingVolume )
				{
					pContainingVolume = FindContainingVolume( pNode->GetPos() + LTVector(-vDims.x,vDims.y,vDims.z) );
				}

				if( !pContainingVolume )
				{
					pContainingVolume = FindContainingVolume( pNode->GetPos() + LTVector(-vDims.x,vDims.y,-vDims.z) );
				}

				if( !pContainingVolume )
				{
					pContainingVolume = FindContainingVolume( pNode->GetPos() + LTVector(-vDims.x,-vDims.y,vDims.z) );
				}

				if( !pContainingVolume )
				{
					pContainingVolume = FindContainingVolume( pNode->GetPos() + LTVector(-vDims.x,-vDims.y,-vDims.z) );
				}

				// If we have a containing volume, add this node to it's neighbor list.
				if( pContainingVolume )
				{
					pNode->SetContainingVolume(*pContainingVolume);

					if( pContainingVolume->EndNeighborNodes() == std::find(pContainingVolume->BeginNeighborNodes(), pContainingVolume->EndNeighborNodes(),pNode->GetID()) )
					{
						pContainingVolume->AddNeighborNode( pNode->GetID() );
					}
				}
			}
		}
	}}		

	// Now add any other volume nodes (like the volume's center point), 
	// and connect all the volume's current neighbor nodes together as neighbors of each other.
	{for ( VolumeList::iterator iVolume = m_aVolumes.begin(); 
	       iVolume != m_aVolumes.end(); ++iVolume )
	{
		g_pAINodeMgr->AddVolumeNodes(&(*iVolume));
	}}

	// Now that we have volumes, we can update the wall walk nodes to find
	// their drop destionation.
	{for ( CAINodeMgr::NodeList::iterator iter = g_pAINodeMgr->BeginNodes();
	       iter != g_pAINodeMgr->EndNodes(); ++iter )
	{
		if( CAINodeWallWalk * pWWNode = dynamic_cast<CAINodeWallWalk*>( *iter ) )
		{
			pWWNode->FindDropVolume();
		}
	}}

#ifdef USE_AIPOLY
	// These guys take too long to load. The neighbor checking is horrendous!

  // Build poly list
#ifdef AIPOLY_LOAD_TIMING
	LTCounter poly_counter;
	g_pServerDE->StartCounter(&poly_counter);
#endif

#ifdef AIPOLY_DEBUG
	uint32 nNumPolyNeighbors = 0;
	uint32 nNumValidPolies = 0;
	double fSumRadius = 0;
#endif

	const uint32 g_nMaxVertices = 32;
	HPOLY hPoly = LTNULL;
	LTVector vertex_buffer[g_nMaxVertices];

	while( LT_FINISHED != g_pLTServer->GetNextPoly(&hPoly) )
	{

		HOBJECT hPolyObject;
		g_pLTServer->GetHPolyObject(hPoly,hPolyObject);

		if( LT_YES == g_pLTServer->Physics()->IsWorldObject(hPolyObject) )
		{
			LTPlane * pPlane = LTNULL;
			uint32 num_poly_vertices = 0;

			AIPoly & added_poly = m_aPolies[hPoly];
			added_poly.vertexList.assign(g_nMaxVerticesInPoly, LTVector(0,0,0) );
			if( LT_OK == g_pLTServer->Common()->GetPolyInfo(hPoly, &pPlane, &added_poly.vertexList[0], g_nMaxVerticesInPoly, &num_poly_vertices) )
			{

				if( num_poly_vertices > g_nMaxVerticesInPoly )
				{
#ifndef _FINAL
					g_pLTServer->CPrint("Poly at (%.2f,%.2f,%.2f) has %d vertices, maximum is %d vertices.",
						vertex_buffer[0].x, vertex_buffer[0].y, vertex_buffer[0].z,
						num_poly_vertices,
						g_nMaxVerticesInPoly );
#endif
					num_poly_vertices = g_nMaxVerticesInPoly;
				}

				added_poly.plane = *pPlane;
				added_poly.finish(num_poly_vertices);

#ifdef AIPOLY_DEBUG
				++nNumValidPolies;

				fSumRadius += added_poly.radius;
#endif

				// Check for neighbors
#ifdef AIPOLY_FIND_NEIGHBORS
				for( PolyMap::iterator iter = m_aPolies.begin(); 
					 iter != m_aPolies.end(); ++iter )
				{
					AIPoly & current_poly = iter->second;

					if(    !current_poly.vertexList.empty() 
						&& &added_poly != &current_poly )
					{
						if( IsNeighborPoly(added_poly,current_poly) )
						{
							current_poly.neighborList.push_back(&added_poly);
							added_poly.neighborList.push_back(&current_poly);
#ifdef AIPOLY_DEBUG
							++nNumPolyNeighbors;
#endif
						}
					}
				}
#endif	
			} //if( LT_OK == g_pLTServer->Common()->GetPolyInfo(hPoly, &pPlane, vertex_buffer, g_nMaxVertices, &num_poly_vertices) )
			else
			{
	//			ASSERT(0);
			}

		} //if( LT_YES == g_pLTServer->Physics()->IsWorldObject(hPolyObject) )

	} //while( LT_FINISHED != g_pLTServer->GetNextPoly(&hPoly) )

#ifdef AIPOLY_LOAD_TIMING
	const uint32 poly_ticks = g_pServerDE->EndCounter(&poly_counter);
	g_pLTServer->CPrint("AI Poly loading took %f seconds.",
		double(poly_ticks)*10e-6 );
#endif

#ifdef AIPOLY_DEBUG
	g_pServerDE->CPrint( "Added %d polies, %d valid, having %d neighbors.", m_aPolies.size(), nNumValidPolies, nNumPolyNeighbors );
	g_pServerDE->CPrint( "Average radius is %f.", fSumRadius/nNumValidPolies );
#endif

#endif //USE_AIPOLY

	// All done

#ifdef AIVOLUME_LOAD_TIMING
	const uint32 neighbor_ticks = g_pServerDE->EndCounter(&neighbor_finding_counter);
	g_pLTServer->CPrint("AI Volume neighbor finding took %f seconds.",
		double(neighbor_ticks)*10e-6 );

	const uint32 volume_ticks = g_pServerDE->EndCounter(&volume_load_counter);
	g_pLTServer->CPrint("AI Volume loading took %f seconds.",
		double(volume_ticks)*10e-6 );

	g_pServerDE->CPrint("Added %d volumes, %d connections", m_aVolumes.size(), cTotalNeighbors);

#endif

#ifdef AIVOLUMEMGR_DEBUG
	g_pServerDE->CPrint("Added %d volumes, %d connections", m_aVolumes.size(), cTotalNeighbors);
#endif

	m_bInitialized = LTTRUE;
}



void CAIVolumeMgr::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	*hRead >> m_bInitialized;

	uint32 num_volumes;
	*hRead >> num_volumes;
	for ( uint32 iVolume = 0 ; iVolume < num_volumes; iVolume++ )
	{
		m_aVolumes.push_back(CAIVolume(hRead));
		m_aVolumeIDs.push_back( &m_aVolumes.back() );
	}

	// After all the volumes have been loaded, we can 
	// restore their neighbor information.
	for( VolumeList::iterator iter = m_aVolumes.begin();
		  iter != m_aVolumes.end(); ++iter )
	{
		iter->RestoreNeighbors();
	}
}

void CAIVolumeMgr::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	*hWrite << m_bInitialized;
	
	uint32 size = m_aVolumes.size();
	*hWrite << size;
	for ( VolumeList::iterator iVolume = m_aVolumes.begin(); 
	      iVolume != m_aVolumes.end(); ++iVolume )
	{
		iVolume->Save(hWrite);
	}
}

CAIVolume* CAIVolumeMgr::FindContainingVolume(const LTVector& vPos)
{
	if(m_aVolumes.empty()) return LTNULL;

	for ( VolumeList::iterator iVolume = m_aVolumes.begin(); 
	      iVolume != m_aVolumes.end(); ++iVolume )
	{
		if ( iVolume->Contains(vPos) )
		{
			return &(*iVolume);
		}
	}

	return LTNULL;
}

CAIVolume* CAIVolumeMgr::FindContainingVolume(const LTVector& vPos, const LTVector & vDims)
{
	if(m_aVolumes.empty()) return LTNULL;

	for ( VolumeList::iterator iVolume = m_aVolumes.begin(); 
	      iVolume != m_aVolumes.end(); ++iVolume )
	{
		if ( iVolume->ContainsWithNeighbor(vPos,vDims) )
		{
			return &(*iVolume);
		}
	}

	return LTNULL;
}

CAIVolume* CAIVolumeMgr::FindFallToVolume(const LTVector& vPos)
{
	if(m_aVolumes.empty()) return LTNULL;

	CAIVolume * pFoundVolume = LTNULL;
	LTFLOAT    fMinVertDist = FLT_MAX;

	for ( VolumeList::iterator iVolume = m_aVolumes.begin(); 
	      iVolume != m_aVolumes.end(); ++iVolume )
	{
		if( iVolume->GetCenter().y < vPos.y )
		{
			LTVector vRelPos = LTVector(vPos.x, iVolume->GetCenter().y, vPos.z);

			if( iVolume->Contains(vRelPos) )
			{
				const LTFLOAT fCurVertDist = vPos.y - vRelPos.y;
				if( !pFoundVolume || fMinVertDist > fCurVertDist )
				{
					pFoundVolume = &(*iVolume);
					fMinVertDist = fCurVertDist;
				}
			}
		}
	}

	return pFoundVolume;
}

#ifndef _FINAL

void DrawRecursiveVolumes(const CAIVolume & volume, DebugLineSystem * pLineSystem, int recursion_level = 0)
{
	const int nMaxVolumesToDraw = 5;

	const LTVector color = Color::Yellow;
	const LTFLOAT  alpha = 0.7f;

	const LTVector neighbor_color = Color::Green;
	const LTFLOAT  neighbor_alpha = 0.7f;

	if( recursion_level > nMaxVolumesToDraw )
		return;

	if( !pLineSystem )
		return;

	const LTVector & vVolumeCenter = volume.GetCenter();

	CAINode * pVolumeNode = g_pAINodeMgr ? g_pAINodeMgr->GetNode(volume.GetNodeID()) : LTNULL;

	if( pVolumeNode && !pVolumeNode->IsActive() )
		return;

	for( CAIVolume::FaceList::const_iterator face_iter = volume.BeginFaces();
		 face_iter != volume.EndFaces(); ++face_iter )
	{
		for( uint32 i = 0; i < face_iter->vertexList.size(); ++i )
		{
			for( uint32 j = i+1; j < face_iter->vertexList.size(); ++j )
			{
				*pLineSystem << LineSystem::Line( face_iter->vertexList[i], face_iter->vertexList[j], color, alpha);
			}
		}
	}

	// Draw the neighbor connections
	for( CAIVolume::NeighborNodeList::const_iterator neighbor_node_iter = volume.BeginNeighborNodes();
		 neighbor_node_iter != volume.EndNeighborNodes(); ++neighbor_node_iter )
	{
		const CAIVolumeNeighborNode * pNode = dynamic_cast<const CAIVolumeNeighborNode *>( g_pAINodeMgr->GetNode(*neighbor_node_iter) );
		if( pNode && pNode->IsActive() )
		{
			const LTVector endpoint_half_line = (pNode->GetConnectionEndpoint2() - pNode->GetConnectionEndpoint1())/2.0f;

			*pLineSystem
				<< LineSystem::Line( pNode->GetPos() - endpoint_half_line, pNode->GetPos() + endpoint_half_line, neighbor_color, neighbor_alpha )
				<< LineSystem::Arrow( pNode->GetPos(), pNode->GetPos() + pNode->GetConnectionPlane().Normal()*10.0f, neighbor_color, neighbor_alpha );

		}
	}


	for(  CAIVolume::NeighborList::const_iterator neighbor_volume_iter = volume.BeginNeighbors();
	      neighbor_volume_iter != volume.EndNeighbors(); ++neighbor_volume_iter )
	{
		DrawRecursiveVolumes( *(*neighbor_volume_iter), pLineSystem, recursion_level + 1 );
	}

	return;
}

#endif

#ifndef _FINAL

class SortVolumesAround
{
	const LTVector & vAroundPos;
public :

	SortVolumesAround( const LTVector & vPos )
		: vAroundPos(vPos) {}

	bool operator()( const CAIVolume * pVolA, const CAIVolume * pVolB)
	{
		return (pVolA->GetCenter() - vAroundPos).MagSqr() < (pVolB->GetCenter() - vAroundPos).MagSqr();
	}
};

#endif

bool CAIVolumeMgr::FindPath(const LTVector &vGoalPos, const CAIVolume * pDest, const CAIVolume * pSrc, LTFLOAT fMaxSearchCost /* = -1.0f */)
{
	static uint32 s_nVolumePathingIteration = 0;
	static std::vector<CAIVolume*> path_heap;

	_ASSERT( pSrc && pDest );
	if( !pDest || !pSrc ) 
		return false;

	if( fMaxSearchCost < 0.0f )
	{
		fMaxSearchCost = g_fMaxSearchCost;
	}

	// This is a hack to get non-const versions of the source and destination nodes.
	CAIVolume * pSrcNode = GetVolumePtr(pSrc->GetID());
	CAIVolume * pDestNode = GetVolumePtr(pDest->GetID());

	++s_nVolumePathingIteration;

	pSrcNode->ResetPathing(s_nVolumePathingIteration);

	pDestNode->ResetPathing(s_nVolumePathingIteration);
	pDestNode->SetIsGoal(true);

	if( pDest == pSrc )
		return true;

	CAIVolume * const pEndNode = AStar::FindPath<CAIVolume>(*pSrcNode,VolumeCost(),VolumeHeuristic(vGoalPos),path_heap,fMaxSearchCost);

	return pEndNode && pEndNode->IsGoal();
}

#ifndef _FINAL

void CAIVolumeMgr::DrawTheVolumes()
{

	std::vector<CAIVolume*> sorted_volumes;


	{for(VolumeList::iterator iter = m_aVolumes.begin();
	    iter != m_aVolumes.end(); ++iter)
	{
		sorted_volumes.push_back( &(*iter) );
	}}


	CCharacterMgr::PlayerIterator player_iter = g_pCharacterMgr->BeginPlayers();
	if( player_iter != g_pCharacterMgr->EndPlayers() )
	{
		CPlayerObj * pPlayerObj = *player_iter;
		const LTVector & vPlayerPos = pPlayerObj->GetPosition();

		std::sort( sorted_volumes.begin(), sorted_volumes.end(), SortVolumesAround( vPlayerPos ) );
	}


	const LTVector color = Color::Yellow;
	const LTFLOAT  alpha = 0.7f;

	const LTVector neighbor_color = Color::Green;
	const LTFLOAT  neighbor_alpha = 0.7f;

	{for(std::vector<CAIVolume*>::iterator iter = sorted_volumes.begin();
	    iter != sorted_volumes.end() && (iter - sorted_volumes.begin()) < 100; ++iter)
	{
		const LTVector & vVolumeCenter = (*iter)->GetCenter();

		LineSystem::GetSystem((*iter),"AIVolumes") << LineSystem::Clear();

		CAINode * pVolumeNode = g_pAINodeMgr ? g_pAINodeMgr->GetNode((*iter)->GetNodeID()) : LTNULL;

		if( pVolumeNode && !pVolumeNode->IsActive() )
			continue;

		for( CAIVolume::FaceList::const_iterator face_iter = (*iter)->BeginFaces();
			 face_iter != (*iter)->EndFaces(); ++face_iter )
		{
			for( uint32 i = 0; i < face_iter->vertexList.size(); ++i )
			{
				for( uint32 j = i+1; j < face_iter->vertexList.size(); ++j )
				{
					LineSystem::GetSystem((*iter),"AIVolumes") << LineSystem::Line( face_iter->vertexList[i], face_iter->vertexList[j], color, alpha);
				}
			}
		}

		// Draw the neighbor connections
		for( CAIVolume::NeighborNodeList::const_iterator neighbor_node_iter = (*iter)->BeginNeighborNodes();
			 neighbor_node_iter != (*iter)->EndNeighborNodes(); ++neighbor_node_iter )
		{
			const CAIVolumeNeighborNode * pNode = dynamic_cast<const CAIVolumeNeighborNode *>( g_pAINodeMgr->GetNode(*neighbor_node_iter) );
			if( pNode && pNode->IsActive() )
			{
				const LTVector endpoint_half_line = (pNode->GetConnectionEndpoint2() - pNode->GetConnectionEndpoint1())/2.0f;
				LineSystem::GetSystem((*iter),"AIVolumes") 
					<< LineSystem::Line( pNode->GetPos() - endpoint_half_line, pNode->GetPos() + endpoint_half_line, neighbor_color, neighbor_alpha )
					<< LineSystem::Arrow( pNode->GetPos(), pNode->GetPos() + pNode->GetConnectionPlane().Normal()*10.0f, neighbor_color, neighbor_alpha );
			}
		}

#ifdef AIVOLUMEMGR_DEBUG
		g_pLTServer->CPrint("Volume has %d neighbors.",
			iter->GetNumNeighborVolumes() );
#endif
	}}

	
}

#endif

#ifndef _FINAL

void CAIVolumeMgr::ClearTheVolumes()
{
	if(m_aVolumes.empty()) return;

//	LineSystem::GetSystem("AIVolumes") << LineSystem::Clear();

	if(m_aVolumes.empty()) return;

	for(VolumeList::iterator iter = m_aVolumes.begin();
	    iter != m_aVolumes.end(); ++iter)
	{
		LineSystem::GetSystem(&(*iter),"AIVolumes") << LineSystem::Clear();
	}

}

#endif

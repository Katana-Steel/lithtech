// ----------------------------------------------------------------------- //
//
// MODULE  : AIVolume.cpp
//
// PURPOSE : Creates volumetric description of level
//
// CREATED : 12.14.1999
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIVolume.h"

#include <limits.h>
#include <algorithm>

#include "ContainerCodes.h"
#include "Door.h"
#include "AIVolumeMgr.h"
#include "AINodeMgr.h"
#include "AINode.h"
#include "AIUtils.h"
#include "DebugLineSystem.h"
#include "CharacterMovementDefs.h" // for operator>>(ILTMessage&,LTPlane&).

const uint32 g_nMaxVerticesInFace = 32;


const LTFLOAT g_fContainsPointEpsilon = 0.1f;

BEGIN_CLASS(AIVolume)
	ADD_SOLID_FLAG(LTFALSE,PF_HIDDEN)
	ADD_BOOLPROP_FLAG_HELP(DoorVolume, LTFALSE, 0, "Makes the volume a door volume.")
	ADD_BOOLPROP_FLAG_HELP(StairVolume, LTFALSE, 0, "Makes AIs use their falling-down-stairs death animation when in this volume.")
	ADD_BOOLPROP_FLAG_HELP(WallWalkOnly, LTFALSE, PF_HIDDEN, "This is field is ignored.")
	ADD_BOOLPROP_FLAG_HELP(LadderVolume, LTFALSE, PF_HIDDEN, "Tells the AI to look up, or down to get to desired point.")
	ADD_BOOLPROP_FLAG_HELP(StartActive, LTTRUE, 0, "If false, is equivalent of turning volume off with AIVolumeMgr.")
END_CLASS_DEFAULT(AIVolume, GameBase, NULL, NULL)

#define NEIGHBOR_LINK(number) \
	ADD_OBJECTPROP_FLAG_HELP(Neighbor##number, "", 0, "AI can go to this point.")

ILTMessage & operator<<(ILTMessage & out, const LTBBox & box)
{
#ifdef __LINUX
	out << &box.GetCenter();
	out << &box.GetDims();
#else
	out << box.GetCenter();
	out << box.GetDims();
#endif
	return out;
}

ILTMessage & operator>>(ILTMessage & in, LTBBox & box)
{
	LTVector center;
	LTVector dims;

	in >> center;
	in >> dims;

	box = LTBBox(center,dims);

	return in;
}

ILTMessage & operator<<(ILTMessage & out, /*const*/ CAIVolume::DoorInfo & door)
{
	out << door.isAITriggerable;
	out << door.handle;
	return out;
}

ILTMessage & operator>>(ILTMessage & in, CAIVolume::DoorInfo & door)
{
	in >> door.isAITriggerable;
	in >> door.handle;

	return in;
}

ILTMessage & operator<<(ILTMessage & out, /*const*/ Face & face)
{
	out << face.plane;
	out.WriteDWord(face.vertexList.size());
	for( uint32 i = 0; i < face.vertexList.size(); ++i )
	{
		out << face.vertexList[i];
	}

	return out;
}

ILTMessage & operator>>(ILTMessage & in, Face & face)
{
	in >> face.plane;

	const int num_vertices = in.ReadDWord();
	face.vertexList.clear();
	face.vertexList.assign(num_vertices, LTVector( ));
	for( int i = 0; i < num_vertices; ++i )
	{
		in >> face.vertexList[i];
	}

	return in;
}

ILTMessage & operator<<(ILTMessage & out, /*const*/ CAINeighborInfo & x)
{
	x.Save(&out);
	return out;
}

ILTMessage & operator>>(ILTMessage & in, CAINeighborInfo & x)
{
	x.Load(&in);
	return in;
}


static bool lesser_vector(const LTVector & v1, const LTVector & v2)
{

	return v1.x < v2.x 
		|| (v1.x == v2.x && v1.y < v2.y )
		|| (v1.x == v2.x && v1.y == v2.y && v1.z < v2.z);
}

// Statics

const LTFLOAT c_fNeighborThreshhold = 0.5f;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIVolume::AIVolume()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

AIVolume::AIVolume() 
	: GameBase(OT_WORLDMODEL),
	  m_bIsDoor(false),
	  m_bIsStair(false),
	  m_bIsWallWalkOnly(false),
	  m_bIsLadder(false),
	  m_bStartActive(true)
{
}


void AIVolume::AddNeighborLink(const LTSmartLink & new_neighbor)
{ 
	if( new_neighbor )
	{
		NeighborLinkList::iterator iter = std::find(m_NeighborLinkList.begin(), m_NeighborLinkList.end(), new_neighbor);
		if( iter == m_NeighborLinkList.end() )
		{
			m_NeighborLinkList.push_back(new_neighbor);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIVolume::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 AIVolume::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			g_pLTServer->SetNextUpdate(m_hObject, 0.01f);

			uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp(static_cast<ObjectCreateStruct*>(pData) );
			}

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

			pStruct->m_Flags |= FLAG_VISIBLE;
			pStruct->m_ObjectType = OT_WORLDMODEL;
			SAFE_STRCPY(pStruct->m_Filename, pStruct->m_Name);
			pStruct->m_SkinName[0] = '\0';
			//pStruct->m_ContainerCode = CC_VOLUME;

			return dwRet;
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

void AIVolume::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	if (!g_pLTServer || !pData)
		return;

	// Read all our flags.
	if( g_pLTServer->GetPropGeneric( "DoorVolume", &genProp ) == DE_OK )
	{
		m_bIsDoor = (genProp.m_Bool == LTTRUE);
	}

	if( g_pLTServer->GetPropGeneric( "StairVolume", &genProp ) == DE_OK )
	{
		m_bIsStair = (genProp.m_Bool == LTTRUE);
	}
		
	if( g_pLTServer->GetPropGeneric( "WallWalkOnly", &genProp ) == DE_OK )
	{
		m_bIsWallWalkOnly = (genProp.m_Bool == LTTRUE);
	}

	if( g_pLTServer->GetPropGeneric( "LadderVolume", &genProp ) == DE_OK )
	{
		m_bIsLadder = (genProp.m_Bool == LTTRUE);
	}

	if( g_pLTServer->GetPropGeneric( "StartActive", &genProp ) == DE_OK )
	{
		m_bStartActive = (genProp.m_Bool == LTTRUE);
	}

	// Read our neighbor links (only wall walk nodes use these, for now).
	char prop_name[15];
	for(int i = 0; i < 5; ++i )
	{
		sprintf(prop_name,"Neighbor%d",i+1);
		if( g_pServerDE->GetPropGeneric( prop_name, &genProp ) == LT_OK )	
		{
			if ( genProp.m_String[0] )
				m_NeighborNameList.push_back(genProp.m_String);
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIVolume::PostStartWorld()
//
//	PURPOSE:	Called after all the objects and geometry has been loaded.
//
// ----------------------------------------------------------------------- //
void AIVolume::PostStartWorld(uint8 nLoadGameFlags)
{
	GameBase::PostStartWorld(nLoadGameFlags);

	if( nLoadGameFlags != LOAD_RESTORE_GAME )
	{
		// Find and link to neighbors.
		for( NeighborNameList::iterator iter = m_NeighborNameList.begin();
			 iter != m_NeighborNameList.end(); ++iter )
		{
			ObjArray<HOBJECT,1> objArray;
			if( LT_OK == g_pServerDE->FindNamedObjects( const_cast<char*>( iter->c_str() ),
														objArray ) 
				&& objArray.NumObjects() > 0 )
			{
				HOBJECT hNeighbor = objArray.GetObject(0);
				ASSERT(hNeighbor);

				// Don't allow circular loops.
				if( hNeighbor != m_hObject )
				{
					// Be sure the other node hasn't already attached to us.
					if( m_NeighborLinkList.end() == std::find( m_NeighborLinkList.begin(), m_NeighborLinkList.end(), hNeighbor ) )
					{
						m_NeighborLinkList.push_back( hNeighbor );
					}

					AIVolume * pNeighbor = dynamic_cast<AIVolume*>( g_pLTServer->HandleToObject( hNeighbor ) );
					if( pNeighbor )
					{
						// Be sure the other node hasn't already attached to us.
						if( pNeighbor->m_NeighborLinkList.end() == std::find( pNeighbor->m_NeighborLinkList.begin(), pNeighbor->m_NeighborLinkList.end(), m_hObject) )
						{
							pNeighbor->m_NeighborLinkList.push_back( m_hObject );
						}
					}
				}
				else
				{
#ifndef _FINAL
					LTVector vPos;
					g_pLTServer->GetObjectPos(m_hObject,&vPos);
					g_pLTServer->CPrint("Node %s at (%.2f,%.2f,%.2f) points to itself!",
						g_pLTServer->GetObjectName(m_hObject),
						vPos.x, vPos.y, vPos.z );
#endif
				}
			}
			else
			{
#ifndef _FINAL
					LTVector vPos;
					g_pLTServer->GetObjectPos(m_hObject,&vPos);
					g_pLTServer->CPrint("Node %s at (%.2f,%.2f,%.2f) refers to an unknown object named \"%s\"!",
						g_pLTServer->GetObjectName(m_hObject),
						vPos.x, vPos.y, vPos.z,
						iter->c_str() );
#endif
			}
		}

		ASSERT( g_pAIVolumeMgr );
		if( g_pAIVolumeMgr )
			g_pAIVolumeMgr->AddVolumeObject(this);
	}
}

static void FindFaceMinMax(const Face & face, LTVector * pvMin, LTVector * pvMax )
{
	if( pvMin ) *pvMin = LTVector(FLT_MAX,FLT_MAX,FLT_MAX);
	if( pvMax ) *pvMax = LTVector(-FLT_MAX,-FLT_MAX,-FLT_MAX);

	for(uint32 i = 0; i < face.vertexList.size(); ++i )
	{
		if( pvMin )
		{
			if( face.vertexList[i].x < pvMin->x )
			{
				pvMin->x = face.vertexList[i].x;
			}
			if( face.vertexList[i].y < pvMin->y )
			{
				pvMin->y = face.vertexList[i].y;
			}
			if( face.vertexList[i].z < pvMin->z )
			{
				pvMin->z = face.vertexList[i].z;
			}

		}
		if( pvMax )
		{
			if( face.vertexList[i].x > pvMax->x )
			{
				pvMax->x = face.vertexList[i].x;
			}
			if( face.vertexList[i].y > pvMax->y )
			{
				pvMax->y = face.vertexList[i].y;
			}
			if( face.vertexList[i].z > pvMax->z )
			{
				pvMax->z = face.vertexList[i].z;
			}
		}		
	}
}

CAINeighborInfo::CAINeighborInfo( HMESSAGEREAD hRead ) 
	: m_pVolume1(NULL),
	  m_pVolume2(NULL),
	  m_vConnectionPos(0.0f,0.0f,0.0f),
	  m_vConnectionEndpoint1(0.0f,0.0f,0.0f),
	  m_vConnectionEndpoint2(0.0f,0.0f,0.0f),
	  m_ConnectionPlane(0.0f,0.0f,0.0f,0.0f)	
{
	Load(hRead);
}

void CAINeighborInfo::RestoreVolumePtr()
{
	m_pVolume1 = g_pAIVolumeMgr->FindContainingVolume( m_vConnectionPos + m_ConnectionPlane.Normal() );
	m_pVolume2 = g_pAIVolumeMgr->FindContainingVolume( m_vConnectionPos - m_ConnectionPlane.Normal() );

	ASSERT( m_pVolume1 && m_pVolume2 );
}

void CAINeighborInfo::Load(HMESSAGEREAD hRead)
{
	// m_vVolumePtr should be set with CAINeighborInfo::RestoreVolumePtr().

	*hRead >> m_vConnectionPos;
	*hRead >> m_vConnectionEndpoint1;
	*hRead >> m_vConnectionEndpoint2;
	*hRead >> m_ConnectionPlane;
}

void CAINeighborInfo::Save(HMESSAGEWRITE hWrite)
{
	*hWrite << m_vConnectionPos;
	*hWrite << m_vConnectionEndpoint1;
	*hWrite << m_vConnectionEndpoint2;
	*hWrite << m_ConnectionPlane;
}




CAIVolume::DoorInfo::DoorInfo(const LTSmartLink & new_handle)
	: handle(new_handle)
{
	if( handle )
	{
		Door * pDoor = dynamic_cast<Door*>( g_pLTServer->HandleToObject(handle) );
		if( pDoor )
		{
			isAITriggerable = (LTTRUE == pDoor->IsAITriggerable());
		}
	}
}



CAIVolume::CAIVolume()
	:	m_dwID(CAIVolume::kInvalidID), 
		m_strName("<null>"),
		m_dwNodeID(CAINode::kInvalidID),
		m_bStartActive(LTTRUE),
		m_vCenter(0,0,0),
		m_vRight(0,0,0),
		m_vUp(0,0,0),
		m_vForward(0,0,0),
	
		m_bIsWallWalkOnly(LTFALSE),
		m_bIsLadder(LTFALSE),
		m_bIsStair(LTFALSE),

	    m_nLastPathingIteration(0),
	    m_bResetNeighbors(false),

	    m_bIsGoal(false),

	    m_fCost(0.0f),
	    m_fHeuristic(0.0f),

	    m_bInOpen(false),
	    m_bInClosed(false),
	    m_pPrevNode(LTNULL)
{
}

CAIVolume::CAIVolume(uint32 iVolume, AIVolume& vol)
	:	m_dwID(iVolume),
	    m_strName( g_pLTServer->GetObjectName(vol.m_hObject) ),
		m_dwNodeID(CAINode::kInvalidID),
		m_bStartActive(vol.m_bStartActive),
	    m_vCenter(0,0,0),
		m_vRight(0,0,0),
		m_vUp(0,0,0),
		m_vForward(0,0,0),
	
		m_bIsWallWalkOnly(LTFALSE),
		m_bIsLadder(LTFALSE),
		m_bIsStair(LTFALSE),

	    m_nLastPathingIteration(0),
	    m_bResetNeighbors(false),

	    m_bIsGoal(false),

	    m_fCost(0.0f),
	    m_fHeuristic(0.0f),

	    m_bInOpen(false),
	    m_bInClosed(false),
	    m_pPrevNode(LTNULL)
{
	Init(vol);
}

CAIVolume::CAIVolume(HMESSAGEREAD hRead)
	:	m_dwID(CAIVolume::kInvalidID),
		m_dwNodeID(CAINode::kInvalidID),
		m_bStartActive(LTTRUE),
		m_vCenter(0,0,0),
		m_vRight(0,0,0),
		m_vUp(0,0,0),
		m_vForward(0,0,0),

		m_bIsWallWalkOnly(LTFALSE),
		m_bIsLadder(LTFALSE),
		m_bIsStair(LTFALSE),

	    m_nLastPathingIteration(0),
	    m_bResetNeighbors(false),

	    m_bIsGoal(false),

	    m_fCost(0.0f),
	    m_fHeuristic(0.0f),

	    m_bInOpen(false),
	    m_bInClosed(false),
	    m_pPrevNode(LTNULL)
{
	Load(hRead);
}

CAIVolume::~CAIVolume()
{
}

void CAIVolume::Load(HMESSAGEREAD hRead)
{
	*hRead >> m_dwID;
	*hRead >> m_strName;
	*hRead >> m_dwNodeID;

	*hRead >> m_bStartActive;

	*hRead >> m_vCenter;
	*hRead >> m_vRight;
	*hRead >> m_vUp;
	*hRead >> m_vForward;

	*hRead >> m_BoundingBox;
	
	uint32 num_neighbors = 0;
	*hRead >> num_neighbors;
	m_aNeighborNodes.clear();
	m_aNeighborNodes.reserve(num_neighbors);
	{for ( uint32 iNeighbor = 0 ; iNeighbor < num_neighbors ; iNeighbor++ )
	{
		m_aNeighborNodes.push_back( hRead->ReadDWord() );
	}}
	
	// m_aNeighbors should be restored with RestoreNieghbors(), called after 
	// all the volumes are loaded.
	num_neighbors = 0;
	*hRead >> num_neighbors;
	m_aNeighbors.clear();
	m_aNeighborIDs.clear();
	{for ( uint32 iNeighbor = 0 ; iNeighbor < num_neighbors ; iNeighbor++ )
	{
		m_aNeighborIDs.push_back( hRead->ReadDWord() );
	}}


	uint32 num_faces;
	*hRead >> num_faces;
	m_FaceList.clear();
	m_FaceList.reserve(num_faces);
	for ( uint32 iFace = 0; iFace < num_faces; ++iFace )
	{
		m_FaceList.push_back(Face());
		*hRead >> m_FaceList.back();
	}

	uint32 num_doors = 0;
	*hRead >> num_doors;
	m_ahDoors.clear();
	m_ahDoors.reserve(num_doors);
	DoorInfo temp;
	for ( uint32 iDoor = 0 ; iDoor < num_doors ; iDoor++ )
	{
		*hRead >> temp;
		m_ahDoors.push_back(temp);
	}

	*hRead >> m_bIsWallWalkOnly;
	*hRead >> m_bIsLadder;
	*hRead >> m_bIsStair;

	m_nLastPathingIteration = 0;
}

void CAIVolume::RestoreNeighbors()
{
	for( NeighborIDList::const_iterator iter = m_aNeighborIDs.begin();
	     iter != m_aNeighborIDs.end(); ++iter )
	{
		CAIVolume * const pNeighborVolume = g_pAIVolumeMgr->GetVolumePtr(*iter);
		if( pNeighborVolume )
			m_aNeighbors.push_back( pNeighborVolume );
	}

	m_aNeighborIDs.clear();
}


void CAIVolume::Save(HMESSAGEWRITE hWrite)
{
	*hWrite << m_dwID;
	*hWrite << m_strName;
	*hWrite << m_dwNodeID;

	*hWrite << m_bStartActive;

	*hWrite << m_vCenter;
	*hWrite << m_vRight;
	*hWrite << m_vUp;
	*hWrite << m_vForward;

	*hWrite << m_BoundingBox;

	uint32 size = m_aNeighborNodes.size();
	*hWrite << size;
	{for ( NeighborNodeList::iterator iter = m_aNeighborNodes.begin();
	      iter != m_aNeighborNodes.end(); ++iter )
	{
		*hWrite << (*iter);
	}}

	size = m_aNeighbors.size();
	*hWrite << size;
	{for ( NeighborList::iterator iter = m_aNeighbors.begin();
	      iter != m_aNeighbors.end(); ++iter )
	{
		hWrite->WriteDWord((*iter)->GetID());
	}}

	size = m_FaceList.size();
	*hWrite << size;
	{for ( FaceList::iterator iter = BeginFaces(); iter != EndFaces(); ++iter )
	{
		*hWrite << (*iter);
	}}

	size = m_ahDoors.size();
	*hWrite << size;
	{for (DoorList::iterator iter = m_ahDoors.begin();
	     iter != m_ahDoors.end(); ++iter )
	{
		*hWrite << (*iter);
	}}

	*hWrite << m_bIsWallWalkOnly;
	*hWrite << m_bIsLadder;
	*hWrite << m_bIsStair;
}

namespace /* unnamed */
{
	class SamePlaneAs
	{
		const LTPlane & test_plane;
	public:
		SamePlaneAs(const LTPlane & new_plane)
			: test_plane(new_plane) {}

		bool operator()(const Face & face) const
		{
			return face.plane.Normal() == test_plane.Normal()
				  && face.plane.Dist() == test_plane.Dist();
		}
	};

	Face ExtendedFace(const Face & orig_face, const Face & extend_face)
	{
		// result will be the returned face.  
		// Set up its non-vertex info.
		Face result;
		result.plane = orig_face.plane;

		// Copy all unique vertices from orig_face and extend_face into result.
		const LTVector * const begin_orig_vertices = &orig_face.vertexList[0];
		const LTVector * const end_orig_vertices = &orig_face.vertexList[0] + orig_face.vertexList.size();

		const LTVector * const begin_extend_vertices = &extend_face.vertexList[0];
		const LTVector * const end_extend_vertices = &extend_face.vertexList[0] + extend_face.vertexList.size();

		// Copy all original vertices that aren't shared by the extended face.
		for( const LTVector * orig_vertex = begin_orig_vertices;
		     orig_vertex < end_orig_vertices; ++orig_vertex )
		{
			if( end_extend_vertices == std::find(begin_extend_vertices,end_extend_vertices,*orig_vertex) )			
			{
				result.vertexList.push_back(*orig_vertex);
			}
		}

		// Copy all extend vertices that aren't shared by the original face.
		for( const LTVector * extend_vertex = begin_extend_vertices;
		     extend_vertex < end_extend_vertices; ++extend_vertex )
		{
			if( end_orig_vertices == std::find(begin_orig_vertices, end_orig_vertices, *extend_vertex) )
			{
				result.vertexList.push_back(*extend_vertex);
			}
		}

		return result;
	}

#ifndef _FINAL
	template<typename FaceIterator>
	bool ConvexFaceList(const FaceIterator & begin_faces, const FaceIterator & end_faces)
	{
		for( FaceIterator outer = begin_faces; outer != end_faces; ++outer)
		{
			for(uint32 i = 0; i < outer->vertexList.size(); ++i)
			{
				for( FaceIterator inner = outer+1; inner != end_faces; ++inner)
				{
					if( inner->plane.DistTo( outer->vertexList[i] ) > g_fContainsPointEpsilon )
					{
						LineSystem::GetSystem("ShowConcave") 
							<< LineSystem::Box( outer->vertexList[i], LTVector(3.0f,3.0f,3.0f), Color::Red );

						for( uint32 j = 0; j < inner->vertexList.size(); ++j )
							for( uint32 k = j+1; k < inner->vertexList.size(); ++k )
								LineSystem::GetSystem("ShowConcave") 
									<< LineSystem::Line( inner->vertexList[j], inner->vertexList[k], Color::Red);

						return false;
					}
				}
			}
		}

		return true;
	}

#endif
}

void CAIVolume::Init(AIVolume& vol)
{
	LTVector vVolumePos;
	LTVector vVolumeDims;

	g_pLTServer->GetObjectPos(vol.m_hObject, &vVolumePos);
	g_pLTServer->GetObjectDims(vol.m_hObject, &vVolumeDims);

	DRotation rRot;
	g_pLTServer->GetObjectRotation(vol.m_hObject, &rRot);
	g_pLTServer->Common()->GetRotationVectors(rRot, m_vUp, m_vRight, m_vForward);

	m_BoundingBox = LTBBox(vVolumePos,vVolumeDims);

	// Fill in the poly info.

	HPOLY hPoly = INVALID_HPOLY;
	LTVector vertex_buffer[g_nMaxVerticesInFace];

	while( g_pLTServer->GetNextWMPoly(&hPoly, vol.m_hObject) == LT_OK )
	{
		Face face;
		LTPlane * pPlane = LTNULL;
		uint32 num_poly_vertices = 0;
		g_pLTServer->Common()->GetPolyInfo(hPoly, &pPlane, vertex_buffer, g_nMaxVerticesInFace, &num_poly_vertices);
		
		if( num_poly_vertices > g_nMaxVerticesInFace )
		{
#ifndef _FINAL
			g_pLTServer->CPrint("Volume \"%s\" at (%.2f,%.2f,%.2f) has more than the maximum of %d vertices.",
				g_pLTServer->GetObjectName(vol.m_hObject),
				vVolumePos.x, vVolumePos.y, vVolumePos.z,
				g_nMaxVerticesInFace );
#endif
			num_poly_vertices = g_nMaxVerticesInFace;
		}

		face.vertexList.assign(vertex_buffer,vertex_buffer + num_poly_vertices);



		face.plane = *pPlane;

		FaceList::iterator find_face = std::find_if( m_FaceList.begin(), m_FaceList.end(),
													  SamePlaneAs(*pPlane) );
		if( find_face != m_FaceList.end() )
		{
			m_FaceList[ find_face - m_FaceList.begin() ] = 
				ExtendedFace(*find_face, face);
		}
		else
		{
			m_FaceList.push_back( face );
		}
	} //while( g_pLTServer->GetNextWMPoly(&hPoly, vol.m_hObject) == LT_OK )


	// Determine the center
	m_vCenter = LTVector(0,0,0);
	for( FaceList::iterator face_iter = m_FaceList.begin();
		 face_iter != m_FaceList.end(); ++face_iter )
	{
		LTVector face_center(0,0,0);
		for(uint32 i = 0; i < face_iter->vertexList.size(); ++i)
		{
			face_center += face_iter->vertexList[i];
		}

		face_center = face_center/float(face_iter->vertexList.size());

		m_vCenter += face_center;
	}
	m_vCenter = m_vCenter/float(m_FaceList.size());


	// Check that the volume is convex.  If it isn't 
	// an error is reported to console.
#ifndef _FINAL
	if( !ConvexFaceList(m_FaceList.begin(),m_FaceList.end()) )
	{
		g_pLTServer->CPrint("Volume \"%s\" at (%.2f,%.2f,%.2f) is not convex.",
			g_pLTServer->GetObjectName(vol.m_hObject),
			m_vCenter.x, m_vCenter.y, m_vCenter.z );
//		for( CAIVolume::FaceList::const_iterator face_iter = BeginFaces();
//		     face_iter != EndFaces(); ++face_iter )
//		{
//			for( int i = 0; i < face_iter->vertexList.size(); ++i )
//			{
//				for( int j = i+1; j < face_iter->vertexList.size(); ++j )
//				{
//					LineSystem::GetSystem(this,"ShowConcave") << LineSystem::Line( face_iter->vertexList[i], face_iter->vertexList[j], Color::Red);
//				}
//			}
//		}

	}
#endif

	// Find any doors located in our volume, if we are a door volume.

	if( vol.IsDoor() )
	{
		for( Door::DoorIterator iter = Door::BeginDoors(); iter != Door::EndDoors(); ++iter )
		{
			HOBJECT hCurDoor = (*iter)->m_hObject;

			ASSERT( hCurDoor );
			if( hCurDoor )
			{
				LTVector vPos;
				LTVector vDims;

				g_pLTServer->GetObjectPos(hCurDoor, &vPos);
				g_pLTServer->GetObjectDims(hCurDoor, &vDims);

				if ( Contains(vPos) )
				{
					m_ahDoors.push_back( DoorInfo(hCurDoor) );
				}
			}
		}
	} //if( vol.IsDoor() )

	// Link to nodes which have linked to our volume.
	for( AIVolume::NeighborLinkList::const_iterator iter = vol.m_NeighborLinkList.begin();
		 iter != vol.m_NeighborLinkList.end(); ++iter )
	{
		if( const AINode * pNode = dynamic_cast<const AINode*>( g_pLTServer->HandleToObject(*iter) ) )
		{
			if( pNode->GetID() != CAINode::kInvalidID )
			{
				m_aNeighborNodes.push_back(pNode->GetID());
			}
		}
	}

	// Deal with other special flags

	m_bIsWallWalkOnly = vol.IsWallWalkOnly();
	m_bIsLadder = vol.IsLadder();
	m_bIsStair = vol.m_bIsStair;
}

bool CAIVolume::Contains(const LTVector& vPos) const
{
	if( !m_BoundingBox.PtTouchingBox(vPos) ) return false;

	for( FaceList::const_iterator face_iter = m_FaceList.begin();
	     face_iter != m_FaceList.end(); ++face_iter )
	{
		if( face_iter->plane.DistTo(vPos) >= g_fContainsPointEpsilon )
			return false;
	}

	return true;
}

bool CAIVolume::ContainsWithNeighbor(const LTVector & vPos) const
{
	if( Contains(vPos) ) return true;

	for(NeighborList::const_iterator iter = BeginNeighbors();
		iter != EndNeighbors(); ++iter )
	{
		if( (*iter)->Contains(vPos) ) return true;
	}

	return false;
}

bool CAIVolume::Contains(const LTVector& vPos, const LTVector & vDims) const
{
	return    Contains(vPos + LTVector( vDims.x, vDims.y, vDims.z))
		   && Contains(vPos + LTVector( vDims.x, vDims.y,-vDims.z))
		   && Contains(vPos + LTVector(-vDims.x, vDims.y, vDims.z))
		   && Contains(vPos + LTVector(-vDims.x, vDims.y,-vDims.z))
		   && Contains(vPos + LTVector( vDims.x,-vDims.y, vDims.z))
		   && Contains(vPos + LTVector( vDims.x,-vDims.y,-vDims.z))
		   && Contains(vPos + LTVector(-vDims.x,-vDims.y, vDims.z))
		   && Contains(vPos + LTVector(-vDims.x,-vDims.y,-vDims.z));
}

bool CAIVolume::ContainsWithNeighbor(const LTVector& vPos, const LTVector & vDims) const
{
	return    ContainsWithNeighbor(vPos + LTVector( vDims.x, vDims.y, vDims.z))
		   && ContainsWithNeighbor(vPos + LTVector( vDims.x, vDims.y,-vDims.z))
		   && ContainsWithNeighbor(vPos + LTVector(-vDims.x, vDims.y, vDims.z))
		   && ContainsWithNeighbor(vPos + LTVector(-vDims.x, vDims.y,-vDims.z))
		   && ContainsWithNeighbor(vPos + LTVector( vDims.x,-vDims.y, vDims.z))
		   && ContainsWithNeighbor(vPos + LTVector( vDims.x,-vDims.y,-vDims.z))
		   && ContainsWithNeighbor(vPos + LTVector(-vDims.x,-vDims.y, vDims.z))
		   && ContainsWithNeighbor(vPos + LTVector(-vDims.x,-vDims.y,-vDims.z));
}


static void swap_coords( LTVector * pvEndpoint1, LTVector * pvEndpoint2, int i )
{
	ASSERT( pvEndpoint1 );
	ASSERT( pvEndpoint2 );

	switch(i)
	{
	case 0:
		std::swap(pvEndpoint1->x,pvEndpoint2->x);
		break;

	case 1:
		std::swap(pvEndpoint1->y,pvEndpoint2->y);
		break;

	case 2:
		std::swap(pvEndpoint1->z,pvEndpoint2->z);
		break;
	}

	return;
}

static bool find_endpoints( LTVector * pvEndpoint1, LTVector * pvEndpoint2, const Face & face)
{
	ASSERT( pvEndpoint1 );
	ASSERT( pvEndpoint2 );

	for( int i = 0; i < 3; ++i )
	{
		swap_coords(pvEndpoint1, pvEndpoint2, i);
		for( int j = 0; j < 3; ++j )
		{
			if( j != i ) swap_coords(pvEndpoint1, pvEndpoint2, j);

			// The two points must be on the plane.
			if(	   (float)fabs(face.plane.DistTo(*pvEndpoint1)) <= c_fNeighborThreshhold 
				&& (float)fabs(face.plane.DistTo(*pvEndpoint2)) <= c_fNeighborThreshhold  )
			{
				// And they must have an area.
				if(	   (float)fabs((pvEndpoint2->x - pvEndpoint1->x)*(pvEndpoint2->y - pvEndpoint1->y)) > c_fNeighborThreshhold
					|| (float)fabs((pvEndpoint2->x - pvEndpoint1->x)*(pvEndpoint2->z - pvEndpoint1->z)) > c_fNeighborThreshhold
					|| (float)fabs((pvEndpoint2->y - pvEndpoint1->y)*(pvEndpoint2->z - pvEndpoint1->z)) > c_fNeighborThreshhold )
				{
					return true;
				}
			}
			if( j != i ) swap_coords(pvEndpoint1, pvEndpoint2, j);
		}
		swap_coords(pvEndpoint1, pvEndpoint2, i);
	}

	return false;
}


static bool InitNeighborInfo(CAIVolume & this_volume, CAIVolume * neighbor_volume_ptr, CAINeighborInfo * pNeighborInfo)
{
	if( !neighbor_volume_ptr ) return false;

	// Go through the faces to see if any share a plane.
	for( CAIVolume::FaceList::const_iterator my_face_iter = this_volume.BeginFaces();
		 my_face_iter != this_volume.EndFaces(); ++my_face_iter )
	{
		for( CAIVolume::FaceList::const_iterator neighbor_face_iter = neighbor_volume_ptr->BeginFaces();
			 neighbor_face_iter != neighbor_volume_ptr->EndFaces(); ++neighbor_face_iter )
		{
			if(    fabs(my_face_iter->plane.Dist() + neighbor_face_iter->plane.Dist()) < c_fNeighborThreshhold
				&& (my_face_iter->plane.Normal()  + neighbor_face_iter->plane.Normal()).MagSqr() < 0.1f )
			{
				// A shared plane has been found. 
				// Now check that the faces actually overlap.  
				// This just checks that the bounding, axis-aligned rectangles for
				// each face over-lap.  
				// 
				// ( I find it helpful to do the algorithm in 1D to understand it).

				LTVector vMyMin;
				LTVector vMyMax;

				LTVector vNeighborMin;
				LTVector vNeighborMax;

				FindFaceMinMax(*my_face_iter, &vMyMin, &vMyMax);
				FindFaceMinMax(*neighbor_face_iter, &vNeighborMin, &vNeighborMax);

				LTVector vConnectionMax( Min(vMyMax.x,vNeighborMax.x),
										 Min(vMyMax.y,vNeighborMax.y),
										 Min(vMyMax.z,vNeighborMax.z) );

				LTVector vConnectionMin( Max(vMyMin.x,vNeighborMin.x),
										 Max(vMyMin.y,vNeighborMin.y),
										 Max(vMyMin.z,vNeighborMin.z) );

				if(    vConnectionMax.x >= vConnectionMin.x
					&& vConnectionMax.y >= vConnectionMin.y
					&& vConnectionMax.z >= vConnectionMin.z )
				{
					// Now try to find the "portal" formed by the interescting faces.
					LTVector vEndpoint1 = vConnectionMin;
					LTVector vEndpoint2 = vConnectionMax;

					if( find_endpoints(&vEndpoint1, &vEndpoint2, *my_face_iter) )
					{
						// Okay, we do have a valid overlap.
						ASSERT( (float)fabs(my_face_iter->plane.DistTo(vEndpoint1)) <= c_fNeighborThreshhold );
						ASSERT( (float)fabs(my_face_iter->plane.DistTo(vEndpoint2)) <= c_fNeighborThreshhold );

						if( pNeighborInfo )
						{
							*pNeighborInfo = CAINeighborInfo(&this_volume, neighbor_volume_ptr, vEndpoint1, vEndpoint2, my_face_iter->plane );

							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

uint32 CAIVolume::InitNeighbors()
{
	uint32 cNumNeighbors = 0;

	for ( CAIVolumeMgr::VolumeList::iterator iNeighborVolume = g_pAIVolumeMgr->BeginVolumes();
		   iNeighborVolume != g_pAIVolumeMgr->EndVolumes(); ++iNeighborVolume )
	{
		CAIVolume & neighbor_volume = *iNeighborVolume;

		// Don't neighbor ourself!
		if ( this != &neighbor_volume )
		{
			// Be sure we don't already have them as a neighbor.
			if( m_aNeighbors.end() == std::find(m_aNeighbors.begin(), m_aNeighbors.end(), &neighbor_volume) )
			{
				// Do a quick neighbor check.
				if ( m_BoundingBox.TouchesBox( neighbor_volume.GetBoundingBox() ) )
				{
					// Do a more thorough neighbor check.
					CAINeighborInfo neighbor_info;
					if( InitNeighborInfo(*this,&neighbor_volume,&neighbor_info) )
					{
						// We have a neighbor!  Record that fact.
						const uint32 neighbor_node_id = g_pAINodeMgr->AddVolumeNeighborNode( neighbor_info, *this );
						m_aNeighborNodes.push_back( neighbor_node_id );
						++cNumNeighbors;

						// And have our neighbor record that fact.
						const uint32 other_neighbor_node_id = g_pAINodeMgr->AddVolumeNeighborNode( neighbor_info, neighbor_volume );
						neighbor_volume.m_aNeighborNodes.push_back( other_neighbor_node_id );
						++cNumNeighbors;

						// Connect the neighbor nodes to each other.
						CAINode * pNeighborNode = g_pAINodeMgr->GetNode(neighbor_node_id);
						CAINode * pOtherNeighborNode = g_pAINodeMgr->GetNode(other_neighbor_node_id);
						
						pNeighborNode->AddNeighbor(other_neighbor_node_id);
						pOtherNeighborNode->AddNeighbor(neighbor_node_id);

						// And connect ourselves!
						m_aNeighbors.push_back(&neighbor_volume);
						neighbor_volume.m_aNeighbors.push_back(this);
					}
				}
			}
		}
	}

	return cNumNeighbors;

} //void CAIVolume::InitNeighbors()

	   
CAIVolume::NeighborList::iterator CAIVolume::BeginNeighbors()
{
	  // Reset our neighbors.
	if( m_bResetNeighbors )
	{
		m_bResetNeighbors = false;

		for( NeighborList::iterator iter = m_aNeighbors.begin(); iter != m_aNeighbors.end(); ++iter )
		{
			(*iter)->ResetPathing(m_nLastPathingIteration);
		}
	}

    return m_aNeighbors.begin();
}	

CAIVolume::NeighborList::iterator CAIVolume::EndNeighbors()
{
	  // Reset our neighbors.
	if( m_bResetNeighbors )
	{
		m_bResetNeighbors = false;

		for( NeighborList::iterator iter = m_aNeighbors.begin(); iter != m_aNeighbors.end(); ++iter )
		{
			(*iter)->ResetPathing(m_nLastPathingIteration);
		}
	}

    return m_aNeighbors.end();
}	

void CAIVolume::ResetPathing(uint32 nPathingIteration )
{
	// If we have already been reset, nothing more to do!
	if( nPathingIteration == m_nLastPathingIteration )
		return;

	// Okay, we really do need to reset ourself!

	// Do the actual reset.
	InternalResetPathing();
	
	// Don't reset again this iteration.
	m_nLastPathingIteration = nPathingIteration;


	// Be sure our neighbors will get reset.
	m_bResetNeighbors = true;
}



void CAIVolume::InternalResetPathing()
{
	m_bIsGoal = false;

	m_fCost = 0.0f;
	m_fHeuristic = 0.0f;

	m_bInOpen = false;
	m_bInClosed = false;
	m_pPrevNode = LTNULL;
}



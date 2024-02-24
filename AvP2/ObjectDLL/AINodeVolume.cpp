// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeVolume.cpp
//
// PURPOSE : AINodePatrol implementation 
//
// CREATED : 11/7/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AINodeVolume.h"
#include "AIVolumeMgr.h"
#include "AIVolume.h"
#include "AIPath.h"
#include "AI.h"
#include "AIUtils.h" // for VectorToString(const LTVector&)
#include "CharacterMovementDefs.h" // for operator>>(ILTMessage&,LTPlane&).

#ifdef _DEBUG
//#include "DebugLineSystem.h"
//#define AINODEVOLUME_DEBUG
#endif

const LTFLOAT c_fPlaneDistEpsilon = 0.01f;

static void FindEntryPos(const CAIVolumeNeighborNode & neighbor_info, LTVector start, LTVector end,
						 const LTVector & vAIDims, LTFLOAT fRadius,
						 LTVector * pvEntryPos, bool * pbClipped = LTNULL );

const char * CAIVolumeNode::szTypeName = "VolumeNode";

CAIVolumeNode::CAIVolumeNode(const CAIVolume & volume, uint32 dwID)
		: CAINode( volume.GetName().c_str(),volume.GetCenter(), 
		           volume.GetRight(), volume.GetUp(), volume.GetForward(), 
				   volume.IsWallWalkOnly(), dwID ),
		  m_dwVolumeID(volume.GetID())
{
}

CAIVolumeNode::CAIVolumeNode(HMESSAGEREAD hRead)
	: CAINode( hRead ),
	  m_dwVolumeID(CAINode::kInvalidID)
{
	InternalLoad(hRead);
}

const CAIVolume & CAIVolumeNode::GetVolume() const
{
	static CAIVolume null_volume;

	const CAIVolume * pVolume = g_pAIVolumeMgr->GetVolumePtr(m_dwVolumeID);
	if( pVolume )
		return *pVolume;

	return null_volume;
}

void CAIVolumeNode::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	CAINode::Load(hRead);

	// Need to have an internal load so that
	// we can just load our own variables from
	// the constructor.
	InternalLoad(hRead);
}

void CAIVolumeNode::InternalLoad(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	*hRead >> m_dwVolumeID;
}


void CAIVolumeNode::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	CAINode::Save(hWrite);

	*hWrite << m_dwVolumeID;
}


const char * CAIVolumeNeighborNode::szTypeName = "VolumeNeighborNode";

CAIVolumeNeighborNode::CAIVolumeNeighborNode(const CAINeighborInfo & neighbor_info, const CAIVolume & volume, uint32 dwID)
	: CAINode( (volume.GetName() + std::string("_NeighborNode") + VectorToString(neighbor_info.GetConnectionPos())).c_str(), 
	            volume.IsWallWalkOnly(), dwID),
	  m_pVolume(&volume),
	  m_dwVolumeID(volume.GetID()),
	  m_vStartConnection(neighbor_info.GetConnectionEndpoint1()),
	  m_vEndConnection(neighbor_info.GetConnectionEndpoint2()),
	  m_ConnectionPlane(neighbor_info.GetConnectionPlane())
{
	_ASSERT( m_pVolume == neighbor_info.GetVolume1Ptr() || m_pVolume == neighbor_info.GetVolume2Ptr() );

	// Be sure the connection plane is pointing away from the center.
	if( m_ConnectionPlane.DistTo(volume.GetCenter()) > 0.0f )
	{
		m_ConnectionPlane = -m_ConnectionPlane;
	}

	SetPos( neighbor_info.GetConnectionPos() - m_ConnectionPlane.Normal() );
}


CAIVolumeNeighborNode::CAIVolumeNeighborNode(HMESSAGEREAD hRead)
	: CAINode( hRead ),
	  m_pVolume(LTNULL),
	  m_dwVolumeID(CAINode::kInvalidID),
	  m_vStartConnection(0,0,0),
	  m_vEndConnection(0,0,0),
	  m_ConnectionPlane(0,0,0,0)
{
	InternalLoad(hRead);
}

void CAIVolumeNeighborNode::SetVolumePtr() const
{
	m_pVolume = g_pAIVolumeMgr->GetVolumePtr(m_dwVolumeID);
}

void CAIVolumeNeighborNode::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	CAINode::Load(hRead);

	// Need to have an internal load so that
	// we can just load our own variables from
	// the constructor.
	InternalLoad(hRead);
}

void CAIVolumeNeighborNode::InternalLoad(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	*hRead >> m_dwVolumeID;
	*hRead >> m_vStartConnection;
	*hRead >> m_vEndConnection;
	*hRead >> m_ConnectionPlane;
}


void CAIVolumeNeighborNode::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	CAINode::Save(hWrite);

	*hWrite << m_dwVolumeID;
	*hWrite << m_vStartConnection;
	*hWrite << m_vEndConnection;
	*hWrite << m_ConnectionPlane;
}


LTFLOAT CAIVolumeNeighborNode::GetClosestDistance(const LTVector & vLocation) const
{
	// Return the closest distance to the destination.

	// Determine where the desitionation is projected onto the plane.
	const LTVector vPlanePos = vLocation - m_ConnectionPlane.Normal();


	// Clip that point to the connection line segment
	const LTVector vLocalUp = LTVector(0,1,0); //neighbor_info.GetVolumePtr()->GetUp();

	LTVector vEndpointDir = m_vEndConnection - m_vStartConnection;
	vEndpointDir -= vLocalUp*vLocalUp.Dot(vEndpointDir);

	const LTFLOAT fLength = vEndpointDir.Mag();

	vEndpointDir /= fLength;

	const DFLOAT fT = (vPlanePos - m_vStartConnection).Dot(vEndpointDir);

	if ( fT > fLength ) 
	{
		return (m_vEndConnection - vLocation).Mag();
	}
	else if ( fT < 0.0f ) 
	{
		return (m_vStartConnection - vLocation).Mag();
	}

	// The projected position is between the two line segments, so use that as our distance.
	return (vPlanePos - vLocation).Mag();
}

LTVector CAIVolumeNeighborNode::AddToPath(CAIPath * pPath, CAI * pAI, const LTVector & vStart, const LTVector & vDest) const
{
	_ASSERT( pPath );
	if( !pPath ) return LTVector(0,0,0);

	if( !m_pVolume )
		SetVolumePtr();

	_ASSERT( m_pVolume );
	if( !m_pVolume )
	{
		CAIPathWaypoint waypt(GetID(), CAIPathWaypoint::eInstructionMoveTo);
		waypt.SetArgumentPosition( GetPos() );

		ASSERT( waypt.GetNodeID() != CAINode::kInvalidID );
		pPath->AddWaypoint(waypt);
		
		return GetPos();
	}


	const CAIVolumeNeighborNode * pPrevNeighborNode = dynamic_cast<const CAIVolumeNeighborNode *>( GetPrevNode() );

	// Check for a door volume.
	if( m_pVolume->HasDoors() )
	{
		// The first node in a door volume must generate an open doors command.
		// The second node does nothing (it doesn't even generate a waypoint!).
		// Because paths are generated end to beginning, the first node is actually
		// processed after the second node.
		if( pPrevNeighborNode && pPrevNeighborNode->GetContainingVolumePtr() != m_pVolume )
		{
			CAIPathWaypoint door_waypt(GetID(), CAIPathWaypoint::eInstructionOpenDoors);

			door_waypt.SetArgumentPosition(GetPos());

			// Add the doors to the waypoint.
			for( CAIVolume::const_DoorIterator iter = m_pVolume->BeginDoors();
				 iter != m_pVolume->EndDoors(); ++iter )
			{
				door_waypt.AddObject(iter->handle);
			}

			pPath->AddWaypoint(door_waypt);
		}

		return vDest;
	}

	// Otherwise its just a normal waypoint, do the usual.
	// A few useful constants.
	const LTVector & vAIDims = pAI->GetDims();
	const DFLOAT fRadius = LTVector(vAIDims.x,0.0f,vAIDims.z).Mag()*1.1f;
	// That 1.1f is just a little buffer.

	// Find the intersection point.
	// Iterate through all the previous path nodes until the line drawn from our current position
	// to the destination goes outside a
	// connection or until we reach the end of the path.  vIntersectionPoint
	// will contain the point most in line between start and destination which
	// still lies inside the connection width.
	
	LTVector vIntersectionPoint = GetPos();
	
	// Now here is a tricky loop,
	// it only increments pCurrentNeighborNode if the path was not clipped.
	bool clipped_path = false;
	const CAIVolumeNeighborNode * pCurrentNeighborNode = this;
	do
	{
		FindEntryPos( *pCurrentNeighborNode, vStart, vDest,
					  vAIDims, fRadius,
					  &vIntersectionPoint, &clipped_path);
	}
	while(    !clipped_path 
		   && (pCurrentNeighborNode = dynamic_cast<const CAIVolumeNeighborNode *>( pCurrentNeighborNode->GetPrevNode() )) );



	// If the previous while loop actually made it to prior connections, we
	// need to recalculate our intersection point based on moving from that prior 
	// connectioin intersection to our destination.
	if( pCurrentNeighborNode != this )
	{
		FindEntryPos( *this,
					  vIntersectionPoint, vDest,
					  vAIDims, fRadius,
					  &vIntersectionPoint);
	}


#ifdef AINODEVOLUME_DEBUG
	LineSystem::GetSystem(this,"ShowSmooth")
		<< LineSystem::Arrow( GetPos(), vIntersectionPoint );
#endif // AINODEVOLUME_DEBUG

	// Finally, create the way point and put it into the path!!
	CAIPathWaypoint waypt(GetID(), CAIPathWaypoint::eInstructionMoveTo);

	_ASSERT( waypt.GetNodeID() != CAINode::kInvalidID );

	// If possible, put the intersection point back into the volume.
	// This tries to set it back by the AI's radius, if that fails it
	// moves closer and closer to the actual intersection point.
	if( m_pVolume->ContainsWithNeighbor( vIntersectionPoint ) )
	{
		waypt.SetArgumentPosition( vIntersectionPoint );
	}
	else if(m_pVolume->ContainsWithNeighbor(vIntersectionPoint + m_ConnectionPlane.Normal()*fRadius/4.0f) )
	{
		waypt.SetArgumentPosition( vIntersectionPoint + m_ConnectionPlane.Normal()*fRadius/4.0f );
	}
	else if(m_pVolume->ContainsWithNeighbor(vIntersectionPoint + m_ConnectionPlane.Normal()*fRadius/3.0f) )
	{
		waypt.SetArgumentPosition( vIntersectionPoint + m_ConnectionPlane.Normal()*fRadius/3.0f );
	}
	else if(m_pVolume->ContainsWithNeighbor(vIntersectionPoint + m_ConnectionPlane.Normal()*fRadius/2.0f) )
	{
		waypt.SetArgumentPosition( vIntersectionPoint + m_ConnectionPlane.Normal()*fRadius/2.0f );
	}
	else if(m_pVolume->ContainsWithNeighbor(vIntersectionPoint + m_ConnectionPlane.Normal()*fRadius) )
	{
		waypt.SetArgumentPosition( vIntersectionPoint + m_ConnectionPlane.Normal()*fRadius );
	}
	else
	{
		_ASSERT( 0 );

		waypt.SetArgumentPosition( GetPos() );
	}

	pPath->AddWaypoint(waypt);

	return waypt.GetArgumentPosition();
}


// Don't make start and end references,  pvEntryPos is sometimes the same as start or end!
static void FindEntryPos(const CAIVolumeNeighborNode & neighbor_info, LTVector start, LTVector end,
						 const LTVector & vAIDims, LTFLOAT fRadius,
						 LTVector * pvEntryPos, bool * pbClipped /* = LTNULL */)
{

	_ASSERT( pvEntryPos ); // Not set up to handle a null return value.


	if( pvEntryPos )
	{
		// Start off assuming the entry position will be within the 
		// neighbor connection.  That way, any special cases (like start being
		// on wrong side of plane) can force clipped to be true.
		if( pbClipped ) 
			*pbClipped = false;


		// To get a smooth path, we need to back the connection plane back to where the
		// waypoint will be.  The waypoint is set back by radius, unless that is further than the center
		// of the volume (which is the case for volumes thinner than the character), in those situations the
		// point needs to be set to the center of the volume so that the next point does start behind it (as it
		// will be forced to the center of the volume as well).
		LTFLOAT fPlaneBackoffDist = -fRadius;
		if( neighbor_info.GetContainingVolumePtr() )
		{
			// Because the connection plane faces away from the normal,
			// the dist is negative.
			const LTFLOAT fDistToCenter = neighbor_info.GetConnectionPlane().DistTo( neighbor_info.GetContainingVolumePtr()->GetCenter() );

			if( fDistToCenter > fPlaneBackoffDist )
			{
				fPlaneBackoffDist = fDistToCenter;
			}
		}

		// Determine where we hit the connection plane if we just walked straight toward the next-walk-through position.
		// The result is stored in vIntersectionPoint.
		int not_used;
		LTPlane connection_plane(neighbor_info.GetConnectionPlane().Normal(),neighbor_info.GetConnectionPlane().Dist() + fPlaneBackoffDist);


		if( LTPLANE_INTERSECT_HIT != connection_plane.LineIntersect( start,
													                 end,
													                 not_used,
													                 pvEntryPos ) )
		{
			// We have gotten in here if the start and end are on the same side 
			// of the connection plane (very common in U shaped volume systems).

			// Just as a fall back, pick the start point as the desired one.
			*pvEntryPos = start;
			
			// Consider this a clipped path!
			if( pbClipped ) *pbClipped = true;
		}



		// Clip the intersection point to the connection line segment
		const LTVector vLocalUp = LTVector(0,1,0); //neighbor_info.GetVolumePtr()->GetUp();
		const LTVector & vEndpoint1 = neighbor_info.GetConnectionEndpoint1() + connection_plane.Normal()*fPlaneBackoffDist;
		const LTVector & vEndpoint2 = neighbor_info.GetConnectionEndpoint2() + connection_plane.Normal()*fPlaneBackoffDist;

		LTVector vEndpointDir = vEndpoint2 - vEndpoint1;
		vEndpointDir -= vLocalUp*vLocalUp.Dot(vEndpointDir);

		DFLOAT fLength = vEndpointDir.Mag();

		if( fLength > 1.0f )
		{
			vEndpointDir /= fLength;
		}
		else
		{
			// The endpoints must be perpindicular to the local up direction.
			// So guess a good endpoint direction.
			vEndpointDir = vLocalUp.Cross(connection_plane.Normal());
			fLength = 1.0f;
		}

		DFLOAT fT = (*pvEntryPos - vEndpoint1).Dot(vEndpointDir);

		if ( fT > (fLength-fRadius) ) 
		{
			fT = fLength-fRadius;
			if( pbClipped ) *pbClipped = true;
		}
		else if ( fT < fRadius ) 
		{
			fT = fRadius;
			if( pbClipped ) *pbClipped = true;
		}

		*pvEntryPos = vEndpoint1 + vEndpointDir*fT;
		*pvEntryPos += vLocalUp*vLocalUp.Dot(vAIDims);
	}
}

// ----------------------------------------------------------------------- //
//
// MODULE  : CAIPath.h
//
// PURPOSE : Path information encapsulation class
//
// CREATED : 7.26.1999
//
// ----------------------------------------------------------------------- //

#ifndef __AI_PATH_H__
#define __AI_PATH_H__

#include "ltsmartlink.h"

#include <deque>
#include <vector>

class CAIPathWaypoint
{
	public :

		enum Instruction
		{
			eInstructionInvalid,
			eInstructionMoveTo,
			eInstructionLadderTo,
			eInstructionWallWalkTo,
			eInstructionOpenDoors,
			eInstructionJumpTo,
			eInstructionWWJumpTo,
			eInstructionDrop,
			eInstructionBezierTo,
//			eInstructionCrawlTo,
//			eInstructionClimbTo,
//			eInstructionWaitForLift,
//			eInstructionEnterLift,
//			eInstructionRideLift,
//			eInstructionExitLift,
		};

		typedef std::vector<LTSmartLink> ObjectArgumentList;
		typedef ObjectArgumentList::const_iterator const_ObjectIterator;

	public :

		// Ctors/Dtors/etc
		
		CAIPathWaypoint( uint32 node_id, Instruction instruction = eInstructionInvalid )
			: m_nNodeID(node_id),
			  m_eInstruction(instruction),
			  m_vPosition(0,0,0),
			  m_vNormal(0,1,0) {}



		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);
		
		// Simple accessors

		uint32 GetNodeID() const { return m_nNodeID; }
		bool  IsValid() const { return m_eInstruction != eInstructionInvalid; }

		Instruction GetInstruction() const { return m_eInstruction; }
		const LTVector& GetArgumentPosition() const { return m_vPosition; }
		const LTVector& GetArgumentNormal()   const { return m_vNormal;   }

		const LTVector & GetArgumentBezierSrc() const  { return m_vBezierSrc; }
		const LTVector & GetArgumentBezierDest() const { return m_vBezierDest; }

		LTSmartLink GetObject(int i) const { if((uint32)i < m_ObjectList.size() ) return m_ObjectList[i]; return LTNULL;}

		const_ObjectIterator BeginObjectArguments() const { return m_ObjectList.begin(); }
		const_ObjectIterator EndObjectArguments() const { return m_ObjectList.end(); }

		// Instruction must always be set for the node.
		// ArgumentPosition should also be set.  If it is not set for some
		// instruction, look at all uses of GetArgumentPosition and then realize
		// that you really do want to set it.
		void SetInstruction(Instruction eInstruction) { m_eInstruction = eInstruction; }
		void SetArgumentPosition(const DVector& vPosition) { m_vPosition = vPosition; }

		void SetArgumentNormal(const DVector& vNormal) { m_vNormal = vNormal; }

		void SetArgumentBezierSrc(const DVector& vVal) { m_vBezierSrc = vVal; }
		void SetArgumentBezierDest(const DVector& vVal) { m_vBezierDest = vVal; }

		void AddObject(const LTSmartLink & hObject) { m_ObjectList.push_back(hObject); }

	private :

		uint32					m_nNodeID;
		Instruction				m_eInstruction;
		LTVector				m_vPosition;
		LTVector				m_vNormal;
		LTVector				m_vBezierSrc;
		LTVector				m_vBezierDest;
		ObjectArgumentList		m_ObjectList;
};


inline ILTMessage & operator<<(ILTMessage & out, /*const*/ CAIPathWaypoint & waypoint)
{
	waypoint.Save(&out);
	return out;
}

inline ILTMessage & operator>>(ILTMessage & in, CAIPathWaypoint & waypoint)
{
	waypoint.Load(&in);
	return in;
}

class CAIPath
{

	public :

		typedef std::deque<CAIPathWaypoint> WaypointList;
		typedef WaypointList::iterator WaypointIterator;
		typedef WaypointList::const_iterator const_WaypointIterator;

	public :

		// Ctors/Dtors/etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Waypoint methods

		// WARNING: this reference will go away after IncrementWaypointIndex, copy it if you need
		// it longer than immediately.
		const CAIPathWaypoint & GetCurrentWaypoint() const 
		{ 
			return HasRemainingWaypoints() ? m_Waypoints.front() : defaultNullWaypoint; 
		}

		const CAIPathWaypoint & GetFinalWaypoint() const 
		{ 
			return HasRemainingWaypoints() ? m_Waypoints.back() : defaultNullWaypoint; 
		}

		bool HasRemainingWaypoints() const { return !m_Waypoints.empty(); }
		
		void IncrementWaypointIndex() { if( HasRemainingWaypoints()) m_Waypoints.pop_front(); }


		// AddWaypoint adds a new waypoint to the _beginning_ of the path!
		// The paths should be built from end to start.
		void AddWaypoint(const CAIPathWaypoint& waypt) { m_Waypoints.push_front( waypt ); }
		void ClearWaypoints() { m_Waypoints.clear(); }



		WaypointIterator        BeginWaypoints() { return m_Waypoints.begin(); }
		const_WaypointIterator  BeginWaypoints() const { return m_Waypoints.begin(); }

		WaypointIterator        EndWaypoints() { return m_Waypoints.begin(); }
		const_WaypointIterator  EndWaypoints() const { return m_Waypoints.end(); }

	private :

		// The path is stored with current point at the front and the final point at the end.
		// As the path is traversed the beginning point is removed.
		// There is no way to go back along a path.
		WaypointList m_Waypoints;
		static CAIPathWaypoint defaultNullWaypoint;
};


inline ILTMessage & operator<<(ILTMessage & out, /*const*/ CAIPath & path)
{
	path.Save(&out);
	return out;
}

inline ILTMessage & operator>>(ILTMessage & in, CAIPath & path)
{
	path.Load(&in);
	return in;
}

#endif 

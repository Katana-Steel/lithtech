// ----------------------------------------------------------------------- //
//
// MODULE  : AIVolume.h
//
// PURPOSE : Creates volumetric description of level
//
// CREATED : 12.14.1999
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AIVOLUME_H__
#define __AIVOLUME_H__

#include "GameBase.h"
#include "ltsmartlink.h"

#include <float.h>
#include <vector>

class CAIVolume;

class AIVolume : public GameBase
{
	friend CAIVolume;

	public :

		typedef std::vector<std::string> NeighborNameList;
		typedef std::vector<LTSmartLink> NeighborLinkList;

	public :

		AIVolume();
		virtual ~AIVolume() {}

		bool IsDoor() const { return m_bIsDoor; }
		bool IsWallWalkOnly() const { return m_bIsWallWalkOnly; }
		bool IsLadder() const { return m_bIsLadder; }

		void AddNeighborLink(const LTSmartLink & new_neighbor);

		NeighborLinkList::const_iterator BeginNeighborLinks() const { return m_NeighborLinkList.begin(); }
		NeighborLinkList::const_iterator EndNeighborLinks() const { return m_NeighborLinkList.end(); }

		virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual void PostStartWorld(uint8 nLoadGameFlags);

	protected :

		void ReadProp(ObjectCreateStruct *pData);

	private :



		bool  m_bIsDoor;
		bool  m_bIsStair;
		bool  m_bIsWallWalkOnly;
		bool  m_bIsLadder;
		bool  m_bStartActive;

		NeighborNameList m_NeighborNameList;
		NeighborLinkList m_NeighborLinkList;

};

struct Face
{
	LTPlane   plane;
	std::vector<LTVector>  vertexList;

	Face()
		: plane(LTVector(0,0,0),0) {}
};

class CAINeighborInfo
{
	public :

		CAINeighborInfo() 
			: m_pVolume1(NULL),
			  m_pVolume2(NULL),
			  m_vConnectionPos(0.0f,0.0f,0.0f),
			  m_vConnectionEndpoint1(0.0f,0.0f,0.0f),
			  m_vConnectionEndpoint2(0.0f,0.0f,0.0f),
			  m_ConnectionPlane(0.0f,0.0f,0.0f,0.0f)	{}

		CAINeighborInfo( CAIVolume * volume1_ptr, CAIVolume * volume2_ptr,
						 const LTVector & vEndpoint1, const LTVector & vEndpoint2,
						 const LTPlane & vConnectionPlane )
			: m_pVolume1(volume1_ptr),
			  m_pVolume2(volume2_ptr),
			  m_vConnectionPos( (vEndpoint1 + vEndpoint2)/2.0f ),
			  m_vConnectionEndpoint1( vEndpoint1 ),
			  m_vConnectionEndpoint2( vEndpoint2 ),
			  m_ConnectionPlane( vConnectionPlane ) {}


		CAINeighborInfo( const CAINeighborInfo & orig, const LTVector & center_offset)
			: m_pVolume1( orig.m_pVolume1 ),
			  m_pVolume2( orig.m_pVolume2 ),
			  m_vConnectionPos( orig.m_vConnectionPos + center_offset ),
			  m_vConnectionEndpoint1( orig.m_vConnectionEndpoint1 ),
			  m_vConnectionEndpoint2( orig.m_vConnectionEndpoint2),
			  m_ConnectionPlane( orig.m_ConnectionPlane ) {}
			  

		CAINeighborInfo( HMESSAGEREAD hRead );


		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);
		void RestoreVolumePtr();


		 const CAIVolume * GetVolume1Ptr() const { return m_pVolume1; }
		 CAIVolume *	   GetVolume1Ptr() { return m_pVolume1; }
		 const CAIVolume * GetVolume2Ptr() const { return m_pVolume2; }
		 CAIVolume *	   GetVolume2Ptr() { return m_pVolume2; }
		 const LTVector& GetConnectionPos() const { return m_vConnectionPos; }
		 const LTVector& GetConnectionPerpDir() const { return m_ConnectionPlane.Normal(); }
		 
		 const LTVector& GetConnectionEndpoint1() const { return m_vConnectionEndpoint1; }
		 const LTVector& GetConnectionEndpoint2() const { return m_vConnectionEndpoint2; }

		 // This one is a hack to enable use of LTPlane::LineIntersect
		 LTPlane	 GetConnectionPlane() const { return m_ConnectionPlane; }


	private :

		CAIVolume *			m_pVolume1;
		CAIVolume *			m_pVolume2;
		LTVector			m_vConnectionPos;
		LTVector			m_vConnectionEndpoint1;
		LTVector			m_vConnectionEndpoint2;
		LTPlane				m_ConnectionPlane;
};


class CAIVolume
{
	LTVector diag_mult(const LTVector & v1, const LTVector & v2) const
	{
		return LTVector(v1.x*v2.x,v1.y*v2.y,v1.z*v2.z);
	}

	public :
		struct DoorInfo
		{
			bool        isAITriggerable;
			LTSmartLink handle;

			DoorInfo()
				: isAITriggerable(true) {}

			DoorInfo(const LTSmartLink & new_handle, bool new_isAITriggerable)
				: handle(new_handle),
				  isAITriggerable(new_isAITriggerable) {}

			explicit DoorInfo(const LTSmartLink & new_handle);
			
			
				
			DoorInfo & operator=(const LTSmartLink & new_handle)
			{
				if( this->handle != new_handle )
				{
					*this = DoorInfo(new_handle);
				}
				return *this;
			}
		};


		typedef std::vector<uint32>		  NeighborNodeList;
		typedef std::vector<CAIVolume*>	  NeighborList;
		typedef std::list<int>            NeighborIDList;
		typedef std::vector<Face>		  FaceList;

		typedef std::vector<DoorInfo>			DoorList;
		typedef DoorList::const_iterator		const_DoorIterator;

		// For pathing.
		typedef LTFLOAT CostType;
		typedef NeighborList::iterator NeighborIterator;

		enum MicrosoftHackToGetConstantsInHeader
		{
			kInvalidID = 0xFFFFFFFF
		};

	public :

		// Ctors/Dtors/etc

		CAIVolume();
		CAIVolume(uint32 iVolume, AIVolume& vol);
		CAIVolume(HMESSAGEREAD hRead);
		~CAIVolume();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);
		void RestoreNeighbors();

		void Init(AIVolume& vol);
		uint32 InitNeighbors();

		// Geometry methods

		bool Contains(const LTVector& vPos) const;
		bool ContainsWithNeighbor(const LTVector & vPos) const;

		bool Contains(const LTVector& vPos, const LTVector & vDims) const;
		bool ContainsWithNeighbor(const LTVector & vPos, const LTVector & vDims) const;

		// Neighbor methods
		void AddNeighborNode(uint32 node_id) { m_aNeighborNodes.push_back(node_id); }
		int GetNumNeighborNodes() { return m_aNeighborNodes.size(); }
		int GetNumNeighborVolumes() { return m_aNeighbors.size(); }

		NeighborList::iterator		  BeginNeighbors();
		NeighborList::const_iterator BeginNeighbors() const { return m_aNeighbors.begin(); }

		NeighborList::iterator		  EndNeighbors();
		NeighborList::const_iterator EndNeighbors() const   { return m_aNeighbors.end(); }

		NeighborNodeList::iterator	      BeginNeighborNodes()		{ return m_aNeighborNodes.begin(); }
		NeighborNodeList::const_iterator BeginNeighborNodes() const { return m_aNeighborNodes.begin(); }

		NeighborNodeList::iterator		  EndNeighborNodes()		  { return m_aNeighborNodes.end(); }
		NeighborNodeList::const_iterator EndNeighborNodes() const { return m_aNeighborNodes.end(); }

		FaceList::iterator		  BeginFaces()       { return m_FaceList.begin(); }
		FaceList::const_iterator BeginFaces() const { return m_FaceList.begin(); }

		FaceList::iterator       EndFaces()       { return m_FaceList.end(); }
		FaceList::const_iterator EndFaces() const { return m_FaceList.end(); }


		// Door methods

		LTBOOL HasDoors() const { return !m_ahDoors.empty(); }
		int GetNumDoors() const { return m_ahDoors.size(); }
		DoorList::const_iterator BeginDoors() const { return m_ahDoors.begin(); }
		DoorList::const_iterator EndDoors()   const { return m_ahDoors.end(); }
		LTSmartLink GetDoor(int iDoor) { if((uint32)iDoor < m_ahDoors.size()) return m_ahDoors[iDoor].handle; return LTNULL;}


		// Wall Walking stuff.
		LTBOOL IsWallWalkOnly() const { return m_bIsWallWalkOnly; }
		LTBOOL IsLadder() const { return m_bIsLadder; }
		LTBOOL IsStair() const { return m_bIsStair; }

		// Simple accessors

		const std::string & GetName() const { return m_strName; }
		uint32			  GetID() const		 { return m_dwID; }
		const LTVector & GetCenter() const  { return m_vCenter; }
		const LTVector & GetRight() const   { return m_vRight; }
		const LTVector & GetUp() const		 { return m_vUp; }
		const LTVector & GetForward() const { return m_vForward; }

		void   SetNodeID(uint32 val)   { m_dwNodeID = val; }
		uint32 GetNodeID() const { return m_dwNodeID; }

		LTVector GetDims()   const { return m_BoundingBox.GetDims();   }

		const LTBBox & GetBoundingBox() const { return m_BoundingBox; }

		LTBOOL  StartActive() const { return m_bStartActive; }

		// For Pathing

		// This should be called on the start and destination node with the current
		// path finding iteration (done in CAIVolumeMgr::FindPath).
		void ResetPathing(uint32 nPathingIteration );

		void  SetIsGoal(bool val) { m_bIsGoal = val; }
		void  SetCost(float val) { m_fCost = val; }
		void  SetInClosed()      { m_bInOpen = false; m_bInClosed = true;  }
		void  SetInOpen()        { m_bInOpen = true;  m_bInClosed = false; }
		void  SetHeuristic(float val) { m_fHeuristic = val; }
		void  SetPrevNode(CAIVolume * prevNode) { m_pPrevNode = prevNode; }

		bool  IsGoal() const { return m_bIsGoal; }
		float GetCost() const { return m_fCost; }
		float GetWeight()   const { return m_fCost + m_fHeuristic; }
		
		bool  InClosed() const { return m_bInClosed; }
		bool  InOpen()   const { return m_bInOpen;   }

		CAIVolume * GetPrevVolume()             { return m_pPrevNode; }
		const CAIVolume * GetPrevVolume() const { return m_pPrevNode; }


	private :
		
		void InitNeighbor(const CAIVolume * pVolumeNeigbor);

		void  InternalResetPathing();

		uint32			m_dwID;
		std::string		m_strName;
		uint32			m_dwNodeID;

		LTBOOL		m_bStartActive;

		LTVector		m_vCenter;
		LTVector		m_vRight;
		LTVector		m_vUp;
		LTVector		m_vForward;

		LTBBox			m_BoundingBox;

		NeighborNodeList	m_aNeighborNodes;  
		NeighborList		m_aNeighbors;  
		NeighborIDList		m_aNeighborIDs;

		FaceList	m_FaceList;
		DoorList	m_ahDoors;

		LTBOOL		m_bIsWallWalkOnly;
		LTBOOL		m_bIsLadder;
		LTBOOL		m_bIsStair;

		// For pathing.  Doesn't need to be saved.
		uint32		m_nLastPathingIteration;
		bool		m_bResetNeighbors;

		bool		m_bIsGoal;

		LTFLOAT		m_fCost;
		LTFLOAT		m_fHeuristic;

		bool		m_bInOpen;
		bool		m_bInClosed;
		CAIVolume *	m_pPrevNode;
};




#endif // __AIVOLUME_H__

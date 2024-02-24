// ----------------------------------------------------------------------- //
//
// MODULE  : CAIVolumeMgr.h
//
// PURPOSE : CAIVolumeMgr definition
//
// CREATED : 12/28/99
//
// ----------------------------------------------------------------------- //

#ifndef __AI_VOLUME_MGR_H__
#define __AI_VOLUME_MGR_H__

#include "GameBase.h"
#include "AIVolume.h"
#include <list>
#include <deque>

//#define USE_AIPOLY

#ifdef USE_AIPOLY
#include <map>
#endif


// Externs

extern class CAIVolumeMgr* g_pAIVolumeMgr;


#ifdef USE_AIPOLY
struct AIPoly
{
	typedef std::vector<LTVector> VertexList;

	LTPlane   plane;
	VertexList  vertexList;
	LTVector  center;
	LTFLOAT   radius;

	std::vector<AIPoly*>  neighborList;

	AIPoly()
		: plane(LTVector(0,0,0),0),
		  center(0,0,0),
		  radius(0) {}

	void finish(uint32 num_vertices);
};
#endif

// Classes

class AIVolumeMgr : public GameBase
{
	public :


	protected :

		virtual DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
};

class CAIVolumeMgr
{
	public :

		typedef std::list<CAIVolume> VolumeList;
		typedef VolumeList::iterator VolumeIterator;
		typedef VolumeList::const_iterator const_VolumeIterator;

		typedef std::list<AIVolume*> VolumeObjectList;

		typedef std::deque<CAIVolume*> VolumeIDList;

#ifdef USE_AIPOLY
		typedef std::map<HPOLY,AIPoly> PolyMap;
#endif

		// Ctors/Dtors/etc

		CAIVolumeMgr();
		~CAIVolumeMgr();

		void Init();
		void Term();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		DBOOL IsInitialized() { return m_bInitialized; }

		void AddVolumeObject(AIVolume * pVolume) { m_aVolumeObjects.push_back(pVolume); }

		// Methods
		
		CAIVolume * GetVolumePtr(uint32 volume_id) { return ( volume_id < m_aVolumeIDs.size() )  ? m_aVolumeIDs[volume_id] : LTNULL; }
		CAIVolume* FindContainingVolume(const LTVector& vPos);
		CAIVolume* FindContainingVolume(const LTVector& vPos, const DVector & vDims);

		CAIVolume* FindFallToVolume(const LTVector& vPos);

		// Simple accessors

		int GetNumVolumes() { return m_aVolumes.size(); }

#ifndef _FINAL
		void ShowVolumes() 
		{
			if( !m_bShowingVolumes )
			{
				m_bShowingVolumes = true;
				DrawTheVolumes();
			}
		}

		void HideVolumes()
		{
			if( m_bShowingVolumes )
			{
				m_bShowingVolumes = false;
				ClearTheVolumes();
			}
		}
#endif

		VolumeList::iterator  BeginVolumes() { return m_aVolumes.begin(); }
		VolumeList::iterator  EndVolumes()   { return m_aVolumes.end();   }

		// This does a simple a-star to see if two volumes are connected.
		// Simple means that only the volume's center-points are used to determine
		// path cost.  If you actually tried to traverse the path returned, you may not
		// always get the shortest path (as it does not account for taking short-cuts within
		// a volume).
		bool FindPath(const LTVector &vGoalPos, const CAIVolume * pDest, const CAIVolume * pSrc, LTFLOAT fMaxSearchCost = -1.0f );

	private :

#ifndef _FINAL
		void DrawTheVolumes();
		void ClearTheVolumes();
#endif

	private :

		VolumeObjectList m_aVolumeObjects;

		bool		m_bInitialized;
		VolumeList	m_aVolumes;
		VolumeIDList m_aVolumeIDs;

		bool		m_bShowingVolumes;

#ifdef USE_AIPOLY
		PolyMap		m_aPolies;
#endif
};

#endif // __AI_VOLUME_MGR_H__

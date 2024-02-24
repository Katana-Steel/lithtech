// ----------------------------------------------------------------------- //
//
// MODULE  : CAINodeMgr.h
//
// PURPOSE : CAINodeMgr definition
//
// CREATED : 12/28/98
//
// ----------------------------------------------------------------------- //

#ifndef __AI_NODE_MGR_H__
#define __AI_NODE_MGR_H__

#include <vector>
#include "AINode.h"

class CAINode;
class CAINodeCover;
class CAINodeSnipe;
class CAINodeAlarm;
class BaseClass;
class CAIVolume;
class CAINeighborInfo;
class CAINodeWallWalk;

// Externs

extern class CAINodeMgr* g_pAINodeMgr;

// Classes
class AINodeMgr : public GameBase
{
	public :


	protected :

		virtual DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
};


class CAINodeMgr
{
	public :

		typedef std::vector<CAINode*>		NodeList;
		typedef std::vector<CAINodeCover *> CoverNodeList;
		typedef std::vector<CAINodeSnipe *> SnipeNodeList;
		typedef std::vector<CAINodeAlarm *> AlarmNodeList;
		typedef std::vector<CAINodeWallWalk *> WWAdvanceNodeList;

		typedef std::list<AINode*>	NodeObjectList;

		typedef std::list< std::pair<CAI*, std::string> >		AITeleportList;

	public : // Public methods

		// Ctors/Dtors/Etc

		CAINodeMgr();
		~CAINodeMgr() { Term(); }

		void Init();
		void Term();

		// Methods

#ifndef _FINAL
		void AddNodeDebug(int level);
		void RemoveNodeDebug();
#endif
		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		void AddNodeObject(AINode * pNode) { m_apNodeObjects.push_back(pNode); }

		void AddTeleportee(CAI * pAI, const std::string & strNodeName);

		// General purpose
		void			SortNodesByDist(CoverNodeList &aNodes, uint32 lo, uint32 hi, const LTVector& vPos);

		// Cover node methods
		CAINodeCover *	FindAdvanceNode(HOBJECT hAI, HOBJECT hTarget);
		CAINodeCover *	FindDodgeNode(HOBJECT hAI, HOBJECT hAttacker);
		CAINodeCover *	FindCowerNode(HOBJECT hAI, HOBJECT hAttacker, HOBJECT hGroup = LTNULL);
		
		CAINodeWallWalk * FindWWAdvanceNode(HOBJECT hAI, HOBJECT hTarget);

		// Snipe node methods
		CAINodeSnipe *	FindSnipeNode(HOBJECT hAI, HOBJECT hTarget = LTNULL, HOBJECT hGroup = LTNULL);
		CAINodeSnipe *	FindVisibleSnipeNode(HOBJECT hAI, HOBJECT hTarget, HOBJECT hGroup = LTNULL, LTFLOAT fVerticalOffset = 50.0f);

		// Alarm node method
		CAINodeAlarm *	FindAlarmNode(HOBJECT hAI, HOBJECT hGroup = LTNULL);

		// Volume nodes.
		uint32 AddVolumeNodes(CAIVolume * pVolume);
		uint32 AddVolumeNeighborNode(const CAINeighborInfo & neighbor_info, const CAIVolume & volume);

		// Simple accesors

		LTBOOL IsInitialized() { return m_bInitialized; }
		uint32 GetNumNodes() { return m_apNodes.size(); }
		uint32 GetNumCoverNodes() { return m_apCoverNodes.size(); }
		uint32 GetNumSnipeNodes() { return m_apSnipeNodes.size(); }

		// Node lookup methods

		CAINode* GetNode(uint32 iNode) { _ASSERT(iNode < m_apNodes.size() || iNode == CAINode::kInvalidID); return  (iNode < m_apNodes.size())  ? m_apNodes[iNode] : LTNULL; }
		CAINode* GetNode(const char *szName);

		NodeList::iterator BeginNodes() { return m_apNodes.begin(); }
		NodeList::iterator EndNodes()   { return m_apNodes.end(); }

	private : // Private member variables

		void FillSpecialNodeLists();

		LTBOOL				m_bInitialized;		// Is the NodeMgr initialized?

		NodeObjectList		m_apNodeObjects;

		NodeList		m_apNodes;		// All the nodes
		CoverNodeList	m_apCoverNodes;	// All cover nodes
		SnipeNodeList	m_apSnipeNodes;	// All snipe nodes
		AlarmNodeList	m_apAlarmNodes;	// All alarm nodes
		WWAdvanceNodeList m_apWWAdvanceNodes; // All wall-walk advance nodes.

		AITeleportList			m_AITeleportList; 
};

#endif // __AI_NODE_MGR_H__

// ----------------------------------------------------------------------- //
//
// MODULE  : CSimpleNodeMgr.h
//
// PURPOSE : CSimpleNodeMgr definition
//
// CREATED : 7/8/01
//
// ----------------------------------------------------------------------- //

#ifndef __SIMPLE_NODE_MGR_H__
#define __SIMPLE_NODE_MGR_H__

#include <vector>
#include "SimpleNode.h"

// Externs

extern class CSimpleNodeMgr* g_pSimpleNodeMgr;


class CSimpleNodeMgr
{
	public :

		typedef std::vector<CSimpleNode*>		SimpleNodeList;

	public : // Public methods

		// Ctors/Dtors/Etc

		CSimpleNodeMgr();
		~CSimpleNodeMgr() { Term(); }

		void Init();
		void Term();

		// Methods

#ifndef _FINAL
		void AddNodeDebug(int level);
		void RemoveNodeDebug();
#endif

		void AddNode(const SimpleNode & new_node);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Path finding.

		bool FindPath(const LTVector &vGoalPos, const CSimpleNode * pDest, const CSimpleNode * pSrc);

		// Simple accesors

		LTBOOL IsInitialized() { return m_bInitialized; }
		uint32 GetNumNodes() { return m_apNodes.size(); }

		// Node lookup methods

		CSimpleNode* GetNode(uint32 iNode) { _ASSERT(iNode < m_apNodes.size() || iNode == CSimpleNode::kInvalidID); return  (iNode < m_apNodes.size())  ? m_apNodes[iNode] : LTNULL; }

		SimpleNodeList::iterator BeginNodes() { return m_apNodes.begin(); }
		SimpleNodeList::iterator EndNodes()   { return m_apNodes.end(); }

	private : // Private member variables

		LTBOOL				m_bInitialized;		// Is the NodeMgr initialized?

		SimpleNodeList		m_apNodes;		// All the nodes
};

#endif // __SIMPLE_NODE_MGR_H__

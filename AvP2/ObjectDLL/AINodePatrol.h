// ----------------------------------------------------------------------- //
//
// MODULE  : AINodePatrol.h
//
// PURPOSE : AINode class definition
//
// CREATED : 11/7/00
//
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_PATROL_H_
#define _AI_NODE_PATROL_H_


#include "AINode.h"

class AINodePatrol : public AINode
{

	friend class CAINodePatrol;

public :

	AINodePatrol()
		: m_fWaitTime(0.0f),
		  m_bCycle(false),
		  m_bCircular(false),
		  m_nID(CAINode::kInvalidID),
		  m_nNextID(CAINode::kInvalidID),
		  m_nPrevID(CAINode::kInvalidID)    {}

	virtual CAINode * MakeCAINode(DDWORD dwID);

	virtual void ReadProp(ObjectCreateStruct *p);

protected :

	virtual void FirstUpdate(); 

private :

	LTFLOAT		m_fWaitTime;

	std::string m_strNextName;
	
	LTSmartLink m_hNext;
	LTSmartLink m_hPrev;

	bool		m_bCycle;
	bool		m_bCircular;

	uint32 m_nID;
	uint32 m_nNextID;
	uint32 m_nPrevID;
};

class CAINodePatrol : public CAINode
{

public :

	CAINodePatrol(AINodePatrol & node, DDWORD dwID);
	CAINodePatrol(HMESSAGEREAD hRead);

	// Used for type identity during load/save.
	static const char * const szTypeName;
	virtual const char * GetTypeName() const { return CAINodePatrol::szTypeName; }

	virtual void Load(HMESSAGEREAD hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	bool GetCycle() const { return m_bCycle; }

	LTFLOAT GetWaitTime() const { return m_fWaitTime; }

	uint32 GetNextID(bool bForward = true) const { return bForward ? m_NextID : m_PrevID; }
	uint32 GetPrevID() const { return GetNextID(false); }

	void SetNextID(uint32 val) { m_NextID = val; }
	void SetPrevID(uint32 val) { m_PrevID = val; }

#ifndef _FINAL
	virtual void DrawDebug(int level);
	virtual void ClearDebug();
#endif

	virtual LTBOOL IsPathingNode() const { return LTFALSE; }

protected :

#ifndef _FINAL
	virtual LTVector GetNodeColor() const { return LTVector(0.1f,0.75f,0.0f); }
#endif

private :

	void InternalLoad(HMESSAGEREAD hRead);

	LTFLOAT m_fWaitTime;

	bool		m_bCycle;

	uint32		m_NextID;
	uint32		m_PrevID;
};


#endif  //_AI_NODE_PATROL_H_


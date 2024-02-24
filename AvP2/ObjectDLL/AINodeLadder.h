// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeLadder.h
//
// PURPOSE : AINodeLadder class definition
//
// CREATED : 3/30/01
//
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_LADDER_H_
#define _AI_NODE_LADDER_H_


#include "AINode.h"

#include <string>
#include <vector>

class CAINodeLadder;

class AINodeLadder : public AINode
{
	friend CAINodeLadder;

	public :
		AINodeLadder()
			: m_bUseBaseName(LTFALSE) {}

		virtual CAINode * MakeCAINode(uint32 dwID);

		virtual void ReadProp(ObjectCreateStruct *p);

	protected :

		virtual void FirstUpdate();

	private :

		LTBOOL m_bUseBaseName;
};

class CAINodeLadder : public CAINode
{
public :

	CAINodeLadder(AINodeLadder & node, DDWORD dwID);
	CAINodeLadder(HMESSAGEREAD hRead);

	virtual void Load(HMESSAGEREAD hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	// Used for type identity during load/save.
	static const char * const szTypeName;
	virtual const char * GetTypeName() const { return CAINodeLadder::szTypeName; }

	virtual LTVector AddToPath(CAIPath * pPath, CAI * pAI, const LTVector & vStart, const LTVector & vDest) const;

	virtual LTBOOL UseVolumePathing() const { return LTFALSE; }

	virtual LTBOOL IsLadder() const { return LTTRUE; }

protected :

#ifndef _FINAL
	virtual LTVector GetNodeColor() const { return LTVector(0.25f,0.0f,1.0f); }
#endif

private :

	void InternalLoad(HMESSAGEREAD hRead);

};


#endif  //_AI_NODE_LADDER_H_


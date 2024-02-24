// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeWallWalk.h
//
// PURPOSE : AINodeWallWalk class definition
//
// CREATED : 10/25/00
//
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_WALLWALK_H_
#define _AI_NODE_WALLWALK_H_


#include "AINode.h"

#include <string>
#include <vector>

class CAINodeWallWalk;

class AINodeWallWalk : public AINode
{
	friend CAINodeWallWalk;

	public :
		AINodeWallWalk()
			: m_bUseBaseName(LTFALSE),
			  m_bDropPoint(LTFALSE),
			  m_bAdvancePoint(LTFALSE) {}

		virtual CAINode * MakeCAINode(uint32 dwID);

		virtual void ReadProp(ObjectCreateStruct *p);

	protected :

		virtual void FirstUpdate();

	private :

		LTBOOL m_bUseBaseName;
		LTBOOL m_bDropPoint;
		LTBOOL m_bAdvancePoint;
};

class CAINodeWallWalk : public CAINode
{
public :

	static const LTFLOAT k_fWallWalkPreference;

public :

	CAINodeWallWalk(AINodeWallWalk & node, DDWORD dwID);
	CAINodeWallWalk(HMESSAGEREAD hRead);

	virtual void Load(HMESSAGEREAD hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	// Used for type identity during load/save.
	static const char * const szTypeName;
	virtual const char * GetTypeName() const { return CAINodeWallWalk::szTypeName; }

	virtual LTVector AddToPath(CAIPath * pPath, CAI * pAI, const LTVector & vStart, const LTVector & vDest) const;

	virtual LTBOOL UseVolumePathing() const { return LTFALSE; }  // We don't want to drop into a volume when we re-path.  We want to stick to wall walk nodes.

	LTBOOL IsAdvanceNode() const { return m_bAdvancePoint; }

	// Should be called after the volumes are loaded to make the drop connection.
	void FindDropVolume();

	// For pathing.
	virtual LTFLOAT GenerateHeuristic(const LTVector & vDestination, LTFLOAT fImpassableCost) const  // Calculates the cost to move to that destination.
	{
		return ( GetPos() - vDestination ).Mag()*k_fWallWalkPreference;
	}

	virtual LTFLOAT GenerateCost(const CAINode & dest_node, LTFLOAT fImpassableCost) const  // Calculates the cost to move to that destination.
	{
		const LTFLOAT fWallWalkPreference = 0.8f;

		if( dest_node.IsWallWalkOnly() )
			return CAINode::GenerateCost(dest_node, fImpassableCost )*k_fWallWalkPreference;

		return CAINode::GenerateCost(dest_node, fImpassableCost );
	}

protected :

#ifndef _FINAL
	virtual LTVector GetNodeColor() const { return LTVector(1.0f,1.0f,0.0f); }
#endif

private :

	void InternalLoad(HMESSAGEREAD hRead);

	LTBOOL m_bDropPoint;
	LTBOOL m_bAdvancePoint;
	uint32 m_dwDropToVolumeID;

};


#endif  //_AI_NODE_WALLWALK_H_


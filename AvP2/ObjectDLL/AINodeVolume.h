// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeVolume.h
//
// PURPOSE : AIVolumeNode and AIVolumeNeighborNode class definition
//
// CREATED : 2/14/00
//
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_VOLUME_H_
#define _AI_NODE_VOLUME_H_


#include "AINode.h"

class CAIVolume;
class CAINeighborInfo;

class CAIVolumeNode : public CAINode
{
public :

	CAIVolumeNode(const CAIVolume & volume, uint32 dwID);
	CAIVolumeNode(HMESSAGEREAD hRead);

	const CAIVolume & GetVolume() const;
	virtual const CAIVolume * GetContainingVolumePtr() const { return &GetVolume(); }


	static const char * szTypeName;
	virtual const char * GetTypeName() const { return CAIVolumeNode::szTypeName; }

#ifndef _FINAL
	virtual void DrawDebug(int level) { if( level > 1 ) CAINode::DrawDebug(level); }
#endif

	virtual void Load(HMESSAGEREAD hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

protected :

#ifndef _FINAL
	LTVector GetNodeColor() const { return LTVector(1.0f,1.0f,0.0f); }
#endif

private:

	void InternalLoad(HMESSAGEREAD hRead);

	uint32  m_dwVolumeID;

};

class CAIVolumeNeighborNode : public CAINode
{
public :

	CAIVolumeNeighborNode(const CAINeighborInfo & neighbor_info, const CAIVolume & volume, uint32 dwID);
	CAIVolumeNeighborNode(HMESSAGEREAD hRead);

	static const char * szTypeName;
	virtual const char * GetTypeName() const { return CAIVolumeNeighborNode::szTypeName; }

	virtual void Load(HMESSAGEREAD hRead);
	virtual void Save(HMESSAGEWRITE hWrite);


	const CAIVolume * GetContainingVolumePtr() const { if( !m_pVolume ) SetVolumePtr(); return m_pVolume; }

	const LTVector & GetConnectionEndpoint1() const { return m_vStartConnection; }
	const LTVector & GetConnectionEndpoint2() const { return m_vEndConnection; }
	const LTPlane & GetConnectionPlane() const { return m_ConnectionPlane; }

	virtual LTFLOAT GenerateHeuristic(const LTVector & vDestination, LTFLOAT fImpassableCost) const
	{
		return GetClosestDistance(vDestination);
	}

	virtual LTFLOAT GenerateCost(const LTVector & vDestination, LTFLOAT fImpassableCost) const
	{
		return GetClosestDistance(vDestination);
	}

	LTFLOAT GetClosestDistance(const LTVector & vLocation) const;

	virtual LTVector AddToPath(CAIPath * pPath, CAI * pAI, const LTVector & vStart, const LTVector & vDest) const;

#ifndef _FINAL
	virtual void DrawDebug(int level) { if( level > 1 ) CAINode::DrawDebug(level); }
#endif

protected :

#ifndef _FINAL
	LTVector GetNodeColor() const { return LTVector(0.0f,1.0f,0.0f); }
#endif

private:


	void SetVolumePtr() const;
	void InternalLoad(HMESSAGEREAD hRead);

	mutable const CAIVolume *	m_pVolume;
	uint32		m_dwVolumeID;
	LTVector	m_vStartConnection;
	LTVector	m_vEndConnection;
	LTPlane		m_ConnectionPlane;
};

#endif  //_AI_NODE_VOLUME_H_


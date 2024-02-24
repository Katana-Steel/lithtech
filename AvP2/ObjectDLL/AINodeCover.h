// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeCover.h
//
// PURPOSE : Tactical node definition
//
// CREATED : 08/28/00
//
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_COVER_H_
#define _AI_NODE_COVER_H_


#include "AINode.h"

#include <string>


class AINodeCover : public AINode
{
	friend class CAINodeCover;

public:
	AINodeCover();

	virtual CAINode * MakeCAINode(DDWORD dwID);

	virtual void ReadProp(ObjectCreateStruct *p);

private :

	std::string	m_strCoverObject;
	LTFLOAT		m_fCoverRadiusSqr;
	LTBOOL		m_bCrouch;
	LTBOOL		m_bCower;
};

class CAINodeCover : public CAINode
{
	public :

		CAINodeCover(AINodeCover & node, DDWORD dwID);
		CAINodeCover(HMESSAGEREAD hRead);

		// Used for type identity during load/save.
		static const char * const szTypeName;
		virtual const char * GetTypeName() const { return CAINodeCover::szTypeName; }

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		LTBOOL IsCoverFromTarget(const CAI & ai) const;
		LTBOOL IsCoverFromThreat(const CAI & ai, HOBJECT hThreat) const;

		LTBOOL	HasCoverObject() const { return !m_strCoverObject.empty(); }
		const std::string	& GetCoverObject() const { return m_strCoverObject; }
		void	ClearCoverObject() { m_strCoverObject = ""; }

		LTFLOAT GetCoverRadiusSqr() const { return m_fCoverRadiusSqr; }
		LTBOOL	GetCrouch() const { return m_bCrouch; }
		LTBOOL	GetCower() const { return m_bCower; }

		virtual LTBOOL IsPathingNode() const { return LTFALSE; }

	protected :

#ifndef _FINAL
		virtual LTVector GetNodeColor() const { return LTVector(0.0f,0.5f,1.0f); }
#endif

	private :

		void InternalLoad(HMESSAGEREAD hRead);

		std::string	m_strCoverObject;			// The name of the cover object
		LTFLOAT		m_fCoverRadiusSqr;			// How close you have to be to this node.
		LTBOOL		m_bCrouch;					// Crouch at this node?
		LTBOOL		m_bCower;					// Cower at this node?
};

#endif // __AI_NODE_COVER_H__

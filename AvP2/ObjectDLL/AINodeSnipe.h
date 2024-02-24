// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeSnipe.h
//
// PURPOSE : Provides sniper vantage points
//
// CREATED : 10/03/00
//
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_SNIPE_H_
#define _AI_NODE_SNIPE_H_

//#include "cpp_engineobjects_de.h"
//#include "LTSmartLink.h"
//#include "LTString.h"
//#include <string>

#include "AINode.h"
//#include "GameBase.h"

class AINodeSnipe : public AINode
{
	friend class CAINodeSnipe;

	public:

		AINodeSnipe();

		virtual CAINode * MakeCAINode(DDWORD dwID);

		virtual void ReadProp(ObjectCreateStruct *p);

	private :

		LTFLOAT		m_fRadiusSqr;
		LTBOOL		m_bCrouch;
};

class CAINodeSnipe : public CAINode
{
	public :

		CAINodeSnipe(AINodeSnipe & node, DDWORD dwID);
		CAINodeSnipe(HMESSAGEREAD hRead);

		// Used for type identity during load/save.
		static const char * const szTypeName;
		virtual const char * GetTypeName() const { return CAINodeSnipe::szTypeName; }

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		LTFLOAT GetRadiusSqr() const { return m_fRadiusSqr; }
		virtual LTBOOL IsPathingNode() const { return LTFALSE; }

	protected :

#ifndef _FINAL
		virtual LTVector GetNodeColor() const { return LTVector(0.5f,0.0f,0.5f); }
#endif

	private :

		void InternalLoad(HMESSAGEREAD hRead);

		LTFLOAT		m_fRadiusSqr;				// How close you have to be to this node.
		LTBOOL		m_bCrouch;					// Crouch at this node?
};

#endif // __AI_NODE_SNIPE_H__

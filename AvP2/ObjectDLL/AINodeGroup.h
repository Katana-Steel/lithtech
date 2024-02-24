// ----------------------------------------------------------------------- //
//
// MODULE  : NodeGroup.h
//
// PURPOSE : AINodes may point to one a NodeGroup object.  This is used by
//			 AIs to restrict the set of nodes that they are aware of.
//
// CREATED : 03/01/01
//
// ----------------------------------------------------------------------- //

#ifndef __AI_NODE_GROUP_H__
#define __AI_NODE_GROUP_H__

#include "cpp_engineobjects_de.h"
#include "ClientServerShared.h"

class AINodeGroup : public BaseClass
{
	public : // Public methods

		AINodeGroup() : BaseClass (OT_NORMAL) {}
};

BEGIN_CLASS(AINodeGroup)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(AINodeGroup, BaseClass, NULL, NULL)

#endif

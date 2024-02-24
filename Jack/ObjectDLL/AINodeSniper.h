// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef _AI_NODE_SNIPER_H_
#define _AI_NODE_SNIPER_H_

#include "AINode.h"

LINKTO_MODULE( AINodeSniper );

class AINodeSniper : public AINodeUseObject
{
	typedef AINodeUseObject super;

	public :

		// Ctors/Dtors/etc

		AINodeSniper();
		virtual ~AINodeSniper();

		// Engine 

		virtual void ReadProp(ObjectCreateStruct *pocs);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Init

		virtual void Init();

		// Status

		virtual EnumNodeStatus	GetStatus( CAI* pAI, const LTVector& vPos, HOBJECT hThreat) const;		

	protected :	
		
		LTFLOAT		m_fFovDp;
		LTFLOAT		m_fThreatRadiusSqr;
};

#endif // _AI_NODE_H_

// ----------------------------------------------------------------------- //
//
// MODULE  : Group.h
//
// PURPOSE : Group - Definition
//
// CREATED : 12/21/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GROUP_H__
#define __GROUP_H__

#include "cpp_engineobjects_de.h"
#include "CommonUtilities.h"

#define MAX_GROUP_TARGETS	50

class Group : public BaseClass
{
	public:

		Group();
		~Group();

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private:

		void		PreCreate(ObjectCreateStruct *pStruct);
		void		Load(HMESSAGEREAD hRead, DDWORD dwSaveFlags);
		void		Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void		HandleTrigger(HOBJECT hSender, char *pMsg);

		HSTRING	m_hstrObjectNames[MAX_GROUP_TARGETS];
};

#endif  // __GROUP_H__



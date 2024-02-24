// ----------------------------------------------------------------------- //
//
// MODULE  : MDTrackerItem.h
//
// PURPOSE : MDTrackerItem - Definition
//
// CREATED : 6/7/2000
//
// ----------------------------------------------------------------------- //

#ifndef __MDTRACKER_ITEM_H__
#define __MDTRACKER_ITEM_H__

#include "ltengineobjects.h"

class MDTrackerItem : public BaseClass
{
	public :

		MDTrackerItem();
		~MDTrackerItem();

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT lData);
		virtual uint32 ObjectMessageFn( HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead );

	private:
		void		SendPositionToClient();
		void		Save(HMESSAGEWRITE pData, DDWORD fData);
		void		Load(HMESSAGEREAD pData, DDWORD fData);

};

#endif // __MDTRACKER_ITEM_H__

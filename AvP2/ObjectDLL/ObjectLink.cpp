// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectLink.h
//
// PURPOSE : CObjectLink class
//
// CREATED : 2/14/00
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ObjectLink.h"

#include <algorithm>


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectLinkBank::CObjectLinkBank
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CObjectLinkBank::CObjectLinkBank()
	: m_hOwner(DNULL) {}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectLinkBank::~CObjectLinkBank
//
//	PURPOSE:	ObjectLinkBank
//
// ----------------------------------------------------------------------- //

CObjectLinkBank::~CObjectLinkBank()
{
	if( m_hOwner != DNULL )
	{
		while( !m_pLinks.empty() )
		{
			CObjectLink * pLink = m_pLinks.back();

			if( pLink ) pLink->BreakLink();

			m_pLinks.pop_back();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectLinkBank::Register
//
//	PURPOSE:	Registers a new ObjectLink into the bank
//
// ----------------------------------------------------------------------- //
void	CObjectLinkBank::Register(CObjectLink & new_link)
{
#ifdef DEBUG
	// be sure this isn't a repeat (doesn't matter, really)
	LinkList iterator = std::find(m_pLinks.begin(),m_pLinks.end(), &new_link);
	ASSERT( iterater == m_pLinks.end() );
#endif

	m_pLinks.push_back( &new_link );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectLinkBank::Deregister
//
//	PURPOSE:	Removes a link from the bank.
//
// ----------------------------------------------------------------------- //
void	CObjectLinkBank::Deregister(CObjectLink & old_link)
{
	m_pLinks.remove(&old_link);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectLinkBank::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //
		
DDWORD CObjectLinkBank::EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate(pObject);
			}
		}
		break;

		case MID_LINKBROKEN :
		{
			if( pData )
			{
				const HOBJECT dead_object = static_cast<HOBJECT>(pData);

				for( LinkList::iterator iter = m_pLinks.begin();
				     iter != m_pLinks.end(); ++iter )
				{
					CObjectLink * pLink = *iter;
					if( pLink && pLink->GetHandle() ==  dead_object )
					{
						pLink->BreakLink();
					}
				} 

			} //if( pData )
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((LMessage*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((LMessage*)pData);
		}
		break;
	}

	return Aggregate::EngineMessageFn(pObject, messageID, pData, fData);
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectLinkBank::InitialUpdate
//
//	PURPOSE:	Handle object initial update
//
// ----------------------------------------------------------------------- //

void CObjectLinkBank::InitialUpdate(LPBASECLASS pObject)
{
	if (!pObject || !pObject->m_hObject) return;

	ASSERT( !m_hOwner );

	if (!m_hOwner) 
		m_hOwner = pObject->m_hObject;

	for( LinkList::iterator iter = m_pLinks.begin();
		 iter != m_pLinks.end(); ++iter )
	{
		// re-assign links, so that interobject links will be created if needed.
		CObjectLink * pLink = *iter;
		if( pLink )
		{
			pLink->Refresh();
		}
	}
	
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectLinkBank::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CObjectLinkBank::Save(LMessage * pMsg)
{
	if (!pMsg) return;

	*pMsg << m_hOwner;
	
	// m_pLists will be rebuilt as each CObjectLink is loaded.
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectLinkBank::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CObjectLinkBank::Load(LMessage * pMsg)
{
	if (!pMsg) return;

	*pMsg >> m_hOwner;

	if( m_hOwner )
	{
		for( LinkList::iterator iter = m_pLinks.begin();
			 iter != m_pLinks.end(); ++iter )
		{
			// re-assign links, so that interobject links will be created if needed.
			CObjectLink * pLink = *iter;
			if( pLink )
			{
				pLink->Refresh();
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectLink::Save
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CObjectLink::Save(LMessage * pMsg) const
{
	ASSERT( pMsg );

	if (!pMsg) return;

	*pMsg << const_cast<HOBJECT>(m_hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectLink::Load
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CObjectLink::Load(LMessage * pMsg)
{
	ASSERT( pMsg );

	if (!pMsg) return;

	*pMsg >> m_hObject;

	if( m_hObject && m_pBank && m_pBank->GetOwner())
	{
		g_pInterface->CreateInterObjectLink(m_pBank->GetOwner(),m_hObject);
	}
}

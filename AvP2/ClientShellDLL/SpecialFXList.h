// ----------------------------------------------------------------------- //
//
// MODULE  : SpecialFXList.h
//
// PURPOSE : List of CSpecialFX class objects
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __SPECIAL_FX_LIST_H__
#define __SPECIAL_FX_LIST_H__

#include "SpecialFX.h"

#define  DEFAULT_MAX_NUM	50


class CSpecialFXList
{
	public :

		struct Element
		{
			CSpecialFX * pSFX;
			LTFLOAT fNextUpdateTime;

			Element(CSpecialFX * sfx)
				: pSFX(sfx),
				fNextUpdateTime(0.0f) {}
		};

		typedef std::list<Element>   List;
		typedef List::iterator       Iterator;
		typedef List::const_iterator const_Iterator;

	public :

		int GetMaxSize()	const { return m_nMaxSize; }
		int GetNumItems()	const { return m_nElements; }
		DBOOL IsEmpty()		const { return (DBOOL)(m_nElements == 0); }

		CSpecialFXList()
			: // m_Elements
			  m_nElements(0),
			  m_nMaxSize(DEFAULT_MAX_NUM) {}

		~CSpecialFXList()
		{
			RemoveAll();
		}

		void SetMaxSize(unsigned int nMaxNum=DEFAULT_MAX_NUM)
		{
			m_nMaxSize = nMaxNum;

			// Make sure we aren't over our new maximum size.
			while( m_nElements > m_nMaxSize )
			{
				delete m_Elements.front().pSFX;
				m_Elements.pop_front();

				--m_nElements;
			}
		}

		DBOOL Add(CSpecialFX* pFX)
		{
			_ASSERT( m_nElements == int(m_Elements.size()) );

			_ASSERT( pFX );

			// Add the new element.
			if( m_nMaxSize > 0 )
			{
				m_Elements.push_back( Element(pFX) );
				++m_nElements;
			}

			// Make sure we aren't over the maximum size.
			while( m_nElements > m_nMaxSize )
			{
				delete m_Elements.front().pSFX;
				m_Elements.pop_front();

				--m_nElements;

#ifdef _DEBUG
				g_pLTClient->CPrint("CSpecialFXList::Add(): FAILED to add special fx to list sized: %d", m_nMaxSize);
#endif
			}


			return DTRUE;
		}

		void Remove(Iterator iter)
		{
			_ASSERT( m_nElements == int(m_Elements.size()) );

			delete iter->pSFX;
			m_Elements.erase(iter);
			--m_nElements;
		}	


		void RemoveAll()
		{
			_ASSERT( m_nElements == int(m_Elements.size()) );

			while(!m_Elements.empty() )
			{
				delete m_Elements.back().pSFX;
				m_Elements.pop_back();
			}

			m_nElements = 0;
		}

		Iterator  Begin() { return m_Elements.begin(); }
		const_Iterator Begin() const { return m_Elements.begin(); }

		Iterator End() { return m_Elements.end(); }
		const_Iterator End() const { return m_Elements.end(); }
		 
	private :

		List  m_Elements;		// Array of special fx
		
		int	  m_nElements;		// Number of elements in array  
								// (stlport does not keep this in its std::list so we need to do it.)
		
		int	  m_nMaxSize;		// Maximum allowable size
};

#endif // __SPECIAL_FX_LIST_H__
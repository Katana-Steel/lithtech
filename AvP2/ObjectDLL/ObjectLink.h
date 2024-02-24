// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectLink.h
//
// PURPOSE : Aggregrate which manages inter-object links.
//
// CREATED : 2/14/00
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __OBJECTLINK_H__
#define __OBJECTLINK_H__

#pragma warning( disable : 4786 )
#include <list>

class CObjectLink;



// Notes: 
//   The bank does not _store_ the CObjectLink, it only points to it.  
// Do not feed temporary variables to the bank! Only globals and member variables 
// of the bank's main object can be used.
//
// The CObjectLink constructor uses CObjectLinkBank.  You must
// put a member variable CObjectLinkBank ahead of the any CObjectLink's
// in the member variable list!!!

class CObjectLinkBank : public IAggregate
{
	public :

		typedef std::list<CObjectLink*> LinkList;

	public :

		CObjectLinkBank();
		virtual ~CObjectLinkBank();

		void	Register(CObjectLink & new_link);
		void	Deregister(CObjectLink & old_link);

		HOBJECT GetOwner() const { return m_hOwner; }
	protected :

		DDWORD EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT lData);

	private :


		void InitialUpdate(LPBASECLASS pObject);
		void Update();

		void Save(LMessage * pMsg);
		void Load(LMessage * pMsg);

		HOBJECT		m_hOwner;			// The object I'm associated with
		LinkList	m_pLinks;			// List of objects handled.
};



class CObjectLink
{

public:

	CObjectLink(CObjectLinkBank * new_bank) 
		: m_hObject(DNULL),
		  m_pBank(new_bank)
	{
		if( m_pBank )
			m_pBank->Register(*this);
	}

	CObjectLink(const CObjectLink & rhs)
		: m_hObject(rhs.m_hObject),
		  m_pBank(rhs.m_pBank)
	{
		if( m_hObject && m_pBank && m_pBank->GetOwner())
		{
			g_pInterface->CreateInterObjectLink(m_pBank->GetOwner(),m_hObject);
		}
	}

	~CObjectLink() 
	{
		if( m_pBank)
		{
			m_pBank->Deregister(*this);
		}
	}


	void BreakLink()
	{
		m_hObject = DNULL;
	}

	HOBJECT GetHandle() const { return m_hObject; }
	operator HOBJECT()  const { return GetHandle(); }\

	DBOOL  IsNull()     const { return m_hObject == DNULL; }

	void SetBank(CObjectLinkBank * new_bank)
	{
		if( m_pBank )
			m_pBank->Deregister(*this);

		m_pBank = new_bank;

		if( m_pBank )
			m_pBank->Register(*this);
	}

	void Refresh( HOBJECT old_owner = DNULL)
	{
		// Use this whenever the bank changes owners.
		if( m_hObject )
		{
			if( old_owner )
			{
				g_pInterface->BreakInterObjectLink(old_owner,m_hObject);
			}

			if( m_pBank && m_pBank->GetOwner())
			{
				g_pInterface->CreateInterObjectLink(m_pBank->GetOwner(),m_hObject);
			}
		}
	}

	void Save(LMessage * pMsg) const;
	void Load(LMessage * pMsg);

	
	CObjectLink & operator=(const CObjectLink & rhs )
	{
		if( &rhs != this )
		{
			if( m_hObject && m_pBank && m_pBank->GetOwner())
			{
				g_pInterface->BreakInterObjectLink(m_pBank->GetOwner(),m_hObject);
			}

			m_hObject = rhs.m_hObject;

			if( m_hObject && m_pBank && m_pBank->GetOwner())
			{
				g_pInterface->CreateInterObjectLink(m_pBank->GetOwner(),m_hObject);
			}
		}

		return *this;
	}

	CObjectLink & operator=(HOBJECT rhs )
	{
		if( m_hObject && m_pBank && m_pBank->GetOwner())
		{
			g_pInterface->BreakInterObjectLink(m_pBank->GetOwner(),m_hObject);
		}

		m_hObject = rhs;

		if( m_hObject && m_pBank && m_pBank->GetOwner())
		{
			g_pInterface->CreateInterObjectLink(m_pBank->GetOwner(),m_hObject);
		}

		return *this;
	}


private:

	HOBJECT m_hObject;  // The object I am linked to.
	CObjectLinkBank * m_pBank; // The bank which watches me.
};


inline ILTMessage & operator<<(ILTMessage & in, const CObjectLink & object_link)
{
	object_link.Save(&in);
	return in;
}

inline ILTMessage & operator>>(ILTMessage & out, CObjectLink & object_link)
{
	object_link.Load(&out);
	return out;
}

#endif // __ObjectLink_H__

//*********************************************************************************
//*********************************************************************************
// Project:		Alien vs. Predator 2
// Purpose:		Retrieves attributes from the ObjectiveButes.txt
//*********************************************************************************
// File:		ObjectivesButeMgr.h
// Created:		December 21, 2000
// Updated:		December 21, 2000
// Author:		Andy Kaplan
//*********************************************************************************
//*********************************************************************************

#ifndef		_OBJECTIVE_BUTE_MGR_H_
#define		_OBJECTIVE_BUTE_MGR_H_

//*********************************************************************************

#include "HierarchicalButeMgr.h"
#include "LTString.h"

//*********************************************************************************

#ifdef _WIN32
	#define OBJECTIVE_BUTES_DEFAULT_FILE	"Attributes\\ObjectiveButes.txt"
#else
	#define OBJECTIVE_BUTES_DEFAULT_FILE	"Attributes/ObjectiveButes.txt"
#endif

#define OBJECTIVE_BUTES_STRING_SIZE		64

//*********************************************************************************

#define MAX_OBJECTIVES 10

enum ObjectiveState
{
	OS_OPEN = 0,
	OS_COMPLETE,
	OS_CANCELED,
	OS_REMOVED,
};

struct ObjectiveData
{
	ObjectiveData ()
	{
		nId		= 0;
		eState	= OS_OPEN;
	}
	void	Save(HMESSAGEWRITE hWrite);
	void	Load(HMESSAGEREAD hRead);

	uint32			nId;
	ObjectiveState	eState;
};

inline void ObjectiveData::Save(HMESSAGEWRITE hWrite)
{
	*hWrite << nId;
	hWrite->WriteWord(eState);
}

inline void ObjectiveData::Load(HMESSAGEREAD hRead)
{
	*hRead >> nId;
	eState = ObjectiveState(hRead->ReadWord());
}

//*********************************************************************************

typedef struct t_ObjectiveButes
{
	t_ObjectiveButes::t_ObjectiveButes();

	// Parent information
	char	m_szParent[OBJECTIVE_BUTES_STRING_SIZE];	// The name of the parent of this bute

	// sound file	
	char	m_szName[OBJECTIVE_BUTES_STRING_SIZE];		// Objective name

	int		m_nObjectiveText;							// Displayed text ID

}	ObjectiveButes;

inline t_ObjectiveButes::t_ObjectiveButes()
{
	memset(this, 0, sizeof(t_ObjectiveButes));
}

//*********************************************************************************

class CObjectiveButeMgr : public CHierarchicalButeMgr
{
	public:

		CObjectiveButeMgr();
		virtual ~CObjectiveButeMgr();

		LTBOOL		Init(ILTCSBase *pInterface, const char* szAttributeFile = OBJECTIVE_BUTES_DEFAULT_FILE);
		void		Term();

		LTBOOL		WriteFile()						{ return m_buteMgr.Save(); }
		void		Reload()						{ m_buteMgr.Parse(m_strAttributeFile); }

		int			GetNumObjectiveButes()			const { return m_nNumObjectiveButes; }

		int			GetObjectiveIdFromName(const char *szName)		const;

		char*		GetObjectiveName(int nSet)		{ CheckObjective(nSet); return m_pObjectiveButes[nSet].m_szName; }

		//-------------------------------------------------------------------------------//

		void		CheckObjective(int nSet)		const { ASSERT(nSet < m_nNumObjectiveButes); ASSERT(m_pObjectiveButes); }

		//-------------------------------------------------------------------------------//
		// Special function to fill in an ObjectiveButes structure

		const ObjectiveButes & GetObjectiveButes(int nSet)		const { CheckObjective(nSet); return m_pObjectiveButes[nSet];  }
		const ObjectiveButes & GetObjectiveButes(const char *szName)	const;

	private:

		// Loading functions (called from Init)
		void LoadObjectiveButes(int nSet, ObjectiveButes &butes);

		// Helper functions to get at the values easier
		CString		GetStringAttrib(int nSet, char *szAttrib);
		LTVector	GetVectorAttrib(int nSet, char *szAttrib);
		CPoint		GetPointAttrib(int nSet, char *szAttrib);
		int			GetIntAttrib(int nSet, char *szAttrib);
		double		GetDoubleAttrib(int nSet, char *szAttrib);

		// Objective attribute set variables
		int				m_nNumObjectiveButes;
		ObjectiveButes	*m_pObjectiveButes;
};

////////////////////////////////////////////////////////////////////////////
//
// CObjectiveButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use ObjectiveMgr
//
////////////////////////////////////////////////////////////////////////////

#ifndef _CLIENTBUILD

#include "iobjectplugin.h"

class CObjectiveButeMgrPlugin : public IObjectPlugin
{
	public:

		CObjectiveButeMgrPlugin()	{}

		virtual LTRESULT	PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength);

	protected :

		static LTBOOL				sm_bInitted;
		static CObjectiveButeMgr	sm_ButeMgr;
};

#endif // _CLIENTBUILD



#endif // _OBJECTIVE_BUTE_MGR_H_

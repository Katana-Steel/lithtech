//*********************************************************************************
//*********************************************************************************
// Project:		Alien vs. Predator 2
// Purpose:		Retrieves attributes from the AnimationButes.txt
//*********************************************************************************
// File:		AnimationButeMgr.h
// Created:		August 5, 2000
// Updated:		August 5, 2000
// Author:		Andy Mattingly
//*********************************************************************************
//*********************************************************************************

#ifndef		_ANIMATION_BUTE_MGR_H_
#define		_ANIMATION_BUTE_MGR_H_

//*********************************************************************************

#include "HierarchicalButeMgr.h"

//*********************************************************************************

#ifdef _WIN32
	#define ANIMATION_BUTES_DEFAULT_FILE	"Attributes\\AnimationButes.txt"
#else
	#define ANIMATION_BUTES_DEFAULT_FILE	"Attributes/AnimationButes.txt"
#endif

#define ANIMATION_BUTES_STRING_SIZE		64

//*********************************************************************************

#define ANIMATION_BUTE_MGR_INVALID		-1

//*********************************************************************************

#define ANIMATION_BUTE_MAX_LAYERS		8
#define ANIMATION_BUTE_MAX_TRACKERS		8

//*********************************************************************************

#define ANIMATION_BUTE_BASICTRACKER		0
#define ANIMATION_BUTE_RANDOMTRACKER	1
#define ANIMATION_BUTE_AIMINGTRACKER	2
#define ANIMATION_BUTE_INDEXTRACKER		3

//*********************************************************************************

#define ANIMATION_BUTE_FLAG_LOOPING		0x01
#define ANIMATION_BUTE_FLAG_END			0x02
#define ANIMATION_BUTE_FLAG_BREAK		0x04
#define ANIMATION_BUTE_FLAG_NOINTERP	0x08
#define ANIMATION_BUTE_FLAG_INITINTERP	0x10

//*********************************************************************************

class CAnimationButeMgr;
extern CAnimationButeMgr* g_pAnimationButeMgr;

//*********************************************************************************

typedef struct t_AnimStyleLayer
{
	t_AnimStyleLayer();

	int		nLayerID;
	int		nAnimStyle;
	char	szReference[ANIMATION_BUTES_STRING_SIZE];
	t_AnimStyleLayer *pNext;

}	AnimStyleLayer;

inline t_AnimStyleLayer::t_AnimStyleLayer()
{
	memset(this, 0, sizeof(t_AnimStyleLayer));
}

#ifndef _CLIENTBUILD

ILTMessage & operator<<(ILTMessage & out, t_AnimStyleLayer & layer);
ILTMessage & operator>>(ILTMessage & in, t_AnimStyleLayer & layer);

#endif

//*********************************************************************************

typedef struct t_AnimStyle
{
	t_AnimStyle();

	char	szName[ANIMATION_BUTES_STRING_SIZE];
	uint16	nHashNum;

	char	(*szReference)[ANIMATION_BUTES_STRING_SIZE];
	char	(*szTrackerFrag)[ANIMATION_BUTES_STRING_SIZE];
	char	(*szAltTrackerFrag)[ANIMATION_BUTES_STRING_SIZE];
	int		nNumReference;

}	AnimStyle;

inline t_AnimStyle::t_AnimStyle()
{
	memset(this, 0, sizeof(t_AnimStyle));
}

//*********************************************************************************

typedef struct t_TrackerSet
{
	t_TrackerSet();

	char	szName[ANIMATION_BUTES_STRING_SIZE];
	uint16	nHashNum;

	char	szClearAnim[ANIMATION_BUTES_STRING_SIZE];
	char	szClearWeightSet[ANIMATION_BUTES_STRING_SIZE];

	int		nTrackerTypes[ANIMATION_BUTE_MAX_TRACKERS];
	int		nTrackerData[ANIMATION_BUTE_MAX_TRACKERS];
	LTBOOL	bMovementAllowed;

}	TrackerSet;

inline t_TrackerSet::t_TrackerSet()
{
	memset(this, 0, sizeof(t_TrackerSet));
}

//*********************************************************************************

typedef struct t_BasicTracker
{
	t_BasicTracker();

	char	szName[ANIMATION_BUTES_STRING_SIZE];

	char	szAnim[ANIMATION_BUTES_STRING_SIZE];
	char	szWeightSet[ANIMATION_BUTES_STRING_SIZE];
	float	fAnimSpeed;
	int		nFlags;

}	BasicTracker;

inline t_BasicTracker::t_BasicTracker()
{
	memset(this, 0, sizeof(t_BasicTracker));
}

//*********************************************************************************

typedef struct t_RandomTracker
{
	t_RandomTracker();

	char	szName[ANIMATION_BUTES_STRING_SIZE];

	char	szWeightSet[ANIMATION_BUTES_STRING_SIZE];
	float	fAnimSpeed;
	int		nFlags;

	char	(*szInitAnim)[ANIMATION_BUTES_STRING_SIZE];
	int		nNumInitAnim;

	char	(*szRandomAnim)[ANIMATION_BUTES_STRING_SIZE];
	int		nNumRandomAnim;
	float	fMinDelay;
	float	fMaxDelay;

}	RandomTracker;

inline t_RandomTracker::t_RandomTracker()
{
	memset(this, 0, sizeof(t_RandomTracker));
}

//*********************************************************************************

typedef struct t_AimingTracker
{
	t_AimingTracker();

	char	szName[ANIMATION_BUTES_STRING_SIZE];

	char	szHighAnim[ANIMATION_BUTES_STRING_SIZE];
	char	szLowAnim[ANIMATION_BUTES_STRING_SIZE];
	char	szWeightSet[ANIMATION_BUTES_STRING_SIZE];

	int		nRange;
	LTBOOL	bAscending;
	float	fAnimSpeed;
	int		nFlags;

}	AimingTracker;

inline t_AimingTracker::t_AimingTracker()
{
	memset(this, 0, sizeof(t_AimingTracker));
}

//*********************************************************************************

typedef struct t_IndexTracker
{
	t_IndexTracker();

	char	szName[ANIMATION_BUTES_STRING_SIZE];

	int		*nIndex;
	char	(*szAnim)[ANIMATION_BUTES_STRING_SIZE];
	char	(*szWeightSet)[ANIMATION_BUTES_STRING_SIZE];

	int		nNumIndex;
	float	fAnimSpeed;
	int		nFlags;

}	IndexTracker;

inline t_IndexTracker::t_IndexTracker()
{
	memset(this, 0, sizeof(t_IndexTracker));
}

//*********************************************************************************

typedef struct t_Transition
{
	t_Transition();

	char	szName[ANIMATION_BUTES_STRING_SIZE];

	int		nFromTrackerSet;
	int		nToTrackerSet;
	int		nTransTrackerSet;

}	Transition;

inline t_Transition::t_Transition()
{
	memset(this, 0, sizeof(t_Transition));
}

//*********************************************************************************
//*********************************************************************************

class CAnimationButeMgr : public CHierarchicalButeMgr
{
	public:

		CAnimationButeMgr();
		virtual ~CAnimationButeMgr();

		LTBOOL		Init(ILTCSBase *pInterface, const char* szAttributeFile = ANIMATION_BUTES_DEFAULT_FILE);
		void		Term();

		LTBOOL		WriteFile()						{ return m_buteMgr.Save(); }
		void		Reload()						{ m_buteMgr.Parse(m_strAttributeFile); }

		//-------------------------------------------------------------------------------//

		int			GetAnimStyleIndex(const char *szName);
		int			GetTrackerSetIndex(const char *szName);

		int			GetTrackerSetFromLayers(AnimStyleLayer *pLayer);

		int			GetTransitionSet(int nFrom, int nTo);

		//-------------------------------------------------------------------------------//

		AnimStyle*		GetAnimStyle(uint16 nIndex)
			{ return ((nIndex < m_nNumAnimStyles) ? &m_pAnimStyles[nIndex] : LTNULL); }

		TrackerSet*		GetTrackerSet(uint16 nIndex)
			{ return ((nIndex < m_nNumTrackerSets) ? &m_pTrackerSets[nIndex] : LTNULL); }

		BasicTracker*	GetBasicTracker(uint16 nIndex)
			{ return ((nIndex < m_nNumBasicTrackers) ? &m_pBasicTrackers[nIndex] : LTNULL); }

		RandomTracker*	GetRandomTracker(uint16 nIndex)
			{ return ((nIndex < m_nNumRandomTrackers) ? &m_pRandomTrackers[nIndex] : LTNULL); }

		AimingTracker*	GetAimingTracker(uint16 nIndex)
			{ return ((nIndex < m_nNumAimingTrackers) ? &m_pAimingTrackers[nIndex] : LTNULL); }

		IndexTracker*	GetIndexTracker(uint16 nIndex)
			{ return ((nIndex < m_nNumIndexTrackers) ? &m_pIndexTrackers[nIndex] : LTNULL); }

		Transition*		GetTransition(uint16 nIndex)
			{ return ((nIndex < m_nNumTransitions) ? &m_pTransitions[nIndex] : LTNULL); }


	private:

		static bool CountTags(const char *szTagName, void *pData);
		static bool LoadButes(const char *szTagName, void *pData);

		//-------------------------------------------------------------------------------//

		uint16		CreateHashValue(const char *szStr);

		//-------------------------------------------------------------------------------//

		void		LoadAnimStyle(const char *szTagName);
		void		LoadTrackerSet(const char *szTagName);
		void		LoadBasicTracker(const char *szTagName);
		void		LoadRandomTracker(const char *szTagName);
		void		LoadAimingTracker(const char *szTagName);
		void		LoadIndexTracker(const char *szTagName);
		void		LoadTransition(const char *szTagName);

	private:

		// Animation attribute variables
		int			m_nNumAnimStyles;
		int			m_nNumTrackerSets;
		int			m_nNumBasicTrackers;
		int			m_nNumRandomTrackers;
		int			m_nNumAimingTrackers;
		int			m_nNumIndexTrackers;
		int			m_nNumTransitions;

		AnimStyle		*m_pAnimStyles;
		TrackerSet		*m_pTrackerSets;
		BasicTracker	*m_pBasicTrackers;
		RandomTracker	*m_pRandomTrackers;
		AimingTracker	*m_pAimingTrackers;
		IndexTracker	*m_pIndexTrackers;
		Transition		*m_pTransitions;

		IndexTable		 m_TrackerSetTable;		
		IndexTable		 m_AnimStyleTable;		
};


#endif // _ANIMATION_BUTE_MGR_H_


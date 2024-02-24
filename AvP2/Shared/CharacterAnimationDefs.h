// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterAnimationDefs.h
//
// PURPOSE : General character animation defines
//
// CREATED : 8/5/00
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_ANIMATION_DEFS_H__
#define __CHARACTER_ANIMATION_DEFS_H__

// ----------------------------------------------------------------------- //

enum
{
	ANIM_TRACKER_ALL = -1,
	ANIM_TRACKER_AIMING_UP = 0,
	ANIM_TRACKER_AIMING_DOWN,
	ANIM_TRACKER_EXPRESSION,
	ANIM_TRACKER_MOVEMENT,
	ANIM_TRACKER_RECOIL,
	ANIM_TRACKER_MAX,
};

// ----------------------------------------------------------------------- //

#define ANIMSTYLELAYER_STYLE			0
#define ANIMSTYLELAYER_WEAPON			1
#define ANIMSTYLELAYER_ACTION			2
#define ANIMSTYLELAYER_EXT_ACTION		3

// ----------------------------------------------------------------------- //

#define EXPRESSION_MIN					0
#define EXPRESSION_NORMAL				0
#define EXPRESSION_BLINK				1
#define EXPRESSION_SCARED				2
#define EXPRESSION_ANGRY				3
#define EXPRESSION_CURIOUS				4
#define EXPRESSION_PAIN					5
#define EXPRESSION_MAX					5

// ----------------------------------------------------------------------- //

typedef struct t_TrackerInfo
{
	t_TrackerInfo::t_TrackerInfo();


	LTAnimTracker	*m_pTracker;		// Pointer to the Lithtech animation tracker
	LTAnimTracker	m_Tracker;			// The actual tracker if we needed to create


	// Specific tracker type data that should be modified to achieve desired results
	LTFLOAT			m_fAimingRange;		// The current range to use if this an aiming tracker (Pos 1.0 -> Neg 1.0)
	int				m_nIndex;			// The current index to use if this is an indexed tracker


	// ----------------------------------------------------------------------- //
	// Current internal attribute tracker information
	int				m_nTrackerType;		// The type of tracker this info represents
	int				m_nTrackerData;		// The index of tracker data this info represents

	char			m_szAnim[64];		// The current animation being played
	HMODELANIM		m_hAnim;			// The current animation being played
	HMODELWEIGHTSET	m_hWeightSet;		// The current weight set being used

	char			m_szDestAnim[64];	// The destination animation (for waiting and trasitions)
	HMODELANIM		m_hDestAnim;		// The destination animation (for waiting and trasitions)
	HMODELWEIGHTSET	m_hDestWeightSet;	// The destination weight set (for waiting and trasitions)


	// Internal tracker update information
	LTFLOAT			m_fLastAimingRange;	// The previous aiming range used in an update
	int				m_nLastIndex;		// The previous index used in an update

	LTFLOAT			m_fRandTime;		// The start time to keep track of playing random animation trackers
	LTFLOAT			m_fRandDelay;		// The random delay time for random animation trackers

	uint16			m_nState;			// General state information about the tracker update

}	TrackerInfo;

// ----------------------------------------------------------------------- //

inline t_TrackerInfo::t_TrackerInfo()
{
	memset(this, 0, sizeof(t_TrackerInfo));

	m_nTrackerType = -1;
	m_nTrackerData = -1;

	m_hAnim = INVALID_MODEL_ANIM;
	m_hWeightSet = INVALID_MODEL_WEIGHTSET;

	m_hDestAnim = INVALID_MODEL_ANIM;
	m_hDestWeightSet = INVALID_MODEL_ANIM;
}

#ifndef _CLIENTBUILD

ILTMessage & operator<<(ILTMessage & out, t_TrackerInfo & tracker_info);
ILTMessage & operator>>(ILTMessage & in, t_TrackerInfo & tracker_info);

#endif

// ----------------------------------------------------------------------- //

#endif //__CHARACTER_ANIMATION_DEFS_H__


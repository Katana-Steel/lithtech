// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterAnimation.h
//
// PURPOSE : General character animation class
//
// CREATED : 8/5/00
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_ANIMATION_H__
#define __CHARACTER_ANIMATION_H__

// ----------------------------------------------------------------------- //

#include "AnimationButeMgr.h"
#include "CharacterAnimationDefs.h"

// ----------------------------------------------------------------------- //
// Animation messages are written in this form
//
// uint8 = Tracker Index
// uint8 = Looping (TRUE / FALSE)
// uint8 = Msg Info (the defines below)
//
//
// NOTE:	DO NOT call anything that will switch the animation during
//			one of these messages. These messages get sent from inside a
//			string key handling function... and changing the animation at
//			that point is bad!


#define ANIMMSG_ANIM_BREAK				100
#define ANIMMSG_ALT_ANIM_BREAK			110
#define ANIMMSG_TRANS_ANIM_BREAK		120

// ----------------------------------------------------------------------- //

#ifdef _CLIENTBUILD
	#include "clientheaders.h"
#else
	#include "serverheaders.h"
#endif

// ----------------------------------------------------------------------- //

#ifdef _CLIENTBUILD
	#define INTERFACE	CClientDE
#else
	#define INTERFACE	CServerDE
#endif

// ----------------------------------------------------------------------- //

class CharacterAnimation
{
	public:

		// Constructor and destructors for our movement
		CharacterAnimation();
		virtual ~CharacterAnimation();


		// Initialization and destruction functions
		virtual LTBOOL	Init(INTERFACE *pInterface, HOBJECT hObj, uint32 nMessageID, uint8 nValidTrackers = 0xFF);
		virtual void	Term();

		virtual void	Enable(LTBOOL bEnable)					{ m_bEnabled = bEnable; }
		virtual LTBOOL	IsEnabled()								{ return m_bEnabled; }


#ifndef _CLIENTBUILD
		// Loading and saving of the character movement class
		virtual void	Save(HMESSAGEWRITE hWrite);
		virtual void	Load(HMESSAGEREAD hRead);
#endif


		// Update the current state
		virtual void	Update();

		// Handle string keys
		virtual void	HandleStringKey(ArgList* pArgList, uint8 nTracker);


		// ----------------------------------------------------------------------- //
		// Alternate animation helper functions

		LTBOOL	PlayAnim(const char *szAnim, LTBOOL bLoop = LTFALSE, LTBOOL bRestartAnim = LTFALSE);	// Plays a full body animation
		LTBOOL	PlayAnim(HMODELANIM hAnim, LTBOOL bLoop = LTFALSE, LTBOOL bRestartAnim = LTFALSE);		// Plays a full body animation		
		LTBOOL	PlayTrackerSet(const char *szTrackerSet);				// Plays a tracker set from the butes
		void	StopAnim(LTBOOL bUpdate = LTTRUE);						// Stops the override anim or tracker set

		LTBOOL	UsingAltAnim()									{ return m_bUsingAltAnim; }
		LTBOOL	UsingTransAnim()								{ return m_bUsingTransAnim; }

		LTBOOL	MovementAllowed(LTBOOL bLastTracker = LTTRUE);			// Tell if this tracker set recommends movement

		// ----------------------------------------------------------------------- //
		// Data modification functions

		void	SetObject(HOBJECT hObject)						{ m_hObject = hObject; }
		void	SetBaseSpeed(LTFLOAT fSpeed);

		// ----------------------------------------------------------------------- //
		// Animation layer control

		void	AddLayer(const char *szStyle, int nID);					// Add a new animation layer with a unique ID
		void	ClearLayers();											// Clear out the animation layers

		LTBOOL		SetLayerReference(int nID, const char *szRef);		// Sets the reference for an animation layer
		const char*	GetLayerReference(int nID);							// Retrieves a reference of a layer

		// ----------------------------------------------------------------------- //
		// Data retrieval functions

		HOBJECT			GetObject()								{ return m_hObject; }
		LTFLOAT			GetBaseSpeed()							{ return m_fBaseSpeed;}
		int				GetTrackerSet()							{ return m_nTrackerSet; }

		LTBOOL			IsValidTracker(int nTInfo) const;
		TrackerInfo*	GetTrackerInfo(int nTInfo);


		uint32			GetAnimLength(int nTInfo);
		uint32			GetAnimTime(int nTInfo);

		HMODELANIM		GetAltAnim() const { return m_bUsingAltAnim ? m_nAltAnim : INVALID_MODEL_ANIM; }

		LTBOOL			HasValidAnims();

	private:

		// ----------------------------------------------------------------------- //
		// Tracker helper functions

		void	SetupTrackerSet(int nTrackerSet, LTBOOL bAllowInitialInterp);

		void	SetupBasicTracker(int nInfo, int nData, LTBOOL bAllowInitialInterp);
		void	SetupRandomTracker(int nInfo, int nData, LTBOOL bAllowInitialInterp);
		void	SetupAimingTracker(int nInfo, int nData, LTBOOL bAllowInitialInterp);
		void	SetupIndexTracker(int nInfo, int nData, LTBOOL bAllowInitialInterp);

		void	UpdateTrackerSet(int nTrackerSet);

		void	UpdateBasicTracker(int nInfo, int nData);
		void	UpdateRandomTracker(int nInfo, int nData);
		void	UpdateAimingTracker(int nInfo, int nData);
		void	UpdateIndexTracker(int nInfo, int nData);

		void	ClearTracker(int nInfo, int nTrackerSet);


		// ----------------------------------------------------------------------- //
		// Misc Helper functions

		HMODELANIM		GetAnim(const char *szName, HMODELANIM hDefault = INVALID_MODEL_ANIM);
		HMODELWEIGHTSET	GetWeightSet(const char *szName, HMODELWEIGHTSET hDefault = INVALID_MODEL_WEIGHTSET);

		LTBOOL	SendAnimationMessage(HOBJECT hObject, uint8 nTracker, LTBOOL bLooping, uint8 nInfo);

		LTBOOL	PlayTransTrackerSet(int nFromSet, int nToSet);


	private:

		// ----------------------------------------------------------------------- //
		// Member variables

		// The main interfaces to the engine functionality
		INTERFACE		*m_pInterface;
		ILTModel		*m_pModel;

		HOBJECT			m_hObject;								// The handle to the character object to animate
		uint32			m_nMessageID;							// The message ID used to inform the object of changes

		LTBOOL			m_bEnabled;								// Is the animation class enabled?


		// Main data for the animation class
		AnimStyleLayer	*m_pLayers;								// The stack of anim style layers
		TrackerInfo		m_pTrackers[ANIM_TRACKER_MAX];			// A list of tracker info information

		LTFLOAT			m_fBaseSpeed;							// This value is used to calculate the anim scale

		int				m_nLastTrackerSet;						// The previous tracker set
		int				m_nTrackerSet;							// The current tracker set being used

		LTBOOL			m_bWaiting;								// Are we waiting to switch tracker sets?


		// Alternate animation variables
		HMODELANIM		m_nAltAnim;								// The full body override animation
		int				m_nAltTrackerSet;						// The override tracker set
		LTBOOL			m_bUsingAltAnim;						// Tells whether we're using an override

		int				m_nTransTrackerSet;						// The full body transition animation
		LTBOOL			m_bUsingTransAnim;						// Tells whether we're using a transition

		LTBOOL			m_bFirstSetup;							// Set to false after the first call to set-up.
};



// ================ INLINES =======================


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::IsValidTracker()
//
// PURPOSE:		Returns true if the index is valid and the tracker has been set-up.
//
// ----------------------------------------------------------------------- //

inline LTBOOL CharacterAnimation::IsValidTracker(int nTInfo) const
{
	return    nTInfo >= 0
		   && nTInfo < ANIM_TRACKER_MAX
		   && nTInfo < ANIMATION_BUTE_MAX_TRACKERS
		   && m_pTrackers[nTInfo].m_pTracker
		   && m_pTrackers[nTInfo].m_nTrackerType != ANIMATION_BUTE_MGR_INVALID
		   && m_pTrackers[nTInfo].m_nTrackerData != ANIMATION_BUTE_MGR_INVALID;
}


#endif //__CHARACTER_ANIMATION_H__


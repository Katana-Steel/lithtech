//*********************************************************************************
//*********************************************************************************
// Project:		Aliens vs. Predator 2
// Purpose:		Retrieves attributes from the CharacterButes.txt
//*********************************************************************************
// File:		CharacterButeMgr.h
// Created:		Jan. 24, 2000
// Updated:		Jan. 24, 2000
// Author:		Andy Mattingly
//*********************************************************************************
//*********************************************************************************

#ifndef		_CHARACTER_BUTE_MGR_H_
#define		_CHARACTER_BUTE_MGR_H_

//*********************************************************************************

#include "HierarchicalButeMgr.h"
#include "CharacterMovementDefs.h"
#include "CharacterAlignment.h"

//*********************************************************************************

#ifdef _WIN32
	#define CHARACTER_BUTES_DEFAULT_FILE	"Attributes\\CharacterButes.txt"
#else
	#define CHARACTER_BUTES_DEFAULT_FILE	"Attributes/CharacterButes.txt"
#endif

#define CHARACTER_BUTES_STRING_SIZE		64

#define MAX_ZOOM_LEVELS					4
#define MAX_ANIM_LAYERS					4

//*********************************************************************************

class CCharacterButeMgr;
extern CCharacterButeMgr* g_pCharacterButeMgr;

//*********************************************************************************

typedef struct t_CharacterButes
{
	t_CharacterButes::t_CharacterButes();

	// Loading and saving the character bute information
	void	Write(HMESSAGEWRITE hWrite);
	void	Read(HMESSAGEREAD hRead);

	void	Copy(const t_CharacterButes &cb);
	void	operator=(const t_CharacterButes &cb)			{ this->Copy(cb); }


	// Object creates structure helper function
	void	FillInObjectCreateStruct(ObjectCreateStruct &ocs, LTBOOL bUseMPDir = LTTRUE) const;


	// Class information
	CharacterClass m_eCharacterClass;                   // The class of this character.
	uint32	m_nId;										// The bute set ID number


	// Parent information
	char	m_szParent[CHARACTER_BUTES_STRING_SIZE];	// The name of the parent of this character


	// Directory information
	char	m_szModelDir[CHARACTER_BUTES_STRING_SIZE];	// The directory the model is located in
	char	m_szSkinDir[CHARACTER_BUTES_STRING_SIZE];	// The directory the skins are located in


	// Model and skin information
	char	m_szName[CHARACTER_BUTES_STRING_SIZE];							// The name of this model
	char	m_szModel[CHARACTER_BUTES_STRING_SIZE];							// Model filename
	char	m_szSkins[MAX_MODEL_TEXTURES][CHARACTER_BUTES_STRING_SIZE];		// Skin filenames
	char	m_szWeaponSkinType[CHARACTER_BUTES_STRING_SIZE];				// Skin appendage for weapon models


	// Multiplayer specific information
	char	m_szMultiModel[CHARACTER_BUTES_STRING_SIZE];		// Model filename for multiplayer
	uint32	m_nDisplayName;										// Used to display a localizable name on the screen
	uint32	m_nScoringClass;									// Used for multiplayer scoring system

	char		m_szSelectAnim[CHARACTER_BUTES_STRING_SIZE];	// Anim to play for the player selection display
	LTVector	m_vSelectPos;									// Offset position for the player selection display
	LTFLOAT		m_fSelectRot;									// A rotation around the up axis of the player selection display
	LTFLOAT		m_fSelectScale;									// A scale for this model in the player selection display


	// Animation information
	char	m_szAnimLayers[MAX_ANIM_LAYERS][CHARACTER_BUTES_STRING_SIZE];	// The animation layers to use for this character
	char	m_szMPAnimLayers[MAX_ANIM_LAYERS][CHARACTER_BUTES_STRING_SIZE];	// The animation layers to use for this character
	LTBOOL	m_bCanBeNetted;										// Can this character be netted wht the Pred Net Gun?
	LTBOOL	m_bCanBeOnFire;										// Can this character be on flames?
	LTBOOL	m_bCanBeFacehugged;									// Can this character be facehugged?
	LTBOOL	m_bCanUseLookAt;									// Can this character's head be rotated?

	LTFLOAT	m_fBaseFootStepVolume;								// Base multiplier for character's footstep sounds (between 0.0 and 1.0), used by AI's HearFootStep sense only.

	char	m_szFootStepSoundDir[CHARACTER_BUTES_STRING_SIZE];	// The default sound name base for foot steps
	char	m_szGeneralSoundDir[CHARACTER_BUTES_STRING_SIZE];	// The default sound name base for general sounds
	char	m_szAIAnimType[CHARACTER_BUTES_STRING_SIZE];		// The AI animation table to use in AIAnimations.txt.

	LTVector			m_vDims;						// The default dimensions for this model
	LTFLOAT				m_fMass;						// The mass of the character model
	LTFLOAT				m_fCameraHeightPercent;			// The percent of the dims to offset the camera height
	uint8				m_nSurfaceType;					// The surface type index (in Surface.txt)
	uint32				m_nSkeleton;					// The index of the skeleton structure to use (in ModelButes.txt)
	uint32				m_nWeaponSet;					// What type of weapons can this character use
	uint32				m_nMPWeaponSet;					// What type of weapons can this character use in Multiplayer
	uint32				m_nMPClassWeaponSet;			// What type of weapons can this character use in Class Based Multiplayer
	uint32				m_nMPWeaponStringRes;			// The string resource ID for the weapon description for this character
	char	m_szVisionSet[CHARACTER_BUTES_STRING_SIZE];	// What type of vision modes are available to this character
	uint32				m_nHUDType;						// What kind of HUD
	LTVector			m_vPouncingDims;				// The pouncing dims.

	// Aiming offsets (for AI's aiming at character).
	LTFLOAT				m_fChestOffsetPercent;          // The chest is at this percent of the dims.
	LTFLOAT				m_fHeadOffsetPercent;           // The head is at this percent of the dims.

	// Vision values.
	CPoint				m_ptFOV;						// Character's field of view for this character
	CPoint				m_ptAimRange;					// Character's max angles for aiming (low angle, high angle)
	LTFLOAT				m_fHeadTiltAngle;				// Character head tilt range
	LTFLOAT				m_fHeadTiltSpeed;				// Character head tilt speed

	// Maximum speed controls for different movement types
	LTFLOAT				m_fBaseWalkSpeed;				// Maximum speed when walking
	LTFLOAT				m_fBaseRunSpeed;				// Maximum speed when running
	LTFLOAT				m_fBaseJumpSpeed;				// Speed to take off from a surface normally
	LTFLOAT				m_fBaseRunJumpSpeed;			// Speed to take off from a surface when running
	LTFLOAT				m_fBaseSpringJumpSpeed;			// Speed to take off from a surface when ducking and stopped
	LTFLOAT				m_fBasePounceSpeed;				// Speed to take off for a pounce
	LTFLOAT				m_fBaseCrouchSpeed;				// Maximum speed when ducked or crawling
	LTFLOAT				m_fBaseLiquidSpeed;				// Maximum speed when submerged within a liquid volume brush (swimming)
	LTFLOAT				m_fBaseLadderSpeed;				// Maximum speed when climbing a ladder

	LTFLOAT				m_fFallDistanceSafe;			// The distance this character can fall without damage
	LTFLOAT				m_fFallDistanceDeath;			// The distance this character will fall and die, even with max health

	LTFLOAT				m_fWalkingTurnRate;				// Turning rate in degrees per second while walking.
	LTFLOAT				m_fRunningTurnRate;				// Turning rate in degrees per second while running.

	LTFLOAT				m_fWalkingAimRate;				// Aiming rate in degrees per second while walking.
	LTFLOAT				m_fRunningAimRate;				// Aiming rate in degrees per second while running.

	LTFLOAT				m_fWalkingKickBack;				// Aiming rate in degrees per second while walking.
	LTFLOAT				m_fRunningKickBack;				// Aiming rate in degrees per second while running.
	// Base acceleration values for different movement states
	LTFLOAT				m_fBaseGroundAccel;				// Acceleration used when moving against a surface
	LTFLOAT				m_fBaseAirAccel;				// Acceleration for mid-air control
	LTFLOAT				m_fBaseLiquidAccel;				// Acceleration when submerged within a liquid volume brush (swimming)

	LTFLOAT				m_fMaxAIPopDist;				// If AI's are asked to move this distance or less, they will pop to the destination.

	// Special directional variables
	LTVector			m_vJumpDirection;				// Jumping direction based off character's facing (right, up, forward)

	// Special state variables
	LTBOOL				m_bWallWalk;					// Should we replace the ducking state with wall walking?

	// Miscellaneous attributes
	LTFLOAT				m_fActivateDist;				// Distance this character can activate things from
	uint32				m_nEMPFlags;					// EMP Effect Flags
	LTFLOAT				m_fBaseStunChance;				// The base percentage chance a character will be effected by a stun attack
	LTFLOAT				m_fZoomLevels[MAX_ZOOM_LEVELS];	// The levels to adjust the zoom to when zooming in or out.
	LTBOOL				m_bCanClimb;					// Can this character climb?
	LTBOOL				m_bCanLoseLimb;					// Can this character loselimbs?

	// Player only pounce data
	LTBOOL	m_bCanPounce;									// Can this character pounce?
	char	m_szPounceWeapon[CHARACTER_BUTES_STRING_SIZE];	// The name of the pounce weapon
	uint32	m_nPounceDamage;								// Pounce damage

	// Statistic attributes
	LTFLOAT				m_fDefaultHitPoints;			// The number of hit points the character starts with when spawned
	LTFLOAT				m_fMaxHitPoints;				// The maximum number of hit points that the character can reach
	LTFLOAT				m_fDefaultArmorPoints;			// The number of armor points the character starts with when spawned
	LTFLOAT				m_fMaxArmorPoints;				// The maximum number of armor points that the character can reach
	LTFLOAT				m_fDamageResistance;			// The amount of damage resistance this character has for damage types that use it

	// Special ability attributes
	LTFLOAT				m_fHeadBiteHealPoints;			// The amount of heal given to the alien when this character type receives a head bite attack. 
	LTFLOAT				m_fClawHealPoints;				// The amount of heal given to the alien when a dead version of this character gets clawed 

	// Air supply variables
	LTFLOAT				m_fAirSupplyTime;				// The amount of time this character can survive without air

	// Recoiling
	LTFLOAT				m_fMinRecoilDamage;			// Only recoil if they take at least this much damage.
	LTFLOAT				m_fRecoilChance;				// Chance that character will recoild if he takes damage.
	LTFLOAT				m_fMinRecoilDelay;				// Wait between min and max before trying to recoil again.
	LTFLOAT				m_fMaxRecoilDelay;				//    same as above.

	// Hit box
	LTFLOAT				m_fHitBoxScale;				// Scale the x-z dims of the hit box by this amount.

}	CharacterButes;

inline t_CharacterButes::t_CharacterButes()
{
	memset(this, 0, sizeof(t_CharacterButes));
}

inline ILTMessage & operator<<(ILTMessage & out, /*const*/ t_CharacterButes & x)
{
	x.Write(&out);
	return out;
}


inline ILTMessage & operator>>(ILTMessage & in, t_CharacterButes & x)
{
	x.Read(&in);
	return in;
}

//*********************************************************************************

class CCharacterButeMgr : public CHierarchicalButeMgr
{
	public:
		CCharacterButeMgr();
		virtual ~CCharacterButeMgr();

		LTBOOL		Init(ILTCSBase *pInterface, const char* szAttributeFile = CHARACTER_BUTES_DEFAULT_FILE);
		void		Term();

		LTBOOL		WriteFile()						{ return m_buteMgr.Save(); }
		void		Reload(ILTCSBase *pInterface)	{ Term(); Init(pInterface); }

		int			GetNumSets()					const { return m_nNumSets; }
		int			GetSetFromModelType(const char *szName)	const;

		//-------------------------------------------------------------------------------//
		uint32			GetCharacterId(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_nId; }

		CharacterClass	GetClass(int nSet)				const { Check(nSet); return m_pButeSets[nSet].m_eCharacterClass; }
		char const*		GetParentType(int nSet)			const { Check(nSet); return m_pButeSets[nSet].m_szParent; }

		char const*		GetName(int nSet)				const { Check(nSet); return m_pButeSets[nSet].m_szName; }
		char const*		GetModelType(int nSet)			const { Check(nSet); return m_pButeSets[nSet].m_szName; }

		char const*		GetDefaultModelDir(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_szModelDir; }
		char const*		GetDefaultSkinDir(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_szSkinDir; }

		char const*		GetFootStepSoundDir(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_szFootStepSoundDir; }
		char const*		GetGeneralSoundDir(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_szGeneralSoundDir; }
		char const*		GetAIAnimType(int nSet)			const { Check(nSet); return m_pButeSets[nSet].m_szAIAnimType; }

		LTBOOL			GetCanLoseLimb(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_bCanLoseLimb; }
		LTBOOL			GetCanClimb(int nSet)			const { Check(nSet); return m_pButeSets[nSet].m_bCanClimb; }
		LTBOOL			GetCanPounce(int nSet)			const { Check(nSet); return m_pButeSets[nSet].m_bCanPounce; }
		char const*		GetPounceWeapon(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_szPounceWeapon; }
		uint32			GetPounceDamage(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_nPounceDamage; }

		CString			GetDefaultModel(int nSet, const char *szDir = LTNULL, LTBOOL bForceSingle = LTFALSE) const;
		CString			GetDefaultSkin(int nSet, int nSkin, const char *szDir = LTNULL) const;
		void			GetDefaultFilenames(int nSet, ObjectCreateStruct &pStruct, const char *szModelDir = LTNULL, const char *szSkinDir = LTNULL) const;
		CString			GetWeaponSkinType(int nSet) const;


		CString			GetMultiplayerModel(int nSet, const char *szDir = LTNULL) const;
		uint32			GetDisplayName(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_nDisplayName; }
		uint32			GetScoringClass(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_nScoringClass; }

		char const*		GetSelectAnim(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_szSelectAnim; }
		LTVector		GetSelectPos(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_vSelectPos; }
		LTFLOAT			GetSelectRot(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fSelectRot; }
		LTFLOAT			GetSelectScale(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_fSelectScale; }


		LTVector	GetDefaultDims(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_vDims; }
		LTFLOAT		GetDefaultMass(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fMass; }
		uint32		GetSkeletonIndex(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_nSkeleton; }
		uint32		GetWeaponSet(int nSet)			const { Check(nSet); return m_pButeSets[nSet].m_nWeaponSet; }
		uint32		GetMPWeaponSet(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_nMPWeaponSet; }
		uint32		GetMPClassWeaponSet(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_nMPClassWeaponSet; }
		uint32		GetMPWeaponStringRes(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_nMPWeaponStringRes; }
		char const*	GetVisionSet(int nSet)			const { Check(nSet); return m_pButeSets[nSet].m_szVisionSet; }
		uint32		GetHUDType(int nSet)			const { Check(nSet); return m_pButeSets[nSet].m_nHUDType; }
		LTVector	GetPouncingDims(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_vPouncingDims; }

		CPoint		GetFOV(int nSet)				const { Check(nSet); return m_pButeSets[nSet].m_ptFOV; }
		CPoint		GetAimRange(int nSet)			const { Check(nSet); return m_pButeSets[nSet].m_ptAimRange; }
		float		GetHeadTiltAngle(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fHeadTiltAngle; }
		float		GetHeadTiltSpeed(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fHeadTiltSpeed; }

		float		GetWalkSpeed(int nSet)			const { Check(nSet); return m_pButeSets[nSet].m_fBaseWalkSpeed; }
		float		GetRunSpeed(int nSet)			const { Check(nSet); return m_pButeSets[nSet].m_fBaseRunSpeed; }
		float		GetNormJumpSpeed(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fBaseJumpSpeed; }
		float		GetRunJumpSpeed(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fBaseRunJumpSpeed; }
		float		GetSpringJumpSpeed(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_fBaseSpringJumpSpeed; }
		float		GetPounceSpeed(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fBasePounceSpeed; }
		float		GetCrouchSpeed(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fBaseCrouchSpeed; }
		float		GetLiquidSpeed(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fBaseLiquidSpeed; }
		float		GetLadderSpeed(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fBaseLadderSpeed; }

		float		GetFallDistanceSafe(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_fFallDistanceSafe; }
		float		GetFallDistanceDeath(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_fFallDistanceDeath; }

		float		GetWalkingTurnRate(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_fWalkingTurnRate; }
		float		GetRunningTurnRate(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_fRunningTurnRate; }

		float		GetWalkingAimRate(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fWalkingAimRate; }
		float		GetRunningAimRate(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fRunningAimRate; }

		float		GetWalkingKickBack(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_fWalkingKickBack; }
		float		GetRunningKickBack(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_fRunningKickBack; }

		float		GetBaseStunChance(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fBaseStunChance; }

		float		GetGroundAccel(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fBaseGroundAccel; }
		float		GetAirAccel(int nSet)			const { Check(nSet); return m_pButeSets[nSet].m_fBaseAirAccel; }
		float		GetLiquidAccel(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fBaseLiquidAccel; }
		float		GetAIPopDist(int nSet)			const { Check(nSet); return m_pButeSets[nSet].m_fMaxAIPopDist; }

		LTVector	GetJumpDirection(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_vJumpDirection; }

		LTBOOL		GetCanWallWalk(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_bWallWalk; }
		LTBOOL		GetCanBeNetted(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_bCanBeNetted; }
		LTBOOL		GetCanBeOnFire(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_bCanBeOnFire; }
		LTBOOL		GetCanBeFacehugged(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_bCanBeFacehugged; }
		LTBOOL		GetCanBeUseLookAt(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_bCanUseLookAt; }

		float		GetActivateDistance(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_fActivateDist; }
		uint32		GetEMPFlags(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_nEMPFlags; }

		float		GetDefaultHitPoints(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_fDefaultHitPoints; }
		float		GetMaxHitPoints(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fMaxHitPoints; }
		float		GetDefaultArmorPoints(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_fDefaultArmorPoints; }
		float		GetMaxArmorPoints(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fMaxArmorPoints; }
		float		GetDamageResistance(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_fDamageResistance; }

		float		GetHeadBiteHealPoints(int nSet)	const { Check(nSet); return m_pButeSets[nSet].m_fHeadBiteHealPoints; }
		float		GetClawHealPoints(int nSet)		const { Check(nSet); return m_pButeSets[nSet].m_fClawHealPoints; }
		float		GetZoomLevel(int nSet, int level)const { Check(nSet); return level>=MAX_ZOOM_LEVELS?1:m_pButeSets[nSet].m_fZoomLevels[level]; }

		//-------------------------------------------------------------------------------//

		void		Check(int nSet)					const { ASSERT(nSet < m_nNumSets); ASSERT(m_pButeSets); }

		//-------------------------------------------------------------------------------//

		// Special function to fill in a CharacterButes structure
		const CharacterButes & GetCharacterButes(int nSet)		const { Check(nSet); return m_pButeSets[nSet];  }
		const CharacterButes & GetCharacterButes(const char *szName)	const;

	private:
		// Loading functions (called from Init)
		static bool CountTags(const char *szTagName, void *pData);
		static bool LoadButes(const char *szTagName, void *pData);

		void LoadCharacterButes(const char * szTagName, CharacterButes &butes);


	private:
		// Character attribute set variables
		int				m_nNumSets;
		CharacterButes	*m_pButeSets;
};

////////////////////////////////////////////////////////////////////////////
//
// CCharacterButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use WeaponMgr
//
////////////////////////////////////////////////////////////////////////////

#ifndef _CLIENTBUILD

#include "iobjectplugin.h"

class CCharacterButeMgrPlugin : public IObjectPlugin
{
	public:

		CCharacterButeMgrPlugin()	{ m_bPlayerOnly = LTFALSE; m_bAIOnly = LTFALSE; }

		virtual LTRESULT	PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength);

		void SetPlayerOnly() { m_bPlayerOnly = LTTRUE; m_bAIOnly = LTFALSE; }
		void SetAIOnly()	 { m_bPlayerOnly = LTFALSE; m_bAIOnly = LTTRUE; }
		void SetAny()		 { m_bPlayerOnly = LTFALSE; m_bAIOnly = LTFALSE; }

	protected :

		LTBOOL				m_bPlayerOnly;
		LTBOOL				m_bAIOnly;

		static LTBOOL				sm_bInitted;
		static CCharacterButeMgr	sm_ButeMgr;
};

#endif // _CLIENTBUILD


#endif // _CHARACTER_BUTE_MGR_H_

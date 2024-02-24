// ----------------------------------------------------------------------- //
//
// MODULE  : BodyProp.h
//
// PURPOSE : Model BodyProp - Definition
//
// CREATED : 6/2/98
//
// ----------------------------------------------------------------------- //

#ifndef __BODY_PROP_H__
#define __BODY_PROP_H__

// ----------------------------------------------------------------------- //

#include "Prop.h"
#include "Character.h"
#include "AIVolume.h"

#include <string>

// ----------------------------------------------------------------------- //
// These are the various ways that characters can die

enum DeathType
{
	CD_NORMAL = 0,
	CD_GIB,
	CD_FREEZE,
	CD_BURN,
	CD_CHESTBURST,
	CD_HEADBITE,
	CD_SPEAR,
	CD_SLICE,
	CD_FACEHUG,
	CD_EXPLODE,
	CD_SHOTGUN,
	CD_BULLET_NOGIB
};

// ----------------------------------------------------------------------- //

class CAttachments;

struct CharInfo
{
	HOBJECT			hObj;
	CDestructible * pDest;
	uint32			nCharacterSet;
};

class BodyProp : public Prop
{
	public:

		// Construction and destruction of this class
		BodyProp();
		~BodyProp();

		void			Setup(CharInfo *pCharInfo, LTBOOL bCarryOverAttachments = LTTRUE, std::string strDefaultDeathAnim = std::string() );
		void			Setup(CCharacter* pChar, LTBOOL bCarryOverAttachments, std::string strDefaultDeathAnim, LTBOOL bForceDefaultDeathAnim);

		void			SetupLimb(BodyProp *pProp, ModelNode eParent, LTBOOL bFirey = LTFALSE);

		void			SetInitialMessage(HSTRING hstrInitialMessage)
							{ FREE_HSTRING(m_hstrInitialMessage); m_hstrInitialMessage = hstrInitialMessage; }

		uint32			GetCharacter() const			{ return m_nCharacterSet; }
		ModelSkeleton	GetModelSkeleton() const		{ return m_eModelSkeleton; }
		CAttachments*	GetAttachments()				{ return m_pAttachments; }

		LTFLOAT			GetHeadBiteHealPoints()			{ return m_fHeadBiteHealPoints; }
		LTFLOAT			GetClawHealPoints()				{ return m_fClawHealPoints; }

		HOBJECT			GetHitBox() const				{ return m_hHitBox; }

		LTFLOAT			GetMaxHitPoints() const			{ return m_damage.GetMaxHitPoints(); }
		LTFLOAT			GetMass() const					{ return m_damage.GetMass(); }

		LTFLOAT			ComputeDamageModifier(ModelNode eModelNode) const;

		void			AddSpear(HOBJECT hSpear, const LTRotation& rRot, ModelNode eModelNode);
		void			AddNet(HOBJECT hNet, const LTRotation& rRot, ModelNode eModelNode);

		void			SetModelNodeLastHit(ModelNode eModelNodeLastHit)	{ m_eNodeLastHit = eModelNodeLastHit; }
		ModelNode		GetModelNodeLastHit()						const	{ return m_eNodeLastHit; }

		void			HideAllPieces();
		LTBOOL			AllPiecesHidden();
		LTBOOL			HeadHidden();
		CDestructible*	GetDestructible() { return &m_damage; } 

		void			SetFadeAway(LTBOOL bFade) { m_bFadeAway = bFade; }
		LTVector		GetDeathDir() const { return m_vDeathDir; }
		LTBOOL			GetCanFade()	{ return m_bFadeAway; }
		LTBOOL			GetCanGib()	{ return m_bCanGIB; }

		LTBOOL			HasAcidPool() const			{ return m_bAcidPool; }
		LTBOOL			GetNodePos(ModelNode eNode, LTVector &vPos, LTBOOL bWorldPos=LTFALSE, LTRotation* pRot=LTNULL);
		LTBOOL			GetNodePosition(ModelNode eNode, LTVector &vPos, LTRotation* rRot=LTNULL);

		LTBOOL			GetUpdateGravity( ) const	{ return m_bUpdateGravity; }
		void			SetUpdateGravity( LTBOOL bUpdateGravity ) { m_bUpdateGravity = bUpdateGravity; }

		void			SetHeadRemoved() { m_bHeadAvailable = LTFALSE; }
		void			IncrementClawHits();
		LTBOOL			IsHeadAvailable() { return m_bHeadAvailable; }

	protected:
		enum Constants
		{
			kMaxSpears = 16,
		};

		// Functions to handle engine messages
		uint32			EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		uint32			ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		virtual void	ReadProp(ObjectCreateStruct *pStruct);
		virtual void	HandleModelString(ArgList* pArgList);
		virtual void	TriggerMsg(HOBJECT hSender, HSTRING hMsg);
		virtual void	Damage(DamageStruct damage);

	private:

		void			Setup(HOBJECT hObj, CDestructible * pDest, LTBOOL bCarryOverAttachments, std::string strDefaultDeathAnim, LTBOOL bForceDefaultDeathAnim);

		// Saving and loading of body props
		void		Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		void		Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		void		Update();

		LTBOOL		UpdateNormalDeath(LTFLOAT fTime);
		LTBOOL		UpdateFreezeDeath(LTFLOAT fTime);

		// Hit box functions
		void		CreateHitBox();
		void		UpdateHitBox();

		// Body prop helper functions
		DeathType	GetDeathType(DamageType eDamageType);
		void		SetupByDeathType(uint32 &dwFlags, uint32 &dwUserFlags);

		void		ExplodeLimb(ModelNode eParentNode, LTBOOL bFirey = LTFALSE, LTVector vVelOffset=LTVector(0.0f, 0.0f, 0.0f), LTBOOL bGib=LTTRUE);
		void		DetachLimb(ModelNode eParentNode, LTBOOL bFirey = LTFALSE);
		void		CreateGibs(ModelNode eNode, LTVector vVelOffset);

		void		RemoveObject();

		const char*	GetSpecialDeathTypeAnim(DeathType eType);
		const char* GetSpecialDeathTypeSound(DeathType eType);
		LTBOOL		HasFlyingDeathAnims();

		LTBOOL		CanDoBloodPool();

		void		UpdateAcidPool();

		LTBOOL		CheckForGibs( LTBOOL bFirey = LTFALSE, LTVector vVelOffset=LTVector(0.0f, 0.0f, 0.0f), LTBOOL bUseHistory=LTTRUE);
		void		DoSpecialDamageSound(HOBJECT hDamager, LTBOOL bHeadOnly=LTFALSE);

	private:

		// Spear related stuff
		uint32				m_cSpears;					// How many spears do we have stuck in us
		SPEARSTRUCT			m_aSpears[kMaxSpears];		// Array of spear HOBJECTs

		// Information about the character who this body represents
		uint32				m_nCharacterSet;
		ModelSkeleton		m_eModelSkeleton;
		CAttachments*		m_pAttachments;

		LTFLOAT				m_fHeadBiteHealPoints;
		LTFLOAT				m_fClawHealPoints;

		// Information about how this character died
		LTSmartLink			m_hLastDamager;
		ModelNode			m_eNodeLastHit;
		DamageType			m_eDamageType;
		DeathType			m_eDeathType;
		LTVector			m_vDeathDir;
		CAIVolume *			m_pCurrentVolume;

		HMODELANIM			m_hDeathAnim;
		HMODELANIM			m_hStaticDeathAnim;

		LTSmartLink			m_hFaceHugger;
		HATTACHMENT			m_hFaceHuggerAttachment;

		LTBOOL				m_bCanGIB;
		LTBOOL				m_bLowViolenceGIB;

		// Message information
		LTString			m_hstrInitialMessage;

		// Update data needed to make sure the body responds properly to everything
		LTSmartLink			m_hHitBox;
		LTVector			m_vColor;

		LTFLOAT				m_fStartTime;
		LTFLOAT				m_fFaceHugTime;
		LTFLOAT				m_fAdjustFactor;
		LTBOOL				m_bFirstUpdate;
		LTBOOL				m_bLimbProp;		// Is this just a limb as opposed to torso with limbs
		LTBOOL				m_bSliding;			// Is the limb sliding?
		LTBOOL				m_bFadeAway;		// Should this body fade away when or wait until gibbed
		LTBOOL				m_bDimsSet;			// Have we re-set the dims?

		LTBOOL				m_bAcidPool;		// Did we make an acid pool?
		LTFLOAT				m_fLastAcidSpew;	// When did we do the last explosion.
		LTFLOAT				m_fLastDamage;		// Last damage amount just in case...
		LTBOOL				m_bFlying;			// Is our body flying?
		LTBOOL				m_bFadeIn;			// Are we fading in from being cloaked?
		LTFLOAT				m_fDecloakStartTime;// The time we started to de-cloak.


		LTBOOL				m_bHeadAvailable;	// Is the head available for healing/trophies?
		uint32				m_nClawHits;		// How many times have we been clawed?

		// Update the gravity flag in the MID_UPDATE handler.
		LTBOOL				m_bUpdateGravity;

		// Dims information
		LTVector			m_vOrigDims;		// The original dims
};

class BodyPropPlugin : public IObjectPlugin
{
	public:
		virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char* const * aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

	private:
		CCharacterButeMgrPlugin m_CharacterButeMgrPlugin;
};

#endif // __BODY_PROP_H__


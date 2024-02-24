// ----------------------------------------------------------------------- //
//
// MODULE  : Destructible.h
//
// PURPOSE : Destructible class
//
// CREATED : 9/23/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DESTRUCTIBLE_H__
#define __DESTRUCTIBLE_H__

#include "iaggregate.h"
#include "ltengineobjects.h"
#include "ClientServerShared.h"
#include "DamageTypes.h"
#include "ltsmartlink.h"

// Use ADD_DESTRUCTIBLE_AGGREGATE() in your class definition to enable
// the following properties in the editor.  For example:
//
//BEGIN_CLASS(CMyCoolObj)
//	ADD_DESTRUCTIBLE_AGGREGATE()
//	ADD_STRINGPROP(Filename, "")
//  ...
//

#define ADD_DESTRUCTIBLE_AGGREGATE(group, flags) \
	PROP_DEFINEGROUP(DamageProperties, (group) | (flags)) \
		ADD_REALPROP_FLAG(Mass, 30.0f, (group) | (flags)) \
		ADD_REALPROP_FLAG(HitPoints, 100.0f, (group) | (flags)) \
		ADD_REALPROP_FLAG(MaxHitPoints, 100.0f, (group) | (flags)) \
		ADD_REALPROP_FLAG(Armor, 100.0f, (group) | (flags)) \
		ADD_REALPROP_FLAG(MaxArmor, 100.0f, (group) | (flags)) \
		ADD_LONGINTPROP_FLAG(DamageTriggerCounter, 0, (group) | (flags)) \
		ADD_STRINGPROP_FLAG(DamageTriggerTarget, "", (group) | (flags)) \
		ADD_STRINGPROP_FLAG(DamageTriggerMessage, "", (group) | (flags)) \
		ADD_LONGINTPROP_FLAG(DamageTriggerNumSends, 1, (group) | (flags)) \
		ADD_STRINGPROP_FLAG(DamagerMessage, "", (group) | (flags)) \
		ADD_STRINGPROP_FLAG(DeathTriggerTarget, "", (group) | (flags)) \
		ADD_STRINGPROP_FLAG(DeathTriggerMessage, "", (group) | (flags)) \
		ADD_STRINGPROP_FLAG(PlayerDeathTriggerTarget, "", (group) | (flags)) \
		ADD_STRINGPROP_FLAG(PlayerDeathTriggerMessage, "", (group) | (flags)) \
		ADD_STRINGPROP_FLAG(KillerMessage, "", (group) | (flags)) \
        ADD_BOOLPROP_FLAG(CanHeal, LTTRUE, (group) | (flags)) \
        ADD_BOOLPROP_FLAG(CanRepair, LTTRUE, (group) | (flags)) \
        ADD_BOOLPROP_FLAG(CanDamage, LTTRUE, (group) | (flags)) \
        ADD_BOOLPROP_FLAG(NeverDestroy, LTTRUE, (group) | (flags))


#define MAX_PROGRESSIVE_DAMAGE	20
#define MAX_DAMAGE_HISTORY		32

struct DamageHistory
{
	DamageHistory()
	{
		fDamage		= 0.0f;
		fTime		= 0.0f;
		eType		= DT_UNSPECIFIED;
		nModelNode	= -1;
	}

	LTFLOAT		fDamage;
	LTFLOAT		fTime;
	DamageType	eType;
	uint32		nModelNode;
};

struct DamageStruct
{
  public :

	DamageStruct()
	{
		Clear();
	}

	void Clear()
	{
		vDir.Init();
		fDamage		= 0.0f;
		fDuration	= 0.0f;
		eType		= DT_UNSPECIFIED;
        hDamager    = LTNULL;
        hContainer  = LTNULL;
		bIgnoreFF	= LTFALSE;
	}

    void Save(HMESSAGEWRITE hWrite)
	{
		if (!g_pLTServer || !hWrite) return;

		g_pLTServer->WriteToMessageVector(hWrite, &vDir);
		g_pLTServer->WriteToMessageFloat(hWrite, fDamage);
		g_pLTServer->WriteToMessageFloat(hWrite, fDuration);
		g_pLTServer->WriteToMessageByte(hWrite, eType);

		*hWrite << hDamager;
		*hWrite << hContainer;
	}

    void Load(HMESSAGEREAD hRead)
	{
		if (!g_pLTServer || !hRead) return;

		g_pLTServer->ReadFromMessageVector(hRead, &vDir);
		fDamage		= g_pLTServer->ReadFromMessageFloat(hRead);
		fDuration	= g_pLTServer->ReadFromMessageFloat(hRead);
		eType		= (DamageType) g_pLTServer->ReadFromMessageByte(hRead);

		*hRead >> hDamager;
		*hRead >> hContainer;
	}

    const static LTFLOAT kInfiniteDamage;

    LTBOOL InitFromMessage(HMESSAGEREAD hRead);
    LTBOOL DoDamage(LPBASECLASS pDamager, HOBJECT hVictim, HOBJECT hContainer=LTNULL);

    LTVector     vDir;
	DamageType	eType;
	LTSmartLink	hDamager;
	LTSmartLink	hContainer;
    LTFLOAT      fDamage;
    LTFLOAT      fDuration;  // Progressive damage only...
    LTBOOL      bIgnoreFF;		// no need for load/save
};


class CDestructible : public IAggregate
{
	public :

		CDestructible();
		virtual ~CDestructible();

        void    Reset(LTFLOAT fHitPts, LTFLOAT fArmorPts);
        LTBOOL   Repair(LTFLOAT fAmount);
        LTBOOL   Heal(LTFLOAT fAmount);

		virtual void HandleDestruction(HOBJECT hDamager);

        void SetMass(LTFLOAT fMass);
        void SetMaxHitPoints(LTFLOAT fmhp)				{ m_fMaxHitPoints = fmhp; }
        void SetHitPoints(LTFLOAT fhp)					{ m_fHitPoints = fhp; }
        void SetMaxArmorPoints(LTFLOAT fmap)			{ m_fMaxArmorPoints = fmap; }
        void SetArmorPoints(LTFLOAT fap, LTBOOL bMatchMax=LTFALSE) { m_fArmorPoints = fap; if(bMatchMax) m_fMaxArmorPoints=fap; }
        void SetCanHeal(LTBOOL bCanHeal)				{ m_bCanHeal = bCanHeal; }
        void SetCanRepair(LTBOOL bCanRepair)			{ m_bCanRepair = bCanRepair; }
        void SetNeverDestroy(LTBOOL bNeverDestroy)		{ m_bNeverDestroy = bNeverDestroy; }
        void SetCanDamage(LTBOOL bCanDamage, LTFLOAT fMin = -1.0f);
        void SetCanCrush(LTBOOL bCanCrush)				{ m_bCanCrush = bCanCrush; }
		void SetDeathType(DamageType dt)				{ m_eDeathType = dt; } 

        void SetApplyDamagePhysics(LTBOOL b)			{ m_bApplyDamagePhysics = b; }

        void SetCantDamageTypes(uint32 dwTypes)			{ m_dwCantDamageTypes |= dwTypes; }
		int32 GetCantDamageTypes()				const	{ return m_dwCantDamageTypes; }
        void ClearCantDamageTypes(uint32 dwTypes)		{ m_dwCantDamageTypes &= ~dwTypes; }
		LTBOOL IsCantDamageType(DamageType dt)			{ return m_dwCantDamageTypes & DamageTypeToFlag(dt); }

		void ClearProgressiveDamages();					// Clears all progressive damages
		void ClearProgressiveDamages(DamageType dt);	// Clears all progressive damages of a certain type.

        void		CanSetRayHit(LTBOOL b)		  { m_bCanSetRayHit = b; }
        LTBOOL		GetCanHeal()            const { return m_bCanHeal; }
        LTBOOL		GetCanCrush()            const { return m_bCanCrush; }
        LTBOOL		GetCanRepair()          const { return m_bCanRepair; }
        LTBOOL		GetCanDamage()          const { return m_bCanDamage; }
        LTFLOAT		GetMass()               const { return m_fMass; }
        LTFLOAT		GetHitPoints()          const { return m_fHitPoints; }
        LTFLOAT		GetArmorPoints()        const { return m_fArmorPoints; }
        LTBOOL		IsDead()                const { return m_bDead; }
		DamageType	GetDeathType()			const { return m_eDeathType; }
		DamageType	GetLastDamageType()		const { return m_eLastDamageType; }
        LTFLOAT		GetDeathDamage()        const { return m_fDeathDamage; }
        LTFLOAT		GetLastDamage()         const { return m_fLastDamage; }
		LTFLOAT		GetLastDamageTime()		const { return m_fLastDamageTime; }
        LTVector	GetDeathDir()           const { return m_vDeathDir; }
        LTVector	GetLastDamageDir()      const { return m_vLastDamageDir; }
        LTFLOAT		GetMaxHitPoints()       const;
        LTFLOAT		GetMaxArmorPoints()     const;
		HOBJECT		GetLastDamager()		const { return m_hLastDamager; }
        LTBOOL		GetNeverDestroy()       const { return m_bNeverDestroy; }
		LTFLOAT		GetMinHealth()			const { return m_fMinHealth;    }
		DamageStruct	GetProgressiveDamage(uint32 nIndex) const { return m_ProgressiveDamage[nIndex]; }
		void		SetProgressiveDamage(uint32 nIndex, DamageStruct damage) { m_ProgressiveDamage[nIndex]=damage; }

		DamageHistory*	GetHistory()		const { return (DamageHistory*)m_pHistory; }
		LTFLOAT		GetHistoryDamageAmount(LTFLOAT fTime, uint32 nModelNode = -1);

		void		SetMassBasedBlockingPriority( LTBOOL bValue ) { m_bMassBasedBlockingPriority = bValue; }
		LTBOOL		GetMassBasedBlockingPriority( ) { return m_bMassBasedBlockingPriority; }

		void		SetBlockingPriority();
		void		SetBlockingPriority( uint8 nBlockingPriority );

	protected :

        uint32 EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT lData);
        uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

        void    ApplyDamage(LTVector vDir, LTFLOAT fDamage, DamageType eType, HOBJECT hDamager);
		void	ProcessPlayerDeath(HOBJECT hDamager);			// This has nothing to do with PlayerDeathTriggerTarget/Message!!!
        LTBOOL   CanRagDoll(LTVector & vDir, DamageType eType);
		void	SetLastDamager(HOBJECT hDamager);

		void	UpdateHistory(DamageHistory &pNewHistory);

		HOBJECT		m_hObject;			// The object I'm associated with
		LTSmartLink	m_hLastDamager;		// Our last damager

	private :

		DamageHistory m_pHistory[MAX_DAMAGE_HISTORY];

		DamageType	m_eDeathType;		// How did we die
		DamageType	m_eLastDamageType;	// What damaged us last
        LTFLOAT		m_fLastDamage;      // Amount of last damage done
        LTFLOAT		m_fDeathDamage;     // Amount of damage done on death blow
        LTVector	m_vDeathDir;        // Direction death came from
        LTVector	m_vLastDamageDir;   // Direction of last damage
		LTFLOAT		m_fLastDamageTime;	// Time of last damage

        LTBOOL		m_bApplyDamagePhysics;  // Rag doll?
        LTFLOAT		m_fMass;
        LTFLOAT		m_fMaxHitPoints;
        LTFLOAT		m_fHitPoints;
        LTFLOAT		m_fMaxArmorPoints;
        LTFLOAT		m_fArmorPoints;
        LTBOOL		m_bDead;

        LTBOOL		m_bIsPlayer;

		// Process heal, repair, damage messages?
        LTBOOL		m_bCanHeal;
        LTBOOL		m_bCanRepair;
        LTBOOL		m_bCanDamage;
        LTBOOL		m_bNeverDestroy;
		LTBOOL		m_bCanCrush;

		LTFLOAT		m_fMinHealth;			// A min health value for the m_bCanDamage = false mode

		HSTRING	m_hstrDamageTriggerTarget;
		HSTRING	m_hstrDamageTriggerMessage;
		HSTRING	m_hstrDamagerMessage;
		HSTRING	m_hstrDeathTriggerTarget;
		HSTRING m_hstrDeathTriggerMessage;
		HSTRING	m_hstrPlayerDeathTriggerTarget;		// Like DeathTrigger, but only sent if PLAYER killed us
		HSTRING m_hstrPlayerDeathTriggerMessage;
		HSTRING	m_hstrKillerMessage;

        uint32  m_nDamageTriggerCounter;    // How many times we have to be damaged before we send the trigger
        uint32  m_nDamageTriggerNumSends;   // Number of times to send damage trigger message
        uint32  m_dwCantDamageTypes;        // What can't damage us

		LTBOOL	m_bCanSetRayHit;			// Should we set the rayhit flag?

		// The blocking priority is based on the mass.
		LTBOOL	m_bMassBasedBlockingPriority;

		// Progressive damage being done...

		DamageStruct	m_ProgressiveDamage[MAX_PROGRESSIVE_DAMAGE];

	private :

        LTBOOL ReadProp(LPBASECLASS pObject, ObjectCreateStruct *pInfo);
		void InitialUpdate(LPBASECLASS pObject);
		void Update(LTFLOAT fDeltaTime);

		void HandleTrigger(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead);
		void HandleDamage(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead);
		void HandleHeal(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead);
		void HandleRepair(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead);
		void HandleCrush(LPBASECLASS pObject, HOBJECT hSender);
        void HandleTouch(LPBASECLASS pObject, HOBJECT hSender, LTFLOAT fForce);
        void Save(HMESSAGEWRITE hWrite, uint8 nType);
        void Load(HMESSAGEREAD hRead, uint8 nType);
        void DoActualHealing(LTFLOAT fAmount);

		void DoDamage(DamageStruct & damage, HOBJECT hSender);
		void UpdateProgressiveDamage(LTFLOAT fDeltaTime);
		void AddProgressiveDamage(DamageStruct & damage);

        LTBOOL DebugDamageOn();
		LTBOOL StunTest(DamageStruct & damage);
};

inline LTBOOL CDestructible::CanRagDoll(LTVector & vDir, DamageType eType)
{
	if (m_hObject)
	{
        uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
		if ((dwFlags & FLAG_SOLID) && (dwFlags & FLAG_GRAVITY))
		{
			return (m_bApplyDamagePhysics && !m_bIsPlayer && (eType != DT_BULLET) && (eType != DT_HIGHCALIBER) && (eType != DT_SHOTGUN) && 
					(m_fMass < INFINITE_MASS) && (vDir.MagSqr() > 0.001f));
		}
	}

    return LTFALSE;
}

#endif // __DESTRUCTIBLE_H__

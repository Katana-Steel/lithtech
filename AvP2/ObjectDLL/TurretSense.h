// ----------------------------------------------------------------------- //
//
// MODULE  : TurretSense.h
//
// PURPOSE : Sense information aggregrate for the turret
//
// CREATED : 2.29.00
//
// ----------------------------------------------------------------------- //

#ifndef __TURRET_SENSE_H__
#define __TURRET_SENSE_H__

#include "Sense.h"
#include "Callback.h"

#include <map>

class CCharacter;
class Turret;
class CTurretVisionInstance;

class CTurretVision : public IAggregate
{

public :

	typedef Callback2v< const CTurretVision &, const CCharacter * > CallbackFunction;

	typedef std::map< const CCharacter *, CTurretVisionInstance * > InstanceList;

	template< class Client, class Member>
	static CallbackFunction
	MakeCallback(Client & client, Member member)
	{
		return make_callback2v<const CTurretVision&, const CCharacter*>(client,member);
	}

	template< class Function >
	static CallbackFunction
	MakeCallback(Function function)
	{
		return make_callback2v<const CTurretVision&, const CCharacter*>(function);
	}

	static CallbackFunction
	MakeCallback()
	{
		return CallbackFunction();
	}

	public :

		CTurretVision()
			: m_hOwner(LTNULL),
			  m_hFocus(LTNULL),
			  m_nFocusRank(-1),
			  m_bFocusIsVisible(false),
			  m_vOwnerUp(0.0f,0.0f,0.0f),
			  m_fMaxRangeSqr(0.0f),
			  m_fMaxPitch(0.0f),
			  m_fMinPitch(0.0f),
			  m_fMaxYaw(0.0f),
			  m_fMinYaw(0.0f),
			  m_fReactionRate(0.0f),
			  m_fDeactionRate(0.0f),
			  m_bCanSeeThrough(LTFALSE),
			  m_bCanShootThrough(LTFALSE)
		{
			m_nCharacterRanks[0] = -1;
			m_nCharacterRanks[1] = -1;
			m_nCharacterRanks[2] = -1;
			m_nCharacterRanks[3] = -1;
			m_nCharacterRanks[4] = -1;

			m_bCanBeShot[0] = LTFALSE;
			m_bCanBeShot[1] = LTFALSE;
			m_bCanBeShot[2] = LTFALSE;
			m_bCanBeShot[3] = LTFALSE;
			m_bCanBeShot[4] = LTFALSE;
		}
			  
		virtual ~CTurretVision() {}

		void  Init(const Turret * pTurret);

		const LTSmartLink & GetOwner() const { return m_hOwner; }
		const LTSmartLink & GetFocus() const { return m_hFocus; }
		int GetFocusRank() const { return m_nFocusRank; }
		bool FocusIsVisible() const { return m_bFocusIsVisible; }
		bool FocusCanBeShotFrom(const LTVector & vShootFromPos, LTFLOAT fWeaponRangeSqr) const;

		int GetRank(int character_class) const;
		LTBOOL GetCanBeShot(int character_class) const;

		LTBOOL GetCanShootThrough() const { return m_bCanShootThrough; }
		LTBOOL GetCanSeeThrough() const { return m_bCanSeeThrough; }

		// This must be set by owner for the sense to do anything.
		void  SetFullCallback(const CallbackFunction & val)    { m_cbFullStimulation = val; }

		void  SetReactionRate(LTFLOAT val);
		void  SetDeactionRate(LTFLOAT val);

		void  SetRanks( int marine_rank,
					    int corporate_rank,
						int predator_rank,
						int alien_rank)
		{
			m_nCharacterRanks[1] = marine_rank;
			m_nCharacterRanks[2] = corporate_rank;
			m_nCharacterRanks[3] = predator_rank;
			m_nCharacterRanks[4] = alien_rank;
		}

		void  SetCanBeShot( LTBOOL marine_canbeshot,
							LTBOOL corporate_canbeshot,
							LTBOOL predator_canbeshot,
							LTBOOL alien_canbeshot)
		{
			m_bCanBeShot[1] = marine_canbeshot;
			m_bCanBeShot[2] = corporate_canbeshot;
			m_bCanBeShot[3] = predator_canbeshot;
			m_bCanBeShot[4] = alien_canbeshot;
		}

		void SetCanShootThrough(LTBOOL val) { m_bCanShootThrough = val; }
		void SetCanSeeThrough(LTBOOL val)   { m_bCanSeeThrough = val; }

		// Called by CTurretVisionInstance
		virtual void FullStimulation(const CCharacter & character);

	protected :

		virtual uint32 EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT lData);

	private :

		void Update();
		void InitialUpdate(const Turret * pTurret);

		void ReadProp(ObjectCreateStruct *pInfo);

		void Save(LMessage * pMsg);
		void Load(LMessage * pMsg);

		bool IsVisible(const CCharacter * pChar);

		void PruneDeadInstances();

	private :

		LTSmartLink m_hOwner;
		LTSmartLink m_hFocus;
		
		int m_nFocusRank;
		bool m_bFocusIsVisible;

		InstanceList m_instances;

		DVector  m_vOwnerUp;
		LTFLOAT  m_fMaxRangeSqr;
		LTFLOAT  m_fMaxPitch;
		LTFLOAT  m_fMinPitch;
		LTFLOAT  m_fMaxYaw;
		LTFLOAT  m_fMinYaw;

		LTFLOAT  m_fReactionRate;
		LTFLOAT  m_fDeactionRate;

		LTBOOL   m_bCanSeeThrough;
		LTBOOL   m_bCanShootThrough;

		CallbackFunction m_cbFullStimulation;

		// Rank is used to determine who is more important.
		int m_nCharacterRanks[5];

		// Can this class be shot.
		LTBOOL m_bCanBeShot[5];
};

class CTurretVisionInstance : public CSenseInstance
{
	public :


		CTurretVisionInstance(CTurretVision & owner, 
			                  const CCharacter & target,
							  LTFLOAT fAccumRate = 1.0f, LTFLOAT fDecayRate = 0.25f )
			: CSenseInstance(fAccumRate, fDecayRate),
			  m_owner(owner),
			  m_target(target),
			  m_bIsVisible(LTTRUE) {}

		virtual ~CTurretVisionInstance() {}

		LTBOOL Check();

		void SetVisible(LTBOOL val) { m_bIsVisible = val; }
		bool IsVisible() const { return LTTRUE == m_bIsVisible; }

		// for debugging
		const CCharacter & GetTarget() const { return m_target; }
		const CTurretVision & GetOwner() const { return m_owner; }

		virtual void Save(LMessage * pMsg);
		virtual void Load(LMessage * pMsg);

	private:

		CTurretVision & m_owner;
		const CCharacter & m_target;

		LTBOOL m_bIsVisible;
};

#endif // __TURRET_SENSE_H__

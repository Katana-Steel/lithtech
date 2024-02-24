// ----------------------------------------------------------------------- //
//
// MODULE  : Explosion.h
//
// PURPOSE : Model Explosion - Definition
//
// CREATED : 11/25/97
//
// BUTCHERED : 5/28/98
// 
// ----------------------------------------------------------------------- //

#ifndef __EXPLOSION_H__
#define __EXPLOSION_H__

// ----------------------------------------------------------------------- //

#include "GameBase.h"
#include "DamageTypes.h"
#include "FXButeMgr.h"
#include "ltsmartlink.h"

// ----------------------------------------------------------------------- //

struct EXPLOSION_CREATESTRUCT
{
	LTFLOAT		fDamageRadius;
	LTFLOAT		fMaxDamage;
	DamageType	eDamageType;

	DamageType  eProgDamageType;
	LTFLOAT     fProgDamage;
	LTFLOAT		fProgDamageDuration;

	LTVector	vPos;
	HOBJECT		hFiredFrom;
	LTBOOL		bRemoveWhenDone;
	uint8		nImpactFXId;
	LTBOOL		bHideFromShooter;

	EXPLOSION_CREATESTRUCT()
		: fDamageRadius(0.0f),
		  fMaxDamage(0.0f),
		  eDamageType(DT_UNSPECIFIED),

		  eProgDamageType(DT_UNSPECIFIED),
		  fProgDamage(0.0f),
		  fProgDamageDuration(0.0f),

		  vPos(0,0,0),
		  hFiredFrom(LTNULL),
		  bRemoveWhenDone(LTTRUE),
		  nImpactFXId(FXBMGR_INVALID_ID),
		  bHideFromShooter(LTFALSE)  {}
};

// ----------------------------------------------------------------------- //

class Explosion : public GameBase
{
	public :

 		Explosion();
		void	Setup(const EXPLOSION_CREATESTRUCT &expCS);

		HOBJECT		GetFiredFrom() const { return m_hFiredFrom; }
		DamageType	GetDamageType() const { return m_eDamageType; }

	protected :

		uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        uint32	ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

#ifndef _FINAL
		virtual LTVector GetBoundingBoxColor();
#endif

		virtual void Start(HOBJECT hFiredFrom=DNULL);

		LTFLOAT			m_fDamageRadius;
		LTFLOAT			m_fMaxDamage;
		DamageType		m_eDamageType;
		LTSmartLink		m_hFiredFrom;
		LTVector		m_vPos;
		LTBOOL			m_bRemoveWhenDone;
		uint8			m_nImpactFXId;
		DamageType		m_eProgDamType;
		LTFLOAT			m_fProgDamage;
		LTFLOAT			m_fProgDamageDur;
		LTBOOL			m_bHideFromShooter;

	private :
	
		void ReadProp();
		void Update();
		void UpdateInContainer(ContainerPhysics* pCPStruct);
		void DamageObjectsInSphere();
		void DamageObject(HOBJECT hObj);

		void CacheFiles();

		void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
};

// ----------------------------------------------------------------------- //

class CExplosionPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT	PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength);

	private:

		CFXButeMgrPlugin	m_FXButeMgrPlugin;
};

// ----------------------------------------------------------------------- //

#endif // __EXPLOSION_H__

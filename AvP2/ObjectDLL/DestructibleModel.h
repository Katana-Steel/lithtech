//----------------------------------------------------------
//
// MODULE  : DestructibleModel.h
//
// PURPOSE : Destructible model aggregate class
//
// CREATED : 4/23/98
//
//----------------------------------------------------------

#ifndef __DESTRUCTABLE_MODEL_H__
#define __DESTRUCTABLE_MODEL_H__

#include "LTString.h"
#include "Destructible.h"
#include "DebrisFuncs.h"
#include "SurfaceMgr.h"

#define DEFAULT_DWMA_MASS		3000.0f

//----------------------------------------------------------
// Use ADD_DESTRUCTIBLE_WORLD_MODEL_AGGREGATE() in your class definition 
// to enable the following properties in the editor.  For example:
//
//BEGIN_CLASS(CMyCoolObj)
//	ADD_DESTRUCTIBLE_WORLD_MODELAGGREGATE()
//	ADD_STRINGPROP(Filename, "")
//  ...
//

#define ADD_DESTRUCTIBLE_MODEL_AGGREGATE(group, flags) \
\
	ADD_DESTRUCTIBLE_AGGREGATE((group), (flags)) \
\
	ADD_REALPROP_FLAG(Mass, DEFAULT_DWMA_MASS, (group) | (flags)) \
	PROP_DEFINEGROUP(ExplosionProperties, (group<<1) | (flags)) \
\
		ADD_BOOLPROP_FLAG_HELP(CreateExplosion, 0, (group<<1) | (flags), "Should an explosion be created when this object dies?") \
		ADD_STRINGPROP_FLAG_HELP(ImpactFXName, "", (group<<1) | (flags) | PF_STATICLIST, "The ImpactFX name from the FX attribute file.")\
		ADD_REALPROP_FLAG_HELP(DamageRadius, 200.0f, (group<<1) | (flags) | PF_RADIUS, "The radius that will be considered for damaging objects when the explosion occurs. Any object touching this radius will be damaged.")\
		ADD_REALPROP_FLAG_HELP(MaxDamage, 200.0f, (group<<1) | (flags), "The maximum amount of damage at the center point of the explosion. The amount of damage falls off towards the edge of the damage radius.")\
		ADD_STRINGPROP_FLAG_HELP(DamageType, "", (group<<1) | (flags) | PF_STATICLIST, "This is the type of damage the object will take. Not all objects will have a special reaction to each damage type... mainly just characters.")\
		ADD_STRINGPROP_FLAG_HELP(Spawn, "", (group<<1) | (flags), "This is an object that will be spawned when the explosion occurs.") \
\
	ADD_DEBRISTYPE_PROPERTY(flags)\
	ADD_STRINGPROP_FLAG(SurfaceOverride, "", (flags) | PF_STATICLIST)


//----------------------------------------------------------

class CDestructibleModelPlugin : public IObjectPlugin
{
  public:

	virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath, 
		const char* szPropName, 
		char* const * aszStrings, 
		uint32* pcStrings, 
		const uint32 cMaxStrings, 
		const uint32 cMaxStringLength);

  protected :

};

//----------------------------------------------------------

class CDestructibleModel : public CDestructible
{
	public:

		CDestructibleModel();
		virtual ~CDestructibleModel();

		void	DoExplosion(HOBJECT hSender);

		void	SetRemoveOnDeath(LTBOOL val)	{ m_bRemoveOnDeath = val; }
		LTBOOL	GetRemoveOnDeath() const		{ return m_bRemoveOnDeath; }

		void	SetDebrisId(uint8 val)			{ m_nDebrisId = val; }
		uint8	GetDebrisId() const				{ return m_nDebrisId; }
		LTBOOL	GetCreateDebris() const			{ return (m_nDebrisId != DEBRISMGR_INVALID_ID); }

		void	SetSurfaceType(const char *szSurf);

	protected:

		uint32	EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT lData);
		uint32	ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);
	
		LTBOOL	ReadProp(ObjectCreateStruct *);
		void	CreateWorldModelDebris();
		void	CreateDebris();
		void	SetSurfaceType();

	private:

		void	Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		void	CacheFiles();
		void	SpawnItem();


		LTBOOL		m_bRemoveOnDeath;

		// Exlposion property variables
		LTBOOL		m_bCreateExplosion;
		uint8		m_nImpactFXId;
		LTFLOAT		m_fDamageRadius;
		LTFLOAT		m_fMaxDamage;
		DamageType	m_eDamageType;

		// Debris property variables
		uint8		m_nDebrisId;
		LTBOOL		m_bCreatedDebris;

		LTString	m_ltstrSpawn;
		LTString	m_ltstrSurfaceOverride;

		uint32		m_dwOriginalFlags;
		LTBOOL		m_bSaveNeverDestroy;
		LTBOOL		m_bSaveCanDamage;
};

//----------------------------------------------------------

#endif // __DESTRUCTABLE_MODEL_H__

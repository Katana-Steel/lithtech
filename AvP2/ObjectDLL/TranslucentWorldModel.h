// ----------------------------------------------------------------------- //
//
// MODULE  : TranslucentWorldModel.h
//
// PURPOSE : TranslucentWorldModel definition
//
// CREATED : 4/12/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TRANSLUCENT_WORLD_MODEL_H__
#define __TRANSLUCENT_WORLD_MODEL_H__

#include "DestructibleModel.h"
#include "GameBase.h"
#include "SFXFuncs.h"

class TranslucentWorldModel : public GameBase
{
	public :

		TranslucentWorldModel();
		virtual ~TranslucentWorldModel();

		virtual LTBOOL		CanShootThrough() { return m_bCanShootThrough; }
		virtual LTBOOL		CanSeeThrough()   { return m_bCanSeeThrough; }

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
		DBOOL	m_bLensFlare;		// Should we make a lens flare
		DFLOAT	m_fInitialAlpha;	// Starting alpha
		DFLOAT	m_fFinalAlpha;		// Ending alpha (if changing)
		DFLOAT	m_fChangeTime;		// Time to change alpha
		DFLOAT	m_fStartTime;		// Time we stated changing alpha

		LENSFLARE	m_LensInfo;		// Lens flare info

		LTBOOL			m_bCanShootThrough;
		LTBOOL			m_bCanSeeThrough;

	private :

		CDestructibleModel m_damage;

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwSaveFlags);

		DBOOL	ReadProp(ObjectCreateStruct *pInfo);
		void	InitialUpdate();
		void	Update();
		void	FirstUpdate();

		void	HandleTrigger(HOBJECT hSender, HSTRING hMsg);

		LTBOOL	m_bFirstUpdate;
};

class CTranslucentWorldModelPlugin : public IObjectPlugin
{
  public:

	virtual DRESULT	PreHook_EditStringList(
		const char* szRezPath, 
		const char* szPropName, 
		char* const * aszStrings, 
		DDWORD* pcStrings, 
		const DDWORD cMaxStrings, 
		const DDWORD cMaxStringLength);

  protected :

	  CDestructibleModelPlugin m_DestructibleModelPlugin;
};

#endif // __TRANSLUCENT_WORLD_MODEL_H__

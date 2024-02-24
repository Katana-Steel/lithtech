// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisMgr.h
//
// PURPOSE : Definition of debris mgr
//
// CREATED : 3/17/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DEBRIS_MGR_H__
#define __DEBRIS_MGR_H__

#include "GameButeMgr.h"
#include "TemplateList.h"
#include "SurfaceDefs.h"

// ----------------------------------------------------------------------- //

class CDebrisMgr;
extern CDebrisMgr* g_pDebrisMgr;

// ----------------------------------------------------------------------- //

#ifdef _WIN32
	#define DEBRISMGR_DEFAULT_FILE		"Attributes\\Debris.txt"
#else
	#define DEBRISMGR_DEFAULT_FILE		"Attributes/Debris.txt"
#endif

#define DEBRISMGR_INVALID_ID		-1

#define DEBRIS_MAX_NAME_LENGTH		64
#define DEBRIS_MAX_FILE_PATH		64

// ----------------------------------------------------------------------- //

#define  DEBRISMGR_FLAG_CREATEONLYONE	(1 << 0)
#define  DEBRISMGR_FLAG_ALWAYSCREATE	(1 << 1)
#define  DEBRISMGR_FLAG_DONTSCALE		(1 << 2)
#define  DEBRISMGR_FLAG_HUMAN_PTRAIL	(1 << 3)
#define  DEBRISMGR_FLAG_PRED_PTRAIL		(1 << 4)
#define  DEBRISMGR_FLAG_ALIEN_TRAIL		(1 << 5)
#define  DEBRISMGR_FLAG_SYNTH_TRAIL		(1 << 6)

const uint32 DEBRISMGR_FLAG_PTRAIL_FILTER = (DEBRISMGR_FLAG_HUMAN_PTRAIL | DEBRISMGR_FLAG_HUMAN_PTRAIL | DEBRISMGR_FLAG_ALIEN_TRAIL | DEBRISMGR_FLAG_SYNTH_TRAIL);

// ----------------------------------------------------------------------- //

struct DEBRIS_MODEL
{
	DEBRIS_MODEL();

	char	szModel[DEBRIS_MAX_NAME_LENGTH];
	char	szSkin[DEBRIS_MAX_NAME_LENGTH];
	uint16	nFlags;
};

// ----------------------------------------------------------------------- //

struct DEBRIS_STRING
{
	DEBRIS_STRING();

	char	szString[DEBRIS_MAX_NAME_LENGTH];
};

// ----------------------------------------------------------------------- //

struct DEBRIS
{
	DEBRIS();
	~DEBRIS();

	LTBOOL	Init(CButeMgr & buteMgr, char* aTagName);
	void	Cache(CDebrisMgr* pDebrisMgr);

	uint8	nId;

	char		szName[DEBRIS_MAX_NAME_LENGTH];
	SurfaceType	eSurfaceType;

	int				nModels;
	DEBRIS_MODEL	*pModels;

    LTVector	vMinVel;
    LTVector	vMaxVel;
    LTVector	vMinDOffset;
    LTVector	vMaxDOffset;
	float		fMinScale;
	float		fMaxScale;
	float		fMinLifetime;
	float		fMaxLifetime;
	float		fFadetime;
	int			nNumber;
    LTBOOL		bRotate;
	int			nMinBounce;
	int			nMaxBounce;
	float		fGravityScale;
	float		fAlpha;

	int				nBounceSnds;
	DEBRIS_STRING	*szBounceSnds;

	int				nExplodeSnds;
	DEBRIS_STRING	*szExplodeSnds;

	char		szPShowerFX[DEBRIS_MAX_NAME_LENGTH];
};

typedef CTList<DEBRIS*> DebrisList;

// ----------------------------------------------------------------------- //

struct MULTI_DEBRIS
{
	MULTI_DEBRIS();
	~MULTI_DEBRIS();

	LTBOOL	Init(CButeMgr & buteMgr, char* aTagName);

	uint8	nId;

	char		szName[DEBRIS_MAX_NAME_LENGTH];

	int				nDebrisFX;
	DEBRIS_STRING	*szDebrisFX;
};

typedef CTList<MULTI_DEBRIS*> MultiDebrisList;

// ----------------------------------------------------------------------- //
class CDebrisMgr : public CGameButeMgr
{
	public:

		CDebrisMgr();
		~CDebrisMgr();

		void		CacheAll();
		void		Reload(ILTCSBase *pInterface) { Term(); m_buteMgr.Term(); Init(pInterface); }

		LTBOOL		Init(ILTCSBase *pInterface, const char* szAttributeFile=DEBRISMGR_DEFAULT_FILE);
		void		Term();

		DEBRIS*		GetDebris(uint8 nId);
		DEBRIS*		GetDebris(char* pName);

		MULTI_DEBRIS* GetMultiDebris(uint8 nId);
		MULTI_DEBRIS* GetMultiDebris(char* pName);

		int			GetNumDebris() const { return m_DebrisList.GetLength(); }

#ifdef _CLIENTBUILD

		void		CreateDebris(uint8 nDebrisId, HOBJECT *hObjects, uint8 nNumObjects, LTVector vPos=LTVector(0,0,0), uint16* pFlags=LTNULL);
		HOBJECT		CreateDebrisObject(uint8 nDebrisId, uint8 nModel, LTVector vPos);
		char*		GetExplodeSound(uint8 nDebrisId);
		char*		GetBounceSound(uint8 nDebrisId);

#endif

	private:

		DebrisList		m_DebrisList;
		MultiDebrisList	m_MultiDebrisList;
};


////////////////////////////////////////////////////////////////////////////
//
// CDebrisMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CDebrisMgr
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CLIENTBUILD

#include "iobjectplugin.h"

class CDebrisMgrPlugin : public IObjectPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char* const * aszStrings,
            uint32* pcStrings,
            const uint32 cMaxStrings,
            const uint32 cMaxStringLength);

        LTBOOL PopulateStringList(char* const * aszStrings, uint32* pcStrings,
            const uint32 cMaxStrings, const uint32 cMaxStringLength);

	protected :

		static CDebrisMgr sm_DebrisMgr;
};

#endif // _CLIENTBUILD

#endif // __DEBRIS_MGR_H__


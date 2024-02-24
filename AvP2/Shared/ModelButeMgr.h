// ----------------------------------------------------------------------- //
//
// MODULE  : ModelButeMgr.h
//
// PURPOSE : ModelButeMgr definition
//
// CREATED : 05.22.1999
//
// ----------------------------------------------------------------------- //

#ifndef __MODELBUTE_MGR_H__
#define __MODELBUTE_MGR_H__

// ----------------------------------------------------------------------- //
// Includes

#include "GameButeMgr.h"
#include "ClientServerShared.h"
#include "ModelDefs.h"

// ----------------------------------------------------------------------- //
// Defines

#ifdef _WIN32
	#define MBMGR_DEFAULT_FILE				"Attributes\\ModelButes.txt"
#else
	#define MBMGR_DEFAULT_FILE				"Attributes/ModelButes.txt"
#endif

#define MBMGR_MAX_NODE_CHILDREN			12

#define MBMGR_MAX_STRAFE_RANGE_SETS		5
#define MBMGR_MAX_STRAFE_RANGE_NODES	2

#define MBMGR_MAX_AIM_RANGE_SETS		10
#define MBMGR_MAX_AIM_RANGE_NODES		10

// ----------------------------------------------------------------------- //
// Classes

class CModelButeMgr;
extern CModelButeMgr* g_pModelButeMgr;

// ----------------------------------------------------------------------- //

class CModelButeMgr : public CGameButeMgr
{
	public : // Public member variables

		CModelButeMgr();
		~CModelButeMgr();

		LTBOOL		Init(ILTCSBase *pInterface, const char* szAttributeFile = MBMGR_DEFAULT_FILE);
		void		Term();

		void		Reload(ILTCSBase *pInterface)	{ Term(); Init(pInterface); }


		// Skeleton

		int				GetNumSkeletons() { return m_cSkeletons; }
		ModelSkeleton	GetSkeletonFromName(const char * szName);
		int				GetSkeletonNumNodes(ModelSkeleton eModelSkeleton);

		CString				GetNameFromSkeleton(ModelSkeleton eModelSkeleton);
		const char*			GetSkeletonDefaultDeathAni(ModelSkeleton eModelSkeleton);
		ModelNode			GetSkeletonDefaultHitNode(ModelSkeleton eModelSkeleton);

		const LTFloatPt & GetSkeletonHeadTurnYawRange(ModelSkeleton eModelSkeleton);
		const LTFloatPt & GetSkeletonHeadTurnPitchRange(ModelSkeleton eModelSkeleton);

		int					GetNumStrafeYawNodes(ModelSkeleton eModelSkeleton, int nSet = 0);
		const ModelNode*	GetStrafeYawNodes(ModelSkeleton eModelSkeleton, int nSet = 0);
		int					GetNumAimPitchNodes(ModelSkeleton eModelSkeleton, int nSet = 0);
		const ModelNode*	GetAimPitchNodes(ModelSkeleton eModelSkeleton, int nSet = 0);

		ModelNode		GetSkeletonNode(ModelSkeleton eModelSkeleton, const char* szName);
		const char*		GetSkeletonNodeName(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		const char*		GetSkeletonNodeDeathAniFrag(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		const char*		GetSkeletonNodePiece(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		const char*		GetSkeletonNodeLimbAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		const char*		GetSkeletonNodeGIBType(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		LTFLOAT			GetSkeletonNodeDamageFactor(ModelSkeleton eModelSkeleton, ModelNode eModelNode);

		LTBOOL			ExplodeNode(ModelSkeleton eModelSkeleton, ModelNode eModelNode, LTFLOAT fDamage);

		LTFLOAT			GetSkeletonNodeMinExplodeDamage(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		LTFLOAT			GetSkeletonNodeMinExplodeChance(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		LTFLOAT			GetSkeletonNodeMaxExplodeDamage(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		LTFLOAT			GetSkeletonNodeMaxExplodeChance(ModelSkeleton eModelSkeleton, ModelNode eModelNode);

		LTBOOL			GetSkeletonNodeCanLiveExplode(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		LTBOOL			GetSkeletonNodeForceCrawl(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		ModelNode		GetSkeletonNodeBleeder(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		ModelNode		GetSkeletonNodeParent(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		HitLocation		GetSkeletonNodeHitLocation(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		LTFLOAT			GetSkeletonNodeHitRadius(ModelSkeleton eModelSkeleton, ModelNode eModelNode);

		LTIntPt			GetSkeletonNodeStrafeYawRange(ModelSkeleton eModelSkeleton, ModelNode eModelNode, int nSet = 0);
		LTIntPt			GetSkeletonNodeAimPitchRange(ModelSkeleton eModelSkeleton, ModelNode eModelNode, int nSet = 0);

		int				GetSkeletonNodeNumChildren(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		ModelNode*		GetSkeletonNodeChildren(ModelSkeleton eModelSkeleton, ModelNode eModelNode);


		// Node Scripts

		int				GetNumNodeScripts() { return m_cNScripts; }

		int				GetNScriptNumPts(ModelNScript eModelNScript);
		const char*		GetNScriptNodeName(ModelNScript eModelNScript);
		uint8			GetNScriptFlags(ModelNScript eModelNScript);
		LTFLOAT			GetNScriptPtTime(ModelNScript eModelNScript, int nPt);
		const LTVector&	GetNScriptPtPosOffset(ModelNScript eModelNScript, int nPt);
		const LTVector&	GetNScriptPtRotOffset(ModelNScript eModelNScript, int nPt);


	protected : // Protected inner classes

		class CNode;
		class CSkeleton;
		class CNScriptPt;
		class CNScript;

	protected : // Protected member variables

		int				m_cSkeletons;
		CSkeleton*		m_aSkeletons;

		int				m_cNScripts;
		CNScript*		m_aNScripts;
};

// ----------------------------------------------------------------------- //
//
// !!!!!!!!!!!!!!!!  CModelButeMgr inner class definitions  !!!!!!!!!!!!!!
//
// ----------------------------------------------------------------------- //

class CModelButeMgr::CNode
{
	public :

		CNode()
		{
			m_szName[0] = 0;
			m_szDeathAniFrag[0] = 0;
			m_szPiece[0] = 0;
			m_szLimbAni[0] = 0;
			m_szGIBType[0] = 0;
			m_fDamageFactor = 0.0f;

			m_fMinExplodeDamage = 0.0f;
			m_fMinExplodeChance = 0.0f;
			m_fMaxExplodeDamage = 0.0f;
			m_fMaxExplodeChance = 0.0f;

			m_bCanLiveExplode = LTFALSE;
			m_bForceCrawl = LTFALSE;
			m_eNodeBleeder = eModelNodeInvalid;
			m_eNodeParent = eModelNodeInvalid;
			m_eHitLocation = HL_UNKNOWN;
			m_fHitRadius = 10.0f;

			for(int i = 0; i < MBMGR_MAX_STRAFE_RANGE_SETS; i++)
				m_ptStrafeYawRange[i] = LTIntPt(0,0);

			for(int j = 0; j < MBMGR_MAX_AIM_RANGE_SETS; j++)
				m_ptAimPitchRange[j] = LTIntPt(0,0);

			m_nNumNodeChildren = 0;
			memset(m_eNodeChildren, 0, sizeof(ModelNode) * MBMGR_MAX_NODE_CHILDREN);
		}

	public :

		// Data read directly from the attributes
		char			m_szName[32];
		char			m_szDeathAniFrag[32];
		char			m_szPiece[32];
		char			m_szLimbAni[32];
		char			m_szGIBType[32];
		LTFLOAT			m_fDamageFactor;

		LTFLOAT			m_fMinExplodeDamage;
		LTFLOAT			m_fMinExplodeChance;
		LTFLOAT			m_fMaxExplodeDamage;
		LTFLOAT			m_fMaxExplodeChance;

		LTBOOL			m_bCanLiveExplode;
		LTBOOL			m_bForceCrawl;
		ModelNode		m_eNodeBleeder;
		ModelNode		m_eNodeParent;
		HitLocation		m_eHitLocation;
		LTFLOAT			m_fHitRadius;

		LTIntPt			m_ptStrafeYawRange[MBMGR_MAX_STRAFE_RANGE_SETS];
		LTIntPt			m_ptAimPitchRange[MBMGR_MAX_AIM_RANGE_SETS];

		// Data calculated from other data
		int				m_nNumNodeChildren;
		ModelNode		m_eNodeChildren[MBMGR_MAX_NODE_CHILDREN];
};

// ----------------------------------------------------------------------- //

class CModelButeMgr::CSkeleton
{
	public :

		CSkeleton()
		{
			m_aNodes = LTNULL;
			m_cNodes = 0;
			m_szDefaultDeathAni[0] = 0;
			m_eModelNodeDefaultHit = eModelNodeInvalid;

			m_fptHeadTurnYawRange.x = 0.0f;
			m_fptHeadTurnYawRange.y = 0.0f;

			m_fptHeadTurnPitchRange.x = 0.0f;
			m_fptHeadTurnPitchRange.y = 0.0f;

			int i, j;

			for(i = 0; i < MBMGR_MAX_STRAFE_RANGE_SETS; i++)
			{
				for(j = 0; j < MBMGR_MAX_STRAFE_RANGE_NODES; j++)
				{
					m_eStrafeYawNodes[i][j] = eModelNodeInvalid;
				}

				m_nStrafeYawNodes[i] = 0;
			}

			for(i = 0; i < MBMGR_MAX_AIM_RANGE_SETS; i++)
			{
				for(j = 0; j < MBMGR_MAX_AIM_RANGE_NODES; j++)
				{
					m_eAimPitchNodes[i][j] = eModelNodeInvalid;
				}

				m_nAimPitchNodes[i] = 0;
			}
		}

	public :

		CString		m_Name;
		CNode*		m_aNodes;
		int			m_cNodes;
		char		m_szDefaultDeathAni[32];
		ModelNode	m_eModelNodeDefaultHit;

		LTFloatPt	m_fptHeadTurnYawRange;
		LTFloatPt	m_fptHeadTurnPitchRange;

		ModelNode	m_eStrafeYawNodes[MBMGR_MAX_STRAFE_RANGE_SETS][MBMGR_MAX_STRAFE_RANGE_NODES];
		int			m_nStrafeYawNodes[MBMGR_MAX_STRAFE_RANGE_SETS];

		ModelNode	m_eAimPitchNodes[MBMGR_MAX_AIM_RANGE_SETS][MBMGR_MAX_AIM_RANGE_NODES];
		int			m_nAimPitchNodes[MBMGR_MAX_AIM_RANGE_SETS];
};

// ----------------------------------------------------------------------- //

class CModelButeMgr::CNScriptPt
{
	public :

		CNScriptPt()
		{
			m_fTime = 0.0f;
			m_vPosOffset.Init();
			m_vRotOffset.Init();
		}

	public :

		LTFLOAT		m_fTime;
		LTVector	m_vPosOffset;
		LTVector	m_vRotOffset;
};

// ----------------------------------------------------------------------- //

class CModelButeMgr::CNScript
{
	public :

		CNScript()
		{
			m_aNScriptPts = LTNULL;
			m_cNScriptPts = 0;

			m_szName[0] = 0;
			m_bFlags = 0;
		}

	public:

		CNScriptPt*		m_aNScriptPts;
		int				m_cNScriptPts;

		char			m_szName[32];
		uint8			m_bFlags;
};



#endif // __MODELBUTE_MGR_H__


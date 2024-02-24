// ----------------------------------------------------------------------- //
//
// MODULE  : ModelButeMgr.cpp
//
// PURPOSE : ModelButeMgr implementation - Controls attributes of all ModelButes
//
// CREATED : 12/02/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ModelButeMgr.h"
#include "CommonUtilities.h"

#ifndef _CLIENTBUILD
	#include "GameServerShell.h"
	#include "MultiplayerMgrDefs.h"
#endif

// ----------------------------------------------------------------------- //
// Globals/statics

CModelButeMgr* g_pModelButeMgr = LTNULL;

const LTVector defVectorReturnValue(0.0f,0.0f,0.0f);
const LTFloatPt  defNullFloatPtReturnValue = { 0.0f, 0.0f };

static char s_aTagName[30];
static char s_aAttName[100];
static char s_szBuffer[1024];

// ----------------------------------------------------------------------- //
// Defines

#define MODELBMGR_SKELETON							"Skeleton"

	#define MODELBMGR_SKELETON_NAME						"Name"
	#define MODELBMGR_SKELETON_DEFAULT_DEATH_ANI		"DefaultDeathAni"
	#define MODELBMGR_SKELETON_DEFAULT_HIT_NODE			"DefaultHitNode"

	#define MODELBMGR_SKELETON_HEAD_TURN_YAW			"HeadTurnYaw"
	#define MODELBMGR_SKELETON_HEAD_TURN_PITCH			"HeadTurnPitch"

	#define MODELBMGR_SKELETON_NODE_NAME				"Node%dName"
	#define MODELBMGR_SKELETON_NODE_DEATH_ANI			"Node%dDeathAni"
	#define MODELBMGR_SKELETON_NODE_RECOIL_ANI			"Node%dRecoilAni"
	#define MODELBMGR_SKELETON_NODE_PIECE				"Node%dPiece"
	#define MODELBMGR_SKELETON_NODE_LIMB_ANI			"Node%dLimbAni"
	#define MODELBMGR_SKELETON_NODE_GIB_TYPE			"Node%dGIBType"
	#define MODELBMGR_SKELETON_NODE_DAMAGE_FACTOR		"Node%dDamageFactor"

	#define MODELBMGR_SKELETON_NODE_MIN_EXPLODE_DAMAGE	"Node%dMinExplodeDamage"
	#define MODELBMGR_SKELETON_NODE_MIN_EXPLODE_CHANCE	"Node%dMinExplodeChance"
	#define MODELBMGR_SKELETON_NODE_MAX_EXPLODE_DAMAGE	"Node%dMaxExplodeDamage"
	#define MODELBMGR_SKELETON_NODE_MAX_EXPLODE_CHANCE	"Node%dMaxExplodeChance"

	#define MODELBMGR_SKELETON_NODE_CAN_LIVE_EXPLODE	"Node%dCanLiveExplode"
	#define MODELBMGR_SKELETON_NODE_FORCE_CRAWL			"Node%dForceCrawl"
	#define MODELBMGR_SKELETON_NODE_BLEEDER				"Node%dBleederNode"
	#define MODELBMGR_SKELETON_NODE_PARENT				"Node%dParent"
	#define MODELBMGR_SKELETON_NODE_LOCATION			"Node%dLocation"
	#define MODELBMGR_SKELETON_NODE_RADIUS				"Node%dRadius"
	#define MODELBMGR_SKELETON_NODE_STRAFE_YAW_RANGE	"Node%dStrafeYawRange%d"
	#define MODELBMGR_SKELETON_NODE_AIM_PITCH_RANGE		"Node%dAimPitchRange%d"

#define MODELBMGR_NODE_SCRIPT						"NodeScript"

	#define MODELBMGR_NSCRIPT_NODE_NAME					"NodeName"
	#define MODELBMGR_NSCRIPT_FLAGS						"Flags"

	#define MODELBMGR_NSCRIPT_NODE_TIME					"Pt%dTime"
	#define MODELBMGR_NSCRIPT_NODE_POSITION_OFFSET		"Pt%dPosOffset"
	#define MODELBMGR_NSCRIPT_NODE_ROTATION_OFFSET		"Pt%dRotOffset"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::CModelButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CModelButeMgr::CModelButeMgr()
{
	m_cSkeletons = 0;
	m_aSkeletons = LTNULL;

	m_cNScripts = 0;
	m_aNScripts = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::~CModelButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CModelButeMgr::~CModelButeMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CModelButeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	int i = 0;

	if (g_pModelButeMgr || !szAttributeFile) return LTFALSE;
	if (!Parse(pInterface, szAttributeFile)) return LTFALSE;

	// Set up global pointer

	g_pModelButeMgr = this;

	// Count the node groups

	m_cSkeletons = 0;
	sprintf(s_aTagName, "%s%d", MODELBMGR_SKELETON, m_cSkeletons);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cSkeletons++;
		sprintf(s_aTagName, "%s%d", MODELBMGR_SKELETON, m_cSkeletons);
	}

	m_aSkeletons = new CModelButeMgr::CSkeleton[m_cSkeletons];

	// Read in the nodegroups

	for ( int iSkeleton = 0 ; iSkeleton < m_cSkeletons ; iSkeleton++ )
	{
		sprintf(s_aTagName, "%s%d", MODELBMGR_SKELETON, iSkeleton);

		// Get the skeleton's name.
		m_aSkeletons[iSkeleton].m_Name = m_buteMgr.GetString(s_aTagName, MODELBMGR_SKELETON_NAME);

		// Get the skeleton's default death anis

		strcpy(m_aSkeletons[iSkeleton].m_szDefaultDeathAni, m_buteMgr.GetString(s_aTagName, MODELBMGR_SKELETON_DEFAULT_DEATH_ANI));

		// Get the skeleton's default hit node

		m_aSkeletons[iSkeleton].m_eModelNodeDefaultHit = (ModelNode)(uint8)m_buteMgr.GetInt(s_aTagName, MODELBMGR_SKELETON_DEFAULT_HIT_NODE);

		// Get the skeleton's head turn limits

		CPoint tmp_pt = m_buteMgr.GetPoint(s_aTagName, MODELBMGR_SKELETON_HEAD_TURN_YAW);

		m_aSkeletons[iSkeleton].m_fptHeadTurnYawRange.x = LTFLOAT(tmp_pt.x);
		m_aSkeletons[iSkeleton].m_fptHeadTurnYawRange.y = LTFLOAT(tmp_pt.y);

		m_aSkeletons[iSkeleton].m_fptHeadTurnYawRange.x *= MATH_PI / 180.0f;
		m_aSkeletons[iSkeleton].m_fptHeadTurnYawRange.y *= MATH_PI / 180.0f;

		tmp_pt = m_buteMgr.GetPoint(s_aTagName, MODELBMGR_SKELETON_HEAD_TURN_PITCH);

		// NOTE: The pitch is reversed becuase of our left-handed coordinate system!
		//   The butes are reversed so that in code, negative is up (as it needs
		//   to be) but in the butes negative is down.
		m_aSkeletons[iSkeleton].m_fptHeadTurnPitchRange.x = -LTFLOAT(tmp_pt.y);
		m_aSkeletons[iSkeleton].m_fptHeadTurnPitchRange.y = -LTFLOAT(tmp_pt.x);

		m_aSkeletons[iSkeleton].m_fptHeadTurnPitchRange.x *= MATH_PI / 180.0f;
		m_aSkeletons[iSkeleton].m_fptHeadTurnPitchRange.y *= MATH_PI / 180.0f;

		// Count the number of nodes

		m_aSkeletons[iSkeleton].m_cNodes = 0;
		sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_NAME, m_aSkeletons[iSkeleton].m_cNodes);

		while (m_buteMgr.Exist(s_aTagName, s_aAttName))
		{
			m_aSkeletons[iSkeleton].m_cNodes++;
			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_NAME, m_aSkeletons[iSkeleton].m_cNodes);
		}

		// Get all of our nodes

		m_aSkeletons[iSkeleton].m_aNodes = new CModelButeMgr::CNode[m_aSkeletons[iSkeleton].m_cNodes];

		for ( int iNode = 0 ; iNode < m_aSkeletons[iSkeleton].m_cNodes ; iNode++ )
		{
			// Get the node's name

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_NAME, iNode);
			strcpy(m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szName, m_buteMgr.GetString(s_aTagName, s_aAttName));

			// Get the node's death animations

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_DEATH_ANI, iNode);
			strcpy(m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szDeathAniFrag, m_buteMgr.GetString(s_aTagName, s_aAttName));

			// Get the node's respective piece

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_PIECE, iNode);
			strcpy(m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szPiece, m_buteMgr.GetString(s_aTagName, s_aAttName));

			// Get the node's limb animation

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_LIMB_ANI, iNode);
			strcpy(m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szLimbAni, m_buteMgr.GetString(s_aTagName, s_aAttName));

			// Get the node's GIB type

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_GIB_TYPE, iNode);
			strcpy(m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szGIBType, m_buteMgr.GetString(s_aTagName, s_aAttName));

			// Get the node's damage factor

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_DAMAGE_FACTOR, iNode);
			m_aSkeletons[iSkeleton].m_aNodes[iNode].m_fDamageFactor = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			// Get the node's explode damage properties

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_MIN_EXPLODE_DAMAGE, iNode);
			m_aSkeletons[iSkeleton].m_aNodes[iNode].m_fMinExplodeDamage = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_MIN_EXPLODE_CHANCE, iNode);
			m_aSkeletons[iSkeleton].m_aNodes[iNode].m_fMinExplodeChance = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_MAX_EXPLODE_DAMAGE, iNode);
			m_aSkeletons[iSkeleton].m_aNodes[iNode].m_fMaxExplodeDamage = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_MAX_EXPLODE_CHANCE, iNode);
			m_aSkeletons[iSkeleton].m_aNodes[iNode].m_fMaxExplodeChance = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			// Get the node's can live explode bool

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_CAN_LIVE_EXPLODE, iNode);
			m_aSkeletons[iSkeleton].m_aNodes[iNode].m_bCanLiveExplode = (LTBOOL)m_buteMgr.GetInt(s_aTagName, s_aAttName);

			// Get the node's force crawl bool

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_FORCE_CRAWL, iNode);
			m_aSkeletons[iSkeleton].m_aNodes[iNode].m_bForceCrawl = (LTBOOL)m_buteMgr.GetInt(s_aTagName, s_aAttName);

			// Get the node's bleeder node

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_BLEEDER, iNode);
			ModelNode eBleeder = (ModelNode)(uint8)m_buteMgr.GetInt(s_aTagName, s_aAttName);
			m_aSkeletons[iSkeleton].m_aNodes[iNode].m_eNodeBleeder = eBleeder;

			// Get the node's parent

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_PARENT, iNode);
			ModelNode eParent = (ModelNode)(uint8)m_buteMgr.GetInt(s_aTagName, s_aAttName);
			m_aSkeletons[iSkeleton].m_aNodes[iNode].m_eNodeParent = eParent;

			// Update the children information of the parent
			if((eParent >= 0) && (eParent < m_aSkeletons[iSkeleton].m_cNodes))
			{
				int nChildren = m_aSkeletons[iSkeleton].m_aNodes[eParent].m_nNumNodeChildren;

				if(nChildren < MBMGR_MAX_NODE_CHILDREN)
				{
					m_aSkeletons[iSkeleton].m_aNodes[eParent].m_eNodeChildren[nChildren] = (ModelNode)iNode;
					m_aSkeletons[iSkeleton].m_aNodes[eParent].m_nNumNodeChildren = nChildren + 1;
				}
			}


			// Get the node's hit location

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_LOCATION, iNode);
			m_aSkeletons[iSkeleton].m_aNodes[iNode].m_eHitLocation = (HitLocation)(uint8)m_buteMgr.GetInt(s_aTagName, s_aAttName, HL_UNKNOWN);

			// Get the node's hit radius

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_RADIUS, iNode);
			m_aSkeletons[iSkeleton].m_aNodes[iNode].m_fHitRadius = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			// Get the nodes strafe yaw range

			for(i = 0; i < MBMGR_MAX_STRAFE_RANGE_SETS; i++)
			{
				sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_STRAFE_YAW_RANGE, iNode, i);

				if(m_buteMgr.Exist(s_aTagName, s_aAttName))
				{
					CPoint pt = m_buteMgr.GetPoint(s_aTagName, s_aAttName);

					m_aSkeletons[iSkeleton].m_aNodes[iNode].m_ptStrafeYawRange[i].x = pt.x;
					m_aSkeletons[iSkeleton].m_aNodes[iNode].m_ptStrafeYawRange[i].y = pt.y;
				}
				else
				{
					m_aSkeletons[iSkeleton].m_aNodes[iNode].m_ptStrafeYawRange[i] = LTIntPt(0,0);
				}


				if(m_aSkeletons[iSkeleton].m_aNodes[iNode].m_ptStrafeYawRange[i].x || m_aSkeletons[iSkeleton].m_aNodes[iNode].m_ptStrafeYawRange[i].y)
				{
					if(m_aSkeletons[iSkeleton].m_nStrafeYawNodes[i] < MBMGR_MAX_STRAFE_RANGE_NODES)
					{
						m_aSkeletons[iSkeleton].m_eStrafeYawNodes[i][m_aSkeletons[iSkeleton].m_nStrafeYawNodes[i]] = (ModelNode)iNode;
						m_aSkeletons[iSkeleton].m_nStrafeYawNodes[i]++;
					}
				}
			}

			// Get the nodes aim pitch range

			for(i = 0; i < MBMGR_MAX_AIM_RANGE_SETS; i++)
			{
				sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_AIM_PITCH_RANGE, iNode, i);

				if(m_buteMgr.Exist(s_aTagName, s_aAttName))
				{
					CPoint pt = m_buteMgr.GetPoint(s_aTagName, s_aAttName);

					m_aSkeletons[iSkeleton].m_aNodes[iNode].m_ptAimPitchRange[i].x = pt.x;
					m_aSkeletons[iSkeleton].m_aNodes[iNode].m_ptAimPitchRange[i].y = pt.y;
				}
				else
				{
					m_aSkeletons[iSkeleton].m_aNodes[iNode].m_ptAimPitchRange[i] = LTIntPt(0,0);
				}


				if(m_aSkeletons[iSkeleton].m_aNodes[iNode].m_ptAimPitchRange[i].x || m_aSkeletons[iSkeleton].m_aNodes[iNode].m_ptAimPitchRange[i].y)
				{
					if(m_aSkeletons[iSkeleton].m_nAimPitchNodes[i] < MBMGR_MAX_AIM_RANGE_NODES)
					{
						m_aSkeletons[iSkeleton].m_eAimPitchNodes[i][m_aSkeletons[iSkeleton].m_nAimPitchNodes[i]] = (ModelNode)iNode;
						m_aSkeletons[iSkeleton].m_nAimPitchNodes[i]++;
					}
				}
			}
		}
	}

	// Count the scripts

	m_cNScripts = 0;
	sprintf(s_aTagName, "%s%d", MODELBMGR_NODE_SCRIPT, m_cNScripts);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cNScripts++;
		sprintf(s_aTagName, "%s%d", MODELBMGR_NODE_SCRIPT, m_cNScripts);
	}

	m_aNScripts = new CModelButeMgr::CNScript[m_cNScripts];

	// Read in the scripts

	for ( int iNScript = 0 ; iNScript < m_cNScripts ; iNScript++ )
	{
		sprintf(s_aTagName, "%s%d", MODELBMGR_NODE_SCRIPT, iNScript);

		// Get the node's name
		strcpy(m_aNScripts[iNScript].m_szName, m_buteMgr.GetString(s_aTagName, MODELBMGR_NSCRIPT_NODE_NAME));

		// Get the scripts flags
		m_aNScripts[iNScript].m_bFlags = (uint8)m_buteMgr.GetInt(s_aTagName, MODELBMGR_NSCRIPT_FLAGS);

		// Count the number of script sections
		m_aNScripts[iNScript].m_cNScriptPts = 0;
		sprintf(s_aAttName, MODELBMGR_NSCRIPT_NODE_TIME, m_aNScripts[iNScript].m_cNScriptPts);

		while (m_buteMgr.Exist(s_aTagName, s_aAttName))
		{
			m_aNScripts[iNScript].m_cNScriptPts++;
			sprintf(s_aAttName, MODELBMGR_NSCRIPT_NODE_TIME, m_aNScripts[iNScript].m_cNScriptPts);
		}

		// Get all of our script sections
		m_aNScripts[iNScript].m_aNScriptPts = new CModelButeMgr::CNScriptPt[m_aNScripts[iNScript].m_cNScriptPts];

		for ( int iNScriptPt = 0 ; iNScriptPt < m_aNScripts[iNScript].m_cNScriptPts ; iNScriptPt++ )
		{
			// Get the node's time
			sprintf(s_aAttName, MODELBMGR_NSCRIPT_NODE_TIME, iNScriptPt);
			m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_fTime = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			// Each time must be greater than the previous... and the first one must be zero
			if(iNScriptPt > 0)
				_ASSERT(m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_fTime > m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt - 1].m_fTime);
			else
				_ASSERT(m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_fTime == 0.0f);

			// Get the node's offset position
			sprintf(s_aAttName, MODELBMGR_NSCRIPT_NODE_POSITION_OFFSET, iNScriptPt);
			m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_vPosOffset = m_buteMgr.GetVector(s_aTagName, s_aAttName);
			
			// Get the node's offset rotation
			sprintf(s_aAttName, MODELBMGR_NSCRIPT_NODE_ROTATION_OFFSET, iNScriptPt);
			m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_vRotOffset = m_buteMgr.GetVector(s_aTagName, s_aAttName);

			m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_vRotOffset.x *= (MATH_PI / 180.0f);
			m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_vRotOffset.y *= (MATH_PI / 180.0f);
			m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_vRotOffset.z *= (MATH_PI / 180.0f);
		}
	}

	// Free up butemgr's memory and what-not.

	m_buteMgr.Term();

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CModelButeMgr::Term()
{
	if (m_aSkeletons)
	{
		for ( int iSkeleton = 0 ; iSkeleton < m_cSkeletons ; iSkeleton++ )
		{
			delete [] m_aSkeletons[iSkeleton].m_aNodes;
		}

		delete [] m_aSkeletons;
		m_aSkeletons = LTNULL;
	}

	if (m_aNScripts)
	{
		for ( int iNScript = 0 ; iNScript < m_cNScripts ; iNScript++ )
		{
			delete [] m_aNScripts[iNScript].m_aNScriptPts;
		}

		delete [] m_aNScripts;
		m_aNScripts = LTNULL;
	}

	g_pModelButeMgr = LTNULL;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetSkeletonFromName
//
//	PURPOSE:	Finds the skeleton index for a given name.
//
// ----------------------------------------------------------------------- //
ModelSkeleton	CModelButeMgr::GetSkeletonFromName(const char * szName)
{
	for( int i = 0; i < m_cSkeletons; ++i )
	{
		if( stricmp( m_aSkeletons[i].m_Name, szName ) == 0 )
		{
			return ModelSkeleton(i);
		}
	}

	return eModelSkeletonInvalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetNameFromSkeleton
//
//	PURPOSE:	Finds the skeleton index for a given name.
//
// ----------------------------------------------------------------------- /
CString CModelButeMgr::GetNameFromSkeleton(ModelSkeleton eModelSkeleton)
{
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons ) return CString("");

	return m_aSkeletons[eModelSkeleton].m_Name;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetSkeleton...
//
//	PURPOSE:	Various skeleton attribute lookups
//
// ----------------------------------------------------------------------- //

int CModelButeMgr::GetSkeletonNumNodes(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);

	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons ) return 0;

	return m_aSkeletons[eModelSkeleton].m_cNodes;	
}

ModelNode CModelButeMgr::GetSkeletonNode(ModelSkeleton eModelSkeleton, const char* szName)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);

	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons ) return eModelNodeInvalid;


	for ( int iNode = 0 ; iNode < m_aSkeletons[eModelSkeleton].m_cNodes ; iNode++ )
	{
		if ( !_stricmp(m_aSkeletons[eModelSkeleton].m_aNodes[iNode].m_szName, szName) )
		{
			return (ModelNode)iNode;
		}
	}

	return eModelNodeInvalid;
}

const char* CModelButeMgr::GetSkeletonDefaultDeathAni(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);

	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons ) return 0;

	return m_aSkeletons[eModelSkeleton].m_szDefaultDeathAni;
}

ModelNode CModelButeMgr::GetSkeletonDefaultHitNode(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);

	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons ) return eModelNodeInvalid;

	return m_aSkeletons[eModelSkeleton].m_eModelNodeDefaultHit;
}


const LTFloatPt & CModelButeMgr::GetSkeletonHeadTurnYawRange(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);

	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons ) return defNullFloatPtReturnValue;

	return m_aSkeletons[eModelSkeleton].m_fptHeadTurnYawRange;
}

const LTFloatPt & CModelButeMgr::GetSkeletonHeadTurnPitchRange(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);

	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons ) return defNullFloatPtReturnValue;

	return m_aSkeletons[eModelSkeleton].m_fptHeadTurnPitchRange;
}

int CModelButeMgr::GetNumStrafeYawNodes(ModelSkeleton eModelSkeleton, int nSet)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);

	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons ) return 0;
	if( nSet < 0 || nSet >= MBMGR_MAX_STRAFE_RANGE_SETS ) return 0;

	return m_aSkeletons[eModelSkeleton].m_nStrafeYawNodes[nSet];
}

const ModelNode* CModelButeMgr::GetStrafeYawNodes(ModelSkeleton eModelSkeleton, int nSet)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);

	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons ) return 0;
	if( nSet < 0 || nSet >= MBMGR_MAX_STRAFE_RANGE_SETS ) return 0;

	return m_aSkeletons[eModelSkeleton].m_eStrafeYawNodes[nSet];
}

int CModelButeMgr::GetNumAimPitchNodes(ModelSkeleton eModelSkeleton, int nSet)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);

	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons ) return 0;
	if( nSet < 0 || nSet >= MBMGR_MAX_AIM_RANGE_SETS ) return 0;

	return m_aSkeletons[eModelSkeleton].m_nAimPitchNodes[nSet];
}

const ModelNode* CModelButeMgr::GetAimPitchNodes(ModelSkeleton eModelSkeleton, int nSet)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);

	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons ) return 0;
	if( nSet < 0 || nSet >= MBMGR_MAX_AIM_RANGE_SETS ) return 0;

	return m_aSkeletons[eModelSkeleton].m_eAimPitchNodes[nSet];
}


// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //


const char* CModelButeMgr::GetSkeletonNodeName(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{

	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons ) return 0;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return 0;


	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szName;
}

const char* CModelButeMgr::GetSkeletonNodeDeathAniFrag(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return 0;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return 0;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szDeathAniFrag;
}

const char* CModelButeMgr::GetSkeletonNodePiece(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return 0;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return 0;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szPiece;
}

const char* CModelButeMgr::GetSkeletonNodeLimbAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return 0;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return 0;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szLimbAni;
}

const char* CModelButeMgr::GetSkeletonNodeGIBType(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return 0;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return 0;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szGIBType;
}

LTFLOAT CModelButeMgr::GetSkeletonNodeDamageFactor(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return 1.0f;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return 1.0f;

#ifndef _CLIENTBUILD

	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		if(g_pGameServerShell->GetGameInfo()->m_bLocationDamage)
			return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fDamageFactor;
		else
			return 1.0f;
	}

#endif

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fDamageFactor;
}

LTBOOL CModelButeMgr::ExplodeNode(ModelSkeleton eModelSkeleton, ModelNode eModelNode, LTFLOAT fDamage)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return LTFALSE;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return LTFALSE;


	LTBOOL bExplode = LTFALSE;

	LTFLOAT fMinDamage = m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fMinExplodeDamage;
	LTFLOAT fMaxDamage = m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fMaxExplodeDamage;

	LTFLOAT fMinChance = m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fMinExplodeChance;
	LTFLOAT fMaxChance = m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fMaxExplodeChance;

	if((fMinChance > 0.0f) || (fMaxChance > 0.0f))
	{
		if(fDamage >= fMinDamage)
		{
			if(fDamage >= fMaxDamage)
			{
				bExplode = (GetRandom(0.0f, 1.0f) <= fMaxChance);
			}
			else
			{
				LTFLOAT fPercent = fDamage / (fMinDamage + fMaxDamage);
				LTFLOAT fChance = fMinChance + ((fMaxChance - fMinChance) * fPercent);

				bExplode = (GetRandom(0.0f, 1.0f) <= fChance);
			}
		}
	}

	return bExplode;
}

LTFLOAT CModelButeMgr::GetSkeletonNodeMinExplodeDamage(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return 1.0f;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return 1.0f;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fMinExplodeDamage;
}

LTFLOAT CModelButeMgr::GetSkeletonNodeMinExplodeChance(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return 1.0f;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return 1.0f;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fMinExplodeChance;
}

LTFLOAT CModelButeMgr::GetSkeletonNodeMaxExplodeDamage(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return 1.0f;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return 1.0f;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fMaxExplodeDamage;
}

LTFLOAT CModelButeMgr::GetSkeletonNodeMaxExplodeChance(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return 1.0f;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return 1.0f;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fMaxExplodeChance;
}

LTBOOL CModelButeMgr::GetSkeletonNodeCanLiveExplode(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return LTFALSE;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return LTFALSE;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_bCanLiveExplode;
}

LTBOOL CModelButeMgr::GetSkeletonNodeForceCrawl(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return LTFALSE;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return LTFALSE;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_bForceCrawl;
}

ModelNode CModelButeMgr::GetSkeletonNodeBleeder(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return eModelNodeInvalid;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return eModelNodeInvalid;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_eNodeBleeder;
}

ModelNode CModelButeMgr::GetSkeletonNodeParent(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return eModelNodeInvalid;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return eModelNodeInvalid;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_eNodeParent;
}

HitLocation CModelButeMgr::GetSkeletonNodeHitLocation(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return HL_UNKNOWN;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return HL_UNKNOWN;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_eHitLocation;
}

LTFLOAT CModelButeMgr::GetSkeletonNodeHitRadius(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return 0.0f;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return 0.0f;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fHitRadius;
}

LTIntPt CModelButeMgr::GetSkeletonNodeStrafeYawRange(ModelSkeleton eModelSkeleton, ModelNode eModelNode, int nSet)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return LTIntPt(0,0);

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return LTIntPt(0,0);

	if( nSet < 0 || nSet >= MBMGR_MAX_STRAFE_RANGE_SETS ) return LTIntPt(0,0);

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_ptStrafeYawRange[nSet];
}

LTIntPt CModelButeMgr::GetSkeletonNodeAimPitchRange(ModelSkeleton eModelSkeleton, ModelNode eModelNode, int nSet)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return LTIntPt(0,0);

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return LTIntPt(0,0);

	if( nSet < 0 || nSet >= MBMGR_MAX_AIM_RANGE_SETS ) return LTIntPt(0,0);

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_ptAimPitchRange[nSet];
}



int CModelButeMgr::GetSkeletonNodeNumChildren(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return 0;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return 0;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_nNumNodeChildren;
}

ModelNode* CModelButeMgr::GetSkeletonNodeChildren(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if( eModelSkeleton < 0 || eModelSkeleton >= m_cSkeletons) return LTNULL;

	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if( eModelNode < 0 || eModelNode >= m_aSkeletons[eModelSkeleton].m_cNodes) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_eNodeChildren;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetNScript...
//
//	PURPOSE:	Various node script attribute lookups
//
// ----------------------------------------------------------------------- //

int CModelButeMgr::GetNScriptNumPts(ModelNScript eModelNScript)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);
	if( eModelNScript < 0 || eModelNScript >- m_cNScripts) return 0;

	return m_aNScripts[eModelNScript].m_cNScriptPts;	
}

const char* CModelButeMgr::GetNScriptNodeName(ModelNScript eModelNScript)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);
	if( eModelNScript < 0 || eModelNScript >- m_cNScripts) return 0;

	return m_aNScripts[eModelNScript].m_szName;
}

uint8 CModelButeMgr::GetNScriptFlags(ModelNScript eModelNScript)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);
	if( eModelNScript < 0 || eModelNScript >- m_cNScripts) return 0;

	return m_aNScripts[eModelNScript].m_bFlags;
}

LTFLOAT CModelButeMgr::GetNScriptPtTime(ModelNScript eModelNScript, int nPt)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);
	if( eModelNScript < 0 || eModelNScript >- m_cNScripts) return 0.0f;

	_ASSERT(nPt >= 0 && nPt < m_aNScripts[eModelNScript].m_cNScriptPts);
	if( nPt < 0 || nPt >= m_aNScripts[eModelNScript].m_cNScriptPts ) return 0.0f;


	return m_aNScripts[eModelNScript].m_aNScriptPts[nPt].m_fTime;
}

const LTVector& CModelButeMgr::GetNScriptPtPosOffset(ModelNScript eModelNScript, int nPt)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);
	if( eModelNScript < 0 || eModelNScript >- m_cNScripts) return defVectorReturnValue;

	_ASSERT(nPt >= 0 && nPt < m_aNScripts[eModelNScript].m_cNScriptPts);
	if( nPt < 0 || nPt >= m_aNScripts[eModelNScript].m_cNScriptPts ) return defVectorReturnValue;

	return m_aNScripts[eModelNScript].m_aNScriptPts[nPt].m_vPosOffset;
}

const LTVector& CModelButeMgr::GetNScriptPtRotOffset(ModelNScript eModelNScript, int nPt)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);
	if( eModelNScript < 0 || eModelNScript >- m_cNScripts) return defVectorReturnValue;

	_ASSERT(nPt >= 0 && nPt < m_aNScripts[eModelNScript].m_cNScriptPts);
	if( nPt < 0 || nPt >= m_aNScripts[eModelNScript].m_cNScriptPts ) return defVectorReturnValue;

	return m_aNScripts[eModelNScript].m_aNScriptPts[nPt].m_vRotOffset;
}



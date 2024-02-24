// ----------------------------------------------------------------------- //
//
// MODULE  : NodeController.h
//
// PURPOSE : NodeController definition
//
// CREATED : 05.20.1999
//
// ----------------------------------------------------------------------- //

#ifndef __NODE_CONTROLLER_H__
#define __NODE_CONTROLLER_H__

#include "ModelDefs.h"

#include <map>

class CCharacterFX;
class BodyPropFX;

#define MAX_NODES				(60)
#define MAX_NODECONTROLS		(32)
#define MAX_RECOILS				(2)


enum Control
{
	eControlInvalid = -1,
	eControlRotationTimed,
	eControlLipSync,
	eControlScript,
	eControlHeadFollowObj,
	eControlHeadFollowPos,
	eControlAimingPitch,
	eControlAimingYaw,
	eControlStrafeYaw,
};


// Our node control struct

typedef LTFLOAT (*RotationFn)(LTFLOAT fDistance, LTFLOAT fTimer, LTFLOAT fDuration);

typedef struct NCSTRUCT
{
	NCSTRUCT()
	{
		bValid = LTFALSE;
		eModelNode = eModelNodeInvalid;
		eControl = eControlInvalid;

		vRotationAxis.Init();
		fRotationDistance = 0.0f;
		fRotationDuration = 0.0f;
		fRotationTimer = 0.0f;
		fnRotationFunction = LTNULL;

		hFollowObj = LTNULL;
		hFollowObjNode = INVALID_MODEL_NODE;

		vFollowPos.Init();

		fFollowRate = 0.0f;
		vFollowAngles.Init();
		fFollowExpireTime = 0.0f;
		bFollowOn = LTFALSE;

		hLipSyncSound = LTNULL;
		pSixteenBitBuffer = LTNULL;
		pEightBitBuffer = LTNULL;
		dwSamplesPerSecond = 0;

		fScriptTime = 0.0f;
		nScript = eModelNScriptInvalid;
		nCurrentScriptPt = 0;
		nLastScriptPt = 0;

		fPitch = 0.0f;
		fCurPitch = 0.0f;
		nPitchSet = 0xFF;

		fYaw = 0.0f;
		fCurYaw = 0.0f;

		vLastStrafePos.Init();
		fLastStrafeAngle = 0.0f;
		nStrafeSet = 0xFF;
	}

	// Members used by all control methods
	LTBOOL		bValid;			// Is the control active or not?
	ModelNode	eModelNode;		// What model node does it control?
	Control		eControl;		// What type of control is this?

	// Timed Rotation members
	LTVector	vRotationAxis;			// What axis are we rotating around?
	LTFLOAT		fRotationDistance;		// How far (in radians) will we rotate maximally?
	LTFLOAT		fRotationDuration;		// How long does the rotation last?
	LTFLOAT		fRotationTimer;			// How far into the rotation are we?
	RotationFn	fnRotationFunction;		// The function that maps time to radians

	// Head follow object rotation members
	HOBJECT		hFollowObj;
	HMODELNODE	hFollowObjNode;

	// Head follow position rotation members
	LTVector	vFollowPos;

	// General head follow members
	LTFLOAT		fFollowRate;
	LTVector	vFollowAngles;
	LTFLOAT		fFollowExpireTime;
	LTBOOL		bFollowOn;

	// Sound rotation members
	HLTSOUND	hLipSyncSound;
	int16*		pSixteenBitBuffer;
	int8*		pEightBitBuffer;
	uint32		dwSamplesPerSecond;

	// Script members
	LTFLOAT		fScriptTime;
	uint8		nScript;
	int			nCurrentScriptPt;
	int			nLastScriptPt;

	// Aiming pitch members
	LTFLOAT		fPitch;
	LTFLOAT		fCurPitch;
	uint8		nPitchSet;

	// Aiming yaw members
	LTFLOAT		fYaw;
	LTFLOAT		fCurYaw;

	// Strafe control members
	LTVector	vLastStrafePos;
	LTFLOAT		fLastStrafeAngle;
	uint8		nStrafeSet;


}	NCSTRUCT;


// Our node struct

typedef struct NSTRUCT
{
	NSTRUCT()
	{
		hModelNode = INVALID_MODEL_NODE;
		eModelNode = eModelNodeInvalid;
		ROT_INIT(rRot);
		origMatTransform.Identity();
		matTransform.Identity();
		cControllers = 0;
	}

	HMODELNODE	hModelNode;
	ModelNode	eModelNode;
	LTRotation	rRot;
	LTMatrix	origMatTransform;
	LTMatrix	matTransform;
	int			cControllers;
}
NSTRUCT;

class CNodeController
{

	protected : // Protected inner classes

		class CNode;
		class CNodeControl;

		typedef std::map<HMODELNODE,NSTRUCT*> NodeIndexTable;

	public : // Public methods

		// Ctors/dtors/etc
		CNodeController();
		~CNodeController();

		LTBOOL	Init(CCharacterFX* pCFX, BodyPropFX* pBPFX, LTBOOL bLocalPlayer);
		void	Term();

		void	Reset();

		void	ResetControlType(Control eType);


		// Simple accessors
		HOBJECT	GetServerObj() { return m_hServerObj; }
		
		// Updates
		void	Update();
		void	UpdateRotationTimed(NCSTRUCT *pNodeControl);
		void	UpdateLipSyncControl(NCSTRUCT *pNodeControl);
		void	UpdateScriptControl(NCSTRUCT *pNodeControl);
		void	UpdateHeadFollowObjControl(NCSTRUCT *pNodeControl);
		void	UpdateHeadFollowPosControl(NCSTRUCT *pNodeControl);
		void	UpdateAimingPitchControl(NCSTRUCT *pNodeControl);
		void	UpdateAimingYawControl(NCSTRUCT *pNodeControl);
		void	UpdateStrafeYawControl(NCSTRUCT *pNodeControl);

		// Message handlers
		LTBOOL	HandleNodeControlMessage(uint8 byMsgId, HMESSAGEREAD hMessage);
		void	HandleNodeControlRecoilMessage(HMESSAGEREAD hMessage);
		void	HandleNodeControlLipSyncMessage(HMESSAGEREAD hMessage);
		void	HandleNodeControlScriptMessage(HMESSAGEREAD hMessage);
		void	HandleNodeControlHeadFollowObjMessage(HMESSAGEREAD hMessage);
		void	HandleNodeControlHeadFollowPosMessage(HMESSAGEREAD hMessage);
		void	HandleNodeControlAimingPitchMessage(HMESSAGEREAD hMessage);
		void	HandleNodeControlAimingYawMessage(HMESSAGEREAD hMessage);
		void	HandleNodeControlStrafeYawMessage(HMESSAGEREAD hMessage);

		void	OnObjectRemove(HOBJECT hObj);

		// Node methods
		NSTRUCT*	FindNode(HMODELNODE hModelNode);
		int			FindNodeControl(ModelNode eNode, Control eControl);

		// Node control methods
		int		AddNodeControl();
		void	AddNodeControlRotationTimed(ModelNode eModelNode, const LTVector& vAxis, LTFLOAT fDistance, LTFLOAT fDuration);

		void	RemoveNodeControl(int iNodeControl);

		// Node control callback methods
		static void NodeControlFn(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat, void *pUserData);
		void		HandleNodeControl(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat);

	private :

		void RotateHeadToPos(NCSTRUCT *pNodeControl, const LTVector & vObjPos);
		HSOUNDDE PlayLipSyncSound(char* szSound, LTFLOAT fRadius, uint32 dwFlags);

	private :

		HOBJECT			m_hServerObj;					// Our handle to the server object.
		ModelSkeleton	m_eModelSkeleton;				// Our model skeleton.

		int			m_cNodeControls;					// The number of active node controls
		NCSTRUCT	m_aNodeControls[MAX_NODECONTROLS];	// The node controls
		int			m_cNodes;							// The number of nodes
		NSTRUCT		m_aNodes[MAX_NODES];				// The nodes

		int			m_cRecoils;							// How many recoils we're controlling
		LTFLOAT		m_fRecoilTimers[MAX_RECOILS];		// Our recoil timers

		LTFLOAT		m_fCurLipSyncRot;					// Current lip-sync node rotation
		LTBOOL		m_bLocalPlayer;						// Are we using character FX?

		LTBOOL		m_bUpdated;							// Set to true when the node control matrices have changed.  
														// Set to false by the node control check function.

		NodeIndexTable m_NodeIndex;						// Correlates HMODELNODE-s to NSTRUCT*-s.  For faster look-up by HandleNodeControl.
};

#endif 


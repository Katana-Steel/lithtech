// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterHitBox.h
//
// PURPOSE : Character hit box object class definition
//
// CREATED : 01/05/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_HIT_BOX_H__
#define __CHARACTER_HIT_BOX_H__

#include "GameBase.h"
#include "ModelDefs.h"
#include "ltsmartlink.h"

#include <vector>
#include <list>

// ----------------------------------------------------------------------- //

class CProjectile;
class CAttachments;
class CCharacter;

// ----------------------------------------------------------------------- //

class CCharacterHitBox : public GameBase
{
	public :
	
		// ----------------------------------------------------------------------- //

		struct HitNode
		{
			const char * szName;
			HMODELNODE	 hNode;
			HMODELSOCKET hSocket;
			HMODELPIECE  hPiece;
			LTFLOAT		 fRadiusSqr;


			HitNode()
				: szName(LTNULL),
				  hNode(INVALID_MODEL_NODE),
				  hSocket(INVALID_MODEL_SOCKET),
				  hPiece(INVALID_MODEL_PIECE),
				  fRadiusSqr(0.0f)  { }
		};

		typedef std::vector<HitNode> HitNodeList;

		// ----------------------------------------------------------------------- //

#ifndef _FINAL
		struct NodeRadiusStruct
		{
			NodeRadiusStruct(const LTSmartLink & model, const HitNode & hit_node)
				: hModel(model),
				  hitNode(hit_node) {}

			LTSmartLink hModel;
			HitNode     hitNode;
		};

		typedef std::list<NodeRadiusStruct> NodeRadiusList;
#endif

		// ----------------------------------------------------------------------- //

	public :

		CCharacterHitBox();
		virtual ~CCharacterHitBox();

		LTBOOL Init(HOBJECT hModel);

		void  Update();
		void  ResetSkeleton();

		LTSmartLink	GetModelObject() const					{ return m_hModel; }
		void		SetOffset(LTVector vOffset)				{ m_vOffset = vOffset; }
		void		SetCanBeHit(LTBOOL val)					{ m_bCanBeHit = val; }
		LTBOOL		CanSeeThrough()							{ return LTTRUE; }

		void		SetUseOverrideDims(LTBOOL bDims)		{ m_bOverrideDims = bDims; }
		void		SetOverrideDims(LTVector vDims)			{ m_vOverrideDims = vDims; }

		LTBOOL		DidProjectileImpact(const CProjectile* pProjectile);
		void		HandleForcedImpact(CProjectile* pProj, ModelNode& eNodeHit, LTVector* pvNodePos);

		LTBOOL HandleImpact(CProjectile* pProj, IntersectInfo * piInfo, 
			LTVector vDir, LTVector * pvFrom, LTVector* pvNodePos = LTNULL);

		// ----------------------------------------------------------------------- //

		virtual DDWORD	EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT lData);
		virtual DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private :

		// ----------------------------------------------------------------------- //

#ifndef _FINAL
		LTVector		GetBoundingBoxColor();
#endif
		LTBOOL			HandleVectorImpact(CProjectile* pProj, const IntersectInfo& iInfo, const LTVector& vDir,
							LTVector * pvFrom, ModelNode* peModelNode, LTVector* pvNodePos);
		LTBOOL			HandleBoxImpact(CProjectile* pProj, const IntersectInfo& iInfo, const LTVector& vDir,
							LTVector * pvFrom, ModelNode* peModelNode, LTVector* pvNodePos);

		LTBOOL			UsingHitDetection();
		void			SetModelNodeLastHit(ModelNode eModelNode);

		ModelSkeleton	GetModelSkeleton();
		LTFLOAT			GetNodeRadius(ModelSkeleton eModelSkeleton, ModelNode eModelNode);

	private :

		// ----------------------------------------------------------------------- //

#ifndef _FINAL
		void			CreateNodeRadiusModels();
		void			RemoveNodeRadiusModels();
		void			UpdateNodeRadiusModels();
#endif

		void			Save(HMESSAGEREAD hRead);
		void			Load(HMESSAGEWRITE hWrite);

		// ----------------------------------------------------------------------- //

	private :

		LTSmartLink		m_hModel;
		CCharacter		*m_pCharacter;
		LTVector		m_vOffset;
		LTVector		m_vModelDims;
		LTBOOL			m_bCanBeHit;

		LTBOOL			m_bOverrideDims;
		LTVector		m_vOverrideDims;

		HitNodeList		m_HitNodes;

#ifndef _FINAL
		// Debugging helper...
		NodeRadiusList	m_NodeRadiusList;
#endif
};

// ----------------------------------------------------------------------- //

#endif  // __CHARACTER_HIT_BOX_H__


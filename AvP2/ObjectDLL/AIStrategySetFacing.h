// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategySetFacing.h
//
// PURPOSE : AI strategy class definitions
//
// CREATED : 9.9.2000
//
// ----------------------------------------------------------------------- //

#ifndef __AI_STRATEGY_SET_FACING_H__
#define __AI_STRATEGY_SET_FACING_H__

#include "float.h"
#include "ltsmartlink.h"

#include "AIStrategy.h"


class CAIStrategySetFacing : public CAIStrategy
{
	public :

		CAIStrategySetFacing(CAI * pAI, bool use_weapon_pos = false)
			: CAIStrategy(pAI),
			  m_bUseWeaponPosition(use_weapon_pos),
			  m_eFacingType(None),
			  m_hFaceObject(LTNULL),
			  m_vFacePos(FLT_MAX,FLT_MAX,FLT_MAX),
			  m_vWeaponOffset(0,0,0),
			  m_bFinishedFacing(true) {}

		virtual ~CAIStrategySetFacing() {}

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		virtual void Update();

		bool  Finished() const { return m_bFinishedFacing; }

		void UseWeaponPosition(LTBOOL val) { m_bUseWeaponPosition = val; }

		void Clear() { m_eFacingType = None; m_bFinishedFacing = true;}

		void FaceTarget() { m_eFacingType = Target; m_bFinishedFacing = false;}
		void Face(const LTSmartLink & hObject) { m_eFacingType = Object; m_hFaceObject = hObject; m_bFinishedFacing = false; }
		void Face(const LTVector & vPos) { m_eFacingType = Position; m_vFacePos = vPos; m_bFinishedFacing = false; }

		bool IsFacingTarget() const { return Target == m_eFacingType ; }
		bool IsFacingObject() const { return Object == m_eFacingType ; }
		bool IsFacingPos() const    { return Position == m_eFacingType ; }

		bool IsFacing(const LTSmartLink & hObject) const { return Target == m_eFacingType && m_hFaceObject == hObject; }
		bool IsFacing(const LTVector & vPos) const { return Position == m_eFacingType && m_vFacePos == vPos; }

		const LTVector & GetFacingPos() const { return m_vFacePos; }
		const LTSmartLink & GetFacingObject() const { return m_hFaceObject; }

	private :

		LTBOOL m_bUseWeaponPosition;

		enum FacingType
		{
			Target,
			Object,
			Position,
			None
		};

		FacingType m_eFacingType;

		LTSmartLink m_hFaceObject;
		LTVector m_vFacePos;

		LTVector m_vWeaponOffset;

		bool    m_bFinishedFacing;
};


#endif // __AI_STRATEGY_SET_FACING_H__

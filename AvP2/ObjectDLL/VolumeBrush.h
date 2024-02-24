// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrush.h
//
// PURPOSE : VolumeBrush definition
//
// CREATED : 1/29/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VOLUME_BRUSH_H__
#define __VOLUME_BRUSH_H__

#include "cpp_engineobjects_de.h"
#include "ContainerCodes.h"
#include "DamageTypes.h"

class VolumeBrush : public BaseClass
{
	public :

		VolumeBrush();
		~VolumeBrush();

		LTFLOAT	GetLiquidGravity()	const { return m_fLiquidGravity; }
		LTFLOAT	GetFriction()		const { return m_fFriction; }
		LTFLOAT	GetViscosity()		const { return m_fViscosity; }
		ContainerCode GetCode()		const { return m_eContainerCode; }
		LTVector	GetCurrent()		const { return m_vCurrent; }
		LTBOOL	GetHidden()			const { return m_bHidden; }

	protected :

		uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		virtual void ReadProp(ObjectCreateStruct *pStruct);
		virtual void UpdatePhysics(ContainerPhysics* pCPStruct);
		virtual void UpdateLiquidPhysics(ContainerPhysics* pCPStruct);

		virtual void WriteSFXMsg(HMESSAGEWRITE hMessage);
		virtual void HandleTrigger(HOBJECT hSender, HSTRING hMsg);

		HSTRING			m_hstrSurfaceSprite;
		HOBJECT			m_hSurfaceObj;

		LTVector		m_vCurrent;
		LTVector		m_vSurfaceColor1;
		LTVector		m_vSurfaceColor2;
		LTVector		m_vLastPos;
		LTVector		m_vFogColor;
		LTVector		m_vTintColor;
		LTVector		m_vLightAdd;
		LTFLOAT			m_fViscosity;
		LTFLOAT			m_fFriction;
		LTFLOAT			m_fDamage;
		LTFLOAT			m_fSurfaceHeight;
		LTFLOAT			m_fLiquidGravity;
		LTFLOAT			m_fXScaleMin;
		LTFLOAT			m_fXScaleMax;
		LTFLOAT			m_fYScaleMin;
		LTFLOAT			m_fYScaleMax;
		LTFLOAT			m_fXScaleDuration;
		LTFLOAT			m_fYScaleDuration;
		LTBOOL			m_bPanWithCurrent;
		LTFLOAT			m_fXPan;
		LTFLOAT			m_fYPan;
		LTFLOAT			m_fFrequency;
		LTFLOAT			m_fMinFogFarZ;
		LTFLOAT			m_fMinFogNearZ;
		LTFLOAT			m_fFogFarZ;
		LTFLOAT			m_fFogNearZ;
		LTFLOAT			m_fSurfAlpha;
		uint32			m_dwNumSurfPolies;
		uint32			m_dwFlags;
		uint32			m_dwSaveFlags;

		DamageType		m_eDamageType;
		ContainerCode	m_eContainerCode;
		LTBOOL			m_bShowSurface;
		LTBOOL			m_bFogEnable;
		LTBOOL			m_bHidden;
		LTBOOL			m_bAdditive;
		LTBOOL			m_bMultiply;

		uint8			m_nSfxMsgId;

		LTBOOL			m_bStartOn;
		LTBOOL			m_bDamageCharactersOnly;
		LTBOOL			m_bDamagePlayersOnly;

	private :

		void CreateSurface();
		void CreateSpecialFXMsg();

		void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		void CacheFiles();

		void PostPropRead(ObjectCreateStruct *pStruct);
		void InitialUpdate();
		void Update();
};

#endif // __VOLUME_BRUSH_H__

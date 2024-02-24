
#ifndef __SPRINKLESFX_H__
#define __SPRINKLESFX_H__


	#include "specialfx.h"
	#include "client_de.h"


	#define MAX_FX_TYPES	8


	class SPRINKLESCREATESTRUCT : public SFXCREATESTRUCT
	{
	public:
		HMESSAGEREAD	m_hMessage;
	};

	
	class FXType;

	
	typedef enum
	{
		FXObj_Particle,
		FXObj_Model
	} FXObjType;


	class FXObj
	{
	public:

		BOOL		IsValid()	{return !!m_pParticle;}

	
	public:
				
		union
		{
			DEParticle	*m_pParticle;
			HOBJECT		m_hModel;
		};

		DVector		m_Angles;
		DVector		m_AnglesVel;

		DVector		m_Velocity;
		DLink		m_Link;
	};


	// These abstract the differences between model and particle sprinkles.
	class FXObjControl
	{
	public:

		virtual void	DebugPrint(FXType *pType, FXObj *pObj)=0;
		virtual	DVector	GetObjPos(FXType *pType, FXObj *pObj)=0;
		virtual void	SetObjPos(FXType *pType, FXObj *pObj, DVector pos)=0;

		virtual void	SetObjAlpha(FXObj *pObj, float alpha)=0;
		virtual void	SetObjColor(FXObj *pObj, DVector &color)=0;
		
		virtual void	ShowObj(FXObj *pObj)=0;
		virtual void	HideObj(FXObj *pObj)=0;
	};


	class FXType
	{
	public:

					FXType();
					~FXType();


		void		AddFreeObj(FXObj *pObj);
		FXObj*		PopFreeObj();
	

	public:

		FXObjType		m_ObjType;
		FXObjControl	*m_pControl;

		// Maximum angular velocity (goes from -X to X).
		DVector		m_AnglesVel;

		float		m_SpawnRadius;	// Radius around the player they spawn in from.
		float		m_Speed;		// How fast they travel.

		DVector		m_ColorMin;
		DVector		m_ColorMax;
		
		// Particle system.
		HOBJECT		m_hObject;
		
		// Particles.
		FXObj		*m_pObjects;
		DDWORD		m_nObjects;

		DLink		m_ActiveList;
		DLink		m_InactiveList;
	};


	class SprinklesFX : public CSpecialFX
	{
	public:

		virtual DBOOL	Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual DBOOL	Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL	Update();

	
	public:

		FXType			m_FXTypes[MAX_FX_TYPES];
		DDWORD			m_nFXTypes;
	};


//-------------------------------------------------------------------------------------------------
// SFX_SprinklesFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_SprinklesFactory : public CSpecialFXFactory
{
	SFX_SprinklesFactory() : CSpecialFXFactory(SFX_SPRINKLES_ID) {;}
	static const SFX_SprinklesFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif




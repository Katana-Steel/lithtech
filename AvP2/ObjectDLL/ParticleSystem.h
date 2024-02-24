// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleSystem.h
//
// PURPOSE : ParticleSystem - Definition
//
// CREATED : 10/23/97
//
// ----------------------------------------------------------------------- //

#ifndef __PARTICLE_SYSTEM_H__
#define __PARTICLE_SYSTEM_H__

#include "ClientSFX.h"

namespace ObjectDLL
{

class ParticleSystem : public CClientSFX
{
	public :

		ParticleSystem();
		~ParticleSystem();

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
		void	 HandleMsg(HOBJECT hSender, HSTRING hMsg);

	private :

		DBOOL	m_bOn;

		DDWORD	m_dwFlags;
		DFLOAT	m_fBurstWait;
		DFLOAT	m_fBurstWaitMin;
		DFLOAT	m_fBurstWaitMax;
		DFLOAT	m_fParticlesPerSecond;
		DFLOAT	m_fParticleLifetime;
		DFLOAT	m_fRadius;
		DFLOAT	m_fGravity;
		DFLOAT	m_fRotationVelocity;
		DFLOAT	m_fViewDist;
		HSTRING	m_hstrTextureName;
		DVector	m_vColor1;
		DVector	m_vColor2;
		DVector	m_vDims;
		DVector	m_vMinVel;
		DVector	m_vMaxVel;

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void CacheFiles();

		DBOOL ReadProp(ObjectCreateStruct *pStruct);
		void InitialUpdate(int nInfo);
};

}	// end namespace ObjectDLL

using namespace ObjectDLL;

#endif // __POLY_GRID_H__

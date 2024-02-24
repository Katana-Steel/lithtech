// -------------------------------------------------------------------- //
//
// MODULE  : IKChain.h
//
// PURPOSE : A contrained IK chain
//
// CREATED : 5/30/01
//
// -------------------------------------------------------------------- //

#ifndef __IK_CHAIN_H__
#define __IK_CHAIN_H__

// -------------------------------------------------------------------- //

class IKChain_Constrained
{
	public:

		// -------------------------------------------------------------------- //
		// Construction and destruction

		IKChain_Constrained();
		~IKChain_Constrained();

		LTBOOL		Init(int nSegments, LTFLOAT fSegmentLength, LTFLOAT fConstraint, LTFLOAT fCorrectionRate);
		void		Term();


		// -------------------------------------------------------------------- //
		// Control functions

		LTBOOL		Update(LTVector vPos, LTVector vDir, LTFLOAT fFrameTime);


		// -------------------------------------------------------------------- //
		// Data retrieve functions

		LTVector	GetPoint(uint8 nPt, LTVector* pvF=LTNULL);
		uint8		GetNumPoints()					{ return (uint8)m_nPoints; }

		LTFLOAT		GetSegmentLength()				{ return m_fSegmentLength; }


	private:

		// -------------------------------------------------------------------- //
		// Misc helper functions

		void		Constrain_Point(LTVector *vPos, LTVector *vDir, LTVector *vPt, LTFLOAT fConstraint, LTFLOAT fFrameTime);


	private:

		// The data that we don't want people screwing around with directly

		LTVector		*m_pPoints;										// The list of points for the chain
		int				m_nPoints;										// The number of points

		LTFLOAT			m_fSegmentLength;								// The length of each segment

		LTFLOAT			m_fConstraint;									// The angle to constrain the chain
		LTFLOAT			m_fCorrectionRate;								// The rate the chain will correct itself

		HOBJECT			m_hLines;										// The debug line system
		HLTLINE			*m_pLines;										// The list of line handles
};

// -------------------------------------------------------------------- //

#endif


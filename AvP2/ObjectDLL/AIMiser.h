// ----------------------------------------------------------------------- //
//
// MODULE  : AIMiser.h
//
// PURPOSE : AI Miser definition
//
// CREATED : 04.20.1999
//
// ----------------------------------------------------------------------- //

#ifndef __AI_MISER_H__
#define __AI_MISER_H__

class CAI;

extern class CAIMiser* g_pAIMiser;

class CAIMiserToken
{
	public :

		// Ctors/Dtors/etc

		CAIMiserToken();

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		// Simple accessors
	
		DFLOAT GetTimeStart() { return m_fTimeStart; }
		DFLOAT GetTimeStop() { return m_fTimeStop; }

	private :

		void SetActivity(DFLOAT fActivity) { m_fActivity = fActivity; }
		DFLOAT GetActivity() { return m_fActivity; }

		void SetTimeStart(DFLOAT fTimeStart) { m_fTimeStart = fTimeStart; }
		void SetTimeStop(DFLOAT fTimeStop) { m_fTimeStop = fTimeStop; }

	private :

		friend class CAIMiser;
		
		DFLOAT		m_fActivity;
		DFLOAT		m_fTimeStart;
		DFLOAT		m_fTimeStop;
};

class CAIMiser
{
	public : // Public methods

		// Ctors/dtors/etc

		CAIMiser();

		void Init();
		void Term();

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		// Methods

		CAIMiserToken RequestTokenDialog(DFLOAT fPriority);
		CAIMiserToken RequestTokenMove(DFLOAT fPriority);
		CAIMiserToken RequestTokenShoot(DFLOAT fPriority);

		void ReleaseTokenDialog(CAIMiserToken& token);
		void ReleaseTokenMove(CAIMiserToken& token);
		void ReleaseTokenShoot(CAIMiserToken& token);
	
	private : // Private member variables

		DFLOAT		m_fActivityDialog;
		DFLOAT		m_fActivityMove;
		DFLOAT		m_fActivityShoot;
};

#endif


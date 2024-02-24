// ----------------------------------------------------------------------- //
//
// MODULE  : RandomSpawner.h
//
// PURPOSE : RandomSpawner - Definition
//
// CREATED : 04.23.1999
//
// ----------------------------------------------------------------------- //

#ifndef __RANDOM_SPAWNER_H__
#define __RANDOM_SPAWNER_H__

class RandomSpawner : public BaseClass
{
	public : // Public constants

		enum Constants
		{
			kMaxSpawners = 32,
		};

	public : // Public methods

		RandomSpawner();
		~RandomSpawner();

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	protected :

		void	TriggerMsg(HOBJECT hSender, HSTRING hMsg);

		DBOOL	ReadProp(ObjectCreateStruct *pData);
		DBOOL	Setup(ObjectCreateStruct *pData);
		void	PostPropRead(ObjectCreateStruct* pData);
		
		DBOOL	InitialUpdate();
		DBOOL	Update();

		void	Setup();
		void	Spawn();
		
		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

	public : // Public member variables

		DBOOL		m_bFirstUpdate;
		HSTRING		m_hstrSpawner;
		int			m_cSpawn;
		HOBJECT*	m_ahSpawners;
};

#endif // __RandomSpawner_H__

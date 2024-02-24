// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectRemover.h
//
// PURPOSE : ObjectRemover - Definition
//
// CREATED : 04.23.1999
//
// ----------------------------------------------------------------------- //

#ifndef __OBJECT_REMOVER_H__
#define __OBJECT_REMOVER_H__

class ObjectRemover : public BaseClass
{
	public : // Public constants

		enum Constants
		{
			kMaxGroups			= 6,
			kMaxObjectsPerGroup	= 6,
		};

	public : // Public methods

		ObjectRemover();
		~ObjectRemover();

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	protected :

		DBOOL	ReadProp(ObjectCreateStruct *pData);
		
		DBOOL	Update();

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

	public : // Public member variables

		int			m_cGroupsToKeep;
		HSTRING		m_ahstrObjects[kMaxGroups][kMaxObjectsPerGroup];
};

#endif // __ObjectRemover_H__


#ifndef __SPRINKLES_H__
#define __SPRINKLES_H__


	#include "cpp_engineobjects_de.h"


	#define MAX_SPRINKLE_TYPES	8


	class SprinkleType
	{
	public:
					SprinkleType();
					~SprinkleType();
		
		HSTRING		m_hFilename;
		HSTRING		m_hSkinName;
		DDWORD		m_Count;
		float		m_Speed;
		float		m_Size;
		float		m_SpawnRadius;
		DVector		m_AnglesVel;
		DVector		m_ColorMin;
		DVector		m_ColorMax;
	};


	class Sprinkles : public BaseClass
	{
	public:

						Sprinkles();
		virtual			~Sprinkles();

		virtual DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

		void			OnPreCreate(ObjectCreateStruct *pStruct);
		void			OnInitialUpdate();
		void			OnUpdate();
	
	
	public:

		SprinkleType	m_Types[MAX_SPRINKLE_TYPES];
		DDWORD			m_nTypes;
	};


#endif




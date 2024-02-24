//------------------------------------------------------------------
//
//   MODULE  : CLIENTFX.H
//
//   PURPOSE : Defines class CClientFX
//
//   CREATED : On 1/25/99 At 3:57:10 PM
//
//------------------------------------------------------------------

#ifndef __CLIENTFX_H_
	#define __CLIENTFX_H_

	// Includes....

	#include "cpp_engineobjects_de.h"

	class CClientFX : public BaseClass
	{
		public :
			
			// Constructor

									CClientFX();

			// Destructor

									~CClientFX();

		protected :

			// Member Functions

			DDWORD					EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

		private :

			void					SendFXMessage();
			void					Save(HMESSAGEWRITE hWrite, DDWORD dwFlags);
			void					Load(HMESSAGEREAD hRead, DDWORD dwFlags);


			HSTRING					m_hstrFxName;
			BOOL					m_bLoop;
	};

#endif

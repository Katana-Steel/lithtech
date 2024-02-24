// ----------------------------------------------------------------------- //
//
// MODULE  : KeyPad.h
//
// PURPOSE : Definition of the Key pad Model
//
// CREATED : 4/30/99
//
// ----------------------------------------------------------------------- //

#ifndef __KEY_PAD_H__
#define __KEY_PAD_H__

#include "Prop.h"

class CCharacter;

class KeyPad : public Prop
{
	public :

		KeyPad();
		~KeyPad();

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		virtual DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	protected :

		DBOOL	ReadProp(ObjectCreateStruct *pData);
		DBOOL	InitialUpdate();

		void	SetupDisabledState();
		void	HandleGadgetMsg(ConParse & parse);

	protected :

		HSTRING	m_hstrDisabledMessage;
		HSTRING m_hstrDisabledTarget;
		HOBJECT	m_hDeciphererModel;

	private :

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
};

#endif // __KEY_PAD_H__

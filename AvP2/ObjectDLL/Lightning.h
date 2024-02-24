// ----------------------------------------------------------------------- //
//
// MODULE  : Lightning.h
//
// PURPOSE : Lightning - Definition
//
// CREATED : 4/17/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTNING_H__
#define __LIGHTNING_H__

#include "ClientSFX.h"
#include "SFXMsgIds.h"
#include "SoundButeMgr.h"
#include <string>

class Lightning : public CClientSFX
{
	public :

		Lightning();
		~Lightning();

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		void	HandleMsg(HOBJECT hSender, HSTRING hMsg);

	private :

		LTBOOL		m_bFirstUpdate;
        LTBOOL		m_bOn;

        LTVector	m_vEndPos;
		std::string m_strEndPosObjectName;
        LTVector	m_vLightColor;
        LTFLOAT		m_fMinWidth;
        LTFLOAT		m_fMaxWidth;
        LTFLOAT		m_fLifeTime;
        LTFLOAT		m_fAlphaLifeTime;
        LTFLOAT		m_fMinDelayTime;
        LTFLOAT		m_fMaxDelayTime;
        LTFLOAT		m_fPerturb;
        LTFLOAT		m_fLightRadius;
        LTBOOL		m_bOneTimeOnly;
        LTBOOL		m_bDynamicLight;
        LTBOOL		m_bAdditive;
        LTBOOL		m_bMultiply;
        uint8		m_nWidthStyle;
        uint8		m_nNumSegments;

		uint16		m_nNearSound;
		uint16		m_nFarSound;
		LTFLOAT		m_fNearRadius;
		LTFLOAT		m_fDelayRadius;

		LTVector	m_vInnerColorStart;
		LTVector	m_vInnerColorEnd;
		LTVector	m_vOuterColorStart;
		LTVector	m_vOuterColorEnd;
					
        LTFLOAT		m_fAlphaStart;
        LTFLOAT		m_fAlphaEnd;

		HSTRING		m_hstrTexture;

        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

        LTBOOL ReadProp(ObjectCreateStruct *pStruct);

		void InitialUpdate();
		void Update();
};

// ----------------------------------------------------------------------- //

class LightningPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT	PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength);

	private:

		CSoundButeMgrPlugin	m_SoundButeMgrPlugin;
};

// ----------------------------------------------------------------------- //

#endif // __LIGHTNING_H__

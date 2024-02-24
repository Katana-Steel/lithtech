// ----------------------------------------------------------------------- //
//
// MODULE  : SFXMgr.h
//
// PURPOSE : Special FX Mgr	- Definition
//
// CREATED : 10/24/97
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SFX_MGR_H__
#define __SFX_MGR_H__

#include "iltclient.h"
#include "SpecialFXList.h"
#include "SFXMsgIds.h"

#pragma warning (disable : 4503)
#include <string>

#define DYN_ARRAY_SIZE		(SFX_TOTAL_NUMBER + 1)
#define	CAMERA_LIST_SIZE	20

struct SfxFilterStruct
{
	SfxFilterStruct()
	{
		bFilterFriends	= LTFALSE;
		nNumTypes		= 0;
		pTypes			= LTNULL;
	}
	int		nNumTypes;
	uint32* pTypes;
	LTBOOL	bFilterFriends;
};

class CSFXMgr
{
	public :

        CSFXMgr()
		{	
			m_pClientDE = LTNULL;
			m_szMode="Normal";
		}
		~CSFXMgr() {}

        LTBOOL   Init(ILTClient* pClientDE);

        CSpecialFX* FindSpecialFX(uint8 nType, HLOCALOBJ hObj);

		CSpecialFXList* GetCameraList() { return &m_cameraSFXList; }

		void	RemoveSpecialFX(HLOCALOBJ hObj);
		void	UpdateSpecialFX();
		void	HandleSFXMsg(HLOCALOBJ hObj, HMESSAGEREAD hMessage);
		void	SetMode(const std::string& mode);

		void	RemoveAll();

        CSpecialFX* CreateSFX(uint8 nId, SFXCREATESTRUCT *psfxCreateStruct,
            HMESSAGEREAD hMessage=LTNULL, HOBJECT hServerObj=LTNULL);

		inline CSpecialFXList* GetFXList(uint8 nType) 
		{ 
			if (nType >= DYN_ARRAY_SIZE) return LTNULL;

			return &m_dynSFXLists[nType]; 
		}
		
		CSpecialFX* GetClientCharacterFX();
		LTBOOL TestForObjectInList(HOBJECT hObj, uint32 nSFXType);

		void OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, float forceMag);
		void OnModelKey(HLOCALOBJ hObj, ArgList *pArgs);
		void OnSFXMessage(HMESSAGEREAD hMessage);
		void PostRenderDraw();
		const std::string GetMode() { return m_szMode; }

	private :

        void    AddDynamicSpecialFX(CSpecialFX* pSFX, uint8 nId);
		void 	UpdateDynamicSpecialFX();
		void	RemoveDynamicSpecialFX(HOBJECT hObj);
		void	RemoveAllDynamicSpecialFX();

        int             GetDynArrayIndex(uint8 nFXId);
        unsigned int    GetDynArrayMaxNum(uint8 nArrayIndex);

		std::string	m_szMode;

        ILTClient*      m_pClientDE;

		CSpecialFXList  m_dynSFXLists[DYN_ARRAY_SIZE]; // Lists of dynamic special fx
		CSpecialFXList	m_cameraSFXList;				// List of camera special fx
};

inline CSpecialFX* CSFXMgr::GetClientCharacterFX()
{
	HOBJECT hPlayer	= g_pLTClient->GetClientObject();

	//loop through players
	for(CSpecialFXList::Iterator iter = m_dynSFXLists[SFX_CHARACTER_ID].Begin(); 
	      iter != m_dynSFXLists[SFX_CHARACTER_ID].End(); ++iter )
	{
		//assign our character pointer
		const HOBJECT hCharacter = iter->pSFX ? iter->pSFX->GetServerObj() : LTNULL;

		// Are we the player?
		if(hCharacter && hCharacter == hPlayer)
			return iter->pSFX;
	}

	return LTNULL;
}
#endif // __SFX_MGR_H__


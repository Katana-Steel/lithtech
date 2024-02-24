// ----------------------------------------------------------------------- //
//
// MODULE  : CheatMgr.h
//
// PURPOSE : General player cheat code handling
//
// CREATED : 4/7/00
//
// ----------------------------------------------------------------------- //

#ifndef __CHEATMGR_H__
#define __CHEATMGR_H__

// ----------------------------------------------------------------------- //

#include "cpp_clientshell_de.h"
#include "client_de.h"
#include "CheatDefs.h"
#include "LithFontMgr.h"

// ----------------------------------------------------------------------- //

struct CheatInfo
{
	char	*szText;
	LTBOOL	bActive;
	LTBOOL	(*fnUpdate)(char *szDebug);
};

// ----------------------------------------------------------------------- //

class CheatMgr
{
	public:
		CheatMgr();
		~CheatMgr();

		LTBOOL	Init();

		LTBOOL	HandleCheat(int argc, char **argv);
		void	ClearCheater()					{ m_bPlayerCheated = LTFALSE; }
		LTBOOL	IsCheater()						{ return m_bPlayerCheated; }

		LTBOOL	UpdateCheats();
		LTBOOL	UpdateDebugText();

	private:

		static	CheatInfo	s_CheatInfo[];					// Cheat code list
		static	LTBOOL		m_bPlayerCheated;				// Has the player cheated?

		// Temp variables for the helper functions
		int					m_argc;
		char				**m_argv;

		// Debug information variables
		LithFont			*m_pFont;
		LithCursor			*m_pCursor;
		HSURFACE			m_hDebugSurf;
		char				m_szDebugString[256];

	private:

		// Helper functions
		void	Process(CheatCode nCheatCode);
		void	SendCheatMessage(CheatCode nCheatCode, uint8 nData);
		void	PlayCheatSound();
		LTBOOL	IsValidMPCheat(CheatCode nCheatCode);

		// --------------------------------------------------------------------------- //
		// Player modification cheats
		void	Cheat_Health();
		void	Cheat_Armor();
		void	Cheat_FullWeapons();
		void	Cheat_Ammo();
		void	Cheat_Tears();
		void	Cheat_God();
		void	Cheat_KFA();
		void	Cheat_Teleport();
		void	Cheat_Clip();
		void	Cheat_Character();
		void	Cheat_ReloadButes();
		void	Cheat_3rdPerson();

		// Debug information cheats
		void	Cheat_Pos();
		void	Cheat_Rot();
		void	Cheat_Dims();
		void	Cheat_Vel();
		void	Cheat_Framerate();
		void	Cheat_TriggerBox();

		// Position control cheats
		void	Cheat_PosWeapon();
		void	Cheat_PosWeaponMuzzle();
		void	Cheat_WeaponBreach();
		void	Cheat_LightScale();
		void	Cheat_LightAdd();
		void	Cheat_VertexTint();
		void	Cheat_FOV();

		// AI cheats
		void	Cheat_RemoveAI();

		// Misc cheats
		void	Cheat_Config();
		void	Cheat_Missions();

		// Easter egg cheats
		void	Cheat_MillerTime();
};

extern CheatMgr *g_pCheatMgr;

#endif // __CHEATMGR_H__

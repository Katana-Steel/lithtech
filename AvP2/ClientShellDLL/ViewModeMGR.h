// ----------------------------------------------------------------------- //
//
// MODULE  : ViewModeMGR.h
//
// PURPOSE : Controls the objects that contribute to a view. This object
//			 handles the decisions of what modes are available and what 
//			 objects need to do what for that mode.
//
// CREATED : 06/08/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef VIEW_MODE_MGR_H
#define VIEW_MODE_MGR_H

#include "ltBaseDefs.h"

#include <string>

struct StateChange;
struct ViewModeData;

class ViewModeMGR
{
	public:

		ViewModeMGR();
		~ViewModeMGR();

		void RegisterTexture(StateChange** stateChange, const char* commandLine);
		void MakeStarlightObject(HMESSAGEREAD hMessage);

		void Init();
		void LevelExit();

		void Update(LTBOOL bAlive);

		void SetNormalMode();

		void SetNetMode(LTBOOL bOn);
		void SetRailMode(LTBOOL bOn);
		void SetPredZoomMode(LTBOOL bOn);
		void SetMenuMode(LTBOOL bOn);
		void SetOnFireMode(LTBOOL bOn);
		void SetCineMode(LTBOOL bOn);

		void SetEvacMode(LTBOOL bOn);
		void SetEvacRotation(LTRotation rRot);
		void SetEvacAlpha(LTFLOAT fAlpha);

		void SetOrientMode(LTBOOL bOn);
		void SetOrientAngle(LTFLOAT fAngle);
		void ResetEnvMap();

		LTBOOL GetRailMode();

		const std::string GetCurrentModeName();
		const std::string GetLastModeName();
        const LTVector GetVertexTint();

		LTBOOL CanChangeMode();
		LTBOOL NextMode();
		LTBOOL PrevMode();

		void SetVisionSound();
		HLTSOUND m_hLoopingVMSound;

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

	private:

		ViewModeData*	m_pData;
		std::string		m_LastModeName;
		LTBOOL			m_bCineMode;
};

#endif		// VIEW_MODE_MGR_H

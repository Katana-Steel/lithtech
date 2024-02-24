// ----------------------------------------------------------------------- //
//
// MODULE  : Camera.h
//
// PURPOSE : Camera class definition
//
// CREATED : 5/20/98
//
// ----------------------------------------------------------------------- //

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "ltengineobjects.h"
#include <vector>

#define ADD_CAMERA_PROPERTIES(groupflag) \
	ADD_BOOLPROP_FLAG_HELP(Widescreen, LTFALSE, groupflag, "Should this camera use the letterbox style view?") \
	ADD_LONGINTPROP_FLAG_HELP(XFOV, 75, groupflag, "The horizontal camera FOV. This value is in degrees and should be between 1 and 180.") \
	ADD_LONGINTPROP_FLAG_HELP(YFOV, 60, groupflag, "The vertical camera FOV. This value is in degrees and should be between 1 and 180.") \
	ADD_BOOLPROP_FLAG_HELP(StartActive, LTFALSE, groupflag, "Should the camera start active?") \
	ADD_REALPROP_FLAG_HELP(ActiveTime, -1.0f, groupflag, "How long should this camera stay active once turned on?") \
	ADD_BOOLPROP_FLAG_HELP(OneTime, LTTRUE, groupflag, "Should this camera only be used once?") \
	ADD_BOOLPROP_FLAG_HELP(IsListener, LTFALSE, groupflag, "Should this camera represent a listening position for sounds when the camera is active?") \
	ADD_BOOLPROP_FLAG_HELP(AllowPlayerMovement, LTFALSE, groupflag, "Should the player be allow to move and rotate during this camera's active state?") \


class Camera : public BaseClass
{
	friend class CinematicTrigger;

	public :

		typedef std::vector<Camera*> CameraList;
		typedef CameraList::const_iterator CameraIterator;

		static CameraIterator BeginCameras() { return m_sCameraList.begin(); }
		static CameraIterator EndCameras()   { return m_sCameraList.end(); }

	public:

		Camera();
		~Camera();

	protected:

		uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		uint32	ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	private:

		void	InitialUpdate(int nInfo);
		void	Update();

		void	TriggerMsg(HOBJECT hSender, HSTRING hMsg);
		LTBOOL	ReadProps(LTBOOL bCreateSFXMsg=LTTRUE);

		void	CreateSFXMsg();
		void	TurnOn();
		void	TurnOff();

		void	Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		LTBOOL		m_bWidescreen;			// Is the camera displayed as widescreen, or fullscreen
		uint8		m_nXFOV;				// The horizontal FOV
		uint8		m_nYFOV;				// The vertical FOV
		LTBOOL		m_bStartActive;         // The camera starts active
		LTFLOAT		m_fActiveTime;          // How long camera stays on
		LTFLOAT		m_fTurnOffTime;         // Time to turn the camera off
		LTBOOL		m_bOneTime;             // Do we activate only one time
		LTBOOL		m_bIsListener;          // Listen for sounds at camera position
		LTBOOL		m_bAllowPlayerMovement; // Can the player move when the camera is live

		static CameraList m_sCameraList;
};

LTBOOL IsExternalCameraActive();

#endif // __CAMERA_H__

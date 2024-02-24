// ----------------------------------------------------------------------- //
//
// MODULE  : Controller.h
//
// PURPOSE : Controller - Definition
//
// CREATED : 4/17/99
//
// PURPOSE :
//
// The controller object is general-purpose object that can be set into one of several states
// by sending it a message.  The messages it accepts are (parenthesis means the parameter is
// necessary, brackets means the parameter is optional).
//
// FADE <parameter type> <destination value> <duration> [Wave type]
// - Fades to the specified destination value over time.
//
// FLICKER <interval min> <interval max> <message to send> [count, default -1 which means forever]
// - Sends a trigger message to the objects in a random time between the specified interval.
//
// OFF 
// - Stops whatever it's doing.
//
// Supported parameter types:
// Alpha - value 0-1
// Color - values 0-255, must be specified in quotes like "1 2 3"
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__


	#include "cpp_engineobjects_de.h"
	#include "CommonUtilities.h"


	#define MAX_CONTROLLER_TARGETS	8
	#define MAX_FLICKERMSG_LEN		64
	#define FLICKER_FOREVER			0xFFFFFFFF


	typedef enum
	{
		CState_Off=0,
		CState_Fade,
		CState_Flicker
	} CState;


	typedef enum
	{
		Param_Alpha=0,	// Alpha 0 - 1
		Param_Color		// Color 0 - 255
	} ParamType;

	
	// Generic parameter value.
	class ParamValue
	{
	public:
		float	GetAlpha()			{return m_Color.x;}
		void	SetAlpha(float x)	{m_Color.x = x;}

		DVector	GetColor()			{return m_Color;}
		void	SetColor(DVector x)	{m_Color = x;}

		void	Load(HMESSAGEREAD hRead);
		void	Save(HMESSAGEWRITE hWrite);
		
		DVector	m_Color;
	};


	class FadeState
	{
	public:
					FadeState()
					{
						m_hTarget = DNULL;
					}
		
		ParamValue	m_StartVal;
		char		m_ObjectName[32];
		HOBJECT		m_hTarget;			// The object being controlled.
	};


	class CommandMode
	{
	public:
		
	};


	class Controller : public BaseClass
	{
	public:

				Controller();

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);


	public:

		void		PreCreate(ObjectCreateStruct *pStruct);
		void		InitialUpdate();
		void		FirstUpdate();
		void		Update();
		void		OnLinkBroken(HOBJECT hObj);

		void		Load(HMESSAGEREAD hRead, DDWORD dwSaveFlags);
		void		Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);

		void		HandleTrigger(HOBJECT hSender, char *pMsg);
		void		HandleFadeCommand(char *pMsg, ConParse *pParse);
		void		HandleFlickerCommand(char *pMsg, ConParse *pParse);
		void		HandleOffCommand(char *pMsg, ConParse *pParse);

#ifndef _FINAL
		void		ShowTriggerError(char *pMsg);
#endif
		ParamValue	ParseValue(ParamType paramType, char *pValue);
		void		SetupCurrentValue(FadeState *pState);
		void		InterpolateValue(FadeState *pState, float t);


	public:

		CState		m_State;			// What state are we in?
		DBOOL		m_bFirstUpdate;

		// Vars for FLICKER state.
		float		m_fNextFlickerTime;	// Next time to flicker.
		float		m_fIntervalMin;		// Flicker interval..
		float		m_fIntervalMax;
		DWORD		m_FlickerCounter;	// Decremented each time it flickers.
		char		m_FlickerMsg[MAX_FLICKERMSG_LEN];
		
		// Vars for FADE state.
		float		m_fStartTime;		// When we started fading.
		float		m_fDuration;		// How long the fade takes place over.
		WaveType	m_WaveType;			// What kind of wave we're using.
		ParamType	m_ParamType;		// What parameter we're controlling.
		ParamValue	m_DestValue;		// What value we're fading to.
		
		// One for each object.
		FadeState	m_Fades[MAX_CONTROLLER_TARGETS];
	};


#endif



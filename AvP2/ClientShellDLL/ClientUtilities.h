// ----------------------------------------------------------------------- //
//
// MODULE  : ClientUtilities.h
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_UTILITIES_H__
#define __CLIENT_UTILITIES_H__

#include "iltclient.h"
#include "CommonUtilities.h"
#include "ClientServerShared.h"
#include "CameraMgrDefs.h"
#include <set>
#include <string>
#include <list>


extern const int g_kNumCommands;

const int kMaxSave	= 99;

struct WEAPON;
struct BARREL;

enum CommandType
{
	COM_SHARED,
	COM_MARINE,
	COM_ALIEN,
	COM_PREDATOR,
};


struct CommandData
{
	int		nStringID;
	int		nCommandID;
	int		nActionStringID;
	uint8	nType;
};

extern CommandData g_CommandArray[];

#define IsKeyDown(key)  (GetAsyncKeyState(key) & 0x80000000)
#define WasKeyDown(key) (GetAsyncKeyState(key) & 0x00000001)
#define WisKeyDown(key) (GetAsyncKeyState(key) & 0x80000001)


//**************************************************************

void StartProfile();
void EndProfile(const char *szPrint = LTNULL);

//**************************************************************


struct DSize
{
	DSize()		{ cx = 0; cy = 0; }
	
	unsigned long	cx;
	unsigned long	cy;
};

char* CommandName(int nCommand);

HSURFACE CropSurface(HSURFACE hSurf, HDECOLOR hBorderColor);

LTBOOL GetScreenPos(HCAMERA hCamera, HOBJECT hObj, LTIntPt &ptRval, LTBOOL bTargetTorso=LTFALSE);


DBOOL GetAttachmentSocketTransform(HOBJECT hObj, char* pSocketName, DVector & vPos, DRotation & rRot);
void GetAttachmentsAlpha(HOBJECT hObj, LTFLOAT *pAlphas, uint32 nElements);
void SetAttachmentsAlpha(HOBJECT hObj, LTFLOAT fAlpha, uint32 nUsrFlagTest);
void SetAttachmentsColor(HOBJECT hObj, LTVector vColor, uint32 nFlagsTest = 0, uint32 nFlags2Test = 0);


void PlayWeaponSound(WEAPON *pWeaon, BARREL *pBarrel, LTVector vPos, WeaponModelKeySoundId eSoundId,
					 LTBOOL bLocal=LTFALSE);

// Takes the hCamera and determines the scale factors to apply to a sprite overlay.
void GetCameraScale(HCAMERA hCamera, LTFLOAT distance, LTFLOAT &scaleX, LTFLOAT &scaleY);

int		GetConsoleInt(char* sKey, int nDefault);
void	GetConsoleString(char* sKey, char* sDest, char* sDefault);
void	WriteConsoleString(char* sKey, char* sValue);
void	WriteConsoleInt(char* sKey, int nValue);
LTFLOAT GetConsoleFloat(char* sKey, LTFLOAT fDefault);
void	WriteConsoleFloat(char* sKey, LTFLOAT fValue);


#endif // __CLIENT_UTILITIES_H__
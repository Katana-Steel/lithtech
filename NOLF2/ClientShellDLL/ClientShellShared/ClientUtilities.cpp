// ----------------------------------------------------------------------- //
//
// MODULE  : ClientUtilities.cpp
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "StdAfx.h"
#include <stdlib.h>
#include "ClientUtilities.h"
#include "GameClientShell.h"
#include "VarTrack.h"
#include "vkdefs.h"
#include "WeaponMgr.h"
#include "ClientServerShared.h"
#include "ClientMultiplayerMgr.h"

//-------------------------------------------------------------------------------------------
// CropSurface
//
// Crops the given surface to smallest possible area
// Arguments:
//		hSurf - surface to be cropped
//		hBorderColor - color of area to be cropped
// Return:
//		cropped surface if successful, original surface otherwise
//-------------------------------------------------------------------------------------------
HSURFACE CropSurface ( HSURFACE hSurf, HLTCOLOR hBorderColor )
{
	if (!g_pGameClientShell) return hSurf;

    if (!hSurf) return LTNULL;

    uint32 nWidth, nHeight;
	g_pLTClient->GetSurfaceDims (hSurf, &nWidth, &nHeight);

    LTRect rcBorders;
    memset (&rcBorders, 0, sizeof (LTRect));
	g_pLTClient->GetBorderSize (hSurf, hBorderColor, &rcBorders);

	if (rcBorders.left == 0 && rcBorders.top == 0 && rcBorders.right == 0 && rcBorders.bottom == 0) return hSurf;

	HSURFACE hCropped = g_pLTClient->CreateSurface (nWidth - rcBorders.left - rcBorders.right, nHeight - rcBorders.top - rcBorders.bottom);
	if (!hCropped) return hSurf;

    LTRect rcSrc;
	rcSrc.left = rcBorders.left;
	rcSrc.top = rcBorders.top;
	rcSrc.right = nWidth - rcBorders.right;
	rcSrc.bottom = nHeight - rcBorders.bottom;

	g_pLTClient->DrawSurfaceToSurface (hCropped, hSurf, &rcSrc, 0, 0);

	g_pLTClient->DeleteSurface (hSurf);

	return hCropped;
}


// The following two functions should be used to determine how long a block
// of code takes to execute.  For example:
//
// StartTimingCounter();
// float p1 = 30.0f, p2 = 50.0f;
// Function(p1, p2);
// EndTimingCounter("Function(%.2f, %.2f)", p1, p2);
//
// If "Function" took 1000 ticks to execute, the above code would print in
// the console:
//		Function(30.00, 50.00) : 1000 ticks
//
// NOTE:  The timing information is only printed to the console if the server
// console variable "ShowTiming" is set to 1. (i.e., showtiming 1)

#ifndef __PSX2      // Defiend in serverutils.cpp
extern VarTrack	g_vtShowTimingTrack;
static LTCounter s_counter;

void StartTimingCounter()
{
    if (!g_pLTClient || g_vtShowTimingTrack.GetFloat() < 1.0f) return;

    g_pLTClient->StartCounter(&s_counter);
}

void EndTimingCounter(const char *msg, ...)
{
    if (!g_pLTClient || g_vtShowTimingTrack.GetFloat() < 1.0f) return;

    uint32 dwTicks = g_pLTClient->EndCounter(&s_counter);

	// parse the message

	char pMsg[256];
	va_list marker;
	va_start(marker, msg);
	int nSuccess = vsnprintf(pMsg,sizeof(pMsg), msg, marker);
	va_end(marker);

	if (nSuccess < 0) return;

    g_pLTClient->CPrint("%s : %d ticks", pMsg, dwTicks);
}
#endif


void GetConsoleString(char const* sKey, char* sDest, char const* sDefault)
{
    if (g_pLTClient)
	{
        HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(( char* )sKey);
		if (hVar)
		{
            const char* sValue = g_pLTClient->GetVarValueString(hVar);
			if (sValue)
			{
				strcpy(sDest, sValue);
				return;
			}
		}
	}

	strcpy(sDest, sDefault);
}

char*  GetConsoleTempString(char const* sKey, char const* sDefault)
{
	static char szTmp[256];
	szTmp[0] = '\0';
    if (g_pLTClient)
	{
        HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(( char* )sKey);
		if (hVar)
		{
            const char* sValue = g_pLTClient->GetVarValueString(hVar);
			if (sValue)
			{
				SAFE_STRCPY(szTmp, sValue);
				return szTmp;
			}
		}
	}

	SAFE_STRCPY(szTmp, sDefault);
	return szTmp;
}

int GetConsoleInt(char const* sKey, int nDefault)
{
    if (g_pLTClient)
	{
        HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(( char* )sKey);
		if (hVar)
		{
            float fValue = g_pLTClient->GetVarValueFloat(hVar);
			return((int)fValue);
		}
	}

	return(nDefault);
}
bool GetConsoleBool( char const* sKey, bool bDefault)
{
    if (g_pLTClient)
	{
        HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(( char* )sKey);
		if (hVar)
		{
            float fValue = g_pLTClient->GetVarValueFloat(hVar);
			return(fValue != 0.0f);
		}
	}

	return(bDefault);
}

LTFLOAT GetConsoleFloat(char const* sKey, LTFLOAT fDefault)
{
    if (g_pLTClient)
	{
        HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(( char* )sKey);
		if (hVar)
		{
            float fValue = g_pLTClient->GetVarValueFloat(hVar);
			return(fValue);
		}
	}

	return(fDefault);
}

void WriteConsoleString(char const* sKey, char const* sValue)
{
    if (g_pLTClient)
	{
		char sTemp[256];
#ifndef _WIN32
        snprintf(sTemp, sizeof(sTemp), "+%s \"%s\"", sKey, sValue);
#else
        wsprintf(sTemp, "+%s \"%s\"", sKey, sValue);
#endif
        g_pLTClient->RunConsoleString(sTemp);
	}
}

void WriteConsoleInt(char const* sKey, int nValue)
{
    if (g_pLTClient)
	{
		char sTemp[256];
#ifndef _WIN32
        snprintf(sTemp, sizeof(sTemp), "+%s %i", sKey, nValue);
#else
		wsprintf(sTemp, "+%s %i", sKey, nValue);
#endif
        g_pLTClient->RunConsoleString(sTemp);
	}
}

void WriteConsoleBool(char const* sKey, int bValue)
{
    if (g_pLTClient)
	{
		char sTemp[256];
        snprintf(sTemp, sizeof(sTemp), "+%s %i", sKey, (bValue ? 1 : 0));
        g_pLTClient->RunConsoleString(sTemp);
	}
}

void WriteConsoleFloat(char const* sKey, LTFLOAT fValue)
{
    if (g_pLTClient)
	{
		char sTemp[256];
		snprintf(sTemp, sizeof(sTemp), "+%s %f", sKey, fValue);
        g_pLTClient->RunConsoleString(sTemp);
	}
}


static VarTrack s_cvarFirePitchShift;
void PlayWeaponSound(WEAPON const *pWeapon, const LTVector &vPos, PlayerSoundId eSoundId,
					 LTBOOL bLocal)
{
	if (!pWeapon) return;

 	if (!s_cvarFirePitchShift.IsInitted())
	{
		s_cvarFirePitchShift.Init(g_pLTClient, "PitchShiftFire", NULL, -1.0f);
	}

	char* pSnd = LTNULL;

	LTFLOAT fRadius = WEAPON_SOUND_RADIUS;

	switch (eSoundId)
	{
		case PSI_FIRE:
		{
			pSnd = pWeapon->szFireSound;
			fRadius = (LTFLOAT) pWeapon->nFireSoundRadius;
		}
		break;

		case PSI_ALT_FIRE:
		{
			pSnd = pWeapon->szAltFireSound;
			fRadius = (LTFLOAT) pWeapon->nFireSoundRadius;
		}
		break;

		case PSI_SILENCED_FIRE:
		{
			pSnd = pWeapon->szSilencedFireSound;
			fRadius = (LTFLOAT) pWeapon->nFireSoundRadius;
		}
		break;

		case PSI_DRY_FIRE:
		{
			pSnd = pWeapon->szDryFireSound;
			fRadius = (LTFLOAT) pWeapon->nFireSoundRadius;
		}
		break;

		case PSI_RELOAD:
		case PSI_RELOAD2:
		case PSI_RELOAD3:
		{
			pSnd = pWeapon->szReloadSounds[eSoundId - PSI_RELOAD];
			fRadius = (LTFLOAT)pWeapon->nWeaponSoundRadius;
		}
		break;

		case PSI_SELECT:
		{
			pSnd = pWeapon->szSelectSound;
			fRadius = (LTFLOAT)pWeapon->nWeaponSoundRadius;
		}
		break;

		case PSI_DESELECT:
		{
			pSnd = pWeapon->szDeselectSound;
			fRadius = (LTFLOAT)pWeapon->nWeaponSoundRadius;
		}
		break;

		case PSI_WEAPON_MISC1:
		case PSI_WEAPON_MISC2:
		case PSI_WEAPON_MISC3:
		case PSI_WEAPON_MISC4:
		case PSI_WEAPON_MISC5:
		{
			pSnd = pWeapon->szMiscSounds[eSoundId - PSI_WEAPON_MISC1];
			fRadius = (LTFLOAT)pWeapon->nWeaponSoundRadius;
		}
		break; 

		case PSI_INVALID:
		default : break;
	}

	if (pSnd && pSnd[0])
	{
		uint32 dwFlags = PLAYSOUND_REVERB;
		float fPitchShift = 1.0f;
		if (s_cvarFirePitchShift.GetFloat() > 0.0f)
		{
			dwFlags |= PLAYSOUND_CTRL_PITCH;
			fPitchShift = s_cvarFirePitchShift.GetFloat();
		}

 		if (bLocal)
		{
			g_pClientSoundMgr->PlaySoundLocal(pSnd, SOUNDPRIORITY_PLAYER_HIGH,
				dwFlags, SMGR_DEFAULT_VOLUME, fPitchShift, WEAPONS_SOUND_CLASS );
		}
		else
		{
 			g_pClientSoundMgr->PlaySoundFromPos(const_cast<LTVector&>(vPos), pSnd,
				fRadius, SOUNDPRIORITY_PLAYER_HIGH, dwFlags,
				SMGR_DEFAULT_VOLUME, fPitchShift, -1.0f, WEAPONS_SOUND_CLASS );
		}
	}
}


LTRESULT SendEmptyServerMsg(uint32 nMsgID, uint32 nFlags)
{
	LTRESULT nResult;

	CAutoMessage cMsg;

	cMsg.Writeuint8(nMsgID);

	nResult = g_pLTClient->SendToServer(cMsg.Read(), nFlags);

	return nResult;
}

#ifndef _WIN32

#include <string_view>
#include <charconv>
#include <algorithm>
#include <vector>

#define FORMAT_MESSAGE_FROM_STRING 0
static void FormatMessage(int flags, char* source, int message_id, int language_id, char* dest, int len, va_list* args)
{
	if (flags != FORMAT_MESSAGE_FROM_STRING)
	{
		assert(!"ERROR: only FORMAT_MESSAGE_FROM_STRING is handled");
		return;
	}

	char r=' ';
	auto find_format = [r](char x) mutable {
			bool b=false;
			if (r=='%') {
				if(x==r) {   // if %% don't count
					r = ' '; // and reset the look back
					return false;
				} else       // else we found one
					b=true;
			}
			r=x; // save the current char for the
			return b;};

	std::string_view src{source};
	auto num_args = std::count_if(src.begin(), src.end(), find_format);

	if(num_args > 9)
	{
		assert(!"ERROR: only up to 9 arguments is supported");
		return;
	}

	if (num_args == 0) // don't need to do any formating
	{
		if(src.size() <= len)
			strcpy(dest, src.data());
		return;
	}

	std::vector<std::string> s_args{};
	auto digit = std::string_view{"!d!"};
	auto str_tok = std::string_view{"!s!"};
	s_args.reserve(9);
	const char *opts[] ={
		"%1!", "%2!", "%3!", "%4!", "%5!", "%6!", "%7!", "%8!", "%9!"
	};
	for (auto opt: opts)
	{
		auto found = src.find(opt);
		if(found == src.npos)
			break;
		auto ptr = found + (strlen(opt) - 1);
		auto type = src.substr(ptr, 3);
		if (type == digit) {
			s_args.push_back(std::to_string(va_arg(*args, int)));
		} else if (type == str_tok) {
			s_args.emplace_back(va_arg(*args, const char*));
		} else {
			assert(!"Format Agrument Type not supported");
		}
	}

	// now start copying the message to the destination buffer
	int bytes_left = len - 1;
	char *dst_ptr = dest;
	for (int i = 0; i < src.size(); i++)
	{
		if (src.at(i) == '%' && src.at(i+1) != '%')
		{
			auto a = src.substr(i+1, 1);    //no string exceeds 9 arguments, so assume 1 digit
			int arg_num{};
			std::from_chars(a.data(), a.data()+a.size(), arg_num);
			a = s_args.at(--arg_num); //zero based vector
			if (bytes_left >= a.length()) {
				strcpy(dst_ptr, a.data());
				bytes_left -= a.length();
				dst_ptr += a.length();
			}
			i += 1 + digit.length();
		} else if ( src.at(i) == '%' && src.at(i+1) == '%' && bytes_left > 0) {
			*dst_ptr = '%';
			--bytes_left;
			++dst_ptr;
			i += 2;
		} else {
			*dst_ptr = src.at(i);
			--bytes_left;
			++dst_ptr;
		}
		if (bytes_left == 0)
			break;
	}
	*dst_ptr = '\0';
	dest[len - 1] = '\0';
}

#endif

void FormatString(int messageCode, char *outBuf, int outBufLen,  ...)
{
    va_list marker;

	void* pModule;
	g_pLTClient->GetEngineHook("cres_hinstance",&pModule);
	HMODULE hModule = (HINSTANCE)pModule;

	*outBuf = '\0';

    if (hModule)
    {
        va_start(marker, outBufLen);

		char tmpBuffer[kMaxStringBuffer];
		uint32 nBytes = LoadString(hModule, messageCode, (char*)tmpBuffer, sizeof(tmpBuffer));
		if (nBytes)
		{
			FormatMessage(FORMAT_MESSAGE_FROM_STRING,tmpBuffer,0,0,outBuf,outBufLen,&marker);
		}

        va_end(marker);

    }


}

void LoadString(int messageCode, char *outBuf, int outBufLen)
{
	void* pModule;
	g_pLTClient->GetEngineHook("cres_hinstance",&pModule);
	HMODULE hModule = (HINSTANCE)pModule;

	*outBuf = '\0';

    if (hModule)
    {
		uint32 nBytes = LoadString(hModule, messageCode, (char*)outBuf, outBufLen);
    }
}

static char s_szStringBuffer[kMaxStringBuffer];
char* FormatTempString(int messageCode, ...)
{
    va_list marker;

	void* pModule;
	g_pLTClient->GetEngineHook("cres_hinstance",&pModule);
	HMODULE hModule = (HINSTANCE)pModule;

	s_szStringBuffer[0] = '\0';

    if (hModule)
    {
        va_start(marker, messageCode);

		char tmpBuffer[kMaxStringBuffer];
		uint32 nBytes = LoadString(hModule, messageCode, tmpBuffer, sizeof(tmpBuffer));
		if (nBytes)
		{
			FormatMessage(FORMAT_MESSAGE_FROM_STRING,tmpBuffer,0,0,s_szStringBuffer,sizeof(s_szStringBuffer),&marker);
		}

        va_end(marker);

    }

	return s_szStringBuffer;

}

char* LoadTempString(int messageCode)
{
	void* pModule;
	g_pLTClient->GetEngineHook("cres_hinstance",&pModule);
	HMODULE hModule = (HINSTANCE)pModule;

	s_szStringBuffer[0] = '\0';

    if (hModule)
    {
		uint32 nBytes = LoadString(hModule, messageCode, s_szStringBuffer, sizeof(s_szStringBuffer));
    }

	return s_szStringBuffer;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::IsMultiplayerGame()
//
//	PURPOSE:	See if we are playing a multiplayer game
//
// --------------------------------------------------------------------------- //

bool IsMultiplayerGame()
{
	if( !g_pClientMultiplayerMgr )
		return false;

	int nGameMode = g_pClientMultiplayerMgr->GetStartGameRequest( ).m_Type;
    if (nGameMode == STARTGAME_NORMAL || nGameMode == GAMEMODE_NONE) 
		return false;

    return true;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool GetIntersectionUnderPoint
//
//  PURPOSE:	Helper for finding the normal and point of intersection beneath a given point
//
// ----------------------------------------------------------------------- //

bool GetIntersectionUnderPoint( LTVector &vInPt, HOBJECT *pFilterList, LTVector &vOutNormal, LTVector &vOutPt )
{
	ClientIntersectQuery	iq;
	ClientIntersectInfo		ii;
	
	vOutNormal.Init(0, 1, 0);

	iq.m_Flags	= IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;
	iq.m_From	= vInPt;
	iq.m_To		= iq.m_From + LTVector( 0, -1, 0) * 256.0f;

	iq.m_FilterFn  = ObjListFilterFn;
	iq.m_pUserData = pFilterList;

	if( g_pLTClient->IntersectSegment( &iq, &ii ) )
	{
		if( ii.m_hObject )
		{
			vOutNormal	= ii.m_Plane.m_Normal;
			vOutPt		= ii.m_Point;

			return true;
		}
	}
	
	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GetContouringNormal
//
//  PURPOSE:	Find the normal of the plane we would like to contour to
//				given a position and dims of an object...
//
// ----------------------------------------------------------------------- //

LTVector GetContouringNormal( LTVector &vPos, LTVector &vDims, LTVector &vForward, LTVector &vRight, HOBJECT *pFilterList )
{
	LTVector	avPt[4];		// points we are casting the rays from
	LTVector	avInterPt[4];	// points of intersection
	LTVector	avNormal[4];	// normals constructed from the points of intersection
	LTVector	avEdge[4];

	// Develop the points we wish to cast rays from...
	// We keep the height from the object incase the vehicle has clipped into the world.

	avPt[0] = vPos + ( vForward ) - ( vRight );	// 0----1
	avPt[1] = vPos + ( vForward ) + ( vRight );	// |    |
	avPt[2] = vPos - ( vForward ) + ( vRight );	// |    |
	avPt[3] = vPos - ( vForward ) - ( vRight );	// 3----2

	// Find the point of intersection that is under the vehicle...
	// If none was found just use the point with the height factored in.
	
	if( !GetIntersectionUnderPoint( avPt[0], pFilterList, avNormal[0], avInterPt[0] ) )
	{
		avInterPt[0] = avPt[0];
		avInterPt[0].y -= vDims.y;
	}

	if( !GetIntersectionUnderPoint( avPt[1], pFilterList, avNormal[1], avInterPt[1] ) )
	{
		avInterPt[1] = avPt[1];
		avInterPt[1].y -= vDims.y;
	}

	if( !GetIntersectionUnderPoint( avPt[2], pFilterList, avNormal[2], avInterPt[2] ) )
	{
		avInterPt[2] = avPt[2];
		avInterPt[2].y -= vDims.y;
	}

	if( !GetIntersectionUnderPoint( avPt[3], pFilterList, avNormal[3], avInterPt[3] ) )
	{
		avInterPt[3] = avPt[3];
		avInterPt[3].y -= vDims.y;
	}

	// Move the points to the origin...

	avInterPt[0] -= vPos;
	avInterPt[1] -= vPos;
	avInterPt[2] -= vPos;
	avInterPt[3] -= vPos;

	// Develop the vectors that will construct the 4 planes...

	avEdge[0] = (avInterPt[1] - avInterPt[0]).Unit();
	avEdge[1] = (avInterPt[2] - avInterPt[1]).Unit();
	avEdge[2] = (avInterPt[3] - avInterPt[2]).Unit();
	avEdge[3] = (avInterPt[0] - avInterPt[3]).Unit();

	// Find the normals of the planes...

	avNormal[0] = -avEdge[3].Cross( avEdge[0] );
	avNormal[1] = -avEdge[0].Cross( avEdge[1] );
	avNormal[2] = -avEdge[1].Cross( avEdge[2] );
	avNormal[3] = -avEdge[2].Cross( avEdge[3] );

	avNormal[0].Normalize();
	avNormal[1].Normalize();
	avNormal[2].Normalize();
	avNormal[3].Normalize();

	// Average the normals...
	
	LTVector vNormal = avNormal[0] + avNormal[1] + avNormal[2] + avNormal[3];
	vNormal.Normalize();

	return vNormal;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GetContouringInfo
//
//  PURPOSE:	Get the pitch amount and percents to apply for pitch and roll
//				based on the forward direction and plane normal...
//
// ----------------------------------------------------------------------- //

void GetContouringInfo( LTVector &vForward, LTVector &vNormal, 
					   float &fOutAmount, float &fOutPitchPercent, float &fOutRollPercent )
{
	LTVector	vPlaneF = (vNormal.y >= 1.0f) ? vForward : vNormal;
	
	vPlaneF.y = 0.0f;
	vPlaneF.Normalize();

	LTRotation	rPlaneRot( vPlaneF, LTVector(0, 1, 0));
	LTVector	vPlaneR = rPlaneRot.Right();
	
	// Calculate how much pitch and roll we should apply...

	fOutPitchPercent	= vForward.Dot( vPlaneF );
	fOutRollPercent		= vForward.Dot( vPlaneR );

	// Figure out the length of the foward vector projected on the xz plane.  This
	// is needed because Euler angles are calculated cummulatively, not just based
	// on the global coordinate axis.

	float fXZLen = (float)sqrt( 1.0f - vNormal.y * vNormal.y );

	// Subtract the pitch from 90 degrees cause we want to be parallel to the plane

	fOutAmount = MATH_HALFPI - (float)atan2( vNormal.y, fXZLen );
}

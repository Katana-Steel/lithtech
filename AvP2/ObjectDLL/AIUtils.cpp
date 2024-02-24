// ----------------------------------------------------------------------- //
//
// MODULE  : AIUtils.cpp
//
// PURPOSE : AI helper stuff
//
// CREATED : 05.10.1999
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIUtils.h"
#include "ServerUtilities.h"
#include "GameServerShell.h"
#include "AITarget.h"
#include "Weapon.h"
#include "FXButeMgr.h"
#include "AIButeMgr.h"
#include "RCMessage.h"
#include "MsgIDs.h"

// Globals

int g_cIntersectSegmentCalls = 0;


LTFLOAT GetAIUpdateDelta()
{
	const LTFLOAT c_fMax  = 0.15f;
	const LTFLOAT c_fMin  = 0.08f;
	const LTFLOAT c_fStep = 0.01f; // Increase the update delta by this amount.

	static LTFLOAT fUpdateDelta = (c_fMax + c_fMin)*0.5f - c_fStep;

	fUpdateDelta += c_fStep;
	if( fUpdateDelta > c_fMax )
	{
		fUpdateDelta = c_fMin;
	}

	ASSERT( fUpdateDelta >= c_fMin );

	return fUpdateDelta;
}

const LTFLOAT c_fAIDeactivationTime	= 10.0f;

const LTFLOAT c_fFacingThreshhold	= .999999f;

const char c_szKeyFireWeapon[]		= "FIRE";
const char c_szKeyBodySlump[]		= "NOISE";
const char c_szKeyPickUp[]			= "PICKUP";

const char c_szActivate[]			= "ACTIVATE";

#ifndef _FINAL

void AICPrint(HOBJECT hAI, const char *pMsg, ...)
{
	va_list marker;
	char msg[500];
	const int msg_length = sizeof(msg);

	va_start(marker, pMsg);
	_vsnprintf(msg, msg_length, pMsg, marker);
	va_end(marker);
	msg[msg_length] = '\0';

	g_pServerDE->CPrint("%f %s : %s",
		g_pServerDE->GetTime(),
		hAI ? g_pServerDE->GetObjectName(hAI) : "",
		msg);
}

void AICPrint(const LPBASECLASS pObject, const char *pMsg, ...)
{
	va_list marker;
	char msg[500];
	const int msg_length = sizeof(msg);

	va_start(marker, pMsg);
	_vsnprintf(msg, msg_length, pMsg, marker);
	va_end(marker);
	msg[msg_length] = '\0';

	g_pServerDE->CPrint("%f %s : %s",
		g_pServerDE->GetTime(),
		pObject ? g_pServerDE->GetObjectName(pObject->m_hObject) : "",
		msg);
}

void AIErrorPrint(HOBJECT hAI, const char *pMsg, ...)
{
	va_list marker;
	char msg[500];
	const int msg_length = sizeof(msg);

	va_start(marker, pMsg);
	_vsnprintf(msg, msg_length, pMsg, marker);
	va_end(marker);
	msg[msg_length] = '\0';

	g_pServerDE->CPrint("%f %s : %s",
		g_pServerDE->GetTime(),
		hAI ? g_pServerDE->GetObjectName(hAI) : "",
		msg);

	RCMessage null;
	g_pLTServer->SendToClient(*null, MID_CONSOLE_COMMAND, NULL, 0);

}

void AIErrorPrint(const LPBASECLASS pObject, const char *pMsg, ...)
{
	va_list marker;
	char msg[500];
	const int msg_length = sizeof(msg);

	va_start(marker, pMsg);
	_vsnprintf(msg, msg_length, pMsg, marker);
	va_end(marker);
	msg[msg_length] = '\0';

	g_pServerDE->CPrint("%f %s : %s",
		g_pServerDE->GetTime(),
		pObject ? g_pServerDE->GetObjectName(pObject->m_hObject) : "",
		msg);

	RCMessage null;
	g_pLTServer->SendToClient(*null, MID_CONSOLE_COMMAND, NULL, 0);
}

#endif


std::string VectorToString(const LTVector & vec)
{
	char temp[256];
	sprintf(temp,"(%.2f %.2f %.2f)",
		vec.x, vec.y, vec.z );

	return std::string(temp);
}


ILTMessage & operator<<(ILTMessage & out, const std::string & x)
{
	// Write the size of the string.
	out.WriteWord(x.size());

	// Write each byte of the string.
	for( std::string::const_iterator iter = x.begin();
	     iter != x.end(); ++iter )
	{
		out.WriteByte(*iter);
	}

	// Write the terminating null, just to be safe.
	out.WriteByte(0);
	return out;
}

ILTMessage & operator>>(ILTMessage & in,  std::string & x)
{
	// Remove anything that was in the string.
	x.clear();

	// Read the size of the string, and each byte of the string.
	const uint16 string_size = in.ReadWord();

	x.reserve(string_size);
	for(int i = 0; i < string_size; ++i)
	{
		x += in.ReadByte();
	}

	// Read the terminating null (was not needed, just there for safety).
	in.ReadByte();

	return in;
}

// GetDifficultyFactor

LTFLOAT GetDifficultyFactor()
{
	ASSERT( g_pAIButeMgr && g_pGameServerShell );

	if (!g_pGameServerShell || !g_pAIButeMgr) return 1.0f;

	return g_pAIButeMgr->GetDifficultyFactor(g_pGameServerShell->GetGameType().GetDifficulty());
}

#ifndef _FINAL
bool ShouldShow(HOBJECT hObject)
{
	ASSERT( g_pLTServer);

	static CVarTrack vtShowAIFilter(g_pLTServer,"ShowAIFilter","all");

	if(    0 == stricmp(vtShowAIFilter.GetStr(),"all")
		|| 0 != strstr(g_pLTServer->GetObjectName(hObject), vtShowAIFilter.GetStr()) )
	{
		return true;
	}

	return false;
}
#endif

SequintialBaseName::SequintialBaseName( const std::string & name )
	: m_strName(name),
	  m_nValue(0)
{
	std::string::reverse_iterator riter = m_strName.rbegin();
	int nMultiplier = 1;
	for( ; riter != m_strName.rend() && isdigit(*riter); ++riter )
	{
		m_nValue += nMultiplier*((*riter) - '0');
		nMultiplier *= 10;
	}

	m_strName.erase(riter.base(),m_strName.end());
}

HOBJECT SequintialBaseName::FindObject() const
{
	const int nMaxNameLength = 255;
	char szName[nMaxNameLength + 1];
	szName[0] = 0;
	szName[nMaxNameLength] = 0;

	HOBJECT hResult = LTNULL;

	_snprintf(szName, nMaxNameLength+1, "%s%d",m_strName.c_str(),m_nValue);
	ASSERT( !szName[nMaxNameLength] );
	szName[nMaxNameLength] = 0;

	if( LT_NOTFOUND == FindNamedObject(szName, hResult ) )
	{
		// If that didn't work, try using a prefixed zero.
		_snprintf(szName, nMaxNameLength+1, "%s%02d", m_strName.c_str(),m_nValue);
		ASSERT( !szName[nMaxNameLength] );
		szName[nMaxNameLength] = 0;

		FindNamedObject(szName, hResult );
	}

	return hResult;
}

LTVector TargetAimPoint(const CAI & ai, const CAITarget & target, const CWeapon & weapon)
{
	LTVector vResult(0,0,0);

	if( !target.IsValid() ) return vResult;

	vResult = target.GetPosition();

	//
	// Aim someplace beside target's crotch.
	//
	const CCharacter * pCharTarget = target.GetCharacterPtr();
	if( pCharTarget )
	{
		vResult += target.GetTargetOffset();
	}

	//
	// Add a little failure.
	//
	if( (vResult - ai.GetPosition()).MagSqr() > target.GetLag().MagSqr() )
		vResult += target.GetLag();


	// Acount for projectiles (moving vResult to the point we
	// want to aim at to account for gravity)
	const AMMO * pAmmoData = weapon.GetAmmoData();
	if( pAmmoData && pAmmoData->eType != VECTOR
		&& pAmmoData->pProjectileFX ) 
	{
		const LTVector & vWeaponPos = ai.GetWeaponPosition( const_cast<CWeapon*>(&weapon) );

		LTVector shot_horiz_vector = (vResult - vWeaponPos);
		const LTFLOAT shot_vert_offset = shot_horiz_vector.y;
		shot_horiz_vector.y = 0.0f;

		const LTFLOAT shot_horiz_dist = shot_horiz_vector.Mag();
		const LTFLOAT shot_speed = LTFLOAT(pAmmoData->pProjectileFX->nVelocity);
		
		// Replace the target's position, with their predicted position.
		vResult -= target.GetPosition();
		vResult += target.GetPredictedPosition( shot_horiz_dist / shot_speed + ai.GetUpdateDelta() );

		if( pAmmoData->pProjectileFX->dwObjectFlags & FLAG_GRAVITY )
		{
			// If we can, aim at the target's feet.
			if( target.GetCharacterPtr() )
			{
				vResult -= target.GetTargetOffset();
				vResult.y -= target.GetCharacterPtr()->GetDims().y;
			}

			const LTFLOAT gravity = 2000.0f;
			const LTFLOAT speed_sqr_div_grav = shot_speed*shot_speed / gravity;
			vResult.y += speed_sqr_div_grav - (LTFLOAT)sqrt( speed_sqr_div_grav*speed_sqr_div_grav - shot_horiz_dist*shot_horiz_dist - 2*speed_sqr_div_grav*shot_vert_offset);
		}


#ifdef TARGET_PREDICTION_DEBUG
		g_pLTServer->CPrint("Target at range %f, proj. speed is %f, shot time is %f.",
			shot_dist, LTFLOAT(pAmmoData->pProjectileFX->nVelocity), shot_dist / LTFLOAT(pAmmoData->pProjectileFX->nVelocity) );

		LineSystem::GetSystem(this,"ShowPrediction") 
			<< LineSystem::Clear()
			<< LineSystem::Box(target.GetPosition(),LTVector(50,50,50), Color::Red)
			<< LineSystem::Box(goal_pos,LTVector(50,50,50),Color::Blue);
#endif

	} //if( pAmmoData && pAmmoData->eType != VECTOR....

	return vResult;
}


bool NeedReload(const CWeapon & weapon, const AIWBM_AIWeapon & weapon_butes)
{
	const BARREL * pBarrelData = weapon.GetBarrelData();
	if( pBarrelData && pBarrelData->nShotsPerClip > 0 )
	{
		int nReloadPoint = weapon_butes.nMaxBurstShots;
		if( nReloadPoint > pBarrelData->nShotsPerClip )
			nReloadPoint = weapon_butes.nMinBurstShots;

		if(	weapon.GetAmmoInClip() < nReloadPoint )
		{
			return true;
		}
	}

	return false;
}

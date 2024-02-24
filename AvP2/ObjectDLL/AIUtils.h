// ----------------------------------------------------------------------- //
//
// MODULE  : AIUtils.h
//
// PURPOSE : AI helper stuff
//
// CREATED : 04.29.1999
//
// ----------------------------------------------------------------------- //

#ifndef __AI_UTILS_H__
#define __AI_UTILS_H__

#include <string>


// Helper functions

inline LTBOOL IsTrueChar(char ch)
{
	return (ch == 't' || ch == 'T' || ch == 'y' || ch == 'Y' || ch == '1');
}

inline LTBOOL IsFalseChar(char ch)
{
	return (ch == 'f' || ch == 'F' || ch == 'n' || ch == 'N' || ch == '0');
}

// Enums

enum SenseOutcome
{
	soNone					= 255,

	// Stimulation outcomes

	soFullStimulation		= 1,
	soFalseStimulationLimit	= 2,
	soFalseStimulation		= 3,

	// Delay outcomes

	soReactionDelayFinished	= 4,
};

// Statics

extern int g_cIntersectSegmentCalls;

// Constants

const LTFLOAT c_fCos90	= 0.0f;
const LTFLOAT c_fCos88	= 0.034899496f;
const LTFLOAT c_fCos85	= 0.087155742f;
const LTFLOAT c_fCos80	= 0.173648178f;
const LTFLOAT c_fCos75	= 0.258819045f;
const LTFLOAT c_fCos70	= 0.342020143f;
const LTFLOAT c_fCos60	= 0.5f;
const LTFLOAT c_fCos55	= 0.573576436f;
const LTFLOAT c_fCos45	= 0.707106781f;
const LTFLOAT c_fCos35	= 0.819152044f;
const LTFLOAT c_fCos30	= 0.866025404f;
const LTFLOAT c_fCos20	= 0.93969262f;
const LTFLOAT c_fCos15	= 0.965925826f;
const LTFLOAT c_fCos10	= 0.984807753f;
const LTFLOAT c_fCos5	= 0.996194498f;
const LTFLOAT c_fCos2	= 0.999390827f;

const LTFLOAT c_fSin90	= 1.0f;
const LTFLOAT c_fSin80	= c_fCos10;
const LTFLOAT c_fSin70	= c_fCos20;
const LTFLOAT c_fSin60	= c_fCos30;
const LTFLOAT c_fSin45	= c_fCos45;
const LTFLOAT c_fSin35	= c_fCos55;
const LTFLOAT c_fSin30	= c_fCos60;
const LTFLOAT c_fSin15	= c_fCos75;
const LTFLOAT c_fSin10	= c_fCos80;
const LTFLOAT c_fSin5	= c_fCos85;
const LTFLOAT c_fSin2	= c_fCos88;

LTFLOAT GetAIUpdateDelta();
extern const LTFLOAT c_fAIDeactivationTime;

extern const LTFLOAT c_fFacingThreshhold;

extern const char c_szKeyFireWeapon[];
extern const char c_szKeyBodySlump[];
extern const char c_szKeyPickUp[];

extern const char c_szActivate[];

extern char c_szNoReaction[];




// Utility functions.

#ifndef _FINAL

void AICPrint(HOBJECT hAI, const char *pMsg, ...);
void AICPrint(const LPBASECLASS pObject, const char *pMsg, ...);

void AIErrorPrint(HOBJECT hAI, const char *pMsg, ...);
void AIErrorPrint(const LPBASECLASS pObject, const char *pMsg, ...);

#endif

std::string VectorToString(const LTVector & vec);

// This _should_ go into ServerUtilities.h, but stlport does not
// behave well in a pre-compiled header  (stdafx.h includes ServerUtilities.h).
ILTMessage & operator<<(ILTMessage & out, const std::string & x);
ILTMessage & operator>>(ILTMessage & in,  std::string & x);

#ifndef _FINAL
bool ShouldShow(HOBJECT hObject);
#endif

inline std::string FullSoundName(const std::string & unique_part)
{
#ifdef _WIN32
	std::string full_name = "Sounds\\Dialogue\\";
#else
	std::string full_name = "Sounds/Dialogue/";
#endif

	full_name += unique_part;
	full_name += ".wav";

	return full_name;
}

struct SequintialBaseName
{
	std::string m_strName;
	int			m_nValue;

	SequintialBaseName( const std::string & name );

	HOBJECT FindObject() const;
};


LTFLOAT GetDifficultyFactor();

// Weapons stuff
class CAITarget;
class CWeapon;
class CAI;
struct AIWBM_AIWeapon;

LTVector TargetAimPoint(const CAI & ai, const CAITarget & target, const CWeapon & weapon);
bool     NeedReload(const CWeapon & weapon, const AIWBM_AIWeapon & weapon_butes);


#endif


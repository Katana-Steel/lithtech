// ----------------------------------------------------------------------- //
//
// MODULE  : AIMiser.cpp
//
// PURPOSE : Implementation of AI Miser
//
// CREATED : 04.20.1999
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIMiser.h"
#include "AIButeMgr.h"

CAIMiser* g_pAIMiser;

// CAIMiserToken

CAIMiserToken::CAIMiserToken()
{
	m_fActivity = 0.0f;
	m_fTimeStart = 0.0f;
	m_fTimeStop = 0.0f;
}

void CAIMiserToken::Save(HMESSAGEWRITE hWrite)
{
	SAVE_FLOAT(m_fActivity);
	SAVE_FLOAT(m_fTimeStart);
	SAVE_FLOAT(m_fTimeStop);
}
						 
void CAIMiserToken::Load(HMESSAGEREAD hRead)
{
	LOAD_FLOAT(m_fActivity);
	LOAD_FLOAT(m_fTimeStart);
	LOAD_FLOAT(m_fTimeStop);
}

// CAIMiser

CAIMiser::CAIMiser()
{
	m_fActivityDialog = -1.0f;
	m_fActivityMove = -1.0f;
	m_fActivityShoot = -1.0f;
}

void CAIMiser::Init()
{
	g_pAIMiser = this;
}

void CAIMiser::Term()
{
	g_pAIMiser = NULL;
}

CAIMiserToken CAIMiser::RequestTokenDialog(DFLOAT fPriority)
{
	CAIMiserToken token;

	DFLOAT fDelay = m_fActivityDialog * (1.0f - fPriority);

	token.SetTimeStart(g_pServerDE->GetTime() + fDelay);
	token.SetTimeStop(g_pServerDE->GetTime() + fDelay + 1.0f + 2.0f*fPriority);
	token.SetActivity(1.0f);

	m_fActivityDialog += token.GetActivity();

	return token;
}

CAIMiserToken CAIMiser::RequestTokenMove(DFLOAT fPriority)
{
	CAIMiserToken token;

	DFLOAT fDelay = m_fActivityMove * (1.0f - fPriority);

	token.SetTimeStart(g_pServerDE->GetTime() + fDelay);
	token.SetTimeStop(g_pServerDE->GetTime() + fDelay + 1.0f + 2.0f*fPriority);
	token.SetActivity(1.0f);

	m_fActivityMove += token.GetActivity();

	return token;
}

CAIMiserToken CAIMiser::RequestTokenShoot(DFLOAT fPriority)
{
	CAIMiserToken token;

	DFLOAT fDelay = m_fActivityShoot * (1.0f - fPriority);

	token.SetTimeStart(g_pServerDE->GetTime() + fDelay);
	token.SetTimeStop(g_pServerDE->GetTime() + fDelay + 1.0f + 2.0f*fPriority);
	token.SetActivity(1.0f);

	m_fActivityShoot += token.GetActivity();

	return token;
}

void CAIMiser::ReleaseTokenDialog(CAIMiserToken& token)
{
	m_fActivityDialog -= token.GetActivity();
}

void CAIMiser::ReleaseTokenMove(CAIMiserToken& token)
{
	m_fActivityMove -= token.GetActivity();
}

void CAIMiser::ReleaseTokenShoot(CAIMiserToken& token)
{
	m_fActivityShoot -= token.GetActivity();
}

void CAIMiser::Save(HMESSAGEWRITE hWrite)
{
	SAVE_FLOAT(m_fActivityDialog);
	SAVE_FLOAT(m_fActivityMove);
	SAVE_FLOAT(m_fActivityShoot);
}
						 
void CAIMiser::Load(HMESSAGEREAD hRead)
{
	LOAD_FLOAT(m_fActivityDialog);
	LOAD_FLOAT(m_fActivityMove);
	LOAD_FLOAT(m_fActivityShoot);
}

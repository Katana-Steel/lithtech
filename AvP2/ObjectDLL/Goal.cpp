// ----------------------------------------------------------------------- //
//
// MODULE  : Goal.cpp
//
// PURPOSE : Path information implementation
//
// CREATED : 5/19/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Goal.h"
//#include "AIHuman.h"
//#include "AIHumanState.h"
#include "AI.h"
#include "AIState.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"
#include "ServerSoundMgr.h"
#include "SoundButeMgr.h"
#include "AIAnimButeMgr.h"
#include "AIActionMisc.h"

void Goal::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	// createName set at construction.
	*hRead >> m_bActive;

	*hRead >> m_nRandomSoundBute;
	*hRead >> m_tmrRandomSound;
}

void Goal::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	// createName set at construction.
	*hWrite << m_bActive;

	*hWrite << m_nRandomSoundBute;
	*hWrite << m_tmrRandomSound;
}

void Goal::HandleGoalParameters(const char * szParameters)
{
	// The string we receive should be parsable.

	ConParse parse( const_cast<char*>(szParameters) );

	while( g_pLTServer->Common()->Parse(&parse) == LT_OK )
	{
		if( parse.m_nArgs > 0 )
			HandleParameter(parse.m_Args,parse.m_nArgs);
	}
}


void Goal::Start() 
{ 
	m_bActive = true; 

	m_nRandomSoundBute = -1;

	// Possibly play an initial animation.
	const char * szAnimSetName = GetInitialAnimationSet();

	if( szAnimSetName && szAnimSetName[0] )
	{
		const AIAnimSet* pAnimSet = g_pAIAnimButeMgr->GetAnimSetPtr( GetAIPtr()->GetButes()->m_szAIAnimType, szAnimSetName);
		if( pAnimSet && (pAnimSet->m_fPlayChance > GetRandom(0.0f,1.0f))  )
		{
			AIActionPlayAnim* pPlayAnim = dynamic_cast<AIActionPlayAnim*>( GetAIPtr()->SetNextAction(CAI::ACTION_PLAYANIM) );
			if( pPlayAnim )
			{
				pPlayAnim->Init(pAnimSet->GetRandomAnim());
			}
		}
	}

	if( GetRandomSoundName() )
	{
		CString strSoundButes = g_pServerSoundMgr->GetSoundSetName( g_pCharacterButeMgr->GetGeneralSoundDir(GetAIPtr()->GetCharacter()), GetRandomSoundName() );

		if( !strSoundButes.IsEmpty() )
		{
			m_nRandomSoundBute = g_pSoundButeMgr->GetSoundSetFromName(strSoundButes);

			if( m_nRandomSoundBute > 0 )
			{
				const SoundButes & butes = g_pSoundButeMgr->GetSoundButes(m_nRandomSoundBute);
				m_tmrRandomSound.Start( butes.m_fMinDelay );
			}
		}
	}
}

void Goal::Update()
{
	if( m_nRandomSoundBute > 0 && m_tmrRandomSound.Stopped() )
	{
		GetAIPtr()->PlayExclamationSound( GetRandomSoundName() );

		if( GetAIPtr()->IsPlayingVoicedSound() )
			m_tmrRandomSound.Start();
	}
}


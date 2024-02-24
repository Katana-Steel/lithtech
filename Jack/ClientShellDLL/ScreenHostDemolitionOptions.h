// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostDemolitionOptions.h
//
// PURPOSE : Interface screen for hosting multi player demolition games
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef __SCREENHOSTDEMOLITIONOPTIONS_H__
#define __SCREENHOSTDEMOLITIONOPTIONS_H__

#include "BaseScreen.h"
#include "NetDefs.h"

class CScreenHostDemolitionOptions : public CBaseScreen
{
public:
	CScreenHostDemolitionOptions();
	virtual ~CScreenHostDemolitionOptions();

	// Build the screen
    LTBOOL   Build();

    void    OnFocus(LTBOOL bFocus);


protected:


protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	int			m_nMaxPlayers;
	int			m_nRunSpeed;
	int			m_nRounds;
	LTBOOL		m_bFriendlyFire;

	int			m_nFragScore;

	LTBOOL		m_bWeaponsStay;


	CLTGUISlider*	m_pMaxPlayers;
	CLTGUITextCtrl*	m_pTeam1; 
	CLTGUITextCtrl*	m_pTeam2; 
	CLTGUITextCtrl*	m_pTeam1Name; 
	CLTGUITextCtrl*	m_pTeam2Name; 

};

#endif // _SCREEN_HOST_DM_OPTIONS_H_
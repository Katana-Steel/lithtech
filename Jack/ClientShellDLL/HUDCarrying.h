// ----------------------------------------------------------------------- //
//
// MODULE  : HUDCarrying.h
//
// PURPOSE : HUDItem to display an icon while carrying a body
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_CARRYING_H
#define __HUD_CARRYING_H

#include "HUDItem.h"



//******************************************************************************************
//** HUD Carry Icon display
//******************************************************************************************
class CHUDCarrying : public CHUDItem
{
public:
	CHUDCarrying();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

	bool		AddKey( uint32 nKeyId );
	bool		RemoveKey( uint32 nKeyId );
	void		ClearAllKeys( );

private:
    LTIntPt		m_BasePos;
	uint16		m_nSize;

	enum	eCarryBodyIcons
	{
		eBodyCanDrop,
		eBodyNoDrop,
		eBodyCanCarry,
		kNumCarryBodyIcons
	};


	LTPoly_GT4	m_Poly;
	HTEXTURE	m_hIcon[kNumCarryBodyIcons];
	
	struct ChassisPieceIcon
	{
		ChassisPieceIcon( )
		{
			m_hCarryingIcon = m_hPickupIcon = NULL;
		}

		HTEXTURE	m_hCarryingIcon;
		HTEXTURE	m_hPickupIcon;
	};

	typedef std::vector< ChassisPieceIcon > ChassisPieceIconList;
	ChassisPieceIconList m_lstChassisPieceIcon;

	// Holds individual keyicon.
	struct KeyIcon
	{
		KeyIcon( )
		{
			m_hIcon = NULL;
			m_nKeyIndex = 0;
		}

		bool operator==( KeyIcon const& other ) const
		{
			return ( m_nKeyIndex == other.m_nKeyIndex );
		}

		HTEXTURE	m_hIcon;
		uint32		m_nKeyIndex;
		LTIntPt		m_Size;
	};

	// Can have 0 or more keys shown in carrying list.
	typedef std::vector< KeyIcon > KeyIconList;
	KeyIconList m_lstKeyIcons;
};

#endif
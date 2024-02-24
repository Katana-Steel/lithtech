// WeaponChooser.h: interface for the CWeaponChooser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WEAPONCHOOSER_H__1762B140_8553_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_WEAPONCHOOSER_H__1762B140_8553_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//external dependencies
#include "stdlith.h"
#include "LithFontMgr.h"

class CPlayerStats;

class CWeaponChooser  
{
public:
	virtual ~CWeaponChooser() {}
	virtual void	Init()			= 0;
	virtual void	Term()			= 0;
	virtual DBOOL	Open()			= 0;
	virtual void	Close()			= 0;
	virtual DBOOL	IsOpen()		= 0;
	virtual void	NextWeapon()	= 0;
	virtual void	PrevWeapon()	= 0;
	virtual void	Draw()			= 0;
	virtual DDWORD	GetCurrentSelection() = 0;
};

class CMarineWepChooser : public CWeaponChooser
{
public:
	CMarineWepChooser();
	~CMarineWepChooser();

	void	Term();
	DBOOL	Open();
	void	Close();
	DBOOL	IsOpen()	{return m_bIsOpen;}
	void	NextWeapon();
	void	PrevWeapon();
	void	Draw();
	DDWORD	GetCurrentSelection() { return m_nCurWeapon; }

	//new AvP3 Functions
	void	Init();

private:
	DBOOL		HaveAmmo(DBYTE nWeaponID);
	void		DrawAmmoBars(HSURFACE hScreen, DBYTE nWeaponID, DWORD nXOffset, DWORD nYOffset);

	HSURFACE*	m_ahWeaponSurf;
	DBOOL		m_bIsOpen;
	LTBOOL		m_bClosing;

	float		m_fStartTime;
	float		m_fFinishTime;

	//new AvP3 Variables
	HSURFACE	m_ahSmallIcon[2];	//0=green block, 1=red block
	HSURFACE	m_hSelectBox;		//border box for selected weapon
	int			m_nCurWeapon;		//current weapon index
	DDWORD		m_nLastWeapon;		//index number of last weapon
	DDWORD		m_nFirstWeapon;		//index number of first weapon
	DDWORD		m_nMaxWeapons;		//max number of weapons
	DDWORD		m_nIconWidth;		//width in pixels of icon images
	DDWORD		m_nIconHeight;		//height in pixels of icon images
	DDWORD		m_nSmIconWidth;		//width in pixels of small icon images
	DDWORD		m_nSmIconHeight;	//height in pixels of small icon images
	LithFont*	m_plfIconFontWht;	//pointer to white lith font for icon labels		
	LithFont*	m_plfIconFontBlk;	//pointer to black lith font for icon labels
	float		m_fResetTime;		//time chooser was last reset
	DDWORD		m_nScreenWidth;		//current screen width
	DDWORD		m_nScreenHeight;	//current screen height
	CPlayerStats*	m_pPlayerStats;	//pointer to player stats class

	//new AvP3 Functions
	void		LoadImages();
	void		LoadFonts();
};


class CPredatorWepChooser : public CWeaponChooser
{
public:
	CPredatorWepChooser();
	~CPredatorWepChooser();

	void	Term();
	DBOOL	Open();
	void	Close();
	DBOOL	IsOpen()	{return m_bIsOpen;}
	void	NextWeapon();
	void	PrevWeapon();
	void	Draw();
	DDWORD	GetCurrentSelection() { return m_nCurWeapon; }

	//new AvP3 Functions
	void	Init();

private:
	DBOOL		HaveAmmo(DBYTE nWeaponID);
	void		DrawAmmoBars(HSURFACE hScreen, DBYTE nWeaponID, DWORD nXOffset, DWORD nYOffset, DBYTE nId);

	HSURFACE*	m_ahWeaponSurf;
	DBOOL		m_bIsOpen;
	LTBOOL		m_bClosing;
	float		m_fStartTime;
	float		m_fFinishTime;

	//new AvP3 Variables
	HSURFACE	m_ahEnergyIcon[2];	//0=green block, 1=red block
	HSURFACE	m_hSelectBox;		//border box for selected weapon
	int			m_nCurWeapon;		//current weapon index
	DDWORD		m_nLastWeapon;		//index number of last weapon
	DDWORD		m_nFirstWeapon;		//index number of first weapon
	DDWORD		m_nMaxWeapons;		//max number of weapons
	DDWORD		m_nIconWidth;		//width in pixels of icon images
	DDWORD		m_nIconHeight;		//height in pixels of icon images
	DDWORD		m_nSmIconWidth;		//width in pixels of small icon images
	DDWORD		m_nSmIconHeight;	//height in pixels of small icon images
	LithFont*	m_plfIconFontWht;	//pointer to white lith font for icon labels		
	LithFont*	m_plfIconFontBlk;	//pointer to black lith font for icon labels
	float		m_fResetTime;		//time chooser was last reset
	DDWORD		m_nScreenWidth;		//current screen width
	DDWORD		m_nScreenHeight;	//current screen height
	CPlayerStats*	m_pPlayerStats;	//pointer to player stats class

	//new AvP3 Functions
	void		LoadImages();
	void		LoadFonts();
	void		SetDrawOffset();

};
#endif // !defined(AFX_WEAPONCHOOSER_H__1762B140_8553_11D3_B2DB_006097097C7B__INCLUDED_)

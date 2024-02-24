// ----------------------------------------------------------------------- //
//
// MODULE  : HudMgr.h
//
// PURPOSE : HudMgr definition
//
// CREATED : 01.21.2000
//
// ----------------------------------------------------------------------- //

#ifndef __HUDMGR_H
#define __HUDMGR_H

//external dependencies

#include "Subtitle.h"
#include "LTGUIMgr.h"

struct WEAPON;
struct BARREL;
struct AMMO;
struct AMMO_POOL;
class CPlayerStats;
class CMotionDetector;

#define MAX_PICKUP_LIST		20
#define MAX_COUNTER_ITEMS	16


enum CounterType
{
	CT_INVALID=-1,
	CT_AMMO_ONLY,
	CT_CLIPS,
	CT_CLIP,
};

struct PickupList
{
	PickupList::PickupList()
	{
		bUsed		= LTFALSE;
		hSurface	= LTNULL;
		ptLocation	= LTIntPt(0,0);
		fStartTime	= 0.0f;
	}
	
	void PickupList::Reset()
	{
		bUsed		= LTFALSE;
		hSurface	= LTNULL;
		ptLocation	= LTIntPt(0,0);
		fStartTime	= 0.0f;
	}
	
	LTBOOL		bUsed;
	HSURFACE	hSurface;
	LTIntPt		ptLocation;
	LTFLOAT		fStartTime;	
};

struct CounterItem
{
	CounterItem::CounterItem()
	{
		eType		= CT_INVALID;
		hStr		= LTNULL;
		nNumDigits	= 0; 
		hIcon		= LTNULL;
		ptStrLoc	= LTIntPt(0,0);
		ptIconLoc	= LTIntPt(0,0);
		pFont		= 0;
		pBarrel		= LTNULL;
		pAmmo		= LTNULL;
		pPool		= LTNULL;
		pBarrel		= LTNULL;
		pAmmo		= LTNULL;
		pPool		= LTNULL;
	}

	void CounterItem::Reset()
	{
		eType		= CT_INVALID;
		if(hStr) 
			{ 
				g_pLTClient->FreeString(hStr); hStr=LTNULL; 
			}
		hIcon		= LTNULL;	// Shared surface (no need to clean up)
		nNumDigits	= 0; 
		ptStrLoc	= LTIntPt(0,0);
		ptIconLoc	= LTIntPt(0,0);
		pFont		= 0;
		pBarrel		= LTNULL;
		pAmmo		= LTNULL;
		pPool		= LTNULL;
		pBarrel		= LTNULL;
		pAmmo		= LTNULL;
		pPool		= LTNULL;
	}

	CounterType	eType;
	HSTRING		hStr;
	uint8		nNumDigits;
	HSURFACE	hIcon;
	LTIntPt		ptStrLoc;
	LTIntPt		ptIconLoc;
	int			pFont;
	BARREL*		pBarrel;
	AMMO*		pAmmo;
	AMMO_POOL*	pPool;
};

//prototypes
class CHudElement;
class CHudMgr;

class CHudElement
{
public:
	//constructor / destructor
	CHudElement			();
	~CHudElement		() { Term(); } 

	//utility functions
	void 	Init				(char * pTag, uint32 nIndex);
	void	Term				();
	void	LoadAnimation		(char * pTag, uint32 nIndex);
	void	SetRects			();

	void	(CHudMgr::*m_pDrawFunction)		(CHudElement *pHudElement);
	void	(CHudMgr::*m_pUpdateFunction)	(CHudElement *pHudElement);

	//setting functions
	void SetOffset		(LTIntPt ptOffset)	{ m_ptOffset=ptOffset; }
	void SetEnabled		(LTBOOL bEnabled)	{ m_bEnabled=bEnabled; }
	void SetSurface		(HSURFACE hSurf)	{ m_hSurface=hSurf; }
	void SetAltSurfAry	(HSURFACE* hSurf)	{ m_ahAltSurf=hSurf; }
	void SetString		(HSTRING hStr);
	void SetAltString	(HSTRING hStr);
	void SetDisplayRect	(LTRect rc)			{ m_rcDspSrc=rc; }
	void SetScaleX		(LTFLOAT fX)		{ m_fHorScale=fX; }
	void SetScaleY		(LTFLOAT fY)		{ m_fVerScale=fY; }
	void SetDestPt		(LTIntPt pt)		{ m_ptDest = pt; }


	//getting functions
	LTIntPt	GetOffset	()		{ return m_ptOffset; }
	LTBOOL	GetEnabled	()		{ return m_bEnabled; }
	HSURFACE GetSurface ()		{ return m_hSurface; }
	HSURFACE* GetAltSurfAry ()	{ return m_ahAltSurf; }
	HSURFACE* GetAnimSurf  ()	{ return m_ahAnimSurf; }
	int		GetDestX	()		{ return m_ptDest.x; }
	int		GetDestY	()		{ return m_ptDest.y; }
	LTRect	GetHalfSrcRect	()	{ return m_rcHalfHeight; }
	LTRect	GetImgSrcRect	()	{ return m_rcImgSrc; }
	uint32	GetFontIndex	()	{ return m_nFontIndex; }
	LTFLOAT	GetTranslucency ()	{ return m_fTranslucency; }
	HSTRING GetString		()	{ return m_hszString; }
	HSTRING GetAltString	()	{ return m_hszAltString; }
	uint32	GetNumAnimFrames()	{ return m_nNumAnim; }
	LTRect	GetDisplayRect()	{ return m_rcDspSrc; }
	LTFLOAT	GetScaleX()			{ return m_fHorScale; }
	LTFLOAT	GetScaleY()			{ return m_fVerScale; }

private:
	uint32		m_nID;			//hud element id
	LTBOOL		m_bEnabled;		//enables this hud element
	LTIntPt		m_ptOffset;		//where this element is displayed
	uint32		m_nOffsetType;	//type of offset
	LTIntPt		m_ptDest;		//destination top left corner
	LTRect		m_rcHalfHeight;	//top half of source rectangle
	LTRect		m_rcImgSrc;		//entire source rectangle
	LTRect		m_rcDspSrc;		//display source rectangle
	LTBOOL		m_bTransparent; //does surface use transparancy
	LTBOOL		m_bTranslucent; //does surface use translucency
	LTFLOAT		m_fTranslucency;//amount of translucency
	uint32		m_nFontIndex;	//index into font array
	uint32		m_nNumAnim;		//number of animation frames
	HSURFACE	m_hSurface;		//this element's surface
	HSURFACE*	m_ahAnimSurf;	//array of animation surfaces	
	HSURFACE*	m_ahAltSurf;	//optional alternate staging surface
	HSTRING		m_hszString;	//string
	HSTRING		m_hszAltString;	//alt string
	LTFLOAT		m_fHorScale;	//horizontal scaling factor
	LTFLOAT		m_fVerScale;	//vertical scaling factor
};


class CHudMgr
{
public:
	//constructor / destructor
	CHudMgr				();
	~CHudMgr			() { Term(); }

	//utility functions
	void Save			(HMESSAGEWRITE hWrite);
	void Load			(HMESSAGEREAD hRead);
	LTBOOL Init			(uint32 nIndex=0, LTBOOL bRestore=LTFALSE, LTBOOL bForce=LTFALSE);
	void SetFunctionPointers (char * pTag, uint32 nIndex);
	void LoadFonts		(char * pTag);
	void DrawHud		();
	void UpdateHud		();
	void Reset			() { m_nHudId = -1; }
	
	//set functions
	void SetEnabled		(LTBOOL bEnabled=DTRUE) { m_bHudEnabled=bEnabled; }
	void SetMDEnabled	(LTBOOL bEnabled=DTRUE) { m_bMDEnabled=bEnabled; }
	void SetMDDraw		(LTBOOL bEnabled=DTRUE) { m_bMDDraw=bEnabled; }


	//get functions
	LTBOOL GetHudEnabled () { return m_bHudEnabled; }

	//interface functions
	void	OnObjectMove		(HOBJECT hObj, LTBOOL bTeleport, LTVector pPos);
	void	OnObjectRemove		(HLOCALOBJ hObj);
	void	NewWeapon			(WEAPON* pNewWeapon);
	void	SetTrackerObject	(HMESSAGEREAD hMessage);
	void	RemoveTracker		();
	void	AddAmmoPickup		(uint32 nAmmoPoolId);
	void	AddWeaponPickup		(uint32 nWepId);
	void	AddHealthPickup		();
	void	AddArmorPickup		();
	void	AddMaskPickup		();
	void	AddNightVisionPickup();
	void	AddCloakPickup		();
	void	DrawCloseCaption	();
	void	ResetCounter();

	void	ShowBossHealth		(LTBOOL bShow) { m_bBossHealthOn = bShow; }
	void	SetBossHealth		(LTFLOAT fRatio) { m_fBossHealthRatio = fRatio; }

private:
	//hud drawing functions
	void NormalDrawTransparent		(CHudElement *pHudElement);
	void NormalDrawNonTransparent	(CHudElement *pHudElement);
	void NullFunction0				(CHudElement *pHudElement){}
	void DrawBmFontText				(CHudElement *pHudElement);
	void DrawHealthArmorFill		(CHudElement *pHudElement);
	void DrawAlienHealth			(CHudElement *pHudElement);
	void DrawPredatorHealth			(CHudElement *pHudElement);
	void DrawPredatorEnergy			(CHudElement *pHudElement);
	void DrawMarineEKG				(CHudElement *pHudElement);
	void DrawMarineAmmoCounter		(CHudElement *pHudElement);
	void DrawAirMeter				(CHudElement *pHudElement);
	void DrawMarineAirMeterFill		(CHudElement *pHudElement);
	void DrawPredatorAirMeterFill	(CHudElement *pHudElement);
	void DrawMarinePickupList		(CHudElement *pHudElement);
	void DrawChestbursterHealth		(CHudElement *pHudElement);
	void DrawPredatorPickupList		(CHudElement *pHudElement);
	void DrawExoArmor				(CHudElement *pHudElement);
	void DrawExoEnergy				(CHudElement *pHudElement);
	void DrawBatteryMeter			(CHudElement *pHudElement);
	void DrawBatteryMeterFill		(CHudElement *pHudElement);
	void DrawBossHealth				(CHudElement *pHudElement);
	void DrawBossHealthFill			(CHudElement *pHudElement);
	void DrawSkullCounter			(CHudElement *pHudElement);

	//hud update functions
	void NoUpdate					(CHudElement *pHudElement){ return; }
	void UpdateHealthStr			(CHudElement *pHudElement);
	void UpdateArmorStr				(CHudElement *pHudElement);
	void UpdateHealthFill			(CHudElement *pHudElement);
	void UpdateArmorFill			(CHudElement *pHudElement);
	void UpdateAlienHealth			(CHudElement *pHudElement);
	void UpdateMarineEKG			(CHudElement *pHudElement);
	void UpdateMarineAmmoCount		(CHudElement *pHudElement);
	void UpdatePickupList			(CHudElement *pHudElement);
	void UpdateExoArmor				(CHudElement *pHudElement);
	void UpdateExoEnergy			(CHudElement *pHudElement);
	void UpdateFlareStr				(CHudElement *pHudElement);

	//ammo counter functions
	void NoCounter			(CHudElement *pHudElement) { return; }
	void NormalCounter		(CHudElement *pHudElement);
	void MultiAmmoCounter	(CHudElement *pHudElement);
	void SpearAmmoCounter	(CHudElement *pHudElement);
	void ExosuitAmmoCounter	(CHudElement *pHudElement);

	void NoDrawCounter			(CHudElement *pHudElement) { return; }
	void NormalDrawCounter		(CHudElement *pHudElement);
	void SpearAmmoDrawCounter	(CHudElement *pHudElement);

	//ammo counter init functions
	void InitNormalCounter	(WEAPON* pNewWeapon);
	void InitMultiCounter	(WEAPON* pNewWeapon);
	void InitExoCounter		();

	//current ammo counter function pointer
	void (CHudMgr::*m_pAmmoCounterFunction)	(CHudElement *pHudElement);
	void (CHudMgr::*m_pAmmoDrawFunction)	(CHudElement *pHudElement);

	//misc support functions for drawing and update
	HSURFACE	GetEKGSurface	(CHudElement *pHudElement);
	void		AddPickupIcon	(char *szIcon);
	void		LoadAmmoIcon (WEAPON* pNewWeapon);
	void		Term();

	uint32			m_nHudId;			//what hud is loaded
	HSURFACE		m_hAmmoIcon;		//ammo icon for pred ammo counters
	LTBOOL			m_bHudEnabled;		//enables the entire hud
	LTBOOL			m_bMDEnabled;		//enables the Motion detector, global override
	LTBOOL			m_bMDDraw;			//local toggle
	uint32			m_nNumHudElements;	//number of hud elements
	uint32			m_nNumHudFonts;		//number of hud fonts
	CHudElement*	m_paHudElements;	//array of hud elements
	LithFont**		m_aplfFonts;		//pointer to font array
	CPlayerStats*	m_pPlayerStats;		//pointer to player stats class
	CMotionDetector*m_pMotionDetector;	//pointer to motion detector object
	LTBOOL			m_bShowPickupList;	//should we show the pickup list.

	//ammo counter members
	BARREL*			m_pBaseBarrel;		//used for multi-ammo weapons (granade launcher)
	uint32			m_nNumBarrels;		//used for multi-ammo weapons (granade launcher)
	HSTRING*		m_aClipsString;		//array of strings for ammo counter num clips-1
	HSTRING*		m_aClipString;		//array of strings for ammo counter amount in active clip
	HSTRING*		m_aBarrelLabel;		//array of strings for ammo counter barrel label
	uint32			m_nActiveBarrel;	//index to active barrel
	uint32			m_WeaponID;			//the id of the weapon

	//pickup list info
	PickupList		m_PickupList[MAX_PICKUP_LIST];	//list of pick-up objects to be displayed

	//array of ammo counter items.
	CounterItem		m_CounterItems[MAX_COUNTER_ITEMS];	//list of ammo counter items to be displayed 


	LTBOOL			m_bBossHealthOn;	// TRUE if boss health meter is being displayed
	LTFLOAT			m_fBossHealthRatio;	// (current / max) health
};

#endif
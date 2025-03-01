//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

// Defines structures used by DEdit's d3d renderer.
#ifndef __DRAW_D3D_H__
#define __DRAW_D3D_H__
//#define DIRECT3D_VERSION 0x0700

	#include "drawmgr.h"
	#include "regiondoc.h"
	#include "regionview.h"
	#include "d3d.h"
	#include "edithelpers.h"
	#include "drawbase.h"
	#include "ddraw.h"

	//typedef struct IDirect3D3* LPDIRECT3D3;
	//typedef struct IDirect3DDevice3* LPDIRECT3DDEVICE3;
	//typedef struct IDirect3DMaterial3* LPDIRECT3DMATERIAL3;
	//typedef struct IDirect3DViewport3* LPDIRECT3DVIEWPORT3;
	//typedef struct IDirect3DVertexBuffer* LPDIRECT3DVERTEXBUFFER;

	//typedef float D3DVALUE, * LPD3DVALUE;

	//typedef struct _D3DPrimCaps {
	//	DWORD dwSize;
	//	DWORD dwMiscCaps;                 /* Capability flags */
	//	DWORD dwRasterCaps;
	//	DWORD dwZCmpCaps;
	//	DWORD dwSrcBlendCaps;
	//	DWORD dwDestBlendCaps;
	//	DWORD dwAlphaCmpCaps;
	//	DWORD dwShadeCaps;
	//	DWORD dwTextureCaps;
	//	DWORD dwTextureFilterCaps;
	//	DWORD dwTextureBlendCaps;
	//	DWORD dwTextureAddressCaps;
	//	DWORD dwStippleWidth;             /* maximum width and height of */
	//	DWORD dwStippleHeight;            /* of supported stipple (up to 32x32) */
	//} D3DPRIMCAPS, * LPD3DPRIMCAPS;

	//typedef struct _D3DDeviceDesc7 {
	//	DWORD            dwDevCaps;              /* Capabilities of device */
	//	D3DPRIMCAPS      dpcLineCaps;
	//	D3DPRIMCAPS      dpcTriCaps;
	//	DWORD            dwDeviceRenderBitDepth; /* One of DDBB_8, 16, etc.. */
	//	DWORD            dwDeviceZBufferBitDepth;/* One of DDBD_16, 32, etc.. */

	//	DWORD       dwMinTextureWidth, dwMinTextureHeight;
	//	DWORD       dwMaxTextureWidth, dwMaxTextureHeight;

	//	DWORD       dwMaxTextureRepeat;
	//	DWORD       dwMaxTextureAspectRatio;
	//	DWORD       dwMaxAnisotropy;

	//	D3DVALUE    dvGuardBandLeft;
	//	D3DVALUE    dvGuardBandTop;
	//	D3DVALUE    dvGuardBandRight;
	//	D3DVALUE    dvGuardBandBottom;

	//	D3DVALUE    dvExtentsAdjust;
	//	DWORD       dwStencilCaps;

	//	DWORD       dwFVFCaps;
	//	DWORD       dwTextureOpCaps;
	//	WORD        wMaxTextureBlendStages;
	//	WORD        wMaxSimultaneousTextures;

	//	DWORD       dwMaxActiveLights;
	//	D3DVALUE    dvMaxVertexW;
	//	GUID        deviceGUID;

	//	WORD        wMaxUserClipPlanes;
	//	WORD        wMaxVertexBlendMatrices;

	//	DWORD       dwVertexProcessingCaps;

	//	DWORD       dwReserved1;
	//	DWORD       dwReserved2;
	//	DWORD       dwReserved3;
	//	DWORD       dwReserved4;
	//} D3DDEVICEDESC7, * D3DDEVICEDESC;

	typedef struct TextureFormat_t
	{
		DDPIXELFORMAT	m_PF;
		
		// The texture type.
		// Higher numbers are better.  -1 means we don't support this format.
		int			m_TextureCode;
		
		// How many bits for r, g, and b (if not palettized).
		DWORD		m_RBits, m_GBits, m_BBits, m_ABits;

		// Shifting information for converting colors.
		// Note: there is no need for an alpha shift, because you can just
		//       OR by the (1-bit) alpha mask to turn alpha on in a pixel.
		DWORD		m_RRightShift, m_RLeftShift;
		DWORD		m_GRightShift, m_GLeftShift;
		DWORD		m_BRightShift, m_BLeftShift;

		DLink		m_Link;
	} TextureFormat;

	
	class D3DRender : public DrawBase
	{
	public:

					D3DRender(CRegionView *pView);
					~D3DRender();

		BOOL		Init(int deviceNum, int renderMode, CRect *pRect);
		void		Term();
	
	// Overrides.
	public:

		void		DrawTagRectPoly();
		void		Draw();
		void		Resize(int width, int height);

		void		DrawFlatPoly2(TLVertex *pPoints, DWORD nPoints, CVector *pNormal, COLORREF color);
		void		DrawTexturedPoly(CEditPoly *pPoly);
		void		DrawTintedTexturedPoly(CEditPoly* pPoly, DWORD nTintColor);

		//provides full control over rendering a textured polygon
		void		DrawTexturedPoly(	TLVertex* pVerts, uint32 nNumVerts,  
										BOOL bTint, DWORD nTintColor, DFileIdent_t *pTextureFile, 
										DFileIdent_t *pDetailFile, DWORD nFlatShade, uint32 nClipMask = 0xFFFFFFFF);

		void		DrawLine(TLVertex *pVerts);
		void		DrawVert(CVector &pos, D3DCOLOR borderColor, D3DCOLOR fillColor, int nSize);
		
		//draws a model for the specified object
		virtual void	DrawModel(CBaseEditObj* pObject, uint32 nMode, CMeshShapeList* pShapeList, LTMatrix& mObjTrans);

		BOOL		ZEnable(BOOL bEnable);


	public:

		void		InitVars();


	public:

		//determines if we are going to be displaying lightmaps this update
		bool					m_bDisplayLightMaps;

		//determines if we will be displaying vertex lit polygons this update
		bool					m_bDisplayVertexLight;

		//determines if we will be shading the brush polygons this update
		bool					m_bShadePolygons;


		// Used to determine the Z format we'll be using.
		BOOL					m_bGotZFormat;
		DDPIXELFORMAT			m_ZFormat;

		BOOL					m_bPerspectiveProjection;
		CDibMgr					*m_pDibMgr;	
		DLink					m_Link;
		
		LPDIRECTDRAW4			m_pDirectDraw;
		LPDIRECT3D3				m_pDirect3d;
		LPDIRECTDRAWSURFACE4	m_pSurface;
		LPDIRECTDRAWSURFACE4	m_pZSurface;
		LPDIRECTDRAWSURFACE4	m_pPrimarySurface;
		LPDIRECT3DDEVICE3		m_pDevice;
		LPDIRECT3DMATERIAL3		m_pMaterial;
		LPDIRECT3DVIEWPORT3		m_pViewport;
		LPDIRECTDRAWCLIPPER		m_pClipper;

		DWORD					m_SurfaceWidth, m_SurfaceHeight;

		// Device caps.
		D3DDEVICEDESC			m_Caps;

		// All the texture formats supported by this card.
		DLink					m_TextureFormats;
		TextureFormat			*m_pTextureFormat; // The texture format we chose to use.

		DLink					m_Textures;
		int						m_BindingNumber;  // Each drawmgr gets a unique number..

		int						m_DeviceNum, m_RenderMode;

	protected:

		//used for internally drawing a textured poly, with or without tint
		void		InternalDrawTexturedPoly(CEditPoly* pPoly, BOOL bTint, DWORD nTintColor);
	};


#endif  // __DRAW_D3D_H__




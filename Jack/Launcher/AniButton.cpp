// AniButton.cpp : implementation file
//

#include "stdafx.h"
#include "launcher.h"
#include "AniButton.h"
#include "dlgex.h"
#include "playsound.h"
#include "vfw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAniButton

CAniButton::CAniButton()
:	m_bPlaying	( FALSE ),
	m_nAniID	( 0 )
{
}

CAniButton::~CAniButton()
{
}


void CAniButton::DoDataExchange(CDataExchange* pDX) 
{	
	CButton::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAvibtntestDlg)
	DDX_Control(pDX, IDOK, m_AnimateCtrl);
	//}}AFX_DATA_MAP

}

BEGIN_MESSAGE_MAP(CAniButton, CButton)
	//{{AFX_MSG_MAP(CAniButton)
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAniButton message handlers



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTextCheckBox::AutoLoad
//
//  PURPOSE:	Open the .avi
//
//	NOTES:		Make sure LoadAvi is called before AutoLoad 
//				when adding an CAniButton to a dialog.
//
// ----------------------------------------------------------------------- //

BOOL CAniButton::AutoLoad(UINT nID, CWnd* pParent)
{
	// first attach the CBitmapCheckButton to the dialog control
	if (!SubclassDlgItem(nID, pParent))
		return FALSE;

	CRect	Rect;
	GetClientRect( Rect );

	// Create the animated control and size the button to the avi
	if( !::IsWindow( m_AnimateCtrl ) )
	{
		m_AnimateCtrl.Create( WS_CHILD | WS_VISIBLE | ACS_TRANSPARENT, Rect, this, 0 );
		m_AnimateCtrl.Open( m_nAniID );
		m_AnimateCtrl.GetClientRect(Rect);

   		SetWindowPos(NULL, 0, 0, Rect.Width()+2, Rect.Height()+2,
			SWP_NOMOVE|SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE);

		//Rect.OffsetRect(1,1);
       
		m_AnimateCtrl.MoveWindow(Rect); 
	}

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAniButton::DrawItem
//
//  PURPOSE:	Manualy draw the button
//
// ----------------------------------------------------------------------- //

void CAniButton::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct ) 
{
	// See if we have focus
	CDialogEx	*pParent = (CDialogEx*)GetParent();
	if( (pParent->GetFocusButton() == this) && (::IsWindow( m_AnimateCtrl )) )
	{
		if( !m_bPlaying )
		{
			m_AnimateCtrl.Play( 0, -1, 1 );
			m_bPlaying = TRUE;
		}
	}
	else
	{
		m_bPlaying = FALSE;
	}
	
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAniButton::OnMouseMove
//
//  PURPOSE:	Give us focus
//
// ----------------------------------------------------------------------- //

void CAniButton::OnMouseMove( UINT nFlags, CPoint point ) 
{
	// Let our parent know we have focus
	CDialogEx *pParent = (CDialogEx*)GetParent();
	if( pParent->GetFocusButton() != this ) 
	{
		pParent->SetFocusButton( this );

		PlaySound(IDR_SELECT);
	}
	
	CButton::OnMouseMove(nFlags, point);
}




// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAniButton::LoadAVI
//
//  PURPOSE:	Set the resource ID
//
// ----------------------------------------------------------------------- //

void CAniButton::LoadAVI(UINT nAniID)
{
    m_nAniID = nAniID;
}



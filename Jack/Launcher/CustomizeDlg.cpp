// CustomizeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "launcher.h"
#include "CustomizeDlg.h"
#include "textcheckbox.h"
#include "IO.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define CD_CLOSE_X		419
#define CD_CLOSE_Y		6	
#define CD_OK_X			115	
#define CD_OK_Y			387
#define CD_CANCEL_X		227
#define CD_CANCEL_Y		387
#define CD_NEXT_X		313
#define CD_NEXT_Y		340
#define CD_PREVIOUS_X	27
#define CD_PREVIOUS_Y	340

#define CD_COLUMN_1_X	28
#define CD_ROW_Y_START	74
#define CD_ROW_Y_OFFSET	21 // 20

////////////////////////////////////////////////////////////////////////////
// CustomizeDlg dialog


CustomizeDlg::CustomizeDlg(CWnd* pParent /*=NULL*/)
	: CMoveDialog(CustomizeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CustomizeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	for( int i = 0; i < CD_MAX_MODS_PER_PAGE; ++i )
	{
		m_apTextCheckBox[i] = NULL;
	}
}


void CustomizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CMoveDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CustomizeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CustomizeDlg, CMoveDialog)
	//{{AFX_MSG_MAP(CustomizeDlg)
	ON_BN_CLICKED(IDC_CD_CLOSE, OnClose)
	ON_BN_CLICKED(IDC_CD_CANCEL, OnCancel)
	ON_BN_CLICKED(IDC_CD_OK, OnOK)
	ON_BN_CLICKED(IDC_CD_NEXT, OnNext)
	ON_BN_CLICKED(IDC_CD_PREVIOUS, OnPrevious)

	ON_BN_CLICKED(IDC_CD_MOD_01, OnModButtonClicked)
	ON_BN_CLICKED(IDC_CD_MOD_02, OnModButtonClicked)
	ON_BN_CLICKED(IDC_CD_MOD_03, OnModButtonClicked)
	ON_BN_CLICKED(IDC_CD_MOD_04, OnModButtonClicked)
	ON_BN_CLICKED(IDC_CD_MOD_05, OnModButtonClicked)
	ON_BN_CLICKED(IDC_CD_MOD_06, OnModButtonClicked)
	ON_BN_CLICKED(IDC_CD_MOD_07, OnModButtonClicked)
	ON_BN_CLICKED(IDC_CD_MOD_08, OnModButtonClicked)
	ON_BN_CLICKED(IDC_CD_MOD_09, OnModButtonClicked)
	ON_BN_CLICKED(IDC_CD_MOD_10, OnModButtonClicked)
	ON_BN_CLICKED(IDC_CD_MOD_11, OnModButtonClicked)
	ON_BN_CLICKED(IDC_CD_MOD_12, OnModButtonClicked)

	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CustomizeDlg message handlers

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CustomizeDlg::OnClose
//
//	PURPOSE:	Close button handler
//
// ----------------------------------------------------------------------- //

void CustomizeDlg::OnClose()
{
	OnCancel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CustomizeDlg::OnInitDialog
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CustomizeDlg::OnInitDialog() 
{
	SetBackGround( IDB_CD_BACKGROUND );

	CMoveDialog::OnInitDialog();

	// Add the buttons to our dialog box...
	
	AddButton( IDC_CD_CLOSE, CD_CLOSE_X, CD_CLOSE_Y );
	AddButton( IDC_CD_OK, CD_OK_X, CD_OK_Y );
	AddButton( IDC_CD_CANCEL, CD_CANCEL_X, CD_CANCEL_Y );
	AddButton( IDC_CD_NEXT, CD_NEXT_X, CD_NEXT_Y );
	AddButton( IDC_CD_PREVIOUS, CD_PREVIOUS_X, CD_PREVIOUS_Y );

	// Create the buttons for a 'page' of mods...

	static UINT aModID[CD_MAX_MODS_PER_PAGE] = { IDC_CD_MOD_01, IDC_CD_MOD_02, IDC_CD_MOD_03, IDC_CD_MOD_04,
												 IDC_CD_MOD_05, IDC_CD_MOD_06, IDC_CD_MOD_07, IDC_CD_MOD_08,
												 IDC_CD_MOD_09, IDC_CD_MOD_10, IDC_CD_MOD_11, IDC_CD_MOD_12 };

	char szMod[64] = {0};
	for( int i = 0; i < CD_MAX_MODS_PER_PAGE; ++i )
	{
		int iPosY = (CD_ROW_Y_START + (i * CD_ROW_Y_OFFSET));

		sprintf( szMod, "CJ Modification %i", i);
		m_apTextCheckBox[i] = (CTextCheckBox*)AddTextCheckBox( aModID[i], CD_COLUMN_1_X, iPosY, szMod );

		if( !m_apTextCheckBox[i] )
			return false;

		m_apTextCheckBox[i]->ShowWindow( SW_HIDE );
	}

	// See if there was a previously selected mod...

	char szSelectedMod[256] = {0};
	DWORD bufSize = sizeof(szSelectedMod);
	
	if( theApp.m_RegMgr.GetField( FIELD_SELECTEDMOD, szSelectedMod, bufSize ) )
	{
		if( szSelectedMod[0] )
		{
			char szFile[MAX_PATH] = {0};
			sprintf( szFile, "%s\\%s", DIR_MODS, szSelectedMod );

			CFileFind	cFileFinder;

			BOOL bFound = cFileFinder.FindFile( szFile );
			if( bFound )
			{
				cFileFinder.FindNextFile();

				// Make sure it is actually a directory...

				if( !cFileFinder.IsDirectory() )
					bFound = false;
			}

			// End the search...
			
			cFileFinder.Close();

			// Only set the selected mod if it still exists...

			if( bFound )
			{
				m_sSelectedMod = szSelectedMod;
			}
		}
	}
	
	// Find all the mods we can chose and set the check box text to the name of the mod
	// and set the button visible...

	BuildCustomList();
	
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CustomizeDlg::OnCancel
//
//	PURPOSE:	Cancel button handler
//
// ----------------------------------------------------------------------- //

void CustomizeDlg::OnCancel() 
{
	CMoveDialog::OnCancel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CustomizeDlg::OnOK
//
//	PURPOSE:	Ok button handler
//
// ----------------------------------------------------------------------- //

void CustomizeDlg::OnOK() 
{
	// Write out the desired mod to the registry key...
	
	theApp.m_RegMgr.SetField( FIELD_SELECTEDMOD, const_cast<char*>(m_sSelectedMod.c_str()) );

	CMoveDialog::OnOK();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CustomizeDlg::OnPaint
//
//	PURPOSE:	WM_PAINT handler
//
// ----------------------------------------------------------------------- //

void CustomizeDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	PaintBackGround(&dc);
	
	// Do not call CMoveDialog::OnPaint() for painting messages
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CustomizeDlg::BuildCustomList
//
//	PURPOSE:	Builds the list of mods to choose from...
//
// ----------------------------------------------------------------------- //

bool CustomizeDlg::BuildCustomList()
{
	if( !DirExist( DIR_MODS ))
	{
		theApp.MessageBox( IDS_NOCUSTOMDIR, IDS_GAME_NAME, MB_OK | MB_ICONEXCLAMATION );
		return false;
	}

	// Start fresh...

	m_setMods.clear();

	// Begin searching the mods directory for any subdirectories that we consider to be a mod...
	
	char szFiles[MAX_PATH] = {0};
	sprintf( szFiles, "%s\\*", DIR_MODS);

	CFileFind	cFileFinder;

	BOOL bFound = cFileFinder.FindFile( szFiles );
	while( bFound )
	{
		bFound = cFileFinder.FindNextFile();

		// Ignore the 'dots' directories and all files...

		if( !cFileFinder.IsDirectory() || cFileFinder.IsDots() )
			continue;
		
		// Add the title, not the path, to the list of mod names...

		m_setMods.insert( std::string(cFileFinder.GetFileTitle()) );
	}

	// End the search...
	
	cFileFinder.Close();

	// Set the text and visibility of the buttons...

	StringSet::const_iterator iter = m_setMods.begin();
	for( int i = 0; i < CD_MAX_MODS_PER_PAGE; ++i, ++iter )
	{
		if( iter == m_setMods.end() )
			break;

		if( !m_apTextCheckBox[i] )
			return false;
		
		m_apTextCheckBox[i]->SetWindowText( (*iter).c_str() );
		m_apTextCheckBox[i]->ShowWindow( SW_NORMAL );
		m_apTextCheckBox[i]->SetCheck( FALSE );
		m_apTextCheckBox[i]->EnableWindow( (m_sSelectedMod.empty() ? TRUE : FALSE) );

		if( *iter == m_sSelectedMod )
		{
			m_apTextCheckBox[i]->SetCheck( TRUE );
			m_apTextCheckBox[i]->EnableWindow( TRUE );
		}
	}

	// If there are more mods we need to enable the 'next' button...
	// We are on the first page so no need to enable the 'prev' button...

	GetDlgItem( IDC_CD_NEXT )->EnableWindow( m_setMods.size() > CD_MAX_MODS_PER_PAGE ? TRUE : FALSE );
	GetDlgItem( IDC_CD_PREVIOUS )->EnableWindow( FALSE );
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CustomizeDlg::OnNext
//
//	PURPOSE:	Next button handler
//
// ----------------------------------------------------------------------- //

void CustomizeDlg::OnNext()
{
	// Get the name of the last mod from the current page...

	CString sModName;
	m_apTextCheckBox[CD_MAX_MODS_PER_PAGE - 1]->GetWindowText( sModName );
	
	StringSet::const_iterator iter = m_setMods.find( std::string(sModName) );

	// Increment the iterator so we are on the next mod...
	
	++iter;

	int nMods;
	for( nMods = 0; nMods < CD_MAX_MODS_PER_PAGE; ++nMods, ++iter )
	{
		if( iter == m_setMods.end() )
			break;

		if( !m_apTextCheckBox[nMods] )
			return;
		
		m_apTextCheckBox[nMods]->SetWindowText( (*iter).c_str() );
		m_apTextCheckBox[nMods]->ShowWindow( SW_NORMAL );
		m_apTextCheckBox[nMods]->SetCheck( FALSE );
		m_apTextCheckBox[nMods]->EnableWindow( (m_sSelectedMod.empty() ? TRUE : FALSE) );

		if( *iter == m_sSelectedMod )
		{
			m_apTextCheckBox[nMods]->SetCheck( TRUE );
			m_apTextCheckBox[nMods]->EnableWindow( TRUE );
		}
		
		// Force a re-draw...

		m_apTextCheckBox[nMods]->Invalidate( );
	}

	// If there are more mods we need to enable the 'next' button...
	// We have gone to a new page so the 'prev' button needs to be enabled...

	BOOL bNext = ( (nMods >= CD_MAX_MODS_PER_PAGE) && (iter != m_setMods.end()) );
	GetDlgItem( IDC_CD_NEXT )->EnableWindow( bNext );
	
	GetDlgItem( IDC_CD_PREVIOUS )->EnableWindow( TRUE );

	// If we didn't fill out a whole page of mods then we need to hide the rest of the buttons...

	for( ; nMods < CD_MAX_MODS_PER_PAGE; ++nMods )
	{
		m_apTextCheckBox[nMods]->ShowWindow( SW_HIDE );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CustomizeDlg::OnPrevious
//
//	PURPOSE:	Previous button handler
//
// ----------------------------------------------------------------------- //

void CustomizeDlg::OnPrevious()
{
	// Get the name of the first mod from the current page...

	CString sModName;
	m_apTextCheckBox[0]->GetWindowText( sModName );
	
	StringSet::const_iterator iter = m_setMods.find( std::string(sModName) );

	// Increment the iterator so we are on the next mod...
	
	--iter;

	int nMods;
	for( nMods = CD_MAX_MODS_PER_PAGE - 1; nMods > -1; --nMods, --iter )
	{
		if( !m_apTextCheckBox[nMods] )
			return;
		
		m_apTextCheckBox[nMods]->SetWindowText( (*iter).c_str() );
		m_apTextCheckBox[nMods]->ShowWindow( SW_NORMAL );
		m_apTextCheckBox[nMods]->SetCheck( FALSE );
		m_apTextCheckBox[nMods]->EnableWindow( (m_sSelectedMod.empty() ? TRUE : FALSE) );

		if( *iter == m_sSelectedMod )
		{
			m_apTextCheckBox[nMods]->SetCheck( TRUE );
			m_apTextCheckBox[nMods]->EnableWindow( TRUE );
		}
		
		// Force a re-draw...

		m_apTextCheckBox[nMods]->Invalidate( );

		if( iter == m_setMods.begin() )
			break;
	}

	// If there are more mods we need to enable the 'next' button...
	// We have gone to a new page so the 'prev' button needs to be enabled...

	GetDlgItem( IDC_CD_NEXT )->EnableWindow( TRUE );
	GetDlgItem( IDC_CD_PREVIOUS )->EnableWindow( (iter == m_setMods.begin() ? FALSE : TRUE) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CustomizeDlg::OnModButtonClicked
//
//	PURPOSE:	One of the mod buttons has been clicked...
//
// ----------------------------------------------------------------------- //

void CustomizeDlg::OnModButtonClicked()
{
	// Figure out which button was clicked and figure out if it was 
	// just selected or deselected...

	int nSelected = -1;
	m_sSelectedMod.clear();

	for( int i = 0; i < CD_MAX_MODS_PER_PAGE; ++i )
	{
		if( !m_apTextCheckBox[i] )
			return;

		if( m_apTextCheckBox[i]->GetCheck() )
		{
			nSelected = i;

			// Cache the selected mod...
			
			CString sModName;
			m_apTextCheckBox[i]->GetWindowText( sModName );
			m_sSelectedMod = std::string( sModName );

			break;
		}
	}

	// If none of the mods are selected be sure to enable them all.  Otherwise
	// disable them all but the selected one.

	for( i = 0; i < CD_MAX_MODS_PER_PAGE; ++i )
	{
		if( !m_apTextCheckBox[i] )
			return;

		if( nSelected == i )
			continue;

		m_apTextCheckBox[i]->EnableWindow( (nSelected >= 0 ? FALSE : TRUE) );
	}
}
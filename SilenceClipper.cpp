/****************************************************************************

                              SilenceClipper.cpp

						Copyright 2000, John J. Bolton
	----------------------------------------------------------------------

	$Header: //depot/SilenceClipper/SilenceClipper.cpp#2 $

	$NoKeywords: $

****************************************************************************/

#include "stdafx.h"

#include "SilenceClipper.h"

#include <vector>

#include "SilenceClipperDlg.h"
#include "ClipperThread.h"
#include "Misc/SafeStr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

BEGIN_MESSAGE_MAP( CSilenceClipperApp, CWinApp )
	//{{AFX_MSG_MAP( CSilenceClipperApp )
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND( ID_HELP, CWinApp::OnHelp )
END_MESSAGE_MAP()




/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

CSilenceClipperApp theApp;




/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

CSilenceClipperApp::CSilenceClipperApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}



/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/


BOOL CSilenceClipperApp::InitInstance()
{
	ASSERT( FALSE );

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Create the list of files

	CStringVector file_list( GetFiles( m_lpCmdLine ) );

	// Create the dialog

	CSilenceClipperDlg dlg;
	m_pMainWnd = &dlg;

	// Start the file processing. The processing will stop when canceled or
	// the object is deleted.

	ClipperThread	clipper_thread( file_list, &dlg ); 
	
	// Display the dialog box until the Cancel button is clicked.

	dlg.DoModal( &clipper_thread );

	// Abort the processing if it is still running

	clipper_thread.Abort();

	// Clean up the file list

	for ( CStringVector::iterator i = file_list.begin(); i != file_list.end(); i++ )
	{
		delete *i;
	}

	return FALSE;
}

CSilenceClipperApp::CStringVector CSilenceClipperApp::GetFiles( CString const & lpCmdLine )
{
	CStringVector files;
	int start = 0;
	int end = 0;

	do
	{
		// Find the end of the string and extract the pathname

		end = lpCmdLine.Find( ' ', start );
		CString short_pathname = ( end != -1 ) ? lpCmdLine.Mid( start, end-start ) : lpCmdLine.Mid( start );

		// Convert it to its long version

		char long_pathname[ _MAX_PATH ];
		if ( GetLongPathName( LPCTSTR( short_pathname ), long_pathname, _MAX_PATH ) == 0 )
		{
			SafeStrcpy( long_pathname, LPCTSTR( short_pathname ), sizeof( long_pathname ) );
			
			CString error_msg;
			error_msg = "An error occurred when trying to find the \"long\" path for\n'";
			error_msg += short_pathname;
			error_msg += "'";
			AfxMessageBox( LPCTSTR( error_msg ), MB_OK|MB_ICONEXCLAMATION );
		}

		// Save the long version

		files.push_back( new CString( long_pathname ) );

		// Bump the start to the next string

		start = end + 1;
	} while ( end != -1 );

	return files;
}

#if !defined( SILENCECLIPPER_H_INCLUDED )
#define SILENCECLIPPER_H_INCLUDED

#pragma once

/****************************************************************************

                               SilenceClipper.h

						Copyright 2000, John J. Bolton
	----------------------------------------------------------------------

	$Header: //depot/SilenceClipper/SilenceClipper.h#2 $

	$NoKeywords: $

****************************************************************************/

#include <vector>

class CSilenceClipperApp : public CWinApp
{
public:
	CSilenceClipperApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL( CSilenceClipperApp )
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG( CSilenceClipperApp )
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	typedef std::vector< CString * > CStringVector;

	CStringVector GetFiles( CString const & lpCmdLine );
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined( SILENCECLIPPER_H_INCLUDED )

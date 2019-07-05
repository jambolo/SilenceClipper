#if !defined( SILENCECLIPPERDLG_H_INCLUDED )
#define SILENCECLIPPERDLG_H_INCLUDED

#pragma once

/****************************************************************************

                              SilenceClipperDlg.h

						Copyright 2000, John J. Bolton
	----------------------------------------------------------------------

	$Header: //depot/SilenceClipper/SilenceClipperDlg.h#2 $

	$NoKeywords: $

****************************************************************************/

#include "resource.h"

class ClipperThread;

/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

class CSilenceClipperDlg : public CDialog
{
public:
	// Custom Windows messages handled by this dialog box

	enum TCustomWindowsMessages
	{
		WM_THREADFINISHED = WM_APP,
		WM_THREADUPDATE
	};

// Construction
	CSilenceClipperDlg( CWnd* pParent = NULL );	// standard constructor

	// Override
	virtual int DoModal( ClipperThread * clipper_thread );

// Dialog Data
	//{{AFX_DATA( CSilenceClipperDlg )
	enum { IDD = IDD_SILENCECLIPPER_DIALOG };
	CProgressCtrl	m_ProgressBar;
	CString	m_FileName;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL( CSilenceClipperDlg )
	protected:
	virtual void DoDataExchange( CDataExchange* pDX );	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG( CSilenceClipperDlg )
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	LONG OnThreadFinished( UINT wParam, LONG lParam );
	LONG OnThreadUpdate(UINT wParam, LONG lParam);

	ClipperThread * m_ClipperThread;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined( SILENCECLIPPERDLG_H_INCLUDED )

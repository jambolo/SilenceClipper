#if !defined( STDAFX_H_INCLUDED )
#define STDAFX_H_INCLUDED

#pragma once

/****************************************************************************

                                   StdAfx.h

						Copyright 2000, John J. Bolton
	----------------------------------------------------------------------

	$Header: //depot/SilenceClipper/StdAfx.h#2 $

	$NoKeywords: $

****************************************************************************/

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxmt.h>
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
//}}AFX_INSERT_LOCATION}}

#include <mmsystem.h>

#endif // !defined( STDAFX_H_INCLUDED )

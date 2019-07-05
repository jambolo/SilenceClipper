#if !defined( CLIPPERTHREAD_H_INCLUDED )
#define CLIPPERTHREAD_H_INCLUDED

#pragma once

/****************************************************************************

                             ClipperThread.h

						Copyright 2000, John J. Bolton
	----------------------------------------------------------------------

	$Header: //depot/SilenceClipper/ClipperThread.h#2 $

	$NoKeywords: $

****************************************************************************/

#include "Thread/Thread.h"

#include <vector>

class CSilenceClipperDlg;

class ClipperThread : public Thread  
{
public:
	ClipperThread( std::vector< CString * > const files, CSilenceClipperDlg * dialog );
	virtual ~ClipperThread();

	// Get the current complete percentage (0-100) and file being processed
	void GetProcessingState( float * ppercent_complete, CString * pcurrent_file );

	// Signal the thread to start processing
	void Start() { m_StartEvent.SetEvent(); }

	// Signal the thread to stop processing
	void Abort() { m_AbortEvent.SetEvent(); }

protected:

	// Thread main

	int Main( int command );

private:

	// Set the current complete percentage (0-100) and file being processed
	void SetProcessingState( float percent_complete, CString const & current_file );

	std::vector< CString * >	m_Files;
	CSilenceClipperDlg *		m_Dialog;
	CString						m_CurrentFile;
	float						m_PercentComplete;

	// Critical section to guard thread state
	CCriticalSection			m_StateCriticalSection;

	// This event signals that the thread should start processing
	CEvent						m_StartEvent;

	// This event signals that the thread should abort
	CEvent						m_AbortEvent;
};


#endif // !defined( CLIPPERTHREAD_H_INCLUDED )

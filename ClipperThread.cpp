/****************************************************************************

                            ClipperThread.cpp

						Copyright 2000, John J. Bolton
	----------------------------------------------------------------------

	$Header: //depot/SilenceClipper/ClipperThread.cpp#2 $

	$NoKeywords: $

****************************************************************************/

#include "stdafx.h"

#include "ClipperThread.h"

#include <sstream>
#include "SilenceClipperDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static void DoClip( CString const & filename );

/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

ClipperThread::ClipperThread( std::vector< CString * > const files, CSilenceClipperDlg * dialog )
	: Thread( 0 ),
	m_Files( files ),
	m_Dialog( dialog )
{
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

ClipperThread::~ClipperThread()
{
	// We have to tell the thread to exit and then wait, otherwise we will
	// delete member variables that the thread might still be using.

	End( true );
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

int ClipperThread::Main( int command )
{
	// Wait for the dialog box to appear

	CSingleLock( &m_StartEvent, TRUE );

	std::vector< CString * >::const_iterator i; 
	for ( i = m_Files.begin(); i != m_Files.end(); i++ )
	{
		// Check if the processing is canceled

		if ( ::WaitForSingleObject( m_AbortEvent.m_hObject, 0 ) == WAIT_OBJECT_0 )
			return CMD_EXIT;

		// Tell the dialog box to update

		SetProcessingState( float( i - m_Files.begin() ) / float( m_Files.size() ) * 100.f, *(*i) );
		::PostMessage( m_Dialog->GetSafeHwnd(), CSilenceClipperDlg::WM_THREADUPDATE, 0, 0 );

		// Process

		DoClip( *(*i) );
	}

	// Tell the dialog box to do a final update

	SetProcessingState( 100.f, CString() );
	::PostMessage( m_Dialog->GetSafeHwnd(), CSilenceClipperDlg::WM_THREADUPDATE, 0, 0 );

	// Tell the dialog box that we are done

	::PostMessage( m_Dialog->GetSafeHwnd(), CSilenceClipperDlg::WM_THREADFINISHED, 0, 0 );

	return CMD_EXIT;
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

void ClipperThread::GetProcessingState( float *ppercent_complete, CString *pcurrent_file )
{
	CSingleLock( &m_StateCriticalSection, TRUE );

	*ppercent_complete = m_PercentComplete;
	*pcurrent_file = m_CurrentFile;
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

void ClipperThread::SetProcessingState( float percent_complete, CString const & current_file )
{
	CSingleLock( &m_StateCriticalSection, TRUE );

	m_PercentComplete = percent_complete;
	m_CurrentFile = current_file;
}


/********************************************************************************************************************/
/*																													*/
/* Find the first non-zero byte.											*/
/*																													*/
/********************************************************************************************************************/

static char * memnz( char const * s, long count )
{
	__int32 const * s_32 = reinterpret_cast< __int32 const * >( s );
	while ( *s_32 == 0 && count >= sizeof( __int32 ) )
	{
		++s_32;
		count -= sizeof( __int32 );
	}

	s = reinterpret_cast< char const * >( s_32 );

	while ( *s == 0 && count > 0 )
	{
		++s;
		--count;
	}

	return const_cast< char * >( s );
}


/********************************************************************************************************************/
/*																													*/
/* Find the last non-zero byte.												*/
/*																													*/
/********************************************************************************************************************/

static char * memrnz( char const * s, long count )
{
	s += count;

	int a = count % sizeof( __int32 );

	--s;
	--a;
	while ( *s == 0 && a >= 0 )
	{
		--s;
		--a;
	}

	++s;

	if ( a >= 0 )
		return const_cast< char * >( s );

	__int32 const * s_32 = reinterpret_cast< __int32 const * >( s );
	count -= count % sizeof( __int32 );

	--s_32;
	count -= sizeof( __int32 );
	while ( *s_32 == 0 && count >= 0 )
	{
		--s_32;
		count -= sizeof( __int32 );
	}

	++s_32;
	count += sizeof( __int32 );

	s = reinterpret_cast< char const * >( s_32 );

	--s;
	--count;
	while ( *s == 0 && count >= 0 )
	{
		--s;
		--count;
	}

	++s;

	return const_cast< char * >( s );
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

static void DoClip( CString const & filename )
{
	HMMIO			hmmioIn = NULL;  // handle to open input WAVE file
	HMMIO			hmmioOut = NULL; // handle to open output WAVE file
	MMCKINFO		ckInRIFF;   // chunk info. for input RIFF chunk
	MMCKINFO		ckOutRIFF;  // chunk info. for output RIFF chunk
	MMCKINFO		ckIn;	// info. for a chunk in input file
	MMCKINFO		ckOut;	   // info. for a chunk in output file
	WAVEFORMATEX	wave_format; // contents of 'fmt' chunks
	MMIOINFO		mmioinfo;	// for errors
	char *			input_buffer = 0;
	long			output_size;
	char *			start;
	char *			end;
	MMRESULT		mmio_error;

	/********************************************************************************************************************/

	// Open the input file for reading
	memset( &mmioinfo, 0, sizeof mmioinfo );
	hmmioIn = mmioOpen( const_cast< char * >( LPCTSTR( filename ) ), &mmioinfo, MMIO_READ );
	if ( hmmioIn == NULL )
	{
		mmio_error = mmioinfo.wErrorRet;
		goto DISPLAY_ERROR_MSGBOX;
	}

	/* Descend the input file into the 'RIFF' chunk. */
	if ( ( mmio_error = mmioDescend( hmmioIn, &ckInRIFF, NULL, 0 ) ) != 0 )
		goto DISPLAY_ERROR_MSGBOX;

	/* Make sure the input file is a WAVE file. */
	if ( ( ckInRIFF.ckid != FOURCC_RIFF ) ||
		( ckInRIFF.fccType != mmioFOURCC( 'W', 'A', 'V', 'E' ) ) )
	{
		mmio_error = MMIOERR_INVALIDFILE;
		goto DISPLAY_ERROR_MSGBOX;
	}

	/* Search the input file for for the 'fmt ' chunk. */
	ckIn.ckid = mmioFOURCC( 'f', 'm', 't', ' ' );
	if ( ( mmio_error = mmioDescend( hmmioIn, &ckIn, &ckInRIFF, MMIO_FINDCHUNK ) ) != 0 )
		goto DISPLAY_ERROR_MSGBOX;

	// Expect the 'fmt' chunk to be at least as large as < PCMWAVEFORMAT >
	if ( ckIn.cksize == ( long ) sizeof( PCMWAVEFORMAT ) ||
		 ckIn.cksize == ( long ) sizeof( WAVEFORMATEX ) )
	{
		/* Read the 'fmt ' chunk into < wave_format >. */
		if ( mmioRead( hmmioIn, ( HPSTR ) &wave_format, ( long )sizeof( PCMWAVEFORMAT ) ) !=
			( long ) sizeof( PCMWAVEFORMAT ) )
		{
			{
				mmio_error = MMIOERR_CANNOTREAD;
				goto DISPLAY_ERROR_MSGBOX;
			}
		}

		wave_format.cbSize = 0;	// Extend it to a standard WAVEFORMATEX
	}
	else
	{
		mmio_error = MMIOERR_INVALIDFILE;
		goto DISPLAY_ERROR_MSGBOX;
	}

	/* Ascend the input file out of the 'fmt ' chunk. */
	if ( ( mmio_error = mmioAscend( hmmioIn, &ckIn, 0 ) ) != 0 )
		goto DISPLAY_ERROR_MSGBOX;

	/* Make sure the input file is PCM WAVE file. */
	if ( wave_format.wFormatTag != WAVE_FORMAT_PCM )
	{
		mmio_error = MMIOERR_INVALIDFILE;
		goto DISPLAY_ERROR_MSGBOX;
	}

	/* Search the input file for for the 'data' chunk. */
	ckIn.ckid = mmioFOURCC( 'd', 'a', 't', 'a' );
	if ( ( mmio_error = mmioDescend( hmmioIn, &ckIn, &ckInRIFF, MMIO_FINDCHUNK ) ) != 0 )
		goto DISPLAY_ERROR_MSGBOX;

	input_buffer = new char[ ckIn.cksize ];
	if ( mmioRead( hmmioIn, input_buffer, ckIn.cksize ) != long( ckIn.cksize ) )
	{
		mmio_error = MMIOERR_CANNOTREAD;
		goto DISPLAY_ERROR_MSGBOX;
	}

	mmioClose( hmmioIn, 0 );
	hmmioIn = NULL;

	/********************************************************************************************************************/

	output_size = ckIn.cksize - ckIn.cksize % wave_format.nBlockAlign;

	start = memnz( input_buffer, output_size );
	start -= ( start - input_buffer ) % wave_format.nBlockAlign;
	output_size -= start - input_buffer;

	end = memrnz( start, output_size );
	output_size = end - start + wave_format.nBlockAlign - 1;
	output_size -= output_size % wave_format.nBlockAlign;

	/********************************************************************************************************************/

	/* Open the output file for writing  */
	memset( &mmioinfo, 0, sizeof mmioinfo );
	hmmioOut = mmioOpen( const_cast< char * >( LPCTSTR( filename ) ), &mmioinfo, MMIO_READWRITE | MMIO_CREATE );
	if ( hmmioOut == NULL )
	{
		mmio_error = mmioinfo.wErrorRet;
		goto DISPLAY_ERROR_MSGBOX;
	}

	/* Create the output file RIFF chunk of form type 'WAVE'. */
	memset( &ckOutRIFF, 0, sizeof ckOutRIFF );
	ckOutRIFF.fccType = mmioFOURCC( 'W', 'A', 'V', 'E' );
	if ( ( mmio_error = mmioCreateChunk( hmmioOut, &ckOutRIFF, MMIO_CREATERIFF ) ) != 0 )
		goto DISPLAY_ERROR_MSGBOX; // cannot write file, probably

	/* We are now descended into the 'RIFF' chunk we just created.
	* Now create the 'fmt ' chunk. Since we know the size of this chunk,
	* specify it in the MMCKINFO structure so MMIO doesn't have to seek
	* back and set the chunk size after ascending from the chunk. */
	memset( &ckOut, 0, sizeof ckOut );
	ckOut.ckid = mmioFOURCC( 'f', 'm', 't', ' ' );
	ckOut.cksize = sizeof( wave_format );
	if ( ( mmio_error = mmioCreateChunk( hmmioOut, &ckOut, 0 ) ) != 0 )
		goto DISPLAY_ERROR_MSGBOX; // cannot write file, probably

	/* Write the PCMWAVEFORMAT structure to the 'fmt ' chunk. */
	if ( mmioWrite( hmmioOut, ( HPSTR ) &wave_format, sizeof( wave_format ) )
		!= sizeof( wave_format ) )
	{
		mmio_error = MMIOERR_CANNOTWRITE;
		goto DISPLAY_ERROR_MSGBOX;
	}

	/* Ascend out of the 'fmt ' chunk, back into the 'RIFF' chunk. */
	if ( ( mmio_error = mmioAscend( hmmioOut, &ckOut, 0 ) ) != 0 )
		goto DISPLAY_ERROR_MSGBOX; // cannot write file, probably

	/* Create the 'data' chunk that holds the waveform samples. */
	memset( &ckOut, 0, sizeof ckOut );
	ckOut.ckid = mmioFOURCC( 'd', 'a', 't', 'a' );
	ckOut.cksize = output_size;
	if ( ( mmio_error = mmioCreateChunk( hmmioOut, &ckOut, 0 ) ) != 0 )
		goto DISPLAY_ERROR_MSGBOX; // cannot write file, probably

	if ( mmioWrite( hmmioOut, start, output_size ) != output_size )
	{
		mmio_error = MMIOERR_CANNOTWRITE;
		goto DISPLAY_ERROR_MSGBOX;
	}

	delete input_buffer;
	input_buffer = 0;

	/* Ascend the output file out of the 'data' chunk -- this will cause * the chunk size of the 'data' chunk to be written. */
	if ( ( mmio_error = mmioAscend( hmmioOut, &ckOut, 0 ) ) != 0 )
		goto DISPLAY_ERROR_MSGBOX; // cannot write file, probably

	/* Ascend the output file out of the 'RIFF' chunk -- this will cause * the chunk size of the 'RIFF' chunk to be written. */
	if ( ( mmio_error = mmioAscend( hmmioOut, &ckOutRIFF, 0 ) ) != 0 )
		goto DISPLAY_ERROR_MSGBOX; // cannot write file, probably

	mmioClose( hmmioOut, 0 );
	hmmioOut = NULL;

	return;

	/********************************************************************************************************************/

DISPLAY_ERROR_MSGBOX:
	{
		std::ostringstream message;
		char * error_text;

		switch ( mmio_error )
		{
		case MMIOERR_FILENOTFOUND:
			error_text = "file not found";
			break;

		case MMIOERR_OUTOFMEMORY:
			error_text = "out of memory";
			break;

		case MMIOERR_CANNOTOPEN:
			error_text = "cannot open";
			break;

		case MMIOERR_CANNOTCLOSE:
			error_text = "cannot close";
			break;

		case MMIOERR_CANNOTREAD:
			error_text = "cannot read";
			break;

		case MMIOERR_CANNOTWRITE:
			error_text = "cannot write";
			break;

		case MMIOERR_CANNOTSEEK:
			error_text = "cannot seek";
			break;

		case MMIOERR_CANNOTEXPAND:
			error_text = "cannot expand file";
			break;

		case MMIOERR_CHUNKNOTFOUND:
			error_text = "chunk not found";
			break;

		case MMIOERR_UNBUFFERED:
			error_text = "unbuffered";
			break;

		case MMIOERR_PATHNOTFOUND:
			error_text = "path incorrect";
			break;

		case MMIOERR_ACCESSDENIED:
			error_text = "file was protected";
			break;

		case MMIOERR_SHARINGVIOLATION:
			error_text = "file in use";
			break;

		case MMIOERR_NETWORKERROR:
			error_text = "network not responding";
			break;

		case MMIOERR_TOOMANYOPENFILES:
			error_text = "no more file handles";
			break;

		case MMIOERR_INVALIDFILE:
			error_text = "default error file error";
			break;

		case MMSYSERR_NOERROR:
			error_text = "no error";
			break;

		case MMSYSERR_ERROR:
			error_text = "unspecified error";
			break;

		case MMSYSERR_BADDEVICEID:
			error_text = "device ID out of range";
			break;

		case MMSYSERR_NOTENABLED:
			error_text = "driver failed enable";
			break;

		case MMSYSERR_ALLOCATED:
			error_text = "device already allocated";
			break;

		case MMSYSERR_INVALHANDLE:
			error_text = "device handle is invalid";
			break;

		case MMSYSERR_NODRIVER:
			error_text = "no device driver present";
			break;

		case MMSYSERR_NOMEM:
			error_text = "memory allocation error";
			break;

		case MMSYSERR_NOTSUPPORTED:
			error_text = "function isn't supported";
			break;

		case MMSYSERR_BADERRNUM:
			error_text = "error value out of range";
			break;

		case MMSYSERR_INVALFLAG:
			error_text = "invalid flag passed";
			break;

		case MMSYSERR_INVALPARAM:
			error_text = "invalid parameter passed";
			break;

		case MMSYSERR_HANDLEBUSY:
			error_text = "handle being used simultaneously on another thread (eg callback)";
			break;

		case MMSYSERR_INVALIDALIAS:
			error_text = "specified alias not found";
			break;

		case MMSYSERR_BADDB:
			error_text = "bad registry database";
			break;

		case MMSYSERR_KEYNOTFOUND:
			error_text = "registry key not found";
			break;

		case MMSYSERR_READERROR:
			error_text = "registry read error";
			break;

		case MMSYSERR_WRITEERROR:
			error_text = "registry write error";
			break;

		case MMSYSERR_DELETEERROR:
			error_text = "registry delete error";
			break;

		case MMSYSERR_VALNOTFOUND:
			error_text = "registry value not found";
			break;

		case MMSYSERR_NODRIVERCB:
			error_text = "driver does not call DriverCallback";
			break;

		default:
			error_text = "unspecified error";
			break;
		}

		message << "File I/O Error: " << error_text;
		MessageBox( NULL, message.str().c_str(), "Silence Clipper", MB_OK | MB_ICONEXCLAMATION );
	}

	delete input_buffer;

	/* Close the files ( unless they weren't opened successfully ).*/
	if ( hmmioIn != NULL )
		mmioClose( hmmioIn, 0 );
	if ( hmmioOut != NULL )
		mmioClose( hmmioOut, 0 );
}

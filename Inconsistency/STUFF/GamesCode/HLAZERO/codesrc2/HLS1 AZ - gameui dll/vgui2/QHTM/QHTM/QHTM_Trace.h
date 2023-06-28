/*----------------------------------------------------------------------
Copyright (c) 1998,1999 Gipsysoft. All Rights Reserved.
Please see the file "licence.txt" for licencing details.
File:	QHTM_Trace.h
Owner:	russf@gipsysoft.com
Purpose:	<Description of module>.
----------------------------------------------------------------------*/
#ifndef QHTM_TRACE_H
#define QHTM_TRACE_H

#ifdef QHTM_TRACE_ENABLED	
	#undef TRACE
	
	void _cdecl QHTMTrace( LPCSTR pcszFormat, ... );
	#define TRACE ::QHTMTrace

#else		//	QHTM_TRACE_ENABLED
	//
	//	Do nothing!
#endif	//	QHTM_TRACE_ENABLED

#endif //QHTM_TRACE_H
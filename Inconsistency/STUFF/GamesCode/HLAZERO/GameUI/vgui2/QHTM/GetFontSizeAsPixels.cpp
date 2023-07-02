/*----------------------------------------------------------------------
Copyright (c) 1998 Gipsysoft. All Rights Reserved.
Please see the file "licence.txt" for licencing details.

File:	GetFontSizeAsPixels.cpp
Owner:	russf@gipsysoft.com
Purpose:	Given a HTML font size it returns a size in pixels for the
					device passed.
					REVIEW - russf - OS dependent code (takes a HDC)
----------------------------------------------------------------------*/
#include "stdafx.h"
#include <math.h>
#include <DebugHlp.h>

static const int arrPixels[ 7 ][5] = 
		{//	0		1		2		3		4		
				 6  ,7	,7	,9	,12	//	1 
				,8	,9	,9	,12	,14	//	2 
				,9	,9 	,12	,14	,16	//	3 
				,9	,12	,14	,16	,18	//	4 
				,12 ,16	,18	,20	,24	//	5 
				,16	,20	,24	,27	,31	//	6 
				,24	,29	,36	,41	,47	//	7 
		};

int GetFontSizeAsPixels( HDC hdc, int nSize, UINT nZoomLevel )
//
//	Return the font size as pixels. Basically converts the HTML logical font sizes to pixels.
//	If the nSize passed is negative then it will assume the number is a point size request
{
	ASSERT( nZoomLevel < 5 );
	
	//
	//	If the size requested is smaller than zero then it's a point size.
	if( nSize < 0 )
	{
		nSize = abs( nSize );
	}
	else
	{
		//
		//	Zero based array but not zero based font sizes
		nSize--;
		//
		//	Otherwise it's a HTML psuedo-size which we lookup
		nSize = arrPixels[ max( 0, min( nSize, sizeof( arrPixels ) / sizeof( arrPixels[ 0 ] ) ) ) ][nZoomLevel];
	}

	int	nLogPixelsY = 0;
	if (::GetObjectType(hdc)==OBJ_METADC)
	{
		HDC		hScreenDC = GetDC(NULL);
		nLogPixelsY = GetDeviceCaps( hScreenDC, LOGPIXELSY);
		ReleaseDC(NULL, hScreenDC);
	}
	else
		nLogPixelsY = GetDeviceCaps( hdc, LOGPIXELSY);

	//
	//	Convert the point size to pixel size for the destination device and we are away!
	const int nFontSize = -MulDiv( nSize, nLogPixelsY, 72 );
	return nFontSize;
}


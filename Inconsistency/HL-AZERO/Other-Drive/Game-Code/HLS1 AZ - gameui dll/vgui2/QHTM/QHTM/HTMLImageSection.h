/*----------------------------------------------------------------------
Copyright (c) 1998 Gipsysoft. All Rights Reserved.
Please see the file "licence.txt" for licencing details.

File:	HTMLImageSection.h
Owner:	russf@gipsysoft.com
Purpose:	HTML Image section.
----------------------------------------------------------------------*/
#ifndef HTMLIMAGESECTION_H
#define HTMLIMAGESECTION_H

#ifndef HTMLSECTIONABC_H
	#include "HTMLSectionABC.h"
#endif	//	HTMLSECTIONABC_H

class CImage;

class CHTMLImageSection : public CHTMLSectionABC
{
public:
	CHTMLImageSection( CParentSection *psect, CImage *pImage, int nBorder );
	virtual ~CHTMLImageSection();

	virtual void OnDraw( CDrawContext &dc );
	virtual void OnTimer( int nTimerID );

private:
	CImage *m_pImage;
	int			m_nBorder;
	int m_nTimerID;
	UINT  m_nFrame;

private:
	CHTMLImageSection();
	CHTMLImageSection( const CHTMLImageSection & );
	CHTMLImageSection& operator =( const CHTMLImageSection & );
};


#endif //HTMLIMAGESECTION_H
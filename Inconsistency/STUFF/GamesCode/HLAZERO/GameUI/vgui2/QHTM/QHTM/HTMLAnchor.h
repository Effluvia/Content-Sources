/*----------------------------------------------------------------------
Copyright (c) 1998,1999 Gipsysoft. All Rights Reserved.
Please see the file "licence.txt" for licencing details.
File:	HTMLAnchor.h
Owner:	russf@gipsysoft.com
Purpose:	<Description of module>.
----------------------------------------------------------------------*/
#ifndef HTMLANCHOR_H
#define HTMLANCHOR_H

#ifndef QHTM_TYPES_H
	#include "QHTM_Types.h"
#endif	//	QHTM_TYPES_H

class CHTMLAnchor : public CHTMLParagraphObject
{
public:
	CHTMLAnchor( LPCTSTR  pcszLinkName, LPCTSTR  pcszLinkTarget, LPCTSTR  pcszLinkTitle, COLORREF crLink, COLORREF crHover);
	CHTMLAnchor();

	StringClass m_strLinkName;
	StringClass m_strLinkTarget;
	StringClass m_strLinkTitle;
	COLORREF	m_crLink;
	COLORREF	m_crHover;

private:
#ifdef _DEBUG
	virtual void Dump() const;
#endif	//	_DEBUG
private:
	CHTMLAnchor( const CHTMLAnchor &);
	CHTMLAnchor& operator =( const CHTMLAnchor &);
};

#endif //HTMLANCHOR_H
/*----------------------------------------------------------------------
Copyright (c) 1998 Gipsysoft. All Rights Reserved.
Please see the file "licence.txt" for licencing details.
File:	htmlparsebase.cpp
Owner:	russf@gipsysoft.com
Purpose:	Base class used for parsing HTML
----------------------------------------------------------------------*/
#include "stdafx.h"
#include <DebugHlp.h>
#include <WinHelper.h>
#include <stdlib.h>
#include <debughlp.h>
#include <TextABC.h>
#include "HTMLParseBase.h"
#include "StaticString.h"
#include "SimpleString.h"

#ifndef MapClass
	#include <map.h>
	#define MapClass				Container::CMap
#endif	//	MapClass

#ifdef _DEBUG
	#define String( s ) (LPCTSTR)CSimpleString( s.GetData(), s.GetLength() )
	//#define PTRACE TRACE
	#define PTRACE
#else		//	_DEBUG
		#define PTRACE
		#define String
#endif	//	_DEBUG

struct
{
	LPCTSTR pcszName;
	TCHAR ch;
}
g_arrCharacters[] =
{
	_T("nbsp"),		_T('�'),	//CDATA "&#160;" -- no-break space -->
	_T("quot"), _T('\"'),
	_T("amp"),	_T('&'),
	_T("lt"),		_T('<'),
	_T("gt"),		_T('>'),
	_T("iexcl"),	_T('�'),	//CDATA "&#161;" -- inverted exclamation mark -->
	_T("cent"),		_T('�'),	//CDATA "&#162;" -- cent sign -->
	_T("pound"),	_T('�'),	//CDATA "&#163;" -- pound sterling sign -->
	_T("curren"),	_T('�'),	//CDATA "&#164;" -- general currency sign -->
	_T("yen"),		_T('�'),	//CDATA "&#165;" -- yen sign -->
	_T("brvbar"),	_T('�'),	//CDATA "&#166;" -- broken (vertical) bar -->
	_T("sect"),		_T('�'),	//CDATA "&#167;" -- section sign -->
	_T("uml"),		_T('�'),	//CDATA "&#168;" -- umlaut (dieresis) -->
	_T("copy"),		_T('�'),	//CDATA "&#169;" -- copyright sign -->
	_T("ordf"),		_T('�'),	//CDATA "&#170;" -- ordinal indicator, feminine -->
	_T("laquo"),	_T('�'),	//CDATA "&#171;" -- angle quotation mark, left -->
	_T("not"),		_T('�'),	//CDATA "&#172;" -- not sign -->
	_T("shy"),		_T('�'),	//CDATA "&#173;" -- soft hyphen -->
	_T("reg"),		_T('�'),	//CDATA "&#174;" -- registered sign -->
	_T("macr"),		_T('�'),	//CDATA "&#175;" -- macron -->
	_T("deg"),		_T('�'),	//CDATA "&#176;" -- degree sign -->
	_T("plusmn"), _T('�'),	//CDATA "&#177;" -- plus-or-minus sign -->
	_T("sup2"),		_T('�'),	//CDATA "&#178;" -- superscript two -->
	_T("sup3"),		_T('�'),	//CDATA "&#179;" -- superscript three -->
	_T("acute"),	_T('�'),	//CDATA "&#180;" -- acute accent -->
	_T("micro"),	_T('�'),	//CDATA "&#181;" -- micro sign -->
	_T("para"),		_T('�'),	//CDATA "&#182;" -- pilcrow (paragraph sign) -->
	_T("middot"),	_T('�'),	//CDATA "&#183;" -- middle dot -->
	_T("cedil"),	_T('�'),	//CDATA "&#184;" -- cedilla -->
	_T("sup1"),		_T('�'),	//CDATA "&#185;" -- superscript one -->
	_T("ordm"),		_T('�'),	//CDATA "&#186;" -- ordinal indicator, masculine -->
	_T("raquo"),	_T('�'),	//CDATA "&#187;" -- angle quotation mark, right -->
	_T("frac14"), _T('�'),//CDATA "&#188;" -- fraction one-quarter -->
	_T("frac12"), _T('�'),//CDATA "&#189;" -- fraction one-half -->
	_T("frac34"), _T('�'),//CDATA "&#190;" -- fraction three-quarters -->
	_T("iquest"), _T('�'),//CDATA "&#191;" -- inverted question mark -->
	_T("Agrave"), _T('�'),//CDATA "&#192;" -- capital A, grave accent -->
	_T("Aacute"), _T('�'),//CDATA "&#193;" -- capital A, acute accent -->
	_T("Acirc"),	_T('�'),	//CDATA "&#194;" -- capital A, circumflex accent -->
	_T("Atilde"), _T('�'),//CDATA "&#195;" -- capital A, tilde -->
	_T("Auml"),		_T('�'),	//CDATA "&#196;" -- capital A, dieresis or umlaut mark -->
	_T("Aring"),	_T('�'),	//CDATA "&#197;" -- capital A, ring -->
	_T("AElig"),	_T('�'),	//CDATA "&#198;" -- capital AE diphthong (ligature) -->
	_T("Ccedil"),	_T('c'),//CDATA "&#199;" -- capital C, cedilla -->
	_T("Egrave"),	_T('�'),//CDATA "&#200;" -- capital E, grave accent -->
	_T("Eacute"),	_T('�'),//CDATA "&#201;" -- capital E, acute accent -->
	_T("Ecirc"),	_T('�'),	//CDATA "&#202;" -- capital E, circumflex accent -->
	_T("Euml"),		_T('�'),	//CDATA "&#203;" -- capital E, dieresis or umlaut mark -->
	_T("Igrave"),	_T('�'),//CDATA "&#204;" -- capital I, grave accent -->
	_T("Iacute"),	_T('�'),//CDATA "&#205;" -- capital I, acute accent -->
	_T("Icirc"),	_T('�'),	//CDATA "&#206;" -- capital I, circumflex accent -->
	_T("Iuml"),		_T('�'),	//CDATA "&#207;" -- capital I, dieresis or umlaut mark -->
	_T("ETH"),		_T('�'),		//CDATA "&#208;" -- capital Eth, Icelandic -->
	_T("Ntilde"), _T('�'),//CDATA "&#209;" -- capital N, tilde -->
	_T("Ograve"), _T('�'),//CDATA "&#210;" -- capital O, grave accent -->
	_T("Oacute"), _T('�'),//CDATA "&#211;" -- capital O, acute accent -->
	_T("Ocirc"),	_T('�'),	//CDATA "&#212;" -- capital O, circumflex accent -->
	_T("Otilde"),	_T('�'),//CDATA "&#213;" -- capital O, tilde -->
	_T("Ouml"),		_T('�'),	//CDATA "&#214;" -- capital O, dieresis or umlaut mark -->
	_T("times"),	_T('�'),	//CDATA "&#215;" -- multiply sign -->
	_T("Oslash"),	_T('�'),//CDATA "&#216;" -- capital O, slash -->
	_T("Ugrave"),	_T('�'),//CDATA "&#217;" -- capital U, grave accent -->
	_T("Uacute"),	_T('�'),//CDATA "&#218;" -- capital U, acute accent -->
	_T("Ucirc"),	_T('�'),	//CDATA "&#219;" -- capital U, circumflex accent -->
	_T("Uuml"),		_T('�'),	//CDATA "&#220;" -- capital U, dieresis or umlaut mark -->
	_T("Yacute"),	_T('�'),//CDATA "&#221;" -- capital Y, acute accent -->
	_T("THORN"), _T('�'),	//CDATA "&#222;" -- capital THORN, Icelandic -->
	_T("szlig"), _T('�'),	//CDATA "&#223;" -- small sharp s, German (sz ligature) -->
	_T("agrave"), _T('�'),//CDATA "&#224;" -- small a, grave accent -->
	_T("aacute"), _T('�'),//CDATA "&#225;" -- small a, acute accent -->
	_T("acirc"), _T('�'),	//CDATA "&#226;" -- small a, circumflex accent -->
	_T("atilde"), _T('�'),//CDATA "&#227;" -- small a, tilde -->
	_T("auml"), _T('�'),	//CDATA "&#228;" -- small a, dieresis or umlaut mark -->
	_T("aring"), _T('�'),	//CDATA "&#229;" -- small a, ring -->
	_T("aelig"), _T('�'),	//CDATA "&#230;" -- small ae diphthong (ligature) -->
	_T("ccedil"), _T('�'),//CDATA "&#231;" -- small c, cedilla -->
	_T("egrave"), _T('�'),//CDATA "&#232;" -- small e, grave accent -->
	_T("eacute"), _T('�'),//CDATA "&#233;" -- small e, acute accent -->
	_T("ecirc"), _T('�'),	//CDATA "&#234;" -- small e, circumflex accent -->
	_T("euml"), _T('�'),	//CDATA "&#235;" -- small e, dieresis or umlaut mark -->
	_T("igrave"), _T('�'),//CDATA "&#236;" -- small i, grave accent -->
	_T("iacute"), _T('�'),//CDATA "&#237;" -- small i, acute accent -->
	_T("icirc"), _T('�'),	//CDATA "&#238;" -- small i, circumflex accent -->
	_T("iuml"), _T('�'),	//CDATA "&#239;" -- small i, dieresis or umlaut mark -->
	_T("eth"), _T('�'),		//CDATA "&#240;" -- small eth, Icelandic -->
	_T("ntilde"), _T('�'),//CDATA "&#241;" -- small n, tilde -->
	_T("ograve"), _T('�'),//CDATA "&#242;" -- small o, grave accent -->
	_T("oacute"), _T('�'),//CDATA "&#243;" -- small o, acute accent -->
	_T("ocirc"), _T('�'),	//CDATA "&#244;" -- small o, circumflex accent -->
	_T("otilde"), _T('�'),//CDATA "&#245;" -- small o, tilde -->
	_T("ouml"), _T('�'),	//CDATA "&#246;" -- small o, dieresis or umlaut mark -->
	_T("divide"), _T('�'),//CDATA "&#247;" -- divide sign -->
	_T("oslash"), _T('�'),//CDATA "&#248;" -- small o, slash -->
	_T("ugrave"), _T('�'),//CDATA "&#249;" -- small u, grave accent -->
	_T("uacute"), _T('�'),//CDATA "&#250;" -- small u, acute accent -->
	_T("ucirc"), _T('�'),	//CDATA "&#251;" -- small u, circumflex accent -->
	_T("uuml"), _T('�'),	//CDATA "&#252;" -- small u, dieresis or umlaut mark -->
	_T("yacute"), _T('�'),//CDATA "&#253;" -- small y, acute accent -->
	_T("thorn"), _T('�'),	//CDATA "&#254;" -- small thorn, Icelandic -->
	_T("yuml"), _T('�'),	//CDATA "&#255;" -- small y, dieresis or umlaut mark -->
};

MapClass<LPCTSTR , TCHAR> g_mapCharacters;


struct StructToken
{
	StructToken( LPCTSTR pcszName, CHTMLParseBase::Token	tok )
		: m_strName( pcszName ), m_tok( tok ) {}
	CStaticString m_strName;
	CHTMLParseBase::Token	m_tok;
};

const StructToken g_tokens[] =
{
	 StructToken( _T("body"),		CHTMLParseBase::tokBody )
	,StructToken(  _T("font"),		CHTMLParseBase::tokFont )
	,StructToken( _T("b"),		CHTMLParseBase::tokBold )
	,StructToken( _T("strong"),		CHTMLParseBase::tokBold )
	,StructToken( _T("u"),		CHTMLParseBase::tokUnderline )
	,StructToken( _T("cite"),		CHTMLParseBase::tokItalic )
	,StructToken( _T("i"),		CHTMLParseBase::tokItalic )
	,StructToken( _T("strike"),		CHTMLParseBase::tokStrikeout )
	,StructToken( _T("del"),		CHTMLParseBase::tokStrikeout )
	,StructToken( _T("em"),		CHTMLParseBase::tokItalic )
	,StructToken( _T("img"),		CHTMLParseBase::tokImage )
	,StructToken( _T("td"),		CHTMLParseBase::tokTableDef )
	,StructToken( _T("tr"),		CHTMLParseBase::tokTableRow )
	,StructToken( _T("th"),		CHTMLParseBase::tokTableHeading )
	,StructToken( _T("table"),		CHTMLParseBase::tokTable )
	,StructToken( _T("hr"),		CHTMLParseBase::tokHorizontalRule )
	,StructToken( _T("p"),		CHTMLParseBase::tokParagraph )
	,StructToken( _T("br"),		CHTMLParseBase::tokBreak )
	,StructToken( _T("a"),		CHTMLParseBase::tokAnchor )
	,StructToken( _T("H1"),		CHTMLParseBase::tokH1 )
	,StructToken( _T("H2"),		CHTMLParseBase::tokH2 )
	,StructToken( _T("H3"),		CHTMLParseBase::tokH3 )
	,StructToken( _T("H4"),		CHTMLParseBase::tokH4 )
	,StructToken( _T("H5"),		CHTMLParseBase::tokH5 )
	,StructToken( _T("H6"),		CHTMLParseBase::tokH6 )
	,StructToken( _T("li"),		CHTMLParseBase::tokListItem )
	,StructToken( _T("ol"),		CHTMLParseBase::tokOrderedList )
	,StructToken( _T("ul"),		CHTMLParseBase::tokUnorderedList )
	,StructToken( _T("dir"),		CHTMLParseBase::tokUnorderedList )
	,StructToken( _T("menu"),		CHTMLParseBase::tokUnorderedList )
	,StructToken( _T("blockquote"),	CHTMLParseBase::tokBlockQuote )
	,StructToken( _T("address"),	CHTMLParseBase::tokAddress )
	,StructToken( _T("title"),		CHTMLParseBase::tokDocumentTitle )
	,StructToken( _T("center"),		CHTMLParseBase::tokCenter )
	,StructToken( _T("div"),		CHTMLParseBase::tokDiv )
	,StructToken( _T("pre"),		CHTMLParseBase::tokPre )
	,StructToken( _T("xmp"),		CHTMLParseBase::tokPre )
	,StructToken( _T("code"),		CHTMLParseBase::tokCode )
	,StructToken( _T("meta"),		CHTMLParseBase::tokMeta )	
	,StructToken( _T("head"),		CHTMLParseBase::tokHead )
	,StructToken( _T("html"),		CHTMLParseBase::tokHTML )
	,StructToken( _T("sub"),		CHTMLParseBase::tokSub )
	,StructToken( _T("sup"),		CHTMLParseBase::tokSup )
};

MapClass< CStaticString, CHTMLParseBase::Token > g_mapToken;

struct StructParameter
{
	StructParameter( LPCTSTR pcszName, CHTMLParseBase::Param	param )
		: m_strName( pcszName ), param( param ) {}
	CStaticString m_strName;
	CHTMLParseBase::Param	param;
};

const StructParameter g_params[] =
{
	StructParameter( _T("align"),			CHTMLParseBase::pAlign )
	,StructParameter( _T("valign"),			CHTMLParseBase::pVAlign )
	,StructParameter( _T("noshade"),			CHTMLParseBase::pNoShade )
	,StructParameter( _T("size"),			CHTMLParseBase::pSize )
	,StructParameter( _T("width"),			CHTMLParseBase::pWidth )
	,StructParameter( _T("height"),			CHTMLParseBase::pHeight )
	,StructParameter( _T("name"),			CHTMLParseBase::pName )
	,StructParameter( _T("href"),			CHTMLParseBase::pHref )
	,StructParameter( _T("title"),			CHTMLParseBase::pTitle )
	,StructParameter( _T("src"),			CHTMLParseBase::pSrc )
	,StructParameter( _T("color"),			CHTMLParseBase::pColor )
	,StructParameter( _T("bgcolor"),			CHTMLParseBase::pBColor )
	,StructParameter( _T("face"),			CHTMLParseBase::pFace )
	,StructParameter( _T("alt"),			CHTMLParseBase::pAlt )
	,StructParameter( _T("border"),			CHTMLParseBase::pBorder )
	,StructParameter( _T("nowrap"),			CHTMLParseBase::pNoWrap )
	,StructParameter( _T("cellspacing"),		CHTMLParseBase::pCellSpacing )
	,StructParameter( _T("cellpadding"),		CHTMLParseBase::pCellPadding )
	,StructParameter( _T("bordercolor"),		CHTMLParseBase::pBorderColor )
	,StructParameter( _T("bordercolorlight"),	CHTMLParseBase::pBorderColorLight )
	,StructParameter( _T("bordercolordark"),		CHTMLParseBase::pBorderColorDark )
	,StructParameter( _T("link"),			CHTMLParseBase::pLink )
	,StructParameter( _T("alink"),			CHTMLParseBase::pALink )
	,StructParameter( _T("value"),			CHTMLParseBase::pValue )
	,StructParameter( _T("type"),			CHTMLParseBase::pType )
	,StructParameter( _T("margintop"),		CHTMLParseBase::pMarginTop )
	,StructParameter( _T("marginbottom"),		CHTMLParseBase::pMarginBottom )
	,StructParameter( _T("marginleft"),		CHTMLParseBase::pMarginLeft )
	,StructParameter( _T("marginright"),		CHTMLParseBase::pMarginRight )
	,StructParameter( _T("content"),		CHTMLParseBase::pContent )
	,StructParameter( _T("http-equiv"),		CHTMLParseBase::pHTTPEquiv )
};
MapClass< CStaticString, CHTMLParseBase::Param> g_mapParam;

static inline LPCTSTR SkipWhitespace( LPCTSTR pcsz )
//
//	Skip past any whitespace
{
#ifdef UNICODE
	while( *pcsz && _istspace( *pcsz ) ) pcsz++;
#else	//	UNICODE
	while( *pcsz && isspace( *pcsz ) ) pcsz++;
#endif	//	UNICODE
	return pcsz;
}


static LPCTSTR FASTCALL GetTagToken( const CStaticString &strTag, CHTMLParseBase::Token &tok, bool &bIsEnd )
//
//	set the tok value to be a token
//	Set the bIsEnd value if the token passed is an end token
//
//	return the start of the first parameter or NULL if there is no parameters
{
	LPCTSTR pcszTagText = strTag.GetData();
	LPCTSTR pRetVal = NULL;
	tok = CHTMLParseBase::tokIgnore;
	//
	//	Skip past the whitespace at the begining and check if the tag passed
	//	is an end tag.
	pcszTagText = SkipWhitespace( pcszTagText );
	if( *pcszTagText == '/' )
	{
		bIsEnd = true;
		pcszTagText++;
	}
	else
	{
		bIsEnd = false;
	}

	LPCTSTR pcszTagStart = pcszTagText;
	if( *pcszTagStart )
	{
		//
		//	Skip past the whitespace and copy the tag name into a local buffer
		//	then lookup into the map of tokens to see if teh tag we have is one of our
		//	supported tags, if not then we just ignore it.
		pcszTagText = SkipWhitespace( pcszTagText );
		LPCTSTR pcszTagTextStart = pcszTagText;
		LPCTSTR pcszTagEnd = strTag.GetEndPointer();

#ifdef UNICODE
		while( pcszTagText < pcszTagEnd && !_istspace( *pcszTagText ) )
#else	//	UNICODE
		while( pcszTagText < pcszTagEnd && !isspace( *pcszTagText ) )
#endif	//	UNICODE
		{
			pcszTagText++;
		}

		const CStaticString strTagToken( pcszTagTextStart, pcszTagText - pcszTagTextStart );

		CHTMLParseBase::Token *ptok = g_mapToken.Lookup( strTagToken );
		if( ptok )
		{
			tok = *ptok;
		}

		//	Skip any whitespace until we get to the first parameter and
		//	if the string does not point to a null then we will return it.
		pcszTagText = SkipWhitespace( pcszTagText );
		if( *pcszTagText )
		{
			pRetVal = pcszTagText;
		}
	}
	return pRetVal;
}


static bool FASTCALL GetParameterPair( CStaticString &strParams, CStaticString &strName, CStaticString &strParameter )
//
//	Extract the parameter pairs from pcszParams into pszNameBuffer and pszParameter
//
//	return null if pcszParams has no data left
{
	LPCTSTR pcszParams = strParams.GetData();
	LPCTSTR pcszParamsEnd = strParams.GetEndPointer();
	pcszParams = SkipWhitespace( pcszParams );
	if( *pcszParams )
	{
		//
		//	Get the parameter name
		LPCTSTR pcszName = pcszParams;
#ifdef UNICODE
		while( pcszParams < pcszParamsEnd && !_istspace( *pcszParams ) && *pcszParams != '=' )
#else	//	UNICODE
		while( pcszParams < pcszParamsEnd && !isspace( *pcszParams ) && *pcszParams != '=' )
#endif	//	UNICODE
		{
			pcszParams++;
		}
		strName.Set( pcszName, pcszParams - pcszName );

		//
		//	Now extract the parameter if there is one
		pcszParams = SkipWhitespace( pcszParams );
		if( pcszParams < pcszParamsEnd && *pcszParams == '=' )
		{
			pcszParams++;			//	skip the equals sign
			pcszParams = SkipWhitespace( pcszParams );
			TCHAR cEnd = ' ';
			if( *pcszParams == '"' )
			{
				cEnd = '"';
				pcszParams++;
			}

			LPCTSTR pcszParam = pcszParams;
			while( pcszParams < pcszParamsEnd && *pcszParams != cEnd )
			{
				pcszParams++;
			}

			strParameter.Set( pcszParam, pcszParams - pcszParam );

			if( *pcszParams == '"' )
			{
				pcszParams++;
			}


			PTRACE("\t\tParameter %s Value %s\n", String( strName ), String( strParameter ) );
			strParams.Set( pcszParams, pcszParamsEnd - pcszParams );
			return true;
		}
	}
	return false;
}


const int knStartBufferSize = 1024;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static int g_nDummy = CHTMLParseBase::Initialise();

CHTMLParseBase::CHTMLParseBase( LPCTSTR  pcszStream, UINT uLength )
	: m_pcszStream( pcszStream )
	, m_pcszStreamMax( m_pcszStream + uLength )
	, m_bPreviousCharWasSpace( false )
	, m_bPreviousWasSpecial( false )
	, m_bPreFormatted( false )
{
	//	Must have a string pointer
	ASSERT( m_pcszStream );
	//	Must be a valid string for the length requested.
	ASSERT_VALID_STR_LEN( m_pcszStream, uLength );


	//	CHTMLParseBase::Initialise() should have been called...
	ASSERT( g_mapCharacters.GetSize() );
	ASSERT( g_mapToken.GetSize() );
	ASSERT( g_mapParam.GetSize() );
}


int CHTMLParseBase::Initialise()
{
	//
	//	Initialise the lookup map for special characters
	if( g_mapCharacters.GetSize() == 0 )
	{
		for( UINT n = 0; n < countof( g_arrCharacters ); n++ )
		{
			ASSERT( g_arrCharacters[n].ch <= 255 );
			g_mapCharacters.SetAt( g_arrCharacters[n].pcszName, g_arrCharacters[n].ch );
		}
	}

	//
	//	Initialise the map for tags.
	if( g_mapToken.GetSize() == 0 )
	{
		for( UINT n = 0; n < countof( g_tokens ); n++ )
		{
			g_mapToken.SetAt( g_tokens[n].m_strName, g_tokens[n].m_tok );
		}
	}

	//
	//	Initialise the map for parameters.
	if( g_mapParam.GetSize() == 0 )
	{
		for( UINT n = 0; n < countof( g_params ); n++ )
		{
			g_mapParam.SetAt( g_params[n].m_strName, g_params[n].param );
		}
	}
	return 1;
}

CHTMLParseBase::~CHTMLParseBase()
{
}


inline void CHTMLParseBase::EatStreamWhitespace()
{
#ifdef UNICODE
	while( _istspace( *m_pcszStream ) && m_pcszStream < m_pcszStreamMax  )
#else	//	UNICODE
	while( isspace( *m_pcszStream ) && m_pcszStream < m_pcszStreamMax  )
#endif	//	UNICODE
		m_pcszStream++;
}


bool CHTMLParseBase::ParseBase()
{
	TCHAR ch;
	while( GetChar( ch ) )
	{
		if( ch == '<' && !m_bPreviousWasSpecial )
		{
			TCHAR cNextChar = *m_pcszStream;
			if( cNextChar == _T('/') )
				EatStreamWhitespace();

			CStaticString strTag;
			if( GetTag( strTag ) )
			{
				OnGotTagText( strTag );

				if( cNextChar != '/' && !m_bPreFormatted )
					EatStreamWhitespace();
			}
		}
		else
		{
			if( !AddTokenChar( ch ) )
			{
				return false;
			}
		}
	}

	//
	//	let any derived classes know that we are done.
	OnEndDoc();
	return true;
}


bool CHTMLParseBase::GetTagChar( TCHAR &ch )
//
//	Get a character from the stream
//	ONLY USED WHEN GETTING A TAG - does not convert special characters
//
//	Return false if there are no more characters in the stream
{
	//	Must have a stream pointer
	ASSERT( m_pcszStream );

	m_bPreviousWasSpecial = false;
	if( m_pcszStream > m_pcszStreamMax )
	{
		return false;
	}
	ch = *m_pcszStream++;
	return true;
}


bool CHTMLParseBase::GetChar( TCHAR &ch )
//
//	Get a character from the stream.
//	It wil turn all whitespace characters to a space character and
//	will interpret the code characters (&amp) as standard characters
//
//	Return false if there are no more characters in the stream
{
	//	Must have a stream pointer
	ASSERT( m_pcszStream );

	m_bPreviousWasSpecial = false;

	if( m_bPreFormatted )
	{
		if( m_pcszStream >= m_pcszStreamMax )
		{
			return false;
		}
		ch = *m_pcszStream++;
	}
	else
	{
		do
		{
			if( m_pcszStream >= m_pcszStreamMax )
			{
				return false;
			}
			ch = *m_pcszStream++;
		}
		while( ch == _T('\r') );
	}

	//
	//	If ther character we have is an ampersand then we attempt
	//	to read it as a special character, if that fails then we put back our
	//	stream position
	if( ch == _T('&') )
	{
		LPCTSTR pcszStream = m_pcszStream;
		TCHAR cLocal;
		if( GetSpecialChar( cLocal ) )
		{
			ch = cLocal;
			m_bPreviousWasSpecial = true;
		}
		else
		{
			m_pcszStream = pcszStream;
		}
	}

	if( !m_bPreFormatted )
	{
		if( ch == _T('\t') || ch == _T('\n') )
			ch = _T(' ');
	}
	return true;
}


bool CHTMLParseBase::AddTokenChar( const TCHAR ch )
//
//	Add a character to the token buffer, if need be expand the token buffer
//	as required, actually expands the token buffer by the size of the token buffer.
{
	//
	//	Handle multiple white space characters as though they
	//	are just one.
	if( !m_bPreFormatted )
	{
		if( ch ==_T(' ') )
		{
			if( m_bPreviousCharWasSpace )
				return true;

			m_bPreviousCharWasSpace = true;
		}
		else
		{
			m_bPreviousCharWasSpace = false;
		}
	}
	OnGotText( ch );
	return true;
}


bool CHTMLParseBase::GetSpecialChar( TCHAR &ch )
//
//	Parse the stream for a special character
{
	TCHAR szLocalTokenBuffer[ 256 ];
	UINT nCount = 0;
	TCHAR cLocal;
	while( GetChar( cLocal ) && cLocal != _T(';') )
	{
		szLocalTokenBuffer[ nCount ] = cLocal;
		nCount++;
		if( nCount >= countof( szLocalTokenBuffer ) )
			return false;
	}
	szLocalTokenBuffer[ nCount ] = _T('\000');
	bool bRetVal = false;
	if( szLocalTokenBuffer[ 0 ] == _T('#') )
	{
		ch = (TCHAR)_ttol( &szLocalTokenBuffer[ 1 ] );
		bRetVal = true;
	}
	else
	{
		TCHAR *pch = g_mapCharacters.Lookup( szLocalTokenBuffer );
		if( pch )
		{
			ASSERT( *pch <= 255 );
			ch = *pch;
			bRetVal = true;
		}
	}
	return bRetVal;
}


bool CHTMLParseBase::GetTag( CStaticString &strTag )
//
//	Get a tag, a tag is all of the text that appears between the angle brackets
{
	LPCTSTR pcszTagStart = m_pcszStream;
	TCHAR cLocal;
	bool bInQuotes = false;
	int nEnclosing = 1;
	while( GetTagChar( cLocal ) && (bInQuotes || nEnclosing ) )
	{
		if( cLocal == _T('<') )
			nEnclosing++;
		if( cLocal == _T('>') )
		{
			if( !--nEnclosing )
				break;
		}
		if( cLocal == _T('\"') )
			bInQuotes = !bInQuotes;
	}
	strTag.Set( pcszTagStart, m_pcszStream - pcszTagStart - 1 );
	return true;
}


void CHTMLParseBase::OnGotText( TCHAR )
//
//	Callback when some text has been interrupted with a tag or end tag.
{
}


void CHTMLParseBase::OnGotComment( LPCTSTR , UINT )
//
//	Send when a comment is found
{
}


void CHTMLParseBase::OnGotTagText( const CStaticString &strTag )
//
//	Called when a tag has been found
//	If the tag is '<img src="blah.jpg">' then pcszTagText will be 'img src="blah.jpg"'
{
	Token tok;
	bool bIsEnd;
	LPCTSTR pcszParamStart = GetTagToken( strTag, tok, bIsEnd );
	if( tok == tokIgnore )
	{
		LPCTSTR pcszTagText = strTag.GetData();

		//
		//	Comments come in as an entire tag
		if( strTag.GetLength() > 3 && !_tcsnicmp( pcszTagText, _T("!--" ), 3 ) )
		{
			//
			//	Skip the first three bytes to get rid of the comment tag
			pcszTagText += 3;

			LPCTSTR pcszEndComment = strTag.GetEndPointer();
			if( pcszEndComment > pcszTagText + 2 )
			{
				if( !_tcsnicmp( pcszEndComment - 2, _T("--" ), 2 ) )
					pcszEndComment -= 2;
			}

			//
			//	Send the comment to our parser
			OnGotComment( pcszTagText, pcszEndComment - pcszTagText );
			return;
		}
		return;
	}

	if( bIsEnd )
	{
		OnGotEndTag( tok );

		switch( tok )
		{
		case tokPre:
			m_bPreFormatted = false;
			break;

		case tokH1:
		case tokH2:
		case tokH3:
		case tokH4:
		case tokH5:
		case tokH6:
		case tokTable:
		case tokTableDef:
		case tokTableRow:
			EatStreamWhitespace();
		}
	}
	else
	{
		switch( tok )
		{
		case tokPre:
			m_bPreFormatted = true;
			m_bPreviousCharWasSpace = false;
		}

		CParameters plist;

		//
		//	If there are some parameters then parse them and build up the
		//	parameter list
		if( pcszParamStart )
		{
			PTRACE( "Tag %s\n", String( strTag ) );
			CStaticString strNameBuffer;
			CParameterPair ppair;
			CStaticString strParam( pcszParamStart, strTag.GetEndPointer() - pcszParamStart );
			while( strParam.GetLength() && GetParameterPair( strParam, strNameBuffer, ppair.m_strValue ) )
			{
				CHTMLParseBase::Param *p = g_mapParam.Lookup( strNameBuffer );
				if( p )
				{
					ppair.m_param = *p;
				}
				else
				{
					ppair.m_param = pUnknown;
				}
				plist.Add( ppair );
			}
		}
		OnGotTag( tok, plist );
	}
}


void CHTMLParseBase::OnGotTag( const Token, const CParameters & )
//
//	Called when a tag is parsed, complete with the parameter list
{
}

void CHTMLParseBase::OnGotEndTag( const Token )
//
//	Called when an end tag is parsed
{
}

void CHTMLParseBase::OnEndDoc()
{
}

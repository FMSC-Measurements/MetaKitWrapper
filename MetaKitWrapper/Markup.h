// Markup.h: interface for the CMarkup class.
//
// Markup Release 6.6
// Copyright (C) 1999-2004 First Objective Software, Inc. All rights reserved
// Go to www.firstobject.com for the latest CMarkup and EDOM documentation
// Use in commercial applications requires written permission
// This software is provided "as is", with no warranty.

using namespace std;

#if !defined(AFX_MARKUP_H__948A2705_9E68_11D2_A0BF_00105A27C570__INCLUDED_)
#define AFX_MARKUP_H__948A2705_9E68_11D2_A0BF_00105A27C570__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning( disable: 4711 ) // function 'function' selected for automatic inline expansion

#include <afxtempl.h>

#ifdef _DEBUG
#define _DS(i) (i?&((LPCTSTR)m_csDoc)[m_aPos[i].nStartL]:0)
#define MARKUP_SETDEBUGSTATE m_pMainDS=_DS(m_iPos); m_pChildDS=_DS(m_iPosChild)
#else
#define MARKUP_SETDEBUGSTATE
#endif

class CMarkup  
{
public:
	CMarkup() { SetDoc( NULL ); m_nFlags=0; };
	CMarkup( LPCTSTR szDoc ) { SetDoc( szDoc ); };
	CMarkup( const CMarkup& markup ) { *this = markup; };
	void operator=( const CMarkup& markup );
	virtual ~CMarkup() {};

	// Navigate
	bool Load( LPCTSTR szFileName );
	bool SetDoc( LPCTSTR szDoc );
	bool IsWellFormed();
	bool FindElem( LPCTSTR szName=NULL );
	bool FindChildElem( LPCTSTR szName=NULL );
	bool IntoElem();
	bool OutOfElem();
	void ResetChildPos() { x_SetPos(m_iPosParent,m_iPos,0); };
	void ResetMainPos() { x_SetPos(m_iPosParent,0,0); };
	void ResetPos() { x_SetPos(0,0,0); };
	CString GetTagName() const;
	CString GetChildTagName() const { return x_GetTagName(m_iPosChild); };
	CString GetData() const { return x_GetData(m_iPos); };
	CString GetChildData() const { return x_GetData(m_iPosChild); };
	CString GetAttrib( LPCTSTR szAttrib ) const { return x_GetAttrib(m_iPos,szAttrib); };
	CString GetChildAttrib( LPCTSTR szAttrib ) const { return x_GetAttrib(m_iPosChild,szAttrib); };
	CString GetAttribName( int n ) const;
	bool SavePos( LPCTSTR szPosName=_T("") );
	bool RestorePos( LPCTSTR szPosName=_T("") );
	const CString& GetError() const { return m_csError; };

	enum MarkupNodeType
	{
		MNT_ELEMENT					= 1,  // 0x01
		MNT_TEXT					= 2,  // 0x02
		MNT_WHITESPACE				= 4,  // 0x04
		MNT_CDATA_SECTION			= 8,  // 0x08
		MNT_PROCESSING_INSTRUCTION	= 16, // 0x10
		MNT_COMMENT					= 32, // 0x20
		MNT_DOCUMENT_TYPE			= 64, // 0x40
		MNT_EXCLUDE_WHITESPACE		= 123,// 0x7b
	};

	// Create
	bool Save( LPCTSTR szFileName );
	const CString& GetDoc() const { return m_csDoc; };
	bool AddElem( LPCTSTR szName, LPCTSTR szData=NULL ) { return x_AddElem(szName,szData,false,false); };
	bool InsertElem( LPCTSTR szName, LPCTSTR szData=NULL ) { return x_AddElem(szName,szData,true,false); };
	bool AddChildElem( LPCTSTR szName, LPCTSTR szData=NULL ) { return x_AddElem(szName,szData,false,true); };
	bool InsertChildElem( LPCTSTR szName, LPCTSTR szData=NULL ) { return x_AddElem(szName,szData,true,true); };
	bool AddAttrib( LPCTSTR szAttrib, LPCTSTR szValue ) { return x_SetAttrib(m_iPos,szAttrib,szValue); };
	bool AddChildAttrib( LPCTSTR szAttrib, LPCTSTR szValue ) { return x_SetAttrib(m_iPosChild,szAttrib,szValue); };
	bool AddAttrib( LPCTSTR szAttrib, int nValue ) { return x_SetAttrib(m_iPos,szAttrib,nValue); };
	bool AddChildAttrib( LPCTSTR szAttrib, int nValue ) { return x_SetAttrib(m_iPosChild,szAttrib,nValue); };
	bool AddSubDoc( LPCTSTR szSubDoc ) { return x_AddSubDoc(szSubDoc,false,false); };
	bool InsertSubDoc( LPCTSTR szSubDoc ) { return x_AddSubDoc(szSubDoc,true,false); };
	CString GetSubDoc() const { return x_GetSubDoc(m_iPos); };
	bool AddChildSubDoc( LPCTSTR szSubDoc ) { return x_AddSubDoc(szSubDoc,false,true); };
	bool InsertChildSubDoc( LPCTSTR szSubDoc ) { return x_AddSubDoc(szSubDoc,true,true); };
	CString GetChildSubDoc() const { return x_GetSubDoc(m_iPosChild); };

	// Modify
	bool RemoveElem();
	bool RemoveChildElem();
	bool SetAttrib( LPCTSTR szAttrib, LPCTSTR szValue ) { return x_SetAttrib(m_iPos,szAttrib,szValue); };
	bool SetChildAttrib( LPCTSTR szAttrib, LPCTSTR szValue ) { return x_SetAttrib(m_iPosChild,szAttrib,szValue); };
	bool SetAttrib( LPCTSTR szAttrib, int nValue ) { return x_SetAttrib(m_iPos,szAttrib,nValue); };
	bool SetChildAttrib( LPCTSTR szAttrib, int nValue ) { return x_SetAttrib(m_iPosChild,szAttrib,nValue); };
	bool SetData( LPCTSTR szData, int nCDATA=0 ) { return x_SetData(m_iPos,szData,nCDATA); };
	bool SetChildData( LPCTSTR szData, int nCDATA=0 ) { return x_SetData(m_iPosChild,szData,nCDATA); };


protected:

#ifdef _DEBUG
	LPCTSTR m_pMainDS;
	LPCTSTR m_pChildDS;
#endif

	CString m_csDoc;
	CString m_csError;
	int m_nFlags;

	struct ElemPos
	{
		ElemPos() { Clear(); };
		ElemPos( const ElemPos& pos ) { *this = pos; };
		bool IsEmptyElement() const { return (nStartR == nEndL + 1); };
		void Clear() { memset( this, 0, sizeof(ElemPos) ); };
		void AdjustStart( int n ) { nStartL+=n; nStartR+=n; };
		void AdjustEnd( int n ) { nEndL+=n; nEndR+=n; };
		int nStartL;
		int nStartR;
		int nEndL;
		int nEndR;
		int nReserved;
		int iElemParent;
		int iElemChild;
		int iElemNext;
	};

	int m_iPosParent;
	int m_iPos;
	int m_iPosChild;
	int m_iPosFree;
	int m_nNodeType;

	struct TokenPos
	{
		TokenPos( LPCTSTR sz ) { Clear(); szDoc = sz; };
		void Clear() { nL=0; nR=-1; nNext=0; bIsString=false; };
		bool Match( LPCTSTR szName )
		{
			int nLen = nR - nL + 1;
		// To ignore case, define MARKUP_IGNORECASE
		#ifdef MARKUP_IGNORECASE
			return ( (_tcsncicmp( &szDoc[nL], szName, nLen ) == 0)
		#else
			return ( (_tcsnccmp( &szDoc[nL], szName, nLen ) == 0)
		#endif
				&& ( szName[nLen] == _T('\0') || _tcschr(_T(" =/["),szName[nLen]) ) );
		};
		int nL;
		int nR;
		int nNext;
		LPCTSTR szDoc;
		bool bIsString;
	};

	struct SavedPos
	{
		int iPosParent;
		int iPos;
		int iPosChild;
	};
	CMap<CString,LPCTSTR,SavedPos,SavedPos> m_mapSavedPos;

	void x_SetPos( int iPosParent, int iPos, int iPosChild )
	{
		m_iPosParent = iPosParent;
		m_iPos = iPos;
		m_iPosChild = iPosChild;
		m_nNodeType = iPos?MNT_ELEMENT:0;
		MARKUP_SETDEBUGSTATE;
	};

	struct PosArray
	{
		PosArray() { Clear(); };
		~PosArray() { Release(); };
		enum { PA_SEGBITS = 16, PA_SEGMASK = 0xffff };
		void RemoveAll() { Release(); Clear(); };
		void Release() { for (int n=0;n<SegsUsed();++n) delete[] (char*)pSegs[n]; if (pSegs) delete[] (char*)pSegs; };
		void Clear() { nSegs=0; nSize=0; pSegs=NULL; };
		int GetSize() const { return nSize; };
		int SegsUsed() const { return ((nSize-1)>>PA_SEGBITS) + 1; };
		ElemPos& operator[](int n) const { return pSegs[n>>PA_SEGBITS][n&PA_SEGMASK]; };
		ElemPos** pSegs;
		int nSize;
		int nSegs;
	};
	PosArray m_aPos;

	int x_GetFreePos() { if (m_iPosFree==m_aPos.GetSize()) x_AllocPosArray(); return m_iPosFree++; };
	int x_ReleasePos() { --m_iPosFree; return 0; };
	bool x_AllocPosArray( int nNewSize = 0 );
	bool x_ParseDoc();
	int x_ParseElem( int iPos );
	int x_ParseError( LPCTSTR szError, LPCTSTR szName = NULL );
	static bool x_FindChar( LPCTSTR szDoc, int& nChar, _TCHAR c );
	static bool x_FindAny( LPCTSTR szDoc, int& nChar );
	static bool x_FindToken( TokenPos& token );
	CString x_GetToken( const TokenPos& token ) const;
	int x_FindElem( int iPosParent, int iPos, LPCTSTR szPath );
	CString x_GetTagName( int iPos ) const;
	CString x_GetData( int iPos ) const;
	CString x_GetAttrib( int iPos, LPCTSTR szAttrib ) const;
	bool x_AddElem( LPCTSTR szName, LPCTSTR szValue, bool bInsert, bool bAddChild );
	CString x_GetSubDoc( int iPos ) const;
	bool x_AddSubDoc( LPCTSTR szSubDoc, bool bInsert, bool bAddChild );
	bool x_FindAttrib( TokenPos& token, LPCTSTR szAttrib=NULL ) const;
	bool x_SetAttrib( int iPos, LPCTSTR szAttrib, LPCTSTR szValue );
	bool x_SetAttrib( int iPos, LPCTSTR szAttrib, int nValue );
	bool x_CreateNode( CString& csNode, int nNodeType, LPCTSTR szText );
	void x_LocateNew( int iPosParent, int& iPosRel, int& nOffset, int nLength, int nFlags );
	int x_ParseNode( TokenPos& token );
	bool x_SetData( int iPos, LPCTSTR szData, int nCDATA );
	int x_RemoveElem( int iPos );
	void x_DocChange( int nLeft, int nReplace, const CString& csInsert );
	void x_PosInsert( int iPos, int nInsertLength );
	void x_Adjust( int iPos, int nShift, bool bAfterPos = false );
	CString x_TextToDoc( LPCTSTR szText, bool bAttrib = false ) const;
	CString x_TextFromDoc( int nLeft, int nRight ) const;
};

#endif // !defined(AFX_MARKUP_H__948A2705_9E68_11D2_A0BF_00105A27C570__INCLUDED_)

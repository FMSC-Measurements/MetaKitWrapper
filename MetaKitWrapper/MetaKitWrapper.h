// MetaKitWrapper.h

#pragma once

#include "Stdafx.h"
#include "mk4.h"
#include "mk4str.h"
//#include <string>
//#include <cstring>
//#include "string.h"
#include <atlstr.h> 

#ifndef VGMAXBUF
  #define VGMAXBUF 1000
#endif

#ifndef VGMAXOPEN
  #define VGMAXOPEN 25
#endif

// defines for import, export file types
#define DATATYPE_MK4 1
#define DATATYPE_CSVWHDR 2
#define DATATYPE_CSVNOHDR 3
#define DATATYPE_DBF 4
#define DATATYPE_MSACCESS 5
#define DATATYPE_XML 6

// mode for data import, export to append or to replace
#define DATAMODE_APPEND 1
#define DATAMODE_REPLACE 2

// Error condition defines
#define ERR_NONE 0
#define ERR_DBNOTOPEN -2
#define ERR_VIEWNOTOPEN -3
#define ERR_NOPROP -4
#define ERR_NOFILE -5

// file open defines
#define FILE_DOES_EXIST 0
#define FILE_DOESNOT_EXIST -1
#define FILE_NOTALREADY_OPEN 0
#define FILE_ALREADY_OPEN -2
#define FILE_ACCESS_DENIED -3
#define FILE_ERROR -4  //catch all for other file errors

#define DELETEANDNULL(a) if (a!=NULL) {delete a; a=NULL;}
#define DELETEANDNULLALL(a) if (a!=NULL) {delete [] a; a=NULL;}

using namespace System;

namespace MetaKitWrapper
{

	public class MetaKitManager
	{
	private:
		bool _DbOpen;
		char* _FileName;
		c4_Storage* _Database;
		c4_View m_ActiveViewHandle[VGMAXOPEN];
		char* m_ptrActiveTableName[VGMAXOPEN];
		char* m_ptrActiveTableStructure[VGMAXOPEN];
		int _Views;

	public:

		MetaKitManager();

		bool OpenDB(char* filename);
		bool CloseDB();

		bool IsOpen();


		int GetDataItem(const int ViewNum, const long RowNum, const int ColNum, char* FieldType,
			CString* StrRet, long* LongOrIntRet, double* DoubleOrFloatRet);

		int GetInt(const int ViewNum, const long RowNum, const int ColNum, int* RetVal);
		int GetString(const int ViewNum, const long RowNum, const int ColNum,  CString& RetVal);
		int GetFloat(const int ViewNum, const long RowNum, const int ColNum,  float* RetVal);
		int GetDouble(const int ViewNum, const long RowNum, const int ColNum,  double* RetVal);
		int GetLong(const int ViewNum, const long RowNum, const int ColNum, long* RetVal);

		int StringToChar(const CString StringToConvert, char** ptrCharArray);

		int OpenAView(CString sAViewName);

		int GetRowCount(int ViewNum);
	};


	public ref class MetaKit
	{
	private:
		char* _FileName;
		MetaKitManager* _Manager;

	public:

		~MetaKit();

		bool OpenDB(String^ filename);
		bool CloseDB();

		String^ GetString(int viewNum, int rowNum, int columnNum);
		int GetInt(int viewNum, int rowNum, int columnNum);
		long GetLong(int viewNum, int rowNum, int columnNum);
		float GetFloat(int viewNum, int rowNum, int columnNum);
		double GetDouble(int viewNum, int rowNum, int columnNum);

		int GetDataItem(int ViewNum, int RowNum, int ColNum, String^& FieldType,
			String^& Str, int& LongOrIntRet, double& DoubleOrFloatRet);

		int OpenView(String^ viewName);

		int GetRowCount(int viewNum);
	};
}

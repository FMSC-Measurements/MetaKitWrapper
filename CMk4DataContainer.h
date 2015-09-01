#ifndef _CMK4DATACONTAINER_H
#define _CMK4DATACONTAINER_H

//@START_USER1
#include "mk4.h"
#include "mk4str.h"
#ifdef WINCE
  #define EXCLUDEIMPORTEXPORT
#endif

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <afxwin.h>
#ifndef q4_WINCE
  #include <direct.h> // for file find - likely not supported by Win CE
  #include <time.h>
#endif

#ifndef EXCLUDEIMPORTEXPORT
  #include <afxdao.h>

//  #include "\ProgProj\CMarkup\Markup.h"  /* Rich /*
  #include "Markup.h"  /* Matt */

#endif

// Structure for Default data creation
struct DEFAULTVALUES 
{
    char cMethod;
    char* ptrConstant;
    char* ptrValidateMin;
    char* ptrValidateMax;
    char* ptrValidateList;
    int iDisplayLength;
};
// Default methods
#define DEFNONE 'N'
#define DEFVALUE 'V'
#define DEFCOPY 'C'
#define DEFINC 'I'

// macro for a smart delete to test ptr for null, delete, and set to null
#define DELETEANDNULL(a) if (a!=NULL) {delete a; a=NULL;}
#define DELETEANDNULLALL(a) if (a!=NULL) {delete [] a; a=NULL;}

// defines for DB status
#define DB_NOT_AVAIL  -1
#define DB_AVAIL_STORE_OPEN 1
#define DB_AVAIL_STORE_CLOSED_HAS_VIEWS 2
#define DB_AVAIL_STORE_CLOSED_NO_VIEWS 3
//@END_USER1


/*@NOTE_95
Member functions to open a database file with data members to track actual
tables and data. Currently, uses MetaKit for the implementation.
*/

class CMk4DataContainer
{

//@START_USER2
//@END_USER2

//
// Group: GlobalData
//

private:
    bool m_bdbIsValid;
    char* m_ptrFileName;
    static bool m_bOpenViewsInMemory;

protected:
    CString m_sVersionNumber;
    int m_iDBStatus;

public:
    c4_View m_ActiveViewHandle[VGMAXOPEN];
    CString m_sDDColNameForTable;
    CString m_sDDViewName;
    char* m_ptrActiveTableName[VGMAXOPEN];
    char* m_ptrActiveTableStructure[VGMAXOPEN];
    c4_Storage* m_ptrDataStore;
    CString m_sDDColNameForColumnName;
    CString m_sDDColNameForFType;
    CString m_sDDColNameForIsUsed;
    CString m_sDDStructure;
    CString m_sListOfFirstLevelTables;
    CString m_sFileName;
    CString m_sDDColNameForColHead;
    CString m_sDDColNameForDispLen;
    int m_iParentIndex[VGMAXOPEN];
    char* m_ptrParentTableName[VGMAXOPEN];
    bool m_bViewUsesDD[VGMAXOPEN];
    int m_iNumDefFields[VGMAXOPEN];
    DEFAULTVALUES* m_ptrDefaultValues[VGMAXOPEN];
    CString m_sDDColNameForDefValue;
    CString m_sMsgLogViewName;
    bool m_bDoStringTrim;
    bool m_bViewIsCopyInMemory[VGMAXOPEN];
    bool GetbdbIsValid() const;
    char* GetPtrFileName() const;
    const CString& GetSVersionNumber() const;
    int GetIDBStatus() const;
    bool GetBDoStringTrim() const;
    void SetBDoStringTrim(bool bDoStringTrim);
	int GetUsedViewCount();


//
// Group: ClassBuilder methods
//

private:
    void ConstructorInclude();
    void DestructorInclude();

//
// Group: DBFileManagement
//

private:
    int BuildListOfViews();
    int Initialize();
    int ReopenDB();
    int OpenStorage();

public:
    int OpenDBFile(const char* ptrFileToOpen, bool bOpenViewsInMemory = FALSE);
    int OpenDBFile(CString sFileToOpen, bool bOpenViewsInMemory = FALSE);
    int SaveData();
    int CloseDB(bool bCloseFileOnlyLeaveViewsInMemory = FALSE);
	bool DBAvailable();

//
// Group: Data access
//

private:
    int SetDataItemWorker(const int ViewNum, const long RowNum,
                          const int ColNum, const char DataTypeIn,
                          const CString StrIn, const int IntIn,
                          const long LongIn, const float FloatIn,
                          const double DoubleIn);

public:
    int GetDataItem(const int ViewNum, const long RowNum, const int ColNum,
                    char* FieldType, CString* StrRet, long* LongOrIntRet,
                    double* DoubleOrFloatRet);
    int GetDataItemAs(const int ViewNum, const long RowNum, const int ColNum,
                      int* RetVal);
    int GetDataItemAs(const int ViewNum, const long RowNum, const int ColNum,
                      CString& RetVal);
    int GetDataItemAs(const int ViewNum, const long RowNum, const int ColNum,
                      float* RetVal);
    int GetDataItemAs(const int ViewNum, const long RowNum, const int ColNum,
                      double* RetVal);
    int GetDataItemAs(const int ViewNum, const long RowNum, const int ColNum,
                      long* RetVal);
    
	
	
	int SetDataItem(const int ViewNum, const long RowNum, const int ColNum,
                    const char DataTypeIn, const CString StrIn, const int IntIn,
                    const long LongIn, const float FloatIn,
                    const double DoubleIn);
    int SetDataItemFrom(const int ViewNum, const long RowNum, const int ColNum,
                        const int SetVal);
    int SetDataItemFrom(const int ViewNum, const long RowNum, const int ColNum,
                        const CString& SetVal);
    int SetDataItemFrom(const int ViewNum, const long RowNum, const int ColNum,
                        const float SetVal);
    int SetDataItemFrom(const int ViewNum, const long RowNum, const int ColNum,
                        const double SetVal);
    int SetDataItemFrom(const int ViewNum, const long RowNum, const int ColNum,
                        const long SetVal);
    int StringToChar(const CString StringToConvert, char** ptrCharArray);
    long AppendARow(const int ViewNum, long lInsertAt = -1);
    long DeleteARow(const int ViewNum, long nRow);
    int FindColIndexByName(const int iViewNum, CString sColName);
    long IncrementField(int iViewIndex, long lRow, int iCol, int iIncrement = 1);
    int ValidateFieldWithDD(int iViewIndex, long lRow, int iCol,
                            CString& sFieldName, CString& sErrMsg);
    int ValidateRowWithDD(int iViewIndex, long lRow, int* iCol,
                          CString& sFieldName, CString& sErrMsg);
    int ValidateViewWithDD(int iViewIndex, long* lRow, int* iCol,
                           CString& sFieldName, CString& sErrMsg);
    int KeyValueAccess(CString sViewName, CString sSelectionCriteria,
                       CString sValueFieldName, CString& sValue,
                       BOOL bSetValue = FALSE);

//
// Group: ViewManagement
//

private:
    int MoveColToFront(CString& refStructure, CString& refColName);

public:
    int CloseAView(int iToClose);
    int CloseAView(const char* ptrAViewName);
    int DeleteView(const char* ptrAViewName);
    int ExportView(const int FileType, const CString FileName,
                   const char* TableName, const int Mode);
    c4_View& GetViewByName(const char* ptrAViewName, int* iErr);
    int FindViewIndexByName(const char* ptrAViewName);
    int FindFreeViewIndex(const char* ptrAViewName);
    int ImportView(int FileType, CString FileName, const char* TableName,
                   int Mode);
    int OpenAView(const char* ptrAViewName,
                  bool bOpenViewInMemory = m_bOpenViewsInMemory);
    int OpenAView(CString sAViewName,
                  bool bOpenViewInMemory = m_bOpenViewsInMemory);
#ifndef q4_WINCE                  
    int SortViewOn(const char* ptrViewName, const char* ptrSortCol);
#endif
    bool ViewIsOpen(const char* ptrAViewName, int* ViewIndex);
    int MakeDataDictView();
    int MakeViewFromDataDictionary(const char* ptrAViewName,
                                   const char* ptrSortCol);
    int FindFreeViewIndex(const char* ptrAViewName,
                          const char* ptrParentViewName);
    int FindViewIndexByName(const CString& strAViewName);
    int AttachDDToView(const int ViewNum);
    int DetachDDFromView(const int ViewNum);
	int DeriveView(const CString sRawViewName,
                    const CString sProjectionList,
                    const CString sSelectCriteria = "",
                    const CString sSortOn = "",
                    CString sDerViewName = "",
                    const CString sSelectRangeCriteria = "");

    int ExportAllViews(const CString FileName, int iDataType = DATATYPE_MK4);
    int DoesViewExist(const char* ptrAViewName);
    int ImportAllViews(CString sFileName, int iDataType = DATATYPE_MK4,
                       CString sFileNameForDB = "");
    int RenameView(CString sOldName, CString sNewName);
    int CopyViewFromFileToMemory(CString sViewNameOnFile,
                                 int iViewIndexInMemory);
    int CopyViewFromMemoryToFile(CString sViewNameOnFile,
                                 int iViewIndexInMemory);
    int TransposeFromRowToUnattachedView(CString sRawViewName, long lRowIndex,
                                         CString sNewViewName = "");
    int TransposeFromUnattachedViewToRowAndClose(int iViewIndex,
                                                 CString sRawViewName,
                                                 long lRowIndex);
    int RedefineAColumn(CString sViewContainingColumn, CString sOldColumnName,
                        CString sNewColumnName, char cNewFieldType = ' ');

//
// Group: Utility
//

private:
    void PopMsg(CString Message, int ErrCode);

public:
    void NextToken(CString& sMessage, CString& sToken,
                   const CString sDelimiters = ",");
    int ReadLine(FILE* pfile, CString& sLineRead);
    int LogMessage(const int MsgNum, const CString MsgType, const CString Note,
                   CString sProgram = "");
    int LogMessage(const CString sMsg, const CString sMsgType,
                   const CString sNote, CString sProgram = "");
    int MakeProjectionList(CString& sProjectionList, const CString& sTableName,
                           const CString& sOrderColumn = "");
    int MakeViewTemplateFromList(c4_View& ViewTemplate, c4_View& ParentView,
                                 const CString& sAttributeList);
    int TrimAllStringsInTable(const CString& sTableToTrim);
    int MakeListOfUniqueValues(const CString& sTableName,
                               const CString& sColumnName, CString& sListUnique);
    int FileAlreadyOpen(const CString sDataFile);
    int FileExists(const CString sDataFile);
	int FileExistsAndAvailable(const CString sDataFile);
    int RenameAsBackup(CString& sFileToRename);
    int DescribeStructure(CString& sStructure, const int iView,
                          bool bReturnCSVFieldList = FALSE);
    int MakeOrderFromList(CString sOrderList, CString sTableName,
                          CString sOrderColumn);

//
// Non-Grouped Members
//

//
// Non-Grouped Methods
//

public:
    CMk4DataContainer(const char* ptrFileToOpen);
    CMk4DataContainer();
    virtual ~CMk4DataContainer();
};

#endif


#ifdef CB_INLINES
#ifndef _CMK4DATACONTAINER_H_INLINES
#define _CMK4DATACONTAINER_H_INLINES


//
// Group: GlobalData
//
/*@NOTE_88
Returns the value of member 'm_bdbIsValid'.
*/
inline bool CMk4DataContainer::GetbdbIsValid() const
{//@CODE_88
    return m_bdbIsValid;
}//@CODE_88



/*@NOTE_83
Returns the value of member 'm_ptrFileName'.
*/
inline char* CMk4DataContainer::GetPtrFileName() const
{//@CODE_83
    return m_ptrFileName;
}//@CODE_83



/*@NOTE_505
Returns the value of member 'm_bDoStringTrim'.
*/
inline bool CMk4DataContainer::GetBDoStringTrim() const
{//@CODE_505
    return m_bDoStringTrim;
}//@CODE_505



/*@NOTE_506
Set the value of member 'm_bDoStringTrim' to 'bDoStringTrim'.
*/
inline void CMk4DataContainer::SetBDoStringTrim(bool bDoStringTrim)
{//@CODE_506
    m_bDoStringTrim = bDoStringTrim;
}//@CODE_506



/*@NOTE_594
Returns the value of member 'm_sVersionNumber'.
*/
inline const CString& CMk4DataContainer::GetSVersionNumber() const
{//@CODE_594
    return m_sVersionNumber;
}//@CODE_594



/*@NOTE_641
Returns the value of member 'm_iDBStatus'.
*/
inline int CMk4DataContainer::GetIDBStatus() const
{//@CODE_641
    return m_iDBStatus;
}//@CODE_641



// End of Group: GlobalData

//@START_USER3
//@END_USER3

#endif
#endif

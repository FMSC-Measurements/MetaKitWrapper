
// Master include file
#include "Mk4Data.h"


#if (_WIN32_WCE < 300)
#include "preCE3fixes.h"
#endif

//@START_USER2
//@END_USER2



//
// Group: GlobalData
//
// Static members
bool CMk4DataContainer::m_bOpenViewsInMemory = FALSE;
// End of Group: GlobalData





//
// Group: DBFileManagement
//
/*@NOTE_121
Open the db file in a member function rather than the class constructor. Name
of file need not be known at the time of the storage constructor. Open or
reopen a file when it needs to be. Checks to make sure this file is not already
open.
*/
int CMk4DataContainer::OpenDBFile(const char* ptrFileToOpen,
                                  bool bOpenViewsInMemory)
{//@CODE_121
    int value;
    
    value = ERR_NONE;  // set return value
    
    // check to see if already an open db, close if there is one open
    if (m_iDBStatus != DB_NOT_AVAIL)
    {
        CloseDB();
    }
    
    // save the request to open views as copies in memory
    m_bOpenViewsInMemory = bOpenViewsInMemory;
    
    // if file exists make sure it is not already open since twice will corrupt
    // skip this if on win ce
#ifndef q4_WINCE
    int iTemp = FileAlreadyOpen(ptrFileToOpen);
    if (iTemp != 0)
        return ERR_NOFILE;
#endif
    
    // copy the file name into class
    m_ptrFileName = (char*) new char[(strlen(ptrFileToOpen) + 1)];
    strcpy(m_ptrFileName, ptrFileToOpen);
    m_sFileName = m_ptrFileName;    // save a copy as a string also
    // *** need to check for file existance
    
    // open up the database file
    OpenStorage();
    // *** need to check for success
    
    // set db status flag
    m_iDBStatus = DB_AVAIL_STORE_OPEN;
    
    BuildListOfViews();
    
    return value;
}//@CODE_121


/*@NOTE_174
Override open function to open with CString rather than char array. Converts to
a char array then calls open with char array.
*/
int CMk4DataContainer::OpenDBFile(CString sFileToOpen, bool bOpenViewsInMemory)
{//@CODE_174
    int value;
    
    value = 0;      // initialize return value
    
    // convert the string into a char for SBCS
    char * temp = NULL;    // to hold the string
	StringToChar(sFileToOpen, &temp);
    
    value = OpenDBFile(temp, bOpenViewsInMemory);
    
    DELETEANDNULLALL(temp);    // clean up allocation
  
    return value;
    
    /*
    // need to handle unicode conversion of string if ce based
    // this is a first cut at a conversion
    #ifdef q4_WINCE
    char* sbcharstring;
    sbcharstring = (char*) new char[cs.GetLength() + 1];
    ((c4_StringProp&) p)(r) = sbcharstring;
    delete sbcharstring;
    // else, handle as non unicode
    #else
    ((c4_StringProp&) p)(r) = cs;
    #endif
    */
}//@CODE_174


/*@NOTE_192
Commit the changes made to the storage to make them permanant.
*/
int CMk4DataContainer::SaveData()
{//@CODE_192
    int value = 0;
    int i;
    CString sTemp;
    bool bNeedToCloseStorage = FALSE;

    if (m_iDBStatus == DB_NOT_AVAIL)
    {
        // error if store not open
        value = ERR_DBNOTOPEN;
#ifndef q4_WINCE
        PopMsg("Committing ata", value);
#endif
        return value;
    }

    // check to see if the storage is open
    if (m_ptrDataStore == NULL)
    {
        OpenStorage();
        bNeedToCloseStorage = TRUE;
    }

    // copy any views in memory back to the file
    for (i = 0; i < VGMAXOPEN; i++)
    {
        if (m_ptrActiveTableName[i] != NULL && m_bViewIsCopyInMemory[i])
        {
            sTemp = m_ptrActiveTableName[i];
            CopyViewFromMemoryToFile(sTemp, i);
        }
    }
    
    // make the data permenant
    m_ptrDataStore->Commit();
    
    //close the storage if needed
    if (bNeedToCloseStorage)
    {
        // make the data permenant
        //m_ptrDataStore->Commit();
        DELETEANDNULL(m_ptrDataStore);
    }

    return value;
}//@CODE_192


/*@NOTE_148
Build a comma delimited list of first level views by examining all of the
storage properties and looking for the view(V) property type. Store the string
list with the view names in a global data class member.
*/
int CMk4DataContainer::BuildListOfViews()
{//@CODE_148
    int value = 0;
    
    int nviews;
    m_sListOfFirstLevelTables = ""; // blank out the list
    
    if (m_iDBStatus == DB_NOT_AVAIL)
    {
        // error if store not open
        value = ERR_DBNOTOPEN;
#ifndef q4_WINCE
        PopMsg("Building List of Views", value);
#endif
        return value;
    }
    
    // build the comma delimited list of view names in this file
    nviews = m_ptrDataStore->NumProperties();   // the number of attributes (views) in db
    for (int k = 0; k < nviews; k++)
    {
        // iterate through props and build list of names
        c4_Property prop(m_ptrDataStore->NthProperty(k));
        CString sName = prop.Name();
        if (m_sListOfFirstLevelTables.GetLength() > 0)
            m_sListOfFirstLevelTables += ", ";
        if (prop.Type() == 'V')
            m_sListOfFirstLevelTables += sName;
    }
    
    
    /* old stuff, superseeded
    CString desc, views, temp, temp2;
    int loc, loc2;
    
      value = 0;
      // start off with the mk4 description/structure
      desc = ptrCharsToParse;
      loc = 1;    // initialize so it goes into while loop
      // loop through and find the all of the views
      while (loc > 0) 
      {
      loc = desc.Find(":V");      // a view is indicated by :V in the structure
      if (loc > 0)
      {
      // get everything up to (not including) the :V
      temp = desc.Left(loc - 1);
      // get the back end of the string up to next delimiter, ie. table name
      loc2 = temp.ReverseFind(',');   // find delimiter at front of name
      temp2 = temp.Right(temp.GetLength() - loc2 - 1);    // grab the table name
      if (views.GetLength() > 0)
      views += ", "; // add the comma delimiter to list
      views += temp2;     // add the name to the list
      
        // lop off front for next itteration
        temp = desc.Right(desc.GetLength() - loc);
        desc = temp;
        }
        }   // while a :V has been found
        
          // set the list into member variable
          if (m_sListOfFirstLevelTables != NULL)
          delete m_sListOfFirstLevelTables;
          m_sListOfFirstLevelTables = new char[views.GetLength() + 1];
          strcpy(m_sListOfFirstLevelTables, views);
    */
    
    return value;
}//@CODE_148


/*@NOTE_140
Initialize the variables in the class. Just sets a bunch of constants.
*/
int CMk4DataContainer::Initialize()
{//@CODE_140
    int value;
    
    value = 0;
    
    // Initialize all of the member variables
    m_bdbIsValid = false;
    m_iDBStatus = DB_NOT_AVAIL;
    m_ptrFileName = NULL;
    m_ptrDataStore = NULL;
    m_sListOfFirstLevelTables;
    int i;
    for (i = 0; i < VGMAXOPEN; i++)
    {
        m_ptrActiveTableName[i] = NULL;
        m_ptrActiveTableStructure[i] = NULL;
        m_ActiveViewHandle[i] = c4_View();    // initialize to empty
        m_ptrParentTableName[i] = NULL;
        m_iParentIndex[i] = -1;
        m_bViewUsesDD[i] = FALSE;
        m_ptrDefaultValues[i] = NULL;
        m_bViewIsCopyInMemory[i] = FALSE;
    }
    
    // will have to deal with this: wanting string to be unicode - can't be for metakit
    // will need to re-declare as char* or char and not cstring.
    // set the structure of the data dictionary--these exact names are always used
    m_sDDStructure = "DataDictionary[TableName:S,FieldType:S,Description:S,ColumnName:S,IsUsed:S,Length:I,Min:S,Max:S,Validate:S,Callbacks:S,DefaultValue:S,FieldEdit:S,CompoundSelection:S,Order1:I,Order2:I,Order3:I,Order4:I,Order5:I,Order6:I,Order7:I,Order8:I,Order9:I,Order10:I,Order11:I,Order12:I]";
#if defined q4_WINCE
    m_sDDViewName = m_sDDStructure.SpanExcluding(TEXT("["));    // pull off the view name
#else    
    m_sDDViewName = m_sDDStructure.SpanExcluding("[");    // pull off the view name
#endif
    m_sDDColNameForTable = "TableName";
    m_sDDColNameForIsUsed = "IsUsed";
    m_sDDColNameForFType = "FieldType";
    m_sDDColNameForColumnName = "ColumnName";
    m_sDDColNameForColHead = "ColumnName";
    m_sDDColNameForDispLen = "Length";
    m_sDDColNameForDefValue = "DefaultValue";
    m_sMsgLogViewName = "MessageLog";
    m_sVersionNumber = "01.00.2003.12.10";
    
    return value;
}//@CODE_140


/*@NOTE_196
Re Open the database.  Generally, when views are added or removed, the db
should be colsed and reopened to update the class info.
*/
int CMk4DataContainer::ReopenDB()
{//@CODE_196
    int value;
    value = ERR_NONE;
    
    CString FileName = m_sFileName;
    OpenDBFile(FileName);
    
    return value;
}//@CODE_196


/*@NOTE_102
Close the active database and clean up anything hanging around. This will close
any open views.
*/
int CMk4DataContainer::CloseDB(bool bCloseFileOnlyLeaveViewsInMemory)
{//@CODE_102
    int value;
    value = 0;
    
    int i;      // loop counter
    
    if (m_iDBStatus == DB_NOT_AVAIL)
    {
        // error if store not open
        value = ERR_DBNOTOPEN;
        return value;
    }
    
    // clean up anything left hanging around
    
    // clean up some file name allocated memory
    // set db status
    if (bCloseFileOnlyLeaveViewsInMemory)
    { // do nothing to file name if leaving around
        // initialize flag to no views, reset below if some remain open
        m_iDBStatus = DB_AVAIL_STORE_CLOSED_NO_VIEWS;
    }

    // close derived views first
    // do not close if derived view is from a memory view
    for (i = 0; i < VGMAXOPEN; i++)
    {
        if (m_iParentIndex[i] >= 0 
            && m_ptrActiveTableName[i] != NULL
            && (m_bViewIsCopyInMemory[m_iParentIndex[i]] && bCloseFileOnlyLeaveViewsInMemory))
                CloseAView(i);
    }
    
    // then close all other views
    // do not close memory views if requested to leave there
    for (i = 0; i < VGMAXOPEN; i++)
    {
        if (m_ptrActiveTableName[i] != NULL)  // there is view in this slot
        {
            if (m_bViewIsCopyInMemory[i] && bCloseFileOnlyLeaveViewsInMemory)
            {  // leave in memory if requested
                m_iDBStatus = DB_AVAIL_STORE_CLOSED_HAS_VIEWS;
            }
            else
            {  // get rid of this view
                CloseAView(i);
            }
        }
    }
    
    // blank out list
    m_sListOfFirstLevelTables.Empty();
    
    if (!bCloseFileOnlyLeaveViewsInMemory)
    { // clean up file name for regular close
        DELETEANDNULLALL(m_ptrFileName);
        m_iDBStatus = DB_NOT_AVAIL;
    }
    // release any memory  for the storage
    DELETEANDNULL(m_ptrDataStore);
    
    return value;
}//@CODE_102


/*@NOTE_629
Will actually open the mk4 storage file.
*/
int CMk4DataContainer::OpenStorage()
{//@CODE_629
    int value=-1;
    
    // release any memory  for the storage
    DELETEANDNULL(m_ptrDataStore);
    
    // open up the database file
    m_ptrDataStore = new c4_Storage(m_ptrFileName, true);

    return value;
}//@CODE_629


// End of Group: DBFileManagement


//
// Group: Data access
//
/*@NOTE_240
The main mk4 access routine which will be called by the GetDataItemAs members.
Has place holders for all possible mk4 data types and flag that indicates which
one has the actual data that has been retrieved from storage. By using this
function, caller does not need to know how the data is stored, and need not
instantiate the c4_Property.
*/
int CMk4DataContainer::GetDataItem(const int ViewNum, const long RowNum,
                                   const int ColNum, char* FieldType,
                                   CString* StrRet, long* LongOrIntRet,
                                   double* DoubleOrFloatRet)
{//@CODE_240
    int value;
    value = 0;  // set return value
    
    CString cs;       // temp string to hold contents
    c4_RowRef r = m_ActiveViewHandle[ViewNum][RowNum];
    c4_Property p = m_ActiveViewHandle[ViewNum].NthProperty(ColNum);
    
    // handle the data in the MK4 storage types
    *FieldType = p.Type();
    switch (*FieldType)
    {
        case 'I':
            *LongOrIntRet = (int)((c4_IntProp&) p)(r);
            break;
        
        case 'L':
            *LongOrIntRet = (long)((c4_LongProp&) p)(r);
            break;
        
        case 'F':
            *DoubleOrFloatRet = (float)((c4_FloatProp&) p)(r);
            break;
        
        case 'D':
            *DoubleOrFloatRet = (double)((c4_DoubleProp&) p)(r);
            break;
        
        case 'S':
            *StrRet = (const char*)((c4_StringProp&) p)(r);
            break;
        
            /*    do not handle memos or byte fields initially, may need to include
            case 'M': // backward compatibility
            case 'B':
            (p(r)).GetData(data);
            sprintf(buff, " (%db)", data.Size());
            break;
            */
        
        default:
            value = 1;  
    }
    
    
    return value;
}//@CODE_240


/*@NOTE_197
Generic way to retrieve a data item from a view. Returns int value 0 if ok, and
>0 for an error number. Since the stored data type is known to Mk4, the
requested data type may imply a conversion; this should also allow dealing with
conversions for Unicode(UCS2). There are multiple member functions with
overrides for all data types available.
*/
int CMk4DataContainer::GetDataItemAs(const int ViewNum, const long RowNum,
                                     const int ColNum, int* RetVal)
{//@CODE_197
    int value;
    
    // these are possible return values from call to get mk4 data
    char FieldType; // the storage type in Mk4-tells which ret value is filled
    // only one of these return variables will have a value
    CString StrRet; // returned if type is string
    long LongOrIntRet; // returned if type is long
    double DoubleOrFloatRet; // returned if type is double
    
    value = 0;    // clear error return value
    // get the typed value from the mk4 table-stored type in FieldType
    GetDataItem(ViewNum, RowNum, ColNum, &FieldType, &StrRet, 
        &LongOrIntRet, &DoubleOrFloatRet);
    // convert the value to the requested type
    switch (FieldType)
    {
        case 'I':
        case 'L':
            *RetVal = LongOrIntRet;
            break;
        
        case 'F':
        case 'D':
            *RetVal = (int)(DoubleOrFloatRet + 0.5);
            break;
        
        case 'S':
            *RetVal = _ttoi(StrRet);
            break;
        
            /*    do not handle memos or byte fields initially, may need to include
            case 'M': // backward compatibility
            case 'B':
            (p(r)).GetData(data);
            sprintf(buff, " (%db)", data.Size());
            break;
            */
        
        default:
            // dont know what to do so return error
            *RetVal = 0;
            value = 1;
    }
    
    
    return value;
}//@CODE_197


/*@NOTE_202
Generic way to retrieve a data item from a view. Returns int value 0 if ok, and
>0 for an error number. Since the stored data type is known to Mk4, the
requested data type may imply a conversion; this should also allow dealing with
conversions for Unicode(UCS2). There are multiple member functions with
overrides for all data types available.
*/
int CMk4DataContainer::GetDataItemAs(const int ViewNum, const long RowNum,
                                     const int ColNum, CString& RetVal)
{//@CODE_202
    int value;
    value = 0; // set the status return value
    
    // these are possible return values from call to get mk4 data
    char FieldType; // the storage type in Mk4-tells which ret value is filled
    // only one of these return variables will have a value
    CString StrRet; // returned if type is string
    long LongOrIntRet; // returned if type is long
    double DoubleOrFloatRet; // returned if type is double
    
    value = 0;    // clear error return value
    
    // get the typed value from the mk4 table-stored type in FieldType
    GetDataItem(ViewNum, RowNum, ColNum, &FieldType, &StrRet, 
        &LongOrIntRet, &DoubleOrFloatRet);
    // convert the value to the requested type
    switch (FieldType)
    {
        case 'I':
        case 'L':
            RetVal.Format(TEXT("%ld"), LongOrIntRet);
            break;
        
        case 'F':
            RetVal.Format(TEXT("%g"), DoubleOrFloatRet);
            break;
        
        case 'D':
            RetVal.Format(TEXT("%.15g"), DoubleOrFloatRet);
            break;
        
        case 'S':
            RetVal = StrRet;
            break;
        
            /*    do not handle memos or byte fields initially, may need to include
            case 'M': // backward compatibility
            case 'B':
            (p(r)).GetData(data);
            sprintf(buff, " (%db)", data.Size());
            break;
            */
        
        default:
            // dont know what to do so return error
            value = 1;
    }
    
    
    return value;
}//@CODE_202


/*@NOTE_207
Generic way to retrieve a data item from a view. Returns int value 0 if ok, and
>0 for an error number. Since the stored data type is known to Mk4, the
requested data type may imply a conversion; this should also allow dealing with
conversions for Unicode(UCS2). There are multiple member functions with
overrides for all data types available.
*/
int CMk4DataContainer::GetDataItemAs(const int ViewNum, const long RowNum,
                                     const int ColNum, float* RetVal)
{//@CODE_207
    int value;
    // these are possible return values from call to get mk4 data
    char FieldType; // the storage type in Mk4-tells which ret value is filled
    // only one of these return variables will have a value
    CString StrRet; // returned if type is string
    long LongOrIntRet; // returned if type is long
    double DoubleOrFloatRet; // returned if type is double
    // TCHAR* stopchar; // character that stops conversion
    
    value = 0;    // clear error return value
    // get the typed value from the mk4 table-stored type in FieldType
    GetDataItem(ViewNum, RowNum, ColNum, &FieldType, &StrRet, 
        &LongOrIntRet, &DoubleOrFloatRet);
    // convert the value to the requested type
    switch (FieldType)
    {
        case 'I':
        case 'L':
            *RetVal = (float)LongOrIntRet;
            break;
        
        case 'F':
        case 'D':
            *RetVal = (float)DoubleOrFloatRet;
            break;
        
        case 'S':
            *RetVal = (float)(_tcstod(StrRet, NULL));
            break;
        
            /*    do not handle memos or byte fields initially, may need to include
            case 'M': // backward compatibility
            case 'B':
            (p(r)).GetData(data);
            sprintf(buff, " (%db)", data.Size());
            break;
            */
        
        default:
            // dont know what to do so return error
            *RetVal = 0;
            value = 1;
    }
    
    
    return value;
}//@CODE_207


/*@NOTE_212
Generic way to retrieve a data item from a view. Returns int value 0 if ok, and
>0 for an error number. Since the stored data type is known to Mk4, the
requested data type may imply a conversion; this should also allow dealing with
conversions for Unicode(UCS2). There are multiple member functions with
overrides for all data types available.
*/
int CMk4DataContainer::GetDataItemAs(const int ViewNum, const long RowNum,
                                     const int ColNum, double* RetVal)
{//@CODE_212
    int value;
    
    // these are possible return values from call to get mk4 data
    char FieldType; // the storage type in Mk4-tells which ret value is filled
    // only one of these return variables will have a value
    CString StrRet; // returned if type is string
    long LongOrIntRet; // returned if type is long
    double DoubleOrFloatRet; // returned if type is double
    // TCHAR* stopchar; // character that stops conversion
    
    value = 0;    // clear error return value
    
    // get the typed value from the mk4 table-stored type in FieldType
    GetDataItem(ViewNum, RowNum, ColNum, &FieldType, &StrRet, 
        &LongOrIntRet, &DoubleOrFloatRet);
    // convert the value to the requested type
    switch (FieldType)
    {
        case 'I':
        case 'L':
            *RetVal = LongOrIntRet;
            break;
        
        case 'F':
        case 'D':
            *RetVal = DoubleOrFloatRet;
            break;
        
        case 'S':
            *RetVal = _tcstod(StrRet, NULL);
            break;
        
            /*    do not handle memos or byte fields initially, may need to include
            case 'M': // backward compatibility
            case 'B':
            (p(r)).GetData(data);
            sprintf(buff, " (%db)", data.Size());
            break;
            */
        
        default:
            // dont know what to do so return error
            *RetVal = 0;
            value = 1;
    }
    
    
    return value;
}//@CODE_212


/*@NOTE_260
Generic way to retrieve a data item from a view. Returns int value 0 if ok, and
>0 for an error number. Since the stored data type is known to Mk4, the
requested data type may imply a conversion; this should also allow dealing with
conversions for Unicode(UCS2). There are multiple member functions with
overrides for all data types available.
*/
int CMk4DataContainer::GetDataItemAs(const int ViewNum, const long RowNum,
                                     const int ColNum, long* RetVal)
{//@CODE_260
    int value;
    
    // these are possible return values from call to get mk4 data
    char FieldType; // the storage type in Mk4-tells which ret value is filled
    // only one of these return variables will have a value
    CString StrRet; // returned if type is string
    long LongOrIntRet; // returned if type is long
    double DoubleOrFloatRet; // returned if type is double
    
    value = 0;    // clear error return value
    // get the typed value from the mk4 table-stored type in FieldType
    GetDataItem(ViewNum, RowNum, ColNum, &FieldType, &StrRet, 
        &LongOrIntRet, &DoubleOrFloatRet);
    // convert the value to the requested type
    switch (FieldType)
    {
        case 'I':
        case 'L':
            *RetVal = LongOrIntRet;
            break;
        
        case 'F':
        case 'D':
            *RetVal = (long)(DoubleOrFloatRet + 0.5);
            break;
        
        case 'S':
            *RetVal = _ttol(StrRet);
            break;
        
            /*    do not handle memos or byte fields initially, may need to include
            case 'M': // backward compatibility
            case 'B':
            (p(r)).GetData(data);
            sprintf(buff, " (%db)", data.Size());
            break;
            */
        
        default:
            // dont know what to do so return error
            *RetVal = 0;
            value = 1;
    }
    
    
    return value;
}//@CODE_260


/*@NOTE_250
The main mk4 access routine which will be called by the SetDataItemAs members.
Has place holders for all possible mk4 data types and flag that indicates which
one has the actual data. By using this function, caller does not need to know
how the data is stored, and need not instantiate the c4_Property.
*/
int CMk4DataContainer::SetDataItem(const int ViewNum, const long RowNum,
                                   const int ColNum, const char DataTypeIn,
                                   const CString StrIn, const int IntIn,
                                   const long LongIn, const float FloatIn,
                                   const double DoubleIn)
{//@CODE_250
    int value;  // return value
    // reset return value
    value = 0;
    
    int RawViewNum = m_iParentIndex[ViewNum];
    if (RawViewNum < 0)
    {
        // this is a raw view, so just call worker to make data save
        value = SetDataItemWorker(ViewNum, RowNum, ColNum, DataTypeIn,
            StrIn, IntIn, LongIn, FloatIn, DoubleIn);
    }
    else 
    {    // this is a derived view so unmap to raw view
        // set the pointers to the specified row and column
        c4_RowRef r = m_ActiveViewHandle[ViewNum][RowNum];
        c4_Property p = m_ActiveViewHandle[ViewNum].NthProperty(ColNum);
        // check if this is a derived view and if so, unmap to parent view
        long iRawRowNum;
        int iRawColNum;
        // find out which row in raw view corresponds to row in derived view
        iRawRowNum = m_ActiveViewHandle[RawViewNum].GetIndexOf(m_ActiveViewHandle[ViewNum][RowNum]);
        // since the derived view may have permuted cols, find corresponding
        CString sTemp = p.Name();    // the name of the property
     
		char* cTemp = NULL;
		StringToChar(sTemp, &cTemp);
 //       char* cTemp = new char[sTemp.GetLength() + 1];
//#if defined q4_WINCE
//        wcstombs(cTemp, sTemp, sTemp.GetLength() + 1);
//#else
 //       strcpy(cTemp, sTemp.operator LPCTSTR());
//#endif
        iRawColNum = m_ActiveViewHandle[RawViewNum].FindPropIndexByName(cTemp);
        DELETEANDNULLALL(cTemp);
        // set the row and col to raw view rather than derived
        // r = m_ActiveViewHandle[RawViewNum][iRawRowNum];
        // p = m_ActiveViewHandle[RawViewNum].NthProperty(iRawColNum);
        value = SetDataItemWorker(RawViewNum, iRawRowNum, iRawColNum, DataTypeIn,
            StrIn, IntIn, LongIn, FloatIn, DoubleIn);
    }   
    
    return value;
}//@CODE_250


/*@NOTE_220
Generic way to set a data item into a view. Returns int value 0 if ok, and >0
for an error number. Since the stored data type is known to Mk4, the requested
data type may imply a conversion; this should also allow dealing with
conversions for Unicode(UCS2). There are multiple member functions with
overrides for all data types available.
*/
int CMk4DataContainer::SetDataItemFrom(const int ViewNum, const long RowNum,
                                       const int ColNum, const int SetVal)
{//@CODE_220
    int value;
    
    // a list of possible values to pass to data access routine
    // set the passed value into the appropriate variable and set the type
    char FieldType = 'I';
    CString StrIn;
    int IntIn = SetVal;
    long LongIn = 0;
    float FloatIn = 0.0;
    double DoubleIn = 0.0;
    
    // set the passed value into the data table
    value = SetDataItem(ViewNum, RowNum, ColNum, FieldType,
        StrIn, IntIn, LongIn, FloatIn, DoubleIn);
    
    return value;
}//@CODE_220


/*@NOTE_225
Generic way to set a data item into a view. Returns int value 0 if ok, and >0
for an error number. Since the stored data type is known to Mk4, the requested
data type may imply a conversion; this should also allow dealing with
conversions for Unicode(UCS2). There are multiple member functions with
overrides for all data types available.
*/
int CMk4DataContainer::SetDataItemFrom(const int ViewNum, const long RowNum,
                                       const int ColNum, const CString& SetVal)
{//@CODE_225
    int value;
    
    // a list of possible values to pass to data access routine
    // set the passed value into the appropriate variable and set the type
    char FieldType = 'S';
    CString StrIn = SetVal;
    int IntIn = 0;
    long LongIn = 0;
    float FloatIn = 0.0;
    double DoubleIn = 0.0;
    
    // set the passed value into the data table
    value = SetDataItem(ViewNum, RowNum, ColNum, FieldType,
        StrIn, IntIn, LongIn, FloatIn, DoubleIn);
    
    return value;
}//@CODE_225


/*@NOTE_230
Generic way to set a data item into a view. Returns int value 0 if ok, and >0
for an error number. Since the stored data type is known to Mk4, the requested
data type may imply a conversion; this should also allow dealing with
conversions for Unicode(UCS2). There are multiple member functions with
overrides for all data types available.
*/
int CMk4DataContainer::SetDataItemFrom(const int ViewNum, const long RowNum,
                                       const int ColNum, const float SetVal)
{//@CODE_230
    int value;
    
    // a list of possible values to pass to data access routine
    // set the passed value into the appropriate variable and set the type
    char FieldType = 'F';
    CString StrIn;
    int IntIn = 0;
    long LongIn = 0;
    float FloatIn = SetVal;
    double DoubleIn = 0.0;
    
    // set the passed value into the data table
    value = SetDataItem(ViewNum, RowNum, ColNum, FieldType,
        StrIn, IntIn, LongIn, FloatIn, DoubleIn);
    
    return value;
}//@CODE_230


/*@NOTE_235
Generic way to set a data item into a view. Returns int value 0 if ok, and >0
for an error number. Since the stored data type is known to Mk4, the requested
data type may imply a conversion; this should also allow dealing with
conversions for Unicode(UCS2). There are multiple member functions with
overrides for all data types available.
*/
int CMk4DataContainer::SetDataItemFrom(const int ViewNum, const long RowNum,
                                       const int ColNum, const double SetVal)
{//@CODE_235
    int value;
    
    // a list of possible values to pass to data access routine
    // set the passed value into the appropriate variable and set the type
    char FieldType = 'D';
    CString StrIn;
    int IntIn = 0;
    long LongIn = 0;
    float FloatIn = 0.0;
    double DoubleIn = SetVal;
    
    // set the passed value into the data table
    value = SetDataItem(ViewNum, RowNum, ColNum, FieldType,
        StrIn, IntIn, LongIn, FloatIn, DoubleIn);
    
    return value;
}//@CODE_235


/*@NOTE_265
Generic way to set a data item into a view. Returns int value 0 if ok, and >0
for an error number. Since the stored data type is known to Mk4, the requested
data type may imply a conversion; this should also allow dealing with
conversions for Unicode(UCS2). There are multiple member functions with
overrides for all data types available.
*/
int CMk4DataContainer::SetDataItemFrom(const int ViewNum, const long RowNum,
                                       const int ColNum, const long SetVal)
{//@CODE_265
    int value;
    
    // a list of possible values to pass to data access routine
    // set the passed value into the appropriate variable and set the type
    char FieldType = 'L';
    CString StrIn;
    int IntIn = 0;
    long LongIn = SetVal;
    float FloatIn = 0.0;
    double DoubleIn = 0.0;
    
    // set the passed value into the data table
    value = SetDataItem(ViewNum, RowNum, ColNum, FieldType,
        StrIn, IntIn, LongIn, FloatIn, DoubleIn);
    
    return value;
}//@CODE_265


/*@NOTE_352
The main mk4 access routine which will be called by the SetDataItemAs members.
Has place holders for all possible mk4 data types and flag that indicates which
one has the actual data.
*/
int CMk4DataContainer::SetDataItemWorker(const int ViewNum, const long RowNum,
                                         const int ColNum,
                                         const char DataTypeIn,
                                         const CString StrIn, const int IntIn,
                                         const long LongIn, const float FloatIn,
                                         const double DoubleIn)
{//@CODE_352
    int value;  // return value
    // some temporary typed variables
    int tempint;
    long templong;
    float tempfloat;
    double tempdouble;
    char* SBCSstring = NULL;
    CString sTemp;
    
    // reset return value
    value = 0;
    SBCSstring = NULL;
    
    // set the pointers to the specified row and column
    c4_RowRef r = m_ActiveViewHandle[ViewNum][RowNum];
    c4_Property p = m_ActiveViewHandle[ViewNum].NthProperty(ColNum);
    char DataTypeStored = p.Type(); // retrieve the stored type
    
    // compare the data type coming in with the data type in the storage
    // if the same, just set it, but if different, convert in to stored type
    switch (DataTypeIn)
    {
        case 'S':
            // TCHAR* stopchar;
            sTemp = StrIn;
            if (m_bDoStringTrim)
            {
                // trim left and right before storing if global flag is set
                sTemp.TrimRight();
                sTemp.TrimLeft();
            }
            switch (DataTypeStored)
            {
            case 'S':
                StringToChar(sTemp, &SBCSstring);
                ((c4_StringProp&) p)(r) = SBCSstring;
                DELETEANDNULLALL(SBCSstring);
                break;
            case 'I':
                tempint = _ttoi(sTemp);
                ((c4_IntProp&) p)(r) = tempint;
                break;
            case 'F':
                tempfloat = (float)(_tcstod(sTemp, NULL));
                ((c4_FloatProp&) p)(r) = tempfloat;
                break;
            case 'L':
                templong = _ttol(sTemp);
                ((c4_LongProp&) p)(r) = templong;
                break;
            case 'D':
                tempdouble = _tcstod(sTemp, NULL);
                ((c4_DoubleProp&) p)(r) = tempdouble;
                break;
            default:
                value = 1;
                return value;
            }
            break;
            case 'I':
                switch (DataTypeStored)
                {
                case 'I':
                    ((c4_IntProp&) p)(r) = IntIn;
                    break;
                case 'S':
                    SBCSstring = (char*) new char[21];
                    sprintf(SBCSstring, "%d", IntIn);
                    ((c4_StringProp&) p)(r) = SBCSstring;
                    DELETEANDNULLALL(SBCSstring);
                    break;
                case 'L':
                    ((c4_LongProp&) p)(r) = IntIn;
                    break;
                case 'F':
                    ((c4_FloatProp&) p)(r) = IntIn;
                    break;
                case 'D':
                    ((c4_DoubleProp&) p)(r) = IntIn;
                    break;
                default:
                    value = 1;
                    return value;
                }
                break;
                case 'L':
                    switch (DataTypeStored)
                    {
                    case 'L':
                        ((c4_LongProp&) p)(r) = LongIn;
                        break;
                    case 'S':
                        SBCSstring = (char*) new char[21];
                        sprintf(SBCSstring, "%ld", LongIn);
                        ((c4_StringProp&) p)(r) = SBCSstring;
                        DELETEANDNULLALL(SBCSstring);
                        break;
                    case 'I':
                        ((c4_IntProp&) p)(r) = LongIn;
                        break;
                    case 'F':
                        ((c4_FloatProp&) p)(r) = LongIn;
                        break;
                    case 'D':
                        ((c4_DoubleProp&) p)(r) = LongIn;
                        break;
                    default:
                        value = 1;
                        return value;
                    }
                    break;
                    case 'F':
                        switch (DataTypeStored)
                        {
                        case 'F':
                            ((c4_FloatProp&) p)(r) = FloatIn;
                            break;
                        case 'S':
                            SBCSstring = (char*) new char[21];
                            sprintf(SBCSstring, "%g", FloatIn);
                            ((c4_StringProp&) p)(r) = SBCSstring;
                            DELETEANDNULLALL(SBCSstring);
                            break;
                        case 'D':
                            ((c4_DoubleProp&) p)(r) = FloatIn;
                            break;
                        case 'I':
                            ((c4_IntProp&) p)(r) = (long)(FloatIn + 0.5);
                            break;
                        case 'L':
                            ((c4_LongProp&) p)(r) = (long)(FloatIn + 0.5);
                            break;
                        default:
                            value = 1;
                            return value;
                        }
                        break;
                        case 'D':
                            switch (DataTypeStored)
                            {
                            case 'D':
                                ((c4_DoubleProp&) p)(r) = DoubleIn;
                                break;
                            case 'S':
                                SBCSstring = (char*) new char[21];
                                sprintf(SBCSstring, "%g", DoubleIn);
                                ((c4_StringProp&) p)(r) = SBCSstring;
                                DELETEANDNULLALL(SBCSstring);
                                break;
                            case 'F':
                                ((c4_FloatProp&) p)(r) = DoubleIn;
                                break;
                            case 'I':
                                ((c4_IntProp&) p)(r) = (long)(DoubleIn + 0.5);
                                break;
                            case 'L':
                                ((c4_LongProp&) p)(r) = (long)(DoubleIn + 0.5);
                                break;
                            default:
                                value = 1;
                                return value;
                            }
                            break;
                            default:
                                value = 2;
                                return value;
    }
    
    
    /*  ************   *************      **************         old code below here
    vvvvvvvvvvvvvv
    // handle the data in the MK4 storage types
    switch (p.Type())
    {
        case 'I':
        // If the sscanf was successful, it will return num fields found
        // only reset value if it found one
        #if defined q4_WINCE
        if (sscanf((const char*)cs.operator LPCTSTR(), "%ld", &tempint) > 0)
        ((c4_IntProp&) p)(r) = tempint;
        #else
        if (sscanf(cs, "%ld", &tempint) > 0)
        ((c4_IntProp&) p)(r) = tempint;
        #endif
        break;
    
          case 'L':
          // If the sscanf was successful, it will return num fields found
          // only reset value if it found one
          #if defined q4_WINCE
          if (sscanf((const char*)cs.operator LPCTSTR(), "%ld", &templong) > 0)
          ((c4_IntProp&) p)(r) = tempint;
          #else
          if (sscanf(cs, "%ld", &templong) > 0)
          ((c4_LongProp&) p)(r) = templong;
          #endif
          break;
      
            case 'F':
            #if defined q4_WINCE
            if (sscanf((const char*)cs.operator LPCTSTR(), "%g", &tempfloat) > 0)
            ((c4_FloatProp&) p)(r) = tempfloat;
            #else
            if (sscanf(cs, "%g", &tempfloat) > 0)
            ((c4_FloatProp&) p)(r) = tempfloat;
            #endif
            break;
        
              case 'D':
              #if defined q4_WINCE
              if (sscanf((const char*)cs.operator LPCTSTR(), "%.12g", &tempdouble) > 0)
              ((c4_DoubleProp&) p)(r) = tempdouble;
              #else
              if (sscanf(cs, "%.12g", &tempdouble) > 0)
              ((c4_DoubleProp&) p)(r) = tempdouble;
              #endif
              break;
          
                case 'S':
                // need to handle unicode conversion of string if ce based
                #ifdef q4_WINCE
                {
                int len = cs.GetLength();
                char* sbcharstring;
                sbcharstring = (char*) new char[len + 1];
                //        wcsrtombs(sbcharstring, cs, len, NULL);
                wcstombs(sbcharstring, cs, len);
                ((c4_StringProp&) p)(r) = sbcharstring;
                delete sbcharstring;
                } // Here is a new one. Must be within a block or else error C2361:  
                //   initialization of 'identifier' is skipped by 'default' label.
                //   http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wcepb40/htm/c2361.asp
            
                  // else, handle as non unicode
                  #else
                  ((c4_StringProp&) p)(r) = cs;
                  #endif
                  break;
              
                    //    do not handle memos or byte fields initially, may need to include
                    // case 'M': // backward compatibility
                    // case 'B':
                    //    (p (r)).GetData(data);
                    //    sprintf(buff, " (%db)", data.Size());
                    //    break;
                    //
                
                      // if none of the above, do not know what to do with it
                      default:
                      return UG_ERROR;
                  }
                  ^^^^^^^^^  
                  *********            ****************              *****************  old code above here */
                  return value;
}//@CODE_352


/*@NOTE_362
Convert from CString to a Char* and deal with Unicode. This could be thought of
as a smart 'new' function since it allocates memory with new. BE SURE TO CALL
CORRESPONDING DELETE.
*/
int CMk4DataContainer::StringToChar(const CString StringToConvert,
                                    char** ptrCharArray)
{//@CODE_362
    int value = 0;
    
    // Note: need to do this probably, but requires initialization of pointer to NULL
    // first check to see if the pointer already points to something, and delete if so
    // if (*ptrCharArray != NULL) delete *ptrCharArray;
    // *ptrCharArray = NULL;
    
    // convert the string into a char; for CE which uses unicode, convert to SBCS
    // NOTE: since GetLength returns the number of bytes, CE may be allocating twice the space needed
    value = StringToConvert.GetLength();
    *ptrCharArray = new char[value + 1];
#if defined q4_WINCE
    value = wcstombs(*ptrCharArray, StringToConvert, value + 1);
#else
    strcpy(*ptrCharArray, StringToConvert.operator LPCTSTR());
#endif
    
    return value;
}//@CODE_362


/*@NOTE_365
Add a row to a mk4 view.  Check if it is a derived view and add to parent if
so. Also check for data dictionary control and fill in defaults. Return the row
number just added. Overloaded second parameter allows row to be inserted,
*/
long CMk4DataContainer::AppendARow(const int ViewNum, long lInsertAt)
{//@CODE_365
    int i;        // looping variable
    c4_Row r;    // create a metakit row to add
    
    // test if this is a derived view and if so, unmap to parent
    int iparentview = m_iParentIndex[ViewNum];    // parent view index
    int iv;    // the view index to append to
    
    // find the row which will be used to base the new row on
    long lPreviousRow;    // the row that the new row based on
    // if insertion specified, set the insertion point
    if (lInsertAt < 0)
    {
        lPreviousRow = m_ActiveViewHandle[ViewNum].GetSize() - 1;
    }
    else 
    {
        lPreviousRow = lInsertAt;
    }
    
    // set the view index to point to a raw view
    if (iparentview < 0)
    {
        // no parent, so a raw view
        iv = ViewNum;
    }
    else 
    {    // has parent, so this is derived view
        iv = iparentview;
        // find out which row in raw view corresponds to row in derived view
        if (lPreviousRow >= 0) 
            lPreviousRow = m_ActiveViewHandle[iv].GetIndexOf(m_ActiveViewHandle[ViewNum][lPreviousRow]);
    }
    
    // add the row to the db, to view if raw or to parent view if derived
    // changed to allow insertion
    // long inumrow = m_ActiveViewHandle[iv].GetSize();
    // m_ActiveViewHandle[iv].InsertAt(inumrow, r);
    // how strange... inserting in the middle does not work... this is workaround
    // there is a known bug that crashes MK when you insert a row in a raw view that has a derived view
    // need to ignore the insert at parameter if a derived view
    long inumrow;
    if (lInsertAt < 0 || iparentview >= 0)
    {
        inumrow = m_ActiveViewHandle[iv].GetSize();
    }
    else 
    {
        inumrow = lPreviousRow + 1;
    }
    m_ActiveViewHandle[iv].InsertAt(inumrow, r);
    
    // if under dd control, and there is a previous row, fill in defaults for row
    // need to use the parent view DD control, not the derived view since not all rows
    if (m_bViewUsesDD[iv])
    {
        CString sTemp;    // a place to hold temporary values
        long lTemp = 0;    // a place to hold a numeric variable for increment
        long lTemp2 = 0;  // for converting a string increment to long
        int icol;    // the column index for this field
        // loop through all fields to find those set with default values
        for (i = 0; i < m_iNumDefFields[iv]; i++)
        {
            // check for default action
            if (m_ptrDefaultValues[iv][i].cMethod != DEFNONE)
            {
                // first determine column index for this field
                // icol = m_ActiveViewHandle[iv].FindPropIndexByName(m_ActiveViewHandle[ViewNum].NthProperty(i).Name());
                icol = i;
                // then change the value
                switch (m_ptrDefaultValues[iv][i].cMethod)
                {
                    case DEFCOPY:
                        if (lPreviousRow >= 0)
                        {
                            GetDataItemAs(iv, lPreviousRow, icol, sTemp);
                            SetDataItemFrom(iv, inumrow, icol, sTemp);
                        }
                        break;
                    case DEFINC:
                        if (lPreviousRow >= 0)
                        {
                            GetDataItemAs(iv, lPreviousRow, icol, &lTemp);
                            if (m_ptrDefaultValues[iv][i].ptrConstant != NULL &&
                                strlen(m_ptrDefaultValues[iv][i].ptrConstant)>0)
                            { 
                                sscanf(m_ptrDefaultValues[iv][i].ptrConstant, "%ld", &lTemp2);
                            }
                            else 
                            {
                                lTemp2 = 1;
                            }
                            lTemp += lTemp2;
                            SetDataItemFrom(iv, inumrow, icol, lTemp);
                        }
                        break;
                    case DEFVALUE:
                        if (m_ptrDefaultValues[iv][i].ptrConstant != NULL)
                            SetDataItemFrom(iv, inumrow, icol, m_ptrDefaultValues[iv][i].ptrConstant);
                        break;
                }
            }
        }
    }
    
    return inumrow; // return the row number just added
}//@CODE_365


/*@NOTE_548
Delete a Row from a View.  The returned value will be the remaining number of
rows.  If Using this with a Grid, you will need to run the following command
when this functions is returned:  SetNumberRows(iValue,TRUE)
*/
long CMk4DataContainer::DeleteARow(const int ViewNum, long nRow)
{//@CODE_548
    long iValue= 0;    // return value
    // test if this is a derived view and if so, unmap to parent
    // TRACE(TEXT("Enter CMK4DataContainer::DeleteARow.\n"));
    int iv;    // the view index to delete from
    long lRowToDelete;
    int iparentview = m_iParentIndex[ViewNum];    // parent view index
    if (iparentview < 0)           // no parent, so a raw view
    {
        iv = ViewNum;
        lRowToDelete = nRow;
    }
    else
    {
        iv = iparentview;
        // find out which row in raw view corresponds to row in derived view
        lRowToDelete = m_ActiveViewHandle[iv].GetIndexOf(m_ActiveViewHandle[ViewNum][nRow]);
    }
    
    int iSize = m_ActiveViewHandle[iv].GetSize();
    if (iSize > 0)
    {
        m_ActiveViewHandle[iv].RemoveAt(lRowToDelete);
        if (iparentview < 0)
            iValue = m_ActiveViewHandle[iv].GetSize();
        else
            iValue = m_ActiveViewHandle[ViewNum].GetSize();
    }
    else
        iValue = 0;
    
    return (iValue);
}//@CODE_548


/*@NOTE_551
Get the index of the named column in the requested view.
*/
int CMk4DataContainer::FindColIndexByName(const int iViewNum, CString sColName)
{//@CODE_551
    int icol;   // the index for the requested column return value
    char * ptrColName = NULL;  // holds the string conversion of col name as a char
    
    StringToChar(sColName, &ptrColName);
    // look up column index
    icol = m_ActiveViewHandle[iViewNum].FindPropIndexByName(ptrColName);
    DELETEANDNULLALL(ptrColName); // clean up allocation
    
    return icol;
}//@CODE_551


/*@NOTE_554
Increment a requested row and column field by 1.
*/
long CMk4DataContainer::IncrementField(int iViewIndex, long lRow, int iCol,
                                       int iIncrement)
{//@CODE_554
    long value;
    
    GetDataItemAs(iViewIndex, lRow, iCol, &value);
    value += iIncrement;
    SetDataItemFrom(iViewIndex, lRow, iCol, value);
    
    return value;
}//@CODE_554


/*@NOTE_564
Validate a field using info in the data dictionary. Returns 0 if valid, and -1
if not.
*/
int CMk4DataContainer::ValidateFieldWithDD(int iViewIndex, long lRow, int iCol,
                                           CString& sFieldName,
                                           CString& sErrMsg)
{//@CODE_564
    int value = 0;
    CString sTemp, sLegalValues;
    double dTemp1, dTemp2;
    // validate data in the raw view, so may need to unmap
    int iRawCol; // row and col in raw view
    long lRawRow; // row and col in raw view
    int iRawView; // raw view index, would be as input if it is not derived

    // make sure dd is attached
    if (!m_bViewUsesDD[iViewIndex])
    {
        sErrMsg = "DD not attached";
        return -4;
    }
    
    // do some initialization
    sFieldName.Empty();
    sErrMsg.Empty();
    sFieldName = m_ActiveViewHandle[iViewIndex].NthProperty(iCol).Name();
    iRawView = m_iParentIndex[iViewIndex];

    // unmap to raw row and column if necessary
    if (iRawView <0)
    { // it is a raw view, so use values passed
        iRawView = iViewIndex;
        iRawCol = iCol;
        lRawRow = lRow;
    }
    else
    { // derived view, so set to unmapped values
        // find out which row in raw view corresponds to row in derived view
        lRawRow = m_ActiveViewHandle[iRawView].GetIndexOf(m_ActiveViewHandle[iViewIndex][lRow]);
        // since the derived view may have permuted cols, find corresponding
		char* cTemp = NULL;
        StringToChar(sFieldName, &cTemp);
        iRawCol = m_ActiveViewHandle[iRawView].FindPropIndexByName(cTemp);
        DELETEANDNULLALL(cTemp);
    }


    // if min value set, check it
    if (m_ptrDefaultValues[iRawView][iRawCol].ptrValidateMin != NULL)
    {
        if (strlen(m_ptrDefaultValues[iRawView][iRawCol].ptrValidateMin) > 0)
        {
            GetDataItemAs(iRawView, lRawRow, iRawCol, &dTemp1);  // get the field value
            dTemp2 = atof(m_ptrDefaultValues[iRawView][iRawCol].ptrValidateMin);
            if (dTemp1 < dTemp2)
            {
                // error if value less than min
                sErrMsg.Format(TEXT("Value R%d, C%d less than %.2f"), lRawRow + 1, iRawCol + 1, dTemp2);
                return -1;
            }
        }
    }
    
    // if max value set, check it
    if (m_ptrDefaultValues[iRawView][iRawCol].ptrValidateMax != NULL)
    {
        if (strlen(m_ptrDefaultValues[iRawView][iRawCol].ptrValidateMax) > 0)
        {
            GetDataItemAs(iRawView, lRawRow, iRawCol, &dTemp1);  // get the field value
            dTemp2 = atof(m_ptrDefaultValues[iRawView][iRawCol].ptrValidateMax);
            if (dTemp1 > dTemp2)
            {
                // error if value greater than max
                sErrMsg.Format(TEXT("Value R%d, C%d greater than %.2f"), lRawRow + 1, iRawCol + 1, dTemp2);
                return -2;
            }
        }
    }
    
    // if list is set, check it
    if (m_ptrDefaultValues[iRawView][iRawCol].ptrValidateList != NULL)
    {
        // handle a list specified as a table column
        if (m_ptrDefaultValues[iRawView][iRawCol].ptrValidateList[0] == '[')
        {
            // handle table list values here
        }
        else 
        {  // handle hard coded dd lists here
            GetDataItemAs(iRawView, lRawRow, iRawCol, sTemp);  // get the field value
            sLegalValues = m_ptrDefaultValues[iRawView][iRawCol].ptrValidateList;
            if (sTemp.GetLength() == 0 && sLegalValues.Find(TEXT("~")) >= 0)
            {
                // a zero length string with a ~ list element is ok, so no further check
            }
            else if ((sTemp.GetLength() == 0) || (sLegalValues.Find(sTemp) < 0))
            {
                // not found in list
                sErrMsg.Format(TEXT("Value <%s> in R%d, C%d not in list"), sTemp, lRawRow + 1, iRawCol + 1);
                return -3;
            }
        }
    }
   
    return value;
}//@CODE_564


/*@NOTE_570
Validate all fields in a row using info in the data dictionary. Loops through
all fields and makes repeated calls to validate field function. Returns 0 if
valid, and -1 if not. Stops at first field on row that is in error and fills in
the field number.
*/
int CMk4DataContainer::ValidateRowWithDD(int iViewIndex, long lRow, int* iCol,
                                         CString& sFieldName, CString& sErrMsg)
{//@CODE_570
    int value = 0;
    int i;  // loop var
    
    for (i = 0; i < m_iNumDefFields[iViewIndex]; i++)
    {
        value = ValidateFieldWithDD(iViewIndex, lRow, i, sFieldName, sErrMsg);
        if (value != 0)
        {
            // if test fails, pass back error code and field number
            *iCol = i;
            return value;
        }
    }
    
    return value;
}//@CODE_570


/*@NOTE_578
Validate all fields in a view using info in the data dictionary. Loops through
all rows and calls the validate row function. Returns 0 if valid, and -1 if
not. Stops at first error and fills in the row and field number.
*/
int CMk4DataContainer::ValidateViewWithDD(int iViewIndex, long* lRow, int* iCol,
                                          CString& sFieldName, CString& sErrMsg)
{//@CODE_578
    int value = 0;
    long l;  // loop var
    
    long lCount = m_ActiveViewHandle[iViewIndex].GetSize();  // number of rows
    
    for (l = 0; l < lCount; l++)
    {
        value = ValidateRowWithDD(iViewIndex, l, iCol, sFieldName, sErrMsg);
        if (value != 0)
        {
            // if test fails, pass back error code and field number
            *lRow = l;
            return value;
        }
    }
    
    return value;
}//@CODE_578


/*@NOTE_648
For data stored in an .ini style table (ie. Block, Key, Value), allows you to
specify the block and key, and to get/set the associated value.
*/
int CMk4DataContainer::KeyValueAccess(CString sViewName,
                                      CString sSelectionCriteria,
                                      CString sValueFieldName, CString& sValue,
                                      BOOL bSetValue)
{//@CODE_648

    // look for the value in the view
    int iView=-1, iDView=-1;  // indexes of view
    bool bNeedToClose = FALSE;  // prog flag that is set when we need to close
    int iRet=0; // return value

    iView = FindViewIndexByName(sViewName); // try to find it if open
    if (iView < 0)
    { // is not already open
        iView = OpenAView(sViewName); // try to open view
        if (iView < 0) return -1; // not available so error
        bNeedToClose = TRUE; // since opened here, need to close at end
    }

    // look up values using a derived view member
    iDView = DeriveView(sViewName, sValueFieldName,
        sSelectionCriteria, "", "_KeyRetrieve_");
    if (iDView < 0)
    {  // did not open
        iRet = -2;
    }
    else if (m_ActiveViewHandle[iDView].GetSize() == 1)
    {  // ok, there is only one value, so get/set it
        if (bSetValue)
            SetDataItemFrom(iDView, 0,0, sValue);
        else
            GetDataItemAs(iDView, 0,0, sValue);
    }
    else
    {  // not exactly one value, so may need to handle as csv string
        iRet = -3;
    }
    
    // clean up
    if (bNeedToClose) CloseAView(iView);
    if (iDView >=0) CloseAView(iDView);

    return iRet;
}//@CODE_648


// End of Group: Data access


//
// Group: ViewManagement
//
/*@NOTE_100
Close a view that was previously opened.  The passed parameter is the index
into the class member array of views of a named table.
*/
int CMk4DataContainer::CloseAView(int iToClose)
{//@CODE_100
    int value;
    
    // TRACE("\n***\nEntering CloseAView for %d\n", iToClose);
    
    value = 0;      // set return to ok
    
    if (m_iDBStatus == DB_NOT_AVAIL)
    {
        // error if store not open
        value = ERR_DBNOTOPEN;
#ifndef q4_WINCE
        TRACE("Closing View with db not open");
#endif
        return value;
    }
    
    // check for a legit view number
    if ((iToClose < 0) || (iToClose > VGMAXOPEN))
    {
        TRACE(TEXT("Error in Close a View, view index out of range\n"));
        value = 2;
        return value;
    }
    
    // first check to see that it is indeed in use, return error
    if (m_ptrActiveTableName[iToClose] == NULL)
    {
        TRACE(TEXT("Error in Close a View, **1**\n"));
        value = 1;
        return value;
    }
    
    // TRACE("Closing %s view.\n", m_ptrActiveTableName[iToClose]);
    
    // clean up attached DD by calling detach member
    if (m_bViewUsesDD[iToClose] == TRUE)
    {
        DetachDDFromView(iToClose);    // clean up attached DD
    };
    
    // if this is a copy in memory, then be sure to copy it back to file
    if (m_bViewIsCopyInMemory[iToClose])
    {
        SaveData();
    }
    
    // clean up mem for table struct string
    DELETEANDNULLALL(m_ptrActiveTableStructure[iToClose]);
    
    // clean up mem for parent table name string
    DELETEANDNULLALL(m_ptrParentTableName[iToClose]);
    
    // clean up mem for table name string
    DELETEANDNULLALL(m_ptrActiveTableName[iToClose]);
    
    // reset the derived flag
    m_iParentIndex[iToClose] = -1;
    
    // get rid of the view
    // This does not seem to be necessary
    m_ActiveViewHandle[iToClose] = c4_View();
    m_bViewIsCopyInMemory[iToClose] = FALSE;
    
    return value;
}//@CODE_100


/*@NOTE_146
Close a view by name.
*/
int CMk4DataContainer::CloseAView(const char* ptrAViewName)
{//@CODE_146
    int value;
    if (m_ptrDataStore ==NULL)
        return -1;    // error if store not open
    value = CloseAView(FindViewIndexByName(ptrAViewName));
    return value;
}//@CODE_146


/*@NOTE_190
Delete a view from an MK4 storage. Since I couldn't find a function in MetaKit
that deletes a view, created this one.
*/
int CMk4DataContainer::DeleteView(const char* ptrAViewName)
{//@CODE_190
    int value;
    CString sOldstructure;
    CString sNewstructure;
    CString t1;    // working area
    TCHAR prev, trail;    // char before and after view structure
    int begin, end;    // the beginning and end of the view def in structure
    
    value = ERR_NONE;
    if (m_iDBStatus == DB_NOT_AVAIL)
    {
        // error if store not open
        value = ERR_DBNOTOPEN;
        TRACE(TEXT("Deleting View, db not open\n"));
        return value;
    }
    sOldstructure = m_ptrDataStore->Description();    // get current structure
    sNewstructure = sOldstructure;    // work area to make sNewstructure
    
    // can find partial, so add [ to string
    t1 = ptrAViewName;
    t1 += "[";
    //begin = sNewstructure.Find((LPCTSTR)ptrAViewName);    // find the view in structure
    begin = sNewstructure.Find(t1);    // find the view in structure
    
    if (begin < 0)
    {
        return -1;
    }
    end = sNewstructure.Find(']', begin);    // find the end of this view
    t1 = sNewstructure.Mid(begin, end - begin + 1); // extract out full view spec
    if (begin == 0)
        prev = ' ';    // prepare to check if leading comma
    else 
        prev = sNewstructure.GetAt(begin - 1);
    if (end == sNewstructure.GetLength() - 1)
        trail = ' '; // prepare to check trailing
    else 
        trail = sNewstructure.GetAt(end + 1);
    if (trail == ',')
        t1 += ',';    // if trailing comma, remove it
    else if (prev == ',')
        t1.Insert(0, ',');    // else if leading, remove it
    
#if defined q4_WINCE
    char* cTemp = NULL;
    StringToChar(t1, &cTemp);
    sNewstructure.Replace((unsigned short)cTemp, (unsigned short)TEXT(""));
    DELETEANDNULLALL(cTemp);
    //    sNewstructure.Replace((unsigned short)t1.operator LPCTSTR(), (unsigned short)_T(""));    // do actual removal
#else
    sNewstructure.Replace(t1, "");    // do actual removal
#endif
    
    // now reset the structure
#if defined q4_WINCE
    cTemp = new char[sNewstructure.GetLength() + 1];
    wcstombs(cTemp, sNewstructure, sNewstructure.GetLength() + 1);
    m_ptrDataStore->SetStructure(cTemp);
    DELETEANDNULLALL(cTemp);
#else
    m_ptrDataStore->SetStructure(sNewstructure);
#endif
    m_ptrDataStore->Commit();
    BuildListOfViews();
    
    return value;
}//@CODE_190


/*@NOTE_152
Export a metakit table to another format. The argument FileType specifies the
type, (see the defines in the header file). 

DATATYPE_MK4 1 for csv with mk4 header

DATATYPE_CSVWHDR 2 for csv with first row the col names seperated by comma

DATATYPE_CSVNOHDR 3 for just the data and nothing about the headers

DATATYPE_DBF 4 for dBase file format (not yet implemented

Other formats defined as needed
*/
int CMk4DataContainer::ExportView(const int FileType, const CString FileName,
                                  const char* TableName, const int Mode)
{//@CODE_152
    int value = 0;  // the return value
#ifndef EXCLUDEIMPORTEXPORT
    int ViewIndex;  // the index of the view in the array of open views
    long numrows, numcols;  // the number of rows and columns in the data table for looping
    int i, j;  // looping vars
    bool NeedToClose = FALSE;   // tracking var to see if we need to close the view
    FILE* pfile = NULL;    // handle for text file output
    
    // for MS Access file handling
    CDaoDatabase OutputDatabase;
    CDaoRecordset OutputRecordSet(&OutputDatabase);
    CString sSqlCmd;
    CString sAttribute;  // holds col or attribute name for xml out
    char cFieldType;
    CString sFieldName, sFieldValue;
    CMarkup xml;  // for generating xml output
    CString sStructure;  // structure for ouput in view xml element
    CString sDefaultMarkup;  // to contain basic xml skeleton for new files
    // build the string that has template for skeleton xml for new files
    sDefaultMarkup = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" standalone=\"yes\"?>\n";
    sDefaultMarkup += "<mk4export\n version=\"1.0\"\n";
#ifdef FSCRUISER
    sDefaultMarkup += " creator=\"USFS FScruiser - http://www.fs.fed.us/fmsc/measure/\"\n";
#else
    sDefaultMarkup += " creator=\"USFS MK4 Editor - http://www.fs.fed.us/fmsc/measure/\"\n";
#endif
    // this stuff is the schema for gpx format
    // sDefaultMarkup += " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n";
    // sDefaultMarkup += " xmlns=\"http://www.topografix.com/GPX/1/0\"\n";
    // sDefaultMarkup += " xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd http://www.topografix.com/GPX/Private/TopoGrafix/0/2 http://www.topografix.com/GPX/Private/TopoGrafix/0/2/topografix.xsd\"";
    sDefaultMarkup += ">\n";
    sDefaultMarkup += "</mk4export>";
    
    if (m_iDBStatus == DB_NOT_AVAIL)
    {
        // error if store not open
        value = ERR_DBNOTOPEN;
        TRACE("Exporting View", value);
        return value;
    }
    
    // open the view if not already open
    if (!ViewIsOpen(TableName, &ViewIndex))
    {
        ViewIndex = OpenAView(TableName);
        NeedToClose = TRUE; // set flag so we can close view after we use it
    }
    
    // ****** 1 ****** Open output file
    // open the file, append or overwrite as needed
    if (FileType == DATATYPE_MSACCESS)
    {
        // Access file, so open db
        if (FileExists(FileName) == FILE_DOES_EXIST)
        {
            OutputDatabase.Open(FileName);
        }
        else 
        {
            OutputDatabase.Create(FileName);
        }
    }
    else if (FileType == DATATYPE_XML)
    {
        if (FileExists(FileName) == FILE_DOES_EXIST)
        {
            xml.Load(FileName);
        }
        else 
        {
            xml.SetDoc(sDefaultMarkup);
        }
    }
    else
    {  // text file, so open for write
        switch (Mode)
        {
            case DATAMODE_APPEND :        
                pfile = fopen((const char*)FileName.operator LPCTSTR(), "a");            
                break;
            case DATAMODE_REPLACE :
                pfile = fopen((const char*)FileName.operator LPCTSTR(), "w");            
                break;
        }
    }
    
    // ****** 2 ****** Process headers
    // get the number of rows and columns
    numrows = m_ActiveViewHandle[ViewIndex].GetSize();
    numcols = m_ActiveViewHandle[ViewIndex].NumProperties();
    // then write out mk4 structure syntax
    CString sDescription;  // string to get description from container
    DescribeStructure(sDescription, ViewIndex);
    // process the headers
    switch (FileType)
    {
        case DATATYPE_MK4 :
            // write out the block delimiter
            fprintf(pfile, "%c%s%c\n", '[', m_ptrActiveTableName[ViewIndex], ']');
            // then write out mk4 structure syntax
            // fprintf(pfile, "%s\n", m_ptrActiveTableStructure[ViewIndex]);
            // try new member to get description for derived view
            fprintf(pfile, "%s\n", sDescription);
            // do not break, so we also write out comma values too
        case DATATYPE_CSVWHDR :
            // write out titles on line as csv
            for (i = 0; i < numcols; i++)
            {
                if (i > 0)
                    fprintf(pfile, ",");
                fprintf(pfile, "%s", m_ActiveViewHandle[ViewIndex].NthProperty(i).Name());
            }
            fprintf(pfile, "\n");
            break;
        case DATATYPE_CSVNOHDR :
            break;
        case DATATYPE_XML :
            // create table (view) block
            xml.FindElem("mk4export");
            xml.AddChildElem("view");
            xml.IntoElem();
            xml.AddChildElem("name", TableName);
            DescribeStructure(sStructure, ViewIndex);
            xml.AddChildElem("mk4structure", sStructure);
            DescribeStructure(sStructure, ViewIndex, TRUE);
            xml.AddChildElem("attributes", sStructure);
            break;
        case DATATYPE_MSACCESS :
            // create sql statement that creates table
            sSqlCmd = "CREATE TABLE ";
            sSqlCmd += TableName;
            sSqlCmd += " (";
            for (i = 0; i < numcols; i++)
            {
                if (i > 0)
                    sSqlCmd += ", ";
                sFieldName = m_ActiveViewHandle[ViewIndex].NthProperty(i).Name();
                sFieldName.Replace("/", "_D_");
                sFieldName.Replace("%", "__P_");
                sFieldName.Insert(0, "MK4_");
                sSqlCmd += sFieldName;
                sSqlCmd += " ";
                cFieldType = m_ActiveViewHandle[ViewIndex].NthProperty(i).Type();
                switch (cFieldType)
                {
                    // for any integers
                case 'I':
                case 'L':
                    sSqlCmd += "INTEGER";
                    break;
                    // for any real types
                case 'F':
                case 'D':
                    sSqlCmd += "DOUBLE";
                    break;
                    // for anything else, use string
                default:
                    sSqlCmd += "VARCHAR(255)";
                    break;
                }
            }
            sSqlCmd += ");";
            // TRACE("SQL command is:<%s>", sSqlCmd);
            OutputDatabase.Execute(sSqlCmd);
            break;
    }
    
    // ****** 3 ****** Write rows of data
    // only try to access if there is a valid view
    if (ViewIndex >= 0)
    {
        // if MS Access, open table
        if (FileType == DATATYPE_MSACCESS)
        {
            sSqlCmd = "SELECT * FROM ";
            sSqlCmd += TableName;
            OutputRecordSet.Open(AFX_DAO_USE_DEFAULT_TYPE, sSqlCmd, 0);
        }
        // loop through the rows
        for (i = 0; i < numrows; i++)
        {
            // if MS Access, create record
            if (FileType == DATATYPE_MSACCESS)
            {
                OutputRecordSet.AddNew();
            }
            // if XML, create new record and go into it
            if (FileType == DATATYPE_XML)
            {
                xml.AddChildElem("rec");
                xml.IntoElem();
            }
            // loop through the cols
            for (j = 0; j < numcols; j++)
            {
                // changed to avoid case statement
                // use the data container to get value as a string
                GetDataItemAs(ViewIndex, i, j, sFieldValue);
                // since we are writting a comma delimited file, commas within field cause problems
                // replace the commas with tildas
                sFieldValue.Replace(_T(','), _T('^'));
                sAttribute = m_ActiveViewHandle[ViewIndex].NthProperty(j).Name();
                // now write out the data field to the file
                switch (FileType)
                {
                    case DATATYPE_CSVNOHDR :
                    case DATATYPE_MK4 :
                    case DATATYPE_CSVWHDR :
                        if (j > 0)
                        {
                            fprintf(pfile, ",");  // write out the comma delimiter
                        }
                    
                        // fprintf(pfile, "\""); // write out the beginning quote
                        fprintf(pfile, "%s", sFieldValue); // the actual field
                        // fprintf(pfile, "\""); // write out the trailing quote
                        break;
                    case DATATYPE_MSACCESS:
                        sFieldName = m_ActiveViewHandle[ViewIndex].NthProperty(j).Name();
                        sFieldName.Replace("/", "_D_");
                        sFieldName.Replace("%", "__P_");
                        sFieldName.Insert(0, "MK4_");
                        OutputRecordSet.SetFieldValue((LPCSTR)sFieldName, (LPCSTR)sFieldValue);
                        break;
                    case DATATYPE_XML:
                        // add the value as attribute pair
                        //////////////// mjo added here down to deal with attributes that start with a number.
                        CString sFirstChar;
                        sFirstChar = sAttribute.GetAt(0);
                        if (sFirstChar.FindOneOf("0123456789") > -1)
                        {
                            sAttribute.Insert(0, "_");
                        }
                    
                        //////////////// mjo added here up
                        xml.AddAttrib(sAttribute, sFieldValue);
                        break;
                }
            } // end of j loop
            // now write out the end of line or update record
            switch (FileType)
            {
                case DATATYPE_CSVNOHDR :
                case DATATYPE_MK4 :
                case DATATYPE_CSVWHDR :
                    fprintf(pfile, "\n"); // write out the end of line
                    break;
                case DATATYPE_MSACCESS:
                    OutputRecordSet.Update();
                    break;
                case DATATYPE_XML:
                    xml.OutOfElem();  // get out of record
                    break;
            }
        }     // end of i loop
        xml.OutOfElem();  // get out of view
    } // end of if valid file
    
    // close table if opened here
    if (NeedToClose)
        CloseAView(TableName);
    // close the file if not an access file
    switch (FileType)
    {
        case DATATYPE_CSVNOHDR :
        case DATATYPE_MK4 :
        case DATATYPE_CSVWHDR :
            fclose(pfile);
            break;
        case DATATYPE_MSACCESS:
            break;
        case DATATYPE_XML:
            xml.Save(FileName);
            break;
    }
    
#endif
    
    return value;
}//@CODE_152


/*@NOTE_115
Search through the open views by name and return the view reference to that
view.
*/
c4_View& CMk4DataContainer::GetViewByName(const char* ptrAViewName, int* iErr)
{//@CODE_115
    int i;            // looping or temp  variable
    
    *iErr = 0;      // set flag to no error
    
                    /*  replace code with call to new function
                    // find the view if it has been opened
                    for (i = 0; i < VGMAXOPEN; i++)
                    {
                    if (m_ptrActiveTableName[i] != NULL)
                    {
                    if (strcmp(m_ptrActiveTableName[i], ptrAViewName) == 0)
                    {
                    return m_ActiveViewHandle[i];
                    }
                    }
                    }
    */
    
    if (m_iDBStatus == DB_NOT_AVAIL)
    {
        // error if store not open
        *iErr = ERR_DBNOTOPEN;      // set flag to error
        int value = *iErr;
#ifndef q4_WINCE
        PopMsg("Getting View by Name", value);
#endif
        return m_ActiveViewHandle[0];
    }
    
    if (ViewIsOpen(ptrAViewName, &i))
        return m_ActiveViewHandle[i];
    
    *iErr = 1;      // set flag to error
    return m_ActiveViewHandle[0];
}//@CODE_115


/*@NOTE_118
Search through the open views by name and return the index of  that view.
*/
int CMk4DataContainer::FindViewIndexByName(const char* ptrAViewName)
{//@CODE_118
    int i;        // looping variable
    
                  /* replace code with call to function
                  // find the view if it has been opened
                  for (i = 0; i < VGMAXOPEN; i++)
                  {
                  if (m_ptrActiveTableName[i] != NULL)
                  {
                  if (strcmp(m_ptrActiveTableName[i], ptrAViewName) == 0)
                  {
                  return i;
                  }
                  }
                  }
    */
    
    if (ViewIsOpen(ptrAViewName, &i))
        return i;
    return -1;
}//@CODE_118


/*@NOTE_165
Request for a free index from view list so that caller can open a view in a way
other than the default, 'as is', way.  Caller supplies the name to be used for
the view. Close in the regular way.
*/
int CMk4DataContainer::FindFreeViewIndex(const char* ptrAViewName)
{//@CODE_165
    int value;    // return variable
    int i;  // loop variable
    
    value = -1;
    
    if (m_iDBStatus == DB_NOT_AVAIL)
    {
        // error if store not open
        value = ERR_DBNOTOPEN;
        PopMsg("In Get View Index", value);
        return value;
    }
    // Find an open slot in the array to stick this
    for (i = 0; i < VGMAXOPEN; i++)
    {
        if (m_ptrActiveTableName[i] == NULL)
        {
            // found a free slot, save the table name
            m_ptrActiveTableName[i] = (char*) new char[(strlen(ptrAViewName) +1)];
            strcpy(m_ptrActiveTableName[i], ptrAViewName);
            
            // then set up for return
            value = i;  // set up return value of the index being used
            
            break;          // since we completed processing, just break out of for loop
        } // end of process when empty slot found
    }   // end of for loop
    
    return value;
}//@CODE_165


/*@NOTE_155
Import an external data source into a metakit table. The argument FileType
specifies the type, (see the defines in the header file). 

DATATYPE_MK4 1 for csv with mk4 header

DATATYPE_CSVWHDR 2 for csv with first row the col names seperated by comma

DATATYPE_CSVNOHDR 3 for just the data and nothing about the headers

DATATYPE_DBF 4 for dBase file format(not yet implemented

Other formats defined as needed
*/
int CMk4DataContainer::ImportView(int FileType, CString FileName,
                                  const char* TableName, int Mode)
{//@CODE_155
    int value = 0;  // the return value
    int ViewIndex = -1;  // the index of the view in the array of open views
    long numcols;  // the number of rows and columns in the data table for looping
    int iCol;  // looping var over columns
    int iRow;   // the index of a row, usually just added
    bool NeedToClose = FALSE;   // tracking var to see if we need to close the view
    FILE* pfile = NULL;    // handle for text file input
    CString sLine;      // a line read from the file
    CString sToken;      // temp store to hold the next token
//    TCHAR cDelimiter = _T(',');
    bool IsNewTable = FALSE;    // flag indicates a new table, no structure test
    bool bKeepLooking = TRUE;    // flag used in mk4 load for searching
	CString sTableName = TableName;

	char* cFileName = NULL;
	StringToChar(FileName, &cFileName);
	    
    if (m_iDBStatus == DB_NOT_AVAIL)
    {
        // error if store not open
        value = ERR_DBNOTOPEN;
        LogMessage(value, "W", "Importing View");
        goto finishup;
    }

    // ****** 1 ****** Open import file
    if (FileType == DATATYPE_MSACCESS)
    {
        // open access file
    }
    else 
    {  // open text file
        pfile = fopen(cFileName, "r");
        if (pfile == NULL)
        {
            LogMessage("Can not open import file ", "W", "");
            value = -1;
            goto finishup;
        };
    }

    // ****** 2 ****** get the headers or structure info
    switch (FileType)
    {
        case DATATYPE_CSVWHDR :     // handle the csv headers, only for existing tables
            ReadLine(pfile, sLine);
            break;
        case DATATYPE_MK4 :     // handle mk4 syntax to avoid any interaction
            // Look for this table name block in the file
            ReadLine(pfile, sLine);
            while (bKeepLooking) 
            {
                if (sLine.GetLength() > 0 && sLine[0] == '[')
                {
                    // if this is a block and the right table name
                   // if (sLine.Find(TableName) >= 0)
                    if (sLine.Find(sTableName) >= 0)
                        bKeepLooking = FALSE;
                }
                if (feof(pfile))
                {
                    // obviously not in this file, so error
                    LogMessage("Can not find table ", "W", "");
                    value = -1;
                    goto finishup;
                }
                ReadLine(pfile, sLine);  // read the mk4 header
                // strip off any trailing commas put in by spreadsheet
                iCol = sLine.Find(']') + 1;
                if (sLine.GetLength() > iCol)
                    sLine.Delete(iCol, (sLine.GetLength() - iCol));
            }
            // sLine should have the mk4 header in it for named table now
            ReadLine(pfile, sToken);    // just for effect to get rid of col names
        
            break;
        case DATATYPE_CSVNOHDR :
            break;
        case DATATYPE_MSACCESS :
            break;
    }
    
    // ****** 3 ****** prepare mk4 view to accept the data
    // open the view if not already open
    if (!ViewIsOpen(TableName, &ViewIndex))
    {
        // check to see if the table name is instantiated, and open if it is
        ViewIndex = OpenAView(TableName);
        NeedToClose = TRUE; // set flag so we can close view after we use it
    };
    
    // Check again to see if view was instantiated and was opened, -1 does not exist
    // if not open, and if datatype_mk4, ie. with mk4 data structure, create it
    if (ViewIndex < 0 && FileType == DATATYPE_MK4)
    {
        ViewIndex = FindFreeViewIndex(TableName);
        char* ptr_cTemp = NULL;
        StringToChar(sLine, &ptr_cTemp);
        m_ActiveViewHandle[ViewIndex] = m_ptrDataStore->GetAs(ptr_cTemp);
        DELETEANDNULLALL(ptr_cTemp);
        IsNewTable = TRUE;
        NeedToClose = TRUE; // set flag so we can close view after we use it
    }
    else if (ViewIndex <0 && FileType == DATATYPE_MSACCESS)
    {
        // build view structure from access file and open a view with it
        IsNewTable = TRUE;
        NeedToClose = TRUE; // set flag so we can close view after we use it
    }
    
    // check to make sure the structures match
    if (ViewIndex >=0  && !IsNewTable)
    {
        if (FileType == DATATYPE_MK4)
        {
            // if it is an mk4 structure coming in, compare it
            CString sTest = m_ptrActiveTableStructure[ViewIndex];
            if (sLine.CompareNoCase(sTest) != 0)
            {
                LogMessage("Different structure for import", "W", "");
                value = -1;
                goto finishup;
            }
        }
        else if (FileType == DATATYPE_CSVWHDR)
        {
            // if a csv header, best you can do is count items and hope
        }
    }
    
    // ****** 4 ****** empty view if replace mode
    // if append mode, do nothing, but if replace mode, clear the view
    if (ViewIndex >=0)
    {
        switch (Mode)
        {
            case DATAMODE_APPEND :        
                break;
            case DATAMODE_REPLACE :
                m_ActiveViewHandle[ViewIndex].SetSize(0, - 1);
                break;
        }
    }
    
    // ****** 5 ****** loop through records and put in table
    // only try to access if there is a valid view
    if (ViewIndex >= 0)
    {
        c4_Row rrRow;
        numcols = m_ActiveViewHandle[ViewIndex].NumProperties();
        // loop through the rows
        ReadLine(pfile, sLine); // prime the pump and get first row of data
        // read lines till end of file, or beginning of next block
        while (sLine.GetLength() == 0 || (!feof(pfile) && sLine[0] != '[')) 
        {   
            // skip empty lines, only process lines with something
            if (sLine.GetLength() > 0)
            {
                // now save the data to the view
                iRow = m_ActiveViewHandle[ViewIndex].Add(rrRow);   // add a line to the view
                switch (FileType)
                {
                    case DATATYPE_CSVNOHDR :
                    case DATATYPE_MK4 :
                    case DATATYPE_CSVWHDR :
                        // loop through the columns and set values for this row
                        for (iCol = 0; iCol < numcols; iCol++)
                        {
                            NextToken(sLine, sToken);
                            // since we are reading a comma delimited file, commas within field cause problems
                            // undo conversion done in export and replace the carets with commas
                            sToken.Replace(_T('^'), _T(','));
                            SetDataItemFrom(ViewIndex, iRow, iCol, sToken);  // set the value
                        }
                        break;
                    case DATATYPE_MSACCESS :
                        break;
                }
            }
            ReadLine(pfile, sLine);
        }
    } // end of if valid view
    
finishup:
    // close table if opened here
    if (NeedToClose)
        CloseAView(TableName);
    // close the file
    if (FileType == DATATYPE_MSACCESS)
    {
        // destructor should close mdb file automatically
    }
    else 
    {
        if (pfile != NULL)
            fclose(pfile);
    }

	DELETEANDNULLALL(cFileName);

    return value;
}//@CODE_155


/*@NOTE_103
Opens a given named view in the data store.  This opens the MK4 table with the
structure that is in the file.
*/
int CMk4DataContainer::OpenAView(const char* ptrAViewName,
                                 bool bOpenViewInMemory)
{//@CODE_103
    int value = -1;  // the return value which is the array index, set to error return
    int i;      // loop and counting temp var
    // char temp[VGMAXBUF];        // buffer to compose a structure for GetAs function
    CString sTemp;
    CString Note = TEXT("In OpenAView, opening ");  // compose note for message log if needed
    bool bNeedToCloseFile = FALSE; // if file is not open, remember to close it
    char* cTemp = NULL; // temporary character pointer
    
    Note = Note + ptrAViewName;
    
    // *** *** *** ***
    // check to see that mk4 file is open
    if (m_iDBStatus == DB_NOT_AVAIL)
    {
        if (m_ptrFileName != NULL && bOpenViewInMemory)
        {  // file is there, but has been closed, so open it
            m_ptrDataStore = new c4_Storage(m_ptrFileName, true);
            bNeedToCloseFile = TRUE;
        }
        else
        {  // error if store not open
            value = ERR_DBNOTOPEN;
            //LogMessage(TEXT("MK4 file not open"), TEXT("W"), Note);
            TRACE(TEXT("MK4 file not open") + Note);
            return value;
        }
    }
    
    // *** *** *** ***
    // Make sure this table not already open, if it is close and reopen below
    // changed logic to close and reopen
    // if (ViewIsOpen(ptrAViewName, &value)) return value;
    // check for the actual name if regular, or altered if memory copy

    // close the view if open
    if (ViewIsOpen(ptrAViewName, &value))
    {
        CloseAView(ptrAViewName);   // close and reopen below, may change index in array
    }
    
    // *** *** *** ***
    // check to see if this view is instantiated in the db
    if (DoesViewExist(ptrAViewName) < 0)
    {
        // did not find in list
        //LogMessage(TEXT("View not instantiated"), TEXT("W"), Note);
        TRACE(TEXT("View not instantiated") + Note);
        return value;
    }
    /* does not work if a view has name that has subpart equal to view name
    sTemp = ptrAViewName;
    if (m_sListOfFirstLevelTables.Find(sTemp, 0) < 0)
    {
        // did not find in list
        //LogMessage(TEXT("View not instantiated"), TEXT("W"), Note);
        TRACE(TEXT("View not instantiated") + Note);
        return value;
    }
    */
    
    // *** *** *** ***
    // Find an open slot in the array to stick this
    for (i = 0; i < VGMAXOPEN; i++)
    {
        if (m_ptrActiveTableName[i] == NULL)
        {
            // found a free slot, so open the view, and gather all info about this view
            
            // save the table name
            m_ptrActiveTableName[i] = (char*) new char[(strlen(ptrAViewName) +1)];
            strcpy(m_ptrActiveTableName[i], ptrAViewName);
            
            // retrieve and save the table structure
            m_ptrActiveTableStructure[i] = (char*) new char[(strlen(m_ptrDataStore->Description(ptrAViewName)) + 1)];
            strcpy(m_ptrActiveTableStructure[i], m_ptrDataStore->Description(ptrAViewName));
            
            // open the db view by building the GetAs structure string
            if (strlen(m_ptrActiveTableStructure[i]) > 0)
            {
                // make up a char array with the table structure to include name and columns
                sTemp = m_ptrActiveTableName[i];
                sTemp += TEXT("[");
                sTemp += m_ptrActiveTableStructure[i];
                sTemp += TEXT("]");
                // get rid of old part
                DELETEANDNULLALL(m_ptrActiveTableStructure[i]);
                // save the complete structure
                m_ptrActiveTableStructure[i] = (char*) new char[sTemp.GetLength() + 1];
                StringToChar(sTemp, &cTemp);//
                strcpy(m_ptrActiveTableStructure[i], cTemp);// sTemp
                DELETEANDNULLALL(cTemp);
            }
            
            // open the view from the store
            if (bOpenViewInMemory)
            {
                sTemp = ptrAViewName;
                CopyViewFromFileToMemory(sTemp, i);
                m_bViewIsCopyInMemory[i] = TRUE;
            }
            else
            {
                // open view as instantiated with View member rather than with the GetAs
                // m_ActiveViewHandle[i] = m_ptrDataStore->GetAs(temp);
                m_ActiveViewHandle[i] = m_ptrDataStore->View(ptrAViewName);
            }
            
            // then set up for retrun
            value = i;  // set up return value of the index being used
            
            // close file if it was opened here
            if (bNeedToCloseFile)
            {
                DELETEANDNULL(m_ptrDataStore);
            }
            
            break;          // since we completed processing, just break out of for loop
        } // end of process when empty slot found
    }   // end of for loop
    
    return value;
}//@CODE_103


/*@NOTE_176
Override to open a view with a CString rather than char array. Converts to char
array and opens with char array.
*/
int CMk4DataContainer::OpenAView(CString sAViewName, bool bOpenViewInMemory)
{//@CODE_176
    int value;
    value = 0;      // initialize return value
    
    // convert the string into a char
    char* temp = NULL;
	StringToChar(sAViewName, &temp);
    
    value = OpenAView(temp, bOpenViewInMemory);
    
    DELETEANDNULLALL(temp);    // clean up allocation
    
    return value;
}//@CODE_176


/*@NOTE_184
Physically sort the rows of a named view on one or more columns. This will
probably be depricated.
*/
#ifndef q4_WINCE
int CMk4DataContainer::SortViewOn(const char* ptrViewName,
                                  const char* ptrSortCol)
{//@CODE_184
    int value;  // return value
    //    int seq;
    int i;      // loop and work variable
    int loc;    // location counter for parsing string
    int len;    // length of string
    //    long nrows;     // number of rows in the view
    int ncoltomove = 0; // the number of columns to sort on
    bool LeaveOpen = FALSE; // flag to identify when view needs to be left open
    CString t1;
    CString t2;     // temp working strings
    CString origstructure;   // the structure of the original table
    CString sNewstructure;   // the structure after being reordered with sort cols first
    CString dupstructure;     // the clone of structure, but with different table name
    
#define MAXPROPS 5
    CString sArrayOfNames[MAXPROPS];            // the names of props
    int iColNum[MAXPROPS];
    
    value = 0;
    
    if (m_iDBStatus == DB_NOT_AVAIL)
    {
        // error if store not open
        value = ERR_DBNOTOPEN;
#ifndef q4_WINCE
        PopMsg("Sorting View", value);
#endif
        return value;
    }
    
    // find out how many columns to sort on
    t1 = ptrSortCol;    // make a copy of the comma delimited list
    t1.TrimRight();  // get rid of any spaces
    t1.TrimLeft();  // get rid of any spaces
    if (t1.GetLength() <= 0)
        return -1;     // if there are no cols requested, just return
    
    // starting from the back, find a column name (token),
    // and move it to the front of mk4 structure
    while (t1.GetLength() > 0 && ncoltomove <= MAXPROPS) 
    {
        ncoltomove++;
        len = t1.GetLength();   // the length of the comma delimited list
        loc = t1.ReverseFind(',');  // find the delimiter, starting from end
        if (loc <= 0)
            t2 = t1;  // if no delimiter, then use the whole thing
        else 
            t2 = t1.Right(len- (loc + 1));   // or just the last token
        t2.TrimRight();  // get rid of any spaces
        t2.TrimLeft();  // get rid of any spaces
        if (loc <= 0)
            t1.Empty();  // if no delimiter, then just clear rest
        else 
            t1.Delete(loc, len - loc);   // or just clear the last token
        t1.TrimRight();  // get rid of any spaces
        t1.TrimLeft();  // get rid of any spaces
        sArrayOfNames[ncoltomove - 1] = t2;
    }       // end of while still something in string loop
    
    // *** ready to start the sort here
    // if the view is open already, close it
    if (ViewIsOpen(ptrViewName, &i))
    {
        CloseAView(ptrViewName);
        LeaveOpen = TRUE;   // after we recreate the view, leave open if it was
    }
    
    // build a leagal mk4 structure statement for original table
    origstructure = ptrViewName;
    origstructure += TEXT("[");
    origstructure += m_ptrDataStore->Description(ptrViewName);
    origstructure += TEXT("]");
    // open original view
    c4_View origview = m_ptrDataStore->View(ptrViewName);
    
    // create the sorted view
    // 
    c4_View sortedview;
    char* cTemp = NULL;

    for (i = 0; i < ncoltomove && i <= MAXPROPS; i++) 
    {
        cTemp = NULL;
        StringToChar(sArrayOfNames[i], &cTemp);
        iColNum[i] = origview.FindPropIndexByName(cTemp);
        DELETEANDNULLALL(cTemp);

        if (iColNum[i] < 0)
            return ERR_NOPROP;
    }
    if (ncoltomove == 1)
        sortedview = origview.SortOn((origview.NthProperty(iColNum[0])));
    else if (ncoltomove == 2)
        sortedview = origview.SortOn(
        (origview.NthProperty(iColNum[1]),
        origview.NthProperty(iColNum[0])));
    else if (ncoltomove == 3)
        sortedview = origview.SortOn(
        (origview.NthProperty(iColNum[2]),
        origview.NthProperty(iColNum[1]),
        origview.NthProperty(iColNum[0])));
    
    // make a structure the same except that it has view name duplicate
    dupstructure = origstructure;
//#if defined q4_WINCE
//    dupstructure.Replace((unsigned short)ptrViewName, (unsigned short)TEXT("duplicate"));
//#else
    dupstructure.Replace(ptrViewName, TEXT("duplicate"));    
//#endif
    // make a copy of sorted records on the file
    cTemp = NULL;
    StringToChar(dupstructure, &cTemp);
    c4_View dup = m_ptrDataStore->GetAs(cTemp);
    DELETEANDNULLALL(cTemp);

    dup.SetSize(0);
    
    // copy sorted records into duplicate table as an insertion
    dup.InsertAt(0, sortedview);
    
    // save the changes
    m_ptrDataStore->Commit();
    
    // now recopy it back to original
    origview.SetSize(0);
    origview.InsertAt(0, dup);
    DeleteView("duplicate");
    m_ptrDataStore->Commit();
    
    // clean up anything left hanging around
    dup = c4_View();
    origview = c4_View();
    sortedview = c4_View();
    
    // reopen the view if it already was
    if (LeaveOpen)
        OpenAView(ptrViewName);
    
    /* *****************   ***   ***
    // Although this is some useful code, using the SortOn does not
    // require reording the data as Ordered or Hash does
    // save the original structure, build a leagal mk4 structure statement
    origstructure = ptrViewName;
    origstructure += "[";
    origstructure += m_ptrDataStore->Description(ptrViewName);
    origstructure += "]";
    
    // permute the col names in the structure with sort cols first
    sNewstructure = origstructure;
    // find out how many columns to sort on
    t1 = ptrSortCol;    // make a copy of the comma delimited list
    t1.TrimRight();  // get rid of any spaces
    t1.TrimLeft();  // get rid of any spaces
    if (t1.GetLength() <= 0)
        return -1;     // if there are no cols requested, just return
    
    // starting from the back, find a column name (token),
    // and move it to the front of mk4 structure
    while (t1.GetLength() > 0) 
    {
        ncoltomove++;
        len = t1.GetLength();   // the length of the comma delimited list
        loc = t1.ReverseFind(',');  // find the delimiter, starting from end
        if (loc <= 0)
            t2 = t1;  // if no delimiter, then use the whole thing
        else 
            t2 = t1.Right(len- (loc + 1));   // or just the last token
        t2.TrimRight();  // get rid of any spaces
        t2.TrimLeft();  // get rid of any spaces
        if (t2.GetLength() > 0)
            MoveColToFront(sNewstructure , t2);     // rearrange structure
        if (loc <= 0)
            t1.Empty();  // if no delimiter, then just clear rest
        else 
            t1.Delete(loc, len - loc);   // or just clear the last token
        t1.TrimRight();  // get rid of any spaces
        t1.TrimLeft();  // get rid of any spaces
        ArrayOfStrings[ncoltomove - 1] = t2;
    }       // end of while still something in string loop
    
    
    // make a structure the same except that it has view name duplicate
    dupstructure = sNewstructure;
    dupstructure.Replace(ptrViewName, "duplicate");
    
    // *** ready to start the sort here
    // if the view is open already, close it
    if (ViewIsOpen(ptrViewName, &i))
    {
        CloseAView(ptrViewName);
        LeaveOpen = TRUE;   // after we recreate the view, leave open if it was
    }
    
    // *************   Open the table, sort and save  ******
    
    // first reorder cols so we can use Ordered member
    c4_View sNewstructureview = m_ptrDataStore->GetAs(sNewstructure);
    
    // make a copy of sorted records on the file
    c4_View dupofordered = m_ptrDataStore->GetAs(dupstructure);
    dupofordered.SetSize(0);
    
    // ****** try several options for sorting records
    
    // 1 - sort by reordered cols using Ordered member function
    // appears Ordered does not actually sort records, just used for searching
    // c4_View sortedview = sNewstructureview.Ordered(ncoltomove);
    
    // 2 - Try sorting using the Hash member function
    // works, but again not in sorted order
    // c4_View HashTable = m_ptrDataStore->GetAs("TempHash[_H:I,_R:I]");
    // c4_View sortedview = sNewstructureview.Hash(HashTable, ncoltomove);
    
    // 3 - Use SortOn member
    // finally found one that works
    c4_Property pAge('I', "age");
    c4_View sortedview = sNewstructureview.SortOn(pAge);
    
    // *** Several ways to copy sorted records, by insert or one row at a time
    // 1 - try the Duplicate member function, this one results in empty table
    // dupofordered = sortedview.Duplicate();
    
    // 2 - copy sorted records into duplicate table as an insertion
    dupofordered.InsertAt(0, sortedview);
    
    // 3 - copy row at a time
    // nrows =  sortedview.GetSize();
    // c4_Row RowToCopy;
    // for (i = 0; i<nrows; i++) {
    //    RowToCopy = sortedview[i];
    //    dupofordered.Add(RowToCopy);
    //}   // for loop through all records
    
    // commit the changes
    m_ptrDataStore->Commit();
    
    // then toss the old stuff and recopy sorted
    // sNewstructureview.SetSize(0);
    // sNewstructureview.InsertAt(0, dupofordered);
    // m_ptrDataStore->Commit();
    
    // reorder the cols to recreate the original structure
    sNewstructureview = m_ptrDataStore->GetAs(origstructure);
    // clean out the data from the temp sort table
    // dupofordered.SetSize(0);
    m_ptrDataStore->Commit();
    
    // rename the backup view and delete original
    // c4_ViewProp oldname(ptrViewName);
    // c4_ViewProp newname("backup");
    // m_ptrDataStore->Rename(oldname, newname);
    *************************  ***   ***   */
    
    /*       ***** some old stuff, probably of no use
    // set up mk4 properties to retrieve info for the table we want to create or restructure
    c4_StringProp pColNameForTable(m_sDDColNameForTable);
    c4_StringProp pColNameForIsUsed(m_sDDColNameForIsUsed);
    c4_IntProp pSortColName(ptrSortCol);
    c4_IntProp pSortColName2(ptrSortCol);
    c4_StringProp pFieldType(m_sDDColNameForFType);
    c4_StringProp pCharName(m_sDDColNameForColumnName);
    
    c4_View dd = m_ptrDataStore->View(m_sDDViewName);
    c4_View ddo = dd.Ordered(1);
    
    // loop through the records and recode the sorted order column sequentially
    nrows =  ddo.GetSize();
    seq = 1;
    for (i = 0; i < nrows; i++)
    {
        if (pSortColName(dd3[i]) > 0)
        {
            pSortColName2(dd[dd3.GetIndexOf(dd3[i])]) = seq;
            // c4_RowRef r = dd[dd3.GetIndexOf(dd3[i])];
            //(pSortColName)(r) = seq;
            seq++;
        }
    }   // for loop through all records
    */
    
    return value;
}//@CODE_184
#endif /* not defined q4_WINCE */


/*@NOTE_162
Find out if the view name already in use, ie. open. If it is, return the index.
*/
bool CMk4DataContainer::ViewIsOpen(const char* ptrAViewName, int* ViewIndex)
{//@CODE_162
    bool value;
    int i;
    
    value = FALSE;
    if (m_iDBStatus == DB_NOT_AVAIL)
    {
        // error if store not open
        *ViewIndex = ERR_DBNOTOPEN;
#ifndef q4_WINCE
        PopMsg("Checking for view", *ViewIndex);
#endif
        return value;
    }
    // Check to see if view is open, if it is just return the index
    for (i = 0; i < VGMAXOPEN; i++)
    {
        if (m_ptrActiveTableName[i] != NULL)
        {
            if (strcmp(m_ptrActiveTableName[i], ptrAViewName) == 0)
            {
                // printf("Table %s already open\n", ptrAViewName);
                *ViewIndex = i;
                value = TRUE;
            }
        }
    }
    
    return value;
}//@CODE_162


/*@NOTE_150
Adds a table/view to the database that contains the data dictionary structure.
This is the basic unit on which some member functions are built(ie. make a
view)
*/
int CMk4DataContainer::MakeDataDictView()
{//@CODE_150
    int value;
    value = ERR_NONE;
#ifndef q4_WINCE
    // check that a storaage is open
    if (m_iDBStatus != DB_NOT_AVAIL) 
    {
        char* cTemp = NULL;
        StringToChar(m_sDDStructure, &cTemp);
        m_ptrDataStore->GetAs(cTemp);
        DELETEANDNULLALL(cTemp);
        //            m_ptrDataStore->GetAs((const char*)m_sDDStructure.operator LPCTSTR());  // instantiate the dd
        m_ptrDataStore->Commit();    // make it permenant
        BuildListOfViews();
    } 
    else 
    {
        value = ERR_DBNOTOPEN;
        PopMsg("Making Data Dictionary", value);
        return value;
    }
    
#endif
    return value;
}//@CODE_150


/*@NOTE_151
Parses the data dictionary to create the specified mk4 view structure and then
proceed to open it. If the view already exists, this function may restructure
the view(and possibly delete data if columns are dropped).  The programmer must
do the necessary checks to guard against inadvertant data loss before calling
this function.
*/
int CMk4DataContainer::MakeViewFromDataDictionary(const char* ptrAViewName,
                                                  const char* ptrSortCol)
{//@CODE_151
    int value = ERR_NONE;  // return value
    int i; // loop variable
    //    long j;
    //    int seq;    // to keep track of seq number when renumbering order col values
    CString temp;    // used as a temporaty store for string fields
    CString temp2;    // used as a temporaty store for string fields
    char* cTemp = NULL;
    
    bool LeaveOpen = FALSE;     // flag to indicate view was open, so need to leave open
//    bool LeaveDDOpen = FALSE;     // flag to indicate view was open, so need to leave open
    
    // be sure the database is open
    if (m_iDBStatus != DB_NOT_AVAIL)
    {
        // if the requested view is open already, close it
        if (ViewIsOpen(ptrAViewName, &i))
        {
            CloseAView(ptrAViewName);
            LeaveOpen = TRUE;   // after we recreate the view, leave open if it was
        }
        
        // Opening the view with the GetAs will create/restructure the data
        // build the structure from the definitions of the cols in the data dictionary
#if defined q4_WINCE
        // set up mk4 properties to retrieve info for the table we want to create or restructure
        
        StringToChar(m_sDDColNameForTable, &cTemp);
        c4_StringProp pColNameForTable(cTemp);
        DELETEANDNULLALL(cTemp);
        
        StringToChar(m_sDDColNameForIsUsed, &cTemp);
        c4_StringProp pColNameForIsUsed(cTemp);
        DELETEANDNULLALL(cTemp);
        
        c4_IntProp pSortColName(ptrSortCol);
        
        StringToChar(m_sDDColNameForFType, &cTemp);
        c4_StringProp pFieldType(cTemp);
        DELETEANDNULLALL(cTemp);
        
        StringToChar(m_sDDColNameForColumnName, &cTemp);
        c4_StringProp pCharName(cTemp);
        DELETEANDNULLALL(cTemp);
        
        // get all of the records from dd for this table definition, ie. requested table name
        StringToChar(m_sDDViewName, &cTemp);
        c4_View dd = m_ptrDataStore->View(cTemp);
        
        DELETEANDNULLALL(cTemp);
        
        c4_View dd3 = dd.Select((pColNameForTable[ptrAViewName])).SortOn(pSortColName);
        
#else
        // set up mk4 properties to retrieve info for the table we want to create or restructure
        c4_StringProp pColNameForTable(m_sDDColNameForTable);
        c4_StringProp pColNameForIsUsed(m_sDDColNameForIsUsed);
        c4_IntProp pSortColName(ptrSortCol);
        // c4_IntProp pSortColName2(ptrSortCol);
        c4_StringProp pFieldType(m_sDDColNameForFType);
        c4_StringProp pCharName(m_sDDColNameForColumnName);
        
        // get all of the records from dd for this table definition, ie. requested table name
        c4_View dd = m_ptrDataStore->View(m_sDDViewName);
        c4_View dd3 = dd.Select((pColNameForTable[ptrAViewName])).SortOn(pSortColName);
#endif
        
        /*  This does not seem to work
        // resequence columns
        // loop through the records and recode the sorted order column sequentially
        int nrows =  dd.GetSize();
        seq = 1;
        for (j = 0; j < nrows; j++)
        {
        c4_RowRef r = dd[j];
        // (const char*) ((c4_StringProp&) p) (r)
        CString TableName = (const char *)((c4_StringProp&) pColNameForTable)(r);
        if (TableName.Compare(ptrAViewName)== 0)
        {
        int ltmp = (int)((c4_IntProp&) pSortColName)(r);
        if (ltmp > 0)
        {
        ((c4_IntProp&) pSortColName)(r) = seq;
        // pSortColName(dd3[j]) = seq;
        seq++;
        }
        }
        }   // for loop through all records
        */
        // c4_View dd3 = dd.Select((pColNameForTable[ptrAViewName])+(pColNameForIsUsed["Y"],pColNameForIsUsed["M"])).SortOn(pSortColName);
        // c4_View dd2 = dd.Select((pColNameForTable[ptrAViewName]));
        // c4_View dd3 = dd2.Select((pColNameForIsUsed["Y"],pColNameForIsUsed["M"])).SortOn(pSortColName);
        
        // now build the mk4 structure statement from DD specs
        CString TableStructure;
        TableStructure = ptrAViewName;  // start with table name
        TableStructure += TEXT("[");    // then add syntax to begin field defs
        
        // find out how many columns are in the desired table, ie. each described on one line
        int iNumColsInTable = dd3.GetSize();  // one col for each row definition in dd
        if (iNumColsInTable <= 0)
            return -1;  // either no dd or nothing in it for this name
        
        // loop through the records finding all of the ordering greater than 0
        for (i = 0; i < iNumColsInTable; i++)
        {
            temp = pColNameForIsUsed(dd3[i]);
            long itmp = (long) pSortColName(dd3[i]);
            if ((itmp > 0) && (!temp.Compare(TEXT("Y")) || !temp.Compare(TEXT("M"))))
            {
                if (TableStructure.GetAt(TableStructure.GetLength() - 1) != TEXT('['))  
                    TableStructure += TEXT(",");   // after first one
                temp2 = (const char*) pCharName(dd3[i]);  // get the field name
                temp2.Remove(' ');  // get rid of any blanks blanks
                TableStructure += temp2; // append field name
                // append field type as stored in DD
                temp = pFieldType(dd3[i]);
                /*  Changed the contents that are in the table so that  we store character
                if (!temp.Compare(_T("string")))
                TableStructure += ":S";
                if (!temp.Compare(_T("int")))
                TableStructure += ":I";
                if (!temp.Compare(_T("float")))
                TableStructure += ":F";
                if (!temp.Compare(_T("double")))
                TableStructure += ":D";
                */
                if (!temp.Compare(TEXT("S")) && !temp.Compare(TEXT("I")) && !temp.Compare(TEXT("L")) &&
                    !temp.Compare(TEXT("F")) && !temp.Compare(TEXT("D"))) 
                    temp = TEXT("S");
                TableStructure += TEXT(":");
                TableStructure += temp;
            }
        }   // for loop through all records
        
        // loop through the records finding all of the ordering equal to 0
        for (i = 0; i < iNumColsInTable; i++)
        {
            temp = pColNameForIsUsed(dd3[i]);
            if ((pSortColName(dd3[i]) == 0) && (!temp.Compare(TEXT("Y")) || !temp.Compare(TEXT("M"))))
            {
                if (TableStructure.GetAt(TableStructure.GetLength() - 1) != TEXT('['))  
                    TableStructure += TEXT(",");   // after first one
                temp2 = (const char*) pCharName(dd3[i]);  // get the field name
                temp2.Remove(' ');  // get rid of any blanks blanks
                TableStructure += temp2; // append field name
                // append field type
                temp = pFieldType(dd3[i]);
                /* changed contents in table
                if (temp.Compare(_T("string")) == 0)
                TableStructure += ":S";
                if (temp.Compare(_T("int")) == 0)
                TableStructure += ":I";
                if (temp.Compare(_T("float")) == 0)
                TableStructure += ":F";
                if (temp.Compare(_T("double")) == 0)
                TableStructure += ":D";
                */
                if (!temp.Compare(TEXT("S")) && !temp.Compare(TEXT("I")) && !temp.Compare(TEXT("L")) &&
                    !temp.Compare(TEXT("F")) && !temp.Compare(TEXT("D"))) 
                    temp = TEXT("S");
                TableStructure += TEXT(":");
                TableStructure += temp;
            }
        }   // for loop through all records
        TableStructure += TEXT("]");
        // finally, instantiate the table
        //#if defined q4_WINCE
        StringToChar(TableStructure, &cTemp);
        m_ptrDataStore->GetAs(cTemp);
        DELETEANDNULLALL(cTemp);
        //#else
        //        m_ptrDataStore->GetAs(TableStructure);
        //#endif
        m_ptrDataStore->Commit();
        BuildListOfViews();
        // clean up views
        dd = c4_View();
        dd3 = c4_View();
        
        /* ************** some example code for metakit
        // accessing a view with data
        c4_StringProp pName("name"), pCountry("country");        
        c4_Storage storage("myfile.dat");     
        c4_View vAddress = storage.View("address");       
        for (int i = 0; i < vAddress.GetSize(); ++i)             
        ... pName(vAddress[i]) ... pCountry(vAddress[i]) ...
        }
        // doing some sorting
        c4_View vSorted = vAddress.SortOn(pName);
        vSorted = vAddress.SortOn((pCountry, pName));      
        ASSERT(vSorted.GetSize() == vAddress.GetSize());
        c4_View vSome = vAddress.Select(pCountry["UK"]).SortOn(pName);
        for (int i = 0; i < vSome.GetSize(); ++i)
        {
        CString name = pName(vSome[i]);
        printf("'%s' lives in the UKn", (const char*) name);
        printf("Entry 'vAddress[%d]' is that same personn",
        vAddress.GetIndexOf(vSome[i]));
        }
        ************************   end of example code */
    }       // end of if storage open
    else 
    {
        value = ERR_DBNOTOPEN;
#ifndef q4_WINCE
        TRACE("Making view from DD, no db open");
#endif
        return value;
    }
    return value;
}//@CODE_151


/*@NOTE_187
Move the specified column to the front of a mk4 structure.  This is an internal
function that is used by the SortOnCol member, and not for general use.
Probably will be depricated.
*/
int CMk4DataContainer::MoveColToFront(CString& refStructure,
                                      CString& refColName)
{//@CODE_187
    int value;
    int foundat;    // the position where the col name was found
    int begstruct;  // the point in the structure where the [ was found
    CString temp;   // holds the cut string for pasting
    
    value = 0;
    foundat = refStructure.Find(refColName);    // look for sub string
    if (foundat < 0)
        return -1;     // if not found, then return error
    begstruct = refStructure.Find('['); // look for the start of structure
    if (begstruct < 0)
        return -1;     // if not found, then return error
    temp = refStructure.Mid(foundat, refColName.GetLength() + 2); // extract spec
    refStructure.Delete(foundat, refColName.GetLength() + 2);     // remove from structure
    // remove any extra commas
    refStructure.Replace(TEXT("[,"), TEXT("["));
    refStructure.Replace(TEXT(",,"), TEXT(","));
    if (refStructure.GetAt(foundat + 1) != ']')
        temp += TEXT(",");  // more than one col spec in structure, so put in a comma
    refStructure.Insert(begstruct + 1, temp);
    
    return value;
}//@CODE_187


/*@NOTE_346
Request for a free index from view list so that caller can open a view in a way
other than the default, 'as is', way.  Caller supplies the name to be used for
the view. Close in the regular way. This override is when there is a derived
view so you can enter the parent view name.
*/
int CMk4DataContainer::FindFreeViewIndex(const char* ptrAViewName,
                                         const char* ptrParentViewName)
{//@CODE_346
    int value;    // return variable
    
    value = -1;
    
    // find a free slot by calling the unloaded function
    value = FindFreeViewIndex(ptrAViewName);
    // save the parent table name
    if (ptrParentViewName != NULL)
    {
        m_ptrParentTableName[value] = (char*) new char[(strlen(ptrParentViewName) +1)];
        strcpy(m_ptrParentTableName[value], ptrParentViewName);
    }
    // save the derived flag
    m_iParentIndex[value] = FindViewIndexByName(ptrParentViewName);
    
    return value;
}//@CODE_346


/*@NOTE_350
Search through the open views by name and return the index of  that view.
Override to accept CString name.
*/
int CMk4DataContainer::FindViewIndexByName(const CString& strAViewName)
{//@CODE_350
    int i;        // looping variable
    
    char* cTemp = NULL;
    StringToChar(strAViewName, &cTemp);
    
    //    if (ViewIsOpen(strAViewName, &i)) return i;
    if (ViewIsOpen(cTemp, &i))
    {
        DELETEANDNULLALL(cTemp);
        return i;
    }
    DELETEANDNULLALL(cTemp);
    return -1;
}//@CODE_350


/*@NOTE_369
Attach Data dictionary control to a view. DD is used for default values,
validation and gui presentation. When the DD is attached, this routine will
parse the information about this view from the DD, and create a memory data
structure to hold it.
*/
int CMk4DataContainer::AttachDDToView(const int ViewNum)
{//@CODE_369
    int value = 0; // set up default error return
    int iv; // index for view, set to view or parent if derived
    
    // if DD already attached, detach so it can be reattached
    if (m_bViewUsesDD[ViewNum])
        DetachDDFromView(ViewNum);
    
    // if this is a derived view, just make sure that dd attached to parent
    // do not attach dd to a derived view
    // get the flag/index for parent
    iv = m_iParentIndex[ViewNum];
    if (iv >= 0) // this is a derived view if it has a parent (>= 0)
    {
        // check to make sure the dd attached to parent view
        if (!m_bViewUsesDD[iv]) 
        { // not yet attached to parent, so attach it
            value = AttachDDToView(iv);
        }
        m_bViewUsesDD[ViewNum] = TRUE;
        return value;
    }

    // so  if we got to here, this is a raw view, so attach it
    // if the dd is open, look up the defaults, else, open it to get them
    int iDDView = FindViewIndexByName(m_sDDViewName); // index of the dd view
    bool bOpened = FALSE;   // flag that indicates opened here (TRUE), or already open (FALSE)
    if (iDDView < 0)
    {
        iDDView = OpenAView(m_sDDViewName);
        if (iDDView < 0)
            return iDDView;    // no dd available
        bOpened = TRUE; // flag so that it can be closed after setup
    }
    
    
    // Get the name of the table that has definitions in the dd
    CString sViewName;
    sViewName = m_ptrActiveTableName[ViewNum];   // the name of the raw data table
    
    int idvindex;	// index for derived view of dd
    char* ptrTemp = NULL;	// work string
    // some temporary string handling
    CString sTemp, sTemp2;
    CString sSelCrit;  // selection criteria dynamically built to look up dd
    CString sColName;   // the name of the column in table
    
    // *******************************************
    // regardless of raw or derived view, just:
    //   create the structure that holds specs for each col in table
    //   get the col name and look it up in the dd, (ie. use derived view)
    //   copy values off into structure
    
    // find column indexes in dd for the default value, validation and field names
    StringToChar(m_sDDColNameForDefValue, &ptrTemp);
    int iDefVal;
    iDefVal =  m_ActiveViewHandle[iDDView].FindPropIndexByName(ptrTemp);
    DELETEANDNULLALL(ptrTemp);
    StringToChar(m_sDDColNameForColumnName, &ptrTemp);
    int iFieldName;
    iFieldName =  m_ActiveViewHandle[iDDView].FindPropIndexByName(ptrTemp);
    DELETEANDNULLALL(ptrTemp);
    // get the indexes for the validation cols
    int iMinIndex;
    iMinIndex = m_ActiveViewHandle[iDDView].FindPropIndexByName("Min");
    int iMaxIndex;
    iMaxIndex = m_ActiveViewHandle[iDDView].FindPropIndexByName("Max");
    int iListIndex;
    iListIndex = m_ActiveViewHandle[iDDView].FindPropIndexByName("Validate");
    int iDisplayLengthIndex;
    iDisplayLengthIndex = m_ActiveViewHandle[iDDView].FindPropIndexByName("Length");
    
    // get the number of attributes in the table
    m_iNumDefFields[ViewNum] = m_ActiveViewHandle[ViewNum].NumProperties();	// num cols
    // create the data structures necessary to hold the default values
    m_ptrDefaultValues[ViewNum] =  new DEFAULTVALUES[m_iNumDefFields[ViewNum]];
    
    // loop through each col in table and look up in dd to get params
    for (int i = 0; i < m_iNumDefFields[ViewNum]; i++)
    {
        // set the method and constant to none, then change later if present
        m_ptrDefaultValues[ViewNum][i].cMethod = DEFNONE;
        m_ptrDefaultValues[ViewNum][i].ptrConstant = NULL;
        m_ptrDefaultValues[ViewNum][i].ptrValidateMin = NULL;
        m_ptrDefaultValues[ViewNum][i].ptrValidateMax = NULL;
        m_ptrDefaultValues[ViewNum][i].ptrValidateList = NULL;
        
        // get the name of the ith col in table
        sColName = m_ActiveViewHandle[ViewNum].NthProperty(i).Name();
        // derive a view from dd for this table and col name
        sSelCrit = m_sDDColNameForTable + "="+sViewName+
            ","+m_sDDColNameForColumnName + "="+sColName;
        idvindex = DeriveView(m_sDDViewName, "", sSelCrit);  // derived view w/1
        
        // should be only one row, 0, so pull values from it
        if (m_ActiveViewHandle[idvindex].GetSize()== 1)
        {
            // skip if not 1 row
            // get the default field spec and parse it to get method
            GetDataItemAs(idvindex, 0, iDefVal, sTemp);  // get the default from dd
            sTemp.TrimLeft();	// remove blanks
            sTemp.TrimRight();	// remove blanks
            if (sTemp.GetLength() > 0)
            {
                // parse method and constant
                m_ptrDefaultValues[ViewNum][i].cMethod = (char)sTemp[0];    // save control
                sTemp.Delete(0, 1);    // remove the control character
                if (sTemp.GetLength() > 0)     // parse the constant if present
                    StringToChar(sTemp, &m_ptrDefaultValues[ViewNum][i].ptrConstant);
            }
            // get the min, max and validate list values
            GetDataItemAs(idvindex, 0, iMinIndex, sTemp);  // get the min from dd
            sTemp.TrimLeft();	// remove blanks
            if (sTemp.GetLength() > 0)
            {
                StringToChar(sTemp, &m_ptrDefaultValues[ViewNum][i].ptrValidateMin);
            }
            GetDataItemAs(idvindex, 0, iMaxIndex, sTemp);  // get the max from dd
            sTemp.TrimLeft();	// remove blanks
            if (sTemp.GetLength() > 0)
            {
                StringToChar(sTemp, &m_ptrDefaultValues[ViewNum][i].ptrValidateMax);
            }
            GetDataItemAs(idvindex, 0, iListIndex, sTemp);  // get the list from dd
            sTemp.TrimLeft();	// remove blanks
            if (sTemp.GetLength() > 0)
            {
                StringToChar(sTemp, &m_ptrDefaultValues[ViewNum][i].ptrValidateList);
            }
            GetDataItemAs(idvindex, 0, iDisplayLengthIndex, &m_ptrDefaultValues[ViewNum][i].iDisplayLength);  // get the length from dd
        }
        
        // done with this row, so close derived view
        CloseAView(idvindex);
    }
    
    
    // if the dd view had to be opened here, then close it
    if (bOpened)
        CloseAView(iDDView);
    
    // turn on flag to use dd for this view
    m_bViewUsesDD[ViewNum] = TRUE;  // set flag that dd will be used on this view
    
    return value;
}//@CODE_369


/*@NOTE_372
Detach Data dictionary control from a view. Only raw views need to have dd
attached since derived views will unmap back to raw view. DD is used for
default values, validation and gui presentation.  Detatching will undo
everything the attach did.
*/
int CMk4DataContainer::DetachDDFromView(const int ViewNum)
{//@CODE_372
    int value = -1; // set up default error return

    // check to see if this is a derived or raw view
    // if derived view, just reset dd flag
//    if (m_iParentIndex[ViewNum] >= 0) // this is a derived view if it has a parent (>= 0)
//    {
//        m_bViewUsesDD[ViewNum] = FALSE;
//        return 0;
//    }

    // make sure that the view number is valid
    if (ViewNum >=0)
    {
		// check to see if this is a derived or raw view
		// if derived view, just reset dd flag
		if (m_iParentIndex[ViewNum] >= 0) // this is a derived view if it has a parent (>= 0)
		{
			m_bViewUsesDD[ViewNum] = FALSE;
		 return 0;
		}

        if (m_ptrDefaultValues[ViewNum] != NULL)
        {
            // check to find out if something there
            // get rid of all of the allocated strings for each attribute
            for (int i = 0; i < m_iNumDefFields[ViewNum]; i++)
            {
                DELETEANDNULLALL(m_ptrDefaultValues[ViewNum][i].ptrConstant);
                DELETEANDNULLALL(m_ptrDefaultValues[ViewNum][i].ptrValidateMin);
                DELETEANDNULLALL(m_ptrDefaultValues[ViewNum][i].ptrValidateMax);
                DELETEANDNULLALL(m_ptrDefaultValues[ViewNum][i].ptrValidateList);
            }
            // then get rid of the array of structures and reset pointers
            DELETEANDNULLALL(m_ptrDefaultValues[ViewNum]);
            m_bViewUsesDD[ViewNum] = FALSE;
            value = 0;
        }
    }
    return value;
}//@CODE_372


/*@NOTE_387
Derive a view from a raw view, using specifications passed.
Input parameters are:
CString sRawViewName--the name of the view participating in the derived
view, 
CString sProjectionList--csv list of column names from parent to show up
in derived,
CString sSelectCriteria--csv list of select criteria that subsets rows,
CString sSortOn--csv list of column names from parent to sort on,
CString  sSelectRangeCriteria--like select, but specifies ranges.
Can do any combination of Projection, Selection, ranges and sorting.
If you do not want any feature, pass empty string (the default).
Can pass empty CString for any of last four fields.
The derived view will be in one of the slots of the array of views,
and will have the name of the raw  view with D prepended (unless supplied as arg).
Return value will be the array index, with -1 returned for error.
*/

int CMk4DataContainer::DeriveView(const CString sRawViewName,
                                    const CString sProjectionList,
                                    const CString sSelectCriteria,
                                    const CString sSortOn,
                                    CString sDerViewName,
                                    const CString sSelectRangeCriteria)
{//@CODE_387
    int iDerView;    // index of derived view, will be returned
    int iRawView;    // index of raw view

    int iNumProj = 0;  // the number of projection columns
    int iProjLen = 0;    // the string length of incoming projection columns

    int iNumSel = 0;  // the number of selection criteria
    int iSelLen = 0;    // the string length of incoming select criteria
    int iSelIndex = -1;    // the index in data container that has view
    CString sSelCriteriaViewName = TEXT("SelCrit");    // view name of selection

    int iNumSelR = 0;  // the number of selection criteria
    int iSelRLen = 0;    // the string length of incoming select criteria
    int iSelRIndex = -1;    // the index in data container that has view
    CString sSelRCriteriaViewName = TEXT("SelRCrit");    // view name of selection

    int iNumSort = 0;   // the number of sort criteria
    int iSortLen = 0;    // the string length of incoming sort criteria

    char* ptrTemp=NULL;  // a working pointer to character array
    char* ptrTemp2=NULL;  // a working pointer to character array
    CString sTemp;    // a working string
    CString sToken;    // a working string to hold token

    // views for holding templates of projection and sorting
    c4_View vColSubset; // a view that contains the mask for the projection
    c4_View vColSort; // a view that contains the mask for sort

    // raw view must be open, so open if not already, and leave it open
    iRawView = FindViewIndexByName(sRawViewName); // index of the raw view
    if (iRawView < 0)
    {
        // if not already open, then open it
        iRawView = OpenAView(sRawViewName);
    }
    if (iRawView < 0)
    {
        // if not in file, then an error
        return iRawView;
    }

    // ***********************************************************************
    // The Projection, or column subset
    // create an unattached mk4 view with the column subset for doing projection
    // ***********************************************************************
    if ((iProjLen = sProjectionList.GetLength()) > 0)
    {
        iNumProj = MakeViewTemplateFromList(vColSubset, m_ActiveViewHandle[iRawView], sProjectionList);
    }

    // ***********************************************************************
    // The selection, or row subsetting
    // build a view with the number of cols specified and row 0 contains value
    // ***********************************************************************
    if ((iSelLen = sSelectCriteria.GetLength()) > 0)
    {
        // select requested
        char* ptr_cTemp = NULL;
        StringToChar(sSelCriteriaViewName, &ptr_cTemp);
        iSelIndex = FindFreeViewIndex(ptr_cTemp);
        DELETEANDNULLALL(ptr_cTemp);
        bool bViewCreated = FALSE;
        sTemp =  sSelectCriteria;    // make a copy of selection criteria
        // parse the criteria into a created view with values in first row
        while (sTemp.GetLength() > 0)
        {
            NextToken(sTemp, sToken, "=");    // get let side of equal
            StringToChar(sToken, &ptrTemp);
            iNumSel++;    // keep track of the number of selection criteria
            if (!bViewCreated)
            {
                // create the view the first time
                m_ActiveViewHandle[iSelIndex]=
                    m_ActiveViewHandle[iRawView]
                    .NthProperty(m_ActiveViewHandle[iRawView]
                    .FindPropIndexByName(ptrTemp));
                bViewCreated = TRUE;
                AppendARow(iSelIndex);
            }
            else
            {    // else just add a prop
                // copy prop by name from original view to new view
                m_ActiveViewHandle[iSelIndex].AddProperty(
                    m_ActiveViewHandle[iRawView]
                    .NthProperty(m_ActiveViewHandle[iRawView]
                    .FindPropIndexByName(ptrTemp))
                    );
            }
            // find out col number just created
            int iCol = m_ActiveViewHandle[iSelIndex].NumProperties() - 1;
            DELETEANDNULLALL(ptrTemp);
            NextToken(sTemp, sToken);    // get right side of equal
            // save the value
            SetDataItemFrom(iSelIndex, 0, iCol, sToken);
        }
    }

    // ***********************************************************************
    // The range selection, or row subsetting
    // build a view with the number of cols specified and row 0/1 contain values
    // ***********************************************************************
    if ((iSelRLen = sSelectRangeCriteria.GetLength()) > 0)
    {
        // select requested
        StringToChar(sSelRCriteriaViewName, &ptrTemp);
        iSelRIndex = FindFreeViewIndex(ptrTemp);
        DELETEANDNULL(ptrTemp);
        bool bViewCreated = FALSE;
        sTemp =  sSelectRangeCriteria;    // make a copy of selection criteria
        // parse the criteria into a created view with values in first row
        while (sTemp.GetLength() > 0)
        {
            NextToken(sTemp, sToken, "=");    // get left side of equal
            StringToChar(sToken, &ptrTemp);
            iNumSelR++;    // keep track of the number of selection criteria
            if (!bViewCreated)
            {
                // create the view the first time
                m_ActiveViewHandle[iSelRIndex]=
                    m_ActiveViewHandle[iRawView]
                    .NthProperty(m_ActiveViewHandle[iRawView]
                    .FindPropIndexByName(ptrTemp));
                bViewCreated = TRUE;
                AppendARow(iSelRIndex);
                AppendARow(iSelRIndex);
            }
            else
            {    // else just add a prop
                // copy prop by name from original view to new view
                m_ActiveViewHandle[iSelRIndex].AddProperty(
                    m_ActiveViewHandle[iRawView]
                    .NthProperty(m_ActiveViewHandle[iRawView]
                    .FindPropIndexByName(ptrTemp))
                    );
            }
            DELETEANDNULL(ptrTemp);  // free temp chars
            
            // find out col number just created
            int iCol = m_ActiveViewHandle[iSelRIndex].NumProperties() - 1;
            NextToken(sTemp, sToken);    // get right side of equal, part 1
            sToken.Replace(_T("["),_T(""));
            // save the value
            SetDataItemFrom(iSelRIndex, 0, iCol, sToken);
            NextToken(sTemp, sToken);    // get right side of equal, part 2
            sToken.Replace(_T("]"),_T(""));
            // save the value
            SetDataItemFrom(iSelRIndex, 1, iCol, sToken);
        }
    }

    // ***********************************************************************
    // The sorting
    // create an unattached mk4 view with the column subset for doing sort
    // ***********************************************************************
    if ((iSortLen = sSortOn.GetLength()) > 0)
    {
        // sort requested
        iNumSort = MakeViewTemplateFromList(vColSort, m_ActiveViewHandle[iRawView], sSortOn);
    }


    // ***********************************************************************
    // Actual derived view
    // create the derived view here
    // ***********************************************************************
    // open up the derived view
    CString derview;
    if (sDerViewName.GetLength() == 0)
    {
        // none supplied so use default
        derview = TEXT("D") + sRawViewName;    // make a name for the derived view
    }
    else
    {
        derview = sDerViewName;
    }
    StringToChar(derview, &ptrTemp);
    StringToChar(sRawViewName, &ptrTemp2);
    iDerView = FindFreeViewIndex(ptrTemp, ptrTemp2);
    DELETEANDNULLALL(ptrTemp);
    DELETEANDNULLALL(ptrTemp2);

    // assign number code for all possible combinations of project, select, sort
    // uses integer, but could use binary
    // will be 0 for off and 1 for on for each position
    int iDeriveType=0;
    if (iNumSort > 0) iDeriveType += 1;
    if (iNumSel > 0) iDeriveType += 10;
    if (iNumProj > 0) iDeriveType +=100;
    if (iNumSelR > 0) iDeriveType += 1000;
    switch (iDeriveType)
    {
        case 0:
            // do nothing
            m_ActiveViewHandle[iDerView] = m_ActiveViewHandle[iRawView];
            break;
        case 1:
            // do sort, no projection or selection
            m_ActiveViewHandle[iDerView] = m_ActiveViewHandle[iRawView].SortOn(vColSort);
            break;
        case 10:
            // do selection, no sort or projection
            m_ActiveViewHandle[iDerView] = m_ActiveViewHandle[iRawView].Select(m_ActiveViewHandle[iSelIndex].GetAt(0));
            break;
        case 11:
            // do sort and selection, no projection
            m_ActiveViewHandle[iDerView] = m_ActiveViewHandle[iRawView].Select(m_ActiveViewHandle[iSelIndex].GetAt(0)).SortOn(vColSort);
            break;
        case 100:
            // do projection, no sort or selection
            m_ActiveViewHandle[iDerView] = m_ActiveViewHandle[iRawView].Project(vColSubset);
            break;
        case 101:
            // do sort and projection, no selection
            m_ActiveViewHandle[iDerView] = m_ActiveViewHandle[iRawView].SortOn(vColSort).Project(vColSubset);
            break;
        case 110:
            // do selection and projection, no sort
            m_ActiveViewHandle[iDerView] = m_ActiveViewHandle[iRawView].Select(m_ActiveViewHandle[iSelIndex].GetAt(0)).Project(vColSubset);
            break;
        case 111:
            // do selection, projection and sort
            m_ActiveViewHandle[iDerView] = m_ActiveViewHandle[iRawView].Select(m_ActiveViewHandle[iSelIndex].GetAt(0)).SortOn(vColSort).Project(vColSubset);
            break;
        case 1000:
            // do range select
            m_ActiveViewHandle[iDerView] = m_ActiveViewHandle[iRawView].SelectRange(m_ActiveViewHandle[iSelRIndex].GetAt(0),m_ActiveViewHandle[iSelRIndex].GetAt(1));
            break;
        case 1001:
            // do range select, sort, no projection or selection
            m_ActiveViewHandle[iDerView] = m_ActiveViewHandle[iRawView].SelectRange(m_ActiveViewHandle[iSelRIndex].GetAt(0),m_ActiveViewHandle[iSelRIndex].GetAt(1)).SortOn(vColSort);
            break;
        case 1010:
            // do range select, selection, no sort or projection
            m_ActiveViewHandle[iDerView] = m_ActiveViewHandle[iRawView].Select(m_ActiveViewHandle[iSelIndex].GetAt(0)).SelectRange(m_ActiveViewHandle[iSelRIndex].GetAt(0),m_ActiveViewHandle[iSelRIndex].GetAt(1));
            break;
        case 1011:
            // do range select, sort and selection, no projection
            m_ActiveViewHandle[iDerView] = m_ActiveViewHandle[iRawView].Select(m_ActiveViewHandle[iSelIndex].GetAt(0)).SelectRange(m_ActiveViewHandle[iSelRIndex].GetAt(0),m_ActiveViewHandle[iSelRIndex].GetAt(1)).SortOn(vColSort);
            break;
        case 1100:
            // do range select, projection, no sort or selection
            m_ActiveViewHandle[iDerView] = m_ActiveViewHandle[iRawView].SelectRange(m_ActiveViewHandle[iSelRIndex].GetAt(0),m_ActiveViewHandle[iSelRIndex].GetAt(1)).Project(vColSubset);
            break;
        case 1101:
            // do range select, sort and projection, no selection
            m_ActiveViewHandle[iDerView] = m_ActiveViewHandle[iRawView].SelectRange(m_ActiveViewHandle[iSelRIndex].GetAt(0),m_ActiveViewHandle[iSelRIndex].GetAt(1)).SortOn(vColSort).Project(vColSubset);
            break;
        case 1110:
            // range select, do selection and projection, no sort
            m_ActiveViewHandle[iDerView] = m_ActiveViewHandle[iRawView].Select(m_ActiveViewHandle[iSelIndex].GetAt(0)).SelectRange(m_ActiveViewHandle[iSelRIndex].GetAt(0),m_ActiveViewHandle[iSelRIndex].GetAt(1)).Project(vColSubset);
            break;
        case 1111:
            // range select, do selection, projection and sort
            m_ActiveViewHandle[iDerView] = m_ActiveViewHandle[iRawView].Select(m_ActiveViewHandle[iSelIndex].GetAt(0)).SelectRange(m_ActiveViewHandle[iSelRIndex].GetAt(0),m_ActiveViewHandle[iSelRIndex].GetAt(1)).SortOn(vColSort).Project(vColSubset);
            break;
    }
    
    // ???diagnostic to export view to see if derive worked
    // ExportView(DATATYPE_CSVWHDR,"DeriveTest.txt", derview, DATAMODE_REPLACE);

    if (iSelIndex >=0)
    {
        CloseAView(iSelIndex);    // clean up temp view stuff
        iSelIndex = -1;
    }
    
    if (iSelRIndex >=0)
    {
        CloseAView(iSelRIndex);    // clean up temp view stuff
        iSelRIndex = -1;
    }
    
    return iDerView;
}//@CODE_387


/*@NOTE_545
Export all of the first level views as mk4 export unless export to access is
requested. Will simply call the ExportView() function once for each first level
view with overwrite on the first and append for the remaining. Will ALWAYS make
a backup and start from scratch to avoid any complex merge logic.
*/
int CMk4DataContainer::ExportAllViews(const CString FileName, int iDataType)
{//@CODE_545
    int value = 0;
#ifndef EXCLUDEIMPORTEXPORT
    // rename backup file name
    CString sTemp = FileName;
    CString sBackup = sTemp;
    RenameAsBackup(sTemp);
    // loop through all first level views and dump them to the export file
    // first make sure the list of tables is current
    BuildListOfViews();
    // then parse and call the export function for each view
    CString sViewsToExport = m_sListOfFirstLevelTables;
    CString sToken;
    NextToken(sViewsToExport, sToken, ",");
    ExportView(iDataType, FileName, sToken, DATAMODE_REPLACE);
    while (sViewsToExport.GetLength() > 0) 
    {
        NextToken(sViewsToExport, sToken, ",");
        // replace table if Access, or append if text
        if (iDataType == DATATYPE_MSACCESS)
        {
            ExportView(iDataType, FileName, sToken, DATAMODE_REPLACE);
        }
        else 
        {
            ExportView(iDataType, FileName, sToken, DATAMODE_APPEND);
        }
    }
#endif // EXCLUDEIMPORTEXPORT
    return value;
}//@CODE_545


/*@NOTE_583
Check the metakit description to determine if a view exists. Simply looks for
string in the mk4 description and returns the index in the description where
found, or -1 if not found. Can return a false positive if the checked view name
is part of another view name since it is only doing a find substring.
*/
int CMk4DataContainer::DoesViewExist(const char* ptrAViewName)
{//@CODE_583
    //CString Description;    // hold mk4 data structure
    CString sTemp, sToken;
    int iFound = -1; // location in string if found
    int iOrigLength, iNewLength;
    
    if (m_iDBStatus == DB_NOT_AVAIL)
    {
        // error if store not open
        return iFound;
    }
    
	CString sViewName = ptrAViewName;

    //Description = m_ptrDataStore->Description();    // get current structure
    sTemp = m_sListOfFirstLevelTables; //m_ptrDataStore->Description();    // get current structure
    iOrigLength = sTemp.GetLength();
    while (sTemp.GetLength() > 0)
    {
        iNewLength = sTemp.GetLength();  // for calculating position
        NextToken(sTemp, sToken,_T(",[]:"));  // get the next view name from string
        if (!sToken.CompareNoCase(sViewName))  // if it is found, break ptrAViewName
        {
            iFound = iOrigLength - iNewLength; // calculate position in string
            return iFound;
        }
    }
    //iFound = Description.Find((LPCTSTR)ptrAViewName);    // find the view in structure
    
    return iFound;
}//@CODE_583


/*@NOTE_598
Import all tables from Access file or the CSV file that was written with this
program. Default will be the MK4 dump unless specifically requested as Access
import. Will simply call the ImportView() function once for each first level
view. Will ALWAYS make a backup and start from scratch to avoid any complex
merge logic.
*/
int CMk4DataContainer::ImportAllViews(CString sFileName, int iDataType,
                                      CString sFileNameForDB)
{//@CODE_598
    int value = 0;
#ifndef EXCLUDEIMPORTEXPORT
    CString sImportedFileName,sBackupFileName;
    FILE* pfile = NULL;    // handle for text file input
    bool bKeepLooking = TRUE;    // flag used in mk4 load for searching
    CString sLine;      // a line read from the file
    CString sToken;      // temp store to hold the next token
    CString sListOfTables;  // list of tables parsed from input
    
    // close any db that may be open
    CloseDB();

    // Then build a name for the import file based on the original with .MK4 appended
    if (sFileNameForDB.GetLength() <= 0) {
        sImportedFileName = sFileName + ".MK4";
    }
    else
    {
        sImportedFileName = sFileNameForDB;
    }
    
    // rename as backup if it already exists
    sBackupFileName = sImportedFileName;
    RenameAsBackup(sBackupFileName);
    
    // open a new metakit file with that name
    OpenDBFile(sImportedFileName);
    
    // get the data from the file into the newly created file
    if (iDataType == DATATYPE_MK4)
    {
        // importing a MK4 dump file
        // build a list of view names and call the import function for each
        // open the text file
        pfile = fopen((const char*)sFileName.operator LPCTSTR(), "r");
        if (pfile == NULL)
        {
            LogMessage("Can not open import file ", "W", "");
            value = -1;
            goto finishup;
        };
        // Look for any table names in the file
        ReadLine(pfile, sLine);
        while (!feof(pfile) && bKeepLooking) 
        {
            if (sLine.GetLength()>0 && sLine[0] == '[')
            {
                // This starts a view block, so get name
                sLine.Delete(0, 1);
                NextToken(sLine, sToken, "]");
                if (sListOfTables.GetLength() > 0)
                    sListOfTables +=",";
                sListOfTables += sToken;
            }
            ReadLine(pfile, sLine);
        }
        fclose(pfile);
        pfile = NULL;
        // now call the import function once for each view
        while (sListOfTables.GetLength() > 0) 
        {
            NextToken(sListOfTables, sToken);
            ImportView(iDataType, sFileName, sToken, DATAMODE_REPLACE);
        }
        SaveData();
    }
    else if (iDataType == DATATYPE_MSACCESS)
    {
        // import a MS Access file
        // build a list of view names and call the import function
    }
    
finishup:
    // close the file
    if (pfile != NULL)
        fclose(pfile);
    // close the db
    CloseDB();
    
#endif // EXCLUDEIMPORTEXPORT
    return value;
}//@CODE_598


/*@NOTE_605
Rename a first level view.
*/
int CMk4DataContainer::RenameView(CString sOldName, CString sNewName)
{//@CODE_605
    int value=0;
    // rename the backup view and delete original
    char * ptrOldName = NULL;
    char * ptrNewName = NULL;
    StringToChar(sOldName, &ptrOldName);
    StringToChar(sNewName, &ptrNewName);
    c4_ViewProp oldname((const char *)ptrOldName);
    c4_ViewProp newname((const char *)ptrNewName);
    m_ptrDataStore->Rename(oldname, newname);
    DELETEANDNULLALL(ptrOldName);
    DELETEANDNULLALL(ptrNewName);

    return value;
}//@CODE_605


int CMk4DataContainer::CopyViewFromFileToMemory(CString sViewNameOnFile,
                                                int iViewIndexInMemory)
{//@CODE_620
    int value = -1;
    char * ptrAViewName = NULL;
    StringToChar(sViewNameOnFile, &ptrAViewName);
    int iViewInFile;
    
    // open the storage view not using OpenAView member, but with raw MK4
    iViewInFile = FindFreeViewIndex("Temp");  // find slot in view array
    
    // make a copy of it
    if (iViewInFile >= 0)  // found slot, so make copy
    {
        m_ActiveViewHandle[iViewInFile] = m_ptrDataStore->View(ptrAViewName);
        m_ActiveViewHandle[iViewIndexInMemory] = m_ActiveViewHandle[iViewInFile].Clone();
        m_ActiveViewHandle[iViewIndexInMemory].InsertAt(0,m_ActiveViewHandle[iViewInFile]);
        value = iViewIndexInMemory;
        CloseAView(iViewInFile);  // clean up storage view since it was copied to mem
    }

    DELETEANDNULLALL(ptrAViewName);  // clean up allocated char array
    return value;
}//@CODE_620


int CMk4DataContainer::CopyViewFromMemoryToFile(CString sViewNameOnFile,
                                                int iViewIndexInMemory)
{//@CODE_621
    int value = -1;

    char * ptrAViewName = NULL;
    StringToChar(sViewNameOnFile, &ptrAViewName);
    int iViewInFile;
    bool bNeedToClose = FALSE;
    
    // check to see if the storage is open
    if (m_ptrDataStore == NULL)
    {
        OpenStorage();
        bNeedToClose = TRUE;
    }
    
    // open the storage view not using OpenAView member, but with raw MK4
    iViewInFile = FindFreeViewIndex("Temp");  // find slot in view array
    
    // replace contents with memory data
    if (iViewInFile >= 0)  // found slot, so make copy
    {
        m_ActiveViewHandle[iViewInFile] = m_ptrDataStore->View(ptrAViewName); // open
        m_ActiveViewHandle[iViewInFile].SetSize(0); // throw away old records
        m_ActiveViewHandle[iViewInFile].InsertAt(0,m_ActiveViewHandle[iViewIndexInMemory]);
        value = iViewIndexInMemory;
        CloseAView(iViewInFile);  // clean up storage view since it was copied to mem
    }

    DELETEANDNULLALL(ptrAViewName);  // clean up allocated char array

    //close the storage if needed
    if (bNeedToClose)
    {
        // make the data permenant
        m_ptrDataStore->Commit();
        DELETEANDNULL(m_ptrDataStore);
    }

    return value;
}//@CODE_621


/*@NOTE_630
Make a temporary two column unattached view from a row in another view. The
first column will be the heading, and the second will be the value. There will
be as many rows in this view as columns in the raw view. Returns the view index
of the created view. In lieu of creating a form for form based data entry (ie.
one dialog per record), transpose it into a view so it we can use a grid for
data entry; the first column will be the field name (or even the long
description from data dict) and the second column will be value for that field.
*/
int CMk4DataContainer::TransposeFromRowToUnattachedView(CString sRawViewName,
                                                        long lRowIndex,
                                                        CString sNewViewName)
{//@CODE_630
    int value = -1;  // return value of created view index
    int iNumCol;  // the number of columns in raw view
    int iRawView; // the index of the raw view
    long i;  // loop index
    CString sTemp, sTemp2;  // for getting and setting values from views
    
    // look up the raw view index
    iRawView = FindViewIndexByName(sRawViewName);
    if (iRawView<0) return value;  // not open, so abort
    
    // find an open view index in the data container
    CString derview;
    char* ptrTemp = NULL;
    char* ptrTemp2 = NULL;
    if (sNewViewName.GetLength() == 0)
    {
        // none supplied so use default
        derview.Format(_T("Row%d%s"),lRowIndex,sRawViewName);    // make a name for the derived view
    }
    else 
    {
        derview = sNewViewName;
    }
    StringToChar(derview, &ptrTemp);
    //StringToChar(sRawViewName, &ptrTemp2);
    value = FindFreeViewIndex(ptrTemp);
    DELETEANDNULLALL(ptrTemp);
    //DELETEANDNULL(ptrTemp2);
    
    // create the view
    c4_StringProp pItem("Item");
    c4_StringProp pValue("Value");
    //c4_Row row;
    //m_ActiveViewHandle[value] = c4_View.
    //m_ActiveViewHandle[value].AddProperty(pItem);
    //m_ActiveViewHandle[value].AddProperty(pValue);
    
    // loop through columns of raw view and create rows in derived view
    iNumCol = m_ActiveViewHandle[iRawView].NumProperties();
    //m_ActiveViewHandle[value].SetSize(iNumCol);
    for (i=0; i<iNumCol; i++)
    {
        sTemp = m_ActiveViewHandle[iRawView].NthProperty(i).Name();
        //SetDataItemFrom(value, i, 0, sTemp);
        GetDataItemAs(iRawView, lRowIndex, i, sTemp2);
        //SetDataItemFrom(value, i, 1, sTemp);
        StringToChar(sTemp, &ptrTemp);
        StringToChar(sTemp2, &ptrTemp2);
        //pItem(row) = ptrTemp;
        //pValue(row) = ptrTemp2;
        //m_ActiveViewHandle[value].Add(pItem[sTemp] + pValue[sTemp2]);
        m_ActiveViewHandle[value].Add(pItem[ptrTemp] + pValue[ptrTemp2]);
        //m_ActiveViewHandle[value].Add(row);
        DELETEANDNULLALL(ptrTemp);
        DELETEANDNULLALL(ptrTemp2);
    }
    

    return value;
}//@CODE_630


/*@NOTE_634
Complement function to save the data back to the raw view. The temporary view
will be destroyed after the data is saved.
*/
int CMk4DataContainer::TransposeFromUnattachedViewToRowAndClose(int iViewIndex,
                                                                CString sRawViewName,
                                                                long lRowIndex)
{//@CODE_634
    int value = -1;  // return value
    int iNumCol;  // the number of columns in raw view
    int iRawView; // the index of the raw view
    int i;  // loop index
    CString sTemp;  // for getting and setting values from views
    
    // look up the raw view index
    iRawView = FindViewIndexByName(sRawViewName);
    if (iRawView<0) return value;  // not open, so abort
    
    // loop through rows of derived view and put values in cols of raw view
    iNumCol = m_ActiveViewHandle[iRawView].NumProperties();
    value = iNumCol;
    // assume a 1 to 1 mapping of derived view rows to raw view columns
    for (i=0; i<iNumCol; i++)
    {
        GetDataItemAs(iViewIndex, i, 1, sTemp);
        SetDataItemFrom(iRawView, lRowIndex, i, sTemp);
    }
    CloseAView(iViewIndex);
    return value;
}//@CODE_634


/*@NOTE_642
Main function is to rename a column in a view without loosing existing data in
the column. The view must not be open; if open, it will be closed and not open
on return. And, the view must be in the data dictionary with current
specifications. Optionally, you can change the field type in the process by
supplying the new type. But, you cannot change the field type without changing
the name.
*/
int CMk4DataContainer::RedefineAColumn(CString sViewContainingColumn,
                                       CString sOldColumnName,
                                       CString sNewColumnName,
                                       char cNewFieldType)
{//@CODE_642
    int iColumnIndexOld, iColumnIndexNew; // prop name for old and new
    char cPropTypeForNew, cPropTypeForOld; // property type for old and new
    int iNumRows; // number of rows in the view
    long i;  // loop variable
    CString sTemp;
    int iViewIndexDD=-1, iViewIndexTargetTable=-1, iDerivedViewIndexDD=-1;
    bool bNeedToCloseDD = FALSE; // if dd was not open, will need to close
    CString sSelectionCriteria; // for building query
    int iRet=0;  // return value
    int iRows;
    int iCols;
	char* cNewColumnName = NULL;
    
    // make sure the names are different and if user wants type change
    if (!sOldColumnName.CompareNoCase(sNewColumnName))
    {
        iRet = -1;
        goto finishup;
    }
    
    // get the index of the dd, or if not open, open it
    iViewIndexDD = FindViewIndexByName(m_sDDViewName);
    if (iViewIndexDD < 0)
    {
        iViewIndexDD = OpenAView(m_sDDViewName);
        bNeedToCloseDD = TRUE;
    }
    if (iViewIndexDD < 0)
    {
        iRet = -2;
        goto finishup;
    }
    
    // open the view, it should be closed, but check just in case
    iViewIndexTargetTable = FindViewIndexByName(sViewContainingColumn);
    if (iViewIndexTargetTable < 0)
    {
        iViewIndexTargetTable = OpenAView(sViewContainingColumn);
    } 
    else if (m_iParentIndex[iViewIndexTargetTable]>=0)
    {  // this is derived, so error
        iRet = -3;
        goto finishup;
    }
    // if still not open, does not exist so error
    if (iViewIndexTargetTable < 0)
    {
        iRet = -4;
        goto finishup;
    }
    
    // create and add the new property
    iColumnIndexNew = FindColIndexByName(iViewIndexTargetTable ,sNewColumnName);
    if (iColumnIndexNew >= 0) // new name already exists
    {
        iRet = -5;
        goto finishup;
    }
    iColumnIndexOld = FindColIndexByName(iViewIndexTargetTable ,sOldColumnName);
    if (iColumnIndexOld < 0) // cant find old column name
    {
        iRet = -6;
        goto finishup;
    }
    cPropTypeForOld = m_ActiveViewHandle[iViewIndexTargetTable].NthProperty(iColumnIndexOld).Type();
    //if (cNewFieldType != ' ')
        //cPropTypeForNew = cNewFieldType;
    //else
        cPropTypeForNew = cPropTypeForOld;

    if (TRUE)
    {  // had to put this inside if to avoid compile error
	   // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vccore98/html/c2362.asp
		StringToChar(sNewColumnName, &cNewColumnName);
        c4_Property oProp(cPropTypeForNew, cNewColumnName);
        m_ActiveViewHandle[iViewIndexTargetTable].AddProperty(oProp);
    }

   iColumnIndexNew = FindColIndexByName(iViewIndexTargetTable ,cNewColumnName);

   DELETEANDNULLALL(cNewColumnName);

   if (iColumnIndexNew < 0) // did not add it
   {
        iRet = -7;
        goto finishup;
   }

    // copy all of the data from old column to the new column
   iNumRows = m_ActiveViewHandle[iViewIndexTargetTable].GetSize();
   for (i=0; i<iNumRows; i++)
   {
       GetDataItemAs(iViewIndexTargetTable, i, iColumnIndexOld, sTemp);
       SetDataItemFrom(iViewIndexTargetTable, i, iColumnIndexNew, sTemp);
   }
    
   // update the data dictionary for this row/column
    sSelectionCriteria.Format(_T("TableName=%s,ColumnName=%s"),sViewContainingColumn,sOldColumnName);
    sTemp.Format(_T("%s,%s"), m_sDDColNameForColumnName, m_sDDColNameForFType); // proj list
    iDerivedViewIndexDD = DeriveView(m_sDDViewName,sTemp,sSelectionCriteria);
    if (iDerivedViewIndexDD==-1)
    {   // did not derive view
        iRet = -8;
        goto finishup;
    }
    iRows = m_ActiveViewHandle[iDerivedViewIndexDD].GetSize();
    iCols = m_ActiveViewHandle[iDerivedViewIndexDD].NumProperties();
    if (m_ActiveViewHandle[iDerivedViewIndexDD].GetSize()!=1
        ||m_ActiveViewHandle[iDerivedViewIndexDD].NumProperties()!=2)
    {   // should only be one row and one column
        iRet = -9;
        goto finishup;
    }
    SetDataItemFrom(iDerivedViewIndexDD, 0, 0, sNewColumnName);
    //sTemp.Format("%c", cPropTypeForNew);
    //SetDataItemFrom(iDerivedViewIndexDD, 0, 1, sTemp);
    
   // now close the view, remake it from dd, and new column should be there
    CloseAView(iViewIndexTargetTable);
    iViewIndexTargetTable=-1; // reset index to closed

	char* cViewContainingColumn;
	StringToChar(sViewContainingColumn, &cViewContainingColumn);

    MakeViewFromDataDictionary(cViewContainingColumn, "Order1");

	DELETEANDNULLALL(cViewContainingColumn);

    SaveData();

    
   // clean up things that were opened
finishup:
    if (iDerivedViewIndexDD >=0) CloseAView(iDerivedViewIndexDD);
    if (bNeedToCloseDD && iViewIndexDD >=0) CloseAView(iViewIndexDD);
    if (iViewIndexTargetTable >=0) CloseAView(iViewIndexTargetTable);

    return iRet;
}//@CODE_642


// End of Group: ViewManagement


//
// Group: Utility
//
/*@NOTE_408
Accepts a cstring message and parses out next token. Returns the token in a
cstring and the strips it off of the original message. Note: this is a
DESTRUCTIVE return in the sense that the input message has the token removed.
You can optionally specify the delimiter, currently a comma.
*/
void CMk4DataContainer::NextToken(CString& sMessage, CString& sToken,
                                  const CString sDelimiters)
{//@CODE_408
    int iLoc = sMessage.FindOneOf(sDelimiters);  // look for the first field delimiter
    if (iLoc >= 0)
    {
        sToken = sMessage.Left(iLoc);   // pull off everything to delimiter
        sMessage.Delete(0, iLoc + 1);    // remove token and delimiter from string
    }
    else 
    {
        sToken = sMessage;  // no delimiter, so use entire string
        sMessage = TEXT("");    // remove token and delimiter from string
    }
    sToken.TrimLeft();  // remove leading and trailing whitespace, ie. space, tab, EOL
    sToken.TrimRight();  // remove leading and trailing whitespace, ie. space, tab, EOL
    
    return;
}//@CODE_408


/*@NOTE_403
Read a line of text from an open ASCII file and stuff it into a CString. Need
to open the file, but do not need to prime pump with getch(). When this
returns, the pointer will be at the LF of the CR/LF eol, so the first thing it
does is getch.
*/
int CMk4DataContainer::ReadLine(FILE* pfile, CString& sLineRead)
{//@CODE_403
    int value=-1;   // set up return value
    sLineRead = TEXT("");
    char ch = ' ';
    while (!feof(pfile) && ch != 10) 
    { // 10=LF, so quit reading on eol or eof
        ch = (char)(fgetc(pfile));
        // add the char to the buffer, but do not store eol, 13=CR, so just skip it
        if (ch != 13 && ch != 10)
            sLineRead = sLineRead + ch; 
    };
    value = sLineRead.GetLength();
    
    return value;
}//@CODE_403


/*@NOTE_392
Log a message into the message log view.  The message can be either a number or
CString. The entry will be automatically time stamped.  May want to encrypt for
secure audit trail.
*/
int CMk4DataContainer::LogMessage(const int MsgNum, const CString MsgType,
                                  const CString Note, CString sProgram)
{//@CODE_392
	CString Msg;
	Msg.Format(_T("%d"), MsgNum);
    return LogMessage(Msg, MsgType, Note, sProgram);
}//@CODE_392


/*@NOTE_395
Log a message into the message log view.  The message can be either a number or
CString. The entry will be automatically time stamped.  May want to encrypt for
secure audit trail.
*/
int CMk4DataContainer::LogMessage(const CString sMsg, const CString sMsgType,
                                  const CString sNote, CString sProgram)
{//@CODE_395
    int value = -1;
    int iLogView;   // the message log view index
    long lRowNum;   // the row number just added
    int iCol; // a column pointer
    bool bNeedToClose = FALSE;
    char cTemp[] = "MessageLog[MsgNum:S,Msg:S,Date:S,Time:S,Level:S,Note:S,Program:S]";
    CString sLogViewName = TEXT("MessageLog");  // name of view to record message in
    CString sTime;
    CString sDate;
    // struct tm *today;

    // open up the log view in the file if it exists
    // see if log view is already open
    iLogView = FindViewIndexByName(sLogViewName); // index of the log view
    if (iLogView < 0)
    {
        // if not already open, then try to open it if it exists on file
        iLogView = OpenAView(sLogViewName);
        bNeedToClose = TRUE;
    }
    if (iLogView < 0)
    {
        // if view not in file, then create it
        iLogView = FindFreeViewIndex(cTemp);// sMk4Structure
        m_ActiveViewHandle[iLogView] = m_ptrDataStore->GetAs(cTemp);// sMk4Structure
        bNeedToClose = TRUE;
    }
    if (iLogView < 0)
    {
        // if still not open, a problem
        return value;
    }
    
//#ifdef q4_WINCE
    
    SYSTEMTIME oTime;
    GetLocalTime(&oTime);
    
    // Format the time
    sTime.Format(TEXT("%02d:%02d"), oTime.wHour, oTime.wMinute);
    
    // Format the date
    sDate.Format(TEXT("%02d/%02d/%d"), oTime.wMonth, oTime.wDay, oTime.wYear);
    
/*
#else
    time_t ltime;
    int iTemp;    
    // get the system date and time stamp
    time(&ltime);
    sDate = ctime(&ltime);
    // ctime leaves some funky char on the end, so strip off
    iTemp = sDate.GetLength() - 1;
    sDate.Delete(iTemp);
#endif
*/
    
    /*
    // Here are some alternate ways to get the time
    // 1. use the CTime class
    CTime theTime = CTime::GetCurrentTime();
    
      / 2. or use the time functions
      // Convert to time structure and adjust for PM if necessary.
      today = localtime(&ltime);
      if (today->tm_hour > 12)
      {
      strcpy(ampm, "PM");
      today->tm_hour -= 12;
      }
      if (today->tm_hour == 0)  // Adjust if midnight hour.
      today->tm_hour = 12;
      
        // Use strftime to build a customized time string.
        strftime(tmpbuf, 128, "%A, %d %B %Y", today);
    */
    
    // append a record and log the message
    lRowNum = AppendARow(iLogView);
    // fill data into fields if they exist
    iCol = FindColIndexByName(iLogView,"Msg");
    if (iCol>=0)
        SetDataItemFrom(iLogView, lRowNum, iCol, sMsg);
    iCol = FindColIndexByName(iLogView,"Date");
    if (iCol>=0)
        SetDataItemFrom(iLogView, lRowNum, iCol, sDate);
    iCol = FindColIndexByName(iLogView,"Time");
    if (iCol>=0)
        SetDataItemFrom(iLogView, lRowNum, iCol, sTime);
    iCol = FindColIndexByName(iLogView,"Level");
    if (iCol>=0)
        SetDataItemFrom(iLogView, lRowNum, iCol, sMsgType);
    iCol = FindColIndexByName(iLogView,"Note");
    if (iCol>=0)
        SetDataItemFrom(iLogView, lRowNum, iCol, sNote);
    iCol = FindColIndexByName(iLogView,"Program");
    if (iCol>=0)
        SetDataItemFrom(iLogView, lRowNum, iCol, sProgram);
    SaveData();
    if (bNeedToClose)
        CloseAView(iLogView);
    value = 0;
    
    return value;
}//@CODE_395


/*@NOTE_415
Create a list of columns from DD that can be used in a metakit projection. This
list is comma delimited made from an ordering specified in a column of the DD.
*/
int CMk4DataContainer::MakeProjectionList(CString& sProjectionList,
                                          const CString& sTableName,
                                          const CString& sOrderColumn)
{//@CODE_415
    int value = 0;
    char* ptrTemp = NULL;  // working variable
    //char* ptrTemp2;  // working variable
    CString sTemp;  // some temporary string handling
    CString sTemp2;  // some temporary string handling
    sProjectionList.Empty();  // clean out any old list
    int idddvindex;  // index for dd
    
    // if the dd is open, look up the column ordering
    int iDDView = FindViewIndexByName(m_sDDViewName); // index of the dd view
    bool bOpened = FALSE;   // flag that indicates opened here (TRUE), or already open (FALSE)
    if (iDDView < 0)
    {
        iDDView = OpenAView(m_sDDViewName);
        bOpened = TRUE; // flag so that it can be closed after setup
    }
    
    // Pull out the data defs for the requested table from the DD
    
    // derive a table from DD to pull defs from
    /*  replaced below with call to derive view
    StringToChar(m_sDDColNameForTable, &ptrTemp);   // the name of the DD col for table name
    c4_StringProp spTable(ptrTemp); // an mk4 string prop for table name column
    DELETEANDNULL(ptrTemp);
    CString derddview = TEXT("DDD");    // make a name for the derived DD view
    StringToChar(derddview, &ptrTemp);
    StringToChar(m_sDDViewName, &ptrTemp2);
    idddvindex = FindFreeViewIndex(ptrTemp, ptrTemp2);
    DELETEANDNULL(ptrTemp);
    DELETEANDNULL(ptrTemp2);
    StringToChar(sOrderColumn, &ptrTemp);
    c4_IntProp ip(ptrTemp);    // an mk4 property to sort on
    DELETEANDNULL(ptrTemp);
    StringToChar(sTableName, &ptrTemp);
    m_ActiveViewHandle[idddvindex] = m_ActiveViewHandle[iDDView].Select(spTable[ptrTemp]).SortOn(ip);
    DELETEANDNULL(ptrTemp);
    */
    // replace with the derive a view
    sTemp = m_sDDColNameForTable + "=" + sTableName;  // select criteria for table
    idddvindex = DeriveView("DataDictionary", "", sTemp, sOrderColumn, "DDDInMakeProjection");
    
    // get the number of definitions for cols or properties in this view
    int iNumCols = m_ActiveViewHandle[idddvindex].GetSize();
    
    // create a CSV list of definitions in the dd for requested table
    StringToChar(m_sDDColNameForColumnName, &ptrTemp);
    int iFieldNameIndex = m_ActiveViewHandle[idddvindex].FindPropIndexByName(ptrTemp);  // the attirbute index for the field name
    DELETEANDNULLALL(ptrTemp);
    StringToChar(sOrderColumn, &ptrTemp);
    int iFieldOrderIndex = m_ActiveViewHandle[idddvindex].FindPropIndexByName(ptrTemp);  // the attirbute index for the field order
    DELETEANDNULLALL(ptrTemp);
	StringToChar(m_sDDColNameForIsUsed, &ptrTemp);
    int iFieldIsUsedIndex = m_ActiveViewHandle[idddvindex].FindPropIndexByName(ptrTemp);  // the attirbute index for is used
    DELETEANDNULLALL(ptrTemp);   
	int iOrder=0; // the actual value of the order attribute from the DD
    
    // iterate through the properties and build CString list
    for (int i = 0; i < iNumCols; i++)
    {
        // get the field name and add corresponding property if order is > 0
        if (sOrderColumn.IsEmpty())
            iOrder=1;
        else
            GetDataItemAs(idddvindex, i, iFieldOrderIndex, &iOrder);  // get field order from dd
        GetDataItemAs(idddvindex, i, iFieldIsUsedIndex, sTemp);  // get field name from dd
        if (sTemp[0] == 'N')
            iOrder = 0;  // do not use if IsUsed set to N

        if (iOrder > 0)
        {
            GetDataItemAs(idddvindex, i, iFieldNameIndex, sTemp);  // get field name from dd
            // if it has an order, then add to list
            value++;    // increment count of attributes in list
            if (sProjectionList.GetLength() > 0)
            {
                // something in list, so add comma, then item
                sProjectionList = sProjectionList + TEXT(", ");
                sProjectionList = sProjectionList + sTemp;
            }
            else 
            {  // nothing in list, so add item, no comma  
                sProjectionList = sTemp;
            }
        }
    }
    
    
    // if the dd view had to be opened here, then close it
    if (bOpened)
        CloseAView(iDDView);
    CloseAView(idddvindex);  // close the derived dd view
    
    return value;
}//@CODE_415


/*@NOTE_419
Take an empty view(or empty it), a parent view, and a CString list, and make an
empty view template from it. The primary use of this template is as the
argument for projections or sorting.
*/
int CMk4DataContainer::MakeViewTemplateFromList(c4_View& ViewTemplate,
                                                c4_View& ParentView,
                                                const CString& sAttributeList)
{//@CODE_419
    char* ptrTemp = NULL;    // working variable
    int iNumCols = 0;    // number of attributes copied to template for return
    if (sAttributeList.GetLength() > 0)
    {
        // if there is projection list, use it
        CString sColList = sAttributeList;
        CString sToken;
        while (sColList.GetLength() > 0) 
        {
            iNumCols++;
            NextToken(sColList, sToken);
            StringToChar(sToken, &ptrTemp);
            ViewTemplate.AddProperty(
                ParentView.NthProperty(ParentView.FindPropIndexByName(ptrTemp))
                );
            DELETEANDNULLALL(ptrTemp);
        }
    }
    else 
    {    // else use all columns
        int iNumCols = ParentView.NumProperties();
        for (int i = 0; i < iNumCols; i++)
        {
            ViewTemplate.AddProperty(ParentView.NthProperty(i));
        }
    }
    
    return iNumCols;
}//@CODE_419


/*@NOTE_193
Pop up a message to the user; can be used for error handling. Replace this with
the Log message function.
*/
void CMk4DataContainer::PopMsg(CString Message, int ErrCode)
{//@CODE_193
#ifndef q4_WINCE
    // CString MsgTitle;
    CString MsgContent;
    
    if (ErrCode == ERR_NONE)
    {
        // MsgTitle = "No Error";
        MsgContent = "Operation completed successfully.\n";
        MsgContent += Message;
    }
    else 
    {
        // MsgTitle = "Error";
        MsgContent.Format(_T("Internal Error.\nPlease Contact Developer.\nError # %d\n"), ErrCode);
        MsgContent += Message;
    }
    AfxMessageBox(MsgContent, NULL, MB_OK | MB_ICONINFORMATION);
    
#endif    
}//@CODE_193


/*@NOTE_502
Utility function to trim all string fields in a table.  Since selections, etc.
are sensitive to extra spaces on front or rear of strings, make one call to
this function to trim all strings in a table or view.
*/
int CMk4DataContainer::TrimAllStringsInTable(const CString& sTableToTrim)
{//@CODE_502
    // Open the view if not already open
    int iViewIndex; // the index of the view to open
    bool bNeedToClose = FALSE;
    char* ptr_cTemp = NULL;
    StringToChar(sTableToTrim, &ptr_cTemp);
    if (!ViewIsOpen(ptr_cTemp, &iViewIndex))
    {
        iViewIndex = OpenAView(ptr_cTemp);
        bNeedToClose = TRUE;
    }; // <- why is there a semicolon here?
    DELETEANDNULLALL(ptr_cTemp);
    
    // loop through entire table using the get/set functions which will trim strings
    if (iViewIndex >= 0)
    {
        // only do this if open view
        int iNumCols = m_ActiveViewHandle[iViewIndex].NumProperties();
        int iNumRows = m_ActiveViewHandle[iViewIndex].GetSize();
        int i, j;    // loop variables
        CString RetVal; // string to pass return value around
        for (i = 0; i < iNumCols; i++)
        {
            for (j = 0; j < iNumRows; j++)
            {
                GetDataItemAs(iViewIndex, j, i, RetVal);
                SetDataItemFrom(iViewIndex, j, i, RetVal);
            }
        }
    }
    if (bNeedToClose)
    {    
        char* ptr_cTemp = NULL;
        StringToChar(sTableToTrim, &ptr_cTemp);
        CloseAView(ptr_cTemp);
        DELETEANDNULLALL(ptr_cTemp);
    }
    
    return iViewIndex;
}//@CODE_502


/*@NOTE_508
From a specific column in a specific table, find out all unique values and make
a comma seperated list.
*/
int CMk4DataContainer::MakeListOfUniqueValues(const CString& sTableName,
                                              const CString& sColumnName,
                                              CString& sListUnique)
{//@CODE_508
    int NumInList;   // set up return value
    int ViewIndex;  // the index in data container where view is
    int idvindex;   // array index of derived view in data container
    bool bNeedToClose = FALSE;  // flag to indicate if view shoud be closed
    char* ptrTemp = NULL;  // pointer for conversion of string
    char* ptrTemp2 = NULL;  // pointer for conversion of string
    CString sTemp;
    CString sUniquePlusTableName = "Unique"+sTableName;
    
    // reset the comma list string to empty
    sListUnique = "";
    
    // open the view if not already open
    char* ptr_cTemp = NULL;
    StringToChar(sTableName, &ptr_cTemp);    
    if (!ViewIsOpen(ptr_cTemp, &ViewIndex))
    {
        ViewIndex = OpenAView(sTableName);
        bNeedToClose = TRUE; // set flag so we can close view after we use it
    }
    DELETEANDNULLALL(ptr_cTemp);
    
    // build a projection mask for just the column(s?) specified
    c4_View vColToSelect;
    StringToChar(sColumnName, &ptrTemp);
    c4_Property p = m_ActiveViewHandle[ViewIndex].NthProperty(
        m_ActiveViewHandle[ViewIndex].FindPropIndexByName(ptrTemp));
    vColToSelect.AddProperty(p);
    DELETEANDNULLALL(ptrTemp);
    
    // derive a view from the original, with only the column specified, ie. a projection
    StringToChar(sTableName, &ptrTemp2);
    StringToChar(sUniquePlusTableName, &ptrTemp);
    idvindex = FindFreeViewIndex(ptrTemp, ptrTemp2);
    m_ActiveViewHandle[idvindex] = m_ActiveViewHandle[ViewIndex].Project(vColToSelect).Unique();
    DELETEANDNULLALL(ptrTemp);
    DELETEANDNULLALL(ptrTemp2);
    
    // for each row in derived view, copy values to comma delimited list
    NumInList = m_ActiveViewHandle[idvindex].GetSize();
    for (int i = 0; i < NumInList; i++)
    {
        GetDataItemAs(idvindex, i, 0, sTemp);
        if (sListUnique.GetLength() <= 0) 
            sListUnique = sTemp;
        else
            sListUnique = sListUnique + ", " + sTemp;
    }
    
    // close table if opened here
    if (bNeedToClose) 
    {
        char* ptr_cTemp = NULL;
        StringToChar(sTableName, &ptr_cTemp);
        CloseAView(ptr_cTemp);
        DELETEANDNULLALL(ptr_cTemp);
    }
    //    char* ptr_cTemp;
    StringToChar(sUniquePlusTableName, &ptr_cTemp);
    CloseAView(ptr_cTemp);   // close the derived view
    DELETEANDNULLALL(ptr_cTemp);
    
    return NumInList;
}//@CODE_508


/*@NOTE_558
Check to see if the file is already open and return an error code if it is. In
MetaKit, if two programs try to write to a file at the same time, it corrupts
the db. Returns 0 if not already open, or -1 if it is.
*/
int CMk4DataContainer::FileAlreadyOpen(const CString sDataFile)
{//@CODE_558
    CFileException e;
    CFile cfile;
    
    // test to see if the file exists, and if not, just return since file cannot be open
    // no need to do this
    //if (FileExists(sDataFile) == FILE_DOESNOT_EXIST)
        //return FILE_NOTALREADY_OPEN;
    
    // test to see if the file can be opened exclusively, error if not
    int iResult = cfile.Open(sDataFile, CFile::shareExclusive, &e);
    if (!iResult)
    {
        if (e.m_cause == CFileException::sharingViolation)
        {
            return FILE_ALREADY_OPEN;
        }
    }
    
    // file opened successfully, so close it
    if (iResult!=0) cfile.Close(); // close it only if open
    return FILE_NOTALREADY_OPEN;
}//@CODE_558


/*@NOTE_560
Check to see if the file exists. Returns 0 if file exists, or -1 if it does
not.
*/
int CMk4DataContainer::FileExists(const CString sDataFile)
{//@CODE_560
    CFileException e;
    CFile cfile;
    
    // see if the file exists
    int iResult = cfile.Open(sDataFile, CFile::modeRead, &e);
    if (!iResult)
    {
        if (e.m_cause == CFileException::fileNotFound)
        {
            return FILE_DOESNOT_EXIST;
        }
    }
    
    // file opened successfully, so it exists, close it
    cfile.Close();
    return FILE_DOES_EXIST;
}//@CODE_560


/*
Check to see if the file exists, and make sure it isn't already open or forbidden. 
Returns 0 if file exists, -1 if it does not, and -2 through -4 for error conditions
not. 
(ie FILE_DOES_EXIST, FILE_DOESNOT_EXIST, FILE_ACCESS_DENIED, FILE_ALREADY_OPEN and 
    FILE_ERROR)
*/
int CMk4DataContainer::FileExistsAndAvailable(const CString sDataFile)
{
    CFileException e;
    CFile cfile;
    
    // see if the file exists
    int iResult = cfile.Open(sDataFile, CFile::modeRead, &e);
    if (!iResult)
    {
        if (e.m_cause == CFileException::fileNotFound)
        {
            return FILE_DOESNOT_EXIST;
        }
		else if (e.m_cause == CFileException::accessDenied)
		{
			return FILE_ACCESS_DENIED;
		}
		else if(e.m_cause == CFileException::sharingViolation)
		{
			return FILE_ALREADY_OPEN;
		}
		else
		{
			return FILE_ERROR;
		}
    }
    else
    {
		// file opened successfully, so it exists, close it
		cfile.Close();
		return FILE_DOES_EXIST;
	}    
}

/*Returns true if the database's status is not DB_NOT_AVAIL*/
bool CMk4DataContainer::DBAvailable()
{
	return m_iDBStatus != DB_NOT_AVAIL;
}

/*@NOTE_595
Takes a fully qualified path name and renames it with a .BAKnn, where the nn is
the next available sequential number that avoids a collision.
*/
int CMk4DataContainer::RenameAsBackup(CString& sFileToRename)
{//@CODE_595
    int value = 0;
    
    CString sBackFileName;
    CFile pCFile;
    bool bKeepLooking = TRUE;
    int iVer;
    // check for existance of file and if it already esists, back it up
    if (FileExists(sFileToRename) == FILE_DOES_EXIST)
    {
        // rename with Bnn extention
        iVer = 0;
        while (bKeepLooking) 
        {
            iVer++;
            sBackFileName.Format(TEXT("%s.BAK%d"), sFileToRename, iVer);
            if (FileExists(sBackFileName) == FILE_DOESNOT_EXIST)
            {
                pCFile.Rename(sFileToRename, sBackFileName);  // rename backup
                bKeepLooking = FALSE;  // reset so we fall out of while
            }
        }
        value = 1;  // set file name changed return value
        sFileToRename = sBackFileName; // Backup file name will be passed back by reference.
    }
    
    return value;
}//@CODE_595


/*@NOTE_602
For an open view, build a fully qualified MK4 data structure statement; this
statement should be valid for use in the GetAs member.
*/
int CMk4DataContainer::DescribeStructure(CString& sStructure, const int iView,
                                         bool bReturnCSVFieldList)
{//@CODE_602
    int value = 0;
    
    CString sTemp, sTempStruct;  // working character strings
    CString sFieldList;  // csv list of field or attribute names
    
    /*  do not use this since derived views do not have, but build from scratch below
    // retrieve and save the table structure
    sTempStruct = m_ptrDataStore->Description(m_ptrActiveTableName[iView]);
    //sTempStruct = m_ActiveViewHandle[iView].Description();
    */
    
    int i;  // loop var
    
    sTempStruct = m_ptrActiveTableName[iView];  // pull off table name
    sTempStruct += "[";  // add prop structure bracket
    int iNumProps = m_ActiveViewHandle[iView].NumProperties();
    
    // add comma seperated description of each prop to temporary string
    for (i=0; i<iNumProps; i++)
    {
        // build mk4 structure list
        if (sTemp.GetLength() > 0) 
            sTemp += ",";  // add comma field seperator
        if (sFieldList.GetLength() > 0) 
            sFieldList += ",";  // add comma field seperator
        sFieldList += m_ActiveViewHandle[iView].NthProperty(i).Name();
        sTemp += m_ActiveViewHandle[iView].NthProperty(i).Name();
        sTemp += ":";
        // build csv list 
        sTemp += m_ActiveViewHandle[iView].NthProperty(i).Type();
       // if (sFieldList.GetLength() > 0) // mjo commented out these 3 lines.
       //    sFieldList += ",";  // add comma field seperator
       // sFieldList += m_ActiveViewHandle[iView].NthProperty(i).Name();
    }
    sTempStruct += sTemp;  // add props to description from temp
    sTempStruct += "]";  // finally finsih off
            
    /*
    // open the db view by building the GetAs structure string
    if (sTempStruct.GetLength() > 0)
    {
        // make up a char array with the table structure to include name and columns
        sTemp = m_ptrActiveTableName[iView];
        sTemp += TEXT("[");
        sTemp += sTempStruct;
        sTemp += TEXT("]");
    }
    */
    
    // return either the complete mk4 structure or the csv field list
    if (bReturnCSVFieldList)
    {
        sStructure= sFieldList;
    }
    else
    {
        sStructure= sTempStruct;
    }

    return value;
}//@CODE_602


/*@NOTE_609
Fill out the order column  in DD based on a comma seperated list of selected
fields. This is the inverse of MakeProjectionList.
*/
int CMk4DataContainer::MakeOrderFromList(CString sOrderList, CString sTableName,
                                         CString sOrderColumn)
{//@CODE_609
    CString sTemp;  // some temporary string handling
    CString sSelectCriteria;  // some temporary string handling
    CString sToken;  // some temporary string handling
    int idddvindex;  // index for dd derived view
    int iNumRecs;  // for the number of records
    long i;  // loop var
    
    // if the dd is open, look up the column ordering
    int iDDView = FindViewIndexByName(m_sDDViewName); // index of the dd view
    bool bOpened = FALSE;   // flag that indicates opened here (TRUE), or already open (FALSE)
    if (iDDView < 0)
    {
        iDDView = OpenAView(m_sDDViewName);
        bOpened = TRUE; // flag so that it can be closed after setup
    }
    if (iDDView <0) return 1;  // DD not available, so return

    // reset everything in the list to zero, or not selected
    sSelectCriteria = m_sDDColNameForTable + "=" + sTableName;  // select criteria for table
    idddvindex = DeriveView("DataDictionary", sOrderColumn, sSelectCriteria);
    iNumRecs = m_ActiveViewHandle[idddvindex].GetSize();
    for (i=0; i<iNumRecs; i++)
    {
        SetDataItemFrom(idddvindex, i, 0, 0);
    }
    CloseAView(idddvindex);

    // then loop through the order list and assign value to order column
    sTemp = sOrderList;
    i = 0;  // initialize index counter
    while (!sTemp.IsEmpty())
    {
        NextToken(sTemp,sToken);
        i++;  // increment index counter
        sSelectCriteria = m_sDDColNameForTable + "=" + sTableName +
            "," + m_sDDColNameForColumnName + "=" + sToken;
        idddvindex = DeriveView("DataDictionary", sOrderColumn, sSelectCriteria);
        if (m_ActiveViewHandle[idddvindex].GetSize() == 1)
        {
            SetDataItemFrom(idddvindex, 0, 0, i);
        }
        CloseAView(idddvindex);  // clean up
    }
    
    
    // if the dd view had to be opened here, then close it
    if (bOpened)
        CloseAView(iDDView);
    //CloseAView(idddvindex);  // close the derived dd view
    
    return 0;
}//@CODE_609

int CMk4DataContainer::GetUsedViewCount()
{
    int count = 0;
	for (int i = 0; i < VGMAXOPEN; i++)
    {
        if (m_ptrActiveTableName[i] != NULL)
        {
			count++;
        }
    }   // end of for loop
    
    return count;
}

// End of Group: Utility

/*@NOTE_105
Construct a container object and open a MK4 file by name.
*/
CMk4DataContainer::CMk4DataContainer(const char* ptrFileToOpen) //@INIT_105
    : m_iDBStatus(DB_NOT_AVAIL)
{//@CODE_105
    ConstructorInclude();
    
    // Put in your own code
    
    Initialize();
    OpenDBFile(ptrFileToOpen);
}//@CODE_105


/*@NOTE_141
Just create the class and members without actually opening a file; the caller
needs to open a db file at some point with the file open member function.
*/
CMk4DataContainer::CMk4DataContainer() //@INIT_141
    : m_iDBStatus(DB_NOT_AVAIL)
{//@CODE_141
    ConstructorInclude();
    
    // Put in your own code
    
    Initialize();
}//@CODE_141


/*@NOTE_96
Destructor method to clean up the stuff created during the life of the object
*/
CMk4DataContainer::~CMk4DataContainer()
{//@CODE_96
    DestructorInclude();
    
    int i;
    i = CloseDB();
}//@CODE_96


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_97
Method which must be called first in a constructor.
*/
void CMk4DataContainer::ConstructorInclude()
{
}


/*@NOTE_98
Method which must be called first in a destructor.
*/
void CMk4DataContainer::DestructorInclude()
{
}


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3

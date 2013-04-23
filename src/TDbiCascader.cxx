
//////////////////////////////////////////////////////////////////////////
////////////////////////////     ROOT API     ////////////////////////////
//////////////////////////////////////////////////////////////////////////


#include <cstdlib>
#include <memory>
#include <sstream>

#include "TList.h"
#include "TROOT.h"
#include "TSQLStatement.h"
#include "TSystem.h"

#include "TDbi.hxx"
#include "TDbiCascader.hxx"
#include "TDbiString.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>
#include "UtilString.hxx"
#include <TDbiLog.hxx>




ClassImp(CP::TDbiCascader)

//   Definition of static data members
//   *********************************


//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................
///\verbatim
///
///  Purpose:  Default constructor
///
///  Arguments: beQuiet - If true, fail quietly. If not be more verbose.
///
///  Return:    n/a
///
///  Throws:  CP::ENoEnvironment(); if there is no environment defined
///               throw CP::EBadDatabase(); if any other error occured
///  Contact:   N. West, S. Claret
///
///  Specification:-
///  =============
///
///  o  Create Cascader (set up its connections according to values from
///     ENV_TSQL_* environment variables).
///
///  o  If ENV_TSQL_TMP_TBLS is set, call ProcessTmpTblsFile() to initialize
///     Temporary Tables on one of the connections in the cascade.
///
///  Program Notes:-
///  =============
///
///  Current cascader configuration comes from 3 environmental
///  variables:-
///
///    ENV_TSQL_URL             a semi-colon separated list of URLs
///    ENV_TSQL_USER            user name (one or a semi-colon separated list)
///    ENV_TSQL_PSWD            password (one or a semi-colon separated list)
///    ENV_TSQL_TMP_TBLS        path to a file containing SQL statements to create and fill temporary tables (optional T2K extension) 
///
///    or the _UPDATE alternatives e.g. ENV_TSQL_UPDATE_USER
///
/// The _UPDATE versions take priority.
///\endverbatim
CP::TDbiCascader::TDbiCascader(bool beQuiet):
fTempCon(-1), fGlobalSeqNoDbNo(-1)
{


  DbiTrace( "Creating CP::TDbiCascader" << "  ");

// Extract args from  ENV_TSQL environmental variables
  const char*      strUser = gSystem->Getenv("ENV_TSQL_UPDATE_USER");
  if ( ! strUser ) strUser = gSystem->Getenv("ENV_TSQL_USER");
  const char*      strPswd = gSystem->Getenv("ENV_TSQL_UPDATE_PSWD");
  if ( ! strPswd ) strPswd = gSystem->Getenv("ENV_TSQL_PSWD");
  const char*       strUrl = gSystem->Getenv("ENV_TSQL_UPDATE_URL");
  if ( !strUrl )    strUrl = gSystem->Getenv("ENV_TSQL_URL");
  const char*       strTmpTbls = gSystem->Getenv("ENV_TSQL_UPDATE_TMP_TBLS");
  if ( !strTmpTbls )    strTmpTbls = gSystem->Getenv("ENV_TSQL_TMP_TBLS");
  
  std::string userList     = ( strUser ) ? strUser : "";
  std::string pswdList     = ( strPswd ) ? strPswd : "";
  std::string urlList      = ( strUrl  ) ? strUrl  : "";
  std::string tmpTbls      = ( strTmpTbls ) ? strTmpTbls : "";

  if ( urlList == "" || userList == "" || pswdList == "" ) {
    /*
       If beQuiet is true be mute about why we are failing, leave it to the caller to deal with.
    */
    if(!beQuiet)
    {
       std::cout<<"error!"<<std::endl;
        DbiSevere("Cannnot open a Database cascade;\n"
        << "   the environmental variables ENV_TSQL_*:-" << std::endl
        << "USER: \"" << userList << "\" PSWD:\"" << pswdList
        << "\" URL:\"" << urlList << std::endl
        << " are either not defined or empty.\n"
        << "   Please check your settings of ENV_TSQL_USER,"
                << " ENV_TSQL_PSWD and ENV_TSQL_URL\n" );

    throw CP::ENoEnvironment();
   }
  }

  std::vector<std::string> users, pswds, urls;
  CP::UtilString::StringTok(users, userList, ";");
  CP::UtilString::StringTok(pswds, pswdList, ";");
  CP::UtilString::StringTok(urls,  urlList,  ";");

  bool fail = false;

  for (unsigned entry = 0; entry < urls.size(); ++entry ) {
    std::string url  = urls[entry];
    std::string user = ( entry >= users.size() ) ? users[0] : users[entry];
    std::string pswd = ( entry >= pswds.size() ) ? pswds[0] : pswds[entry];

    // Handle empty password designated as '\0' (an empty null terminated character string)
    if ( pswd == "\\0" ) pswd = "";

    CP::TDbiConnection* con;
    // If we are testing the cascade for validity, just try connecting once, otherwise use defaults.
    if(!beQuiet)
      con  = new CP::TDbiConnection(url,user,pswd,1);
    else
      con  = new CP::TDbiConnection(url,user,pswd);

    fConnections.push_back(con);
    if ( ! con->Open() ) {
      fail = true;
      continue;
    }

//  Attempt to locate first GlobalSeqNo/GLOBALSEQNO table.
    if ( fGlobalSeqNoDbNo != -1 ) continue;
    std::auto_ptr<CP::TDbiStatement>  stmtDb(new CP::TDbiStatement(*con));
    if ( ! stmtDb.get() ) continue;
    TSQLStatement* stmt = stmtDb->ExecuteQuery("Select * from GLOBALSEQNO where 1=0");
    if ( stmt ) {
      fGlobalSeqNoDbNo = fConnections.size()-1;
      delete stmt;
      stmt = 0;
    }

//  Check for presence of a DBI_STATE_FLAG table

    if ( this->GetTableDbNo("DBI_STATE_FLAGS",entry) != -1 ) {
      if(!beQuiet)  DbiSevere("  POSSIBLE VERSION SHEAR DETECTED !!!\n"
        << "    The DBI_STATE_FLAGS table is present on cascade entry " << entry << ".  This table will\n"
        << "    only be introduced to manage backward incompatible changes that could lead\n"
        << "    to version shear between the code and the database.  This version of the\n"
        << "    code does not support the change the presence of that table indicates\n"
		  << "    so has to shut down. \n");
	fail = true;
    }
  }

  DbiInfo( *this);
  
  if (tmpTbls != "") {
    DbiLog("Cascader is processing Temp Tables File ENV_TSQL_TMP_TBLS=" << tmpTbls);
    if (ProcessTmpTblsFile(tmpTbls) == -1) {
      DbiSevere("Temporary Tables file ENV_TSQL_TMP_TBLS=" << tmpTbls << " could not be not processed.");
      // Abort the job if there was a problem processing ENV_TSQL_TMP_TBLS.
      // --> Ensures that results will not be produced under the (false) belief that temporary tables were used.
      // If this is crashing your jobs, simply fix or unset ENV_TSQL_TMP_TBLS.
      fail = true;
    }
  }
  
  //  Abort, if there have been any failures.
  if ( fail ) {
    throw CP::EBadDatabase();
  }

}

//.....................................................................

CP::TDbiCascader::~TDbiCascader() {
//
//
//  Purpose: Destructor
//


  DbiTrace( "Destroying CP::TDbiCascader" << "  ");

  for (Int_t dbNo = this->GetNumDb()-1; dbNo >= 0; --dbNo) delete fConnections[dbNo];

}

//.....................................................................
///\verbatim
///
///  Purpose:  Output CP::TDbiCascader status to message stream.
///
///  Arguments:
///    os           in    ostream to output on
///    cascader     in    Cascader to be output
///
///  Return:        ostream
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Output CP::TDbiCascader status to ostream.
///
///  Program Notes:-
///  =============
///
///  None.
///\endverbatim
ostream& CP::operator<<(ostream& os, const CP::TDbiCascader& cascader) {


  os << "CP::TDbiCascader Status:- " << std::endl
     << "Status   URL" << std::endl << std::endl;

  int maxDb = cascader.GetNumDb();
  for (Int_t dbNo = 0; dbNo < maxDb; ++dbNo)
      os << cascader.GetStatusAsString(dbNo) << " "
         << ( ( dbNo == cascader.fGlobalSeqNoDbNo ) ? "(auth)  " : "        ")
         << cascader.GetURL(dbNo) << std::endl;
  os << std::endl;
  return os;

}
 bool CP::TDbiCascader::canConnect()
{
  DbiTrace(" bool CP::TDbiCascader::canConnect() started");

  try
    {
      CP::TDbiCascader test(false);
      //   test.NOOP();
      DbiTrace(" bool CP::TDbiCascader::canConnect() return true");
      return true;
    }
  catch(...)
    {
      DbiTrace(" bool CP::TDbiCascader::canConnect() return false");
      return false;
    }
}
//.....................................................................
///\verbatim
///
///  Purpose:  Allocate a unique (either locally or globally) SEQNO.
///
///  Arguments:
///   tableName       in    The table for which the SEQNO is required.
///   requireGlobal   in    The type of SEQNO required:-
///                           > 0  Must be global
///                           = 0  Must be global if supplied dbNo is authorising
///                                and table isn't temporary otherwise local
///                           < 0  Must be local
///   dbNo            in     The entry in the cascade for which the SEQNO is required
///
///  Return:    The allocated SEQNO or 0 if failure.
///
///  Contact:   N. West
///
///  Program Notes:-
///  =============
///
///  Requests for SEQNOs take account of any pre-existing entries; local entries
///  should only be used for development and this allows for the LOCALSEQNO table
///  and the local data to be wiped at different times without causing conflicts.
///  Global entries should not be a problem as the GLOBALSEQNO table isn't wiped
///  but it provides protection in case the table is damaged (which has happened!).
///\endverbatim
Int_t CP::TDbiCascader::AllocateSeqNo(const std::string& tableName,
                                 Int_t requireGlobal, /* =0 */
                                 Int_t dbNo  /* = 0 */) const {

  bool isTemporary = IsTemporaryTable(tableName,dbNo);

  //  Deal with global requests.

  if (     requireGlobal > 0
      || ( requireGlobal == 0 && dbNo == fGlobalSeqNoDbNo && ! isTemporary ) ) {
    if ( fGlobalSeqNoDbNo < 0 ) {
      DbiWarn( "Unable to issue global SEQNO - no authorising DB in cascade\n"
			       << "  will issue local one instead" << "  ");
    }
    else if ( isTemporary ) {
      DbiWarn( "Unable to issue global SEQNO - " << tableName << " is temporary\n"
			       << "  will issue local one instead" << "  ");
    }
    else return this->ReserveNextSeqNo(tableName,true,fGlobalSeqNoDbNo);
  }

  // Deal with local requests

  return this->ReserveNextSeqNo(tableName,false,dbNo);

}

//.....................................................................
///\verbatim
///
///  Purpose:  Return a Statement to caller.
///
///  Arguments:
///    dbNo     in    Database no. in cascade for which statement
///                   is required.
///
///  Return:    Created statement (or 0 if creation failed).
///             User must delete.
///  Program Notes:-
///  =============
///
///  As the caller is responsible for destroying the statement after use
///  consider:-
///
///  #include <memory>
///  auto_ptr<CP::TDbiStatement> stmt(cascader.CreateStatement(dbNo));
///
///\endverbatim
CP::TDbiStatement* CP::TDbiCascader::CreateStatement(UInt_t dbNo) const {


  if ( this->GetStatus(dbNo) == kFailed ) return 0;
  CP::TDbiConnection& conDb = *fConnections[dbNo];
  CP::TDbiStatement* stmtDb = new CP::TDbiStatement(conDb);
  stmtDb->PrintExceptions();
  return stmtDb;

}
//.....................................................................
///\verbatim
///
///  Purpose:  Creat temporary table with associated validity table
///
///  Arguments:
///    tableNameMc  in    Table name
///    tableDescr   in    Table description as parenthesised comma
///                       separated list e.g.:-
///                       "(MyInt int, MyFloat float, MyString text)"
///
///  Return:    The database cascade number on which the table was
///             created or = -1 if unable to create.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Loop over all databases in cascade, starting at the highest
///    priority i.e. entry 0 and find the first that will accept create
///    temporary table requests.  Return -1 if none will.
///
///  o Make connection permanent so that temporary data won't be lost.
///
///  o Generate and submit requests to create temporary table with
///    associated validity table and return the cascade number of
///    the database that has accepted the tables, recording table
///    name in fTemporaryTables.
///
///\endverbatim
Int_t CP::TDbiCascader::CreateTemporaryTable(const std::string& tableNameMc,
                                       const std::string& tableDescr) {


//  Check that input args look plausible.

  std::string tableName = CP::UtilString::ToUpper(tableNameMc);
  if (    tableName == ""
       || tableDescr[0] != '('
       || tableDescr[tableDescr.size()-1] != ')' ) {
       DbiSevere( "Illegal input args:-" << "  "
			   << "     Table Name: " << tableName
			   << "  Table Description: " << tableDescr
			   <<"  ");
    return -1;
  }

// Find a DB that will accept the command.
  std::string sqlMakeTable;

  Int_t dbNoAcc       = -1;
  std::auto_ptr<CP::TDbiStatement> stmtDb;
  for (UInt_t dbNoTry = 0; dbNoTry < fConnections.size(); ++dbNoTry ) {
    stmtDb.reset(this->CreateStatement(dbNoTry));
    if ( stmtDb.get() ) {
      sqlMakeTable = " create temporary table ";
      sqlMakeTable += tableName;
      sqlMakeTable += " ";
      sqlMakeTable += tableDescr;
      sqlMakeTable += ";";
      stmtDb->ExecuteUpdate(sqlMakeTable.c_str());
      if ( stmtDb->GetExceptionLog().IsEmpty() ) {
        dbNoAcc = dbNoTry;
        this->GetConnection(dbNoAcc)->SetTableExists(tableName);
        break;
      }
        stmtDb->PrintExceptions();
   }//Accumulate Table Name Mode Active Flag
  }

  if ( dbNoAcc < 0 ) {
    if ( stmtDb.get()) stmtDb->PrintExceptions();
    return -1;
  }

// Make connection permanent if not already.
  CP::TDbiConnection& conDb = *fConnections[dbNoAcc];
  if ( conDb.IsTemporary() ) {
    conDb.SetPermanent();
    DbiInfo( "Making connection: " << conDb.GetUrl()
			   << " permanent to preserve temporary tables." << "  ");
  }

// Create SQL to create auxillary validity table and write to same Db.
  sqlMakeTable = TDbi::GetVldDescr(tableName.c_str(),true);

  DbiLog( "Validity Table creation: "
			 << " Database: " << dbNoAcc << " "
                         << sqlMakeTable << "  ");
  stmtDb->ExecuteUpdate(sqlMakeTable.c_str());
  if ( stmtDb->PrintExceptions() ) return -1;
  this->GetConnection(dbNoAcc)->SetTableExists(tableName+"VLD");
  fTemporaryTables[tableName] = dbNoAcc;
  return dbNoAcc;

}

/// \name T2K-Extensions
//@{


///  \brief
///  Creates and fills temporary tables by executing the statements in SQLFilePath.
///
///  Reason: Since T2K uses an external python script to create & fill tables (rather
///          than the MINOS C++ classes), we need a way to execute the SQL generated
///          by this script on a connection in the cascade.
///
///  Note: fTemporaryTables is only ever accessed by IsTemporaryTable().  It would
///        probably be best to dispense with fTemporaryTables and reimplement IsTemporaryTable()
///        to check dynamically.  This would simplify ProcessTmpTblsFile() & ParseTmpTblsSQLLine().
///  \param[in] SQLFilePath in Path to a file containing SQL statements for creating and filling the temporary tables.
///
///  \return dbNo of the database on which the SQL was executed or -1 if there was a problem
///
///  \author Simon Claret t2kcompute@comp.nd280.org
int CP::TDbiCascader::ProcessTmpTblsFile(const std::string& SQLFilePath) {
  int tempConDbNo = GetTempCon();
  
  if (tempConDbNo == -1) {
    DbiSevere("Could not find a cascade connection that accepts Temporary Tables.");
    return -1;
  }
  
  std::ifstream SQLFileStream(SQLFilePath.c_str());
  if (!SQLFileStream.is_open()) {
    DbiSevere("Failed to open file.");
    return -1;
  }
  
  std::string line;
  std::string tableName;
  
  while (!SQLFileStream.eof()) {
  
    tableName = "";
  
    // Get a line from the file
    std::getline(SQLFileStream, line);
  
    switch (ParseTmpTblsSQLLine(line, tableName)) {
    case LINE_APPEARS_VALID:
      if (ExecTmpTblsSQLStmt(tempConDbNo, line, tableName) == false) return -1;
      break;
    case LINE_BLANK:
      DbiInfo("Skipped a blank line.");
      break;
    default:
      DbiSevere("A line in ENV_TSQL_TMP_TBLS=" << SQLFilePath << " appears invalid and was not executed.");
      return -1;
    }

  }//end while
  
  SQLFileStream.close();
  
  return tempConDbNo;
}

///  \brief Returns the dbNo of the first connection in the cascade 
///         supporting Temporary Tables.
///
///         The selected connection is made permanent.
///
///  \return    fTempCon or -1 if no connection supports temporary tables
///
///  \author Simon Claret t2kcompute@comp.nd280.org
int CP::TDbiCascader::GetTempCon() {
  if (fTempCon == -1) {
    for (unsigned int i=0; i < fConnections.size(); ++i) {
      
      if (fConnections[i]->SupportsTmpTbls()==true) {
        fTempCon = i;
        DbiInfo( "Cascader set the temporary connection fTempCon to dbNo " << i << " (" << fConnections[i]->GetUrl() << ").");
        if (fConnections[i]->IsTemporary()) {
          fConnections[i]->SetPermanent();
          DbiInfo( "Cascader made connection " << i << " permanent to preserve temporary tables.");
        }
        break;
      }

    }
  }
  return fTempCon;
}

///  \brief  Helper function for ProcessTempTablesFile() that parses out the
///          the table name from an SQL CREATE TEMPORARY TABLE statement (if
///          present).  Also performs basic checks on the statement.
///
///  \param[in] line A single line from the SQL file
///  \param[out] tableName Parsed table name will be written here if found
///
///  \return    LINE_APPEARS_VALID  if the line appears to contain a valid SQL statement or LINE_BLANK if the line is blank or LINE_INVALID if the line is invalid
///
///  \author Simon Claret t2kcompute@comp.nd280.org
int CP::TDbiCascader::ParseTmpTblsSQLLine(const std::string& line, std::string &tableName) {
  // state of the lineItr
  std::string::const_iterator lineItr;
  char prevChar = ' ';
  bool lineIsEmpty = true;
  
  // state for accTblNameMode
  std::string targetStr("CREATE TEMPORARY TABLE");
  std::string::const_iterator targetStrItr = targetStr.begin();
  // Accumulate Table Name Mode Active (flag)
  // Means: targetStr has been found, and we are currently 
  //        copying characters into tempTableName
  bool accTblNameModeActive = false;
  std::string tempTableName = "";
    
  // Step through the line:
  //  If it begins with targetStr, set tempTableName
  // (would be easier to do this with regular expressions)
  for (lineItr = line.begin(); lineItr != line.end(); ++lineItr) {

    // Ignore leading and repeated whitespaces
    if (prevChar == ' ' && *lineItr == ' ') continue;
    prevChar = *lineItr;
    lineIsEmpty = false;

    // Activate accTblNameMode if targetStr is found at beginning of line
    if (targetStrItr != targetStr.end()) {
    
      if (*targetStrItr == *lineItr) {       
        targetStrItr++;     
        if (targetStrItr == targetStr.end()) {
          accTblNameModeActive = true;
          continue;
        }
      } else {
        targetStrItr = targetStr.end();
      }
    
    }
      
    // extract table name
    // need to add test cases for: leading space, trailing space, no leading space, no trailing space
    if (accTblNameModeActive == true) {
      // skip leading spaces
      if (tempTableName.length()==0 && *lineItr==' ') continue;
      // end of table name reached
      else if (*lineItr == '(' || *lineItr == ' ') accTblNameModeActive = false;
      else tempTableName.push_back(*lineItr);
    }
      
  }//end for
  
  if (lineIsEmpty == true)
  {
    tableName = "";
    return LINE_BLANK;
  } else {
    tableName = tempTableName;
    return LINE_APPEARS_VALID;
  }

}

///  \brief    Helper function for ProcessTempTablesFile() that executes
///            an SQL statement.
///
///            If a corresponding tableName is provided, it is registered as appropriate.
///
///  \param[in] line An sql statement.
///  \param[in] tableName Parsed table name (if this is a CREATE TEMPORARY TABLE statement)
///
///  \return true success or false failure
///
///  \author Simon Claret t2kcompute@comp.nd280.org

bool CP::TDbiCascader::ExecTmpTblsSQLStmt(int tempConDbNo, const std::string& line, const std::string& tableName) {
  TDbiStatement* stmt = CreateStatement(tempConDbNo);
  bool retVal = false;
  
  if ( stmt && stmt->ExecuteUpdate(line) ) {
    retVal = true;
    
    // Application-level tableName registration
    if ( tableName.length() > 0 ){
      GetConnection(tempConDbNo)->SetTableExists(tableName);
      DbiInfo("Cascader registered table " << tableName << " to connection " << tempConDbNo);
      if ( !(tableName.length() >= 3 && tableName.substr(tableName.length()-3, 3) == "VLD") ) {
        fTemporaryTables[tableName] = tempConDbNo;
        DbiLog("Cascader registered temporary table " << tableName << " to connection " << tempConDbNo);
      }
    }
    
  } else {
    DbiSevere("Failed to execute SQL statement: " << line);
  }
  
  delete stmt;
  return retVal;
}

//@}

//.....................................................................
///
///
///  Purpose:  Return a connection to caller (CP::TDbiCascader retains ownership)
///
const CP::TDbiConnection* CP::TDbiCascader::GetConnection(UInt_t dbNo) const{


  if ( this->GetStatus(dbNo) == kFailed ) return 0;
  return fConnections[dbNo];

}
//.....................................................................
///
///
///  Purpose:  Return a connection to caller (CP::TDbiCascader retains ownership)

CP::TDbiConnection* CP::TDbiCascader::GetConnection(UInt_t dbNo) {


  if ( this->GetStatus(dbNo) == kFailed ) return 0;
  return fConnections[dbNo];

}


//.....................................................................
///  Purpose:  Return Database Name for cascade entry number.
std::string CP::TDbiCascader::GetDbName(UInt_t dbNo) const {
//
//

  std::string dbName;

  if ( dbNo < this->GetNumDb() ) dbName = fConnections[dbNo]->GetDbName();
  else DbiWarn( "Database does not contain entry " << dbNo << "  ");
  return dbName;

}

//.....................................................................
///
///
///  Purpose:  Return number of first DB in cascade with name dbName.
///
///  Return:   Database number corresponding to dbName or -1 if none.
Int_t CP::TDbiCascader::GetDbNo(const std::string& dbName) const {


  for ( unsigned dbNo = 0; dbNo < this->GetNumDb(); ++dbNo) {
    if ( dbName == fConnections[dbNo]->GetDbName() ) return dbNo;
  }

  DbiWarn( "Database does not contain entry " << dbName << "  ");
  return -1;

}

//.....................................................................
///
///
///  Purpose:  Return DB connection status as a string.
///
///  Arguments:
///    dbNo         in    Database number (0..GetNumDb()-1)
std::string CP::TDbiCascader::GetStatusAsString(UInt_t dbNo) const {


  Int_t status = GetStatus(dbNo);

  switch ( status ) {
  case kClosed:  return "Closed";
  case kOpen:    return "Open  ";
  default:       return "Failed";
  }

}
//.....................................................................
///
///
///  Purpose:  Return cascade number of first database that holds table
///            or -1 if none.
Int_t CP::TDbiCascader::GetTableDbNo(const std::string& tableName,
                                Int_t selectDbNo /* -1 */) const {


// If selectDbNo >= 0 only look in this entry in the cascade.

// If table name has any lower case letters then fail.
  std::string::const_iterator itr    = tableName.begin();
  std::string::const_iterator itrEnd = tableName.end();
  while ( itr != itrEnd ) if ( islower(*itr++) ) return -1;

// Loop over cascade looking for table.

  for (UInt_t dbNoTry = 0; dbNoTry < fConnections.size(); ++dbNoTry ) {
    if ( selectDbNo >= 0 && (UInt_t) selectDbNo != dbNoTry ) continue;
    const CP::TDbiConnection* con =  this->GetConnection(dbNoTry);
    if ( con && con->TableExists(tableName) ) return dbNoTry;
  }

  return -1;

}
//.....................................................................
///\verbatim
///
///  Purpose: Hold temporary connections open
///
///  Specification:-
///  =============
///
///  Hold all connections open by telling them that they have a
///  connected statement.
///
///  Program Notes:-
///  =============
///
///  See CP::TDbiConnectionMaintainer for use.
///\endverbatim
void CP::TDbiCascader::HoldConnections() {


  for (UInt_t dbNo = 0; dbNo < fConnections.size(); ++dbNo )
    fConnections[dbNo]->ConnectStatement();
}

//.....................................................................
///  Purpose:  Return kTRUE if tableName is temporary in cascade member dbNo
Bool_t CP::TDbiCascader::IsTemporaryTable(const std::string& tableName,
                                     Int_t dbNo) const {
//
//LL


    std::map<std::string,Int_t>::const_iterator itr
                                     = fTemporaryTables.find(tableName);
  return (     itr != fTemporaryTables.end()
           &&  (*itr).second == dbNo );

}


// Private Locker object

//.....................................................................
///\verbatim
///
///  Purpose:  Ctor: Create a lock on a table accessed via connection.
///            (will be released by dtor)
///  Arguments:
///          stmtDB    in  CP::TDbiStatement (given to Lock).
///\endverbatim
CP::TDbiCascader::Lock::Lock(CP::TDbiStatement* stmtDB, const std::string& seqnoTable, const std::string& dataTable) :
fStmt(stmtDB),
fSeqnoTableName(seqnoTable),
fDataTableName(dataTable),
fLocked(kFALSE)
{


  if ( ! fStmt ) {
       DbiSevere( "Cannot obtain statment to set lock" << "  ");
    return;
  }

  this->SetLock(kTRUE);

}
//.....................................................................
///  Purpose:  Dtor: Clear lock
CP::TDbiCascader::Lock::~Lock() {
//
//


  this->SetLock(kFALSE);
  delete fStmt;
  fStmt = 0;

}

//.....................................................................
///\verbatim
///
///  Purpose:  Set or clear lock.
///
///  Arguments:
///    setting      in    Required setting of lock. [default kTRUE]
///
///  Program Notes:-
///  =============
///
///  No-op if locked otherwise use the LOCK TABLES command.
///\endverbatim

 void CP::TDbiCascader::Lock::SetLock(Bool_t setting) {


  if ( setting == fLocked || ! fStmt ) return;

  std::string sql;

  if ( setting ) {
    sql = "LOCK TABLES ";
    sql += fSeqnoTableName + " WRITE";
    if ( fDataTableName != "" ) sql += ", " + fDataTableName  + "VLD WRITE";
  }
  else {
    sql = "UNLOCK TABLES;";
  }
  DbiLog( "Lock requested: " << setting
                         << " issuing lock command: " << sql << "  ");
  fStmt->ExecuteUpdate(sql.c_str());
  if ( fStmt->GetExceptionLog().IsEmpty() ) fLocked = setting;
  fStmt->PrintExceptions();

}
//.....................................................................
///\verbatim
///
///  Purpose: Release temporary connections held open by HoldConnections.
///
///
///  Specification:-
///  =============
///
///  Undo HoldConnections() by telling all connections that they no longer
/// have a connected statement.
///
///  Program Notes:-
///  =============
///
///  See CP::TDbiConnectionMaintainer for use.
///\endverbatim
void CP::TDbiCascader::ReleaseConnections() {


  for (UInt_t dbNo = 0; dbNo < fConnections.size(); ++dbNo )
    fConnections[dbNo]->DisConnectStatement();
}
//.....................................................................
///\verbatim
///
///  Purpose:  Reserve the next higher available unique (either locally or globally) SEQNO.
///            in the appropriate SEQNO table.
///
///  Arguments:
///   tableName       in    The table for which the SEQNO is required.
///   isGlobal        in    = true - reserve in GLOBALSEQNO table(dbNo must be authorizing)
///                         = false - reserve in LOCALSEQNO table (creating if required)
///   dbNo            in    The entry in the cascade holding the SEQNO table.
///
///  Return:    The allocated SEQNO or 0 if failure.
///
///  Contact:   N. West
///
///  Program Notes:-
///  =============
///
///  Requests for local SEQNOs may result in the creation of a LOCALSEQNO table.
///\endverbatim
Int_t CP::TDbiCascader::ReserveNextSeqNo(const std::string& tableName,
                                    Bool_t isGlobal,
                                    UInt_t dbNo) const {

  CP::TDbiString sql;

  std::string seqnoTableName = isGlobal ? "GLOBALSEQNO" : "LOCALSEQNO";
  bool seqnoTableNameExists = this->TableExists(seqnoTableName,dbNo);
  bool tableNameExists      = this->TableExists(tableName,dbNo);

  std::auto_ptr<CP::TDbiStatement> stmtDb(this->CreateStatement(dbNo) );
  if ( ! stmtDb.get() ) return 0;

  // Check that required SEQNO table exists.

  if ( isGlobal ) {
    if ( ! seqnoTableNameExists ) {
         DbiSevere( "Unable to issue global SEQNO - " << dbNo
			       << " is not an authorising DB" << "  ");
      return 0;
    }
  }
  else {
    if ( ! seqnoTableNameExists ) {
      sql.Clear();
      sql << "CREATE TABLE " << seqnoTableName
	  << "(TABLENAME      CHAR(64) NOT NULL PRIMARY KEY,\n"
	  << " LASTUSEDSEQNO  INT )";

      DbiLog( "Database: " << dbNo
			     << " create local SEQNO table query: " << sql.c_str() << "  ");
      stmtDb->ExecuteUpdate(sql.c_str());
      if ( stmtDb->PrintExceptions() ) return 0;
      sql.Clear();
      sql << "INSERT INTO " <<  seqnoTableName << " VALUES ('*',0)";
      DbiLog( "Database: " << dbNo
			     << " prime local SEQNO table query: " << sql.c_str() << "  ");
      stmtDb->ExecuteUpdate(sql.c_str());
      if ( stmtDb->PrintExceptions() ) return 0;
    }
  }

// Lock seqno table by creating a lock object on the stack.
// Table will be unlocked when lock object destroyed.

  std::string dataTable;
// Only pass in table name if it's not temporary and exists in
// the selected DB otherwise Lock will try to lock a non-existent table.
  if (  ! this->IsTemporaryTable(tableName,dbNo)
       && tableNameExists ) dataTable = tableName;
  Lock lock(this->CreateStatement(dbNo),seqnoTableName,dataTable);
  if ( ! lock.IsLocked() ) {
       DbiSevere( "Unable to lock " << seqnoTableName << "  ");
    return 0;
  }

// Find row containing last used SeqNo for this table.
// Not that comparison is case insensitive.
  sql.Clear();
  sql << "select * from " << seqnoTableName << " where TABLENAME = '*' or TABLENAME = '";
  sql << tableName + "' order by TABLENAME";
  DbiLog( " query: " << sql.c_str() << "  ");
  TSQLStatement* stmt = stmtDb->ExecuteQuery(sql.c_str());
  stmtDb->PrintExceptions(CP::TDbiLog::DebugLevel );
  Int_t seqNoDefault = 0;
  if ( stmt && stmt->NextResultRow() ) {
    seqNoDefault = stmt->GetInt(1);
  }
  else {
       DbiSevere( "Unable to find default SeqNo"
			   << " due to above error" << "  ");
    delete stmt;
    stmt = 0;
    return 0;
  }
  Int_t seqNoTable = seqNoDefault;
  if ( stmt->NextResultRow() ) {
    seqNoTable = stmt->GetInt(1);
  }
  delete stmt;
  stmt = 0;
  DbiLog( "  query returned last used seqno: " << seqNoTable << "  ");

//  If the table exists, make sure that the seqNo hasn't already been used.
//  This is paranoia code and expensive, so only do the check once for
//  each tableName/isGlobal/dbNo combination.

  static std::string checkedCombinations;
  std::ostringstream combination;
  combination << ":" << tableName << isGlobal << dbNo << ":";
  bool notChecked = checkedCombinations.find(combination.str()) == std::string::npos;
  if ( notChecked ) checkedCombinations += combination.str();
  if ( tableNameExists && notChecked ) {
    Int_t seqNoMin = seqNoDefault;
    Int_t seqNoMax = seqNoDefault + TDbi::kMAXLOCALSEQNO;
    sql.Clear();
    sql << "select max(SEQNO) from " << tableName << "VLD"
	<< " where SEQNO between " << seqNoMin << " and " << seqNoMax;
    DbiLog( "Database: " << dbNo
                         << " max  SEQNO query: " << sql.c_str() << "  ");
    stmt  =  stmtDb->ExecuteQuery(sql.c_str());
    if ( stmtDb->PrintExceptions() ) return 0;
    Int_t  minValue = 0;
    // Queries returning group function results can be null.
    if (  stmt && stmt->NextResultRow() && ! stmt->IsNull(0) ) {
      minValue = stmt->GetInt(0);
     if ( minValue <= 0 ) minValue = 0;  // Should never happen.
    }
    delete stmt;
    stmt = 0;

    if ( minValue > seqNoTable ) {
         DbiSevere(  "Database: " << dbNo << " "
	  << seqnoTableName << " has last used SEQNO of "
	  << seqNoTable << " for table " << tableName
	  << ",\n    but the highest SEQNO in the band " << seqNoMin << " to " << seqNoMax
	  << " is " <<  minValue << " for that table\n    "
	  << seqnoTableName << " is out of date! It will be updated for " << tableName << "  ");
	seqNoTable = minValue;
    }
  }


//  Update last used SeqNo and record in table.
  sql.Clear();
  sql << "delete from " << seqnoTableName << " where TABLENAME='";
  sql << tableName + "'";
  DbiLog( "SEQNO entry removal: " << sql.c_str() << "  ");
  stmtDb->ExecuteUpdate(sql.c_str());
  if ( stmtDb->PrintExceptions() ) return 0;

  seqNoTable++;

  sql.Clear();
  sql << "insert into  " << seqnoTableName << " values('";
  sql << tableName + "'," << seqNoTable << ")";
  DbiLog( "SEQNO entry add: " << sql.c_str() << "  ");
  stmtDb->ExecuteUpdate(sql.c_str());
  if ( stmtDb->PrintExceptions() ) return 0;

  return seqNoTable;

}
//.....................................................................
///  Purpose: Set connection permanent.
void CP::TDbiCascader::SetPermanent(UInt_t dbNo,
			       Bool_t permanent /* = true */ ) {
//
//


  if ( dbNo < fConnections.size() ) fConnections[dbNo]->SetPermanent(permanent);

}


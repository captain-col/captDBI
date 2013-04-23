// $Id: TDbiDatabaseManager.cxx,v 1.2 2011/06/09 14:44:29 finch Exp $
#include <vector>
#include <cstdlib>

#include "TSystem.h"

#include "TString.h"

#include "TDbiExceptionLog.hxx"
#include "TDbiBinaryFile.hxx"
#include "TDbiCache.hxx"
#include "TDbiCascader.hxx"
#include "TDbiConfigSet.hxx"
#include "TDbiServices.hxx"
#include "TDbiDatabaseManager.hxx"
#include "TDbiTableProxy.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;
#include "UtilString.hxx"

ClassImp(ND::TDbiDatabaseManager)

//   Definition of static data members
//   *********************************


ND::TDbiDatabaseManager* ND::TDbiDatabaseManager::fgInstance       = 0;
int                    ND::TDbiDatabaseManager::Cleaner::fgCount = 0;

//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

ND::TDbiDatabaseManager::TDbiDatabaseManager() :
fCascader(0)
{
//
//
//  Purpose:  Constructor
//
//  Arguments: None.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create factory

//  Program Notes:-
//  =============

//  None.


// Create cascader for database access.

  fCascader = new ND::TDbiCascader;

// Get any environment configuration.

  this->SetConfigFromEnvironment();

  DbiTrace( "Creating ND::TDbiDatabaseManager"
    << "  ");


}

//.....................................................................

ND::TDbiDatabaseManager::~TDbiDatabaseManager() {
//
//
//  Purpose: Destructor
//
//  Arguments:
//    None.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o  Destroy all ND::TDbiTableProxies if Shutdown required.



  if (  ND::TDbiExceptionLog::GetGELog().Size() ) {
     DbiInfo( "Database Global Exception Log contains "
	  << ND::TDbiExceptionLog::GetGELog().Size() << " entries:-" << "  ");;
     ND::TDbiExceptionLog::GetGELog().Print();
  }

  int shutdown = 0;
  if (    ! this->GetConfig().Get("Shutdown",shutdown)
	 || shutdown == 0 ) {
    DbiInfo( "DatabaseInterface shutdown not requested" << "  ");
    return;
  }

  DbiInfo( "DatabaseInterface shutting down..." << "  ");

// Destroy all owned objects.

  for ( std::map<std::string,ND::TDbiTableProxy*>::iterator itr = fTPmap.begin();
        itr != fTPmap.end();
        ++itr) {
    ND::TDbiTableProxy* tp = (*itr).second;
    delete tp;
  }

  delete fCascader;
  fCascader = 0;

  DbiTrace(  "Destroying ND::TDbiDatabaseManager" << "  ");

  DbiInfo( "DatabaseInterface shutdown complete." << "  ");
  ND::TDbiDatabaseManager::fgInstance = 0;

}

//.....................................................................

void ND::TDbiDatabaseManager::ApplySqlCondition() const {
//
//
//  Purpose: Apply Sql condition to existing ND::TDbiTableProxys.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Apply global Sql condition, together with any prevailing rollback to
//    all existing ND::TDbiTableProxys.

  std::map<std::string,ND::TDbiTableProxy*>::const_iterator itr = fTPmap.begin();
  std::map<std::string,ND::TDbiTableProxy*>::const_iterator itrEnd = fTPmap.end();
  for ( ; itr != itrEnd; ++itr) this->ApplySqlCondition(itr->second);
}

//.....................................................................

void ND::TDbiDatabaseManager::ApplySqlCondition(ND::TDbiTableProxy* proxy) const {
//
//
//  Purpose: Apply Sql condition to specific ND::TDbiTableProxy.
//
//  Arguments:
//    proxy        in    ND::TDbiTableProxy to which condition is to be applied.
//
//  Contact:   N. West
//

  string sqlFull = fSqlCondition;
  const string tableName(proxy->GetTableName());
  const string& date = fRollbackDates.GetDate(tableName);
  if ( date.size() ) {
    if ( sqlFull.size() ) sqlFull += " and ";
    sqlFull += fRollbackDates.GetType(tableName);
    sqlFull += " < \'";
    sqlFull += date;
    sqlFull += "\'";
  }
  const string& epoch_condition = fEpochRollback.GetEpochCondition(tableName);
  if ( epoch_condition.size() ) {
    if ( sqlFull.size() ) sqlFull += " and ";
    sqlFull += epoch_condition;
  }

  proxy->SetSqlCondition(sqlFull);
}

//.....................................................................

void ND::TDbiDatabaseManager::ClearRollbacks() {

  fEpochRollback.Clear();
  fRollbackDates.Clear();
  this->ApplySqlCondition();
}

//.....................................................................

void ND::TDbiDatabaseManager::ClearSimFlagAssociation() {

  fSimFlagAss.Clear();
}
//.....................................................................
///
///
///  Purpose:  Reconfigure after internal registry update.
///
///
///    Throws  EBadTDbiRegistryKeys()  if TDbiRegistry contains any unknown keys
void ND::TDbiDatabaseManager::Config() {
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o

//  Program Notes:-
//  =============

//  None.

  TDbiRegistry& reg = this->GetConfig();

  //Load up SimFlag Associations and remove them from the TDbiRegistry.
  fSimFlagAss.Set(reg);

  //Load up Rollback dates and epochs and remove them from the TDbiRegistry.
  fEpochRollback.Set(reg);
  fRollbackDates.Set(reg);

  //Apply any rollback now in force.
  this->ApplySqlCondition();

  // If Level 2 cache enabled establish working directory
  // for ND::TDbiBinaryFile.
  const char*  dir;
  if ( reg.Get("Level2Cache",dir) ) {
    // Expand any environmental variables.
    TString tmp(dir);
    //  March 2004 ExpandPathName returns false even if it works, so test for failure
    //  by looking for an unexpanded symbol.
    gSystem->ExpandPathName(tmp);
    if ( tmp.Contains("$" ) ) {
      dir = "./";
      DbiWarn( "Directory name expansion failed, using "
  			     << dir << " instead" << "  ");
    }
    else {
      dir = tmp.Data();
    }

    ND::TDbiBinaryFile::SetWorkDir(dir);
    DbiLog( "ND::TDbiDatabaseManager: Setting L2 Cache to: " << dir << "  ");
  }

  // Check for request to make all cascade connections permanent
  // and remove from the TDbiRegistry.

  int connectionsPermanent = 0;
  if ( reg.Get("MakeConnectionsPermanent",connectionsPermanent) ) {
    reg.RemoveKey("MakeConnectionsPermanent");
    Int_t dbNo =fCascader->GetNumDb();
    if ( connectionsPermanent > 0 ) {
      while ( --dbNo >= 0 ) fCascader->SetPermanent(dbNo);
      DbiInfo( "Making all database connections permanent" << "  ");
      // Inform ND::TDbiServices so that ND::TDbiConnection can check when opening new connections.
      ND::TDbiServices::fAsciiDBConectionsTemporary = false;
    }
    else {
      while ( --dbNo >= 0 ) fCascader->SetPermanent(dbNo,false);
      DbiInfo( "Forcing all connections, including ASCII DB, to be temporary" << "  ");
      // Inform ND::TDbiServices so that ND::TDbiConnection can check when opening new connections.
      ND::TDbiServices::fAsciiDBConectionsTemporary = true;
    }
  }

  // Check for request to order context queries and remove from the TDbiRegistry.

  int OrderContextQuery = 0;
  if ( reg.Get("OrderContextQuery",OrderContextQuery) ) {
    reg.RemoveKey("OrderContextQuery");
    if ( OrderContextQuery ) {
      ND::TDbiServices::fOrderContextQuery = true;
      DbiInfo( "Forcing ordering of all context queries" << "  ");
    }
  }

  // Abort if TDbiRegistry contains any unknown keys

  const char* knownKeys[]   = { "Level2Cache",
	 		        "Shutdown" };
  int numKnownKeys          = sizeof(knownKeys)/sizeof(char*);
  bool hasUnknownKeys       = false;

  TDbiRegistry::TDbiRegistryKey keyItr(&this->GetConfig());
  while ( const char* foundKey = keyItr() ) {
    bool keyUnknown = true;
    for (int keyNum = 0; keyNum < numKnownKeys; ++keyNum ) {
      if ( ! strcmp(foundKey,knownKeys[keyNum]) ) keyUnknown = false;
    }
    if ( keyUnknown ) {
     DbiSevere( "FATAL: "
	<< "Illegal registry item: " << foundKey << "  ");
       hasUnknownKeys = true;
    }
  }

  if ( hasUnknownKeys ) {
    DbiSevere( "FATAL: " << "Aborting due to illegal registry items." << "  ");
    throw  EBadTDbiRegistryKeys();
  }
}

//.....................................................................

ND::TDbiTableProxy& ND::TDbiDatabaseManager::GetTableProxy
                                    (const std::string& tableNameReq,
                                     const ND::TDbiTableRow* tableRow) {
//
//
//  Purpose:  Locate, or if necessary create, ND::TDbiTableProxy for
//            named table.
//
//  Arguments:
//    tableNameReq in    Name of table requested.
//    tableRow     in    Example of a Table Row object.
//
//  Return:    ND::TDbiTableProxy for table.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Locate, or if necessary create, ND::TDbiTableProxy for named table.
//
//  o If creating apply prevailing SQL condition.

//  Program Notes:-
//  =============

//  None.

// Force upper case name.
  std::string tableName = ND::UtilString::ToUpper(tableNameReq);
  std::string proxyName = tableName;

  proxyName.append("::");
  proxyName.append(tableRow->ClassName());
  ND::TDbiTableProxy* qpp = fTPmap[proxyName];
  if ( ! qpp ) {
    qpp = new ND::TDbiTableProxy(fCascader,tableName,tableRow);
    this->ApplySqlCondition(qpp);
    fTPmap[proxyName] = qpp;
  }

  return *qpp;

}

//.....................................................................

ND::TDbiDatabaseManager& ND::TDbiDatabaseManager::Instance() {
//
//
//  Purpose: Locate, or create, ND::TDbiDatabaseManager singleton.
//
//  Arguments:     None.
//
//  Return:    ND::TDbiDatabaseManager singleton.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Locate, or if necessary create, ND::TDbiDatabaseManager singleton.

//  Program Notes:-
//  =============

//  None.

  if ( ! fgInstance ) {
// Delete is handled by Cleaner class based on #include count
    fgInstance = new ND::TDbiDatabaseManager();
  }
  return *fgInstance;

}

//.....................................................................

void ND::TDbiDatabaseManager::PurgeCaches() {
//
//
//  Purpose: Purge all caches.
//
//  Arguments:
//    None.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o  Purge all caches.


//  Program Notes:-
//  =============

//  None.

// Pruge all caches.

  for ( std::map<std::string,ND::TDbiTableProxy*>::iterator itr = fTPmap.begin();
        itr != fTPmap.end();
        ++itr) {
    ND::TDbiTableProxy* tp = (*itr).second;
    tp->GetCache()->Purge();
  }

}

//.....................................................................

void ND::TDbiDatabaseManager::RefreshMetaData(const std::string& tableName) {
//
//
//  Purpose: Refresh meta data for specied table.
//
//  Arguments:
//    tableName    in    Name of table to be refreshed.


//  Program Notes:-
//  =============

//  This method is currently only used by ND::TDbiSqlValPacket after
//  it has created a new table in the database.  In such cases
//  the pre-existing corresponding TDbiTableProxy.hxxas to be refreshed.

  std::map<std::string,ND::TDbiTableProxy*>::iterator itr = fTPmap.begin();
  std::map<std::string,ND::TDbiTableProxy*>::iterator itrEnd = fTPmap.end();
  for ( ; itr != itrEnd; ++itr) {
    ND::TDbiTableProxy* table = (*itr).second;
    if ( table && table->GetTableName() == tableName ) table->RefreshMetaData();
  }

}
//.....................................................................

void ND::TDbiDatabaseManager::SetConfigFromEnvironment() {
//
//
//  Purpose:  Set up configuration from ENV_DBI environmental variable
//            which consists of a semi-colon separated list of DBI
//            configuration requests.

  const char* strENV_DBI = gSystem->Getenv("ENV_DBI");
  if ( strENV_DBI == 0  || strlen(strENV_DBI) == 0 ) return;

  DbiInfo( "\nConfiguring DatabaseInterface from the environmental "
             << "variable ENV_DBI:-\n  " << strENV_DBI << "  ");
  std::vector<std::string> configRequests;
  ND::UtilString::StringTok(configRequests, strENV_DBI, ";");

  for (unsigned entry = 0; entry < configRequests.size(); ++entry )
                  this->Set(configRequests[entry].c_str());
  this->Update();
}

//.....................................................................

void ND::TDbiDatabaseManager::SetSqlCondition(const std::string& sql) {
//
//
//  Purpose: Record and apply global SQL condition.
//
//  Arguments:
//    sql          in    SQL condition string (excluding where).
//
//                       See Program Notes.
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Record global SQL condition and apply to existing ND::TDbiTableProxys.


//  Program Notes:-
//  =============

//  The SQL condition must behave as a single expression as
//  additional conditions e.g. rollback, may be appended using
// AND.  This means that if the expression contains sub-expressions
//  combined using OR, then the entire expression should be enclosed
//  in parentheses.


  fSqlCondition = sql;
  this->ApplySqlCondition();
}

//.....................................................................

void ND::TDbiDatabaseManager::ShowStatistics() const {
//
//
//  Purpose:  Show total statistics.
//
//  Contact:   N. West

 std::ostream& msg=TDbiLog::GetLogStream();
  msg << "\n\nCache statistics:-\n\n"
      << "Table Name                             "
      << "    Current   Maximum     Total     Total\n"
      << "                                       "
      << "       Size      Size   Adopted    Reused" << endl;

// Loop over all owned objects.

  for ( std::map<std::string,ND::TDbiTableProxy*>::const_iterator itr = fTPmap.begin();
        itr != fTPmap.end();
        ++itr) {
    const ND::TDbiTableProxy* tp = (*itr).second;
    std::string name = (*itr).first;
    if ( name.size() < 40 ) name.append(40-name.size(),' ');
    msg << name;
//  Only want to look at cache so by-pass constness.
    const_cast<ND::TDbiTableProxy*>(tp)->GetCache()->ShowStatistics(msg);
   msg << endl;
  }
  msg << "\n" << endl;

//  Only want to look at cascader so by-pass constness.

  DbiInfo( const_cast<ND::TDbiDatabaseManager*>(this)->GetCascader());


}


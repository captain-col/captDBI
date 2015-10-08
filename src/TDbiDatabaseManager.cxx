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
#include <MsgFormat.hxx>
#include "UtilString.hxx"

ClassImp(CP::TDbiDatabaseManager)

//   Definition of static data members
//   *********************************


CP::TDbiDatabaseManager* CP::TDbiDatabaseManager::fgInstance       = 0;
int CP::TDbiDatabaseManager::Cleaner::fgCount = 0;

//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

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
CP::TDbiDatabaseManager::TDbiDatabaseManager() :
    fCascader(0) {

    fCascader = new CP::TDbiCascader;

    // Get any environment configuration.
    this->SetConfigFromEnvironment();

    DbiTrace("Creating CP::TDbiDatabaseManager");
}

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
//  o  Destroy all CP::TDbiTableProxies if Shutdown required.
CP::TDbiDatabaseManager::~TDbiDatabaseManager() {

    if (CP::TDbiExceptionLog::GetGELog().Size()) {
        DbiInfo("Database Global Exception Log contains "
                << CP::TDbiExceptionLog::GetGELog().Size() << " entries:-");
        CP::TDbiExceptionLog::GetGELog().Print();
    }

    int shutdown = 0;
    if (! this->GetConfig().Get("Shutdown",shutdown)
        || shutdown == 0) {
        DbiInfo("DatabaseInterface shutdown not requested" << "  ");
        return;
    }

    DbiInfo("DatabaseInterface shutting down..." << "  ");

    // Destroy all owned objects.

    for (std::map<std::string,CP::TDbiTableProxy*>::iterator itr 
             = fTPmap.begin();
         itr != fTPmap.end();
         ++itr) {
        CP::TDbiTableProxy* tp = (*itr).second;
        delete tp;
    }

    delete fCascader;
    fCascader = 0;

    DbiTrace("Destroying CP::TDbiDatabaseManager" << "  ");

    DbiInfo("DatabaseInterface shutdown complete." << "  ");
    CP::TDbiDatabaseManager::fgInstance = 0;

}

//
//
//  Purpose: Apply Sql condition to existing CP::TDbiTableProxys.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Apply global Sql condition, together with any prevailing rollback to
//    all existing CP::TDbiTableProxys.
void CP::TDbiDatabaseManager::ApplySqlCondition() const {
    std::map<std::string,CP::TDbiTableProxy*>::const_iterator itr
        = fTPmap.begin();
    std::map<std::string,CP::TDbiTableProxy*>::const_iterator itrEnd 
        = fTPmap.end();
    for (; itr != itrEnd; ++itr) {
        this->ApplySqlCondition(itr->second);
    }
}


//
//
//  Purpose: Apply Sql condition to specific CP::TDbiTableProxy.
//
//  Arguments:
//    proxy        in    CP::TDbiTableProxy to which condition is to be applied.
//
//  Contact:   N. West
//
void CP::TDbiDatabaseManager::ApplySqlCondition(
    CP::TDbiTableProxy* proxy) const {

    std::string sqlFull = fSqlCondition;
    const std::string tableName(proxy->GetTableName());
    const std::string& date = fRollbackDates.GetDate(tableName);
    if (date.size()) {
        if (sqlFull.size()) {
            sqlFull += " and ";
        }
        sqlFull += fRollbackDates.GetType(tableName);
        sqlFull += " < \'";
        sqlFull += date;
        sqlFull += "\'";
    }
    const std::string& epoch_condition
        = fEpochRollback.GetEpochCondition(tableName);
    if (epoch_condition.size()) {
        if (sqlFull.size()) {
            sqlFull += " and ";
        }
        sqlFull += epoch_condition;
    }

    proxy->SetSqlCondition(sqlFull);
}

void CP::TDbiDatabaseManager::ClearRollbacks() {
    fEpochRollback.Clear();
    fRollbackDates.Clear();
    this->ApplySqlCondition();
}

void CP::TDbiDatabaseManager::ClearSimFlagAssociation() {
    fSimFlagAss.Clear();
}

///
///
///  Purpose:  Reconfigure after internal registry update.
///
///
///    Throws  EBadTDbiRegistryKeys()  if TDbiRegistry contains any unknown keys
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o

//  Program Notes:-
//  =============

//  None.
void CP::TDbiDatabaseManager::Config() {
    TDbiRegistry& reg = this->GetConfig();

    //Load up SimFlag Associations and remove them from the TDbiRegistry.
    fSimFlagAss.Set(reg);

    //Load up Rollback dates and epochs and remove them from the TDbiRegistry.
    fEpochRollback.Set(reg);
    fRollbackDates.Set(reg);

    //Apply any rollback now in force.
    this->ApplySqlCondition();

    // If Level 2 cache enabled establish working directory
    // for CP::TDbiBinaryFile.
    const char*  dir;
    if (reg.Get("Level2Cache",dir)) {
        // Expand any environmental variables.
        TString tmp(dir);
        //  March 2004 ExpandPathName returns false even if it works, so test
        //  for failure by looking for an unexpanded symbol.
        gSystem->ExpandPathName(tmp);
        if (tmp.Contains("$")) {
            dir = "./";
            DbiWarn("Directory name expansion failed, using "
                    << dir << " instead" << "  ");
        }
        else {
            dir = tmp.Data();
        }

        CP::TDbiBinaryFile::SetWorkDir(dir);
        DbiLog("CP::TDbiDatabaseManager: Setting L2 Cache to: " << dir << "  ");
    }

    // Check for request to make all cascade connections permanent
    // and remove from the TDbiRegistry.

    int connectionsPermanent = 0;
    if (reg.Get("MakeConnectionsPermanent",connectionsPermanent)) {
        reg.RemoveKey("MakeConnectionsPermanent");
        Int_t dbNo =fCascader->GetNumDb();
        if (connectionsPermanent > 0) {
            while (--dbNo >= 0) {
                fCascader->SetPermanent(dbNo);
            }
            DbiInfo("Making all database connections permanent" << "  ");
            // Inform CP::TDbiServices so that CP::TDbiConnection can check
            // when opening new connections.
            CP::TDbiServices::fAsciiDBConectionsTemporary = false;
        }
        else {
            while (--dbNo >= 0) {
                fCascader->SetPermanent(dbNo,false);
            }
            DbiInfo("Forcing all connections, including ASCII DB, to be temporary" << "  ");
            // Inform CP::TDbiServices so that CP::TDbiConnection can check
            // when opening new connections.
            CP::TDbiServices::fAsciiDBConectionsTemporary = true;
        }
    }

    // Check for request to order context queries and remove from the TDbiRegistry.

    int OrderContextQuery = 0;
    if (reg.Get("OrderContextQuery",OrderContextQuery)) {
        reg.RemoveKey("OrderContextQuery");
        if (OrderContextQuery) {
            CP::TDbiServices::fOrderContextQuery = true;
            DbiInfo("Forcing ordering of all context queries" << "  ");
        }
    }

    // Abort if TDbiRegistry contains any unknown keys

    const char* knownKeys[]   = { "Level2Cache",
                                  "Shutdown"
                                };
    int numKnownKeys          = sizeof(knownKeys)/sizeof(char*);
    bool hasUnknownKeys       = false;

    TDbiRegistry::TDbiRegistryKey keyItr(&this->GetConfig());
    while (const char* foundKey = keyItr()) {
        bool keyUnknown = true;
        for (int keyNum = 0; keyNum < numKnownKeys; ++keyNum) {
            if (! strcmp(foundKey,knownKeys[keyNum])) {
                keyUnknown = false;
            }
        }
        if (keyUnknown) {
            DbiSevere("FATAL: "
                      << "Illegal registry item: " << foundKey << "  ");
            hasUnknownKeys = true;
        }
    }

    if (hasUnknownKeys) {
        DbiSevere("FATAL: " << "Aborting due to illegal registry items." << "  ");
        throw  EBadTDbiRegistryKeys();
    }
}

//.....................................................................

CP::TDbiTableProxy&
CP::TDbiDatabaseManager::GetTableProxy(const std::string& tableNameReq,
                                       const CP::TDbiTableRow* tableRow) {
//
//
//  Purpose:  Locate, or if necessary create, CP::TDbiTableProxy for
//            named table.
//
//  Arguments:
//    tableNameReq in    Name of table requested.
//    tableRow     in    Example of a Table Row object.
//
//  Return:    CP::TDbiTableProxy for table.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Locate, or if necessary create, CP::TDbiTableProxy for named table.
//
//  o If creating apply prevailing SQL condition.

//  Program Notes:-
//  =============

//  None.

// Force upper case name.
    std::string tableName = CP::UtilString::ToUpper(tableNameReq);
    std::string proxyName = tableName;

    DbiTrace("Proxy request " << tableNameReq);
    DbiTrace("Proxy table   " << tableName);
    DbiTrace("Proxy name    " << proxyName);

    proxyName.append("::");
    proxyName.append(tableRow->ClassName());
    CP::TDbiTableProxy* qpp = fTPmap[proxyName];
    if (! qpp) {
        qpp = new CP::TDbiTableProxy(fCascader,tableName,tableRow);
        this->ApplySqlCondition(qpp);
        fTPmap[proxyName] = qpp;
    }

    return *qpp;

}

//.....................................................................

CP::TDbiDatabaseManager& CP::TDbiDatabaseManager::Instance() {
//
//
//  Purpose: Locate, or create, CP::TDbiDatabaseManager singleton.
//
//  Arguments:     None.
//
//  Return:    CP::TDbiDatabaseManager singleton.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Locate, or if necessary create, CP::TDbiDatabaseManager singleton.

//  Program Notes:-
//  =============

//  None.

    if (! fgInstance) {
// Delete is handled by Cleaner class based on #include count
        fgInstance = new CP::TDbiDatabaseManager();
    }
    return *fgInstance;

}

//.....................................................................

void CP::TDbiDatabaseManager::PurgeCaches() {
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

    for (std::map<std::string,CP::TDbiTableProxy*>::iterator itr = fTPmap.begin();
         itr != fTPmap.end();
         ++itr) {
        CP::TDbiTableProxy* tp = (*itr).second;
        tp->GetCache()->Purge();
    }

}

//.....................................................................

void CP::TDbiDatabaseManager::RefreshMetaData(const std::string& tableName) {
//
//
//  Purpose: Refresh meta data for specied table.
//
//  Arguments:
//    tableName    in    Name of table to be refreshed.


//  Program Notes:-
//  =============

//  This method is currently only used by CP::TDbiSqlValPacket after
//  it has created a new table in the database.  In such cases
//  the pre-existing corresponding TDbiTableProxy.hxxas to be refreshed.

    std::map<std::string,CP::TDbiTableProxy*>::iterator itr = fTPmap.begin();
    std::map<std::string,CP::TDbiTableProxy*>::iterator itrEnd = fTPmap.end();
    for (; itr != itrEnd; ++itr) {
        CP::TDbiTableProxy* table = (*itr).second;
        if (table && table->GetTableName() == tableName) {
            table->RefreshMetaData();
        }
    }

}
//.....................................................................

void CP::TDbiDatabaseManager::SetConfigFromEnvironment() {
//
//
//  Purpose:  Set up configuration from ENV_DBI environmental variable
//            which consists of a semi-colon separated list of DBI
//            configuration requests.

    const char* strENV_DBI = gSystem->Getenv("ENV_DBI");
    if (strENV_DBI == 0  || strlen(strENV_DBI) == 0) {
        return;
    }

    DbiInfo("\nConfiguring DatabaseInterface from the environmental "
            << "variable ENV_DBI:-\n  " << strENV_DBI << "  ");
    std::vector<std::string> configRequests;
    CP::UtilString::StringTok(configRequests, strENV_DBI, ";");

    for (unsigned entry = 0; entry < configRequests.size(); ++entry) {
        this->Set(configRequests[entry].c_str());
    }
    this->Update();
}

//.....................................................................

void CP::TDbiDatabaseManager::SetSqlCondition(const std::string& sql) {
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
//  o Record global SQL condition and apply to existing CP::TDbiTableProxys.


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

void CP::TDbiDatabaseManager::ShowStatistics() const {
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
        << "       Size      Size   Adopted    Reused" << std::endl;

// Loop over all owned objects.

    for (std::map<std::string,CP::TDbiTableProxy*>::const_iterator itr = fTPmap.begin();
         itr != fTPmap.end();
         ++itr) {
        const CP::TDbiTableProxy* tp = (*itr).second;
        std::string name = (*itr).first;
        if (name.size() < 40) {
            name.append(40-name.size(),' ');
        }
        msg << name;
//  Only want to look at cache so by-pass constness.
        const_cast<CP::TDbiTableProxy*>(tp)->GetCache()->ShowStatistics(msg);
        msg << std::endl;
    }
    msg << "\n" << std::endl;

//  Only want to look at cascader so by-pass constness.

    DbiInfo(const_cast<CP::TDbiDatabaseManager*>(this)->GetCascader());


}


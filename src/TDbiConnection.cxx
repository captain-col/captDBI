
//////////////////////////////////////////////////////////////////////////
////////////////////////////     ROOT API     ////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "TDbi.hxx"
#include "TDbiAsciiDbImporter.hxx"
#include "TDbiConnection.hxx"
#include "TDbiExceptionLog.hxx"
#include "TDbiServices.hxx"
#include "TDbiLog.hxx"

#include <TList.h>
#include <TString.h>
#include <TSystem.h>

#include <MsgFormat.hxx>

#include <cctype>
#include <cstdlib>
#include <list>
#include <sstream>
#include <string>

#include <TDbiLog.hxx>

ClassImp(CP::TDbiConnection)


//   Definition of static data members
//   *********************************


//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................
//      Throws CP::EBadConnection() if can not make connection
CP::TDbiConnection::TDbiConnection(const std::string& url      /* = "" */,
                                   const std::string& user     /* = "" */,
                                   const std::string& password /* = "" */,
                                   int maxConnects) :
    fUrlString(url),
    fUrl(url.c_str()),
    fUser(user),
    fPassword(password),
    fUrlValidated(false),
    fNumConnectedStatements(0),
    fIsTemporary(true),
    fServer(0) {

    fMaxConnectionAttempts = maxConnects;

    DbiTrace("Creating CP::TDbiConnection" << "  ");
    if (this->Open()) {
        DbiInfo("Successfully opened connection to: "
                << this->GetUrl() << "  ");
        fUrlValidated =  true;

        // Initialise the list existing supported tables.
        this->SetTableExists();

        //  If URL looks O.K., check that both client and server support
        //  prepared statements.
        if (fUrlValidated) {
            if (!fServer->HasStatement()) {
                DbiError("  This client does not support prepared statements."
                          << "  ");
                fUrlValidated = false;
            }

            std::string serverInfo(fServer->ServerInfo());
            if (!fServer->HasStatement()) {
                DbiError("This server (" << serverInfo
                          << ") does NOT support prepared statements." << "  ");
                fUrlValidated = false;
            }
            if (fUrlValidated) {
                DbiInfo("This client, and server (" << serverInfo
                        << ") supports prepared statements." << "  ");
            }
            else {
                DbiError(
                    std::endl
                    << "This server does not support prepared statements."
                    << std::endl
                    << "If using MYSQL please upgrade to version >4.1"
                    << std::endl);
            }

        }
    }
    if (!fUrlValidated) {
        DbiError("FATAL: " << "Aborting due to above errors" << "  ");
        throw CP::EBadConnection();
    }
    fDbName = fUrl.GetFile();
}

CP::TDbiConnection::~TDbiConnection() {
    DbiTrace("Destroying CP::TDbiConnection" << "  ");
    this->Close(true);

}

///  Close server connection unless active (or always if forced).  This
///  returns true if the connection was closed.
Bool_t CP::TDbiConnection::Close(Bool_t force /* = false */) {

    this->ClearExceptionLog();
    if (this->IsClosed()) {
        return true;
    }

    if (fNumConnectedStatements) {
        if (! force) {
            DbiInfo("Unable to close connection: " << this->GetUrl()
                    << "; it still has  "
                    << fNumConnectedStatements << "active statements. ");
            return false;
        }
        DbiInfo("Closing connection: " << this->GetUrl()
                << "; even though it still has "
                << fNumConnectedStatements << " active statements. ");
    }

    delete fServer;
    fServer = 0;
    DbiDebug("Closed connection: " << this->GetUrl() << "  ");
    return true;

}

///  Purpose: Close idle connection. Idle means there are no active
///  connections to this database.
void CP::TDbiConnection::CloseIdleConnection() {

    if (fIsTemporary &&  fNumConnectedStatements == 0) {
        this->Close();
    }

}

///  Open the connection if necessary and get a prepared statment.  This
///  returns a TSQLStatement pointer that is owned by the caller, or NULL if
///  there is a failure.
TSQLStatement* CP::TDbiConnection::CreatePreparedStatement(
    const std::string& sql) {

    TSQLStatement* stmt = 0;
    if (! this->Open()) {
        return stmt;
    }
    try {
        DbiTrace("CreatePreparedStatement: " << sql);
        stmt = fServer->Statement(sql.c_str());
    }
    catch (...) {
        DbiError("Catch statement: " << sql);
        if (stmt) delete stmt;
        stmt = NULL;
    }
    if (!stmt) {
        DbiWarn("Statement failed: " << sql);
        fExceptionLog.AddEntry(*fServer);
    }
    else {
        stmt->EnableErrorOutput(false);
    }

    return stmt;
}

///  Open the connection if necessary and return a pointer to a TSQLServer.
///
///  The server returned remains is being borrowed from the CP::TDbiConnection
///  and remains under its ownership and must not be deleted.  However the
///  caller must invoke the Connect() method on this CP::TDbiConnection before
///  borrowing it and must invoke the DisConnect() when it has finished using
///  it to ensure the CP::TDbiConnection does not close it prematurely i.e.:-
///
/// \code
///            void Demo(CP::TDbiConnection* con) {
///              con->Connect();
///              TSQLServer* server = con->GetServer();
///              // Do stuff
///              con->DisConnect();
///            }
///\endcode
TSQLServer* CP::TDbiConnection::GetServer() {
    if (! this->Open()) {
        return 0;
    }
    return fServer;
}

/// Return a reference to a string containing the URL.  The string is shared,
/// so make a copy of it before subsequent call to this function.
///
/// \note Don't ask me why TUrl::GetUrl() is non-const, just accept that it
/// is!
const std::string& CP::TDbiConnection::GetUrl() const {



    static std::string url;
    url = const_cast<CP::TDbiConnection*>(this)->fUrl.GetUrl();
    return url;

}

///  Open the connection if necessary.
Bool_t CP::TDbiConnection::Open() {

    this->ClearExceptionLog();
    if (!this->IsClosed()) {
        return true;
    }

    if (!fUrl.IsValid()) {
        std::ostringstream oss;
        oss << "Unable to open connection: URL '"
            << fUrlString << "' is invalid";
        DbiSevere(oss.str() << "  ");
        fExceptionLog.AddEntry(oss.str());
        return false;
    }

    // Make several attempts (or more if URL is known to be O.K.) to open
    // connection.
    int maxAttempt = fUrlValidated ?  100: fMaxConnectionAttempts ;
    for (int attempt = 1; attempt <= maxAttempt; attempt++) {
        fServer = TSQLServer::Connect(fUrlString.c_str(),
                                      fUser.c_str(),
                                      fPassword.c_str());
        if (!fServer) {
            std::ostringstream oss;
            oss << "Failing to open: " << fUrlString 
                << " for user " << fUser
                << " and password " << fPassword 
                << " (attempt " << attempt << ")";
            fExceptionLog.AddEntry(oss.str());
            if (fMaxConnectionAttempts > attempt) {
                if (attempt == 1) {
                    DbiSevere(" retrying ... " << "  ");
                }
                DbiLog(" Waiting "<<attempt<<" seconds before trying again");
                gSystem->Sleep(attempt*1000);
            }
        }

        else {
            fServer->EnableErrorOutput(false);
            if (attempt > 1) {
                DbiWarn("... Connection opened on attempt " << attempt << "  ");
            }
            DbiDebug("Successfully opened connection to: " 
                     << fUrlString << "  ");

            // If this is an ASCII database, populate it and make the
            // connection permanent unless even ASCII DB connections are
            // temporary.
            TString ascii_file = fUrl.GetAnchor();
            if (ascii_file.IsNull()) {
                return true;
            }
            gSystem->Setenv("DBI_CATALOGUE_PATH",
                            gSystem->DirName(fUrl.GetAnchor()));
            CP::TDbiAsciiDbImporter importer(ascii_file,fServer);
            const CP::TDbiExceptionLog& el(importer.GetExceptionLog());
            if (! el.IsEmpty()) {
                delete fServer;
                fServer = 0;
                return false;
            }
            fIsTemporary = CP::TDbiServices::AsciiDBConectionsTemporary();
            // Add imported tables names.
            const std::list<std::string> tableNames(
                importer.GetImportedTableNames());
            std::list<std::string>::const_iterator itr(tableNames.begin());
            std::list<std::string>::const_iterator itrEnd(tableNames.end());
            while (itr != itrEnd) {
                this->SetTableExists(*itr);
                ++itr;
            }
            return true;
        }
    }
    DbiSevere("... Failed to open a connection to: " << fUrlString
              << " for user " << fUser << " and pwd " << fPassword << "  ");

    return false;

}

/// Print all warning at supplied Msg log level.  This returns true if
///warnings have occurred.
Bool_t CP::TDbiConnection::PrintExceptionLog(Int_t level) const {

    switch (level) {
    case TDbiLog::QuietLevel:
        break;
    case TDbiLog::LogLevel:
        DbiInfo(fExceptionLog);
        break;
    case TDbiLog::InfoLevel:
        DbiInfo(fExceptionLog);
        break;
    case TDbiLog::VerboseLevel:
        DbiVerbose(fExceptionLog);
        break;
    }

    return fExceptionLog.Size() != 0;
}

///  Record an exception that has occurred while a client was using its
///  TSQLServer.
void  CP::TDbiConnection::RecordException() {
    fExceptionLog.AddEntry(*fServer);
}

/// Add name to list of existing tables (necessary when creating tables).  If
/// tableName is empty then refresh list from the database.
void  CP::TDbiConnection::SetTableExists(const std::string& tableName) {

    if (tableName.empty()) {
        TList* tableList = fServer->GetTablesList();
        TIter next(tableList);
        while (TObject* obj = next()) {
            std::string tn(obj->GetName());
            this->SetTableExists(tn);
        }
        delete tableList;
    }
    else {
        if (! this->TableExists(tableName)) {
            fExistingTableList += ",'";
            fExistingTableList += tableName;
            fExistingTableList += "'";
        }
    }
}

///  Purpose: Check to see table exists in connected database.
Bool_t  CP::TDbiConnection::TableExists(const std::string& tableName) const {
    std::string test("'");
    test += tableName;
    test += "'";
    return fExistingTableList.find(test) != std::string::npos;
}

///  Test if this connection supports Temporary Tables.  This is a consequence
///   of privileges granted to the user of the connection, rather than a
///   property of the database itself.  This returns true if Temporary Tables
///   are supported or false Temporary Tables are not supported.
///
///  \note False may also indicate there was another undefined problem with
///  the connection.  (It would be better to raise an exception when these
///  edge cases occur).
bool CP::TDbiConnection::SupportsTmpTbls() {
    bool retVal = false;
    TSQLStatement* stmt 
        = CreatePreparedStatement("CREATE TEMPORARY TABLE"
                                  " TEST_TDbiConnection_SupportsTmpTbls"
                                  " ( id integer );");

    if (stmt) {
        if (stmt->Process()) {
            TSQLStatement* stmt2 
                = CreatePreparedStatement(
                    "DROP TABLE TEST_TDbiConnection_SupportsTmpTbls;");
            if (stmt2 && stmt2->Process()) {
                retVal = true;
            }
            delete stmt2;
        }
    }

    delete stmt;
    return retVal;
}


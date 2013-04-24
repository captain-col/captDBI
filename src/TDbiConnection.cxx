
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

    fUrl(url.c_str()),
    fUser(user),
    fPassword(password),
    fUrlValidated(false),
    fNumConnectedStatements(0),
    fIsTemporary(true),
    fServer(0) {
//
//
//  Purpose:  Default constructor

    fMaxConnectionAttempts = maxConnects;

    DbiTrace("Creating CP::TDbiConnection" << "  ");

    if (this->Open()) {
        DbiInfo("Successfully opened connection to: " << this->GetUrl() << "  ");
        fUrlValidated =  true;

        // Initialise the list existing supported tables.
        this->SetTableExists();

        //  If URL looks O.K., check that both client and server support prepared statements.
        if (fUrlValidated) {
            if (! fServer->HasStatement()) {
                DbiSevere("  This client does not support prepared statements." << "  ");
                fUrlValidated = false;
            }

            std::string serverInfo(fServer->ServerInfo());
            if (serverInfo < "4.1") {
                DbiSevere("This MySQL server (" << serverInfo
                          << ") does NOT support prepared statements." << "  ");
                fUrlValidated = false;
            }
            if (fUrlValidated) {
                DbiInfo("This client, and MySQL server (" << serverInfo
                        << ") supports prepared statements." << "  ");
            }
            else {

                DbiSevere("\n"
                          << "This version of MySQL does not support prepared statements.\n"
                          << "\n"
                          << "Please upgrade to MySQL (client and server) version 4.1 or greater \n"
                          << "\n"
                          << "  ");
            }

        }
    }
    if (! fUrlValidated) {
        DbiSevere("FATAL: " << "Aborting due to above errors" << "  ");
        throw CP::EBadConnection();
    }
    fDbName = fUrl.GetFile();


}

//.....................................................................

CP::TDbiConnection::~TDbiConnection() {
//
//
//  Purpose: Destructor


    DbiTrace("Destroying CP::TDbiConnection" << "  ");
    this->Close(true);

}
//.....................................................................
///\verbatim
///
///  Purpose: Close server connection unless active (or always if forced) .
///
///  Return:  true if connection now closed.
///\endverbatim
Bool_t CP::TDbiConnection::Close(Bool_t force /* = false */) {

    this->ClearExceptionLog();
    if (this->IsClosed()) {
        return true;
    }

    if (fNumConnectedStatements) {
        if (! force) {
            DbiInfo("Unable to close connection: " << this->GetUrl()
                    << "; it still has  "
                    << fNumConnectedStatements << "active statements. " << "  ");
            return false;
        }
        DbiInfo("Closing connection: " << this->GetUrl()
                << "; even though it still has "
                << fNumConnectedStatements << " active statements. " << "  ");
    }

    delete fServer;
    fServer = 0;
    DbiDebug("Closed connection: " << this->GetUrl() << "  ");
    return true;

}

//.....................................................................
///
///
///  Purpose:  Close idle connection. Idle means there are no active connections to this database.

void CP::TDbiConnection::CloseIdleConnection() {

    if (fIsTemporary &&  fNumConnectedStatements == 0) {
        this->Close();
    }

}


//.....................................................................
///\verbatim
///
///  Purpose:  Open if necessary and get a prepared statment.
///
///  Return:    Statement - Caller must take ownership.
///             will be 0 if failure.
///\endverbatim
TSQLStatement* CP::TDbiConnection::CreatePreparedStatement(const std::string& sql) {

    TSQLStatement* stmt = 0;
    if (! this->Open()) {
        return stmt;
    }
    stmt = fServer->Statement(sql.c_str());
    if (! stmt) {
        fExceptionLog.AddEntry(*fServer);
    }
    else {
        stmt->EnableErrorOutput(false);
    }

    return stmt;
}
//.....................................................................
///\verbatim
///
///  Purpose:  Open if necessary and get a TSQLServer.
///
///  Return:    Server ( = 0 if connection not open).
///
///  WARNING:  The server returned remains is being borrowed from the
///            CP::TDbiConnection and remains under its ownership and must
///            not be deleted.  However the caller must invoke the
///            Connect() method on this CP::TDbiConnection before borrowing
///            it and must invoke the DisConnect() when it has finished
///            using it to ensure the CP::TDbiConnection does not close it
///            prematurely i.e.:-
///
///            void Demo(CP::TDbiConnection* con) {
///              con->Connect();
///              TSQLServer* server = con->GetServer();
///              // Do stuff
///              con->DisConnect();
///            }
///\endverbatim
TSQLServer* CP::TDbiConnection::GetServer() {


    if (! this->Open()) {
        return 0;
    }
    return fServer;
}

//.....................................................................
///\verbatim
/// Don't ask me why TUrl::GetUrl() is non-const, just accept that it is!
///
/// Note: This function returns a reference to a shared string; use the
///       value or make a copy of it before any subsequent call to this
///       function.
///\endverbatim
const std::string& CP::TDbiConnection::GetUrl() const {



    static std::string url;
    url = const_cast<CP::TDbiConnection*>(this)->fUrl.GetUrl();
    return url;

}

//.....................................................................
///
///  Purpose:  Open connection if necessary.
///

Bool_t CP::TDbiConnection::Open() {

    this->ClearExceptionLog();
    if (! this->IsClosed()) {
        return true;
    }

    if (! fUrl.IsValid()) {
        std::ostringstream oss;
        oss << "Unable to open connection: URL '" << fUrl.GetUrl() << "' is invalid";
        DbiSevere(oss.str() << "  ");
        fExceptionLog.AddEntry(oss.str());
        return false;
    }

    // Make several attempts (or more if URL is known to be O.K.) to open connection.
    int maxAttempt = fUrlValidated ?  100: fMaxConnectionAttempts ;
    for (int attempt = 1; attempt <= maxAttempt; attempt++) {
        fServer = TSQLServer::Connect(fUrl.GetUrl(),fUser.c_str(),fPassword.c_str());
        if (! fServer) {
            std::ostringstream oss;
            oss << "Failing to open: " << fUrl.GetUrl() << " for user " << fUser
                << " and password " << fPassword << " (attempt " << attempt << ")";
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
            DbiDebug("Successfully opened connection to: " << fUrl.GetUrl() << "  ");

            // If this is an ASCII database, populate it and make the connection permanent
            // unless even ASCII DB connections are temporary.

            TString ascii_file = fUrl.GetAnchor();
            if (ascii_file.IsNull()) {
                return true;
            }
            gSystem->Setenv("DBI_CATALOGUE_PATH",
                            gSystem->DirName(fUrl.GetAnchor()));
            CP::TDbiAsciiDbImporter importer(ascii_file,fServer);
            const CP::TDbiExceptionLog& el(importer.GetExceptionLog());
            if (! el.IsEmpty()) {
//                DbiSevere( "Failed to populate ASCII database from "
//                           << fUrl.GetUrl() << " " << el << "  ");
                delete fServer;
                fServer = 0;
                return false;
            }
            fIsTemporary = CP::TDbiServices::AsciiDBConectionsTemporary();
            // Add imported tables names.
            const std::list<std::string> tableNames(importer.GetImportedTableNames());
            std::list<std::string>::const_iterator itr(tableNames.begin()), itrEnd(tableNames.end());
            while (itr != itrEnd) {
                this->SetTableExists(*itr);
                ++itr;
            }
            return true;

        }
    }
    DbiSevere("... Failed to open a connection to: " << fUrl.GetUrl()
              << " for user " << fUser << " and pwd " << fPassword << "  ");

    return false;

}//
//  Purpose:  Record an exception that has occurred while a client was using its TSQLServer.


//.....................................................................
///\verbatim
///
///  Purpose:  Print all warning at supplied  Msg log level.
///
///  Return:    kTRUE if warnings have occurred
///\endverbatim
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

//.....................................................................
///
///  Purpose:  Record an exception that has occurred while a client was using its TSQLServer.

void  CP::TDbiConnection::RecordException() {

    fExceptionLog.AddEntry(*fServer);

}

//.....................................................................
///\verbatim
///  Purpose: Add name to list of existing tables (necessary when creating tables)
///
///  Note: If tableName is null refresh list from the database.
///\enbdverbatim
void  CP::TDbiConnection::SetTableExists(const std::string& tableName) {

    if (tableName == "") {
        TSQLStatement* stmt =  CreatePreparedStatement("show tables");
        if (stmt) {
            if (stmt->Process()) {
                stmt->StoreResult();
                while (stmt->NextResultRow()) {
                    std::string tn(stmt->GetString(0));
                    this->SetTableExists(tn);
                }
            }
            delete stmt;
            stmt = 0;
        }
    }
    else {
        if (! this->TableExists(tableName)) {
            fExistingTableList += ",'";
            fExistingTableList += tableName;
            fExistingTableList += "'";
        }
    }
}

//.....................................................................
///  Purpose: Check to see table exists in connected database.
Bool_t  CP::TDbiConnection::TableExists(const std::string& tableName) const {
//

    std::string test("'");
    test += tableName;
    test += "'";
    return fExistingTableList.find(test) != std::string::npos;
}

/// \name T2K-Extensions
//@{


//.....................................................................
///  \brief Test if this connection supports Temporary Tables.
///
///   Note: this is a consequence of privileges granted to the user of
///   the connection, rather than a property of the database itself.
///
///  \return    true Temporary Tables supported or false Temporary Tables not supported
///
///             Note: false may also indicate there was another undefined problem
///                   with the connection.  (It would be better to raise an
///                   exception when these edge cases occur).
///
///  \author Simon Claret t2kcompute@comp.nd280.org

bool CP::TDbiConnection::SupportsTmpTbls() {
    bool retVal = false;
    TSQLStatement* stmt = CreatePreparedStatement("CREATE TEMPORARY TABLE TEST_TDbiConnection_SupportsTmpTbls ( id integer );");

    if (stmt) {
        if (stmt->Process()) {
            TSQLStatement* stmt2 = CreatePreparedStatement("DROP TABLE TEST_TDbiConnection_SupportsTmpTbls;");
            if (stmt2 && stmt2->Process()) {
                retVal = true;
            }
            delete stmt2;
        }
    }
    else {
        // Failed to prepare stmt
        ; // Should hit this case if the connection doesn't support temp tables
    }

    delete stmt;
    return retVal;
}
//@}


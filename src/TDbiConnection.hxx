#ifndef TDbiConnection_hxx_seen
#define TDbiConnection_hxx_seen

/// \file

//////////////////////////////////////////////////////////////////////////
////////////////////////////     ROOT API     ////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \class CP::TDbiConnection
///
/// \brief
/// <b>Concept</b> A managed TSQLServer connection- dropped when idle.
///
/// \brief
/// <b>Purpose</b> To minimise connections.
/// Contact: A.Finch@lancaster.ac.uk
///
#include <string>

#ifndef ROOT_Rtypes
#if !defined(__CINT__) || defined(__MAKECINT__)
#include "Rtypes.h"
#endif
#endif
#include "TSQLServer.h"
#include "TSQLStatement.h"
#include "TUrl.h"

#include "TDbi.hxx"
#include "TDbiExceptionLog.hxx"

namespace CP {
    class TDbiConnection;
};


class CP::TDbiConnection {
public:

    /// Constructors and destructors.
    /// Passed in:
    ///    * url - address of sql server
    ///    * user - username to use
    ///    * password - password to use
    ///    * macConnects = maximum number of connections to attempt before
    ///      giving up, default to 20
    ///
    /// This throws CP::EBadConnection() if can not make connection
    ///
    TDbiConnection(const std::string& url = "",
                   const std::string& user = "",
                   const std::string& password = "",
                   int maxConnects=20);
    virtual ~TDbiConnection();

    const std::string& GetDbName() const {
        return fDbName;
    }
    const std::string& GetPassword() const {
        return fPassword;
    }
    const std::string& GetUrl() const;
    const std::string& GetUser() const {
        return fUser;
    }
    Bool_t IsClosed() const {
        return ! fServer;
    }
    Bool_t IsTemporary() const {
        return fIsTemporary;
    }
    Bool_t TableExists(const std::string& tableName) const;

    bool SupportsTmpTbls(); // (T2K Extension)

    /// Exception log handling
    const TDbiExceptionLog& GetExceptionLog() const {
        return fExceptionLog;
    }

    void ClearExceptionLog() {
        fExceptionLog.Clear();
    }

    /// Print exceptions at level of above and return true if any
    Bool_t PrintExceptionLog(Int_t level = 3) const;

    void RecordException();

    /// Add name to list of existing tables (necessary when creating
    /// tables) Default name = "", reread all tables from database.
    void SetTableExists(const std::string& tableName = "");

    /// Methods used when "borrowing" the server (use same as for
    /// Statement).
    void Connect()    {
        this->ConnectStatement();
    }
    void DisConnect() {
        this->DisConnectStatement();
    }

    /// Increment number of statements relying on this connection
    void ConnectStatement() {
        ++fNumConnectedStatements;
    }

    /// Decrement number of statements relying on this connection and
    /// close if idle
    void DisConnectStatement() {
        --fNumConnectedStatements;
        if (! fNumConnectedStatements) {
            this->CloseIdleConnection();
        }
    }

    /// Connection is permanent, don't close even when idle.
    void SetPermanent(Bool_t permanent = true) {
        fIsTemporary = ! permanent;
    }


    Bool_t Close(Bool_t force = false);
    Bool_t Open();

    /// Get server, opening if necessary
    /// TDbiConnection retains ownership
    TSQLServer* GetServer();

    /// Get statement, opening if necessary.
    /// Caller must take ownership.
    TSQLStatement* CreatePreparedStatement(const std::string& sql);

private:

    void CloseIdleConnection();


    /// Database Name.
    std::string fDbName;

    ///  A comma separate list of existing tables each in single quotes :
    ///  'table1','table2',...
    std::string fExistingTableList;

    /// TSQLServer URL
    TUrl fUrl;

    /// Username
    std::string fUser;

    /// Password
    std::string fPassword;

    /// True if URL works
    Bool_t fUrlValidated;

    /// Maximum number of times to try making a connection.
    int fMaxConnectionAttempts;

    /// Number of connected statements
    Int_t fNumConnectedStatements;

    /// Connection closes after each I/O (no connections left)
    Bool_t fIsTemporary;

    /// TSQLServer or 0 if closed
    TSQLServer* fServer;

    /// Log of exceptions generated.  Cleared by Open Close and
    /// (implicitly) by CreatePreparedStatement, GetServer
    TDbiExceptionLog fExceptionLog;

    ClassDef(TDbiConnection,0)     // Managed TSQLServer

};
#endif


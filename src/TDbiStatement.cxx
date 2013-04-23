
//////////////////////////////////////////////////////////////////////////
////////////////////////////     ROOT API     ////////////////////////////
//////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

#include "TString.h"

#include "TDbiStatement.hxx"
#include "TDbiTableMetaData.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>

#include "UtilString.hxx"

ClassImp(CP::TDbiStatement)


//   Definition of static data members
//   *********************************


//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

CP::TDbiStatement::TDbiStatement(CP::TDbiConnection& conDb) :
fConDb(conDb)
{
//
//
//  Purpose:  Constructor
//
//  Arguments:  None.
//
//  Return:
//
//  conDb    in    The connection associated with the statement.
//


    DbiTrace( "Creating CP::TDbiStatement" << "  ");
    fConDb.ConnectStatement();

}

//.....................................................................

CP::TDbiStatement::~TDbiStatement() {
//
//
//  Purpose: Destructor

    DbiTrace( "Destroying CP::TDbiStatement" << "  ");

    fConDb.DisConnectStatement();
}

//.....................................................................

TSQLStatement* CP::TDbiStatement::CreateProcessedStatement(const TString& sql /* ="" */) {

// Attempt to create a processed statement (caller must delete).  Return 0 if failure.

    TSQLStatement* stmt = fConDb.CreatePreparedStatement(sql.Data());
    if ( ! stmt ) {
        this->AppendExceptionLog(fConDb);
        return 0;
    }
    if ( stmt->Process() ) return stmt;
    this->AppendExceptionLog(stmt);
    delete stmt;
    stmt = 0;
    return 0;

}


//.....................................................................

TSQLStatement* CP::TDbiStatement::ExecuteQuery( const TString& sql) {
//
//
//  Purpose:  Execute SQL.
//  Return:   TSQLStatement with Process() and StoreResult() already performed.

    this->ClearExceptionLog();

    DbiInfo( "SQL:" << fConDb.GetDbName() << ":" << sql << "  ");
    TSQLStatement* stmt = this->CreateProcessedStatement(sql);
    if ( ! stmt ) return 0;
    if ( ! stmt->StoreResult() ) {
        this->AppendExceptionLog(stmt);
        delete stmt;
        stmt = 0;
    }

    // Final sanity check: If there is a statement then the exception log should still
    // be clear otherwise it should not be.
    if ( stmt ) {
        if ( ! fExceptionLog.IsEmpty() ) {
            delete stmt;
            stmt = 0;
        }
    }
    else if ( fExceptionLog.IsEmpty() ) {
        std::ostringstream oss;
        oss << "Unknown failure (no execption but no TSQLStatement either executing " << sql;
        fExceptionLog.AddEntry(oss.str().c_str());
    }
    return stmt;

}

//.....................................................................

Bool_t CP::TDbiStatement::ExecuteUpdate( const TString& sql) {
//
//
//  Purpose:  Translate SQL if required and Execute.
//
//  Return true if all updates successful.


    this->ClearExceptionLog();

    DbiInfo( "SQL:" << fConDb.GetDbName() << ":" << sql << "  ");
    bool ok = fConDb.GetServer()->Exec(sql.Data());
    if ( ! ok ) {
        fConDb.RecordException();
        this->AppendExceptionLog(fConDb);
        return false;
    }

    return fExceptionLog.IsEmpty();

}

//.....................................................................

Bool_t CP::TDbiStatement::PrintExceptions(Int_t level) const {

//  Purpose:  Print accumulated exceptions at supplied Msg level,
//            add them to the Global Exception Log if level >= kWarning
//            and return true if there are any.

    const CP::TDbiExceptionLog& el(this->GetExceptionLog());
    if ( el.IsEmpty() ) return false;
    if(level <= TDbiLog::GetLogLevel())  TDbiLog::GetLogStream ()<<el;
    if ( level >= CP::TDbiLog::WarnLevel  )  CP::TDbiExceptionLog::GetGELog().AddLog(el);
    return true;

}



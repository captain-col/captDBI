#ifndef DBISTATEMENT
#define DBISTATEMENT


//////////////////////////////////////////////////////////////////////////
////////////////////////////     ROOT API     ////////////////////////////
//////////////////////////////////////////////////////////////////////////

/**
 *
 * $Id: TDbiStatement.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 * \class CP::TDbiStatement
 *
 *
 * \brief <b>Concept</b> A connected reusable statement with accompanying
 * exception log.  After deleting, the associated server connection is dropped
 * if then idle.
 *
 * \brief <b>Purpose</b> To minimise connections and to simplify interfacing
 * to different database backends (in the original MINOS implementation, in
 * this implementation only MySQL supported).
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include <list>

#include "TList.h"
#include "TString.h"
#include "TSQLStatement.h"

#include "TDbi.hxx"
#include "TDbiExceptionLog.hxx"
#include "TDbiConnection.hxx"

namespace CP {
class TDbiException;
}

namespace CP {
class TDbiStatement
{

public:

// Constructors and destructors.
           TDbiStatement(TDbiConnection& conDb);
  virtual ~TDbiStatement();

// State testing member functions

// Exception log handling.

/// Print accumulated exceptions at supplied Msg level,
/// add them to the Global Exception Log if level >= kWarning
/// and return true if there are any.
  Bool_t PrintExceptions(Int_t level = 3) const;

  const TDbiExceptionLog& GetExceptionLog() const { return fExceptionLog; }

// State changing member functions

// Database I/O

/// Give caller a TSQLStatement of results (Process() and StoreResult() already performed.).
  TSQLStatement* ExecuteQuery(const TString& sql="");

/// Apply an update and return success/fail.
  Bool_t ExecuteUpdate( const TString& sql="");

private:

  void AppendExceptionLog(TDbiException* e)  { if ( e ) fExceptionLog.AddEntry(*e); }
  void AppendExceptionLog(TSQLStatement* s) { if ( s ) fExceptionLog.AddEntry(*s); }
  void AppendExceptionLog(TDbiConnection& c) {          fExceptionLog.AddLog(c.GetExceptionLog()); }
  void ClearExceptionLog()                  { fExceptionLog.Clear(); }

  TSQLStatement* CreateProcessedStatement(const TString& sql="");

// Data members

///Connection associated with this statement.
  TDbiConnection& fConDb;

/// A log of reported exceptions.
/// Cleared by calling ExecuteQuery, ExecuteUpdate
  TDbiExceptionLog fExceptionLog;

 ClassDef(TDbiStatement,0)     // Managed TSQL_Statement

};
};


#endif // DBISTATEMENT



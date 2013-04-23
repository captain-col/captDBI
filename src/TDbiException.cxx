// $Id: TDbiException.cxx,v 1.1 2011/01/18 05:49:19 finch Exp $


#include <iostream>

#include "TSQLServer.h"
#include "TSQLStatement.h"

#include "TDbiException.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;

ClassImp(ND::TDbiException)

//   Definition of static data members
//   *********************************


//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

/// Create an exception using the msg and code provided.
ND::TDbiException::TDbiException(const char* msg /*= 0*/,
                           Int_t code      /* = -1 */):
fMessage(msg),
fErrorCode(code)
{

  DbiTrace( "Creating ND::TDbiException" << "  ");

}

//.....................................................................
/// Create an exception, getting the error message and code from server.
ND::TDbiException::TDbiException(const TSQLServer& server):
fMessage(server.GetErrorMsg()),
fErrorCode(server.GetErrorCode())
{

  DbiTrace( "Creating ND::TDbiException" << "  ");

}

//.....................................................................
/// Create and exception, taking the message and code from statement.
ND::TDbiException::TDbiException(const TSQLStatement& statement):
fMessage(statement.GetErrorMsg()),
fErrorCode(statement.GetErrorCode())
{

  DbiTrace( "Creating ND::TDbiException" << "  ");

}

//.....................................................................


ND::TDbiException::TDbiException(const ND::TDbiException& that)
{

  DbiTrace( "Creating ND::TDbiException" << "  ");

  *this = that;

}

//.....................................................................


ND::TDbiException::~TDbiException() {


  DbiTrace( "Destroying ND::TDbiException" << "  ");

}

//.....................................................................
/// Append message describing this exception to the std::ostream 'os'.
std::ostream& operator<<(std::ostream& os, const ND::TDbiException& e) {

  os << "Error " << e.GetErrorCode()
     << " (" << e.GetMessage() << ")"  << endl;
  return os;

}


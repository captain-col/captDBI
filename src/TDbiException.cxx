// $Id: TDbiException.cxx,v 1.1 2011/01/18 05:49:19 finch Exp $


#include <iostream>

#include "TSQLServer.h"
#include "TSQLStatement.h"

#include "TDbiException.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>

ClassImp(CP::TDbiException)

//   Definition of static data members
//   *********************************


//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

/// Create an exception using the msg and code provided.
CP::TDbiException::TDbiException(const char* msg /*= 0*/,
                           Int_t code      /* = -1 */):
fMessage(msg),
fErrorCode(code)
{

  DbiTrace( "Creating CP::TDbiException" << "  ");

}

//.....................................................................
/// Create an exception, getting the error message and code from server.
CP::TDbiException::TDbiException(const TSQLServer& server):
fMessage(server.GetErrorMsg()),
fErrorCode(server.GetErrorCode())
{

  DbiTrace( "Creating CP::TDbiException" << "  ");

}

//.....................................................................
/// Create and exception, taking the message and code from statement.
CP::TDbiException::TDbiException(const TSQLStatement& statement):
fMessage(statement.GetErrorMsg()),
fErrorCode(statement.GetErrorCode())
{

  DbiTrace( "Creating CP::TDbiException" << "  ");

}

//.....................................................................


CP::TDbiException::TDbiException(const CP::TDbiException& that)
{

  DbiTrace( "Creating CP::TDbiException" << "  ");

  *this = that;

}

//.....................................................................


CP::TDbiException::~TDbiException() {


  DbiTrace( "Destroying CP::TDbiException" << "  ");

}

//.....................................................................
/// Append message describing this exception to the std::ostream 'os'.
std::ostream& CP::operator<<(std::ostream& os, const CP::TDbiException& e) {

  os << "Error " << e.GetErrorCode()
     << " (" << e.GetMessage() << ")"  << std::endl;
  return os;

}


// $Id: TDbiConnectionMaintainer.cxx,v 1.1 2011/01/18 05:49:19 finch Exp $


#include "TDbiCascader.hxx"
#include "TDbiConnectionMaintainer.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;

ClassImp(ND::TDbiConnectionMaintainer)

//   Definition of static data members
//   *********************************



ND::TDbiConnectionMaintainer::TDbiConnectionMaintainer(ND::TDbiCascader* cascader) {

// For database connections that are not permanent, the connection is
// closed as soon as the last associated ND::TDbiStatement is
// deleted. However, standard context queries involve several separate
// SQL queries, each using its own ND::TDbiStatement and hence each involving
// a separate connection.  To maintain a single connection for the entire
// set of SQL queries a ND::TDbiConnectionMaintainer can be use.  On creation
// it connects to every ND::TDbiConnection which prevents it from closing.  On
// destruction it disconnects from them allowing them to close, if
// temporary, once any other associated TDbiStatement.hxxas been deleted.


  DbiTrace( "Creating ND::TDbiConnectionMaintainer" << "  ");

  fCascader = cascader;
  if ( fCascader ) fCascader->HoldConnections();

}
//.....................................................................


ND::TDbiConnectionMaintainer::~TDbiConnectionMaintainer() {

  if ( fCascader ) fCascader->ReleaseConnections();


  DbiTrace( "Destroying ND::TDbiConnectionMaintainer" << "  ");

}

/*    Template for New Member Function

//.....................................................................

ND::TDbiConnectionMaintainer:: {
//
//
//  Purpose:
//
//  Arguments:
//    xxxxxxxxx    in    yyyyyy
//
//  Return:
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o

//  Program Notes:-
//  =============

//  None.


}

*/



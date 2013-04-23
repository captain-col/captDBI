// $Id: TDbiTableRow.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $

#include "TDbiTableRow.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;

ClassImp(ND::TDbiTableRow)

//   Definition of static data members
//   *********************************



//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

ND::TDbiTableRow::TDbiTableRow():
  fOwner(0)
{
//
//
//  Purpose:  Default constructor


    DbiTrace( "Creating ND::TDbiTableRow" << "  ");
}
//.....................................................................

ND::TDbiTableRow::TDbiTableRow(const ND::TDbiTableRow& from) : TObject(from)
{
//
//
//  Purpose:  Copy constructor.


  DbiTrace( "Creating copy ND::TDbiTableRow" << "  ");

//  Nothing to copy; no data member in ND::TDbiTableRow.

}

//.....................................................................

ND::TDbiTableRow::~TDbiTableRow() {
//
//
//  Purpose: Destructor


  DbiTrace( "Destroying ND::TDbiTableRow" << "  ");

}



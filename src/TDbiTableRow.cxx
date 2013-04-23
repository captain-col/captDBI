// $Id: TDbiTableRow.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $

#include "TDbiTableRow.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;

ClassImp(CP::TDbiTableRow)

//   Definition of static data members
//   *********************************



//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

CP::TDbiTableRow::TDbiTableRow():
  fOwner(0)
{
//
//
//  Purpose:  Default constructor


    DbiTrace( "Creating CP::TDbiTableRow" << "  ");
}
//.....................................................................

CP::TDbiTableRow::TDbiTableRow(const CP::TDbiTableRow& from) : TObject(from)
{
//
//
//  Purpose:  Copy constructor.


  DbiTrace( "Creating copy CP::TDbiTableRow" << "  ");

//  Nothing to copy; no data member in CP::TDbiTableRow.

}

//.....................................................................

CP::TDbiTableRow::~TDbiTableRow() {
//
//
//  Purpose: Destructor


  DbiTrace( "Destroying CP::TDbiTableRow" << "  ");

}



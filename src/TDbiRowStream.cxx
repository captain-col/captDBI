///////////////////////////////////////////////////////////////////////
// $Id: TDbiRowStream.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $
//
// ND::TDbiRowStream
//
// Package: ND::TDbi (Database Interface).

// Begin_Html<img src="../../pedestrians.gif" align=center>
// <a href="../source_warning.html">Warning for beginners</a>.<br>
// Also see <a href="../../root_crib/index.html">The ROOT Crib</a> and
// <a href="../index.html">The MINOS Class User Guide</a>End_Html
//
// N. West 04/2001
//
// Concept: I/O buffer for a row of a table.
//
// Purpose: This forms the base class for ND::TDbiInRowStream (input)
//          and ND::TDbiWriter (input)
//
///////////////////////////////////////////////////////////////////////

#include "TDbiTableMetaData.hxx"
#include "TDbiRowStream.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;

#include "UtilString.hxx"

ClassImp(ND::TDbiRowStream)


//   Definition of static data members
//   *********************************



//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

ND::TDbiRowStream::TDbiRowStream(const ND::TDbiTableMetaData* metaData) :
fCurCol(1),
fMetaData(metaData),
fIsVLDTable(false)
{
//
//
//  Purpose:  Default constructor
//
//  Arguments:
//     metaData   in  Meta data for table.
//     tableName  in  Table name.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o  Create RowStream.


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Creating ND::TDbiRowStream" << "  ");

  std::string suffix(this->TableName(),this->TableName().size()-3);
  fIsVLDTable = suffix == "VLD";

}


//.....................................................................

ND::TDbiRowStream::~TDbiRowStream() {
//
//
//  Purpose: Destructor
//
//  Arguments:
//    None.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o  Destroy RowStream.


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Destroying ND::TDbiRowStream" << "  ");

}

//.....................................................................

const ND::TDbiFieldType& ND::TDbiRowStream::ColFieldType(UInt_t col) const {
//
//
//  Purpose: Return specified column type, if defined
//
//  Arguments: None.

  return fMetaData->ColFieldType(col);
}
//.....................................................................

string ND::TDbiRowStream::ColName(UInt_t col) const {
//
//
//  Purpose: Return specified column name, if defined

  return fMetaData->ColName(col);
}
//.....................................................................

const ND::TDbiFieldType& ND::TDbiRowStream::CurColFieldType() const {
//
//
//  Purpose: Return current column type, if defined
//
//  Arguments: None.
//
//  Return: Current column type, if defined. Unknown otherwise.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Current column name, if defined. unknown otherwise.

//  Program Notes:-
//  =============

//  None.

  return fMetaData->ColFieldType(fCurCol);
}

//.....................................................................

string ND::TDbiRowStream::CurColName() const {
//
//
//  Purpose: Return current column name, if defined
//
//  Arguments: None.
//
//  Return: Current column name, if defined. "Undefined" otherwise.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Current column name, if defined. "Undefined" otherwise.

//  Program Notes:-
//  =============

//  None.

  return fMetaData->ColName(fCurCol);
}

//.....................................................................

UInt_t ND::TDbiRowStream::NumCols() const {
//
//
//  Purpose: Return the number of columns.
//
//  Arguments: None.
//
//  Return: the number of columns
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Return the number of columns.

//  Program Notes:-
//  =============

//  None.

  return fMetaData->NumCols();

}

//.....................................................................

Bool_t ND::TDbiRowStream::HasEpoch() const {
  return fMetaData->HasEpoch();
}

//.....................................................................

string ND::TDbiRowStream::TableName() const {
//
//
//  Purpose: Return table name in upper case.
//

  return ND::UtilString::ToUpper(fMetaData->TableName());
}

//.....................................................................

string ND::TDbiRowStream::TableNameTc() const {
//
//
//  Purpose: Return table name in true case.

  return fMetaData->TableName();
}
/*    Template for New Member Function

//.....................................................................

ND::TDbiRowStream:: {
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





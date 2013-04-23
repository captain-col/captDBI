// $Id: TDbiExceptionLog.cxx,v 1.1 2011/01/18 05:49:19 finch Exp $


//////////////////////////////////////////////////////////////////////////
////////////////////////////     ROOT API     ////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "TDbiExceptionLog.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;

ClassImp(CP::TDbiExceptionLog)

//   Definition of static data members
//   *********************************


CP::TDbiExceptionLog CP::TDbiExceptionLog::fgGELog;

//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

CP::TDbiExceptionLog::TDbiExceptionLog(const CP::TDbiException* e) {

  DbiTrace( "Creating CP::TDbiExceptionLog" << "  ");
  if ( e ) this->AddEntry(*e);

}
//.....................................................................


CP::TDbiExceptionLog::~TDbiExceptionLog() {


  DbiTrace( "Destroying CP::TDbiExceptionLog" << "  ");

}

//.....................................................................

std::ostream& operator<<(std::ostream& os, const CP::TDbiExceptionLog& el) {

  if ( el.IsEmpty() ) {
    os << "The database exception log is empty" << endl;
  }
  else {
    os << "Database exception log:-" << endl;
    std::vector<CP::TDbiException>::const_iterator itr(el.GetEntries().begin()),
                                               itrEnd(el.GetEntries().end());
    while ( itr != itrEnd ) {
      os << *itr << endl;
      ++itr;
    }
  }

  return os;

}

//.....................................................................
///
///
/// Purpose:  Add all entries from el.
void CP::TDbiExceptionLog::AddLog(const CP::TDbiExceptionLog& el) {

  const std::vector<CP::TDbiException>& ve = el.GetEntries();
  std::vector<CP::TDbiException>::const_iterator itr(ve.begin()), itrEnd(ve.end());
  while ( itr != itrEnd ) this->AddEntry(*itr++);

}

//.....................................................................
///  Purpose: Copy exception log starting at 'start' (default 0)
void CP::TDbiExceptionLog::Copy(CP::TDbiExceptionLog& that, UInt_t start) const {
//
//


  UInt_t maxEntry = fEntries.size();
  while (start <= maxEntry ) that.AddEntry(fEntries[start++]);

}
//.....................................................................
/// Purpose:  Print contents to cout.
void CP::TDbiExceptionLog::Print () const {
//
//


  std::cout << *this << std::endl;

}





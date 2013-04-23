///////////////////////////////////////////////////////////////////////
// $Id: TDbiTimerManager.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $
//
// CP::TDbiTimerManager
//
// Package: CP::TDbi (Database Interface).
//
// N. West 01/2002
//
// Concept: Manager of a set of simple timers.
//
// Purpose: To find out why this is all soooo sssllloooowwww!
//
///////////////////////////////////////////////////////////////////////

#include "TDbiTimer.hxx"
#include "TDbiTimerManager.hxx"
#include <MsgFormat.h>
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;

ClassImp(CP::TDbiTimerManager)


//   Definition of static data members
//   *********************************


CP::TDbiTimerManager CP::TDbiTimerManager::gTimerManager;

// Definition of member functions (alphabetical order)
// ***************************************************

//.....................................................................

CP::TDbiTimerManager::TDbiTimerManager() :
fEnabled(kTRUE)
{
//
//
//  Purpose:  Default constructor
//
//  Contact:   N. West
//


  DbiTrace( "Creating CP::TDbiTimerManager" << "  ");

}

//.....................................................................

CP::TDbiTimerManager::~TDbiTimerManager() {
//
//
//  Purpose: Destructor
//
//  Contact:   N. West
//


  DbiTrace( "Destroying CP::TDbiTimerManager" << "  ");
  while ( this->GetCurrent() ) this->Pop();

}

//.....................................................................

CP::TDbiTimer* CP::TDbiTimerManager::GetCurrent() {
//
//
//  Purpose:  Get the current timer if any.
//
//  Return:   Curent timer or null if none.
//
//  Contact:   N. West

  return fTimers.empty() ? 0 : *(fTimers.begin());
}
//.....................................................................

CP::TDbiTimer* CP::TDbiTimerManager::Pop() {
//
//
//  Purpose:   Remove the most recent timer, and resume the previous.
//
//  Return:    Previous timer (if any).
//

  if ( fTimers.empty() ) return 0;

  CP::TDbiTimer* timer = this->GetCurrent();
  delete timer;
  timer = 0;
  fTimers.pop_front();
  timer = this->GetCurrent();
  if ( timer ) timer->Resume();
  return timer;

}

//.....................................................................

CP::TDbiTimer* CP::TDbiTimerManager::Push() {
//
//
//  Purpose:   Suspend current time and add new timer to stack.
//
//  Return:    New timer.
//

  CP::TDbiTimer* timer = this->GetCurrent();
  if ( timer ) timer->Suspend();
  fTimers.push_front(new CP::TDbiTimer);
  return this->GetCurrent();

}

//.....................................................................

void CP::TDbiTimerManager::RecBegin(string tableName, UInt_t rowSize) {
//
//
//  Purpose:  Record the start of initial query on supplied table.
//
//  Arguments:
//    tableName    in    Name of table.
//    rowSize      in    Size of row object
//
//  Contact:   N. West

//  Suspend current timer, if any, and start a new one.

  if ( ! fEnabled ) return;
  CP::TDbiTimer* timer = this->Push();
  timer->RecBegin(tableName, rowSize);
}
//.....................................................................

void CP::TDbiTimerManager::RecEnd(UInt_t numRows) {
//
//
//  Purpose:  Record the end of query.
//
//  Arguments:
//    numRows    in    Number of rows found in query
//
//  Contact:   N. West

  if ( ! fEnabled ) return;

//  Terminate the current timer and resume the previous one.

  CP::TDbiTimer* timer = this->GetCurrent();
  if ( timer ) timer->RecEnd(numRows);
  timer = this->Pop();

}

//.....................................................................

void CP::TDbiTimerManager::RecFillAgg(Int_t /* aggNo */) {
//
//
//  Purpose:  Record filling of aggregate.
//
//  Arguments:
//    aggNo        in    Aggregate number.
//
//  Contact:   N. West

  if ( ! fEnabled ) return;

//  Currently a no-op.

}

//.....................................................................

void CP::TDbiTimerManager::RecMainQuery() {
//
//
//  Purpose:  Record the start of main query.
//
//  Contact:   N. West

  if ( ! fEnabled ) return;
  CP::TDbiTimer* timer = this->GetCurrent();
  if ( timer ) timer->RecMainQuery();

}

//.....................................................................

void CP::TDbiTimerManager::StartSubWatch(UInt_t subWatch) {
//
//
//  Purpose:  Start specified SubWatch if SubWatch timers enabled.
//
//  Arguments:
//    subWatch     in    SubWatch number ( 0 .. kMaxSubWatch-1 ).
//
//  Contact:   N. West

  if ( ! fEnabled ) return;
  CP::TDbiTimer* timer = this->GetCurrent();
  if ( timer ) timer->StartSubWatch(subWatch);

}

/*    Template for New Member Function

//.....................................................................

CP::TDbiTimerManager:: {
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




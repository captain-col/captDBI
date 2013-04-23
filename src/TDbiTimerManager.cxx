///////////////////////////////////////////////////////////////////////
// $Id: TDbiTimerManager.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $
//
// ND::TDbiTimerManager
//
// Package: ND::TDbi (Database Interface).
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

ClassImp(ND::TDbiTimerManager)


//   Definition of static data members
//   *********************************


ND::TDbiTimerManager ND::TDbiTimerManager::gTimerManager;

// Definition of member functions (alphabetical order)
// ***************************************************

//.....................................................................

ND::TDbiTimerManager::TDbiTimerManager() :
fEnabled(kTRUE)
{
//
//
//  Purpose:  Default constructor
//
//  Contact:   N. West
//


  DbiTrace( "Creating ND::TDbiTimerManager" << "  ");

}

//.....................................................................

ND::TDbiTimerManager::~TDbiTimerManager() {
//
//
//  Purpose: Destructor
//
//  Contact:   N. West
//


  DbiTrace( "Destroying ND::TDbiTimerManager" << "  ");
  while ( this->GetCurrent() ) this->Pop();

}

//.....................................................................

ND::TDbiTimer* ND::TDbiTimerManager::GetCurrent() {
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

ND::TDbiTimer* ND::TDbiTimerManager::Pop() {
//
//
//  Purpose:   Remove the most recent timer, and resume the previous.
//
//  Return:    Previous timer (if any).
//

  if ( fTimers.empty() ) return 0;

  ND::TDbiTimer* timer = this->GetCurrent();
  delete timer;
  timer = 0;
  fTimers.pop_front();
  timer = this->GetCurrent();
  if ( timer ) timer->Resume();
  return timer;

}

//.....................................................................

ND::TDbiTimer* ND::TDbiTimerManager::Push() {
//
//
//  Purpose:   Suspend current time and add new timer to stack.
//
//  Return:    New timer.
//

  ND::TDbiTimer* timer = this->GetCurrent();
  if ( timer ) timer->Suspend();
  fTimers.push_front(new ND::TDbiTimer);
  return this->GetCurrent();

}

//.....................................................................

void ND::TDbiTimerManager::RecBegin(string tableName, UInt_t rowSize) {
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
  ND::TDbiTimer* timer = this->Push();
  timer->RecBegin(tableName, rowSize);
}
//.....................................................................

void ND::TDbiTimerManager::RecEnd(UInt_t numRows) {
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

  ND::TDbiTimer* timer = this->GetCurrent();
  if ( timer ) timer->RecEnd(numRows);
  timer = this->Pop();

}

//.....................................................................

void ND::TDbiTimerManager::RecFillAgg(Int_t /* aggNo */) {
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

void ND::TDbiTimerManager::RecMainQuery() {
//
//
//  Purpose:  Record the start of main query.
//
//  Contact:   N. West

  if ( ! fEnabled ) return;
  ND::TDbiTimer* timer = this->GetCurrent();
  if ( timer ) timer->RecMainQuery();

}

//.....................................................................

void ND::TDbiTimerManager::StartSubWatch(UInt_t subWatch) {
//
//
//  Purpose:  Start specified SubWatch if SubWatch timers enabled.
//
//  Arguments:
//    subWatch     in    SubWatch number ( 0 .. kMaxSubWatch-1 ).
//
//  Contact:   N. West

  if ( ! fEnabled ) return;
  ND::TDbiTimer* timer = this->GetCurrent();
  if ( timer ) timer->StartSubWatch(subWatch);

}

/*    Template for New Member Function

//.....................................................................

ND::TDbiTimerManager:: {
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




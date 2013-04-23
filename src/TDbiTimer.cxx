///////////////////////////////////////////////////////////////////////
// $Id: TDbiTimer.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $
//
// CP::TDbiTimer
//
// Package: CP::TDbi (Database Interface).
//
// N. West 01/2002
//
// Concept: Simple timer record query progress and final times.
//
// Purpose: To find out why this is all soooo sssllloooowwww!
//
///////////////////////////////////////////////////////////////////////

#include "TDbiTimer.hxx"
#include <MsgFormat.h>
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;

ClassImp(CP::TDbiTimer)


//   Definition of static data members
//   *********************************


// Definition of member functions (alphabetical order)
// ***************************************************

//.....................................................................

CP::TDbiTimer::TDbiTimer() :
fCurSubWatch(0),
fRowSize(0),
fQueryStage(kPassive)
{
//
//
//  Purpose:  Default constructor
//
//  Contact:   N. West
//


  DbiTrace( "Creating CP::TDbiTimer" << "  ");

  fWatch.Stop();
  for ( int subWatch = 0; subWatch <  kMaxSubWatch; ++subWatch) {
    fSubWatches[subWatch].Stop();
  }

}


//.....................................................................

CP::TDbiTimer::~TDbiTimer() {
//
//
//  Purpose: Destructor
//
//  Contact:   N. West
//


  DbiTrace( "Destroying CP::TDbiTimer" << "  ");

}

//.....................................................................

void CP::TDbiTimer::RecBegin(string tableName, UInt_t rowSize) {
//
//
//  Purpose:  Record the start of initial query on supplied table.
//
//  Arguments:
//    tableName    in    Name of table.
//    rowSize      in    Size of row object
//
//  Contact:   N. West

  fQueryStage = kInitialQuery;
  fTableName = tableName;
  fRowSize = rowSize;
  fWatch.Start();
  for ( int subWatch = 0; subWatch <  kMaxSubWatch; ++subWatch) {
    // Use Start to reset the counter (Reset doesn't do this).
    fSubWatches[subWatch].Start();
    fSubWatches[subWatch].Stop();
  }
  if ( fCurSubWatch >= 0 ) this->StartSubWatch(0);

}
//.....................................................................

void CP::TDbiTimer::RecEnd(UInt_t numRows) {
//
//
//  Purpose:  Record the end of query.
//
//  Arguments:
//    numRows    in    Number of rows found in query
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Record the end of query.  If query never reached the Main Query
//    phase then quit as query must have been satisfied
//    by the cache.
//
// o  If SubWatches enabled print them out if the main timer has
//    recorded significant activity.

//  None.

  if ( fQueryStage != kMainQuery ) return;

  Float_t tableSize = numRows * fRowSize/1.0e+3;
  string units = "Kb";
  if ( tableSize > 1000. ) {
    tableSize /= 1000.;
    units = "Mb";
  }
  MsgFormat ffmt("%6.1f");

  DbiInfo( "CP::TDbiTimer:" <<  fTableName
			<< ": Query done. "  << numRows
  			<< "rows, " << ffmt(tableSize) << units
  			<< " Cpu" << ffmt(fWatch.CpuTime())
  			<< " , elapse" << ffmt(fWatch.RealTime())
                        << "  ");

  fWatch.Stop();
  fQueryStage = kPassive;

  if ( fCurSubWatch >= 0 && fWatch.RealTime() > 5. ) {
    for ( int subWatch = 0; subWatch <  kMaxSubWatch; ++subWatch) {
    static const Char_t* subWatchNames[kMaxSubWatch]
      = { "Query database     ",
          "Create row objects ",
          "Retrieve TSQL rows ",
          "Fill row objects   "};
       DbiInfo(  "      SubWatch " <<  subWatchNames[subWatch]
            << ": Cpu" << ffmt(fSubWatches[subWatch].CpuTime())
  	    << " , elapse" << ffmt(fSubWatches[subWatch].RealTime())
	    << " , Starts " << fSubWatches[subWatch].Counter()
            << "  ");
    }
  }
}
//.....................................................................

void CP::TDbiTimer::RecMainQuery() {
//
//
//  Purpose:  Record the start of main query.
//
//  Contact:   N. West

  fQueryStage = kMainQuery;

}
//.....................................................................

void CP::TDbiTimer::RecFillAgg(Int_t /* aggNo */) {
//
//
//  Purpose:  Record filling of aggregate.
//
//  Arguments:
//    aggNo        in    Aggregate number.
//
//  Contact:   N. West

}

void CP::TDbiTimer::Resume() {
//
//
//  Purpose:  Resume timer and any partial timer.
//
//  Contact:   N. West

  if ( fCurSubWatch >= 0 ) fSubWatches[fCurSubWatch].Start(kFALSE);
  fWatch.Start(kFALSE);
}

//.....................................................................

void CP::TDbiTimer::StartSubWatch(UInt_t subWatch) {
//
//
//  Purpose:  Start specified SubWatch if SubWatch timers enabled.
//
//  Arguments:
//    subWatch     in    SubWatch number ( 0 .. kMaxSubWatch-1 ).
//
//  Contact:   N. West

  if (     fCurSubWatch < 0
        || subWatch >= kMaxSubWatch ) return;

  fSubWatches[fCurSubWatch].Stop();
  fCurSubWatch = subWatch;
  fSubWatches[fCurSubWatch].Start(kFALSE);

}

void CP::TDbiTimer::Suspend() {
//
//
//  Purpose:  Suspend timer and any partial timer.
//
//  Contact:   N. West

  if ( fCurSubWatch >= 0 ) fSubWatches[fCurSubWatch].Stop();
  fWatch.Stop();
}

/*    Template for New Member Function

//.....................................................................

CP::TDbiTimer:: {
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




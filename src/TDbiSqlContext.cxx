// $Id: TDbiSqlContext.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $

#include "TDbiSqlContext.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;

ClassImp(ND::TDbiSqlContext)


//   Definition of static data members
//   *********************************


// Definition of member functions (alphabetical order)
// ***************************************************

//.....................................................................

ND::TDbiSqlContext::TDbiSqlContext(const std::string& ctxt) :
ND::TDbiString(ctxt),
fIntervalType(kUndefined),
fDetType(ND::DbiDetector::kUnknown),
fSimType(ND::DbiSimFlag::kUnknown)
{
//
//  Purpose:  Default constructor
//
//  Contact:   N. West
//


  DbiTrace( "Creating ND::TDbiSqlContext" << "  ");

}
//.....................................................................

ND::TDbiSqlContext::TDbiSqlContext(IntervalType interval,
                             ND::TVldTimeStamp start,
                             ND::TVldTimeStamp end,
		   	     ND::DbiDetector::Detector_t det,
                             ND::DbiSimFlag::SimFlag_t sim) :
fIntervalType(interval),
fTimeStart(start),
fTimeEnd(end),
fDetType(det),
fSimType(sim)
{
//
//  Purpose:  Main constructor
//
//  Contact:   N. West
//


  DbiTrace( "Creating ND::TDbiSqlContext" << "  ");

  switch (interval) {

  case kBefore:
    (*this) <<     "TIMEEND <= \'"   << start.AsString("s") << '\'';
    break;

  case kAfter:
    (*this) <<     "TIMESTART >= \'" << end.AsString("s")   << '\'';
    break;

  case kMisses:
    (*this) <<     "TIMESTART >= \'" << end.AsString("s")   << '\''
            << " or TIMEEND <= \'"   << start.AsString("s") << '\'';
    break;

  case kThroughout:
    (*this) <<     "TIMESTART <= \'" << start.AsString("s") << '\''
	    << " and TIMEEND >= \'"  << end.AsString("s")   << '\'';
    break;

  case kWithin:
    (*this) <<     "TIMESTART >= \'" << start.AsString("s") << '\''
	    << " and TIMEEND <= \'"  << end.AsString("s")   << '\'';
    break;

  case kOverlaps:
    (*this) <<     "TIMESTART < \'"  << end.AsString("s")   << '\''
	    << " and TIMEEND > \'"   << start.AsString("s") << '\'';
    break;

  case kStarts:
    (*this) <<     "TIMESTART >= \'" << start.AsString("s") << '\''
	    << " and TIMESTART < \'" << end.AsString("s")   << '\'';
    break;

  case kEnds:
    (*this) <<     "TIMEEND > \'"    << start.AsString("s") << '\''
	    << " and TIMEEND <=  \'" << end.AsString("s")   << '\'';
    break;

  default:
    (*this) << " 1 = 0 "; //An impossible condition (hopefully)
  }

  (*this) << " and DetectorMask & " << static_cast<unsigned int>(fDetType)
	  << " and SimMask & "      << static_cast<unsigned int>(fSimType);
}


//.....................................................................

ND::TDbiSqlContext::~TDbiSqlContext() {
//
//
//  Purpose: Destructor
//
//  Contact:   N. West
//


  DbiTrace( "Destroying ND::TDbiSqlContext" << "  ");

}


/*    Template for New Member Function

//.....................................................................

ND::TDbiSqlContext:: {
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



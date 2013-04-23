// $Id: TDbiSqlContext.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $

#include "TDbiSqlContext.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>

ClassImp(CP::TDbiSqlContext)


//   Definition of static data members
//   *********************************


// Definition of member functions (alphabetical order)
// ***************************************************

//.....................................................................

CP::TDbiSqlContext::TDbiSqlContext(const std::string& ctxt) :
CP::TDbiString(ctxt),
fIntervalType(kUndefined),
fDetType(CP::DbiDetector::kUnknown),
fSimType(CP::DbiSimFlag::kUnknown)
{
//
//  Purpose:  Default constructor
//
//  Contact:   N. West
//


  DbiTrace( "Creating CP::TDbiSqlContext" << "  ");

}
//.....................................................................

CP::TDbiSqlContext::TDbiSqlContext(IntervalType interval,
                             CP::TVldTimeStamp start,
                             CP::TVldTimeStamp end,
		   	     CP::DbiDetector::Detector_t det,
                             CP::DbiSimFlag::SimFlag_t sim) :
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


  DbiTrace( "Creating CP::TDbiSqlContext" << "  ");

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

CP::TDbiSqlContext::~TDbiSqlContext() {
//
//
//  Purpose: Destructor
//
//  Contact:   N. West
//


  DbiTrace( "Destroying CP::TDbiSqlContext" << "  ");

}


/*    Template for New Member Function

//.....................................................................

CP::TDbiSqlContext:: {
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



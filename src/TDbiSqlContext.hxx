// $Id: TDbiSqlContext.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $

#ifndef DBISQLCONTEXT
#define DBISQLCONTEXT

////////////////////////////////////////////////////////////////////////
///                                             
/// \class  ND::TDbiSqlContext                                                                 
/// \brief <b> Concept:</b> SQL corresponding to an Extended Context Query.           
///                                                                   
/// <b> Purpose: </b> To hide the SQL needed to query the VLD tables.           
///                                                                    
////////////////////////////////////////////////////////////////////////

#include "string"

#include "TDbiString.hxx"
#include "TVldContext.hxx"


namespace ND {
class TDbiSqlContext : public TDbiString
{

public:


// Types and enum

  enum  IntervalType { kBefore,      kAfter,    kMisses,
                       kThroughout,  kWithin,   kOverlaps,
                       kStarts,      kEnds,
                       kUndefined };

// Constructors and destructors.
           TDbiSqlContext(const std::string& ctxt = "");
           TDbiSqlContext(IntervalType interval,
                         ND::TVldTimeStamp start,
                         ND::TVldTimeStamp end,
			 ND::DbiDetector::Detector_t det,
                         ND::DbiSimFlag::SimFlag_t sim);

  virtual ~TDbiSqlContext();

// State testing member functions

  ND::TVldTimeStamp             GetTimeStart() const { return fTimeStart; }
  ND::TVldTimeStamp             GetTimeEnd()   const { return fTimeEnd; }
  ND::DbiDetector::Detector_t     GetDetector()  const { return fDetType; }
  ND::DbiSimFlag::SimFlag_t       GetSimFlag()   const { return fSimType; }


// State changing member functions

private:

// Data members

 IntervalType             fIntervalType;
 ND::TVldTimeStamp             fTimeStart;
 ND::TVldTimeStamp             fTimeEnd;
 ND::DbiDetector::Detector_t     fDetType;
 ND::DbiSimFlag::SimFlag_t       fSimType;


 ClassDef(TDbiSqlContext,0)     // SQL for Extended Context Query

};
};


#endif // DBISQLCONTEXT



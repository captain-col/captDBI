// $Id: TDbiSqlContext.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $

#ifndef DBISQLCONTEXT
#define DBISQLCONTEXT

////////////////////////////////////////////////////////////////////////
///                                             
/// \class  CP::TDbiSqlContext                                                                 
/// \brief <b> Concept:</b> SQL corresponding to an Extended Context Query.           
///                                                                   
/// <b> Purpose: </b> To hide the SQL needed to query the VLD tables.           
///                                                                    
////////////////////////////////////////////////////////////////////////

#include "string"

#include "TDbiString.hxx"
#include "TVldContext.hxx"


namespace CP {
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
                         CP::TVldTimeStamp start,
                         CP::TVldTimeStamp end,
			 CP::DbiDetector::Detector_t det,
                         CP::DbiSimFlag::SimFlag_t sim);

  virtual ~TDbiSqlContext();

// State testing member functions

  CP::TVldTimeStamp             GetTimeStart() const { return fTimeStart; }
  CP::TVldTimeStamp             GetTimeEnd()   const { return fTimeEnd; }
  CP::DbiDetector::Detector_t     GetDetector()  const { return fDetType; }
  CP::DbiSimFlag::SimFlag_t       GetSimFlag()   const { return fSimType; }


// State changing member functions

private:

// Data members

 IntervalType             fIntervalType;
 CP::TVldTimeStamp             fTimeStart;
 CP::TVldTimeStamp             fTimeEnd;
 CP::DbiDetector::Detector_t     fDetType;
 CP::DbiSimFlag::SimFlag_t       fSimType;


 ClassDef(TDbiSqlContext,0)     // SQL for Extended Context Query

};
};


#endif // DBISQLCONTEXT



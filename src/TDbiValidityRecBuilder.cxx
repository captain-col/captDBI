// $Id: TDbiValidityRecBuilder.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $

#include "DbiDetector.hxx"
#include "DbiSimFlag.hxx"
#include "TDbiDBProxy.hxx"
#include "TDbiResultSetNonAgg.hxx"
#include "TDbiInRowStream.hxx"
#include "TDbiSimFlagAssociation.hxx"
#include "TDbiValidityRec.hxx"
#include "TDbiValidityRecBuilder.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;
#include "TVldContext.hxx"

ClassImp(ND::TDbiValidityRecBuilder)

//   Definition of static data members
//   *********************************


//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

ND::TDbiValidityRecBuilder::TDbiValidityRecBuilder(const ND::TDbiDBProxy& proxy,
                                             const ND::TVldContext& vc,
                                             const TDbi::Task& task,
                                             Int_t selectDbNo     /* Default: -1 */,
                                             Bool_t findFullTimeWindow /* Default: true*/
                                             ):
fIsExtendedContext(kFALSE),
fTask(task)
{
//
//  Purpose:  Constructor for specific context queries.
//
//  Arguments:
//    proxy        in    Database proxy for this table.
//    vc           in    The Validity Context for the query.
//    task         in    The task of the query.
//    selectDbNo   in    If >=0 only look in selected DB.
//    findFullTimeWindow
//                 in    Attempt to find full validity of query
//                        i.e beyond TDbi::GetTimeGate
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create ValidityRecBuilder by applying a validity query to
//    the database table and building a set of ND::TDbiValidityRec
//    objects from the resulting ND::TDbiInRowStream.
//
// o  Try each member of the cascade in turn until a query succeeds or the
//    cascade is exhausted.
//
// o  For each member of the cascade try each associated SimFlag in turn
//    until a query succeeds or the list of associated SimFlags is exhausted.


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Creating ND::TDbiValidityRecBuilder" << "  ");

  ND::DbiDetector::Detector_t     det(vc.GetDetector());
  ND::DbiSimFlag::SimFlag_t       sim(vc.GetSimFlag());
  ND::TVldTimeStamp             ts(vc.GetTimeStamp());
  ND::DbiSimFlag::SimFlag_t       simTry(sim);
  const string& tableName = proxy.GetTableName();
  Int_t sumTimeWindows = 0;
  Int_t numTimeWindows = 0;

//  Contruct a default (gap) validity record fGap.
//  If findFullTimeWindow then gap will be unlimited but will be trimmed
//  after each query.

  this->MakeGapRec(vc, tableName, findFullTimeWindow);

//  Force aggregate -1 into first slot.
  this->AddNewGap(-1);

  const ND::TVldTimeStamp curVTS = vc.GetTimeStamp();

// Check to see if table exists.

  unsigned int numVRecIn = 0;

  if ( ! proxy.TableExists() ) {
       DbiWarn(  "ND::TDbiValidityRecBuilder::Query for table:"
         << proxy.GetTableName()
         << ", table does not exist!" << "  ");
  }

  else {

//  Loop over all databases in cascade until valid data found.

//  Determine the resolution scheme for resolving VLD overlaps.
    Bool_t resolveByCreationDate = ! proxy.HasEpoch();

    UInt_t numDb     = proxy.GetNumDb();
    Bool_t foundData = kFALSE;

    for ( UInt_t dbNo = 0; dbNo < numDb && ! foundData; ++dbNo ) {

      // Skip if cascade entry does not have this table or selection in force and not the required one.
      if ( ! proxy.TableExists(dbNo) ) continue;
      if ( selectDbNo >= 0 && selectDbNo != (int) dbNo ) continue;

//    Loop over all associated SimFlags.

      ND::TDbiSimFlagAssociation::SimList_t simList
                    = ND::TDbiSimFlagAssociation::Instance().Get(sim);

      ND::TDbiSimFlagAssociation::SimList_t::iterator listItr    = simList.begin();
      ND::TDbiSimFlagAssociation::SimList_t::iterator listItrEnd = simList.end();
      while ( listItr !=  listItrEnd && ! foundData ) {

        simTry = *listItr;
        ++listItr;
        ND::TVldContext vcTry(det,simTry,ts);

//      Apply validity query and build result set.

        ND::TDbiInRowStream* rs = proxy.QueryValidity(vcTry,fTask,dbNo);

//      Build a result from the result set and drop result set.

        ND::TDbiValidityRec tr;
        ND::TDbiResultSetNonAgg result(rs,&tr,0,kFALSE);
        delete rs;

//      Loop over all entries in ND::TDbiResultSet and, for each Aggregate,
//      find effective validity range of best, or of gap if none.

//      Initialise lowest priority VLD to a gap. It will be used by FindTimeBoundaries.
	const ND::TDbiValidityRec* lowestPriorityVrec = &fGap;

        UInt_t numRows = result.GetNumRows();
        for (UInt_t row = 0; row < numRows; ++row) {
          const ND::TDbiValidityRec* vr = dynamic_cast<const ND::TDbiValidityRec*>(
                                            result.GetTableRow(row));

          Int_t aggNo = vr->GetAggregateNo();

//        If starting a new aggregate prime it as a gap.
          Int_t index = this->IndexOfAggno(aggNo);
          if ( index < 0 ) index = this->AddNewGap(aggNo);

//        Trim the validity record for the current aggregate
//        number by this record and see if we have found valid
//        data yet.

          ND::TDbiValidityRec& curRec = fVRecs[index];
          curRec.Trim(curVTS, *vr);
          if ( ! curRec.IsGap() ) {
            foundData = kTRUE;

//          Fill in entry's database number - its not stored in the
//          database table!
            curRec.SetDbNo(dbNo);
	  }

//        Find the lowest priority non-gap VLD  that is used
	  if (    lowestPriorityVrec->IsGap()
               || lowestPriorityVrec->IsHigherPriority(curRec,resolveByCreationDate)
		  )lowestPriorityVrec = &curRec;

//        Count the number used and sum the time windows
	  ++numVRecIn;
          const ND::TVldRange range = vr->GetVldRange();
          Int_t timeInterval =   range.GetTimeEnd().GetSec()
                               - range.GetTimeStart().GetSec();
          if ( timeInterval < 5 ) {
               DbiWarn(  "Detected suspiciously small validity time interval in \n"
	    << "table " << tableName << " validity rec " << *vr << "  ");
	  }
          sumTimeWindows += timeInterval;
          ++numTimeWindows;
        }

//      If finding findFullTimeWindow then find bounding limits
//      for the cascade and sim flag and trim all validity records
//      and the default (gap) validity record.
        if ( findFullTimeWindow ) {
          ND::TVldTimeStamp start, end;
          proxy.FindTimeBoundaries(vcTry,fTask,dbNo,*lowestPriorityVrec,resolveByCreationDate,start,end);
     DbiDebug("Trimming validity records to "
    << start << " .. " << end << "  ");
          std::vector<ND::TDbiValidityRec>::iterator itr(fVRecs.begin()), itrEnd(fVRecs.end());
          for( ; itr != itrEnd; ++itr ) itr->AndTimeWindow(start,end);
	  fGap.AndTimeWindow(start,end);
        }
      }

    }
  }

//  If the query found no records in any database then
//  the tables will still have something - the aggno = -1 gap

// If the associated SimFlag is different to the original request make
// sure that all the ND::TDbiValidityRec are valid for the request.
  if ( sim != simTry ) {
    DbiLog( "Imposing SimFlag of " << sim << " on ND::TDbiValidityRecs which matched " << simTry << "  ");
    for ( unsigned int irec = 0; irec < GetNumValidityRec(); ++irec ) {
      ND::TDbiValidityRec& vrec = const_cast<ND::TDbiValidityRec&>(GetValidityRec(irec));
      const ND::TVldRange& vr(vrec.GetVldRange());
      ND::TVldRange vr_mod(vr.GetDetectorMask(),sim,vr.GetTimeStart(),vr.GetTimeEnd(),vr.GetDataSource());
      vrec.SetVldRange(vr_mod);
    }
  }

  // hand edit applied 
 if( TDbiLog::GetLogLevel() >= TDbiLog::VerboseLevel)
 {
    std::ostream& msg=TDbiLog::GetLogStream();

  msg << "ND::TDbiValidityRecBuilder:" << endl
      << " Query: " << vc.AsString() << endl
      << " using associated SimFlag: " << ND::DbiSimFlag::AsString(simTry)
      << " for " << ND::DbiSimFlag::AsString(sim)
      << " found " << numVRecIn
      << " vrecs in database, for " << fVRecs.size()
      << " aggregates:-." << endl;

  for ( unsigned int irec = 0; irec < GetNumValidityRec(); ++irec ) {
    const ND::TDbiValidityRec& vrec = GetValidityRec(irec);
    if ( vrec.GetAggregateNo() != -2
	 )   msg << " " << irec << " " << GetValidityRec(irec) << endl;
  }

}

// Adjust the time gate if grossly wrong.
  if ( numTimeWindows > 0 ) {
    Int_t timeGateCalc = 3 * fVRecs.size() * sumTimeWindows/numTimeWindows;
    // Limit to 100 days and allow for overflow.
    if ( timeGateCalc > 100*24*60*60 || timeGateCalc < 0
       ) timeGateCalc = 100*24*60*60;
    Int_t timeGateCurr = TDbi::GetTimeGate(tableName);
    if (    timeGateCurr < timeGateCalc/10
	 || timeGateCurr > timeGateCalc*10
	    ) {
      TDbi::SetTimeGate(tableName,timeGateCalc);
      if ( timeGateCalc != TDbi::GetTimeGate(tableName) ) {
	   DbiWarn(  "The ignored time gate setting was calculated with the following data:-"
	  << "\n   Context: " << vc << " task " << task  << " findFullTimeWindow " << findFullTimeWindow
	  << "\n   Number of vrecs " << numTimeWindows
	  << " total time (secs) of all vrecs " << sumTimeWindows
	  << " Number of aggregates " << fVRecs.size() << "  ");
      }
    }
  }

  return;

}

//.....................................................................

ND::TDbiValidityRecBuilder::TDbiValidityRecBuilder(const ND::TDbiDBProxy& proxy,
                                             const string& context,
                                             const TDbi::Task& task):
fIsExtendedContext(kTRUE),
fTask(task)
{
//
//  Purpose:  Constructor for extended context queries.
//
//  Arguments:
//    proxy        in    Database proxy for this table.
//    context      in    The SQL corresponding to the extended context for the query.
//    task         in    The task of the query.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create ValidityRecBuilder by applying a validity query to
//    the database table and building a set of ND::TDbiValidityRec
//    objects from the resulting ND::TDbiInRowStream.


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Creating ND::TDbiValidityRecBuilder"
			    << " for extended context " << context << "  ");

// Prime fVRecs with a gap that will not be used but makes the result
// look like an aggregated result.

  DbiVerbose( "Initialising with gap record " << fGap << "  ");
  this->AddNewGap(-1);

// Check to see if table exists.

  unsigned int numVRecIn = 0;

  if ( ! proxy.TableExists() ) {
    DbiWarn(  "ND::TDbiValidityRecBuilder::Query for table:"
         << proxy.GetTableName()
         << ", table does not exist!" << "  ");
  }

  else if ( context == "" ) {
    DbiWarn(  "ND::TDbiValidityRecBuilder::Null query for table:"
         << proxy.GetTableName() << "  ");
  }

  else {

//  Loop over all databases in cascade until valid data found.

    UInt_t numDb     = proxy.GetNumDb();
    Bool_t foundData = kFALSE;

    for ( UInt_t dbNo = 0; dbNo < numDb && ! foundData; ++dbNo ) {

//    Apply validity query and build result set.

      ND::TDbiInRowStream* rs = proxy.QueryValidity(context,fTask,dbNo);

//    Build a result from the result set and drop result set.

      ND::TDbiValidityRec tr;
      ND::TDbiResultSetNonAgg result(rs,&tr,0,kFALSE);
      delete rs;

//    Loop over all entries in ND::TDbiResultSet and add them all to set.

      UInt_t numRows = result.GetNumRows();
      for (UInt_t row = 0; row < numRows; ++row) {
        const ND::TDbiValidityRec* vr = dynamic_cast<const ND::TDbiValidityRec*>(
                                          result.GetTableRow(row));

//      Cannot use AddNewAgg - aggregate numbers may be duplicated.
        Int_t index = fVRecs.size();
        fVRecs.push_back(ND::TDbiValidityRec(*vr));
        fAggNoToIndex[vr->GetAggregateNo()] = index;
        foundData = kTRUE;
	
//      Fill in entry's database number - its not stored in the
//      database table!
        fVRecs[index].SetDbNo(dbNo);

//      Count the number used.
        ++numVRecIn;
      }
    }
  }

  // hand edit required
  // if( TDbiLog::GetLogLevel() >= TDbiLog::kLogLevel
    std::ostream& msg=TDbiLog::GetLogStream();

  msg << "ND::TDbiValidityRecBuilder:" << endl
      << " Extended context query: " << context << endl
      << " found " << numVRecIn
      << " vrecs in database, for " << fVRecs.size()
      << " records:-." << endl;

  for ( unsigned int irec = 0; irec < GetNumValidityRec(); ++irec ) {
    msg << " " << irec << " " << GetValidityRec(irec) << endl;
  }
  return;



}

//.....................................................................

ND::TDbiValidityRecBuilder::TDbiValidityRecBuilder(const ND::TDbiValidityRec& vr,
                                             const std::string tableName):
fIsExtendedContext(kFALSE),
fTask(vr.GetTask())
{
//
//  Purpose:  Constructor for an existing query result.
//
//  Arguments:
//    vr           in    The ND::TDbiValidityRec from the query.
//    tableName    in    The name of the table that satisfied the query


  DbiTrace( "Creating ND::TDbiValidityRecBuilder" << "  ");

  const ND::TVldRange&          vrange(vr.GetVldRange());
  // This is the only way I can find to handle Detector and SimFlag!
  ND::TVldContext vc( (ND::DbiDetector::Detector_t) vrange.GetDetectorMask(),
                 (ND::DbiSimFlag::SimFlag_t) vrange.GetSimMask(),
                 vrange.GetTimeStart());
  this->MakeGapRec(vc,tableName);
  this->AddNewAgg(vr,vr.GetAggregateNo());

}
//.....................................................................

ND::TDbiValidityRecBuilder::~TDbiValidityRecBuilder() {
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
//  o  Destroy ValidityRecBuilder.


//  Program Notes:-
//  =============

//  None.


  DbiTrace(  "Destroying ND::TDbiValidityRecBuilder" << "  ");

}

//.....................................................................

std::string ND::TDbiValidityRecBuilder::GetL2CacheName() const {
//
//
//  Purpose:  Return the associated Level 2 Cache Name.
//
//  Specification:-
//  =============
//
//  o For format see static ND::TDbiValidityRec::GetL2CacheName.
//
//  Gaps are excluded (and if all are gaps the returned name is empty).


// Extended Context queries are not L2 cached.
  if ( this->IsExtendedContext() ) return "";

  UInt_t seqLo = 0;
  UInt_t seqHi = 0;
  ND::TVldTimeStamp ts;
  std::vector<ND::TDbiValidityRec>::const_iterator itr = fVRecs.begin();
  std::vector<ND::TDbiValidityRec>::const_iterator end = fVRecs.end();

  for (; itr != end; ++itr) {
    const ND::TDbiValidityRec& vr = *itr;
    if ( ! vr.IsGap() ) {
      if ( seqLo == 0 ) {
	seqLo = vr.GetSeqNo();
	seqHi = vr.GetSeqNo();
	ts    = vr.GetCreationDate();
      }
      else {
	if ( seqLo < vr.GetSeqNo() )        seqLo = vr.GetSeqNo();
	if ( seqHi > vr.GetSeqNo() )        seqHi = vr.GetSeqNo();
	if ( ts    < vr.GetCreationDate() ) ts    = vr.GetCreationDate();
      }
    }
  }

  if ( seqLo == 0 ) return "";

  return ND::TDbiValidityRec::GetL2CacheName(seqLo,seqHi,ts);

}

//.....................................................................

UInt_t ND::TDbiValidityRecBuilder::AddNewAgg(const ND::TDbiValidityRec& vrec,Int_t aggNo) {
//
//
//  Purpose:   Add new aggregate into tables and return its index.
//
//  Program Notes:-
//  =============
//
//  aggNo is passed in and not obtained from vrec, so that it can be the gap.

  //  Make sure it really doesn't exist.
  int index = this->IndexOfAggno(aggNo);
  if ( index >=0 ) return index;

  // It doesn't so add it.
  index = fVRecs.size();
  fVRecs.push_back(vrec);
  fAggNoToIndex[aggNo] = index;
  return index;

}

//.....................................................................

UInt_t ND::TDbiValidityRecBuilder::AddNewGap(Int_t aggNo) {
//
//
//  Purpose:  Add new aggregate gap into tables and return its index

  UInt_t index = this->AddNewAgg(fGap,aggNo);
  fVRecs[index].SetAggregateNo(aggNo);
  return index;

}

//.....................................................................

const ND::TDbiValidityRec&
              ND::TDbiValidityRecBuilder::GetValidityRec(Int_t rowNo) const {
//
//
//  Purpose: Return validity record for supplied row number.

  return (rowNo < 0 || rowNo >= (int) fVRecs.size()) ? fGap : fVRecs[rowNo];
}

//.....................................................................

const ND::TDbiValidityRec&
              ND::TDbiValidityRecBuilder::GetValidityRecFromSeqNo(UInt_t SeqNo) const {
//
//
//  Purpose: Return validity record for supplied SeqNo.

  std::vector<ND::TDbiValidityRec>::const_iterator itr = fVRecs.begin();
  std::vector<ND::TDbiValidityRec>::const_iterator end = fVRecs.end();

  for (; itr != end; ++itr) if ( itr->GetSeqNo() == SeqNo ) return *itr;
  return fGap;
}

//.....................................................................

Int_t ND::TDbiValidityRecBuilder::IndexOfAggno(Int_t aggNo) const {
//
//
//  Purpose:   Get index of AggNo or -1 if missing.

  std::map<Int_t,UInt_t>::const_iterator itr = fAggNoToIndex.find(aggNo);
  if ( itr == fAggNoToIndex.end() ) return -1;
  return itr->second;

}

//.....................................................................

void ND::TDbiValidityRecBuilder::MakeGapRec(const ND::TVldContext& vc,
                                       const string& tableName,
                                       Bool_t findFullTimeWindow) {
//
//
//  Purpose:  Construct Gap Record
//
//  Arguments:
//    vc           in    The Validity Context for the query.
//    tableName    in    The name of the table being queried.
//    findFullTimeWindow
//                 in    Attempt to find full validity of query
//                        i.e. beyond TDbi::GetTimeGate
//
//  Return:        None.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create the data member gap record fGap centered on the current
//    query time.

//  Program Notes:-
//  =============

//  None.

  Int_t timeGate = TDbi::GetTimeGate(tableName);
  time_t contextSec = vc.GetTimeStamp().GetSec() - timeGate;
  ND::TVldTimeStamp startGate(contextSec,0);
  contextSec += 2*timeGate;
  ND::TVldTimeStamp endGate(contextSec,0);
  if ( findFullTimeWindow ) {
    startGate = ND::TVldTimeStamp(0,0);
    endGate   = ND::TVldTimeStamp(0x7FFFFFFF,0);
  }
  ND::TVldRange gapVR(vc.GetDetector(), vc.GetSimFlag(), startGate, endGate, "Gap");
  fGap =  ND::TDbiValidityRec(gapVR, fTask, -2, 0, 0, kTRUE,ND::TVldTimeStamp(0));
}



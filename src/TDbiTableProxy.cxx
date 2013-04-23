// $Id: TDbiTableProxy.cxx,v 1.2 2011/06/09 14:44:29 finch Exp $

#include "DbiDetector.hxx"
#include "DbiSimFlag.hxx"
#include "TDbiBinaryFile.hxx"
#include "TDbiCache.hxx"
#include "TDbiConnectionMaintainer.hxx"
#include "TDbiResultSetAgg.hxx"
#include "TDbiResultSetNonAgg.hxx"
#include "TDbiInRowStream.hxx"
#include "TDbiTableProxy.hxx"
#include "TDbiTableRow.hxx"
#include "TDbiTimerManager.hxx"
#include "TDbiValidityRec.hxx"
#include "TDbiValidityRecBuilder.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;

ClassImp(ND::TDbiTableProxy)

//   Definition of static data members
//   *********************************


//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

///
///
///  Purpose:  Constructor
///
///  Arguments:
///             in  cascader   Reference to one and only cascader
///             in  tableName  Table name.
///             in  tableRow   Example table row object.
///
///  Return:    n/a
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Create table proxy for supplied table name.
///
///
///  Program Notes:-
///  =============
///
ND::TDbiTableProxy::TDbiTableProxy(ND::TDbiCascader* cascader,
                             const string& tableName,
                             const ND::TDbiTableRow* tableRow) :
fCascader(cascader),
fMetaData(tableName),
fMetaValid(tableName+"VLD"),
fCanL2Cache(kFALSE),
fCache(0),
fDBProxy(*cascader,tableName,&fMetaData,&fMetaValid,this),
fExists(0),
fTableName(tableName),
fTableRow(tableRow->CreateTableRow())
{
//one.


  fCache = new ND::TDbiCache(*this,fTableName);
  this->RefreshMetaData();
  fExists = fDBProxy.TableExists();
  fCanL2Cache = tableRow->CanL2Cache();
  if ( fCanL2Cache )
    DbiInfo( "ND::TDbiTableProxy: Can use L2 cache for table " << this->GetRowName() << "  ");
  else
    DbiInfo( "ND::TDbiTableProxy:  L2 cache not allowed for table " << this->GetRowName() << "  ");
   
  DbiTrace( "Creating ND::TDbiTableProxy "
			  << fTableName.c_str() << " at " << this
                          << ( fExists ? " (table exists)"
                                       : " (table missing)" )
			  << "  ");
}

//.....................................................................

ND::TDbiTableProxy::~TDbiTableProxy() {
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
//  o  Destroy object.


//  Program Notes:-
//  =============

//  None.


    DbiTrace( "Destroying ND::TDbiTableProxy "
			    << fTableName << " at " << this
			    << "  ");
  delete fCache;
  delete fTableRow;

}
//.....................................................................

Bool_t ND::TDbiTableProxy::CanReadL2Cache() const {
//

  return fCanL2Cache && ND::TDbiBinaryFile::CanReadL2Cache();

}
Bool_t ND::TDbiTableProxy::CanWriteL2Cache() const {
//

  return fCanL2Cache && ND::TDbiBinaryFile::CanWriteL2Cache();

}

//.....................................................................

const ND::TDbiResultSet* ND::TDbiTableProxy::Query(const ND::TVldContext& vc,
                                      const TDbi::Task& task,
                                      Bool_t findFullTimeWindow) {
//
//
//  Purpose:  Apply context specific query to database table and return result.
//
//  Arguments:
//    vc           in    The Validity Context for the query.
//    task         in    The task of the query.
//    findFullTimeWindow
//                 in    Attempt to find full validity of query
//                        i.e. beyond TDbi::GetTimeGate
//
//  Return:    Query result (never zero even if query fails).
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Apply query to database table and return result.

//  Program Notes:-
//  =============

//  None.

//  See if there is one already in the cache for universal aggregate no.

  if ( const ND::TDbiResultSet* result = fCache->Search(vc,task)
     ) return result;

  ND::TDbiConnectionMaintainer cm(fCascader);  //Stack object to hold connections

// Make Global Exception Log bookmark
  UInt_t startGEL = ND::TDbiExceptionLog::GetGELog().Size()+1;

// Build a complete set of effective validity record from the database.
  ND::TDbiValidityRecBuilder builder(fDBProxy,vc,task,-1,findFullTimeWindow);

// Deal with non-aggregated data.

  if ( builder.NonAggregated() ) {

    ND::TDbiValidityRec effVRec = builder.GetValidityRec(0);
//  Force off const - we haven't finished with ND::TDbiResultSet yet!
    ND::TDbiResultSet* result = const_cast<ND::TDbiResultSet*>(Query(effVRec));
//  Record latest entries from Global Exception Log.
    result->CaptureExceptionLog(startGEL);
    return result;
  }

// Deal with aggregated data.

// Don't look in the level 2 cache if more than half of the
// component aggregates are already in the cache;
// for in this case, the chances are that we have just
// crossed a validity boundary in only a few aggregates and
// we don't want to waste time loading in a full set only to throw
// it away again.

  if ( this->CanReadL2Cache() ) {
    UInt_t numPresent  = 0;
    UInt_t numRequired = 0;
    Int_t maxRow = builder.GetNumValidityRec() - 1;
    for ( Int_t rowNo = 1; rowNo <= maxRow; ++rowNo ) {
      const ND::TDbiValidityRec& vrec = builder.GetValidityRec(rowNo);
      if ( fCache->Search(vrec) ) ++numPresent;
      else if ( ! vrec.IsGap() ) ++numRequired;
    }
    if ( numRequired < numPresent ) DbiInfo(  "Skipping search of L2 cache; already have "
      << numPresent << " aggregates, and only require a further "
      << numRequired << "  ");
    else this->RestoreFromL2Cache(builder);
  }

  ND::TDbiResultSet* result = new ND::TDbiResultSetAgg(fTableName,
                                       fTableRow,
                                       fCache,
                                       &builder,
                                       &fDBProxy);
// Record latest entries from Global Exception Log.
  result->CaptureExceptionLog(startGEL);

  fCache->Adopt(result);
  this->SaveToL2Cache(builder.GetL2CacheName(),*result);
  return result;

}
//.....................................................................

const ND::TDbiResultSet* ND::TDbiTableProxy::Query(const string& context,
                                      const TDbi::Task& task,
                                      const string& data,
                                      const string&fillOpts) {
//
//
//  Purpose:  Apply extended context query to database table and return result.
//
//  Arguments:
//    context      in    The Validity Context (see ND::TDbiSqlContext)
//    task         in    The task of the query.
//    data         in    Optional SQL extension to secondary query.
//    fillOpts     in    Optional fill options (available to ND::TDbiTableRow)
//
//  Return:    Query result (never zero even if query fails).
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Apply extended context query to database table and return result.
//
//  o Don't save/restore to L2 cache: encoding the query name as a file name
//    would be cumbersome and in any case extended queries are abnormal
//    so optimisation is unwarranted.


//  Construct the query's "SQL Qualifiers" by forming the 3 strings
//  (which task encoded into the context) into a single semi-colon
//  separated string.

  std::ostringstream os;
  os << context;
  if ( task != TDbi::kAnyTask
       ) os << " and  Task = " << task;
  os <<  ';' << data << ';' << fillOpts;
  string sqlQualifiers = os.str();

  DbiVerbose(  "Extended query: sqlQualifiers: " << sqlQualifiers << "  ");

//  See if there is one already in the cache.

  if ( const ND::TDbiResultSet* result = fCache->Search(sqlQualifiers)
     ) return result;

  ND::TDbiConnectionMaintainer cm(fCascader);  //Stack object to hold connections

// Make Global Exception Log bookmark
  UInt_t startGEL = ND::TDbiExceptionLog::GetGELog().Size()+1;

// Build a complete set of effective validity records from the database.
  ND::TDbiValidityRecBuilder builder(fDBProxy,context,task);

// For extended context queries, ND::TDbiValidityRecBuilder will always
// assemble a result that has to be represented by a ND::TDbiResultSetAgg

  ND::TDbiResultSet* result = new ND::TDbiResultSetAgg(fTableName,
                                       fTableRow,
                                       fCache,
                                       &builder,
                                       &fDBProxy,
                                       sqlQualifiers);
// Record latest entries from Global Exception Log.
  result->CaptureExceptionLog(startGEL);

  fCache->Adopt(result);
  return result;

}
//.....................................................................

const ND::TDbiResultSet* ND::TDbiTableProxy::Query(UInt_t seqNo,UInt_t dbNo) {
//
//
//  Purpose:  Apply non-agregate query to database table and return result.
//
//  Arguments:
//    seqNo        in    The sequence number of validity record that satisfies the query.
//    dbNo         in    Database number in the cascade.
//
//  Return:    Query result (never zero even if query fails).

  ND::TDbiConnectionMaintainer cm(fCascader);  //Stack object to hold connections

// Make Global Exception Log bookmark
  UInt_t startGEL = ND::TDbiExceptionLog::GetGELog().Size()+1;

  // Apply SEQNO query to cascade member.
  ND::TDbiInRowStream* rs = fDBProxy.QueryValidity(seqNo,dbNo);
  ND::TDbiValidityRec tr;
  ND::TDbiResultSetNonAgg result(rs,&tr,0,kFALSE);
  delete rs;

  // If query failed, return an empty result.
  if ( result.GetNumRows() == 0 ) {
    ND::TDbiResultSetNonAgg* empty = new ND::TDbiResultSetNonAgg();
//  Record latest entries from Global Exception Log.
    empty->CaptureExceptionLog(startGEL);
    fCache->Adopt(empty);
    return empty;
  }

// Otherwise perform a validity rec query, but don't
// allow result to be used; it's validity has not been trimmed
// by neighbouring records.

  const ND::TDbiValidityRec* vrec
       = dynamic_cast<const ND::TDbiValidityRec*>(result.GetTableRow(0));
//  Force off const - we haven't finished with ND::TDbiResultSet yet!
  ND::TDbiResultSet* res = const_cast<ND::TDbiResultSet*>(Query(*vrec,kFALSE));
// Record latest entries from Global Exception Log.
  res->CaptureExceptionLog(startGEL);
  return res;

}
//.....................................................................

const ND::TDbiResultSet* ND::TDbiTableProxy::Query(const ND::TDbiValidityRec& vrec,
                                      Bool_t canReuse /* = kTRUE */) {
//
//
//  Purpose:  Apply non-agregate query to database table and return result.
//
//  Arguments:
//    vrec         in    The validity record that satisfies the query.
//    canReuse     in    True if result is to be cached.
//
//  Return:    Query result (never zero even if query fails).
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Apply non-aggregated query to main database table. Cache if required,
//    and return result.


// See if it can be recovered from the level 2 disk cache.

  ND::TDbiConnectionMaintainer cm(fCascader);  //Stack object to hold connections

// Make Global Exception Log bookmark
  UInt_t startGEL = ND::TDbiExceptionLog::GetGELog().Size()+1;

  if ( canReuse ) {
    ND::TDbiValidityRecBuilder builder(vrec,this->GetTableName());
    if ( this->RestoreFromL2Cache(builder) ) {
      const ND::TDbiResultSet* res = fCache->Search(vrec);
      if ( res ) return res;
    }
  }

  unsigned int seqNo = vrec.GetSeqNo();
  ND::TDbiResultSet* result = 0;

//  If no records, create an empty ND::TDbiResultSet.
  if ( ! seqNo ) {
    result = new ND::TDbiResultSetNonAgg(0,0,&vrec);
  }

//  If query does not apply to this table, report error and
//  produce an empty ND::TDbiResultSet.

  else if (vrec.GetTableProxy()->GetTableName() != GetTableName() ) {
       DbiSevere(  "Unable to satisfy ND::TDbiValidityRec keyed query:" << "  "
       << vrec
       << " was filled by " << vrec.GetTableProxy()->GetTableName()
       << " not by this ND::TDbiTableProxy ("
       << GetTableName() << ")" << "  ");
    result = new ND::TDbiResultSetNonAgg(0,0,&vrec);
  }

  else {


// Apply query, and build DiResult from its ND::TDbiInRowStream.

    ND::TDbiInRowStream* rs = fDBProxy.QuerySeqNo(seqNo,vrec.GetDbNo());
    result = new ND::TDbiResultSetNonAgg(rs,fTableRow,&vrec);
    delete rs;
  }

// Record latest entries from Global Exception Log.
  result->CaptureExceptionLog(startGEL);

//  Cache in memory and on disk if required and return the results.

  fCache->Adopt(result);
  if ( canReuse ) this->SaveToL2Cache(vrec.GetL2CacheName(),*result);
  else result->SetCanReuse(kFALSE);

  return result;

}

//.....................................................................

void ND::TDbiTableProxy::RefreshMetaData() {
//
//
//  Purpose:  Refresh meta data for table.
//

  fDBProxy.StoreMetaData(fMetaData);
  fDBProxy.StoreMetaData(fMetaValid);

}
//.....................................................................

ND::TVldTimeStamp ND::TDbiTableProxy::QueryOverlayCreationDate(const ND::TDbiValidityRec& vrec,
					             UInt_t dbNo)
{
//
//  Purpose:  Determine a suitable Creation Date so that this validity
//            record, if written to the selected DB, will overlay
//            correctly.
//
//  Specification:-
//  =============
//
//  o Determine optimal Creation Date to overlay new data.  See Program Notes.

//  Program Notes:-
//  =============


// It is normal practice, particularly for calibration data, to have
// overlapping the validity records.  Each time a new set of runs are
// processed the start time of the validity is set to the start time of
// the first run and the end time is set beyond the start time by an
// interval that characterises the stability of the constants.  So long
// as a new set of constants is created before the end time is reached
// there will be no gap.  Where there is an overlap the Creation Date is
// used to select the later constants on the basis that later is better.
// However, if reprocessing old data it is also normal practice to
// process recent data first and in this case the constants for earlier
// data get later creation dates and overlay works the wrong way.  To
// solve this, the creation date is faked as follows:-
//
//
//   1.  For new data i.e. data that does not overlay any existing data,
//       the creation date is set to the validity start time.
//
//   2.  For replacement data i.e. data that does overlay existing data,
//       the creation date is set to be one minute greater than the Creation
//       Date on the current best data.
//
// This scheme ensures that new data will overlay existing data at the
// start of its validity but will be itself overlaid by data that has
// a later start time (assuming validity record start times are more
// than a few minutes apart)


  //  Create a context that corresponds to the start time of the validity
  //  range.  Note that it is O.K. to use SimFlag and Detector masks
  //  even though this could make the context ambiguous because the
  //  context is only to be used to query the database and the SimFlag and
  //  Detector values will be ORed against existing data so will match
  //  all possible data that this validity range could overlay which is
  //  just what we want.

  const ND::TVldRange& vr(vrec.GetVldRange());
  ND::TVldContext vc((ND::DbiDetector::Detector_t) vr.GetDetectorMask(),
                  (ND::DbiSimFlag::SimFlag_t) vr.GetSimMask(),
                                       vr.GetTimeStart());

  ND::TDbiConnectionMaintainer cm(fCascader);  //Stack object to hold connections

  // Build a complete set of effective validity records from the
  // selected database.
  ND::TDbiValidityRecBuilder builder(fDBProxy,vc,vrec.GetTask(),dbNo);

  // Pick up the validity record for the current aggregate.
  const ND::TDbiValidityRec& vrecOvlay(builder.GetValidityRecFromAggNo(vrec.GetAggregateNo()));

  // If its a gap i.e. nothing is overlayed, return the start time, otherwise
  // return its Creation Date plus one minute.
  ND::TVldTimeStamp ovlayTS(vr.GetTimeStart());
  if ( ! vrecOvlay.IsGap() ) {
    time_t overlaySecs = vrecOvlay.GetCreationDate().GetSec();
    ovlayTS = ND::TVldTimeStamp(overlaySecs + 60,0);
  }

  DbiDebug( "Looking for overlay creation date for: "
		   << vrec << "found it would overlap: "
		   << vrecOvlay << " so overlay creation date set to "
		   << ovlayTS.AsString("s") << "  ");
  return ovlayTS;

}
//.....................................................................

Bool_t ND::TDbiTableProxy::RestoreFromL2Cache(const ND::TDbiValidityRecBuilder& builder) {
//
//
//  Purpose: Restore results from named level 2 disk cache into memory cache.
//  Returns true if anything restored

//  Specification:-
//  =============
//
//  o Restore to cache but only if enabled and exists.

  const string name(builder.GetL2CacheName());
  DbiDebug( "Request to restore query result  " << name
			 << "  ");
  if ( ! this->CanReadL2Cache() ) return kFALSE;
  string cacheFileName;
  if (  name != ""
   ) cacheFileName =  this->GetTableName() + "_"
                    + this->GetRowName() + "_"
                    +  name + ".dbi_cache";
  ND::TDbiBinaryFile bf(cacheFileName.c_str());
  if ( ! bf.IsOK() ) {
    DbiDebug( "Caching disabled or cannot open "
			   << bf.GetFileName() << "  ");
    return kFALSE;
  }

  static bool warnOnce = true;
  if ( warnOnce ) {
    DbiWarn( "\n\n\n"
     << " WARNING:  Reading from the Level 2 cache has been activated.\n"
     << " *******   This should only be used for development and never for production !!!\n\n\n");
    warnOnce = false;
  }

  DbiInfo( "Restoring query result from " << bf.GetFileName() << "  ");
  ND::TDbiTimerManager::gTimerManager.RecMainQuery();

  ND::TDbiResultSet* result    = 0;
  unsigned numRowsRest = 0;
  unsigned numRowsIgn  = 0;
  UInt_t numNonAgg     = 0;
  bf >> numNonAgg;

  while ( numNonAgg-- ) {
    if ( ! bf.IsOK() ) break;
    if ( ! result ) result = new ND::TDbiResultSetNonAgg;
    bf >> *result;

//  The original query may have had a validity range truncated by
//  the time window, so replace its ND::TDbiValidityRec with the one
//  just obtained from the database.
    const ND::TDbiValidityRec& vrec = result->GetValidityRec();
    UInt_t seqNo = vrec.GetSeqNo();
    DbiDebug( "Fix up L2 cache ND::TDbiValidityRec, by replacing: " << vrec
			   << "    with: " << builder.GetValidityRecFromSeqNo(seqNo) << "  ");
//  Sneaky end-run round const to fix-up ND::TDbiValidityRec.
    (const_cast<ND::TDbiValidityRec&>(vrec)) = builder.GetValidityRecFromSeqNo(seqNo);

//  Adopt only if not already in memory cache.
    if ( ! fCache->Search(vrec) ) {
      numRowsRest += result->GetNumRows();
      fCache->Adopt(result);
      result = 0;
    }
    else numRowsIgn += result->GetNumRows();
  }
  DbiInfo( "   a total of " << numRowsRest << " were restored ("
			<< numRowsIgn << " ignored - already in memory)" << "  ");

  delete result;
  result = 0;

  return numRowsRest > 0;

}
//.....................................................................

Bool_t ND::TDbiTableProxy::SaveToL2Cache(const string& name, ND::TDbiResultSet& res) {
//
//
//  Purpose: Save result to named level 2 cache. Returns true if saved.

//  Specification:-
//  =============
//
//  o Save to cache but only if enabled and suitable.

  DbiDebug( "Request to save query result as " << name
                         << " ; row supports L2 cache ?"<<fCanL2Cache
                         << " ; binary file can write L2 Cache?"<< ND::TDbiBinaryFile::CanWriteL2Cache()
			 << " ; data from DB? " << res.ResultsFromDb()
			 << " ; can be saved? " << res.CanSave() << "  ");
  if ( ! this->CanWriteL2Cache() || ! res.ResultsFromDb() || ! res.CanSave() ) return kFALSE;

  string cacheFileName;
  if (  name != ""
   ) cacheFileName =  this->GetTableName() + "_"
                    + this->GetRowName() + "_"
                    +  name + ".dbi_cache";
  ND::TDbiBinaryFile bf(cacheFileName.c_str(),kFALSE);
  if ( bf.IsOK() ) {
    DbiInfo( "Saving query result (" << res.GetNumRows()
			  << " rows) to " << bf.GetFileName() << "  ");
     ND::TDbiTimerManager::gTimerManager.RecMainQuery();

    // if writing a ND::TDbiResultSetNonAgg, add leading count of 1. (if writing
    // a ND::TDbiResultSetAgg it will writes its one leading count.
    if ( dynamic_cast<ND::TDbiResultSetNonAgg*>(&res) ) {
      UInt_t numNonAgg = 1;
      bf << numNonAgg;
    }
    bf << res;
    return kTRUE;
  }
  DbiDebug( "Caching disabled or cannot open "
			 << bf.GetFileName() << "  ");
  return kFALSE;

}
//.....................................................................

void ND::TDbiTableProxy::SetSqlCondition(const string& sql) {
//
//
//  Purpose:  Apply Sql condition to its ND::TDbiDBProxy.
//
//  Arguments:
//   sql           in    SQL condition string (excluding where).
//
//  Return:  n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Apply Sql condition to its ND::TDbiDBProxy.

//  Program Notes:-
//  =============

//  None.

  fDBProxy.SetSqlCondition(sql);

}


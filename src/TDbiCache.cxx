// $Id: TDbiCache.cxx,v 1.1 2011/01/18 05:49:19 finch Exp $

#include "TDbiCache.hxx"
#include "TDbiResultSet.hxx"
#include "TDbiResultKey.hxx"
#include "TDbiResultSetNonAgg.hxx"
#include "TDbiSimFlagAssociation.hxx"
#include "TDbiValidityRec.hxx"
#include <MsgFormat.hxx>
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>
#include "TVldContext.hxx"

ClassImp(CP::TDbiCache)

// Typedefs

  typedef CP::TDbiCache::ResultList_t ResultList_t;
  typedef std::map<Int_t,ResultList_t>::const_iterator ConstCacheItr_t;
  typedef std::map<Int_t,ResultList_t>::iterator CacheItr_t;
  typedef ResultList_t::const_iterator ConstSubCacheItr_t;
  typedef ResultList_t::iterator SubCacheItr_t;


//   Definition of static data members
//   *********************************


//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.



//.....................................................................
///\verbatim
///
///  Purpose:  Constructor
///
///  Arguments:
///             in  qp         Owning CP::TDbiTableProxy.
///             in  tableName  Name of associated table
///
///  Return:    n/a
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Create empty cache.
///
///
///  Program Notes:-
///  =============
///
///  None.
///\endverbatim
CP::TDbiCache::TDbiCache(CP::TDbiTableProxy& qp,const std::string& tableName) :
fTableProxy(qp),
fTableName(tableName),
fCurSize(0),
fMaxSize(0),
fNumAdopted(0),
fNumReused(0)
{


  DbiTrace( "Creating CP::TDbiCache" << "  ");

}

//.....................................................................
///\verbatim
///
///  Purpose: Destructor
///
///  Arguments:
///    None.
///
///  Return:    n/a
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o  Destroy cache and all owned CP::TDbiResultSets.
///
///
///  Program Notes:-
///  =============
///
///  None.
///\endverbatim
CP::TDbiCache::~TDbiCache() {



  DbiTrace( "Destroying CP::TDbiCache" << "  ");

  // Purge the AggNo == -1 cache before deleting.  For extended
  // context queries it can have CP::TDbiResultSetAggs that are clients of
  // CP::TDbiResultSetNonAggs in the same cache, so purging will remove clientless
  // CP::TDbiResultSetAggs which should in turn make their CP::TDbiResultSetNonAggs
  // clientless.
  if ( this->GetSubCache(-1) ) this->Purge(fCache[-1]);

  for ( CacheItr_t itr = fCache.begin(); itr != fCache.end(); ++itr) {
    ResultList_t& subCache = itr->second;
    for ( SubCacheItr_t sitr = subCache.begin();
          sitr != subCache.end();
          ++sitr) delete *sitr;
  }

}

//.....................................................................
///\verbatim
///
///  Purpose:  Adopt and own a CP::TDbiResultSet
///
///  Arguments:
///    res          in    The CP::TDbiResiult to be adopted.
///    generateKey  in    If true generate key
///
///  Return:  None.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Create new sub-cache for aggregate if necessary.
///
///  o Purge sub-cache of unwanted data and adopt new result.
///
///  Program Notes:-
///  =============
///
///  New entries are added to the end of the sub-cache unwanted entries
///  are always removed from the beginning so sub-cache is a FIFO.
///\endverbatim
void CP::TDbiCache::Adopt(CP::TDbiResultSet* res,bool generateKey) {

  if ( ! res ) return;
  int aggNo = res->GetValidityRec().GetAggregateNo();

//  Prime sub-cache if necessary.
  if ( ! this->GetSubCache(aggNo) ) {
    ResultList_t emptyList;
    fCache[aggNo] = emptyList;
  }

//  Purge expired entries and add new result to cache.
  ResultList_t& subCache = fCache[aggNo];
  Purge(subCache, res);
  subCache.push_back(res);
  ++fCurSize;
  ++fNumAdopted;
  DbiDebug( "Adopting result for " << res->TableName()
			 << "  " <<   res->GetValidityRecGlobal()
			 << "\nCache size now " << fCurSize << "  ");
  if ( fCurSize > fMaxSize ) fMaxSize = fCurSize;
  // If required generate key.
  if ( generateKey ) {
    res->GenerateKey();
    DbiInfo( "Caching new results: ResultKey: " <<  *res->GetKey());
  }
}

//.....................................................................
///\verbatim
///
///  Purpose:  Return sub-cache for aggregate or 0 if none..
///\endverbatim
const ResultList_t* CP::TDbiCache::GetSubCache(Int_t aggNo) const {

  ConstCacheItr_t itr = fCache.find(aggNo);
  return ( itr == fCache.end() ) ? 0 : &itr->second;

}

//.....................................................................
///\verbatim
///
///  Purpose: Purge all sub-caches.
///
///  Arguments:  None.
///
/// Return:   None.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Purge all sub_caches.
///
///  Program Notes:-
///  =============
///
///  The first sub-cached to be purged must be sub-cache -1 as
///  its members may be aggregated and consequently will be
///  connected to members in other caches.
///\endverbatim
void CP::TDbiCache::Purge() {




  for ( CacheItr_t itr = fCache.begin(); itr != fCache.end(); ++itr
      ) Purge(itr->second);

}
//.....................................................................
///\verbatim
///
///  Purpose: Purge surplus sub-cache memebers.
///
///  Arguments:
///    subCache   in/out  The sub-cache to be purged
///    res        in      Optional CP::TDbiResultSet (default =0)
///
///  Return:   None.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Purge surplus sub-cache members i.e. those without clients.
///
///  o If a CP::TDbiResultSet is supplied, only purge entries that have
///    expired relative to it or are stale.
///
///  Program Notes:-
///  =============
///\endverbatim
void CP::TDbiCache::Purge(ResultList_t& subCache, const CP::TDbiResultSet* res) {


//  Passing a CP::TDbiResultSet allows the sub-cache to hold entries
//  for different detector types, simulation masks and tasks.

    for ( SubCacheItr_t itr = subCache.begin(); itr != subCache.end(); ) {
    CP::TDbiResultSet* pRes = *itr;

     if (      pRes->GetNumClients() == 0
          && (    ! res
               || pRes->CanDelete(res)  ) ) {

      DbiDebug( "Purging " << pRes->GetValidityRec()
		  	     << " from " << pRes->TableName()
			     << " cache. Cache size now "
			     << fCurSize-1 << "  ");
      delete pRes;
//    Erasing increments iterator.
      itr = subCache.erase(itr);
      --fCurSize;

    }
    else {
      ++itr;
    }
  }

}
//.....................................................................
///\verbatim
///
///  Purpose:  Search sub-cache for CP::TDbiResultSet set matching a CP::TDbiValidityRec.
///            with an optional sqlQualifiers string.
///  Return:   Pointer to matching CP::TDbiResultSet, or = 0 if none.
///\endverbatim
const CP::TDbiResultSet* CP::TDbiCache::Search(const CP::TDbiValidityRec& vrec,
                                  const std::string& sqlQualifiers) const {


  Int_t aggNo = vrec.GetAggregateNo();

  DbiTrace( "Secondary cache search of table " << fTableName
			 << " for  " << vrec
			    << (sqlQualifiers != "" ? sqlQualifiers : "" ) << "  ");
  const ResultList_t* subCache = this->GetSubCache(aggNo);
  if ( ! subCache ) {
    DbiTrace( "Secondary cache search failed." << "  ");
    return 0;
  }

  ConstSubCacheItr_t itrEnd = subCache->end();
  for ( ConstSubCacheItr_t itr = subCache->begin();
        itr != itrEnd;
        ++itr) {
    CP::TDbiResultSet* res = *itr;
    if ( res->Satisfies(vrec,sqlQualifiers) ) {
      fNumReused += res->GetNumAggregates();
      DbiTrace( "Secondary cache search succeeded.  Result set no. of rows: "
				<< res->GetNumRows() << "  ");
      return res;
    }
  }

  DbiTrace( "Secondary cache search failed." << "  ");
  return 0;
}

//.....................................................................
///\verbatim
///
///  Purpose:  Search primary cache for CP::TDbiResultSet set matching a new query.
///
///  Arguments:
///    vc           in    Context of new query
///    task         in    Task of new query
///
///  Return:   Pointer to matching CP::TDbiResultSet, or = 0 if none.
///\endverbatim
const CP::TDbiResultSet* CP::TDbiCache::Search(const CP::TVldContext& vc,
                                  const TDbi::Task& task ) const {

  DbiTrace( "Primary cache search of table " << fTableName
			 << " for  " << vc
			 << " with task " << task << "  ");
  const ResultList_t* subCache = this->GetSubCache(-1);
  if ( ! subCache ) {
    DbiTrace( "Primary cache search failed - sub-cache -1 is empty" << "  ");
    return 0;
  }

  // Loop over all possible SimFlag associations.

  CP::DbiDetector::Detector_t     det(vc.GetDetector());
  CP::DbiSimFlag::SimFlag_t       sim(vc.GetSimFlag());
  CP::TVldTimeStamp              ts(vc.GetTimeStamp());

  CP::TDbiSimFlagAssociation::SimList_t simList
                  = CP::TDbiSimFlagAssociation::Instance().Get(sim);

  CP::TDbiSimFlagAssociation::SimList_t::iterator listItr    = simList.begin();
  CP::TDbiSimFlagAssociation::SimList_t::iterator listItrEnd = simList.end();
  while ( listItr !=  listItrEnd ) {

    CP::DbiSimFlag::SimFlag_t simTry = *listItr;
    CP::TVldContext vcTry(det,simTry,ts);

    DbiDebug( "  Searching cache with SimFlag: "
			   << CP::DbiSimFlag::AsString(simTry) << "  ");
    for ( ConstSubCacheItr_t itr = subCache->begin();
          itr != subCache->end();
          ++itr) {
      CP::TDbiResultSet* res = *itr;
      if ( res->Satisfies(vcTry,task) ) {
        fNumReused += res->GetNumAggregates();
	DbiTrace( "Primary cache search succeeded. Result set no. of rows: "
				  << res->GetNumRows() << "  ");
        return res;
      }
    }

  DbiTrace( "Primary cache search failed." << "  ");
  ++listItr;
  }

  return 0;
}
//.....................................................................
///\verbatim
///
///  Purpose:  Search primary cache for CP::TDbiResultSet set matching a new query.
///
///  Arguments:
///    sqlQualifiers  in  The SQL qualifiers (context-sql;data-sql;fill-options)
///
///  Return:   Pointer to matching CP::TDbiResultSet, or = 0 if none.
///\endverbatim
const CP::TDbiResultSet* CP::TDbiCache::Search(const std::string& sqlQualifiers) const {

  DbiTrace( "Primary cache search of table " << fTableName
			 << " for  SQL " << sqlQualifiers << "  ");
  const ResultList_t* subCache = this->GetSubCache(-1);
  if ( ! subCache ) {
    DbiTrace( "Primary cache search failed" << "  ");
    return 0;
  }
  for ( ConstSubCacheItr_t itr = subCache->begin();
        itr != subCache->end();
        ++itr) {
    CP::TDbiResultSet* res = *itr;
    if ( res->Satisfies(sqlQualifiers) ) {
      fNumReused += res->GetNumAggregates();
      DbiTrace( "Primary cache search succeeded Result set no. of rows: "
				<< res->GetNumRows() << "  ");
      return res;
    }
  }
  DbiTrace( "Primary cache search failed" << "  ");
  return 0;
}

//.....................................................................
///\verbatim
///
///  Purpose: Set all entries in the cache as stale i.e. don't reuse.
///
///  Arguments: None
///
///  Return:    n/a
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Set all entries in the cache as stale i.e. don't reuse.
///
///  Program Notes:-
///  =============
///
///  This member function can be used to effectively clear the cache.
///  As existing CP::TDbiResultSet objects currently in the cache may currently
///  have clients, its not possible simply to delete them, so instead
///  this function marks them as stale so they will not be reused and
///  will eventually be dropped once all their clients have disconnected.
///\endverbatim

void CP::TDbiCache::SetStale() {

  for ( CacheItr_t cacheItr = fCache.begin();
        cacheItr != fCache.end();
        ++cacheItr
	) {
    ResultList_t& subcache = cacheItr->second;

    for ( SubCacheItr_t subcacheItr = subcache.begin();
          subcacheItr != subcache.end();
          ++subcacheItr ) (*subcacheItr)->SetCanReuse(kFALSE);
  }

}

//.....................................................................
///\verbatim
///
///  Purpose: Display statistics for cache on supplied ostream.
///
///  Arguments:
///    msg          in    ostream to output on.
///
///  Return: Updated ostream.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Output : Current Size, Max size, Adopted and Resused as
///             4 10 character wide fields.
///
///  Program Notes:-
///  =============
///
///  None.
///\endverbatim
std::ostream& CP::TDbiCache::ShowStatistics(std::ostream& msg) const {

  MsgFormat ifmt("%10i");

  msg << ifmt(fCurSize) << ifmt(fMaxSize)
      << ifmt(fNumAdopted) << ifmt(fNumReused);
  return msg;

}

/*    Template for New Member Function

//.....................................................................

CP::TDbiCache:: {
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




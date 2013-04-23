// $Id: TDbiCache.cxx,v 1.1 2011/01/18 05:49:19 finch Exp $

#include "TDbiCache.hxx"
#include "TDbiResultSet.hxx"
#include "TDbiResultKey.hxx"
#include "TDbiResultSetNonAgg.hxx"
#include "TDbiSimFlagAssociation.hxx"
#include "TDbiValidityRec.hxx"
#include <MsgFormat.h>
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;
#include "TVldContext.hxx"

ClassImp(ND::TDbiCache)

// Typedefs

  typedef ND::TDbiCache::ResultList_t ResultList_t;
  typedef map<Int_t,ResultList_t>::const_iterator ConstCacheItr_t;
  typedef map<Int_t,ResultList_t>::iterator CacheItr_t;
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
///             in  qp         Owning ND::TDbiTableProxy.
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
ND::TDbiCache::TDbiCache(ND::TDbiTableProxy& qp,const string& tableName) :
fTableProxy(qp),
fTableName(tableName),
fCurSize(0),
fMaxSize(0),
fNumAdopted(0),
fNumReused(0)
{


  DbiTrace( "Creating ND::TDbiCache" << "  ");

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
///  o  Destroy cache and all owned ND::TDbiResultSets.
///
///
///  Program Notes:-
///  =============
///
///  None.
///\endverbatim
ND::TDbiCache::~TDbiCache() {



  DbiTrace( "Destroying ND::TDbiCache" << "  ");

  // Purge the AggNo == -1 cache before deleting.  For extended
  // context queries it can have ND::TDbiResultSetAggs that are clients of
  // ND::TDbiResultSetNonAggs in the same cache, so purging will remove clientless
  // ND::TDbiResultSetAggs which should in turn make their ND::TDbiResultSetNonAggs
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
///  Purpose:  Adopt and own a ND::TDbiResultSet
///
///  Arguments:
///    res          in    The ND::TDbiResiult to be adopted.
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
void ND::TDbiCache::Adopt(ND::TDbiResultSet* res,bool generateKey) {

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
const ResultList_t* ND::TDbiCache::GetSubCache(Int_t aggNo) const {

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
void ND::TDbiCache::Purge() {




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
///    res        in      Optional ND::TDbiResultSet (default =0)
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
///  o If a ND::TDbiResultSet is supplied, only purge entries that have
///    expired relative to it or are stale.
///
///  Program Notes:-
///  =============
///\endverbatim
void ND::TDbiCache::Purge(ResultList_t& subCache, const ND::TDbiResultSet* res) {


//  Passing a ND::TDbiResultSet allows the sub-cache to hold entries
//  for different detector types, simulation masks and tasks.

    for ( SubCacheItr_t itr = subCache.begin(); itr != subCache.end(); ) {
    ND::TDbiResultSet* pRes = *itr;

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
///  Purpose:  Search sub-cache for ND::TDbiResultSet set matching a ND::TDbiValidityRec.
///            with an optional sqlQualifiers string.
///  Return:   Pointer to matching ND::TDbiResultSet, or = 0 if none.
///\endverbatim
const ND::TDbiResultSet* ND::TDbiCache::Search(const ND::TDbiValidityRec& vrec,
                                  const string& sqlQualifiers) const {


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
    ND::TDbiResultSet* res = *itr;
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
///  Purpose:  Search primary cache for ND::TDbiResultSet set matching a new query.
///
///  Arguments:
///    vc           in    Context of new query
///    task         in    Task of new query
///
///  Return:   Pointer to matching ND::TDbiResultSet, or = 0 if none.
///\endverbatim
const ND::TDbiResultSet* ND::TDbiCache::Search(const ND::TVldContext& vc,
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

  ND::DbiDetector::Detector_t     det(vc.GetDetector());
  ND::DbiSimFlag::SimFlag_t       sim(vc.GetSimFlag());
  ND::TVldTimeStamp              ts(vc.GetTimeStamp());

  ND::TDbiSimFlagAssociation::SimList_t simList
                  = ND::TDbiSimFlagAssociation::Instance().Get(sim);

  ND::TDbiSimFlagAssociation::SimList_t::iterator listItr    = simList.begin();
  ND::TDbiSimFlagAssociation::SimList_t::iterator listItrEnd = simList.end();
  while ( listItr !=  listItrEnd ) {

    ND::DbiSimFlag::SimFlag_t simTry = *listItr;
    ND::TVldContext vcTry(det,simTry,ts);

    DbiDebug( "  Searching cache with SimFlag: "
			   << ND::DbiSimFlag::AsString(simTry) << "  ");
    for ( ConstSubCacheItr_t itr = subCache->begin();
          itr != subCache->end();
          ++itr) {
      ND::TDbiResultSet* res = *itr;
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
///  Purpose:  Search primary cache for ND::TDbiResultSet set matching a new query.
///
///  Arguments:
///    sqlQualifiers  in  The SQL qualifiers (context-sql;data-sql;fill-options)
///
///  Return:   Pointer to matching ND::TDbiResultSet, or = 0 if none.
///\endverbatim
const ND::TDbiResultSet* ND::TDbiCache::Search(const string& sqlQualifiers) const {

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
    ND::TDbiResultSet* res = *itr;
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
///  As existing ND::TDbiResultSet objects currently in the cache may currently
///  have clients, its not possible simply to delete them, so instead
///  this function marks them as stale so they will not be reused and
///  will eventually be dropped once all their clients have disconnected.
///\endverbatim

void ND::TDbiCache::SetStale() {

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
ostream& ND::TDbiCache::ShowStatistics(ostream& msg) const {

  MsgFormat ifmt("%10i");

  msg << ifmt(fCurSize) << ifmt(fMaxSize)
      << ifmt(fNumAdopted) << ifmt(fNumReused);
  return msg;

}

/*    Template for New Member Function

//.....................................................................

ND::TDbiCache:: {
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




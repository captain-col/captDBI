// $Id: TDbiResultSetHandle.tpl,v 1.1 2011/01/18 05:49:20 finch Exp $

#include <cassert>
#include <cstdlib>

#include "TDbiResultKey.hxx"
#include "TDbiResultSetHandle.hxx"
#include "TDbiSqlContext.hxx"
#include "TDbiTableProxy.hxx"
#include "TDbiTimerManager.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;
#include "TVldTimeStamp.hxx"

ClassImpT(TDbiResultSetHandle,T)

//   Definition of static data members
namespace ND {
//   *********************************

//CVSID("$Id: TDbiResultSetHandle.tpl,v 1.1 2011/01/18 05:49:20 finch Exp $");

template<class T>
map<string,ND::TDbiTableProxy*>  TDbiResultSetHandle<T>::fgNameToProxy;

template<class T>
ND::TDbiTableProxy* TDbiResultSetHandle<T>::fgTableProxy = 0;


//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

template<class T>
TDbiResultSetHandle<T>::TDbiResultSetHandle() :
fAbortTest(TDbi::kDisabled),
fTableProxy(TDbiResultSetHandle<T>::GetTableProxy()),
fResult(0),
fDetType(ND::DbiDetector::kUnknown),
fSimType(ND::DbiSimFlag::kUnknown )
{
//
//
//  Purpose:  Default constructor
//
//  Arguments: None
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create ResultHandle.


//  Program Notes:-
//  =============

//  None.


  T pet;
  DbiTrace(  "(TRACE) Creating TDbiResultSetHandle for " << pet.GetName()
        << " Table Proxy at " << &fTableProxy << "  ");
}
//.....................................................................

template<class T>
TDbiResultSetHandle<T>::TDbiResultSetHandle(const TDbiResultSetHandle& that) :
fAbortTest(that.fAbortTest),
fTableProxy(that.fTableProxy),
fResult(that.fResult),
fDetType(that.fDetType),
fSimType(that.fSimType)
{
//
//
//  Purpose:  Copy constructor


  T pet;
  DbiTrace(  "Creating copy TDbiResultSetHandle for " << pet.GetName()
        << " Table Proxy at " << &fTableProxy << "  ");
  if ( fResult ) fResult->Connect();

}

//.....................................................................

template<class T>
TDbiResultSetHandle<T>::TDbiResultSetHandle(const ND::TVldContext& vc,
                              TDbi::Task task,
                              TDbi::AbortTest abortTest,
                              Bool_t findFullTimeWindow) :
fAbortTest(abortTest),
fTableProxy(TDbiResultSetHandle<T>::GetTableProxy()),
fResult(0),
fDetType(vc.GetDetector()),
fSimType(vc.GetSimFlag())
{
//
//
//  Purpose:  Construct and apply context specific query to default table.
//
//  Arguments:
//    vc           in    The Validity Context of the new query
//    task         in    The task of the new query
//    abortTest    in    Test which if failed triggers abort.
//    findFullTimeWindow
//                 in    Attempt to find full validity of query
//                        i.e. beyond TDbi::GetTimeGate
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create ResultHandle.


//  Program Notes:-
//  =============

//  None.


  T pet;
  DbiTrace( "Creating TDbiResultSetHandle for "
	         	  << pet.GetName() << " Table Proxy at "
			  << &fTableProxy << "  ");
  NewQuery(vc, task, findFullTimeWindow);

}

//.....................................................................

template<class T>
TDbiResultSetHandle<T>::TDbiResultSetHandle(const string& tableName,
                              const ND::TVldContext& vc,
                              TDbi::Task task,
                              TDbi::AbortTest abortTest,
                              Bool_t findFullTimeWindow) :
fAbortTest(abortTest),
fTableProxy(TDbiResultSetHandle<T>::GetTableProxy(tableName)),
fResult(0),
fDetType(vc.GetDetector()),
fSimType(vc.GetSimFlag())
{
//
//
//  Purpose:  Construct and apply context specific query to alternative table.
//
//  Arguments:
//    tableName    in    Name of table to use.
//    vc           in    The Validity Context of the new query
//    task         in    The task of the new query
//    abortTest    in    Test which if failed triggers abort.
//    findFullTimeWindow
//                 in    Attempt to find full validity of query
//                        i.e. beyond TDbi::GetTimeGate
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create ResultHandle.


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Creating TDbiResultSetHandle for "
	         	  << tableName << " Table Proxy at "
			  << &fTableProxy << "  ");
  NewQuery(vc, task, findFullTimeWindow);

}
//.....................................................................

template<class T>
TDbiResultSetHandle<T>::TDbiResultSetHandle(const string& tableName,
                              const ND::TDbiSqlContext& context,
			      const TDbi::Task& task,
                              const string& data,
                              const string& fillOpts,
                              TDbi::AbortTest abortTest) :
fAbortTest(abortTest),
fTableProxy(TDbiResultSetHandle<T>::GetTableProxy(tableName)),
fResult(0),
fDetType(context.GetDetector()),
fSimType(context.GetSimFlag())
{
//
//
//  Purpose:  Apply an extended context query to alternative table.
//
//  Arguments:
//    tableName    in    Name of table to use.
//    context      in    The Validity Context (see ND::TDbiSqlContext)
//    task         in    The task of the query. Default: TDbi::kAnyTask,
//    data         in    Optional SQL extension to secondary query. Default: "".
//    fillOpts     in    Optional fill options (available to ND::TDbiTableRow)
//                       Default: "".
//    abortTest    in    Test which if failed triggers abort.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create ResultHandle.


//  Program Notes:-
//  =============

//  None.


  T pet;
  DbiTrace( "Creating TDbiResultSetHandle for "
	   << tableName << " Table Proxy at "
	   << &fTableProxy << "  "
	   << "Extended context " << context.GetString() << "  ");

  NewQuery(context,task,data,fillOpts);

}

//.....................................................................

template<class T>
TDbiResultSetHandle<T>::TDbiResultSetHandle(const string& tableName,
                              const ND::TDbiValidityRec& vrec,
                              TDbi::AbortTest abortTest) :
fAbortTest(abortTest),
fTableProxy(TDbiResultSetHandle<T>::GetTableProxy(tableName)),
fResult(0)
{
//
//
//  Purpose:  Apply context specific query to alternative table.
//
//  Arguments:
//    tableName    in    Name of table to use.
//    vrec         in    The validity record that satisfies the query.
//    abortTest    in    Test which if failed triggers abort.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create ResultHandle.


//  Program Notes:-
//  =============

//  None.


  this->SetContext(vrec);
  T pet;
  DbiTrace( "Creating TDbiResultSetHandle for "
	         	  << tableName << " Table Proxy at "
			  << &fTableProxy << "  ");
  NewQuery(vrec);

}
//.....................................................................

template<class T>
TDbiResultSetHandle<T>::TDbiResultSetHandle(const string& tableName,
                              UInt_t seqNo,
                              UInt_t dbNo,
                              TDbi::AbortTest abortTest) :
fAbortTest(abortTest),
fTableProxy(TDbiResultSetHandle<T>::GetTableProxy(tableName)),
fResult(0)
{
//
//
//  Purpose:  Apply context specific query to alternative table.
//
//  Arguments:
//    tableName    in    Name of table to use.
//    seqno        in    The SEQNO of validity record that satisfies the query.
//    dbNo         in    The database number holding the validity record
//    abortTest    in    Test which if failed triggers abort.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create ResultHandle.


//  Program Notes:-
//  =============

//  None.



  T pet;
  DbiTrace( "(TRACE) Creating TDbiResultSetHandle for "
	         	  << tableName << " Table Proxy at "
			  << &fTableProxy << "  ");
  NewQuery(seqNo,dbNo);

}

//.....................................................................

template<class T>
TDbiResultSetHandle<T>::~TDbiResultSetHandle() {
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
//  o  Disconnect any associated Results and destroy TDbiResultSetHandle.


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "(TRACE) Destroying TDbiResultSetHandle" << "  ");
  Disconnect();

}
//.....................................................................

template<class T>
Bool_t TDbiResultSetHandle<T>::ApplyAbortTest() {
//
//
//  Purpose:  Apply abort test and return true if must abort.

  if (    fAbortTest == TDbi::kDisabled
       || this->GetNumRows() > 0 ) return kFALSE;

  bool tableExists = fTableProxy.TableExists();

  if ( ! tableExists ) {
    DbiSevere( "Fatal error: table "
                           << fTableProxy.GetTableName() << " does not exist"
			   << "  ");
    return kTRUE;
  }
  if ( fAbortTest == TDbi::kDataMissing) {
    DbiSevere( "Fatal error: no data found in existing table  "
                           << fTableProxy.GetTableName() << "  ");
    return kTRUE;
  }

  return kFALSE;
}

//.....................................................................

template<class T>
void TDbiResultSetHandle<T>::Disconnect() {
//
//
//  Purpose: Disconnect previous results.
//
//  Arguments: None.
//
//  Return:   None.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o  Disconnect any previous results.

//  Program Notes:-
//  =============

//  The Disconnect message is sent to ND::TDbiResultSet so that ND::TDbiCache
//  knows when its ND::TDbiResultSet objects are free of clients.

  if ( fResult && ND::TDbiDatabaseManager::IsActive() ) {
    fResult->Disconnect();
  }
  fResult = 0;

}
//.....................................................................

template<class T>
const ND::TDbiResultKey* TDbiResultSetHandle<T>::GetKey() const {
//
//
//  Purpose:  Return associated result key or an empty one if none exists.
//

  return fResult ? fResult->GetKey() : ND::TDbiResultKey::GetEmptyKey();

}
//.....................................................................

template<class T>
Int_t TDbiResultSetHandle<T>::GetResultID() const {
//
//
//  Purpose:  Return the ID of the current ND::TDbiResultSet.
//
  return fResult ? fResult->GetID() : 0;

}
//.....................................................................

template<class T>
ND::TDbiTableProxy& TDbiResultSetHandle<T>::GetTableProxy() {
//
//
//  Purpose:  Private static function to find default associated
//            ND::TDbiTableProxy.
//
//  Arguments: None.
//
//
//  Return:   Associated ND::TDbiTableProxy.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Ask TDbiRegistry for default Table Proxy and return it.

//  Program Notes:-
//  =============

//  This function creates an example Table Row object which
//  ND::TDbiTableProxy can copy and then use to make futher copies when
//  processing Result Sets.

  if ( ! fgTableProxy ) {
    T pet;
    fgTableProxy = &ND::TDbiDatabaseManager::Instance()
                                     .GetTableProxy(pet.GetName(),&pet);
  }
  return *fgTableProxy;
}

//.....................................................................

template<class T>
ND::TDbiTableProxy& TDbiResultSetHandle<T>::GetTableProxy(const string& tableName){
//
//
//  Purpose:  Private static function to find an alternative
//            associated ND::TDbiTableProxy.
//
//  Arguments:
//    tableName    in    Alternative table name
//
//  Return:   Associated ND::TDbiTableProxy.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Ask TDbiRegistry for alternative  Table Proxy and return it.

//  Program Notes:-
//  =============

//  This function creates an example Table Row object which
//  ND::TDbiTableProxy can copy and then use to make futher copies when
//  processing Result Sets.


// Check for request for default table.
  if ( tableName == "" ) return  TDbiResultSetHandle::GetTableProxy();

// See if we have seen this name before.
  map<string,ND::TDbiTableProxy*>::const_iterator itr
                                        = fgNameToProxy.find(tableName);
  if ( itr != fgNameToProxy.end() ) return *( (*itr).second );

// No, so ask the TDbiRegistry for it and save it for next time.
  T pet;
  ND::TDbiTableProxy* proxy = &ND::TDbiDatabaseManager::Instance()
                                     .GetTableProxy(tableName,&pet);
  fgNameToProxy[tableName] = proxy;
  return *proxy;
}

//.....................................................................

template<class T>
UInt_t TDbiResultSetHandle<T>::GetNumRows() const {
//
//
//  Purpose:  Get number or rows in query.
//
//  Arguments: None.
//
//
//  Return:    Number of rows found in query, = 0 if no or failed query.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Return number of rows found in query, = 0 if no or failed query.


//  Program Notes:-
//  =============

//  None.

  return ( fResult && ND::TDbiDatabaseManager::IsActive() )
          ? fResult->GetNumRows() : 0;

}
//.....................................................................

template<class T>
const T* TDbiResultSetHandle<T>::GetRow(UInt_t rowNum) const {
//
//
//  Purpose: Return pointer to concrete Table Row object at given row.
//
//  Arguments:
//    rowNum       in    Required row number
//
//  Return:        Pointer to concrete Table Row object at given row,
//                 =0 if none
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Return pointer to concrete Table Row object at given row or
//    0 if none.

//  Program Notes:-
//  =============

// None.

  return ( fResult && ND::TDbiDatabaseManager::IsActive() ) ?
      dynamic_cast<const T*>(fResult->GetTableRow(rowNum))
    : 0;
}
//.....................................................................

template<class T>
const T* TDbiResultSetHandle<T>::GetRowByIndex(UInt_t index) const {
//
//
//  Purpose: Return pointer to concrete Table Row object at given index.
//
//  Arguments:
//    rowNum       in    Required index number.
//
//  Return:        Pointer to concrete Table Row object at given index,
//                 =0 if none
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Return pointer to concrete Table Row object at given index or
//    0 if none.

//  Program Notes:-
//  =============

//  None.

  return ( fResult && ND::TDbiDatabaseManager::IsActive() ) ?
      dynamic_cast<const T*>(fResult->GetTableRowByIndex(index))
    : 0;
}

//.....................................................................

template<class T>
const ND::TDbiValidityRec* TDbiResultSetHandle<T>::GetValidityRec(
                              const ND::TDbiTableRow* row) const {
//
//
//  Purpose:  Return associated ND::TDbiValidityRec (if any).
//
//  Contact:   N. West

  return ( fResult && ND::TDbiDatabaseManager::IsActive() )
      ? &(fResult->GetValidityRec(row)) : 0 ;

}

//.....................................................................

template<class T>
UInt_t TDbiResultSetHandle<T>::NextQuery(Bool_t forwards) {
//
//
//  Purpose:  Get next validity range for previous query.
//
//  Arguments:
//    forwards     in    true to step direction forwards (default)
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Take the validity range of the current query and step across
//    the boundary to the adjacent range.


// Door stops.
  static ND::TVldTimeStamp startOfTime(0,0);
  static ND::TVldTimeStamp endOfTime(0x7FFFFFFF,0);

  if ( ! fResult ) return 0;

  DbiTrace( "\n\nStarting next query: direction "
                             << ( forwards ?  "forwards" : "backwards" ) << "\n" << "  ");

  const ND::TDbiValidityRec& vrec = fResult->GetValidityRec();
  const ND::TVldRange& vrnge      = vrec.GetVldRange();

  // If we are heading towards the final boundary, just return the same query.
  if (   forwards && vrnge.GetTimeEnd()   == endOfTime )   return fResult->GetNumRows();
  if ( ! forwards && vrnge.GetTimeStart() == startOfTime ) return fResult->GetNumRows();

  // Step across boundary and construct new context.
  // The end time limit is exclusive, so stepping to the end
  // does cross the boundary
  time_t ts = forwards ? vrnge.GetTimeEnd().GetSec()
                       : vrnge.GetTimeStart().GetSec() - 1;
  ND::TVldContext vc(fDetType,fSimType,ND::TVldTimeStamp(ts,0));

  return this->NewQuery(vc,vrec.GetTask(), true);

}


//.....................................................................

template<class T>
UInt_t TDbiResultSetHandle<T>::NewQuery(ND::TVldContext vc,
                                 TDbi::Task task,
                                 Bool_t findFullTimeWindow) {
//
//
//  Purpose:  Apply new query.
//
//  Arguments:
//    vc           in    The Validity Context of the new query
//    task         in    The task of the new query
//    findFullTimeWindow
//                 in    Attempt to find full validity of query
//                        i.e. beyond TDbi::GetTimeGate
//
//
//  Return:    The number of rows retrieved by query.  =0 if error.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Disconnect any previous results and apply new query to
//    associated database table

//  Program Notes:-
//  =============

//  None.

  if ( ! ND::TDbiDatabaseManager::IsActive() ) {
    fResult = 0;
    return 0;
  }
  fDetType = vc.GetDetector();
  fSimType = vc.GetSimFlag();

  DbiTrace( "\n\nStarting context query: "
                             << vc  << " task " << task << "\n" << "  ");

  ND::TDbiTimerManager::gTimerManager.RecBegin(fTableProxy.GetTableName(), sizeof(T));
  Disconnect();
  fResult = fTableProxy.Query(vc,task,findFullTimeWindow);
  fResult->Connect();
  ND::TDbiTimerManager::gTimerManager.RecEnd(fResult->GetNumRows());

  if ( this->ApplyAbortTest() ) {
    DbiSevere( "FATAL: "
                           << "while applying validity context query for "
			   << vc.AsString() << " with task " << task << "  ");
    throw  ND::EQueryFailed();
  }
  DbiTrace( "\nCompleted context query: "
                             << vc  << " task " << task
                             << " Found:  " << fResult->GetNumRows() << " rows\n" << "  ");
  return fResult->GetNumRows();

}
//.....................................................................

template<class T>
UInt_t TDbiResultSetHandle<T>::NewQuery(const ND::TDbiSqlContext& context,
			         const TDbi::Task& task,
                                 const string& data,
                                 const string& fillOpts) {
//
//
//  Purpose:  Apply new query.
//
//  Arguments:
//    context      in    The Validity Context (see ND::TDbiSqlContext)
//    task         in    The task of the query. Default: 0
//    data         in    Optional SQL extension to secondary query. Default: "".
//    fillOpts     in    Optional fill options (available to ND::TDbiTableRow)
//
//
//  Return:    The number of rows retrieved by query.  =0 if error.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Disconnect any previous results and apply new query to
//    associated database table

//  Program Notes:-
//  =============

//  None.

  if ( ! ND::TDbiDatabaseManager::IsActive() ) {
    fResult = 0;
    return 0;
  }
  fDetType = context.GetDetector();
  fSimType = context.GetSimFlag();

  DbiTrace( "\n\nStarting extended context query: "
                             << context.GetString()  << " task " << task
                             << " data " << data << " fillOpts " << fillOpts << "\n" <<"  ");

  ND::TDbiTimerManager::gTimerManager.RecBegin(fTableProxy.GetTableName(), sizeof(T));
  Disconnect();
  fResult = fTableProxy.Query(context.GetString(),task,data,fillOpts);
  fResult->Connect();
  ND::TDbiTimerManager::gTimerManager.RecEnd(fResult->GetNumRows());
  if ( this->ApplyAbortTest() ) {
    DbiSevere( "FATAL: " << "while applying extended context query for "
			   <<  context.c_str()<< " with task " << task
			   << " secondary query SQL: " << data
			   << "  and fill options: " << fillOpts << "  ");
    throw  ND::EQueryFailed();
  }

  DbiTrace( "\n\nCompleted extended context query: "
                             << context.GetString()  << " task " << task
                             << " data " << data << " fillOpts" << fillOpts
                             << " Found:  " << fResult->GetNumRows() << " rows\n" << "  ");

  return fResult->GetNumRows();

}
//.....................................................................

template<class T>
UInt_t TDbiResultSetHandle<T>::NewQuery(const ND::TDbiValidityRec& vrec) {
//
//
//  Purpose:  Apply new query.
//
//  Arguments:
//    vrec         in    The validity record that satisfies the query.
//
//
//  Return:    The number of rows retrieved by query.  =0 if error.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Disconnect any previous results and apply new query to
//    associated database table

//  Program Notes:-
//  =============

//  None.

  if ( ! ND::TDbiDatabaseManager::IsActive() ) {
    fResult = 0;
    return 0;
  }
  DbiTrace( "\n\nStarting ND::TDbiValidityRec query: "
                             << vrec << "\n" << "  ");

  this->SetContext(vrec);
  ND::TDbiTimerManager::gTimerManager.RecBegin(fTableProxy.GetTableName(), sizeof(T));
  Disconnect();

// Play safe and don't allow result to be used; it's validity may not
// have been trimmed by neighbouring records.
  fResult = fTableProxy.Query(vrec,kFALSE);
  fResult->Connect();
  ND::TDbiTimerManager::gTimerManager.RecEnd(fResult->GetNumRows());
  if ( this->ApplyAbortTest() ) {
    DbiSevere( "FATAL: " << "while applying validity rec query for "
			   << vrec << "  ");
    throw  ND::EQueryFailed();
  }
  DbiTrace( "\n\nCompletedND::TDbiValidityRec query: "
                             << vrec
                             << " Found:  " << fResult->GetNumRows() << " rows\n"  << "  ");
  return fResult->GetNumRows();

}
//.....................................................................

template<class T>
UInt_t TDbiResultSetHandle<T>::NewQuery(UInt_t seqNo,UInt_t dbNo) {
//
//
//  Purpose:  Apply new query.
//    seqno        in    The SEQNO of validity record that satisfies the query.
//    dbNo         in    The database number holding the validity record
//
//  Arguments:
//
//
//  Return:    The number of rows retrieved by query.  =0 if error.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Disconnect any previous results and apply new query to
//    associated database table

//  Program Notes:-
//  =============

//  None.

  if ( ! ND::TDbiDatabaseManager::IsActive() ) {
    fResult = 0;
    return 0;
  }
  DbiTrace( "\n\nStarting SeqNo query: "
                             << seqNo << "\n" << "  ");
  ND::TDbiTimerManager::gTimerManager.RecBegin(fTableProxy.GetTableName(), sizeof(T));
  Disconnect();
  fResult = fTableProxy.Query(seqNo,dbNo);
  fResult->Connect();
  ND::TDbiTimerManager::gTimerManager.RecEnd(fResult->GetNumRows());
  if ( this->ApplyAbortTest() ) {
    DbiSevere( "while applying SEQNO query for "
			   << seqNo << " on database " << dbNo << "  ");
    throw  ND::EQueryFailed();
  }
  this->SetContext(fResult->GetValidityRec());
  DbiTrace( "\n\nCompleted SeqNo query: "
                             << seqNo
                             << " Found:  " << fResult->GetNumRows() << " rows\n" << "  ");
  return fResult->GetNumRows();

}

//.....................................................................

template<class T>
Bool_t TDbiResultSetHandle<T>::ResultsFromDb() const {
//
//
//  Purpose: Return true if at least part of result comes directly from
//           database (as opposed to memory or level 2 disk cache).

  return fResult ? fResult->ResultsFromDb() : kFALSE;

}
//.....................................................................

template<class T>
void TDbiResultSetHandle<T>::SetContext(const ND::TDbiValidityRec& vrec) {
//
//
//  Purpose:  Attempt to construct context detector type and sim flag
//
//  Arguments:
//    vrec         in    ND::TDbiValidityRec from which to construct context
//
//
//  Contact:   N. West
//

//  Program Notes:-
//  =============

//  Queries that step to an adjacent validity range need a detector
//  type and simulation flag from which to construct a context. However
//  queries can be made using a ND::TDbiValidityRec and then only a ND::TVldRange
//  is known so all we can do is choose representative values from
//  its detector and simulation masks.

  const ND::TVldRange& vrng = vrec.GetVldRange();
  Int_t detMask        = vrng.GetDetectorMask();
  Int_t simMask        = vrng.GetSimMask();

  fDetType = ND::DbiDetector::kUnknown;
  if      ( detMask & ND::DbiDetector::kFar )      fDetType = ND::DbiDetector::kFar;
  else if ( detMask & ND::DbiDetector::kNear)      fDetType = ND::DbiDetector::kNear;
  else if ( detMask & ND::DbiDetector::kCalDet)    fDetType = ND::DbiDetector::kCalDet;
  else if ( detMask & ND::DbiDetector::kCalib)     fDetType = ND::DbiDetector::kCalib;
  else if ( detMask & ND::DbiDetector::kTestStand) fDetType = ND::DbiDetector::kTestStand;
  else if ( detMask & ND::DbiDetector::kMapper)    fDetType = ND::DbiDetector::kMapper;

  fSimType = ND::DbiSimFlag::kUnknown;
  if      ( simMask & ND::DbiSimFlag::kData)        fSimType = ND::DbiSimFlag::kData;
  else if ( simMask & ND::DbiSimFlag::kMC)          fSimType = ND::DbiSimFlag::kMC;
  else if ( simMask & ND::DbiSimFlag::kReroot)      fSimType = ND::DbiSimFlag::kReroot;
  else if ( simMask & ND::DbiSimFlag::kDaqFakeData) fSimType = ND::DbiSimFlag::kDaqFakeData;

}

//.....................................................................

template<class T>
ND::TDbiTableProxy& TDbiResultSetHandle<T>::TableProxy() const  {
//
//
//  Purpose:  Return associated ND::TDbiTableProxy (if any).
//
//  Contact:   N. West

  assert( ND::TDbiDatabaseManager::IsActive() );
  return fTableProxy;
}

/*    Template for New Member Function

//.....................................................................

template<class T>
TDbiResultSetHandle<T>:: {
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
}



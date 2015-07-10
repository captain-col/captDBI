// $Id: TDbiResultSetHandle.tpl,v 1.1 2011/01/18 05:49:20 finch Exp $

#include "TDbiResultKey.hxx"
#include "TDbiResultSetHandle.hxx"
#include "TDbiSqlContext.hxx"
#include "TDbiTableProxy.hxx"
#include "TDbiTimerManager.hxx"
#include "TDbiLog.hxx"
#include "MsgFormat.hxx"
#include "TVldTimeStamp.hxx"

#include <cassert>
#include <cstdlib>

ClassImpT(TDbiResultSetHandle,T)

///   Definition of static data members
namespace CP {

    template<class T>
    std::map<std::string,CP::TDbiTableProxy*>  TDbiResultSetHandle<T>::fgNameToProxy;
    
    template<class T>
    CP::TDbiTableProxy* TDbiResultSetHandle<T>::fgTableProxy = 0;
    

    ///    Definition of all member functions (static or otherwise)
    ///    *******************************************************
    ///
    ///    -  ordered: ctors, dtor, operators then in alphabetical order. 
    
    ///...................................................................

    template<class T>
    TDbiResultSetHandle<T>::TDbiResultSetHandle() :
        fAbortTest(TDbi::kDisabled),
        fTableProxy(TDbiResultSetHandle<T>::GetTableProxy()),
        fResult(0),
        fDetType(CP::DbiDetector::kUnknown),
        fSimType(CP::DbiSimFlag::kUnknown ) {
        /// 
        ///
        ///  Purpose:  Default constructor
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
        ///  o Create ResultHandle.


        T pet;
        DbiTrace(  "(TRACE) Creating TDbiResultSetHandle for " << pet.GetName()
                   << " Table Proxy at " << &fTableProxy << "  ");
    }
    ///.....................................................................

    template<class T>
    TDbiResultSetHandle<T>::TDbiResultSetHandle(const TDbiResultSetHandle& that) :
    fAbortTest(that.fAbortTest),
    fTableProxy(that.fTableProxy),
    fResult(that.fResult),
    fDetType(that.fDetType),
    fSimType(that.fSimType)
    {
        ///
        ///
        ///  Purpose:  Copy constructor


        T pet;
        DbiTrace(  "Creating copy TDbiResultSetHandle for " << pet.GetName()
                   << " Table Proxy at " << &fTableProxy << "  ");
        if ( fResult ) fResult->Connect();

    }

    ///.....................................................................

    template<class T>
    TDbiResultSetHandle<T>::TDbiResultSetHandle(const CP::TVldContext& vc,
                                                TDbi::Task task,
                                                TDbi::AbortTest abortTest,
                                                Bool_t findFullTimeWindow) :
        fAbortTest(abortTest),
        fTableProxy(TDbiResultSetHandle<T>::GetTableProxy()),
        fResult(0),
        fDetType(vc.GetDetector()),
        fSimType(vc.GetSimFlag())
    {
        ///
        ///
        ///  Purpose:  Construct and apply context specific query to default table.
        ///
        ///  Arguments:
        ///    vc           in    The Validity Context of the new query
        ///    task         in    The task of the new query
        ///    abortTest    in    Test which if failed triggers abort.
        ///    findFullTimeWindow
        ///                 in    Attempt to find full validity of query
        ///                        i.e. beyond TDbi::GetTimeGate
        ///
        ///  Return:    n/a
        ///
        ///  Contact:   N. West
        ///
        ///  Specification:-
        ///  =============
        ///
        ///  o Create ResultHandle.


        ///  Program Notes:-
        ///  =============

        ///  None.


        T pet;
        DbiTrace( "Creating TDbiResultSetHandle for "
                  << pet.GetName() << " Table Proxy at "
                  << &fTableProxy << "  ");
        NewQuery(vc, task, findFullTimeWindow);

    }

    ///.....................................................................

    template<class T>
    TDbiResultSetHandle<T>::TDbiResultSetHandle(const std::string& tableName,
                                                const CP::TVldContext& vc,
                                                TDbi::Task task,
                                                TDbi::AbortTest abortTest,
                                                Bool_t findFullTimeWindow) :
        fAbortTest(abortTest),
        fTableProxy(TDbiResultSetHandle<T>::GetTableProxy(tableName)),
        fResult(0),
        fDetType(vc.GetDetector()),
        fSimType(vc.GetSimFlag())
    {
        ///
        ///
        ///  Purpose:  Construct and apply context specific query to alternative table.
        ///
        ///  Arguments:
        ///    tableName    in    Name of table to use.
        ///    vc           in    The Validity Context of the new query
        ///    task         in    The task of the new query
        ///    abortTest    in    Test which if failed triggers abort.
        ///    findFullTimeWindow
        ///                 in    Attempt to find full validity of query
        ///                        i.e. beyond TDbi::GetTimeGate
        ///  Return:    n/a
        ///
        ///  Contact:   N. West
        ///
        ///  Specification:-
        ///  =============
        ///
        ///  o Create ResultHandle.


        ///  Program Notes:-
        ///  =============

        ///  None.


        DbiTrace( "Creating TDbiResultSetHandle for "
                  << tableName << " Table Proxy at "
                  << &fTableProxy << "  ");
        NewQuery(vc, task, findFullTimeWindow);

    }
    ///.....................................................................

    template<class T>
    TDbiResultSetHandle<T>::TDbiResultSetHandle(const std::string& tableName,
                                                const CP::TDbiSqlContext& context,
                                                const TDbi::Task& task,
                                                const std::string& data,
                                                const std::string& fillOpts,
                                                TDbi::AbortTest abortTest) :
        fAbortTest(abortTest),
        fTableProxy(TDbiResultSetHandle<T>::GetTableProxy(tableName)),
        fResult(0),
        fDetType(context.GetDetector()),
        fSimType(context.GetSimFlag())
    {
        ///
        ///
        ///  Purpose:  Apply an extended context query to alternative table.
        ///
        ///  Arguments:
        ///    tableName    in    Name of table to use.
        ///    context      in    The Validity Context (see CP::TDbiSqlContext)
        ///    task         in    The task of the query. Default: TDbi::kAnyTask,
        ///    data         in    Optional SQL extension to secondary query. Default: "".
        ///    fillOpts     in    Optional fill options (available to CP::TDbiTableRow)
        ///                       Default: "".
        ///    abortTest    in    Test which if failed triggers abort.
        ///
        ///  Return:    n/a
        ///
        ///  Contact:   N. West
        ///
        ///  Specification:-
        ///  =============
        ///
        ///  o Create ResultHandle.


        ///  Program Notes:-
        ///  =============

        ///  None.


        T pet;
        DbiTrace( "Creating TDbiResultSetHandle for "
                  << tableName << " Table Proxy at "
                  << &fTableProxy << "  "
                  << "Extended context " << context.GetString() << "  ");

        NewQuery(context,task,data,fillOpts);

    }

    ///.....................................................................

    template<class T>
    TDbiResultSetHandle<T>::TDbiResultSetHandle(const std::string& tableName,
                                                const CP::TDbiValidityRec& vrec,
                                                TDbi::AbortTest abortTest) :
        fAbortTest(abortTest),
        fTableProxy(TDbiResultSetHandle<T>::GetTableProxy(tableName)),
        fResult(0)
    {
        ///
        ///
        ///  Purpose:  Apply context specific query to alternative table.
        ///
        ///  Arguments:
        ///    tableName    in    Name of table to use.
        ///    vrec         in    The validity record that satisfies the query.
        ///    abortTest    in    Test which if failed triggers abort.
        ///
        ///  Return:    n/a
        ///
        ///  Contact:   N. West
        ///
        ///  Specification:-
        ///  =============
        ///
        ///  o Create ResultHandle.


        ///  Program Notes:-
        ///  =============

        ///  None.


        this->SetContext(vrec);
        T pet;
        DbiTrace( "Creating TDbiResultSetHandle for "
                  << tableName << " Table Proxy at "
                  << &fTableProxy << "  ");
        NewQuery(vrec);

    }
    ///.....................................................................

    template<class T>
    TDbiResultSetHandle<T>::TDbiResultSetHandle(const std::string& tableName,
                                                UInt_t seqNo,
                                                UInt_t dbNo,
                                                TDbi::AbortTest abortTest) :
        fAbortTest(abortTest),
        fTableProxy(TDbiResultSetHandle<T>::GetTableProxy(tableName)),
        fResult(0)
    {
        ///
        ///
        ///  Purpose:  Apply context specific query to alternative table.
        ///
        ///  Arguments:
        ///    tableName    in    Name of table to use.
        ///    seqno        in    The SEQNO of validity record that
	///                       satisfies the query. 
        ///    dbNo         in    The database number holding the validity
	///                       record 
        ///    abortTest    in    Test which if failed triggers abort.
        ///
        ///  Return:    n/a
        ///
        ///  Contact:   N. West
        ///
        ///  Specification:-
        ///  =============
        ///
        ///  o Create ResultHandle.


        ///  Program Notes:-
        ///  =============

        ///  None.



        T pet;
        DbiTrace( "(TRACE) Creating TDbiResultSetHandle for "
                  << tableName << " Table Proxy at "
                  << &fTableProxy << "  ");
        NewQuery(seqNo,dbNo);

    }

    ///.....................................................................

    template<class T>
    TDbiResultSetHandle<T>::~TDbiResultSetHandle() {
        ///
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
        ///  o  Disconnect any associated Results and destroy
        ///    TDbiResultSetHandle. 


        ///  Program Notes:-
        ///  =============

        ///  None.


        DbiTrace( "(TRACE) Destroying TDbiResultSetHandle" << "  ");
        Disconnect();

    }
    ///.....................................................................

    template<class T>
    Bool_t TDbiResultSetHandle<T>::ApplyAbortTest() {
        ///
        ///
        ///  Purpose:  Apply abort test and return true if must abort.

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

    ///.....................................................................

    template<class T>
    void TDbiResultSetHandle<T>::Disconnect() {
        ///
        ///
        ///  Purpose: Disconnect previous results.
        ///
        ///  Arguments: None.
        ///
        ///  Return:   None.
        ///
        ///  Contact:   N. West
        ///
        ///  Specification:-
        ///  =============
        ///
        ///  o  Disconnect any previous results.

        ///  Program Notes:-
        ///  =============

        ///  The Disconnect message is sent to CP::TDbiResultSet so that CP::TDbiCache
        ///  knows when its CP::TDbiResultSet objects are free of clients.

        if ( fResult && CP::TDbiDatabaseManager::IsActive() ) {
            fResult->Disconnect();
        }
        fResult = 0;

    }
    ///.....................................................................

    template<class T>
    const CP::TDbiResultKey* TDbiResultSetHandle<T>::GetKey() const {
        ///
        ///
        ///  Purpose:  Return associated result key or an empty one if none exists.
        ///

        return fResult ? fResult->GetKey() : CP::TDbiResultKey::GetEmptyKey();

    }
    ///.....................................................................

    template<class T>
    Int_t TDbiResultSetHandle<T>::GetResultID() const {
        ///
        ///
        ///  Purpose:  Return the ID of the current CP::TDbiResultSet.
        ///
        return fResult ? fResult->GetID() : 0;

    }
    ///.....................................................................

    template<class T>
    CP::TDbiTableProxy& TDbiResultSetHandle<T>::GetTableProxy() {
        ///
        ///
        ///  Purpose:  Private static function to find default associated
        ///            CP::TDbiTableProxy.
        ///
        ///  Arguments: None.
        ///
        ///
        ///  Return:   Associated CP::TDbiTableProxy.
        ///
        ///  Contact:   N. West
        ///
        ///  Specification:-
        ///  =============
        ///
        ///  o Ask TDbiRegistry for default Table Proxy and return it.

        ///  Program Notes:-
        ///  =============

        ///  This function creates an example Table Row object which
        ///  CP::TDbiTableProxy can copy and then use to make futher copies when
        ///  processing Result Sets.

        if ( ! fgTableProxy ) {
            T pet;
            fgTableProxy = &CP::TDbiDatabaseManager::Instance()
                .GetTableProxy(pet.GetName(),&pet);
        }
        return *fgTableProxy;
    }

    ///.....................................................................

    template<class T>
    CP::TDbiTableProxy& TDbiResultSetHandle<T>::GetTableProxy(const std::string& tableName){
        ///
        ///
        ///  Purpose:  Private static function to find an alternative
        ///            associated CP::TDbiTableProxy.
        ///
        ///  Arguments:
        ///    tableName    in    Alternative table name
        ///
        ///  Return:   Associated CP::TDbiTableProxy.
        ///
        ///  Contact:   N. West
        ///
        ///  Specification:-
        ///  =============
        ///
        ///  o Ask TDbiRegistry for alternative  Table Proxy and return it.

        ///  Program Notes:-
        ///  =============

        ///  This function creates an example Table Row object which
        ///  CP::TDbiTableProxy can copy and then use to make futher copies when
        ///  processing Result Sets.


        /// Check for request for default table.
        if ( tableName == "" ) return  TDbiResultSetHandle::GetTableProxy();

        /// See if we have seen this name before.
        std::map<std::string,CP::TDbiTableProxy*>::const_iterator itr
            = fgNameToProxy.find(tableName);
        if ( itr != fgNameToProxy.end() ) return *( (*itr).second );

        /// No, so ask the TDbiRegistry for it and save it for next time.
        T pet;
        CP::TDbiTableProxy* proxy = &CP::TDbiDatabaseManager::Instance()
            .GetTableProxy(tableName,&pet);
        fgNameToProxy[tableName] = proxy;
        return *proxy;
    }

    ///.....................................................................

    template<class T>
    UInt_t TDbiResultSetHandle<T>::GetNumRows() const {
        ///
        ///
        ///  Purpose:  Get number or rows in query.
        ///
        ///  Arguments: None.
        ///
        ///
        ///  Return:    Number of rows found in query, = 0 if no or failed query.
        ///
        ///  Contact:   N. West
        ///
        ///  Specification:-
        ///  =============
        ///
        ///  o Return number of rows found in query, = 0 if no or failed query.


        ///  Program Notes:-
        ///  =============

        ///  None.

        return ( fResult && CP::TDbiDatabaseManager::IsActive() )
            ? fResult->GetNumRows() : 0;

    }
    ///.....................................................................

    template<class T>
    const T* TDbiResultSetHandle<T>::GetRow(UInt_t rowNum) const {
        ///
        ///
        ///  Purpose: Return pointer to concrete Table Row object at given row.
        ///
        ///  Arguments:
        ///    rowNum       in    Required row number
        ///
        ///  Return:        Pointer to concrete Table Row object at given row,
        ///                 =0 if none
        ///
        ///  Contact:   N. West
        ///
        ///  Specification:-
        ///  =============
        ///
        ///  o Return pointer to concrete Table Row object at given row or
        ///    0 if none.

        ///  Program Notes:-
        ///  =============

        /// None.

        return ( fResult && CP::TDbiDatabaseManager::IsActive() ) ?
            dynamic_cast<const T*>(fResult->GetTableRow(rowNum))
            : 0;
    }
    ///.....................................................................

    template<class T>
    const T* TDbiResultSetHandle<T>::GetRowByIndex(UInt_t index) const {
        ///
        ///
        ///  Purpose: Return pointer to concrete Table Row object at given index.
        ///
        ///  Arguments:
        ///    rowNum       in    Required index number.
        ///
        ///  Return:        Pointer to concrete Table Row object at given index,
        ///                 =0 if none
        ///
        ///  Contact:   N. West
        ///
        ///  Specification:-
        ///  =============
        ///
        ///  o Return pointer to concrete Table Row object at given index or
        ///    0 if none.

        ///  Program Notes:-
        ///  =============

        ///  None.

        return ( fResult && CP::TDbiDatabaseManager::IsActive() ) ?
            dynamic_cast<const T*>(fResult->GetTableRowByIndex(index))
            : 0;
    }

    ///.....................................................................

    template<class T>
    const CP::TDbiValidityRec* TDbiResultSetHandle<T>::GetValidityRec(
        const CP::TDbiTableRow* row) const {
        ///
        ///
        ///  Purpose:  Return associated CP::TDbiValidityRec (if any).
        ///
        ///  Contact:   N. West

        return ( fResult && CP::TDbiDatabaseManager::IsActive() )
            ? &(fResult->GetValidityRec(row)) : 0 ;

    }

    ///.....................................................................

    template<class T>
    UInt_t TDbiResultSetHandle<T>::NextQuery(Bool_t forwards) {
        ///
        ///
        ///  Purpose:  Get next validity range for previous query.
        ///
        ///  Arguments:
        ///    forwards     in    true to step direction forwards (default)
        ///
        ///  Contact:   N. West
        ///
        ///  Specification:-
        ///  =============
        ///
        ///  o Take the validity range of the current query and step across
        ///    the boundary to the adjacent range.


        /// Door stops.
        static CP::TVldTimeStamp startOfTime(0,0);
        static CP::TVldTimeStamp endOfTime(0x7FFFFFFF,0);

        if ( ! fResult ) return 0;

        DbiTrace( "\n\nStarting next query: direction "
                  << ( forwards ?  "forwards" : "backwards" ) << "\n" << "  ");

        const CP::TDbiValidityRec& vrec = fResult->GetValidityRec();
        const CP::TVldRange& vrnge      = vrec.GetVldRange();

        /// If we are heading towards the final boundary, just return the same query.
        if (   forwards && vrnge.GetTimeEnd()   == endOfTime )   return fResult->GetNumRows();
        if ( ! forwards && vrnge.GetTimeStart() == startOfTime ) return fResult->GetNumRows();

        /// Step across boundary and construct new context.
        /// The end time limit is exclusive, so stepping to the end
        /// does cross the boundary
        time_t ts = forwards ? vrnge.GetTimeEnd().GetSec()
            : vrnge.GetTimeStart().GetSec() - 1;
        CP::TVldContext vc(fDetType,fSimType,CP::TVldTimeStamp(ts,0));

        return this->NewQuery(vc,vrec.GetTask(), true);

    }


    ///.....................................................................

    template<class T>
    UInt_t TDbiResultSetHandle<T>::NewQuery(CP::TVldContext vc,
                                            TDbi::Task task,
                                            Bool_t findFullTimeWindow) {
        ///
        ///
        ///  Purpose:  Apply new query.
        ///
        ///  Arguments:
        ///    vc           in    The Validity Context of the new query
        ///    task         in    The task of the new query
        ///    findFullTimeWindow
        ///                 in    Attempt to find full validity of query
        ///                        i.e. beyond TDbi::GetTimeGate
        ///
        ///
        ///  Return:    The number of rows retrieved by query.  =0 if error.
        ///
        ///  Contact:   N. West
        ///
        ///  Specification:-
        ///  =============
        ///
        ///  o Disconnect any previous results and apply new query to
        ///    associated database table

        ///  Program Notes:-
        ///  =============

        ///  None.

        if ( ! CP::TDbiDatabaseManager::IsActive() ) {
            fResult = 0;
            return 0;
        }
        fDetType = vc.GetDetector();
        fSimType = vc.GetSimFlag();

        DbiTrace( "\n\nStarting context query: "
                  << vc  << " task " << task << "\n" << "  ");

        CP::TDbiTimerManager::gTimerManager.RecBegin(fTableProxy.GetTableName(), sizeof(T));
        Disconnect();
        fResult = fTableProxy.Query(vc,task,findFullTimeWindow);
        fResult->Connect();
        CP::TDbiTimerManager::gTimerManager.RecEnd(fResult->GetNumRows());

        if ( this->ApplyAbortTest() ) {
            DbiSevere( "FATAL: "
                       << "while applying validity context query for "
                       << vc.AsString() << " with task " << task << "  ");
            throw  CP::EQueryFailed();
        }
        DbiTrace( "\nCompleted context query: "
                  << vc  << " task " << task
                  << " Found:  " << fResult->GetNumRows() << " rows\n" << "  ");
        return fResult->GetNumRows();

    }
    ///.....................................................................

    template<class T>
    UInt_t TDbiResultSetHandle<T>::NewQuery(const CP::TDbiSqlContext& context,
                                            const TDbi::Task& task,
                                            const std::string& data,
                                            const std::string& fillOpts) {
        ///
        ///
        ///  Purpose:  Apply new query.
        ///
        ///  Arguments:
        ///    context      in    The Validity Context (see CP::TDbiSqlContext)
        ///    task         in    The task of the query. Default: 0
        ///    data         in    Optional SQL extension to secondary query. Default: "".
        ///    fillOpts     in    Optional fill options (available to CP::TDbiTableRow)
        ///
        ///
        ///  Return:    The number of rows retrieved by query.  =0 if error.
        ///
        ///  Contact:   N. West
        ///
        ///  Specification:-
        ///  =============
        ///
        ///  o Disconnect any previous results and apply new query to
        ///    associated database table

        ///  Program Notes:-
        ///  =============

        ///  None.

        if ( ! CP::TDbiDatabaseManager::IsActive() ) {
            fResult = 0;
            return 0;
        }
        fDetType = context.GetDetector();
        fSimType = context.GetSimFlag();

        DbiTrace( "\n\nStarting extended context query: "
                  << context.GetString()  << " task " << task
                  << " data " << data << " fillOpts " << fillOpts << "\n" <<"  ");

        CP::TDbiTimerManager::gTimerManager.RecBegin(fTableProxy.GetTableName(), sizeof(T));
        Disconnect();
        fResult = fTableProxy.Query(context.GetString(),task,data,fillOpts);
        fResult->Connect();
        CP::TDbiTimerManager::gTimerManager.RecEnd(fResult->GetNumRows());
        if ( this->ApplyAbortTest() ) {
            DbiSevere( "FATAL: " << "while applying extended context query for "
                       <<  context.c_str()<< " with task " << task
                       << " secondary query SQL: " << data
                       << "  and fill options: " << fillOpts << "  ");
            throw  CP::EQueryFailed();
        }

        DbiTrace( "\n\nCompleted extended context query: "
                  << context.GetString()  << " task " << task
                  << " data " << data << " fillOpts" << fillOpts
                  << " Found:  " << fResult->GetNumRows() << " rows\n" << "  ");

        return fResult->GetNumRows();

    }
    ///.....................................................................

    template<class T>
    UInt_t TDbiResultSetHandle<T>::NewQuery(const CP::TDbiValidityRec& vrec) {
        ///
        ///
        ///  Purpose:  Apply new query.
        ///
        ///  Arguments:
        ///    vrec         in    The validity record that satisfies the query.
        ///
        ///
        ///  Return:    The number of rows retrieved by query.  =0 if error.
        ///
        ///  Contact:   N. West
        ///
        ///  Specification:-
        ///  =============
        ///
        ///  o Disconnect any previous results and apply new query to
        ///    associated database table

        ///  Program Notes:-
        ///  =============

        ///  None.

        if ( ! CP::TDbiDatabaseManager::IsActive() ) {
            fResult = 0;
            return 0;
        }
        DbiTrace( "\n\nStarting CP::TDbiValidityRec query: "
                  << vrec << "\n" << "  ");

        this->SetContext(vrec);
        CP::TDbiTimerManager::gTimerManager.RecBegin(fTableProxy.GetTableName(), sizeof(T));
        Disconnect();

        /// Play safe and don't allow result to be used; it's validity may not
        /// have been trimmed by neighbouring records.
        fResult = fTableProxy.Query(vrec,kFALSE);
        fResult->Connect();
        CP::TDbiTimerManager::gTimerManager.RecEnd(fResult->GetNumRows());
        if ( this->ApplyAbortTest() ) {
            DbiSevere( "FATAL: " << "while applying validity rec query for "
                       << vrec << "  ");
            throw  CP::EQueryFailed();
        }
        DbiTrace( "\n\nCompletedCP::TDbiValidityRec query: "
                  << vrec
                  << " Found:  " << fResult->GetNumRows() << " rows\n"  << "  ");
        return fResult->GetNumRows();

    }
    ///.....................................................................

    template<class T>
    UInt_t TDbiResultSetHandle<T>::NewQuery(UInt_t seqNo,UInt_t dbNo) {
        ///
        ///
        ///  Purpose:  Apply new query.
        ///    seqno        in    The SEQNO of validity record that satisfies the query.
        ///    dbNo         in    The database number holding the validity record
        ///
        ///  Arguments:
        ///
        ///
        ///  Return:    The number of rows retrieved by query.  =0 if error.
        ///
        ///  Contact:   N. West
        ///
        ///  Specification:-
        ///  =============
        ///
        ///  o Disconnect any previous results and apply new query to
        ///    associated database table

        ///  Program Notes:-
        ///  =============

        ///  None.

        if ( ! CP::TDbiDatabaseManager::IsActive() ) {
            fResult = 0;
            return 0;
        }
        DbiTrace( "\n\nStarting SeqNo query: "
                  << seqNo << "\n" << "  ");
        CP::TDbiTimerManager::gTimerManager.RecBegin(fTableProxy.GetTableName(), sizeof(T));
        Disconnect();
        fResult = fTableProxy.Query(seqNo,dbNo);
        fResult->Connect();
        CP::TDbiTimerManager::gTimerManager.RecEnd(fResult->GetNumRows());
        if ( this->ApplyAbortTest() ) {
            DbiSevere( "while applying SEQNO query for "
                       << seqNo << " on database " << dbNo << "  ");
            throw  CP::EQueryFailed();
        }
        this->SetContext(fResult->GetValidityRec());
        DbiTrace( "\n\nCompleted SeqNo query: "
                  << seqNo
                  << " Found:  " << fResult->GetNumRows() << " rows\n" << "  ");
        return fResult->GetNumRows();

    }

    ///.....................................................................

    template<class T>
    Bool_t TDbiResultSetHandle<T>::ResultsFromDb() const {
        ///
        ///
        ///  Purpose: Return true if at least part of result comes directly from
        ///           database (as opposed to memory or level 2 disk cache).

        return fResult ? fResult->ResultsFromDb() : kFALSE;

    }
    ///.....................................................................

    template<class T>
    void TDbiResultSetHandle<T>::SetContext(const CP::TDbiValidityRec& vrec) {
        ///
        ///
        ///  Purpose:  Attempt to construct context detector type and sim flag
        ///
        ///  Arguments:
        ///    vrec         in    CP::TDbiValidityRec from which to construct context
        ///
        ///
        ///  Contact:   N. West
        ///

        ///  Program Notes:-
        ///  =============

        ///  Queries that step to an adjacent validity range need a detector
        ///  type and simulation flag from which to construct a context. However
        ///  queries can be made using a CP::TDbiValidityRec and then only a CP::TVldRange
        ///  is known so all we can do is choose representative values from
        ///  its detector and simulation masks.

        const CP::TVldRange& vrng = vrec.GetVldRange();
        Int_t detMask        = vrng.GetDetectorMask();
        Int_t simMask        = vrng.GetSimMask();

        fDetType
            = CP::DbiDetector::Detector_t(detMask&CP::DbiDetector::FullMask());

        fSimType = CP::DbiSimFlag::kUnknown;
        if ( simMask & CP::DbiSimFlag::kData) {
            fSimType = CP::DbiSimFlag::kData;
        }	 
        else if ( simMask & CP::DbiSimFlag::kMC) {
            fSimType = CP::DbiSimFlag::kMC;
        }
        else if ( simMask & CP::DbiSimFlag::kDaqFakeData) {
            fSimType = CP::DbiSimFlag::kDaqFakeData;
        }

    }

    ///.....................................................................

    template<class T>
    CP::TDbiTableProxy& TDbiResultSetHandle<T>::TableProxy() const  {
        ///
        ///
        ///  Purpose:  Return associated CP::TDbiTableProxy (if any).
        ///
        ///  Contact:   N. West

        assert( CP::TDbiDatabaseManager::IsActive() );
        return fTableProxy;
    }

};



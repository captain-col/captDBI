
//////////////////////////////////////////////////////////////////////////
////////////////////////////     ROOT API     ////////////////////////////
//////////////////////////////////////////////////////////////////////////


#include <memory>
#include <cassert>

#include "TCollection.h"
#include "TList.h"
#include "TSQLColumnInfo.h"
#include "TSQLStatement.h"
#include "TSQLServer.h"
#include "TSQLTableInfo.h"
#include "TString.h"

#include "DbiDetector.hxx"
#include "DbiSimFlag.hxx"
#include "TDbiDBProxy.hxx"
#include "TDbiCascader.hxx"
#include "TDbiFieldType.hxx"
#include "TDbiInRowStream.hxx"
#include "TDbiServices.hxx"
#include "TDbiString.hxx"
#include "TDbiStatement.hxx"
#include "TDbiTableMetaData.hxx"
#include "TDbiTimerManager.hxx"
#include "TDbiValidityRec.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>
#include "TVldContext.hxx"

#include "UtilString.hxx"

ClassImp(CP::TDbiDBProxy)

//   Typedefs
//   ********

//   Definition of static data members
//   *********************************


//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.


//.....................................................................

CP::TDbiDBProxy::TDbiDBProxy(CP::TDbiCascader& cascader,
                             const std::string& tableName,
                             const CP::TDbiTableMetaData* metaData,
                             const CP::TDbiTableMetaData* metaValid,
                             const CP::TDbiTableProxy* tableProxy) :
    fCascader(cascader),
    fMetaData(metaData),
    fMetaValid(metaValid),
    fTableName(tableName),
    fTableProxy(tableProxy) {
//
//
//  Purpose:  Constructor
//
//  Arguments:
//  cascader   in   Reference to one and only cascader
//  tableName  in   Table name.
//  metaData   in   Meta data for main table.
//  metaValid  in   Meta data for validity.
//  tableProxy in   Owning CP::TDbiTableProxy.
//


}

//.....................................................................

CP::TDbiDBProxy::~TDbiDBProxy() {
//
//
//  Purpose: Destructor


    DbiTrace("Destroying CP::TDbiDBProxy "
             << fTableName << " at " << this
             << "  ");

}
//.....................................................................

void CP::TDbiDBProxy::FindTimeBoundaries(const CP::TVldContext& vc,
                                         const TDbi::Task& task,
                                         UInt_t dbNo,
                                         const CP::TDbiValidityRec& lowestPriorityVrec,
                                         Bool_t resolveByCreationDate,
                                         CP::TVldTimeStamp& start,
                                         CP::TVldTimeStamp& end) const {
//
//
//  Purpose: Find next time boundaries beyond standard time gate.
//
//  Arguments:
//    vc                    in    The Validity Context for the query.
//    task                  in    The task of the query.
//    dbNo                  in    Database number in cascade (starting at 0).
//    lowestPriorityVrec    in    Lowest priority VLD in the time gate
//    resolveByCreationDate in  The method for resolving VLDs:
//                                 if true use CREATIONDATE (MINOS scheme)
//                                 if false use EPOCH,TIMESTART,INSERTDATE (T2K scheme)
//    start                 out   Lower time boundary or CP::TVldTimeStamp(0,0) if none
//    end                   out   Upper time boundary or CP::TVldTimeStamp(0x7FFFFFFF,0) if none
//
//  Specification:-
//  =============
//
//  o Find the next time boundary (either TIMESTART or TIMEEND)
//    outside the current time gate that is potentionally of higher priority.
//
//  Explanation:-
//  ===========
//
//
//  In oder to keep the number of VLDs to a manageable level when doing a
//  Standard Context query, a time gate around the context is imposed and
//  any VLD rejected if it does not overlap the query. However, this
//  implies that the maximum validity of the query result set is also
//  limited by the time gate and hence the interface will again have to
//  query the database when a context outside the time gate is
//  requested. This function attempts to open up the validity of the
//  result set by looking for the nearest start or end of a VLD outside
//  the time gate that is of higher priority. In MINOS, priority is just
//  CREATIONDATE, so the function just looks for VLDs with a higher
//  CREATIONDATE.  For T2K the priority scheme is more complicated:
//  EPOCH,TIMESTART,INSERTDATE and it is not possible to construct a SQL
//  query that only considers higher priority VLDs so the function makes a
//  simplification and only rejects VLDs with a lower EPOCH number.  The
//  consequence of this is that it may accept VLDs that are of a lower
//  priority than the current one and hence the result set validity does
//  not get opened up to its full extent.  All this means is that the
//  interface will have to make another query earlier that it need do but
//  at least it will not miss a VLD boundary and should still keep the current
//  results longer than it would have if this function had not been
//  called.

    DbiVerbose("FindTimeBoundaries for table " <<  fTableName
               << " context " << vc
               << " task " << task
               << " Lowest priority VLD creation date: " <<  lowestPriorityVrec.GetCreationDate()
               << " and EPOCH: " << lowestPriorityVrec.GetEpoch()
               << " resolve by: " << (resolveByCreationDate ? "CREATIONDATE" : "EPOCH")
               << " database " << dbNo << "  ");

//  Set the limits wide open
    start = CP::TVldTimeStamp(0,0);
    end   = CP::TVldTimeStamp(0x7FFFFFFF,0);

//  Construct a Time Gate on the current date.

    const CP::TVldTimeStamp curVTS = vc.GetTimeStamp();
    Int_t timeGate = TDbi::GetTimeGate(this->GetTableName());
    time_t vcSec = curVTS.GetSec() - timeGate;
    CP::TVldTimeStamp startGate(vcSec,0);
    vcSec += 2*timeGate;
    CP::TVldTimeStamp endGate(vcSec,0);
    std::string startGateString(TDbi::MakeDateTimeString(startGate));
    std::string endGateString(TDbi::MakeDateTimeString(endGate));

// Extract information for CP::TVldContext.

    CP::DbiDetector::Detector_t    detType(vc.GetDetector());
    CP::DbiSimFlag::SimFlag_t       simFlg(vc.GetSimFlag());

// Use an std::auto_ptr to manage ownership of CP::TDbiStatement and TSQLStatement
    std::auto_ptr<CP::TDbiStatement> stmtDb(fCascader.CreateStatement(dbNo));

    for (int i_limit =1; i_limit <= 4; ++i_limit) {
        CP::TDbiString sql("select ");
        if (i_limit == 1) sql  << "min(TIMESTART) from " << fTableName
                                   << "VLD where TIMESTART > '" << endGateString << "' ";
        if (i_limit == 2) sql  << "min(TIMEEND) from " << fTableName
                                   << "VLD where TIMEEND > '" << endGateString  << "' ";
        if (i_limit == 3) sql  << "max(TIMESTART) from " << fTableName
                                   << "VLD where TIMESTART < '" << startGateString << "' ";
        if (i_limit == 4) sql  << "max(TIMEEND) from " << fTableName
                                   << "VLD where TIMEEND < '" << startGateString  << "' ";
        sql << " and DetectorMask & " << static_cast<unsigned int>(detType)
            << " and SimMask & " << static_cast<unsigned int>(simFlg)
            << " and  Task = " << task;
        if (resolveByCreationDate) {
            sql << " and CREATIONDATE >= '" << TDbi::MakeDateTimeString(lowestPriorityVrec.GetCreationDate()) << "'";
        }
        else {
            sql << " and EPOCH >= " << lowestPriorityVrec.GetEpoch();
        }
        DbiVerbose("  FindTimeBoundaries query no. " << i_limit
                   << " SQL:" <<sql.c_str() << "  ");

        std::auto_ptr<TSQLStatement> stmt(stmtDb->ExecuteQuery(sql.c_str()));
        stmtDb->PrintExceptions(CP::TDbiLog::DebugLevel);

//  If the query returns data, convert to a time stamp and trim the limits
        TString date;
        if (! stmt.get() || ! stmt->NextResultRow() || stmt->IsNull(0)) {
            continue;
        }
        date = stmt->GetString(0);
        if (date.IsNull()) {
            continue;
        }
        CP::TVldTimeStamp ts(TDbi::MakeTimeStamp(date.Data()));
        DbiVerbose("  FindTimeBoundaries query result: " << ts << "  ");
        if (i_limit <= 2 && ts < end) {
            end   = ts;
        }
        if (i_limit >= 3 && ts > start) {
            start = ts;
        }

    }

    DbiVerbose("FindTimeBoundaries for table " <<  fTableName
               << " found " << start << " .. " << end << "  ");

}
//.....................................................................

UInt_t CP::TDbiDBProxy::GetNumDb() const {
//
//
//  Purpose:  Return the number of databases in the cascade.

    return fCascader.GetNumDb();

}
//.....................................................................

Bool_t CP::TDbiDBProxy::HasEpoch() const {
    return fMetaValid->HasEpoch();
}
//.....................................................................

CP::TDbiInRowStream*  CP::TDbiDBProxy::QueryAllValidities(UInt_t dbNo,UInt_t seqNo) const {
//
//
//  Purpose:  Apply all validities query to database..
//
//  Arguments:
//    dbNo         in    Database number in cascade (starting at 0).
//    seqNo        in    Just this SEQNO if >0 or all if 0 [default: 0]
//
//  Return:    New CP::TDbiResultSet object.
//             NB  Caller is responsible for deleting..
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Ask for every row in associated validity range table and return
//    query results qualifying selection by fSqlCondition if defined.

//  Program Notes:-
//  =============

//  This function is provided to support database maintenance
//  rather than standard CP::TDbi related I/O


// Generate SQL for validity table.

    CP::TDbiString sql;

    sql << "select * from " << fTableName << "VLD";
    if (fSqlCondition != "") {
        sql << " where " << fSqlCondition;
    }
    if (seqNo > 0) {
        if (fSqlCondition == "") {
            sql << " where ";
        }
        else {
            sql << " and ";
        }
        sql << "seqno = " << seqNo;
    }
    sql  << ";" << '\0';

    DbiVerbose("Database: " << dbNo
               << " query: " << sql.GetString() << "  ");

//  Apply query and return result..

    CP::TDbiStatement* stmtDb = fCascader.CreateStatement(dbNo);
    return new CP::TDbiInRowStream(stmtDb,sql,fMetaValid,fTableProxy,dbNo);

}



//.....................................................................

CP::TDbiInRowStream*  CP::TDbiDBProxy::QuerySeqNo(UInt_t seqNo, UInt_t dbNo) const {
//
//
//  Purpose:  Apply sequence query to database..
//
//  Arguments:
//    seqNo        in    The sequence number for the query.
//    dbNo         in    Database number in cascade (starting at 0).
//
//  Return:    New CP::TDbiResultSet object.
//             NB  Caller is responsible for deleting..
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Apply sequence query to table and return query results

//  Program Notes:-
//  =============

//  None..

// Generate SQL.

    CP::TDbiTimerManager::gTimerManager.RecMainQuery();
    CP::TDbiString sql;
    sql << "select * from " << fTableName << " where "
        << "    SEQNO= " << seqNo;

    if (CP::TDbiServices::OrderContextQuery()) {
        sql << " order by ROW_COUNTER";
    }

    DbiVerbose("Database: " << dbNo
               << " SeqNo query: " << sql.c_str() << "  ");

//  Apply query and return result..

    CP::TDbiStatement* stmtDb = fCascader.CreateStatement(dbNo);
    return new CP::TDbiInRowStream(stmtDb,sql,fMetaData,fTableProxy,dbNo);

}

//.....................................................................

CP::TDbiInRowStream*  CP::TDbiDBProxy::QuerySeqNos(SeqList_t& seqNos,
                                                   UInt_t dbNo,
                                                   const std::string& sqlData,
                                                   const std::string& fillOpts) const {
//
//
//  Purpose:  Apply query for a list of sequence numbers to database..
//
//  Arguments:
//    seqNos       in    The vector of sequence numbers for the query.
//                       Should be in acsending order, see Program Notes
//    dbNo         in    Database number in cascade (starting at 0).
//    sqlData      in    Optional SQL extension to secondary query.
//    fillOpts     in    Optional fill options (available to CP::TDbiTableRow
//                       objects when filling.
//
//  Return:    New CP::TDbiResultSet object.
//             NB  Caller is responsible for deleting..
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Apply sequence query to table and return query results

//  Program Notes:-
//  =============

//  Where possible the SQL query is kept to a minimm by using
//  `BETWEEN' comparison to bracket ranges of numbers.

// Generate SQL.

    if (seqNos.size() == 0) {
        return 0;
    }

    CP::TDbiTimerManager::gTimerManager.RecMainQuery();
    CP::TDbiString sql;
    sql << "select * from " << fTableName << " where ";

    if (sqlData != "") {
        sql << "( ";
    }
    Bool_t first = kTRUE;
    SeqList_t::const_iterator itr1 = seqNos.begin();

    while (itr1 != seqNos.end()) {
        UInt_t seq1 = *itr1;
        UInt_t seq2 = seq1;
        SeqList_t::const_iterator itr2 = itr1;
        while (itr2 != seqNos.end() && seq2 == *itr2) {
            ++itr2;
            ++seq2;
        }
        if (first) {
            first = kFALSE;
        }
        else {
            sql << "or ";
        }
        if (seq2 > seq1 + 1) {
            sql << "SEQNO between " << seq1 << " and " << seq2-1 << ' ';
            itr1 = itr2;
        }
        else {
            sql << "SEQNO = " << seq1 << ' ';
            ++itr1;
        }
    }

    if (sqlData != "") {
        sql << ")  and " << "(" << sqlData << ")" << " ";
    }

    sql << "order by SEQNO";

    if (CP::TDbiServices::OrderContextQuery()) {
        sql << ",ROW_COUNTER";
    }

    DbiVerbose("Database: " << dbNo
               << " SeqNos query: " << sql.c_str() << "  ");

//  Apply query and return result..

    CP::TDbiStatement* stmtDb = fCascader.CreateStatement(dbNo);
    return new CP::TDbiInRowStream(stmtDb,sql,fMetaData,fTableProxy,dbNo,fillOpts);

}

//.....................................................................

CP::TDbiInRowStream*  CP::TDbiDBProxy::QueryValidity(const CP::TVldContext& vc,
                                                     const TDbi::Task& task,
                                                     UInt_t dbNo) const {
//
//
//  Purpose:  Apply validity query to database..
//
//  Arguments:
//    vc           in    The Validity Context for the query.
//    task         in    The task of the query.
//    dbNo         in    Database number in cascade (starting at 0).
//
//  Return:    New CP::TDbiResultSet object.
//             NB  Caller is responsible for deleting..
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Apply query to associated validity range table and return query
//    results qualifying selection by fSqlCondition if defined.

//  Program Notes:-
//  =============

//  Rather than search over the full table this function places
//  a time gate round the context time and then trims any validity
//  range returned to this gate.  See FindTimeBoundaries to get a
//  more accurate validity range.

//  Construct a search window on the current date.

    const CP::TVldTimeStamp curVTS = vc.GetTimeStamp();
    Int_t timeGate = TDbi::GetTimeGate(this->GetTableName());
    time_t vcSec = curVTS.GetSec() - timeGate;
    CP::TVldTimeStamp startGate(vcSec,0);
    vcSec += 2*timeGate;
    CP::TVldTimeStamp endGate(vcSec,0);

// Extract information for CP::TVldContext.

    std::string startGateString(TDbi::MakeDateTimeString(startGate));
    std::string endGateString(TDbi::MakeDateTimeString(endGate));
    CP::DbiDetector::Detector_t    detType(vc.GetDetector());
    CP::DbiSimFlag::SimFlag_t       simFlg(vc.GetSimFlag());

// Generate SQL for context.

    CP::TDbiString context;
    context << "    TimeStart <= '" << endGateString << "' "
            << "and TimeEnd    > '" << startGateString << "' "
            << "and DetectorMask & " << static_cast<unsigned int>(detType)
            << " and SimMask & " << static_cast<unsigned int>(simFlg);

//  Apply query and return result..

    return this->QueryValidity(context.GetString(),task,dbNo);

}
//.....................................................................

CP::TDbiInRowStream*  CP::TDbiDBProxy::QueryValidity(const std::string& context,
                                                     const TDbi::Task& task,
                                                     UInt_t dbNo) const {
//
//
//  Purpose:  Apply validity query to database..
//
//  Arguments:
//    context      in    The Validity Context (see CP::TDbiSqlContext)
//    task         in    The task of the query.
//    dbNo         in    Database number in cascade (starting at 0).
//
//  Return:    New CP::TDbiResultSet object.
//             NB  Caller is responsible for deleting..
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Apply query to associated validity range table and return query
//    results qualifying selection by fSqlCondition if defined.


// Generate SQL for validity table.

    CP::TDbiString sql;

// In the MINOS  scheme queries are  ordered by creation date (the later the better)
// but if the table has an EPOCH column then the T2K scheme is used (EPOCH,TIMESTART,INSERTDATE)

    std::string orderByName("CREATIONDATE desc");
    if (this->HasEpoch()) {
        orderByName = "EPOCH desc,TIMESTART desc,INSERTDATE desc";
    }
    sql << "select * from " << fTableName << "VLD"
        << " where " ;
    if (fSqlCondition != ""
       ) {
        sql << fSqlCondition << " and ";
    }
    sql << context;
    if (task != TDbi::kAnyTask
       ) sql << " and  Task = " << task
                 << " order by " << orderByName << ";" << '\0';

    DbiVerbose("Database: " << dbNo
               << " query: " << sql.c_str() << "  ");

//  Apply query and return result..

    CP::TDbiStatement* stmtDb = fCascader.CreateStatement(dbNo);
    return new CP::TDbiInRowStream(stmtDb,sql,fMetaValid,fTableProxy,dbNo);

}
//.....................................................................

CP::TDbiInRowStream*  CP::TDbiDBProxy::QueryValidity(UInt_t seqNo,
                                                     UInt_t dbNo) const {
//
//
//  Purpose:  Apply validity query to database..
//
//  Arguments:
//    seqNo        in    The SEQNO of the validity rec.
//
//  Return:    New CP::TDbiResultSet object.
//             NB  Caller is responsible for deleting..
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Apply query to associated validity range table and return query
//    results qualifying selection by fSqlCondition if defined.


// Generate SQL for validity table.

    CP::TDbiString sql;
    sql << "select * from " << fTableName << "VLD where ";
    if (fSqlCondition != "") {
        sql << fSqlCondition << " and ";
    }
    sql << "SEQNO = " << seqNo << ";";

    DbiVerbose("Database: " << dbNo
               << " SEQNO query: " << sql.c_str() << "  ");

//  Apply query and return result..

    CP::TDbiStatement* stmtDb = fCascader.CreateStatement(dbNo);
    return new CP::TDbiInRowStream(stmtDb,sql,fMetaValid,fTableProxy,dbNo);

}

//.....................................................................

Bool_t CP::TDbiDBProxy::RemoveSeqNo(UInt_t seqNo,
                                    UInt_t dbNo) const {
//
//
//  Purpose:  Remove sequence number in main and auxiliary tables.
//
//  Arguments:
//    seqNo        in    The sequence number to be removed.
//    dbNo         in    Database number in cascade (starting at 0).
//
//  Return:    kTRUE if output successful,otherwise kFALSE.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Remove sequence number in main and auxiliary tables.

//  Program Notes:-
//  =============

//  None.


// Generate SQL to remove SeqNo in main table.
    CP::TDbiString sql;
    sql  << "delete from  " << fTableName
         << " where SEQNO = " << seqNo << ";"
         << '\0';

    DbiVerbose("Database: " << dbNo
               << " RemoveSeqNo SQL: " << sql.c_str() << "  ");

//  Apply query.
    std::auto_ptr<CP::TDbiStatement> stmtDb(fCascader.CreateStatement(dbNo));
    if (! stmtDb.get()) {
        return false;
    }
    if (! stmtDb->ExecuteUpdate(sql.c_str()) || stmtDb->PrintExceptions()) {
        DbiSevere("SQL: " << sql.c_str()
                  << " Failed. " << "  ");
        return false;
    }

// Generate SQL to remove SeqNo in validity table.
    sql.GetString().erase();
    sql << "delete from  " << fTableName
        << "VLD where SEQNO = " << seqNo << ";"
        << '\0';

    DbiVerbose("Database: " << dbNo
               << " RemoveSeqNo SQL: " << sql.c_str() << "  ");

//  Apply query.
    if (! stmtDb->ExecuteUpdate(sql.c_str()) ||  stmtDb->PrintExceptions()) {
        DbiSevere("SQL: " << sql.c_str()
                  << " Failed. " << "  ");
        return false;
    }

    return true;

}

//.....................................................................

Bool_t CP::TDbiDBProxy::ReplaceInsertDate(const CP::TVldTimeStamp& ts,
                                          UInt_t SeqNo,
                                          UInt_t dbNo) const {
//
//
//  Purpose:  Replace insertion date for row in auxiliary table.
//
//  Arguments:
//    ts           in    Time stamp for new insertion date.
//    SeqNo        in    The sequence number of the row to be replaced.
//    dbNo         in    Database number in cascade (starting at 0).
//
//  Return:    kTRUE if output successful,otherwise kFALSE.
//
//  Contact:   N. West


// Generate SQL.
    CP::TDbiString sql;
    sql << "update  " << fTableName
        << "VLD set INSERTDATE = \'" << ts.AsString("s")
        << "\' where SEQNO = " << SeqNo << ";"
        << '\0';

    DbiVerbose("Database: " << dbNo
               << " ReplaceInsertDate SQL: "
               << sql.c_str() << "  ");

//  Apply query.
    std::auto_ptr<CP::TDbiStatement> stmtDb(fCascader.CreateStatement(dbNo));
    if (! stmtDb.get()) {
        return false;
    }
    if (! stmtDb->ExecuteUpdate(sql.c_str()) || stmtDb->PrintExceptions()) {
        DbiSevere("SQL: " << sql.c_str()
                  << " Failed. " << "  ");
        return false;
    }

    return true;

}
//.....................................................................

Bool_t CP::TDbiDBProxy::ReplaceSeqNo(UInt_t oldSeqNo,
                                     UInt_t newSeqNo,
                                     UInt_t dbNo) const {
//
//
//  Purpose:  Replace sequence number in main and auxiliary tables.
//
//  Arguments:
//    oldSeqNo     in    The old sequence number.
//    nwSeqNo      in    The new sequence number.
//    dbNo         in    Database number in cascade (starting at 0).
//
//  Return:    kTRUE if output successful,otherwise kFALSE.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Replace sequence number in main and auxiliary tables.

//  Program Notes:-
//  =============

//  None.

    if (! fCascader.GetConnection(dbNo)) {
        DbiWarn("Cannot renumber " << oldSeqNo
                << " no connection to cascade entry " << dbNo << "  ");
        return false;
    }

// Generate SQL to replace SeqNo in validity table.
    CP::TDbiString sql;
    sql << "update  " << fTableName
        << "VLD set SEQNO = " << newSeqNo
        << " where SEQNO = " << oldSeqNo << ";"
        << '\0';

    DbiVerbose("Database: " << dbNo
               << " ReplaceSeqNo SQL: " << sql.c_str() << "  ");

//  Apply query.
    std::auto_ptr<CP::TDbiStatement> stmtDb(fCascader.CreateStatement(dbNo));
    if (! stmtDb.get()) {
        return false;
    }
    if (! stmtDb->ExecuteUpdate(sql.c_str()) || stmtDb->PrintExceptions()) {
        DbiSevere("SQL: " << sql.c_str()
                  << " Failed. " << "  ");
        return false;
    }

// Generate SQL to replace SeqNo in main table.
    sql.GetString().erase();
    sql  << "update  " << fTableName
         << " set SEQNO = " << newSeqNo
         << " where SEQNO = " << oldSeqNo << ";"
         << '\0';

    DbiVerbose("Database: " << dbNo
               << " ReplaceSeqNo SQL: " << sql.c_str() << "  ");

//  Apply query.
    if (! stmtDb->ExecuteUpdate(sql.c_str()) || stmtDb->PrintExceptions()) {
        DbiSevere("SQL: " << sql.c_str()
                  << " Failed. " << "  ");
        return false;
    }

    return true;

}

//.....................................................................

void  CP::TDbiDBProxy::StoreMetaData(CP::TDbiTableMetaData& metaData) const {
//
//
//  Purpose:  Store table meta data.
//
//  Arguments:
//    metaData   in    Empty CP::TDbiTableMetaData object apart from table name.



    const char* tableName = metaData.TableName().c_str();
    DbiVerbose("Get meta-data for table: " << tableName << "  ");

//  Check each Db in turn until table found and store table meta data.

    for (UInt_t dbNo = 0; dbNo < fCascader.GetNumDb(); dbNo++) {
        CP::TDbiConnection* connection = fCascader.GetConnection(dbNo);
        TSQLServer* server = connection->GetServer();
        if (! server) {
            continue;
        }
        connection->Connect();
        TSQLTableInfo* meta = server->GetTableInfo(tableName);
        if (! meta) {
            connection->DisConnect();
            continue;
        }
        DbiVerbose("Meta-data query succeeded on cascade entry " << dbNo << "  ");

        // Clear out any existing data, although there should not be any.
        metaData.Clear();

        const TList* cols =  meta->GetColumns();
        TIter colItr(cols);
        int col = 0;
        while (TSQLColumnInfo* colInfo = dynamic_cast<TSQLColumnInfo*>(colItr.Next())) {

            ++col;
            std::string name(colInfo->GetName());
            name = CP::UtilString::ToUpper(name);
            metaData.SetColName(name,col);

            CP::TDbiFieldType fldType(colInfo->GetSQLType(),
                                      colInfo->GetLength(),
                                      colInfo->GetTypeName());

            // For now continue to check for unsigned (even though not supported)
            if (!colInfo->IsSigned()) {
                fldType.SetUnsigned();
            }
            metaData.SetColFieldType(fldType,col);

            metaData.SetColIsNullable(col,colInfo->IsNullable());

            DbiVerbose("Column "         << col << " " << name
                       << " SQL type "      << colInfo->GetSQLType()
                       << " SQL type name " << colInfo->GetTypeName()
                       << " DBI type "      << fldType.AsString()
                       << " data size: "    << fldType.GetSize()
                       << " col size: "     << colInfo->GetLength() << "  ");

        }
        delete meta;
        connection->DisConnect();
        return;
    }
}

//.....................................................................

Bool_t CP::TDbiDBProxy::TableExists(Int_t selectDbNo) const {
//
//
//  Purpose:  Return true if table exists on selected cascade entry
//            or any entry if selectDbNo=-1 (default)


    return fCascader.TableExists(fTableName,selectDbNo);

}



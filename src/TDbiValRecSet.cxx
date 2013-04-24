// $Id: TDbiValRecSet.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $

#include "TDbiDBProxy.hxx"
#include "TDbiResultSetNonAgg.hxx"
#include "TDbiInRowStream.hxx"
#include "TDbiTableProxy.hxx"
#include "TDbiDatabaseManager.hxx"
#include "TDbiValidityRec.hxx"
#include "TDbiValRecSet.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>

ClassImp(CP::TDbiValRecSet)


//   Definition of static data members
//   *********************************


//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

CP::TDbiValRecSet::TDbiValRecSet(const std::string& tableName,UInt_t dbNo,UInt_t seqNo) :
    fDbNo(dbNo),
    fResult(0) {
//
//
//  Purpose:  Constructor
//
//  Arguments:
//    tableName    in    Table Name whose CP::TDbiValidityRecs are required.
//    dbNo         in    Database number in the cascade.
//    seqNo        in    Just this SEQNO if >0 or all if 0 [default: 0]
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o  For the required table in the required database, load every
//     CP::TDbiValidityRec into CP::TDbiResultSet.


//  Program Notes:-
//  =============

//  None.


    DbiTrace("Creating CP::TDbiValRecSet" << "  ");

// Get Db proxy for the table.
    CP::TDbiValidityRec pet;
    const CP::TDbiDBProxy& proxy = CP::TDbiDatabaseManager::Instance()
                                   .GetTableProxy(tableName,&pet)
                                   .GetDBProxy();

// Collect up all validity records.
    CP::TDbiInRowStream* rset = proxy.QueryAllValidities(dbNo,seqNo);
    fResult = new CP::TDbiResultSetNonAgg(rset,&pet,0,kFALSE);
    delete rset;


}


//.....................................................................

CP::TDbiValRecSet::~TDbiValRecSet() {
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
//  o  Destroy RowStream.


//  Program Notes:-
//  =============

//  None.


    DbiTrace("Destroying CP::TDbiValRecSet" << "  ");
    delete fResult;

}

//.....................................................................

UInt_t CP::TDbiValRecSet::GetNumRows() const {
//
//
//  Purpose:  Return the number of rows.
//
//  Arguments: None.
//
//  Return:
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Return the number of rows.

//  Program Notes:-
//  =============

//  None.

    return fResult ? fResult->GetNumRows() : 0;
}

//.....................................................................

const std::string CP::TDbiValRecSet::GetTableName() const {
//
//
//  Purpose:  Return the table name.
//
//  Arguments: None.
//
//  Return:
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Return the table name.

//  Program Notes:-
//  =============

//  None.

    return fResult ? fResult->TableName() : "Unknown";
}

//.....................................................................

const CP::TDbiValidityRec* CP::TDbiValRecSet::GetTableRow(UInt_t rowNum) const {
//
//
//  Purpose:  Return CP::TDbiValidityRec at supplied row number.
//
//  Arguments:
//    rowNum      in    Row number whose entry is required or 0 if none.
//
//  Return:
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Return CP::TDbiValidityRec at supplied row number.

//  Program Notes:-
//  =============

//  None.

    return fResult ?
           dynamic_cast<const CP::TDbiValidityRec*>(fResult->GetTableRow(rowNum))
           : 0;
}

//.....................................................................

const CP::TDbiValidityRec* CP::TDbiValRecSet::GetTableRowBySeqNo(UInt_t seqNo
                                                                ) const {
//
//
//  Purpose:  Return TDbiValidityRec.hxxaving supplied SeqNo.
//
//  Arguments:
//    seqNo        in    Sequence number of required entry or 0 if none.
//
//  Return:
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Return CP::TDbiValidityRec at supplied row number.

//  Program Notes:-
//  =============

//  None.

    UInt_t numRows = GetNumRows();
    if (numRows == 0) {
        return 0;
    }

// Create look-up table if not yet built.

    if (fSeqNoToRec.size() == 0) {
        for (UInt_t irow = 0; irow < numRows; ++irow) {
            const CP::TDbiValidityRec* vrec = GetTableRow(irow);
            fSeqNoToRec[vrec->GetSeqNo()] = vrec;
        }
    }

    std::map<UInt_t,const CP::TDbiValidityRec*>::const_iterator itr = fSeqNoToRec.find(seqNo);
    return (itr == fSeqNoToRec.end()) ? 0 : itr->second;

}



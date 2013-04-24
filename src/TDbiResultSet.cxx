// $Id: TDbiResultSet.cxx,v 1.2 2011/06/08 09:49:18 finch Exp $

#include <sstream>

#include "TDbiBinaryFile.hxx"
#include "TDbiResultKey.hxx"
#include "TDbiResultSet.hxx"
#include "TDbiInRowStream.hxx"
#include "TDbiServices.hxx"
#include "TDbiTableRow.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>

ClassImp(CP::TDbiResultSet)

//   Definition of static data members
//   *********************************

Int_t CP::TDbiResultSet::fgLastID(0);

//  Global functions
//  *****************

//.....................................................................

CP::TDbiBinaryFile& CP::operator<<(CP::TDbiBinaryFile& bf,
                                   const CP::TDbiResultSet& res) {

// Writing is a const operation, but uses a non-const method, so cast away const.

    CP::TDbiResultSet& res_tmp = const_cast< CP::TDbiResultSet&>(res);
    res_tmp.Streamer(bf);
    return bf;
}

//.....................................................................

CP::TDbiBinaryFile& CP::operator>>(CP::TDbiBinaryFile& bf,
                                   CP::TDbiResultSet& res) {

    res.Streamer(bf);
    return bf;
}

// Definition of member functions (alphabetical order)
// ***************************************************


//.....................................................................

CP::TDbiResultSet::TDbiResultSet(CP::TDbiInRowStream* resultSet,
                                 const CP::TDbiValidityRec* vrec,
                                 const std::string& sqlQualifiers) :
    fID(++fgLastID),
    fCanReuse(kTRUE),
    fEffVRec(0),
    fKey(0),
    fResultsFromDb(kFALSE),
    fNumClients(0),
    fTableName("Unknown"),
    fSqlQualifiers(sqlQualifiers) {
//
//
//  Purpose:  Default constructor
//
//  Arguments:
//      resultSet     in   Pointer CP::TDbiInRowStream from query. May be null.
//      vrec          in   Pointer to validity record from query.
//                         May be null
//      sqlQualifier  in   Extended Context sql qualifiers
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create Result from CP::TDbiInRowStream generated by query.


//  Program Notes:-
//  =============

// None.


    DbiTrace("Creating CP::TDbiResultSet" << "  ");

    if (vrec) {
        fEffVRec = *vrec;
    }
    if (resultSet) {
        fTableName = resultSet->TableNameTc();
    }

}

//.....................................................................

CP::TDbiResultSet::~TDbiResultSet() {
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
//  o  Destroy Result.


//  Program Notes:-
//  =============

//  None.


    DbiTrace("Destroying CP::TDbiResultSet." << "  ");

    if (fNumClients) DbiWarn("Warning: Destroying CP::TDbiResultSet with " << fNumClients
                                 << " clients " << "  ");

    delete fKey;
    fKey = 0;
    fIndexKeys.clear();

}
//.....................................................................

void CP::TDbiResultSet::BuildLookUpTable() const {
//
//
//  Purpose:  Build look-up Natural Index -> TableRow.
//
//  Contact:   N. West
//
//  Program Notes:-
//  =============
//
//  This member function assumes that the sub-class can support
//  the GetTableRow(...) and GetNumRows methods so take care if
//  called in the sub-class ctor.

//  Extended Context serach can produce duplicates.
    Bool_t duplicatesOK = this->IsExtendedContext();

    DbiVerbose("Building look-uptable. Allow duplicates: "
               << duplicatesOK << "  ");

    for (Int_t rowNo = this->GetNumRows()-1;
         rowNo >= 0;
         --rowNo) {
        const CP::TDbiTableRow* row  = this->GetTableRow(rowNo);
        UInt_t index            = row->GetIndex(rowNo);
//  Ensure we use this class's GetTableRowByIndex, the method is
//  virtual but if the subclass has called this method then it must
//  be the right one to use. [Actually CP::TDbiResultSetAgg overrides
//  GetTableRowByIndex, but only to make building lazy].
        const CP::TDbiTableRow* row2 = this->CP::TDbiResultSet::GetTableRowByIndex(index);

        DbiVerbose("Look-up. Row no " << rowNo
                   << " index " << index
                   << " row,row2 " << (void*) row << "," << (void*) row2 << "  ");

        if (row2 != 0 && row2 != row && ! duplicatesOK) {
            std::ostringstream msg;
            msg << "Duplicated row natural index: " << index
                << " Found at row " <<  rowNo
                << " of table " <<  this->TableName()
                << ":-\n     index of agg " <<  row->GetAggregateNo();
            if (row->GetOwner()) {
                msg << "(SEQNO " << row->GetOwner()->GetValidityRec(row).GetSeqNo() << ")";
            }
            msg << " matches agg " <<  row2->GetAggregateNo();
            if (row2->GetOwner()) {
                msg << "(SEQNO " << row2->GetOwner()->GetValidityRec(row2).GetSeqNo() << ")";
            }
            DbiSevere(msg.str() << "  ");
        }

        else {
            fIndexKeys[index] = row;
        }
    }

}

//.....................................................................

Bool_t CP::TDbiResultSet::CanDelete(const CP::TDbiResultSet* res) {
//
//
//  Purpose:  See if suitable for deleting.
//
//  Arguments:
//    res          in    A CP::TDbiResultSet that is currently used.
//                       Optional: default = 0.

//  If the query would be satisfied apart from the date, then
//  assume we have moved out of the validity window, never
//  to return!

    if (res
        && this->CanReuse()
        && this->GetValidityRec().HasExpired(res->GetValidityRec())
       ) {
        this->SetCanReuse(kFALSE);
    }

    return ! this->GetNumClients() && ! this->CanReuse();
}

//.....................................................................

void CP::TDbiResultSet::CaptureExceptionLog(UInt_t startFrom) {
//
//
//  Purpose: Capture Exception Log from latest entries in Global Exception Log.

    fExceptionLog.Clear();
    CP::TDbiExceptionLog::GetGELog().Copy(fExceptionLog,startFrom);

}

//.....................................................................

void CP::TDbiResultSet::GenerateKey() {
//
//
//  Purpose: Create key if set not empty and not already done.

    if (fKey || this->GetNumRows() == 0) {
        return;
    }
    fKey = this->CreateKey();

}

//.....................................................................

const CP::TDbiResultKey* CP::TDbiResultSet::GetKey() const {

//  Purpose:  Get the associated CP::TDbiResultKey, or an empty one if none exists.

    return fKey ? fKey : CP::TDbiResultKey::GetEmptyKey();

}

//.....................................................................

const CP::TDbiTableRow* CP::TDbiResultSet::GetTableRowByIndex(UInt_t index) const {
//
//
//  Purpose:  Return row corresponding to Natural Index (or 0 if none)
//
//  Contact:   N. West
//

// Use Find rather than operator[] to avoid creating entry
// if index missing
    IndexToRow_t::const_iterator idx = fIndexKeys.find(index);
    return (idx == fIndexKeys.end()) ? 0 : (*idx).second;

}

//.....................................................................

Bool_t CP::TDbiResultSet::Satisfies(const CP::TVldContext& vc,
                                    const TDbi::Task& task) {
//
//
//  Purpose:  Check to see if this Result satisfies specific context query.
//
//  Arguments:
//    vc           in    Context of query
//    task         in    Task of query
//
//  Return:    kTRUE if can satisfy query.
//
//  Contact:   N. West
//
//  Program Notes:-
//  =============

//  If it cannot satisfy the query in a way that suggests
//  that its unlikely that it will satisfy future queries
//  then mark as cannot reuse so that it becomes eligible
//  for purging.

// Extended Context queries cannot satisfy specific context queries.

    Bool_t isExtendedContext = this->IsExtendedContext();
    Bool_t canReuse          = this->CanReuse();
    Bool_t isCompatible      = this->GetValidityRec().IsCompatible(vc,task);
    Bool_t hasExpired        = this->GetValidityRec().HasExpired(vc,task);
    UInt_t numClients        = this->GetNumClients();
    DbiDebug("    Checking result with CP::TDbiValidityRec:- \n      " << this->GetValidityRec()
             << "  With extended context: " << isExtendedContext
             << " CanReuse: " << canReuse
             << " Is Compatible: " << isCompatible
             << " HasExpired: " <<  hasExpired
             << " number of clients: " << numClients
             << "  ");

    if (isExtendedContext) {
        return kFALSE;
    }

    if (canReuse && isCompatible) {
        return kTRUE;
    }

//  If the query would be satisfied apart from the date, then
//  assume we have moved out of the validity window, never
//  to return!

    if (canReuse && hasExpired && numClients == 0)  {
        DbiDebug("    Marking result as not reusable" << "  ");
        this-> SetCanReuse(kFALSE);
    }

    return kFALSE;

}
//.....................................................................

void CP::TDbiResultSet::Streamer(CP::TDbiBinaryFile& file) {
//
//
//  Purpose:  I/O to binary file
//
//  Program Notes:-
//  =============

//  Don't store fIndexKeys; it will be rebuilt on input by the subclass.

    if (file.IsReading()) {
        DbiDebug("    Restoring CP::TDbiResultSet ..." << "  ");
        file >> fCanReuse;
        fEffVRec.Streamer(file);
        DbiVerbose("    Restored " << fEffVRec << "  ");
        fResultsFromDb = kFALSE;
        fNumClients    = 0;
        file >> fTableName;
        DbiVerbose("    Restored string " << fTableName << "  ");
    }
    else if (file.IsWriting()) {
        DbiDebug("    Saving CP::TDbiResultSet ..." << "  ");
        file << fCanReuse;
        DbiVerbose("    Saving " << fEffVRec << "  ");
        fEffVRec.Streamer(file);
        DbiVerbose("    Saving string " << fTableName << "  ");
        file << fTableName;
    }
}

/*    Template for New Member Function

//.....................................................................

CP::TDbiResultSet:: {
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



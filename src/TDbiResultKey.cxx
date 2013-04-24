// $Id: TDbiResultKey.cxx,v 1.2 2012/06/14 10:55:22 finch Exp $

#include <iostream>
#include <map>
#include <sstream>

#include "TDbiResultKey.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>

ClassImp(CP::TDbiResultKey)


//   Definition of static data members
//   *********************************


CP::TDbiResultKey CP::TDbiResultKey::fgEmptyKey;

//   Global Functions
//   ****************

std::ostream& CP::operator<<(std::ostream& os, const CP::TDbiResultKey& key) {
    os << key.AsString() << std::endl;
    return os;
}


// Definition of member functions (alphabetical order)
// ***************************************************

//.....................................................................

CP::TDbiResultKey::TDbiResultKey(const CP::TDbiResultKey* that /* =0 */) :
    fNumVRecKeys(0) {
    //
    //
    //  Purpose:  Default constructor
    //


    DbiTrace("Creating CP::TDbiResultKey" << "  ");
    if (that) {
        *this = *that;
    }
}

///.....................................................................

CP::TDbiResultKey::TDbiResultKey(std::string tableName,
                                 std::string rowName,
                                 UInt_t seqno,
                                 CP::TVldTimeStamp ts) :
    fTableName(tableName),
    fRowName(rowName),
    fNumVRecKeys(0) {
    //
    //
    //  Purpose:  Standard constructor
    //
    //  Contact:   N. West
    //
    //  Specification:-
    //  =============
    //
    //  o  Create CP::TDbiResultKey.


    DbiTrace("Creating CP::TDbiResultKey" << "  ");

    this->AddVRecKey(seqno,ts);
}

//.....................................................................

CP::TDbiResultKey::~TDbiResultKey() {
    //
    //
    //  Purpose: Destructor
    //
    //  Contact:   N. West
    //
    //  Specification:-
    //  =============
    //
    //  o  Destroy CP::TDbiResultKey


    DbiTrace("Destroying CP::TDbiResultKey" << "  ");

}

//.....................................................................

void CP::TDbiResultKey::AddVRecKey(UInt_t seqno, CP::TVldTimeStamp ts) {
    //
    //
    //  Purpose:  Add a CP::TDbiValidityRec key.
    //

    fVRecKeys.push_back(VRecKey(seqno,ts));
    ++fNumVRecKeys;

}

//.....................................................................

std::string CP::TDbiResultKey::AsString() const {
    //
    //
    //  Purpose:  Return a string that summarises this key giving:-
    //            1)  The table and row names.
    //            2)  The number of validity records (aggregates)
    //            3)  The range of SEQNOs
    //            4)  The range of CREATIONDATEs.

    std::ostringstream os;
    os << "Table:" << fTableName << " row:" << fRowName;
    if (fVRecKeys.empty()) {
        os << " No vrecs";
    }
    else {
        os << ".  " << fNumVRecKeys << " vrec";
        if (fNumVRecKeys > 1) {
            os << "s (seqno min..max;creationdate min..max):";
        }
        else {
            os << " (seqno;creationdate):";
        }
        os << " ";
        std::list<VRecKey>::const_iterator itr    = fVRecKeys.begin();
        std::list<VRecKey>::const_iterator itrEnd = fVRecKeys.end();
        UInt_t seqnoMin    = itr->SeqNo;
        UInt_t seqnoMax    = seqnoMin;
        CP::TVldTimeStamp tsMin = itr->CreationDate;
        CP::TVldTimeStamp tsMax = tsMin;
        ++itr;
        while (itr != itrEnd) {
            UInt_t       seqno = itr->SeqNo;
            CP::TVldTimeStamp ts    = itr->CreationDate;
            if (seqno < seqnoMin) {
                seqnoMin = seqno;
            }
            if (seqno > seqnoMax) {
                seqnoMax = seqno;
            }
            if (ts < tsMin) {
                tsMin    = ts;
            }
            if (ts > tsMax) {
                tsMax    = ts;
            }
            ++itr;
        }
        os << seqnoMin;
        if (seqnoMin < seqnoMax) {
            os << ".." << seqnoMax;
        }
        os << ";" << tsMin.AsString("s");
        if (tsMin < tsMax) {
            os << ".." <<  tsMax.AsString("s");
        }
    }
    return std::string(os.str());

}
//.....................................................................

Float_t CP::TDbiResultKey::Compare(const CP::TDbiResultKey* that) const {
    //
    //
    //  Purpose:  Compare 2 CP::TDbiResultKeys
    //
    //  Return:    = -2. Table names don't match.
    //             = -1. Table names match but row names don't
    //                   i.e. contain different CP::TDbiTableRow sub-classes.
    //             >= f  Table and row names match and fraction f of the
    //                   SEQNOs have same creation date.
    //                   So f = 1.  = perfect match.

    //  Program Notes:-
    //  =============

    //  None.

    // Check in table and row names.
    if (fTableName != that->fTableName) {
        return -2.;
    }
    if (fRowName   != that->fRowName) {
        return -1.;
    }

    // Pick the key with the most entries and compare the other to it.

    DbiDebug("Comparing " << *this << " to "
             << *that << "  ");

    const CP::TDbiResultKey* keyBig   = this;
    const CP::TDbiResultKey* keySmall = that;
    if (that->GetNumVrecs() > this->GetNumVrecs()) {
        keyBig   = that;
        keySmall = this;
    }
    int numVrecs = keyBig->GetNumVrecs();
    if (numVrecs == 0) {
        return 0.;
    }

    std::map<UInt_t,CP::TVldTimeStamp> seqnoToCreationDate;
    std::list<CP::TDbiResultKey::VRecKey>::const_iterator itrEnd = keyBig->fVRecKeys.end();
    for (std::list<CP::TDbiResultKey::VRecKey>::const_iterator itr = keyBig->fVRecKeys.begin();
         itr != itrEnd;
         ++itr) {
        seqnoToCreationDate[itr->SeqNo] = itr->CreationDate;
    }
    float match = 0;
    itrEnd = keySmall->fVRecKeys.end();
    for (std::list<CP::TDbiResultKey::VRecKey>::const_iterator itr = keySmall->fVRecKeys.begin();
         itr != itrEnd;
         ++itr) {
        DbiDebug("Comparing seqno " << itr->SeqNo << " with creation date " << itr->CreationDate
                 << " to " <<  seqnoToCreationDate[itr->SeqNo] << "  ");
        if (seqnoToCreationDate[itr->SeqNo] == itr->CreationDate) {
            ++match;
        }
    }
    DbiDebug("Match results: " << match << " out of " << numVrecs << "  ");

    return match/numVrecs;

}

//.....................................................................

std::string CP::TDbiResultKey::GetTableRowName() const {
    //
    //
    //  Purpose:  Return TableName::RowName

    std::ostringstream os;
    os << fTableName << "::" << fRowName;
    return os.str();

}
/*    Template for New Member Function

//.....................................................................

CP::TDbiResultKey:: {
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



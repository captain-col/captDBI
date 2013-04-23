// $Id: TDbiResultKey.cxx,v 1.2 2012/06/14 10:55:22 finch Exp $

#include <iostream>
#include <map>
#include <sstream>

#include "TDbiResultKey.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;

ClassImp(ND::TDbiResultKey)


//   Definition of static data members
//   *********************************


ND::TDbiResultKey ND::TDbiResultKey::fgEmptyKey;

//   Global Functions
//   ****************

std::ostream& operator<<(std::ostream& os, const ND::TDbiResultKey& key) {
    os << key.AsString() << endl;
    return os;
}


// Definition of member functions (alphabetical order)
// ***************************************************

//.....................................................................

ND::TDbiResultKey::TDbiResultKey(const ND::TDbiResultKey* that /* =0 */) :
fNumVRecKeys(0)
{
    //
    //
    //  Purpose:  Default constructor
    //
    
    
    DbiTrace( "Creating ND::TDbiResultKey" << "  ");
    if ( that ) *this = *that;
}

///.....................................................................

ND::TDbiResultKey::TDbiResultKey(std::string tableName,
    std::string rowName,
    UInt_t seqno,
    ND::TVldTimeStamp ts) :
fTableName(tableName),
fRowName(rowName),
fNumVRecKeys(0)
{
    //
    //
    //  Purpose:  Standard constructor
    //
    //  Contact:   N. West
    //
    //  Specification:-
    //  =============
    //
    //  o  Create ND::TDbiResultKey.
    
    
    DbiTrace( "Creating ND::TDbiResultKey" << "  ");
    
    this->AddVRecKey(seqno,ts);
}

//.....................................................................

ND::TDbiResultKey::~TDbiResultKey() {
    //
    //
    //  Purpose: Destructor
    //
    //  Contact:   N. West
    //
    //  Specification:-
    //  =============
    //
    //  o  Destroy ND::TDbiResultKey
    
    
    DbiTrace( "Destroying ND::TDbiResultKey" << "  ");
    
}

//.....................................................................

void ND::TDbiResultKey::AddVRecKey(UInt_t seqno, ND::TVldTimeStamp ts) {
    //
    //
    //  Purpose:  Add a ND::TDbiValidityRec key.
    //
    
    fVRecKeys.push_back(VRecKey(seqno,ts));
    ++fNumVRecKeys;
    
}

//.....................................................................

std::string ND::TDbiResultKey::AsString() const {
    //
    //
    //  Purpose:  Return a string that summarises this key giving:-
    //            1)  The table and row names.
    //            2)  The number of validity records (aggregates)
    //            3)  The range of SEQNOs
    //            4)  The range of CREATIONDATEs.
    
    std::ostringstream os;
    os << "Table:" << fTableName << " row:" << fRowName;
    if ( fVRecKeys.empty() ) os << " No vrecs";
    else {
    	os << ".  " << fNumVRecKeys << " vrec";
    	if ( fNumVRecKeys > 1 ) os << "s (seqno min..max;creationdate min..max):";
    	else                    os << " (seqno;creationdate):";
    	os << " ";
    	std::list<VRecKey>::const_iterator itr    = fVRecKeys.begin();
    	std::list<VRecKey>::const_iterator itrEnd = fVRecKeys.end();
    	UInt_t seqnoMin    = itr->SeqNo;
    	UInt_t seqnoMax    = seqnoMin;
    	ND::TVldTimeStamp tsMin = itr->CreationDate;
    	ND::TVldTimeStamp tsMax = tsMin;
    	++itr;
    	while ( itr != itrEnd ) {
    	    UInt_t       seqno = itr->SeqNo;
    	    ND::TVldTimeStamp ts    = itr->CreationDate;
    	    if ( seqno < seqnoMin ) seqnoMin = seqno;
    	    if ( seqno > seqnoMax ) seqnoMax = seqno;
    	    if (    ts < tsMin    ) tsMin    = ts;
    	    if (    ts > tsMax    ) tsMax    = ts;
    	    ++itr;
    	}
    	os << seqnoMin;
    	if ( seqnoMin < seqnoMax ) os << ".." << seqnoMax;
    	os << ";" << tsMin.AsString("s");
    	if ( tsMin < tsMax ) os << ".." <<  tsMax.AsString("s");
    }
    return std::string(os.str());
    
}
//.....................................................................

Float_t ND::TDbiResultKey::Compare(const ND::TDbiResultKey* that) const {
    //
    //
    //  Purpose:  Compare 2 ND::TDbiResultKeys
    //
    //  Return:    = -2. Table names don't match.
    //             = -1. Table names match but row names don't
    //                   i.e. contain different ND::TDbiTableRow sub-classes.
    //             >= f  Table and row names match and fraction f of the
    //                   SEQNOs have same creation date.
    //                   So f = 1.  = perfect match.
    
    //  Program Notes:-
    //  =============
    
    //  None.
    
    // Check in table and row names.
    if ( fTableName != that->fTableName ) return -2.;
    if ( fRowName   != that->fRowName   ) return -1.;
    
    // Pick the key with the most entries and compare the other to it.
    
    DbiDebug( "Comparing " << *this << " to "
    	<< *that << "  ");
    
    const ND::TDbiResultKey* keyBig   = this;
    const ND::TDbiResultKey* keySmall = that;
    if ( that->GetNumVrecs() > this->GetNumVrecs() ) {
    	keyBig   = that;
    	keySmall = this;
    }
    int numVrecs = keyBig->GetNumVrecs();
    if ( numVrecs == 0 ) return 0.;
    
    std::map<UInt_t,ND::TVldTimeStamp> seqnoToCreationDate;
    std::list<ND::TDbiResultKey::VRecKey>::const_iterator itrEnd = keyBig->fVRecKeys.end();
    for (  std::list<ND::TDbiResultKey::VRecKey>::const_iterator itr = keyBig->fVRecKeys.begin();
    	itr != itrEnd;
    	++itr ) seqnoToCreationDate[itr->SeqNo] = itr->CreationDate;
    float match = 0;
    itrEnd = keySmall->fVRecKeys.end();
    for (  std::list<ND::TDbiResultKey::VRecKey>::const_iterator itr = keySmall->fVRecKeys.begin();
    	itr != itrEnd;
    	++itr ) {
    DbiDebug( "Comparing seqno " << itr->SeqNo << " with creation date " << itr->CreationDate
    	<< " to " <<  seqnoToCreationDate[itr->SeqNo] << "  ");
    if ( seqnoToCreationDate[itr->SeqNo] == itr->CreationDate ) ++match;
    	}
    	DbiDebug( "Match results: " << match << " out of " << numVrecs << "  ");
    	
    	return match/numVrecs;
    	
}

//.....................................................................

std::string ND::TDbiResultKey::GetTableRowName() const {
    //
    //
    //  Purpose:  Return TableName::RowName
    
    std::ostringstream os;
    os << fTableName << "::" << fRowName;
    return os.str();
    
}
/*    Template for New Member Function

//.....................................................................

ND::TDbiResultKey:: {
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



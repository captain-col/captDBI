// $Id: TDbiString.cxx,v 1.2 2012/06/14 10:55:22 finch Exp $

#include <sstream>

#include "TDbiString.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>

ClassImp(CP::TDbiString)


//   Definition of static data members
//   *********************************


// Definition of member functions (alphabetical order)
// ***************************************************

//.....................................................................

CP::TDbiString::TDbiString() {
//
//
//  Purpose:  Default constructor
//
//  Arguments:  None.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o  Create CP::TDbiString.


//  Program Notes:-
//  =============

//  None.


    DbiTrace("Creating CP::TDbiString" << "  ");

}

//.....................................................................

CP::TDbiString::TDbiString(const Char_t* str) :
    fString(str) {
//
//
//  Purpose:  Constructor from Char_t*.
//
//  Arguments:
//     str        in  Initial string.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o  Create CP::TDbiString.


//  Program Notes:-
//  =============

//  None.


    DbiTrace("Creating CP::TDbiString" << "  ");

}

//.....................................................................

CP::TDbiString::TDbiString(const std::string& str) :
    fString(str) {
//
//
//  Purpose:  Constructor from string.
//
//  Arguments:
//     str        in  Initial string.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o  Create CP::TDbiString.


//  Program Notes:-
//  =============

//  None.


    DbiTrace("Creating CP::TDbiString" << "  ");

}

//.....................................................................

CP::TDbiString::~TDbiString() {
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
//  o  Destroy CP::TDbiString


//  Program Notes:-
//  =============

//  None.


    DbiTrace("Destroying CP::TDbiString" << "  ");

}

#define OUT(t,v)               \
    std::ostringstream out;         \
    out << v;                  \
    fString.append(out.str()); \
    return *this;


CP::TDbiString& CP::TDbiString::operator<<(Int_t data)   {
    OUT(Int_t,data)
}
CP::TDbiString& CP::TDbiString::operator<<(UInt_t data)  {
    OUT(UInt_t,data)
}
CP::TDbiString& CP::TDbiString::operator<<(Float_t data) {
    OUT(Float_t,data)
}
CP::TDbiString& CP::TDbiString::operator<<(Char_t data) {
    fString.append(1,data); return *this;
}
CP::TDbiString& CP::TDbiString::operator<<(const Char_t* data) {
    fString.append(data); return *this;
}
CP::TDbiString& CP::TDbiString::operator<<(const std::string& data) {
    fString.append(data); return *this;
}



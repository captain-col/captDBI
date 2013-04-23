// $Id: TDbiString.cxx,v 1.2 2012/06/14 10:55:22 finch Exp $

#include <sstream>

#include "TDbiString.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;
using std::ostringstream;

ClassImp(ND::TDbiString)


//   Definition of static data members
//   *********************************


// Definition of member functions (alphabetical order)
// ***************************************************

//.....................................................................

ND::TDbiString::TDbiString()
{
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
//  o  Create ND::TDbiString.


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Creating ND::TDbiString" << "  ");

}

//.....................................................................

ND::TDbiString::TDbiString(const Char_t* str) :
fString(str)
{
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
//  o  Create ND::TDbiString.


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Creating ND::TDbiString" << "  ");

}

//.....................................................................

ND::TDbiString::TDbiString(const std::string& str) :
fString(str)
{
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
//  o  Create ND::TDbiString.


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Creating ND::TDbiString" << "  ");

}

//.....................................................................

ND::TDbiString::~TDbiString() {
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
//  o  Destroy ND::TDbiString


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Destroying ND::TDbiString" << "  ");

}

#define OUT(t,v)               \
    ostringstream out;         \
    out << v;                  \
    fString.append(out.str()); \
    return *this;


ND::TDbiString& ND::TDbiString::operator<<(Int_t data)   { OUT(Int_t,data) }
ND::TDbiString& ND::TDbiString::operator<<(UInt_t data)  { OUT(UInt_t,data) }
ND::TDbiString& ND::TDbiString::operator<<(Float_t data) { OUT(Float_t,data) }
ND::TDbiString& ND::TDbiString::operator<<(Char_t data) {
                             fString.append(1,data); return *this; }
ND::TDbiString& ND::TDbiString::operator<<(const Char_t* data) {
                               fString.append(data); return *this; }
ND::TDbiString& ND::TDbiString::operator<<(const std::string& data) {
                               fString.append(data); return *this; }



///////////////////////////////////////////////////////////////////////
// $Id: TDbiOutRowStream.cxx,v 1.2 2012/06/14 10:55:22 finch Exp $
//
// ND::TDbiOutRowStream

#include <sstream>

#include "TDbiFieldType.hxx"
#include "TDbiOutRowStream.hxx"
#include "TDbiTableMetaData.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;
using std::ostringstream;
using std::setprecision;
#include "UtilString.hxx"
#include "TVldTimeStamp.hxx"

ClassImp(ND::TDbiOutRowStream)

#define OUT(t,v)                         \
  if ( ! StoreDefaultIfInvalid(t) ) {    \
    ostringstream out;                   \
    out << setprecision(16)<< v;         \
    Store(out.str());                    \
  }                                      \

// If writing unsigned dat as signed, convert bit pattern to signed,
// extending sign bit if necessary.
// For BIGINT (size 8) make an exception.  It's used only as
// an alternative to unsigned int so can written without conversion.
#define OUT2(t,v)                         \
  const ND::TDbiFieldType& fType = this->ColFieldType(this->CurColNum());             \
  if ( fType.IsSigned() && fType.GetSize() != 8 ) {                              \
    Int_t v_signed = (Int_t) v;                                                  \
    if ( fType.GetType() == TDbi::kTiny  && v & 0x80   ) v_signed |= 0xffffff00;  \
    if ( fType.GetType() == TDbi::kShort && v & 0x8000 ) v_signed |= 0xffff0000;  \
    OUT(TDbi::kInt,v_signed); }                                                   \
  else {                                                                         \
    OUT(t,v);                                                                    \
  }                                                                              \

//   Definition of static data members
//   *********************************


//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

ND::TDbiOutRowStream::TDbiOutRowStream(const ND::TDbiTableMetaData* metaData) :
ND::TDbiRowStream(metaData),
fBadData(kFALSE)
{
//
//
//  Purpose:  Default constructor
//
//  Arguments:
//     metaData in  Meta data for table to be written to..


  DbiTrace( "Creating ND::TDbiOutRowStream" << "  ");

}


//.....................................................................

ND::TDbiOutRowStream::~TDbiOutRowStream() {
//
//
//  Purpose: Destructor


  DbiTrace( "Destroying ND::TDbiOutRowStream" << "  ");

}

//.....................................................................

ND::TDbiOutRowStream& ND::TDbiOutRowStream::operator<<(Bool_t src) {
                                     OUT(TDbi::kBool,src);  return *this;}

ND::TDbiOutRowStream& ND::TDbiOutRowStream::operator<<(Char_t src) {
                                     OUT(TDbi::kChar,src);  return *this;}

ND::TDbiOutRowStream& ND::TDbiOutRowStream::operator<<(const Char_t* src) {
                                     OUT(TDbi::kString,src);  return *this;}

ND::TDbiOutRowStream& ND::TDbiOutRowStream::operator<<(Short_t src) {
                                     OUT(TDbi::kShort,src);  return *this;}

ND::TDbiOutRowStream& ND::TDbiOutRowStream::operator<<(UShort_t src) {
                                     OUT2(TDbi::kUShort,src); return *this;}

ND::TDbiOutRowStream& ND::TDbiOutRowStream::operator<<(Int_t src) {
                                     OUT(TDbi::kInt,src);  return *this;}

ND::TDbiOutRowStream& ND::TDbiOutRowStream::operator<<(UInt_t src) {
                                     OUT2(TDbi::kUInt,src);  return *this;}

ND::TDbiOutRowStream& ND::TDbiOutRowStream::operator<<(Float_t src) {
                                     OUT(TDbi::kFloat,src); return *this;}

ND::TDbiOutRowStream& ND::TDbiOutRowStream::operator<<(Double_t src) {
                                     OUT(TDbi::kDouble,src);  return *this;}

ND::TDbiOutRowStream& ND::TDbiOutRowStream::operator<<(const string& src) {
                                    OUT(TDbi::kString,src); return *this;}

ND::TDbiOutRowStream& ND::TDbiOutRowStream::operator<<(const ND::TVldTimeStamp& src) {
  if ( ! StoreDefaultIfInvalid(TDbi::kDate) )
                          Store(TDbi::MakeDateTimeString(src).c_str());
  return *this;
}




//.....................................................................

Bool_t ND::TDbiOutRowStream::StoreDefaultIfInvalid(TDbi::DataTypes type) {
//
//
//  Purpose:  Store default value if illegal type supplied.
//
//  Arguments:
//    type         in    Type of supplied value.
//
//  Return:    kTRUE if illegal type supplied.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o If type of supplied value is incompatible with current column
//    type store default value, report error and return kTRUE, otherwise
//    return kFALSE.

//  Program Notes:-
//  =============

//  None.

  ND::TDbiFieldType typeSupplied(type);
  ND::TDbiFieldType typeRequired(CurColFieldType());
  if ( typeSupplied.IsCompatible(typeRequired) ) return kFALSE;

  string udef = typeRequired.UndefinedValue();
     DbiSevere(  "In table " << TableNameTc()
      << " column "<< CurColNum()
      << " (" << CurColName() << ")"
      << " of type " << typeRequired.AsString()
      << " is incompatible with user type " << typeSupplied.AsString()
      << ", value \"" << udef
      << "\" will be substituted." <<  "  ");
  Store(udef.c_str());
  fBadData = kTRUE;
  return kTRUE;

}
//.....................................................................

void ND::TDbiOutRowStream::Store(const string& str)  {
//
//
//  Purpose: Store string value as comma separated values but exclude SeqNo.
//
//  Arguments:
//    str          in    Value to be stored.
//
//  Return:
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Store string value in CSV with separator from
//    previous value and increment fNumValues.  If required
//    enclose value in quotes. If storing the SeqNo column then
//    store a single '?' instead which will be replace during
//    SQL generation - see

//  Program Notes:-
//  =============

//  None.

  UInt_t concept = CurColFieldType().GetConcept();
  string delim = "";
  if (    concept == TDbi::kString
       || concept == TDbi::kDate
       || concept == TDbi::kChar ) delim = "\'";

  if ( CurColNum()> 1 ) fCSV += ',';
  fCSV += delim;
  if ( concept != TDbi::kString ) fCSV += str;
//  When exporting strings, take care of special characters.
  else {
    ND::UtilString::MakePrintable(str.c_str(),fCSV);
  }
  fCSV += delim;
  IncrementCurCol();
}



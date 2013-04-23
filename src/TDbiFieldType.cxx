// $Id: TDbiFieldType.cxx,v 1.2 2012/06/14 10:55:22 finch Exp $



//////////////////////////////////////////////////////////////////////////
////////////////////////////     ROOT API     ////////////////////////////
//////////////////////////////////////////////////////////////////////////



#include <iostream>

#include <sstream>

#include "TSQLServer.h"
#include "TString.h"

#include "TDbiFieldType.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;
using std::ostringstream;

ClassImp(ND::TDbiFieldType)

//   Definition of static data members
//   *********************************


//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

ND::TDbiFieldType::TDbiFieldType(Int_t type /* = TDbi::kInt */)
{
//
//
//  Purpose:  Default constructor
//
//  Arguments:
//    type     in    Data type  (default TDbi::kInt).



  this->Init(type);

}
//.....................................................................

ND::TDbiFieldType::TDbiFieldType(Int_t type,
                           Int_t size,
                           const char* typeName) {


//
//
//  Purpose:  Constructor for TSQL meta-data.
//
//  Arguments:
//    type      in    value from TSQLColumnInfo::GetSQLType()
//    size      in    value from TSQLColumnInfo::GetLength()
//    typeName  in    value from TSQLColumnInfo::GetTypeName()


  TString name(typeName);
  name.ToUpper();

  // Handle integer types.

  if ( type == TSQLServer::kSQL_INTEGER || type == TSQLServer::kSQL_NUMERIC ) {

    // TSQLServer reports e.g. int(32) as size 32, (even though maximum display is 11)
    // so treat any type starting int or INT as size kMaxInt (i.e. standard 4 byte int)
    if ( name.BeginsWith("INT") ) size = kMaxInt;
    if      ( size <= kMaxTinyInt  ) { this->Init(TDbi::kTiny);  return; }
    else if ( size <= kMaxSmallInt ) { this->Init(TDbi::kShort); return; }
    else if ( size <= kMaxInt      ) { this->Init(TDbi::kInt);   return; }
    else                             { this->Init(TDbi::kLong);  return; }

  }

  // Handle floating point types

  if ( type == TSQLServer::kSQL_FLOAT  ) { this->Init(TDbi::kFloat);  return; }
  if ( type == TSQLServer::kSQL_DOUBLE ) { this->Init(TDbi::kDouble);  return; }

  // Handle cases where type is determined uniquely by type name.

  if ( name == "BINARY_FLOAT" )  { this->Init(TDbi::kFloat);  return; }
  if ( name == "BINARY_DOUBLE" ) { this->Init(TDbi::kDouble); return; }
  if ( name == "TINYTEXT" )      { this->Init(TDbi::kString,kMaxMySQLVarchar);   return; }
  if ( name == "TEXT" )          { this->Init(TDbi::kString,kMaxMySQLText);   return; }
  if ( name == "DATE" )          { this->Init(TDbi::kDate);   return; }
  if ( name == "DATETIME" )      { this->Init(TDbi::kDate);   return; }

  // Handle character types

  if ( type == TSQLServer::kSQL_CHAR && size <= kMaxChar ) {
    this->Init(TDbi::kChar,size);
    return;
  }
  if ( type == TSQLServer::kSQL_CHAR || type == TSQLServer::kSQL_VARCHAR ) {
    if ( size < kMaxMySQLVarchar ) Init(TDbi::kString,size);
    else                           Init(TDbi::kString,kMaxMySQLText);
    return;
  }

  // Anything else is bad news!

     DbiSevere(  "Unable to form SQL ND::TDbiFieldType from: " << type << "  ");
  this->Init(TDbi::kUnknown);

}

//.....................................................................

ND::TDbiFieldType::TDbiFieldType(const ND::TDbiFieldType& from)
{
//
//
//  Purpose:  Copy constructor

//  Program Notes:-
//  =============

//  Make explicit for leak checking.


  *this = from;
}

//.....................................................................

ND::TDbiFieldType::TDbiFieldType(const string& sql,
                           Int_t size )
{
//
//
//  Purpose:  Constructor from a MySQL type string




  if (         sql == "TINYINT" )   this->Init(TDbi::kTiny);

  else if (    sql == "SMALLINT" )  this->Init(TDbi::kShort);

  else if (    sql == "INT"
            || sql == "INTEGER"
            || sql == "NUMERIC" )   this->Init(TDbi::kInt);

  else if (     sql == "BIGINT" )   this->Init(TDbi::kLong);

  else if (    sql == "FLOAT"
            || sql == "REAL")       this->Init(TDbi::kFloat);

  else if (    sql == "DOUBLE" )    this->Init(TDbi::kDouble);

  else if (    sql == "CHAR"
            || sql == "VARCHAR"
            || sql == "TEXT"
            || sql == "TINYTEXT" ) {

    if      ( sql == "TINYTEXT" ) size = kMaxMySQLVarchar;
    else if ( sql == "TEXT"     ) size = kMaxMySQLText;
    else {
      if ( size < 0 ) {
        if ( sql == "CHAR" ) size = 1;
	else                 size = kMaxMySQLVarchar -1;
      }
    }
    if ( fSize <= kMaxChar ) this->Init(TDbi::kChar,size);
    else                     this->Init(TDbi::kString,size);

  }

  else if ( sql == "DATETIME" )     this->Init(TDbi::kDate);

  else {
       DbiSevere(  "Unable to type from SQL: " << sql << "  ");
                                    this->Init(TDbi::kUnknown);
  }

}


//.....................................................................

ND::TDbiFieldType::~TDbiFieldType() {
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
//  o  Destroy FieldType.


//  Program Notes:-
//  =============

//  None.



}

//.....................................................................

string ND::TDbiFieldType::AsString() const {
//
//
//  Purpose:  Return field type as a string.
//
//  Arguments: None.
//
//  Return:    Field type as a string e.g. "Int"
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Return field type as a string.

//  Program Notes:-
//  =============

//  None.

  switch ( fType ) {

  case  TDbi::kBool    : return "Bool";
  case  TDbi::kChar    : return "Char";
  case  TDbi::kUChar   : return "UChar";
  case  TDbi::kTiny    : return "Tiny";
  case  TDbi::kUTiny   : return "UTiny";
  case  TDbi::kShort   : return "Short";
  case  TDbi::kUShort  : return "UShort";
  case  TDbi::kInt     : return "Int";
  case  TDbi::kUInt    : return "UInt";
  case  TDbi::kLong    : return "Long";
  case  TDbi::kULong   : return "ULong";
  case  TDbi::kFloat   : return "Float";
  case  TDbi::kDouble  : return "Double";
  case  TDbi::kString  : return "String";
  case  TDbi::kTString : return "TString";
  case  TDbi::kDate    : return "Date";
  }
  return "Unknown";
}
//.....................................................................

string ND::TDbiFieldType::AsSQLString() const {
//
//
//  Purpose:  Return field type as a string suitable for MySQL column.

  ostringstream os;

  switch ( fType ) {

  case  TDbi::kBool    :   os << "CHAR";          break;

  case  TDbi::kUTiny   :
  case  TDbi::kTiny    :   os << "TINYINT";       break;

  case  TDbi::kShort   :
  case  TDbi::kUShort  :   os << "SMALLINT";      break;

  case  TDbi::kInt     :
  case  TDbi::kUInt    :   os << "INT";           break;

  case  TDbi::kLong    :
  case  TDbi::kULong   :   os << "BIGINT";        break;

  case  TDbi::kFloat   :   os << "FLOAT";         break;

  case  TDbi::kDouble  :   os << "DOUBLE";        break;

  case  TDbi::kChar    :
  case  TDbi::kUChar   :
  case  TDbi::kString  :
  case  TDbi::kTString :
    if      ( fSize == 1)                os << "CHAR";
    else if ( fSize <= kMaxChar)         os << "CHAR("    << fSize << ')';
    else if ( fSize <  kMaxMySQLVarchar) os << "VARCHAR(" << fSize << ')';
    else if ( fSize == kMaxMySQLVarchar) os << "TINYTEXT";
    else                                 os << "TEXT";
    break;

  case  TDbi::kDate    :   os << "DATETIME";      break;

  default :               os << "Unknown";

  }

  return os.str();

}


//.....................................................................


void ND::TDbiFieldType::Init(Int_t type  /* Type as defined by TDbi::DataTypes */,
                        Int_t size  /* Size in bytes (default: -1 - take size from type)*/ ) {
//
//
//  Purpose:  Initialise object.
//

  switch ( type ) {

  case TDbi::kBool :
      fType      = TDbi::kBool;
      fConcept   = TDbi::kBool;
      fSize      = 1;
      break;

    case TDbi::kChar :
      fType      = TDbi::kChar;
      fConcept   = TDbi::kChar;
      fSize      = 1;
      break;

    case TDbi::kUChar :
      fType      = TDbi::kUChar;
      fConcept   = TDbi::kUChar;
      fSize      = 1;
      break;

    case TDbi::kTiny :
      fType      = TDbi::kTiny;
      fConcept   = TDbi::kInt;
      fSize      = 1;
      break;

    case TDbi::kUTiny :
      fType      = TDbi::kUTiny;
      fConcept   = TDbi::kUInt;
      fSize      = 1;
      break;

    case TDbi::kShort :
      fType      = TDbi::kShort;
      fConcept   = TDbi::kInt;
      fSize      = 2;
      break;

    case TDbi::kUShort :
      fType      = TDbi::kUShort;
      fConcept   = TDbi::kUInt;
      fSize      = 2;
      break;

    case TDbi::kInt :
      fType      = TDbi::kInt;
      fConcept   = TDbi::kInt;
      fSize      = 4;
      break;

    case TDbi::kUInt :
      fType      = TDbi::kUInt;
      fConcept   = TDbi::kUInt;
      fSize      = 4;
      break;

    case TDbi::kLong :
      fType    = TDbi::kLong;
      fConcept = TDbi::kInt;
      fSize    = 8;
      break;

    case TDbi::kULong :
      fType    = TDbi::kULong;
      fConcept = TDbi::kUInt;
      fSize    = 8;
      break;

    case TDbi::kFloat :
      fType      = TDbi::kFloat;
      fConcept   = TDbi::kFloat;
      fSize      = 4;
      break;

    case TDbi::kDouble :
      fType      = TDbi::kDouble;
      fConcept   = TDbi::kFloat;
      fSize      = 8;
      break;

    case TDbi::kString :
      fType      = TDbi::kString;
      fConcept   = TDbi::kString;
      fSize      = 65535;
      break;

    case TDbi::kTString :
      fType      = TDbi::kTString;
      fConcept   = TDbi::kString;
      fSize      = 65535;
      break;

    case TDbi::kDate :
      fType      = TDbi::kDate;
      fConcept   = TDbi::kDate;
      fSize      = 4;
      break;

    case TDbi::kUnknown :
      fType      = TDbi::kUnknown;
      fConcept   = TDbi::kUnknown;
      break;

    default :
         DbiSevere(  "Unable to form Root ND::TDbiFieldType from: " << type << "  ");
      fType      = TDbi::kUnknown;
      fConcept   = TDbi::kUnknown;
      fSize      = 0;
  }

  // Override fSize if necessary.

  if ( size      >= 0  ) fSize      = size;

}

//.....................................................................

Bool_t ND::TDbiFieldType::IsCompatible(const ND::TDbiFieldType& other) const {
//
//
//  Purpose: Return kTRUE if this is compatible with or can accept
//           the other type.
//
//  Arguments:
//    other        in    The ND::TDbiFieldType to be compared
//
//  Return: kTRUE if the this type can accept other type.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o If the two types have the same concept or have compatible
//    concepts return kTRUE.
//
//  o Also return true if the other type can be be used as input.
//

//  Program Notes:-
//  =============

//  None.

#define MATCHES(x,y)  \
    ( (fConcept == x) && (concept == y) )\
  || ((fConcept == y) && (concept == x) )
  UInt_t concept = other.GetConcept();
  if ( fConcept == concept ) return kTRUE;
  if ( MATCHES(TDbi::kBool,  TDbi::kChar)  ) return kTRUE;
  if ( MATCHES(TDbi::kBool,  TDbi::kUChar) ) return kTRUE;
  if ( MATCHES(TDbi::kInt,   TDbi::kChar)  ) return kTRUE;
  if ( MATCHES(TDbi::kUInt,  TDbi::kUChar) ) return kTRUE;
//  Allow unsigned to match signed because the TSQLResultSetL interface
//  does not support unsigned types but its GetShort and GetInt
//  methods will return unsigned data intact so we must trust
//  that the user knows what they are doing!
  if ( MATCHES(TDbi::kUInt,  TDbi::kInt)   ) return kTRUE;
//  Allow char to be input to string.
  if ( concept == TDbi::kChar && fConcept == TDbi::kString ) return kTRUE;

return kFALSE;

}

//.....................................................................

string ND::TDbiFieldType::UndefinedValue() const {
//
//
//  Purpose:  Return value to be used when unknown.
//
//  Arguments: None
//
//  Return:   Value as a string to be used when unknown.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Return value to be used when unknown.

//  Program Notes:-
//  =============

//  For each conceptual data type there is an associated
//  data type to be used when attemping to get value from outside
//  the table or when table type incompatible with data.


  switch ( fConcept ) {
    case TDbi::kBool   : return "0";
    case TDbi::kChar   : return "";
    case TDbi::kUChar  : return "";
    case TDbi::kInt    : return "-999";
    case TDbi::kUInt   : return " 999";
    case TDbi::kFloat  : return "-999.";
    case TDbi::kString : return "";
    case TDbi::kDate   : return "1980-00-00 00:00:00";

    default :
         DbiSevere( "Unable to define undefined type for: "
	    << fConcept << "  ");
      return "";
  }
}




//////////////////////////////////////////////////////////////////////////
////////////////////////////     ROOT API     ////////////////////////////
//////////////////////////////////////////////////////////////////////////


#include <cassert>
#include <cctype>
#include <sstream>

#include "TString.h"

#include "TDbi.hxx"
#include "TDbiString.hxx"
#include "TDbiTableMetaData.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;

ClassImp(ND::TDbiTableMetaData)

//   Definition of static data members
//   *********************************


ND::TDbiTableMetaData::ColumnAttributes ND::TDbiTableMetaData::fgDummy;

//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

ND::TDbiTableMetaData::TDbiTableMetaData(const string& tableName) :
fNumCols(0),
fTableName(tableName)
{
//
//
//  Purpose:  Default constructor

//  Program Notes:-
//  =============

//  This is filled by the owning ND::TDbiTableProxy, as it is too low level to
//  use the cascade to fill itself, hence the friend status granted to ND::TDbiDBProxy.


  DbiTrace( "Creating ND::TDbiTableMetaData" << "  ");

}

//.....................................................................

ND::TDbiTableMetaData::~TDbiTableMetaData() {
//
//
//  Purpose: Destructor


  DbiTrace( "Destroying ND::TDbiTableMetaData" << "  ");

}

///.....................................................................

void ND::TDbiTableMetaData::Clear() {
//
//
//  Purpose: Clear out existing data (apart from table name)
//
//  Contact:   N. West

  fColAttr.clear();
  fNumCols = 0;

}

//.....................................................................

void ND::TDbiTableMetaData::ExpandTo(UInt_t colNum) {
//
//
//  Purpose: Expand table if required to specied colNum
//
//  Arguments:
//    colNum       in    Column number to expand to (1,2...).
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

  assert ( colNum < MAXCOL );
  while ( fNumCols < colNum ) {
    fColAttr.push_back(ColumnAttributes());
    ++fNumCols;
  }
}

//.....................................................................

const  ND::TDbiTableMetaData::ColumnAttributes&  ND::TDbiTableMetaData::GetAttributes(Int_t colNum) const {

// Return a column attributes (will be dummy entry if requesting invalid column)

  if ( colNum > 0 && colNum <= (signed) fNumCols ) return fColAttr[colNum-1];
  fgDummy.SetDefault();
  return fgDummy;

}

//.....................................................................

string ND::TDbiTableMetaData::GetToken(const char*& strPtr) {
//
//
//  Purpose:  Skip spaces and return next token from string and move pointer on.

  string token;

// Skip white space and quit if at EOS.
  while ( isspace(*strPtr) ) ++strPtr;
  if ( *strPtr == 0 ) return token;

// Collect the first character whatever it is.
  char firstChar = *strPtr++;
  token = firstChar;
  if ( ! isalnum(firstChar) && firstChar != '_' ) return token;

// Collect more if collecting alphanumeric + underscore string
  while ( isalnum(*strPtr) || *strPtr == '_' ) token += *strPtr++;
  return token;

}

//.....................................................................

 ND::TDbiTableMetaData::ColumnAttributes& ND::TDbiTableMetaData::SetAttributes(Int_t colNum) {

// Return a column attributes (will be dummy entry if requesting invalid column)

  this->ExpandTo(colNum);
  // Using const metho so must cast away constness.
  return const_cast<ND::TDbiTableMetaData::ColumnAttributes&>(this->GetAttributes(colNum));

}


//.....................................................................

void ND::TDbiTableMetaData::SetColFieldType(const ND::TDbiFieldType& fieldType,
                                       Int_t colNum) {
//
//
//  Purpose:  Define field type for specified column.
//
//  Arguments:
//    fieldType    in    Column field type.
//    colNum       in    Column number (1,2...)
//

  ColumnAttributes&  attrib(this->SetAttributes(colNum));
  UInt_t concept     = fieldType.GetConcept();
  attrib.Type        = fieldType;
  attrib.MustDelimit = concept == TDbi::kString || concept == TDbi::kDate || concept == TDbi::kChar;
  attrib.Concept     = concept;

}

//.....................................................................

void ND::TDbiTableMetaData::SetFromSql(const string& sql) {
//
//
//  Purpose:  Reconstruct this object using SQL to create table.


  TString SqlUpper(sql);
  SqlUpper.ToUpper();

  const char* strPtr = SqlUpper.Data();

  string token1(ND::TDbiTableMetaData::GetToken(strPtr));
  string token2(ND::TDbiTableMetaData::GetToken(strPtr));
  string token3(ND::TDbiTableMetaData::GetToken(strPtr));
  string token4(ND::TDbiTableMetaData::GetToken(strPtr));

  if ( token1 != "CREATE" || token2 != "TABLE" || token4 != "(" ) {
    DbiSevere( "Cannot recreate: SQL " << SqlUpper
			   << " does not start CREATE TABLE ... (" << "  ");
    return;
  }

  this->Clear();
  fTableName = token3;
  DbiLog( "Recreating  ND::TDbiTableMetaData for table " << fTableName << "  ");

// Loop processing column specifications.
  Int_t col = 0;

  string delim;
  while ( delim != ")" ) {
    string name = ND::TDbiTableMetaData::GetToken(strPtr);

//  Deal with INDEX and PRIMARY KEY
    if ( name == "INDEX" ||  name == "KEY" || name == "PRIMARY" ) {
      if ( name == "PRIMARY" || name == "KEY" )delim = ND::TDbiTableMetaData::GetToken(strPtr);
      delim = ND::TDbiTableMetaData::GetToken(strPtr);
      if ( delim == "(" ) while ( delim != ")" ) delim = ND::TDbiTableMetaData::GetToken(strPtr);
      delim = ND::TDbiTableMetaData::GetToken(strPtr);
      continue;
    }

//  Collect name and type.
    ++col;
    this->SetColName(name,col);
    this->SetColIsNullable(col);

    string type = ND::TDbiTableMetaData::GetToken(strPtr);
    int precision = 0;
    delim = ND::TDbiTableMetaData::GetToken(strPtr);
    if ( delim == "(" ) {
      delim = ND::TDbiTableMetaData::GetToken(strPtr);
      std::istringstream is(delim);
      is >> precision;
      delim = ND::TDbiTableMetaData::GetToken(strPtr);
      delim = ND::TDbiTableMetaData::GetToken(strPtr);
    }
    ND::TDbiFieldType ft(type,precision);
    this->SetColFieldType(ft,col);
    DbiLog( "  Column: " << col << " name " << this->ColName(col)
			  << " type " << this->ColFieldType(col).AsString()
			  << " precision " << precision << "  ");

//  Collect optional qualifiers.

    while ( delim != ","  &&  delim != ")" ) {
      string opt2 = ND::TDbiTableMetaData::GetToken(strPtr);
      if ( delim == "NOT" && opt2 == "NULL") {
        this->SetColIsNullable(col,false);
        delim = ND::TDbiTableMetaData::GetToken(strPtr);
      }
      else if ( delim == "PRIMARY" && opt2 == "KEY") {
        delim = ND::TDbiTableMetaData::GetToken(strPtr);
      }
      else if ( delim == "AUTO_INCREMENT") {
        delim = opt2;
      }
      else if ( delim == "UNSIGNED") {
        delim = opt2;
      }
      else {
        DbiWarn( "Column: " << col << " name " << name << " type " << ft.AsString()
			      << " ignoring unknown option: " << delim << "  ");
        delim = opt2;
      }
    }
  }

}


//.....................................................................

string ND::TDbiTableMetaData::Sql() const {
//
//
//  Purpose:  Return SQL string to create table.
//
//  Return:    SQL command to create required table.

  Bool_t mainTable = fTableName.substr(fTableName.size()-3,3) != "VLD";

  string tableName = fTableName;
  ND::TDbiString sql;
  sql.GetString() = "";
  sql << "create table " << tableName << "(";


//  Assemble columns.

  int numCols = this->NumCols();
  for(int i=1; i<= numCols; i++) {

    sql << this->ColName(i) << " " ;
    sql << this->ColFieldType(i).AsSQLString();

    if(    this->ColName(i) == "SEQNO"
        && ! mainTable )          sql << " not null primary key" ;

    else if( ! this->ColIsNullable(i)
            || this->ColName(i) == "SEQNO"
            || this->ColName(i) == "ROW_COUNTER"
           ) sql << " not null" ;

    if (i < numCols)              sql << ", ";

  }

// Deal with key/index/constraint.

// Nothing to do for the special xxxSEQNO tables.
  if ( fTableName == "GLOBALSEQNO" ||  fTableName == "LOCALSEQNO" ) {
    sql << ")";
  }

  else {
   if ( mainTable ) {
      sql << ", primary key (SEQNO,ROW_COUNTER)";
    }
    else {
      sql << ", key TIMESTART (TIMESTART), key TIMEEND (TIMEEND)";
    }
    sql << ")";
  }

  return sql.GetString();

}



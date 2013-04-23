// $Id: TDbiSqlValPacket.cxx,v 1.2 2012/06/14 10:55:22 finch Exp $

#include <algorithm>
#include <memory>
#include <sstream>
#include <vector>

#include "TDbi.hxx"
#include "TDbiCascader.hxx"
#include "TDbiConfigSet.hxx"
#include "TDbiOutRowStream.hxx"
#include "TDbiResultSetHandle.hxx"
#include "TDbiInRowStream.hxx"
#include "TDbiSqlValPacket.hxx"
#include "TDbiStatement.hxx"
#include "TDbiTableProxy.hxx"
#include "TDbiTableRow.hxx"
#include "TDbiDatabaseManager.hxx"
#include "TDbiValidityRec.hxx"
#include "TDbiValRecSet.hxx"

#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::auto_ptr;
using std::endl;
using std::istringstream;
using std::ostringstream;
using std::setw;
using std::setfill;
using std::setprecision;
#include "UtilString.hxx"
#include "TVldRange.hxx"

ClassImp(ND::TDbiSqlValPacket)

#ifdef IRIX6
// SGI barfs at generating an operator != for this enum if it
// is anonymous and in the Fill() method.
enum EFillState { kLOOKING_FOR_HEADER,
                  kLOOKING_FOR_TRAILER };
#endif

//   Definition of static data members
//   *********************************


//   Definition of file static members functions
//   *******************************************

static bool compStringPtrs(const string* str1, const string* str2 ) {
  return *str1 < *str2; }

//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................
/// Default constructor.
ND::TDbiSqlValPacket::TDbiSqlValPacket() :
fNumErrors(0),
fSeqNo(0),
fNumStmts(0)
{
//
//
//  Purpose:  Default ctor.


  DbiTrace( "Creating ND::TDbiSqlValPacket" << "  ");
}

//.....................................................................
///\verbatim
///
///  Purpose:  Constructor reading from input stream.
///
///  Arguments:
///      is    in/out    Input stream.
///
///  Return:   n/a.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o  Create ND::TDbiSqlValPacket and fill from input stream by calling Fill .
///\endverbatim
ND::TDbiSqlValPacket::TDbiSqlValPacket(std::ifstream& is) :
fNumErrors(0),
fSeqNo(0),
fNumStmts(0)
{


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Creating ND::TDbiSqlValPacket" << "  ");

  Fill(is);

}
//.....................................................................
///\verbatim
///
///  Purpose:  Constructor from a ND::TDbiValidityRec.
///
///  Arguments:
///      vrec  in   ND::TDbiValidityRec from which to create packet.     .
///
///  Return:   n/a.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o  Create ND::TDbiSqlValPacket from ND::TDbiValidityRec and associated data.
///
///
///  Program Notes:-
///  =============
///
///  This member function uses ND::TDbiConfigSet as a generic concrete
///  ND::TDbiTableRow that can be used to create SQL for any type of table.
///\endverbatim
ND::TDbiSqlValPacket::TDbiSqlValPacket(const ND::TDbiValidityRec& vrec) :
fNumErrors(0),
fSeqNo(vrec.GetSeqNo()),
fNumStmts(0),
fTableName(vrec.GetTableProxy()->GetTableName()),
fCreationDate(vrec.GetCreationDate())
{


  DbiTrace( "Creating ND::TDbiSqlValPacket" << "  ");

  const ND::TDbiTableProxy& tableProxy = *vrec.GetTableProxy();
  Int_t seqNo  = vrec.GetSeqNo();
  UInt_t dbNo  = vrec.GetDbNo();

  // Create the SQL for the ND::TDbiValidityRec itself.
  this->AddRow(tableProxy,0,vrec);

  // Create the SQL for the rows.

  const ND::TDbiDBProxy& dbProxy = tableProxy.GetDBProxy();
  ND::TDbiInRowStream* rset = dbProxy.QuerySeqNo(seqNo,dbNo);


  for(; ! rset->IsExhausted(); rset->FetchRow()) {
    string str;
    rset->RowAsCsv(str);
    this->AddRow(str);
  }
  delete rset;
  rset = 0;
}

//.....................................................................

ND::TDbiSqlValPacket::~TDbiSqlValPacket() {
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
//  o  Destroy ND::TDbiSqlValPacket.


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Destroying ND::TDbiSqlValPacket" << "  ");

  Clear();

}

//.....................................................................
/// Add data row tblProxy vrec row to this object.
Bool_t ND::TDbiSqlValPacket::AddDataRow(const ND::TDbiTableProxy& tblProxy,
                                   const ND::TDbiValidityRec* vrec,
                                   const ND::TDbiTableRow& row){
//
//
//  Purpose: Add data row.
//

  if ( this->GetNumSqlStmts() == 0 ) {
       DbiSevere(  "Cannot add data row - packet does not have a VLD row"  << "  ");
    ++fNumErrors;
    return kFALSE;
  }

  return this->AddRow(tblProxy,vrec,row);

}

//.....................................................................
/// add row to sql statements
void ND::TDbiSqlValPacket::AddRow(const string & row){
//
//
//  Purpose: Add row.
//

  string sql("INSERT INTO ");
  sql += this->GetTableName();
  if ( this->GetNumSqlStmts() == 0 ) sql += "VLD";
  sql += " VALUES (" + row + ");";
  ostringstream seqno;
  seqno << this->GetSeqNo();
  this->SetSeqNoOnRow(sql,seqno.str());
  fSqlStmts.push_back(sql);
  ++fNumStmts;

}

//.....................................................................

Bool_t ND::TDbiSqlValPacket::AddRow(const ND::TDbiTableProxy& tblProxy,
                               const ND::TDbiValidityRec* vrec,
                               const ND::TDbiTableRow& row){
//
//
//  Purpose: Add row.
//


  bool isVld = this->GetNumSqlStmts() == 0;
  const ND::TDbiTableMetaData& meta = isVld ? tblProxy.GetMetaValid() : tblProxy.GetMetaData();
  ND::TDbiOutRowStream outRow(&meta);

// Store dummy SEQNO and ROW_COUNTER for data rows.
  if ( ! isVld ) {
    outRow << 0;          // dummy SEQNO
    outRow <<  fNumStmts; // ROW_COUNTER
  }
  row.Store(outRow,vrec);
  if ( ! outRow.HasGoodData() ) {
    if ( ! outRow.IsComplete() ) {
         DbiSevere(  "Incomplete data supplied for row " << this->GetNumSqlStmts()-1
        << " of table "
        << tblProxy.GetTableName() << "  ");
    }
    else {
         DbiSevere(  "Complete but bad data supplied for table "
        << tblProxy.GetTableName() << "  ");
    }
    ++fNumErrors;
    return kFALSE;
  }
  this->AddRow(outRow.GetCSV());
  return kTRUE;
}
//.....................................................................
///\verbatim
///  Purpose:  Compare to another ND::TDbiSqlValPacket
///
///  Arguments:
///    that         in    The other ND::TDbiSqlValPacket to be compared.
///    log          in    If true list differences to MSG("ND::TDbi",kInfo)
///    thisName     in    Optional name for this packet (default: this)
///    thatName     in    Optional name for that packet (default: that)
///
///  Return:    kIdentical   if identical (apart from InsertDate)
///             kUpdate      "that" is more up to date
///             kOutOfDate   "that" is out of date
///             kConflict    records are incompatible
///
///  Contact:   N. West
///\endverbatim
ND::TDbiSqlValPacket::CompResult_t ND::TDbiSqlValPacket::Compare(
                                const ND::TDbiSqlValPacket& that,
                                Bool_t log,
                                const Char_t* thisName,
                                const Char_t* thatName ) const {
//
//

  if ( this->IsEqual(that,log,thisName,thatName ) ) return kIdentical;

  if (    fSeqNo           != that.fSeqNo
       || fTableName       != that.fTableName     ) return kConflict;

  std::vector<std::string> valuesThis = this->GetStmtValues(0);
  std::vector<std::string> valuesThat = that.GetStmtValues(0);

  // Assume CreationDate is the 9th element (counting from 0).
  int comp = valuesThis[9].compare(valuesThat[9]);

  if ( comp < 0 ) {
    if ( log ) DbiInfo(  "  Difference classified as Update" << "  ");;
    return kUpdate;
  }
  else if ( comp > 0 ) {
    if ( log ) DbiInfo(  "  Difference classified as OutOfDate" << "  ");
    return kOutOfDate;
  }
  return kConflict;

}


//.....................................................................
///\verbatim
///
///  Purpose:  Create table in specified database.
///
///  Arguments:
///    dbNo         in    Number of database in the cascade.
///
///  Return:    kTRUE if successfully created.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o If SQL to create tables is available, use it to create
///    the main and auxiliary tables and refresh the corresponding
///    ND::TDbiTableProxy.
///\endverbatim
Bool_t ND::TDbiSqlValPacket::CreateTable(UInt_t dbNo) const {

//  Program Notes:-
//  =============

//  None.

  if ( ! CanBeStored() ) return kFALSE;

  // Locate required ND::TDbiStatement.
  auto_ptr<ND::TDbiStatement> stmtDb(ND::TDbiDatabaseManager::Instance()
                               .GetCascader()
                               .CreateStatement(dbNo));
  if ( ! stmtDb.get() ) {
    DbiWarn(  "Attempting to write to non-existant cascade entry " << dbNo
      << "  ");
    return kFALSE;
  }
  if ( fSqlMySqlMetaMain == "" || fSqlMySqlMetaVld  == "" ) {
    DbiWarn(  "No SQL available to create table " << fTableName
      << " in cascade entry: " << dbNo << "  ");
    return kFALSE;
  }

  stmtDb->ExecuteUpdate(fSqlMySqlMetaVld.c_str());
  if ( stmtDb->PrintExceptions() ) return kFALSE;
  stmtDb->ExecuteUpdate(fSqlMySqlMetaMain.c_str());
  if ( stmtDb->PrintExceptions() ) return kFALSE;

  ND::TDbiDatabaseManager::Instance().RefreshMetaData(this->GetTableName());

  return kTRUE;

}

//.....................................................................
///\verbatim
///
///  Purpose:  Refill object from input string.
///
///  Arguments:
///      is    in/out    Input stream.
///
///  Return:   kTRUE "is" object has been refilled.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Collect SQL statements, and possibly metadata (as SQL to
///    create table) bracketed by >>>> ... <<<<< checking that
///    the table name and SeqNos match.  Count and discard any data that
///    looks bad.

Bool_t ND::TDbiSqlValPacket::Fill(std::ifstream& is) {

//  Program Notes:-
//  =============

//  None.

#ifdef IRIX6
  EFillState state = kLOOKING_FOR_HEADER;
#else
  enum { kLOOKING_FOR_HEADER,
         kLOOKING_FOR_TRAILER } state = kLOOKING_FOR_HEADER;
#endif

  enum { kMAXTABLENAMELEN        = TDbi::kMAXTABLENAMELEN,
         kHEADER_TRAILER_MAX_LEN = kMAXTABLENAMELEN + 20 };

  string nameHead;
  string nameTrail;
  UInt_t seqNoHead  = 0;
  UInt_t seqNoTrail = 0;

  string line;
  string msg;
  string sql;
  int lineNum = 0;

  this->Reset();
  //  Loop until EOF reading lines.

  while ( ! is.eof() ) {
    getline(is,line);
    ++lineNum;
    // Skip null lines.
    if (line.size() == 0 ) continue;

    // Look for header line
    if ( state == kLOOKING_FOR_HEADER ) {
      if ( line.substr(0,5) == ">>>>>" ) {
        if ( line.size() >= kHEADER_TRAILER_MAX_LEN ) {
          Report("Bad header",lineNum,line);
          continue;
	}

	// Look for optional metadata.
        if ( line.find("Metadata") != string::npos ) {
          getline(is,fSqlMySqlMetaVld);
          ++lineNum;
          getline(is,fSqlMySqlMetaMain);
          ++lineNum;
          getline(is,line);
          ++lineNum;
          if (   line.substr(0,5) != "<<<<<"
              || line.find("Metadata") == string::npos ) {
            Report("Bad metadata",lineNum,line);
            continue;
	  }
          getline(is,line);
          ++lineNum;
          if ( line.size() >= kHEADER_TRAILER_MAX_LEN ) {
            Report("Bad header",lineNum,line);
            continue;
	  }
	}

        //  Collect table name and SeqNo.
        istringstream istr(line.substr(5));
        istr.width(kMAXTABLENAMELEN);
	istr >> nameHead >> seqNoHead;
        if ( ! istr.eof() ) {
          Report("Input error",lineNum,line);
          continue;
	}

	// Header looks good, start to collect SQL.
        state = kLOOKING_FOR_TRAILER;
        sql = "";
      }
      else {
        Report("Not header",lineNum,line);
      }
    }

    //Look for trailer line.

    else {
      if ( line.substr(0,5) == "<<<<<" ) {
        if ( line.size() >= kHEADER_TRAILER_MAX_LEN
           ) msg = "Bad trailer";

        else {

	  //  Collect table name and SeqNo.
          istringstream istr(line.substr(5));
          istr.width(kMAXTABLENAMELEN);
	  istr >> nameTrail >> seqNoTrail;
          if ( ! istr.eof() ) msg = "Input error";

	  else if (    nameTrail != nameHead
                    || seqNoHead != seqNoTrail ) {
            msg = "Header/Trailer mismatch: Header: ";
            msg += istr.str();
	  }
	  else if ( GetNumSqlStmts() == 0
                  ) msg = "No SQL statements between Header/Trailer";
	  else {

 	    // Trailer looks good return with object filled.
            fSeqNo     = seqNoHead;
            fTableName = nameHead;

	    //Dig out the creation date from the first record.
            string date = this->GetStmtValues(0)[9];
	    //Remove the quotes.
            date.erase(0,1);
            date.erase(date.size()-1,1);
            fCreationDate = TDbi::MakeTimeStamp(date);
            return kTRUE;

	  }
	}

	// Trailer bad, start again!
        Report(msg.c_str(),lineNum,line);
        state = kLOOKING_FOR_HEADER;
      }

      // Not a trailer line, must be SQL, collect and append lines
      // until a trailing ; found.
      else {
        sql += line;
        if ( sql[sql.size()-1] == ';') {
          fSqlStmts.push_back(sql);
          ++fNumStmts;
          sql = "";
	}
      }
    }

  }

  if ( state != kLOOKING_FOR_HEADER
     ) Report("Unexpected EOF",lineNum,"EOF");
  this->Reset();
  return kFALSE;

}
//.....................................................................
/// Return a selected statment
string ND::TDbiSqlValPacket::GetStmt(UInt_t stmtNo) const {
//
//
//  Purpose:  Return a selected statment

  if ( stmtNo >= this->GetNumSqlStmts() ) return "";

  // Locate statement
  std::list<std::string>::const_iterator itr = fSqlStmts.begin();
  while ( stmtNo ) { ++itr; --stmtNo; }

  return *itr;

}
//.....................................................................
///\verbatim
///
///  Purpose:  Return all the values associated with a selected statment
///
///  Arguments:
///    stmtNo       in    The statement number (starting at 0)
///
///  Return:              vector of string values.
///                       Empty vector if stmt does not exits.
///
///  Contact:   N. West
///\endverbatim
std::vector<std::string> ND::TDbiSqlValPacket::GetStmtValues(UInt_t stmtNo) const {


  std::vector<std::string> vec;
  if ( stmtNo >= this->GetNumSqlStmts() ) return vec;

  // Locate statement
  std::string str = this->GetStmt(stmtNo);

  // Extract ...(list-of-values)... and parse it into tokens.
  std::string::size_type pos = str.find('(');
  ++pos;
  std::string::size_type n = str.find(')') - pos;
  ND::UtilString::StringTok(vec,str.substr(pos,n),",");

  return vec;

}

//.....................................................................

///\verbatim
///  Purpose:  Test for equality to another ND::TDbiSqlValPacket
///
///  Arguments:
///    that         in    The other ND::TDbiSqlValPacket to be compared.
///    log          in    If true list differences to MSG("ND::TDbi",kInfo)
///    thisName     in    Optional name for this packet (default: this)
///    thatName     in    Optional name for that packet (default: that)
///
///  Return:    kTRUE if identical (apart from InsertDate).
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Compare to another ND::TDbiSqlValPacket.
///
///  Program Notes:-
///  =============
///
///  The InsertDate reflects the date when the data was written into the
/// local database so has to be excluded from the comparison.
///\endverbatim

Bool_t ND::TDbiSqlValPacket::IsEqual(const ND::TDbiSqlValPacket& that,
                                Bool_t log,
                                const Char_t* thisName,
                                const Char_t* thatName ) const {
//
  if (    fSeqNo           != that.fSeqNo
       || fTableName       != that.fTableName
       || fNumStmts != that.fNumStmts ) {
if ( log ) TDbiLog::GetLogStream()<< "Conflict found:"
             << " for : " << thisName << " , " << thatName << " :-\n"
	     << "  SeqNo " << fSeqNo << "," << that.fSeqNo
	     << "\n  TableName " << fTableName << "," << that.fTableName
	     << "\n  Size " << fNumStmts << ","
	     << that.fNumStmts << endl;
    return kFALSE;
  }

  list<string>::const_iterator itrThisBegin = fSqlStmts.begin();
  list<string>::const_iterator itrThisEnd   = fSqlStmts.end();
  list<string>::const_iterator itrThatBegin = that.fSqlStmts.begin();
  list<string>::const_iterator itrThatEnd   = that.fSqlStmts.end();

  list<string>::const_iterator itrThis = itrThisBegin;
  list<string>::const_iterator itrThat = itrThatBegin;

  Bool_t isEqual = kTRUE;

  // Strip off InsertDate from first statement (assume its
  // the last parameter in list).

  string strThis = (*itrThis).substr(0,(*itrThis).rfind(','));
  string strThat = (*itrThat).substr(0,(*itrThat).rfind(','));
  if ( strThis != strThat ) {
  if ( ! log ) return kFALSE;
  isEqual = kFALSE;
  DbiInfo(  "Difference on VLD record " << ":-\n"
      << "  " << thisName << ": " << strThis  << "  "
      << "  " << thatName << ": " << strThat  << "  ");
  }

  // Rows can come in any order (after the first) so we have
  // to sort before comparing.  However, if we are lucky, they
  // may be in the same order or inverse order so do a quick
  // test to see if there are no conflicts assuming these
  // relative orderings.

  ++itrThis;
  ++itrThat;
  while ( itrThis != itrThisEnd && (*itrThis) == (*itrThat) ) {
//     cout << "Debug: trying forward compare ..." << *itrThis
// 	 << "::" << *itrThat << endl;
    ++itrThis;
    ++itrThat;
  }
  if ( itrThis == itrThisEnd ) return isEqual;

  itrThis = itrThisBegin;
  itrThat = itrThatEnd;
  ++itrThis;
  --itrThat;

  while ( itrThis != itrThisEnd &&  (*itrThis) == (*itrThat) ) {
//   cout << "Debug: trying reverse compare ..." << *itrThis
// 	 << "::" << *itrThat << endl;
    ++itrThis;
    --itrThat;
  }
  if ( itrThis == itrThisEnd ) return isEqual;

  // O.K., we are out of luck so set up pointers to both sets
  // and sort these.

  typedef vector<const string*>      shadow_list_t;
  typedef shadow_list_t::iterator  shadow_list_itr_t;

  shadow_list_t shadowThis;
  shadow_list_t shadowThat;

  itrThis = itrThisBegin;
  itrThat = itrThatBegin;
  ++itrThis;
  ++itrThat;

  while ( itrThis != itrThisEnd ) {
    shadowThis.push_back(&(*itrThis));
    shadowThat.push_back(&(*itrThat));
    ++itrThis;
    ++itrThat;
  }

  shadow_list_itr_t shadowThisItr    = shadowThis.begin();
  shadow_list_itr_t shadowThisItrEnd = shadowThis.end();
  shadow_list_itr_t shadowThatItr    = shadowThat.begin();
  shadow_list_itr_t shadowThatItrEnd = shadowThat.end();

  sort(shadowThisItr,shadowThisItrEnd,compStringPtrs);
  sort(shadowThatItr,shadowThatItrEnd,compStringPtrs);

  while ( shadowThisItr != shadowThisItrEnd ) {
    if ( (**shadowThisItr) != (**shadowThatItr) ){
      if ( ! log ) return kFALSE;
      isEqual = kFALSE;
      DbiInfo( "Difference on data record "
         << ":-\n"
         << "  " << thisName << ": " << **shadowThisItr  << "  "
         << "  " << thatName << ": " << **shadowThatItr  << "  ");
    }
   ++shadowThisItr;
   ++shadowThatItr;
  }

  return isEqual;

}

//.....................................................................
/// Print the current state
void ND::TDbiSqlValPacket::Print(Option_t * /* option */) const {
//
//
//  Purpose:  Print the current state.

  DbiInfo(  "ND::TDbiSQLValPacket:"
    << " table \"" << fTableName << "\" "
    << " SeqNo " << fSeqNo
    << " NumErrors " << fNumErrors
    << "  "
    << "   CreationDate " << fCreationDate
    << "  ");

  DbiInfo(  "   MySQL Main table creation: \"" << fSqlMySqlMetaMain << "\"" << "  ");

  DbiInfo(  "   MySQL VLD table creation: \"" << fSqlMySqlMetaVld << "\"" << "  ");

  if ( GetNumSqlStmts() > 0 ) {
    std::list<string>::const_iterator itr    = fSqlStmts.begin();
    std::list<string>::const_iterator itrEnd = fSqlStmts.end();
    for (; itr != itrEnd; ++itr)
      DbiInfo(  "   SqlStmt \"" << *itr << "\"" << "  ");
  }
  else {
    DbiInfo( "   No SqlStmts." << "  ");
  }

}


//.....................................................................
//
///  Purpose:  Recreate and define first row (VLD - the validity record).
void ND::TDbiSqlValPacket::Recreate(const string& tableName,
                               const ND::TVldRange& vr,
                               Int_t aggNo,
                               TDbi::Task task,             /*  Default: 0 */
                               ND::TVldTimeStamp creationDate   /*  Default: now */
                               ) {
//



  this->Clear();

  ND::TDbiDatabaseManager& tablePR = ND::TDbiDatabaseManager::Instance();
  if ( ! tablePR.GetCascader().TableExists(tableName) ) {
       DbiSevere( "Cannot create packet - table " << tableName
			   << " does not exist." << "  ");
    fNumErrors = 1;
    return;
  }

  fTableName = tableName;


  // Create a ND::TDbiValidityRec from the supplied data.
  ND::TDbiValidityRec vrec(vr,task,aggNo,0);

  //  Create a ND::TDbiOutRowStream that can serialise this validity record
  ND::TDbiConfigSet dummy;     // For validity row any ND::TDbiTableRow will do.
  const ND::TDbiTableMetaData&  metaValid = tablePR.GetTableProxy(tableName,&dummy)
                                              .GetMetaValid();
  ND::TDbiOutRowStream buff(&metaValid);

  vrec.Store(buff,0);
  this->AddRow(buff.GetCSV());
  this->SetCreationDate(creationDate);

}

//.....................................................................
///\verbatim
///  Purpose:  Report and count errors.
///
///  Arguments:
///    msg          in    Message to be reported.
///    lineNum      in    Line number.
///    line         in    Input line causing error.
///
///  Return:
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Reset object, report and count errors.
///\endverbatim
void ND::TDbiSqlValPacket::Report(const char* msg,
                             UInt_t lineNum,
                             const string& line) {
//

//  Program Notes:-
//  =============

//  None.

     DbiSevere( " on line " << lineNum
			 <<":- \n  " << line << "  ");
  this->Reset();
  ++fNumErrors;

}
//.....................................................................
///  Purpose:  Clear out back to unfilled status except for fNumErrors.

void ND::TDbiSqlValPacket::Reset() {
//
//
//
  fSeqNo       = 0;
  fSqlMySqlMetaMain = "";
  fSqlMySqlMetaVld  = "";
  fSqlStmts.clear();
  fNumStmts    = 0;
  fTableName   = "";

}
//.....................................................................
/// Purpose:  Set creation date.
void ND::TDbiSqlValPacket::SetCreationDate(ND::TVldTimeStamp ts) {
//
//
//
  fCreationDate = ts;

  //  Update the validity row assuming:  "...,'creationdate',insertdate);"
  if ( this->GetNumSqlStmts() == 0 ) return;

  string& vldRow = *fSqlStmts.begin();
  string::size_type locEnd = vldRow.rfind(',');
  if ( locEnd == string::npos ) return;
  locEnd -=2;
  string::size_type locStart = vldRow.rfind(',',locEnd);
  if ( locStart == string::npos ) return;
  locStart+=2;
  vldRow.replace(locStart,locEnd-locStart+1,ts.AsString("s"));

}
//.....................................................................
/// Set EPOCH
void ND::TDbiSqlValPacket::SetEpoch(UInt_t epoch) {
//
//
//  Purpose:  Set EPOCH

  //  Update the validity row assuming:  "(seqno,timestart,timeend,epoch,....)
  if ( this->GetNumSqlStmts() == 0 ) return;

  string& vldRow = *fSqlStmts.begin();
  string::size_type locStart = 0;
  for (int field = 0; field < 3; ++field) {
    locStart = vldRow.find(',',locStart+1);
    if ( locStart == string::npos ) return;
  }
  string::size_type locEnd = vldRow.find(',',locStart+1);
  if ( locEnd == string::npos ) return;
  locEnd  -=1;
  locStart+=1;
  ostringstream epoch_str;
  epoch_str << epoch;
  vldRow.replace(locStart,locEnd-locStart+1,epoch_str.str());

}

//.....................................................................
///  Purpose:  Set up meta-data as SQL table creation statements.

void ND::TDbiSqlValPacket::SetMetaData() const {
//
//
//
  ND::TDbiDatabaseManager& tbprxreg = ND::TDbiDatabaseManager::Instance();

  //  Locate the table in the cascade.
  ND::TDbiCascader& cas = tbprxreg.GetCascader();
  Int_t dbNo = cas.GetTableDbNo(this->GetTableName());
  if ( dbNo < 0 ) return;

  //  Any table proxy will do to get the meta-data so use the one for a
  //  ND::TDbiConfigSet;
  ND::TDbiConfigSet dummy;
  const ND::TDbiTableMetaData & metaVld =  tbprxreg.GetTableProxy(this->GetTableName(),&dummy)
                                     .GetMetaValid();
  const ND::TDbiTableMetaData & metaMain = tbprxreg.GetTableProxy(this->GetTableName(),&dummy)
                                     .GetMetaData();
  fSqlMySqlMetaVld   = metaVld.Sql();
  fSqlMySqlMetaMain  = metaMain.Sql();

}

//.....................................................................
/// Set sequence number
void ND::TDbiSqlValPacket::SetSeqNo(UInt_t seqno) {
//
//
//  Purpose:  Set Sequence number.

  fSeqNo = seqno;

  //  Update all rows
  if ( this->GetNumSqlStmts() == 0 ) return;

  ostringstream tmp;
  tmp << seqno;
  const string seqnoStr = tmp.str();

  std::list<string>::iterator itr    = fSqlStmts.begin();
  std::list<string>::iterator itrEnd = fSqlStmts.end();
  for (; itr != itrEnd; ++itr) SetSeqNoOnRow(*itr,seqnoStr);


}

//.....................................................................
///  Purpose:  Set Sequence number on supplied row
void ND::TDbiSqlValPacket::SetSeqNoOnRow(string& row,const string& seqno) {
//
//

//  Update row assuming:  "...(seqno, ...."

  string::size_type locStart = row.find('(');
  if ( locStart == string::npos ) return;
  ++locStart;
  string::size_type locEnd = row.find(',',locStart);
  if ( locEnd == string::npos ) return;
  row.replace(locStart,locEnd-locStart,seqno);

}

//.....................................................................
///\verbatim
///
///  Purpose:  Output validity packet to specified database.
///
///  Arguments:
///    dbNo         in    Number of database in the cascade.
///    replace      in    If true replace existing SeqNo (default: kFALSE).
///
///  Return:    kTRUE if successfully stored.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Output validity packet to specified database modifying
///    InsertDate to be current date.
///\endverbatim
Bool_t ND::TDbiSqlValPacket::Store(UInt_t dbNo, Bool_t replace) const {


  if ( ! CanBeStored() ) return kFALSE;

  //Just use any old table row object just to get a ND::TDbiDBProxy.
  ND::TDbiConfigSet pet;
  ND::TDbiTableProxy& tp =  ND::TDbiDatabaseManager::Instance()
                      .GetTableProxy(this->GetTableName(),&pet);
  if ( replace ) {
    const ND::TDbiDBProxy & proxy = tp.GetDBProxy();
    if ( ! proxy.RemoveSeqNo(this->GetSeqNo(),dbNo) ) return kFALSE;
  }

  // Locate required ND::TDbiStatement.
  auto_ptr<ND::TDbiStatement> stmtDb(ND::TDbiDatabaseManager::Instance()
                               .GetCascader()
                               .CreateStatement(dbNo));
  if ( ! stmtDb.get() ) {
    DbiWarn(  "Attempting to write to non-existant cascade entry " << dbNo
      << "  ");
    return kFALSE;
  }


  // Loop processing all SQL statements
  Bool_t first = kTRUE;
  int combineInserts = 0;
  int maxInserts = 20;
  string sqlInserts;

  for (list<string>::const_iterator itr = fSqlStmts.begin();
       itr != fSqlStmts.end();
       ++itr) {
    if ( first ) {
//    On first statement replace InsertDate by current datetime.
      string sql = *itr;
      list<string>::size_type locDate = sql.rfind(",\'");
      if ( locDate !=  string::npos ) {
        ND::TVldTimeStamp now;
        sql.replace(locDate+2,19,TDbi::MakeDateTimeString(now));
      }
      stmtDb->ExecuteUpdate(sql.c_str());
      if ( stmtDb->PrintExceptions() ) return kFALSE;
      first = kFALSE;
      continue;
    }

    string sql = *itr;

//  Reduce database I/O by combining groups of insert commands.

    string::size_type insertIndex = sql.find("VALUES (");
    if ( insertIndex == string::npos) {
         DbiSevere( "Unexpected SQL : " << sql
			     << "\n  should be of form INSERT INTO ... VALUES (...);" << "  ");
      return kFALSE;
    }
    ++combineInserts;
    if ( combineInserts == 1 ) sqlInserts = sql;
    else {
      sqlInserts[sqlInserts.size()-1] = ',';
      sqlInserts += sql.substr(insertIndex+7);
    }
    if ( combineInserts >= maxInserts ) {
      stmtDb->ExecuteUpdate(sqlInserts.c_str());
      if ( stmtDb->PrintExceptions() ) return kFALSE;
      combineInserts = 0;
    }
  }

// Deal with last group of inserts.
  if ( combineInserts ) {
    stmtDb->ExecuteUpdate(sqlInserts.c_str());
    combineInserts = 0;
    if ( stmtDb->PrintExceptions() ) return kFALSE;
  }

  return kTRUE;

}
//.....................................................................
//
///\verbatim
///  Purpose:  Export to an iostream.
///
///  Arguments:
///     ios          in/out    Output stream.
///     addMetadata  in        if kTRUE, output SQL metadata (in form
///                            of a create table SQL)
///
///  Return:    kTRUE is I/O successful.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Export to an iostream.
///\endverbatim
Bool_t ND::TDbiSqlValPacket::Write(std::ofstream& ios,
                              Bool_t addMetadata) const {

//  Program Notes:-
//  =============

//  None.

  if ( ! CanBeStored() ) return kFALSE;
  if ( addMetadata ) {
    if ( fSqlMySqlMetaMain.size() == 0 ) this->SetMetaData();
    if ( fSqlMySqlMetaMain.size() == 0 ) {
      DbiWarn(  "Cannot write metadata; no associated ND::TDbiTableProxy "
      << "  ");
    }
    else {


      ios << ">>>>>" << GetTableName() << " Metadata [MySQL]" << endl;
      ios << fSqlMySqlMetaVld   << endl;
      ios << fSqlMySqlMetaMain  << endl;
      ios << "<<<<<" << GetTableName() << " Metadata" << endl;
    }
  }

  ios << ">>>>>" << GetTableName() << "  " << GetSeqNo() << endl;

  for ( list<string>::const_iterator itr = fSqlStmts.begin();
        itr != fSqlStmts.end();
        ++itr) ios << (*itr) << endl;

  ios << "<<<<<" << GetTableName() << "  " << GetSeqNo() << endl;

  return kTRUE;

}



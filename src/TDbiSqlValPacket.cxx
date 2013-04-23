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
#include <MsgFormat.hxx>
#include "UtilString.hxx"
#include "TVldRange.hxx"

ClassImp(CP::TDbiSqlValPacket)

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

static bool compStringPtrs(const std::string* str1, const std::string* str2 ) {
  return *str1 < *str2; }

//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................
/// Default constructor.
CP::TDbiSqlValPacket::TDbiSqlValPacket() :
fNumErrors(0),
fSeqNo(0),
fNumStmts(0)
{
//
//
//  Purpose:  Default ctor.


  DbiTrace( "Creating CP::TDbiSqlValPacket" << "  ");
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
///  o  Create CP::TDbiSqlValPacket and fill from input stream by calling Fill .
///\endverbatim
CP::TDbiSqlValPacket::TDbiSqlValPacket(std::ifstream& is) :
fNumErrors(0),
fSeqNo(0),
fNumStmts(0)
{


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Creating CP::TDbiSqlValPacket" << "  ");

  Fill(is);

}
//.....................................................................
///\verbatim
///
///  Purpose:  Constructor from a CP::TDbiValidityRec.
///
///  Arguments:
///      vrec  in   CP::TDbiValidityRec from which to create packet.     .
///
///  Return:   n/a.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o  Create CP::TDbiSqlValPacket from CP::TDbiValidityRec and associated data.
///
///
///  Program Notes:-
///  =============
///
///  This member function uses CP::TDbiConfigSet as a generic concrete
///  CP::TDbiTableRow that can be used to create SQL for any type of table.
///\endverbatim
CP::TDbiSqlValPacket::TDbiSqlValPacket(const CP::TDbiValidityRec& vrec) :
fNumErrors(0),
fSeqNo(vrec.GetSeqNo()),
fNumStmts(0),
fTableName(vrec.GetTableProxy()->GetTableName()),
fCreationDate(vrec.GetCreationDate())
{


  DbiTrace( "Creating CP::TDbiSqlValPacket" << "  ");

  const CP::TDbiTableProxy& tableProxy = *vrec.GetTableProxy();
  Int_t seqNo  = vrec.GetSeqNo();
  UInt_t dbNo  = vrec.GetDbNo();

  // Create the SQL for the CP::TDbiValidityRec itself.
  this->AddRow(tableProxy,0,vrec);

  // Create the SQL for the rows.

  const CP::TDbiDBProxy& dbProxy = tableProxy.GetDBProxy();
  CP::TDbiInRowStream* rset = dbProxy.QuerySeqNo(seqNo,dbNo);


  for(; ! rset->IsExhausted(); rset->FetchRow()) {
    std::string str;
    rset->RowAsCsv(str);
    this->AddRow(str);
  }
  delete rset;
  rset = 0;
}

//.....................................................................

CP::TDbiSqlValPacket::~TDbiSqlValPacket() {
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
//  o  Destroy CP::TDbiSqlValPacket.


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Destroying CP::TDbiSqlValPacket" << "  ");

  Clear();

}

//.....................................................................
/// Add data row tblProxy vrec row to this object.
Bool_t CP::TDbiSqlValPacket::AddDataRow(const CP::TDbiTableProxy& tblProxy,
                                   const CP::TDbiValidityRec* vrec,
                                   const CP::TDbiTableRow& row){
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
void CP::TDbiSqlValPacket::AddRow(const std::string & row){
//
//
//  Purpose: Add row.
//

  std::string sql("INSERT INTO ");
  sql += this->GetTableName();
  if ( this->GetNumSqlStmts() == 0 ) sql += "VLD";
  sql += " VALUES (" + row + ");";
  std::ostringstream seqno;
  seqno << this->GetSeqNo();
  this->SetSeqNoOnRow(sql,seqno.str());
  fSqlStmts.push_back(sql);
  ++fNumStmts;

}

//.....................................................................

Bool_t CP::TDbiSqlValPacket::AddRow(const CP::TDbiTableProxy& tblProxy,
                               const CP::TDbiValidityRec* vrec,
                               const CP::TDbiTableRow& row){
//
//
//  Purpose: Add row.
//


  bool isVld = this->GetNumSqlStmts() == 0;
  const CP::TDbiTableMetaData& meta = isVld ? tblProxy.GetMetaValid() : tblProxy.GetMetaData();
  CP::TDbiOutRowStream outRow(&meta);

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
///  Purpose:  Compare to another CP::TDbiSqlValPacket
///
///  Arguments:
///    that         in    The other CP::TDbiSqlValPacket to be compared.
///    log          in    If true list differences to MSG("CP::TDbi",kInfo)
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
CP::TDbiSqlValPacket::CompResult_t CP::TDbiSqlValPacket::Compare(
                                const CP::TDbiSqlValPacket& that,
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
///    CP::TDbiTableProxy.
///\endverbatim
Bool_t CP::TDbiSqlValPacket::CreateTable(UInt_t dbNo) const {

//  Program Notes:-
//  =============

//  None.

  if ( ! CanBeStored() ) return kFALSE;

  // Locate required CP::TDbiStatement.
  std::auto_ptr<CP::TDbiStatement> stmtDb(CP::TDbiDatabaseManager::Instance()
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

  CP::TDbiDatabaseManager::Instance().RefreshMetaData(this->GetTableName());

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

Bool_t CP::TDbiSqlValPacket::Fill(std::ifstream& is) {

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

  std::string nameHead;
  std::string nameTrail;
  UInt_t seqNoHead  = 0;
  UInt_t seqNoTrail = 0;

  std::string line;
  std::string msg;
  std::string sql;
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
        if ( line.find("Metadata") != std::string::npos ) {
          getline(is,fSqlMySqlMetaVld);
          ++lineNum;
          getline(is,fSqlMySqlMetaMain);
          ++lineNum;
          getline(is,line);
          ++lineNum;
          if (   line.substr(0,5) != "<<<<<"
              || line.find("Metadata") == std::string::npos ) {
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
        std::istringstream istr(line.substr(5));
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
            std::istringstream istr(line.substr(5));
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
            std::string date = this->GetStmtValues(0)[9];
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
std::string CP::TDbiSqlValPacket::GetStmt(UInt_t stmtNo) const {
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
std::vector<std::string> CP::TDbiSqlValPacket::GetStmtValues(UInt_t stmtNo) const {


  std::vector<std::string> vec;
  if ( stmtNo >= this->GetNumSqlStmts() ) return vec;

  // Locate statement
  std::string str = this->GetStmt(stmtNo);

  // Extract ...(list-of-values)... and parse it into tokens.
  std::string::size_type pos = str.find('(');
  ++pos;
  std::string::size_type n = str.find(')') - pos;
  CP::UtilString::StringTok(vec,str.substr(pos,n),",");

  return vec;

}

//.....................................................................

///\verbatim
///  Purpose:  Test for equality to another CP::TDbiSqlValPacket
///
///  Arguments:
///    that         in    The other CP::TDbiSqlValPacket to be compared.
///    log          in    If true list differences to MSG("CP::TDbi",kInfo)
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
///  o Compare to another CP::TDbiSqlValPacket.
///
///  Program Notes:-
///  =============
///
///  The InsertDate reflects the date when the data was written into the
/// local database so has to be excluded from the comparison.
///\endverbatim

Bool_t CP::TDbiSqlValPacket::IsEqual(const CP::TDbiSqlValPacket& that,
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
	     << that.fNumStmts << std::endl;
    return kFALSE;
  }

  std::list<std::string>::const_iterator itrThisBegin = fSqlStmts.begin();
  std::list<std::string>::const_iterator itrThisEnd   = fSqlStmts.end();
  std::list<std::string>::const_iterator itrThatBegin = that.fSqlStmts.begin();
  std::list<std::string>::const_iterator itrThatEnd   = that.fSqlStmts.end();

  std::list<std::string>::const_iterator itrThis = itrThisBegin;
  std::list<std::string>::const_iterator itrThat = itrThatBegin;

  Bool_t isEqual = kTRUE;

  // Strip off InsertDate from first statement (assume its
  // the last parameter in list).

  std::string strThis = (*itrThis).substr(0,(*itrThis).rfind(','));
  std::string strThat = (*itrThat).substr(0,(*itrThat).rfind(','));
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
// 	 << "::" << *itrThat << std::endl;
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
// 	 << "::" << *itrThat << std::endl;
    ++itrThis;
    --itrThat;
  }
  if ( itrThis == itrThisEnd ) return isEqual;

  // O.K., we are out of luck so set up pointers to both sets
  // and sort these.

  typedef std::vector<const std::string*>      shadow_list_t;
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
void CP::TDbiSqlValPacket::Print(Option_t * /* option */) const {
//
//
//  Purpose:  Print the current state.

  DbiInfo(  "CP::TDbiSQLValPacket:"
    << " table \"" << fTableName << "\" "
    << " SeqNo " << fSeqNo
    << " NumErrors " << fNumErrors
    << "  "
    << "   CreationDate " << fCreationDate
    << "  ");

  DbiInfo(  "   MySQL Main table creation: \"" << fSqlMySqlMetaMain << "\"" << "  ");

  DbiInfo(  "   MySQL VLD table creation: \"" << fSqlMySqlMetaVld << "\"" << "  ");

  if ( GetNumSqlStmts() > 0 ) {
    std::list<std::string>::const_iterator itr    = fSqlStmts.begin();
    std::list<std::string>::const_iterator itrEnd = fSqlStmts.end();
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
void CP::TDbiSqlValPacket::Recreate(const std::string& tableName,
                               const CP::TVldRange& vr,
                               Int_t aggNo,
                               TDbi::Task task,             /*  Default: 0 */
                               CP::TVldTimeStamp creationDate   /*  Default: now */
                               ) {
//



  this->Clear();

  CP::TDbiDatabaseManager& tablePR = CP::TDbiDatabaseManager::Instance();
  if ( ! tablePR.GetCascader().TableExists(tableName) ) {
       DbiSevere( "Cannot create packet - table " << tableName
			   << " does not exist." << "  ");
    fNumErrors = 1;
    return;
  }

  fTableName = tableName;


  // Create a CP::TDbiValidityRec from the supplied data.
  CP::TDbiValidityRec vrec(vr,task,aggNo,0);

  //  Create a CP::TDbiOutRowStream that can serialise this validity record
  CP::TDbiConfigSet dummy;     // For validity row any CP::TDbiTableRow will do.
  const CP::TDbiTableMetaData&  metaValid = tablePR.GetTableProxy(tableName,&dummy)
                                              .GetMetaValid();
  CP::TDbiOutRowStream buff(&metaValid);

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
void CP::TDbiSqlValPacket::Report(const char* msg,
                             UInt_t lineNum,
                             const std::string& line) {
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

void CP::TDbiSqlValPacket::Reset() {
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
void CP::TDbiSqlValPacket::SetCreationDate(CP::TVldTimeStamp ts) {
//
//
//
  fCreationDate = ts;

  //  Update the validity row assuming:  "...,'creationdate',insertdate);"
  if ( this->GetNumSqlStmts() == 0 ) return;

  std::string& vldRow = *fSqlStmts.begin();
  std::string::size_type locEnd = vldRow.rfind(',');
  if ( locEnd == std::string::npos ) return;
  locEnd -=2;
  std::string::size_type locStart = vldRow.rfind(',',locEnd);
  if ( locStart == std::string::npos ) return;
  locStart+=2;
  vldRow.replace(locStart,locEnd-locStart+1,ts.AsString("s"));

}
//.....................................................................
/// Set EPOCH
void CP::TDbiSqlValPacket::SetEpoch(UInt_t epoch) {
//
//
//  Purpose:  Set EPOCH

  //  Update the validity row assuming:  "(seqno,timestart,timeend,epoch,....)
  if ( this->GetNumSqlStmts() == 0 ) return;

  std::string& vldRow = *fSqlStmts.begin();
  std::string::size_type locStart = 0;
  for (int field = 0; field < 3; ++field) {
    locStart = vldRow.find(',',locStart+1);
    if ( locStart == std::string::npos ) return;
  }
  std::string::size_type locEnd = vldRow.find(',',locStart+1);
  if ( locEnd == std::string::npos ) return;
  locEnd  -=1;
  locStart+=1;
  std::ostringstream epoch_str;
  epoch_str << epoch;
  vldRow.replace(locStart,locEnd-locStart+1,epoch_str.str());

}

//.....................................................................
///  Purpose:  Set up meta-data as SQL table creation statements.

void CP::TDbiSqlValPacket::SetMetaData() const {
//
//
//
  CP::TDbiDatabaseManager& tbprxreg = CP::TDbiDatabaseManager::Instance();

  //  Locate the table in the cascade.
  CP::TDbiCascader& cas = tbprxreg.GetCascader();
  Int_t dbNo = cas.GetTableDbNo(this->GetTableName());
  if ( dbNo < 0 ) return;

  //  Any table proxy will do to get the meta-data so use the one for a
  //  CP::TDbiConfigSet;
  CP::TDbiConfigSet dummy;
  const CP::TDbiTableMetaData & metaVld =  tbprxreg.GetTableProxy(this->GetTableName(),&dummy)
                                     .GetMetaValid();
  const CP::TDbiTableMetaData & metaMain = tbprxreg.GetTableProxy(this->GetTableName(),&dummy)
                                     .GetMetaData();
  fSqlMySqlMetaVld   = metaVld.Sql();
  fSqlMySqlMetaMain  = metaMain.Sql();

}

//.....................................................................
/// Set sequence number
void CP::TDbiSqlValPacket::SetSeqNo(UInt_t seqno) {
//
//
//  Purpose:  Set Sequence number.

  fSeqNo = seqno;

  //  Update all rows
  if ( this->GetNumSqlStmts() == 0 ) return;

  std::ostringstream tmp;
  tmp << seqno;
  const std::string seqnoStr = tmp.str();

  std::list<std::string>::iterator itr    = fSqlStmts.begin();
  std::list<std::string>::iterator itrEnd = fSqlStmts.end();
  for (; itr != itrEnd; ++itr) SetSeqNoOnRow(*itr,seqnoStr);


}

//.....................................................................
///  Purpose:  Set Sequence number on supplied row
void CP::TDbiSqlValPacket::SetSeqNoOnRow(std::string& row,const std::string& seqno) {
//
//

//  Update row assuming:  "...(seqno, ...."

  std::string::size_type locStart = row.find('(');
  if ( locStart == std::string::npos ) return;
  ++locStart;
  std::string::size_type locEnd = row.find(',',locStart);
  if ( locEnd == std::string::npos ) return;
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
Bool_t CP::TDbiSqlValPacket::Store(UInt_t dbNo, Bool_t replace) const {


  if ( ! CanBeStored() ) return kFALSE;

  //Just use any old table row object just to get a CP::TDbiDBProxy.
  CP::TDbiConfigSet pet;
  CP::TDbiTableProxy& tp =  CP::TDbiDatabaseManager::Instance()
                      .GetTableProxy(this->GetTableName(),&pet);
  if ( replace ) {
    const CP::TDbiDBProxy & proxy = tp.GetDBProxy();
    if ( ! proxy.RemoveSeqNo(this->GetSeqNo(),dbNo) ) return kFALSE;
  }

  // Locate required CP::TDbiStatement.
  std::auto_ptr<CP::TDbiStatement> stmtDb(CP::TDbiDatabaseManager::Instance()
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
  std::string sqlInserts;

  for (std::list<std::string>::const_iterator itr = fSqlStmts.begin();
       itr != fSqlStmts.end();
       ++itr) {
    if ( first ) {
//    On first statement replace InsertDate by current datetime.
      std::string sql = *itr;
      std::list<std::string>::size_type locDate = sql.rfind(",\'");
      if ( locDate !=  std::string::npos ) {
        CP::TVldTimeStamp now;
        sql.replace(locDate+2,19,TDbi::MakeDateTimeString(now));
      }
      stmtDb->ExecuteUpdate(sql.c_str());
      if ( stmtDb->PrintExceptions() ) return kFALSE;
      first = kFALSE;
      continue;
    }

    std::string sql = *itr;

//  Reduce database I/O by combining groups of insert commands.

    std::string::size_type insertIndex = sql.find("VALUES (");
    if ( insertIndex == std::string::npos) {
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
Bool_t CP::TDbiSqlValPacket::Write(std::ofstream& ios,
                              Bool_t addMetadata) const {

//  Program Notes:-
//  =============

//  None.

  if ( ! CanBeStored() ) return kFALSE;
  if ( addMetadata ) {
    if ( fSqlMySqlMetaMain.size() == 0 ) this->SetMetaData();
    if ( fSqlMySqlMetaMain.size() == 0 ) {
      DbiWarn(  "Cannot write metadata; no associated CP::TDbiTableProxy "
      << "  ");
    }
    else {


        ios << ">>>>>" << GetTableName() << " Metadata [MySQL]" << std::endl;
      ios << fSqlMySqlMetaVld   << std::endl;
      ios << fSqlMySqlMetaMain  << std::endl;
      ios << "<<<<<" << GetTableName() << " Metadata" << std::endl;
    }
  }

  ios << ">>>>>" << GetTableName() << "  " << GetSeqNo() << std::endl;

  for ( std::list<std::string>::const_iterator itr = fSqlStmts.begin();
        itr != fSqlStmts.end();
        ++itr) ios << (*itr) << std::endl;

  ios << "<<<<<" << GetTableName() << "  " << GetSeqNo() << std::endl;

  return kTRUE;

}



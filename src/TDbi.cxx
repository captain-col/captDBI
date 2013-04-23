///////////////////////////////////////////////////////////////////////
// $Id: TDbi.cxx,v 1.2 2012/06/14 10:55:22 finch Exp $
//
// CP::TDbi
//
// N. West 12/2000
//
// Purpose: CP::TDbi consists entirely of static data and member functions
//          used for package interface.
//
// General Remarks about CP::TDbi
// =========================
//
// This primary purposes of this package are:-
//
//  o  To test out ideas.
//
//  o  To serve as an interim DBI for CalDet in 2001..
//
// Internal Structure
// ==================
//
//  Layered as follows, Higher levels can call those in lower levels
//  Names on the same layer cannot call each other.  The CP::TDbi prefix
//  is omitted on all but CP::TDbi itself.
//
//  Validate
//  ConfigStream
//  Writer
//  LogEntry
//  ResultPtr
//  SqlValPacket
//  ValRecSet
//  TableProxyTDbiRegistry
//  TableProxy
//  ResultAgg
//  ValidityRecBuilder
//  ResultNonAgg
//  Cache  DBProxy
//  Result
//  Record
//  ResultKey
//  ConfigSet ValidityRec  DemoData1  DemoData2 DemoData3
//  BinaryFile
//  TableRow
//  ResultSet OutRowStream
//  RowStream
//  ConnectionMaintainer
//  Cascader
//  SimFlagAssociation
//  Statement
//  Connection
//  TableMetaData
//  RollbackDates FieldType
//  CP::TDbi String Services
//  CP::TDbiAsciiDbImporter
//  CP::TDbiAsciiTablePreparer
//  ExceptionLog
//  Exception
//
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <sstream>

#include "TList.h"

#include "DbiDetector.hxx"
#include "DbiSimFlag.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;
using std::string;
#include "TDbi.hxx"
#include "TDbiException.hxx"
#include "TDbiExceptionLog.hxx"


//   Definition of static data members
//   *********************************


static std::map<std::string,Int_t> fgTimegateTable;

// Definition of member functions (alphabetical order)
// ***************************************************

//.....................................................................

Int_t TDbi::GetTimeGate(const std::string& tableName) {

//  Purpose: Get a suitable time gate for table.

//  Program Notes:-
//  =============

//  A time gate is applied to all primary (validity) queries to save time
//  and ensure that gaps are not missed. Ideally the gate needs to be
//  of order of a typical validity range.  Much longer and unneeded
//  VLD records are processed, much shorter and the query results will
//  expire too soon.  Both increase I/O.
//
//  The system provides a default for each table and CP::TDbiValidityRecBuilder
//  updates it if grossly wrong.

  // Set default if looking up table for the first time.
  std::map<std::string,Int_t>::iterator
      tablePtr = fgTimegateTable.find(tableName);
  if ( tablePtr == fgTimegateTable.end()
     ) TDbi::SetTimeGate(tableName,10*24*60*60);

  DbiDebug( "Returning time gate " << fgTimegateTable[tableName]
                          << " for " << tableName << "  ");
  return fgTimegateTable[tableName];

}

//.....................................................................

std::string TDbi::GetVldDescr(const char* tableName,
                             Bool_t isTemporary /*= false*/) {
//
//
//  Purpose:  Return string "create [temporary] table tableNameXXX" to make table xxxVLD
//
//  Program Notes:-
//  =============

//  If the format of Validity Tables is changed then this function must
//  be updated as that format is hardwired into an SQL CREATE TABLE
//  command in this function.

  string sql;
  sql += "create ";
  if ( isTemporary ) sql += "temporary ";
  sql += "table ";
  sql += tableName;
  sql += "VLD ( ";
  sql += "  SEQNO integer not null primary key,";
  sql += "  TIMESTART datetime not null,";
  sql += "  TIMEEND datetime not null,";
  sql += "  EPOCH tinyint,";
  sql += "  REALITY tinyint,";
  sql += "  DETECTORMASK tinyint,";
  sql += "  SIMMASK tinyint,";
  sql += "  TASK integer,";
  sql += "  AGGREGATENO integer,";
  sql += "  CREATIONDATE datetime not null,";
  sql += "  INSERTDATE datetime not null ) ";
  return sql;
}

//.....................................................................

string TDbi::MakeDateTimeString(const CP::TVldTimeStamp& timeStamp) {
//
//
//  Purpose: Convert CP::TVldTimeStamp to SQL DateTime string.
//
//  Arguments:
//    timeStamp    in    CP::TVldTimeStamp to be converted.
//
//  Return: SQL DateTime string corresponding to CP::TVldTimeStamp.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Return SQL DateTime string corresponding to CP::TVldTimeStamp.

//  Program Notes:-
//  =============

//  The format of a  SQL DateTime string is:-
//
//            YYYY-MM-DD HH:MM:SS
//    e.g.    2001-01-03 00:00:00
//            0123456789012345678901234567890123"

 return timeStamp.AsString("s");

}
//.....................................................................

CP::TVldTimeStamp TDbi::MakeTimeStamp(const std::string& sqlDateTime,
                                Bool_t* ok) {
//
//
//  Purpose:  Convert SQL DateTime string to CP::TVldTimeStamp.
//
//  Arguments:
//    sqlDateTime  in   SQL DateTime string to be convered.
//    ok           in    Optional return flag
//                       If not supplied, report illegal dates
//
//  Return:  CP::TVldTimeStamp corresponding to SQL DateTime string.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o  Return CP::TVldTimeStamp corresponding to SQL DateTime string.

//  Program Notes:-
//  =============

//  Note that there is only white space between day and hour
//  so no need for dummy separator.

  struct date {
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;};
  char dummy;

  static string lo = "1970-01-01 00:00:00";
  static string hi = "2038-01-19 03:14:07";

  // Set up defaults from 0:0am today.
  CP::TVldTimeStamp nowTS;
  int nowDate = nowTS.GetDate();
  date defaultDate = {nowDate/10000, nowDate/100%100, nowDate%100,0,0,0};
  date input       = defaultDate;

  std::istringstream in(sqlDateTime);
  in >> input.year >> dummy >> input.month >> dummy >> input.day
     >> input.hour >> dummy >> input.min   >> dummy >> input.sec;

  if ( ok ) *ok = kTRUE;
  if (  sqlDateTime < lo || sqlDateTime > hi ) {
    if ( ok ) *ok = kFALSE;
    else {
      static int bad_date_count = 0;
      if ( ++bad_date_count <= 20 ) {
	const char* last = (bad_date_count == 20) ? "..Last Message.. " : "";
        DbiSevere(  "Bad date string: " << sqlDateTime
			    << " parsed as "
			    << input.year  << " "
			    << input.month << " "
			    << input.day   << " "
			    << input.hour  << " "
			    << input.min   << " "
			    << input.sec
			    << "\n    Outside range " << lo
			    << " to " << hi << last << "  ");
      }
    }

    input = defaultDate;
  }

  return CP::TVldTimeStamp(input.year,input.month,input.day,
                      input.hour,input.min,input.sec);

}
//.....................................................................

Bool_t TDbi::NotGlobalSeqNo(UInt_t seqNo) {
  return seqNo <= kMAXLOCALSEQNO;
}


//.....................................................................

void TDbi::SetLogLevel(int level) {
//
//
//  Purpose:  Set MessageService log level for package.
//
//  Arguments:
//    level        in    Log level.
//
//  Return:    None.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o  Set MessageService log level for package.

//  Program Notes:-
//  =============

//  None.

 CP::TDbiLog::SetLogLevel((CP::TDbiLog::LogPriority)level);

}

//.....................................................................

void TDbi::SetTimeGate(const std::string& tableName, Int_t timeGate) {

//  Purpose: Set a time gate for table.

//  Program Notes:  See GetTimeGate.

  if ( timeGate > 15 && timeGate <= 100*24*60*60 ) {
    fgTimegateTable[tableName] = timeGate;
    DbiDebug( "Setting time gate " << timeGate
                            << " for " << tableName << "  ");
  }
  else {
    DbiWarn( "Ignoring  invalid time gate setting " << timeGate
                             << " for " << tableName << "  ");
  }
}

// ********************************************************************

/*    Template for New Member Function

//.....................................................................

TDbi:: {
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




//----------------------------------------------------------------------------------------------
//
// To run this demo:-
// 
// 1) Set up a database (say t2k_db) on a mysql server (say mysqlserver.t2k.com) 
//    with a write access account (say t2k_writer,t2k_writer_psw) using the 
//    following environmental variables:-
// 
//    export ENV_TSQL_URL="mysql://mysqlserver.t2k.com/t2k_db"
//    export ENV_TSQL_USER="t2k_writer"
//    export ENV_TSQL_PSWD="t2k_writer_psw"
//
// 2) Prime it:-
//
//    database_updater.py apply_local_update $OAOFFLINEDATABASEROOT/demo/demo_db_table.update
// 
// 3) Run the demo:-
// 
//    demooaOfflineDatabase.exe 
//
//----------------------------------------------------------------------------------------------


#include "TEventContext.hxx"
#include "TDbiLog.hxx"
#include "TDemo_DB_Table.hxx"
#include "TResultSetHandle.hxx"
#include "TTFBChannelId.hxx"

#include "DatabaseUtils.hxx"
#include <iostream>
#include <string>

int main() {

  // Can be boosted up to VerboseLevel / DebugLevel, see ND::TDbiLog documentation for more information
  ND::TDbiLog::SetLogLevel(ND::TDbiLog::LogLevel);
  ND::TDbiLog::SetDebugLevel(ND::TDbiLog::SevereLevel);

  // Examine the Database table DEMO_DB_TABLE every 5 days between the start
  // and end date.

  std::string start_date_time = "2008-12-30 00:00:00";
  std::string end_date_time   = "2009-03-01 00:00:00";

  Int_t start_unix_time = DateTimeToUnixTime(start_date_time);
  Int_t end_unix_time   = DateTimeToUnixTime(end_date_time);

  UInt_t required_id      = 2288517116UL;

  for (Int_t current_unix_time = start_unix_time; current_unix_time <= end_unix_time; current_unix_time += 5*24*60*60) {
    ND::TEventContext context;
    context.SetTimeStamp(current_unix_time);
    // Prevent context being treated as MC.
    context.SetPartition(0);  
    ND::TResultSetHandle<ND::TDemo_DB_Table> rs(context);
    Int_t numRows(rs.GetNumRows());
    if ( ! numRows )  DbiLog("Applying query at " << UnixTimeToDateTime(current_unix_time) << " ... failed to find any results.");
    else {
      DbiLog("Applying query at " << UnixTimeToDateTime(current_unix_time) << " ... result set contains " 
                                    << numRows << " rows as follows:-");
      for (Int_t irow = 0; irow<numRows; ++irow) rs.GetRow(irow)->Print();
      const ND::TDemo_DB_Table* required_row = rs.GetRowByIndex(required_id);
      if ( required_row ) {
	DbiLog("  required row " << ND::TTFBChannelId(required_id).AsString());
	required_row->Print();
      }
      else DbiLog("  cannot find required row" << ND::TTFBChannelId(required_id).AsString());      
    }
  }

  return 0;
}

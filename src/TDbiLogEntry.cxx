// $Id: TDbiLogEntry.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $

#include <iostream>
#include <sstream>

#include "TString.h"
#include "TSystem.h"
#include "TUrl.h"

#include "TDbi.hxx"
#include "TDbiCascader.hxx"
#include "TDbiLogEntry.hxx"
#include "TDbiOutRowStream.hxx"
#include "TDbiResultSetHandle.hxx"
#include "TDbiInRowStream.hxx"
#include "TDbiTableProxy.hxx"
#include "TDbiDatabaseManager.hxx"
#include "TDbiValidityRec.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;


ClassImp(ND::TDbiLogEntry)


//   Definition of static data members
//   *********************************


//  Instantiate associated Result Pointer and writer classes.
//  ********************************************************

#include "TDbiResultSetHandle.tpl"
template class  ND::TDbiResultSetHandle<ND::TDbiLogEntry>;

#include "TDbiWriter.tpl"
template class  ND::TDbiWriter<ND::TDbiLogEntry>;

//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

ND::TDbiLogEntry::TDbiLogEntry(const string& tableName, /* = "" */
                         const string& reason,    /* = "" */
	                 Int_t detMask,           /* = full mask */
	                 Int_t simMask,           /* = full mask */
                         TDbi::Task task,          /* = 0  */
                         Int_t logSeqNoMin,       /* = 0  */
                         Int_t logSeqNoMax,       /* = 0  */
                         Int_t logNumSeqNo):      /* = 0  */
fDbNo(0),
fSeqNo(0),
fLogTableName(tableName),
fLogDetMask(detMask),
fLogSimMask(simMask),
fLogTask(task),
fLogSeqNoMin(logSeqNoMin),
fLogSeqNoMax(logSeqNoMax),
fLogNumSeqNo(logNumSeqNo),
fUserName("unknown user"),
fProcessName(gProgName),
fHostName(gSystem->HostName())
{
//
//
//  Purpose:  Constructor


  if ( fLogSeqNoMax == 0 ) fLogSeqNoMax = logSeqNoMin;
  if ( fLogNumSeqNo == 0 && logSeqNoMin != 0
       ) fLogNumSeqNo = fLogSeqNoMax - fLogSeqNoMin +1;

  this->SetServerName();
  this->SetReason(reason);

  // Try to get username from the environment.
  const char* userName =  gSystem->Getenv("USER");
  if ( ! userName ) userName =  gSystem->Getenv("USERNAME");
  if ( ! userName ) userName =  gSystem->Getenv("LOGNAME");
  if ( userName ) fUserName = userName;

}
//.....................................................................

ND::TDbiLogEntry::~TDbiLogEntry() {
//
//
//  Purpose:  Destructor


}
//.....................................................................

std::ostream& operator<<(ostream& s, const ND::TDbiLogEntry& logEntry) {
//
//
//  Purpose:  Output Log Entry to message stream.
//
//  Arguments:
//    s            in    Message stream
//    logEntry        in    Configuration set to be output
//
//  Return:        Message stream
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Output configuration set to message stream.

//  Program Notes:-
//  =============

//  None.

  s << "ND::TDbiLogEntry: Table  " << logEntry.GetLogTableName();
  if ( logEntry.GetLogSeqNoMin() ==  logEntry.GetLogSeqNoMax() )
    s << " SEQNO: " << logEntry.GetLogSeqNoMin();
  else
     s << " SEQNO min: " << logEntry.GetLogSeqNoMin()
       << " SEQNO max: " << logEntry.GetLogSeqNoMax()
       << " No. SEQNOs: " << logEntry.GetLogNumSeqNo();
  s << " DetectorMask: " << logEntry.GetDetectorMask()
    << " SimMask: " << logEntry.GetSimMask()
    << " Task: " << logEntry.GetTask()
    << "\n Updated on " << logEntry.GetUpdateTime().AsString("s")
    << " by " << logEntry.GetUserName()
    << " running " << logEntry.GetProcessName()
    << " on " << logEntry.GetHostName()
    << " connected to " << logEntry.GetServerName()
    << "\n Reason for update:-\n " << logEntry.GetReason() << endl;

  return s;

}

//.....................................................................

void ND::TDbiLogEntry::Fill(ND::TDbiInRowStream& rs,
		       const ND::TDbiValidityRec* vrec) {
//
//
//  Purpose:  Fill oject from Result Set

  rs >> fLogTableName
     >> fLogSeqNoMin
     >> fLogSeqNoMax
     >> fLogNumSeqNo
     >> fUserName
     >> fProcessName
     >> fHostName
     >> fServerName
     >> fReason;
  fLogDetMask = fLogSimMask = fLogTask = 0;
  if ( vrec ) {
    fLogDetMask = vrec->GetVldRange().GetDetectorMask();
    fLogSimMask = vrec->GetVldRange().GetSimMask();
    fLogTask    = vrec->GetTask();
    fUpdateTime = vrec->GetVldRange().GetTimeStart();
  }
}

//.....................................................................

void ND::TDbiLogEntry::Recreate(const string& tableName, /* = "" */
                           const string& reason,    /* = "" */
	                   Int_t detMask,           /* = full mask */
	                   Int_t simMask,           /* = full mask */
                           TDbi::Task task,          /* = 0  */
                           Int_t logSeqNoMin,       /* = 0  */
                           Int_t logSeqNoMax,       /* = 0  */
                           Int_t logNumSeqNo)       /* = 0  */
{
//
//
//  Purpose:  Recreate, unless new state consistent with old.

  if (    fSeqNo > 0
       && ( tableName    == ""                       || tableName   == fLogTableName )
       && ( detMask      == ND::DbiDetector::FullMask()     || detMask     == fLogDetMask )
       && ( simMask      == ND::DbiSimFlag::FullMask()      || simMask     == fLogSimMask )
       && ( task         == 0                        || task        == fLogTask )
       && ( logSeqNoMin  == 0                        || logSeqNoMin ==  fLogSeqNoMin)
       && ( logSeqNoMax  == 0                        || logSeqNoMax == fLogSeqNoMax )
       && ( logNumSeqNo  == 0                        || logNumSeqNo == fLogNumSeqNo )
	  ) {
    if ( reason != "" )  this->SetReason(reason);
    return;
  }


  fDbNo = 0;
  fSeqNo =0;
  fLogTableName = tableName;
  fLogDetMask = detMask;
  fLogSimMask = simMask;
  fLogTask = task;
  fLogSeqNoMin = logSeqNoMin;
  fLogSeqNoMax = logSeqNoMax;
  fLogNumSeqNo = logNumSeqNo;
  this->SetReason(reason);

}

//.....................................................................

void ND::TDbiLogEntry::SetReason(const string& reason) {
//
//
//  Purpose: Set reason.
//           String can  be "@file-containing-reason" in which case
//           contents of file is used to define the reason excluding
//           any line begining:-
//
//               FIXUP-FILE:
//           or  BROADCAST:
//
//           This allows DMauto fix-up files to provide log entries as well
//           as updates.

  fReason = reason;

  // If fReason starts '@' treat remainder as file name
  // to be read into fReason.
  if ( fReason.size() && fReason[0] == '@' ) {
    string fileName(fReason,1);
    fReason.clear();
    ifstream reasonFile(fileName.c_str());
    if ( ! reasonFile.is_open() ) {
      DbiSevere( "Cannot read \"Reason File\" " << fileName << "  ");
    }
    else {
      string line;
      while ( ! reasonFile.eof() ) {
        getline(reasonFile,line);
	if (    line.substr(0,11) == "FIXUP-FILE:"
	     || line.substr(0,10) == "BROADCAST:" ) {
          continue;
	}
        if ( fReason.size() ) fReason += '\n';
        fReason += line;
      }
      reasonFile.close();
    }
  }
}

//.....................................................................

void ND::TDbiLogEntry::SetServerName() {
//
//
//  Purpose: Set DB Server name from cascade number.

  string urlStr = ND::TDbiDatabaseManager::Instance().GetCascader().GetURL(fDbNo);
  TUrl url(urlStr.c_str());
  fServerName = url.GetHost();

}
//.....................................................................

void ND::TDbiLogEntry::Store(ND::TDbiOutRowStream& ors,
                         const ND::TDbiValidityRec* /* vrec */) const {
//
//
//  Purpose:  Stream object to output row stream

 ors << fLogTableName
     << fLogSeqNoMin
     << fLogSeqNoMax
     << fLogNumSeqNo
     << fUserName
     << fProcessName
     << fHostName
     << fServerName
     << fReason;
}

//.....................................................................

Bool_t ND::TDbiLogEntry::Write(UInt_t dbNo,
                          Int_t logSeqNo)     /* =0 */ {
//
//
//  Purpose: Write, or rewrite this entry, adding logSeqNo to range of
//           update SEQNOs if non-zero.

  if ( logSeqNo > 0 ) {
    if ( fLogNumSeqNo == 0 ) {
      fLogSeqNoMin = logSeqNo;
      fLogSeqNoMax = logSeqNo;
    }
    if ( logSeqNo < fLogSeqNoMin ) fLogSeqNoMin = logSeqNo;
    if ( logSeqNo > fLogSeqNoMax ) fLogSeqNoMax = logSeqNo;
    ++fLogNumSeqNo;
  }

  if ( ! this->HasReason() || fLogNumSeqNo == 0) {
    DbiSevere( "Cannot write LogEntry - no reason and/or no SEQNOs defined "
			   << "  ");
    return kFALSE;
  }

  if ( fSeqNo != 0 && dbNo != fDbNo ) {
    DbiSevere( "Cannot write LogEntry - attempting to switch from database"
			   << fDbNo << " to " << dbNo << "  ");
    return kFALSE;
  }

  fDbNo = dbNo;
  ND::TDbiTableProxy& tblProxy = ND::TDbiDatabaseManager::Instance()
                            .GetTableProxy("DBILOGENTRY",this);
  bool replace = true;

  // Allocate SEQNO before first write.
  if ( fSeqNo == 0 ) {
    replace = false;
    int seqNo = tblProxy.GetCascader().AllocateSeqNo("DBILOGENTRY",0,fDbNo);
    if ( seqNo <= 0 ) {
         DbiSevere(  "Cannot get sequence number for table DBILOGENTRY" << "  ");
      return kFALSE;
    }
    fSeqNo = seqNo;
  }

  // Construct a ND::TDbiValidityRec.
  ND::TVldRange     vr(fLogDetMask,fLogSimMask,fUpdateTime,fUpdateTime,"ND::TDbiLogEntry");
  ND::TDbiValidityRec vrec(vr,fLogTask,-1,0);

  // Now build and output ND::TDbiSqlValPacket.
  ND::TDbiSqlValPacket packet;
  packet.Recreate("DBILOGENTRY",
                  vrec.GetVldRange(),
                  -1,
                  vrec.GetTask(),
                  vrec.GetCreationDate());
  packet.AddDataRow(tblProxy,0,*this);
  packet.SetSeqNo(fSeqNo);
  return packet.Store(fDbNo,replace);
;
}
/*    Template for New Member Function

//.....................................................................

ND::TDbiLogEntry:: {
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



// $Id: TDbiLogEntry.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $

#include "TDbi.hxx"
#include "TDbiCascader.hxx"
#include "TDbiLogEntry.hxx"
#include "TDbiOutRowStream.hxx"
#include "TDbiResultSetHandle.hxx"
#include "TDbiInRowStream.hxx"
#include "TDbiTableProxy.hxx"
#include "TDbiDatabaseManager.hxx"
#include "TDbiValidityRec.hxx"
#include "TDbiLog.hxx"
#include "MsgFormat.hxx"

#include <TString.h>
#include <TSystem.h>
#include <TUrl.h>

#include <iostream>
#include <sstream>

ClassImp(CP::TDbiLogEntry)


//   Definition of static data members
//   *********************************


//  Instantiate associated Result Pointer and writer classes.
//  ********************************************************

#include "TDbiResultSetHandle.tpl"
template class  CP::TDbiResultSetHandle<CP::TDbiLogEntry>;

#include "TDbiWriter.tpl"
template class  CP::TDbiWriter<CP::TDbiLogEntry>;

//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

CP::TDbiLogEntry::TDbiLogEntry(const std::string& tableName, /* = "" */
                               const std::string& reason,    /* = "" */
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
    fHostName(gSystem->HostName()) {
//
//
//  Purpose:  Constructor


    if (fLogSeqNoMax == 0) {
        fLogSeqNoMax = logSeqNoMin;
    }
    if (fLogNumSeqNo == 0 && logSeqNoMin != 0
       ) {
        fLogNumSeqNo = fLogSeqNoMax - fLogSeqNoMin +1;
    }

    this->SetServerName();
    this->SetReason(reason);

    // Try to get username from the environment.
    const char* userName =  gSystem->Getenv("USER");
    if (! userName) {
        userName =  gSystem->Getenv("USERNAME");
    }
    if (! userName) {
        userName =  gSystem->Getenv("LOGNAME");
    }
    if (userName) {
        fUserName = userName;
    }

}
//.....................................................................

CP::TDbiLogEntry::~TDbiLogEntry() {
//
//
//  Purpose:  Destructor


}
//.....................................................................

std::ostream& operator<<(std::ostream& s, const CP::TDbiLogEntry& logEntry) {
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

    s << "CP::TDbiLogEntry: Table  " << logEntry.GetLogTableName();
    if (logEntry.GetLogSeqNoMin() ==  logEntry.GetLogSeqNoMax()) {
        s << " SEQNO: " << logEntry.GetLogSeqNoMin();
    }
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
      << "\n Reason for update:-\n " << logEntry.GetReason() << std::endl;

    return s;

}

//.....................................................................

void CP::TDbiLogEntry::Fill(CP::TDbiInRowStream& rs,
                            const CP::TDbiValidityRec* vrec) {
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
    if (vrec) {
        fLogDetMask = vrec->GetVldRange().GetDetectorMask();
        fLogSimMask = vrec->GetVldRange().GetSimMask();
        fLogTask    = vrec->GetTask();
        fUpdateTime = vrec->GetVldRange().GetTimeStart();
    }
}

//.....................................................................

void CP::TDbiLogEntry::Recreate(const std::string& tableName, /* = "" */
                                const std::string& reason,    /* = "" */
                                Int_t detMask,           /* = full mask */
                                Int_t simMask,           /* = full mask */
                                TDbi::Task task,          /* = 0  */
                                Int_t logSeqNoMin,       /* = 0  */
                                Int_t logSeqNoMax,       /* = 0  */
                                Int_t logNumSeqNo) {     /* = 0  */
//
//
//  Purpose:  Recreate, unless new state consistent with old.

    if (fSeqNo > 0
        && (tableName    == ""                       || tableName   == fLogTableName)
        && (detMask      == CP::DbiDetector::FullMask()     || detMask     == fLogDetMask)
        && (simMask      == CP::DbiSimFlag::FullMask()      || simMask     == fLogSimMask)
        && (task         == 0                        || task        == fLogTask)
        && (logSeqNoMin  == 0                        || logSeqNoMin ==  fLogSeqNoMin)
        && (logSeqNoMax  == 0                        || logSeqNoMax == fLogSeqNoMax)
        && (logNumSeqNo  == 0                        || logNumSeqNo == fLogNumSeqNo)
       ) {
        if (reason != "") {
            this->SetReason(reason);
        }
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

void CP::TDbiLogEntry::SetReason(const std::string& reason) {
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
    if (fReason.size() && fReason[0] == '@') {
        std::string fileName(fReason,1);
        fReason.clear();
        std::ifstream reasonFile(fileName.c_str());
        if (! reasonFile.is_open()) {
            DbiSevere("Cannot read \"Reason File\" " << fileName << "  ");
        }
        else {
            std::string line;
            while (! reasonFile.eof()) {
                getline(reasonFile,line);
                if (line.substr(0,11) == "FIXUP-FILE:"
                    || line.substr(0,10) == "BROADCAST:") {
                    continue;
                }
                if (fReason.size()) {
                    fReason += '\n';
                }
                fReason += line;
            }
            reasonFile.close();
        }
    }
}

//.....................................................................

void CP::TDbiLogEntry::SetServerName() {
//
//
//  Purpose: Set DB Server name from cascade number.

    std::string urlStr = CP::TDbiDatabaseManager::Instance().GetCascader().GetURL(fDbNo);
    TUrl url(urlStr.c_str());
    fServerName = url.GetHost();

}
//.....................................................................

void CP::TDbiLogEntry::Store(CP::TDbiOutRowStream& ors,
                             const CP::TDbiValidityRec* /* vrec */) const {
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

Bool_t CP::TDbiLogEntry::Write(UInt_t dbNo,
                               Int_t logSeqNo) {   /* =0 */
//
//
//  Purpose: Write, or rewrite this entry, adding logSeqNo to range of
//           update SEQNOs if non-zero.

    if (logSeqNo > 0) {
        if (fLogNumSeqNo == 0) {
            fLogSeqNoMin = logSeqNo;
            fLogSeqNoMax = logSeqNo;
        }
        if (logSeqNo < fLogSeqNoMin) {
            fLogSeqNoMin = logSeqNo;
        }
        if (logSeqNo > fLogSeqNoMax) {
            fLogSeqNoMax = logSeqNo;
        }
        ++fLogNumSeqNo;
    }

    if (! this->HasReason() || fLogNumSeqNo == 0) {
        DbiSevere("Cannot write LogEntry - no reason and/or no SEQNOs defined "
                  << "  ");
        return kFALSE;
    }

    if (fSeqNo != 0 && dbNo != fDbNo) {
        DbiSevere("Cannot write LogEntry - attempting to switch from database"
                  << fDbNo << " to " << dbNo << "  ");
        return kFALSE;
    }

    fDbNo = dbNo;
    CP::TDbiTableProxy& tblProxy = CP::TDbiDatabaseManager::Instance()
                                   .GetTableProxy("DBILOGENTRY",this);
    bool replace = true;

    // Allocate SEQNO before first write.
    if (fSeqNo == 0) {
        replace = false;
        int seqNo = tblProxy.GetCascader().AllocateSeqNo("DBILOGENTRY",0,fDbNo);
        if (seqNo <= 0) {
            DbiSevere("Cannot get sequence number for table DBILOGENTRY" << "  ");
            return kFALSE;
        }
        fSeqNo = seqNo;
    }

    // Construct a CP::TDbiValidityRec.
    CP::TVldRange     vr(fLogDetMask,fLogSimMask,fUpdateTime,fUpdateTime,"CP::TDbiLogEntry");
    CP::TDbiValidityRec vrec(vr,fLogTask,-1,0);

    // Now build and output CP::TDbiSqlValPacket.
    CP::TDbiSqlValPacket packet;
    packet.Recreate("DBILOGENTRY",
                    vrec.GetVldRange(),
                    -1,
                    vrec.GetTask(),
                    vrec.GetCreationDate());
    packet.AddDataRow(tblProxy,0,*this);
    packet.SetSeqNo(fSeqNo);
    return packet.Store(fDbNo,replace);
}


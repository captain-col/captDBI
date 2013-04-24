#ifndef DBILOGENTRY_H
#define DBILOGENTRY_H

/**
 *
 * $Id: TDbiLogEntry.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 * \class CP::TDbiLogEntry
 *
 *
 * \brief
 * <b>Concept</b>  An single entry in the Database Log which records
 *   all non-automatic updates to the database.
 *
 * \brief
 * <b>Purpose</b> Capture and present updates together with the reasons
 *   for them.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include "DbiDetector.hxx"
#include "DbiSimFlag.hxx"
#include "TDbi.hxx"
#include "TDbiTableRow.hxx"
#include "TVldTimeStamp.hxx"

#include "TDbiResultSetHandle.hxx"

#include "TObject.h"

#include <string>
#include <iosfwd>

namespace CP {
    class TDbiLogEntry;
    class TDbiOutRowStream;
    class TDbiInRowStream;
    class TDbiTableProxy;
    class TDbiValidityRec;
    std::ostream& operator<<(std::ostream& s, const CP::TDbiLogEntry& logEntry);
}


namespace CP {
    class TDbiLogEntry : public TDbiTableRow {

    public:

        using TObject::Write;

// Constructors and destructors.
        TDbiLogEntry(const std::string& tableName = "",
                     const std::string& reason = "",
                     Int_t detMask = CP::DbiDetector::FullMask(),
                     Int_t simMask = CP::DbiSimFlag::FullMask(),
                     TDbi::Task task = 0,
                     Int_t logSeqNoMin = 0,
                     Int_t logSeqNoMax = 0,
                     Int_t logNumSeqNo = 0);
        virtual ~TDbiLogEntry();

// State testing member functions.

// Inherited responsibilities.
        virtual TDbiTableRow* CreateTableRow() const {
            return new TDbiLogEntry;
        }
        Int_t GetAggregateNo() const {
            return -1;
        }

// New member functions.
        const std::string& GetLogTableName() const {
            return fLogTableName;
        }
        Int_t GetDetectorMask() const {
            return fLogDetMask;
        }
        Int_t GetLogSeqNoMin() const {
            return fLogSeqNoMin;
        }
        Int_t GetLogSeqNoMax() const {
            return fLogSeqNoMax;
        }
        Int_t GetLogNumSeqNo() const {
            return fLogNumSeqNo;
        }
        Int_t GetSimMask() const {
            return fLogSimMask;
        }
        TDbi::Task GetTask() const {
            return fLogTask;
        }
        const std::string& GetUserName() const {
            return fUserName;
        }
        const std::string& GetProcessName() const {
            return fProcessName;
        }
        const std::string& GetHostName() const {
            return fHostName;
        }
        const std::string& GetServerName() const {
            return fServerName;
        }
        const std::string& GetReason() const {
            return fReason;
        }
        CP::TVldTimeStamp GetUpdateTime() const {
            return fUpdateTime;
        }

        Bool_t HasReason() const {
            return fReason.size() > 0;
        }

// State changing member functions
        void SetReason(const std::string& reason);
        void SetDetectorMask(Int_t detMask) {
            fLogDetMask = detMask;
        }
        void SetSimMask(Int_t simMask) {
            fLogSimMask = simMask;
        }
        void SetTask(TDbi::Task task) {
            fLogTask = task;
        }
        void SetUpdateTime(const CP::TVldTimeStamp& updateTime) {
            fUpdateTime = updateTime;
        }
        void Recreate(const std::string& tableName = "",
                      const std::string& reason = "",
                      Int_t detMask = CP::DbiDetector::FullMask(),
                      Int_t simMask = CP::DbiSimFlag::FullMask(),
                      TDbi::Task task = 0,
                      Int_t logSeqNoMin = 0,
                      Int_t logSeqNoMax = 0,
                      Int_t logNumSeqNo = 0);

// I/O  member functions
        virtual void Fill(TDbiInRowStream& rs,
                          const TDbiValidityRec* vrec);
        virtual void Store(TDbiOutRowStream& ors,
                           const TDbiValidityRec* vrec) const;
        Bool_t Write(UInt_t dbNo,Int_t logSeqNo=0);

    private:
// Constructors and destructors.
        TDbiLogEntry(const TDbiLogEntry& from); // No!

        void SetServerName();

// Data members

/// Database no. written to. =0 if not output
        UInt_t fDbNo;

/// Seq. No. used to write this object. =0 if not output
        Int_t fSeqNo;

/// Table being updated.
        std::string fLogTableName;

/// Detector type mask of update.
        Int_t fLogDetMask;

/// SimFlag  mask of update.
        Int_t fLogSimMask;

/// Task of update
        TDbi::Task fLogTask;

/// Minimum Seq. No. being updated.
        Int_t fLogSeqNoMin;

/// Maximum Seq. No. being updated.
        Int_t fLogSeqNoMax;

/// Number of Seq. Nos. being updated.
        Int_t fLogNumSeqNo;

/// Time of update.
        CP::TVldTimeStamp fUpdateTime;

/// User performing update.
        std::string fUserName;

/// Process performing update.
        std::string fProcessName;

/// Host performing update.
        std::string fHostName;

/// Db server being updated.
        std::string fServerName;

/// Reason for update.
        std::string fReason;


// Note: fLogDetMask,fLogSimMask,fLogTask,fUpdateTime are used to construct the TDbiValidityRec
//       of the update; they are not store in main table.

        ClassDef(TDbiLogEntry,0)    // Configuration data.

    };
};


#endif  // DBILOGENTRY_H


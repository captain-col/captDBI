#ifndef TDBIVALIDITYREC_H
#define TDBIVALIDITYREC_H

/**
 *
 * $Id: TDbiValidityRec.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 * \class CP::TDbiValidityRec
 *
 *
 * \brief
 * <b>Concept</b> A concrete data type corresponding to a single row in
 *  an  auxillary validity range database table.
 *
 * \brief
 * <b>Purpose</b> To resolve ambiguities in the case of multiple matches
 *   to a query and to record results of validity range trimming due to
 *   neighbouring database entries.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include "TDbi.hxx"
#include "TDbiTableRow.hxx"
#include "TVldRange.hxx"
#include "TVldTimeStamp.hxx"

#include <iosfwd>
#include <string>

namespace CP {
    class TDbiBinaryFile;
    class TDbiTableProxy;
    class TDbiValidityRec;
    std::ostream& operator<<(std::ostream& s, const CP::TDbiValidityRec& vRec);
}


namespace CP {
    class TDbiValidityRec : public TDbiTableRow {


    public:

// Constructors and destructors.
        TDbiValidityRec(Int_t dbNo = -1, Bool_t isGap = kTRUE);
        TDbiValidityRec(const TDbiValidityRec& from);
        TDbiValidityRec(const CP::TVldRange& range,
                        TDbi::Task task,
                        Int_t aggNo,
                        UInt_t seqNo,
                        Int_t dbNo = -1,
                        Bool_t isGap = kFALSE,
                        CP::TVldTimeStamp time = CP::TVldTimeStamp(),
                        UInt_t epoch = 0);
        virtual ~TDbiValidityRec();

// State testing member functions

        virtual TDbiTableRow* CreateTableRow() const {
            return new TDbiValidityRec;
        }
        Int_t GetAggregateNo() const {
            return fAggregateNo;
        }
        CP::TVldTimeStamp GetCreationDate() const {
            return fCreationDate;
        }
        UInt_t GetDbNo() const {
            return fDbNo;
        }
        UInt_t GetEpoch() const {
            return fEpoch;
        }
        CP::TVldTimeStamp GetInsertDate() const {
            return fInsertDate;
        }
        TDbi::Task GetTask() const {
            return fTask;
        }
        std::string GetL2CacheName() const;
        UInt_t GetSeqNo() const {
            return fSeqNo;
        }
        const TDbiTableProxy* GetTableProxy() const {
            return fTableProxy;
        }
        const CP::TVldRange& GetVldRange() const {
            return fVldRange;
        }
        Bool_t HasExpired(const TDbiValidityRec& other) const;
        Bool_t HasExpired(const CP::TVldContext& vc,
                          const TDbi::Task& task) const;
        Bool_t IsCompatible(const CP::TVldContext& vc,
                            const TDbi::Task& task) const;
        Bool_t IsGap() const {
            return fIsGap;
        }
        Bool_t IsHigherPriority(const TDbiValidityRec& that,
                                Bool_t useCreationDate = false) const;

        static     std::string GetL2CacheName(UInt_t seqLo, UInt_t seqHi, CP::TVldTimeStamp ts);

// State changing member functions
        void AndTimeWindow(const CP::TVldTimeStamp& start,
                           const CP::TVldTimeStamp& end);
        void SetAggregateNo(Int_t aggNo) {
            fAggregateNo = aggNo;
        }
        void SetDbNo(Int_t dbNo) {
            fDbNo = dbNo;
        }
        void SetEpoch(UInt_t epoch) {
            fEpoch = epoch;
        }
        void SetTableProxy(const TDbiTableProxy* tp) {
            fTableProxy = tp;
        }
        void SetTimeWindow(const CP::TVldTimeStamp& start,
                           const CP::TVldTimeStamp& end);
        void SetVldRange(const CP::TVldRange& range) {
            fVldRange=range;
        }
        void Trim(const CP::TVldTimeStamp& queryTime,
                  const TDbiValidityRec& other);

// I/O  member functions
        virtual void Fill(TDbiInRowStream& rs,
                          const TDbiValidityRec* vrec);
        virtual void Store(TDbiOutRowStream& ors,
                           const TDbiValidityRec* vrec) const;
        void Streamer(TDbiBinaryFile& file);

    private:

// Data members


/// Aggregate number or:-
///    -1 Non-aggregated data or multiple aggregates
///    -2 undefined aggregates
        Int_t fAggregateNo;

///  Creation date
        CP::TVldTimeStamp fCreationDate;

/// Cascade no. of source db =-1 unknown
        Int_t fDbNo;

///  Insertion date
        CP::TVldTimeStamp fInsertDate;

///  kTRUE if corresponds to a gap.
        Bool_t fIsGap;

/// Task number
        TDbi::Task fTask;

/// Sequence number or 0 if compound.
        Int_t fSeqNo;


/// TDbiTableProxy that filled this (if any).
        const TDbiTableProxy* fTableProxy;

/// Validity range of record.
        CP::TVldRange fVldRange;

/// Calibration Epoch
        UInt_t fEpoch;

/// Reality (for possible, but improbable future use)
        UInt_t fReality;

        ClassDef(TDbiValidityRec,0)  // Validity range table row.

    };
};


#endif  // TDBIVALIDITYREC_H


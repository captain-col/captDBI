#ifndef DBIVALIDITYRECBUILDER_H
#define DBIVALIDITYRECBUILDER_H

/**
 *
 * $Id: TDbiValidityRecBuilder.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 * \class CP::TDbiValidityRecBuilder
 *
 *
 * \brief
 * <b>Concept</b> Constructor for the set of effective TDbiValidityRec
 *   objects for the current query.
 *
 * \brief
 * <b>Purpose</b> Parse a TDbiInRowStream from a validity query and build
 *   TDbiValidityRecs for the best, with CP::TVldRange trimmed by neighbouring
 *   validity records of higher priority. For non-aggregated tables
 *   there will only be one TDbiValidityRec, but for aggregated data,
 *   there will be one for each aggregate
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include <string>
#include <map>
#include <vector>

#include "TDbi.hxx"
#include "TDbiValidityRec.hxx"
#include "TVldContext.hxx"


namespace CP {
    class TDbiDBProxy;
}

namespace CP {
    class TDbiValidityRecBuilder {


    public:

// Typedefs and enums

        enum { kMAX_AGGREGATE_NO = 1000 };

// Constructors and destructors
        TDbiValidityRecBuilder(const TDbiDBProxy& proxy,
                               const CP::TVldContext& vc,
                               const TDbi::Task& task,
                               Int_t selectDbNo = -1,
                               Bool_t findFullTimeWindow = true);
        TDbiValidityRecBuilder(const TDbiDBProxy& proxy,
                               const std::string& context,
                               const TDbi::Task& task);
        TDbiValidityRecBuilder(const TDbiValidityRec& vr,
                               const std::string tableName);

        virtual ~TDbiValidityRecBuilder();

// State testing member functions
        std::string GetL2CacheName() const;
        TDbi::Task GetTask() const {
            return fTask;
        }
        UInt_t GetNumValidityRec() const {
            return fVRecs.size();
        }
///             Get index of AggNo or -1 if missing.
        Int_t IndexOfAggno(Int_t aggNo) const;
        Bool_t IsExtendedContext() const {
            return fIsExtendedContext;
        }
        const TDbiValidityRec& GetValidityRec(Int_t rowNo) const;
        const TDbiValidityRec& GetValidityRecFromAggNo(Int_t aggNo) const {
            return this->GetValidityRec(this->IndexOfAggno(aggNo));
        }
        const TDbiValidityRec& GetValidityRecFromSeqNo(UInt_t SeqNo) const;
        Bool_t NonAggregated() const {
            return  ! this->IsExtendedContext()
                    && this->GetNumValidityRec() == 1;
        }

// State changing member functions


    private:

// State changing member functions

///  Add new aggregate into tables and return its index
        UInt_t AddNewAgg(const TDbiValidityRec& vrec,Int_t aggNo);
///  Add new aggregate gap into tables and return its index
        UInt_t AddNewGap(Int_t aggNo);

        void MakeGapRec(const CP::TVldContext& vc,
                        const std::string& tableName,
                        Bool_t findFullTimeWindow = true);

// Data members

        TDbiValidityRec fGap;     // Default (gap) validity record
/// True for extended context query.
        Bool_t fIsExtendedContext;
        TDbi::Task fTask;    // Query task
/// Table of effective ValidityRecs.
/// For context specific queries indexed by row number 0,..
/// The first row is guaranteed to correspond to aggregate -1,
/// although this may be a gap.  Other aggregates may come in any order.
/// For extended context queries first entry is a gap (to be consistent
/// with aggregated query) and all entries are filled in the order they
/// are loaded.

        std::vector<TDbiValidityRec> fVRecs;
/// Map of Aggregate number to index in fVRecs.
        std::map<Int_t,UInt_t>  fAggNoToIndex;


        ClassDef(TDbiValidityRecBuilder,0) // Creator of eff. ValidityRecs

    };
};


#endif  // DBIVALIDITYRECBUILDER_H


#ifndef DBIRESULTAGG_H
#define DBIRESULTAGG_H

/**
 *
 *
 *
 * \class CP::TDbiResultSetAgg
 *
 *
 * \brief ResultAgg is a concrete sub-class of   CP::TDbiResultSet.
 *
 * <b>Concept</b>   ResultAgg is a concrete sub-class of
 *    CP::TDbiResultSet that represents a single database query for either:-
 *
 *         1)  A specific context with aggregated data (i.e. rows
 *             with AggNo != -1)
 *
 *          or:-
 *
 *         2)  Any extended context query, even if that only produces
 *             a single row of unaggregated data.
 *
 *         The table is indexed using the index returned by
 *         TDbiTableRow::GetIndex which can be overridden by concrete
 *         subclasses.
 *
 * <b>Purpose</b> To present the user with a single logical table even
 *   though the component parts may held in multiple  TDbiResultSets and
 *   be loaded by separate I/O operations.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include "TDbiResultSet.hxx"

#include <string>
#include <vector>

namespace CP {
    class TDbiCache;
    class TDbiTDbiBinaryFile;
    class TDbiDBProxy;
    class TDbiInRowStream;
    class TDbiTableRow;
    class TDbiValidityRecBuilder;

    class TDbiResultSetAgg : public TDbiResultSet {


    public:

// Only some Satisfies methods are overriden so include base class ones.
        using TDbiResultSet::Satisfies;

// Constructors and destructors.
        TDbiResultSetAgg(const std::string& tableName = "Unknown",
                         const TDbiTableRow* tableRow = 0,
                         TDbiCache* cache = 0,
                         const TDbiValidityRecBuilder* vrecBuilder = 0,
                         const TDbiDBProxy* proxy=0,
                         const std::string& sqlQualifiers = "");
        virtual ~TDbiResultSetAgg();

// State testing member functions

        virtual         TDbiResultKey* CreateKey() const;
        virtual                UInt_t GetNumAggregates() const {
            return 1 + fResults.size();
        };
        virtual                UInt_t GetNumRows() const {
            return fSize;
        }
        virtual    const TDbiTableRow* GetTableRow(UInt_t row) const;
        virtual const TDbiValidityRec& GetValidityRec(
            const TDbiTableRow* row=0) const;

//  State changing member functions.

        virtual     Bool_t Satisfies(const std::string& sqlQualifiers);
        virtual void Streamer(TDbiBinaryFile& bf);


    private:

// Data members

        /// Array of TDbiResultSets (vector<TDbiResultSet*>).
        /// The array index is NOT the aggregate number; it simply matches
        /// the rows are supplied by the TDbiValidityRecBuilder with first
        /// row of TDbiValidityRecBuilder excluded  i.e. row n of builder
        /// creates a TDbiResultSet pointed to by fResults[n-1];
        std::vector<const TDbiResultSet*>   fResults;

        /// Look-up: Row no. -> TableRow
        std::vector<const TDbiTableRow*> fRowKeys;

        /// Current set size.
        UInt_t fSize;

        ClassDef(TDbiResultSetAgg,0)     // Results table of aggregated data.

    };
};


#endif  // DBIRESULTAGG_H


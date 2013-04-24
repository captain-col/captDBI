#ifndef DBIROWSTREAM_H
#define DBIROWSTREAM_H
/**
 *
 * $Id: TDbiRowStream.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 * \class CP::TDbiRowStream
 *
 *
 * \brief
 * <b>Concept</b>  I/O buffer for a row of a table.
 *
 * \brief
 * <b>Purpose</b> This forms the base class CP::for TDbiInRowStream (input)
 *  and TDbiWriter (input)
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */
#include "TDbiFieldType.hxx"

#include <string>

namespace CP {
    class TDbiTableMetaData;

    class TDbiRowStream {

    public:

// Constructors and destructors.
        TDbiRowStream(const TDbiTableMetaData* metaData);
        virtual ~TDbiRowStream();

// State testing member functions
        virtual
        const TDbiFieldType& ColFieldType(UInt_t col) const;
        virtual       std::string ColName(UInt_t col) const;
        virtual
        const TDbiFieldType& CurColFieldType() const;
        virtual       std::string CurColName() const;
        virtual       UInt_t CurColNum() const {
            return fCurCol;
        }
        Bool_t HasEpoch() const;
        Bool_t HasRowCounter() const {
            return fHasRowCounter;
        }
        Bool_t IsVLDTable() const {
            return fIsVLDTable;
        }
        virtual       UInt_t NumCols() const;
        virtual       std::string TableName() const;
        virtual       std::string TableNameTc() const;
        virtual const TDbiTableMetaData*
        MetaData() const {
            return fMetaData;
        }
// State changing member functions

        virtual    void DecrementCurCol() {
            fCurCol = (fCurCol <= 1) ? 1 : fCurCol -1;
        }
// Allow IncrementCurCol to step off end (to catch attempt to read
// too much data)
        virtual    void IncrementCurCol() {
            ++fCurCol;
        }

    protected:

        virtual    void ClearCurCol() {
            fCurCol = 1;
        }

    private:


// Data members

/// Current column in query (1..fNumCols)
        UShort_t fCurCol;

/// True if main table has ROW_COUNTER as second column
        Bool_t fHasRowCounter;

/// Owned by TDbiTableProxy
        const TDbiTableMetaData* fMetaData;

/// True if reading data for a *VLD table.
        Bool_t fIsVLDTable;

        ClassDef(TDbiRowStream,0)     // I/O of a table row.

    };
};
#endif  // DBIROWSTREAM_H

#ifndef DBIRESULTNONAGG_H
#define DBIRESULTNONAGG_H

/**
 *
 *
 * \class CP::TDbiResultSetNonAgg
 *
 *
 * \brief 
 * <b>Concept</b>  ResultNonAgg is a concrete sub-class CP::of TDbiResultSet
 *   that represents a single database query for a non-aggregated table.
 *
 * <b>Purpose</b> See TDbiResultSet.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include "TDbiResultSet.hxx"

#if !defined(__CINT__) || defined(__MAKECINT__)
#include "Rtypes.h"
#endif
#include <string>
#include <vector>

namespace CP {
class TDbiBinaryFile;
class TDbiInRowStream;
class TDbiTableRow;
}

namespace CP {
class TDbiResultSetNonAgg : public TDbiResultSet

{

public:

// Only some Satisfies methods are overriden so include base class ones.
using TDbiResultSet::Satisfies;


// Constructors and destructors.
           TDbiResultSetNonAgg(TDbiInRowStream* resultSet = 0,
                           const TDbiTableRow* tableRow = 0,
                           const TDbiValidityRec* vrec = 0,
                           Bool_t dropSeqNo = kTRUE,
                           const std::string& sqlQualifiers = "");
  virtual ~TDbiResultSetNonAgg();


// State testing member functions

 virtual  TDbiResultKey* CreateKey() const;
 virtual                UInt_t GetNumAggregates() const { return 1; }
 virtual                UInt_t GetNumRows() const {
                                                  return fRows.size(); }
 virtual    const TDbiTableRow* GetTableRow(UInt_t rowNum) const;
 virtual    const TDbiTableRow* GetTableRowByIndex(UInt_t index) const;

//  State changing member functions.

 virtual Bool_t Owns(const TDbiTableRow* row ) const;
         Bool_t Satisfies(const TDbiValidityRec& vrec,
                          const std::string& sqlQualifiers = "");
 virtual void   Streamer(TDbiBinaryFile& file);

private:

    void DebugCtor() const;

// Data members

/// Set of table rows eqv. to ResultSet
  std::vector<TDbiTableRow*> fRows;

/// Not null only if table rows read from BinaryFile.
 char* fBuffer;

ClassDef(TDbiResultSetNonAgg,0)     //Example non-aggregated data.

};
};


#endif  // DBIRESULTNONAGG_H


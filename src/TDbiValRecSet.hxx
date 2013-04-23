#ifndef DBIVALRECSET
#define DBIVALRECSET

/**
 *
 * $Id: TDbiValRecSet.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 * \class CP::TDbiValRecSet
 *
 *
 * \brief
 * <b>Concept</b>  A set of TDbiValidityRecs
 *
 * \brief
 * <b>Purpose</b> Provides a sequential access mechanism for an entire
 *   table via its auxillary Validity Range table
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include <map>
#include <string>

namespace CP {
class TDbiResultSet;
class TDbiValidityRec;
}

namespace CP {
class TDbiValRecSet
{

public:

// Constructors and destructors.
           TDbiValRecSet(const std::string& tableName,UInt_t dbNo,UInt_t seqNo=0);
  virtual ~TDbiValRecSet();

// State testing member functions
               UInt_t GetDbNo() const { return fDbNo; }
               UInt_t GetNumRows() const;
         const std::string GetTableName() const;
const TDbiValidityRec* GetTableRow(UInt_t rowNum) const;
const TDbiValidityRec* GetTableRowBySeqNo(UInt_t seqNo) const;

// State changing member functions

private:

// Data members

//// Database number used to fill
  UInt_t fDbNo;

/// Result holding table of TDbiValidityRec
  TDbiResultSet* fResult;

/// Lookup SeqNo -> TDbiValidityRec
/// lazy creation - see GetTableRowBySeqNo
    mutable std::map<UInt_t,const TDbiValidityRec*>fSeqNoToRec;

 ClassDef(TDbiValRecSet,0)     //  A set of TDbiValidityRecs

};
};


#endif  // DBIVALRECSET


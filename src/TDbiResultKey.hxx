#ifndef DBIRESULTKEY
#define DBIRESULTKEY

/**
 *
 * $Id: TDbiResultKey.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 * \class ND::TDbiResultKey
 *
 *
 * \brief
 * <b>Concept</b> Summarises the composition of a single TDbiResultSet.
 *
 * \brief
 * <b>Purpose</b> To serve as summary of a single query.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include <list>
#include <iosfwd>
#include <string>

#include "TObject.h"

#include "TVldTimeStamp.hxx"

namespace ND {
class TDbiResultKey;
}
std::ostream& operator<<(std::ostream& os, const ND::TDbiResultKey& key);

namespace ND {
class TDbiResultKey : public TObject
{

public:

  using TObject::Compare;

// Constructors and destructors.
           TDbiResultKey(const TDbiResultKey* that = 0);
           TDbiResultKey(std::string tableName,
                        std::string rowName,
                        UInt_t seqno,
                        ND::TVldTimeStamp ts);
  virtual ~TDbiResultKey();

// State testing member functions
  std::string AsString() const;
      Float_t Compare(const TDbiResultKey* that) const;
static const TDbiResultKey*
              GetEmptyKey() { return &fgEmptyKey; }
        Int_t GetNumVrecs() const { return fNumVRecKeys; }
  std::string GetTableRowName() const;
       Bool_t IsEqualTo(const TDbiResultKey* that) const {
	 return this->Compare(that) == 1.; }

// State changing member functions

  void AddVRecKey(UInt_t seqno, ND::TVldTimeStamp ts);
  
  
  // push VRecKey put to the public: portion so that rootcint dict compiles
  /// Container for  a SeqNo and  CreationDate pair. 
  struct VRecKey {
    VRecKey() : SeqNo(0), CreationDate() {}
    VRecKey(UInt_t seqno, ND::TVldTimeStamp ts) : SeqNo(seqno), CreationDate(ts) {}
    UInt_t SeqNo;
    ND::TVldTimeStamp CreationDate;
  };

private:

  static TDbiResultKey fgEmptyKey;

// Data members

/// Name of database table
  std::string fTableName;

/// Name of row objects.
  std::string fRowName;
	
///  Key for individual TDbiValidityRec
   std::list<TDbiResultKey::VRecKey> fVRecKeys;

/// Optimisation: list::size() is slow!
   Int_t fNumVRecKeys;

 ClassDef(TDbiResultKey,1)     // output string stream

};
};


#endif // DBIRESULTKEY



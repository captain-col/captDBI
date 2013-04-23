#ifndef DBICACHE_H
#define DBICACHE_H

/**
 *
 *
 * \class ND::TDbiCache
 *
 *
 * \brief
 * <b>Concept</b> Cache holding TDbiResultSet s for a specific database table.
 *
 * \brief
 * <b>Purpose</b> A TDbiCache is an object that  minimises database I/O
 *   by caching query results.  Queries are always first sent to the
 *   cache and only if not present are they sent down to the database.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include <list>
#include <map>
#include <string>
using std::string;

#include "TDbi.hxx"

namespace ND {
class TVldContext;
};
namespace ND {
class TDbiResultSet;
class TDbiDatabaseManager;
class TDbiTableProxy;
class TDbiValidityRec;
}
//class ostream;

namespace ND {
class TDbiCache
{

  friend   class TDbiValidate;  // To allow access to purge.
public:

// Typedefs

typedef std::list<ND::TDbiResultSet*> ResultList_t;

// Constructors and destructors.

           TDbiCache(TDbiTableProxy& qp,
                    const string& tableName);
  virtual ~TDbiCache();


// State testing member functions
          UInt_t GetMaxSize() const { return fMaxSize; }
          UInt_t GetCurSize() const { return fCurSize; }
          UInt_t GetNumAdopted() const { return fNumAdopted; }
          UInt_t GetNumReused() const { return fNumReused; }
// Primary searches.
const TDbiResultSet* Search(const ND::TVldContext& vc,
                        const TDbi::Task& task) const;
const TDbiResultSet* Search(const string& sqlQualifiers) const;

/// Secondary search.
const TDbiResultSet* Search(const TDbiValidityRec& vr,
                        const string& sqlQualifiers = "") const;
      ostream& ShowStatistics(ostream& msg) const;

// State changing member functions
    void Adopt(TDbiResultSet* res,bool generateKey = true);
    void Purge();
    void SetStale();

protected:

// State testing member functions

// State changing member functions

private:

// Disabled (not implemented) copy constructor and asignment.
 TDbiCache(const TDbiCache&);
 ND::TDbiCache& operator=(const ND::TDbiCache&);

  const ResultList_t* GetSubCache(Int_t aggNo) const;
                void Purge(ResultList_t& subCache, const TDbiResultSet* res=0);

// Data members


/// TableProxy owning cache.
  TDbiTableProxy&  fTableProxy;

/// Name of associated table.
  const string& fTableName;

/// Map of sub-caches indexed by aggregate number.
/// Each sub-cache is a list of owned results for
/// that aggregate.
  std::map<Int_t,ResultList_t> fCache;

/// Current size
  mutable UInt_t fCurSize;

/// Max (high water) size
  mutable UInt_t fMaxSize;

/// Total number adopted
  mutable UInt_t fNumAdopted;

/// Number reused i.e. found.
  mutable UInt_t fNumReused;


ClassDef(TDbiCache,0)  //Query result cache for specific database table.

};
};


#endif  // DBICACHE_H



#ifndef DBIDBPROXY_H
#define DBIDBPROXY_H

/**
 *
 * $Id: TDbiDBProxy.hxx,v 1.1 2011/01/18 05:49:19 finch Exp $
 *
 * \class CP::TDbiDBProxy
 *
 *
 * \brief
 * <b>Concept</b> Object to query a specific database table.
 *
 * \brief
 * <b>Purpose</b> To encapsulate the standard context query and
 * the management of SEQNOs and meta-data.  This allows clients to
 * execute queries without specifying SQL. TDbiDBProxy also supports
 * "Extended Context" queries which provides a framework in which
 * clients can extended the basic query and can use SQL to fine tune
 * their requests.
 *
 * \brief
 * <b>Uage Notes</b> The object can be "programmed" with an SQL condition
 * using SetSqlCondition.  If not null the string is prefixed with
 *"where" and used when responding to the following validity queries:-
 *   QueryAllValidities
 *   QueryValidity
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include "TDbi.hxx"

#include <string>
#include <list>
#include <vector>

namespace CP {
class TDbiCascader;
class TDbiInRowStream;
class TDbiTableMetaData;
class TDbiTableProxy;
class TDbiValidityRec;
}
namespace CP {
class TVldContext;
};
namespace CP {
class TVldTimeStamp;
};

namespace CP {
class TDbiDBProxy
{

public:

#ifndef __CINT__
typedef const std::vector<UInt_t> SeqList_t;
#endif

// Constructors.
           TDbiDBProxy(TDbiCascader& cascader,
                      const std::string& tableName,
                      const TDbiTableMetaData* metaData,
                      const TDbiTableMetaData* metaValid,
                      const TDbiTableProxy* tableProxy);
  virtual ~TDbiDBProxy();

// State testing member functions
              Bool_t HasEpoch() const;
              UInt_t GetNumDb() const;
       const std::string& GetTableName() const { return fTableName; }
const TDbiTableProxy* GetTableProxy() const { return fTableProxy; }
                void StoreMetaData(TDbiTableMetaData& metaData) const;
              Bool_t TableExists(Int_t selectDbNo=-1) const;

// Query (input) member functions
           void FindTimeBoundaries(const CP::TVldContext& vc,
                                   const TDbi::Task& task,
                                   UInt_t dbNo,
                                   const TDbiValidityRec& lowestPriorityVrec,
				   Bool_t resolveByCreationDate,
                                   CP::TVldTimeStamp& start,
                                   CP::TVldTimeStamp& end) const;
  TDbiInRowStream* QueryAllValidities(UInt_t dbNo,UInt_t seqNo=0) const;
  TDbiInRowStream* QuerySeqNo(UInt_t seqNo,UInt_t dbNo) const;
#ifndef __CINT__
/// Secondary query for aggregate and extended context queries.
  TDbiInRowStream* QuerySeqNos(SeqList_t& seqNos,
                            UInt_t dbNo,
                            const std::string& sqlData = "",
                            const std::string& fillOpts = "") const;
#endif
  TDbiInRowStream* QueryValidity(const CP::TVldContext& vc,
                              const TDbi::Task& task,
                              UInt_t dbNo) const;
  TDbiInRowStream* QueryValidity(const std::string& context,
                              const TDbi::Task& task,
                              UInt_t dbNo) const;
  TDbiInRowStream* QueryValidity(UInt_t seqNo,
                              UInt_t dbNo) const;

// Store (output) member functions
         Bool_t ReplaceInsertDate(const CP::TVldTimeStamp& ts,
                                  UInt_t SeqNo,
                                  UInt_t dbNo) const;
         Bool_t RemoveSeqNo(UInt_t seqNo,
                            UInt_t dbNo) const;
         Bool_t ReplaceSeqNo(UInt_t oldSeqNo,
                             UInt_t newSeqNo,
                             UInt_t dbNo) const;

// State changing member functions
	   void SetSqlCondition(const std::string& sql) {
                                                  fSqlCondition = sql; }

private:

// Disabled (not implemented) copy constructor and asignment.

 TDbiDBProxy(const TDbiDBProxy&);
 CP::TDbiDBProxy& operator=(const CP::TDbiDBProxy&);

// Data members

/// Reference to one and only cascader
  TDbiCascader& fCascader;

/// Owned by TDbiTableProxy
  const TDbiTableMetaData* fMetaData;

/// Owned by TDbiTableProxy
  const TDbiTableMetaData* fMetaValid;

/// Optional condition to be applied.
/// See Usage Notes.
  std::string fSqlCondition;

/// Table Name
  std::string fTableName;

/// Owning TDbiTableProxy.
   const TDbiTableProxy* fTableProxy;

ClassDef(TDbiDBProxy,0)     //  Proxy for physical database.

};
};

#endif  // DBIDBPROXY_H


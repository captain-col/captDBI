#ifndef TDBIRESULT_H
#define TDBIRESULT_H

/**
 *
 *
 * \class CP::TDbiResultSet
 *
 *
 * \brief
 * <b>Concept</b> Abstract base class representing the Result of a single
 *   database query. If query suceeded the Result will hold (own) a vector
 *   of table row objects that correspond to result of the query. It will
 *   also own a TDbiValidityRec that gives the range over which the result
 *   is valid.
 *
 * \brief
 * <b>Purpose</b> To provide suitable objects to cache.  TDbiResultSets can
 *   be checked to see if they satisfy new queries.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include <map>
#include <string>
using std::string;

#include "TDbi.hxx"
#include "TDbiExceptionLog.hxx"
#include "TDbiValidityRec.hxx"

typedef std::map<UInt_t,const CP::TDbiTableRow*> IndexToRow_t;

namespace CP {
class TDbiBinaryFile;
class TDbiResultKey;
class TDbiResultSet;
class TDbiInRowStream;
class TDbiTableRow;
}
namespace CP {
class TVldContext;
};

CP::TDbiBinaryFile& operator<<(CP::TDbiBinaryFile& bf, const CP::TDbiResultSet& res);
CP::TDbiBinaryFile& operator>>(CP::TDbiBinaryFile& bf, CP::TDbiResultSet& res);

namespace CP {
class TDbiResultSet
{

public:

// Constructors and destructors.
           TDbiResultSet(TDbiInRowStream* resultSet = 0,
                     const TDbiValidityRec* vrec = 0,
                     const string& sqlQualifiers = "");
  virtual ~TDbiResultSet();

// State testing member functions

 virtual                Bool_t CanReuse() const  { return fCanReuse; }
 virtual                Bool_t CanSave() const  { return kTRUE; }
 virtual                  void Connect() const { ++fNumClients; }
 virtual         TDbiResultKey* CreateKey() const = 0;
 virtual                  void Disconnect() const { --fNumClients; }
        const TDbiExceptionLog& GetExceptionLog() const { return fExceptionLog; }
                         Int_t GetID() const { return fID; }
 virtual   const TDbiResultKey* GetKey() const;
 virtual                UInt_t GetNumAggregates() const =0;
 virtual                UInt_t GetNumClients() const {
                                                   return fNumClients; }
 virtual                UInt_t GetNumRows() const =0;
                 const string& GetSqlQualifiers() const { return fSqlQualifiers; }
 virtual    const TDbiTableRow* GetTableRow(UInt_t rowNum) const =0;
 virtual    const TDbiTableRow* GetTableRowByIndex(UInt_t index) const;
 virtual const TDbiValidityRec& GetValidityRec(
                                  const TDbiTableRow* /* row */ = 0) const {
                                        return GetValidityRecGlobal(); }
 virtual const TDbiValidityRec& GetValidityRecGlobal() const {
                                                      return fEffVRec; }
                        Bool_t IsExtendedContext() const {
                                       return this->GetSqlQualifiers() != ""; }
 virtual                Bool_t Owns(const TDbiTableRow* /* row */) const { return kFALSE; }
                        Bool_t ResultsFromDb() const { return fResultsFromDb; }
 virtual         const string& TableName() const { return fTableName; }

// State changing member functions
                          void CaptureExceptionLog(UInt_t startFrom);

/// Return true if no clients and unlikely to be reused.
 virtual                Bool_t CanDelete(const TDbiResultSet* res = 0);

/// All TDbiResultSet classes can satisfy this type of primary
/// query so impliment here.
 virtual    Bool_t Satisfies(const CP::TVldContext& vc,
                             const TDbi::Task& task);
/// Not all TDbiResultSet classes can satisfy these types of
/// query so those that do must override.
virtual     Bool_t Satisfies(const string&) {return kFALSE;}
virtual     Bool_t Satisfies(const TDbiValidityRec&,
                             const string& = "") {return kFALSE;}

/// Key handling
 virtual    void GenerateKey();

 virtual void Streamer(TDbiBinaryFile& file);
 virtual void SetCanReuse(Bool_t reuse)  { fCanReuse = reuse ; }

protected:
         void SetResultsFromDb() { fResultsFromDb = kTRUE; }

// State testing member functions

         void BuildLookUpTable() const;
       Bool_t LookUpBuilt() const { return fIndexKeys.size() > 0; }

//  State changing member functions.

 virtual void SetTableName(const string& tableName)  {
                                               fTableName = tableName; }
 virtual void SetValidityRec(const TDbiValidityRec& vRec)  {
                                                      fEffVRec = vRec; }


private:

// Data members

/// Unique ID within the current job
  Int_t fID;

//// Set kTRUE if can be reused
  Bool_t fCanReuse;

//// Effective validity record
  TDbiValidityRec fEffVRec;

//// Look-up: Index -> TableRow
  mutable IndexToRow_t fIndexKeys;

//// Only non-zero for top-level result
  const TDbiResultKey* fKey;

/// True is at least part didn't come from cache.
  Bool_t fResultsFromDb;
//// Number of clients
  mutable Int_t fNumClients;

//// Table name
  string fTableName;

/// Null unless Extended Context query in which case it contains:-
/// context-sql;data-sql;fill-options
  string fSqlQualifiers;

/// Exception log produced when query was executed.
  TDbiExceptionLog fExceptionLog;


/// Used to allocate unique ID within the current job
  static  Int_t fgLastID;


ClassDef(TDbiResultSet,0)     //Abstract base representing query result

};
};


#endif  // TDBIRESULT_H


#ifndef DBIRESULTSETHANDLE_H
// this will switch users code from using TDbiResultSetHandle to CP::TDbiResultSetHandle
#define USE_NEW_DBI_API
#define DBIRESULTSETHANDLE_H

/**
 *
 * $Id: TDbiResultSetHandle.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 * \class CP::TDbiResultSetHandle
 *
 *
 * \brief
 * <b>Concept</b>  Templated class CP::of pointers to Result objects.
 *   ResultHandle objects are lightweight and provide type safe access
 *   to a specific table.
 *
 * \brief
 * <b>Purpose</b> ResultHandles are the primary application interface
 *  to the  TDbi.  Users instantiate ResultHandles with the information
 *   necessary define a table query and then interrogate the object
 *   for the results.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include "TDbi.hxx"
#include "TDbiResultSet.hxx"
#include "TDbiDatabaseManager.hxx"  //Only for cleaner
#include "TDbiValidityRec.hxx"
#include "TVldContext.hxx"

#include <map>
#include <string>

namespace CP {
class TDbiResultSet;
class TDbiResultKey;
class TDbiSqlContext;
class TDbiTableProxy;
}

namespace CP {
template <class T> class TDbiResultSetHandle
{

public:

// Constructors and destructors.
           TDbiResultSetHandle();
           TDbiResultSetHandle(const TDbiResultSetHandle& that);
           TDbiResultSetHandle(const CP::TVldContext& vc,
                        TDbi::Task task = TDbi::kDefaultTask,
                        TDbi::AbortTest abortTest = TDbi::kTableMissing,
                        Bool_t findFullTimeWindow = true);
           TDbiResultSetHandle(const std::string& tableName,
                        const CP::TVldContext& vc,
                        TDbi::Task task = TDbi::kDefaultTask,
                        TDbi::AbortTest abortTest = TDbi::kTableMissing,
                        Bool_t findFullTimeWindow = true);
            TDbiResultSetHandle(const std::string& tableName,
                         const TDbiSqlContext& context,
			 const TDbi::Task& task = TDbi::kAnyTask,
                         const std::string& data = "",
                         const std::string& fillOpts = "",
                         TDbi::AbortTest abortTest = TDbi::kTableMissing);
            TDbiResultSetHandle(const std::string& tableName,
                         const TDbiValidityRec& vrec,
                         TDbi::AbortTest abortTest = TDbi::kTableMissing);
            TDbiResultSetHandle(const std::string& tableName,
                         UInt_t seqNo,
                         UInt_t dbNo,
                         TDbi::AbortTest abortTest = TDbi::kTableMissing);
   virtual ~TDbiResultSetHandle();


// State testing member functions
const TDbiResultKey* GetKey() const;
      UInt_t GetNumRows() const;
const TDbiResultSet* GetResult() const { return fResult; }
       Int_t GetResultID() const;
    const T* GetRow(UInt_t rowNum) const;
    const T* GetRowByIndex(UInt_t index) const;
    const TDbiValidityRec* GetValidityRec(const TDbiTableRow* row=0) const;
           TDbiTableProxy& TableProxy() const;
	           Bool_t ResultsFromDb() const;

    static TDbiTableProxy& GetTableProxy();
    static TDbiTableProxy& GetTableProxy(const std::string& tableName);

// State changing member functions
    UInt_t NextQuery(Bool_t forwards = kTRUE);
    UInt_t NewQuery(CP::TVldContext vc,
                    TDbi::Task task=0,
                    Bool_t findFullTimeWindow = true);
    UInt_t NewQuery(const TDbiSqlContext& context,
		    const TDbi::Task& task = 0,
                    const std::string& data = "",
                    const std::string& fillOpts = "");
    UInt_t NewQuery(const TDbiValidityRec& vrec);
    UInt_t NewQuery(UInt_t seqNo,UInt_t dbNo);


private:
  void Disconnect();
  void SetContext(const TDbiValidityRec& vrec);
Bool_t ApplyAbortTest();

// 12 Jun 2002: Disabled (not implemented)  assignment


// Data members

/// Test which if failed triggers abort.
  TDbi::AbortTest fAbortTest;

/// Proxy for associated database table.
  TDbiTableProxy& fTableProxy;


// State from last query.

/// Pointer to query result. May be zero!
const TDbiResultSet* fResult;

/// from query context.
  CP::DbiDetector::Detector_t fDetType;

/// from query context.
  CP::DbiSimFlag::SimFlag_t fSimType;

#ifndef __CINT__ //  Hide map from CINT; it complains about missing Streamer() etc.
static std::map<std::string,TDbiTableProxy*> fgNameToProxy;
#endif  // __CINT__
static             TDbiTableProxy* fgTableProxy;

ClassDefT(TDbiResultSetHandle<T>,0)  // ResultHandle for specific database table.

};
};
ClassDefT2(TDbiResultSetHandle,T)

#endif  // DBIRESULTSETHANDLE_H


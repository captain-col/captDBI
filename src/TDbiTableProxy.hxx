#ifndef DBITABLEPROXY_H
#define DBITABLEPROXY_H

/**
 *
 *
 * \class CP::TDbiTableProxy
 *
 *
 * \brief
 * <b>Concept</b>  Object to query a specific database table.
 *
 * \brief
 * <b>Purpose</b> A TDbiTableProxy is an object that knows how to send
 *  queries to a specific database table. Internally it uses a cache
 *  to minimise I/O.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include <string>
#include <sstream>
using std::string;

#include "TDbi.hxx"
#include "TDbiDBProxy.hxx"
#include "TDbiTableMetaData.hxx"
#include "TDbiValidityRec.hxx"
#include "TVldContext.hxx"
#include "TVldTimeStamp.hxx"

namespace CP {
class TDbiCache;
class TDbiCascader;
class TDbiResultSet;
class TDbiDatabaseManager;
class TDbiTableRow;
class TDbiValidityRec;
class TDbiValidityRecBuilder;
}

namespace CP {
class TDbiTableProxy
{

  friend class TDbiDatabaseManager;    //Allow Resistry access to ctor/dtor.

public:

// State testing member functions
      const TDbiDBProxy& GetDBProxy() const { return fDBProxy; }
           TDbiCascader& GetCascader() { return *fCascader; }
const TDbiTableMetaData& GetMetaData() const { return fMetaData; }
const TDbiTableMetaData& GetMetaValid() const { return fMetaValid; }
                 string GetRowName() const {
                      return fTableRow ? fTableRow->ClassName() : "Unknown";}
                 string GetTableName() const { return fTableName;}
// State changing member functions
              TDbiCache* GetCache() { return fCache;}
              //
///\verbatim
///  Purpose:  Apply context specific query to database table and return result.
///
///  Arguments:
///    vc           in    The Validity Context for the query.
///    task         in    The task of the query.
///    findFullTimeWindow
///                 in    Attempt to find full validity of query
///                        i.e. beyond TDbi::GetTimeGate
///
///  Return:    Query result (never zero even if query fails).
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Apply query to database table and return result.
///
///  Program Notes:-
///  =============
///
///  None.
///
///  See if there is one already in the cache for universal aggregate no.
/// \endverbatim
       const TDbiResultSet* Query(const CP::TVldContext& vc,
                              const TDbi::Task& task,
                              Bool_t findFullTimeWindow = true);
///
///\verbatim
///  Purpose:  Apply extended context query to database table and return result.
///
///  Arguments:
///    context      in    The Validity Context (see CP::TDbiSqlContext)
///    task         in    The task of the query.
///    data         in    Optional SQL extension to secondary query.
///    fillOpts     in    Optional fill options (available to CP::TDbiTableRow)
///
///  Return:    Query result (never zero even if query fails).
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Apply extended context query to database table and return result.
///
///  o Don't save/restore to L2 cache: encoding the query name as a file name
///    would be cumbersome and in any case extended queries are abnormal
///    so optimisation is unwarranted.
///
///
///  Construct the query's "SQL Qualifiers" by forming the 3 strings
///  (which task encoded into the context) into a single semi-colon
///  separated string.
/// \endverbatim                              
       const TDbiResultSet* Query(const string& context,
                              const TDbi::Task& task,
                              const string& data,
                              const string& fillOpts);
///\verbatim
///
///  Purpose:  Apply non-agregate query to database table and return result.
///
///  Arguments:
///    seqNo        in    The sequence number of validity record that satisfies the query.
///    dbNo         in    Database number in the cascade.
///
///  Return:    Query result (never zero even if query fails).
///\endverbatim
       const TDbiResultSet* Query(UInt_t seqNo,UInt_t dbNo);
///\verbatim
///
///  Purpose:  Apply non-agregate query to database table and return result.
///
///  Arguments:
///    vrec         in    The validity record that satisfies the query.
///    canReuse     in    True if result is to be cached.
///
///  Return:    Query result (never zero even if query fails).
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Apply non-aggregated query to main database table. Cache if required,
///    and return result.
///\endverbatim
       const TDbiResultSet* Query(const TDbiValidityRec& vrec,
                              Bool_t canReuse = kTRUE);
///\verbatim
///
///  Purpose:  Determine a suitable Creation Date so that this validity
///            record, if written to the selected DB, will overlay
///            correctly.
///
///  Specification:-
///  =============
///
///  o Determine optimal Creation Date to overlay new data.  See Program Notes.
///
///  Program Notes:-
///  =============
///
///
/// It is normal practice, particularly for calibration data, to have
/// overlapping the validity records.  Each time a new set of runs are
/// processed the start time of the validity is set to the start time of
/// the first run and the end time is set beyond the start time by an
/// interval that characterises the stability of the constants.  So long
/// as a new set of constants is created before the end time is reached
/// there will be no gap.  Where there is an overlap the Creation Date is
/// used to select the later constants on the basis that later is better.
/// However, if reprocessing old data it is also normal practice to
/// process recent data first and in this case the constants for earlier
/// data get later creation dates and overlay works the wrong way.  To
/// solve this, the creation date is faked as follows:-
///
///
///   1.  For new data i.e. data that does not overlay any existing data,
///       the creation date is set to the validity start time.
///
///   2.  For replacement data i.e. data that does overlay existing data,
///       the creation date is set to be one minute greater than the Creation
///       Date on the current best data.
///
/// This scheme ensures that new data will overlay existing data at the
/// start of its validity but will be itself overlaid by data that has
/// a later start time (assuming validity record start times are more
/// than a few minutes apart)
///
///\endverbatim
 
           CP::TVldTimeStamp QueryOverlayCreationDate(const TDbiValidityRec& vrec,
					         UInt_t dbNo);
///
///
///  Purpose:  Refresh meta data for table.
///
 	           void RefreshMetaData();
///\verbatim
///
///  Purpose:  Apply Sql condition to its CP::TDbiDBProxy.
///
///  Arguments:
///   sql           in    SQL condition string (excluding where).
///
///  Return:  n/a
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Apply Sql condition to its CP::TDbiDBProxy.
///
///  Program Notes:-
///  =============
///
///  None.
///\endverbatim
 	           void SetSqlCondition(const string& sql);
                 Bool_t TableExists() const { return fExists; }

protected:

///Constructors (protected because created and owned by TDbiDatabaseManager).
///\verbatim
///
///  Purpose:  Constructor
///
///  Arguments:
///             in  cascader   Reference to one and only cascader
///             in  tableName  Table name.
///             in  tableRow   Example table row object.
///
///  Return:    n/a
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Create table proxy for supplied table name.
///\endverbatim
           TDbiTableProxy(TDbiCascader* cascader,
                         const string& tableName,
                         const TDbiTableRow* tableRow);
  virtual ~TDbiTableProxy();

// State testing member functions

// State changing member functions

private:

// Disabled (not implemented) copy constructor and asignment.

 TDbiTableProxy(const TDbiTableProxy&);
 CP::TDbiTableProxy& operator=(const CP::TDbiTableProxy&);


/// Level 2 (disk) cache management.
 Bool_t CanReadL2Cache() const;
 Bool_t CanWriteL2Cache() const;
///\verbatim
///
///  Purpose: Restore results from named level 2 disk cache into memory cache.
///  Returns true if anything restored
///
///  Specification:-
///  =============
///
///  o Restore to cache but only if enabled and exists.
///\endverbatim
 Bool_t RestoreFromL2Cache(const TDbiValidityRecBuilder& builder);
///\verbatim
///
///  Purpose: Save result to named level 2 cache. Returns true if saved.
///
///  Specification:-
///  =============
///
///  o Save to cache but only if enabled and suitable.
///\endverbatim
 Bool_t SaveToL2Cache(const string& name, TDbiResultSet& res);

// Data members (fMeta* must precede fDBProxy, it has to be created
//               first - see initialiser list)


/// Pointer  to one and only cascader
  TDbiCascader* fCascader;

/// Meta data for main(data) table.
  TDbiTableMetaData fMetaData;

/// Meta data for aux. (validity)table.
  TDbiTableMetaData fMetaValid;

/// True if row supports L2 cache.
   Bool_t fCanL2Cache;

/// Associated cache for result.
  TDbiCache* fCache;

/// Proxy to database
  TDbiDBProxy fDBProxy;

/// true if table exists;
  Bool_t fExists;

/// Table Name
string  fTableName;

/// Pet object used to create new rows.
 TDbiTableRow* fTableRow;

ClassDef(TDbiTableProxy,0)        // Object to query a specific table.

};
};


#endif  // DBITABLEPROXY_H



#ifndef DBICASCADER_H
#define DBICASCADER_H

//////////////////////////////////////////////////////////////////////////
////////////////////////////     ROOT API     ////////////////////////////
//////////////////////////////////////////////////////////////////////////

/**
 *
 *
 * \class ND::TDbiCascader
 *
 *
 * \brief
 * <b>Concept</b> A cascade (prioritorised list) of TDbiConnection s, one
 *  for each database in the cascade.
 *
 * \brief
 * <b>Purpose</b> Implements the concept of a cascade allowing user to
 *   overrride parts of the standard database by introducing higher
 *   priority non-standard ones above it in a cascade.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include <map>
#include <ostream>
using std::ostream;
#include <string>
using std::string;
#include <vector>
using std::vector;

// T2K additions
#include <fstream> 
using std::ifstream;

#if !defined(__CINT__) || defined(__MAKECINT__)
#include "Rtypes.h"
#endif

#include "TDbiConnection.hxx"
#include "TDbiStatement.hxx"
namespace ND {
class TDbiCascader;
ostream& operator<<(ostream& os, const ND::TDbiCascader& cascader) ;
}
class TSQL_Statement;

namespace ND {
class TDbiCascader
{

  friend class TDbiDatabaseManager;  //Only it can create
  friend ostream& operator<<(ostream& s, const ND::TDbiCascader& cascader);

public:

/// Check we can connect to the database defined in the environment variables. Allows a
/// user program to skip using the database if it doesn't need to and it is not there, or
/// if it does need it then die in a civilised manner.
  static bool canConnect();
  friend class TDbiValidate;      //Allow TDbiValidate access SetAuthorisingEntry

  enum Status { kFailed, kClosed, kOpen };

// State testing member functions

// Cascade entry-specific getters.

/// Create a TDbiStatement.  Caller must delete.
 TDbiStatement* CreateStatement(UInt_t dbNo) const;

/// Return associated TDbiConnection. TDbiCascader retains ownership.
const TDbiConnection*
               GetConnection(UInt_t dbNo) const;
      TDbiConnection*
               GetConnection(UInt_t dbNo) ;

        string GetDbName(UInt_t dbNo) const;
         Int_t GetDbNo(const string& dbName) const;
         Int_t GetStatus(UInt_t dbNo) const {
                 if ( dbNo >= GetNumDb() || ! fConnections[dbNo]  ) return kFailed;
                 return fConnections[dbNo]->IsClosed() ? kClosed : kOpen; }
        string GetStatusAsString(UInt_t dbNo) const ;
        string GetURL(UInt_t dbNo) const {
                      return ( dbNo < GetNumDb() ) ? fConnections[dbNo]-> GetUrl(): ""; }
        Bool_t IsTemporaryTable(const string& tableName,
                                Int_t dbNo) const;
// Cascade-wide getters.

         Int_t AllocateSeqNo(const string& tableName,
                             Int_t requireGlobal = 0,
                             Int_t dbNo = 0) const;
         Int_t GetAuthorisingDbNo() const { return fGlobalSeqNoDbNo; }
        UInt_t GetNumDb() const {return fConnections.size();}
	Int_t  GetTableDbNo(const string& tableName, Int_t selectDbNo = -1) const;
        Bool_t TableExists(const string& tableName, Int_t selectDbNo = -1) const {
	                    return this->GetTableDbNo(tableName,selectDbNo) >= 0; }

// State changing member functions

           Int_t CreateTemporaryTable(const string& tableName,
                                      const string& tableDescr);
           int ProcessTmpTblsFile(const string& SQLFilePath); // (T2K Extension)
           int GetTempCon(); // (T2K Extension)
           int ParseTmpTblsSQLLine(const string& line, string &tableName); // (T2K Extension)
           bool ExecTmpTblsSQLStmt(int tempConDbNo, const string& line, const string& tableName); // (T2K Extension)

	   void HoldConnections();
	   void ReleaseConnections();
	   void SetPermanent(UInt_t dbNo, Bool_t permanent = true);

protected:

private:

     Int_t ReserveNextSeqNo(const string& tableName,
                            Bool_t isGlobal,
                            UInt_t dbNo) const;
     void SetAuthorisingEntry(Int_t entry) {fGlobalSeqNoDbNo = entry;}

// Constructors and destructors.
     TDbiCascader(bool beQuiet=false);
  virtual ~TDbiCascader();
TDbiCascader(const TDbiCascader&);  // Not implemented

// Data members

// First connection in the cascade supporting Temporary Tables 
// Access through GetTempCon() only.
  Int_t fTempCon; // (T2K Extension)

/// 1st db in cascade with GlobalSeqNo table
  Int_t fGlobalSeqNoDbNo;

/// Vector of TDbiConnections, one for each DB
  vector<TDbiConnection*> fConnections;

/// Mapping Name->DbNo for temporary tables.
  std::map<string,Int_t> fTemporaryTables;

// (T2K Extension)
// Return codes for ParseTmpTblsSQLLine()
static const int LINE_APPEARS_VALID = 1;
static const int LINE_BLANK = 0;
static const int LINE_INVALID = -1;




/// Private Locker object used by TDbiCascader
class Lock {

public:
       Lock(TDbiStatement* stmtDB, const string& seqnoTable, const string& dataTable);
      ~Lock();

Bool_t IsLocked() const { return fLocked; }

private:

  void SetLock(Bool_t setting = kTRUE);

   TDbiStatement* fStmt;            // Statement to be used to issue lock
          string fSeqnoTableName;  // The SEQNO table that is locked
          string fDataTableName;   // The data table that is locked
          Bool_t fLocked;          // Lock status

};

 ClassDef(TDbiCascader,0)     //A cascade of TDbiConnectionss

};
};


#endif  // DBISKEL_H


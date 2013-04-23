#ifndef DBISQLVALPACKET
#define DBISQLVALPACKET

/**
 *
 *
 * \class CP::TDbiSqlValPacket
 *
 *
 * \brief
 * <b>Concept</b>  The SQL statements to generate a Validity Packet
 *   i.e a single TDbiValidityRec row and its associated main table
 *   data rows.
 *
 * \brief
 * <b>Purpose</b> Used as part of database maintenance as the unit of
 *   transfer between databases.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */


#include <fstream>
#include <list>
#include <vector>
#include <string>
using std::string;

#if !defined(__CINT__) || defined(__MAKECINT__)
#include "Rtypes.h"
#endif

#include "TDbi.hxx"
#include "TDbiTableProxy.hxx"
#include "TVldTimeStamp.hxx"

namespace CP {
class TDbiTableRow;
class TDbiValidityRec;
}
namespace CP {
class TVldRange;
};

namespace CP {
class TDbiSqlValPacket
{

public:

// Types and enum

  typedef enum ECompResult {
    kIdentical,
    kUpdate,
    kOutOfDate,
    kConflict
  } CompResult_t;

// Constructors and destructors.
           TDbiSqlValPacket();
           TDbiSqlValPacket(std::ifstream& is);
           TDbiSqlValPacket(const TDbiValidityRec& vrec);
  virtual ~TDbiSqlValPacket();

// State testing member functions
        Bool_t CanBeStored() const {
             return (fSeqNo > 0 && fNumErrors == 0 && this->GetNumSqlStmts()> 0)
            ? kTRUE : kFALSE; };
  CompResult_t Compare(const TDbiSqlValPacket& that,
                       Bool_t log = kFALSE,
                       const Char_t* thisName = "this",
                       const Char_t* thatName = "that" ) const;
        Bool_t CreateTable(UInt_t dbNo) const;
        UInt_t GetNumErrors() const { return fNumErrors; }
        UInt_t GetNumSqlStmts() const { return fNumStmts; }
        UInt_t GetSeqNo() const { return fSeqNo; }
  CP::TVldTimeStamp GetCreationDate() const { return fCreationDate; }
	string GetStmt(UInt_t stmtNo) const;
	std::vector<string> GetStmtValues(UInt_t stmtNo) const;
 const string& GetTableName() const{ return fTableName; }
        Bool_t IsEqual(const TDbiSqlValPacket& that,
                       Bool_t log = kFALSE,
                       const Char_t* thisName = "this",
                       const Char_t* thatName = "that" ) const;

//  I/O
        Bool_t Fill(std::ifstream& is);
        Bool_t Store(UInt_t dbNo, Bool_t replace = kFALSE) const;
        Bool_t Write(std::ofstream& ios,
                     Bool_t addMetadata = kFALSE) const;

//  Reconstruct.	
          void Recreate(const string& tableName,
                        const CP::TVldRange& vr,
                        Int_t aggNo,
                        TDbi::Task task = 0,
                        CP::TVldTimeStamp creationDate = CP::TVldTimeStamp());
        Bool_t AddDataRow(const TDbiTableProxy& tblProxy,
                          const TDbiValidityRec* vrec,
                          const TDbiTableRow& row);

//  State changing member functions
	  void Clear() { this->Reset(); fNumErrors = 0; }
          void SetEpoch(UInt_t epoch);
          void SetCreationDate(CP::TVldTimeStamp ts);
          void SetSeqNo(UInt_t seqno);
          void Reset();  //Doesn't clear fNumErrors.

  virtual void Print(Option_t *option="") const;

protected:

private:
          void AddRow(const string & row);
        Bool_t AddRow(const TDbiTableProxy& tblProxy,
                      const TDbiValidityRec* vrec,
                      const TDbiTableRow& row);
          void Report(const char* msg,
                      UInt_t line_num,
                      const string& line);
	  void SetMetaData() const;
          void SetSeqNoOnRow(string& row,const string& seqno);

  TDbiSqlValPacket(const TDbiSqlValPacket& );  // Not allowed.


// Data members

/// Number of error encountered while filling.
  UInt_t fNumErrors;

/// Sequence number or 0 if not filled.
  UInt_t fSeqNo;

/// MySQL SQL to create main table. May be empty until needed.
  mutable string fSqlMySqlMetaMain;

/// As fSqlMySqlMetaMain but for aux. table.
  mutable string fSqlMySqlMetaVld;

/// Set of SQL statements to generate packet.
  std::list<string>  fSqlStmts;

/// Number of statements
  UInt_t fNumStmts;

/// Table name or null if not filled.
  string fTableName;

/// Creation date, or object creation date if unfilled.
  CP::TVldTimeStamp fCreationDate;

 ClassDef(TDbiSqlValPacket,0)           // SQL to generate Validity Packet.

};
};
#endif  // DBISQLVALPACKET


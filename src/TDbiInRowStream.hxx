#ifndef DBIRESULTSET_H
#define DBIRESULTSET_H


//////////////////////////////////////////////////////////////////////////
////////////////////////////     ROOT API     ////////////////////////////
//////////////////////////////////////////////////////////////////////////

/**
 *
 *
 * \class CP::TDbiInRowStream
 *
 *
 * \brief
 * <b>Concept</b> Input stream with type checking.
 *
 * \brief
 * <b>Purpose</b> This is an interface to TSQLResultSet. Its primary
 *   purpose is to provide an >> operator with built-type checking to
 *   simplify the writing of TDbiTableRow subclasses.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */


#include <string>

#include "TString.h"

#include "TDbi.hxx"
#include "TDbiRowStream.hxx"

namespace CP {
class TDbiString;
class TDbiStatement;
class TDbiTableMetaData;
class TDbiTableProxy;
}
class TSQLStatement;
namespace CP {
class TVldTimeStamp;
};

namespace CP {
class TDbiInRowStream : public TDbiRowStream
{

public:

// Constructors and destructors.
///\verbatim
///
///  Purpose:  Default constructor
///
///  Arguments:
///     stmtDb     in  CP::TDbiStatement to be used for query.  May be zero.
///     sql        in  The query to be applied to the statement.
///     metaData   in  Meta data for query.
///     tableProxy in  Source CP::TDbiTableProxy.
///     dbNo       in  Cascade no. of source.
///
///  Return:    n/a
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o  Create ResultSet for query.
///\endverbatim
           TDbiInRowStream(TDbiStatement* stmtDb,
                        const TDbiString& sql,
                        const TDbiTableMetaData* metaData,
                        const TDbiTableProxy* tableProxy,
                        UInt_t dbNo,
                        const std::string& fillOpts = "");
  virtual ~TDbiInRowStream();

// State testing member functions
              Bool_t CurColExists() const;
              std::string CurColString() const;
              std::string CurColValue() const { LoadCurValue(); return fValString;}
              UInt_t CurRowNum() const { return fCurRow;}
              UInt_t GetDbNo() const { return fDbNo; }
              std::string GetFillOpts() const { return fFillOpts;}
const TDbiTableProxy* GetTableProxy() const { return fTableProxy; }
/// IsBeforeFirst not needed for ROOT API, but leave a dummy
/// for now so as not to disturb TDbiInRowStream API.
  Bool_t IsBeforeFirst() const { return false; };
              Bool_t IsExhausted() const { return fExhausted; }
	        void RowAsCsv(std::string& row) const;

// State changing member functions

 CP::TDbiInRowStream& operator>>(Bool_t& dest);
 CP::TDbiInRowStream& operator>>(Char_t& dest);
 CP::TDbiInRowStream& operator>>(Short_t& dest);
 CP::TDbiInRowStream& operator>>(UShort_t& dest);
 CP::TDbiInRowStream& operator>>(Int_t& dest);
 CP::TDbiInRowStream& operator>>(UInt_t& dest);
 CP::TDbiInRowStream& operator>>(Long_t& dest);
 CP::TDbiInRowStream& operator>>(ULong_t& dest);
 CP::TDbiInRowStream& operator>>(Float_t& dest);
 CP::TDbiInRowStream& operator>>(Double_t& dest);
 CP::TDbiInRowStream& operator>>(std::string& dest);
 CP::TDbiInRowStream& operator>>(CP::TVldTimeStamp& dest);

  Bool_t FetchRow();

private:

  std::string& AsString(TDbi::DataTypes type);
 Bool_t LoadCurValue() const;
 TString GetStringFromTSQL(Int_t col) const;

// Data members

/// Current row in query (0...)
  Int_t fCurRow;

/// Cascade no. of source
  UInt_t fDbNo;

/// Owned TDbiStatement. May be 0.
/// It's only use is to create a TSQLStatement in the ctor but it is not
/// destroyed until the dtor to prevent a premature attempt to close the
/// connection.
  TDbiStatement* fStatement;

/// Pointer to owned statement, may be 0.
  TSQLStatement* fTSQLStatement;

/// True is result set missing or exhausted.
  Bool_t fExhausted;

/// TDbiTableProxy that produced this set.
  const TDbiTableProxy* fTableProxy;

/// Buffer for assembling value
mutable  std::string fValString;

/// Optional fill options.
  std::string fFillOpts;

ClassDef(TDbiInRowStream,0)     //ResultSet from Query to database table

};
};


#endif  // DBIRESULTSET_H


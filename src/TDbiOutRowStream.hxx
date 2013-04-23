#ifndef DBIOUTROWSTREAM_H
#define DBIOUTROWSTREAM_H

/**
 *
 * $Id: TDbiOutRowStream.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 * \class CP::TDbiOutRowStream
 *
 *
 * \brief
 * <b>Concept</b> Output stream for the values of a single table row
 *  with type checking and deferred sequence number asignment.
 *
 * \brief
 * <b>Purpose</b> This is a helper class CP::for TDbiSqlValPacket. Its
 *  primary purpose is to provide an << operator with built-type
 *  checking to simplify the writing of TDbiTableRow subclasses.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include <string>

#include "TDbi.hxx"
#include "TDbiRowStream.hxx"

namespace CP {
class TDbiTableMetaData;
}
namespace CP {
class TVldTimeStamp;
};
namespace CP {
class TDbiValidityRec;
}

namespace CP {
class TDbiOutRowStream : public TDbiRowStream
{

  friend class TDbiConfigSet; //To allow it to use private Store method

public:

// Constructors and destructors.
           TDbiOutRowStream(const TDbiTableMetaData* metaData);
  virtual ~TDbiOutRowStream();

// State testing member functions
       Bool_t HasGoodData() const { return ! fBadData && IsComplete(); }
const std::string& GetCSV() const { return fCSV; }
       Bool_t IsComplete() const { return CurColNum() == NumCols()+1; }

// State changing member functions

CP::TDbiOutRowStream& operator<<(Bool_t src);
CP::TDbiOutRowStream& operator<<(Char_t src);
CP::TDbiOutRowStream& operator<<(const Char_t* src);
CP::TDbiOutRowStream& operator<<(Short_t src);
CP::TDbiOutRowStream& operator<<(UShort_t src);
CP::TDbiOutRowStream& operator<<(Int_t src);
CP::TDbiOutRowStream& operator<<(UInt_t src);
/*  TDbiOutRowStream& operator<<(Long_t& src); */
/*  TDbiOutRowStream& operator<<(ULong_t& src); */
TDbiOutRowStream& operator<<(Float_t src);
TDbiOutRowStream& operator<<(Double_t src);
TDbiOutRowStream& operator<<(const std::string& src);
TDbiOutRowStream& operator<<(const CP::TVldTimeStamp& src);

            void Clear() { fBadData = kFALSE; fCSV.erase(); ClearCurCol();}

private:

// State testing member functions

// State changing member functions
   void Store(const std::string& str);
 Bool_t StoreDefaultIfInvalid(TDbi::DataTypes type);

// Data members

/// Set KTRUE if streamed bad data
  Bool_t fBadData;

/// Comma separated list of values.
  std::string fCSV;

ClassDef(TDbiOutRowStream,0)  //Output stream for single table row

};
};


#endif  // DBIOUTROWSTREAM_H


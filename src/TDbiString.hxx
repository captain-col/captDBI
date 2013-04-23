// $Id: TDbiString.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $

#ifndef DBISTRING
#define DBISTRING

/**
 *
 * $Id: TDbiString.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 * \class CP::TDbiString
 *
 *
 * \brief
 * <b>Concept</b> Output string stream  - string that can be assembled
 * from basic types.
 *
 * \brief
 * <b>Purpose</b> This is used to assemble SQL commands.  It was written
 * to plug what was then a deficiency in gcc ostrstream.  It could be removed
 * now if I had the energy.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include <string>

#ifndef ROOT_Rtypes
#if !defined(__CINT__) || defined(__MAKECINT__)
#include "Rtypes.h"
#endif
#endif

namespace CP {
class TDbiString
{

public:

// Constructors and destructors.
           TDbiString();
           TDbiString(const Char_t* str);
           TDbiString(const std::string& str);
  virtual ~TDbiString();

// State testing member functions
  const Char_t* c_str() const { return fString.c_str(); }
  const std::string& GetString() const { return fString; }

// State changing member functions
  CP::TDbiString& operator<<(Int_t data);
  CP::TDbiString& operator<<(UInt_t data);
  CP::TDbiString& operator<<(Float_t data);
  CP::TDbiString& operator<<(Char_t data);
  CP::TDbiString& operator<<(const Char_t* data);
  CP::TDbiString& operator<<(const std::string& data);
        void Clear() { fString.clear(); }
  std::string& GetString() { return fString; }

private:


// Data members

/// The underlying string
 std::string fString;

 ClassDef(TDbiString,0)     // output string stream

};
};


#endif // DBISTRING



#ifndef DBIBINARYFILE_H
#define DBIBINARYFILE_H

/**
 *
 * \class ND::TDbiBinaryFile
 *
 *
 * \brief
 * <b>Concept</b> Binary file for reading/writing TDbi related objects.
 *
 * \brief
 * <b>Purpose</b> To save/restore cache to speed up startup when running
 *   in the same context.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */


#include <fstream>
using std::fstream;
using std::ios_base;

#include <string>
using std::string;

#include <vector>
using std::vector;

#ifndef ROOT_Rtypes
#if !defined(__CINT__) || defined(__MAKECINT__)
#include "Rtypes.h"
#endif
#endif

namespace ND {
class TDbiTableRow;
}
namespace ND {
class TVldTimeStamp;
};
namespace ND {
class TVldRange;
};

namespace ND {
class TDbiBinaryFile
{

public:
///
///  Purpose:  Default Constructor.
///
///  Arguments:
///    fileName     in    File name (default: "" => file is a dummy)
///    input        in    true if reading (default = kTRUE)
///
///  Specification:-
///  =============
///
///  If file name or fgWorkDir is dummy, or the appropriate access is not set
/// then name is set to dummy otherwise fgWorkDir is prepended to the name.
  TDbiBinaryFile(const char* fileName= "",
                Bool_t input = kTRUE);
  ~TDbiBinaryFile();

// State testing.
  Bool_t  IsOK() const { return ! fHasErrors;}
  Bool_t  IsReading() const { return this->IsOK() && fReading; }
  Bool_t  IsWriting() const { return this->IsOK() && ! fReading; }

// State changing.

  void Close();

/// Builtin data type I/O.

  ND::TDbiBinaryFile& operator >> (Bool_t& num);
  ND::TDbiBinaryFile& operator << (const Bool_t& num);
  ND::TDbiBinaryFile& operator >> (Int_t& num);
  ND::TDbiBinaryFile& operator << (const Int_t& num);
  ND::TDbiBinaryFile& operator >> (UInt_t& num);
  ND::TDbiBinaryFile& operator << (const UInt_t& num);
  ND::TDbiBinaryFile& operator >> (Double_t& num);
  ND::TDbiBinaryFile& operator << (const Double_t& num);

/// Simple Virtual object I/O.
/// (i.e. object with vptr but only built-in data types)

  ND::TDbiBinaryFile& operator >> (ND::TVldTimeStamp& ts);
  ND::TDbiBinaryFile& operator << (const ND::TVldTimeStamp& ts);

/// String I/O.
/// Warning: Implimentation assumes that string does not contain
///          a null character.

  ND::TDbiBinaryFile& operator >> (string& str);
  ND::TDbiBinaryFile& operator << (const string& str);

/// Compound object I/O.

  ND::TDbiBinaryFile& operator >> (ND::TVldRange& vr);
  ND::TDbiBinaryFile& operator << (const ND::TVldRange& vr);

// Vector I/O.
///\brief Read a vector of objects inheriting from ND::TDbiTableRow.
///
///\verbatim
/// Vector I/O.
/// ***********
///
///
///  Purpose: Read a vector of objects inheriting from ND::TDbiTableRow.
///
///   NB:     On entry, array must be empty.
///
///
///           The objects are written into a buffer that is a contiguous
///           area of memory that is allocated to receive it. After a
///           successful read the user must call ReleaseArrayBuffer to take
///           control over this buffer as it will be automatically release
///           when the next array input occurs otherwise.
///
///  For the format of record see the operator <<.
///\endverbatim
  ND::TDbiBinaryFile& operator >>  (vector<ND::TDbiTableRow*>& arr);
///\verbatim
///
///  Purpose: Write a vector of objects inheriting from ND::TDbiTableRow.
///
///  Format of record:-
///
///  Int_t   StartMarker  Start of record marker = 0xaabbccdd
///  Int_t   arrSize      Size of vector
///
///  If size of vector > 0 this is folowed by:-
///
///  string  objName      Name of object
///  Int_t   objSize      Size of object
///  char*                The data arrSize*objSize bytes long
///
///  The record concludes:-
///
///  Int_t     EndMarker  End of record marker = 0xddbbccaa
///\endverbatim
  ND::TDbiBinaryFile& operator <<  (vector<ND::TDbiTableRow*>& arr);
  char* ReleaseArrayBuffer() { char* buff = fArrayBuffer;
                               fArrayBuffer = 0;
                               return buff; }

/// Global control of all created TDbiBinaryFile objects.

  static Bool_t CanReadL2Cache()  { return fgWorkDir.size() && fgReadAccess; }
  static Bool_t CanWriteL2Cache() { return fgWorkDir.size() && fgWriteAccess; }
  static   void SetWorkDir(const string& dir) { fgWorkDir = dir;
                  if ( fgWorkDir[fgWorkDir.size()-1] != '/' ) fgWorkDir += '/'; }
  static   void SetReadAccess(Bool_t access = kTRUE) { fgReadAccess = access; }
  static   void SetWriteAccess(Bool_t access = kTRUE) { fgWriteAccess = access; }

private:

/// The functions that do the low-level I/O.

 Bool_t CanRead();
 Bool_t CanWrite();
   void CheckFileStatus();

 Bool_t Read(char* bytes, UInt_t numBytes);
 Bool_t Write(const char* bytes, UInt_t numBytes);

// CINT does not recognise fstream; only ifstream and ofstream.
#if !defined(__CINT__)

/// Associated file, may be null.
  fstream*  fFile;
#endif

 Bool_t   fReading;
 Bool_t   fHasErrors;
 char*    fArrayBuffer;
 string   fFileName;

 static string fgWorkDir;    //Level 2 Cache directory or null if none.
 static Bool_t fgReadAccess; //Have read access if true.
 static Bool_t fgWriteAccess;//Have write access if true.

};
};

#endif  // DBIBINARYFILE_HfArrayBuffer


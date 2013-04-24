#ifndef DBIBINARYFILE_H
#define DBIBINARYFILE_H

/// \class CP::TDbiBinaryFile
///
///
/// \brief
/// <b>Concept</b> Binary file for reading/writing TDbi related objects.
///
/// \brief
/// <b>Purpose</b> To save/restore cache to speed up startup when running
///   in the same context.
///
/// Contact: A.Finch@lancaster.ac.uk


#include <fstream>
#include <string>
#include <vector>

#ifndef ROOT_Rtypes
#if !defined(__CINT__) || defined(__MAKECINT__)
#include "Rtypes.h"
#endif
#endif

namespace CP {
    class TDbiTableRow;
    class TVldTimeStamp;
    class TVldRange;
    class TDbiBinaryFile;
};

class CP::TDbiBinaryFile {

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
    ///  If file name or fgWorkDir is dummy, or the appropriate access is not
    /// set then name is set to dummy otherwise fgWorkDir is prepended to the
    /// name.
    TDbiBinaryFile(const char* fileName= "",
                   Bool_t input = kTRUE);
    ~TDbiBinaryFile();

    /// State testing.
    std::string  GetFileName() const {
        return fFileName;
    }
    Bool_t  IsOK() const {
        return ! fHasErrors;
    }
    Bool_t  IsReading() const {
        return this->IsOK() && fReading;
    }
    Bool_t  IsWriting() const {
        return this->IsOK() && ! fReading;
    }

    /// State changing.

    void Close();

    /// Builtin data type I/O.

    CP::TDbiBinaryFile& operator >> (Bool_t& num);
    CP::TDbiBinaryFile& operator << (const Bool_t& num);
    CP::TDbiBinaryFile& operator >> (Int_t& num);
    CP::TDbiBinaryFile& operator << (const Int_t& num);
    CP::TDbiBinaryFile& operator >> (UInt_t& num);
    CP::TDbiBinaryFile& operator << (const UInt_t& num);
    CP::TDbiBinaryFile& operator >> (Double_t& num);
    CP::TDbiBinaryFile& operator << (const Double_t& num);

    /// Simple Virtual object I/O.
    /// (i.e. object with vptr but only built-in data types)

    CP::TDbiBinaryFile& operator >> (CP::TVldTimeStamp& ts);
    CP::TDbiBinaryFile& operator << (const CP::TVldTimeStamp& ts);

    /// String I/O.
    /// Warning: Implimentation assumes that std::string does not contain
    ///          a null character.

    CP::TDbiBinaryFile& operator >> (std::string& str);
    CP::TDbiBinaryFile& operator << (const std::string& str);

    /// Compound object I/O.

    CP::TDbiBinaryFile& operator >> (CP::TVldRange& vr);
    CP::TDbiBinaryFile& operator << (const CP::TVldRange& vr);

    /// Vector I/O.
    ///\brief Read a vector of objects inheriting from CP::TDbiTableRow.
    ///
    ///\verbatim
    /// Vector I/O.
    ///
    ///
    ///  Purpose: Read a vector of objects inheriting from CP::TDbiTableRow.
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
    CP::TDbiBinaryFile& operator >> (std::vector<CP::TDbiTableRow*>& arr);
    ///\verbatim
    ///
    ///  Purpose: Write a vector of objects inheriting from CP::TDbiTableRow.
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
    CP::TDbiBinaryFile& operator << (std::vector<CP::TDbiTableRow*>& arr);
    char* ReleaseArrayBuffer() {
        char* buff = fArrayBuffer;
        fArrayBuffer = 0;
        return buff;
    }

    /// Global control of all created TDbiBinaryFile objects.

    static Bool_t CanReadL2Cache()  {
        return fgWorkDir.size() && fgReadAccess;
    }
    static Bool_t CanWriteL2Cache() {
        return fgWorkDir.size() && fgWriteAccess;
    }
    static   void SetWorkDir(const std::string& dir) {
        fgWorkDir = dir;
        if (fgWorkDir[fgWorkDir.size()-1] != '/') {
            fgWorkDir += '/';
        }
    }
    static   void SetReadAccess(Bool_t access = kTRUE) {
        fgReadAccess = access;
    }
    static   void SetWriteAccess(Bool_t access = kTRUE) {
        fgWriteAccess = access;
    }

private:

    /// The functions that do the low-level I/O.

    Bool_t CanRead();
    Bool_t CanWrite();
    void CheckFileStatus();

    Bool_t Read(char* bytes, UInt_t numBytes);
    Bool_t Write(const char* bytes, UInt_t numBytes);

    /// CINT does not recognise fstream; only ifstream and ofstream.
#if !defined(__CINT__)

    /// Associated file, may be null.
    std::fstream*  fFile;
#endif

    Bool_t   fReading;
    Bool_t   fHasErrors;
    char*    fArrayBuffer;
    std::string   fFileName;

    static std::string fgWorkDir;    //Level 2 Cache directory or null if none.
    static Bool_t fgReadAccess; //Have read access if true.
    static Bool_t fgWriteAccess;//Have write access if true.

};

#endif


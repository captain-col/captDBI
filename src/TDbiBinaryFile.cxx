// $Id: TDbiBinaryFile.cxx,v 1.2 2013/04/19 09:44:20 finch Exp $

#include <iostream>

#include "TClass.h"
#include "TObject.h"
#include "Api.h"
#include "TSystem.h"

#include "TDbiBinaryFile.hxx"
#include "TDbiTableRow.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>
#include "TVldRange.hxx"
#include "TVldTimeStamp.hxx"

enum Markers { StartMarker = 0xaabbccdd,
               EndMarker   = 0xddbbccaa
             };

//   Local utilities.
//   ***************

void* GetVTptr(const void* obj) {

//  Return an object's virtual table pointer.

    void* ptr;
    memcpy(&ptr,obj,4);
    return ptr;

}
void SetVTptr(void* obj, const void* vt) {

//  Set an object's virtual table pointer.

    memcpy(obj,&vt,4);

}
//   Definition of static data members
//   *********************************

std::string CP::TDbiBinaryFile::fgWorkDir;
Bool_t CP::TDbiBinaryFile::fgReadAccess  = kTRUE;
Bool_t CP::TDbiBinaryFile::fgWriteAccess = kTRUE;



// Definition of member functions (is same order as TDbiBinaryFile.hxx)
// *****************************************************************


//.....................................................................

CP::TDbiBinaryFile::TDbiBinaryFile(const char* fileName, Bool_t input) :
    fFile(0),
    fReading(input),
    fHasErrors(kFALSE),
    fArrayBuffer(0) {
//
//
//  Purpose:  Default Constructor.
//
//  Arguments:
//    fileName     in    File name (default: "" => file is a dummy)
//    input        in    true if reading (default = kTRUE)

//  Specification:-
//  =============
//
//  If file name or fgWorkDir is dummy, or the appropriate access is not set
//  then name is set to dummy otherwise fgWorkDir is prepended to the name.

    // Complete the file name.
    fFileName = fileName;
    if (fFileName != "") {
        Bool_t access = input ? fgReadAccess : fgWriteAccess;
        if (fgWorkDir == "" || ! access) {
            fFileName = "";
        }
        else {
            fFileName = fgWorkDir +  fFileName;
        }
    }

    // Open the file.
    std::ios_base::openmode mode = std::ios_base::in|std::ios_base::binary;
    if (! input) {
        mode = std::ios_base::out|std::ios_base::binary;
    }

    if (fFileName == "") {
        fHasErrors = kTRUE;
    }
    else {
        fFile = new fstream(fFileName.c_str(),mode);
        if (! fFile->is_open() || ! fFile->good()) {
            DbiDebug("Cannot open " << fFileName
                     << "; all I/O will fail." << "  ");
            fHasErrors = kTRUE;
        }
    }

}
//.....................................................................
///  Purpose:  Default Destructor.
CP::TDbiBinaryFile::~TDbiBinaryFile() {
//
//


    delete[] fArrayBuffer;
    fArrayBuffer = 0;
    this->Close();
    delete fFile;
    fFile = 0;

}
//.....................................................................
///  Purpose:  Close file.
void CP::TDbiBinaryFile::Close() {
//
//


    if (fFile) {
        fFile->close();
    }
}

//  Builtin data type I/O.
//  **********************

#define READ_BUILTIN(t)                                   \
    \
    CP::TDbiBinaryFile& CP::TDbiBinaryFile::operator >> (t& v) {        \
        UInt_t numBytes = sizeof(v);                            \
        char* bytes = reinterpret_cast<char*>(&v);              \
        this->Read(bytes,numBytes);                             \
        return *this;                                           \
    }
#define WRITE_BUILTIN(t)                                  \
    \
    CP::TDbiBinaryFile& CP::TDbiBinaryFile::operator << (const t& v) {  \
        UInt_t numBytes = sizeof(v);                            \
        const char* bytes = reinterpret_cast<const char*>(&v);  \
        this->Write(bytes,numBytes);                            \
        return *this;                                           \
    }

READ_BUILTIN(Bool_t)
WRITE_BUILTIN(Bool_t)
READ_BUILTIN(Int_t)
WRITE_BUILTIN(Int_t)
READ_BUILTIN(UInt_t)
WRITE_BUILTIN(UInt_t)
READ_BUILTIN(Double_t)
WRITE_BUILTIN(Double_t)


//  Simple Virtual object I/O
//  *************************

#define READ_SIMPLE(t)                                    \
    \
    CP::TDbiBinaryFile& CP::TDbiBinaryFile::operator >> (t& v) {        \
        void* vt = GetVTptr(&v);                                \
        UInt_t numBytes = sizeof(v);                            \
        char* bytes = reinterpret_cast<char*>(&v);              \
        this->Read(bytes,numBytes);                             \
        SetVTptr(&v,vt);                                        \
        return *this;                                           \
    }
#define WRITE_SIMPLE(t)                                   \
    \
    CP::TDbiBinaryFile& CP::TDbiBinaryFile::operator << (const t& v) {  \
        UInt_t numBytes = sizeof(v);                            \
        const char* bytes = reinterpret_cast<const char*>(&v);  \
        this->Write(bytes,numBytes);                            \
        return *this;                                           \
    }

READ_SIMPLE(CP::TVldTimeStamp)
WRITE_BUILTIN(CP::TVldTimeStamp)


//  String I/O.
//  ***********

//.....................................................................

CP::TDbiBinaryFile& CP::TDbiBinaryFile::operator >> (std::string& str) {

    if (this->CanRead()) {
        getline(*fFile,str,'\0');
        this->CheckFileStatus();
    }
    return *this;
}
//.....................................................................

CP::TDbiBinaryFile& CP::TDbiBinaryFile::operator << (const std::string& str) {

    UInt_t numBytes = str.size()+1;
    this->Write(str.c_str(),numBytes);
    return *this;
}

//.....................................................................

CP::TDbiBinaryFile& CP::TDbiBinaryFile::operator >> (CP::TVldRange& vr) {

    if (this->CanRead()) {
        Int_t        detectorMask;
        Int_t        simMask;
        CP::TVldTimeStamp timeStart;
        CP::TVldTimeStamp timeEnd;
        std::string str;
        (*this) >> detectorMask
                >> simMask
                >> timeStart
                >> timeEnd
                >> str;
        TString dataSource(str.c_str());
        CP::TVldRange tmp(detectorMask,simMask,timeStart,timeEnd,dataSource);
        vr = tmp;
    }
    return *this;
}
//.....................................................................

CP::TDbiBinaryFile& CP::TDbiBinaryFile::operator << (const CP::TVldRange& vr) {

    if (this->CanWrite()) {
        std::string str(vr.GetDataSource().Data());
        (*this) << vr.GetDetectorMask()
                << vr.GetSimMask()
                << vr.GetTimeStart()
                << vr.GetTimeEnd()
                << str;
    }
    return *this;
}

//.....................................................................

CP::TDbiBinaryFile& CP::TDbiBinaryFile::operator >> (std::vector<CP::TDbiTableRow*>& arr) {


    if (! this->CanRead()) {
        return *this;
    }

    if (arr.size()) {
        DbiSevere("Attempting to read into non-empty array" << "  ");
        return *this;
    }

// Check for start of array marker.

    UInt_t marker = 0;
    (*this) >> marker;
    if (marker != StartMarker) {
        DbiSevere("Cannot find start of array marker" << "  ");
        this->Close();
        this->CheckFileStatus();
        return *this;
    }

//  Get array size and deal with non-empty arrays.

    Int_t arrSize = 0;
    (*this) >> arrSize;

    if (arrSize) {
        Int_t objSize  = 0;
        std::string objName;
        (*this) >> objName >> objSize;

//  Ensure that sizes look sensible and use ROOT to instatiate
//  an example object so that we can get the address of the
//  virtual table.

        TClass objClass(objName.c_str());
        Int_t objSizefromRoot = objClass.Size();
        void* obj = objClass.New();
        void* vt  = GetVTptr(obj);
//  This only works if the address of the sub-class object is the same
//  as the underlying base class, which should be true in this simple case.
        CP::TDbiTableRow* tr = reinterpret_cast<CP::TDbiTableRow*>(obj);
        delete tr;

        DbiVerbose("Restoring array of " << arrSize << " "
                   << objName << " objects"
                   << "  VTaddr " << std::ios::hex << vt << std::ios::dec
                   << " object size "  << objSize << "(from file) "
                   << objSizefromRoot << "(from ROOT)"
                   << "  ");

        if (arrSize < 0 || objSize != objSizefromRoot) {
            DbiSevere("Illegal  array size ("<< arrSize
                      << ") or object size(" << objSize
                      << "," << objSizefromRoot << ")" << "  ");
            this->Close();
            this->CheckFileStatus();
            return *this;
        }

//  Allocate buffer and load in array.
        delete[] fArrayBuffer;
        Int_t buffSize = arrSize*objSize;
        fArrayBuffer = new char[buffSize];
        this->Read(fArrayBuffer,buffSize);

//  Fix up VT pointers and populate the vector.

        char* elem = fArrayBuffer;
        arr.reserve(arrSize);
        for (int row = 0; row < arrSize; ++row) {
            SetVTptr(elem,vt);
            arr.push_back(reinterpret_cast<CP::TDbiTableRow*>(elem));
            elem += objSize;
        }

    }

//  Check for end of array marker.

    (*this) >> marker;
    if (marker != EndMarker) {
        DbiSevere("Cannot find end of array marker" << "  ");
        this->Close();
        this->CheckFileStatus();
    }

    return *this;

}

///.....................................................................
CP::TDbiBinaryFile& CP::TDbiBinaryFile::operator << (std::vector<CP::TDbiTableRow*>& arr) {


    if (! this->CanWrite()) {
        return *this;
    }

    UInt_t marker = StartMarker;
    (*this) << marker;
    Int_t arrSize = arr.size();
    (*this) << arrSize;

    if (arrSize) {
        CP::TDbiTableRow* obj = arr[0];
        Int_t objSize  = obj->IsA()->Size();
        std::string objName = obj->ClassName();
        (*this) << objName << objSize;
        for (int row = 0; row < arrSize; ++row) {
            obj = arr[row];
            const char* p = reinterpret_cast<const char*>(arr[row]);
            this->Write(p,objSize);
        }

    }

    marker = EndMarker;
    (*this) << marker;

    return *this;

}

// The functions that do the low-level I/O.
// ****************************************

//.....................................................................

Bool_t CP::TDbiBinaryFile::CanRead() {

    if (! fReading) {
        DbiSevere("Attempting to read from a write-only file" << "  ");
        return kFALSE;
    }
    return this->IsOK();

}
//.....................................................................

Bool_t CP::TDbiBinaryFile::CanWrite() {

    if (fReading) {
        DbiSevere("Attempting to write to a read-only file" << "  ");
        return kFALSE;
    }
    return this->IsOK();

}

//.....................................................................

void CP::TDbiBinaryFile::CheckFileStatus() {

//  If file was good but has just gone bad, report and close it.
//  Delete it if writing.

    if (fFile
        && ! fHasErrors
        && (! fFile->is_open() || ! fFile->good())) {
        DbiSevere("File not open or has gone bad,"
                  << " all further I/O will fail." << "  ");
        fHasErrors = kTRUE;
        this->Close();

        //Delete file if writing.
        if (! fReading) {
            DbiSevere("Erasing " << fFileName << "  ");
            gSystem->Unlink(fFileName.c_str());
        }

    }

}

//.....................................................................

Bool_t CP::TDbiBinaryFile::Read(char* bytes, UInt_t numBytes) {
//
//
//  Purpose: Low-level I/O with error checking.
//

    if (! this->CanRead()) {
        return kFALSE;
    }

    fFile->read(bytes,numBytes);
    this->CheckFileStatus();
    return ! fHasErrors;
}

//.....................................................................

Bool_t CP::TDbiBinaryFile::Write(const char* bytes, UInt_t numBytes) {
//
//
//  Purpose: Low-level I/O with error checking.
//

    if (! this->CanWrite()) {
        return kFALSE;
    }

    fFile->write(bytes,numBytes);
    this->CheckFileStatus();
    return ! fHasErrors;
}



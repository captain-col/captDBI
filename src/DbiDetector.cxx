//
// Body for Detector namespace so that CINT recognizes its existence
//
#include "DbiDetector.hxx"
#include "TString.h"

//_____________________________________________________________________________
Int_t CP::DbiDetector::FullMask() {
    return kmCAPTAIN|kCAPTAIN;
}
//_____________________________________________________________________________
const Char_t* CP::DbiDetector::AsString(Detector_t detector) {
    switch (detector) {
    case kUnknown:   return "Unknown";    break;
    case kmCAPTAIN:  return "mCAPTAIN";   break;
    case kCAPTAIN:   return "CAPTAIN";    break;
    default:         return "?Unknown?";  break;
    }
}

#ifdef JUNK
//_____________________________________________________________________________
CP::DbiDetector::Detector_t CP::DbiDetector::CharToEnum(Char_t c) {
    switch (c) {
    case 'N':
    case 'n':
    case '1':
    case 0x01:
        return kNear;
    case 'F':
    case 'f':
    case '2':
    case 0x02:
        return kFar;
    case 'C':
    case 'c':
    case '4':
    case 0x04:
        return kCalDet;
    case 'T':
    case 't':
    case '8':
    case 0x08:
        return kTestStand;
    case 'M':
    case 'm':
    case 0x10:
        return kMapper;
    default:
        return kUnknown;
    }
}
#endif

//_____________________________________________________________________________
const Char_t* CP::DbiDetector::MaskToString(Int_t mask) {
    // Return a mask of Detector as a string
    //
    // Result is a pointer to a statically allocated string.
    // User should copy this into their own buffer before calling
    // this method again.

    static Char_t newstring[255] = "";

    Char_t* ptr = newstring;  // start at the beginning

    *ptr = 0; // start with nothing
    Int_t fullmask = CP::DbiDetector::FullMask();

    for (Int_t i=0; i<32; i++) {
        CP::DbiDetector::Detector_t adet = (CP::DbiDetector::Detector_t)(1<<i);
        if (mask & adet & fullmask) {
            const Char_t* toadd = CP::DbiDetector::AsString(adet);
            if (ptr != newstring) {
                *ptr++ = ',';
            }
            strcpy(ptr,toadd);
            ptr += strlen(toadd);
        }
    }
    *ptr++ = 0; // ensure trailing 0

    return newstring;
}

//_____________________________________________________________________________


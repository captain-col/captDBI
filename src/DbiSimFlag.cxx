//
// Body for SimFlag class so that CINT recognizes its existence
//
#include "DbiSimFlag.hxx"
#include "TMath.h"
#include "TString.h"

//_____________________________________________________________________________

Int_t CP::DbiSimFlag::FullMask() {
    return kData|kDaqFakeData|kMC|kReroot;
}

//_____________________________________________________________________________

const Char_t* CP::DbiSimFlag::AsString(SimFlag_t simFlag) {
    // static function to return mapping enum --> string

    switch (simFlag) {
    case kUnknown:     return "Unknown";     break;
    case kData:        return "Data";        break;
    case kDaqFakeData: return "DaqFakeData"; break;
    case kMC:          return "MC";          break;
    case kReroot:      return "Reroot";      break;
    default:           return "?Data?";      break;
    }
}

//_____________________________________________________________________________

const Char_t* CP::DbiSimFlag::MaskToString(Int_t mask) {
    // Return a mask of SimFlags as a string
    //
    // Result is a pointer to a statically allocated string.
    // User should copy this into their own buffer before calling
    // this method again.

    static Char_t newstring[255] = "";

    Char_t* ptr = newstring;  // start at the beginning

    *ptr = 0; // start with nothing
    Int_t fullmask = CP::DbiSimFlag::FullMask();

    for (Int_t i=0; i<32; i++) {
        CP::DbiSimFlag::SimFlag_t flag = (CP::DbiSimFlag::SimFlag_t)(1<<i);
        if (mask & flag & fullmask) {
            const Char_t* toadd = CP::DbiSimFlag::AsString(flag);
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

CP::DbiSimFlag::SimFlag_t CP::DbiSimFlag::StringToEnum(const Char_t* chars, Int_t maxChar) {
    // convert a set of chars to a valid enum

    Int_t mask = CP::DbiSimFlag::StringToMask(chars,maxChar);

    switch (mask) {
    case kUnknown:     return kUnknown;     break;
    case kData:        return kData;        break;
    case kDaqFakeData: return kDaqFakeData; break;
    case kMC:          return kMC;          break;
    case kReroot:      return kReroot;      break;
    default:           return kUnknown;     break;
    }

}

//_____________________________________________________________________________

Int_t CP::DbiSimFlag::StringToMask(const Char_t* chars, Int_t maxChar) {
    // convert a set of chars to a mask of enum's
    // simple tests for unique characters: {d,f,m,r}

    Int_t mask  = 0;

    TString thestring(chars);
    if (maxChar>0 && maxChar<thestring.Length()) {
        thestring.Resize(maxChar);
    }

    thestring.ToLower();
    if (thestring.Contains("d")) {
        mask |= kData;
    }
    if (thestring.Contains("f")) {
        mask |= kDaqFakeData;
    }
    if (thestring.Contains("m")) {
        mask |= kMC;
    }
    if (thestring.Contains("r")) {
        mask |= kReroot;
    }

    return mask;
}

//_____________________________________________________________________________

Int_t CP::DbiSimFlag::Compact(SimFlag_t simFlag) {
    // turn mask-like enum value into sequential int

    switch (simFlag) {
    case kUnknown:     return -1;  break;
    case kData:        return  0;  break;
    case kDaqFakeData: return  1;  break;
    case kMC:          return  2;  break;
    case kReroot:      return  3;  break;
    default:           return -1;  break;
    }

}

//_____________________________________________________________________________

CP::DbiSimFlag::SimFlag_t CP::DbiSimFlag::Expand(Int_t compactSimFlag) {
    // turn sequential int into mask-like enum value

    switch (compactSimFlag) {
    case  0:  return kData;        break;
    case  1:  return kDaqFakeData; break;
    case  2:  return kMC;          break;
    case  3:  return kReroot;      break;
    default:  return kUnknown;     break;
    }
}

//_____________________________________________________________________________

//
// Body for Detector namespace so that CINT recognizes its existence
//
#include "DbiDetector.hxx"
#include "TString.h"

//_____________________________________________________________________________
Int_t ND::DbiDetector::FullMask()
{
   return kNear|kFar|kCalib|kTestStand|kMapper;
}
//_____________________________________________________________________________
const Char_t* ND::DbiDetector::AsString(Detector_t detector)
{
   switch (detector) {
   case kUnknown:   return "Unknown";    break;
   case kNear:      return "Near";       break;
   case kFar:       return "Far";        break;
   case kCalDet:    return "CalDet";     break;
   case kTestStand: return "TestStand";  break;
   case kMapper:    return "Mapper";     break;
   default:         return "?Unknown?";  break;
   }
}

//_____________________________________________________________________________
ND::DbiDetector::Detector_t ND::DbiDetector::CharToEnum(Char_t c)
{
  switch(c) {
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

//_____________________________________________________________________________
Char_t* ND::DbiDetector::MaskToString(Int_t mask)
{
   // Return a mask of Detector as a string
   //
   // Result is a pointer to a statically allocated string.
   // User should copy this into their own buffer before calling
   // this method again.

   static Char_t newstring[255] = "";
   
   Char_t* ptr = newstring;  // start at the beginning

   *ptr = 0; // start with nothing
   Int_t fullmask = ND::DbiDetector::FullMask();
   
   for (Int_t i=0; i<32; i++) {
      ND::DbiDetector::Detector_t adet = (ND::DbiDetector::Detector_t)(1<<i);
      if (mask & adet & fullmask) {
         const Char_t* toadd = ND::DbiDetector::AsString(adet);
         if (ptr != newstring) *ptr++ = ',';
         strcpy(ptr,toadd);
         ptr += strlen(toadd);
      }
   }
   *ptr++ = 0; // ensure trailing 0

   return newstring;
}

//_____________________________________________________________________________
ND::DbiDetector::Detector_t ND::DbiDetector::StringToEnum(const Char_t* chars, Int_t maxChar)
{
   // convert a set of chars to a valid enum

   Int_t mask = ND::DbiDetector::StringToMask(chars,maxChar);

   switch (mask) {
   case kUnknown:   return kUnknown;    break;
   case kNear:      return kNear;       break;
   case kFar:       return kFar;        break;
   case kCalib:     return kCalib;      break;
   case kTestStand: return kTestStand;  break;
   case kMapper:    return kMapper;     break;
   default:         return kUnknown;    break;
   }

}

//_____________________________________________________________________________
Int_t ND::DbiDetector::StringToMask(const Char_t* chars, Int_t maxChar)
{
   // convert a set of chars to a mask of enum's
   // simple tests for unique characters: {n,f,c,t,m}

   Int_t mask  = 0;

   TString thestring(chars);
   if (maxChar>0 && maxChar<thestring.Length()) thestring.Resize(maxChar);

   thestring.ToLower();
   if (thestring.Contains("n")) mask |= kNear;
   if (thestring.Contains("f")) mask |= kFar;
   if (thestring.Contains("c")) mask |= kCalib;
   // For the unusual case of TestStand require something more than 
   // just the first char.  Sue K. points out that passing "CalDet" 
   // (as opposed to "Calib") triggers kTestStand if we don't
   // make this additional requirement.
   //if (thestring.Contains("t")) mask |= kTestStand;
   if (thestring.Contains("test")) mask |= kTestStand;
   if (thestring.Contains("m")) mask |= kMapper;



   return mask;
}

//_____________________________________________________________________________


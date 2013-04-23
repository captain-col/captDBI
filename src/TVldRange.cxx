////////////////////////////////////////////////////////////////////////////
// $Id: TVldRange.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $
//
// CP::TVldRange
//
// CP::TVldRange is delimits/identifies the allowed "context" values
// associated with the associated DBI returned information
//
// Author:  R. Hatcher 2000.05.03
//
////////////////////////////////////////////////////////////////////////////

#include "TVldRange.hxx"
#include "TVldContext.hxx"

#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;
//CVSID("$Id: TVldRange.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $");

ClassImp(CP::TVldRange)

//_____________________________________________________________________________
std::ostream& operator<<(std::ostream& os, const CP::TVldRange& vldr)
{
  return os << vldr.AsString();
}

//_____________________________________________________________________________
CP::TVldRange::TVldRange()
   : fDetectorMask(0), fSimMask(0),
     fTimeStart(), fTimeEnd(), fDataSource("unknown")
{
   // Default constructor
}
//_____________________________________________________________________________
CP::TVldRange::TVldRange(const Int_t detMask, const Int_t simMask,
                         const CP::TVldTimeStamp &tstart,
                         const CP::TVldTimeStamp &tend,
                         const TString &source)
   : fDetectorMask(detMask), fSimMask(simMask),
     fTimeStart(tstart), fTimeEnd(tend), fDataSource(source)
{
   // normal constructor
}

//_____________________________________________________________________________
CP::TVldRange::~TVldRange()
{
   // delete all the owned sub-objects

}

//_____________________________________________________________________________
const char* CP::TVldRange::AsString(Option_t *option) const
{
   // Return the CP::TVldRange as a string
   //
   // Result is a pointer to a statically allocated string.
   // User should copy this into their own buffer before calling
   // this method again.
   //
   // option "a": give detector/simflag masks as alpha chars
   // option "c": compact (single line)
   // option "s": drop nsec part of times
   // option "1": include only "start time"
   // option "-": don't include "source" info

   static char newstring[255] = " ";

   TString opt = option;
   opt.ToLower();

   Bool_t opt_a = opt.Contains("a");
   Bool_t opt_c = opt.Contains("c");

   TString detbits;
   if (opt_a) {
     detbits = CP::DbiDetector::MaskToString(fDetectorMask);
   }
   else {
      sprintf(newstring,"det %#4.4x",fDetectorMask);
      detbits = newstring;
   }

   TString simbits;
   if (opt_a) {
     simbits = CP::DbiSimFlag::MaskToString(fSimMask);
   }
   else {
      sprintf(newstring,"sim %#4.4x",fSimMask);
      simbits = newstring;
   }

   // CP::TVldTimeStamp::AsString returns pointer to statically allocated string
   // one needs to copy this before calling it again in same function call
   static char timeopt[4] = "c  ";
   timeopt[0] = (opt.Contains("s")?'s':'c');
   TString start_str = fTimeStart.AsString(timeopt);
   TString end_str;
   if ( ! opt.Contains("1")) {
      end_str = fTimeEnd.AsString(timeopt);
      if ( !opt_c ) end_str.Prepend("\n\t ");
      else          end_str.Prepend(" ");
   }
   if ( ! opt_c ) start_str.Prepend("\n\t ");

   TString source;
   if ( ! opt.Contains("-")) {
      source +=  (opt_c) ? " '" : "\n\t from source: ";
      source += fDataSource;
      source +=  (opt_c) ? "'" : "";
   }

   sprintf(newstring,
              "|%s|%s|%s%s%s",
              (const char*)detbits,
              (const char*)simbits,
              (const char*)start_str,
              (const char*)end_str,
              (const char*)source);

   return newstring;
}

//_____________________________________________________________________________
Bool_t CP::TVldRange::IsCompatible(const CP::TVldContext &vldc) const
{
   // compare CP::TVldContext with this CP::TVldRange to see if the
   // the tagged set is compatible

   Int_t detector = (Int_t)vldc.GetDetector();
   Int_t simflag  = (Int_t)vldc.GetSimFlag();

   // account for case where both CP::TVldContext and CP::TVldRange
   // are using "kUnknown" which has no bits set
   if ( ! (detector & fDetectorMask) &&
        (detector      != CP::DbiDetector::kUnknown ||
         fDetectorMask != CP::DbiDetector::kUnknown    ) ) return kFALSE;
   if ( ! (simflag  & fSimMask) &&
        (simflag  != CP::DbiSimFlag::kUnknown ||
         fSimMask != CP::DbiSimFlag::kUnknown   ) ) return kFALSE;

   // the start time is taken as inclusive, but the end time is exclusive

   if ( vldc.GetTimeStamp() <  fTimeStart ) return kFALSE;
   if ( vldc.GetTimeStamp() >= fTimeEnd   ) return kFALSE;

   return kTRUE;
}

//_____________________________________________________________________________
Bool_t CP::TVldRange::IsCompatible(const CP::TVldContext *vldc) const
{
   // compare CP::TVldContext with this CP::TVldRange to see if the
   // the tagged set is compatible

   return IsCompatible(*vldc);
}

//_____________________________________________________________________________
void CP::TVldRange::Print(Option_t *option) const
{
   // Print this object

   printf("%s\n",AsString(option));
}

//_____________________________________________________________________________
void CP::TVldRange::TrimTo(const CP::TVldRange& vldr)
{
   // Trim this range to the intersection (ie. more restricted)
   // limits of it's initial value and that of the argument

   fDetectorMask &= vldr.fDetectorMask;
   fSimMask      &= vldr.fSimMask;
   if (fTimeStart < vldr.fTimeStart) fTimeStart = vldr.fTimeStart;
   if (fTimeEnd   > vldr.fTimeEnd  ) fTimeEnd   = vldr.fTimeEnd;
   if (!fDataSource.Contains(vldr.fDataSource)) {
      fDataSource   += ", ";
      fDataSource   += vldr.fDataSource;
   }
}

//_____________________________________________________________________________


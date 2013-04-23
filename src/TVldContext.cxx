////////////////////////////////////////////////////////////////////////////
// $Id: TVldContext.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $
//
// ND::TVldContext
//
// ND::TVldContext is tag for identifying the required "context" when
// interfacing with the DBI
//
// Author:  R. Hatcher 2000.05.03
//
////////////////////////////////////////////////////////////////////////////

#include "TVldContext.hxx"

#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;
//CVSID("$Id: TVldContext.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $");

ClassImp(ND::TVldContext)

//_____________________________________________________________________________
std::ostream& operator<<(std::ostream& os, const ND::TVldContext& vldc)
{
   if (os.good()) {
      if (os.tie()) os.tie()->flush(); // instead of opfx
      os << vldc.AsString();
   }
   // instead of os.osfx()
   if (os.flags() & std::ios::unitbuf) os.flush();
   return os;
}

//_____________________________________________________________________________
ND::TVldContext::TVldContext(const ND::TEventContext& context)
  : fDetector(ND::DbiDetector::kNear), fSimFlag(ND::DbiSimFlag::kData), fTimeStamp(context.GetTimeStamp(),0)
{
   // constructor from T2K context
   if(context.GetPartition() & ND::TEventContext::kMCData)
      fSimFlag = ND::DbiSimFlag::kMC;
}

//_____________________________________________________________________________
ND::TVldContext::TVldContext(const ND::DbiDetector::Detector_t &detector,
                       const ND::DbiSimFlag::SimFlag_t mcFlag,
                       const ND::TVldTimeStamp &tstamp)
   : fDetector(detector), fSimFlag(mcFlag), fTimeStamp(tstamp)
{
   // normal constructor
}

//_____________________________________________________________________________
const char* ND::TVldContext::AsString(Option_t *option) const
{
   // Return a formatted string of the contents of this object
   // User should copy result because it points to a
   // statically allocated string.

   static char newstring[80] = " ";

   switch (option[0]) {
   case 'c':
   case 'C':
     sprintf(newstring,"{%c%c %s}",
             ND::DbiDetector::AsString(GetDetector())[0],
             ND::DbiSimFlag::AsString(GetSimFlag())[0],
             fTimeStamp.AsString("c"));
     break;
   default:
     sprintf(newstring,"{%6.6s|%6.6s|%s}",
             ND::DbiDetector::AsString(GetDetector()),
             ND::DbiSimFlag::AsString(GetSimFlag()),
             fTimeStamp.AsString("c"));
   }

   return newstring;
}

//_____________________________________________________________________________
void ND::TVldContext::Print(Option_t *option) const
{
   // Print this object

   printf("%s\n",AsString(option));

}

//_____________________________________________________________________________
Bool_t ND::TVldContext::IsNull() const
{
   // Return true if this was initialized by default ctor
   // we can only test detector type and simflag
   return fDetector==ND::DbiDetector::kUnknown && fSimFlag == ND::DbiSimFlag::kUnknown;

}

//_____________________________________________________________________________



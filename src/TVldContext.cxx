////////////////////////////////////////////////////////////////////////////
// $Id: TVldContext.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $
//
// CP::TVldContext
//
// CP::TVldContext is the tag for identifying the required "context" when
// interfacing with the DBI
//
// Author:  R. Hatcher 2000.05.03
//
////////////////////////////////////////////////////////////////////////////

#include "TVldContext.hxx"

#include <TDbiLog.hxx>
#include <MsgFormat.hxx>

ClassImp(CP::TVldContext)

//_____________________________________________________________________________
std::ostream& CP::operator<<(std::ostream& os, const CP::TVldContext& vldc) {
    if (os.good()) {
        if (os.tie()) {
            os.tie()->flush();   // instead of opfx
        }
        os << vldc.AsString();
    }
    // instead of os.osfx()
    if (os.flags() & std::ios::unitbuf) {
        os.flush();
    }
    return os;
}

//_____________________________________________________________________________
CP::TVldContext::TVldContext(const CP::TEventContext& context)
    : fDetector(CP::DbiDetector::kUnknown), 
      fSimFlag(CP::DbiSimFlag::kData), 
      fTimeStamp(context.GetTimeStamp(),0) {

    // constructor from T2K context
    if (context.IsMC()) {
        fSimFlag = CP::DbiSimFlag::kMC;
    }

    if (context.IsMiniCAPTAIN()) {
        fDetector = CP::DbiDetector::kmCAPTAIN;
    }
    if (context.IsCAPTAIN()) {
        fDetector = CP::DbiDetector::kCAPTAIN;
    }

}

//_____________________________________________________________________________
CP::TVldContext::TVldContext(const CP::DbiDetector::Detector_t& detector,
                             const CP::DbiSimFlag::SimFlag_t mcFlag,
                             const CP::TVldTimeStamp& tstamp)
    : fDetector(detector), fSimFlag(mcFlag), fTimeStamp(tstamp) {
    // normal constructor
}

//_____________________________________________________________________________
const char* CP::TVldContext::AsString(Option_t* option) const {
    // Return a formatted string of the contents of this object
    // User should copy result because it points to a
    // statically allocated string.

    static char newstring[80] = " ";

    switch (option[0]) {
    case 'c':
    case 'C':
        sprintf(newstring,"{%c%c %s}",
                CP::DbiDetector::AsString(GetDetector())[0],
                CP::DbiSimFlag::AsString(GetSimFlag())[0],
                fTimeStamp.AsString("c"));
        break;
    default:
        sprintf(newstring,"{%6.6s|%6.6s|%s}",
                CP::DbiDetector::AsString(GetDetector()),
                CP::DbiSimFlag::AsString(GetSimFlag()),
                fTimeStamp.AsString("c"));
    }

    return newstring;
}

//_____________________________________________________________________________
void CP::TVldContext::Print(Option_t* option) const {
    // Print this object

    printf("%s\n",AsString(option));

}

//_____________________________________________________________________________
Bool_t CP::TVldContext::IsNull() const {
    // Return true if this was initialized by default ctor
    // we can only test detector type and simflag
    return fDetector==CP::DbiDetector::kUnknown
        && fSimFlag == CP::DbiSimFlag::kUnknown;

}

//_____________________________________________________________________________



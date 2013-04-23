/**
 * \class ND::TVldContext
 *
 * \ingroup Validity
 *
 * \brief A tag for identifying the required "context" when interfacing with the DBI.
 *
 * 
 *
 * \author (last to touch it) $Author: finch $
 *
 * \version $Revision: 1.1 $
 *
 * \date $Date: 2011/01/18 05:49:20 $
 *
 * Contact: R. Hatcher
 *
 * Created on: 2000.05.03
 *
 */

#ifndef VLDCONTEXT_H
#define VLDCONTEXT_H

#include "TObject.h"
#include "DbiDetector.hxx"
#include "DbiSimFlag.hxx"
#include "TVldTimeStamp.hxx"

#include "TEventContext.hxx"

// make the TVldContext cout'able
#include <iosfwd>

namespace ND {
class TVldContext;

class TVldContext : public TObject {

 public:

   TVldContext();                        // necessary for streamer io
   TVldContext(const ND::TEventContext& context);
   TVldContext(const ND::DbiDetector::Detector_t &detector,
              const ND::DbiSimFlag::SimFlag_t mcflag,
              const ND::TVldTimeStamp &time);
   virtual ~TVldContext();

   /// Return a formatted string of the contents of this object
   /// User should copy result because it points to a 
   /// statically allocated string.
   const char* AsString(Option_t *option = "") const;

   ND::DbiDetector::Detector_t     GetDetector()  const { return fDetector;}
   ND::DbiSimFlag::SimFlag_t       GetSimFlag()   const { return fSimFlag;}
   ND::TVldTimeStamp             GetTimeStamp() const { return fTimeStamp;}

   /// Return true if this was initialized by default ctor
   /// we can only test detector type and simflag
   Bool_t                   IsNull() const;
   /// Return opposite of IsNull()
   Bool_t                   IsValid() const { return !IsNull(); }

   void                     Print(Option_t *option = "") const;

   friend Bool_t operator< (const TVldContext &lhs, const TVldContext &rhs);
   friend Bool_t operator==(const TVldContext &lhs, const TVldContext &rhs);
   friend Bool_t operator!=(const TVldContext &lhs, const TVldContext &rhs);
   friend Bool_t operator<=(const TVldContext &lhs, const TVldContext &rhs);
   friend Bool_t operator> (const TVldContext &lhs, const TVldContext &rhs);
   friend Bool_t operator>=(const TVldContext &lhs, const TVldContext &rhs);

 protected:
   //   TVldContext(const TObject &object);           // hide these
   //   TVldContext(const TVldContext &tf);
   //   TVldContext& operator=(const TObject &rhs);
   //   TVldContext& operator=(const TVldContext &tf);

   ND::DbiDetector::Detector_t       fDetector;
   ND::DbiSimFlag::SimFlag_t         fSimFlag;
   ND::TVldTimeStamp               fTimeStamp;

 private:

   ClassDef(ND::TVldContext,2)  // ND::TVldTimeStamp+Detector+SimFlag

};
}; std::ostream& operator<<(std::ostream& os, const ND::TVldContext& vldts);

#ifndef __CINT__
//=============================================================================
// Implementation details -- inlines need to be hidden from CINT
//=============================================================================
inline Bool_t ND::operator< (const ND::TVldContext &lhs, const ND::TVldContext &rhs)
{ 
   // sorting is a little tricky with three fields
   // this sorts first by time then detector then simflag
   // *WARNING* be very careful if you change this
   if ( lhs.fTimeStamp  < rhs.fTimeStamp ) return true;
   if ( lhs.fTimeStamp == rhs.fTimeStamp ) {
      if ( lhs.fDetector  < rhs.fDetector ) return true;
      if ( lhs.fDetector == rhs.fDetector ) {
         if (lhs.fSimFlag  < rhs.fSimFlag ) return true;
      }
   }
   return false;
}

inline Bool_t ND::operator==(const ND::TVldContext &lhs, const ND::TVldContext &rhs)
{ 
   // equal if all components match
   return 
     lhs.fDetector  == rhs.fDetector &&
     lhs.fSimFlag   == rhs.fSimFlag  &&
     lhs.fTimeStamp == rhs.fTimeStamp;
}

inline Bool_t ND::operator!=(const ND::TVldContext &lhs, const ND::TVldContext &rhs)
{ 
   // not equal if any component doesn't match
   return 
     lhs.fDetector  != rhs.fDetector ||
     lhs.fSimFlag   != rhs.fSimFlag  ||
     lhs.fTimeStamp != rhs.fTimeStamp;
}

inline Bool_t ND::operator<=(const ND::TVldContext &lhs, const ND::TVldContext &rhs)
{
  return (lhs<rhs) || (lhs==rhs);
}

inline Bool_t ND::operator>(const ND::TVldContext &lhs, const ND::TVldContext &rhs)
{
  return !(lhs<rhs) && !(lhs==rhs);
}

inline Bool_t ND::operator>=(const ND::TVldContext &lhs, const ND::TVldContext &rhs)
{
  return !(lhs<rhs);
}

inline ND::TVldContext::TVldContext() 
  :  fDetector(ND::DbiDetector::kUnknown), 
     fSimFlag(ND::DbiSimFlag::kUnknown), 
     fTimeStamp() {
   // Default constructor (unknown detector, simflag, timestamp=now)
}

inline ND::TVldContext::~TVldContext() { ; }

#endif /* __CINT__ */
#endif // VLDCONTEXT_H

/**
 * \class CP::TVldContext
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

namespace CP {
    class TVldContext;
    std::ostream& operator<<(std::ostream& os, const CP::TVldContext& vldts);

    class TVldContext : public TObject {
    public:

        TVldContext();                        // necessary for streamer io
        TVldContext(const CP::TEventContext& context);
        TVldContext(const CP::DbiDetector::Detector_t& detector,
                    const CP::DbiSimFlag::SimFlag_t mcflag,
                    const CP::TVldTimeStamp& time);
        virtual ~TVldContext();

        /// Return a formatted string of the contents of this object
        /// User should copy result because it points to a
        /// statically allocated string.
        const char* AsString(Option_t* option = "") const;

        CP::DbiDetector::Detector_t     GetDetector()  const {
            return fDetector;
        }
        CP::DbiSimFlag::SimFlag_t       GetSimFlag()   const {
            return fSimFlag;
        }
        CP::TVldTimeStamp             GetTimeStamp() const {
            return fTimeStamp;
        }

        /// Return true if this was initialized by default ctor
        /// we can only test detector type and simflag
        Bool_t                   IsNull() const;
        /// Return opposite of IsNull()
        Bool_t                   IsValid() const {
            return !IsNull();
        }

        void                     Print(Option_t* option = "") const;

        friend Bool_t operator< (const TVldContext& lhs, const TVldContext& rhs);
        friend Bool_t operator==(const TVldContext& lhs, const TVldContext& rhs);
        friend Bool_t operator!=(const TVldContext& lhs, const TVldContext& rhs);
        friend Bool_t operator<=(const TVldContext& lhs, const TVldContext& rhs);
        friend Bool_t operator> (const TVldContext& lhs, const TVldContext& rhs);
        friend Bool_t operator>=(const TVldContext& lhs, const TVldContext& rhs);

    protected:
        //   TVldContext(const TObject &object);           // hide these
        //   TVldContext(const TVldContext &tf);
        //   TVldContext& operator=(const TObject &rhs);
        //   TVldContext& operator=(const TVldContext &tf);

        CP::DbiDetector::Detector_t       fDetector;
        CP::DbiSimFlag::SimFlag_t         fSimFlag;
        CP::TVldTimeStamp               fTimeStamp;

    private:

        ClassDef(CP::TVldContext,2)  // CP::TVldTimeStamp+Detector+SimFlag

    };
};

#ifndef __CINT__
//=============================================================================
// Implementation details -- inlines need to be hidden from CINT
//=============================================================================
inline Bool_t CP::operator< (const CP::TVldContext& lhs, const CP::TVldContext& rhs) {
    // sorting is a little tricky with three fields
    // this sorts first by time then detector then simflag
    // *WARNING* be very careful if you change this
    if (lhs.fTimeStamp  < rhs.fTimeStamp) {
        return true;
    }
    if (lhs.fTimeStamp == rhs.fTimeStamp) {
        if (lhs.fDetector  < rhs.fDetector) {
            return true;
        }
        if (lhs.fDetector == rhs.fDetector) {
            if (lhs.fSimFlag  < rhs.fSimFlag) {
                return true;
            }
        }
    }
    return false;
}

inline Bool_t CP::operator==(const CP::TVldContext& lhs, const CP::TVldContext& rhs) {
    // equal if all components match
    return
        lhs.fDetector  == rhs.fDetector &&
        lhs.fSimFlag   == rhs.fSimFlag  &&
        lhs.fTimeStamp == rhs.fTimeStamp;
}

inline Bool_t CP::operator!=(const CP::TVldContext& lhs, const CP::TVldContext& rhs) {
    // not equal if any component doesn't match
    return
        lhs.fDetector  != rhs.fDetector ||
        lhs.fSimFlag   != rhs.fSimFlag  ||
        lhs.fTimeStamp != rhs.fTimeStamp;
}

inline Bool_t CP::operator<=(const CP::TVldContext& lhs, const CP::TVldContext& rhs) {
    return (lhs<rhs) || (lhs==rhs);
}

inline Bool_t CP::operator>(const CP::TVldContext& lhs, const CP::TVldContext& rhs) {
    return !(lhs<rhs) && !(lhs==rhs);
}

inline Bool_t CP::operator>=(const CP::TVldContext& lhs, const CP::TVldContext& rhs) {
    return !(lhs<rhs);
}

inline CP::TVldContext::TVldContext()
    :  fDetector(CP::DbiDetector::kUnknown),
       fSimFlag(CP::DbiSimFlag::kUnknown),
       fTimeStamp() {
    // Default constructor (unknown detector, simflag, timestamp=now)
}

inline CP::TVldContext::~TVldContext() {
    ;
}

#endif /* __CINT__ */
#endif // VLDCONTEXT_H

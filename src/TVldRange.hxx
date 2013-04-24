/**
 * \class CP::TVldRange
 *
 * \ingroup Validity
 *
 * \brief Delimits and identifies the allowed "context" values
 * associated with the associated DBI returned information
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

#ifndef VLDRANGE_H
#define VLDRANGE_H

#include "TObject.h"
#include "TString.h"
#include "DbiDetector.hxx"
#include "TVldTimeStamp.hxx"
#include <iosfwd>

namespace CP {
    class TVldContext;

    class TVldRange;

    class TVldRange : public TObject {

    public:

        TVldRange();                        // necessary for streamer io
        TVldRange(const Int_t detMask, const Int_t simMask,
                  const TVldTimeStamp& tstart, const TVldTimeStamp& tend,
                  const TString& source);
        virtual ~TVldRange();

        /** Return the TVldRange as a string
         *
         * Result is a pointer to a statically allocated string.
         * User should copy this into their own buffer before calling
         * this method again.
         *
         * option "a": give detector/simflag masks as alpha chars
         * option "c": compact (single line)
         * option "s": drop nsec part of times
         * option "1": include only "start time"
         * option "-": don't include "source" info
         */
        const char* AsString(Option_t* option = "") const;

        /** Compare CP::TVldContext with this TVldRange to see if the
         * the tagged set is compatible.*/
        Bool_t IsCompatible(const CP::TVldContext& vldc) const;
        /** Compare CP::TVldContext with this TVldRange to see if the
         * the tagged set is compatible.*/
        Bool_t IsCompatible(const CP::TVldContext* vldc) const;

        Int_t  GetDetectorMask()    const {
            return fDetectorMask;
        }
        Int_t  GetSimMask()         const {
            return fSimMask;
        }
        TVldTimeStamp GetTimeStart() const {
            return fTimeStart;
        }
        TVldTimeStamp GetTimeEnd()   const {
            return fTimeEnd;
        }
        TString GetDataSource()     const {
            return fDataSource;
        }

        /// Print to stdout.  See AsString()  for options.
        void Print(Option_t* option = "") const;

        /// Trim this range to the intersection (ie. more restricted)
        /// limits of it's initial value and that of the argument
        void TrimTo(const TVldRange& vldr);
        void SetTimeStart(const TVldTimeStamp& tstart) {
            fTimeStart = tstart;
        }
        void SetTimeEnd(const TVldTimeStamp& tend) {
            fTimeEnd = tend;
        }

    protected:

        Int_t              fDetectorMask; /// or's of CP::DbiDetector::Detector_t
        Int_t              fSimMask;      /// or's of data/mc conditions
        TVldTimeStamp       fTimeStart;
        TVldTimeStamp       fTimeEnd;
        TString            fDataSource;

    private:

        ClassDef(CP::TVldRange,1)  // TVldRange version 1
    };

    std::ostream& operator<<(std::ostream& os, const CP::TVldRange& vldts);

};

#endif // VLDRANGE_H

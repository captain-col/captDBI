////////////////////////////////////////////////////////////////////////////
// $Id: DbiDetector.hxx,v 1.1 2011/01/18 05:49:19 finch Exp $
//
// Detector
//
// Detector defines Detector_t which is an enumeration of the
// legal detector configurations
//
// Author:  R. Hatcher 2000.05.03
//
////////////////////////////////////////////////////////////////////////////

#ifndef DETECTOR_H
#define DETECTOR_H

#ifndef ROOT_Rtypes
#if !defined(__CINT__) || defined(__MAKECINT__)
#include "Rtypes.h"
#endif
#endif


namespace CP {
    namespace DbiDetector {
        typedef enum EDetector {
            kUnknown   = 0x00,
            kmCAPTAIN  = 0x01,
            kCAPTAIN   = 0x02,
            kDefault   = 0x01,  // Set the default mask for TDbiConfigStream...
        } Detector_t;

        /// Return the DbiDetector mask with all bits set.
        Int_t       FullMask();

        /// Translate the detector enum into a static constant character array.
        const Char_t*          AsString(Detector_t detector);

        /// Translate the detector enum as a pure integer into a static
        /// character array that is valid until the next call to this method.
        const Char_t*          MaskToString(Int_t mask);

    }

    // make "DetectorType" a synonym for "Detector"
    namespace DbiDetectorType = DbiDetector;
}

#endif // DETECTOR_H

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

// Make "DetectorType" a synonym for "Detector"
//typedef DetectorType Detector;
//#define DetectorType Detector

namespace CP {
    namespace DbiDetector {
        typedef enum EDetector {
            kUnknown   = 0x00,
            kNear      = 0x01,
            kFar       = 0x02,
            kCalDet    = 0x04,
            kCalib     = 0x04,
            kTestStand = 0x08,
            kMapper    = 0x10
        } Detector_t;

        // no ctor or dtor's - this class consists of only static members

        Int_t       FullMask();

        // Translation enum to/from character strings

        const Char_t*          AsString(Detector_t detector);
        DbiDetector::Detector_t   CharToEnum(Char_t c);
        Char_t*                MaskToString(Int_t mask);
        DbiDetector::Detector_t   StringToEnum(const Char_t* chars, Int_t maxChar=0);
        Int_t                  StringToMask(const Char_t* chars, Int_t maxChar=0);

    }

// make "DetectorType" a synonym for "Detector"
    namespace DbiDetectorType = DbiDetector;
}

#endif // DETECTOR_H

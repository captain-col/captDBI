////////////////////////////////////////////////////////////////////////////
// $Id: DbiSimFlag.hxx,v 1.1 2011/01/18 05:49:19 finch Exp $
//
// SimFlag
//
// SimFlag defines SimFlag_t which is an enumeration of the
// legal data sources
//
// Author:  R. Hatcher 2000.05.09
//
////////////////////////////////////////////////////////////////////////////

#ifndef SIMFLAG_H
#define SIMFLAG_H

#ifndef ROOT_Rtypes
#if !defined(__CINT__) || defined(__MAKECINT__)
#include "Rtypes.h"
#endif
#endif

namespace CP{
namespace DbiSimFlag {

   typedef enum ESimFlag {
      kUnknown      = 0x00,
      kData         = 0x01,
      kDaqFakeData  = 0x02, 
      kMC           = 0x04,
      kReroot       = 0x08
   } SimFlag_t;

   Int_t       FullMask();

   // Translation enum to/from character strings

   const Char_t*       AsString(SimFlag_t detector);
   const Char_t*       MaskToString(Int_t mask);
   DbiSimFlag::SimFlag_t  StringToEnum(const Char_t* chars, Int_t maxChar=0);
   Int_t               StringToMask(const Char_t* chars, Int_t maxChar=0);

   // Translation from mask to compact value
   Int_t               Compact(SimFlag_t simFlag);
   DbiSimFlag::SimFlag_t  Expand(Int_t compactSimFlag);

}
}

#endif // SIMFLAG_H

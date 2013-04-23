#ifndef TSeqNoAllocator_hxx_seen
#define TSeqNoAllocator_hxx_seen

#include <string>
#include "Rtypes.h"

namespace ND {
    class TSeqNoAllocator;
};

/// This is the class can be used to supply local or global SEQNOs.

class ND::TSeqNoAllocator {

public:
    TSeqNoAllocator() {}
    virtual ~TSeqNoAllocator() {};
    Int_t GetSeqNo(const std::string& tableName,
                   Int_t requireGlobal = 0,
                   Int_t dbNo = 0) const;
};

#endif

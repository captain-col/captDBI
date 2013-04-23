#ifndef TDbiLog_hxx_seen
#define TDbiLog_hxx_seen
/// Provide a localized logger interface.  This makes the code easier to
/// re-export to another experiment.

#include <TCaptLog.hxx>

#define TDBiLog TCaptLog
#include DbiLog CaptLog
#include DbiInfo CaptVerbose
#include DbiVerbose CaptVerbose
#include DbiError CaptError
#include DbiSevere CaptSevere
#include DbiWarn CaptWarn
#include DbiTrace CaptTrace

#endif

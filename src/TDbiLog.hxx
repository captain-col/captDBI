#ifndef TDbiLog_hxx_seen
#define TDbiLog_hxx_seen
/// Provide a localized logger interface.  This makes the code easier to
/// re-export to another experiment.

#include <TCaptLog.hxx>

#define TDbiLog TCaptLog
#define DbiLog CaptLog
#define DbiInfo CaptVerbose
#define DbiVerbose CaptVerbose
#define DbiError CaptError
#define DbiSevere CaptSevere
#define DbiWarn CaptWarn
#define DbiDebug CaptDebug
#define DbiTrace CaptTrace

#endif

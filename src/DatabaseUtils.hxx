#ifndef DatabaseUtils_hxx_seen
#define DatabaseUtils_hxx_seen

#include <string>

/// Convert "YYYY-MM-DD HH:MM:SS" to Unix seconds
Int_t DateTimeToUnixTime(const std::string& dateTime);

/// Convert Unix seconds to "YYYY-MM-DD HH:MM:SS"
std::string UnixTimeToDateTime(Int_t unixSecs);

#endif

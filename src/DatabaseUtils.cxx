#include "TDbi.hxx"

#include "TVldTimeStamp.hxx"

Int_t DateTimeToUnixTime(const std::string& dateTime) {
    return TDbi::MakeTimeStamp(dateTime).GetSec();
}

std::string UnixTimeToDateTime(Int_t unixSecs) {
    return TDbi::MakeDateTimeString(CP::TVldTimeStamp(unixSecs,0));
}



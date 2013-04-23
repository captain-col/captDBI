#include <TDbiLog.hxx>
#include "TSeqNoAllocator.hxx"
#include "Rtypes.h"

#include <cstdlib>
#include <iostream>
#include <string>

/// Standalone utility to issue local and global SEQNOs.

/// Invocation: 
///   seqno=`allocate_seq_no.exe <tableName> { <reqGlobal> <dbNo> } | tail -1` 

/// Where:-
///   tableName   in    The table for which the SEQNO is required.
///   reqGlobal   in    The type of SEQNO required:-
///                        > 0  Must be global
///                        = 0  Must be global if supplied dbNo is authorising
///                             and table isn't temporary otherwise local
///                        < 0  Must be local
///   dbNo        in    The entry in the cascade for which the SEQNO 
///                        is required.
///
///   seqno       out    The allocated SEQNO (or 0 if unable to allocate one)
///
///   | tail -1       Is need to filter the output and keep only the last line.


int main(int argc, char **argv) {
    CP::TDbiLog::SetDebugLevel(CP::TDbiLog::WarnLevel);
    CP::TDbiLog::SetLogLevel(CP::TDbiLog::QuietLevel);
    if ( argc < 2 ) {
        CaptError("ERROR: Insufficient arguments to allocate_seq_no.exe.");
        std::cout << "0" << std::endl;
        return 1;
    }
    std::string table_name(argv[1]);
    Int_t requireGlobal = 0;
    Int_t dbNo          = 0;
    if ( argc > 2 ) requireGlobal = atoi(argv[2]);
    if ( argc > 3 ) dbNo          = atoi(argv[3]);
    CP::TSeqNoAllocator sna = CP::TSeqNoAllocator();
    Int_t seqno = sna.GetSeqNo(table_name,requireGlobal,dbNo);
    std::cout << seqno << std::endl;
    return 0;
}

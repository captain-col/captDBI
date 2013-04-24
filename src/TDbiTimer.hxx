// $Id: TDbiTimer.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $

#ifndef DBITIMER
#define DBITIMER

////////////////////////////////////////////////////////////////////////
/// \class CP::TDbiTimer
///
///
/// \brief <b> Concept: </b> Simple timer, record query progress and final times.
///
/// <b> Purpose: </b> To find out why this is all soooo sssllloooowwww!
///
////////////////////////////////////////////////////////////////////////

#include <string>

#include "TStopwatch.h"
#include "TDbiFieldType.hxx"

namespace CP {
    class TDbiTableMetaData;
}

namespace CP {
    class TDbiTimer {

    public:

// Constructors and destructors.
        TDbiTimer();
        virtual ~TDbiTimer();

// State testing member functions

// State changing member functions
        void RecBegin(std::string tableName, UInt_t rowSize);
        void RecEnd(UInt_t numRows);
        void RecFillAgg(Int_t aggNo);
        void RecMainQuery();
        void Resume();
        void StartSubWatch(UInt_t subWatch);
        void Suspend();

// Global timer used for all queries to the DBI.

    private:

        enum  QueryStage { kPassive, kInitialQuery, kMainQuery };
        enum { kMaxSubWatch = 4 };  // Must be > 0.

// Data members

        Int_t fCurSubWatch; // Current SubWatch or -1 if disabled.
        UInt_t fRowSize;     // Size of row object
        QueryStage fQueryStage;  // Query stage
        TStopwatch fSubWatches[kMaxSubWatch]; // Partial timers
        std::string fTableName;   // Current table
        TStopwatch fWatch;       // Timer

        ClassDef(TDbiTimer,0)     // Simple query timer

    };
};


#endif // DBITIMER



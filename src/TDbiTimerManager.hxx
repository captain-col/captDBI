// $Id: TDbiTimerManager.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $

#ifndef DBITIMERMANAGER
#define DBITIMERMANAGER

////////////////////////////////////////////////////////////////////////
/// \class CP::TDbiTimerManager
///
/// \brief <b> Concept: </b>Manager of a set of simple timers.
///
/// <b> Purpose: </b> To find out why this is all soooo sssllloooowwww!
///
////////////////////////////////////////////////////////////////////////

#include <string>
#include <list>

namespace CP {
    class TDbiTableMetaData;
    class TDbiTimer;
}

namespace CP {
    class TDbiTimerManager {

    public:

// Constructors and destructors.
        TDbiTimerManager();
        virtual ~TDbiTimerManager();

// State testing member functions

// State changing member functions
        void Enable(Bool_t flag=kTRUE,
                    Bool_t enableSubWatch = kFALSE) {
            fEnabled = flag;
            fSubWatchEnabled = enableSubWatch;
        }
        void RecBegin(std::string tableName, UInt_t rowSize);
        void RecEnd(UInt_t numRows);
        void RecFillAgg(Int_t aggNo);
        void RecMainQuery();
        void StartSubWatch(UInt_t subWatch);

// Global timer manager used for all queries to the DBI.

        static TDbiTimerManager gTimerManager;

    private:

        TDbiTimer* GetCurrent();
        TDbiTimer* Pop();
        TDbiTimer* Push();

// Data members

        Bool_t fEnabled;     // Enable/disable
        Bool_t fSubWatchEnabled;
        // SubWatch Enable/disable (not used now).
        std::list<TDbiTimer*> fTimers;      // Push-down stack of timers.

        ClassDef(TDbiTimerManager,0)    // Simple query timer

    };
};


#endif // DBITIMERMANAGER



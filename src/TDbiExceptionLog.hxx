#ifndef DBIEXCEPTIONLOG
#define DBIEXCEPTIONLOG

//////////////////////////////////////////////////////////////////////////
////////////////////////////     ROOT API     ////////////////////////////
//////////////////////////////////////////////////////////////////////////

///
/// Id: TDbiExceptionLog.hxx,v 1.1 2011/01/18 05:49:19 finch Exp 
///
/// \class CP::TDbiExceptionLog
///
///
/// \brief
/// <b>Concept</b> An object that records database exceptions
///
/// \brief <b>Purpose</b>To provide a place to record (in memory) exceptions
/// from the time they arise in the lower levels of the DBI and below until
/// they can be analysed in the upper levels of the DBI and beyond. They are
/// stored in a std::vector of CP::TDbiException s
///
/// Contact: A.Finch@lancaster.ac.uk
///
///

#include "TDbiException.hxx"

#include "Rtypes.h"

#include <iosfwd>
#include <string>
#include <vector>

class TSQLServer;
class TSQLStatement;

namespace CP {
    class TDbiExceptionLog;
    std::ostream& operator<<(std::ostream& s, const CP::TDbiExceptionLog& el);
}

class CP::TDbiExceptionLog {
public:
    TDbiExceptionLog(const TDbiException* e = 0);
    virtual ~TDbiExceptionLog();

    /// State testing member functions

    Bool_t IsEmpty() const { return fEntries.size() == 0; }
    const std::vector<CP::TDbiException>&
    GetEntries() const { return fEntries; }
    void Print() const;
    UInt_t Size() const { return fEntries.size(); }
    void Copy(TDbiExceptionLog& that, UInt_t start=0) const;

    /// State changing member functions

    void AddLog(const TDbiExceptionLog& el);
    void AddEntry(const TDbiException& e) { fEntries.push_back(e); }
    void AddEntry(const char* errMsg, Int_t code = -1) {
        this->AddEntry(TDbiException(errMsg,code));}
    void AddEntry(const std::string& errMsg, Int_t code = -1) {
        this->AddEntry(TDbiException(errMsg.c_str(),code));}
    void AddEntry(const TSQLServer& server) {
        this->AddEntry(TDbiException(server));}
    void AddEntry(const TSQLStatement& statement) {
        this->AddEntry(TDbiException(statement));}
    void Clear() { fEntries.clear(); }

    /// The Global Exception Log
    static TDbiExceptionLog& GetGELog() { return fgGELog;}

private:

    /// The exception entries.
    std::vector<TDbiException> fEntries;

    /// Global Exception Log
    static TDbiExceptionLog fgGELog;

    ClassDef(TDbiExceptionLog,0) // Object to hold database exceptions

};

#endif



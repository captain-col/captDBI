#ifndef DBIEXCEPTION
#define DBIEXCEPTION


/**
 *
 *
 * \class CP::TDbiException
 *
 *
 * \brief
 * <b>Concept</b> An object that records a single database exception.
 *
 * \brief
 * <b>Purpose</b>To provide a place to record an exception.  Currently
 * it is embedded in TSQLServer and TSQStatement. Each exception has an associated message string
 * and error code.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include <iosfwd>
#include <string>
#include "Rtypes.h"

namespace CP {
    class TDbiException;
    std::ostream& operator<<(std::ostream& s, const CP::TDbiException& e);
}
class TSQLServer;
class TSQLStatement;

class ostreamx;


namespace CP {
    class TDbiException

    {

    public:
        TDbiException(const char* msg = 0, Int_t code = -1);
        TDbiException(const TSQLServer& server);
        TDbiException(const TSQLStatement& statement);
        TDbiException(const TDbiException& that);
        virtual ~TDbiException();

// State testing member functions

        const std::string& GetMessage() const {
            return fMessage;
        }
        Int_t GetErrorCode() const {
            return fErrorCode;
        }

    private:


// Data members

    private:

/// The message associated with the exception.
        std::string fMessage;

/// The exception error code.
        Int_t fErrorCode;


        ClassDef(TDbiException,0) // Object to hold a single database exception record.

    };
};

#endif // DBIEXCEPTIONLOG


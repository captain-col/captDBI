#ifndef DBICONFIGSTREAM_H
#define DBICONFIGSTREAM_H

/**
 *
 * $Id: TDbiConfigStream.hxx,v 1.1 2011/01/18 05:49:19 finch Exp $
 *
 * \class CP::TDbiConfigStream
 *
 *
 * \brief
 * <b>Concept</b>  I/O of configuration data and equivalent TDbiRegistry objs.
 *
 * \brief
 * <b>Purpose</b> Used to fill/store TDbiRegistry objects.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include <string>

#include "TDbi.hxx"
#include "TDbiConfigSet.hxx"
#include "TDbiResultSetHandle.hxx"
#include "TDbiRegistry.hxx"
#include "TVldContext.hxx"

namespace CP {
    class TDbiConfigStream;
    class TDbiFieldType;
}
namespace CP {
    class TDbiRegistry;
    std::ostream& operator<<(std::ostream& s,
                             const CP::TDbiConfigStream& cfStream);
}
#include <iosfwd>


namespace CP {
    class TDbiConfigStream {


    public:

        static      CP::TVldContext fgDefaultContext;

// Constructors and destructors.
        TDbiConfigStream();
        TDbiConfigStream(const std::string& SoftwName,
                         const std::string& ConfigName="default",
                         CP::TVldContext vc = TDbiConfigStream::fgDefaultContext,
                         TDbi::Task task=0,
                         const std::string& tableName="SOFTWARE_CONFIGURATION");
        virtual ~TDbiConfigStream();

// State testing member functions
        const CP::TDbiConfigStream& operator>>(TDbiRegistry* reg);
        const TDbiConfigSet* GetConfigSet() const {
            return fCFSet;
        }

// State changing member functions

        CP::TDbiConfigStream& operator<<(const TDbiRegistry* reg);
        Bool_t IsEmpty() {
            return fCFSet == 0;
        }
        TDbiValidityRec& GetValidityRec() {
            return fVRec;
        }
        Bool_t Write(UInt_t dbNo = 0,
                     const std::string& logComment = "",
                     Bool_t localTest = false);

    private:

        TDbiConfigStream(const TDbiConfigStream&);  // Not implimented

// Data members

        ///  Pointer to the associated configuration set.  May point either
        ///  to a row of fCFSetTable (or 0) or, after operator <<, to fCFSetModified
        const TDbiConfigSet*
        fCFSet;

        /// Pointer to ConfigSet table
        CP::TDbiResultSetHandle<TDbiConfigSet>
        fCFSetTable;

        /// Local CP::TDbiConfigSet (empty until/unless filled using operator <<).
        TDbiConfigSet fCFSetModified;

        /// Associated validity rec (used when writing back to the database).
        TDbiValidityRec fVRec;

        /// Name of the configuration set
        std::string    fConfigName;

        /// Name of the software system to be configured
        std::string    fSoftwName;

        ClassDef(TDbiConfigStream,0)  // Input/output stream of configuration data

    };
};

#endif  // DBICONFIGSTREAM_H


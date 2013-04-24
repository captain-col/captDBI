// $Id: TDbiServices.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $

#ifndef DBISERVICES
#define DBISERVICES

/**
 *
 * $Id: TDbiServices.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 * \class CP::TDbiServices
 *
 *
 * \brief
 * <b>Concept</b> Register of support services.
 *
 * \brief
 * <b>Purpose</b> To break dependency loops and to simplify communication of
 * some elements of the global environment, TDbiDatabaseManager registers
 * some support services and environment with TDbiServices, a class CP::which
 * only contains static methods.  Other TDbi classes can then obtain the
 * necessary information from TDbiServices and avoids having those classes ask
 * TDbiDatabaseManager directly.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

namespace CP {
    class TDbiServices {

        friend class TDbiDatabaseManager;

    public:

        static bool AsciiDBConectionsTemporary() {
            return fAsciiDBConectionsTemporary;
        }
        static bool OrderContextQuery() {
            return fOrderContextQuery;
        }

    private:

// Data members

        static bool fAsciiDBConectionsTemporary;
        static bool fOrderContextQuery;

    };
};


#endif // DBISERVICES



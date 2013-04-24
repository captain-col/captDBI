/**
 * \class CP::UtilString
 *
 * \ingroup Util
 *
 * \brief A collection of useful string utilities besides what's
 * normally available.
 *
 * \author (last to touch it) $Author: finch $
 *
 * \version $Revision: 1.1 $
 *
 * \date $Date: 2011/01/18 05:49:20 $
 *
 * Contact: messier@huhepl.harvard.edu
 *
 * Created on: Fri Apr 15 10:17:43 2005
 *
 * $Id: UtilString.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 */
#ifndef UTILSTRING_H
#define UTILSTRING_H
#include <vector>
#include <string>
#include <sstream>

namespace CP {
    namespace UtilString {
        bool atob(const char* s);
        bool atob(const char* s, bool& isvalid);
        int  cmp_nocase(const std::string& s1, const std::string& s2);
        int  cmp_wildcard(const std::string& s, const std::string& w);
        void MakePrintable(const char* in,
                           std::string& out);
        void StringTok(std::vector<std::string>& ls,
                       const std::string& str,
                       const std::string& tok);
        bool IsBool(const char* s);
        bool IsInt(const char* s);
        bool IsFloat(const char* s);

        std::string ToLower(const std::string& str);
        std::string ToUpper(const std::string& str);
        //Has to be inline, won't work otherwise
        template <class T>
        std::string ToString(const T& t, std::ios_base & (*f)(std::ios_base&) = std::dec) {
            std::ostringstream oss;
            oss << f << t;
            return oss.str();
        }
    }
}
#endif // UTILSTRING_H
////////////////////////////////////////////////////////////////////////

/**
 * \class CP::UtilStream
 *
 * \ingroup Util
 *
 * \brief Stream related utility functions.
 *
 *
 *
 * \author (last to touch it) $Author: finch $
 *
 * \version $Revision: 1.1 $
 *
 * \date $Date: 2011/01/18 05:49:20 $
 *
 * Contact: bv@bnl.gov
 *
 * Created on: Fri Apr 15 10:16:56 2005
 *
 * $Id: UtilStream.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 */



#ifndef UTILSTREAM_H
#define UTILSTREAM_H

#include <iostream>
#include <string>
namespace CP {

    namespace Util {

        /// discard all whitespace chars until hitting a non-whitespace
        void eat_whitespace(std::istream& is);

        /// return a string of words.  Strings are delimited by single quotes.
        /// any leading white space is eaten.
        std::string read_quoted_string(std::istream& is);
    }
}

#endif  // UTILSTREAM_H

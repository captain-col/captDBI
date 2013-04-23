#include "UtilStream.hxx"

using namespace std;

void CP::Util::eat_whitespace(std::istream& is)
{
    char c;
    while (is.get(c)) {
        if (!isspace(c)) {
            is.putback(c);
            break;
        }
    }
}

std::string CP::Util::read_quoted_string(std::istream& is)
{
    eat_whitespace(is);

    char c;

    if (!is.get(c)) return "";
    if (c == '\'') {
        string stot;
        while (is.get(c)) {
            if (c == '\'') break;
            stot += c;
        }
        return stot;
    }
    else {
        is.putback(c);
        return "";
    }
}

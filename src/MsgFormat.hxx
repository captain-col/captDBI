//////////////////////////////////////////////////////////////////////
// $Id: MsgFormat.hxx,v 1.2 2012/06/14 10:55:22 finch Exp $
//
// MsgFormat
//
// A class for handling formatted numerical output to streams.
//
// messier@huhepl.harvard.edu
//////////////////////////////////////////////////////////////////////
#ifndef MsgFormat_hxx_seen
#define MsgFormat_hxx_seen

#include <MsgBoundFormat.hxx>

#include <iostream>
#include <iomanip>

// declare function so we can make it a friend
std::ostream& operator<<(std::ostream&, const MsgBoundFormat&);

//......................................................................

class MsgFormat {
    friend std::ostream& operator<<(std::ostream&, const MsgBoundFormat&);
public:
    typedef std::ios_base::fmtflags fmtflags;

    MsgFormat(int p=6) :
        prc(p), wdt(0),
        fmt(static_cast<fmtflags>(0)), flc(' ') { }

    MsgFormat(int p, int w) :
        prc(p), wdt(w),
        fmt(static_cast<fmtflags>(0)), flc(' ') { }

    MsgFormat(const char* f);

    MsgBoundFormat operator()(double d) const {
        return MsgBoundFormat(*this,d);
    }

    MsgFormat& fixed()      {
        fmt=std::ios::fixed;
        return *this;
    }
    MsgFormat& general()    {
        fmt=std::ios::dec;
        return *this;
    }
    MsgFormat& hex()        {
        fmt=std::ios::hex;
        return *this;
    }
    MsgFormat& oct()        {
        fmt=std::ios::oct;
        return *this;
    }
    MsgFormat& scientific() {
        fmt=std::ios::scientific;
        return *this;
    }

    MsgFormat& precision(int p) {
        prc=p;
        return *this;
    }
    MsgFormat& width(int w)     {
        wdt=w;
        return *this;
    }

    MsgFormat& set_fill(char c) {
        flc = c;
        return *this;
    }

    MsgFormat& left_justify(int b=1) {
        if (b) {
            fmt |= std::ios::left;
            fmt &= (~std::ios::right);
        }
        else {
            fmt &= ~std::ios::left;
        }
        return *this;
    }
    MsgFormat& right_justify(int b=1) {
        if (b) {
            fmt |= std::ios::right;
            fmt &= (~std::ios::left);
        }
        else {
            fmt &= ~std::ios::right;
        }
        return *this;
    }
    MsgFormat& show_base(int b=1) {
        if (b) {
            fmt |= std::ios::showbase;
        }
        else {
            fmt &= ~std::ios::showbase;
        }
        return *this;
    }
    MsgFormat& plus(int b=1) {
        if (b) {
            fmt |= std::ios::showpos;
        }
        else {
            fmt &= ~std::ios::showpos;
        }
        return *this;
    }
    MsgFormat& trailing_zeros(int b=1) {
        if (b) {
            fmt |= std::ios::showpoint;
        }
        else {
            fmt &= ~std::ios::showpoint;
        }
        return *this;
    }

private:
    int      prc;  // Precision
    int      wdt;  // Width
    fmtflags fmt;  // Format
    char     flc;  // Fill character
};

#endif

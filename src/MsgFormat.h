//////////////////////////////////////////////////////////////////////
// $Id: MsgFormat.h,v 1.2 2012/06/14 10:55:22 finch Exp $
//
// MsgFormat
//
// A class for handling formatted numerical output to streams.
//
// messier@huhepl.harvard.edu
//////////////////////////////////////////////////////////////////////
#ifndef MSGFORMAT_H
#define MSGFORMAT_H
#ifndef IOSTREAM
#include <iostream>
#define IOSTREAM
#endif
#ifndef IOMANIP
#include <iomanip>
#define IOMANIP
#endif
#ifndef MSGBOUNDFORMAT_H
#include <MsgBoundFormat.h>
#endif

//using namespace std;
using std::ostream;
using std::ios_base;
//using std::fmtflags;
//using std::fmt;
using std::ios;
// declare function so we can make it a friend
ostream& operator<<(ostream&, const MsgBoundFormat&);

//......................................................................

class MsgFormat 
{
  friend ostream& operator<<(ostream&, const MsgBoundFormat&);
public:
// These definitions changed from gcc-v2 to gcc-v3
#if __GNUC__ == 3 && __GNUC_MINOR__ >= 4
  typedef ios_base::fmtflags fmtflags;
#elif __GNUC__ == 4
  typedef ios_base::fmtflags fmtflags;
#else
#ifdef _CPP_BITS_IOSBASE_H
  typedef ios_base::fmtflags fmtflags;
#else
  typedef int fmtflags;
#endif
#endif

  MsgFormat(int p=6) : 
    prc(p), wdt(0), fmt(static_cast<fmtflags>(0)), flc(' ') { }
  MsgFormat(int p, int w) : 
    prc(p), wdt(w), fmt(static_cast<fmtflags>(0)), flc(' ') { }
  MsgFormat(const char* f);

  MsgBoundFormat operator()(double d) const { 
    return MsgBoundFormat(*this,d); 
  }

  MsgFormat& fixed()      { fmt=ios::fixed;      return *this;}
  MsgFormat& general()    { fmt=ios::dec;        return *this;}
  MsgFormat& hex()        { fmt=ios::hex;        return *this;}  
  MsgFormat& oct()        { fmt=ios::oct;        return *this;}
  MsgFormat& scientific() { fmt=ios::scientific; return *this;}
  
  MsgFormat& precision(int p) { prc=p; return *this;}
  MsgFormat& width(int w)     { wdt=w; return *this;}

  MsgFormat& set_fill(char c) { flc = c; return *this; }

  MsgFormat& left_justify(int b=1) {
    if (b) { fmt |= ios::left; fmt &= (~ios::right); }
    else fmt &= ~ios::left;
    return *this;
  }
  MsgFormat& right_justify(int b=1) {
    if (b) { fmt |= ios::right; fmt &= (~ios::left); }
    else fmt &= ~ios::right;
    return *this;
  }
  MsgFormat& show_base(int b=1) {
    if (b) fmt |= ios::showbase; 
    else fmt &= ~ios::showbase; 
    return *this;
  }
  MsgFormat& plus(int b=1) {
    if (b) fmt |= ios::showpos; 
    else fmt &= ~ios::showpos; 
    return *this;
  }
  MsgFormat& trailing_zeros(int b=1) {
    if (b) fmt |= ios::showpoint; 
    else fmt &= ~ios::showpoint;
    return *this;
  }

 private:
  int      prc;  // Precision
  int      wdt;  // Width
  fmtflags fmt;  // Format
  char     flc;  // Fill character
};

#endif // MSGFORMAT_H

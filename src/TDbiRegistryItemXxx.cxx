////////////////////////////////////////////////////////////////////////
//
// $Id: TDbiRegistryItemXxx.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $
//
// TDbiRegistryItemXxx
//
// Package: TDbiRegistry
//
// see header file for comments.
//
// Contact: bv@bnl.gov
//
// Created on: Wed Oct 25 17:16:38 2000
//
////////////////////////////////////////////////////////////////////////


#include <TBuffer.h>
#include <TObject.h>

#include "TDbiRegistryItem.hxx"
#include "TDbiRegistryItemXxx.hxx"

#include <UtilStream.hxx>

#include <string>
using namespace std;

namespace ND
// Only for GCC 3.3 and above.  See bug 3797
{
// http://gcc.gnu.org/bugzilla/show_bug.cgi?id=3797
#if (__GNUC__ == 3 && __GNUC_MINOR__ >= 3) || (__GNUC__ >=4 )
template<> const char* TDbiRegistryItemXxx<char>::GetTypeAsString(void) const
{ return "char"; }

template<> const char* TDbiRegistryItemXxx<int>::GetTypeAsString(void) const
{ return "int"; }

template<> const char* TDbiRegistryItemXxx<double>::GetTypeAsString(void) const
{ return "double"; }

template<> const char* TDbiRegistryItemXxx<const char*>::GetTypeAsString(void) const
{ return "string"; }

template<> const char* TDbiRegistryItemXxx<ND::TDbiRegistry>::GetTypeAsString(void) const
{ return "TDbiRegistry"; }

template<> std::ostream& TDbiRegistryItemXxx<ND::TDbiRegistry>::PrintStream(std::ostream& os) const
{ return fData->PrintStream(os); }

template<> std::istream& TDbiRegistryItemXxx<ND::TDbiRegistry>::ReadStream(std::istream& is)
{ if (!fData) fData = new TDbiRegistry(); return fData->ReadStream(is); }
#endif

}
TBuffer& operator>>(TBuffer &buf, int*& xptr)
{
    int x;
    buf >> x;
    xptr = new int(x);
    return buf;
}
TBuffer& operator>>(TBuffer &buf, double*& xptr)
{
    double x;
    buf >> x;
    xptr = new double(x);
    return buf;
}
TBuffer& operator>>(TBuffer &buf, float*& xptr)
{
    float x;
    buf >> x;
    xptr = new float(x);
    return buf;
}

#if 0
TBuffer& operator>>(TBuffer &buf, bool*& xptr)
{
    int i;
    buf >> i;
    if (i) xptr = new bool(true);
    else   xptr = new bool(false);
    return buf;
}
#endif

//......................................

TBuffer& operator<<(TBuffer &buf, int*& xptr)
{
    buf << *xptr;
    return buf;
}
TBuffer& operator<<(TBuffer &buf, double*& xptr)
{
    buf << *xptr;
    return buf;
}
TBuffer& operator<<(TBuffer &buf, float*& xptr)
{
    buf << *xptr;
    return buf;
}
#if 0
TBuffer& operator<<(TBuffer &buf, bool*& xptr)
{
    buf << (*xptr ? 1 : 0);
    return buf;
}
#endif

#if 0
TBuffer& operator<<(TBuffer &buf, const char*& xptr)
{
    const char *x = xptr;
    --x;
    do {
        ++x;
        buf  << *x;
    } while (*x);
    return buf;
}
#endif
#include <string>
namespace ND
{
template<>
void TDbiRegistryItemXxx<const char*>::Streamer(TBuffer &buf)
{
    if (buf.IsReading()) {
        Version_t v = buf.ReadVersion();
        if (v) { }
        TDbiRegistryItem::Streamer(buf);

        std::string str = "";
        char x[2];
        x[1] = '\0';

        do {                    // read out string one byte at a time
            buf >> x[0];
            str += x;
        } while (x[0]);

        char** ppchar = new char*;
        *ppchar = new char[str.length() + 1];
        strcpy(*ppchar,str.c_str());
        (*ppchar)[str.length()] = '\0'; // paranoia
        fData = const_cast<const char**>(ppchar);
    }
    else {
        buf.WriteVersion(IsA());
        TDbiRegistryItem::Streamer(buf);
        buf << (*fData);
    }
}



// Must do this special because ROOT treats char and char* asymmetric
template<>
void TDbiRegistryItemXxx<char>::Streamer(TBuffer &buf)
{
    if (buf.IsReading()) {
        Version_t v = buf.ReadVersion();
        if (v) { }
        TDbiRegistryItem::Streamer(buf);

        char c;
        buf >> c;
        fData = new char(c);
    }
    else {
        buf.WriteVersion(IsA());
        TDbiRegistryItem::Streamer(buf);
        buf << *fData;
    }
}

template<>
std::ostream& TDbiRegistryItemXxx<const char*>::PrintStream(std::ostream& os) const
{
    os << "'" << *fData << "'";
    return os;
}

template<>
std::istream& TDbiRegistryItemXxx<const char*>::ReadStream(std::istream& is)
{
    string stot = Util::read_quoted_string(is);

    if (!fData) {
        char** ppchar = new char*;
        *ppchar = 0;
        fData = const_cast<const char**>(ppchar);
    }
    if (*fData) delete [] *fData;

    char* pchar = new char[stot.length() + 1];
    strcpy(pchar,stot.c_str());
    *fData = const_cast<const char*>(pchar);
    return is;
}


template<>
TDbiRegistryItemXxx<const char*>::~TDbiRegistryItemXxx()
{
    if (fData) {
        if (*fData) {
            delete [] *fData;
            *fData = 0;
        }
        delete fData;
        fData = 0;
    }
}

template<>
TDbiRegistryItem* TDbiRegistryItemXxx<const char*>::Dup(void) const
{
    char** ppc = new char*;
    (*ppc) = new char [strlen(*fData)+1];
    strcpy(*ppc,*fData);
    const char** ppcc = const_cast<const char**>(ppc);
    return new TDbiRegistryItemXxx< const char* >(ppcc);
}
}



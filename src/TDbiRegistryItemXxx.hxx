////////////////////////////////////////////////////////////////////////
///
/// $Id: TDbiRegistryItemXxx.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
///
/// \class  CP::TDbiRegistryItemXxx<T>
///
/// \brief Encapsulates a value of type T (really!).
///
/// Contact: A.Finch@lancaster.ac.uk  bv@bnl.gov
///
///Created on: Wed Oct 25 17:11:44 2000
///
////////////////////////////////////////////////////////////////////////

#ifndef TDBIREGISTRYITEMXXX_H
#define TDBIREGISTRYITEMXXX_H

#include <TBuffer.h>
#include <TObject.h>

#include <TDbiRegistryItem.hxx>

#include <typeinfo>
#include <iostream>

namespace CP {
/// Encapsulates a value of type T
    class TDbiRegistry;
    template<class T> class TDbiRegistryItemXxx : public CP::TDbiRegistryItem {
    public:
        // Only Want TDbiRegistry to call Get/Set/etc methods
        friend class TDbiRegistry;

        TDbiRegistryItemXxx();
        ~TDbiRegistryItemXxx();

        void Dump() const {
            std::cerr << *fData;
        }
        void Print(Option_t* /* option */ ="") const {
            std::cout << *fData;
        }

        virtual std::ostream& PrintStream(std::ostream& os) const {
            return os << *fData;
        }
        virtual std::istream& ReadStream(std::istream& is) {
            if (!fData) {
                fData = new T;
            }
            return is >> *fData;
        }

        virtual const std::type_info& GetType() const {
            return typeid(T);
        }
        virtual const char* GetTypeAsString() const {
            return "void";
        }

    private:

        TDbiRegistryItemXxx(T* data);
        T* Get(void);               // Only let TDbiRegistry call
        void Set(T* data);          // these methods.
        TDbiRegistryItem* Dup(void) const;

        T* fData;

        ClassDefT(TDbiRegistryItemXxx<T>,1)
    };
    ClassDefT2(CP::TDbiRegistryItemXxx,T)
}


// Provide these so C++ base types act like ROOT types inside
// a ROOT Streamer function.  This lets the same template to be used.
TBuffer& operator>>(TBuffer& buf, int*& xptr);
TBuffer& operator>>(TBuffer& buf, double*& xptr);
TBuffer& operator>>(TBuffer& buf, float*& xptr);
//TBuffer& operator>>(TBuffer &buf, bool*& xptr);

TBuffer& operator<<(TBuffer& buf, int*& xptr);
TBuffer& operator<<(TBuffer& buf, double*& xptr);
TBuffer& operator<<(TBuffer& buf, float*& xptr);
//TBuffer& operator<<(TBuffer &buf, bool*& xptr);

//TBuffer& operator<<(TBuffer &buf, char*& xptr);

#ifndef __CINT__

#include <iostream>

ClassImpT(TDbiRegistryItemXxx,T)

namespace CP {
    template<class T>
    TDbiRegistryItemXxx<T>::TDbiRegistryItemXxx(void) : fData(0) {
    }

    template<class T>
    TDbiRegistryItemXxx<T>::TDbiRegistryItemXxx(T* data) : fData(data) {
    }

    template<class T>
    TDbiRegistryItemXxx<T>::~TDbiRegistryItemXxx() {
        if (fData) {
            delete fData;
        }
    }

    template<class T>
    TDbiRegistryItem* TDbiRegistryItemXxx<T>::Dup(void) const {
        return new TDbiRegistryItemXxx<T>(new T(*fData));
    }
// see TDbiRegistryItemXxx.cxx for implementation
    template<>
    TDbiRegistryItem* TDbiRegistryItemXxx<const char*>::Dup(void) const;


    template<class T>
    T* TDbiRegistryItemXxx<T>::Get(void) {
        return fData;
    }

    template<class T>
    void TDbiRegistryItemXxx<T>::Set(T* data) {
        if (fData) {
            delete fData;
        }
        fData = data;
    }


// These specialized templates are in TDbiRegistryItemXxx.cxx
//
    template<>
    void TDbiRegistryItemXxx<const char*>::Streamer(TBuffer& buf);
    template<>
    void TDbiRegistryItemXxx<char>::Streamer(TBuffer& buf);
    template<>
    TDbiRegistryItemXxx<const char*>::~TDbiRegistryItemXxx();

    template<> std::ostream& TDbiRegistryItemXxx<const char*>::PrintStream(std::ostream& os) const;
    template<> std::istream& TDbiRegistryItemXxx<const char*>::ReadStream(std::istream& is);
    template<> const char* TDbiRegistryItemXxx<char>::GetTypeAsString(void) const;
    template<> const char* TDbiRegistryItemXxx<int>::GetTypeAsString(void) const;
    template<> const char* TDbiRegistryItemXxx<double>::GetTypeAsString(void) const;
    template<> const char* TDbiRegistryItemXxx<const char*>::GetTypeAsString(void) const;
    template<> const char* TDbiRegistryItemXxx<TDbiRegistry>::GetTypeAsString(void) const;

}
// Only for less than GCC 3.3 - see bug 3797:
// http://gcc.gnu.org/bugzilla/show_bug.cgi?id=3797
#if (__GNUC__ == 3 && __GNUC_MINOR__ < 3) || __GNUC__ == 2

#include <TDbiRegistry.hxx>

template<> const char* TDbiRegistryItemXxx<char>::GetTypeAsString(void) const {
    return "char";
}

template<> const char* TDbiRegistryItemXxx<int>::GetTypeAsString(void) const {
    return "int";
}

template<> const char* TDbiRegistryItemXxx<double>::GetTypeAsString(void) const {
    return "double";
}

template<> const char* TDbiRegistryItemXxx<const char*>::GetTypeAsString(void) const {
    return "string";
}

template<> const char* TDbiRegistryItemXxx<TDbiRegistry>::GetTypeAsString(void) const {
    return "TDbiRegistry";
}

template<> std::ostream& TDbiRegistryItemXxx<TDbiRegistry>::PrintStream(std::ostream& os) const {
    return fData->PrintStream(os);
}

template<> std::istream& TDbiRegistryItemXxx<TDbiRegistry>::ReadStream(std::istream& is) {
    if (!fData) {
        fData = new TDbiRegistry();
    } return fData->ReadStream(is);
}
#endif

// This assumes that objects spring forth (like ROOT objects)
namespace CP {
    template<class T>
    void TDbiRegistryItemXxx<T>::Streamer(TBuffer& buf) {
        if (buf.IsReading()) {
            Version_t v = buf.ReadVersion();
            if (v) { }
            CP::TDbiRegistryItem::Streamer(buf);

            buf >> fData;
        }
        else {
            buf.WriteVersion(IsA());
            CP::TDbiRegistryItem::Streamer(buf);
            buf << fData;
        }
    }
}


#include <TDbiRegistry.hxx>
namespace CP {

    template<>
    std::ostream& TDbiRegistryItemXxx<CP::TDbiRegistry>::PrintStream(std::ostream& os) const;

    template<>
    std::istream& TDbiRegistryItemXxx<CP::TDbiRegistry>::ReadStream(std::istream& is);
}

#endif // __CINT__


#endif  // TDBIREGISTRYITEMXXX_H

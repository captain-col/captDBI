#include "TEventContext.hxx"
#include "TResultSetHandle.hxx"
#include "TDbiLog.hxx"

#include "TVldContext.hxx"

// Before instantiating templated CP::TDbiResultSetHandle classes, which will use MessageService
// insure that __CVSID__ symbol is defined.

#include <TDbiLog.hxx>
#include <MsgFormat.hxx>

#include "TDbiResultSetHandle.tpl"
#define  DbiResultPtr CP::TDbiResultSetHandle

//_____________________________________________________________________________

template<class T> CP::TResultSetHandle<T>::TResultSetHandle()  {

    // Default constructor

    DbiTrace("CP::TResultSetHandle: Default ctor at:" << (void*) this);

}


//_____________________________________________________________________________

template<class T> CP::TResultSetHandle<T>::TResultSetHandle(const TResultSetHandle& from) :
    CP::TDbiResultSetHandle<T>(from) {

    // Copy constructor

    DbiTrace("CP::TResultSetHandle: Copy ctor at:" << (void*) this);

}


//_____________________________________________________________________________

template<class T> CP::TResultSetHandle<T>::TResultSetHandle(const TEventContext& context,
                                                            Int_t task /* =0 */) :
    CP::TDbiResultSetHandle<T>(CP::TVldContext(context),task) {

    // Standard context query constructor

    DbiTrace("CP::TResultSetHandle: Standard context query ctor at:" << (void*) this);

}

//_____________________________________________________________________________

template<class T> CP::TResultSetHandle<T>::~TResultSetHandle()  {

    // Destructor

    DbiTrace("CP::TResultSetHandle: dtor at:" << (void*) this);

}



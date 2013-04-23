#include "TEventContext.hxx"
#include "TResultSetHandle.hxx"
#include "TDbiLog.hxx"

#include "TVldContext.hxx"

// Before instantiating templated ND::TDbiResultSetHandle classes, which will use MessageService
// insure that __CVSID__ symbol is defined.

#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;

#include "TDbiResultSetHandle.tpl"
#define  DbiResultPtr ND::TDbiResultSetHandle

//_____________________________________________________________________________

template<class T> ND::TResultSetHandle<T>::TResultSetHandle()  {

   // Default constructor

    DbiTrace("ND::TResultSetHandle: Default ctor at:" << (void*) this);

}


//_____________________________________________________________________________

template<class T> ND::TResultSetHandle<T>::TResultSetHandle(const TResultSetHandle& from) :
  ND::TDbiResultSetHandle<T>(from) {

   // Copy constructor

    DbiTrace("ND::TResultSetHandle: Copy ctor at:" << (void*) this);

}


//_____________________________________________________________________________

template<class T> ND::TResultSetHandle<T>::TResultSetHandle(const TEventContext& context,
							    Int_t task /* =0 */) :
  ND::TDbiResultSetHandle<T>(ND::TVldContext(context),task) {

   // Standard context query constructor

    DbiTrace("ND::TResultSetHandle: Standard context query ctor at:" << (void*) this);

}

//_____________________________________________________________________________

template<class T> ND::TResultSetHandle<T>::~TResultSetHandle()  {

   // Destructor

    DbiTrace("ND::TResultSetHandle: dtor at:" << (void*) this);

}



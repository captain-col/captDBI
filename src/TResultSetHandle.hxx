#ifndef TResultSetHandle_hxx_seen
#define TResultSetHandle_hxx_seen

#include "TDbiResultSetHandle.hxx"

namespace CP {
    class TEventContext;

    /// This is the base class for all results sets of rows of type T
    template <class T> class TResultSetHandle : public TDbiResultSetHandle<T> {

    public:

        TResultSetHandle();
        TResultSetHandle(const TResultSetHandle& from);
        TResultSetHandle(const TEventContext& context,
                         Int_t task = 0);
        virtual ~TResultSetHandle();

    private:

        ClassDef(TResultSetHandle,1);

    };
};
#endif

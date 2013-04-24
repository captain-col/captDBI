////////////////////////////////////////////////////////////////////////////
// $Id: TVldValidate.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
//
/// \class CP::TVldValidate
///
/// \brief TVldValidate is a test harness for Validity package
///
/// Author:  R. Hatcher 2001.01.03
//
////////////////////////////////////////////////////////////////////////////

#ifndef VLDVALIDATE_H
#define VLDVALIDATE_H

namespace CP {
    class TVldTimeStamp;
}
#include "TObject.h"
namespace CP {

    class TVldValidate : public TObject {

    public:

        TVldValidate();                        // necessary for streamer io
        virtual ~TVldValidate();

        Bool_t  RunAllTests();

        Bool_t  TestTimeStamp(void);
        Bool_t  TestContext(void);
        Bool_t  TestRange(void);

        Bool_t  TestFileIO(void);

    protected:
        //   TVldValidate(const TObject &object);           // hide these
        //   TVldValidate(const TVldValidate &tf);
        //   TVldValidate& operator=(const TObject &rhs);
        //   TVldValidate& operator=(const TVldValidate &tf);

        Bool_t CompareTimeStamps(const char* label,
                                 CP::TVldTimeStamp& test, CP::TVldTimeStamp& std);

    private:

        ClassDef(CP::TVldValidate,1)  // TVldValidate version 1
    };
};

#endif // VLDVALIDATE_H

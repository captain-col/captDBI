
#ifndef DBIFIELDTYPE_H
#define DBIFIELDTYPE_H


//////////////////////////////////////////////////////////////////////////
////////////////////////////     ROOT API     ////////////////////////////
//////////////////////////////////////////////////////////////////////////


#include "TDbi.hxx"

#include <string>

namespace CP {
    /// Database neutral table column field types.  This handles non-standard
    /// type names used by some SQL implementations (MySQL seems to be a
    /// particularly bad offender) and translates them into standard types.
    class TDbiFieldType {


    public:

        /// The maximum number of characters needed to display an object of
        /// this type.
        enum PrecisionLimits { kMaxTinyInt       = 4, // MySQL special
                               kMaxSmallInt      = 6,
                               kMaxInt           = 11,
                               kMaxChar          = 3,
                               kMaxMySQLVarchar  = 255,
                               kMaxMySQLText     = 65535
                             };

        // Constructors and destructors.
        TDbiFieldType(Int_t type = TDbi::kInt);
        TDbiFieldType(Int_t type,
                      Int_t size,
                      const char* typeName);
        TDbiFieldType(const TDbiFieldType& from);
        TDbiFieldType(const std::string& sql,
                      Int_t size);
        virtual ~TDbiFieldType();

        // State testing member functions

        std::string AsString() const;
        std::string AsSQLString() const;
        std::string UndefinedValue() const;
        UInt_t GetConcept() const {
            return fConcept;
        }
        UInt_t GetSize() const {
            return fSize;
        }
        UInt_t GetType() const {
            return fType;
        }
        Bool_t IsCompatible(const TDbiFieldType& other) const;
        Bool_t IsEqual(const TDbiFieldType& other) const {
            return fType == other.fType;
        }
        Bool_t IsSmaller(const TDbiFieldType& other) const {
            return fSize < other.fSize;
        }
        Bool_t IsSigned() const          {
            return fConcept ==  TDbi::kInt;
        }


        // State modifying member functions

        void SetUnsigned() {
            if (this->IsSigned()) {
                ++fConcept;
                ++fType;
            }
        }

    private:

        void Init(Int_t type, Int_t size = -1);

        // Data members

        /// Concept e.g. kInt or kFloat
        unsigned int fConcept;
        /// Size in bytes(0 if unknown).
        unsigned int fSize;
        /// Concept e.g. kShort or kDouble
        unsigned int fType;

        ClassDef(TDbiFieldType,0)        // Table column field types.

    };
};

#endif  // DBIFIELDTYPE_H


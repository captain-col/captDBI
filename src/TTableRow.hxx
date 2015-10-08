#ifndef TTableRow_hxx_seen
#define TTableRow_hxx_seen

#include "TDbiTableRow.hxx"
#include "TDbiLog.hxx"
#include "TResultInputStream.hxx"

namespace CP {
    class TTableRow;
    class TResultInputStream;
    class  TDbiTableRow;
};

/// This is the base class for all table rows

class CP::TTableRow : public TDbiTableRow {


public:

    TTableRow();
    TTableRow(const TTableRow& from);
    virtual ~TTableRow();

    virtual TDbiTableRow* CreateTableRow() const {
        return this->MakeTableRow();
    }

    virtual void Fill(TDbiInRowStream& rs, const TDbiValidityRec* vrec) {
        TResultInputStream ris(rs);
        this->Fill(ris);
    }
#define PURE_VIRTUAL_FILL
#ifdef PURE_VIRTUAL_FILL
    virtual void Fill(CP::TResultInputStream& ris) = 0;
#else
    virtual void Fill(CP::TResultInputStream& ris) {
        DbiError("CP::TTableRow::Fill undefined in subclass");
    }
#endif
    virtual TTableRow* MakeTableRow() const =0;

    /// Return class name but strip off leading "CP::T"
    const char* GetName() const {
        const char* name = TObject::GetName();
        if (strncmp("CP::",name,4) == 0) {
            name += 4;
        }
        if (strncmp("T",   name,1) == 0) {
            name += 1;
        }
        return name;
    }


private:
    ClassDef(TTableRow,1)

};

#endif

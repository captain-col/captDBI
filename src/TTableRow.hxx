#ifndef TTableRow_hxx_seen
#define TTableRow_hxx_seen

#include "TDbiTableRow.hxx"
#include "TDbiLog.hxx"
#include "TResultInputStream.hxx"

namespace ND {
    class TTableRow;
    class TResultInputStream;
class  TDbiTableRow;
};

/// This is the base class for all table rows

class ND::TTableRow : public TDbiTableRow {


public:

    TTableRow();
    TTableRow(const TTableRow& from);
    virtual ~TTableRow();

    virtual TDbiTableRow* CreateTableRow() const { return this->MakeTableRow();}
 
    virtual void Fill(TDbiInRowStream& rs, const TDbiValidityRec* vrec) { 
        TResultInputStream ris(rs);
	this->Fill(ris); 
    }
    virtual void Fill(ND::TResultInputStream& ris) {
	DbiError("Calling base class ND::TTableRow::Fill method - this should be redefined in subclass");

    }
    virtual TTableRow* MakeTableRow() const =0;

    /// Return class name but strip off leading "ND::T"
    const char* GetName() const {
	const char* name = TObject::GetName();
	if ( strncmp("ND::",name,4) == 0 ) name += 4;
	if ( strncmp("T",   name,1) == 0 ) name += 1;
	return name;
    }


private:
    ClassDef(TTableRow,1)

};

#endif

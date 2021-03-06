#ifndef TDemo_DB_Table_hxx_seen
#define TDemo_DB_Table_hxx_seen

#include "TChannelId.hxx"
#include "TResultSetHandle.hxx"
#include "TTableRow.hxx"

namespace CP {
    class TDemo_DB_Table;
    class TResultInputStream;
    class TTableRow;
};

/// This is an example TTableRow subclass used to read rows of demonstration
/// DEMO_DB_TABLE.  It can be used as the starting point for other subclasses.
///
/// For table XXX_YYY (all tables names are upper case) the corresponding
/// TTableRow subclass is
/// \code
///     CP::TXxx_Yyy  (arbitrary capitalisation).
/// \endcode
/// This is a demonstration table row
class CP::TDemo_DB_Table : public CP::TTableRow {

    //using TObject::Print;
    using TTableRow::Fill;

public:

    /// Required default constructor
    TDemo_DB_Table();

    /// Required copy constructor
    TDemo_DB_Table(const TDemo_DB_Table& from);

    virtual ~TDemo_DB_Table();

    /// Standard Getters (here they just give access to the row's state but
    /// could equally well perform services based on that state e.g. apply
    /// calibration.)
    TChannelId GetChannelId() const {
        return fChannelId;
    }
    Int_t GetIParm1() const {
        return fIParm1;
    }
    Int_t GetIParm2() const {
        return fIParm2;
    }
    Int_t GetIParm3() const {
        return fIParm3;
    }
    Float_t GetFParm1() const {
        return fFParm1;
    }
    Float_t GetFParm2() const {
        return fFParm2;
    }
    Float_t GetFParm3() const {
        return fFParm3;
    }

    /// Return unique "Natural Index" of row in table.  Optional, the default
    /// is to return the row position, which is unique but arbitrary.
    virtual UInt_t GetIndex(UInt_t defIndex) const {
        return fChannelId.AsUInt();
    }

    /// Required method to create new row.
    virtual CP::TTableRow* MakeTableRow() const {
        return new TDemo_DB_Table;
    }

    /// Required method to fill self from a ResultInputStream.
    virtual           void Fill(CP::TResultInputStream& ris);
    virtual           void Print(const Option_t* = "") const;

private:

    /// Channel ID
    TChannelId fChannelId;
    /// Integer parameters
    Int_t fIParm1, fIParm2, fIParm3;
    /// Floating point parameters
    Float_t fFParm1, fFParm2, fFParm3;

    ClassDef(TDemo_DB_Table,1)
};

#endif

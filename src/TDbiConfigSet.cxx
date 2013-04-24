// $Id: TDbiConfigSet.cxx,v 1.1 2011/01/18 05:49:19 finch Exp $

#include "TDbi.hxx"
#include "TDbiConfigSet.hxx"
#include "TDbiOutRowStream.hxx"
#include "TDbiInRowStream.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>

#include <iostream>

ClassImp(CP::TDbiConfigSet)


//   Definition of static data members
//   *********************************


//  Instantiate associated Result Pointer and Writer classes.
//  ********************************************************

#include "TDbiResultSetHandle.tpl"
template class  CP::TDbiResultSetHandle<CP::TDbiConfigSet>;

#include "TDbiWriter.tpl"
template class  CP::TDbiWriter<CP::TDbiConfigSet>;

//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

CP::TDbiConfigSet::~TDbiConfigSet() {
//
//
//  Purpose:  Destructor


  for ( std::vector<Param*>::iterator itr = fParams.begin();
        itr != fParams.end();
        ++itr ) delete (*itr);

}

//.....................................................................
///\verbatim
///
///  Purpose:  Output configuration set to message stream.
///
///  Arguments:
///    s            in    Message stream
///    cfSet        in    Configuration set to be output
///
///  Return:        Message stream
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Output configuration set to message stream.
///\endverbatim
std::ostream& CP::operator<<(std::ostream& s, const CP::TDbiConfigSet& cfSet) {

//  Program Notes:-
//  =============

//  None.

  s << "CP::TDbiConfigSet: Number of parameters: "
    << cfSet.GetNumParams() << std::endl;

  for ( UInt_t iParam = 0; iParam < cfSet.GetNumParams(); ++iParam) {
    s << "   "  << cfSet.GetParamName(iParam) << ":  "
      << cfSet.GetParamValue(iParam) << " ("
      << cfSet.GetParamType(iParam).AsString() << ")" << std::endl;
  }

  return s;

}



//.....................................................................
///\verbatim
///
///  Purpose:  Fill object from Result Set
///
///  Arguments:
///    rs           in    Result Set used to fill object
///    vrec         in    Associated validity record (or 0 if filling
///                                                    CP::TDbiValidityRec)
///
///  Return:
///
///  Contact:   N. West
////
///  Specification:-
///  =============
///
///  o Fill object from current (and only) row of Result Set.
///\endverbatim
void CP::TDbiConfigSet::Fill(CP::TDbiInRowStream& rs,
                        const CP::TDbiValidityRec* vrec) {

//  Program Notes:-
//  =============

//  None.


// Don't count leading SeqNo or ROW_COUNTER, they have already been skipped.
  UInt_t numParams = rs.NumCols()-2;

  for (UInt_t iParam = 0; iParam < numParams; ++iParam ) {
    Param* par = new Param;
    par->Name  = rs.CurColName();
    par->Value = rs.CurColValue();
    par->Type  = rs.CurColFieldType();

   fParams.push_back(par);
    rs.IncrementCurCol();
  }

  fAggregateNo =  vrec->GetAggregateNo ();

}
//.....................................................................
///\verbatim
///
///  Purpose:  Get the name of selected parameter.
///
///  Arguments:
///    parNo        in    parNo (in range 0..GetNumParams())
///
///  Return:    The name of selected parameter
///              or "" if parNo out of range.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Get the name of selected parameter  or "" if parNo out of range.
///\endverbatim
std::string CP::TDbiConfigSet::GetParamName(UInt_t parNo) const {


//  Program Notes:-
//  =============

//  None.

 return ( parNo <= GetNumParams() ) ?
   fParams[parNo]->Name : "";

}
//.....................................................................

CP::TDbiFieldType CP::TDbiConfigSet::GetParamType(UInt_t parNo) const {
//
//
//  Purpose:  Get the type of selected parameter.
//
//  Arguments:
//    parNo        in    parNo (in range 0..GetNumParams())
//
//  Return:    The type of selected parameter
//              or TDbi::kUnknown if parNo out of range.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Get the type of selected parameter  or TDbi::kUnknown if
//    parNo out of range.

//  Program Notes:-
//  =============

//  None.

 return ( parNo <= GetNumParams() ) ?
   fParams[parNo]->Type : CP::TDbiFieldType(TDbi::kUnknown);

}
//.....................................................................
///\verbatim
///  Purpose:  Get the value of selected parameter.
///
///  Arguments:
///    parNo        in    parNo (in range 0..GetNumParams())
///
///  Return:    The value of selected parameter
///              or "" if parNo out of range.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Get the value of selected parameter  or "" if parNo out of range.
///\endverbatim
std::string CP::TDbiConfigSet::GetParamValue(UInt_t parNo) const {

//  Program Notes:-
//  =============

//  None.

 return ( parNo <= GetNumParams() ) ?
   fParams[parNo]->Value : "";

}

//.....................................................................
/// Purpose:  Add another entry to the end of the existing row.
void CP::TDbiConfigSet::PushBack(const std::string& name,
                            const std::string& value,
                            const CP::TDbiFieldType& type) {
//
//

  fParams.push_back(new Param(name,value,type));
}

//.....................................................................
//
///\verbatim
///  Purpose:  Stream object to output row stream
///
///  Arguments:
///    ors          in    Output row stream.
///    vrec         in    =0.  If filling other table rows it points
///                       to the associated validity record.
///
///  Return:
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o  Stream object to output row stream.
///
///  Program Notes:-
///  =============
///
///  This method sneaks round the back of the CP::TDbiRowStream interface
///  and directly uses the private Store method as the data is already
///  in string form.  Its all in a good cause because this allows
///  CP::TDbiConfigSet to output data from any type of table.
///\endverbatim
void CP::TDbiConfigSet::Store(CP::TDbiOutRowStream& ors,
                         const CP::TDbiValidityRec* /* vrec */) const {

  for ( std::vector<Param*>::const_iterator itr = fParams.begin();
        itr != fParams.end();
        ++itr ) ors.Store((*itr)->Value.c_str());

}



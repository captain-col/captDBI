////////////////////////////////////////////////////////////////////////
//
// $Id: TDbiRegistryItem.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
//
// TDbiRegistryItem
//
// Package: TDbiRegistry
//
// Empty base class.
//
// Contact: bv@bnl.gov
//
// Created on: Wed Oct 25 17:12:25 2000
//
////////////////////////////////////////////////////////////////////////

#ifndef TDBIREGISTRYITEM_H
#define TDBIREGISTRYITEM_H

#include <TObject.h>

namespace CP {
///
/// An item stored in a registry. This class does not do  a right lot, but  TDbiRegistryItemXxx which inherits from it, does a lot.
    class TDbiRegistryItem : public TObject {

    public:

        TDbiRegistryItem();
        virtual ~TDbiRegistryItem();
        virtual TDbiRegistryItem* Dup(void) const = 0;

        virtual std::ostream& PrintStream(std::ostream& os) const = 0;
        virtual std::istream& ReadStream(std::istream& is) = 0;
        virtual void Print(Option_t* option="") const = 0;

        virtual const std::type_info& GetType() const = 0;
        virtual const char* GetTypeAsString() const = 0;


        ClassDef(CP::TDbiRegistryItem,1)
    };                              // end of class TDbiRegistryItem
}


#endif  // TDBIREGISTRYITEM_H

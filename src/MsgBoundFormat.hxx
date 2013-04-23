////////////////////////////////////////////////////////////////////////
// $Id: MsgBoundFormat.h,v 1.1 2011/01/18 05:49:19 finch Exp $
//
// A simple class to hold a value to be printed together with the format
// to use to print it,
//
// messier@huhepl.harvard.edu
//
////////////////////////////////////////////////////////////////////////
#ifndef MSGBOUNDFORMAT_H
#define MSGBOUNDFORMAT_H

class MsgFormat;

//......................................................................

class MsgBoundFormat{
public:
    MsgBoundFormat(const MsgFormat& ff, double v) : f(ff), val(v) { }
    
    const MsgFormat& f; // Format to use to print
    double val;         // Value to print
};

#endif // MSGBOUNDFORMAT_H

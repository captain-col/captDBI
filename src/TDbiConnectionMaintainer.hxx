#ifndef DBICONNECTIONMAINTAINER
#define DBICONNECTIONMAINTAINER

/**
 *
 * $Id: TDbiConnectionMaintainer.hxx,v 1.1 2011/01/18 05:49:19 finch Exp $
 *
 * \class ND::TDbiConnectionMaintainer
 *
 *
 * \brief
 * <b>Concept</b> An object that hold open all TDbiConnections throught
 * its lifetime.
 *
 * \brief
 * <b>Purpose</b> To minimse the number of connections made and broken
 *   when performing multiple SQL queries.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

namespace ND {
class TDbiCascader;
}

namespace ND {
class TDbiConnectionMaintainer

{

public:

           TDbiConnectionMaintainer(TDbiCascader* cascader = 0);
	   virtual ~TDbiConnectionMaintainer();

// Data members

private:


/// May be null if made by (useless) default ctor.
  TDbiCascader* fCascader;

 ClassDef(TDbiConnectionMaintainer,0) // Object to hold open connections

};
};


#endif // DBICONNECTIONMAINTAINER



#ifdef __CINT__
#pragma link C++ class ND::TDbiConfigSet;
#pragma link C++ class ND::TDbiResultSetHandle<ND::TDbiConfigSet>+;
#pragma link C++ class ND::TDbiWriter<ND::TDbiConfigSet>+;

// Normally this would be part of ND::TDbiLogEntry_LinkDef but TDbiWriter.hxxas a 
// DBiLogEntry and this confuses ROOT's dictionary generator.

#pragma link C++ class ND::TDbiWriter<ND::TDbiLogEntry>+;

#pragma link C++ class ND::TDbiResultSetHandle<ND::TDbiValidityRec>+;

#endif

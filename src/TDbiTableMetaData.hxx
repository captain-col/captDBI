#ifndef DBITABLEMETADATA_H
#define DBITABLEMETADATA_H


//////////////////////////////////////////////////////////////////////////
////////////////////////////     ROOT API     ////////////////////////////
//////////////////////////////////////////////////////////////////////////

/**
 *
 * $Id: TDbiTableMetaData.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 * \class CP::TDbiTableMetaData
 *
 *
 * \brief
 * <b>Concept</b> Table meta data i.e. data about the table itself
 *
 * \brief
 * <b>Purpose</b> To provide information on the names and types of columns
 *
 *
 * Contact: A.Finch@lancaster.ac.uk
 *to transient TDbiRowStream objects.
 *
 */

#include "TDbiFieldType.hxx"

#include <string>
#include <vector>

namespace CP {
class TDbiTableMetaData
{

  friend class TDbiDBProxy;  //See ctor program notes.

public:

// Constructors and destructors.
           TDbiTableMetaData(const std::string& tableName = "Unknown");
  virtual ~TDbiTableMetaData();

// State testing member functions

/// Return SQL string to create table.
  std::string Sql() const;

  std::string TableName() const { return fTableName; }
  Bool_t HasEpoch() const      {return  this->NumCols() >=4 && this->ColName(4) == "EPOCH"; }
  UInt_t NumCols() const { return fNumCols;}

// Column attribute getters (columns number starts from 1 NOT zero)

 const TDbiFieldType& ColFieldType(Int_t colNum) const    { return GetAttributes(colNum).Type;}
              UInt_t ColFieldConcept(Int_t colNum) const { return GetAttributes(colNum).Concept;}
              Bool_t ColIsNullable(Int_t colNum) const   { return GetAttributes(colNum).IsNullable;}
              Bool_t ColMustDelimit(Int_t colNum) const  { return GetAttributes(colNum).MustDelimit;}
              std::string ColName(Int_t colNum) const         { return GetAttributes(colNum).Name;}

// State changing member functions

/// Recreate from SQL used to create table.
      void SetFromSql(const std::string& sql);

protected:

      void Clear();

// Column attribute setters (columns number starts from 1 NOT zero)

      void SetColIsNullable(Int_t colNum, Bool_t isNullable = true) { SetAttributes(colNum).IsNullable = isNullable;}
      void SetColName(const std::string& name, Int_t colNum)             { SetAttributes(colNum).Name = name;}
      void SetColFieldType(const TDbiFieldType& type, Int_t colNum);

 private:

///   Use to parse table creation SQL - move to UtilStd::String?
  static std::string GetToken(const char*& strPtr);

///  Sanity check: limit number of columns.
  enum { MAXCOL = 1000};

  void ExpandTo(UInt_t colNum);

/// Column attributes
 struct ColumnAttributes {
   ColumnAttributes() { this->SetDefault(); }
   void SetDefault() {
     Name        = "Unknown";
     Concept     = TDbi::kUnknown;
     MustDelimit = false;
     IsNullable  = false;
     Type        = TDbi::kUnknown;
   }
   std::string Name;
   Int_t Concept;
   Bool_t MustDelimit;
   Bool_t IsNullable;
   TDbiFieldType Type;
 };

/// Dummy attributes (used when requesting invalid column)
  static ColumnAttributes fgDummy;


///  Return a column attributes (will be dummy entry if requesting invalid column)
  const ColumnAttributes& GetAttributes(Int_t colNum) const;

///  Return a setable column attributes (will be dummy entry if requesting invalid column)
  ColumnAttributes& SetAttributes(Int_t colNum);

// Data members

///  Column attributes indexed by column (starting from 0)
  std::vector<ColumnAttributes> fColAttr;

/// Number of columns.
  UInt_t fNumCols;

/// Table name (either XXX or XXXVLD)
  std::string fTableName;

ClassDef(TDbiTableMetaData,0)     //TableMetaData for database table

};
};


#endif  // DBITABLEMETADATA_H


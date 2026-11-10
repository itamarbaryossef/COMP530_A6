
#ifndef SELECTION_TEST_HELPERS_H
#define SELECTION_TEST_HELPERS_H

// Helpers for the RegularSelection tests (StudentQUnitStage1.cc).  They use
// only the operator's public interface (the constructor and run ()), so they
// work with any correct implementation

#include "RegularSelection.h"
#include "TestHelpers.h"

// runs a RegularSelection over input, writing into a fresh heap table called
// outName (stored in outName.bin) with schema outSchema, and returns the
// output's rows, sorted (see dumpSorted).  The caller unlinks outName.bin
inline vector <string> runSelection (MyDB_BufferManagerPtr mgr, MyDB_TableReaderWriterPtr input,
	MyDB_SchemaPtr outSchema, string predicate, vector <string> projections, string outName = "selOut") {

	MyDB_TableReaderWriterPtr out = loadTable (mgr, outName, outSchema);
	RegularSelection op (input, out, predicate, projections);
	op.run ();
	return dumpSorted (out);
}

// the same, for the common case of copying every supplier attribute: the
// output has the supplier schema, and every input attribute is projected
inline vector <string> selectSuppliers (MyDB_BufferManagerPtr mgr, MyDB_TableReaderWriterPtr input,
	string predicate, string outName = "selOut") {

	return runSelection (mgr, input, supplierSchema ("o_"), predicate, supplierProjections ("s_"), outName);
}

#endif

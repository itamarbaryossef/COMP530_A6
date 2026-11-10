
#ifndef BPLUS_SELECTION_TEST_HELPERS_H
#define BPLUS_SELECTION_TEST_HELPERS_H

// Helpers for the BPlusSelection tests (StudentQUnitStage2.cc).  They use
// only the operator's public interface (the constructor and run ()), so they
// work with any correct implementation

#include "BPlusSelection.h"
#include "TestHelpers.h"

// runs a BPlusSelection over the key range [low, high] of input, writing into
// a fresh heap table called outName (stored in outName.bin) with schema
// outSchema, and returns the output's rows, sorted (see dumpSorted).  The
// caller unlinks outName.bin
inline vector <string> runBPlusSelection (MyDB_BufferManagerPtr mgr, MyDB_BPlusTreeReaderWriterPtr input,
	MyDB_SchemaPtr outSchema, MyDB_AttValPtr low, MyDB_AttValPtr high, string predicate,
	vector <string> projections, string outName = "selOut") {

	MyDB_TableReaderWriterPtr out = loadTable (mgr, outName, outSchema);
	BPlusSelection op (input, out, low, high, predicate, projections);
	op.run ();
	return dumpSorted (out);
}

// the same, for the common case of copying every supplier attribute (the
// tree's attributes must have the "s_" prefix)
inline vector <string> bplusSuppliers (MyDB_BufferManagerPtr mgr, MyDB_BPlusTreeReaderWriterPtr input,
	MyDB_AttValPtr low, MyDB_AttValPtr high, string predicate, string outName = "selOut") {

	return runBPlusSelection (mgr, input, supplierSchema ("o_"), low, high, predicate, supplierProjections ("s_"), outName);
}

#endif

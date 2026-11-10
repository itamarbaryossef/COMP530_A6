
#ifndef JOIN_TEST_HELPERS_H
#define JOIN_TEST_HELPERS_H

// Helpers for the join tests (StudentQUnitStage3.cc).  They use only the
// operators' public interfaces (the constructors and run ()), so they work
// with any correct implementation of SortMergeJoin

#include "ScanJoin.h"
#include "SortMergeJoin.h"
#include "TestHelpers.h"

// everything that describes one join, in the terms both join operators use
struct JoinSpec {
	string finalPredicate;
	vector <string> projections;
	pair <string, string> equalityCheck;
	string leftPredicate;
	string rightPredicate;
};

// the output schema for a join: one attribute per projection, named o_0,
// o_1, ..., whose type is the type of that projection computed over a
// record combining the left and right attributes
inline MyDB_SchemaPtr joinOutSchema (MyDB_TableReaderWriterPtr left, MyDB_TableReaderWriterPtr right,
	vector <string> projections) {

	MyDB_SchemaPtr combined = make_shared <MyDB_Schema> ();
	for (auto &a : left->getTable ()->getSchema ()->getAtts ())
		combined->appendAtt (a);
	for (auto &a : right->getTable ()->getSchema ()->getAtts ())
		combined->appendAtt (a);
	MyDB_RecordPtr rec = make_shared <MyDB_Record> (combined);

	MyDB_SchemaPtr out = make_shared <MyDB_Schema> ();
	for (size_t i = 0; i < projections.size (); i++)
		out->appendAtt (make_pair ("o_" + to_string (i), rec->getType (projections[i])));
	return out;
}

// runs a SortMergeJoin of left and right into a fresh heap table called
// outName (stored in outName.bin), and returns the output's rows, sorted
// (see dumpSorted).  The caller unlinks outName.bin
inline vector <string> runSortMergeJoin (MyDB_BufferManagerPtr mgr, MyDB_TableReaderWriterPtr left,
	MyDB_TableReaderWriterPtr right, JoinSpec spec, string outName = "smjOut") {

	MyDB_TableReaderWriterPtr out = loadTable (mgr, outName, joinOutSchema (left, right, spec.projections));
	SortMergeJoin op (left, right, out, spec.finalPredicate, spec.projections, spec.equalityCheck,
		spec.leftPredicate, spec.rightPredicate);
	op.run ();
	return dumpSorted (out);
}

// the same join run by the given ScanJoin, as a second answer key
inline vector <string> runScanJoin (MyDB_BufferManagerPtr mgr, MyDB_TableReaderWriterPtr left,
	MyDB_TableReaderWriterPtr right, JoinSpec spec, string outName = "scanOut") {

	MyDB_TableReaderWriterPtr out = loadTable (mgr, outName, joinOutSchema (left, right, spec.projections));
	vector <pair <string, string>> eqs {spec.equalityCheck};
	ScanJoin op (left, right, out, spec.finalPredicate, spec.projections, eqs,
		spec.leftPredicate, spec.rightPredicate);
	op.run ();
	return dumpSorted (out);
}

#endif

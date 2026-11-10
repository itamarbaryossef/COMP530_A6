
#ifndef AGG_TEST_HELPERS_H
#define AGG_TEST_HELPERS_H

// Helpers for the aggregation tests (StudentQUnitStage4.cc).  They use only
// the operator's public interface (the constructor and run ()), and look only
// at the contents of the output table, so they work with any correct
// implementation of Aggregate

#include "Aggregate.h"
#include "TestHelpers.h"

// attribute types, for building output schemas
inline MyDB_AttTypePtr intType () { return make_shared <MyDB_IntAttType> (); }
inline MyDB_AttTypePtr doubleType () { return make_shared <MyDB_DoubleAttType> (); }
inline MyDB_AttTypePtr stringType () { return make_shared <MyDB_StringAttType> (); }

// a schema with one attribute per type, named o_0, o_1, ...  For an aggregate,
// list the grouping attributes' types first, then the aggregates' types
inline MyDB_SchemaPtr aggOutSchema (vector <MyDB_AttTypePtr> types) {
	MyDB_SchemaPtr s = make_shared <MyDB_Schema> ();
	for (size_t i = 0; i < types.size (); i++)
		s->appendAtt (make_pair ("o_" + to_string (i), types[i]));
	return s;
}

// runs an Aggregate over input into a fresh heap table called outName (stored
// in outName.bin) with schema outSchema, and returns the output's rows, sorted
// (see dumpSorted).  The caller unlinks outName.bin
inline vector <string> runAggregate (MyDB_BufferManagerPtr mgr, MyDB_TableReaderWriterPtr input,
	MyDB_SchemaPtr outSchema, vector <pair <MyDB_AggType, string>> aggs, vector <string> groupings,
	string predicate, string outName = "aggOut") {

	MyDB_TableReaderWriterPtr out = loadTable (mgr, outName, outSchema);
	Aggregate op (input, out, aggs, groupings, predicate);
	op.run ();
	return dumpSorted (out);
}

// field i (from 0) of a row printed as "a|b|c|"
inline string fieldOf (string row, int i) {
	stringstream ss (row);
	string f;
	for (int j = 0; j <= i; j++)
		if (!getline (ss, f, '|'))
			return "";
	return f;
}

// the same, as a number
inline double numOf (string row, int i) {
	return atof (fieldOf (row, i).c_str ());
}

// the total, over all rows, of field i
inline double fieldTotal (vector <string> rows, int i) {
	double t = 0;
	for (auto &r : rows)
		t += numOf (r, i);
	return t;
}

// true if, in every row, AVG (field avgAt) times COUNT (field cntAt) equals
// SUM (field sumAt), to within a relative tolerance
inline bool avgTimesCountIsSum (vector <string> rows, int sumAt, int avgAt, int cntAt) {
	for (auto &r : rows) {
		double sum = numOf (r, sumAt), prod = numOf (r, avgAt) * numOf (r, cntAt);
		if (fabs (sum - prod) > 1e-6 * max (1.0, fabs (sum))) {
			cout << "AVG x COUNT != SUM in row " << r << "\n";
			return false;
		}
	}
	return true;
}

// for an aggregate with no GROUP BY over no qualifying input: SQL would return
// one row with COUNT 0 (and NULL sums), but MyDB has no NULL, so we accept
// either no rows at all or a single row whose COUNT (field cntAt) is 0
inline bool emptyAggregateOK (vector <string> rows, int cntAt) {
	return rows.empty () || (rows.size () == 1 && fieldOf (rows[0], cntAt) == "0");
}

#endif

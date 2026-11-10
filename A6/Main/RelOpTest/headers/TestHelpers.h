
#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

// Helpers for writing tests of the relational operators.  Shared by my
// tests (RelOpQUnit.cc) and yours (StudentQUnitStageN.cc).  Feel free to add
// to this file; it is compiled into every test program.

#include "MyDB_AttType.h"
#include "MyDB_BufferManager.h"
#include "MyDB_BPlusTreeReaderWriter.h"
#include "MyDB_Record.h"
#include "MyDB_Schema.h"
#include "MyDB_Table.h"
#include "MyDB_TableReaderWriter.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

/*************************************************/
/** schemas for the tables shipped in Build/    **/
/*************************************************/

// the TPC-H supplier schema (supplier.tbl, supplierBig.tbl).  Every
// attribute name gets the given prefix, so that two copies of the table
// can be told apart in a join ("l_suppkey" versus "r_suppkey")
inline MyDB_SchemaPtr supplierSchema (string prefix = "") {
	MyDB_SchemaPtr s = make_shared <MyDB_Schema> ();
	s->appendAtt (make_pair (prefix + "suppkey", make_shared <MyDB_IntAttType> ()));
	s->appendAtt (make_pair (prefix + "name", make_shared <MyDB_StringAttType> ()));
	s->appendAtt (make_pair (prefix + "address", make_shared <MyDB_StringAttType> ()));
	s->appendAtt (make_pair (prefix + "nationkey", make_shared <MyDB_IntAttType> ()));
	s->appendAtt (make_pair (prefix + "phone", make_shared <MyDB_StringAttType> ()));
	s->appendAtt (make_pair (prefix + "acctbal", make_shared <MyDB_DoubleAttType> ()));
	s->appendAtt (make_pair (prefix + "comment", make_shared <MyDB_StringAttType> ()));
	return s;
}

// the TPC-H nation schema (nation.tbl, 25 rows)
inline MyDB_SchemaPtr nationSchema (string prefix = "") {
	MyDB_SchemaPtr s = make_shared <MyDB_Schema> ();
	s->appendAtt (make_pair (prefix + "nationkey", make_shared <MyDB_IntAttType> ()));
	s->appendAtt (make_pair (prefix + "name", make_shared <MyDB_StringAttType> ()));
	s->appendAtt (make_pair (prefix + "regionkey", make_shared <MyDB_IntAttType> ()));
	s->appendAtt (make_pair (prefix + "comment", make_shared <MyDB_StringAttType> ()));
	return s;
}

// the TPC-H region schema (region.tbl, 5 rows)
inline MyDB_SchemaPtr regionSchema (string prefix = "") {
	MyDB_SchemaPtr s = make_shared <MyDB_Schema> ();
	s->appendAtt (make_pair (prefix + "regionkey", make_shared <MyDB_IntAttType> ()));
	s->appendAtt (make_pair (prefix + "name", make_shared <MyDB_StringAttType> ()));
	s->appendAtt (make_pair (prefix + "comment", make_shared <MyDB_StringAttType> ()));
	return s;
}

// the beer-drinker schemas from lecture (likes.tbl, serves.tbl, freq.tbl):
// a schema of two string attributes with the given names
inline MyDB_SchemaPtr twoStrings (string first, string second) {
	MyDB_SchemaPtr s = make_shared <MyDB_Schema> ();
	s->appendAtt (make_pair (first, make_shared <MyDB_StringAttType> ()));
	s->appendAtt (make_pair (second, make_shared <MyDB_StringAttType> ()));
	return s;
}

// projections that copy every attribute of the supplier schema with the
// given prefix, in schema order ("[s_suppkey]", "[s_name]", ...)
inline vector <string> supplierProjections (string prefix = "") {
	// keep the schema alive: getAtts () returns a reference into it
	MyDB_SchemaPtr s = supplierSchema (prefix);
	vector <string> p;
	for (auto &a : s->getAtts ())
		p.push_back ("[" + a.first + "]");
	return p;
}

/*************************************************/
/** making tables                               **/
/*************************************************/

// creates a heap table called name, stored in name.bin, and loads it from
// the given .tbl file (or leaves it empty if fileName is "")
inline MyDB_TableReaderWriterPtr loadTable (MyDB_BufferManagerPtr mgr, string name, MyDB_SchemaPtr schema, string fileName = "") {
	unlink ((name + ".bin").c_str ());
	MyDB_TablePtr t = make_shared <MyDB_Table> (name, name + ".bin", schema);
	MyDB_TableReaderWriterPtr rw = make_shared <MyDB_TableReaderWriter> (t, mgr);
	if (fileName != "")
		rw->loadFromTextFile (fileName);
	return rw;
}

// the same, but the table is a B+-tree on the attribute sortAtt
inline MyDB_BPlusTreeReaderWriterPtr loadBPlus (MyDB_BufferManagerPtr mgr, string name, MyDB_SchemaPtr schema, string sortAtt, string fileName = "") {
	unlink ((name + ".bin").c_str ());
	MyDB_TablePtr t = make_shared <MyDB_Table> (name, name + ".bin", schema);
	MyDB_BPlusTreeReaderWriterPtr rw = make_shared <MyDB_BPlusTreeReaderWriter> (sortAtt, t, mgr);
	if (fileName != "")
		rw->loadFromTextFile (fileName);
	return rw;
}

// an attribute value, for the low and high bounds of a B+-tree selection
inline MyDB_AttValPtr intVal (int v) {
	MyDB_IntAttValPtr r = make_shared <MyDB_IntAttVal> ();
	r->set (v);
	return r;
}
inline MyDB_AttValPtr doubleVal (double v) {
	MyDB_DoubleAttValPtr r = make_shared <MyDB_DoubleAttVal> ();
	r->set (v);
	return r;
}
inline MyDB_AttValPtr stringVal (string v) {
	MyDB_StringAttValPtr r = make_shared <MyDB_StringAttVal> ();
	r->set (v);
	return r;
}

/*************************************************/
/** looking at results                          **/
/*************************************************/

// every record in the table, printed the way MyDB prints records
// ("a|b|c|"), sorted.  Sorting makes the result a multiset: two tables with
// the same rows in different orders dump the same
inline vector <string> dumpSorted (MyDB_TableReaderWriterPtr table) {
	vector <string> rows;
	MyDB_RecordPtr rec = table->getEmptyRecord ();
	MyDB_RecordIteratorAltPtr iter = table->getIteratorAlt ();
	while (iter->advance ()) {
		iter->getCurrent (rec);
		stringstream ss;
		ss << rec;
		rows.push_back (ss.str ());
	}
	sort (rows.begin (), rows.end ());
	return rows;
}

// the number of records in the table
inline int countRecords (MyDB_TableReaderWriterPtr table) {
	int n = 0;
	MyDB_RecordPtr rec = table->getEmptyRecord ();
	MyDB_RecordIteratorAltPtr iter = table->getIteratorAlt ();
	while (iter->advance ()) {
		iter->getCurrent (rec);
		n++;
	}
	return n;
}

// asks oracle.py (in the current directory, which should be Build/) for the
// answer to a SQL query, as sorted rows in MyDB's print format.  See the
// comment at the top of oracle.py for the table and column names
inline vector <string> oracle (string sql) {
	string cmd = "python3 oracle.py --sorted \"" + sql + "\"";
	vector <string> rows;
	FILE *p = popen (cmd.c_str (), "r");
	if (p == nullptr) {
		cout << "could not run " << cmd << "\n";
		return rows;
	}
	char buf[65536];
	while (fgets (buf, sizeof (buf), p) != nullptr) {
		string line (buf);
		while (!line.empty () && (line.back () == '\n' || line.back () == '\r'))
			line.pop_back ();
		rows.push_back (line);
	}
	pclose (p);
	return rows;
}

// the first (only) value of the first row of an oracle query, as a string
inline string oracleOne (string sql) {
	vector <string> rows = oracle (sql);
	if (rows.empty ())
		return "";
	string r = rows[0];
	return r.substr (0, r.find ('|'));
}

// compares two dumps field by field.  Fields that both parse as numbers are
// compared to within tol (so 4192.400000 and 4192.4000001 are the same);
// everything else must match exactly.  Prints the first difference
inline bool sameRows (vector <string> got, vector <string> expected, double tol = 1e-4) {
	if (got.size () != expected.size ()) {
		cout << "got " << got.size () << " rows, expected " << expected.size () << "\n";
		return false;
	}
	for (size_t i = 0; i < got.size (); i++) {
		stringstream a (got[i]), b (expected[i]);
		string fa, fb;
		bool ok = true;
		while (true) {
			bool ha = (bool) getline (a, fa, '|');
			bool hb = (bool) getline (b, fb, '|');
			if (ha != hb) { ok = false; break; }
			if (!ha) break;
			if (fa == fb) continue;
			char *ea, *eb;
			double da = strtod (fa.c_str (), &ea), db = strtod (fb.c_str (), &eb);
			bool numA = (*ea == 0 && !fa.empty ()), numB = (*eb == 0 && !fb.empty ());
			if (numA && numB && fabs (da - db) <= tol * max (1.0, fabs (db)))
				continue;
			ok = false;
			break;
		}
		if (!ok) {
			cout << "row " << i << " differs:\n   got      " << got[i] << "\n   expected " << expected[i] << "\n";
			return false;
		}
	}
	return true;
}

#endif

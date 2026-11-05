/****************************************************
** COMP 530 A6 TESTS -- Fall 2026                   **
**                                                 **
** Five examples, one per operator plus a          **
** pipeline, all on the beer-drinker tables from   **
** lecture, where every answer is known by hand    **
** and confirmed by oracle.py.  These show the     **
** format and the helpers; they are not a test     **
** suite.  Yours go in StudentQUnitStageN.cc.      **
**                                                 **
**   relOpUnitTest          run all                **
**   relOpUnitTest N        start from test N      **
**   relOpUnitTest N only   run test N and stop    **
**                                                 **
** Run from Build/, where the .tbl files and       **
** oracle.py are.  main () returns the number of   **
** failures.                                       **
****************************************************/

#ifndef RELOP_TEST_H
#define RELOP_TEST_H

#include "Aggregate.h"
#include "BPlusSelection.h"
#include "QUnit.h"
#include "RegularSelection.h"
#include "ScanJoin.h"
#include "SortMergeJoin.h"
#include "TestHelpers.h"
#include <iostream>

#define FALLTHROUGH_INTENDED do {} while (0)

static bool runOne = false;

int main (int argc, char *argv[]) {
	int start = 1;
	if (argc > 1 && argv[1][0] >= '0' && argv[1][0] <= '9') {
		start = atoi (argv[1]);
	}
	if (argc > 2 && string (argv[2]) == "only") {
		runOne = true;
		cout << "running only test " << start << endl;
	} else {
		cout << "start from test " << start << endl;
	}

	QUnit::UnitTest qunit (cerr, QUnit::normal);
	MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");

	switch (start) {

	case 1:
	{
		// RegularSelection: who likes PBR?  Homer and Marge
		cout << "TEST 1..." << flush;
		MyDB_TableReaderWriterPtr likes = loadTable (mgr, "likes", twoStrings ("l_dr", "l_beer"), "likes.tbl");
		MyDB_SchemaPtr outSchema = make_shared <MyDB_Schema> ();
		outSchema->appendAtt (make_pair ("dr", make_shared <MyDB_StringAttType> ()));
		MyDB_TableReaderWriterPtr out = loadTable (mgr, "selOut", outSchema);
		vector <string> projections {"[l_dr]"};
		RegularSelection op (likes, out, "== ([l_beer], string[PBR])", projections);
		op.run ();
		vector <string> rows = dumpSorted (out);
		cout << rows.size () << " drinkers like PBR" << endl << flush;
		QUNIT_IS_TRUE (rows == (vector <string> {"Homer|", "Marge|"}));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT dr FROM likes WHERE beer = 'PBR'")));
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 2:
	{
		// BPlusSelection: a B+-tree on the drinker, and what Homer likes.
		// PBR and Coors
		cout << "TEST 2..." << flush;
		MyDB_BPlusTreeReaderWriterPtr likes = loadBPlus (mgr, "likesTree", twoStrings ("l_dr", "l_beer"), "l_dr", "likes.tbl");
		MyDB_SchemaPtr outSchema = make_shared <MyDB_Schema> ();
		outSchema->appendAtt (make_pair ("beer", make_shared <MyDB_StringAttType> ()));
		MyDB_TableReaderWriterPtr out = loadTable (mgr, "selOut", outSchema);
		vector <string> projections {"[l_beer]"};
		BPlusSelection op (likes, out, stringVal ("Homer"), stringVal ("Homer"), "bool[true]", projections);
		op.run ();
		vector <string> rows = dumpSorted (out);
		cout << rows.size () << " beers for Homer" << endl << flush;
		QUNIT_IS_TRUE (rows == (vector <string> {"Coors|", "PBR|"}));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT beer FROM likes WHERE dr = 'Homer'")));
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 3:
	{
		// SortMergeJoin: who likes PBR and goes to Moe's?  Homer.  The
		// ScanJoin (which you are given) must agree: for any join, it is a
		// second answer key
		cout << "TEST 3..." << flush;
		MyDB_TableReaderWriterPtr likes = loadTable (mgr, "likes", twoStrings ("l_dr", "l_beer"), "likes.tbl");
		MyDB_TableReaderWriterPtr freq = loadTable (mgr, "freq", twoStrings ("f_dr", "f_bar"), "freq.tbl");
		MyDB_SchemaPtr outSchema = make_shared <MyDB_Schema> ();
		outSchema->appendAtt (make_pair ("dr", make_shared <MyDB_StringAttType> ()));
		vector <string> projections {"[l_dr]"};

		MyDB_TableReaderWriterPtr out = loadTable (mgr, "joinOut", outSchema);
		SortMergeJoin op (likes, freq, out, "== ([l_dr], [f_dr])", projections,
			make_pair (string ("[l_dr]"), string ("[f_dr]")),
			"== ([l_beer], string[PBR])", "== ([f_bar], string[Moe's])");
		op.run ();
		vector <string> rows = dumpSorted (out);

		MyDB_TableReaderWriterPtr out2 = loadTable (mgr, "joinOut2", outSchema);
		vector <pair <string, string>> eqs {make_pair (string ("[l_dr]"), string ("[f_dr]"))};
		ScanJoin op2 (likes, freq, out2, "== ([l_dr], [f_dr])", projections, eqs,
			"== ([l_beer], string[PBR])", "== ([f_bar], string[Moe's])");
		op2.run ();
		vector <string> rows2 = dumpSorted (out2);

		cout << rows.size () << " drinkers like PBR and go to Moe's" << endl << flush;
		QUNIT_IS_TRUE (rows == (vector <string> {"Homer|"}));
		QUNIT_IS_TRUE (sameRows (rows, rows2));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT l.dr FROM likes l, freq f WHERE l.dr = f.dr AND l.beer = 'PBR' AND f.bar = 'Moe''s'")));
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 4:
	{
		// Aggregate: how many people like each beer?  PBR 2, Coors 1, Duff 1
		cout << "TEST 4..." << flush;
		MyDB_TableReaderWriterPtr likes = loadTable (mgr, "likes", twoStrings ("l_dr", "l_beer"), "likes.tbl");
		MyDB_SchemaPtr outSchema = make_shared <MyDB_Schema> ();
		outSchema->appendAtt (make_pair ("beer", make_shared <MyDB_StringAttType> ()));
		outSchema->appendAtt (make_pair ("cnt", make_shared <MyDB_IntAttType> ()));
		MyDB_TableReaderWriterPtr out = loadTable (mgr, "aggOut", outSchema);
		vector <pair <MyDB_AggType, string>> aggs {make_pair (MyDB_AggType :: cnt, "int[0]")};
		vector <string> groupings {"[l_beer]"};
		Aggregate op (likes, out, aggs, groupings, "bool[true]");
		op.run ();
		vector <string> rows = dumpSorted (out);
		cout << rows.size () << " beers" << endl << flush;
		QUNIT_IS_TRUE (rows == (vector <string> {"Coors|1|", "Duff|1|", "PBR|2|"}));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT beer, count(*) FROM likes GROUP BY beer")));
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 5:
	{
		// a pipeline: join freq with serves on the bar to get, for each
		// drinker, the beers served where they go; then count per drinker.
		// Homer 2 (Moe's: PBR, Duff), Lenny 2, Marge 1, Carl 1
		cout << "TEST 5..." << flush;
		MyDB_TableReaderWriterPtr freq = loadTable (mgr, "freq", twoStrings ("f_dr", "f_bar"), "freq.tbl");
		MyDB_TableReaderWriterPtr serves = loadTable (mgr, "serves", twoStrings ("s_bar", "s_beer"), "serves.tbl");
		MyDB_TableReaderWriterPtr joined = loadTable (mgr, "joinOut", twoStrings ("j_dr", "j_beer"));
		vector <string> projections {"[f_dr]", "[s_beer]"};
		SortMergeJoin join (freq, serves, joined, "== ([f_bar], [s_bar])", projections,
			make_pair (string ("[f_bar]"), string ("[s_bar]")), "bool[true]", "bool[true]");
		join.run ();
		vector <string> rows = dumpSorted (joined);
		cout << rows.size () << " (drinker, beer) pairs..." << flush;
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT f.dr, s.beer FROM freq f, serves s WHERE f.bar = s.bar")));

		MyDB_SchemaPtr outSchema = make_shared <MyDB_Schema> ();
		outSchema->appendAtt (make_pair ("dr", make_shared <MyDB_StringAttType> ()));
		outSchema->appendAtt (make_pair ("cnt", make_shared <MyDB_IntAttType> ()));
		MyDB_TableReaderWriterPtr out = loadTable (mgr, "aggOut", outSchema);
		vector <pair <MyDB_AggType, string>> aggs {make_pair (MyDB_AggType :: cnt, "int[0]")};
		vector <string> groupings {"[j_dr]"};
		Aggregate agg (joined, out, aggs, groupings, "bool[true]");
		agg.run ();
		rows = dumpSorted (out);
		cout << rows.size () << " drinkers" << endl << flush;
		QUNIT_IS_TRUE (rows == (vector <string> {"Carl|1|", "Homer|2|", "Lenny|2|", "Marge|1|"}));
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	default:
		break;
	}

	for (string f : {"likes.bin", "likesTree.bin", "freq.bin", "serves.bin", "selOut.bin", "joinOut.bin", "joinOut2.bin", "aggOut.bin", "tempFile"})
		unlink (f.c_str ());

	return qunit.errors ();
}

#endif

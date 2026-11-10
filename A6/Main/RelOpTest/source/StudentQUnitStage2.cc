/****************************************************
** COMP 530 A6 -- Stage 2 tests: BPlusSelection    **
**                                                 **
** This file is yours (well, your AI's).  Rules:   **
**  - every test is a case in the switch below,    **
**    numbered from 1, each ending in              **
**    FALLTHROUGH_INTENDED; if (runOne) break;     **
**  - every test prints "TEST N..." first, cleans  **
**    up its own files, and can run on its own     **
**  - expected answers come from oracle.py, from   **
**    a hand-checkable table, or from another      **
**    operator; never from running the code under  **
**    test and pasting what it printed             **
**  - helpers go in headers/TestHelpers.h or a new **
**    header under headers/; test code goes only   **
**    in this file                                 **
**                                                 **
**   studentUnitTestStage2          run all        **
**   studentUnitTestStage2 N        start from N   **
**   studentUnitTestStage2 N only   run N and stop **
****************************************************/

#include "Aggregate.h"
#include "BPlusSelection.h"
#include "QUnit.h"
#include "RegularSelection.h"
#include "ScanJoin.h"
#include "SortMergeJoin.h"
#include "BPlusSelectionTestHelpers.h"
#include "SelectionTestHelpers.h"
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

	switch (start) {

	case 1:
	{
		// an example of the format: a B+-tree on the drinker's name, and the
		// beers that Homer likes (PBR and Coors, by hand and by the oracle)
		cout << "TEST 1..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
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
		unlink ("likesTree.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 2:
	{
		// the whole table is in range, but no row satisfies the predicate
		cout << "TEST 2..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "suppTree", supplierSchema ("s_"), "s_suppkey", "supplier.tbl");
		vector <string> rows = bplusSuppliers (mgr, tree, intVal (1), intVal (10000), "> ([s_acctbal], double[10000.0])");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE suppkey BETWEEN 1 AND 10000 AND acctbal > 10000.0");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 0);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("suppTree.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 3:
	{
		// the whole table is in range, and every row satisfies the predicate
		cout << "TEST 3..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "suppTree", supplierSchema ("s_"), "s_suppkey", "supplier.tbl");
		vector <string> rows = bplusSuppliers (mgr, tree, intVal (1), intVal (10000), "> ([s_acctbal], double[-1000.0])");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE suppkey BETWEEN 1 AND 10000 AND acctbal > -1000.0");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 10000);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("suppTree.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 4:
	{
		// every row satisfies the predicate, but only suppliers 100 to 104 are
		// in range (5 rows, by hand: suppkey is a key)
		cout << "TEST 4..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "suppTree", supplierSchema ("s_"), "s_suppkey", "supplier.tbl");
		vector <string> rows = bplusSuppliers (mgr, tree, intVal (100), intVal (104), "bool[true]");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE suppkey BETWEEN 100 AND 104");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 5);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("suppTree.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 5:
	{
		// every row satisfies the predicate, but no row is in range: (a) a range
		// above the largest key, (b) a range below the smallest
		cout << "TEST 5..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "suppTree", supplierSchema ("s_"), "s_suppkey", "supplier.tbl");
		vector <string> rowsA = bplusSuppliers (mgr, tree, intVal (20000), intVal (30000), "bool[true]", "selOutA");
		vector <string> expectedA = oracle ("SELECT * FROM supplier WHERE suppkey BETWEEN 20000 AND 30000");
		vector <string> rowsB = bplusSuppliers (mgr, tree, intVal (-50), intVal (0), "bool[true]", "selOutB");
		vector <string> expectedB = oracle ("SELECT * FROM supplier WHERE suppkey BETWEEN -50 AND 0");
		cout << rowsA.size () << " and " << rowsB.size () << " rows (expected " << expectedA.size ()
			<< " and " << expectedB.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rowsA.size (), 0);
		QUNIT_IS_EQUAL ((int) rowsB.size (), 0);
		QUNIT_IS_TRUE (sameRows (rowsA, expectedA));
		QUNIT_IS_TRUE (sameRows (rowsB, expectedB));
		unlink ("suppTree.bin");
		unlink ("selOutA.bin");
		unlink ("selOutB.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 6:
	{
		// low > high: the range [200, 100] is empty, so nothing comes out
		// (by hand)
		cout << "TEST 6..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "suppTree", supplierSchema ("s_"), "s_suppkey", "supplier.tbl");
		vector <string> rows = bplusSuppliers (mgr, tree, intVal (200), intVal (100), "bool[true]");
		cout << rows.size () << " rows" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 0);
		unlink ("suppTree.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 7:
	{
		// low = high, matching exactly one record (supplier 4242)
		cout << "TEST 7..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "suppTree", supplierSchema ("s_"), "s_suppkey", "supplier.tbl");
		vector <string> rows = bplusSuppliers (mgr, tree, intVal (4242), intVal (4242), "bool[true]");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE suppkey BETWEEN 4242 AND 4242");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 1);
		QUNIT_IS_TRUE (rows.size () == 1 && rows[0].substr (0, 5) == "4242|");
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("suppTree.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 8:
	{
		// 100 rows in range, but only one of them satisfies the predicate (names
		// are unique)
		cout << "TEST 8..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "suppTree", supplierSchema ("s_"), "s_suppkey", "supplier.tbl");
		vector <string> rows = bplusSuppliers (mgr, tree, intVal (100), intVal (199), "== ([s_name], string[Supplier#000000150])");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE suppkey BETWEEN 100 AND 199 AND name = 'Supplier#000000150'");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 1);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("suppTree.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 9:
	{
		// several rows satisfy the predicate and are in range, but not all of
		// the rows that satisfy it: nation 3 among suppliers 1 to 2000
		cout << "TEST 9..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "suppTree", supplierSchema ("s_"), "s_suppkey", "supplier.tbl");
		vector <string> rows = bplusSuppliers (mgr, tree, intVal (1), intVal (2000), "== ([s_nationkey], int[3])");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE suppkey BETWEEN 1 AND 2000 AND nationkey = 3");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		int all = (int) oracle ("SELECT * FROM supplier WHERE nationkey = 3").size ();
		QUNIT_IS_TRUE (rows.size () > 1);
		QUNIT_IS_TRUE ((int) rows.size () < all);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("suppTree.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 10:
	{
		// both bounds are inclusive: [1000, 1009], where 1000 and 1009 are both
		// keys, gives 10 rows including suppliers 1000 and 1009 (by hand)
		cout << "TEST 10..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "suppTree", supplierSchema ("s_"), "s_suppkey", "supplier.tbl");
		vector <string> rows = bplusSuppliers (mgr, tree, intVal (1000), intVal (1009), "bool[true]");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE suppkey BETWEEN 1000 AND 1009");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 10);
		bool has1000 = false, has1009 = false;
		for (auto &r : rows) {
			if (r.substr (0, 5) == "1000|") has1000 = true;
			if (r.substr (0, 5) == "1009|") has1009 = true;
		}
		QUNIT_IS_TRUE (has1000);
		QUNIT_IS_TRUE (has1009);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("suppTree.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 11:
	{
		// the predicate is on a non-key attribute: of the 500 suppliers in
		// range, only those with a negative balance come out
		cout << "TEST 11..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "suppTree", supplierSchema ("s_"), "s_suppkey", "supplier.tbl");
		vector <string> rows = bplusSuppliers (mgr, tree, intVal (1), intVal (500), "< ([s_acctbal], double[0.0])");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE suppkey BETWEEN 1 AND 500 AND acctbal < 0.0");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0);
		QUNIT_IS_TRUE (rows.size () < 500);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("suppTree.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 12:
	{
		// every Stage 1 supplier predicate, with the whole table in range: the
		// output must match RegularSelection on a heap copy of supplier, and
		// the oracle
		cout << "TEST 12..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "suppTree", supplierSchema ("s_"), "s_suppkey", "supplier.tbl");
		MyDB_TableReaderWriterPtr heap = loadTable (mgr, "suppHeap", supplierSchema ("s_"), "supplier.tbl");
		vector <pair <string, string>> preds {
			{"> ([s_acctbal], double[10000.0])",
				"acctbal > 10000.0"},
			{"> ([s_acctbal], double[-1000.0])",
				"acctbal > -1000.0"},
			{"== ([s_suppkey], int[4242])",
				"suppkey = 4242"},
			{"== ([s_nationkey], int[3])",
				"nationkey = 3"},
			{"|| (== ([s_nationkey], int[3]), < ([s_acctbal], double[0.0]))",
				"nationkey = 3 OR acctbal < 0.0"},
			{"> (* ([s_acctbal], double[2.0]), double[19000.0])",
				"acctbal * 2.0 > 19000.0"},
			{"&& (== ([s_nationkey], int[3]), < ([s_acctbal], double[0.0]))",
				"nationkey = 3 AND acctbal < 0.0"},
			{"! (== ([s_nationkey], int[3]))",
				"NOT nationkey = 3"},
			{"< ([s_name], string[Supplier#000000010])",
				"name < 'Supplier#000000010'"},
			{"> (+ ([s_acctbal], [s_nationkey]), int[9990])",
				"acctbal + nationkey > 9990"},
			{"> (* ([s_nationkey], double[400.5]), [s_acctbal])",
				"nationkey * 400.5 > acctbal"},
			{"|| (&& (== ([s_nationkey], int[3]), > ([s_acctbal], double[9000.0])), && (== ([s_nationkey], int[5]), < ([s_acctbal], double[0.0])))",
				"(nationkey = 3 AND acctbal > 9000.0) OR (nationkey = 5 AND acctbal < 0.0)"},
			{"> (um ([s_acctbal]), double[900.0])",
				"-acctbal > 900.0"},
			{"== ([s_acctbal], double[4192.40])",
				"acctbal = 4192.40"},
		};
		int bad = 0;
		for (auto &p : preds) {
			vector <string> rows = bplusSuppliers (mgr, tree, intVal (1), intVal (10000), p.first, "selOutB");
			vector <string> regRows = selectSuppliers (mgr, heap, p.first, "selOutR");
			vector <string> expected = oracle ("SELECT * FROM supplier WHERE " + p.second);
			bool ok = sameRows (rows, regRows) && sameRows (rows, expected);
			if (!ok) {
				cout << "\n   mismatch on " << p.first << "\n";
				bad++;
			}
			QUNIT_IS_TRUE (sameRows (rows, regRows));
			QUNIT_IS_TRUE (sameRows (rows, expected));
		}
		cout << preds.size () - bad << " of " << preds.size () << " predicates agree" << endl << flush;
		unlink ("suppTree.bin");
		unlink ("suppHeap.bin");
		unlink ("selOutB.bin");
		unlink ("selOutR.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 13:
	{
		// a key with many duplicates, which span several leaf pages: a tree on
		// nationkey, (a) [3, 3] and (b) [5, 7]
		cout << "TEST 13..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "nationTree", supplierSchema ("s_"), "s_nationkey", "supplier.tbl");
		vector <string> rowsA = bplusSuppliers (mgr, tree, intVal (3), intVal (3), "bool[true]", "selOutA");
		vector <string> expectedA = oracle ("SELECT * FROM supplier WHERE nationkey BETWEEN 3 AND 3");
		vector <string> rowsB = bplusSuppliers (mgr, tree, intVal (5), intVal (7), "bool[true]", "selOutB");
		vector <string> expectedB = oracle ("SELECT * FROM supplier WHERE nationkey BETWEEN 5 AND 7");
		cout << rowsA.size () << " and " << rowsB.size () << " rows (expected " << expectedA.size ()
			<< " and " << expectedB.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rowsA.size () > 1);
		QUNIT_IS_TRUE (rowsB.size () > rowsA.size ());
		QUNIT_IS_TRUE (sameRows (rowsA, expectedA));
		QUNIT_IS_TRUE (sameRows (rowsB, expectedB));
		unlink ("nationTree.bin");
		unlink ("selOutA.bin");
		unlink ("selOutB.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 14:
	{
		// a string key, with inclusive string bounds that are both keys
		cout << "TEST 14..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "nameTree", supplierSchema ("s_"), "s_name", "supplier.tbl");
		vector <string> rows = bplusSuppliers (mgr, tree, stringVal ("Supplier#000000100"), stringVal ("Supplier#000000199"), "bool[true]");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE name BETWEEN 'Supplier#000000100' AND 'Supplier#000000199'");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 100);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("nameTree.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 15:
	{
		// a double key: (a) bounds that are values in the data (inclusive),
		// (b) bounds that fall between values
		cout << "TEST 15..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "balTree", supplierSchema ("s_"), "s_acctbal", "supplier.tbl");
		vector <string> rowsA = bplusSuppliers (mgr, tree, doubleVal (4192.40), doubleVal (4641.08), "bool[true]", "selOutA");
		vector <string> expectedA = oracle ("SELECT * FROM supplier WHERE acctbal BETWEEN 4192.40 AND 4641.08");
		vector <string> rowsB = bplusSuppliers (mgr, tree, doubleVal (4192.405), doubleVal (4641.075), "bool[true]", "selOutB");
		vector <string> expectedB = oracle ("SELECT * FROM supplier WHERE acctbal BETWEEN 4192.405 AND 4641.075");
		cout << rowsA.size () << " and " << rowsB.size () << " rows (expected " << expectedA.size ()
			<< " and " << expectedB.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rowsA.size () > rowsB.size ());
		QUNIT_IS_TRUE (sameRows (rowsA, expectedA));
		QUNIT_IS_TRUE (sameRows (rowsB, expectedB));
		unlink ("balTree.bin");
		unlink ("selOutA.bin");
		unlink ("selOutB.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 16:
	{
		// ranges that overlap one end of the table: (a) [-100, 5] gives
		// suppliers 1 to 5, (b) [9996, 20000] gives 9996 to 10000 (by hand)
		cout << "TEST 16..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "suppTree", supplierSchema ("s_"), "s_suppkey", "supplier.tbl");
		vector <string> rowsA = bplusSuppliers (mgr, tree, intVal (-100), intVal (5), "bool[true]", "selOutA");
		vector <string> expectedA = oracle ("SELECT * FROM supplier WHERE suppkey BETWEEN -100 AND 5");
		vector <string> rowsB = bplusSuppliers (mgr, tree, intVal (9996), intVal (20000), "bool[true]", "selOutB");
		vector <string> expectedB = oracle ("SELECT * FROM supplier WHERE suppkey BETWEEN 9996 AND 20000");
		cout << rowsA.size () << " and " << rowsB.size () << " rows (expected " << expectedA.size ()
			<< " and " << expectedB.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rowsA.size (), 5);
		QUNIT_IS_EQUAL ((int) rowsB.size (), 5);
		QUNIT_IS_TRUE (sameRows (rowsA, expectedA));
		QUNIT_IS_TRUE (sameRows (rowsB, expectedB));
		unlink ("suppTree.bin");
		unlink ("selOutA.bin");
		unlink ("selOutB.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 17:
	{
		// a large, multi-level tree (320000 rows, 32 copies of every key):
		// suppliers 500 to 600 in nations 0 to 4
		cout << "TEST 17..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "bigTree", supplierSchema ("s_"), "s_suppkey", "supplierBig.tbl");
		vector <string> rows = bplusSuppliers (mgr, tree, intVal (500), intVal (600), "< ([s_nationkey], int[5])");
		vector <string> expected = oracle ("SELECT * FROM supplierBig WHERE suppkey BETWEEN 500 AND 600 AND nationkey < 5");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("bigTree.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 18:
	{
		// computed and reordered projections over a range: name, 2 * acctbal,
		// suppkey + 1 for nation 3 among suppliers 1 to 2000
		cout << "TEST 18..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "suppTree", supplierSchema ("s_"), "s_suppkey", "supplier.tbl");
		MyDB_SchemaPtr outSchema = make_shared <MyDB_Schema> ();
		outSchema->appendAtt (make_pair ("o_name", make_shared <MyDB_StringAttType> ()));
		outSchema->appendAtt (make_pair ("o_doublebal", make_shared <MyDB_DoubleAttType> ()));
		outSchema->appendAtt (make_pair ("o_nextkey", make_shared <MyDB_IntAttType> ()));
		vector <string> projections {"[s_name]", "* ([s_acctbal], double[2.0])", "+ ([s_suppkey], int[1])"};
		vector <string> rows = runBPlusSelection (mgr, tree, outSchema, intVal (1), intVal (2000), "== ([s_nationkey], int[3])", projections);
		vector <string> expected = oracle ("SELECT name, acctbal * 2.0, suppkey + 1 FROM supplier WHERE suppkey BETWEEN 1 AND 2000 AND nationkey = 3");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("suppTree.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 19:
	{
		// two projections of the same type, in the opposite order to the input
		// (beer, then drinker), over drinkers in [Homer, Lenny].  By hand:
		// Homer likes Coors and PBR, Lenny likes Duff; Marge is out of range
		cout << "TEST 19..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_BPlusTreeReaderWriterPtr tree = loadBPlus (mgr, "likesTree", twoStrings ("l_dr", "l_beer"), "l_dr", "likes.tbl");
		vector <string> rows = runBPlusSelection (mgr, tree, twoStrings ("o_beer", "o_dr"), stringVal ("Homer"), stringVal ("Lenny"),
			"bool[true]", vector <string> {"[l_beer]", "[l_dr]"});
		cout << rows.size () << " (beer, drinker) rows" << endl << flush;
		QUNIT_IS_TRUE (rows == (vector <string> {"Coors|Homer|", "Duff|Lenny|", "PBR|Homer|"}));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT beer, dr FROM likes WHERE dr BETWEEN 'Homer' AND 'Lenny'")));
		unlink ("likesTree.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	default:
		break;
	}

	unlink ("tempFile");
	return qunit.errors ();
}

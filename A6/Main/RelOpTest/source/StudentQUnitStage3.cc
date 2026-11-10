/****************************************************
** COMP 530 A6 -- Stage 3 tests: SortMergeJoin     **
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
**   studentUnitTestStage3          run all        **
**   studentUnitTestStage3 N        start from N   **
**   studentUnitTestStage3 N only   run N and stop **
****************************************************/

#include "Aggregate.h"
#include "BPlusSelection.h"
#include "QUnit.h"
#include "RegularSelection.h"
#include "ScanJoin.h"
#include "SortMergeJoin.h"
#include "JoinTestHelpers.h"
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
		// an example of the format: who likes PBR and goes to Moe's (Homer,
		// by hand and by the oracle).  The ScanJoin is a second answer key:
		// for any join, it must produce the same rows as your SortMergeJoin
		cout << "TEST 1..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr likes = loadTable (mgr, "likes", twoStrings ("l_dr", "l_beer"), "likes.tbl");
		MyDB_TableReaderWriterPtr freq = loadTable (mgr, "freq", twoStrings ("f_dr", "f_bar"), "freq.tbl");
		MyDB_SchemaPtr outSchema = make_shared <MyDB_Schema> ();
		outSchema->appendAtt (make_pair ("dr", make_shared <MyDB_StringAttType> ()));
		MyDB_TableReaderWriterPtr out = loadTable (mgr, "joinOut", outSchema);
		vector <string> projections {"[l_dr]"};
		SortMergeJoin op (likes, freq, out, "== ([l_dr], [f_dr])", projections,
			make_pair (string ("[l_dr]"), string ("[f_dr]")),
			"== ([l_beer], string[PBR])", "== ([f_bar], string[Moe's])");
		op.run ();
		vector <string> rows = dumpSorted (out);
		cout << rows.size () << " drinkers like PBR and go to Moe's" << endl << flush;
		QUNIT_IS_TRUE (rows == (vector <string> {"Homer|"}));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT l.dr FROM likes l, freq f WHERE l.dr = f.dr AND l.beer = 'PBR' AND f.bar = 'Moe''s'")));
		unlink ("likes.bin");
		unlink ("freq.bin");
		unlink ("joinOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 2:
	{
		// both inputs empty: nothing comes out (by hand)
		cout << "TEST 2..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "emptySupplier", supplierSchema ("s_"));
		MyDB_TableReaderWriterPtr nat = loadTable (mgr, "emptyNation", nationSchema ("n_"));
		JoinSpec spec {"== ([s_nationkey], [n_nationkey])",
			vector <string> {"[s_suppkey]", "[n_name]"},
			make_pair (string ("[s_nationkey]"), string ("[n_nationkey]")),
			"bool[true]", "bool[true]"};
		vector <string> rows = runSortMergeJoin (mgr, supp, nat, spec);
		vector <string> scan = runScanJoin (mgr, supp, nat, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 0);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		unlink ("emptySupplier.bin");
		unlink ("emptyNation.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 3:
	{
		// the left input is empty: nothing comes out (by hand)
		cout << "TEST 3..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "emptySupplier", supplierSchema ("s_"));
		MyDB_TableReaderWriterPtr nat = loadTable (mgr, "nation", nationSchema ("n_"), "nation.tbl");
		JoinSpec spec {"== ([s_nationkey], [n_nationkey])",
			vector <string> {"[s_suppkey]", "[n_name]"},
			make_pair (string ("[s_nationkey]"), string ("[n_nationkey]")),
			"bool[true]", "bool[true]"};
		vector <string> rows = runSortMergeJoin (mgr, supp, nat, spec);
		vector <string> scan = runScanJoin (mgr, supp, nat, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 0);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		unlink ("emptySupplier.bin");
		unlink ("nation.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 4:
	{
		// the right input is empty: nothing comes out (by hand)
		cout << "TEST 4..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		MyDB_TableReaderWriterPtr nat = loadTable (mgr, "emptyNation", nationSchema ("n_"));
		JoinSpec spec {"== ([s_nationkey], [n_nationkey])",
			vector <string> {"[s_suppkey]", "[n_name]"},
			make_pair (string ("[s_nationkey]"), string ("[n_nationkey]")),
			"bool[true]", "bool[true]"};
		vector <string> rows = runSortMergeJoin (mgr, supp, nat, spec);
		vector <string> scan = runScanJoin (mgr, supp, nat, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 0);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		unlink ("supplier.bin");
		unlink ("emptyNation.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 5:
	{
		// no key matches: no beer is named like a bar (by hand)
		cout << "TEST 5..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr likes = loadTable (mgr, "likes", twoStrings ("l_dr", "l_beer"), "likes.tbl");
		MyDB_TableReaderWriterPtr freq = loadTable (mgr, "freq", twoStrings ("f_dr", "f_bar"), "freq.tbl");
		JoinSpec spec {"== ([l_beer], [f_bar])",
			vector <string> {"[l_dr]", "[f_dr]"},
			make_pair (string ("[l_beer]"), string ("[f_bar]")),
			"bool[true]", "bool[true]"};
		vector <string> rows = runSortMergeJoin (mgr, likes, freq, spec);
		vector <string> scan = runScanJoin (mgr, likes, freq, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 0);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT l.dr, f.dr FROM likes l, freq f WHERE l.beer = f.bar")));
		unlink ("likes.bin");
		unlink ("freq.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 6:
	{
		// exactly one match: regionkey * 10000 = suppkey only for region 1
		// (AMERICA) and supplier 10000
		cout << "TEST 6..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr reg = loadTable (mgr, "region", regionSchema ("r_"), "region.tbl");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		JoinSpec spec {"== (* ([r_regionkey], int[10000]), [s_suppkey])",
			vector <string> {"[r_name]", "[s_suppkey]"},
			make_pair (string ("* ([r_regionkey], int[10000])"), string ("[s_suppkey]")),
			"bool[true]", "bool[true]"};
		vector <string> rows = runSortMergeJoin (mgr, reg, supp, spec);
		vector <string> scan = runScanJoin (mgr, reg, supp, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows == (vector <string> {"AMERICA|10000|"}));
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT r.name, s.suppkey FROM region r, supplier s WHERE r.regionkey * 10000 = s.suppkey")));
		unlink ("region.bin");
		unlink ("supplier.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 7:
	{
		// several matches, checkable by hand: likes joined with serves on the
		// beer
		cout << "TEST 7..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr likes = loadTable (mgr, "likes", twoStrings ("l_dr", "l_beer"), "likes.tbl");
		MyDB_TableReaderWriterPtr serves = loadTable (mgr, "serves", twoStrings ("b_bar", "b_beer"), "serves.tbl");
		JoinSpec spec {"== ([l_beer], [b_beer])",
			vector <string> {"[l_dr]", "[l_beer]", "[b_bar]"},
			make_pair (string ("[l_beer]"), string ("[b_beer]")),
			"bool[true]", "bool[true]"};
		vector <string> rows = runSortMergeJoin (mgr, likes, serves, spec);
		vector <string> scan = runScanJoin (mgr, likes, serves, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows == (vector <string> {"Homer|Coors|Hilltop|", "Homer|PBR|Moe's|", "Lenny|Duff|Moe's|", "Marge|PBR|Moe's|"}));
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT l.dr, l.beer, s.bar FROM likes l, serves s WHERE l.beer = s.beer")));
		unlink ("likes.bin");
		unlink ("serves.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 8:
	{
		// every record matches every other: the 5 nations of region 0 joined
		// with themselves on the region (5 x 5 = 25)
		cout << "TEST 8..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr natA = loadTable (mgr, "nationA", nationSchema ("a_"), "nation.tbl");
		MyDB_TableReaderWriterPtr natB = loadTable (mgr, "nationB", nationSchema ("b_"), "nation.tbl");
		JoinSpec spec {"== ([a_regionkey], [b_regionkey])",
			vector <string> {"[a_name]", "[b_name]"},
			make_pair (string ("[a_regionkey]"), string ("[b_regionkey]")),
			"== ([a_regionkey], int[0])", "== ([b_regionkey], int[0])"};
		vector <string> rows = runSortMergeJoin (mgr, natA, natB, spec);
		vector <string> scan = runScanJoin (mgr, natA, natB, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 25);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT a.name, b.name FROM nation a, nation b WHERE a.regionkey = b.regionkey AND a.regionkey = 0 AND b.regionkey = 0")));
		unlink ("nationA.bin");
		unlink ("nationB.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 9:
	{
		// repeated keys on the left (about 400 suppliers per nation), each
		// matching one nation
		cout << "TEST 9..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		MyDB_TableReaderWriterPtr nat = loadTable (mgr, "nation", nationSchema ("n_"), "nation.tbl");
		JoinSpec spec {"== ([s_nationkey], [n_nationkey])",
			vector <string> {"[s_suppkey]", "[n_name]"},
			make_pair (string ("[s_nationkey]"), string ("[n_nationkey]")),
			"bool[true]", "bool[true]"};
		vector <string> rows = runSortMergeJoin (mgr, supp, nat, spec);
		vector <string> scan = runScanJoin (mgr, supp, nat, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 10000);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT s.suppkey, n.name FROM supplier s, nation n WHERE s.nationkey = n.nationkey")));
		unlink ("supplier.bin");
		unlink ("nation.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 10:
	{
		// repeated keys on the right: each nation matches about 400 suppliers
		cout << "TEST 10..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr nat = loadTable (mgr, "nation", nationSchema ("n_"), "nation.tbl");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		JoinSpec spec {"== ([n_nationkey], [s_nationkey])",
			vector <string> {"[n_name]", "[s_suppkey]"},
			make_pair (string ("[n_nationkey]"), string ("[s_nationkey]")),
			"bool[true]", "bool[true]"};
		vector <string> rows = runSortMergeJoin (mgr, nat, supp, spec);
		vector <string> scan = runScanJoin (mgr, nat, supp, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 10000);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT n.name, s.suppkey FROM nation n, supplier s WHERE n.nationkey = s.nationkey")));
		unlink ("nation.bin");
		unlink ("supplier.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 11:
	{
		// repeated keys on both sides: suppliers 1 to 200 joined with
		// themselves on the nation
		cout << "TEST 11..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr suppA = loadTable (mgr, "supplierA", supplierSchema ("a_"), "supplier.tbl");
		MyDB_TableReaderWriterPtr suppB = loadTable (mgr, "supplierB", supplierSchema ("b_"), "supplier.tbl");
		JoinSpec spec {"== ([a_nationkey], [b_nationkey])",
			vector <string> {"[a_suppkey]", "[b_suppkey]"},
			make_pair (string ("[a_nationkey]"), string ("[b_nationkey]")),
			"! (> ([a_suppkey], int[200]))", "! (> ([b_suppkey], int[200]))"};
		vector <string> rows = runSortMergeJoin (mgr, suppA, suppB, spec);
		vector <string> scan = runScanJoin (mgr, suppA, suppB, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT a.suppkey, b.suppkey FROM supplier a, supplier b WHERE a.nationkey = b.nationkey AND a.suppkey <= 200 AND b.suppkey <= 200")));
		unlink ("supplierA.bin");
		unlink ("supplierB.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 12:
	{
		// inputs big enough to be sorted in many runs: with 4 KB pages and a
		// 64-page buffer, supplier takes hundreds of pages
		cout << "TEST 12..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (4096, 64, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		MyDB_TableReaderWriterPtr nat = loadTable (mgr, "nation", nationSchema ("n_"), "nation.tbl");
		JoinSpec spec {"== ([s_nationkey], [n_nationkey])",
			vector <string> {"[s_suppkey]", "[n_name]"},
			make_pair (string ("[s_nationkey]"), string ("[n_nationkey]")),
			"bool[true]", "bool[true]"};
		vector <string> rows = runSortMergeJoin (mgr, supp, nat, spec);
		vector <string> scan = runScanJoin (mgr, supp, nat, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 10000);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT s.suppkey, n.name FROM supplier s, nation n WHERE s.nationkey = n.nationkey")));
		unlink ("supplier.bin");
		unlink ("nation.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 13:
	{
		// a big input in many runs with the default buffer: supplierBig (32
		// copies of every supplier) joined with supplier on the key, for keys up to 50
		cout << "TEST 13..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr big = loadTable (mgr, "supplierBig", supplierSchema ("a_"), "supplierBig.tbl");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("b_"), "supplier.tbl");
		JoinSpec spec {"== ([a_suppkey], [b_suppkey])",
			vector <string> {"[a_suppkey]", "[b_name]"},
			make_pair (string ("[a_suppkey]"), string ("[b_suppkey]")),
			"! (> ([a_suppkey], int[50]))", "! (> ([b_suppkey], int[50]))"};
		vector <string> rows = runSortMergeJoin (mgr, big, supp, spec);
		vector <string> scan = runScanJoin (mgr, big, supp, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 1600);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT a.suppkey, b.name FROM supplierBig a, supplier b WHERE a.suppkey = b.suppkey AND a.suppkey <= 50 AND b.suppkey <= 50")));
		unlink ("supplierBig.bin");
		unlink ("supplier.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 14:
	{
		// the left selection alone removes some matches: only suppliers with a
		// positive balance
		cout << "TEST 14..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		MyDB_TableReaderWriterPtr nat = loadTable (mgr, "nation", nationSchema ("n_"), "nation.tbl");
		JoinSpec spec {"== ([s_nationkey], [n_nationkey])",
			vector <string> {"[s_suppkey]", "[n_name]"},
			make_pair (string ("[s_nationkey]"), string ("[n_nationkey]")),
			"> ([s_acctbal], double[0.0])", "bool[true]"};
		vector <string> rows = runSortMergeJoin (mgr, supp, nat, spec);
		vector <string> scan = runScanJoin (mgr, supp, nat, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0 && rows.size () < 10000);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT s.suppkey, n.name FROM supplier s, nation n WHERE s.nationkey = n.nationkey AND s.acctbal > 0.0")));
		unlink ("supplier.bin");
		unlink ("nation.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 15:
	{
		// the right selection alone removes some matches: nations outside
		// region 1
		cout << "TEST 15..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		MyDB_TableReaderWriterPtr nat = loadTable (mgr, "nation", nationSchema ("n_"), "nation.tbl");
		JoinSpec spec {"== ([s_nationkey], [n_nationkey])",
			vector <string> {"[s_suppkey]", "[n_name]"},
			make_pair (string ("[s_nationkey]"), string ("[n_nationkey]")),
			"bool[true]", "! (== ([n_regionkey], int[1]))"};
		vector <string> rows = runSortMergeJoin (mgr, supp, nat, spec);
		vector <string> scan = runScanJoin (mgr, supp, nat, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0 && rows.size () < 10000);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT s.suppkey, n.name FROM supplier s, nation n WHERE s.nationkey = n.nationkey AND NOT n.regionkey = 1")));
		unlink ("supplier.bin");
		unlink ("nation.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 16:
	{
		// arithmetic in the join: each supplier joins the nation whose key is
		// one more than its own
		cout << "TEST 16..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		MyDB_TableReaderWriterPtr nat = loadTable (mgr, "nation", nationSchema ("n_"), "nation.tbl");
		JoinSpec spec {"== (+ ([s_nationkey], int[1]), [n_nationkey])",
			vector <string> {"[s_suppkey]", "[n_nationkey]"},
			make_pair (string ("+ ([s_nationkey], int[1])"), string ("[n_nationkey]")),
			"bool[true]", "bool[true]"};
		vector <string> rows = runSortMergeJoin (mgr, supp, nat, spec);
		vector <string> scan = runScanJoin (mgr, supp, nat, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT s.suppkey, n.nationkey FROM supplier s, nation n WHERE s.nationkey + 1 = n.nationkey")));
		unlink ("supplier.bin");
		unlink ("nation.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 17:
	{
		// the right input runs out first: only nations 0 to 4 on the right
		cout << "TEST 17..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		MyDB_TableReaderWriterPtr nat = loadTable (mgr, "nation", nationSchema ("n_"), "nation.tbl");
		JoinSpec spec {"== ([s_nationkey], [n_nationkey])",
			vector <string> {"[s_suppkey]", "[n_name]"},
			make_pair (string ("[s_nationkey]"), string ("[n_nationkey]")),
			"bool[true]", "< ([n_nationkey], int[5])"};
		vector <string> rows = runSortMergeJoin (mgr, supp, nat, spec);
		vector <string> scan = runScanJoin (mgr, supp, nat, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT s.suppkey, n.name FROM supplier s, nation n WHERE s.nationkey = n.nationkey AND n.nationkey < 5")));
		unlink ("supplier.bin");
		unlink ("nation.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 18:
	{
		// the left input runs out first: only nations 0 to 4 on the left
		cout << "TEST 18..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr nat = loadTable (mgr, "nation", nationSchema ("n_"), "nation.tbl");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		JoinSpec spec {"== ([n_nationkey], [s_nationkey])",
			vector <string> {"[n_name]", "[s_suppkey]"},
			make_pair (string ("[n_nationkey]"), string ("[s_nationkey]")),
			"< ([n_nationkey], int[5])", "bool[true]"};
		vector <string> rows = runSortMergeJoin (mgr, nat, supp, spec);
		vector <string> scan = runScanJoin (mgr, nat, supp, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT n.name, s.suppkey FROM nation n, supplier s WHERE n.nationkey = s.nationkey AND n.nationkey < 5")));
		unlink ("nation.bin");
		unlink ("supplier.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 19:
	{
		// a double join key: suppliers 1 to 5000 and 5001 to 10000 with equal
		// balances
		cout << "TEST 19..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr suppA = loadTable (mgr, "supplierA", supplierSchema ("a_"), "supplier.tbl");
		MyDB_TableReaderWriterPtr suppB = loadTable (mgr, "supplierB", supplierSchema ("b_"), "supplier.tbl");
		JoinSpec spec {"== ([a_acctbal], [b_acctbal])",
			vector <string> {"[a_suppkey]", "[b_suppkey]", "[a_acctbal]"},
			make_pair (string ("[a_acctbal]"), string ("[b_acctbal]")),
			"! (> ([a_suppkey], int[5000]))", "> ([b_suppkey], int[5000])"};
		vector <string> rows = runSortMergeJoin (mgr, suppA, suppB, spec);
		vector <string> scan = runScanJoin (mgr, suppA, suppB, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT a.suppkey, b.suppkey, a.acctbal FROM supplier a, supplier b WHERE a.acctbal = b.acctbal AND a.suppkey <= 5000 AND b.suppkey > 5000")));
		unlink ("supplierA.bin");
		unlink ("supplierB.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 20:
	{
		// a join predicate with a condition beyond key equality that uses both
		// sides: suppkey < 2000 * regionkey
		cout << "TEST 20..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		MyDB_TableReaderWriterPtr nat = loadTable (mgr, "nation", nationSchema ("n_"), "nation.tbl");
		JoinSpec spec {"&& (== ([s_nationkey], [n_nationkey]), < ([s_suppkey], * ([n_regionkey], int[2000])))",
			vector <string> {"[s_suppkey]", "[n_name]"},
			make_pair (string ("[s_nationkey]"), string ("[n_nationkey]")),
			"bool[true]", "bool[true]"};
		vector <string> rows = runSortMergeJoin (mgr, supp, nat, spec);
		vector <string> scan = runScanJoin (mgr, supp, nat, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0 && rows.size () < 10000);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT s.suppkey, n.name FROM supplier s, nation n WHERE s.nationkey = n.nationkey AND s.suppkey < n.regionkey * 2000")));
		unlink ("supplier.bin");
		unlink ("nation.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 21:
	{
		// the extra condition rejects some pairs inside groups that repeat on
		// both sides: suppliers 1 to 300 in the same nation, with a.acctbal < b.acctbal
		cout << "TEST 21..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr suppA = loadTable (mgr, "supplierA", supplierSchema ("a_"), "supplier.tbl");
		MyDB_TableReaderWriterPtr suppB = loadTable (mgr, "supplierB", supplierSchema ("b_"), "supplier.tbl");
		JoinSpec spec {"&& (== ([a_nationkey], [b_nationkey]), < ([a_acctbal], [b_acctbal]))",
			vector <string> {"[a_suppkey]", "[b_suppkey]"},
			make_pair (string ("[a_nationkey]"), string ("[b_nationkey]")),
			"! (> ([a_suppkey], int[300]))", "! (> ([b_suppkey], int[300]))"};
		vector <string> rows = runSortMergeJoin (mgr, suppA, suppB, spec);
		vector <string> scan = runScanJoin (mgr, suppA, suppB, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT a.suppkey, b.suppkey FROM supplier a, supplier b WHERE a.nationkey = b.nationkey AND a.suppkey <= 300 AND b.suppkey <= 300 AND a.acctbal < b.acctbal")));
		unlink ("supplierA.bin");
		unlink ("supplierB.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 22:
	{
		// key groups that appear on only one side, between groups that match:
		// suppliers 1 to 300 against the nations of region 2
		cout << "TEST 22..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		MyDB_TableReaderWriterPtr nat = loadTable (mgr, "nation", nationSchema ("n_"), "nation.tbl");
		JoinSpec spec {"== ([s_nationkey], [n_nationkey])",
			vector <string> {"[s_suppkey]", "[n_name]"},
			make_pair (string ("[s_nationkey]"), string ("[n_nationkey]")),
			"! (> ([s_suppkey], int[300]))", "== ([n_regionkey], int[2])"};
		vector <string> rows = runSortMergeJoin (mgr, supp, nat, spec);
		vector <string> scan = runScanJoin (mgr, supp, nat, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT s.suppkey, n.name FROM supplier s, nation n WHERE s.nationkey = n.nationkey AND s.suppkey <= 300 AND n.regionkey = 2")));
		unlink ("supplier.bin");
		unlink ("nation.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 23:
	{
		// a key group too big for one page: the 412 * 32 copies of the nation-3
		// suppliers in supplierBig, all matching one nation
		cout << "TEST 23..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr big = loadTable (mgr, "supplierBig", supplierSchema ("s_"), "supplierBig.tbl");
		MyDB_TableReaderWriterPtr nat = loadTable (mgr, "nation", nationSchema ("n_"), "nation.tbl");
		JoinSpec spec {"== ([s_nationkey], [n_nationkey])",
			vector <string> {"[s_suppkey]", "[n_name]"},
			make_pair (string ("[s_nationkey]"), string ("[n_nationkey]")),
			"bool[true]", "== ([n_nationkey], int[3])"};
		vector <string> rows = runSortMergeJoin (mgr, big, nat, spec);
		vector <string> scan = runScanJoin (mgr, big, nat, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 13184);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT s.suppkey, n.name FROM supplierBig s, nation n WHERE s.nationkey = n.nationkey AND n.nationkey = 3")));
		unlink ("supplierBig.bin");
		unlink ("nation.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 24:
	{
		// a self-join, with the same table object as both inputs: likes with
		// itself on the beer.  By hand: PBR gives Homer and Marge twice each
		// (2 x 2), Coors gives Homer, Duff gives Lenny
		cout << "TEST 24..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr likes = loadTable (mgr, "likes", twoStrings ("l_dr", "l_beer"), "likes.tbl");
		JoinSpec spec {"== ([l_beer], [l_beer])",
			vector <string> {"[l_dr]"},
			make_pair (string ("[l_beer]"), string ("[l_beer]")),
			"bool[true]", "bool[true]"};
		vector <string> rows = runSortMergeJoin (mgr, likes, likes, spec);
		vector <string> scan = runScanJoin (mgr, likes, likes, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows == (vector <string> {"Homer|", "Homer|", "Homer|", "Lenny|", "Marge|", "Marge|"}));
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT a.dr FROM likes a, likes b WHERE a.beer = b.beer")));
		unlink ("likes.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 25:
	{
		// projections from both sides, reordered and computed: the nation
		// name, the supplier key, and acctbal + 1
		cout << "TEST 25..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		MyDB_TableReaderWriterPtr nat = loadTable (mgr, "nation", nationSchema ("n_"), "nation.tbl");
		JoinSpec spec {"== ([s_nationkey], [n_nationkey])",
			vector <string> {"[n_name]", "[s_suppkey]", "+ ([s_acctbal], double[1.0])"},
			make_pair (string ("[s_nationkey]"), string ("[n_nationkey]")),
			"bool[true]", "bool[true]"};
		vector <string> rows = runSortMergeJoin (mgr, supp, nat, spec);
		vector <string> scan = runScanJoin (mgr, supp, nat, spec);
		cout << rows.size () << " rows (ScanJoin: " << scan.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 10000);
		QUNIT_IS_TRUE (sameRows (rows, scan));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT n.name, s.suppkey, s.acctbal + 1.0 FROM supplier s, nation n WHERE s.nationkey = n.nationkey")));
		unlink ("supplier.bin");
		unlink ("nation.bin");
		unlink ("smjOut.bin");
		unlink ("scanOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	default:
		break;
	}

	unlink ("tempFile");
	return qunit.errors ();
}

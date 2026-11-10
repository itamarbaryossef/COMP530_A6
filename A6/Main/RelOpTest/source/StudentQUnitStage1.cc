/****************************************************
** COMP 530 A6 -- Stage 1 tests: RegularSelection   **
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
**   studentUnitTestStage1          run all        **
**   studentUnitTestStage1 N        start from N   **
**   studentUnitTestStage1 N only   run N and stop **
****************************************************/

#include "Aggregate.h"
#include "BPlusSelection.h"
#include "QUnit.h"
#include "RegularSelection.h"
#include "ScanJoin.h"
#include "SortMergeJoin.h"
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
		// an example of the format: the beer drinkers who like PBR.  The
		// answer is known by hand (Homer and Marge) and from the oracle
		cout << "TEST 1..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
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
		unlink ("likes.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 2:
	{
		// an empty input table: whatever the predicate, nothing comes out.
		// Known by hand (the input has no rows)
		cout << "TEST 2..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "emptySupplier", supplierSchema ("s_"));
		vector <string> rows = selectSuppliers (mgr, supp, "bool[true]");
		cout << rows.size () << " rows" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 0);
		unlink ("emptySupplier.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 3:
	{
		// a predicate no row satisfies (every acctbal is below 10000)
		cout << "TEST 3..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = selectSuppliers (mgr, supp, "> ([s_acctbal], double[10000.0])");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE acctbal > 10000.0");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 0);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 4:
	{
		// a predicate every row satisfies (every acctbal is above -1000): the
		// output is a copy of all 10000 suppliers
		cout << "TEST 4..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = selectSuppliers (mgr, supp, "> ([s_acctbal], double[-1000.0])");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE acctbal > -1000.0");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 10000);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 5:
	{
		// a predicate exactly one row satisfies (suppkey is a key)
		cout << "TEST 5..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = selectSuppliers (mgr, supp, "== ([s_suppkey], int[4242])");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE suppkey = 4242");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 1);
		QUNIT_IS_TRUE (rows.size () == 1 && rows[0].substr (0, 5) == "4242|");
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 6:
	{
		// a predicate many rows satisfy: the suppliers in nation 3
		cout << "TEST 6..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = selectSuppliers (mgr, supp, "== ([s_nationkey], int[3])");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE nationkey = 3");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 1);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 7:
	{
		// an OR: a row passes if it is in nation 3 OR has a negative balance;
		// most rows that pass satisfy only one side, so the result is bigger
		// than either side alone
		cout << "TEST 7..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = selectSuppliers (mgr, supp, "|| (== ([s_nationkey], int[3]), < ([s_acctbal], double[0.0]))");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE nationkey = 3 OR acctbal < 0.0");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		int left = (int) oracle ("SELECT * FROM supplier WHERE nationkey = 3").size ();
		int right = (int) oracle ("SELECT * FROM supplier WHERE acctbal < 0.0").size ();
		QUNIT_IS_TRUE ((int) rows.size () > left);
		QUNIT_IS_TRUE ((int) rows.size () > right);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 8:
	{
		// arithmetic in the predicate: 2 * acctbal > 19000
		cout << "TEST 8..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = selectSuppliers (mgr, supp, "> (* ([s_acctbal], double[2.0]), double[19000.0])");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE acctbal * 2.0 > 19000.0");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 9:
	{
		// an AND: in nation 3 AND a negative balance.  Most nation-3 rows
		// fail because only the balance check is false, so the result is
		// smaller than the nation-3 rows alone
		cout << "TEST 9..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = selectSuppliers (mgr, supp, "&& (== ([s_nationkey], int[3]), < ([s_acctbal], double[0.0]))");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE nationkey = 3 AND acctbal < 0.0");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		int left = (int) oracle ("SELECT * FROM supplier WHERE nationkey = 3").size ();
		QUNIT_IS_TRUE (rows.size () > 0);
		QUNIT_IS_TRUE ((int) rows.size () < left);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 10:
	{
		// a NOT: every supplier outside nation 3
		cout << "TEST 10..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = selectSuppliers (mgr, supp, "! (== ([s_nationkey], int[3]))");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE NOT nationkey = 3");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 11:
	{
		// a string comparison: names before Supplier#000000010 (they are
		// zero padded, so these are suppliers 1 through 9)
		cout << "TEST 11..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = selectSuppliers (mgr, supp, "< ([s_name], string[Supplier#000000010])");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE name < 'Supplier#000000010'");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 9);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 12:
	{
		// computed projections, in a different order than the input and
		// fewer of them: name, 2 * acctbal, suppkey + 1 for nation 3
		cout << "TEST 12..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		MyDB_SchemaPtr outSchema = make_shared <MyDB_Schema> ();
		outSchema->appendAtt (make_pair ("o_name", make_shared <MyDB_StringAttType> ()));
		outSchema->appendAtt (make_pair ("o_doublebal", make_shared <MyDB_DoubleAttType> ()));
		outSchema->appendAtt (make_pair ("o_nextkey", make_shared <MyDB_IntAttType> ()));
		vector <string> projections {"[s_name]", "* ([s_acctbal], double[2.0])", "+ ([s_suppkey], int[1])"};
		vector <string> rows = runSelection (mgr, supp, outSchema, "== ([s_nationkey], int[3])", projections);
		vector <string> expected = oracle ("SELECT name, acctbal * 2.0, suppkey + 1 FROM supplier WHERE nationkey = 3");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 13:
	{
		// a large input (320000 rows, many more pages than the buffer holds):
		// the suppliers in nation 7, each of which appears 32 times
		cout << "TEST 13..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplierBig", supplierSchema ("s_"), "supplierBig.tbl");
		vector <string> rows = selectSuppliers (mgr, supp, "== ([s_nationkey], int[7])");
		vector <string> expected = oracle ("SELECT * FROM supplierBig WHERE nationkey = 7");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplierBig.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 14:
	{
		// predicates that mix ints and doubles: (a) a double column plus an
		// int column, compared to an int literal; (b) an int column times a
		// double literal, compared to a double column
		cout << "TEST 14..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rowsA = selectSuppliers (mgr, supp, "> (+ ([s_acctbal], [s_nationkey]), int[9990])", "selOutA");
		vector <string> expectedA = oracle ("SELECT * FROM supplier WHERE acctbal + nationkey > 9990");
		vector <string> rowsB = selectSuppliers (mgr, supp, "> (* ([s_nationkey], double[400.5]), [s_acctbal])", "selOutB");
		vector <string> expectedB = oracle ("SELECT * FROM supplier WHERE nationkey * 400.5 > acctbal");
		cout << rowsA.size () << " and " << rowsB.size () << " rows (expected " << expectedA.size ()
			<< " and " << expectedB.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rowsA.size () > 0);
		QUNIT_IS_TRUE (rowsB.size () > 0);
		QUNIT_IS_TRUE (sameRows (rowsA, expectedA));
		QUNIT_IS_TRUE (sameRows (rowsB, expectedB));
		unlink ("supplier.bin");
		unlink ("selOutA.bin");
		unlink ("selOutB.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 15:
	{
		// an AND nested inside an OR: (nation 3 and acctbal > 9000) or
		// (nation 5 and acctbal < 0)
		cout << "TEST 15..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = selectSuppliers (mgr, supp, "|| (&& (== ([s_nationkey], int[3]), > ([s_acctbal], double[9000.0])), && (== ([s_nationkey], int[5]), < ([s_acctbal], double[0.0])))");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE (nationkey = 3 AND acctbal > 9000.0) OR (nationkey = 5 AND acctbal < 0.0)");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 16:
	{
		// unary minus: -acctbal > 900, that is, acctbal < -900
		cout << "TEST 16..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = selectSuppliers (mgr, supp, "> (um ([s_acctbal]), double[900.0])");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE -acctbal > 900.0");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (rows.size () > 0);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 17:
	{
		// equality on a double: supplier 3 has acctbal 4192.40 (by hand,
		// from supplier.tbl), and no other supplier does
		cout << "TEST 17..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = selectSuppliers (mgr, supp, "== ([s_acctbal], double[4192.40])");
		vector <string> expected = oracle ("SELECT * FROM supplier WHERE acctbal = 4192.40");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 1);
		QUNIT_IS_TRUE (rows.size () == 1 && rows[0].substr (0, 2) == "3|");
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("selOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 18:
	{
		// a second data set, and a string literal with an apostrophe: who
		// goes to Moe's?  Homer and Lenny (by hand, from freq.tbl)
		cout << "TEST 18..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr freq = loadTable (mgr, "freq", twoStrings ("f_dr", "f_bar"), "freq.tbl");
		MyDB_SchemaPtr outSchema = make_shared <MyDB_Schema> ();
		outSchema->appendAtt (make_pair ("dr", make_shared <MyDB_StringAttType> ()));
		vector <string> rows = runSelection (mgr, freq, outSchema, "== ([f_bar], string[Moe's])", vector <string> {"[f_dr]"});
		cout << rows.size () << " drinkers go to Moe's" << endl << flush;
		QUNIT_IS_TRUE (rows == (vector <string> {"Homer|", "Lenny|"}));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT dr FROM freq WHERE bar = 'Moe''s'")));
		unlink ("freq.bin");
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

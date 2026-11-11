/****************************************************
** COMP 530 A6 -- Stage 4 tests: Aggregate         **
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
**   studentUnitTestStage4          run all        **
**   studentUnitTestStage4 N        start from N   **
**   studentUnitTestStage4 N only   run N and stop **
****************************************************/

#include "Aggregate.h"
#include "BPlusSelection.h"
#include "QUnit.h"
#include "RegularSelection.h"
#include "ScanJoin.h"
#include "SortMergeJoin.h"
#include "AggTestHelpers.h"
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
		// an example of the format: how many people like each beer (PBR 2,
		// Coors 1, Duff 1, by hand and by the oracle)
		cout << "TEST 1..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
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
		unlink ("likes.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 2:
	{
		// group by one attribute, with all three aggregates: the balance per
		// nation.  25 nations; the counts add up to the 10000 suppliers
		cout << "TEST 2..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = runAggregate (mgr, supp, aggOutSchema ({intType (), doubleType (), doubleType (), intType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: sum, string ("[s_acctbal]")), make_pair (MyDB_AggType :: avg, string ("[s_acctbal]")), make_pair (MyDB_AggType :: cnt, string ("int[0]"))},
			vector <string> {"[s_nationkey]"}, "bool[true]");
		vector <string> expected = oracle ("SELECT nationkey, SUM(acctbal), AVG(acctbal), COUNT(*) FROM supplier GROUP BY nationkey");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 25);
		QUNIT_IS_TRUE (fabs (fieldTotal (rows, 3) - 10000) < 0.5);
		QUNIT_IS_TRUE (avgTimesCountIsSum (rows, 1, 2, 3));
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 3:
	{
		// group by three attributes (nation, suppkey / 2500, and suppkey mod 2),
		// with all three aggregates
		cout << "TEST 3..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = runAggregate (mgr, supp, aggOutSchema ({intType (), intType (), intType (), doubleType (), doubleType (), intType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: sum, string ("[s_acctbal]")), make_pair (MyDB_AggType :: avg, string ("[s_acctbal]")), make_pair (MyDB_AggType :: cnt, string ("int[0]"))},
			vector <string> {"[s_nationkey]", "/ ([s_suppkey], int[2500])", "- ([s_suppkey], * (/ ([s_suppkey], int[2]), int[2]))"}, "bool[true]");
		vector <string> expected = oracle ("SELECT nationkey, suppkey / 2500, suppkey - (suppkey / 2) * 2, SUM(acctbal), AVG(acctbal), COUNT(*) FROM supplier GROUP BY 1, 2, 3");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), atoi (oracleOne ("SELECT COUNT(*) FROM (SELECT DISTINCT nationkey, suppkey / 2500, suppkey - (suppkey / 2) * 2 FROM supplier)").c_str ()));
		QUNIT_IS_TRUE (fabs (fieldTotal (rows, 5) - 10000) < 0.5);
		QUNIT_IS_TRUE (avgTimesCountIsSum (rows, 3, 4, 5));
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 4:
	{
		// group by two attributes, one an arithmetic expression (suppkey / 1000).
		// Many of these groups have equal XOR-combined hashes, so groups must
		// be told apart by value
		cout << "TEST 4..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = runAggregate (mgr, supp, aggOutSchema ({intType (), intType (), doubleType (), doubleType (), intType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: sum, string ("[s_acctbal]")), make_pair (MyDB_AggType :: avg, string ("[s_acctbal]")), make_pair (MyDB_AggType :: cnt, string ("int[0]"))},
			vector <string> {"[s_nationkey]", "/ ([s_suppkey], int[1000])"}, "bool[true]");
		vector <string> expected = oracle ("SELECT nationkey, suppkey / 1000, SUM(acctbal), AVG(acctbal), COUNT(*) FROM supplier GROUP BY 1, 2");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), atoi (oracleOne ("SELECT COUNT(*) FROM (SELECT DISTINCT nationkey, suppkey / 1000 FROM supplier)").c_str ()));
		QUNIT_IS_TRUE (fabs (fieldTotal (rows, 4) - 10000) < 0.5);
		QUNIT_IS_TRUE (avgTimesCountIsSum (rows, 2, 3, 4));
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 5:
	{
		// every row fails the selection (no balance is over 20000): (a) with a
		// GROUP BY, the number of groups must match the oracle's number of
		// rows; (b) with no GROUP BY, there must be no
		// rows, or one row whose COUNT is 0 (MyDB has no NULL)
		cout << "TEST 5..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rowsA = runAggregate (mgr, supp, aggOutSchema ({intType (), doubleType (), doubleType (), intType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: sum, string ("[s_acctbal]")), make_pair (MyDB_AggType :: avg, string ("[s_acctbal]")), make_pair (MyDB_AggType :: cnt, string ("int[0]"))},
			vector <string> {"[s_nationkey]"}, "> ([s_acctbal], double[20000.0])");
		vector <string> rowsB = runAggregate (mgr, supp, aggOutSchema ({doubleType (), doubleType (), intType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: sum, string ("[s_acctbal]")), make_pair (MyDB_AggType :: avg, string ("[s_acctbal]")), make_pair (MyDB_AggType :: cnt, string ("int[0]"))},
			vector <string> {}, "> ([s_acctbal], double[20000.0])", "aggOutB");
		cout << rowsA.size () << " and " << rowsB.size () << " rows" << endl << flush;
		vector <string> expectedA = oracle ("SELECT nationkey, SUM(acctbal), AVG(acctbal), COUNT(*) FROM supplier WHERE acctbal > 20000.0 GROUP BY nationkey");
		QUNIT_IS_EQUAL ((int) rowsA.size (), (int) expectedA.size ());
		QUNIT_IS_TRUE (emptyAggregateOK (rowsB, 2));
		unlink ("aggOutB.bin");
		unlink ("supplier.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 6:
	{
		// AVG with decimal values: the average suppkey (an int) per nation, into
		// a double; nation 0 averages 5176.816667, so it must not be truncated
		cout << "TEST 6..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = runAggregate (mgr, supp, aggOutSchema ({intType (), doubleType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: avg, string ("[s_suppkey]"))},
			vector <string> {"[s_nationkey]"}, "bool[true]");
		vector <string> expected = oracle ("SELECT nationkey, AVG(suppkey) FROM supplier GROUP BY nationkey");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 25);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 7:
	{
		// group by a string and a number: supplierBig has 32 copies of every
		// supplier, so each (name, nation) group of suppliers 1 to 100 counts 32
		cout << "TEST 7..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr big = loadTable (mgr, "supplierBig", supplierSchema ("s_"), "supplierBig.tbl");
		vector <string> rows = runAggregate (mgr, big, aggOutSchema ({stringType (), intType (), doubleType (), doubleType (), intType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: sum, string ("[s_acctbal]")), make_pair (MyDB_AggType :: avg, string ("[s_acctbal]")), make_pair (MyDB_AggType :: cnt, string ("int[0]"))},
			vector <string> {"[s_name]", "[s_nationkey]"}, "! (> ([s_suppkey], int[100]))");
		vector <string> expected = oracle ("SELECT name, nationkey, SUM(acctbal), AVG(acctbal), COUNT(*) FROM supplierBig WHERE suppkey <= 100 GROUP BY name, nationkey");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 100);
		bool all32 = true;
		for (auto &r : rows)
			if (fieldOf (r, 4) != "32")
				all32 = false;
		QUNIT_IS_TRUE (all32);
		QUNIT_IS_TRUE (avgTimesCountIsSum (rows, 2, 3, 4));
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplierBig.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 8:
	{
		// no GROUP BY, with all three aggregates over all of supplier; also
		// checked against a plain loop over the input
		cout << "TEST 8..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = runAggregate (mgr, supp, aggOutSchema ({doubleType (), doubleType (), intType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: sum, string ("[s_acctbal]")), make_pair (MyDB_AggType :: avg, string ("[s_acctbal]")), make_pair (MyDB_AggType :: cnt, string ("int[0]"))},
			vector <string> {}, "bool[true]");
		vector <string> expected = oracle ("SELECT SUM(acctbal), AVG(acctbal), COUNT(*) FROM supplier");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 1);
		double loopSum = 0;
		int loopCnt = 0;
		MyDB_RecordPtr rec = supp->getEmptyRecord ();
		MyDB_RecordIteratorAltPtr iter = supp->getIteratorAlt ();
		while (iter->advance ()) {
			iter->getCurrent (rec);
			loopSum += rec->getAtt (5)->toDouble ();
			loopCnt++;
		}
		QUNIT_IS_TRUE (rows.size () == 1 && fabs (numOf (rows[0], 0) - loopSum) < 1e-6 * fabs (loopSum));
		QUNIT_IS_TRUE (rows.size () == 1 && fabs (numOf (rows[0], 1) - loopSum / loopCnt) < 1e-6 * fabs (loopSum / loopCnt));
		QUNIT_IS_TRUE (rows.size () == 1 && atoi (fieldOf (rows[0], 2).c_str ()) == loopCnt);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 9:
	{
		// an empty input table: (a) with a GROUP BY there are no groups; (b) with
		// no GROUP BY, there must be no rows, or one row whose COUNT is 0
		cout << "TEST 9..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "emptySupplier", supplierSchema ("s_"));
		vector <string> rowsA = runAggregate (mgr, supp, aggOutSchema ({intType (), doubleType (), doubleType (), intType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: sum, string ("[s_acctbal]")), make_pair (MyDB_AggType :: avg, string ("[s_acctbal]")), make_pair (MyDB_AggType :: cnt, string ("int[0]"))},
			vector <string> {"[s_nationkey]"}, "bool[true]");
		vector <string> rowsB = runAggregate (mgr, supp, aggOutSchema ({doubleType (), doubleType (), intType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: sum, string ("[s_acctbal]")), make_pair (MyDB_AggType :: avg, string ("[s_acctbal]")), make_pair (MyDB_AggType :: cnt, string ("int[0]"))},
			vector <string> {}, "bool[true]", "aggOutB");
		cout << rowsA.size () << " and " << rowsB.size () << " rows" << endl << flush;
		QUNIT_IS_EQUAL ((int) rowsA.size (), 0);
		QUNIT_IS_TRUE (emptyAggregateOK (rowsB, 2));
		unlink ("aggOutB.bin");
		unlink ("emptySupplier.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 10:
	{
		// group by a double (its hash is truncated to an int, so balances like
		// 4192.40 and 4192.10 hash alike): suppliers 1 to 2000
		cout << "TEST 10..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = runAggregate (mgr, supp, aggOutSchema ({doubleType (), intType (), intType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: cnt, string ("int[0]")), make_pair (MyDB_AggType :: sum, string ("[s_suppkey]"))},
			vector <string> {"[s_acctbal]"}, "! (> ([s_suppkey], int[2000]))");
		vector <string> expected = oracle ("SELECT acctbal, COUNT(*), SUM(suppkey) FROM supplier WHERE suppkey <= 2000 GROUP BY acctbal");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), atoi (oracleOne ("SELECT COUNT(DISTINCT acctbal) FROM supplier WHERE suppkey <= 2000").c_str ()));
		QUNIT_IS_TRUE (fabs (fieldTotal (rows, 1) - 2000) < 0.5);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 11:
	{
		// several aggregates in mixed order, with repeated types: each must land
		// in its own column
		cout << "TEST 11..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = runAggregate (mgr, supp, aggOutSchema ({intType (), intType (), doubleType (), doubleType (), intType (), intType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: cnt, string ("int[0]")), make_pair (MyDB_AggType :: sum, string ("[s_acctbal]")), make_pair (MyDB_AggType :: avg, string ("[s_suppkey]")), make_pair (MyDB_AggType :: sum, string ("+ ([s_suppkey], [s_nationkey])")), make_pair (MyDB_AggType :: cnt, string ("int[0]"))},
			vector <string> {"[s_nationkey]"}, "bool[true]");
		vector <string> expected = oracle ("SELECT nationkey, COUNT(*), SUM(acctbal), AVG(suppkey), SUM(suppkey + nationkey), COUNT(*) FROM supplier GROUP BY nationkey");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 25);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 12:
	{
		// aggregates of arithmetic expressions: SUM(acctbal * 2) and
		// AVG(suppkey - nationkey) per nation
		cout << "TEST 12..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = runAggregate (mgr, supp, aggOutSchema ({intType (), doubleType (), doubleType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: sum, string ("* ([s_acctbal], double[2.0])")), make_pair (MyDB_AggType :: avg, string ("- ([s_suppkey], [s_nationkey])"))},
			vector <string> {"[s_nationkey]"}, "bool[true]");
		vector <string> expected = oracle ("SELECT nationkey, SUM(acctbal * 2.0), AVG(suppkey - nationkey) FROM supplier GROUP BY nationkey");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 25);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 13:
	{
		// the selection removes some rows: only positive balances, per nation
		cout << "TEST 13..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = runAggregate (mgr, supp, aggOutSchema ({intType (), doubleType (), doubleType (), intType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: sum, string ("[s_acctbal]")), make_pair (MyDB_AggType :: avg, string ("[s_acctbal]")), make_pair (MyDB_AggType :: cnt, string ("int[0]"))},
			vector <string> {"[s_nationkey]"}, "> ([s_acctbal], double[0.0])");
		vector <string> expected = oracle ("SELECT nationkey, SUM(acctbal), AVG(acctbal), COUNT(*) FROM supplier WHERE acctbal > 0.0 GROUP BY nationkey");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_TRUE (fabs (fieldTotal (rows, 3) - atoi (oracleOne ("SELECT COUNT(*) FROM supplier WHERE acctbal > 0.0").c_str ())) < 0.5);
		QUNIT_IS_TRUE (avgTimesCountIsSum (rows, 1, 2, 3));
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 14:
	{
		// a large input (320000 rows of supplierBig) into the 25 nations
		cout << "TEST 14..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr big = loadTable (mgr, "supplierBig", supplierSchema ("s_"), "supplierBig.tbl");
		vector <string> rows = runAggregate (mgr, big, aggOutSchema ({intType (), doubleType (), doubleType (), intType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: sum, string ("[s_acctbal]")), make_pair (MyDB_AggType :: avg, string ("[s_acctbal]")), make_pair (MyDB_AggType :: cnt, string ("int[0]"))},
			vector <string> {"[s_nationkey]"}, "bool[true]");
		vector <string> expected = oracle ("SELECT nationkey, SUM(acctbal), AVG(acctbal), COUNT(*) FROM supplierBig GROUP BY nationkey");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 25);
		QUNIT_IS_TRUE (fabs (fieldTotal (rows, 3) - 320000) < 0.5);
		QUNIT_IS_TRUE (avgTimesCountIsSum (rows, 1, 2, 3));
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplierBig.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 15:
	{
		// many groups: one per supplier (10000), each with COUNT 1
		cout << "TEST 15..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = runAggregate (mgr, supp, aggOutSchema ({intType (), intType (), doubleType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: cnt, string ("int[0]")), make_pair (MyDB_AggType :: sum, string ("[s_acctbal]"))},
			vector <string> {"[s_suppkey]"}, "bool[true]");
		vector <string> expected = oracle ("SELECT suppkey, COUNT(*), SUM(acctbal) FROM supplier GROUP BY suppkey");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 10000);
		QUNIT_IS_TRUE (fabs (fieldTotal (rows, 1) - 10000) < 0.5);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 16:
	{
		// a SUM of ints into an int column: the sum of suppkey per nation
		cout << "TEST 16..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr supp = loadTable (mgr, "supplier", supplierSchema ("s_"), "supplier.tbl");
		vector <string> rows = runAggregate (mgr, supp, aggOutSchema ({intType (), intType (), intType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: sum, string ("[s_suppkey]")), make_pair (MyDB_AggType :: cnt, string ("int[0]"))},
			vector <string> {"[s_nationkey]"}, "bool[true]");
		vector <string> expected = oracle ("SELECT nationkey, SUM(suppkey), COUNT(*) FROM supplier GROUP BY nationkey");
		cout << rows.size () << " rows (expected " << expected.size () << ")" << endl << flush;
		QUNIT_IS_EQUAL ((int) rows.size (), 25);
		QUNIT_IS_TRUE (sameRows (rows, expected));
		unlink ("supplier.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	case 17:
	{
		// string groups, checkable by hand: how many drinkers go to each bar
		// (Hilltop 2, Moe's 2)
		cout << "TEST 17..." << flush;
		MyDB_BufferManagerPtr mgr = make_shared <MyDB_BufferManager> (131072, 128, "tempFile");
		MyDB_TableReaderWriterPtr freq = loadTable (mgr, "freq", twoStrings ("f_dr", "f_bar"), "freq.tbl");
		vector <string> rows = runAggregate (mgr, freq, aggOutSchema ({stringType (), intType ()}),
			vector <pair <MyDB_AggType, string>> {make_pair (MyDB_AggType :: cnt, string ("int[0]"))},
			vector <string> {"[f_bar]"}, "bool[true]");
		cout << rows.size () << " bars" << endl << flush;
		QUNIT_IS_TRUE (rows == (vector <string> {"Hilltop|2|", "Moe's|2|"}));
		QUNIT_IS_TRUE (sameRows (rows, oracle ("SELECT bar, COUNT(*) FROM freq GROUP BY bar")));
		unlink ("freq.bin");
		unlink ("aggOut.bin");
	}
	FALLTHROUGH_INTENDED;
	if (runOne) break;

	default:
		break;
	}

	unlink ("tempFile");
	return qunit.errors ();
}

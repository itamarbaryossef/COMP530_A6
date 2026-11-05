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

	default:
		break;
	}

	unlink ("tempFile");
	return qunit.errors ();
}

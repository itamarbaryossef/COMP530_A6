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

	default:
		break;
	}

	unlink ("tempFile");
	return qunit.errors ();
}

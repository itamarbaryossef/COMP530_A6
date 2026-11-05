
#ifndef BPLUS_SELECTION_C
#define BPLUS_SELECTION_C

#include "BPlusSelection.h"

BPlusSelection :: BPlusSelection (MyDB_BPlusTreeReaderWriterPtr inputIn, MyDB_TableReaderWriterPtr outputIn,
		MyDB_AttValPtr lowIn, MyDB_AttValPtr highIn,
		string selectionPredicateIn, vector <string> projectionsIn) {
	input = inputIn;
	output = outputIn;
	low = lowIn;
	high = highIn;
	selectionPredicate = selectionPredicateIn;
	projections = projectionsIn;
}

void BPlusSelection :: run () {
}

#endif

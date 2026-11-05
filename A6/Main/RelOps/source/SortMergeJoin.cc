
#ifndef SORTMERGE_CC
#define SORTMERGE_CC

#include "SortMergeJoin.h"

SortMergeJoin :: SortMergeJoin (MyDB_TableReaderWriterPtr leftInputIn, MyDB_TableReaderWriterPtr rightInputIn,
		MyDB_TableReaderWriterPtr outputIn, string finalSelectionPredicateIn,
		vector <string> projectionsIn,
		pair <string, string> equalityCheckIn, string leftSelectionPredicateIn,
		string rightSelectionPredicateIn) {
	output = outputIn;
	finalSelectionPredicate = finalSelectionPredicateIn;
	projections = projectionsIn;
	equalityCheck = equalityCheckIn;
	leftTable = leftInputIn;
	rightTable = rightInputIn;
	leftSelectionPredicate = leftSelectionPredicateIn;
	rightSelectionPredicate = rightSelectionPredicateIn;
	runSize = leftTable->getBufferMgr ()->numPages / 2;
}

void SortMergeJoin :: run () {
}

#endif

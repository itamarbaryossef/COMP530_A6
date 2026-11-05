
#ifndef AGG_CC
#define AGG_CC

#include "Aggregate.h"

Aggregate :: Aggregate (MyDB_TableReaderWriterPtr inputIn, MyDB_TableReaderWriterPtr outputIn,
		vector <pair <MyDB_AggType, string>> aggsToComputeIn,
		vector <string> groupingsIn, string selectionPredicateIn) {
	input = inputIn;
	output = outputIn;
	aggsToCompute = aggsToComputeIn;
	groupings = groupingsIn;
	selectionPredicate = selectionPredicateIn;
}

void Aggregate :: run () {
}

#endif

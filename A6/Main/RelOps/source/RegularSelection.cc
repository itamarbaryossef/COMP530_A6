
#ifndef REG_SELECTION_C
#define REG_SELECTION_C

#include "RegularSelection.h"

RegularSelection :: RegularSelection (MyDB_TableReaderWriterPtr inputIn, MyDB_TableReaderWriterPtr outputIn,
		string selectionPredicateIn, vector <string> projectionsIn) {
	input = inputIn;
	output = outputIn;
	selectionPredicate = selectionPredicateIn;
	projections = projectionsIn;
}

void RegularSelection :: run () {

	// the record we read each input record into; the predicate and the
	// projections are compiled over it, so they always see its current contents
	MyDB_RecordPtr inputRec = input->getEmptyRecord ();

	// the selection predicate
	func pred = inputRec->compileComputation (selectionPredicate);

	// the computations that build each attribute of the output record, in order
	vector <func> finalComputations;
	for (string s : projections) {
		finalComputations.push_back (inputRec->compileComputation (s));
	}

	// the output record, created once and refilled for every accepted record
	MyDB_RecordPtr outputRec = output->getEmptyRecord ();

	// scan the input
	MyDB_RecordIteratorAltPtr myIter = input->getIteratorAlt ();
	while (myIter->advance ()) {

		myIter->getCurrent (inputRec);

		// see if it is accepted by the predicate
		if (!pred ()->toBool ()) {
			continue;
		}

		// run all of the projections
		int i = 0;
		for (auto &f : finalComputations) {
			outputRec->getAtt (i++)->set (f ());
		}

		// the attributes were changed via set (), so the record must be told,
		// or its internal buffer may cause it to write old values
		outputRec->recordContentHasChanged ();
		output->append (outputRec);
	}
}

#endif


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

	// a tree that has never had a record appended has no root (its root
	// location is still -1), and asking it for a range iterator would make it
	// create pages forever; such a tree has no records, so there is no output
	if (input->getTable ()->getRootLocation () == -1) {
		return;
	}

	// the record we read each input record into; the predicate and the
	// projections are compiled over it, so they always see its current contents
	MyDB_RecordPtr inputRec = input->getEmptyRecord ();

	// the selection predicate (it may mention any attribute, not just the key)
	func pred = inputRec->compileComputation (selectionPredicate);

	// the computations that build each attribute of the output record, in order
	vector <func> finalComputations;
	for (string s : projections) {
		finalComputations.push_back (inputRec->compileComputation (s));
	}

	// the output record, created once and refilled for every accepted record
	MyDB_RecordPtr outputRec = output->getEmptyRecord ();

	// iterate over only the records whose key is in [low, high]; the range
	// iterator itself skips every record outside that range
	MyDB_RecordIteratorAltPtr myIter = input->getRangeIteratorAlt (low, high);
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

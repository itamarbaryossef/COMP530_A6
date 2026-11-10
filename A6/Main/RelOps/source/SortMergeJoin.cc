
#ifndef SORTMERGE_CC
#define SORTMERGE_CC

#include "SortMergeJoin.h"
#include "MyDB_PageReaderWriter.h"
#include "Sorting.h"

// builds a comparator that returns true if and only if the value computed by
// lhsF is less than the value computed by rhsF.  This is like the comparator
// built by MyDB_BPlusTreeReaderWriter, except that it compares the results of
// two computations (for example, the two halves of an equality check) rather
// than two attributes.  The comparison is done as int, double, or string,
// depending on the types of the two computations
static function <bool ()> buildComparator (func lhsF, MyDB_AttTypePtr lhsType, func rhsF, MyDB_AttTypePtr rhsType) {

	if (lhsType->promotableToInt () && rhsType->promotableToInt ()) {
		return [lhsF, rhsF] {return lhsF ()->toInt () < rhsF ()->toInt ();};
	} else if (lhsType->promotableToDouble () && rhsType->promotableToDouble ()) {
		return [lhsF, rhsF] {return lhsF ()->toDouble () < rhsF ()->toDouble ();};
	} else {
		return [lhsF, rhsF] {return lhsF ()->toString () < rhsF ()->toString ();};
	}
}

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

	MyDB_BufferManagerPtr mgr = leftTable->getBufferMgr ();

	// sort the left table on equalityCheck.first, keeping only the records
	// accepted by leftSelectionPredicate.  lhsL and rhsL belong to the sorted-run
	// iterator (it uses them to compare the heads of the runs), so we never load
	// records into them ourselves
	MyDB_RecordPtr lhsL = leftTable->getEmptyRecord ();
	MyDB_RecordPtr rhsL = leftTable->getEmptyRecord ();
	function <bool ()> leftSortComp = buildComparator (
		lhsL->compileComputation (equalityCheck.first), lhsL->getType (equalityCheck.first),
		rhsL->compileComputation (equalityCheck.first), rhsL->getType (equalityCheck.first));
	MyDB_RecordIteratorAltPtr leftIter = buildItertorOverSortedRuns (runSize, *leftTable,
		leftSortComp, lhsL, rhsL, leftSelectionPredicate);

	// and the same for the right table, on equalityCheck.second
	MyDB_RecordPtr lhsR = rightTable->getEmptyRecord ();
	MyDB_RecordPtr rhsR = rightTable->getEmptyRecord ();
	function <bool ()> rightSortComp = buildComparator (
		lhsR->compileComputation (equalityCheck.second), lhsR->getType (equalityCheck.second),
		rhsR->compileComputation (equalityCheck.second), rhsR->getType (equalityCheck.second));
	MyDB_RecordIteratorAltPtr rightIter = buildItertorOverSortedRuns (runSize, *rightTable,
		rightSortComp, lhsR, rhsR, rightSelectionPredicate);

	// our own records for the current left and right records
	MyDB_RecordPtr leftRec = leftTable->getEmptyRecord ();
	MyDB_RecordPtr rightRec = rightTable->getEmptyRecord ();

	// the join keys of the current records, and comparators between them
	func leftKey = leftRec->compileComputation (equalityCheck.first);
	func rightKey = rightRec->compileComputation (equalityCheck.second);
	MyDB_AttTypePtr leftKeyType = leftRec->getType (equalityCheck.first);
	MyDB_AttTypePtr rightKeyType = rightRec->getType (equalityCheck.second);
	function <bool ()> leftLess = buildComparator (leftKey, leftKeyType, rightKey, rightKeyType);
	function <bool ()> rightLess = buildComparator (rightKey, rightKeyType, leftKey, leftKeyType);

	// a record holding the first left record of the current group, whose key
	// is the key of the whole group; the right records are compared to it
	MyDB_RecordPtr groupRec = leftTable->getEmptyRecord ();
	func groupKey = groupRec->compileComputation (equalityCheck.first);
	function <bool ()> groupLessRight = buildComparator (groupKey, leftKeyType, rightKey, rightKeyType);
	function <bool ()> rightLessGroup = buildComparator (rightKey, rightKeyType, groupKey, leftKeyType);

	// the schema that results from combining the left and right records
	MyDB_SchemaPtr mySchemaOut = make_shared <MyDB_Schema> ();
	for (auto &p : leftTable->getTable ()->getSchema ()->getAtts ())
		mySchemaOut->appendAtt (p);
	for (auto &p : rightTable->getTable ()->getSchema ()->getAtts ())
		mySchemaOut->appendAtt (p);

	// the combined record, and the final predicate and projections over it
	MyDB_RecordPtr combinedRec = make_shared <MyDB_Record> (mySchemaOut);
	combinedRec->buildFrom (leftRec, rightRec);
	func finalPredicate = combinedRec->compileComputation (finalSelectionPredicate);
	vector <func> finalComputations;
	for (string s : projections) {
		finalComputations.push_back (combinedRec->compileComputation (s));
	}

	// the output record, created once and refilled for every result
	MyDB_RecordPtr outputRec = output->getEmptyRecord ();

	// move both iterators onto their first record
	bool leftHas = leftIter->advance ();
	if (leftHas)
		leftIter->getCurrent (leftRec);
	bool rightHas = rightIter->advance ();
	if (rightHas)
		rightIter->getCurrent (rightRec);

	// the merge
	while (leftHas && rightHas) {

		// if one side's key is smaller, it cannot match anything; skip it
		if (leftLess ()) {
			leftHas = leftIter->advance ();
			if (leftHas)
				leftIter->getCurrent (leftRec);
			continue;
		}
		if (rightLess ()) {
			rightHas = rightIter->advance ();
			if (rightHas)
				rightIter->getCurrent (rightRec);
			continue;
		}

		// the keys are equal.  Copy every left record with this key (the
		// left "group") into a list of pinned pages, adding a page whenever
		// the current one fills up
		leftIter->getCurrent (groupRec);
		vector <MyDB_PageReaderWriter> groupPages;
		MyDB_PageReaderWriter curPage (true, *mgr);
		curPage.clear ();
		groupPages.push_back (curPage);
		while (leftHas && !leftLess () && !rightLess ()) {
			if (!groupPages.back ().append (leftRec)) {
				MyDB_PageReaderWriter newPage (true, *mgr);
				newPage.clear ();
				newPage.append (leftRec);
				groupPages.push_back (newPage);
			}
			leftHas = leftIter->advance ();
			if (leftHas)
				leftIter->getCurrent (leftRec);
		}

		// now join every right record with the same key against the whole
		// left group, checking the full join predicate on each pair
		while (rightHas && !groupLessRight () && !rightLessGroup ()) {

			MyDB_RecordIteratorAltPtr groupIter = getIteratorAlt (groupPages);
			while (groupIter->advance ()) {
				groupIter->getCurrent (leftRec);

				if (finalPredicate ()->toBool ()) {

					// run all of the projections
					int i = 0;
					for (auto &f : finalComputations) {
						outputRec->getAtt (i++)->set (f ());
					}

					// the attributes were changed via set (), so the record
					// must be told, or it may write out old values
					outputRec->recordContentHasChanged ();
					output->append (outputRec);
				}
			}

			rightHas = rightIter->advance ();
			if (rightHas)
				rightIter->getCurrent (rightRec);
		}

		// leftRec was used to scan the group, so reload the record the left
		// iterator is sitting on (the first one after the group), if any
		if (leftHas)
			leftIter->getCurrent (leftRec);
	}
}

#endif

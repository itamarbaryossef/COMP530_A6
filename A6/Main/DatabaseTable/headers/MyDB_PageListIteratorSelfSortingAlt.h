

#ifndef PAGE_LIST_ITER_SORTING_ALT_H
#define PAGE_LIST_ITER_SORTING_ALT_H

#include "MyDB_RecordIteratorAlt.h"
#include "MyDB_PageRecIteratorAlt.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_Record.h"
#include <vector>

using namespace std;

class MyDB_PageListIteratorSelfSortingAlt : public MyDB_RecordIteratorAlt {

public:

	// see MyDB_RecordIteratorAlt.h for what these three do, and for the
	// protocol that governs the order in which they may be called
	void getCurrent (MyDB_RecordPtr intoMe) override {
		myIter->getCurrent (intoMe);
	}

	void *getCurrentPointer () override {
		return myIter->getCurrentPointer ();
	}

	// this one additionally skips over any record that falls outside the
	// [low, high] range that the iterator was built with
	bool advance () override {
		if (myIter == nullptr)		// no pages at all
			return false;
		while (true) {
			if (myIter->advance ()) {
				myIter->getCurrent (myRec);
				if (!lowComparator () && !highComparator ()) {
					return true;
				}
			} else if (curPage == forUs.size () - 1) {
				return false;
			} else {
				curPage++;
				if (sortOrNot)
					forUs[curPage].sortInPlace (comparator, lhs, rhs);	
				myIter = forUs[curPage].getIteratorAlt ();
			}
		}
	}

	// destructor and contructor
	MyDB_PageListIteratorSelfSortingAlt (vector <MyDB_PageReaderWriter> &forUsIn, MyDB_RecordPtr lhsIn, 
		MyDB_RecordPtr rhsIn, function <bool ()> comparatorIn, MyDB_RecordPtr myRecIn, function <bool ()> lowComparatorIn, 
		function <bool ()> highComparatorIn, bool sortOrNotIn) {

		// just remember all of the parameters
		lhs = lhsIn;
		rhs = rhsIn;
		forUs = forUsIn;
		comparator = comparatorIn;				
		lowComparator = lowComparatorIn;
		highComparator = highComparatorIn;
		myRec = myRecIn;
		curPage = 0;
		sortOrNot = sortOrNotIn;

		// set up the first iterator, and we are ready to go!!  (If there are
		// no pages at all, there is nothing to iterate over)
		if (forUs.size () == 0)
			return;
		if (sortOrNot)
			forUs[curPage].sortInPlace (comparator, lhs, rhs);	
		myIter = forUsIn[curPage].getIteratorAlt ();
	}

	~MyDB_PageListIteratorSelfSortingAlt () {}

private:

	MyDB_RecordIteratorAltPtr myIter;
	vector <MyDB_PageReaderWriter> forUs;
	MyDB_RecordPtr lhs, rhs;
	function <bool ()> comparator;
	function <bool ()> lowComparator;
	function <bool ()> highComparator;
	int curPage;
	bool sortOrNot;
	MyDB_RecordPtr myRec;
};

#endif

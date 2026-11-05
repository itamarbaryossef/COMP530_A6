

/****************************************************
** COPYRIGHT 2016, Chris Jermaine, Rice University **
**                                                 **
** The MyDB Database System, COMP 530              **
** Note that this file contains SOLUTION CODE for  **
** A2.  You should not be looking at this file     **
** unless you have completed A2!                   **
****************************************************/


#ifndef TABLE_REC_ITER_ALT_C
#define TABLE_REC_ITER_ALT_C

#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableRecIteratorAlt.h"

void MyDB_TableRecIteratorAlt :: getCurrent (MyDB_RecordPtr intoMe) {
	myIter->getCurrent (intoMe);
}

void *MyDB_TableRecIteratorAlt :: getCurrentPointer () {
	return myIter->getCurrentPointer ();
}

bool MyDB_TableRecIteratorAlt :: advance () {

	// curIsRegular was set when we moved onto this page.  (Asking the page
	// again on every record, via myParent[curPage], means a buffer-manager
	// page lookup per record, which dominated the cost of a table scan)
	if (curIsRegular && myIter->advance ())
		return true;

	if (curPage == myTable->lastPage () || curPage == highPage)
		return false;

	curPage++;
	MyDB_PageReaderWriter p = myParent[curPage];
	curIsRegular = (p.getType () == MyDB_PageType :: RegularPage);
	myIter = p.getIteratorAlt ();
	return advance ();
}

MyDB_TableRecIteratorAlt :: MyDB_TableRecIteratorAlt (MyDB_TableReaderWriter &myParent, MyDB_TablePtr myTableIn,
	int lowPage, int highPageIn) :
	myParent (myParent) {
	myTable = myTableIn;
	curPage = lowPage;
	highPage = highPageIn;
	MyDB_PageReaderWriter p = myParent[curPage];
	curIsRegular = (p.getType () == MyDB_PageType :: RegularPage);
	myIter = p.getIteratorAlt ();		
}

MyDB_TableRecIteratorAlt :: MyDB_TableRecIteratorAlt (MyDB_TableReaderWriter &myParent, MyDB_TablePtr myTableIn) :
	myParent (myParent) {
	myTable = myTableIn;
	curPage = 0;
	highPage = 1999999999;
	MyDB_PageReaderWriter p = myParent[curPage];
	curIsRegular = (p.getType () == MyDB_PageType :: RegularPage);
	myIter = p.getIteratorAlt ();		
}

MyDB_TableRecIteratorAlt :: ~MyDB_TableRecIteratorAlt () {}

#endif

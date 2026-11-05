

/****************************************************
** COPYRIGHT 2016, Chris Jermaine, Rice University **
**                                                 **
** The MyDB Database System, COMP 530              **
** Note that this file contains SOLUTION CODE for  **
** A2.  You should not be looking at this file     **
** unless you have completed A2!                   **
****************************************************/


#ifndef PAGE_LIST_ITER_ALT_H
#define PAGE_LIST_ITER_ALT_H

#include "MyDB_RecordIteratorAlt.h"
#include "MyDB_PageRecIteratorAlt.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_Record.h"
#include <vector>

using namespace std;

class MyDB_PageListIteratorAlt : public MyDB_RecordIteratorAlt {

public:

	// see MyDB_RecordIteratorAlt.h for what these three do, and for the
	// protocol that governs the order in which they may be called
	void getCurrent (MyDB_RecordPtr intoMe) override;
	void *getCurrentPointer () override;
	bool advance () override;

	// destructor and contructor
	MyDB_PageListIteratorAlt (vector <MyDB_PageReaderWriter> &forUs);
	~MyDB_PageListIteratorAlt ();

private:

	MyDB_RecordIteratorAltPtr myIter;
	vector <MyDB_PageReaderWriter> forUs;
	int curPage;
};

#endif

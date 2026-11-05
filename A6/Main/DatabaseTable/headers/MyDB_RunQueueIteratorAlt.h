

/****************************************************
** COPYRIGHT 2016, Chris Jermaine, Rice University **
**                                                 **
** The MyDB Database System, COMP 530              **
** Note that this file contains SOLUTION CODE for  **
** A2.  You should not be looking at this file     **
** unless you have completed A2!                   **
****************************************************/


#ifndef RUN_Q_ITER_ALT_H
#define RUN_Q_ITER_ALT_H

#include "IteratorComparator.h"
#include "MyDB_RecordIteratorAlt.h"
#include "MyDB_RunQueueIteratorAlt.h"
#include "MyDB_PageRecIteratorAlt.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_Record.h"
#include <vector>

using namespace std;
class MyDB_RunQueueIteratorAlt;
typedef shared_ptr <MyDB_RunQueueIteratorAlt> MyDB_RunQueueIteratorAltPtr;

class MyDB_RunQueueIteratorAlt : public MyDB_RecordIteratorAlt {

public:

	// see MyDB_RecordIteratorAlt.h for what these three do, and for the
	// protocol that governs the order in which they may be called
	void getCurrent (MyDB_RecordPtr intoMe) override;
	void *getCurrentPointer () override;
	bool advance () override;

	// build an iterator that uses the given comparator, over the two records
	MyDB_RunQueueIteratorAlt (function <bool ()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs);

	// get access to the internal queue, for building it up
	inline priority_queue <MyDB_RecordIteratorAltPtr,
        vector <MyDB_RecordIteratorAltPtr>, IteratorComparator> &getQ () {
        	return pq;
	}

	~MyDB_RunQueueIteratorAlt ();

private:

	priority_queue <MyDB_RecordIteratorAltPtr, vector <MyDB_RecordIteratorAltPtr>, IteratorComparator> pq;
	bool firstTime;
};

#endif

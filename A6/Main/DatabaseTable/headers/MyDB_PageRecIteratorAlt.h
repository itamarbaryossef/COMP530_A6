

/****************************************************
** COPYRIGHT 2016, Chris Jermaine, Rice University **
**                                                 **
** The MyDB Database System, COMP 530              **
** Note that this file contains SOLUTION CODE for  **
** A2.  You should not be looking at this file     **
** unless you have completed A2!                   **
****************************************************/


#ifndef PAGE_REC_ITER_ALT_H
#define PAGE_REC_ITER_ALT_H

#include "MyDB_PageHandle.h"
#include "MyDB_Record.h"
#include "MyDB_RecordIteratorAlt.h"

class MyDB_PageRecIteratorAlt : public MyDB_RecordIteratorAlt {

public:

	// see MyDB_RecordIteratorAlt.h for what these three do, and for the
	// protocol that governs the order in which they may be called
	void getCurrent (MyDB_RecordPtr intoMe) override;
	void *getCurrentPointer () override;
	bool advance () override;

	// destructor and contructor
	MyDB_PageRecIteratorAlt (MyDB_PageHandle myPageIn); 
	~MyDB_PageRecIteratorAlt ();

private:

	int bytesConsumed;
	int nextRecSize;
	MyDB_PageHandle myPage;
};

#endif

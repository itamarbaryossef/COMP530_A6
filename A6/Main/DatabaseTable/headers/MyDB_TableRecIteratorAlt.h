

/****************************************************
** COPYRIGHT 2016, Chris Jermaine, Rice University **
**                                                 **
** The MyDB Database System, COMP 530              **
** Note that this file contains SOLUTION CODE for  **
** A2.  You should not be looking at this file     **
** unless you have completed A2!                   **
****************************************************/


#ifndef TABLE_REC_ITER_ALT_H
#define TABLE_REC_ITER_ALT_H

#include "MyDB_RecordIteratorAlt.h"
#include "MyDB_Record.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_Table.h"

class MyDB_TableRecIteratorAlt : public MyDB_RecordIteratorAlt {

public:

	// see MyDB_RecordIteratorAlt.h for what these three do, and for the
	// protocol that governs the order in which they may be called
	void getCurrent (MyDB_RecordPtr intoMe) override;
	void *getCurrentPointer () override;
	bool advance () override;

	// destructor and contructor
	MyDB_TableRecIteratorAlt (MyDB_TableReaderWriter &myParent, MyDB_TablePtr myTableIn);
	~MyDB_TableRecIteratorAlt ();
	MyDB_TableRecIteratorAlt (MyDB_TableReaderWriter &myParent, MyDB_TablePtr myTableIn, int lowPage, int highPage);

private:

	MyDB_RecordIteratorAltPtr myIter;
	int curPage;
	int highPage;

	// whether the current page is a regular (record) page; set when we move onto it
	bool curIsRegular;	
	MyDB_TableReaderWriter &myParent;
	MyDB_TablePtr myTable;
};

#endif

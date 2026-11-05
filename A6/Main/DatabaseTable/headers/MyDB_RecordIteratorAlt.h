
#ifndef REC_ITER_ALT_H
#define REC_ITER_ALT_H

#include <memory>
#include "MyDB_Record.h"
using namespace std;

// This pure virtual class is used to iterate through the records in a page or a file.
// Instances of it are created via MyDB_PageReaderWriter.getIteratorAlt () or
// MyDB_TableReaderWriter.getIteratorAlt ().
//
// The name "Alt" is historical: this was the second iterator interface written for
// MyDB, and it is now the only one.  See the assignment handout for the story.
//
// THE PROTOCOL.  An iterator sits ON a record, and the canonical loop is:
//
//		MyDB_RecordIteratorAltPtr myIter = whatever->getIteratorAlt ();
//		while (myIter->advance ()) {
//			myIter->getCurrent (myRec);
//			... do something with myRec ...
//		}
//
// Note that advance () comes FIRST, before the record is ever read.  A freshly
// constructed iterator is not yet sitting on anything, and the first call to
// advance () is what moves it onto the first record (or reports that there is
// no first record, in the case of an empty page or file).
//
// The reason the interface is split this way -- rather than having a single call
// that both reads the current record and moves on -- is that separating the two
// lets you look at a record WITHOUT consuming it.  You can call getCurrent () as
// many times as you like, into as many different records as you like, and the
// iterator does not move until you call advance ().  That is exactly what a
// k-way merge needs: it has to compare the head record of every run, and then
// advance only the run that won.  See IteratorComparator later in the semester.
//
class MyDB_RecordIteratorAlt;
typedef shared_ptr <MyDB_RecordIteratorAlt> MyDB_RecordIteratorAltPtr;

class MyDB_RecordIteratorAlt {

public:

	// Loads the record that the iterator is currently sitting on into intoMe.
	// This does NOT move the iterator, so it is fine to call this more than
	// once, and fine to call it with a different record each time.  It may
	// only be called when the most recent call to advance () returned true.
	virtual void getCurrent (MyDB_RecordPtr intoMe) = 0;

	// Returns the address, within the page, of the record that the iterator is
	// currently sitting on -- that is, of the very record that getCurrent ()
	// would load.  At a later time it is possible to reconstitute that record
	// by calling MyDB_Record.fromBinary (obtainedPointer), ASSUMING that the
	// page holding it has not been swapped out of the buffer pool in the
	// meantime.  The value returned remains valid until the next advance ().
	virtual void *getCurrentPointer () = 0;

	// Moves the iterator forward, past the record it is currently sitting on,
	// and returns true if it has landed on another record, or false if there
	// are no records left.  On a freshly constructed iterator this moves onto
	// the first record, and so advance () must be called once before the first
	// call to getCurrent ().  After that, every call to advance () must be
	// preceded by a call to getCurrent (): the iterator learns how many bytes
	// the current record occupies by reading it, so it cannot skip a record it
	// has never looked at.
	virtual bool advance () = 0;

	// destructor and constructor
	MyDB_RecordIteratorAlt () {};
	virtual ~MyDB_RecordIteratorAlt () {};

};

#endif

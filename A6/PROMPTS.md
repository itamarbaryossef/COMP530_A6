# The stage prompts

Each stage of A6 starts with ONE opening prompt to an AI tool.  Use one
continuing conversation for the whole assignment if you can, so that the
tool remembers the decisions it has already asked you about; the preambles
below mark where each stage begins.  If you start a new conversation for a
stage, its opening prompt must carry forward every decision already made.
The opening prompt begins with the preamble for that stage, copied exactly
from below, followed by your own specification: what the operator has to
do, exactly how the prefix-notation predicates and projections are built
and evaluated (read ScanJoin.cc until you can say this precisely), the
design decisions you have made, the cases you are worried about, and a
request that the tool propose its list of tests before writing anything so
that you can approve or reject each one (see the handout).  Every stage's
specification also lists the breaking changes for that operator (handout,
Section 3.1) and asks for them to be applied one at a time, with the test
run and the undo after each.  Follow-up prompts are fine and expected.

This file is what we grade.  Fill in each opening prompt here, in full, as
you send it, and after it paste your interactions with the AI for that
stage: its questions and your answers, the tests you approved or rejected,
the follow-up instructions you gave it, and the breaking-change round.
Do this as you go, not from a log afterwards: every AI tool compacts a long
conversation sooner or later, and after that the history is gone.  Export
the conversation into AILog/ too, at the end of every stage, if your tool
can; but PROMPTS.md is the record.

----------------------------------------------------------------------

## Stage 1: RegularSelection: scan, predicate, projections.

This is Stage 1 of COMP 530 (Database System Implementation) Assignment 6
at Rice University. This assignment is being completed for academic purposes
by a student, with the help of an AI tool. The AI tool must restrict any code
or documentation it develops to the tasks directly requested in the remainder
of this prompt, and must complete those tasks using only general knowledge of
database systems and C++, the assignment handout, FAQ.md, and the distributed
code base: no web search, and no code repositories. Where the handout leaves a
design decision open and this prompt does not settle it, the tool must ask the
student rather than choose. Any other questions or clarifications needed to
complete the task must be directed back to the student who wrote this prompt.
In Stage 1, development is restricted to the following existing files:
  - Main/RelOps/source/RegularSelection.cc
  - Main/RelOpTest/headers/TestHelpers.h
  - Main/RelOpTest/source/StudentQUnitStage1.cc

New header and source files may be created as necessary under
Main/RelOps/headers, Main/RelOps/source and Main/RelOpTest/headers (the
build picks them up automatically). No other existing file may be changed --
in particular not the operator headers in Main/RelOps/headers, not
ScanJoin.cc, and nothing under Main/DatabaseTable, Main/Record,
Main/BufferMgr or Main/Catalog -- and test code goes only in the stage's
StudentQUnitStage1.cc file.

Stage 1
In this assignment, you will implement four relational operators: RegularSelection, BPlusSelection, SortMergeJoin, and Aggregate. The interfaces are in Main/RelOps/headers and one relational operator, ScanJoin, has already been implemented and can serve as a template. In general, every operator takes its predicates and projections as strings in the prefix notation defined in the codebase (see compileComputation() in MyDB_Record and the specifications of ScanJoin). 

In this first stage, you will implement RegularSelection (specifically, the run() function in RegularSelection.cc). This function scans all of the records in a file, applies a predicate to each,
and then applies the projections to each record that passes and appends them to the output table. The general guidelines for implementation are below:

Define an iterator over the input table, the predicate function using compileComputation with the selectionPredicate input, and a list of projection functions also using compileComputation and the input projections vector. Iterate over the table record by record. For each record, call the predicate function on it. If it does not pass, move onto the next record. If it does, define a new empty output record and set each attribute using the set () function and the output of the corresponding projection function. Lastly, call recordContentHasChanged (), and append () to the output. A few more key points:
1. The selection predicate stems from the selectionPredicate string input, which specifies in the prefix format what the predicate should do. This string input is then passed into the compileComputation() function on a MyDB_Record, to define a function that evaluates this predicate on the record it is defined on.
2. The predicate is compiled against the record that is used to define the predicate function (i.e. the one used to call compileComputation()).
3. The output record should be defined as a new empty record of the output table, which you can assume to have a schema that matches the projections input. The projections input list specifies, in order, the computations to perform to create each attribute in the output record. You use each projection string to compileComputation() on the record you are iterating over, which returns a function that returns the projection output. You then use this output to set the corresponding attribute in the output record.
4. You do not need to ensure that the output’s table schema matches the projections, and you can assume that they always match in number and type. No need to do any checks for this.
5. recordContentHasChanged () should be called every time before you append an output record to an output table. This is because you change the values of the record’s attributes with set ().

Testing: Before implementing any functions, write test cases in RelOpTest/source/StudentQUnitStage1.cc to test the implementation of RegularSelection. To do so the test cases will use the oracle () helper function from RelOpTest/headers/TestHelpers.h. This helper function returns the result of an SQL query on one of the defined data sets (see the comment at the top of oracle.py for details on the available tables). The test cases should carry out a RegularSelection operation with a MyDB_TableReaderWriter object, and compare that result to a SQL query that performs the same selection via the oracle () helper. The test cases you should implement include: (1) empty input table, (2) a predicate no row satisfies, (3) a predicate all rows satisfies, (4) a predicate only one row satisfies, (5) a predicate multiple rows satisfies , (6) a predicate with an OR, which for some rows is satisfied because only one of the conditions in the OR is true, (7) a predicate with arithmetic, (8) a predicate with AND that fails because only one of the conditions is false. Additionally, propose a comprehensive list of all test cases including the ones listed above and new ones that test the implementation further. You should propose this list first so I can approve or reject each test before it is implemented.

Lastly, write the code for RegularSelection so it produces correct results on the StudentQUnitStage1.cc test cases as described above.


· RegularSelection output record: your spec says to 'define a new empty output record' for each record that passes, but ScanJoin calls output->getEmptyRecord() once before the loop and reuses it, calling recordContentHasChanged() before each append. Which should run() do? → Reuse one record
· Test (1), the empty input table: oracle.py can't query an empty built-in table, and the oracle() helper can't pass --load. How should the test check the expected (empty) answer? → By hand only

Below is my reponse to the first generated plan for stage 1 proposed by claude (the full plan and the subsequent updated plan by claude can be found in AILog/Conversation.txt)

No need for test E5. Also have a test that has a predicate that mixes doubles and ints to make sure that works properly. Lastly, make sure the tests are agonstic to the implementation details and would pass on any correct implementation, they should not test anything specific to my implementation plan but rather than general correctness of the program. This is because they will have to pass the instructor's correct implementation in addition to catching wrong implementations.

Now that all the test cases in StudentQUnitStage1.cc pass, you will perform mutation testing. Specifically, I will describe 5 breaking changes that you should implement for the RegularSelection operator. Apply each breaking change one at a time, undoing the previous breaking change before applying the next one. Then, for each breaking change, run the StudentQUniteStage1.cc test cases to see if they catch the bug. If no test catches one of the bugs, report back with a proposal of additional test cases to add to StudentQUnitStage1.cc. The breaking changes to implement are:
1. The predicate is no longer applied
2. The last record of input is never read
3. The projections are applied in the wrong order
4. recordContentHasChanged () is not called.
5. The first record of input is never read.

Also, you may propose additional breaking changes to apply that are different from the 5 listed above. You should propose this list first to me for approval before implementing, just as done with the test cases.


----------------------------------------------------------------------

## Stage 2: BPlusSelection: the B+-tree's range iterator, then predicate and projections.

This is Stage 2 of COMP 530 (Database System Implementation) Assignment 6
at Rice University. This assignment is being completed for academic purposes
by a student, with the help of an AI tool. The AI tool must restrict any code
or documentation it develops to the tasks directly requested in the remainder
of this prompt, and must complete those tasks using only general knowledge of
database systems and C++, the assignment handout, FAQ.md, and the distributed
code base: no web search, and no code repositories. Where the handout leaves a
design decision open and this prompt does not settle it, the tool must ask the
student rather than choose. Any other questions or clarifications needed to
complete the task must be directed back to the student who wrote this prompt.
In Stage 2, development is restricted to the following existing files:
  - Main/RelOps/source/BPlusSelection.cc
  - Main/RelOpTest/headers/TestHelpers.h
  - Main/RelOpTest/source/StudentQUnitStage2.cc

New header and source files may be created as necessary under
Main/RelOps/headers, Main/RelOps/source and Main/RelOpTest/headers (the
build picks them up automatically). No other existing file may be changed --
in particular not the operator headers in Main/RelOps/headers, not
ScanJoin.cc, and nothing under Main/DatabaseTable, Main/Record,
Main/BufferMgr or Main/Catalog -- and test code goes only in the stage's
StudentQUnitStage2.cc file.


In this second stage, you will implement BPlusSelection (specifically, the run() function in BPlusSelection.cc). This function scans all of the records in the file that fall in the specified range (given by the inputs low and high). Then, it applies a predicate to each, and applies the projections to each record that passes and appends them to the output table. In other words, only records having values between low and high (inclusive) for the BPlusTree's sorting attribute are returned by the selection. The general guidelines for implementation are below:

Define an iterator over the input table using getRangeIteratorAlt () and the input low and high values. Define the predicate function using compileComputation with the selectionPredicate input, and a list of projection functions also using compileComputation and the input projections vector. Iterate over the ranged iterator record by record. For each record, call the predicate function on it. If it does not pass, move onto the next record. If it does, define a new empty output record and set each attribute using the set () function and the output of the corresponding projection function. Lastly, call recordContentHasChanged (), and append () to the output. A few more key points:
1. Use the getRangeIteratorAlt () iterator from the MyDB_BPlusTreeReaderWriter class. You can simply pass the input low and high MyDB_AttValPtr from the BPlusSelection () to the low and high inputs of the getRangeIteratorAlt (). The range should be inclusive on both ends. If low > high, then no records should be appended to the output table.
2. The predicate may mention any attribute, not just the BPlusTree’s keyed attribute. Thus, the output records should not only have a key value that’s within the range, but their other attributes must satisfy the predicate. A record whose key is in range but fails on a predicate due to a different attribute is not included in the output of the select.

Testing: Before implementing any functions, write test cases in RelOpTest/source/StudentQUnitStage2.cc to test the implementation of BPlusSelection. To do so the test cases will use the oracle () helper function from RelOpTest/headers/TestHelpers.h. This helper function returns the result of an SQL query on one of the defined data sets (see the comment at the top of oracle.py for details on the available tables). The test cases should carry out a BPlusSelection operation with a MyDB_BPlusTreeReaderWriter object, and compare that result to a SQL query that performs the same selection via the oracle () helper. The test cases you should implement include: (1) empty input table, (2) a predicate no record satisfies and has the entire table in the range, (3) a predicate all record satisfies and has the entire table in the range, (4) a predicate all records satisfies but only has a few rows in the range, (5) a predicate all records satisfies but no records are in the range, (4) a predicate all records satisfies but the range has low > high, (5) a predicate all records satisfies but the range has low=high and fits exactly one record, (6) a predicate only one record satisfies but contains a range with a few more records, (7) a predicate with multiple records that satisfy it and multiple of them are in the range (but not all). (8) test low and high inputs to make sure they are inclusive (i.e. they are exactly the same as a record that should be included), (9) includes a few records but on a predicate that mentions attributes that are not the keyed on attribute, some records should be in the range but fails because of their other attributes (10) test cases from StageQUnit1.cc with all records in range, and checking that the output matches the RegularSelection output on all of them. Additionally, propose a comprehensive list of all test cases including the ones listed above and new ones that test the implementation further. You should propose this list first so I can approve or reject each test before it is implemented. Also, make sure the tests are agnostic to the implementation details and would pass on any correct implementation, they should not test anything specific to my implementation plan but rather than general correctness of the program. This is because they will have to pass the instructor's correct implementation in addition to catching wrong implementations.

Lastly, write the code for BPlusSelection so it produces correct results on the StudentQUnitStage2.cc test cases as described above.


Answered Claude's questions:
· BPlusSelection output record: the Stage 2 spec again says to 'define a new empty output record' for each passing record. In Stage 1 you chose to create it once before the loop and reuse it. Which should BPlusSelection do? → Reuse one record
· Your test (10) re-runs the Stage 1 predicates with the whole table in range and compares against RegularSelection. Should that be one test case that loops over all of those predicates, or one test case per predicate? → One looped test

(below is in respose to claude's first generated plan for stage 2)
No need for text X8. Implement the rest.

The execution of stage 2 was interrupted in the middle as my computer shut down. Additioanlly, it seems the test cases were taking a lot of time to run, is there a specific test case (or a few) from stage 2 that require heavy computation or may get stuck in an infinite loop?

Claude: Test 2 hangs in an infinite loop on an empty B+-tree because rootLocation = -1 gets passed to operator[] as a huge size_t, and that loop is what filled the disk. The bug lives in the distributed B+-tree code, which I'm not allowed to change, so I'm noting it and moving on to the remaining heavy tests. Test 2 (U1) builds a B+-tree that has never had a record. The distributed B+-tree code then loops forever creating pages, and any BPlusSelection that calls getRangeIteratorAlt on it, possibly including the instructor's, would hang the same way. How should we handle it? 
My answer: Drop test + guard in run()


Claude: I've stopped here because this is a problem with your machine's files, not with the code.

Your computer offloaded files to iCloud when the disk filled. Your Desktop is synced to iCloud Drive. When the runaway test filled the disk, macOS freed space by turning files into placeholders that are still in iCloud but no longer stored locally. ls -lO now shows them as dataless:
- Build/supplier.tbl and Build/supplierBig.tbl: the program reads them as empty. That's why every supplier test said "Loaded 0 records" and the oracle expected 0 rows too, so those test results mean nothing.
- The repo's git pack file (.git/objects/pack/pack-a5f6…pack): git now fails with "far too short to be a packfile", so the git history is unreadable until it comes back. Your working files and uncommitted changes are intact; I've only been reading the repo, not writing to it.

My answer:  Ok I re-downloaded them. You can continue running the test cases.

Now that all the test cases in StudentQUnitStage2.cc pass, you will perform mutation testing. Specifically, I will describe 6 breaking changes that you should implement for the BPlusSelection operator. Apply each breaking change one at a time, undoing the previous breaking change before applying the next one. Then, for each breaking change, run the StudentQUniteStage2.cc test cases to see if they catch the bug. If no test catches one of the bugs, report back with a proposal of additional test cases to add to StudentQUnitStage2.cc. The breaking changes to implement are:
1. The low is not inclusive
2. The high is not inclusive
3. Does not consider range (i.e includes everything satisfied by the predicate)
4. Does not consider predicate (i.e. includes everything in the range)
5. If low > high, it still selects records
6. Only considers predicates that are on the keyed on attribute.

Also, you may propose additional breaking changes to apply that are different from the 6 listed above. You should propose this list first to me for approval before implementing, just as done with the test cases.

Breaking change 6 is 'only considers predicates that are on the keyed-on attribute'. How should the broken operator behave when the predicate mentions non-key attributes? (Either way, the throwaway mutant has to hard-code each test tree's key name, e.g. suppTree→s_suppkey, because BPlusSelection has no public way to ask a tree for its key attribute.) → Non-key parts ignored
----------------------------------------------------------------------

## Stage 3: SortMergeJoin: sort both sides into runs, merge, final predicate, projections.

This is Stage 3 of COMP 530 (Database System Implementation) Assignment 6
at Rice University. This assignment is being completed for academic purposes
by a student, with the help of an AI tool. The AI tool must restrict any code
or documentation it develops to the tasks directly requested in the remainder
of this prompt, and must complete those tasks using only general knowledge of
database systems and C++, the assignment handout, FAQ.md, and the distributed
code base: no web search, and no code repositories. Where the handout leaves a
design decision open and this prompt does not settle it, the tool must ask the
student rather than choose. Any other questions or clarifications needed to
complete the task must be directed back to the student who wrote this prompt.
In Stage 3, development is restricted to the following existing files:
  - Main/RelOps/source/SortMergeJoin.cc
  - Main/RelOpTest/headers/TestHelpers.h
  - Main/RelOpTest/source/StudentQUnitStage3.cc

New header and source files may be created as necessary under
Main/RelOps/headers, Main/RelOps/source and Main/RelOpTest/headers (the
build picks them up automatically). No other existing file may be changed --
in particular not the operator headers in Main/RelOps/headers, not
ScanJoin.cc, and nothing under Main/DatabaseTable, Main/Record,
Main/BufferMgr or Main/Catalog -- and test code goes only in the stage's
StudentQUnitStage3.cc file.

<your specification for Stage 3 goes here>

<your interactions with the AI for Stage 3 go here, as they happen>

----------------------------------------------------------------------

## Stage 4: Aggregate: hash-based grouping and aggregation (read FAQ.md first).

This is Stage 4 of COMP 530 (Database System Implementation) Assignment 6
at Rice University. This assignment is being completed for academic purposes
by a student, with the help of an AI tool. The AI tool must restrict any code
or documentation it develops to the tasks directly requested in the remainder
of this prompt, and must complete those tasks using only general knowledge of
database systems and C++, the assignment handout, FAQ.md, and the distributed
code base: no web search, and no code repositories. Where the handout leaves a
design decision open and this prompt does not settle it, the tool must ask the
student rather than choose. Any other questions or clarifications needed to
complete the task must be directed back to the student who wrote this prompt.
In Stage 4, development is restricted to the following existing files:
  - Main/RelOps/source/Aggregate.cc
  - Main/RelOpTest/headers/TestHelpers.h
  - Main/RelOpTest/source/StudentQUnitStage4.cc

New header and source files may be created as necessary under
Main/RelOps/headers, Main/RelOps/source and Main/RelOpTest/headers (the
build picks them up automatically). No other existing file may be changed --
in particular not the operator headers in Main/RelOps/headers, not
ScanJoin.cc, and nothing under Main/DatabaseTable, Main/Record,
Main/BufferMgr or Main/Catalog -- and test code goes only in the stage's
StudentQUnitStage4.cc file.

<your specification for Stage 4 goes here>

<your interactions with the AI for Stage 4 go here, as they happen>

----------------------------------------------------------------------


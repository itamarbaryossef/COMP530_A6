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

<your specification for Stage 1 goes here>

<your interactions with the AI for Stage 1 go here, as they happen>

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

Testing: Before implementing any functions, write test cases in RelOpTest/source/StudentQUnitStage2.cc to test the implementation of BPlusSelection. To do so the test cases will use the oracle () helper function from RelOpTest/headers/TestHelpers.h. This helper function returns the result of an SQL query on one of the defined data sets (see the comment at the top of oracle.py for details on the available tables). The test cases should carry out a BPlusSelection operation with a MyDB_BPlusTreeReaderWriter object, and compare that result to a SQL query that performs the same selection via the oracle () helper. The test cases you should implement include: (1) empty input table, (2) a predicate no record satisfies and has the entire table in the range, (3) a predicate all record satisfies and has the entire table in the range, (4) a predicate all records satisfies but only has a few rows in the range, (5) a predicate all records satisfies but no records are in the range, (4) a predicate all records satisfies but the range has low > high, (5) a predicate all records satisfies but the range has low=high and fits exactly one record, (6) a predicate only one record satisfies but contains a range with a few more records, (7) a predicate with multiple records that satisfy it and multiple of them are in the range (but not all). (8) test low and high inputs to make sure they are inclusive (i.e. they are exactly the same as a record that should be included), (9) includes a few records but on a predicate that mentions attributes that are not the keyed on attribute, some records should be in the range but fails because of their other attributes (10) test cases from StageQUnit1.cc with all records in range, and checking that the output matches the RegularSelection output on all of them. Additionally, propose a comprehensive list of all test cases including the ones listed above and new ones that test the implementation further. You should propose this list first so I can approve or reject each test before it is implemented.

Lastly, write the code for RegularSelection so it produces correct results on the StudentQUnitStage2.cc test cases as described above.


<your interactions with the AI for Stage 2 go here, as they happen>

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


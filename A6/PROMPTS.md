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

<your specification for Stage 2 goes here>

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


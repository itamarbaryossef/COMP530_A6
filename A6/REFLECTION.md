# A6 reflection

Half a page, written after Stage 4.

## What did you have to understand about ScanJoin.cc before you could write a prompt that said how a predicate is built and evaluated, and how did you find it out?
I had to understand how the predicate strings turn into functions that are applied to the records in ScanJoin.cc. After doing so, I could instruct the AI how to apply the input predicates in the selection operators. I figured this out by tracing how the leftSelection predicates are applied to the left input table records in ScanJoin.cc. First, an empty record is created from the input table, which will hold the record we’re currently iterating over. Then, the compileComputation function can be called on that record, to create a function that applies a predicate to it based on the predicate string. Lastly, that function can be called and its output converted to a regular boolean with toBool () to check if the predicate passes or not. I was then able to give these instructions to the AI in the first selection stages. When I reached stage 3, I had to further examine ScanJoin.cc to understand how a finalSelectionPredicate is applied over two different records. A combined record is first created, adding all the attributes from the two different records. Then, the finalSelectionPredicate can be compiled and evaluated on that combined record. This formed the basis for parts of my prompt in stage 3. 


## Which of your breaking changes did none of your existing tests catch, and what did that tell you about your tests?
My fifth breaking change from stage 3, which asks to implement a bug where the merge loop reuses the records that were passed to buildIteratorOverSortedRuns does not cause a test to fail. It turns out that since the implementation always reloads the two comparison records in the merge loop from the runs every time it compares them, reusing the record does not by itself cause a bug that changes the output. Thus, this failed breaking change did not necessarily indicate that a new test needed to be added so I left the test suite unchanged.


## Where did an answer from oracle.py (or from the ScanJoin, or from a beer table) disagree with your code, and who was right?
There was only one time where there was a disagreement and that was because of a bug in the testing functions, which used a schema that was already destroyed ausing the operator to get an empty list of projections (so in that case the oracle was right). Other than that, the operator implementations always agreed with oracle.py, ScanJoin and the beer table every time.

## What did the AI get wrong, and how did you find out?
One thing the AI got wrong is handling of empty B-Plus Tree tables in BPlusSelection. I asked it to implement an empty tree test for stage 2, and that test ended up being stuck in an massive page creation loop as there was no check if the B-Plus tree is empty (rootLocation = -1). It ended up passing that root location as a massive size_t to the [] operator, which caused the page creation loop. I found out about this as the execution of the tests was too long so I shut it down. The AI then helped locate the issue and fix it.


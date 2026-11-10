
#ifndef AGG_CC
#define AGG_CC

#include "Aggregate.h"
#include "MyDB_PageReaderWriter.h"
#include <unordered_map>

Aggregate :: Aggregate (MyDB_TableReaderWriterPtr inputIn, MyDB_TableReaderWriterPtr outputIn,
		vector <pair <MyDB_AggType, string>> aggsToComputeIn,
		vector <string> groupingsIn, string selectionPredicateIn) {
	input = inputIn;
	output = outputIn;
	aggsToCompute = aggsToComputeIn;
	groupings = groupingsIn;
	selectionPredicate = selectionPredicateIn;
}

void Aggregate :: run () {

	int numAggs = (int) aggsToCompute.size ();
	int numGroups = (int) groupings.size ();

	// the output has the grouping attributes first, then the aggregates
	vector <pair <string, MyDB_AttTypePtr>> &outAtts = output->getTable ()->getSchema ()->getAtts ();

	// the schema of the records we keep one of per group:
	// <MyDB_Agg_1, ..., MyDB_Agg_k, MyDB_Grouping_1, ..., MyDB_Grouping_g, MyDB_Count>.
	// The types are taken from the output schema; the count is an int
	MyDB_SchemaPtr aggSchema = make_shared <MyDB_Schema> ();
	for (int i = 0; i < numAggs; i++)
		aggSchema->appendAtt (make_pair ("MyDB_Agg_" + to_string (i + 1), outAtts[numGroups + i].second));
	for (int j = 0; j < numGroups; j++)
		aggSchema->appendAtt (make_pair ("MyDB_Grouping_" + to_string (j + 1), outAtts[j].second));
	aggSchema->appendAtt (make_pair (string ("MyDB_Count"), make_shared <MyDB_IntAttType> ()));
	int countAt = numAggs + numGroups;
	MyDB_RecordPtr aggRec = make_shared <MyDB_Record> (aggSchema);

	// the record we read each input record into
	MyDB_RecordPtr inputRec = input->getEmptyRecord ();

	// a combined record: an aggregate record followed by an input record
	MyDB_SchemaPtr combinedSchema = make_shared <MyDB_Schema> ();
	for (auto &p : aggSchema->getAtts ())
		combinedSchema->appendAtt (p);
	for (auto &p : input->getTable ()->getSchema ()->getAtts ())
		combinedSchema->appendAtt (p);
	MyDB_RecordPtr combinedRec = make_shared <MyDB_Record> (combinedSchema);
	combinedRec->buildFrom (aggRec, inputRec);

	// the computations, over the combined record, that give the new value of
	// each aggregate attribute once one more input record is added to its group:
	// a SUM or AVG adds the expression, a COUNT adds one.  The last one updates
	// MyDB_Count, which is what an AVG is divided by at the end
	vector <func> aggComps;
	for (int i = 0; i < numAggs; i++) {
		string att = "[MyDB_Agg_" + to_string (i + 1) + "]";
		if (aggsToCompute[i].first == MyDB_AggType :: cnt)
			aggComps.push_back (combinedRec->compileComputation ("+ (int[1], " + att + ")"));
		else
			aggComps.push_back (combinedRec->compileComputation ("+ (" + aggsToCompute[i].second + ", " + att + ")"));
	}
	aggComps.push_back (combinedRec->compileComputation ("+ (int[1], [MyDB_Count])"));

	// checks, over the combined record, that the group stored in an aggregate
	// record is really the input record's group (different groups can have
	// the same hash value)
	vector <func> groupMatches;
	for (int j = 0; j < numGroups; j++)
		groupMatches.push_back (combinedRec->compileComputation ("== ([MyDB_Grouping_" + to_string (j + 1) + "], " + groupings[j] + ")"));

	// the grouping computations and the selection predicate, over the input record
	vector <func> groupingComps;
	for (string &g : groupings)
		groupingComps.push_back (inputRec->compileComputation (g));
	func pred = inputRec->compileComputation (selectionPredicate);

	// the pinned pages holding the aggregate records, and the hash table that
	// maps a hash of the grouping values to the locations of the aggregate
	// records with that hash
	MyDB_BufferManagerPtr mgr = input->getBufferMgr ();
	vector <MyDB_PageReaderWriter> pages;
	unordered_map <size_t, vector <void *>> groups;

	// scan the input
	MyDB_RecordIteratorAltPtr myIter = input->getIteratorAlt ();
	while (myIter->advance ()) {

		myIter->getCurrent (inputRec);

		// see if it is accepted by the predicate
		if (!pred ()->toBool ()) {
			continue;
		}

		// hash the grouping values
		size_t hashVal = 0;
		for (auto &f : groupingComps) {
			hashVal ^= f ()->hash ();
		}

		// look for this record's group among the records with this hash
		vector <void *> &candidates = groups [hashVal];
		void *loc = nullptr;
		for (void *c : candidates) {
			aggRec->fromBinary (c);
			bool same = true;
			for (auto &m : groupMatches) {
				if (!m ()->toBool ()) {
					same = false;
					break;
				}
			}
			if (same) {
				loc = c;
				break;
			}
		}

		// if the group is new, create its aggregate record: the grouping
		// values, and all of the aggregates (and the count) at zero
		if (loc == nullptr) {
			for (int i = 0; i < numAggs; i++)
				aggRec->getAtt (i)->fromInt (0);
			for (int j = 0; j < numGroups; j++)
				aggRec->getAtt (numAggs + j)->set (groupingComps[j] ());
			aggRec->getAtt (countAt)->fromInt (0);
			aggRec->recordContentHasChanged ();

			// put it on the last pinned page, or on a new one if that is full
			if (!pages.empty ())
				loc = pages.back ().appendAndReturnLocation (aggRec);
			if (loc == nullptr) {
				MyDB_PageReaderWriter newPage (true, *mgr);
				newPage.clear ();
				pages.push_back (newPage);
				loc = pages.back ().appendAndReturnLocation (aggRec);
			}
			candidates.push_back (loc);
		}

		// update the group's aggregates with this record, and write them back
		aggRec->fromBinary (loc);
		for (int i = 0; i < numAggs; i++)
			aggRec->getAtt (i)->set (aggComps[i] ());
		aggRec->getAtt (countAt)->set (aggComps[numAggs] ());
		aggRec->recordContentHasChanged ();
		aggRec->toBinary (loc);
	}

	// the computations, over an aggregate record, that give the final value of
	// each aggregate: a SUM or COUNT is what was accumulated; an AVG is the
	// accumulated sum divided by the count
	vector <func> finalAggComps;
	for (int i = 0; i < numAggs; i++) {
		string att = "[MyDB_Agg_" + to_string (i + 1) + "]";
		if (aggsToCompute[i].first == MyDB_AggType :: avg)
			finalAggComps.push_back (aggRec->compileComputation ("/ (" + att + ", [MyDB_Count])"));
		else
			finalAggComps.push_back (aggRec->compileComputation (att));
	}

	// write out one output record per group
	MyDB_RecordPtr outputRec = output->getEmptyRecord ();
	bool anyGroup = false;
	for (auto &entry : groups) {
		for (void *loc : entry.second) {
			anyGroup = true;
			aggRec->fromBinary (loc);
			for (int j = 0; j < numGroups; j++)
				outputRec->getAtt (j)->set (aggRec->getAtt (numAggs + j));
			for (int i = 0; i < numAggs; i++)
				outputRec->getAtt (numGroups + i)->set (finalAggComps[i] ());
			outputRec->recordContentHasChanged ();
			output->append (outputRec);
		}
	}

	// with no GROUP BY, SQL gives one row even if no input record qualified:
	// COUNT 0 and NULL sums; MyDB has no NULL, so every aggregate is 0
	if (numGroups == 0 && !anyGroup) {
		for (int i = 0; i < numAggs; i++)
			outputRec->getAtt (i)->fromInt (0);
		outputRec->recordContentHasChanged ();
		output->append (outputRec);
	}
}

#endif

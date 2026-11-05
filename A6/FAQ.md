# A6 FAQ: questions from previous semesters, with my answers

These are real questions students asked about the Aggregate operator, and
the answers I gave.  Read them before you write your Stage 4 prompt; they
describe the implementation strategy I used.  (Your AI should read them too.)

---

**Are we allowed to assume that the results of SUMs and CNTs are all
integers and the results of AVG are doubles?**

Basically, you don't worry about this!  You should get the output schema
(including the return types) from the output database table.

The basic strategy is that you store records with schema identical to the
output database table in your hash table, likely with one additional
attribute that gives the total count (so you can do average).  These
records will have a number of aggregate attributes.  For each of those
attributes, you build up a string that specifies how to update the value
stored in the aggregate attribute.  For example, to handle a sum or an
average, have some code like:

```
if (s.first == MyDB_AggType :: sum || s.first == MyDB_AggType :: avg) {
    aggComps.push_back (combinedRec->compileComputation ("+ (" + s.second + ", [MyDB_AggAtt" + to_string (i) + "])"));
```

So the moral of the story is that you never really worry about the types.
Everything sort of happens automatically, from the given output schema and
the whole record/computation infrastructure.

---

**I know that we can get `func` to compute an attribute value, but how can
we get the attribute from `func`?**

I don't think that you really want to get an attribute from a func.

Example: say that you are trying to update an aggregate record stored in a
hash table, and you've built a list of func objects that compute the new
values that should be stored in an aggregate record.  Then you might have
code like:

```
for (auto &f : aggComps) {
    aggRec->getAtt (numGroups + i++)->set (f ());
}
```

---

**In aggregation we are responsible to update an existing record in our own
aggregation table when there is a match.  I think we can take advantage of
the `buildFrom ()` function which can concatenate two records' values, and
then compile a function on it?  But how do we do the computation over this
concatenated record?  Do you suggest any other easier way to update an
existing record?**

Here's (approximately) what I did.

I first created a schema for my aggregate records, something like:

```
<MyDB_agg_1, MyDB_agg_2, MyDB_agg_3, MyDB_group_1, MyDB_group_2, MyDB_cnt>
```

I took the types of all of the agg attributes and group attributes from the
schema embedded in `output`; the type of the last attribute is an integer.

Then I create a combined record that consists of a record from the file to
be aggregated, plus a record from this schema.

Then, I create a function over the combined record, for each aggregate we
are trying to compute.  Let's say that the 2nd aggregate in `aggsToCompute`
is a SUM aggregate.  Then I would create an aggregate using something like
the string:

```
"+ (" + aggsToCompute[1].second + ", [MyDB_agg_2])"
```

When we process a new input record, we do something like:

```
aggRec->getAtt (1)->set (myFunction ());
```

This will update the attribute in the aggregate record.  Do this for every
aggregate you are computing, and you are good to go!

---

**As for the final step of the aggregate function, we need to iterate over
the hash map and append everything into the output table.  However, the
records that I used for aggregation and the output table's records have
different schemas.  How do you suggest we fix this?  Or can we just call
`output_record->fromBinary (address); output_tbl->append (output_record)`
and assume the user is calling the aggregate function with the right
schema?**

You can assume that the user is calling the aggregate function with the
correct schema.  I actually have a check:

```
// make sure that the number of attributes is OK
if (output->getTable ()->getSchema ()->getAtts ().size () != aggsToCompute.size () + groupings.size ()) {
    cout << "error, the output schema needs to have the same number of atts as (# of aggs to compute + # groups).\n";
    return;
}
```

**However**, you need to be careful.  My aggregate records don't have
exactly the same schema as the output records, because I always add a
count.  Plus, not only that, you can't just blindly copy the values over
(even if the schemas do match) because you might need to compute an
average, which requires dividing by the count.  In the end, I have
something like:

```
// set the grouping atts
for (i = 0; i < numGroups; i++) {
    outRec->getAtt (i)->set (aggRec->getAtt (i));
}

// set the aggregate atts
for (auto a : finalAggComps) {
    outRec->getAtt (i++)->set (a ());
}
outRec->recordContentHasChanged ();
output->append (outRec);
```

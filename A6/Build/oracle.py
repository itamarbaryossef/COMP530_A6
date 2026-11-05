#!/usr/bin/env python3
#
# oracle.py -- an independent answer key for queries over the .tbl files.
#
# Loads the pipe-delimited .tbl files in this directory into an in-memory
# SQLite database and runs the SQL you give it, printing the result rows in
# the same format MyDB prints records (values separated by "|", with a "|"
# after the last one; doubles with six decimal places).  Use it to find out
# what the right answer to a test is BEFORE you ask your code:
#
#   python3 oracle.py "SELECT count(*) FROM supplier WHERE nationkey = 1"
#   python3 oracle.py --sorted "SELECT name, acctbal FROM supplier WHERE nationkey = 1"
#   python3 oracle.py "SELECT n.name, count(*) FROM supplier s, nation n
#                      WHERE s.nationkey = n.nationkey GROUP BY n.name"
#
# --sorted sorts the output lines, so that two multisets of rows can be
# compared with diff or by eye regardless of the order an operator produces
# them in.  --count prints only the number of rows.
#
# The built-in tables and their column names (no l_ / r_ / s_ prefixes; those
# are just the names a test gives the attributes when it builds a schema):
#
#   supplier     supplier.tbl     suppkey, name, address, nationkey, phone, acctbal, comment
#   supplierBig  supplierBig.tbl  the same columns; 32 copies of every supplier row
#   nation       nation.tbl       nationkey, name, regionkey, comment
#   region       region.tbl       regionkey, name, comment
#   likes        likes.tbl        dr, beer
#   serves       serves.tbl       bar, beer
#   freq         freq.tbl         dr, bar
#
# Any other .tbl file can be loaded with
#   --load mytable=myfile.tbl:col1:int,col2:string,col3:double
# (types: int, double, string, bool).  A table is loaded only if the query
# mentions its name, so a missing file only matters if you use it.
#
# Caveats: SQLite's integer division and string comparison behave like
# MyDB's for the data here, but a SUM over many doubles can differ from
# MyDB's in the last decimal places because the additions happen in a
# different order.  Compare doubles to a tolerance, not for equality.

import os
import re
import sqlite3
import sys

BUILTIN = {
	"supplier":    ("supplier.tbl",    "suppkey:int,name:string,address:string,nationkey:int,phone:string,acctbal:double,comment:string"),
	"supplierBig": ("supplierBig.tbl", "suppkey:int,name:string,address:string,nationkey:int,phone:string,acctbal:double,comment:string"),
	"nation":      ("nation.tbl",      "nationkey:int,name:string,regionkey:int,comment:string"),
	"region":      ("region.tbl",      "regionkey:int,name:string,comment:string"),
	"likes":       ("likes.tbl",       "dr:string,beer:string"),
	"serves":      ("serves.tbl",      "bar:string,beer:string"),
	"freq":        ("freq.tbl",        "dr:string,bar:string"),
}

SQLTYPE = {"int": "INTEGER", "double": "REAL", "string": "TEXT", "bool": "TEXT"}


def loadTable (db, name, fileName, schema):
	cols = [c.split (":") for c in schema.split (",")]
	if not os.path.exists (fileName):
		sys.exit ("oracle.py: " + fileName + " not found (needed for table " + name + ")")
	db.execute ("CREATE TABLE " + name + " (" + ", ".join (c + " " + SQLTYPE[t] for c, t in cols) + ")")
	rows = []
	with open (fileName) as f:
		for line in f:
			line = line.rstrip ("\n")
			if line == "":
				continue
			if line.endswith ("|"):
				line = line[:-1]
			fields = line.split ("|")
			if len (fields) != len (cols):
				sys.exit ("oracle.py: " + fileName + ": a line has " + str (len (fields)) + " fields, schema has " + str (len (cols)))
			rows.append (fields)
	db.executemany ("INSERT INTO " + name + " VALUES (" + ",".join ("?" * len (cols)) + ")", rows)
	return [t for c, t in cols]


def fmt (v):
	if isinstance (v, float):
		return "%f" % v
	if v is None:
		return "NULL"
	return str (v)


def main ():
	args = sys.argv[1:]
	sortIt = False
	countOnly = False
	extra = {}
	while args and args[0].startswith ("--"):
		if args[0] == "--sorted":
			sortIt = True
		elif args[0] == "--count":
			countOnly = True
		elif args[0] == "--load" and len (args) > 1:
			name, rest = args[1].split ("=", 1)
			fileName, schema = rest.split (":", 1)
			extra[name] = (fileName, schema)
			args = args[1:]
		else:
			sys.exit ("oracle.py: unknown option " + args[0])
		args = args[1:]
	if len (args) != 1:
		sys.exit ("usage: oracle.py [--sorted] [--count] [--load name=file:col:type,...] \"SQL query\"")
	query = args[0]

	db = sqlite3.connect (":memory:")
	tables = dict (BUILTIN)
	tables.update (extra)
	words = set (re.findall (r"[A-Za-z_][A-Za-z0-9_]*", query))
	for name, (fileName, schema) in tables.items ():
		if name in words:
			loadTable (db, name, fileName, schema)

	rows = db.execute (query).fetchall ()
	if countOnly:
		print (len (rows))
		return
	lines = ["".join (fmt (v) + "|" for v in r) for r in rows]
	if sortIt:
		lines.sort ()
	for l in lines:
		print (l)


if __name__ == "__main__":
	main ()

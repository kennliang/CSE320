# Homework 2 Debugging and Fixing - CSE 320 - Fall 2019
#### Professor Eugene Stark

### **Due Date: Friday 3/8/2019 @ 11:59pm**

# Introduction

In this assignment you are tasked with updating an old piece of
software, making sure it compiles, and works properly in your VM
environment.

Maintaining old code is a chore and an often hated part of software
engineering. It is definitely one of the aspects which are seldom
discussed or thought about by aspiring computer science students.
However, it is prevalent throughout industry and a worthwhile skill to
learn.  Of course, this homework will not give you a remotely
realistic experience in maintaining legacy code or code left behind by
previous engineers but it still provides a small taste of what the
experience may be like.  You are to take on the role of an engineer
whose supervisor has asked you to correct all the errors in the
program, plus add additional functionality.

By completing this homework you should become more familiar
with the C programming language and develop an understanding of:
- How to use tools such as `gdb` and `valgrind` for debugging C code.
- Modifying existing C code.
- C memory management and pointers.
- Working with files and the C standard I/O library.

## The Existing Program

Your goal will be to debug and extend an old version of a program
called `ged2html`, which I (Prof. Stark) wrote in 1994-1995.
The original version ran on BSD Unix systems and MS-DOS.
The version I am handing out is very close to the original version,
except that I have made a few changes for this assignment.
Specifically, I introduced a few bugs here and there to make things
more interesting and educational for you :wink:, and I updated a few things myself
that would have been tedious and not very educational to ask you to do.
Aside from these changes and the introduced bugs, which only involve a few
lines here and there, the code is identical to the original, functioning version.
There is also a bug or two that were undetected in the original program,
but which I detected while working on this assignment, and I have left
them for you to find.

The purpose of the `ged2html` program is to help people who are interested
in researching their family history present their results on the Web
as a set of interlinked HTML files.
The `doc` directory included with the assignment basecode contains the
original documentation files that were distributed with the program.
As its input, the program reads a "GEDCOM" file that contains data on
(among other things) sets of "individuals" and "families".
GEDCOM, which stands for "GEnealogical Data Communication", is a file format
developed by the Family History Department of the Church of Jesus Christ of
Latter-day Saints (the "Mormons") to allow genealogical information to be
shared between software applications.
The `ged2html` program builds a data structure that represents the information
contained in the GEDCOM file, and then it traverses this data structure to
create a set of interlinked HTML output files that can be viewed with a
Web browser.

Though some additional details on the functioning of the `ged2html` program
will be given in a later section, you are not expected to have to fully
understand what it does or how it works in order to be able to complete
the assignment.  This is pretty realistic as far as working on legacy software
is concerned: often one would be given code that needs to be updated, ported,
or have some bugs fixed, and this has to be done without (at least initially)
having a full understanding of structure and function of the code.
In this kind of situation, you have to be careful not to make arbitrary changes
to the code that might impact things that you don't fully understand.
Limit your changes to the minimum necessary to achieve the specified objectives.

Perhaps the easiest way to get an idea of what the program is supposed to do
is to look at some sample input and output.  The `tests/rsrc` directory contains
some sample GEDCOM files (which end with the `.ged` extension).
One of these is `royal92.ged`, which is a 30682-line GEDCOM file containing
information on 3010 individuals and 1422 families of European royalty.
You can read it into a text editor to get an idea of what it looks like. 
The following link: [royal92/INDEX.html](https://bsd7.cs.stonybrook.edu/~cse320/royal92/INDEX.html)
will take you to the HTML files produced by running the command:

<pre>
bin/ged2html -i tests/rsrc/royal92.ged
</pre>

You can browse these HTML files (and view their source, if you like) to get an idea
of the output.

### Getting Started - Obtain the Base Code

Fetch base code for `hw2` as you did for the previous assignments.
You can find it at this link:
[https://gitlab02.cs.stonybrook.edu/cse320/hw2](https://gitlab02.cs.stonybrook.edu/cse320/hw2).

Once again, to avoid a merge conflict with respect to the file `.gitlab-ci.yml`,
use the following command to merge the commits:


	    git merge -m "Merging HW2_CODE" HW2_CODE/master --strategy-option=theirs

Here is the structure of the base code:

<pre>
hw2
├── doc
│   ├── gedstand.t82
│   ├── MSDOS
│   ├── OPTIONS
│   └── README
├── include
│   ├── database.h
│   ├── index.h
│   ├── node.h
│   ├── output.h
│   ├── read.h
│   └── tags.h
├── Makefile
├── rsrc
│   ├── index.tpl
│   └── indiv.tpl
├── src
│   ├── database.c
│   ├── index.c
│   ├── main.c
│   ├── output.c
│   ├── read.c
│   └── tags.c
└── tests
    ├── ged2html_tests.c
    └── rsrc
        ├── garbage.ged
        ├── royal92.ged
        ├── testall
        │   ├── INDEX.html
        │   ├── PERSON1.html
        │   ├── PERSON2.html
        │   ├── PERSON3.html
        │   ├── PERSON4.html
        │   ├── PERSON5.html
        │   ├── PERSON6.html
        │   ├── PERSON7.html
        │   └── PERSON8.html
        └── testall.ged
</pre>

Before you begin work on this assignment, you should read the rest of this
document.  In addition, we additionally advise you to read the
[Debugging Document](DebuggingRef.md).

# Part 1: Debugging and Fixing

The command line arguments and expected operation of the program are described
by the following "usage" message, which is printed within `main()` in `main.c`:

<pre>
Usage: bin/ged2html [-Hciv][-d <max-per-directory>][-s <individual> ...][-u <URL template>][-f <file-template>][-t <individual-template>][-T <index-template>] [-- <gedcom-file> ...]
 -v			Print version information.
 -c			Disable automatic capitalization of surnames.
 -d max_per_directory		Specify number of individuals per subdirectory
			(0 means no subdirectories)
 -i			Cause an index file to be generated containing
			all the individuals in the input.
 -s individuals ...	Limit the production of output files to a specified
			set of zero or more selected individuals.
 -u url_template	Specify a template string for the URL's used
			in anchors in the output files (default '%s.html').
 -f file_template	Specify a template string for the names of the
			HTML files (default '%s.html').
 -t individual_template	Specify an HTML template file for individuals.
 -T index_template	Specify an HTML template file for the index.
 -H			Print a brief message listing the available options.
</pre>

The `-H` and `-v` options cause the program to print a message and terminate
without processing any input.  If neither of those options is given, then the
program performs its normal function of translating GEDCOM to HTML.
The other options can be used to modify the behavior of the program in various
ways.

Under Linux (you only need to worry about making the program compile and run under
Linux, though you will see some `#ifdef`ed code related to MS-DOS)
option processing in `main()` is performed with the help of the GNU `getopt` library.
This library supports a flexible syntax for command-line arguments, including support
for traditional single-character options (prefixed by '-') and "long-form" options
(prefixed by '--'), which need not be single characters.
The library also takes care of some of the "grunt work" in parsing option arguments
and producing error messages.

You can modify anything you want in the assignment, but limit your changes to the
minimum necessary to restore functionality to the program.  Assume that the program
is essentially correct -- it just has a few lingering bugs that need to be fixed.

Complete the following steps:

1. Clean up the code; fixing any compilation issues, so that it compiles
   without error using the compiler options that have been set for you in
   the `Makefile`.

2. Fix bugs.  To get started, try running `bin/ged2html royal92.ged`.
   The program will crash.  Track down and fix the problem.
   Repeat until the program runs without crashing.
   Check the functionality of various option settings and try the other
   data files provided.  You should also use the provided Criterion unit
   tests to help point the way.


3. Use `valgrind` to identify any memory leaks or other memory access
   errors.  Fix any errors you find.

Run `valgrind` using the following command:

	    valgrind --leak-check=full --show-leak-kinds=all [GED2HTML PROGRAM AND ARGS]

Note that the bugs that are present will all manifest themselves in some way
either as program crashes or as memory errors that can be detected by `valgrind`.
It is not necessary to go hunting for obscure issues with the program output.
Also, do not make gratuitous changes to the program output, as this will
interfere with our ability to test your code.

> :scream: You are **NOT** allowed to share or post on PIAZZA
> solutions to the bugs in this program, as this defeats the point of
> the assignment. You may provide small hints in the right direction,
> but nothing more.

# Part 2: Adding Features

The original program uses `getopt()` to do options processing, but it only understands
traditional, single-character options.
For this part of the assignment, rewrite the options processing so that it uses
`getopt_long()` instead of `getopt()`, and it understands the following alternative
forms for the various options (as well as the original short forms):

   - `--help` as equivalent to `-H`
   - `--version` as equivalent to `-v`
   - `--index` as equivalent to `-i`
   - `--select` as equivalent to `-s`
   - `--no-surname-caps` as equivalent to `-c`
   - `--individual-template` as equivalent to `-t`
   - `--index-template` as equivalent to `-T`
   - `--files-per-directory` as equivalent to `-d`
   - `--url-template` as equivalent to `-u`
   - `--filename-template` as equivalent to `-f`

You will probably need to read the Linux "man page" on the `getopt` package.
This can be accessed via the command `man 3 getopt`.  If you need further information,
search for "GNU getopt documentation" on the Web.

> :scream: You MUST use the `getopt_long()` function to process the command line
> arguments passed to the program.  Your program should be able to handle cases where
> the (non-positional) flags are passed IN ANY order.

In addition, add the following new option, which has only a long form:

   - `--change-directory <dirname>`

     This option takes a single argument, which is assumed to be the name of
     an existing directory.  If this option has been specified, then instead of
     creating the HTML output files in the current directory, before the output
     files are produced, the program should use the `chdir()` system call to
     change its current working directory to be the specified directory.
	 The result will be that the HTML files are created in the specified
	 directory.  If the `chdir()` call fails, then instead of producing output
     files, an error message should be printed and the program should exit with
     status `EXIT_FAILURE`.

# Unit Testing

For this assignment, you have been provided with a basic set of
Criterion unit tests to help you debug the program.  We encourage you
to write your own as well as it can help to quickly test inputs to and
outputs from functions in isolation.

In the `tests/ged2html_tests.c` file, there are four unit test examples.
You can run these with the `bin/ged2html_tests` command.
The tests have been constructed so that they will point you at most of the
problems with the program.
They are not true "unit tests", because they all run the program as a black box
using `system()`; the `ged2html` program was unfortunately not designed in a way
that makes it very conducive to using unit testing on individual program modules.
Each test has one or more assertions to make sure that the code functions
properly.  If there was a problem before an assertion, such as a SEGFAULT,
the test will print the error to the screen and continue to run the
rest of the tests.

To obtain more information about each test run, you can use the
verbose print option: `bin/ged2html_tests --verbose=1`.

One of the tests uses `valgrind` to verify that no memory errors are found.
If errors are found, then you can look at the `valgrind.out` file that is
left behind by the test code.
Alternatively, you can better control the information that `valgrind` provides
if you run it manually.

You may write more of your own tests if you wish.  Criterion documentation
for writing your own tests can be found
[here](http://criterion.readthedocs.io/en/master/).
It would be a good idea to have tests that exercise non-default program
options to make sure that the program does not crash when they are used.

# Hand-in Instructions

Ensure that all files you expect to be on your remote
repository are pushed prior to submission.

This homework's tag is: `hw2`

	    $ git submit hw2

# Additional Information

This section provides some additional details on the GEDCOM format and how
it is is processed by the `ged2html` program.  Though it should not be necessary
for this assignment, if you are interested in many more details about GEDCOM
you can refer to the file `doc/gedstand.t82` that is included with the basecode.

A GEDCOM file consists of a sequence of lines of input, each of which starts
with a *level number*, followed by an optional *cross-reference ID*,
then a keyword called a *tag*, and finally data associated with that tag.
Lines with level number 0 introduce *records*, and lines with level numbers
higher than zero define *fields* or *substructures* of the record to which they belong.
Each record starts with a level-0 line and continues up to either the end of the
file or the next level-0 line.  Level numbers are permitted to decrease arbitrarily
or stay the same from one line to the next, but if they increase, they are only
allowed to do so by an increment of one.

For example, the `tests/rsrc/royal92.ged` file starts as follows:

<pre>
0 HEAD
1 SOUR PAF 2.2
1 DEST PAF
1 DATE 20 NOV 1992
1 FILE ROYALS.GED
1 CHAR ANSEL
0 @S1@ SUBM
1 NAME Denis R. Reid
1 ADDR 149 Kimrose Lane
2 CONT Broadview Heights, Ohio 44147-1258
2 CONT Internet Email address:  ah189@cleveland.freenet.edu
1 PHON (216) 237-5364
1 COMM >> In a message to Cliff Manis (cmanis@csoftec.csf.com)
2 CONT >> Denis Reid wrote the following:
2 CONT >> Date: Fri, 25 Dec 92 14:12:32 -0500
</pre>

The first line has level number 0, has no cross-reference ID, has tag `HEAD`,
and has no additional data.  This line introduces the beginning of a *header record*.
Every GEDCOM starts out with a header record in this way.
The subsequent lines with level number 1 define fields of the header record.
The next record has cross reference ID `S1` (enclosed in `@` `@`) and has
tag `SUBM`, which defines a *submitter record*.  A submitter record gives
information about the creator or submitter of the GEDCOM file.

Later on in the file you will see:

<pre>
0 @I1@ INDI
1 NAME Victoria  /Hanover/
1 TITL Queen of England
1 SEX F
1 BIRT
2 DATE 24 MAY 1819
2 PLAC Kensington,Palace,London,England
1 DEAT
2 DATE 22 JAN 1901
2 PLAC Osborne House,Isle of Wight,England
1 BURI
2 PLAC Royal Mausoleum,Frogmore,Berkshire,England
1 REFN 1
1 FAMS @F1@
1 FAMC @F42@
</pre>

The tag `INDI` defines an *individual record*, which gives information about an
individual person.  The cross-reference ID `I1` serves as a unique identifier
within this GEDCOM for this individual, and it allows this record to be linked to
by other records.
This `INDI` record has level-1 substructures introduced by `BIRT` and `DEAT`,
which give information about birth and death events for this individual.
For example, at level 2 within the `BIRT` substructure are fields that give the date
and place of birth.
The `FAMS` and `FAMC` tags define links to *family records* related to this
individual record.  The `FAMS` record defines a *spouse family* link, which
points to a family in which this individual is a spouse (husband or wife).
The `FAMC` record defines a *child family* link, which points to a family in
which this individual is a child.

Still later in the file, you will see examples of family records:

<pre>
0 @F1@ FAM
1 HUSB @I2@
1 WIFE @I1@
1 CHIL @I3@
1 CHIL @I4@
1 CHIL @I5@
1 CHIL @I6@
1 CHIL @I7@
1 CHIL @I8@
1 CHIL @I9@
1 CHIL @I10@
1 CHIL @I11@
1 DIV N
1 MARR
2 DATE 10 FEB 1840
2 PLAC Chapel Royal,St. James Palace,England
</pre>

This record defines a family with unique identifier `F1`, in which the husband
was the individual with identifier `I2`, the wife ws the individual with identifier
`I1`, and there were children with identifiers `I3`, `I4`, and so on.
The `MARR` substructure at level 1 gives information about the event of marriage
by which this family was created.

The end of a GEDCOM file is indicated by a *trailer record*, which looks simply
as follows:

<pre>
0 TRLR
</pre>

The `ged2html` program reads a GEDCOM file line-by-line and builds a data structure.
For each line read, a *node* is created to contain the information from that line:

```c
typedef struct node {
  long lineno;			/* Source line number */
  int  level;			/* Level number at which line appears */
  char *xref;			/* Cross-reference ID */
  struct tag *tag;		/* tag structure for tag */
  char *line;			/* The whole line of input */
  char *rest;			/* The rest of the line after the tag */
  void *hook;			/* Hook to corresponding database node */
  struct node *children;	/* List of subsidiary nodes */
  struct node *siblings;	/* List of siblings */
} NODE;
```

The `siblings` pointer in a `node` structure is used to chain together nodes at the
same level within the "parent" enclosing record or structure having a level
number of one less.
The `children` pointer in a `node` structure is used to point to the immediately
following node, if that node has a level number that is one greater than that of the
current node.
The effect of these pointers is to organize the nodes into a tree as defined by
their level numbers.
So for example, if we have a pointer to the `INDI` record `I1` shown above,
then we could traverse the list of substructures at level 1 by first following the
`children` pointer and then following `siblings` pointers until `NULL` is reached.
As the GEDCOM lines are read and the tree of nodes is constructed, the cross-reference
IDs encountered for level 0 records are entered into a hash table, together with pointers
to the corresponding nodes.  This will enable the nodes to be looked up by ID
in later phases.

Once a complete GEDCOM has been read, the list of all level 0 records is traversed by
starting from the header record and following `siblings` pointers.  For each record,
a structure is allocated of a type that depends on the records tag, and various fields
of this structure are filled in using the sub-nodes of that record.
For example, when a record with tag `INDI` is encountered, a structure of type
`individual_record` is allocated (see the `database.h` header file for definitions of
all the various structures that may be created):

```c
struct individual_record {
  int serial;
  char *xref;
  struct name_structure *personal_name;
  char *title;
  char sex;
  char *refn;
  char *rfn;
  char *afn;
  struct xref *fams, *lastfams;
  struct xref *famc, *lastfamc;
  struct xref *sources, *lastsource;
  struct note_structure *notes, *lastnote;
  struct event_structure *events, *lastevent;
  struct individual_record *next;
};
```

The children of the `INDI` node are processed recursively, and the results of
processing are used to fill in the fields of the `individual_record` structure.
Some child nodes, such as a node with tag `SEX`, just result in simple values to
be stored in a field.  Other child nodes, such as those with tag `BIRT` and `DEAT`,
are process to produce sub-structures that are linked together in lists.
For example, `BIRT` and `DEAT` are examples of *individual events*, which
yield structures of type `struct event_structure` that are linked into a list whose
head is pointed to by `events` and whose tail is pointed to by `lastevent`.
Tags such as `FAMS` and `FAMC` yield structures of type `struct xref`,
which are linked into the corresponding list.   The definition of `struct xref`
is as follows:

```c
struct xref {
  char *id;
  union {
    struct individual_record *individual;
    struct family_record *family;
    struct source_record *source;
  } pointer;
  struct xref *next;
};
```

The purpose of a `struct xref` is to provide a link to a level-0 record.
The `id` field contains a pointer to a string that gives the cross-reference ID
of the pointed-at record.
The `next` field allows the `struct xref` structures to be chained into a list
within an enclosing structure.
The `individual`, `family`, or `source` pointers are initially NULL, but they
will get filled in during the next phase.

When all of the level-0 records have been processed and the corresponding structures
have been filled in, the data is traversed again in a "linking" phase,
whose purpose is to fill in all the `struct xref` structures.  This is done by
using the hash table to look up the identifier stored in the `id` field of the `struct xref`,
obtaining a pointer to the corresponding level-0 record, and storing this pointer
in one of the fields of the `union { ... }  pointer` in the `struct xref`.
This will enable, for example, links from individuals to families, and families
to individuals, to be followed conveniently and efficiently during the output phase.

Once all the linking has been done, the output phase can begin.
For each level-0 record, HTML code is created and output to a file.
The HTML code is created by following a *template*, which is essentially a program,
in a very ugly programming language, that describes how to emit HTML code that is
conditionalized on the content of the GEDCOM.
The file `output.c` contains the interpreter for this ugly programming language.
In the simplest output model, the HTML code for each level-0 record is output to a
separate file, which is named after the cross-reference ID for that record.
There are also provisions for outputting the HTML code for multiple records to a single
file, and for placing files in subdirectories.  These options were provided because
many systems at the time could not efficiently store very large numbers of small files,
or very large numbers of files in a single directory.

Besides the HTML created from the level-0 records, there is also a provision for
creating HTML for a top-level *index*, which can be browsed to identify and navigate
to particular records of interest.


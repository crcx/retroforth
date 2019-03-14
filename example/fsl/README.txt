# Forth Scientific Library

This is a (partial) port of the Forth Scientific Library to RETRO.
It doesn't provide everything at this point, and may not in the
future. I am slowly porting the files as time allows, but this is
not a high priority project for me.

The original files can be found at: https://www.taygeta.com/fsl/scilib.html

I claim no copyright on the ported code; see the individual files
for copyright and license conditions.

## Introduction 

The Forth Scientific Library was created by Skip Carter with the
help of volunteer contributers and reviewers of code. It provides
a collection of routines that may be useful to those doing
scientific or numerically intensive work using Forth. Below is
Skip's original announcement and proposal for the Project.

Currently the Project's activities are being coordinated by
Charles Montgomery, who would welcome comments, suggestions, code
contributions, or whatever, at cgm@physics.utoledo.edu

  
## Skip's Original Introduction  

The Forth language is at an important cross-road with regard to
its use as a general scientific programming language. The new
FORTRAN-90 is just becoming available and long time FORTRAN
programmers are finding it different enough that many are
wondering if they might as well learn a new language instead of
sticking with FORTRAN. If the Forth community plays its hand
right, that alternate language could be Forth. To do so Forth
needs to overcome the standard complaints of the FORTRAN
community:

(1) Its not standardized, so how can I port my software ?
(2) I have lots of pre-existing FORTRAN code that works perfectly
    well and I am not in any hurry to re-write it. Can Forth
    interface with my FORTRAN code ?
(3) There are no scientific libraries in Forth.

The recently adopted ANS Forth handily addresses #1, and in fact
it is the adoption of the standard that makes issues #2 and #3
worth addressing.

With regard to #2, I think the adoption of the standard will help
here, since the interface to other software is the kind of feature
that will distinguish one vendors ANS Forth from anothers. While
the standard does not address such interfaces, I don't think that
there will be too much divergence on how this is done. The Unix
world has no such standard, and I have only encountered 2 different
C-FORTRAN conventions in over 15 years of using Unix.

So #1 is now solved, and the vendors will (I hope!) address #2. The
third point can be addressed by the Forth community itself. Several
potential scientific users of Forth discussed these issues at the
1994 Rochester Forth Conference. It was decided that we should
undertake the project of writing a scientific library in ANS Forth.

The plan is to write a set of Forth words to implement such libraries
as the ACM libraries, BLAS, LINPACK, etc. The libraries will be
publicly available in source form (in some sort of "public" release:
public domain, copyleft, copyrighted but freely distributable, etc).
....

 Everett (Skip) Carter          Phone: 831-641-0645 FAX:  831-641-0647
 Taygeta Scientific Inc.        INTERNET: skip@taygeta.com
 1340 Munras Ave., Suite 314    UUCP:     ...!uunet!taygeta!skip
 Monterey, CA. 93940            WWW: http://www.taygeta.com/skip.html

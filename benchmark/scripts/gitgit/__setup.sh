#!/bin/sh
## Setup shell environment to run a series of benchmarks.
## This script provides core variable settings and will be run
## from other scripts in this directory.
##
## This script is for Unix-based systems *AND* Windows.
##
## Variables prefixed with BM_ are exported to the environment.
## Individual benchmarks should use a different prefix for local
## variables to avoid confusion (and cross-script contamination).
##################################################################

set -x

## The name of the benchmark EXE.
##
## On Linux and Mac, I'm assuming you built the tree in release mode.
##
if [ "`uname`" = "Linux" ]; then
    export BM_EXE=../../../build/libgit2_bench
elif [ "`uname`" = "Darwin" ]; then
    export BM_EXE=../../../build/libgit2_bench
else
##    export BM_EXE=../../../build/RelWithDebInfo/libgit2_bench.exe
    export BM_EXE=../../../build/Debug/libgit2_bench.exe
fi


## Some repos have deep paths in them and our benchmarks inherit
## and (at least on Windows) we want this to be as shallow as
## possible. And we want to be able to choose alternate drives
## for local cloning and etc.
##
## The EXE will call getenv() on this variable name.
if [ "`uname`" = "Linux" ]; then
    export GITBENCH_TEMP=/tmp
elif [ "`uname`" = "Darwin" ]; then
    export GITBENCH_TEMP=$TMPDIR
else
    export GITBENCH_TEMP=C:/t
fi
[ -d $GITBENCH_TEMP ] || mkdir $GITBENCH_TEMP


## Code name for tests based upon the repo we are testing with.
export BM_PWD=`pwd`
export BM_CODE=`basename $BM_PWD`


## Remote URL of the repo.
## (Install a credential helper in advance if a clone will need creds.)
export BM_RURL=https://github.com/git/git.git


## Convert our current branch into a label for the logs.
export BM_LABEL=`git branch | grep '*' | sed 's/\* //' | sed 's|/|__|g'`


## Place to accumulate output and logs.
export BM_LOGS_DIR=$GITBENCH_TEMP/logs/$BM_LABEL/$BM_CODE
[ -d $BM_LOGS_DIR ] || mkdir -p $BM_LOGS_DIR


## Some benchmarks will reference a local/bare repo.  (Others do their own
## remote clone.)
##
## We will try to clone this exactly once and re-use the repo on subsequent
## benchmark runs, so don't mess with it.
export BM_LURL=$GITBENCH_TEMP/$BM_CODE
export BM_FURL=file:///$GITBENCH_TEMP/$BM_CODE


date
[ -d $BM_LURL ] || git clone -q --bare $BM_RURL $BM_LURL
date


## Gather some info about the version of the benchmark
## source tree (to allow the individual benchmarks to
## prepend to their test run logfiles).
##
## We pipe "git log" thru cat to keep it from launching
## a pager.
##
bm_get_info () {
    echo
    echo "################################################################"
    echo "################################################################"
    echo
    date
    echo
    git log --oneline -9 | cat
    echo
    git status -b -s --porcelain
    echo
    echo "________________________________________________________________"
    echo
}

## End of setup.

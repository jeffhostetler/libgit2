#!/bin/sh
## Script to run some checkout benchmarks.
## CD to this directory and run this script from here.
##################################################################

. ./__setup.sh || exit 1


## A short name for this script. We use this to separate
## the log files from other benchmarks.
XX_BM=`basename $0 .sh`

## All log/output for this run will be in the following directory.
XX_LOG_DIR=$BM_LOGS_DIR/$XX_BM/`/bin/date +%Y%m%d.%H%M%S`
[ -d $XX_LOG_DIR ] || mkdir -p $XX_LOG_DIR

## Perf data goes here.  Subsequent runs are APPENDED to this file.
XX_LOG=$XX_LOG_DIR/run.log

## How many times to run the tests (passed into the benchmark).
XX_COUNT=3

## Also run benchmark using git.exe in the body of the test.
XX_USEGIT=--git

## Verbose setting within benchmark.
##XX_VERBOSE=-v

## Write details of the current libgit2 branch to the log before
## we run the benchmark.
bm_get_info >> $XX_LOG

#### TODO Figure out git.exe default value for {local,no-local,no-hardlinks} and
#### TODO make sure that we set the corresponding value in git_clone().
#### date
#### time $BM_EXE $XX_VERBOSE -c $XX_COUNT --logfile $XX_LOG $XX_USEGIT clone $BM_LURL

date
time $BM_EXE $XX_VERBOSE -c $XX_COUNT --logfile $XX_LOG $XX_USEGIT clone --local $BM_LURL
date
time $BM_EXE $XX_VERBOSE -c $XX_COUNT --logfile $XX_LOG $XX_USEGIT clone --no-local $BM_LURL
date
time $BM_EXE $XX_VERBOSE -c $XX_COUNT --logfile $XX_LOG $XX_USEGIT clone --no-hardlinks $BM_LURL
date


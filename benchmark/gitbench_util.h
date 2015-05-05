/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */

#ifndef H_GITBENCH_UTIL_H
#define H_GITBENCH_UTIL_H

/*****************************************************************/

#if defined(_WIN32)
#define BM_GIT_EXE "git.exe"
#else
#define BM_GIT_EXE "/usr/bin/git"
#endif

/*****************************************************************/

#define GITBENCH_EARGUMENTS (INT_MIN+1)

extern int gitbench_util_create_tempdir(char **out);
extern int gitbench_util_set_autocrlf(const char *wd, bool autocrlf);
extern int gitbench_util_set_mergelimit(const char *wd);

/**
 * Invoke "status" using LG2.
 * Record execution time for each iteration under id.
 * Repeat n times.
 * This routine performs a single "step" in an overall "benchmark".
 */
int gitbench_util_status__lg2(
	gitbench_run *run,
	const char *wd,
	int operation_id,
	int status_repeat_count);

/**
 * Invoke "status" using GIT.EXE.
 * Record execution time for each iteration under id.
 * Repeat n times.
 * This routine performs a single "step" in an overall "benchmark".
 */
int gitbench_util_status__exe(
	gitbench_run *run,
	const char *wd,
	int operation_id,
	int status_repeat_count);

/**
 * Invoke "clone" using GIT.EXE.
 * Clone is created in TMP.
 * DO NOT CHECKOUT HEAD.
 * Record execution time under id.
 * This routine performs a single "step" in an overall "benchmark".
 */
int gitbench_util_clone__exe(
	gitbench_run *run,
	const char *url,
	const char *wd,
	int operation_id);


#endif

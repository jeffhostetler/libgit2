/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */

#include <stdio.h>
#include "git2.h"
#include "git2/sys/repository.h"
#include "common.h"
#include "buffer.h"
#include "fileops.h"
#include "gitbench_globals.h"
#include "gitbench_run.h"
#include "gitbench_util.h"
#include "gitbench_opt.h"
#include "gitbench_shell.h"
#include "gitbench_operation.h"
#include "gitbench_benchmark.h"

int gitbench_util_clone__exe(
	gitbench_run *run,
	const char *url,
	const char *wd,
	int operation_id)
{
	const char * argv[10] = {0};
	int k;
	int error;

	gitbench_run_start_operation(run, operation_id);

	k = 0;
	argv[k++] = BM_GIT_EXE;
	argv[k++] = "clone";
	argv[k++] = "--quiet";
	argv[k++] = "--no-checkout";
	argv[k++] = "--local";
	argv[k++] = url;
	argv[k++] = wd;
	argv[k++] = 0;

	if ((error = gitbench_shell(argv, NULL, NULL)) < 0)
		goto done;

#if 0
	k = 0;
	argv[k++] = BM_GIT_EXE;
	argv[k++] = "config";
	argv[k++] = "core.autocrlf";
	argv[k++] = ((benchmark->autocrlf) ? "true" : "false");
	argv[k++] = 0;

	if ((error = gitbench_shell(argv, wd, NULL)) < 0)
		goto done;
#endif

done:
	gitbench_run_finish_operation(run);
	return error;
}

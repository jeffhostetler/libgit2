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

int gitbench_util_status__lg2(
	gitbench_run *run,
	const char *wd,
	int operation_id,
	int status_repeat_count)
{
	git_repository *repo = NULL;
	git_status_list *status = NULL;
	git_status_options status_opts = GIT_STATUS_OPTIONS_INIT;
	int error;
	int x;

	status_opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
	status_opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
		GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX;

	if ((error = git_repository_open(&repo, wd)) < 0)
		goto done;

	for (x = 0; x < status_repeat_count; x++) {
		gitbench_run_start_operation(run, operation_id);
		error = git_status_list_new(&status, repo, &status_opts);
		gitbench_run_finish_operation(run);

		if (error < 0)
			goto done;

		git_status_list_free(status);
		status = NULL;
	}

done:
	git_repository_free(repo);
	return error;
}

int gitbench_util_status__exe(
	gitbench_run *run,
	const char *wd,
	int operation_id,
	int status_repeat_count)
{
	const char * argv[10] = {0};
	int k = 0;
	int error;
	int x;

	argv[k++] = BM_GIT_EXE;
	argv[k++] = "status";
	argv[k++] = "--porcelain";
	argv[k++] = "--branch";
	argv[k++] = 0;

	for (x = 0; x < status_repeat_count; x++) {
		gitbench_run_start_operation(run, operation_id);
		error = gitbench_shell(argv, wd, NULL);
		gitbench_run_finish_operation(run);

		if (error < 0)
			goto done;
	}

done:
	return error;
}


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

int gitbench_util_merge__lg2(
	gitbench_run *run,
	const char *wd,
	const char *ref,
	int operation_id)
{
	git_repository *repo = NULL;
	git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
	git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;
	git_annotated_commit *ac[1] = { NULL };
	git_object *obj = NULL;
	int error;

	gitbench_run_start_operation(run, operation_id);

	if ((error = git_repository_open(&repo, wd)) < 0)
		goto done;
	if ((error = git_revparse_single(&obj, repo, ref)) < 0)
		goto done;
	if ((error = git_annotated_commit_lookup(&ac[0], repo, git_object_id(obj))) < 0)
		goto done;

	error = git_merge(repo, (const git_annotated_commit **)ac, 1,
					  &merge_opts, &checkout_opts);

done:
	gitbench_run_finish_operation(run);
	git_annotated_commit_free(ac[0]);
	git_object_free(obj);
	git_repository_free(repo);
	return error;
}

int gitbench_util_merge__exe(
	gitbench_run *run,
	const char *wd,
	const char *ref,
	int operation_id)
{
	const char * argv[10] = {0};
	int exit_status;
	int error;
	int k = 0;

	argv[k++] = BM_GIT_EXE;
	argv[k++] = "merge";
	argv[k++] = "--no-commit";
	argv[k++] = "--quiet";
	argv[k++] = ref;
	argv[k++] = 0;

	gitbench_run_start_operation(run, operation_id);
	error = gitbench_shell(argv, wd, &exit_status);
	gitbench_run_finish_operation(run);

	/* "git merge" exits with 1 when there are merge conflicts
	 * OR when the target commit cannot be found.  (We get 128
	 * or 129 for usage errors.)
	 *
	 * If we get a 1, assume a conflict.  This implies that
	 * merge finished and we can continue with the timing.
	 * So we ignore the sanitized result and key off the
	 * actual exit status instead.
	 */
	if (exit_status == 1)
		fprintf(gitbench_globals.logfile, "::::: git-merge.exe exited with 1; assuming conflicts\n");
	if ((exit_status == 0) || (exit_status == 1))
		return 0;
	return error;
}

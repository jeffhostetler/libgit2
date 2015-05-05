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

int gitbench_util_checkout__lg2(
	gitbench_run *run,
	const char *wd,
	const char *ref,
	int operation_id)
{
	git_repository *repo = NULL;
	git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
	git_object *obj = NULL;
	int error;

	gitbench_run_start_operation(run, operation_id);

	checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;

	if ((error = git_repository_open(&repo, wd)) < 0)
		goto done;

	if ((error = git_revparse_single(&obj, repo, ref)) < 0)
		goto done;

	if ((error = git_checkout_tree(repo, obj, &checkout_opts)) < 0)
		goto done;

	if ((error = git_repository_set_head_detached(repo, git_object_id(obj))) < 0)
		goto done;

done:
	gitbench_run_finish_operation(run);
	git_object_free(obj);
	git_repository_free(repo);
	return error;
}

int gitbench_util_checkout__exe(
	gitbench_run *run,
	const char *wd,
	const char *ref,
	int operation_id)
{
	const char * argv[10] = {0};
	int k;
	int error;

	k = 0;
	argv[k++] = BM_GIT_EXE;
	argv[k++] = "checkout";
	argv[k++] = "--quiet";
	argv[k++] = "--force";
	argv[k++] = "--detach";
	argv[k++] = ref;
	argv[k++] = 0;

	gitbench_run_start_operation(run, operation_id);
	error = gitbench_shell(argv, wd, NULL);
	gitbench_run_finish_operation(run);

	return error;
}

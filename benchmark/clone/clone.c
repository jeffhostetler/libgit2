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

typedef struct gitbench_benchmark_clone {
	gitbench_benchmark base;

	char *repo_path;

	git_clone_local_t local;
	bool bare;

} gitbench_benchmark_clone;

enum clone_operation_t {
	CLONE_OPERATION_EXE_CLONE = 0,
	CLONE_OPERATION_LG2_CLONE,
};

static gitbench_operation_spec clone_operations[] = {
	{ CLONE_OPERATION_EXE_CLONE, "ExeClone" },
	{ CLONE_OPERATION_LG2_CLONE, "Lg2Clone" },
};
#define CLONE_OPERATIONS_COUNT (sizeof(clone_operations) / sizeof(clone_operations[0]))

static const gitbench_opt_spec clone_cmdline_opts[] = {
	{ GITBENCH_OPT_SWITCH, "help",           0, NULL,      "display help",   GITBENCH_OPT_USAGE_HIDDEN },
	{ GITBENCH_OPT_ARG,    "repository",     0, NULL,      "the repository", GITBENCH_OPT_USAGE_REQUIRED },
	{ GITBENCH_OPT_SWITCH, "local",          0, "local",        "local" },
	{ GITBENCH_OPT_SWITCH, "no-local",       0, "no-local",     "no-local" },
	{ GITBENCH_OPT_SWITCH, "no-hardlinks",   0, "no-hardlinks", "no-hardlinks" },
	{ 0 }
};

static int _do_setup(
	git_buf *wd_path,
	gitbench_benchmark_clone *benchmark,
	gitbench_run *run)
{
	int error;

	GIT_UNUSED(benchmark);

	if ((error = git_buf_joinpath(wd_path, run->buf_sandbox.ptr, "wd")) < 0)
		return error;
	if ((error = git_futils_mkdir(wd_path->ptr, NULL, 0700, GIT_MKDIR_VERIFY_DIR)) < 0)
		return error;

	return error;
}

static int clone_run(gitbench_benchmark *b, gitbench_run *run)
{
	gitbench_benchmark_clone *benchmark = (gitbench_benchmark_clone *)b;
	git_buf wd_path = GIT_BUF_INIT;
	int error;

	if ((error = _do_setup(&wd_path, benchmark, run)) < 0)
		goto done;
	
	if (run->use_git_exe)
		error = gitbench_util_clone__exe(run, benchmark->repo_path, wd_path.ptr, benchmark->bare, benchmark->local, CLONE_OPERATION_EXE_CLONE);
	else
		error = gitbench_util_clone__lg2(run, benchmark->repo_path, wd_path.ptr, benchmark->bare, benchmark->local, CLONE_OPERATION_LG2_CLONE);

done:
	git_buf_free(&wd_path);
	return error;
}

static void clone_free(gitbench_benchmark *b)
{
	gitbench_benchmark_clone *benchmark = (gitbench_benchmark_clone *)b;

	if (!b)
		return;

	git__free(benchmark->repo_path);
	git__free(benchmark);
}

static int clone_configure(
	gitbench_benchmark_clone *benchmark,
	int argc,
	const char **argv)
{
	gitbench_opt_parser parser;
	gitbench_opt opt;

	/* the 3 local-related args should be treated as
	 * a radio group, but we just take the last value
	 * we see.
	 */
	benchmark->local = GIT_CLONE_LOCAL_AUTO;

	/* For "clone" benchmark tests we always use "bare" because we
	 * are only interested in the time to clone the contents of
	 * the ".git" directory (and the various local/no-local/...
	 * options).  We won't actually use the resulting repo, so it
	 * doesn't matter if it is bare or not.
	 */
	benchmark->bare = true;

	gitbench_opt_parser_init(&parser, clone_cmdline_opts, argv + 1, argc - 1);

	while (gitbench_opt_parser_next(&opt, &parser)) {
		if (!opt.spec) {
			fprintf(stderr, "%s: unknown argument: '%s'\n", gitbench_globals.progname, argv[parser.idx]);
			gitbench_opt_usage_fprint(stderr, gitbench_globals.progname, clone_cmdline_opts);
			return GITBENCH_EARGUMENTS;
		}

		if (strcmp(opt.spec->name, "help") == 0) {
			gitbench_opt_usage_fprint(stderr, gitbench_globals.progname, clone_cmdline_opts);
			return GITBENCH_EARGUMENTS;
		} else if (strcmp(opt.spec->name, "repository") == 0) {
			benchmark->repo_path = git__strdup(opt.value);
			GITERR_CHECK_ALLOC(benchmark->repo_path);
		} else if (strcmp(opt.spec->name, "local") == 0) {
			benchmark->local = GIT_CLONE_LOCAL;
		} else if (strcmp(opt.spec->name, "no-local") == 0) {
			benchmark->local = GIT_CLONE_NO_LOCAL;
		} else if (strcmp(opt.spec->name, "no-hardlinks") == 0) {
			benchmark->local = GIT_CLONE_LOCAL_NO_LINKS;
		}
	}

	if (!benchmark->repo_path) {
		gitbench_opt_usage_fprint(stderr, gitbench_globals.progname, clone_cmdline_opts);
		return GITBENCH_EARGUMENTS;
	}

	return 0;
}

int gitbench_benchmark_clone_init(
	gitbench_benchmark **out,
	int argc,
	const char **argv)
{
	gitbench_benchmark_clone *benchmark;
	int error;

	if ((benchmark = git__calloc(1, sizeof(gitbench_benchmark_clone))) == NULL)
		return -1;

	benchmark->base.operation_cnt = CLONE_OPERATIONS_COUNT;
	benchmark->base.operations = clone_operations;
	benchmark->base.run_fn = clone_run;
	benchmark->base.free_fn = clone_free;

	if ((error = clone_configure(benchmark, argc, argv)) < 0) {
		clone_free((gitbench_benchmark *)benchmark);
		return error;
	}

	*out = (gitbench_benchmark *)benchmark;
	return 0;
}

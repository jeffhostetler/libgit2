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

typedef struct gitbench_benchmark_checkoutn {
	gitbench_benchmark base;

	char *repo_url;
	git_vector vec_refs;

	unsigned int autocrlf:1;

	int status_count;

} gitbench_benchmark_checkoutn;

enum checkoutn_operation_t {
	CHECKOUTN_OPERATION_EXE_CLONE = 0,
	CHECKOUTN_OPERATION_EXE_INIT_CO, /* Initial checkout */
	CHECKOUTN_OPERATION_LG2_INIT_CO, /* Initial checkout */
	CHECKOUTN_OPERATION_EXE_DO_CO,   /* Subsequent checkouts */
	CHECKOUTN_OPERATION_LG2_DO_CO,   /* Subsequent checkouts */
	CHECKOUTN_OPERATION_EXE_STATUS,
	CHECKOUTN_OPERATION_LG2_STATUS
};

static gitbench_operation_spec checkoutn_operations[] = {
	{ CHECKOUTN_OPERATION_EXE_CLONE,    "ExeClone" },
	{ CHECKOUTN_OPERATION_EXE_INIT_CO,  "ExeInitCO" },
	{ CHECKOUTN_OPERATION_LG2_INIT_CO,  "Lg2InitCO" },
	{ CHECKOUTN_OPERATION_EXE_DO_CO,    "ExeDoCO" },
	{ CHECKOUTN_OPERATION_LG2_DO_CO,    "Lg2DoCO" },
	{ CHECKOUTN_OPERATION_EXE_STATUS,   "ExeStatus" },
	{ CHECKOUTN_OPERATION_LG2_STATUS,   "Lg2Status" },
};
#define CHECKOUTN_OPERATIONS_COUNT (sizeof(checkoutn_operations) / sizeof(checkoutn_operations[0]))

static const gitbench_opt_spec checkoutn_cmdline_opts[] = {
	{ GITBENCH_OPT_SWITCH, "help",       0, NULL,      "display help",                GITBENCH_OPT_USAGE_HIDDEN },
	{ GITBENCH_OPT_SWITCH, "autocrlf",   0, NULL,      "turn on core.autocrlf=true" },
	{ GITBENCH_OPT_VALUE,  "status",   's', "count",   "times to run status aftwards", GITBENCH_OPT_USAGE_VALUE_REQUIRED },
	{ GITBENCH_OPT_ARG,    "repository", 0, NULL,      "the repository",              GITBENCH_OPT_USAGE_REQUIRED },
	{ GITBENCH_OPT_ARGS,   "refs",       0, NULL, "2 or more references to checkout", GITBENCH_OPT_USAGE_REQUIRED },
	{ 0 }
};




static int _do_core_setup(
	git_buf *wd_path,
	gitbench_benchmark_checkoutn *benchmark,
	gitbench_run *run)
{
	int error;

	if ((error = git_buf_joinpath(wd_path, run->tempdir, "wd")) < 0)
		return error;
	if ((error = git_futils_mkdir(wd_path->ptr, NULL, 0700, GIT_MKDIR_VERIFY_DIR)) < 0)
		return error;

	return error;
}

static int _do_checkoutn(
	gitbench_benchmark_checkoutn *benchmark,
	gitbench_run *run,
	const char *wd)
{
	char *sz_i;
	size_t i;
	int op;
	int error;

	git_vector_foreach(&benchmark->vec_refs, i, sz_i) {
		if (gitbench_globals.verbosity)
			fprintf(gitbench_globals.logfile, ": Checkout %s\n", sz_i);

		if (run->use_git_exe) {
			if (i == 0)
				op = CHECKOUTN_OPERATION_EXE_INIT_CO;
			else
				op = CHECKOUTN_OPERATION_EXE_DO_CO;
			error = gitbench_util_checkout__exe(run, wd, sz_i, op);
		} else {
			if (i == 0)
				op = CHECKOUTN_OPERATION_LG2_INIT_CO;
			else
				op = CHECKOUTN_OPERATION_LG2_DO_CO;
			error = gitbench_util_checkout__lg2(run, wd, sz_i, op);
		}
		if (error < 0)
			goto done;

		/* We can run both version of status regardless of who does the checkout. */
		if ((error = gitbench_util_status__exe(run, wd, CHECKOUTN_OPERATION_EXE_STATUS, benchmark->status_count)) < 0)
			goto done;
		if ((error = gitbench_util_status__lg2(run, wd, CHECKOUTN_OPERATION_LG2_STATUS, benchmark->status_count)) < 0)
			goto done;
	}

done:
	return error;
}


static int checkoutn_run(gitbench_benchmark *b, gitbench_run *run)
{
	gitbench_benchmark_checkoutn *benchmark = (gitbench_benchmark_checkoutn *)b;
	git_buf wd_path = GIT_BUF_INIT;
	int error;

	if ((error = _do_core_setup(&wd_path, benchmark, run)) < 0)
		goto done;

	if ((error = gitbench_util_clone__exe(run, benchmark->repo_url, wd_path.ptr, GIT_CLONE_LOCAL, false, CHECKOUTN_OPERATION_EXE_CLONE)) < 0)
		goto done;

	if ((error = gitbench_util_set_autocrlf(wd_path.ptr, benchmark->autocrlf)) < 0)
		goto done;

	if ((error = _do_checkoutn(benchmark, run, wd_path.ptr)) < 0)
		goto done;

done:
	git_buf_free(&wd_path);
	return error;
}

static void checkoutn_free(gitbench_benchmark *b)
{
	gitbench_benchmark_checkoutn *benchmark = (gitbench_benchmark_checkoutn *)b;
	char *sz_i;
	size_t i;

	if (!b)
		return;

	git__free(benchmark->repo_url);
	git_vector_foreach(&benchmark->vec_refs, i, sz_i)
		git__free(sz_i);
	git__free(benchmark);
}

static int checkoutn_configure(
	gitbench_benchmark_checkoutn *benchmark,
	int argc,
	const char **argv)
{
	gitbench_opt_parser parser;
	gitbench_opt opt;

	gitbench_opt_parser_init(&parser, checkoutn_cmdline_opts, argv + 1, argc - 1);

	while (gitbench_opt_parser_next(&opt, &parser)) {
		if (!opt.spec) {
			git_vector_insert(&benchmark->vec_refs, git__strdup(opt.value));
		} else if (strcmp(opt.spec->name, "help") == 0) {
			gitbench_opt_usage_fprint(stderr, gitbench_globals.progname, checkoutn_cmdline_opts);
			return GITBENCH_EARGUMENTS;
		} else if (strcmp(opt.spec->name, "autocrlf") == 0) {
			benchmark->autocrlf = 1;
		} else if (strcmp(opt.spec->name, "repository") == 0) {
			benchmark->repo_url = git__strdup(opt.value);
			GITERR_CHECK_ALLOC(benchmark->repo_url);
		} else if (strcmp(opt.spec->name, "refs") == 0) {
			git_vector_insert(&benchmark->vec_refs,
							  git__strdup(opt.value));
		} else if (strcmp(opt.spec->name, "status") == 0) {
			char *end;
			long c = strtol(opt.value, &end, 10);
			if (c <= 0 || *end) {
				fprintf(stderr, "%s: invalid status count '%s'\n", gitbench_globals.progname, opt.value);
				gitbench_opt_usage_fprint(stderr, gitbench_globals.progname, checkoutn_cmdline_opts);
				return GITBENCH_EARGUMENTS;
			}
			benchmark->status_count = c;
		} else {
			fprintf(stderr, "%s: unknown argument: '%s'\n", gitbench_globals.progname, argv[parser.idx]);
			gitbench_opt_usage_fprint(stderr, gitbench_globals.progname, checkoutn_cmdline_opts);
			return GITBENCH_EARGUMENTS;
		}
	}

	if (benchmark->status_count == 0)
		benchmark->status_count = 1;

	/* vec_refs[0] is the initial checkout performed during the setup.
	 * vec_refs[1] is the first timed checkout.
	 * vec_refs[2..n] are optional.
	 */

	if (!benchmark->repo_url ||
		(git_vector_length(&benchmark->vec_refs) < 2)) {
		gitbench_opt_usage_fprint(stderr, gitbench_globals.progname, checkoutn_cmdline_opts);
		return GITBENCH_EARGUMENTS;
	}

	return 0;
}

int gitbench_benchmark_checkoutn_init(
	gitbench_benchmark **out,
	int argc,
	const char **argv)
{
	gitbench_benchmark_checkoutn *benchmark;
	int error;

	if ((benchmark = git__calloc(1, sizeof(gitbench_benchmark_checkoutn))) == NULL)
		return -1;

	benchmark->base.operation_cnt = CHECKOUTN_OPERATIONS_COUNT;
	benchmark->base.operations = checkoutn_operations;
	benchmark->base.run_fn = checkoutn_run;
	benchmark->base.free_fn = checkoutn_free;

	git_vector_init(&benchmark->vec_refs, 0, NULL);

	if ((error = checkoutn_configure(benchmark, argc, argv)) < 0) {
		checkoutn_free((gitbench_benchmark *)benchmark);
		return error;
	}

	*out = (gitbench_benchmark *)benchmark;
	return 0;
}

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

typedef struct gitbench_benchmark_merge {
	gitbench_benchmark base;

	char *repo_url;
	char *ref_name_checkout;
	char *ref_name_merge;

	unsigned int autocrlf:1;

	int status_count;

} gitbench_benchmark_merge;

enum merge_operation_t {
	MERGE_OPERATION_EXE_CLONE = 0,
	MERGE_OPERATION_EXE_CHECKOUT,
	MERGE_OPERATION_EXE_MERGE,
	MERGE_OPERATION_LG2_MERGE,
	MERGE_OPERATION_EXE_STATUS,
	MERGE_OPERATION_LG2_STATUS
};

/* descriptions must be 10 chars or less for column header alignment. */
static gitbench_operation_spec merge_operations[] = {
	{ MERGE_OPERATION_EXE_CLONE,    "ExeClone" },
	{ MERGE_OPERATION_EXE_CHECKOUT, "ExeCO" },
	{ MERGE_OPERATION_EXE_MERGE,    "ExeMerge" },
	{ MERGE_OPERATION_LG2_MERGE,    "Lg2Merge" },
	{ MERGE_OPERATION_EXE_STATUS,   "ExeStatus" },
	{ MERGE_OPERATION_LG2_STATUS,   "Lg2Status" },
};
#define MERGE_OPERATIONS_COUNT (sizeof(merge_operations) / sizeof(merge_operations[0]))

static const gitbench_opt_spec merge_cmdline_opts[] = {
	{ GITBENCH_OPT_SWITCH, "help",           0, NULL,      "display help",   GITBENCH_OPT_USAGE_HIDDEN },
	{ GITBENCH_OPT_SWITCH, "autocrlf",       0, NULL,      "turn on core.autocrlf=true" },
	{ GITBENCH_OPT_VALUE,  "ref_checkout", 'r', "refname", "the reference to checkout", GITBENCH_OPT_USAGE_REQUIRED | GITBENCH_OPT_USAGE_VALUE_REQUIRED },
	{ GITBENCH_OPT_VALUE,  "ref_merge",    'm', "refname", "the reference to merge in", GITBENCH_OPT_USAGE_REQUIRED | GITBENCH_OPT_USAGE_VALUE_REQUIRED },
	{ GITBENCH_OPT_VALUE,  "status",       's', "count",   "times to run status aftwards", GITBENCH_OPT_USAGE_VALUE_REQUIRED },
	{ GITBENCH_OPT_ARG,    "repository",     0, NULL,      "the repository",            GITBENCH_OPT_USAGE_REQUIRED },
	{ 0 }
};

static int _do_core_setup(
	git_buf *wd_path,
	gitbench_benchmark_merge *benchmark,
	gitbench_run *run)
{
	int error;

	if ((error = git_buf_joinpath(wd_path, run->tempdir, "wd")) < 0)
		return error;
	if ((error = git_futils_mkdir(wd_path->ptr, NULL, 0700, GIT_MKDIR_VERIFY_DIR)) < 0)
		return error;

	return error;
}

static int merge_run(gitbench_benchmark *b, gitbench_run *run)
{
	gitbench_benchmark_merge *benchmark = (gitbench_benchmark_merge *)b;
	git_buf wd_path = GIT_BUF_INIT;
	int error;

	if ((error = _do_core_setup(&wd_path, benchmark, run)) < 0)
		goto done;

	if ((error = gitbench_util_clone__exe(run, benchmark->repo_url, wd_path.ptr, GIT_CLONE_LOCAL, false, MERGE_OPERATION_EXE_CLONE)) < 0)
		goto done;

	if ((error = gitbench_util_set_autocrlf(wd_path.ptr, benchmark->autocrlf)) < 0)
		goto done;

	if ((error = gitbench_util_set_mergelimit(wd_path.ptr)) < 0)
		goto done;

	if ((error = gitbench_util_checkout__exe(run, wd_path.ptr, benchmark->ref_name_checkout, MERGE_OPERATION_EXE_CHECKOUT)) < 0)
		goto done;

	if (run->use_git_exe)
		error = gitbench_util_merge__exe(run, wd_path.ptr, benchmark->ref_name_merge, MERGE_OPERATION_EXE_MERGE);
	else
		error = gitbench_util_merge__lg2(run, wd_path.ptr, benchmark->ref_name_merge, MERGE_OPERATION_LG2_MERGE);
	if (error < 0)
		goto done;

	/* Always run both version of status since we can.
	 * Note that there is probably a minor penalty for
	 * being first here since that one may have to re-write
	 * the index. I'm going to average a few runs to smooth
	 * this out.
	 */
	if ((error = gitbench_util_status__exe(run, wd_path.ptr, MERGE_OPERATION_EXE_STATUS, benchmark->status_count)) < 0)
		goto done;
	if ((error = gitbench_util_status__lg2(run, wd_path.ptr, MERGE_OPERATION_LG2_STATUS, benchmark->status_count)) < 0)
		goto done;

done:
	git_buf_free(&wd_path);
	return error;
}

static void merge_free(gitbench_benchmark *b)
{
	gitbench_benchmark_merge *benchmark = (gitbench_benchmark_merge *)b;

	if (!b)
		return;

	git__free(benchmark->repo_url);
	git__free(benchmark->ref_name_checkout);
	git__free(benchmark->ref_name_merge);
	git__free(benchmark);
}

static int merge_configure(
	gitbench_benchmark_merge *benchmark,
	int argc,
	const char **argv)
{
	gitbench_opt_parser parser;
	gitbench_opt opt;

	gitbench_opt_parser_init(&parser, merge_cmdline_opts, argv + 1, argc - 1);

	while (gitbench_opt_parser_next(&opt, &parser)) {
		if (!opt.spec) {
			fprintf(stderr, "%s: unknown argument: '%s'\n", gitbench_globals.progname, argv[parser.idx]);
			gitbench_opt_usage_fprint(stderr, gitbench_globals.progname, merge_cmdline_opts);
			return GITBENCH_EARGUMENTS;
		}

		if (strcmp(opt.spec->name, "help") == 0) {
			gitbench_opt_usage_fprint(stderr, gitbench_globals.progname, merge_cmdline_opts);
			return GITBENCH_EARGUMENTS;
		} else if (strcmp(opt.spec->name, "autocrlf") == 0) {
			benchmark->autocrlf = 1;
		} else if (strcmp(opt.spec->name, "repository") == 0) {
			benchmark->repo_url = git__strdup(opt.value);
			GITERR_CHECK_ALLOC(benchmark->repo_url);
		} else if (strcmp(opt.spec->name, "ref_checkout") == 0) {
			benchmark->ref_name_checkout = git__strdup(opt.value);
			GITERR_CHECK_ALLOC(benchmark->ref_name_checkout);
		} else if (strcmp(opt.spec->name, "ref_merge") == 0) {
			benchmark->ref_name_merge = git__strdup(opt.value);
			GITERR_CHECK_ALLOC(benchmark->ref_name_merge);
		} else if (strcmp(opt.spec->name, "status") == 0) {
			char *end;
			long c = strtol(opt.value, &end, 10);
			if (c <= 0 || *end) {
				fprintf(stderr, "%s: invalid status count '%s'\n", gitbench_globals.progname, opt.value);
				gitbench_opt_usage_fprint(stderr, gitbench_globals.progname, merge_cmdline_opts);
				return GITBENCH_EARGUMENTS;
			}
			benchmark->status_count = c;
		}
	}

	if (benchmark->status_count == 0)
		benchmark->status_count = 1;

	if (!benchmark->repo_url ||
		!benchmark->ref_name_checkout ||
		!benchmark->ref_name_merge) {
		gitbench_opt_usage_fprint(stderr, gitbench_globals.progname, merge_cmdline_opts);
		return GITBENCH_EARGUMENTS;
	}

	return 0;
}

int gitbench_benchmark_merge_init(
	gitbench_benchmark **out,
	int argc,
	const char **argv)
{
	gitbench_benchmark_merge *benchmark;
	int error;

	if ((benchmark = git__calloc(1, sizeof(gitbench_benchmark_merge))) == NULL)
		return -1;

	benchmark->base.operation_cnt = MERGE_OPERATIONS_COUNT;
	benchmark->base.operations = merge_operations;
	benchmark->base.run_fn = merge_run;
	benchmark->base.free_fn = merge_free;

	if ((error = merge_configure(benchmark, argc, argv)) < 0) {
		merge_free((gitbench_benchmark *)benchmark);
		return error;
	}

	*out = (gitbench_benchmark *)benchmark;
	return 0;
}

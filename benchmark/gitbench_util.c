/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */

#include <fcntl.h>
#include "common.h"
#include "fileops.h"
#include "buffer.h"
#include "gitbench_globals.h"
#include "gitbench_run.h"
#include "gitbench_util.h"
#include "gitbench_opt.h"
#include "gitbench_shell.h"
#include "gitbench_operation.h"
#include "gitbench_benchmark.h"

/**
 * Does the given path point to a directory where
 * we can create a sandbox?  (That is, the parent
 * directory of our sandbox directory.)
 */
static int tempdir_isvalid(const char *path)
{
	struct stat st;

	if (p_stat(path, &st) != 0)
		return 0;

	if (!S_ISDIR(st.st_mode))
		return 0;

	return (access(path, W_OK) == 0);
}

/**
 * Use environment variables and/or system defaults
 * to select the directory where we should create a
 * sandbox.
 */
static int tempdir_choose(git_buf *pbuf_tempdir)
{
	static const char *env_var_names[] = {
		"GITBENCH_TEMP",
		"GITBENCH_TMP",
		"TMPDIR",
		"TEMP",
		"TMP",
		"USERPROFILE",
		NULL
	};
	int k;

	git_buf_init(pbuf_tempdir, 0);

	for (k=0; (env_var_names[k] != NULL); k++) {
		const char *env_var_value = getenv(env_var_names[k]);
		if (env_var_value && (tempdir_isvalid(env_var_value))) {
			git_buf_sets(pbuf_tempdir, env_var_value);
			return 0;
		}
	}

#ifdef _WIN32
	{
		char buffer[MAX_PATH];
		DWORD len;

		if ((len = GetTempPath((DWORD)sizeof(buffer), buffer)) > 0) {
			git_buf_sets(pbuf_tempdir, buffer);
			return 0;
		}
	}
#else
	{
		if (tempdir_isvalid("/tmp")) {
			get_buf_sets(pbuf_tempdir, "/tmp");
			return 0;
		}
	}
#endif

	giterr_set(GITERR_OS, "Unable to select tempdir for benchmarks");
	return -1;
}

/**
 * Create a "sandbox" directory within a tempdir.
 * That is, something of the form "<tempdir>/libgit2_gitbench_<X>".
 */
int gitbench_util_sandbox_create(git_buf *pbuf_sandbox)
{
	git_buf buf_tempdir = GIT_BUF_INIT;
	static const char *tempname = "libgit2_gitbench_XXXXXX";
	int error;

	if ((error = tempdir_choose(&buf_tempdir)) < 0)
		goto done;

#ifdef GIT_WIN32
	git_path_mkposix(buf_tempdir.ptr);
#endif

	if ((error = git_buf_joinpath(pbuf_sandbox, buf_tempdir.ptr, tempname)) < 0)
		goto done;

#ifdef GIT_WIN32
	if (_mktemp_s(pbuf_sandbox->ptr, pbuf_sandbox->size+1) != 0 ||
		p_mkdir(pbuf_sandbox->ptr, 0700) != 0) {
		error = -1;
		goto done;
	}
#else
	if (mkdtemp(pbuf_sandbox->ptr) == NULL) {
		error = -1;
		goto done;
	}
#endif

done:
	git_buf_free(&buf_tempdir);
	if (error != 0)
		git_buf_free(pbuf_sandbox);
	return error;
}




int gitbench_util_set_autocrlf(const char *wd, bool autocrlf)
{
	const char * argv[10] = {0};
	int k;
	int error;

	k = 0;
	argv[k++] = BM_GIT_EXE;
	argv[k++] = "config";
	argv[k++] = "core.autocrlf";
	argv[k++] = ((autocrlf) ? "true" : "false");
	argv[k++] = 0;

	error = gitbench_shell(argv, wd, NULL);
	return error;
}

int gitbench_util_set_mergelimit(const char *wd)
{
	const char * argv[10] = {0};
	int k;
	int error;

	k = 0;
	argv[k++] = BM_GIT_EXE;
	argv[k++] = "config";
	argv[k++] = "merge.renameLimit";
	argv[k++] = "999999";
	argv[k++] = 0;

	error = gitbench_shell(argv, wd, NULL);
	return error;
}


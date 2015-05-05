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

/**
 * Supply credentials for the call to git_clone().
 * We use values from the optional environment variables.
 *
 * I don't like using the environment like this, but I
 * don't want to hook up a credential helper (which may
 * still prompt the user).
 *
 * This is only used by the libgit2 code; we don't
 * control what git.exe will do -- so to have a
 * fully automated test, you'll need to address
 * that separately.
 */
static int cred_cb(
	git_cred **cred,
	const char *url,
	const char *username_from_url,
	unsigned int allowed_types,
	void *payload)
{
	const char *user;
	const char *pass;

	GIT_UNUSED(url);
	GIT_UNUSED(allowed_types);
	GIT_UNUSED(payload);

	if (username_from_url)
		user = username_from_url;
	else
		user = getenv("GITBENCH_USERNAME");

	pass = getenv("GITBENCH_PASSWORD");

	return git_cred_userpass_plaintext_new(cred, user, pass);
}

int gitbench_util_clone__lg2(
	gitbench_run *run,
	const char *url,
	const char *wd,
	git_clone_local_t local,
	bool bare,
	int operation_id)
{
	git_repository *repo = NULL;
	git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
	git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
	git_remote_callbacks remote_callbacks = GIT_REMOTE_CALLBACKS_INIT;
	int error;

	checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;

	remote_callbacks.credentials = cred_cb;
	remote_callbacks.payload = NULL;

	clone_opts.checkout_opts = checkout_opts;
	clone_opts.remote_callbacks = remote_callbacks;
	clone_opts.bare = bare;
	clone_opts.local = local;

	gitbench_run_start_operation(run, operation_id);
	error = git_clone(&repo, url, wd, &clone_opts);
	gitbench_run_finish_operation(run);

	git_repository_free(repo);
	return error;
}

int gitbench_util_clone__exe(
	gitbench_run *run,
	const char *url,
	const char *wd,
	git_clone_local_t local,
	bool bare,
	int operation_id)
{
	const char * argv[10] = {0};
	int k;
	int error;

	k = 0;
	argv[k++] = BM_GIT_EXE;
	argv[k++] = "clone";
	argv[k++] = "--quiet";

	if (bare)
		argv[k++] = "--bare";
	else
		argv[k++] = "--no-checkout";

	if (local == GIT_CLONE_LOCAL)
		argv[k++] = "--local";
	else if (local == GIT_CLONE_NO_LOCAL)
		argv[k++] = "--no-local";
	else if (local == GIT_CLONE_LOCAL_NO_LINKS)
		argv[k++] = "--no-hardlinks";

	argv[k++] = url;
	argv[k++] = wd;
	argv[k++] = 0;

	gitbench_run_start_operation(run, operation_id);
	error = gitbench_shell(argv, NULL, NULL);
	gitbench_run_finish_operation(run);

	return error;
}

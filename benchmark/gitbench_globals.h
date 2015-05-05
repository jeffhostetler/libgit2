/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */

#ifndef H_GITBENCH_GLOBALS_H
#define H_GITBENCH_GLOBALS_H

struct gitbench_globals
{
	FILE *logfile;
	const char *progname;
	int verbosity;
};

extern struct gitbench_globals gitbench_globals;

#endif
